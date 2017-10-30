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
#include <fcntl.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <net/if_arp.h>
#include <linux/tcp.h>
#include <netdb.h>

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

/**============================================================================
 * @brief Shutdown the socket (see shutdown(2))
 *
 *============================================================================*/
int Socket::shutdown() {
   if ( mFd != -1 ) {
      return ::shutdown( mFd, SHUT_RDWR );
   }
   return -1;
}

/**============================================================================
 * @brief Set the selector that we will be using.
 *
 *============================================================================*/
void Socket::setSelector( SelectorListener *listener, Selector *selector ) {
   if ( mFd != -1 && mConnected ) {
      if ( mReaderWriterSelector != NULL && mReaderWriterListener != NULL ) {
         mReaderWriterSelector->removeListener( mFd, mReaderWriterListener );
      }
      mReaderWriterListener = listener;
      mReaderWriterSelector = selector;
      if ( mReaderWriterSelector != NULL && mReaderWriterListener != NULL ) {
         mReaderWriterSelector->addListener( mFd, POLLIN,
               mReaderWriterListener );
      }
   } else {
      mReaderWriterListener = listener;
      mReaderWriterSelector = selector;
   }
}

/**============================================================================
 * @brief Called to add this socket to a selector, when data arrives you
 * will be informed via listener. Call with NULL to remove a previously
 * setSelector. However normally a Selector/Listener should be used for the
 * lifetime of this object. But if your listener is destroyed before this
 * class you should remove it so we don't call a destroyed object.
 *
 *============================================================================*/
void Socket::setSelector( SocketListener *listener, Selector *selector ) {
   mListener = listener;
   if ( mConnected ) {
      if ( mSelector != NULL ) {
         mSelector->removeListener( mFd, this );
      }
      mSelector = selector;
      if ( mSelector != NULL && mListener != NULL ) {
         mSelector->addListener( mFd, POLLIN, this );
      }
   } else {
      mSelector = selector;
   }
}

/**============================================================================
 * @brief Read up to len of bytes from the socket. THis will often return
 * less than len bytes, it is important to check the return value all time.
 *
 *============================================================================*/

int Socket::read( void *buffer, int len ) {
   TRACE_BEGIN(LOG_LVL_NOISE);

   if ( mReadTimeout ) {
      int retCode = 0;
      fd_set readSet;
      struct timeval timeout;
      int numBytes = 0;

      if ( mReadTimeout < 0 ) {
         return -1;
      }

      FD_ZERO( &readSet );
      FD_SET( ( unsigned )mFd, &readSet );

      timeout.tv_sec = mReadTimeout;
      timeout.tv_usec = 0;

      while (true) {
         if ( 0 == mReadTimeout ) {
            retCode = select( FD_SETSIZE, &readSet, NULL, NULL, NULL );
         } else {
            retCode = select( FD_SETSIZE, &readSet, NULL, NULL, &timeout );
         }
         if ( 0 == retCode ) {
            return -1; // timed out
         }
         if ( -1 == retCode ) {
            if ( EINTR == errno ) {
               continue;
            }
            return -1;
         } else {
            // continue to read.
            break;
         }
      }
      // read data
      numBytes = recv( mFd, buffer, len, MSG_NOSIGNAL );
      if ( numBytes < 0 ) {
         // socket error
         return -1;
      }
      return numBytes;
   } else {
      if ( !mSockStream ) {
         return recvfrom( buffer, len, mLastDatagramSender );
      } else {
         return ::read( mFd, buffer, len );
      }
   }
}

/**============================================================================
 * @brief Set flags for write.
 *
 *============================================================================*/
void Socket::setWriteFlags( int flags ) {
   mWriteFlags = flags;
}

/**============================================================================
 * @brief Set socket non-blocking.
 *
 *============================================================================*/
void Socket::setNonBlocking( bool isNonBlocking ) {
   TRACE_BEGIN(LOG_LVL_INFO);
   int saveflags;

   saveflags = fcntl( mFd, F_GETFL, 0 );
   if ( saveflags < 0 ) {
      return;
   }

   if ( isNonBlocking ) {
      // Set non-blocking property
      fcntl( mFd, F_SETFL, saveflags | O_NONBLOCK );
   } else {
      // Unset non-blocking property
      fcntl( mFd, saveflags & ~O_NONBLOCK );
   }
}

