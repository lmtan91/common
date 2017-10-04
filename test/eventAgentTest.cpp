
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include "EventAgent.h"
#include "EventThread.h"

class TestClass;
typedef AsyncEventAgent0<TestClass> SendAsyncEventAgent0;
class TestClass {
public:
   TestClass() :
            mTheBool( false )
   {
      printf( "TestClass()\n" );
   }
   ~TestClass()
   {
   }

   void AsyncFunc0()
   {
      SendAsyncEventAgent0 *agent = new SendAsyncEventAgent0(
               this, &TestClass::handleAsyncFunc0 );
      agent->send( &mEventThread );
   }
   void AsyncFunc1( uint32_t p1 )
   {
      AsyncEventAgent1<TestClass, uint32_t> *agent = new AsyncEventAgent1<
               TestClass, uint32_t>( this, &TestClass::handleAsyncFunc1, p1 );
      agent->send( &mEventThread );
   }
   void AsyncFunc2( uint32_t p1, int16_t p2 )
   {
      AsyncEventAgent2<TestClass, uint32_t, int16_t> *agent =
               new AsyncEventAgent2<TestClass, uint32_t, int16_t>( this,
                        &TestClass::handleAsyncFunc2, p1, p2 );
      agent->send( &mEventThread );
   }
   void AsyncFunc3( uint32_t p1, int16_t p2, TestClass *p3 )
   {
      AsyncEventAgent3<TestClass, uint32_t, int16_t, TestClass*> *agent =
               new AsyncEventAgent3<TestClass, uint32_t, int16_t, TestClass*>(
                        this, &TestClass::handleAsyncFunc3, p1, p2, p3 );
      agent->send( &mEventThread );
   }
   void AsyncFunc4( uint32_t p1, int16_t p2, TestClass *p3, char *p4 )
   {
      AsyncEventAgent4<TestClass, uint32_t, int16_t, TestClass*, char*> *agent =
               new AsyncEventAgent4<TestClass, uint32_t, int16_t, TestClass*,
                        char*>( this, &TestClass::handleAsyncFunc4, p1, p2, p3,
                        p4 );
      agent->send( &mEventThread );
   }
   void AsyncFunc5( uint32_t p1, int16_t p2, TestClass *p3, char *p4, int p5 )
   {
      AsyncEventAgent5<TestClass, uint32_t, int16_t, TestClass*, char*, int> *agent =
               new AsyncEventAgent5<TestClass, uint32_t, int16_t, TestClass*,
                        char*, int>( this, &TestClass::handleAsyncFunc5, p1, p2,
                        p3, p4, p5 );
      agent->send( &mEventThread );
   }
   void AsyncFunc6( uint32_t p1, int16_t p2, TestClass *p3, char *p4, int p5,
            bool p6 )
   {
      AsyncEventAgent6<TestClass, uint32_t, int16_t, TestClass*, char*, int,
               bool> *agent = new AsyncEventAgent6<TestClass, uint32_t, int16_t,
               TestClass*, char*, int, bool>( this,
               &TestClass::handleAsyncFunc6, p1, p2, p3, p4, p5, p6 );
      agent->send( &mEventThread );
   }
   void AsyncFunc7( uint32_t p1, int16_t p2, TestClass *p3, char *p4, int p5,
            bool p6, char p7 )
   {
      AsyncEventAgent7<TestClass, uint32_t, int16_t, TestClass*, char*, int,
               bool, char> *agent = new AsyncEventAgent7<TestClass, uint32_t,
               int16_t, TestClass*, char*, int, bool, char>( this,
               &TestClass::handleAsyncFunc7, p1, p2, p3, p4, p5, p6, p7 );
      agent->send( &mEventThread );
   }
   void AsyncFunc8( uint32_t p1, int16_t p2, TestClass *p3, char *p4, int p5,
            bool p6, char p7, uint8_t p8 )
   {
      AsyncEventAgent8<TestClass, uint32_t, int16_t, TestClass*, char*, int,
               bool, char, uint8_t> *agent = new AsyncEventAgent8<TestClass,
               uint32_t, int16_t, TestClass*, char*, int, bool, char, uint8_t>(
               this, &TestClass::handleAsyncFunc8, p1, p2, p3, p4, p5, p6, p7,
               p8 );
      agent->send( &mEventThread );
   }
   void AsyncFunc9( uint32_t p1, int16_t p2, TestClass *p3, char *p4, int p5,
            bool p6, char p7, uint8_t p8, uint16_t p9 )
   {
      AsyncEventAgent9<TestClass, uint32_t, int16_t, TestClass*, char*, int,
               bool, char, uint8_t, uint16_t> *agent = new AsyncEventAgent9<
               TestClass, uint32_t, int16_t, TestClass*, char*, int, bool, char,
               uint8_t, uint16_t>( this, &TestClass::handleAsyncFunc9, p1, p2,
               p3, p4, p5, p6, p7, p8, p9 );
      agent->send( &mEventThread );
   }
   void AsyncFunc10( uint32_t p1, int16_t p2, TestClass *p3, char *p4, int p5,
            bool p6, char p7, uint8_t p8, uint16_t p9, uint64_t p10 )
   {
      AsyncEventAgent10<TestClass, uint32_t, int16_t, TestClass*, char*, int,
               bool, char, uint8_t, uint16_t, uint64_t> *agent =
               new AsyncEventAgent10<TestClass, uint32_t, int16_t, TestClass*,
                        char*, int, bool, char, uint8_t, uint16_t, uint64_t>(
                        this, &TestClass::handleAsyncFunc10, p1, p2, p3, p4, p5,
                        p6, p7, p8, p9, p10 );
      agent->send( &mEventThread );
   }

