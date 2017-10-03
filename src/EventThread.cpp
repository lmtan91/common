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
 * EventThread.cpp
 *
 *  Created on: Oct 3, 2017
 *      Author: lia1hc
 */

#include "EventThread.h"
#include "Timer.h"

EventThread::EventThread( const char *name ) :
         mThread( name == NULL ? "EventThread" : name, this,
                  &EventThread::threadMain )
{
   // Ignore return value.
   (void) mThread.Start();
}

EventThread::~EventThread()
{
   EventDispatcher::sendEvent(
            new Event( Event::kShutdownEventId, PRIORITY_HIGH ) );
   printf( "Wait for thread to die\n" );
   // Ignore return value.
   (void) mThread.Join();
}

void EventThread::threadMain()
{
   Event *ev = NULL;
   bool done = false;

   while ( !done ) {
      printf( "Waiting Event\n" );

      ev = mQueue.WaitEvent();

      printf( "Got Event\n" );

      if ( ev == NULL )
         printf( "got a null event\n" );

      // not needed, at this time.
      //AutoLock a( mLock );

      done = handleEvent( ev );
   }
}
