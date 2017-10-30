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
#include <unistd.h>

#include "Socket.h"
#include "logging.h"
#include "TestCase.h"

using namespace std;

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

class UdpSocketTest: public TestCase, public SocketListener {
public:
   UdpSocketTest();
   virtual ~UdpSocketTest() {
   }
   

private:
   void Run();
   void handleData( Socket *s );
   bool handleClose( Socket *s );
   bool handleAccept( Socket *s );

   int mTestState;
};

class TcpSocketTest: public TestCase, public SocketListener {
public:
   TcpSocketTest();
   virtual ~TcpSocketTest() {
      delete mSelector;
   }

private:
   void Run();

   void handleData( Socket *s );
   bool handleClose( Socket *s );
   bool handleAccept( Socket *s );

   int mTestState;
   Selector *mSelector;
};

UdpSocketTest::UdpSocketTest() :
      TestCase( "UdpSocketTest" ) {
   SetTestName( "UDP Socket" );
}

void UdpSocketTest::Run() {
   LOG_NOTICE("UDP Test Started");

   Selector s;

   Socket sock( false );
   ServerSocket server( false );
   Socket::Address server_addr, client_addr;

   int ret = server.bind( server_addr );
   if ( ret < 0 ) {
      TestFailed( "Bind Failed" );
   }

   ret = server.getLocalAddress( server_addr );
   server_addr.setAddress( "127.0.0.1" );
   if ( ret < 0 ) {
      TestFailed( "get local addr failed" );
   }

   LOG_NOTICE("Server name %s:%d", server_addr.getName(), server_addr.getPort());

   server.setSelector( this, &s );
   ret = sock.connect( server_addr );
   if ( ret < 0 ) {
      TestFailed( "connect failed" );
   }

   ret = sock.getLocalAddress( client_addr );
   if ( ret < 0 ) {
      TestFailed( "get local addr failed" );
   }

   ret = server.connect( client_addr );
   if ( ret < 0 ) {
      TestFailed( "server connect failed" );
   }

   uint32_t number = 1000;
   uint32_t new_number;

   if ( ret < 0 ) {
      TestFailed( "connect failed" );
   }

   ret = sock.write( &number, 4 );
   if ( ret < 0 ) {
      TestFailed( "write failed" );
   }

   ret = sock.read( &new_number, 4 );
   if ( ret < 0 ) {
      TestFailed( "read failed" );
   }

   if ( new_number != number + 1 ) {
      TestFailed( "data read failed" );
   }

   number = new_number + 1;

   ret = sock.write( &number, 4 );
   if ( ret < 0 ) {
      TestFailed( "write failed" );
   }

   ret = sock.read( &new_number, 4 );
   if ( new_number != number + 1 ) {
      TestFailed( "FAILED" );
   }
   LOG_NOTICE( "Data sent" );
   mTestState = 0;

   sock.close();
   usleep( 100000 );

   TestPassed();
}

void UdpSocketTest::handleData( Socket *s ) {
   LOG_NOTICE("called");
   printf( "called\n" );
   uint32_t num;

   int ret = s->read( &num, 4 );
   if ( ret != 4 )
      TestFailed( "Read Failed %d", ret );

   num += 1;
   ret = s->write( &num, 4 );

   if ( ret != 4 )
      TestFailed( "Write Failed %d", ret );
}

bool UdpSocketTest::handleClose( Socket *s ) {
   LOG_NOTICE("called");
   TestFailed( "handle close not expected on UdpSocket" );
   return false;
}

bool UdpSocketTest::handleAccept( Socket *s ) {
   LOG_NOTICE( "called" );
   TestFailed( "handle close not expected on UdpSocket" );
   return true;
}

TcpSocketTest::TcpSocketTest() :
      TestCase( "TcpSocketTest" ) {
   SetTestName( "TCP Socket" );
}

void TcpSocketTest::Run() {
   LOG_NOTICE( "TCP Test Started" );

   mSelector = new Selector( "TcpSelector" );

   Socket sock;
   ServerSocket server;
   Socket::Address server_addr, client_addr;

   int res = server.bind( server_addr );

   if ( res < 0 )
      LOG_WARN_PERROR( "Bind Failed" );

   res = server.listen( 5 );

   if ( res < 0 )
      LOG_WARN_PERROR( "Listen Failed" );

   res = server.getLocalAddress( server_addr );
   server_addr.setAddress( "127.0.0.1" );

   if ( res < 0 )
      LOG_WARN_PERROR( "get local addr failed" );

   LOG_NOTICE( "Server name %s:%d", server_addr.getName(), server_addr.getPort() );

   server.setSelector( this, mSelector );
   res = sock.connect( server_addr );

   uint32_t number = 1000;
   uint32_t new_number;

   if ( res < 0 )
      LOG_WARN_PERROR( "connect failed" );

   res = sock.write( &number, 4 );

   if ( res < 0 )
      LOG_WARN_PERROR( "write failed" );

   res = sock.read( &new_number, 4 );
   printf( "new_number=%d\n", new_number );

   if ( new_number != number + 1 )
      LOG_WARN( "FAILED" );

   if ( res < 0 )
      LOG_WARN_PERROR( "write failed" );

   number = new_number + 1;

   res = sock.write( &number, 4 );

   if ( res < 0 )
      LOG_WARN_PERROR( "write failed" );

   res = sock.read( &new_number, 4 );

   if ( new_number != number + 1 )
      LOG_WARN( "FAILED" );

   if ( res < 0 )
      LOG_WARN_PERROR( "write failed" );

   LOG_NOTICE( "Data sent" );

   mTestState = 0;

   sock.close();
   usleep( 100000 );

   if ( mTestState != 1 ) {
      TestFailed( "handleClose not recieved on server" );
   }

   TestPassed();

}

void TcpSocketTest::handleData( Socket *s ) {
   LOG_NOTICE( "called" );
   uint32_t num;

   printf( "TcpSocketTest::handleData\n" );
   int res = s->read( &num, 4 );

   if ( res != 4 )
      TestFailed( "Read Failed %d", res );

   num += 1;
   res = s->write( &num, 4 );

   if ( res != 4 )
      TestFailed( "Write Failed %d", res );
}

bool TcpSocketTest::handleClose( Socket *s ) {
   LOG_NOTICE( "called" );
   mTestState++;
   return false;
}

bool TcpSocketTest::handleAccept( Socket *s ) {
   LOG_NOTICE( "called" );
   printf( "TcpSocketTest::handleAccept\n" );
   s->setSelector( this, mSelector );
   return true;
}

int main( int argc, char*argv[] ) {
   TestRunner runner( argv[0] );

   TestCase *test_set[10];

   test_set[0] = new UdpSocketTest();
   test_set[1] = new TcpSocketTest();

   runner.RunAll( test_set, 2 );

   return 0;
}