   void SyncFunc0()
   {
      SyncEventAgent0<TestClass> *agent = new SyncEventAgent0<TestClass>( this,
               &TestClass::handleSyncFunc0 );
      agent->send( &mEventThread );
   }

   void SyncFunc1( uint64_t &p1 )
   {
      SyncEventAgent1<TestClass, uint64_t &> *agent = new SyncEventAgent1<
               TestClass, uint64_t &>( this, &TestClass::handleSyncFunc1, p1 );
      agent->send( &mEventThread );
      printf( "Sync1 result %lu\n", p1 );

   }

   void SyncFunc2( uint64_t &p1, uint32_t &p2 )
   {
      SyncEventAgent2<TestClass, uint64_t &, uint32_t &> *agent =
               new SyncEventAgent2<TestClass, uint64_t &, uint32_t &>( this,
                        &TestClass::handleSyncFunc2, p1, p2 );
      agent->send( &mEventThread );
      printf( "Sync2 result %lu, %u\n", p1, p2 );
   }

   void SyncFunc3( uint64_t &p1, uint32_t &p2, uint16_t &p3 )
   {
      SyncEventAgent3<TestClass, uint64_t &, uint32_t &, uint16_t &> *agent =
               new SyncEventAgent3<TestClass, uint64_t &, uint32_t &, uint16_t &>(
                        this, &TestClass::handleSyncFunc3, p1, p2, p3 );
      agent->send( &mEventThread );
      printf( "Sync3 result %lu, %u, %u\n", p1, p2, p3 );
   }

   void SyncFunc4( uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4 )
   {
      SyncEventAgent4<TestClass, uint64_t &, uint32_t &, uint16_t &, uint8_t &> *agent =
               new SyncEventAgent4<TestClass, uint64_t &, uint32_t &,
                        uint16_t &, uint8_t &>( this,
                        &TestClass::handleSyncFunc4, p1, p2, p3, p4 );
      agent->send( &mEventThread );
      printf( "Sync4 result %lu, %u, %u, %u\n", p1, p2, p3, p4 );
   }

   void SyncFunc5( uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4,
            int64_t &p5 )
   {
      SyncEventAgent5<TestClass, uint64_t &, uint32_t &, uint16_t &, uint8_t &,
               int64_t &> *agent = new SyncEventAgent5<TestClass, uint64_t &,
               uint32_t &, uint16_t &, uint8_t &, int64_t &>( this,
               &TestClass::handleSyncFunc5, p1, p2, p3, p4, p5 );
      agent->send( &mEventThread );
      printf( "Sync5 result %lu, %u, %u, %u, %ld\n", p1, p2, p3, p4, p5 );
   }

   void SyncFunc6( uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4,
            int64_t &p5, int32_t &p6 )
   {
      SyncEventAgent6<TestClass, uint64_t &, uint32_t &, uint16_t &, uint8_t &,
               int64_t &, int32_t &> *agent = new SyncEventAgent6<TestClass,
               uint64_t &, uint32_t &, uint16_t &, uint8_t &, int64_t &,
               int32_t &>( this, &TestClass::handleSyncFunc6, p1, p2, p3, p4,
               p5, p6 );
      agent->send( &mEventThread );
      printf( "Sync6 result %lu, %u, %u, %u, %ld, %d\n", p1, p2, p3, p4, p5,
               p6 );
   }

