/*==============================================================================
 * @Company
 * @Copyright
 * UNPUBLISHED WORK
 * ALL RIGHTS RESERVED
 *
 * This software is the confidential and proprietary information of
 *
 *
 *
 *
 *
 *============================================================================*/

/*
 * Selector.cpp
 *
 *  Created on: Oct 10, 2017
 *      Author: lia1hc
 */

#include <unistd.h>
#include <fcntl.h>

#include "Selector.h"

Selector::Selector( const char *name ) :
         mLock( true ), mThread( NULL == name ? "Selector" : name, this,
                  &Selector::threadMain ), mUpdateFds( false )
{
   int ret = pipe( mPipe );
   printf( "pipe reader %d writer %d\n", mPipe[ PIPE_READER ],
            mPipe[ PIPE_WRITER ] );
   if ( ret != 0 ) {
      printf( "failed to create pipe\n" );
   }

   mRunning = true;
   mThread.Start();
   mShutdown = false;
}

Selector::~Selector()
{
   shutdown();

   close( mPipe[ PIPE_WRITER ] );
   close( mPipe[ PIPE_READER ] );

   if ( mThread == *Thread::GetCurrent() ) {
      printf( "A selector MUST NOT be deleted by its own thread\n" );
   }

   // remove all listeners.
   while ( mList.size() > 0 ) {
      std::list<ListenerNode*>::iterator i = mList.begin();
      removeListener( ( *i )->mFd, ( *i )->mListener );
   }
}

void Selector::shutdown()
{
   if ( !mShutdown ) {
      mShutdown = true;

      EventDispatcher::sendEvent(
               new Event( Event::kShutdownEventId, PRIORITY_HIGH ) );

      if ( mThread == *Thread::GetCurrent() ) {
         printf( "%s declining to call Join() on %s\n",
                  Thread::GetCurrent()->GetName(), mThread.GetName() );
      }
      else {
         printf( "%s waiting for thread %s to die\n",
                  Thread::GetCurrent()->GetName(), mThread.GetName() );
         mThread.Stop();
         mThread.Join();
      }
   }
}

void Selector::addListener( int fd, short events, SelectorListener *listener,
         uint32_t pri_data )
{
   ListenerNode *node = new ListenerNode();

   node->mFd = fd;
   node->mEvents = events;
   node->mListener = listener;
   node->mPrivateData = pri_data;

   // Set initial state to "Adding" so that this listener will not called until
   // it has been processed by fillPollFds.
   node->mState = ListenerNode::kStateAdding;

   AutoLock l( mLock );

   mList.push_back( node );
   updateListeners();

   printf( "added fd %d events %x, list size %ld\n", fd, events, mList.size() );
}

void Selector::removeListener( int fd, SelectorListener *listener )
{
   AutoLock l( mLock );
   ListenerNode n( fd, listener );
   bool update = false;

   std::list<ListenerNode*>::iterator i = mList.begin();
   while ( i != mList.end() ) {
      if ( n == *( *i ) ) {

         // Set update to true and update state to "Removing" so that this
         // listener will NOT called again after this returns. The node will be
         // clear in the fillPollFds method.
         update = true;
         ( *i )->mState = ListenerNode::kStateRemoving;
      }
      ++i;
   }

   if ( update ) {
      updateListeners();
   }
}

Selector::ListenerNode *Selector::findListener( int fd,
         SelectorListener *listener )
{
   ListenerNode n( fd, listener );
   for (std::list<ListenerNode*>::iterator i = mList.begin(); i != mList.end();
            ++i) {
      if ( n == *( *i ) ) {
         return *i;
      }
   }

   return NULL;
}

void Selector::updateListeners()
{
   // if called from the selector, just set a flag to update the Fds list
   // otherwise send an event to update the Fds.  But we also need to wait
   // for this change to be made effective.  Otherwise we could get events
   // for old Fd on a new listener if the same Fd was used for the new file.
   if ( *Thread::GetCurrent() == mThread ) {
      mUpdateFds = true;
   } else if ( mRunning ) {
      Event *ev = new Event( Event::kSelectorUpdateEventId );
      sendEvent( ev );
      mCondition.Wait( mLock );
   }
   // else threadMain has exited or is exiting so no nothing we are ending.
}

