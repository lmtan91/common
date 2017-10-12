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
 * EventDispatcher.cpp
 *
 *  Created on: Oct 3, 2017
 *      Author: lia1hc
 */
#include <iostream>
#include "EventDispatcher.h"
#include "Timer.h"
#include "logging.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOTICE );
EventDispatcherHelper::EventDispatcherHelper() :
         mLock( true )
{
   TRACE_BEGIN(LOG_LVL_INFO);
}

EventDispatcherHelper::~EventDispatcherHelper()
{
   for (std::list<EventListenerNode*>::iterator i = mEventList.begin();
            i != mEventList.end(); ++i) {
      delete *i;
   }

   mEventList.clear();
}

void EventDispatcherHelper::dispatchEvent( Event *ev )
{
   DebugAutoLock( mLock );

   if ( !ev ) {
      LOG_WARN( "Attemp to dispatch NULL event." );
      return;
   }

   if ( Event::kAgentEventId == ev->getEventId() ) {
      EventAgent *agent = event_cast<EventAgent>( ev );
      if ( agent ) {
         agent->deliver();
      }
   }
   else {
      lookupListeners( ev );
   }
}

void EventDispatcherHelper::lookupListeners( Event *ev )
{
   for (std::list<EventListenerNode*>::iterator i = mEventList.begin();
            i != mEventList.end(); ++i) {
      // Prune dead listener nodes
      if ( ( *i )->mListener == NULL ) {
         delete *i;
         //TODO check if correct
         i = mEventList.erase( i );
         --i;
      } else if ( ( *i )->mEventId == ev->getEventId()
               || ( *i )->mEventId == Event::kInvalidEventId ) {
         IEventListener *listener = ( *i )->mListener;
         listener->receiveEvent( ev );
      }
   }
}

int EventDispatcherHelper::addEventListener( IEventListener *listener,
         int event_id )
{
   DebugAutoLock( mLock );

   EventListenerNode *node = new EventListenerNode;

   node->mListener = listener;
   node->mEventId = event_id;

   mEventList.push_back( node );

   return 0;
}

int EventDispatcherHelper::removeEventListener( IEventListener *listener,
         int event_id )
{
   DebugAutoLock( mLock );

   for (std::list<EventListenerNode*>::iterator i = mEventList.begin();
            i != mEventList.end(); ++i) {
      EventListenerNode *lnode = *i;

      if ( lnode->mEventId == event_id && lnode->mListener == listener ) {
         // Update mListener
         lnode->mListener = NULL;
         return 0;
      }
   }

   // Didn't find it
   return -1;
}

EventDispatcher::EventDispatcher()
{
   // NOTE:  This is a sort of hacky way of preventing a bad condition from
   // occuring.  It was found that when we are processing a signal to do
   // a shutdown (ie.  ctrl-c in tad) the creation of the thread in the
   // TimerManager hangs the system.  I traced this down to a hang up in
   // a call to pthread_create.  The odd thing is that the new thread is
   // spawned (the main is run) but the pthread_create method never returns.
   // To prevent this from hanging us up in the future we will make sure
   // that the TimerManager is initialized when an EventDispatcher is
   // created.
   // TimerManager::getInstance();
}

EventDispatcher::~EventDispatcher()
{
}

/**============================================================================
 * @brief Send a event to the queue, then wait for a signal from the
 * receiving thread to wake up.
 *
 * @param   ev    Event to be sent.
 *============================================================================*/
void EventDispatcher::sendEventSync( Event *ev )
{
   TRACE_BEGIN( LOG_LVL_NOISE );
   // event ref count handled by holder.
   SyncEventHolder holder( ev );
   holder.AddRef();

   mQueue.SendEvent( &holder );
   wakeThread();

   if ( isThreadCurrent() ) {
      LOG_ERR_FATAL(
               "Sending sync event to your current thread, I will die now..." );
   }

   mSyncLock.Lock();
   while ( holder.mDone == false ) {
      // Wait will unlock mSyncLock before going to sleep.
      // After being signaled it will relock before wakeup.
      mSyncWait.Wait( mSyncLock );
   }
   mSyncLock.Unlock();
}

/**============================================================================
 * @brief Send a event to the queue.
 *
 * @param ev      Event will be sent.
 *============================================================================*/
void EventDispatcher::sendEvent( Event *ev )
{
   mQueue.SendEvent( ev );
   wakeThread();
}

void EventDispatcher::sendTimedEvent( Event *ev, uint32_t msecs )
{
   TimerManager *timer = TimerManager::getInstance();
   timer->sendTimedEvent( ev, this, msecs );
}

void EventDispatcher::sendPeriodicEvent( Event *ev, uint32_t msecs )
{
   TimerManager *timer = TimerManager::getInstance();
   timer->sendPeriodicEvent( ev, this, msecs );
}

