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
 * Timer.cpp
 *
 *  Created on: Oct 3, 2017
 *      Author: lia1hc
 */
#include <stdint.h>
#include <iostream>

#include "Mutex.h"
#include "Condition.h"
#include "EventAgent.h"
#include "Timer.h"
#include "TimeUtils.h"

TimerManager *TimerManager::mSingleton = NULL;

TimerManager::TimerManager() :
         mMutex( true ), mTicks( 0 )
{

   if ( mSingleton != NULL )
      std::cout << "Don't create a timer manager use getInstance()"
               << std::endl;

   mClockThread = new
   Runnable<TimerManager>( "clockThread", this, &TimerManager::clockHandler );
   // ignore return value
   (void) mClockThread->Start();
}

TimerManager::~TimerManager()
{

   mClockThread->Stop();
   // ignore return value
   (void) mClockThread->Join();

   delete mClockThread;
}

TimerManager *TimerManager::getInstance()
{
   if ( NULL == mSingleton ) {
      mSingleton = new TimerManager;
   }
   return mSingleton;
}

void TimerManager::destroyManager()
{
   if ( mSingleton != NULL ) {
      delete mSingleton;
      mSingleton = NULL;
   }
}

void TimerManager::clockHandler()
{
   struct timespec start, cur;
   Condition c;
   Mutex m;
   AutoLock l( m );

   TimeUtils::getCurTime( &start );
   while ( !mClockThread->CheckStop() ) {
      // Wait for the tick to timeout
      if ( !c.Wait( m, kMsPerTick ) ) {
         TimeUtils::getCurTime( &cur );

         // Check to see if the clock was set back, or jumps
         // forward by more than 10 seconds. If so, update the
         // starting time to the current time and wait for the next
         // tick.
         if ( TimeUtils::getDifference( &cur, &start ) < 0
                  || TimeUtils::getDifference( &cur, &start ) > 10000 ) {
            TimeUtils::getCurTime( &start );
            handleTick();
         } else {
            // Get the current time and count ticks until cur time and the
            //  actual tick time are less the kMsPerTick apart.
            while ( TimeUtils::getDifference( &cur, &start ) > kMsPerTick ) {
               TimeUtils::addOffset( &start, kMsPerTick );
               handleTick();
            }
         }
      } else {
         std::cout << "Signal received on timer condition?\n";
      }
   }
}

void TimerManager::handleTick()
{

   DebugAutoLock( mMutex );

   mTicks++;

   printf( "%d ticks, %lu items in list\n", mTicks, mList.size() );

   while ( !mList.empty() ) {
      // Make a copy of the TimerNode in the first spot in the list
      TimerNode timer = *mList.begin();

      int32_t diff = timer.mTick - mTicks;

      // List is sorted with nearest items first.  Once we find a node
      // that is in the future we are done.
      if ( diff > 0 )
         break;

      // Remove the timer from the list now
      mList.pop_front();

      // If the timer doesn't have an event then we call the listener
      if ( NULL == timer.mEvent ) {
         if ( timer.mListener != NULL ) {
            timer.mListener->onTimeout( timer.mPrivateData );
         }
      }
      // Otherwise send the event
      else {
         if ( timer.mRepeatMS == 0 ) {
            // Send the event to the specified dispatcher and
            // release our reference to the event
            timer.mDispatcher->sendEvent( timer.mEvent );

            timer.mEvent = NULL;
         } else {
            // Send the periodic event
            timer.mDispatcher->sendEvent( timer.mEvent );
         }
      }

      // We need to re-send it in a few ticks
      if ( timer.mRepeatMS != 0 ) {
         unsigned newTicks = ( timer.mRepeatMS + ( kMsPerTick - 1 )
                  - timer.mRemainingMS ) / kMsPerTick;
         timer.mTick += newTicks;
         timer.mRemainingMS = ( timer.mRepeatMS + timer.mRemainingMS )
                  % kMsPerTick;
         addTimerNode( timer );
      }
   }
}

void TimerManager::addTimer( TimerListener *listener, uint32_t msecs,
         uint32_t private_data )
{

   DebugAutoLock( mMutex );

   if ( listener == NULL )
      abort();

   // Calculate the timeout time in ticks
   uint32_t ticks = ( msecs + kMsPerTick - 1 ) / kMsPerTick;

   // Initialize new timer node
   TimerNode timer;
   timer.mEvent = NULL;
   timer.mDispatcher = NULL;
   timer.mPrivateData = private_data;
   timer.mListener = listener;
   timer.mTick = mTicks + ticks;
   timer.mRepeatMS = 0;
   timer.mRemainingMS = 0;

   printf( "timer at %d ticks\n", timer.mTick );

   addTimerNode( timer );
}