   void SyncFunc7( uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4,
            int64_t &p5, int32_t &p6, int16_t &p7 )
   {
      SyncEventAgent7<TestClass, uint64_t &, uint32_t &, uint16_t &, uint8_t &,
               int64_t &, int32_t &, int16_t &> *agent = new SyncEventAgent7<
               TestClass, uint64_t &, uint32_t &, uint16_t &, uint8_t &,
               int64_t &, int32_t &, int16_t &>( this,
               &TestClass::handleSyncFunc7, p1, p2, p3, p4, p5, p6, p7 );
      agent->send( &mEventThread );
      printf( "Sync7 result %lu, %u, %u, %u, %ld, %d, %d\n", p1, p2, p3, p4, p5,
               p6, p7 );
   }

   void SyncFunc8( uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4,
            int64_t &p5, int32_t &p6, int16_t &p7, int8_t &p8 )
   {
      SyncEventAgent8<TestClass, uint64_t &, uint32_t &, uint16_t &, uint8_t &,
               int64_t &, int32_t &, int16_t &, int8_t &> *agent =
               new SyncEventAgent8<TestClass, uint64_t &, uint32_t &,
                        uint16_t &, uint8_t &, int64_t &, int32_t &, int16_t &,
                        int8_t &>( this, &TestClass::handleSyncFunc8, p1, p2,
                        p3, p4, p5, p6, p7, p8 );
      agent->send( &mEventThread );
      printf( "Sync8 result %lu, %u, %u, %u, %ld, %d, %d, %d\n", p1, p2, p3, p4,
               p5, p6, p7, p8 );
   }

   void SyncFunc9( uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4,
            int64_t &p5, int32_t &p6, int16_t &p7, int8_t &p8, bool &p9 )
   {
      SyncEventAgent9<TestClass, uint64_t &, uint32_t &, uint16_t &, uint8_t &,
               int64_t &, int32_t &, int16_t &, int8_t &, bool&> *agent =
               new SyncEventAgent9<TestClass, uint64_t &, uint32_t &,
                        uint16_t &, uint8_t &, int64_t &, int32_t &, int16_t &,
                        int8_t &, bool&>( this, &TestClass::handleSyncFunc9, p1,
                        p2, p3, p4, p5, p6, p7, p8, p9 );
      agent->send( &mEventThread );
      printf( "Sync9 result %lu, %u, %u, %u, %ld, %d, %d, %d, %d\n", p1, p2, p3,
               p4, p5, p6, p7, p8, p9 );
   }

   void SyncFunc10( uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4,
            int64_t &p5, int32_t &p6, int16_t &p7, int8_t &p8, bool &p9,
            char &p10 )
   {
      SyncEventAgent10<TestClass, uint64_t &, uint32_t &, uint16_t &, uint8_t &,
               int64_t &, int32_t &, int16_t &, int8_t &, bool&, char&> *agent =
               new SyncEventAgent10<TestClass, uint64_t &, uint32_t &,
                        uint16_t &, uint8_t &, int64_t &, int32_t &, int16_t &,
                        int8_t &, bool&, char&>( this,
                        &TestClass::handleSyncFunc10, p1, p2, p3, p4, p5, p6,
                        p7, p8, p9, p10 );
      agent->send( &mEventThread );
      printf( "Sync10 result %lu, %u, %u, %u, %ld, %d, %d, %d, %d, %c\n", p1,
               p2, p3, p4, p5, p6, p7, p8, p9, p10 );
   }

   bool SyncRetFunc0()
   {
      if ( not mEventThread.isThreadCurrent() ) {
         bool result = false;
         SyncRetEventAgent0<TestClass, bool>* agent = new SyncRetEventAgent0<
                  TestClass, bool>( this, &TestClass::SyncRetFunc0 );
         result = agent->send( &mEventThread );
         printf( "SyncRet0 result RetVal(%d)\n", result );
         return result;
      }
      printf( "Received\n" );
      return true;
   }

   bool SyncRetFunc1( uint64_t &p1 )
   {
      if ( not mEventThread.isThreadCurrent() ) {
         bool result = false;
         SyncRetEventAgent1<TestClass, bool, uint64_t &>* agent =
                  new SyncRetEventAgent1<TestClass, bool, uint64_t &>( this,
                           &TestClass::SyncRetFunc1, p1 );
         result = agent->send( &mEventThread );
         printf( "RetVal(%d) p1(%lu)\n", result, p1 );
         return result;
      }

      printf( "Received p1(%lu)\n", p1 );
      p1 += 1024;
      return true;
   }

