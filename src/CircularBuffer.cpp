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

#include "logging.h"
#include "CircularBuffer.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOTICE );

/**============================================================================
 * @brief Constructor
 *
 * @param[in]  buffer   A given buffer.
 * @param[in]  buf_size A buffer size.
 *============================================================================*/
CircularBuffer::CircularBuffer( uint8_t *buffer, int buf_size ) :
      mLock( true ), mFreeBuffer( false ) {
   mBuffer = buffer;
   mReadPtr = mBuffer;
   mWritePtr = mBuffer;
   mBufferSize = buf_size;
   mEnd = mBuffer + buf_size;
   mLen = 0;
}

/**=========================================================================
 * @brief Constructor
 *
 * @param[in]  buf_size A buffer size.
 *=========================================================================*/
CircularBuffer::CircularBuffer( int buf_size )
:
      mLock( true ), mFreeBuffer( true )
{
   mBuffer = new uint8_t[buf_size];
   mReadPtr = mBuffer;
   mWritePtr = mBuffer;
   mBufferSize = buf_size;
   mEnd = mBuffer + buf_size;
   mLen = 0;
}

/**============================================================================
 * @brief Destructor
 *
 *
 *============================================================================*/
CircularBuffer::~CircularBuffer() {
   if ( mFreeBuffer ) {
      delete[] mBuffer;
   }
}

/**============================================================================
 * write data to the CircularBuffer, we write more data than there is free
 * space then we will discard the oldest data in the buffer and write all
 * bytes from the input buffer.  If the input buffer is bigger than the
 * CircularBuffer the results are not garenteed.
 *============================================================================*/
int CircularBuffer::write_overflow( uint8_t *buffer, int size ) {
   TRACE_BEGIN(LOG_LVL_NOISE);

   int offset = 0;

   // If the size specified is larger than the total buffer size
   // then adjust the size we are going to write and where we're going
   // to write from
   if ( size > mBufferSize ) {
      offset = size - mBufferSize;
      size = mBufferSize;
   }

   AutoLock lock( mLock );

   // If the number of bytes we are trying to write is greater than
   // the amount of free space then clear the difference before writing
   // this is the "overflow" mechanism.
   int read_size = size - getFreeSpace();
   if ( read_size > 0 ) {
      int __attribute__((unused)) res = read( NULL, read_size );
      LOG( "overflow reading %d bytes (read %d)", read_size, res );
   }

   return write_internal( buffer + offset, size ) + offset;
}

/**============================================================================
 * write data to the CircularBuffer, we write more data than there is free
 * space then the write will not transfer all bytes from the input buffer.
 *============================================================================*/
int CircularBuffer::write( const uint8_t *buffer, int size ) {
   TRACE_BEGIN( LOG_LVL_NOISE );
   AutoLock lock( mLock );
   return write_internal( buffer, size );
}

/**============================================================================
 * Read bytes from the CircularBuffer and remove them from the
 * CircularBuffer.  If buffer is NULL, consume the data without
 * copying it anywhere.
 *============================================================================*/
int CircularBuffer::read( uint8_t *buffer, int size ) {
   TRACE_BEGIN( LOG_LVL_NOISE );

   AutoLock lock( mLock );

   // limit size to data available, we will return this for the size of
   // this read
   if ( size > length_internal() )
      size = length_internal();

   // if size is zero, return early
   if ( size == 0 )
      return 0;

   // first, prepare to copy data up to the end of the buffer
   int tail_size = mEnd - mReadPtr;

   // truncate tail_size to the requested read size, note that size
   // has already been truncated to no more than data available
   if ( tail_size > size )
      tail_size = size;

   bufferCopy( buffer, mReadPtr, tail_size );
   mReadPtr += tail_size;

   // if tail_size is less than size, then continue at the begining
   // of the buffer
   if ( tail_size < size ) {
      // compute the remainder of the size bytes to read, which we
      // have already truncated to available data
      int head_size = size - tail_size;

      // copy head_size bytes to offset tail_size within our output
      // buffer from the begining of the circular buffer
      if ( buffer != NULL ) {
         bufferCopy( buffer + tail_size, mBuffer, head_size );
      }
      mReadPtr = mBuffer + head_size;
   }

   // update the space now in use in the buffer
   subtract_length_internal( size );

   // signal any waiters for free space.
   mFreeCondition.Broadcast();

   LOG( "wp %p rd %p", mWritePtr, mReadPtr );
   return size;
}

/**============================================================================
 * Copy bytes from the CircularBuffer leaving them in the buffer.
 *============================================================================*/
int CircularBuffer::copy( uint8_t *buffer, int size ) const {
   TRACE_BEGIN( LOG_LVL_NOISE );

   AutoLock lock( mLock );

   // limit size to data available, we will return this for the size
   // of this copy
   if ( size > length_internal() )
      size = length_internal();

   int start_size = size;
   const uint8_t *src = getBytesInternal( 0, start_size );

   // truncate start_size to the requested read size, note that size
   // has already been truncated to no more than data available
   if ( start_size > size )
      start_size = size;

   memcpy( buffer, src, start_size );

   if ( start_size < size ) {
      int end_size = size - start_size;
      int temp_size;
      src = getBytesInternal( start_size, temp_size );
      memcpy( buffer + start_size, src, end_size );
   }

   return size;
}

/**============================================================================
 * Read data from a file into the CircularBuffer.  This will not overflow
 *  the CircularBuffer.
 *============================================================================*/
int CircularBuffer::fillFromFile( int fd, int size ) {

}

/**============================================================================
 * Get the amount of free space in the buffer.
 *============================================================================*/
int CircularBuffer::getFreeSpace() const {
   AutoLock lock( mLock );

   return mBufferSize - length_internal();
}

/**============================================================================
 * How many bytes are available?
 *
 *============================================================================*/
int CircularBuffer::length_internal() const {
   return mLen;
}

/**============================================================================
 * Write some bytes into the buffer.
 *
 *============================================================================*/
int CircularBuffer::write_internal( const uint8_t *buffer, int size ) {

}
