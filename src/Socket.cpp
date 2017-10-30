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

#include <fcntl.h>

#include "Socket.h"
#include "logging.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOTICE );

/**============================================================================
 * @brief Constructor
 *
 * @param[in]   sock_stream     use TCP unless told otherwise
 *============================================================================*/
Socket::Socket( bool sock_stream ) :
      mConnectedAsync( false ), mListener( NULL ), mFd( -1 ), mSelector( NULL ), mReaderWriterSelector(
            NULL ), mReaderWriterListener( NULL ), mConnected( false ), mPrivateData(
            0 ), mSockStream( sock_stream ), mParent( this ), mReadTimeout( 0 ), mWriteFlags(
            MSG_NOSIGNAL )

{
   TRACE_BEGIN(LOG_LVL_INFO);

   if ( sock_stream ) {
      mFd = socket( AF_INET, SOCK_STREAM, 0 );
   } else {
      mFd = socket( AF_INET, SOCK_DGRAM, 0 );
   }

   int on = 1;
   setsockopt( mFd, SOL_SOCKET, SO_REUSEADDR, ( char* ) &on, sizeof( on ) );
   LOG_NOISE("fd %d", mFd);

   if ( -1 == mFd ) {
      LOG_ERR_FATAL( "Failed to create socket" );
   }
}

/**============================================================================
 * @brief Constructor
 *
 * @param fd   Input fd.
 *============================================================================*/
Socket::Socket( int fd ) :
      mConnectedAsync( false ), mListener( NULL ), mFd( fd ), mSelector( NULL ), mReaderWriterSelector(
            NULL ), mReaderWriterListener( NULL ), mConnected( false ), mPrivateData(
            0 ), mSockStream( true ), mParent( this ), mReadTimeout( 0 ), mWriteFlags(
            MSG_NOSIGNAL )
{
   TRACE_BEGIN(LOG_LVL_INFO);
}

/**============================================================================
 * @brief Destructor
 *
 *============================================================================*/
Socket::~Socket() {
   TRACE_BEGIN(LOG_LVL_INFO);
   shutdown();
   close();
}

/**=========================================================================
 * @brief Set socket option SO_PRIORITY, used for QoS
 *
 * @param[in]   prio    Priority.
 *=========================================================================*/
int Socket::setSockPrio( const int prio ) {
   return setsockopt( mFd, SOL_SOCKET, SO_PRIORITY, &prio, sizeof( prio ) );
}

/**============================================================================
 * @brief Connect to this host/port, unless it takes too long.
 *
 * @param[in]   addr    Pair of (host, port) to connect.
 * @param[in]   timeout Timeout to connect.
 *
 * @retval     -1 On error
 *============================================================================*/
int Socket::connect( const Socket::Address &addr, int timeout ) {
   TRACE_BEGIN(LOG_LVL_INFO);
   socklen_t len = 0;
   int len_addr = 0;
   int res = 0;
   int saveflags, ret, back_err;
   const struct sockaddr *saddr = addr.getAddr( len_addr );
   fd_set fd_w;
   struct timeval t;

   t.tv_sec = timeout;
   t.tv_usec = 0;

   saveflags = fcntl( mFd, F_GETFL, 0 );
   if ( saveflags < 0 ) {
      return -1;
   }

   // Set non blocking
   if ( fcntl( mFd, F_SETFL, saveflags | O_NONBLOCK ) < 0 ) {
      return -1;
   }

   // This will return immediately
   res = ::connect( mFd, saddr, len_addr );
   back_err = errno;

   // Restore flags
   if ( fcntl( mFd, F_SETFL, saveflags ) < 0 ) {
      return -1;
   }

   if ( res < 0 && back_err != EINPROGRESS ) {
      return -1;
   }

   FD_ZERO( &fd_w );
   FD_SET( mFd, &fd_w );

   res = select( FD_SETSIZE, NULL, &fd_w, NULL, &t );
   if ( res < 0 ) {
      return -1;
   }

   // 0 means it's timeout and no fds changed
   if ( 0 == res ) {
      return -1;
   }

   // Get the return code from the connect
   len = sizeof( ret );
   res = getsockopt( mFd, SOL_SOCKET, SO_ERROR, &ret, &len );
   if ( res < 0 ) {
      return -1;
   }

   // ret = 0 means success, otherwise it contains the errno
   if ( ret != 0 ) {
      return -1;
   }

   LOG_NOTICE("connect to %s on port %d", addr.getName(), addr.getPort());

   if ( 0 == res ) {
      setConnected( true );
   }

   return res;
}

/**=========================================================================
 * @brief Connect will trigger a call to handleConnect when successful
 *
 * @param[in]   addr    Pair of (host, port) to connect.
 *=========================================================================*/
int Socket::connectAsync( const Socket::Address &addr ) {
   TRACE_BEGIN(LOG_LVL_INFO);
   int len_addr = 0;
   int res = 0;
   int saveflags, back_err;
   const struct sockaddr *saddr = addr.getAddr( len_addr );

   saveflags = fcntl( mFd, F_GETFL, 0 );
   if ( saveflags < 0 ) {
      return -1;
   }

   res = ::connect( mFd, saddr, len_addr );
   back_err = errno;
   if ( res < 0 && back_err != EINPROGRESS ) {
      return -1;
   }

   mConnectedAsync = true;

   // We need to hear POLLOUT once in order to inform us that the socket has
   // connected. Once we get that once, we will want to unregister for POLLOUT,
   // otherwise we'll get it all the time.
   setConnected( true, POLLIN | POLLOUT );

   // Turn off non-blocking
   if ( fcntl( mFd, F_SETFL, saveflags ) < 0 ) {
      return -1;
   }

   return 0;
}
