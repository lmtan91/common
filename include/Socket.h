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

#ifndef INCLUDE_SOCKET_H_
#define INCLUDE_SOCKET_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <vector>

#include "Selector.h"
#include "IReaderWriter.h"

/**============================================================================
 * SocketListener gets more infomation about IO events.
 * 
 * 
 *============================================================================*/
class Socket;
class ServerSocket;
class SocketListener {
public:
    /**=========================================================================
     * @brief Notify when a socket connected with connectAsync is connected.
     *
     *
     * @param[in]   socket  Socket has connected.
     * @param[in]   success Did the connection succeed?
     *=========================================================================*/
    virtual void handleConnected( Socket *socket, bool success ) {
        (void) socket;
        (void) success;
    }

    /**=========================================================================
     * @brief Notify when socket has data available and need to be processed by
     * the user for a socket.
     *
     *
     * @param[in]   socket  Socket that data arrived on.
     *=========================================================================*/
    virtual void handleData( Socket* socket ) = 0;

    /**=========================================================================
     * @brief Notify when socket close occurs from other side.
     * This method is called when a TCP socket detects that the other side of
     * the connection has closed. The user has the choice of returning false and
     * letting the socket close itself or returning true and handling the close
     * and cleanup of the socket.
     *
     *
     * @param[in]   socket  Socket that closed.
     * @retval      true    User will cleanup socket.
     *              false   Socket will close itselt.
     *=========================================================================*/
    virtual bool handleClose( Socket *socket ) {
        (void) socket;
        return false;
    }

    /**=========================================================================
     * @brief Notify when a ServerSocket receives a new connection.
     * This method is called to notify the listener that the ServerSocket
     * specified has accepted a new connection.
     *
     * Since not all SocketListener's are listening to a ServerSocket we
     * provide a safe default that will deny the incoming connection
     *
     * @param[in]   server  ServerSocket that received new connection.
     * @param[in]   socket  Socket representing new connection.
     *
     * @retval      true    Wish to accept the connection.
     *              false   Not wish to accept it. The caller will close and
     *                      free the new socket.
     *=========================================================================*/
    virtual bool handleAccept( ServerSocket *server, Socket *socket ) {
        (void) server;
        return false;
    }

protected:
    virtual ~SocketListener() {
    }
};

class Socket: public SelectorListener, public IReaderWriter {

public:
    class Address {
    public:
        //! Default constructor (255.255.255.255, 0)
        Address() {
            mAddr.sin_family = AF_INET;
            mAddr.sin_port = 0;
            mAddr.sin_addr.s_addr = INADDR_ANY;
            mLen = sizeof(struct sockaddr_in);
        }

        //! Do a DNS lookup on this name, set port (or use 0 as a default)
        Address( const char *name, int port = 0 ) {
            mAddr.sin_family = AF_INET;
            mAddr.sin_port = htons( port );
            setAddress( name );
            mLen = sizeof(struct sockaddr_in);
        }

        //! Set in_addr
        Address( struct in_addr addr, int port = 0 ) {
            mAddr.sin_family = AF_INET;
            mAddr.sin_port = htons( port );
            mAddr.sin_addr = addr;
            mLen = sizeof(struct sockaddr_in);
        }
        //! Set the port but IP address
        explicit Address( int port ) {
            mAddr.sin_family = AF_INET;
            mAddr.sin_port = htons( port );
         mAddr.sin_addr.s_addr = INADDR_ANY;
            mLen = sizeof(struct sockaddr_in);
        }

        //! Destructor
        ~Address() {
        }

        //! Convert in_addr
        const char *getName() const {
            const char *name = inet_ntop( AF_INET, &mAddr.sin_addr, mName,
                    INET_ADDRSTRLEN );
            if ( NULL == name ) {
                mName[0] = '\0';
                name = mName;
            }
            return name;
        }

        //! Get the port we are storing.
        uint16_t getPort() const {
            return ntohs( mAddr.sin_port );
        }

        //! Get the local ip address we are storing
        struct in_addr getIpAddr() {
            return mAddr.sin_addr;
        }

        //! Set the host (doing a DNS lookup)
        void setAddress( const char *name );

        //! Set the port, perform endian conversion.
        void setPort( uint16_t port ) {
            mAddr.sin_port = htons( port );
        }