   bool SyncRetFunc2( uint64_t &p1, uint32_t p2 )
   {
      if ( not mEventThread.isThreadCurrent() ) {
         bool result = false;
         SyncRetEventAgent2<TestClass, bool, uint64_t &, uint32_t>* agent =
                  new SyncRetEventAgent2<TestClass, bool, uint64_t &, uint32_t>(
                           this, &TestClass::SyncRetFunc2, p1, p2 );
         result = agent->send( &mEventThread );
         printf( "RetVal(%d) p1(%lu) p2(%u)\n", result, p1, p2 );
         return result;
      }

      printf( "Received p1(%lu) p2(%u)\n", p1, p2 );
      p1 += 1024;
      p2 += 512;
      return true;
   }

   bool SyncRetFunc3( uint64_t& p1, uint32_t p2, uint16_t& p3 )
   {
      if ( not mEventThread.isThreadCurrent() ) {
         bool result = false;
         SyncRetEventAgent3<TestClass, bool, uint64_t &, uint32_t, uint16_t &>* agent =
                  new SyncRetEventAgent3<TestClass, bool, uint64_t &, uint32_t,
                           uint16_t &>( this, &TestClass::SyncRetFunc3, p1, p2,
                           p3 );
         result = agent->send( &mEventThread );
         printf( "RetVal(%d) p1(%lu) p2(%u) p3(%u)\n", result, p1, p2, p3 );
         return result;
      }

      printf( "Received p1(%lu) p2(%u) p3(%u)\n", p1, p2, p3 );
      p1 += 1024;
      p2 += 512;
      p3 += 256;
      return true;
   }

   class CopyCounterGuy {
   public:
      CopyCounterGuy() :
               mCount( 0 )
      {
      }
      CopyCounterGuy( const CopyCounterGuy& other ) :
               mCount( other.mCount + 1 )
      {
      }
      const CopyCounterGuy& operator=( const CopyCounterGuy& other )
      {
         mCount = other.mCount + 1;
         return *this;
      }
      int getCount()
      {
         return mCount;
      }
   private:
      int mCount;
   };

   bool SyncRetFunc4( uint64_t& p1, uint32_t p2, uint16_t& p3,
            CopyCounterGuy& ccg1 )
   {
      if ( not mEventThread.isThreadCurrent() ) {
         bool result = false;
         SyncRetEventAgent4<TestClass, bool, uint64_t &, uint32_t, uint16_t &,
                  CopyCounterGuy &>* agent = new SyncRetEventAgent4<TestClass,
                  bool, uint64_t &, uint32_t, uint16_t &, CopyCounterGuy &>(
                  this, &TestClass::SyncRetFunc4, p1, p2, p3, ccg1 );
         result = agent->send( &mEventThread );
         printf( "RetVal(%d) p1(%lu) p2(%u) p3(%u) ccg1(%d)\n", result, p1, p2,
                  p3, ccg1.getCount() );
         return result;
      }

      printf( "Received p1(%lu) p2(%u) p3(%u) ccg1(%d)\n", p1, p2, p3,
               ccg1.getCount() );
      p1 += 1024;
      p2 += 512;
      p3 += 256;
      return true;
   }

   CopyCounterGuy SyncRetFunc5( uint64_t& p1, uint32_t p2, uint16_t& p3,
            CopyCounterGuy& ccg1, uint8_t& p4 )
   {
      if ( not mEventThread.isThreadCurrent() ) {
         CopyCounterGuy result;
         SyncRetEventAgent5<TestClass, CopyCounterGuy, uint64_t &, uint32_t,
                  uint16_t &, CopyCounterGuy &, uint8_t &>* agent =
                  new SyncRetEventAgent5<TestClass, CopyCounterGuy, uint64_t &,
                           uint32_t, uint16_t &, CopyCounterGuy &, uint8_t&>(
                           this, &TestClass::SyncRetFunc5, p1, p2, p3, ccg1,
                           p4 );
         result = agent->send( &mEventThread );
         printf( "RetVal(%d) p1(%lu) p2(%u) p3(%u) ccg1(%d) p4(%u)\n",
                  result.getCount(), p1, p2, p3, ccg1.getCount(), p4 );
         return result;
      }

      printf( "Received p1(%lu) p2(%u) p3(%u) ccg1(%d) p4(%u)\n", p1, p2, p3,
               ccg1.getCount(), p4 );
      p1 += 1024;
      p2 += 512;
      p3 += 256;
      p4 += 16;
      CopyCounterGuy newGuy;
      return newGuy;
   }