int EventDispatcher::remove( Event::Id eventId )
{
   if ( !isThreadCurrent() ) {
      SyncRetEventAgent1<EventDispatcher, int, Event::Id>* e =
               new
      SyncRetEventAgent1<EventDispatcher, int, Event::Id>( this,
               &EventDispatcher::remove, eventId );

      // I am a bad person, but it's either this or make all
      // EventAgent's publicly inherit from Event (instead of
      // Protected inheritance), and that's a big ugly change for
      // little benefit.
      ( (Event*) e )->setPriority( PRIORITY_HIGH );

      return e->send( this );
   }

   // It is important that we remove events from the TimerManager first.
   //  This ensure that an event does not get dispatched to the queue after we
   //  clear the queue.
   TimerManager *timer = TimerManager::getInstance();
   timer->removeTimedEvent( eventId, this );
   mQueue.Remove( eventId );
   return 0;
}


int EventDispatcher::remove( Event *ev )
{
   if ( !isThreadCurrent() ) {
      SyncRetEventAgent1<EventDispatcher, int, Event*>* e =
               new
      SyncRetEventAgent1<EventDispatcher, int, Event*>( this,
               &EventDispatcher::remove, ev );

      // I am a bad person, but it's either this or make all
      // EventAgent's publicly inherit from Event (instead of
      // Protected inheritance), and that's a big ugly change for
      // little benefit.
      ( (Event*) e )->setPriority( PRIORITY_HIGH );

      return e->send( this );
   }

   // It is important that we remove events from the TimerManager first.
   //  This ensure that an event does not get dispatched to the queue after we
   //  clear the queue.
   TimerManager *timer = TimerManager::getInstance();
   timer->removeTimedEvent( ev );
   mQueue.Remove( ev );
   return 0;
}

int EventDispatcher::removeAgentsByReceiver( void* recipient )
{
   if ( not isThreadCurrent() ) {
      SyncRetEventAgent1<EventDispatcher, int, void*>* e =
               new
      SyncRetEventAgent1<EventDispatcher, int, void*>( this,
               &EventDispatcher::removeAgentsByReceiver, recipient );

      // I am a bad person, but it's either this or make all
      // EventAgent's publicly inherit from Event (instead of
      // Protected inheritance), and that's a big ugly change for
      // little benefit.
      ( (Event*) e )->setPriority( PRIORITY_HIGH );

      return e->send( this );
   }

   TimerManager *timer = TimerManager::getInstance();
   timer->removeAgentsByReceiver( recipient, this );
   mQueue.RemoveAgentsByReceiver( recipient );
   return 0;
}

int EventDispatcher::removeAll()
{
   if ( !isThreadCurrent() ) {
      SyncRetEventAgent0<EventDispatcher, int>* e = new
      SyncRetEventAgent0<EventDispatcher, int>( this,
               &EventDispatcher::removeAll );

      // I am a bad person, but it's either this or make all
      // EventAgent's publicly inherit from Event (instead of
      // Protected inheritance), and that's a big ugly change for
      // little benefit.
      ( (Event*) e )->setPriority( PRIORITY_HIGH );

      return e->send( this );
   }

   // Same a remove exept we give TimerManager an invalid id so that he will
   //  remove all events for this dispatcher.
   TimerManager *timer = TimerManager::getInstance();
   timer->removeTimedEvent( Event::kInvalidEventId, this );
   mQueue.Flush();
   return 0;
}

bool EventDispatcher::isThreadCurrent()
{
   if ( Thread::GetCurrent() == getDispatcherThread() ) {
      return true;
   }
   return false;
}

int EventDispatcher::addEventListener( IEventListener *listener, int event_id )
{
   return mDispatcher.addEventListener( listener, event_id );
}

int EventDispatcher::removeEventListener( IEventListener *listener,
         int event_id )
{
   return mDispatcher.removeEventListener( listener, event_id );
}

void EventDispatcher::handleSyncEvent( Event *ev )
{
   TRACE_BEGIN( LOG_LVL_NOISE );

   SyncEventHolder *holder = event_cast<SyncEventHolder>( ev );

   if ( holder == NULL ) {
      LOG_ERR_FATAL( "event must not be NULL" );
      return;
   }

   mDispatcher.dispatchEvent( holder->mRealEvent );
   mSyncLock.Lock();
   holder->mDone = true;
   mSyncLock.Unlock();
   mSyncWait.Broadcast();
   // ev will get deleted in the caller.
}

bool EventDispatcher::handleEvent( Event *ev )
{
   TRACE_BEGIN( LOG_LVL_NOISE );

   bool done = false;

   if ( ev == NULL ) {
      LOG_WARN( "called with NULL event" );
      return false;
   }

   switch (ev->getEventId()) {
   case Event::kShutdownEventId:
      LOG_NOTICE( "kShutdownEventId recieved");
      // ignore return value
      (void) removeAll();
      done = true;
      ev->Release();
      break;

   case Event::kSyncEventId:
      handleSyncEvent( ev );
      break;

   default:
      mDispatcher.dispatchEvent( ev );
      ev->Release();
      break;
   }

   return done;
}