/**============================================================================
 * @brief Write len of bytes to the socket. Return the number actually written.
 *
 *============================================================================*/
int Socket::write( const void *buffer, int len ) {
   return ::send( mFd, buffer, len, mWriteFlags );
}

/**============================================================================
 * @brief Close the socket.
 *
 *============================================================================*/
int Socket::close() {
   int ret = 0;
   if ( mFd != -1 ) {
      setConnected( false );
      ret = ::close( mFd );
      LOG_NOISE("fd %d", mFd);
      mFd = -1;
   }
   return ret;
}

/**============================================================================
 * @brief Set we are connected whether or not.
 *
 *============================================================================*/
void Socket::setConnected( bool state, int flags ) {
   LOG_INFO("Connected %d sel %p", state, mSelector);

   // if we are already in this state do nothing
   if ( false == mConnected ) {
      return;
   }
   mConnected = state;
   if ( mSelector != NULL && mListener != NULL ) {
      if ( mConnected ) {
         mSelector->addListener( mFd, flags, this );
      } else {
         mSelector->removeListener( mFd, this );
      }
   }

   if ( mReaderWriterSelector != NULL && mReaderWriterListener != NULL ) {
      if ( mConnected ) {
         mReaderWriterSelector->addListener( mFd, flags,
               mReaderWriterListener );
      } else {
         mReaderWriterSelector->removeListener( mFd, mReaderWriterListener );
      }
   }
}

/**============================================================================
 * @brief IReaderWriter implementation.
 *
 *============================================================================*/
void Socket::processFileEvents( int fd, short events, uint32_t private_data ) {
   TRACE_BEGIN(LOG_LVL_NOISE); LOG("events %x", events);
   bool closeDetected = false;
   bool userHandled = false;

   // Generally, we are only going to be listening for POLLOUT in the case of
   // async connection on a TCP socket (someone called connectAsync). In that
   // case, setConnected would add POLLOUT to our event list and when it becomes
   // writable (connected) we'll trigger this.
   if ( events & POLLOUT ) {
      // IF we WERE actually connected with connectAsync, and haven't called our
      // listener for that yet.
      if ( mConnectedAsync ) {
         // Don't notify anyone with a handleConnected again.
         mConnectedAsync = false;

         // If we have a selector (which I guess we do since we are here), then
         // tweak the flags we are interested in.
         if ( mSelector != NULL ) {
            mSelector->removeListener( mFd, this );
            mSelector->addListener( mFd, POLLIN, this );
         }
         if ( NULL == mListener ) {
            LOG_WARN( "Listener doesn't exist" );
            return;;
         }

         // Figure out whether the connect succeeded or failed.
         socklen_t len;
         int ret = 0;
         len = sizeof( ret );
         int res = getsockopt( fd, SOL_SOCKET, SO_ERROR, &ret, &len );
         if ( res < 0 ) {
            // Notify the listener and be done.
            mListener->handleConnected( this, false );
            return;
         }

         // Notify the listener and be done.
         mListener->handleConnected( this, true );
         return;
      }
   }

   if ( events & POLLIN ) {
      int bytesAvail = getBytesAvailable();
      if ( 0 == bytesAvail ) {
         closeDetected = true;
      } else {
         if ( -1 == bytesAvail ) {
            LOG_WARN_PERROR( "Failed to get number of bytes available" );
         }

         if ( mListener != NULL ) {
            mListener->handleData( this );

            return;
         }
      }
   }
   if ( events & POLLHUP ) {
      closeDetected = true;
   }

   // If we detected a close on the other end of a connection based socket then
   // inform the listener and if needed close the socket.
   if ( closeDetected ) {
      if ( mListener != NULL ) {
         userHandled = mListener->handleClose( this );
      }
      if ( !userHandled ) {
         close();
      }
   }
}

/**============================================================================
 * @brief Get the address of the remote side of this socket.
 *
 *============================================================================*/
int Socket::getRemoteAddress( Socket::Address &addr ) {
   struct sockaddr_in sock_addr;
   socklen_t sock_len = sizeof( sock_addr );

   int res = getpeername( mFd, ( struct sockaddr* ) &sock_addr, &sock_len );
   if ( 0 == res ) {
      addr = Socket::Address( &sock_addr, sock_len );
   } else {
      addr = Socket::Address();
      if ( !mSockStream ) {
         addr = mLastDatagramSender;
         return 0;
      }
   }

   return res;
}

