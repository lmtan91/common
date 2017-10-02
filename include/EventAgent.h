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

#ifndef INCLUDE_EVENTAGENT_H_
#define INCLUDE_EVENTAGENT_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/
#include "Event.h"

/**============================================================================
 * @brief EventAgent class
 *
 * Technically this should probably be called the AgentEvent because
 * it is just a specialized Event that has an interface that the
 * EventDispatcher's in our system know how to dispatch ON the event
 * based on the event id, as opposed to dispatching the event to an
 * IEventListener.
 *============================================================================*/
class EventAgent: public Event {

public:

   /**=========================================================================
    * @brief Constructor
    *
    * @param[in]
    *=========================================================================*/
   EventAgent() :
         Event( Event::kAgentEventId ) {
   }

   SMART_CASTABLE(Event::kAgentEventId)
   ;

   /**=========================================================================
    * @brief Delivery interface.
    * Unlike normal events in the system the EventAgent is special
    * in that the event dispatching classes know to invoke this
    * method ON the event instead of invoking a method on an IEventListener
    * and providing the event.
    *
    *=========================================================================*/
   virtual void deliver() = 0;

   /**=========================================================================
    * @brief Who is this going to?
    * In order to support "remove all queued events going to this target"
    * this needs to be implemented to return the pointer to the object
    * that will handle this delivery.
    *
    *=========================================================================*/
   virtual void *getDeliveryTarget() = 0;

protected:
   /**=========================================================================
    * @brief Destructor
    *
    *
    * @param[in]
    *=========================================================================*/
   virtual ~EventAgent();

};

/**============================================================================
 * @brief EventAgent class
 *
 * Technically this should probably be called the AgentEvent because
 * it is just a specialized Event that has an interface that the
 * EventDispatcher's in our system know how to dispatch ON the event
 * based on the event id, as opposed to dispatching the event to an
 * IEventListener.
 *============================================================================*/
class AsyncEventAgent: protected EventAgent {
public:
   /**=========================================================================
    * @brief Constructor
    *
    *=========================================================================*/
   AsyncEventAgent() :
         EventAgent() {
   }

   using EventAgent::AddRef;
   using EventAgent::Release;

   /**=========================================================================
    * @brief Dispatch asynchronously to the dispatcher specified.
    *
    * @param   dispatcher
    * @return  void
    *=========================================================================*/
   void send( IEventDispatcher *dispatcher ) {
      dispatcher->sendEvent( this );
   }

   /**=========================================================================
    * @brief Dispatch timed asynchronously to the dispatcher specified.
    *
    * @param   dispatcher
    * @param   msecs
    * @return  void
    *=========================================================================*/
   void SendTimed( IEventDispatcher *dispatcher, uint32_t msecs ) {
      dispatcher->sendTimedEvent( this, msecs );
   }

   /**=========================================================================
    * @brief Dispatch periodically to the dispatcher specified.
    *
    * @param   dispatcher
    * @param   msecs
    * @return  void
    *=========================================================================*/
   void sendPeriodically( IEventDispatcher *dispatcher, uint32_t msecs ) {
      dispatcher->sendPeriodicEvent( this, msecs );
   }

   /**=========================================================================
    * @brief Remove previously dispatched from dispatcher specified.
    *
    * @param   dispatcher
    * @return  void
    *=========================================================================*/
   void remove( IEventDispatcher *dispatcher ) {
      dispatcher->remove( this );
   }

protected:
   /**=========================================================================
    * @brief Destructor
    *
    *
    *=========================================================================*/
   virtual ~AsyncEventAgent() {
   }
};

class SyncEventAgent: protected EventAgent {
public:
   /**=========================================================================
    * @brief Constructor
    *
    *=========================================================================*/
   SyncEventAgent() :
         EventAgent() {
   }

   using EventAgent::AddRef;
   using EventAgent::Release;

   /**=========================================================================
    * @brief Dispatch sync.
    *
    * @param   dispatcher
    * @return  void
    *=========================================================================*/
   void send( IEventDispatcher *dispatcher ) {
      AddRef();
      if ( dispatcher->isThreadCurrent() ) {
         deliver();
      } else {
         dispatcher->sendEventSync( this );
      }

      Release();
   }

protected:
   /**=========================================================================
    * @brief Destructor
    *
    *
    *=========================================================================*/
   virtual ~SyncEventAgent() {
   }
};
#endif /* ifndef INCLUDE_EVENTAGENT_H_ */
