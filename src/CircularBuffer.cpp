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
#include <string.h>
#include <assert.h>

#include "TimeUtils.h"
#include "FdReaderWriter.h"
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
   FdReaderWriter readerWriter( fd );
   return fillFromFile( &readerWriter, size );
}

/**============================================================================
 * Read data from a IReaderWriter into the CircularBuffer.  This will not
 *  overflow the CircularBuffer.
 *============================================================================*/
int CircularBuffer::fillFromFile( IReaderWriter *reader, int size ) {
   int freespace = getFreeSpace();
   int res = 0;

   AutoLock lock( mLock );

   if ( size > freespace )
      size = freespace;

   if ( size == 0 )
      return 0;

   LOG_NOISE( "reading %d", size );

   // if buffer wraps around.
   if ( mReadPtr > mWritePtr ) {
      res = reader->read( mWritePtr, size );
      if ( res < 0 )
         return res;

      mWritePtr += res;
   }
   // else buffer doesn't wrap
   else {
      int read_size = size;

      if ( size > mEnd - mWritePtr )
         read_size = mEnd - mWritePtr;

      res = reader->read( mWritePtr, read_size );
      if ( res < 0 )
         return res;

      mWritePtr += res;

      if ( mWritePtr == mEnd )
         mWritePtr = mBuffer;
   }

   // update the space now in use in the buffer
   add_length_internal( res );

   LOG_NOISE( "read %d", res );
   mDataCondition.Broadcast();

   return res;
}

/**============================================================================
 * Drop all data from the buffer.
 *============================================================================*/
void CircularBuffer::clear() {
   AutoLock lock( mLock );

   mWritePtr = mReadPtr = mBuffer;
   mLen = 0;

   mFreeCondition.Broadcast();
}

/**============================================================================
 * Wait for a specified amount of data to arrive into the buffer.  Will
 *  timeout after a specified time.  Time may be set to -1 to wait forever.
 *============================================================================*/
int CircularBuffer::waitForData( int threshold, uint32_t msecs ) {
   TRACE_BEGIN( LOG_LVL_NOISE );

   uint32_t timeout = msecs;
   struct timespec start, cur;
   TimeUtils::getCurTime( &start );

   LOG_NOISE( "Waiting for data %d cur size %d timeout %d", threshold, length_internal(), msecs );

   AutoLock lock( mLock );

   if ( length_internal() < threshold )
      LOG_INFO( "buffer underflow" );

   while (length_internal() < threshold && mDataCondition.Wait( mLock, timeout )) {
      TimeUtils::getCurTime( &cur );
      timeout = msecs - TimeUtils::getDifference( &cur, &start );

      LOG_INFO( "Wating for data %d cur size %d msecs remaining %d", threshold, length_internal(), timeout );
   }

   if ( length_internal() >= threshold )
      return length_internal();
   else
      return 0;
}

/**============================================================================
 * Wait for a specificed amount of free space in the buffer.  Will timeout
 *  after a specified time.  Time may be set to 0 to wait forever.
 *============================================================================*/
int CircularBuffer::waitForFreeSpace( int threshold, uint32_t msecs ) {
   TRACE_BEGIN( LOG_LVL_NOISE );

   uint32_t timeout = msecs;
   struct timespec start, cur;
   TimeUtils::getCurTime( &start );

   LOG( "Wating for data %d cur size %d timeout %d", threshold, length_internal(), msecs );

   AutoLock lock( mLock );

   if ( mBufferSize - length_internal() < threshold )
      LOG_INFO( "buffer underflow" );

   while (mBufferSize - length_internal() < threshold
         && mFreeCondition.Wait( mLock, timeout )) {
      TimeUtils::getCurTime( &cur );
      //    timeout = msecs - TimeUtils::getDifference( &cur, &start );
      if ( msecs ) {
         timeout = msecs - TimeUtils::getDifference( &cur, &start );
      }

      LOG( "Wating for data %d cur size %d msecs remaining %d", threshold, length_internal(), timeout );
   }

   if ( mBufferSize - length_internal() >= threshold )
      return mBufferSize - length_internal();
   else
      return 0;
}

/**============================================================================
 * Get the byte at a specified offset into the buffer.
 *============================================================================*/
int CircularBuffer::byteAt( int i ) const {
   int size;
   const uint8_t *res = getBytes( i, size );

   if ( res == NULL )
      return -1;
   else
      return *res;
}

/**=========================================================================
 * Get a pointer to the data at an offset into the buffer.  Upon returning
 *  you will be told how many bytes following this ptr are valid.  Due to
 *  buffer wrapping this may be less that the length of the remaining data
 *  in the buffer.
 *=========================================================================*/
const uint8_t *CircularBuffer::getBytes( int i, int &size ) const {
   AutoLock lock( mLock );

   return getBytesInternal( i, size );
}