   bool SyncRetFunc6( uint64_t& p1, uint32_t p2, uint16_t& p3,
            CopyCounterGuy& ccg1, uint8_t& p4, int64_t& p5 )
   {
      if ( not mEventThread.isThreadCurrent() ) {
         bool result;
         SyncRetEventAgent6<TestClass, bool, uint64_t &, uint32_t, uint16_t &,
                  CopyCounterGuy &, uint8_t &, int64_t&>* agent =
                  new SyncRetEventAgent6<TestClass, bool, uint64_t &, uint32_t,
                           uint16_t &, CopyCounterGuy &, uint8_t&, int64_t&>(
                           this, &TestClass::SyncRetFunc6, p1, p2, p3, ccg1, p4,
                           p5 );
         result = agent->send( &mEventThread );
         printf( "RetVal(%d) p1(%lu) p2(%u) p3(%u) ccg1(%d) p4(%u) p5(%ld)\n",
                  result, p1, p2, p3, ccg1.getCount(), p4, p5 );
         return result;
      }

      printf( "Received p1(%lu) p2(%u) p3(%u) ccg1(%d) p4(%u) p5(%ld)\n", p1,
               p2, p3, ccg1.getCount(), p4, p5 );
      p1 += 1024;
      p2 += 512;
      p3 += 256;
      p4 += 16;
      p5 -= 1024;
      return true;
   }

   bool SyncRetFunc7( uint64_t& p1, uint32_t p2, uint16_t& p3,
            CopyCounterGuy& ccg1, uint8_t& p4, int64_t& p5, int32_t &p6 )
   {
      if ( not mEventThread.isThreadCurrent() ) {
         bool result;
         SyncRetEventAgent7<TestClass, bool, uint64_t &, uint32_t, uint16_t &,
                  CopyCounterGuy &, uint8_t &, int64_t&, int32_t&>* agent =
                  new SyncRetEventAgent7<TestClass, bool, uint64_t &, uint32_t,
                           uint16_t &, CopyCounterGuy &, uint8_t&, int64_t&,
                           int32_t&>( this, &TestClass::SyncRetFunc7, p1, p2,
                           p3, ccg1, p4, p5, p6 );
         result = agent->send( &mEventThread );
         printf(
                  "RetVal(%d) p1(%lu) p2(%u) p3(%u) ccg1(%d) p4(%u) p5(%ld) p6(%d)\n",
                  result, p1, p2, p3, ccg1.getCount(), p4, p5, p6 );
         return result;
      }

      printf( "Received p1(%lu) p2(%u) p3(%u) ccg1(%d) p4(%u) p5(%ld) p6(%d)\n",
               p1, p2, p3, ccg1.getCount(), p4, p5, p6 );
      p1 += 1024;
      p2 += 512;
      p3 += 256;
      p4 += 16;
      p5 -= 1024;
      p6 -= 512;
      return true;
   }

   bool SyncRetFunc8( uint64_t& p1, uint32_t p2, uint16_t& p3,
            CopyCounterGuy& ccg1, uint8_t& p4, int64_t& p5, int32_t &p6,
            int16_t &p7 )
   {
      if ( not mEventThread.isThreadCurrent() ) {
         bool result;
         SyncRetEventAgent8<TestClass, bool, uint64_t &, uint32_t, uint16_t &,
                  CopyCounterGuy &, uint8_t &, int64_t&, int32_t&, int16_t&>* agent =
                  new SyncRetEventAgent8<TestClass, bool, uint64_t &, uint32_t,
                           uint16_t &, CopyCounterGuy &, uint8_t&, int64_t&,
                           int32_t&, int16_t&>( this, &TestClass::SyncRetFunc8,
                           p1, p2, p3, ccg1, p4, p5, p6, p7 );
         result = agent->send( &mEventThread );
         printf(
                  "RetVal(%d) p1(%lu) p2(%u) p3(%u) ccg1(%d) p4(%u) p5(%ld) p6(%d) p7(%d)\n",
                  result, p1, p2, p3, ccg1.getCount(), p4, p5, p6, p7 );
         return result;
      }

      printf(
               "Received p1(%lu) p2(%u) p3(%u) ccg1(%d) p4(%u) p5(%ld) p6(%d) p7(%d)\n",
               p1, p2, p3, ccg1.getCount(), p4, p5, p6, p7 );
      p1 += 1024;
      p2 += 512;
      p3 += 256;
      p4 += 16;
      p5 -= 1024;
      p6 -= 512;
      p7 -= 256;
      return true;
   }

