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

#ifndef INCLUDE_FDREADERWRITER_H_
#define INCLUDE_FDREADERWRITER_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/
#include "IReaderWriter.h"

/**============================================================================
 * Class description
 * 
 * 
 *============================================================================*/
class FdReaderWriter: public IReaderWriter {

public:

   /**=========================================================================
    * @brief Constructor
    *
    * @param[in]  fd
    *=========================================================================*/
   FdReaderWriter( int fd );

   /**=========================================================================
    * @brief Destructor
    *
    *
    *=========================================================================*/
   virtual ~FdReaderWriter();

   /**=========================================================================
    * Set a selector and start listening for file events on this fd.
    *  This will listen for POLLIN, if you want other event see overloaded
    *  version below.
    *=========================================================================*/
   void setSelector( SelectorListener *listener, Selector *selector );

   /**=========================================================================
    * Set a selector and start listening for file events on this fd.
    *  This will listen for whatever you specify in events.  This should
    *  be valid value for the poll system call.  This include POLLIN,
    *  POLLOUT, etc...
    *=========================================================================*/
   void setSelector( SelectorListener *listener, Selector *selector,
         short events );

   /**=========================================================================
    * Read bytes from the fd
    *=========================================================================*/
   int read( void *buffer, int length );

   /**=========================================================================
    * Write bytes to the fd
    *=========================================================================*/
   int write( const void *buffer, int length );

   /**=========================================================================
    * Close the fd, note that when this class is destroyed it will NOT
    *  close the fd, you must do that yourself.
    *=========================================================================*/
   int close();

private:
   int mFd;
   Selector *mSelector;

};

#endif /* ifndef INCLUDE_FDREADERWRITER_H_ */
