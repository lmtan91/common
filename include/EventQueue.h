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

#ifndef INCLUDE_EVENTQUEUE_H_
#define INCLUDE_EVENTQUEUE_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/
/* Standard */
#include <list>
#include <stdio.h>

#include "Mutex.h"
#include "Condition.h"
#include "Event.h"

/**============================================================================
 * Class for queuing events. This is used internally by EventDispatcher.
 * If this class is used on its own the owner must be aware of event ref counts.
 *
 *============================================================================*/
class EventQueue {

public:

   /**=========================================================================
    * @brief Constructor
    *
    *=========================================================================*/
   EventQueue();

   /**=========================================================================
    * @brief Destructor
    *
    *=========================================================================*/
   virtual ~EventQueue();

   /**=========================================================================
    * @brief Send a event to the queue.
    *
    *
    * @param[in]     ev    Event to be sent.
    *
    * @return        void
    *=========================================================================*/
   void SendEvent( Event *ev );

   /**=========================================================================
    * @brief Wait for an event to arrive. User MUST call release on Event when
    * done with it.
    *
    * @param[in]     timeoutms    Number of miniseconds to wait before timeout.
    * If 0, wait forever.
    *
    * @return        Event
    *=========================================================================*/
   Event *WaitEvent( uint32_t timeoutms = 0 );

   /**=========================================================================
    * @brief Check if an event is pending on the queue. User MUST call realse on
    * Event done with it.
    *
    *
    * @return        Event
    *=========================================================================*/
   Event *PollEvent();

   /**=========================================================================
    * @brief Remove all events on the queue with a specified event_id.
    *
    *
    *=========================================================================*/
   void Remove( Event::Id event_id );

   /**=========================================================================
    * @brief Remove the specified event.
    *
    *
    *=========================================================================*/
   void Remove( Event *ev );

   /**=========================================================================
    * @brief Remove any EventAgents who are going to be delivered to this object.
    *
    *
    *=========================================================================*/
   void RemoveAgentsByReceiver( void *receiver );

   /**=========================================================================
    * @brief Remove all items from queue.
    *
    *
    *=========================================================================*/
   void Flush();

private:
   /**=========================================================================
    * @brief Poll event internally.
    *
    *
    *=========================================================================*/
   Event *pollEventInternal();

   std::list<Event*> mQueue;
   Mutex mLock;
   Condition mWait;
};

#endif /* ifndef INCLUDE_EVENTQUEUE_H_ */
