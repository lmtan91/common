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

#ifndef INCLUDE_CIRCULARBUFFER_H_
#define INCLUDE_CIRCULARBUFFER_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/
#include "Mutex.h"
#include "Condition.h"
/**============================================================================
 * Class description
 * 
 * 
 *============================================================================*/
class CircularBuffer {

public:

   /**=========================================================================
    * @brief Constructor
    *
    * @param[in]  buffer   A given buffer.
    * @param[in]  buf_size A buffer size.
    *=========================================================================*/
   CircularBuffer( uint8_t *buffer, int buf_size );

   /**=========================================================================
    * @brief Constructor
    *
    * @param[in]  buf_size A buffer size.
    *=========================================================================*/
   CircularBuffer( int buf_size );

   /**=========================================================================
    * @brief Destructor
    *
    *
    *=========================================================================*/
   virtual ~CircularBuffer();

   /**=========================================================================
    * write data to the CircularBuffer, we write more data than there is free
    * space then we will discard the oldest data in the buffer and write all
    * bytes from the input buffer.  If the input buffer is bigger than the
    * CircularBuffer the results are not garenteed.
    *=========================================================================*/
   int write_overflow( uint8_t *buffer, int size );

   /**=========================================================================
    * write data to the CircularBuffer, we write more data than there is free
    *  space then the write will not transfer all bytes from the input buffer.
    *=========================================================================*/
   int write( const uint8_t *buffer, int size );

   /**=========================================================================
    * Read bytes from the CircularBuffer and remove them from the
    * CircularBuffer.  If buffer is NULL, consume the data without
    * copying it anywhere.
    *=========================================================================*/
   int read( uint8_t *buffer, int size );

   /**=========================================================================
    * Copy bytes from the CircularBuffer leaving them in the buffer.
    *=========================================================================*/
   int copy( uint8_t *buffer, int size ) const;

   /**=========================================================================
    * Read data from a file into the CircularBuffer.  This will not overflow
    *  the CircularBuffer.
    *=========================================================================*/
   int fillFromFile( int fd, int size );

   /**=========================================================================
    * Get the amount of free space in the buffer.
    *=========================================================================*/
   int getFreeSpace() const;

protected:
   //! How many bytes are available?
   int length_internal() const;

   //! Write some bytes into the buffer
   int write_internal( const uint8_t *buffer, int size );

private:
   //! Pointer to the buffer
   uint8_t *mBuffer;

   //! Pointer to the oldest byte in the buffer
   uint8_t *mReadPtr;

   //! Pointer to the first unused byte in the buffer
   uint8_t *mWritePtr;

   //! Pointer to the end of the data in the buffer
   uint8_t *mEnd;

   //! The number of bytes used in the buffer
   uint32_t mLen;

   //! The size of mBuffer
   int mBufferSize;

   //! The lock we use for protecting access and making this thread safe
   mutable Mutex mLock;

   //! Readers may block on this if data is not available
   Condition mDataCondition;

   //! Writers may block on this if space is not available
   Condition mFreeCondition;

   //! Do we need to call delete[] on this buffer in the destructor?
   bool mFreeBuffer;
};

#endif /* ifndef INCLUDE_CIRCULARBUFFER_H_ */