        //! Be explicit about our assignment operator
        Address& operator=( const Address& rhs ) {
            mAddr.sin_family = rhs.mAddr.sin_family;
            mAddr.sin_addr.s_addr = rhs.mAddr.sin_addr.s_addr;
            mAddr.sin_port = rhs.mAddr.sin_port;
            mLen = rhs.mLen;
            return *this;
        }

        //! Compare for equality
        bool operator==( const Address& rhs ) const {
            return mAddr.sin_addr.s_addr == rhs.mAddr.sin_addr.s_addr
                    && mAddr.sin_port == rhs.mAddr.sin_port && mLen == rhs.mLen;
        }

    private:
        //! Initialize from a sockaddr
        Address( const struct sockaddr_in *addr, int len ) {
            setSockaddr( addr, len );
        }

        //! Conpy out of a sockaddr
        void setSockaddr( const struct sockaddr_in *addr, int len ) {
            memcpy( &mAddr, addr, len );
            mLen = len;
        }

        //! Convert to const sockaddr*
        const struct sockaddr *getAddr( int &len ) const {
            len = mLen;
            return ( struct sockaddr* ) &mAddr;
        }

        //! Our host
        struct sockaddr_in mAddr;
        //! The size of mAddr
        int mLen;
        //! Used to hold the value returned by getName()
        mutable char mName[INET_ADDRSTRLEN];

        friend class Socket;
        friend class ServerSocket;
        friend class MulticastSocket;
    };

    //! A representation of an generic socket address
    class SocketAddress {
    public:
        //! default constructor (255.255.255.255, 0)
        SocketAddress() {
            mAddr.sa_family = AF_INET;
         memset( mAddr.sa_data, 0, sizeof( mAddr.sa_data ) );
        }

        //! Set from an sockaddr
        SocketAddress( struct sockaddr addr, int port = 0 ) {
            ( void ) port;
            mAddr.sa_family = addr.sa_family;
            memcpy( mAddr.sa_data, addr.sa_data, sizeof( mAddr.sa_data ) );
            mLen = sizeof(struct sockaddr);
        }

        //! Destructor
        ~SocketAddress() {
        }

        //! Be explicit about our assignment operator
        SocketAddress& operator=( const SocketAddress& rhs ) {
            mAddr.sa_family = rhs.mAddr.sa_family;
            memcpy( mAddr.sa_data, rhs.mAddr.sa_data, sizeof( mAddr.sa_data ) );
            return *this;
        }

        //! Compare equality
        bool operator==( const SocketAddress& rhs ) const {
            bool ret = memcmp( &mAddr, &rhs, sizeof(struct sockaddr) );
            return ret;
        }

        //! Copy out of sockaddr
        void setSockaddr( const struct sockaddr *addr, int len ) {
            memcpy( &mAddr, addr, len );
            mLen = len;
        }

        //! Convert to const sockaddr*
        const struct sockaddr *getAddr( int &len ) const {
            len = mLen;
            return &mAddr;
        }

        //! Set the address family
        void setFamily( int family ) {
            mAddr.sa_family = family;
        }

        //! Get the address family
        int getFamily( void ) {
            return mAddr.sa_family;
        }

        //! Get address data array
        void getData( char *dest ) {
            memcpy( dest, mAddr.sa_data, sizeof( mAddr.sa_data ) );
        }

        //! Get ptr to address data array
        char *getDataPtr( void ) {
            return mAddr.sa_data;
        }
        friend class Socket;
        friend class ServerSocket;
        friend class MulticastSocket;
        friend class Address;
    private:
        //! Our address
        struct sockaddr mAddr;
        //! The size of mAddr
        int mLen;
    };

    /**=========================================================================
     * @brief Constructor
     *
     * @param[in]   sock_stream     use TCP unless told otherwise
     *=========================================================================*/
    Socket( bool sock_stream = true );

    /**=========================================================================
     * @brief Destructor
     *
     *
     * @param[in]
     *=========================================================================*/
    virtual ~Socket();

    /**=========================================================================
     * @brief Set socket option SO_PRIORITY, used for QoS
     *
     * @param[in]   prio    Priority.
     *=========================================================================*/
    int setSockPrio( const int prio );

    /**=========================================================================
     * @brief Connect to this host/port.
     *
     * @param[in]   addr    Pair of (host, port)
     *=========================================================================*/
    int connect( const Socket::Address &addr );

