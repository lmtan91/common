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
 * threadTest.cpp
 *
 *  Created on: Sep 27, 2017
 *      Author: lia1hc
 */

#include "Thread.h"
#include "Mutex.h"
#include "TestCase.h"

class ThreadTest: public TestCase {
public:
   ThreadTest() :
         TestCase( "ThreadTest" ) {
      SetTestName( "ThreadTest" );
   }

   virtual ~ThreadTest() {
      printf( "ThreadTest::~ThreadTest() Enter()\n" );
   }

private:
   void Run() {
      for (int i = 0; i < 10; i++)
      printf( "ThreadTest::Run() Enter()\n" );
   }
};

int main( int argc, char* argv[] )
{
   ThreadTest *t = new ThreadTest();
   printf( "ThreadTest th=%lu\n", pthread_self() );
   t->Start();
   printf( "ThreadTest End()\n" );
   delete t;

}