/**============================================================================
 * @brief Get the address of this side of this socket.
 *
 *============================================================================*/
int Socket::getLocalAddress( Socket::Address &addr ) {
   struct sockaddr_in sock_addr;
   socklen_t sock_len = sizeof( sock_addr );

   int res = getsockname( mFd, ( struct sockaddr* ) &sock_addr, &sock_len );
   if ( 0 == res ) {
      addr = Socket::Address( &sock_addr, sock_len );
   } else {
      addr = Socket::Address();
   }

   return res;
}

/**============================================================================
 * @brief Get the hw address for a specified interface.
 *
 *============================================================================*/
bool Socket::getHwAddress( const char *ifname, Socket::SocketAddress &addr ) {
   struct ifreq ifr;
   ifr.ifr_addr.sa_family = AF_INET;
   strncpy( irf.ifr_name, ifname, 15 );
   int res = ioctl( mFd, SIOCGIFHWADDR, &ifr );
   if ( 0 == res ) {
      unsigned char *pChar = ( unsigned char* ) &ifr.ifr_hwaddr.sa_data;
      LOG_ERR( "got %02x%02x%02x%02x%02x%02x\n", *pChar + 5, *pChar + 4,
            *pChar + 3, *pChar + 2, *pChar + 1, *pChar );
      addr.setFamily( ifr.ifr_addr.sa_family );
      memcpy( addr.getDataPtr(), ( unsigned char * ) &ifr.ifr_hwaddr.sa_data,
            sizeof( ifr.ifr_hwaddr.sa_data ) );
      return true;
   } else {
      return false;
   }
}

/**============================================================================
 * @brief Get the ARP entry for a specified MAC address.
 *
 *============================================================================*/
bool Socket::getArpEntry( const char *ifname, Socket::Address &sAddr,
      Socket::SocketAddress &saddr ) {
   struct arpreq areq;
   struct sockaddr_in *sin;

   // Retrieve MAC address of remote device from ARP table.
   memset( &areq, 0, sizeof( areq ) );

   sin = ( struct sockaddr_in* ) &areq.arp_pa;
   sin->sin_family = AF_INET;
   sin->sin_addr = sAddr.getIpAddr();

   sin = ( struct sockaddr_in* ) *areq.arp_ha;
   sin->sin_family = ARPHRD_ETHER;

   strncpy( areq.arp_dev, ifname, 15 );

   int res = ioctl( mFd, SIOCGARP, ( caddr_t ) &areq );
   if ( 0 == res && ( areq.arp_flags & ATF_COM ) ) {
      saddr.setFamily( areq.arp_ha.sa_family );
      memcpy( saddr.getDataPtr(), ( unsigned char* ) &areq.arp_ha.sa_data,
            sizeof( areq.arp_ha.sa_data ) );
      return true;
   } else {
      return false;
   }
}

/**============================================================================
 * @brief Get bytes available from the socket.
 *
 * @return  Number of bytes available on the socket or -1 on error.
 *============================================================================*/
int Socket::getBytesAvailable() {
   int bytesAvail;

   int ret = ioctl( mFd, FIONREAD, &bytesAvail );
   if ( ret != 0 ) {
      return -1;
   }
   return bytesAvail;
}

/**============================================================================
 * @brief Call recvfrom (get some data, tell me who it is from)
 *
 *============================================================================*/
int Socket::recvfrom( void *buf, int len, Socket::Address &addr, int flags ) {
   return ::recvfrom( mFd, buf, len, flags, ( sockaddr* ) &addr.mAddr,
         ( socklen_t* ) &addr.mLen );
}

/**=========================================================================
 * @brief Call sendto (send this data to this address, doesn't work well on TCP)
 *
 *=========================================================================*/
int Socket::sendto( const void*buf, int len, const Socket::Address &addr,
      int flags ) {
   return ::sendto( mFd, buf, len, flags, ( const sockaddr* ) &addr.mAddr,
         ( socklen_t ) addr.mLen );
}

/**============================================================================
 * @brief Call recvmsg (scatter-gather reads)
 *
 * @param   buffers Vector of tupples consisting of a memory address and the
 * requested amount of data to receive.
 * @param   addr    Address to receive data from. This field is ignored when
 * using a connected (TCP) socket and required when using an unconnected (UDP)
 * socket.
 * @param   flags   Optional recv() flags.
 *
 * This method allows you to make multiple recv() calls using only one
 * system call.
 *
 * @return  Number of bytes received.
 *
 *============================================================================*/
