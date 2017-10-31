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

#include "logging.h"
#include "FdReaderWriter.h"

SET_LOG_CAT (LOG_CAT_ALL);
SET_LOG_LEVEL (LOG_LVL_NOTICE);

/**============================================================================
 * @brief Constructor
 *
 * @param[in]  fd
 *============================================================================*/
FdReaderWriter::FdReaderWriter( int fd ) :
      mFd( fd ), mSelector( NULL ) {
}  

/**============================================================================
 * @brief Destructor
 *
 *
 *============================================================================*/
FdReaderWriter::~FdReaderWriter() {
}

/**============================================================================
 * Set a selector and start listening for file events on this fd.
 *  This will listen for POLLIN, if you want other event see overloaded
 *  version below.
 *============================================================================*/
void FdReaderWriter::setSelector( SelectorListener *listener,
      Selector *selector ) {
   setSelector( listener, selector, POLLIN );
}

/**============================================================================
 * Set a selector and start listening for file events on this fd.
 *  This will listen for whatever you specify in events.  This should
 *  be valid value for the poll system call.  This include POLLIN,
 *  POLLOUT, etc...
 *============================================================================*/
void FdReaderWriter::setSelector( SelectorListener *listener,
      Selector *selector, short subEvents ) {
   TRACE_BEGIN(LOG_LVL_NOTICE);
   if ( mFd != -1 ) {
      if ( mSelector != NULL )
         mSelector->removeListener( mFd, listener );
      mSelector = selector;
      if ( mSelector != NULL )
         mSelector->addListener( mFd, subEvents, listener );
   }
}

/**============================================================================
 * Read bytes from the fd
 *============================================================================*/
int FdReaderWriter::read( void *buffer, int length ) {
   TRACE_BEGIN(LOG_LVL_NOISE);

   return ::read( mFd, buffer, length );
}

/**============================================================================
 * Write bytes to the fd
 *============================================================================*/
int FdReaderWriter::write( const void *buffer, int length ) {
   TRACE_BEGIN(LOG_LVL_NOISE);

   return ::write( mFd, buffer, length );
}

/**============================================================================
 * Close the fd, note that when this class is destroyed it will NOT
 *  close the fd, you must do that yourself.
 *============================================================================*/
int FdReaderWriter::close() {
   TRACE_BEGIN(LOG_LVL_NOISE);

   mSelector->removeListener( mFd, NULL );
   return ::close( mFd );
}
