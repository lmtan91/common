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

#ifndef INCLUDE_MUTEX_H_
#define INCLUDE_MUTEX_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/
#include <pthread.h>
#include <errno.h>

/**============================================================================
 * Class description
 *
 *
 *============================================================================*/
class Mutex
{

public:

   /**=========================================================================
    * @brief Constructor
    *
    * @param[in]  recursive   Is mutex recursive
    *=========================================================================*/
   //lint -sem(Mutex::create_lock,initializer)
   Mutex( bool recursive );

   /**=========================================================================
    * @brief Constructor
    *
    * @param[in]  name        The name of mutex
    * @param[in]  recursive   Is mutex recursive
    *=========================================================================*/
   //lint -sem(Mutex::create_lock,initializer)
   Mutex( const char *name, bool recursive = false );

   /**=========================================================================
    * @brief Destructor
    *
    *
    * @param[in]
    *=========================================================================*/
   virtual ~Mutex();

   /**=========================================================================
    * @brief Lock mutex until, blocking until lock is available.
    *
    * @return int
    *=========================================================================*/
   //lint -sem(Mutex::Lock,thread_lock)
   int Lock();

   /**=========================================================================
    * @brief Lock mutex without blocking.
    *
    * @return int
    *=========================================================================*/
   int TryLock();

   /**=========================================================================
    * @brief Unlock this mutex
    *
    * @return int
    *=========================================================================*/
   //lint -sem(Mutex::Unlock,thread_lock)
   int Unlock();

   /**=========================================================================
    * @brief Block till lock is available.
    *
    * @return int
    *=========================================================================*/
   //lint -sem(Mutex::TraceLock,thread_lock)
   int TraceLock( const char *file, int line );

   /**=========================================================================
    * @brief Try to lock without blocking.
    *
    * @return int
    *=========================================================================*/
   int TraceTryLock( const char *file, int line );

   /**=========================================================================
    * @brief Unlock this mutex and print out trace info on error
    *
    * @return int
    *=========================================================================*/
   //lint -sem(Mutex::TraceUnlock,thread_unlock)
   int TraceUnlock( const char *file, int line );

   /**=========================================================================
    * @brief Is this lock held by thois thread?
    *
    * @return bool
    *=========================================================================*/
   bool isLocked();

   /**=========================================================================
    * @brief What thread has locked this
    *
    * @return const char*
    *=========================================================================*/
   const char *getOwner();

   /**=========================================================================
    * @brief Global initialization of Mutex object
    *
    *=========================================================================*/
   static void Init()
   {
      if ( !mInited ) {
         mInited = true;
         pthread_mutex_init( &mCriticalSection, NULL );
      }
   }

   /**=========================================================================
    * @brief Global destruction of Mutex object
    *
    *=========================================================================*/
   static void Destroy()
   {
      pthread_mutex_destroy( &mCriticalSection );
   }
   /**=========================================================================
    * @brief Grab global lock.
    *
    * Mutex::mCriticalSection is used in places scatterd around as a global lock
    * It should NOT be held for longer than a couple of instructions
    *=========================================================================*/
   static void EnterCriticalSection()
   {
      pthread_mutex_lock( &mCriticalSection );
   }

   /**=========================================================================
    * @brief Release global lock.
    *
    * Mutex::mCriticalSection is used in places scatterd around as a global lock
    * It should NOT be held for longer than a couple of instructions
    *=========================================================================*/
   static void ExitCriticalSection()
   {
      pthread_mutex_unlock( &mCriticalSection );
   }
private:

   /**=========================================================================
    * @brief Create the internal lock that will be used. Pay attention to
    * the mRecursive member to determine which attributes to set
    *
    *=========================================================================*/
   void create_lock();

   // The lock for this particular Mutex object
   pthread_mutex_t mMutex;

   // Who lock this lock
   Thread *mLockedBy;

   // The name of this lock
   const char *mName;
   /**=========================================================================
    * Is this a recirsive lock? That is, can it be held repeatedly by the same
    * thread, or will grabbing it twice by any thread cause an error?
    *=========================================================================*/
   bool mRecursive;

   /**=========================================================================
    * current lock count for recursive lock. This is used in order to keep
    * track og mLockBy accurately
    *=========================================================================*/
   int mLockCount;

   /**=========================================================================
    * if tracing, this is supposed to be the name of file
    *
    *=========================================================================*/
   const char *mLockFile;

   /**=========================================================================
    * if tracing, this is supposed to be the line number in file
    *
    *=========================================================================*/
   int mLockLine;
   static pthread_mutex_t mCriticalSection;
   static bool mInited;


};

#endif /* ifndef INCLUDE_MUTEX_H_ */
