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
 * Thread.cpp
 *
 *  Created on: Sep 26, 2017
 *      Author: lia1hc
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "Thread.h"
#include "Mutex.h"

bool Thread::mInited;
pthread_key_t Thread::mThreadKey;
pthread_mutex_t Mutex::mCriticalSection;

Thread::Thread( const char * const aName, const int prio ) :
      mThread( 0 ), mJoined( false ), mSystemThread( false )
{
   strncpy( mName, aName, kThreadNameLen );
   mName[ kThreadNameLen - 1 ] = '\0';
   mPrio = prio;
}

Thread::~Thread()
{
   if ( !mSystemThread ) {
      (void) Join();
   }
}

Thread::Thread( const pthread_t aThread, const char * const aName ) :
      mThread( 0 ), mJoined( false ), mSystemThread( true )
{
   if ( NULL == aName ) {
      (void) snprintf( mName, kThreadNameLen, "t0x%lx", aThread );
   }
   else {
      strncpy( mName, aName, kThreadNameLen );
      mName[ kThreadNameLen - 1 ] = '\0';
   }

   // Set this object with specific key
   (void) pthread_setspecific( mThreadKey, this );
}

int32_t Thread::Start()
{
   int ret;
   pthread_attr_t attrs;


   printf( "creating thread %s\n", GetName() );

   ret = pthread_attr_init( &attrs );
   if ( ret ) {
      printf( "pthread_attr_init with error %d\n", ret );
   }

   if ( mPrio > 0 && 0 == geteuid() ) {
      struct sched_param prio;
      prio.sched_priority = sched_get_priority_max( SCHED_RR );
      (void) pthread_attr_setschedpolicy( &attrs, SCHED_RR );
      (void) pthread_attr_setschedparam( &attrs, &prio );
   }

   ret = pthread_create( &mThread, &attrs, start_thread,
            static_cast<void *>( this ) );

   if ( ret != 0 ) {
      printf( "Error from thread_create is %d\n", ret );
   }

   // Destroy attributes
   (void) pthread_attr_destroy( &attrs );

   return ret;
}

void Thread::Stop()
{
   OnStop();
}

int32_t Thread::Join()
{
   int32_t ret = 0;

   if ( !mJoined ) {
      if (*this == *GetCurrent()) {
         printf( "thread %s attempting to join itself: DEADLOCK\n", GetName() );
         return -1;
      }

      ret = pthread_join( mThread, NULL );
      if ( 0 == ret ) {
         mJoined = true;
      } else
         printf( "pthread_join(%s) failed with error %d\n", GetName(), ret );
   }
   return ret;
}

bool Thread::operator ==( const Thread &t ) const
{
   printf( "Thread::operator == Enter()\n" );
   return mThread == t.mThread;
}

void Thread::Exit()
{
   pthread_exit( NULL );
}

Thread *Thread::GetCurrent()
{
   Thread *t = reinterpret_cast<Thread*>( pthread_getspecific( mThreadKey ) );
   if (NULL == t) {
      t = new Thread( pthread_self(), NULL );
   }
   return t;
}

void Thread::Init()
{
   printf( "Thread::Init() Enter\n" );
   if (!mInited) {
      if (pthread_key_create( &mThreadKey, &thread_key_destructor ) != 0) {
         printf( "Failed to create thread key\n" );
      }
      mInited = true;

      new Thread( pthread_self(), "main" );
      Mutex::Init();
   }
}

void Thread::Destroy()
{
   mInited = false;
   Mutex::Destroy();
   //TODO
   //TimerManager::destroyManager();
}

void Thread::OnStop() const
{
   (void) pthread_cancel( mThread );
}

void *Thread::start_thread( void * const arg )
{
   Thread * const aThread = reinterpret_cast<Thread *>( arg );

   (void) pthread_setspecific( mThreadKey, aThread );

   printf( "started pid %d tid %d\n", getpid(),
            static_cast<uint>( pthread_self() ) );

   aThread->OnStart();

   return NULL;
}

void Thread::thread_key_destructor( void * const arg )
{
   Thread *t = static_cast<Thread *>( arg );
   if (t->mSystemThread) {
      delete t;
   }
}
