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

#ifndef INCLUDE_EVENTDISPATCHER_H_
#define INCLUDE_EVENTDISPATCHER_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/
#include <list>
#include "EventQueue.h"
#include "Mutex.h"
#include "EventAgent.h"

/**============================================================================
 * A helper class for anyone implementing IEventDispatcher.  This class will
 * track the list of EventListeners and send an event to all the interested
 * listeners when dispatchEvent is called.
 *============================================================================*/
class EventDispatcherHelper {
public:
   EventDispatcherHelper();
   ~EventDispatcherHelper();

   void dispatchEvent( Event *ev );

   int addEventListener( IEventListener *listener, int event_id );
   int removeEventListener( IEventListener *listener, int event_id );

private:
   void lookupListeners( Event *ev );

   struct EventListenerNode {
      IEventListener *mListener;
      int mEventId;
   };

   Mutex mLock;
   std::list<EventListenerNode*> mEventList;
};

/**============================================================================
 * Class description
 *
 *
 *============================================================================*/
class EventDispatcher: public IEventDispatcher {
public:
   EventDispatcher();
   virtual ~EventDispatcher();

   /**=========================================================================
    * @brief Send a event to the queue, then wait for a signal from the
    * receiving thread to wake up.
    *
    * @param   ev    Event to be sent.
    *=========================================================================*/
   void sendEventSync( Event *ev );

   /**=========================================================================
    * @brief Send a event to the queue.
    *
    * @param ev      Event will be sent.
    *=========================================================================*/
   void sendEvent( Event *ev );

   /**=========================================================================
    * @brief Send a event at a later time.
    *=========================================================================*/
   void sendTimedEvent( Event *ev, uint32_t msecs );

   /**=========================================================================
    * Send a recurring Event with a regular period
    *=========================================================================*/
   void sendPeriodicEvent( Event *ev, uint32_t msecs );

   /**=========================================================================
    * Remove all events with given eventId
    *=========================================================================*/
   int remove( int eventId );

   /**=========================================================================
    * Remove this event.
    *=========================================================================*/
   int remove( Event *ev );

   /**=========================================================================
    * Remove any EventAgent events that will be delivered to this object
    *=========================================================================*/
   int removeAgentsByReceiver( void* recipient );

   /**=========================================================================
    * remove all events from the queue
    *=========================================================================*/
   int removeAll();

   /**=========================================================================
    * @brief Check if thread is current
    *
    * This method checks if the caller's thread is the same as the
    * thread used by this IEventDispatcher.   If it is then this
    * method will return true, otherwise returns false
    *=========================================================================*/
   bool isThreadCurrent();

   /**=========================================================================
    * add an event listener to be called when event_id is received, if event_id
    *  is kInvalidEventId then all events received will be sent to the listener.
    *=========================================================================*/
   int addEventListener( IEventListener *listener, int event_id );

   /**=========================================================================
    * Remove this event listener for this event_id, if and exact match is not
    *  found it will fail.
    *=========================================================================*/
   int removeEventListener( IEventListener *listener, int event_id );

protected:
   struct SyncEventHolder: public Event {
      SyncEventHolder( Event *ev ) :
               Event( Event::kSyncEventId, ev->getPriority() ), mRealEvent(
                        ev ), mDone( false )
      {
      }

      ~SyncEventHolder()
      {
      }

      SMART_CASTABLE (Event::kSyncEventId);

      SmartPtr<Event> mRealEvent;
      bool mDone;
   };

   virtual void wakeThread()
   {
   }
   virtual void onThreadExit()
   {
   }

   /**=========================================================================
    * This method is used to check if a sync event is going to be sent to
    *  the same thread as the sender.  Today we just log a fatal error in this
    *  case.
    *=========================================================================*/
   virtual const Thread *getDispatcherThread()
   {
      return NULL;
   }

   bool handleEvent( Event *ev );

   EventQueue mQueue;

private:
   void handleSyncEvent( Event *ev );

   EventDispatcherHelper mDispatcher;
   Condition mSyncWait;
   Mutex mSyncLock;
};

#endif /* ifndef INCLUDE_EVENTDISPATCHER_H_ */