int Socket::recvmsg( const std::vector<iovec> &buffers,
      const Socket::Address *addr,
      int flags ) {
   msghdr msg;
   msg.msg_name = ( addr ) ? ( void* ) &addr->mAddr : NULL;
   msg.msg_namelen = ( addr ) ? addr->mLen : 0;
   msg.msg_iov = const_cast<iovec*>( &buffers[0] );
   msg.msg_iovlen = buffers.size();
   msg.msg_control = NULL;
   msg.msg_controllen = 0;
   msg.msg_flags = 0;

   return ::recvmsg( mFd, &msg, flags );
}

/**============================================================================
 * @brief Call sendmsg (scatter-gather writes)
 *
 * @param   buffers Vector of tupples consisting a memory address and the
 * requested amount of data to send.
 * @param   addr    Address to send data to. This field is ignored when
 * using a connected (TCP) socket, and required when using an unconnected
 * (UDP) socket.
 * @param   flags   Optional send() flags.
 *
 * This method allows to make multiple send() calls using only one system
 * call.
 *
 * @return  Number of bytes sent.
 *
 *============================================================================*/
int Socket::sendmsg( const std::vector<iovec>&buffers,
      const Socket::Address *addr, int flags ) {
   msghdr msg;
   msg.msg_name = ( addr ) ? ( void* ) &addr->mAddr : NULL;
   msg.msg_namelen = ( addr ) ? addr->mLen : 0;
   msg.msg_iov = const_cast<iovec *>( &buffers[0] );
   msg.msg_iovlen = buffers.size();
   msg.msg_control = NULL;
   msg.msg_controllen = 0;
   msg.msg_flags = 0;

   return ::sendmsg( mFd, &msg, flags );
}

/**============================================================================
 * @brief Get an address associated with this interface (like "eth0")
 *
 *============================================================================*/
int Socket::getInterfaceAddress( const char *if_name, Socket::Address &addr ) {
   struct ifreq ifr;
   ;
   int sockFd;

   sockFd = socket( PF_INET, SOCK_DGRAM, 0 );
   if ( sockFd < 0 ) {
      LOG_ERR_FATAL( "socket failed" );
      return -1;
   }

   strncpy( ifr.ifr_name, if_name, IFNAMSIZ );
   ifr.ifr_name[IFNAMSIZ - 1] = 0;

   ifr.ifr_addr.sa_family = AF_INET;

   int ret = ioctl( sockFd, SIOCGIFADDR, &ifr );
   if ( ret ) {
      LOG_WARN( "SIOCGIFADDR failed for %s", ifr.ifr_name );
      ::close( sockFd );
      return ret;
   }

   addr.setSockaddr( ( struct sockaddr_in* ) &( ifr.ifr_addr ),
         sizeof(struct sockaddr_in) );

   LOG_INFO("Lookup of %s got address %s", if_name, addr.getName());

   ::close( sockFd );

   return ret;
}

/**============================================================================
 * @brief Get the MAC address of this interface.
 *
 *============================================================================*/
int Socket::getHardwareAddress( const char *if_name, char *out_addr ) {
   if ( !if_name || !out_addr ) {
      LOG_WARN( "NULL parameters supplied" );
      return -1;
   }

   // open a socket.
   struct ifreq ifr;
   int sockFd;

   sockFd = socket( PF_INET, SOCK_DGRAM, 0 );
   if ( sockFd < 0 ) {
      LOG_ERR_FATAL( "socket failed" );
      return -1;
   }

   strncpy( ifr.ifr_name, if_name, IFNAMSIZ );
   ifr.ifr_name[IFNAMSIZ - 1] = 0;

   ifr.ifr_addr.sa_family = AF_INET;

   int ret = ioctl( sockFd, SIOCGIFHWADDR, &ifr );
   if ( ret ) {
      LOG_WARN( "failed to get hardware address" );
      ::close( sockFd );

      return ret;
   }

   // copy hardware address into a temp byffer
   unsigned char MAC[14];
   memcpy( MAC, &ifr.ifr_ifru.ifru_hwaddr.sa_data, 14 );

   sprintf( out_addr, "%02X:%02X:%02X:%02X:%02X:%02X", MAC[0], MAC[1], MAC[2],
         MAC[3], MAC[4], MAC[5] );

   ::close( sockFd );

   return ret;
}

