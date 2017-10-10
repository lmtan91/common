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

   if ( mList.size() > 0 ) {
      while ( mList.size() > 0 ) {
         std::list<ListenerNode*>::iterator i = mList.begin();
         removeListener( ( *i )->mFd, ( *i )->mListener );
      }
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
}
