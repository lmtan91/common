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
 * Condition.cpp
 *
 *  Created on: Oct 2, 2017
 *      Author: lia1hc
 */

/* Standard */
#include <stdio.h>
/* Common */
#include "Condition.h"
#include "Mutex.h"
#include "Thread.h"
#include "TimeUtils.h"

Condition::Condition()
{
   pthread_cond_init( &mCond, NULL );
}

Condition::~Condition()
{
   int ret;

   ret = pthread_cond_destroy( &mCond );
   if ( ret != 0 ) {
      printf( "pthread_cond_destroy() failed with %d\n", ret );
   }
}

void Condition::Wait( Mutex &mutex ) {
   Wait( mutex, 0 );
}

bool Condition::Wait( Mutex &mutex, uint32_t timeoutMs ) {
   bool ret = true;

   Thread* lockedBy = mutex.mLockedBy;
   int lockCount = mutex.mLockCount;
   const char* lockFile = mutex.mLockFile;
   int lockLine = mutex.mLockLine;

   // Reset mutex information
   mutex.mLockedBy = NULL;
   mutex.mLockCount = 0;

   if ( timeoutMs > 0 ) {
      struct timespec timeout;
      TimeUtils::getCurTime( &timeout );
      TimeUtils::addOffset( &timeout, timeoutMs );

      if ( pthread_cond_timedwait( &mCond, &mutex.mMutex, &timeout ) != 0 ) {
         ret = false;
      }
   } else {
      pthread_cond_wait( &mCond, &mutex.mMutex );
   }

   mutex.mLockedBy = lockedBy;
   mutex.mLockCount = lockCount;
   mutex.mLockFile = lockFile;
   mutex.mLockLine = lockLine;

   return ret;
}