   bool SyncRetFunc9( uint64_t& p1, uint32_t p2, uint16_t& p3,
            CopyCounterGuy& ccg1, uint8_t& p4, int64_t& p5, int32_t &p6,
            int16_t &p7, int8_t& p8 )
   {
      if ( not mEventThread.isThreadCurrent() ) {
         bool result;
         SyncRetEventAgent9<TestClass, bool, uint64_t &, uint32_t, uint16_t &,
                  CopyCounterGuy &, uint8_t &, int64_t&, int32_t&, int16_t&,
                  int8_t&>* agent = new SyncRetEventAgent9<TestClass, bool,
                  uint64_t &, uint32_t, uint16_t &, CopyCounterGuy &, uint8_t&,
                  int64_t&, int32_t&, int16_t&, int8_t&>( this,
                  &TestClass::SyncRetFunc9, p1, p2, p3, ccg1, p4, p5, p6, p7,
                  p8 );
         result = agent->send( &mEventThread );
         printf(
                  "RetVal(%d) p1(%lu) p2(%u) p3(%u) ccg1(%d) p4(%u) p5(%ld) p6(%d) p7(%d) p8(%d)\n",
                  result, p1, p2, p3, ccg1.getCount(), p4, p5, p6, p7, p8 );
         return result;
      }

      printf(
               "Received p1(%lu) p2(%u) p3(%u) ccg1(%d) p4(%u) p5(%ld) p6(%d) p7(%d) p8(%d)\n",
               p1, p2, p3, ccg1.getCount(), p4, p5, p6, p7, p8 );
      p1 += 1024;
      p2 += 512;
      p3 += 256;
      p4 += 16;
      p5 -= 1024;
      p6 -= 512;
      p7 -= 256;
      p8 -= 16;
      return true;
   }

   bool SyncRetFunc10( uint64_t& p1, uint32_t p2, uint16_t& p3,
            CopyCounterGuy& ccg1, uint8_t& p4, int64_t& p5, int32_t &p6,
            int16_t &p7, int8_t& p8, bool& p9 )
   {
      if ( not mEventThread.isThreadCurrent() ) {
         bool result;
         SyncRetEventAgent10<TestClass, bool, uint64_t &, uint32_t, uint16_t &,
                  CopyCounterGuy &, uint8_t &, int64_t&, int32_t&, int16_t&,
                  int8_t&, bool&>* agent = new SyncRetEventAgent10<TestClass,
                  bool, uint64_t &, uint32_t, uint16_t &, CopyCounterGuy &,
                  uint8_t&, int64_t&, int32_t&, int16_t&, int8_t&, bool&>( this,
                  &TestClass::SyncRetFunc10, p1, p2, p3, ccg1, p4, p5, p6, p7,
                  p8, p9 );
         result = agent->send( &mEventThread );
         printf(
                  "RetVal(%d) p1(%lu) p2(%u) p3(%u) ccg1(%d) p4(%u) p5(%ld) p6(%d) p7(%d) p8(%d) p9(%d)\n",
                  result, p1, p2, p3, ccg1.getCount(), p4, p5, p6, p7, p8, p9 );
         return result;
      }

      printf(
               "Received p1(%lu) p2(%u) p3(%u) ccg1(%d) p4(%u) p5(%ld) p6(%d) p7(%d) p8(%d) p9(%d)\n",
               p1, p2, p3, ccg1.getCount(), p4, p5, p6, p7, p8, p9 );
      p1 += 1024;
      p2 += 512;
      p3 += 256;
      p4 += 16;
      p5 -= 1024;
      p6 -= 512;
      p7 -= 256;
      p8 -= 16;
      p9 = !p9;
      return true;
   }

protected:
   bool mTheBool;
   void handleAsyncFunc0()
   {
      printf( "Received Async0\n" );
   }
   void handleAsyncFunc1( uint32_t p1 )
   {
      printf( "Received Async1 %u\n", p1 );
   }
   void handleAsyncFunc2( uint32_t p1, int16_t p2 )
   {
      printf( "Received Async2 %u, %d\n", p1, p2 );
   }
   void handleAsyncFunc3( uint32_t p1, int16_t p2, TestClass *p3 )
   {
      printf( "Received Async3 %u, %d, %p\n", p1, p2, p3 );
   }
   void handleAsyncFunc4( uint32_t p1, int16_t p2, TestClass *p3, char *p4 )
   {
      printf( "Received Async4 %u, %d, %p, %s\n", p1, p2, p3, p4 );
   }
   void handleAsyncFunc5( uint32_t p1, int16_t p2, TestClass *p3, char *p4,
            int p5 )
   {
      printf( "Received Async5 %u, %d, %p, %s, %d\n", p1, p2, p3, p4, p5 );
   }
   void handleAsyncFunc6( uint32_t p1, int16_t p2, TestClass *p3, char *p4,
            int p5, bool p6 )
   {
      printf( "Received Async6 %u, %d, %p, %s, %d, %d\n", p1, p2, p3, p4, p5,
               p6 );
   }
   void handleAsyncFunc7( uint32_t p1, int16_t p2, TestClass *p3, char *p4,
            int p5, bool p6, char p7 )
   {
      printf( "Received Async7 %u, %d, %p, %s, %d, %d, %c\n", p1, p2, p3, p4,
               p5,
               p6, p7 );
   }
   void handleAsyncFunc8( uint32_t p1, int16_t p2, TestClass *p3, char *p4,
            int p5, bool p6, char p7, uint8_t p8 )
   {
      printf( "Received Async8 %u, %d, %p, %s, %d, %d, %c, %d\n", p1, p2, p3,
               p4,
               p5, p6, p7, p8 );
   }
   void handleAsyncFunc9( uint32_t p1, int16_t p2, TestClass *p3, char *p4,
            int p5, bool p6, char p7, uint8_t p8, uint16_t p9 )
   {
      printf( "Received Async9 %u, %d, %p, %s, %d, %d, %c, %d, %d\n", p1, p2,
               p3,
               p4, p5, p6, p7, p8, p9 );
   }
   void handleAsyncFunc10( uint32_t p1, int16_t p2, TestClass *p3, char *p4,
            int p5, bool p6, char p7, uint8_t p8, uint16_t p9, uint64_t p10 )
   {
      printf( "Received Async10 %u, %d, %p, %s, %d, %d, %c, %d, %d, %lu\n", p1,
               p2, p3, p4, p5, p6, p7, p8, p9, p10 );
   }