void TimerManager::sendTimedEvent( Event *event, IEventDispatcher *dispatcher,
         uint32_t msecs )
{

   DebugAutoLock( mMutex );

   // Calculate the timeout time in ticks
   uint32_t ticks = ( msecs + kMsPerTick - 1 ) / kMsPerTick;

   // Initialize new timer node
   TimerNode timer;
   timer.mEvent = event;
   timer.mDispatcher = dispatcher;
   timer.mPrivateData = 0;
   timer.mListener = NULL;
   timer.mTick = mTicks + ticks;
   timer.mRepeatMS = 0;
   timer.mRemainingMS = 0;

   printf( "timer at %d ticks\n", timer.mTick );

   addTimerNode( timer );
}

void TimerManager::sendPeriodicEvent( Event *event,
         IEventDispatcher *dispatcher, uint32_t period )
{

   DebugAutoLock( mMutex );

   // Calculate the timeout time in ticks
   uint32_t ticks = ( period + kMsPerTick - 1 ) / kMsPerTick;

   // Initialize new timer node
   TimerNode timer;
   timer.mEvent = event;
   timer.mDispatcher = dispatcher;
   timer.mPrivateData = 0;
   timer.mListener = NULL;
   timer.mTick = mTicks + ticks;
   timer.mRepeatMS = period;
   timer.mRemainingMS = 0;

   printf( "timer at %d ticks\n", timer.mTick );

   addTimerNode( timer );
}

void TimerManager::removeTimedEvent( Event *ev )
{

   DebugAutoLock( mMutex );

   std::list<TimerNode>::iterator node = mList.begin();
   while ( node != mList.end() ) {
      // Get reference to the current TimerNode
      TimerNode& timer = *node;

      // Check if the timer node has an event for the dispatcher
      // specified
      if ( (Event*) timer.mEvent == ev ) {
         node = mList.erase( node );
         continue;
      }
      ++node;
   }
}

void TimerManager::removeAgentsByReceiver( void* receiver,
         IEventDispatcher* dispatcher )
{

   DebugAutoLock( mMutex );

   std::list<TimerNode>::iterator i = mList.begin();
   while ( i != mList.end() ) {
      // Get reference to the current TimerNode
      TimerNode& timer = *i;

      if ( ( timer.mEvent != NULL )
               && timer.mEvent->getEventId() == Event::kAgentEventId
               && timer.mDispatcher == dispatcher ) {
         std::cout << "Found an event agent, looking more closely" << std::endl;
         // dynamic_cast to downcast polymorphic type
         EventAgent* agent = dynamic_cast<EventAgent*>( (Event*) timer.mEvent );
         if ( agent != NULL && agent->getDeliveryTarget() == receiver ) {
            std::cout << "Found a match, removing" << std::endl;
            i = mList.erase( i );
            continue;
         }
      }
      ++i;
   }
}

void TimerManager::removeTimedEvent( Event::Id eventId,
         IEventDispatcher *dispatcher )
{

   DebugAutoLock( mMutex );

   std::list<TimerNode>::iterator i = mList.begin();
   while ( i != mList.end() ) {
      // Get reference to the current TimerNode
      TimerNode& timer = *i;

      // Check if the timer node has an event for the dispatcher
      // specified
      if ( timer.mDispatcher == dispatcher && timer.mEvent != NULL ) {
         // Now test if the timer node event has a matching id or if the
         // id is Event::kInvalidEventId.  Event::kInvalidEventId is a
         // special id used to remove all events for a dispatcher
         if ( timer.mEvent->getEventId() == eventId
                  || eventId == Event::kInvalidEventId ) {
            i = mList.erase( i );
            continue;
         }
      }
      ++i;
   }

}

void TimerManager::addTimerNode( const TimerNode& newTimer )
{

   int32_t diff;
   bool inserted = false;

   // Add new timer node to the list (sorted)
   for (std::list<TimerNode>::iterator i = mList.begin(); i != mList.end();
            ++i) {
      // Get reference to the current TimerNode
      TimerNode& timer = *i;
      diff = newTimer.mTick - timer.mTick;

      if ( diff < 0 ) {
         // insert before
         (void) mList.insert( i, newTimer );
         inserted = true;
         break;
      }
   }

   if ( !inserted ) {
      mList.push_back( newTimer );
   }
}
