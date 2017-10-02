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
 * EventQueue.cpp
 *
 *  Created on: Oct 2, 2017
 *      Author: lia1hc
 */
#include "EventQueue.h"
#include "Mutex.h"
#include "EventAgent.h"

EventQueue::EventQueue() :
         mLock( "EventQueue" )
{
}

EventQueue::~EventQueue()
{
}

//TODO test print all event with priority
void EventQueue::SendEvent( Event *ev )
{
   DebugAutoLock( mLock );

   ev->AddRef();

   if ( ev->getPriority() == PRIORITY_NORMAL ) {
      mQueue.push_back( ev );
   }
   else {
      bool done = false;
      for (std::list<Event*>::iterator i = mQueue.begin(); i != mQueue.end();
               ++i) {
         if ( ev->getPriority() > ( *i )->getPriority() ) {
            mQueue.insert( i, ev );
            done = true;
            break;
         }
      }

      if ( !done ) {
         mQueue.push_back( ev );
      }
   }

   printf( "queue size %lu\n", mQueue.size() );

   mWait.Signal();
}

Event *EventQueue::WaitEvent( uint32_t timeoutms )
{
   DebugAutoLock( mLock );
   Event *ev = pollEventInternal();

   while ( NULL == ev ) {
      printf( "timeout %d\n", timeoutms );
      if ( mWait.Wait( mLock, timeoutms ) ) {
         printf( "signaled\n" );
         ev = pollEventInternal();
         if ( NULL == ev ) {
            printf( "Signaled empty queue\n" );
         }
      }
      else if ( timeoutms > 0 ) {
         return NULL;
      }
      printf( "done\n" );
   }

   return ev;
}

Event *EventQueue::PollEvent()
{
   DebugAutoLock( mLock );
   return pollEventInternal();
}

void EventQueue::Remove( Event::Id id )
{
   DebugAutoLock( mLock );

   for (std::list<Event*>::iterator i = mQueue.begin(); i != mQueue.end();
            ++i) {
      if ( id == ( *i )->getEventId() ) {
         ( *i )->Release();
         i = mQueue.erase( i );
         --i;
      }
   }
}

void EventQueue::Remove( Event *ev )
{
   DebugAutoLock( mLock );
   for (std::list<Event*>::iterator i = mQueue.begin(); i != mQueue.end();
            ++i) {
      if ( ev == ( *i ) ) {
         ( *i )->Release();
         i = mQueue.erase( i );
         i--;
      }
   }
}

void EventQueue::RemoveAgentsByReceiver( void *receiver )
{
   DebugAutoLock( mLock );

   for (std::list<Event*>::iterator i = mQueue.begin(); i != mQueue.end();
            ++i) {
      if ( ( *i )->getEventId() == Event::kAgentEventId ) {
         EventAgent* agent = static_cast<EventAgent*>( *i );
         if ( agent->getDeliveryTarget() == receiver ) {
            ( *i )->Release();
            i = mQueue.erase( i );
            --i;
         }
      }
   }
}

void EventQueue::Flush()
{
   DebugAutoLock( mLock );

   while ( !mQueue.empty() ) {
      Event* temp = mQueue.front();
      mQueue.pop_front();
      temp->Release();
   }
}

Event *EventQueue::pollEventInternal()
{
   if ( mQueue.empty() ) {
      return NULL;
   }

   Event *ret = mQueue.front();
   mQueue.pop_front();

   return ret;
}