/**============================================================================
 * @brief Set keepalive checking.
 *
 * Cause the socket to close in (roughly) this many seconds if the remote
 * host becomes unavailable.
 *
 * Set this to a small number (10 is good) to find out if the remote host is
 * gone. Set this to a larger number to keep the socket open when passing
 * through a NAT while the socket might go idle.
 *
 * @note The actual timeout will happen somewhere between min(5, timeout)
 * seconds and timeout + 5 seconds after remote host goes silent. Thus, it's
 * not necessarily possible to get a sub-5-second accuracy in your timeouts.
 *
 * @return true if OK, false if an error occured.
 *============================================================================*/
bool Socket::setKeepAlive( int timeout ) {

   int on = 1;
   if ( setsockopt( mFd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof( on ) ) != 0 ) {
      LOG_WARN_PERROR( "Failed t set SO_KEEPALIVE" );
      return false;
   }

   // how long must the connection be idle before we start the keepalive check?
   int aliveTime = 5;
   if ( setsockopt( mFd, IPPROTO_TCP, TCP_KEEPIDLE, &aliveTime,
         sizeof( aliveTime ) ) != 0 ) {
      LOG_WARN_PERROR( "Failed keepidle" );
      return false;
   }

   // At what interval (in seconds) do wee send out additional probes?
   int interval = 5;
   // set the idle time
   if ( setsockopt( mFd, IPPROTO_TCP, TCP_KEEPINTVL, &interval,
         sizeof( interval ) ) != 0 ) {
      LOG_WARN( "Failed keepintvl" );
      return false;
   }

   // The number of probes is the timeout / interval, rounded up.
   int probes = ( timeout + ( interval - 1 ) ) / interval;
   // Set the number of probes
   if ( setsockopt( mFd, IPPROTO_TCP, TCP_KEEPCNT, &probes, sizeof( probes ) )
         != 0 ) {
      LOG_WARN( "Failed keepcnt" );
      return false;
   }

   return true;
}

/**============================================================================
 * @brief Check the health of the socket based on the given liveness threshold.
 * Check to see if the socket has shown healthy signs of activity within the
 * preceding liveness threshold (in seconds).
 *
 * @retval  0 if the socket is healthy or if any signs of ill health are
 * still within the threshold.
 * @retval  The positive number of seconds since the last positive sign if
 *          the socket shows signs of ill health, and has not shown positive
 *          signs within the threshold time.
 * @retval  -1 if the attempt to get socket info fails for any reason.
 *
 * @note Only use for SOCK_STREAM sockets.
 *
 *============================================================================*/
int Socket::checkSocketHealth( uint32_t thresholdSeconds ) {
   uint8_t thresholdMagnitude = 0;
   for (uint32_t i = thresholdSeconds; i > 1; i /= 2, ++thresholdMagnitude)
      ;

   struct tcp_info info;
   socklen_t info_len = sizeof( info );

   if ( getsockopt( mFd, IPPROTO_TCP, TCP_INFO, &info, &info_len ) ) {
      LOG_WARN_PERROR( "Failed to getsockopt TCP_INFO" );
      return -1;
   }

   // For the "sending" state: see if the number of outstanding
   // retransmits currently exceeds one more than the magnitude of
   // the threshold and if the last ack we have received from the
   // client is older than the threshold
   if ( info.tcpi_retransmits > thresholdMagnitude + 1
         && info.tcpi_last_ack_recv > thresholdSeconds * 1000 ) {
      LOG_ERR( "Failed for retransmits %d", info.tcpi_retransmits );
      LOG_WARN( "last_ack_recv %d, backoff %d", info.tcpi_last_ack_recv,
            info.tcpi_backoff );
      return info.tcpi_last_ack_recv / 1000;
   }
   // For the "zero-window probe" state: See if we have missed any
   // probes, and if the total of missed probes and the backoff
   // exceeds twice the magnitude of the of the threshold, and if the
   // last ack we have received from the client is older than the
   // threshold
   if ( info.tcpi_probes > 0
         && info.tcpi_probes + info.tcpi_backoff > thresholdMagnitude * 2
         && info.tcpi_last_ack_recv > thresholdSeconds * 1000 ) {
      LOG_ERR( "failed for probes %d", info.tcpi_probes );
      LOG_WARN( "last_ack_recv %d, backoff %d", info.tcpi_last_ack_recv,
            info.tcpi_backoff );
      return info.tcpi_last_ack_recv / 1000;
   }
   // Socket health looks good *or* anything bad is still under threshold
   return 0;
}

