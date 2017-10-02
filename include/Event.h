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

#endif /* ifndef INCLUDE_EVENT_H_ */
