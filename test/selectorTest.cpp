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
      }
   }
};

int EventTest::sEventCount = 0;
