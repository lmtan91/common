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
#include "logging.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOTICE );
Selector::Selector( const char *name ) :
         mLock( true ), mThread( NULL == name ? "Selector" : name, this,
                  &Selector::threadMain ), mUpdateFds( false )
{
   TRACE_BEGIN(LOG_LVL_INFO);
   int ret = pipe( mPipe );
   LOG( "pipe reader %d writer %d", mPipe[ PIPE_READER ], mPipe[ PIPE_WRITER ] );
   if ( ret != 0 ) {
      LOG_ERR_FATAL( "failed to create pipe" );
   }

   mRunning = true;
   mThread.Start();
   mShutdown = false;
}

Selector::~Selector()
{
   TRACE_BEGIN( LOG_LVL_INFO );

   shutdown();

   LOG( "Closing pipes" );
   close( mPipe[ PIPE_WRITER ] );
   close( mPipe[ PIPE_READER ] );

   if ( mThread == *Thread::GetCurrent() ) {
      LOG_ERR_FATAL( "A selector MUST NOT be deleted by its own thread" );
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
         LOG_WARN( "%s declining to call Join() on %s",
                  Thread::GetCurrent()->GetName(), mThread.GetName() );
      }
      else {
         LOG_NOISE( "%s waiting for thread %s to die",
                  Thread::GetCurrent()->GetName(), mThread.GetName() );
         mThread.Stop();
         (void) mThread.Join();
      }
   }
}

void Selector::addListener( int fd, short events, SelectorListener *listener,
         uint32_t pri_data )
{
   TRACE_BEGIN( LOG_LVL_INFO );

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

   LOG( "added fd %d events %x, list size %ld", fd, events, mList.size() );
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
   TRACE_BEGIN( LOG_LVL_INFO );
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
   TRACE_BEGIN( LOG_LVL_INFO );
   bool gotEvent = false;
   struct pollfd fds[ kMaxPollFds ];
   int numFds = 0;

   fillPollFds( fds, numFds );

   while ( mRunning ) {
      LOG( "%p on %d files", this, numFds );
      for (int i = 1; i < numFds; i++) {
         LOG( "%p fd %d", this, fds[ i ].fd );
      }

      errno = 0;
      int ret = 0;

      if ( ( ret = poll( fds, numFds, -1 ) ) < 0 ) {
         if ( 0 == errno ) {
            LOG_NOTICE( "Poll returned %d, but didn't set errno", ret );
         }
         else if ( errno == EINTR ) {
            LOG_NOISE( "Poll was interrupted" );
         }
         else {
            LOG_ERR_PERROR( "Poll returned %d", ret );
         }
      }

      LOG( "%p woke up %d", this, ret );

      if ( ret > 0 ) {
         LOG( "got %d from poll\n", ret );

         for (int i = 0; i < numFds; i++) {
            if ( fds[ i ].fd == mPipe[ PIPE_READER ] ) {
               LOG( "got %x on pipe %d", fds[ 0 ].revents, fds[ i ].fd );
               if ( fds[ i ].revents & POLLIN ) {
                  char buf[ 10 ];
                  if ( read( mPipe[PIPE_READER], buf, 4 ) ) {
                  }

                  // We need to handle events after we handle file
                  // descriptor polls because one of the events
                  // that we handle modifies the current list of
                  // file descriptors for poll and we need to handle
                  // any that occured before updating them.
                  gotEvent = true;
               }
               else if ( fds[ i ].revents & ( POLLHUP | POLLNVAL ) ) {
                  LOG_ERR_FATAL( "POLLHUP recieved on pipe" );
               }
            }
            else {
               LOG_NOISE( "got %x on fd %d", fds[ i ].revents, fds[ i ].fd );
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
            LOG_WARN( "got NULL event" );
         }
         else if ( ev->getEventId() == Event::kSelectorUpdateEventId ) {
            LOG( "got kSelectorUpdateEventId" );
            mUpdateFds = true;
            ev->Release();
         } else {
            LOG( "got event %d", ev->getEventId() );
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
   LOG_NOTICE( "Thread exiting" );
}

void Selector::fillPollFds( struct pollfd *fds, int &numFds )
{
   AutoLock m( mLock );
   TRACE_BEGIN( LOG_LVL_INFO );

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
               LOG_ERR( "Max listeners reached, dropping listener" );
            }
            else {
               fds[ numFds ].fd = ( *i )->mFd;
               fds[ numFds ].events = ( *i )->mEvents;
               ++numFds;
            }

            // This is just a warning for debug purposes.   When we
            // reach the "Warn" point this message will be printed
            if ( numFds == kWarnPollFds ) {
               LOG_WARN( "Listeners exceeding warning limit" );
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
      LOG_NOISE( "file entry %d: fd %d events %x", i, fds[ i ].fd, fds[ i ].events );
   }

   LOG( "poll on %d fds, size %ld", numFds, mList.size() );

   mCondition.Broadcast();
}

bool Selector::callListeners( int fd, uint32_t events )
{
   AutoLock l( mLock );
   TRACE_BEGIN( LOG_LVL_INFO );

   ListenerNode match( fd, NULL );
   bool result = false;

   std::list<ListenerNode*>::iterator node = mList.begin();
   while ( node != mList.end() ) {
      if ( ListenerNode::kStateListening != ( *node )->mState ) {

      }
      else if ( *( *node ) == match ) {
         LOG( "got event %x %p", events, *node );

         SelectorListener *interface = ( *node )->mListener;
         uint32_t pd = ( *node )->mPrivateData;

         // If we receive a POLLHUP or a POLLNVAL we are going to
         // automatically mark the ListenerNode for removal.  This
         // prevents us from getting into a tight loop in the case
         // that the user neglects to respond to the poll event by
         // removing their listener.
         if ( events & ( POLLHUP | POLLNVAL ) ) {
            if ( events & POLLHUP ) {
               LOG_INFO( "POLLHUP recieved on fd = %d (%p)", fd, *node );
            }

            if ( events & POLLNVAL ) {
               LOG_WARN( "POLLNVAL recieved on fd = %d (%p)", fd, *node );
            }

            // Update state to "Removing" and set the result to true
            // to indicate that we have made listener updates
            // The node will be cleared in the fillPollFds method.
            ( *node )->mState = ListenerNode::kStateRemoving;
            result = true;
         }

         // Now perform our callback
         if ( interface != NULL ) {
            LOG_NOISE( "eventsCallback %p %d %d", interface, events, fd );
            interface->processFileEvents( fd, events, pd );
            LOG_NOISE( "eventsCallback done" );
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
   int ret = write( mPipe[PIPE_WRITER], &buf, 4 );

   if ( ret != 4 ) {
      LOG_ERR( "write to pipe failed %d", ret );
   }
}
