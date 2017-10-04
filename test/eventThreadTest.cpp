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
 * eventThreadTest.cpp
 *
 *  Created on: Oct 4, 2017
 *      Author: lia1hc
 */


#include "EventThread.h"
#include "Timer.h"

#include <stdio.h>
#include <unistd.h>


int gEventCount = 0;

class TestClass {
public:
   TestClass();
   ~TestClass();

   void Func1();
   void Func2( uint32_t p1, uint16_t p2 );
   void Func3( uint32_t p1 );

   void end( int i );
   void cancel();

private:
   enum {
      EVENT_ID_EV1, EVENT_ID_EV2, EVENT_ID_EV3, EVENT_ID_END
   };

   struct Event1: public Event {
      Event1() :
               Event( EVENT_ID_EV1, PRIORITY_HIGH )
      {
         gEventCount++;
      }
      ~Event1()
      {
         gEventCount--;
      }
      SMART_CASTABLE( EVENT_ID_EV1 )
      ;
   };

   struct Event2: public Event {
      Event2( uint32_t p1, uint16_t p2 ) :
               Event( EVENT_ID_EV2 ), mP1( p1 ), mP2( p2 )
      {
         gEventCount++;
      }
      ~Event2()
      {
         gEventCount--;
      }

      SMART_CASTABLE( EVENT_ID_EV2 )
      ;

      uint32_t mP1;
      uint16_t mP2;
   };

   struct Event3: public Event {
      Event3( uint32_t p1 ) :
               Event( EVENT_ID_EV3 ), mP1( p1 )
      {
         gEventCount++;
      }
      ~Event3()
      {
         gEventCount--;
      }

      SMART_CASTABLE( EVENT_ID_EV3 )
      ;

      uint32_t mP1;
   };

   void ProcessFunc1( Event1 *ev );
   void ProcessFunc2( Event2 *ev );
   void ProcessFunc3( Event3 *ev );

   EventMethod<TestClass, Event1> *mEvent1Handler;
   EventMethod<TestClass, Event2> *mEvent2Handler;
   EventMethod<TestClass, Event3> *mEvent3Handler;

   EventThread mThread;
};

TestClass::TestClass()
{
   mEvent1Handler = new EventMethod<TestClass, Event1>( this,
            &TestClass::ProcessFunc1, &mThread );
   mEvent2Handler = new EventMethod<TestClass, Event2>( this,
            &TestClass::ProcessFunc2, &mThread );
   mEvent3Handler = new EventMethod<TestClass, Event3>( this,
            &TestClass::ProcessFunc3, &mThread );
}

TestClass::~TestClass()
{
   delete mEvent1Handler;
   delete mEvent2Handler;
   delete mEvent3Handler;
}

void TestClass::Func1()
{
   mThread.sendEvent( new Event1() );
}

void TestClass::Func2( uint32_t p1, uint16_t p2 )
{
   mThread.sendEventSync( new Event2( p1, p2 ) );
}

void TestClass::Func3( uint32_t p1 )
{
   mThread.sendEventSync( new Event3( p1 ) );
}

void TestClass::end( int i )
{
   mThread.sendTimedEvent( new Event3( 0 ), i * 1000 );
}

void TestClass::cancel()
{
   mThread.remove( Event3::GetEventId() );
}

void TestClass::ProcessFunc1( Event1 *ev )
{
}

void TestClass::ProcessFunc2( Event2 *ev )
{
   printf( "p1 %d p2 %d\n", ev->mP1, ev->mP2 );
}

void TestClass::ProcessFunc3( Event3 *ev )
{
   printf( "p1 %d\n", ev->mP1 );
}

int main( int argc, char*argv[] )
{
   printf( "Test Started\n" );

   TestClass *c = new TestClass;

   c->Func1();
   c->Func2( 1, 2 );
   c->Func3( 3 );

   c->end( 1 );
   c->end( 2 );

//   c->cancel();

   printf( "Event Count is %d\n", gEventCount );

   sleep( 3 );

   delete c;

   printf( "Event Count is %d\n", gEventCount );

   if ( gEventCount != 0 )
      printf( "Events leaked %d\n", gEventCount );

   return 0;
}

