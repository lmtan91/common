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

#ifndef TIMER_H_
#define TIMER_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/
#include <list>
#include "Event.h"
#include "Thread.h"
#include "Mutex.h"
#include "EventAgent.h"

class TimerListener {
public:
   /**=========================================================================
    * Called by TimerManager when the timeout expires.  You class implements
    *  this and provide an pointer to this interface when a timer is created.
    *=========================================================================*/
   virtual void onTimeout( uint32_t private_data ) = 0;

protected:
   /**=========================================================================
    * @brief Destructor
    *
    *
    *=========================================================================*/
   virtual ~TimerListener()
   {
   }
};

/**============================================================================
 * Class description
 *
 *
 *============================================================================*/
class TimerManager {

public:

   /**=========================================================================
    * @brief Constructor
    * Prevent non-singleton usage
    *
    *=========================================================================*/
   TimerManager();

   /**=========================================================================
    * @brief Destructor
    *
    * Prevent anyone from destroying this in any way other than
    * destroyManager
    *=========================================================================*/
   virtual ~TimerManager();

   /**=========================================================================
    * Add an event to be dispatcher to the given dispatcher at a time in
    *  the furture.  This event's ref count will be incremented to ensure
    *  the event is not destroyed while we wait.  NOTE: use the Selector or
    *  EventThread to send timed events do no call this directly.
    *
    * @param event - the event to send.
    * @param dispatcher - the dispatcher to dispatch event to
    * @param msecs - how many milliseconds to wait before dispatching.
    *=========================================================================*/
   void sendTimedEvent( Event *event, IEventDispatcher *dispatcher,
            uint32_t msecs );

   /**=========================================================================
    * Add an event to be dispatcher to the given dispatcher with a
    *  regular period.  This event's ref count will be incremented to
    *  ensure the event is not destroyed before it is removed.  NOTE: use
    *  the Selector or EventThread to send timed events do no call
    *  this directly.
    *
    * @param event - the event to send.
    * @param dispatcher - the dispatcher to dispatch event to
    * @param msecs - how many milliseconds to wait before dispatching.
    *=========================================================================*/
   void sendPeriodicEvent( Event *event, IEventDispatcher *dispatcher,
            uint32_t period );

   /**=========================================================================
    * Remove all timed event from a given dispatcher with a certian
    * id or if id is kInvalidId then remove all events on the given
    * dispatcher.
    *
    * @param event_id - the event id to remove or kInvalidEventId to
    * remove all.
    * @param dispatcher - the dispatcher for which we are removing events.
    *=========================================================================*/
   void removeTimedEvent( Event::Id event_id, IEventDispatcher *dispatcher );

   /**=========================================================================
    * Remove the timed event.  In this case we don't care what the
    * dispatcher was.
    *
    * @param ev - the event to remove.
    *=========================================================================*/
   void removeTimedEvent( Event *ev );

   /**=========================================================================
    * Remove any EventAgents from this dispatcher with this destination
    *
    * @param reciever - the delivery target for an event agent
    *
    * @param dispatcher - the dispatcher to clear those agents for or
    * NULL for all
    *=========================================================================*/
   void removeAgentsByReceiver( void *reciever, IEventDispatcher *dispatcher );

   /**=========================================================================
    * Add a timer listener.  This listener will be called after msecs.
    *=========================================================================*/
   void addTimer( TimerListener *listener, uint32_t msecs,
            uint32_t private_data );

   /**=========================================================================
    * Get the singleton TimerManager
    *=========================================================================*/
   static TimerManager *getInstance();

   /**=========================================================================
    * Destroy the TimerManager (until the next call to getInstance)
    *=========================================================================*/
   static void destroyManager();

private:
   //! These are the time events we are currently tracking
   struct TimerNode {
      //! The event we will pass
      SmartPtr<Event> mEvent;

      //! The dispatcher (Selector or EventDispatcher) we will send the event
      IEventDispatcher *mDispatcher;

      //! The listener that will be told by the dispatcher of the event
      TimerListener *mListener;

      //! The opaque data that will be given to the listener
      uint32_t mPrivateData;

      //! Which tick will trigger this event
      uint32_t mTick;

      //! After how many ms does this repeat?  (Or 0 for no repetition)
      uint32_t mRepeatMS;

      //! How many leftover ms have we accumulated if this is repeating?
      uint32_t mRemainingMS;
   };

   //! Add a new time event
   void addTimerNode( const TimerNode& node );

   //! Our main loop
   void clockHandler();

   //! Handle a clock tick (every kMsPerTick)
   void handleTick();

   //! How many ms do we wait per tick?
   static const int kMsPerTick = 100;

   //! Our signal handler for dealing with sigalrm
   static void signalHandler( int signal );

   //! Our singleton
   static TimerManager *mSingleton;

   //! Our thread
   Runnable<TimerManager> *mClockThread;

   //! Locking for internal state (only really mList and mTicks)
   Mutex mMutex;

   //! Everything we are waiting on
   std::list<TimerNode> mList;

   //! Number of ticks we have seen
   uint32_t mTicks;
};

#endif /* ifndef TIMER_H_ */
