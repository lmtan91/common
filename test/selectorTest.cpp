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
 * selectorTest.cpp
 *
 *  Created on: Oct 11, 2017
 *      Author: lia1hc
 */
#include <unistd.h>
#include "Selector.h"
#include "TestCase.h"

class EventTest : public TestCase {
public:
   EventTest( Selector *s, int number );
   virtual ~EventTest();

private:
   void Func1();
   void Func2( uint32_t p1, uint16_t p2 );
   int Func3( uint32_t p1 );

   enum {
      kEventEv1, kEventEv2, kEventEv3, kEventLast
   };

   struct Event1 : public Event {
      Event1() :
               Event( kEventEv1, PRIORITY_HIGH )
      {
         EventTest::sEventCount++;
      }

      ~Event1()
      {
         EventTest::sEventCount--;
      }
      SMART_CASTABLE( kEventEv1 )
   };

   struct Event2 : public Event {
      Event2( uint32_t p1, uint16_t p2 )
      :
               Event( kEventEv2 ), mP1( p1 ), mP2( p2 )
      {
         EventTest::sEventCount++;
      }
      ~Event2()
      {
         EventTest::sEventCount--;
      }

      SMART_CASTABLE(kEventEv2)

      uint32_t mP1;
      uint16_t mP2;
   };

   struct Event3 : public Event {
      Event3( uint32_t p1 ) :
               Event( kEventEv3 ), mP1( p1 ), mValid( true )
      {
         EventTest::sEventCount++;
      }
      ~Event3()
      {
         mValid = false;
         EventTest::sEventCount--;
      }
      SMART_CASTABLE(kEventEv3)

      uint32_t mP1;
      bool mValid;
   };

   void ProcessFunc1( Event1 *ev );
   void ProcessFunc2( Event2 *ev );
   void ProcessFunc3( Event3 *ev );

   EventMethod<EventTest, Event1> *mEvent1Handler;
   EventMethod<EventTest, Event2> *mEvent2Handler;
   EventMethod<EventTest, Event3> *mEvent3Handler;

   int mTestNum;
   int mTestState;
   static int sEventCount;
   Selector *mSelector;

   void Run()
   {
      mTestState = 0;

      switch (mTestNum) {
      case 1:
         Func1();
         usleep( 100000 );
         if ( mTestState != 1 ) {
            TestFailed( "Event not received\n" );
         }
         else if ( sEventCount != 0 ) {
            TestFailed( "Event count != 0 (%d)", sEventCount );
         }
         else {
            TestPassed();
         }
         break;
      case 2:
         Func2( 1, 2 );
         if ( mTestState != 1 ) {
            TestFailed( "Event not received\n" );
         }
         else if ( sEventCount != 0 ) {
            TestFailed( "Event count != 0 (%d)", sEventCount );
         } else {
            TestPassed();
         }
         break;

      case 3:
         int res = Func3( 3 );
         // Sync Event no need to sleep.
         if ( mTestState != 1 )
            TestFailed( "Event not recieved" );
         else if ( res != 1000 )
            TestFailed( "Result not valid" );
         else if ( sEventCount != 0 )
            TestFailed( "Event count != 0 (%d)", sEventCount );
         else
            TestPassed();
         break;
      }
   }
};

int EventTest::sEventCount = 0;

EventTest::EventTest( Selector *s, int number ) :
         TestCase( "EventTest" ), mTestNum( number ), mTestState( 0 ), mSelector(
                  s )
{

   switch (number) {
   case 1:
      SetTestName( "AsyncEvent" );
      break;
   case 2:
      SetTestName( "SyncEvent" );
      break;
   case 3:
      SetTestName( "RetSyncEvent" );
      break;
   }
   mEvent1Handler = new EventMethod<EventTest, Event1>( this,
      &EventTest::ProcessFunc1, mSelector );
   mEvent2Handler = new EventMethod<EventTest, Event2>( this,
      &EventTest::ProcessFunc2, mSelector );
   mEvent3Handler = new EventMethod<EventTest, Event3>( this,
      &EventTest::ProcessFunc3, mSelector );
}

EventTest::~EventTest() {
   delete mEvent1Handler;
   delete mEvent2Handler;
   delete mEvent3Handler;
}

void EventTest::Func1() {
   printf( "EventTest::Func1()\n" );
   mSelector->sendEvent( new Event1() );
}

int EventTest::Func3( uint32_t p1 )
{
   SmartPtr<Event3> ev = new Event3( p1 );
   mSelector->sendEventSync( ev );
   if ( ev->mValid == false )
      TestFailed( "Ev had been deleted" );
   return ev->mP1;
}

void EventTest::Func2( uint32_t p1, uint16_t p2 )
{
   mSelector->sendEventSync( new Event2( p1, p2 ) );
}

void EventTest::ProcessFunc1( Event1 *ev )
{
   mTestState++;
}

void EventTest::ProcessFunc2( Event2 *ev )
{
   printf( "p1 %d p2 %d\n", ev->mP1, ev->mP2 );
   mTestState++;
   if ( ev->mP1 != 1 || ev->mP2 != 2 ) {
      TestFailed( "P1 and P2 not expected values\n" );
   }
}

void EventTest::ProcessFunc3( Event3 *ev )
{
   printf( "p1 %d \n", ev->mP1 );
   mTestState++;
   if ( ev->mP1 != 3 ) {
      TestFailed( "P1 and P2 not expected values\n" );
   }
   ev->mP1 = 1000;
}

int main( int argc, char* argv[] )
{
   TestRunner runner( argv[ 0 ] );

   TestCase *test_set[ 10 ];
   Selector testSelector;

   test_set[ 0 ] = new EventTest( &testSelector, 1 );
   test_set[ 1 ] = new EventTest( &testSelector, 2 );
   test_set[ 2 ] = new EventTest( &testSelector, 3 );
   runner.RunAll( test_set, 3 );

   return 0;
}
