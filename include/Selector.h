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

#ifndef INCLUDE_SELECTOR_H_
#define INCLUDE_SELECTOR_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/
#include <sys/poll.h>
#include <list>
#include "EventThread.h"
#include "Mutex.h"

/**============================================================================
 * The interface must be implemented by any class want to receive info about
 * file events.
 *
 *============================================================================*/
class SelectorListener {
public:
   /**=========================================================================
    * @brief The method is called to inform a listener that file event(s) have
    * occurred on a specific file descriptor.
    *
    * @param[in] fd        The file descriptor that event(s) occurred on.
    * @param[in] events    The set of events that occurred.
    * @param[in] pri_data  The private data that was given to addListener.
    *=========================================================================*/
   virtual void processFileEvents( int fd, short events, uint32_t pri_data )=0;

protected:
   virtual ~SelectorListener()
   {
   }
};
/**============================================================================
 * Class description
 *
 *
 *============================================================================*/
class Selector : public EventDispatcher {

public:

   /**=========================================================================
    * @brief Constructor
    *
    * @param[in]  name     This as the thread name.
    *=========================================================================*/
   Selector( const char *name = NULL );

   /**=========================================================================
    * @brief Destructor
    *
    *=========================================================================*/
   virtual ~Selector();

   /**=========================================================================
    * @brief Call to shutdown the thread for the selector and wait for it to end
    * If this is not called the destructor will call it, but sometimes it's not
    * convenient to wait for the destructor.
    *
    *=========================================================================*/
   void shutdown();

   /**=========================================================================
    * @brief Add a listener for a set of poll events.
    *
    * param[in] fd       The file descriptor to listener for events on.
    * param[in] events   The bit field of event to listen for. When these events
    * occur the listener will be called.
    * @param    listener The interface to call when an event occurs.
    * @param    pri_data This data will be passed to the listener when ever the
    * listener is informed of an event.
    *=========================================================================*/
   void addListener( int fd, short events, SelectorListener *listener,
            uint32_t pri_data = 0 );

   /**=========================================================================
    * @brief Remove a listener(s) previously added.  We remove any matches
    * to the fd and the listener interface.
    *
    * @param[in] fd        The file descriptor that was previously added.
    * @param     listener  The interface previously added.
    *=========================================================================*/
   void removeListener( int fd, SelectorListener *listener );

private:

   struct ListenerNode {
      enum {
         kStateAdding, kStateRemoving, kStateListening
      };

      /**======================================================================
       * @brief Default Constructor
       *
       *======================================================================*/
      ListenerNode() :
               mFd( -1 ), mEvents( -1 ), mListener( NULL ), mPrivateData( 0 ), mState(
                        kStateAdding )
      {
      }

      /**======================================================================
       * @brief Constructor
       *
       * @param[in] fd        File descriptor.
       * @param     listener  The listener.
       *======================================================================*/
      ListenerNode( int fd, SelectorListener *listener ) :
               mFd( fd ), mEvents( -1 ), mListener( listener ), mPrivateData(
                        0 ), mState( kStateAdding )
      {
      }

      /**======================================================================
       * @brief Overloading comparison operator ==.
       *
       * @param[in] other        Other listener node.
       * @return    bool
       *======================================================================*/
      bool operator==( const ListenerNode &other )
      {
         if ( mFd == other.mFd
                  && ( ( ( NULL == mListener ) || ( NULL == other.mListener ) )
                           || ( mListener == other.mListener ) ) ) {
            return true;
         } else {
            return false;
         }
      }

      int mFd;
      short mEvents;
      SelectorListener *mListener;
      uint32_t mPrivateData;
      int mState;
   };

   enum {
      PIPE_READER = 0, PIPE_WRITER
   };

   static const int kMaxPollFds = 128;

   static const int kWarnPollFds = 64;

   /**=========================================================================
    * @brief Trigger a call to fillPollFds when it is safe to do so.
    *
    *=========================================================================*/
   void updateListeners();

   /**=========================================================================
    * @brief Call anyone that is listening for events on this fd.
    *
    * @param[in] fd     The file descriptor event(s) have occurred.
    * @param[in] events The event occurred.
    *=========================================================================*/
   bool callListeners( int fd, uint32_t events );

   /**=========================================================================
    * @brief Fill up the pollfds we will be calling poll on.
    *
    * @param[in] fds    The file descriptor arrays.
    * @param[in] events The no of fds.
    *=========================================================================*/
   void fillPollFds( struct pollfd *fds, int &numFds );

   /**=========================================================================
    * @brief This is the main loop, the innermost portion of the Selector
    * event loop.  The thread is here most of the time, unless we are
    * processing an event.
    *=========================================================================*/
   void threadMain();

   /**=========================================================================
    * @brief Send a null event to the selector to wake it up.
    *=========================================================================*/
   void wakeThread();

   /**=========================================================================
    * @brief Get dispatcher thread.
    *
    * @return Thread*
    *=========================================================================*/
   const Thread *getDispatcherThread();

   /**=========================================================================
    * @brief Find ListenerNode corresponding to this fd (and optionally
    * the listener.
    *
    * @param[in]  fd       File descriptor.
    * param       listener The listener optionally.
    * @return     ListenerNode
    *=========================================================================*/
   ListenerNode *findListener( int fd, SelectorListener *listener = NULL );

   std::list<ListenerNode*> mList;

   Mutex mLock;

   int mPipe[ 2 ];

   Runnable<Selector> mThread;

   bool mShutdown;

   bool mRunning;

   bool mUpdateFds;

   Condition mCondition;
};

#endif /* ifndef INCLUDE_SELECTOR_H_ */