    /**=========================================================================
     * @brief Connect to this host/port, unless it takes too long.
     *
     * @param[in]   addr    Pair of (host, port) to connect.
     * @param[in]   timeout Timeout to connect.
     *=========================================================================*/
    int connect( const Socket::Address &addr, int timeout );

    /**=========================================================================
     * @brief Connect will trigger a call to handleConnect when successful
     *
     * @param[in]   addr    Pair of (host, port) to connect.
     *=========================================================================*/
    int connectAsync( const Socket::Address& addr );

   /**=========================================================================
    * @brief Called to add this socket to a selector, when data arrives you
    * will be informed via listener. Call with NULL to remove a previously
    * setSelector. However normally a Selector/Listener should be used for the
    * lifetime of this object. But if your listener is destroyed before this
    * class you should remove it so we don't call a destroyed object.
    *
    *=========================================================================*/
    void setSelector( SocketListener *listener, Selector *selector );

   /**=========================================================================
    * @brief Read up to len of bytes from the socket. THis will often return
    * less than len bytes, it is important to check the return value all time.
    *
    *=========================================================================*/
    virtual int read( void *buffer, int len );

    /**=========================================================================
     * @brief Set flags for write.
     *
     *=========================================================================*/
    void setWriteFlags( int flags );

   /**=========================================================================
    * @brief Set socket non-blocking.
    *
    *=========================================================================*/
    void setNonBlocking( bool isNonBlocking );

   /**=========================================================================
    * @brief Write len of bytes to the socket. Return the number actually written.
    *
    *=========================================================================*/
    int write( const void *buffer, int len );

   /**=========================================================================
    * @brief Call recvfrom (get some data, tell me who it is from)
    *
    *=========================================================================*/
    int recvfrom( void *buf, int len, Socket::Address& addr, int flags = 0 );

   /**=========================================================================
    * @brief Call sendto (send this data to this address, doesn't work well on TCP)
    *
    *=========================================================================*/
    int sendto( const void *buf, int len, const Socket::Address& addr,
            int flags = 0 );

   /**=========================================================================
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
    *=========================================================================*/
    int recvmsg( const std::vector<iovec> &buffers,
            const Socket::Address *addr = NULL, int flags = 0 );

   /**=========================================================================
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
    *=========================================================================*/
    int sendmsg( const std::vector<iovec> &buffer, const Socket::Address *addr =
            NULL, int flags = 0 );

   /**=========================================================================
    * @brief Close the socket.
    *
    *=========================================================================*/
    int close();

    /**=========================================================================
     * @brief Shutdown the socket (see shutdown(2))
     *
     *=========================================================================*/
    int shutdown();

   /**=========================================================================
    * @brief Get bytes available from the socket.
    *
    * @return  Number of bytes available on the socket or -1 on error.
    *=========================================================================*/
    virtual int getBytesAvailable();

   /**=========================================================================
    * @brief Get the address of the remote side of this socket.
    *
    *=========================================================================*/
    int getRemoteAddress( Socket::Address &addr );

   /**=========================================================================
    * @brief Get the address of this side of this socket.
    *
    *=========================================================================*/
    int getLocalAddress( Socket::Address &addr );

   /**=========================================================================
    * @brief Get the parent of this socket. The socket will return itself if it
    * was not created by processing the accept() method of ServerSocket.
    *
    *=========================================================================*/
   Socket *getParent() {
      return mParent;
   }

   /**=========================================================================
    * @brief Set opaque private data associated with this socket.
    *
    *=========================================================================*/
   void setPrivateData( uint32_t pd ) {
      mPrivateData = pd;
   }

   /**=========================================================================
    * @brief Get back opaque private data associated with this socket.
    *
    *=========================================================================*/
   uint32_t getPrivateData() {
      return mPrivateData;
   }

   /**=========================================================================
    * @brief Get an address associated with this interface (like "eth0")
    *
    *=========================================================================*/
   static int getInterfaceAddress( const char *if_name, Socket::Address& addr );

   /**=========================================================================
    * @brief Get the MAC address of this interface.
    *
    *=========================================================================*/
   static int getHardwareAddress( const char *if_name, char *out_name );

   /**=========================================================================
    * @brief Set the amount of time we are willing to wait on a read.
    *
    *=========================================================================*/
   void setReadTimeout( int t ) {
      mReadTimeout = t;
   }

   /**=========================================================================
    * @brief Is this TCP.
    *
    *=========================================================================*/
   bool isSockStream() const {
      return mSockStream;
   }

