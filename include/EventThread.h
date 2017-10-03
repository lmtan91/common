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

#ifndef INCLUDE_EVENTTHREAD_H_
#define INCLUDE_EVENTTHREAD_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/
#include "EventDispatcher.h"
#include "Thread.h"

/**============================================================================
 * Class description
 *
 *
 *============================================================================*/
class EventThread: public EventDispatcher {

public:

   /**=========================================================================
    * @brief Constructor
    *
    * @param[in]  name
    *=========================================================================*/
   EventThread( const char *name = NULL );

   /**=========================================================================
    * @brief Destructor
    *
    *
    *=========================================================================*/
   virtual ~EventThread();

private:

   /**=========================================================================
    * @brief main thread of EventThread.
    *
    *=========================================================================*/
   void threadMain();

   const Thread *getDispatcherThread()
   {
      return &mThread;
   }

   Runnable<EventThread> mThread;
};

/**============================================================================
 * This is a class that can adapt a method into a EventListener.
 *============================================================================*/
template<class ClassType, class EventType>
class EventMethod: public IEventListener {
public:
   typedef void (ClassType::*event_method_t)( EventType *ev );

   /**
    * Construct a EventMethod.
    *
    * @param object The instance of T that they method sould be called on.
    * @param method The member function that will be the events handler.
    */
   EventMethod( ClassType *object, event_method_t method,
            IEventDispatcher *dispatcher ) :
            mObject( object ), mFunc( method ), mDispatcher( dispatcher )
   {
      mDispatcher->addEventListener( this, EventType::GetEventId() );
   }

   virtual ~EventMethod()
   {
      mDispatcher->removeEventListener( this, EventType::GetEventId() );
   }

   void receiveEvent( Event *ev )
   {
      EventType *real_event = event_cast<EventType>( ev );
      ( mObject->*mFunc )( real_event );
   }

private:
   ClassType* mObject;
   event_method_t mFunc;
   IEventDispatcher *mDispatcher;
};

#endif /* ifndef INCLUDE_EVENTTHREAD_H_ */
