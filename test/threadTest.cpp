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

#include "Mutex.h"
#include "TestCase.h"
#include "Thread.h"
#include <iostream>
#include <unistd.h>
using namespace std;
class TestClass {
public:
   TestClass( const char * name ) :
      mThread( name, this, &TestClass::mainThread, false )
      {
      mThread.Start();
      printf( "creating %p\n", &mThread );
   }
   virtual ~TestClass() {
   }

private:
   void mainThread() {
      while (1) {
      printf( "%s=%p\n", mThread.GetName(), mThread.GetCurrent() );
         sleep( 1 );
      }
   }
   Runnable<TestClass> mThread;
};

int main( int argc, char* argv[] )
{
   TestClass *t1 = new TestClass( "threadTest1" );
   TestClass *t2 = new TestClass( "threadTest2" );
   TestClass *t3 = new TestClass( "threadTest3" );
   (void) t1;
   (void) t2;
   (void) t3;
   while (1) {
   }
}


