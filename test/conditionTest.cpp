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
 * conditionTest.cpp
 *
 *  Created on: Oct 6, 2017
 *      Author: lia1hc
 */

#include "Timer.h"
#include "Mutex.h"
#include "Condition.h"
#include <iostream>

using namespace std;
Condition c;
class TestClass {
public:
   TestClass();
   virtual ~TestClass();

   void testFunc();
private:
   Runnable<TestClass> *mThread;
};

TestClass::TestClass() {
   mThread = new Runnable<TestClass>( "conditionTest", this,
      &TestClass::testFunc );

   mThread->Start();
}

TestClass::~TestClass() {
   delete mThread;
}

void TestClass::testFunc() {
   cout << "TestClass::testFunc()\n";
   c.Broadcast();
}

int main( int argc, char* argv[] )
{
   TestClass *t = new TestClass;

   Mutex m;
   AutoLock l( m );

   cout << "hehe=" << c.Wait( m, 10000 ) << endl;

   delete t;
   return 0;
}
