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
#include <unistd.h>
#include <assert.h>

#include "CircularBuffer.h"
#include "logging.h"

SET_LOG_CAT( LOG_CAT_DEFAULT );
SET_LOG_LEVEL( LOG_LVL_NOTICE );


#define BUFFER_SIZE     16
#define RW_SIZE         8

int main( int argc, char *argv[] ) {
   CircularBuffer *buf = new CircularBuffer( BUFFER_SIZE );
   uint8_t buffer[ RW_SIZE];

   for (int i = 0; i < RW_SIZE; i++) {
      buffer[i] = i;
   }

   buf->write( buffer, RW_SIZE );
   for (int i = 0; i < 40; i++) {
      buf->write( buffer, RW_SIZE );
      buf->copy( buffer, RW_SIZE );
      for (int j = 0; j < RW_SIZE; j++) {
         assert( buffer[j] == j );
      }
      buf->read( buffer, RW_SIZE );
      for (int j = 0; j < RW_SIZE; j++) {
         assert( buffer[j] == j );
      }
   }

#if 1
   buf->copy( buffer, RW_SIZE );
   for (int j = 0; j < RW_SIZE; j++) {
      assert( buffer[j] == j );
   }
   buf->read( buffer, RW_SIZE );
   for (int j = 0; j < RW_SIZE; j++) {
      assert( buffer[j] == j );
   }
#endif

   return 0;
}



