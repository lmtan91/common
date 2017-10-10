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
    * @param[in]
    *=========================================================================*/
   Selector();

   /**=========================================================================
    * @brief Destructor
    *
    *
    * @param[in]
    *=========================================================================*/
   virtual ~Selector();

   /**=========================================================================
    * @brief Method description
    *
    *
    * @param[in]
    * @param[out]
    *
    * @return
    * @retval
    *=========================================================================*/
private:
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
};

#endif /* ifndef INCLUDE_SELECTOR_H_ */