   void handleSyncFunc0()
   {
      printf( "Received Sync0\n" );
   }

   void handleSyncFunc1( uint64_t &p1 )
   {
      printf( "Received Sync1, %lu\n", p1 );
      p1 += 1024;
   }

   void handleSyncFunc2( uint64_t &p1, uint32_t &p2 )
   {
      printf( "Received Sync2, %lu, %u\n", p1, p2 );
      p1 += 1024;
      p2 += 512;
   }

   void handleSyncFunc3( uint64_t &p1, uint32_t &p2, uint16_t &p3 )
   {
      printf( "Received Sync3, %lu, %u, %u\n", p1, p2, p3 );
      p1 += 1024;
      p2 += 512;
      p3 += 256;
   }

   void handleSyncFunc4( uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4 )
   {
      printf( "Received Sync4, %lu, %u, %u, %u\n", p1, p2, p3, p4 );
      p1 += 1024;
      p2 += 512;
      p3 += 256;
      p4 += 16;
   }

   void handleSyncFunc5( uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4,
            int64_t &p5 )
   {
      printf( "Received Sync5, %lu, %u, %u, %u, %ld\n", p1, p2, p3, p4, p5 );
      p1 += 1024;
      p2 += 512;
      p3 += 256;
      p4 += 16;
      p5 -= 1024;
   }

   void handleSyncFunc6( uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4,
            int64_t &p5, int32_t &p6 )
   {
      printf( "Received Sync6, %lu, %u, %u, %u, %ld, %d\n", p1, p2, p3, p4,
               p5,
               p6 );
      p1 += 1024;
      p2 += 512;
      p3 += 256;
      p4 += 16;
      p5 -= 1024;
      p6 -= 512;
   }

   void handleSyncFunc7( uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4,
            int64_t &p5, int32_t &p6, int16_t &p7 )
   {
      printf( "Received Sync7, %lu, %u, %u, %u, %ld, %d, %d\n", p1, p2, p3,
               p4,
               p5, p6, p7 );
      p1 += 1024;
      p2 += 512;
      p3 += 256;
      p4 += 16;
      p5 -= 1024;
      p6 -= 512;
      p7 -= 256;
   }

   void handleSyncFunc8( uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4,
            int64_t &p5, int32_t &p6, int16_t &p7, int8_t &p8 )
   {
      printf( "Received Sync8, %lu, %u, %u, %u, %ld, %d, %d, %d\n", p1, p2,
               p3,
               p4, p5, p6, p7, p8 );
      p1 += 1024;
      p2 += 512;
      p3 += 256;
      p4 += 16;
      p5 -= 1024;
      p6 -= 512;
      p7 -= 256;
      p8 -= 16;
   }

   void handleSyncFunc9( uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4,
            int64_t &p5, int32_t &p6, int16_t &p7, int8_t &p8, bool &p9 )
   {
      printf( "Received Sync9, %lu, %u, %u, %u, %ld, %d, %d, %d, %d\n", p1,
               p2,
               p3, p4, p5, p6, p7, p8, p9 );
      p1 += 1024;
      p2 += 512;
      p3 += 256;
      p4 += 16;
      p5 -= 1024;
      p6 -= 512;
      p7 -= 256;
      p8 -= 16;
      p9 = !p9;
   }