/**============================================================================
 * @brief Set the host (doing a DNS lookup).
 *
 *============================================================================*/
void Socket::Address::setAddress( const char *name ) {
   mAddr.sin_addr.s_addr = INADDR_ANY;

   if ( name != NULL ) {
      // Return non-zero if name is a valid dotted-quad
      if ( inet_aton( name, &mAddr.sin_addr ) ) {
         return;
      }

      Mutex::EnterCriticalSection();
      struct hostent *host = gethostbyname( name );
      if ( host != NULL && AF_INET == host->h_addrtype ) {
         mAddr.sin_addr.s_addr = *( ( uint32_t* ) host->h_addr_list[0] );
      }
      Mutex::ExitCriticalSection();
   }
}

/**============================================================================
 * @brief Default to accepting TCP connections.
 *
 *============================================================================*/
ServerSocket::ServerSocket( bool sock_stream ) :
      Socket( sock_stream ) {
   TRACE_BEGIN(LOG_LVL_INFO);

   struct linger l = { true, 5 };

   int res = setsockopt( getFd(), SOL_SOCKET, SO_LINGER, &l, sizeof( l ) );
   if ( res != 0 ) {
      LOG_WARN( "Failed to set linger" );
   }
}

/**============================================================================
 * @brief Destructor.
 *
 *============================================================================*/
ServerSocket::~ServerSocket() {
   TRACE_BEGIN(LOG_LVL_INFO);
}

/**============================================================================
 * @brief Bind to this address.
 *
 *============================================================================*/
int ServerSocket::bind( const Socket::Address &addr ) {
   TRACE_BEGIN(LOG_LVL_INFO);
   int len;
   const struct sockaddr *saddr = addr.getAddr( len );
   int ret = ::bind( getFd(), saddr, len );

   // For UDP sockets set connected on bind, since listen will not be called.
   if ( 0 == ret ) {
      if ( false == isSockStream() ) {
         setConnected( true );
      }

   } else {
      LOG_WARN_PERROR( "Bind failed on %s:%d", addr.getName(), addr.getPort() );
   }

   LOG_INFO("bind to %s on port %d", addr.getName(), addr.getPort());

   return ret;
}

/**============================================================================
 * @brief Listen for at most this many UNACCEPTED connections.
 *
 *============================================================================*/
int ServerSocket::listen( int num ) {
   TRACE_BEGIN(LOG_LVL_INFO);

   int ret = ::listen( getFd(), num );
   if ( 0 == ret ) {
      setConnected( true );
   } else {
      LOG_WARN_PERROR( "Listen failed" );
   }

   return ret;
}

/**============================================================================
 * @brief Block, waiting for the next incoming connection, and return that
 *
 *============================================================================*/
Socket *ServerSocket::accept() {
   TRACE_BEGIN(LOG_LVL_INFO);
   Socket *new_sock = NULL;

   int ret = ::accept( getFd(), NULL, NULL );
   if ( ret != -1 ) {
      new_sock = new Socket( ret );
      new_sock->setConnected( true );
      new_sock->setParent( this );
   } else {
      LOG_WARN_PERROR( "Accept failed" );
   }
   return new_sock;
}

/**============================================================================
 * @brief Handle IO from selector.
 *
 *============================================================================*/
void ServerSocket::processFileEvents( int fd, short events,
      uint32_t private_data ) {
   TRACE_BEGIN(LOG_LVL_INFO);
   if ( events & POLLIN ) {
      if ( isSockStream() ) {
         bool accepted = false;
         // Call accept to initialize the new socket
         Socket *newSock = accept();
         if ( newSock != NULL ) {
            accepted = mListener->handleAccept( this, newSock );

            // If the listener doesn't accept the new connection then close and
            // destroy it.
            if ( !accepted ) {
               newSock->close();
               delete newSock;
            }
         }
      } else {
         if ( mListener != NULL ) {
            mListener->handleData( this );
         }
      }
   }
}
