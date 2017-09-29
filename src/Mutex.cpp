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
 * Mutex.cpp
 *
 *  Created on: Sep 25, 2017
 *      Author: Tan.leMinh
 */

#include <stdio.h>

#include "Thread.h"
#include "Mutex.h"

bool Mutex::mInited;
Mutex::Mutex( bool recursive ) :
      mLockedBy( NULL ), mName( NULL ), mRecursive( recursive ), mLockCount( 0 )
{
   create_lock();
}

Mutex::Mutex( const char *name, bool recursive ) :
      mLockedBy( NULL ), mName( name ), mRecursive( recursive ), mLockCount( 0 )
{
   create_lock();
}

Mutex::~Mutex()
{
   int ret;

   ret = pthread_mutex_destroy( &mMutex );
   if ( ret ) {
      printf( "pthread_mutex_destroy failed with %d\n", ret );
   }
}

const char *Mutex::getOwner()
{
   if ( mLockedBy ) {
      return mLockedBy->GetName();
   } else {
      return NULL;
   }
}

bool Mutex::isLocked()
{
   return ( mLockedBy == Thread::GetCurrent() );
}

int Mutex::TraceUnlock(const char *file, int line)
{
   int ret;
   Thread *lockedBy = mLockedBy;
   int lockCount = mLockCount;
   if ( mRecursive && --mLockCount <= 0 ) {
      mLockedBy = NULL;
      mLockCount = 0;
   }
   else {
      mLockedBy = NULL;
   }

   ret = pthread_mutex_unlock( &mMutex );

   // Check to see if operation not permmited
   if ( EPERM == ret ) {
      printf( "Unlock lock %s that isn't locked by current thread at %s:%d "
            "(help by %s@%s:%d) error %d\n", mName, file, line,
            lockedBy != NULL ? lockedBy->GetName() : "nobody", mLockFile,
            mLockLine, ret );

      mLockedBy = lockedBy;
      mLockCount = lockCount;
   }
   else if ( ret != 0 ) {
      printf( "Unlock lock %s failed with error %d at %s:%d", mName, ret, file,
            line );
   }
   return ret;
}

int Mutex::TraceTryLock( const char *file, int line )
{
   int ret;

   ret = pthread_mutex_trylock( &mMutex );
   // If we locked successfully
   if ( 0 == ret ) {
      mLockedBy = Thread::GetCurrent();
      mLockFile = file;
      mLockLine = line;

      if ( mRecursive && 0 == mLockCount ) {
         ++mLockCount;
      }
   }

   return ret;
}

int Mutex::TraceLock( const char *file, int line )
{
   int ret;

   ret = pthread_mutex_lock( &mMutex );
   if ( EDEADLK == ret ) {
      printf( "Lock %s already taken at %s:%d\n", mName, mLockFile, mLockLine );
   }
   else if ( ret != 0 ) {
      printf( "Lock %s failed with error %d at %s:%d\n", mName, ret, mLockFile,
            mLockLine );
      return ret;
   }

   mLockedBy = Thread::GetCurrent();
   mLockFile = file;
   mLockLine = line;
   if ( mRecursive && 0 == mLockCount ) {
      ++mLockCount;
   }
   return ret;
}

int Mutex::Unlock()
{
   return TraceLock( "unknown", 0 );
}

int Mutex::TryLock()
{
   return TraceTryLock( "unknown", 0 );
}

int Mutex::Lock()
{
   return TraceLock( "unknown", 0 );
}

void Mutex::create_lock()
{
   int ret;
   int type;

   pthread_mutexattr_t attr;
   // avoid warning ignoring return value
   (void) pthread_mutexattr_init( &attr );

   if ( mRecursive ) {
      type = PTHREAD_MUTEX_RECURSIVE_NP;
   }
   else {
      type = PTHREAD_MUTEX_ERRORCHECK_NP;
   }
   (void) pthread_mutexattr_settype( &attr, type );

   ret = pthread_mutex_init( &mMutex, &attr );
   if ( ret ) {
      printf( "pthread_mutex_init failed with %d\n", ret );
   }

   // destroy attribute of mutex
   ret = pthread_mutexattr_destroy( &attr );
   if ( ret ) {
      printf( "pthread_mutexattr_destroy failed with %d\n", ret );
   }
}