   void handleSyncFunc10( uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4,
            int64_t &p5, int32_t &p6, int16_t &p7, int8_t &p8, bool &p9,
            char &p10 )
   {
      printf( "Received Sync10, %lu, %u, %u, %u, %ld, %d, %d, %d, %d, %c\n",
               p1,
               p2, p3, p4, p5, p6, p7, p8, p9, p10 );
      p1 += 1024;
      p2 += 512;
      p3 += 256;
      p4 += 16;
      p5 -= 1024;
      p6 -= 512;
      p7 -= 256;
      p8 -= 16;
      p9 = !p9;
      p10 = toupper( p10 );
   }

   EventThread mEventThread;

};

void runAsyncTests()
{
   TestClass app;

   uint32_t p1 = 102910;
   int16_t p2 = -42;
   TestClass *p3 = &app;
   char * p4 = (char *) "Hello World\n";
   int p5 = -2910;
   bool p6 = true;
   char p7 = 'a';
   uint8_t p8 = 255;
   uint16_t p9 = (uint16_t) -1;
   uint64_t p10 = (uint64_t) -1;

   printf( "BEFIRE call Async\n" );
   app.AsyncFunc0();
   app.AsyncFunc1( p1 );
   app.AsyncFunc2( p1, p2 );
   app.AsyncFunc3( p1, p2, p3 );
   app.AsyncFunc4( p1, p2, p3, p4 );
   app.AsyncFunc5( p1, p2, p3, p4, p5 );
   app.AsyncFunc6( p1, p2, p3, p4, p5, p6 );
   app.AsyncFunc7( p1, p2, p3, p4, p5, p6, p7 );
   app.AsyncFunc8( p1, p2, p3, p4, p5, p6, p7, p8 );
   app.AsyncFunc9( p1, p2, p3, p4, p5, p6, p7, p8, p9 );
   app.AsyncFunc10( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10 );
   sleep( 2 );
   printf( "Async Tests complete!\n" );
}

void runSyncTests()
{
   TestClass app;

   uint64_t p1 = 1024;
   uint32_t p2 = 512;
   uint16_t p3 = 256;
   uint8_t p4 = 16;
   int64_t p5 = -1024;
   int32_t p6 = -512;
   int16_t p7 = -256;
   int8_t p8 = -16;
   bool p9 = true;
   char p10 = 'a';

   app.SyncFunc0();
   app.SyncFunc1( p1 );
   app.SyncFunc2( p1, p2 );
   app.SyncFunc3( p1, p2, p3 );
   app.SyncFunc4( p1, p2, p3, p4 );
   app.SyncFunc5( p1, p2, p3, p4, p5 );
   app.SyncFunc6( p1, p2, p3, p4, p5, p6 );
   app.SyncFunc7( p1, p2, p3, p4, p5, p6, p7 );
   app.SyncFunc8( p1, p2, p3, p4, p5, p6, p7, p8 );
   app.SyncFunc9( p1, p2, p3, p4, p5, p6, p7, p8, p9 );
   app.SyncFunc10( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10 );
   printf( "Sync Tests complete!\n" );
}

void runSyncRetTests()
{
   TestClass app;

   TestClass::CopyCounterGuy returnCCG;

   uint64_t p1 = 1024;
   uint32_t p2 = 512;
   uint16_t p3 = 256;
   TestClass::CopyCounterGuy ccg1;
   uint8_t p4 = 16;
   int64_t p5 = -1024;
   int32_t p6 = -512;
   int16_t p7 = -256;
   int8_t p8 = -16;
   bool p9 = true;

   app.SyncRetFunc0();
   app.SyncRetFunc1( p1 );
   app.SyncRetFunc2( p1, p2 );
   app.SyncRetFunc3( p1, p2, p3 );
   app.SyncRetFunc4( p1, p2, p3, ccg1 );
   returnCCG = app.SyncRetFunc5( p1, p2, p3, ccg1, p4 );
   printf( "Total copies after SyncRetFunc5(%d)\n", returnCCG.getCount() );
   app.SyncRetFunc6( p1, p2, p3, ccg1, p4, p5 );
   app.SyncRetFunc7( p1, p2, p3, ccg1, p4, p5, p6 );
   app.SyncRetFunc8( p1, p2, p3, ccg1, p4, p5, p6, p7 );
   app.SyncRetFunc9( p1, p2, p3, ccg1, p4, p5, p6, p7, p8 );
   app.SyncRetFunc10( p1, p2, p3, ccg1, p4, p5, p6, p7, p8, p9 );

   printf( "SyncRet tests complete!\n" );
}

int main( int argc, char *argv[] )
{
   runAsyncTests();
   //runSyncTests();
   //runSyncRetTests();
}

