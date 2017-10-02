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

#ifndef INCLUDE_THREAD_H_
#define INCLUDE_THREAD_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
/**============================================================================
 * Class description
 *
 *
 *============================================================================*/
class Thread {

public:

   /**=========================================================================
    * @brief Constructor
    *
    * @param[in] aName  Thread name.
    * @param[in] prior
    *=========================================================================*/
   Thread( const char * const aName, const int prio = 0 );

   /**=========================================================================
    * @brief Destructor
    *
    *
    *=========================================================================*/
   virtual ~Thread();

   /**=========================================================================
    * @brief Start the thread
    *
    *
    *=========================================================================*/
   int32_t Start();

   /**=========================================================================
    * @brief Stop thread
    *
    *
    *=========================================================================*/
   void Stop() const;

   /**=========================================================================
    * @brief Get name of thread
    *
    *
    *=========================================================================*/
   const char *GetName()
   {
      return mName;
   }

   /**=========================================================================
    * @brief Join thread
    *
    *
    *=========================================================================*/
   int32_t Join();

   /**=========================================================================
    * @brief comparison operator overloading
    *
    *
    *=========================================================================*/
   bool operator==( const Thread &t ) const;

   /**=========================================================================
    * @brief Exit current running thread
    *
    *
    *=========================================================================*/
   static void Exit();

   /**=========================================================================
    * @brief Get current running thread
    *
    *
    *=========================================================================*/
   static Thread *GetCurrent();

   /**=========================================================================
    * @brief Init thread
    *
    *
    *=========================================================================*/
   static void Init();

   /**=========================================================================
    * @brief Destroy thread
    *
    *
    *=========================================================================*/
   static void Destroy();

   static const unsigned long kThreadNameLen = 32;

protected:

   /**=========================================================================
    * @brief This function exits the thread will stop and Join will return
    *
    *
    *=========================================================================*/
   virtual void OnStart()
   {
      printf( "Thread::OnStart() Enter\n" );
   }

   /**=========================================================================
    * @brief Called by Stop
    *
    *
    *=========================================================================*/
   virtual void OnStop() const;

private:

   /**=========================================================================
    * @brief Constructor
    *
    * @param[in] aThread
    * @param[in] aName     Name of thread
    *=========================================================================*/
   Thread( const pthread_t aThread, const char * const aName );

   /**=========================================================================
    *
    *
    *=========================================================================*/

   char mName[ kThreadNameLen ];
   pthread_t mThread;
   int mPrio;
   bool mJoined;
   bool mSystemThread;
   static pthread_key_t mThreadKey;
   static bool mInited;

   static void *start_thread( void * const arg );
   static void thread_key_destructor( void * const arg );
};

template<class T>
class Runnable: public Thread {
public:
   typedef void (T::*thread_main_type)();

   /**=========================================================================
    * @brief Constructor
    *
    * @param[in] name         Name of thread
    * @param[in] obj          Instance of T that will call the function
    * @param[in] thread_main  Member function that will be called
    *=========================================================================*/
   Runnable(const char* name, T* obj, thread_main_type thread_main,
         bool hard_stop = false, int prio = 0):
         Thread( name, prio ), mObject( obj ), mFunc( thread_main ),
               mStop( false ), mHardStop( hard_stop ) {
      printf( "Runnable::Runnable() Enter=%p\n", this );
      if ( hard_stop ) {
         abort();
      }
   }

   /**=========================================================================
    * @brief Check if stop has been called on a non-hard stop instance
    *
    *=========================================================================*/
   bool CheckStop() const {
      return mStop;
   }
private:
   void OnStart() {
      ( mObject->*mFunc )();
   }

   void OnStop() {
      if ( mHardStop ) {
         Thread::OnStop();
      } else {
         mStop = true;
      }
   }
   T* mObject;
   thread_main_type mFunc;
   bool mStop;
   bool mHardStop;
};
#endif //__cplusplus

__BEGIN_DECLS

const char *GetThreadName();

__END_DECLS

#endif /* ifndef INCLUDE_THREAD_H_ */