   /**=========================================================================
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
    *=========================================================================*/
   bool setKeepAlive( int timeout );

   /**=========================================================================
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
    *=========================================================================*/
   int checkSocketHealth( uint32_t thresholdSecond = 15 );

   /**=========================================================================
    * @brief Get the hw address for a specified interface.
    *
    *=========================================================================*/
   bool getHwAddress( const char *if_name, Socket::SocketAddress &addr );

   /**=========================================================================
    * @brief Get the ARP entry for a specified MAC address.
    *
    *=========================================================================*/
   bool getArpEntry( const char *if_name, Socket::Address &sAddr,
         Socket::SocketAddress &saddr );

protected:
   /**=========================================================================
    * @brief Constructor
    *
    *=========================================================================*/
   Socket( int fd );

   /**=========================================================================
    * @brief Set parent socket (set by ServerSocket during accept.
    *
    *=========================================================================*/
   void setParent( Socket *parentSock ) {
      mParent = parentSock;
   }

   /**=========================================================================
    * @brief Set we are connected whether or not.
    *
    *=========================================================================*/
   void setConnected( bool state, int flags = POLLIN );

   /**=========================================================================
    * @brief Set the selector that we will be using.
    *
    *=========================================================================*/
   void setSelector( SelectorListener *listener, Selector *selector );

   /**=========================================================================
    * @brief IReaderWriter implementation.
    *
    *=========================================================================*/
   virtual void processFileEvents( int fd, short events,
         uint32_t private_data );

   /**=========================================================================
    * @brief Get the selector.
    *
    *=========================================================================*/
   Selector *getSelector() {
      return mSelector;
   }

   /**=========================================================================
    * @brief Get the socket listener.
    *
    *=========================================================================*/
   SocketListener *getListener() {
      return mListener;
   }

   /**=========================================================================
    * @brief Get the socket fd.
    *
    *=========================================================================*/
   int getFd() {
      return mFd;
   }

   //! set to true if we are a TCP socket connected with connectAsync.
   bool mConnectedAsync;
   //! The listener that we will inform of IO events.
   SocketListener *mListener;
private:
   //! The socket fd
   int mFd;

   /**=========================================================================
    * @brief The selector we are associated with. Most of our processing will be
    * done on that selector's thread.
    *
    *=========================================================================*/
   Selector *mSelector;

   /**=========================================================================
    * @brief If we are being used not as a Socket but as an IReaderWriter, then
    * out behaviour is a little different because of the contract on
    * IReaderWriter. This is the selector that gets used in that case.
    *
    *=========================================================================*/
   Selector *mReaderWriterSelector;

   /**=========================================================================
    * @brief If we are being used not as a Socket but as an IReaderWriter, then
    * out behaviour is a little different because of the contract on
    * IReaderWriter. This is listener that gets used in that case.
    *
    *=========================================================================*/
   SelectorListener *mReaderWriterListener;

   //! Have we called connect on this socket yet?
   bool mConnected;

   //!The opaque data associated with this socket.
   uint32_t mPrivateData;

   //! Are we a TCP socket?
   bool mSockStream;

   //! Pointer to parent socket (set by ServerSocket, default self)
   Socket *mParent;

   //! For read timeout.
   int mReadTimeout;

   //! Flags used when writing to socket.
   int mWriteFlags;

   //! The last address we read from (ONLY for UDP socket)
   Address mLastDatagramSender;

   friend class ServerSocket;
};

class ServerSocket: public Socket {
public:
   /**=========================================================================
    * @brief Default to accepting TCP connections.
    *
    *=========================================================================*/
   ServerSocket( bool sock_stream = true );

   /**=========================================================================
    * @brief Destructor.
    *
    *=========================================================================*/
   virtual ~ServerSocket();

   /**=========================================================================
    * @brief Bind to this address.
    *
    *=========================================================================*/
   int bind( const Socket::Address &addr );

   /**=========================================================================
    * @brief Listen for at most this many UNACCEPTED connections.
    *
    *=========================================================================*/
   int listen( int backlog );

   /**=========================================================================
    * @brief Block, waiting for the next incoming connection, and return that
    *
    *=========================================================================*/
   virtual Socket *accept();

protected:
   /**=========================================================================
    * @brief Handle IO from selector.
    *
    *=========================================================================*/
   void processFileEvents( int fd, short events, uint32_t private_data );
};

#endif /* ifndef INCLUDE_SOCKET_H_ */
