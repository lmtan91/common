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

#ifndef INCLUDE_EVENT_H_
#define INCLUDE_EVENT_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/
#include <stdint.h>
#include "RefCount.h"

enum {
   PRIORITY_NORMAL, PRIORITY_HIGH
};
/**============================================================================
 * Class description
 *
 *
 *============================================================================*/
class Event: public RefCount {

public:
   typedef int Id;

   /**=========================================================================
    * @brief Constructor
    *
    * @param[in]  event_id    Id of event.
    * @param[in]  prio        Priority of event.
    *=========================================================================*/
   Event( Id event_id, int prio = PRIORITY_NORMAL ) :
            mEventId( event_id ), mPrio( prio )
   {
   }

   /**=========================================================================
    * @brief Destructor
    *
    *
    *=========================================================================*/
   virtual ~Event()
   {
   }

   static const Id kInvalidEventId = -1;
   static const Id kShutdownEventId = -2;
   static const Id kSyncEventId = -3;
   static const Id kSelectorUpdateEventId = -4;
   static const Id kAgentEventId = -5;

   /**=========================================================================
    * @brief Get event's id.
    *
    *
    * @return  Id
    *=========================================================================*/
   Id getEventId() const
   {
      return mEventId;
   }

   /**=========================================================================
    * @brief Get event's priority.
    *
    *
    * @return  int
    *=========================================================================*/
   int getPriority() const
   {
      return mPrio;
   }

   /**=========================================================================
    * @brief Get event's priority.
    *
    *
    * @return  int
    *=========================================================================*/
   void setPriority( int prio )
   {
      if ( prio > PRIORITY_HIGH ) {
         prio = PRIORITY_HIGH;
      }
      mPrio = prio;
   }

private:
   Id mEventId;
   int mPrio;
};

/**============================================================================
 * This interface is implemented by any class that will recieve events for an
 *  event dispatcher.  This interface can be registered with the EventThread
 *  or the selector classes.
 *============================================================================*/
class IEventListener {
public:
   /**=========================================================================
    * @brief Receive an event.
    *
    * @param   ev
    * @return  void
    *=========================================================================*/
   virtual void receiveEvent( Event *ev ) = 0;

protected:
   /**=========================================================================
    * @brief Destructor
    *
    *
    *=========================================================================*/
   virtual ~IEventListener() {
   }
};

/**============================================================================
 * This interface is implemented by any class that receive messages.
 * There are currently 3 types of event dispatchers: EventThread, EventQueue,
 * and Selector.
 *
 *============================================================================*/
class IEventDispatcher {
public:
   /**=========================================================================
    * @brief Send an event to the queue sync.
    *
    * @param   ev
    * @return  void
    *=========================================================================*/
   virtual void sendEventSync( Event *ev ) = 0;

   /**=========================================================================
    * @brief Send an event to the queue.
    *
    * @param   ev
    * @return  void
    *=========================================================================*/
   virtual void sendEvent( Event *ev ) = 0;

   /**=========================================================================
    * @brief Send an event to the queue at later.
    *
    * @param      ev
    * @param[in]  msec
    * @return     void
    *=========================================================================*/
   virtual void sendTimedEvent( Event *ev, uint32_t msecs ) = 0;

   /**=========================================================================
    * @brief Send a recurring event with a regular period.
    *
    * @param      ev
    * @param[in]  msecs
    * @return     void
    *=========================================================================*/
   virtual void sendPeriodicEvent( Event *ev, uint32_t msecs ) = 0;

   /**=========================================================================
    * @brief Remove all events with specified event_id from the queue.
    *
    * @param   event_id
    * @return  void
    *=========================================================================*/
   virtual int remove( Event::Id event_id ) = 0;

   /**=========================================================================
    * @brief Remove a specific event from the queue.
    *
    * @param   ev
    * @return  void
    *=========================================================================*/
   virtual int remove( Event* ev ) = 0;

   /**=========================================================================
    * @brief Remove all events from the queue.
    *
    * @return  void
    *=========================================================================*/
   virtual int removeAll() = 0;

   /**=========================================================================
    * @brief Check if thread is current.
    *
    * This method checks if the caller's thread is the same as the
    * thread used by this IEventDispatcher.   If it is then this
    * method will return true, otherwise returns false
    *
    * @return  bool
    *=========================================================================*/
   virtual bool isThreadCurrent() = 0;

   /**=========================================================================
    * @brief Add an event listener to be called when event_id is received,
    * if event_id is kInvalidEventId then all events received will be sent to
    * the listener.
    *
    * @param   listener
    * @param   event_id
    * @return  bool
    *=========================================================================*/
   virtual int addEventListener( IEventListener *listener, int event_id ) = 0;

   /**=========================================================================
    * @brief Remove this event listener for the event_id.
    *
    * @param   listener
    * @param   event_id
    * @return  bool
    *=========================================================================*/
   virtual int removeEventListener( IEventListener *listener,
         int event_id ) = 0;

protected:
   /**=========================================================================
    * @brief Destructor
    *
    *
    *=========================================================================*/
   virtual ~IEventDispatcher() {

   }
};

#define SMART_CASTABLE( id ) static int GetEventId() { return id; }

template<class T>
T *event_cast( Event *ev )
{
   if ( ev->getEventId() == T::GetEventId() )
      return static_cast<T*>( ev );
   else
      return NULL;
}

#endif /* ifndef INCLUDE_EVENT_H_ */