void Selector::threadMain()
{
   bool gotEvent = false;
   struct pollfd fds[ kMaxPollFds ];
   int numFds = 0;

   while ( mRunning ) {
      printf( "%p on %d files\n", this, numFds );
      for (int i = 1; i < numFds; i++) {
         printf( "%p fd %d\n", this, fds[ i ].fd );
      }

      errno = 0;
      int ret = 0;

      if ( ( ret = poll( fds, numFds, -1 ) ) < 0 ) {
         if ( 0 == errno ) {
            printf( "Poll returned %d, but didn't set errno\n", ret );
         }
         else if ( errno == EINTR ) {
            printf( "Poll was interrupted\n" );
         }
         else {
            printf( "Poll returned %d\n", ret );
         }
      }

      printf( "%p woke up %d", this, ret );

      if ( ret > 0 ) {
         printf( "got %d from poll", ret );

         for (int i = 0; i < numFds; i++) {
            if ( fds[ i ].fd == mPipe[ PIPE_READER ] ) {
               printf( "got %x on pipe %d", fds[ 0 ].revents, fds[ i ].fd );

               if ( fds[ i ].revents & POLLIN ) {
                  char buf[ 10 ];
                  read( mPipe[ PIPE_READER ], &buf, 4 );

                  // We need to handle events after we handle file
                  // descriptor polls because one of the events
                  // that we handle modifies the current list of
                  // file descriptors for poll and we need to handle
                  // any that occured before updating them.
                  gotEvent = true;
               }
               else if ( fds[ i ].revents & ( POLLHUP | POLLNVAL ) ) {
                  printf( "POLLHUP recieved on pipe\n" );
               }
            }
            else {
               printf( "got %x on fd %d\n", fds[ i ].revents, fds[ i ].fd );
               if ( fds[ i ].revents != 0 ) {
                  // if callListeners removes a listener we need to update
                  // Fds.  However only set if callListeners returns true
                  // This should not be cleared since one of the listeners
                  // could have called removeListener and that call might
                  // have set mUpdateFds
                  if ( callListeners( fds[ i ].fd, fds[ i ].revents ) ) {
                     mUpdateFds = true;
                  }
               }
            }
         }
      }

      // Now that file descriptors have been handled we can deal with
      // events if needed, including updating the poll file descriptor
      // list if it's been changed.
      if ( gotEvent ) {
         Event *ev = mQueue.PollEvent();
         if ( NULL == ev ) {
            printf( "got NULL event\n" );
         }
         else if ( ev->getEventId() == Event::kSelectorUpdateEventId ) {
            printf( "got kSelectorUpdateEventId\n" );
            mUpdateFds = true;
            ev->Release();
         } else {
            printf( "got event %d", ev->getEventId() );
            bool done = EventDispatcher::handleEvent( ev );
            if ( done ) {
               mRunning = false;
            }
         }
         gotEvent = false;
      }

      if ( mUpdateFds ) {
         fillPollFds( fds, numFds );
         mUpdateFds = false;
      }
   }
   printf( "Thread exiting\n" );
}

void Selector::fillPollFds( struct pollfd *fds, int &numFds )
{
   AutoLock m( mLock );

   fds[ 0 ].fd = mPipe[ PIPE_READER ];
   fds[ 0 ].events = POLLIN;

   numFds = 1;

   std::list<ListenerNode*>::iterator i = mList.begin();
   while ( i != mList.end() ) {
      // If the current node is marked as removal then remove it.
      if ( ListenerNode::kStateRemoving == ( *i )->mState ) {
         delete *i;
         i = mList.erase( i );
      }
      else {
         bool needAdd = true;

         // Loop to try to find if this ListenerNode has a duplicate
         // fd to one already added to the array.   Start at index
         // 1 because index 0 is reserved for the internal pipe
         for (int j = 1; j < numFds; j++) {
            if ( fds[ j ].fd == ( *i )->mFd ) {
               fds[ j ].events |= ( *i )->mEvents;
               needAdd = false;
               break;
            }
         }

         // If we didn't find that this is a duplicate then we
         // need to add an entry in the pollfds array and update
         // the numFds value
         if ( needAdd ) {
            // If we reach the maximum size of the pollfds array
            // then we are just going to ignore attempts to add
            // further listener nodes
            if ( kMaxPollFds == numFds ) {
               printf( "Max listeners reached, dropping listener\n" );
            }
            else {
               fds[ numFds ].fd = ( *i )->mFd;
               fds[ numFds ].events = ( *i )->mEvents;
               ++numFds;
            }

            // This is just a warning for debug purposes.   When we
            // reach the "Warn" point this message will be printed
            if ( numFds == kWarnPollFds ) {
               printf( "Listeners exceeding warning limit\n" );
            }
         }

         // All done with this ListenerNode, update state to "Listening" and
         // move to the next.
         ( *i )->mState = ListenerNode::kStateListening;
         ++i;
      }
   }

   // Debug
   for (int i = 0; i < numFds; i++) {
      printf( "file entry %d: fd %d events %x\n", i, fds[ i ].fd,
               fds[ i ].events );
   }

   printf( "poll on %d fds, size %ld\n", numFds, mList.size() );

   mCondition.Broadcast();
}

bool Selector::callListeners( int fd, uint32_t events )
{
   AutoLock l( mLock );
   ListenerNode match( fd, NULL );
   bool result = false;

   std::list<ListenerNode*>::iterator node = mList.begin();
   while ( node != mList.end() ) {
      if ( ListenerNode::kStateListening != ( *node )->mState ) {

      }
      else if ( *( *node ) == match ) {
         printf( "got event %x %p\n", events, *node );

         SelectorListener *interface = ( *node )->mListener;
         uint32_t pd = ( *node )->mPrivateData;

         // If we receive a POLLHUP or a POLLNVAL we are going to
         // automatically mark the ListenerNode for removal.  This
         // prevents us from getting into a tight loop in the case
         // that the user neglects to respond to the poll event by
         // removing their listener.
         if ( events & ( POLLHUP | POLLNVAL ) ) {
            if ( events & POLLHUP ) {
               printf( "POLLHUP recieved on fd = %d (%p)\n", fd, *node );
            }

            if ( events & POLLNVAL ) {
               printf( "POLLNVAL recieved on fd = %d (%p)\n", fd, *node );
            }

            // Update state to "Removing" and set the result to true
            // to indicate that we have made listener updates
            // The node will be cleared in the fillPollFds method.
            ( *node )->mState = ListenerNode::kStateRemoving;
            result = true;
         }

         // Now perform our callback
         if ( interface != NULL ) {
            printf( "eventsCallback %p %d %d\n", interface, events, fd );
            interface->processFileEvents( fd, events, pd );
            printf( "eventsCallback done\n" );
         }
      }
      ++node;
   }

   return result;
}

const Thread *Selector::getDispatcherThread()
{
   return &mThread;
}

void Selector::wakeThread()
{
   char buf[] = "EVNT";
   int ret = write( mPipe[ PIPE_WRITER ], &buf, 4 );

   if ( ret != 4 ) {
      printf( "write to pipe failed %d\n", ret );
   }
}