/**============================================================================
 * Write a byte to the end of the buffer.
 *============================================================================*/
int CircularBuffer::writeByte( uint8_t byte ) {
   AutoLock lock( mLock );

   if ( getFreeSpace() == 0 )
      return -1;

   *mWritePtr = byte;
   mWritePtr++;
   add_length_internal( 1 );

   if ( mWritePtr == mEnd )
      mWritePtr = mBuffer;

   mDataCondition.Broadcast();

   return 0;
}
/**============================================================================
 * Get the amount of free space in the buffer.
 *============================================================================*/
int CircularBuffer::getFreeSpace() const {
   AutoLock lock( mLock );

   return mBufferSize - length_internal();
}

/**============================================================================
 * Get the amount of data in the buffer.
 *============================================================================*/
int CircularBuffer::getLength() const {
   AutoLock lock( mLock );

   return length_internal();
}

/**============================================================================
 * Get the total size of the buffer.
 *============================================================================*/
int CircularBuffer::getSize() const {

   AutoLock lock( mLock );

   return mBufferSize;

}

//! Error-checked memcpy
void CircularBuffer::bufferCopy( uint8_t *dest, const uint8_t *src, int size ) {
   if ( src == NULL || size <= 0 || dest == NULL || src == dest )
      return;
   else
      memcpy( dest, src, size );
}

const uint8_t *CircularBuffer::getBytesInternal( int i, int &size ) const {
   uint8_t *res = NULL;

   if ( i < length_internal() ) {
      res = mReadPtr + i;
      size = mWritePtr - res;

      if ( res >= mWritePtr )
      {
         if ( res >= mEnd ) {
            res = mBuffer + ( res - mEnd );
            size = mWritePtr - res;
         } else
            size = mEnd - res;
      }
   } else
      size = 0;

   assert( size >= 0 );

   return res;
}

/**============================================================================
 * How many bytes are available?
 *
 *============================================================================*/
int CircularBuffer::length_internal() const {
   return mLen;
}

void CircularBuffer::add_length_internal( unsigned int size ) {
   if ( size > mBufferSize - mLen ) {
      LOG_ERR_FATAL( "adding size: %d to mLen: %d "
            "would exceed mBufferSize: %d", size, mLen, mBufferSize );
   }

   mLen += size;

   // If we were in the empty condition, update the read pointer to
   // its new beginning-of-buffer position.
   if ( mReadPtr == mEnd ) {
      mReadPtr = mBuffer;
   }
}

void CircularBuffer::subtract_length_internal( unsigned int size ) {
   if ( size > mLen ) {
      LOG_ERR_FATAL( "subtracting size: %d from mLen: %d would be negative",
            size, mLen );
   }

   mLen -= size;

   // If we were in the full condition, update the write pointer to
   // its new beginning-of-buffer position.
   if ( mWritePtr == mEnd ) {
      mWritePtr = mBuffer;
   }
}

/**============================================================================
 * Write some bytes into the buffer.
 *
 *============================================================================*/
int CircularBuffer::write_internal( const uint8_t *buffer, int size ) {
   TRACE_BEGIN( LOG_LVL_NOISE );

   if ( buffer == NULL )
      return 0;

   // limit size to free space, we will return this for the size of
   // this write
   if ( size > getFreeSpace() ) {
      size = getFreeSpace();
   }

   // if size is zero, return early
   if ( size == 0 ) {
      return 0;
   }

   // first, copy from the write pointer up to the end of the buffer,
   // note that this truncates implicitly to no more than free space
   int tail_size = mEnd - mWritePtr;

   // truncate tail_size to the requested write size, note that size
   // has already been truncated to no more than free space
   if ( tail_size > size ) {
      tail_size = size;
   }

   bufferCopy( mWritePtr, buffer, tail_size );
   mWritePtr += tail_size;

   // if the write pointer is now at the end of the buffer, then
   // reset it to the beginning of the buffer
   if ( mWritePtr == mEnd )
      mWritePtr = mBuffer;

   // if tail_size is less than size, then we haven't filled free
   // space up to the write request size, so continue at the begining
   // of the buffer
   if ( tail_size < size ) {
      // compute the remainder of the size bytes to write, which we
      // have already truncated to available free space
      int head_size = size - tail_size;

      // copy head_size bytes into the begining of the circular
      // buffer from offset tail_size within our input buffer
      if ( buffer != NULL ) {
         bufferCopy( mWritePtr, buffer + tail_size, head_size );
      }
      mWritePtr += head_size;
   }

   // update the space now in use in the buffer
   add_length_internal( size );

   // signal all waiters for data.
   mDataCondition.Broadcast();

   LOG( "wp %p rd %p", mWritePtr, mReadPtr );

   return size;
}
