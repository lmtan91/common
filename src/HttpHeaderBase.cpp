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
#include "HttpHeaderBase.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOTICE );

/**============================================================================
 * @brief Constructor
 *
 *============================================================================*/
HttpHeaderBase::HttpHeaderBase() :
      mLen( 0 ), mNumFields( 0 ) {
   mBufferSize = kBufferInitSize;
   mBuffer = new char[mBufferSize];
}

HttpHeaderBase::~HttpHeaderBase() {
   delete[] mBuffer;
   for (std::list<FieldMap*>::iterator i = mFieldMappings.begin();
         i != mFieldMappings.end(); ++i) {
      delete *i;
   }

   mFieldMappings.clear();
}

const char *HttpHeaderBase::getHeader() {
   if ( mLen == 0 ) {
      buildHeader();
   }
   return ( const char* ) mBuffer;
}

void HttpHeaderBase::printHeader() {
   if ( mLen == 0 ) {
      buildHeader();
   }

   LOG_NOTICE("%s", (char*)mBuffer);
   printf( "%s\n", ( char* ) mBuffer );
}

int HttpHeaderBase::send( IReaderWriter *writer ) const {
   TRACE_BEGIN(LOG_LVL_INFO);

   if ( mLen == 0 ) {
      buildHeader();
   }

   LOG("Writing header of length %d", mLen);

   int res = writer->write( mBuffer, mLen );

   return res;
}

int HttpHeaderBase::send( IReaderWriter *writer, const uint8_t* data,
      int len ) const {
   TRACE_BEGIN(LOG_LVL_INFO);

   if ( mLen == 0 ) {
      buildHeader();
   }

   if ( not data ) {
      return send( writer );
   }

   uint8_t* temp = new uint8_t[mLen + len];
   memcpy( temp, mBuffer, mLen );
   memcpy( temp + mLen, data, len );

   int res = writer->write( temp, mLen + len );
   delete[] temp;
   return res;
}

int HttpHeaderBase::sendto( Socket *writer,
      const Socket::Address& addr ) const {
   TRACE_BEGIN(LOG_LVL_INFO);

   if ( mLen == 0 ) {
      buildHeader();
   }

   LOG("Writing header of length %d", mLen);

   int res = writer->sendto( mBuffer, mLen, addr );

   return res;
}

int HttpHeaderBase::addFieldMap( FieldMap *fieldMap ) {
   TRACE_BEGIN(LOG_LVL_INFO);

   if ( fieldMap == NULL )
      return -1;

   mFieldMappings.push_back( fieldMap );
   return 0;
}

FieldMap::FieldType HttpHeaderBase::getFieldType( const char *field ) const {
   TRACE_BEGIN(LOG_LVL_INFO);

   FieldMap::FieldType type = FieldMap::kInvalidFieldType;

   for (std::list<FieldMap*>::const_iterator i = mFieldMappings.begin();
         i != mFieldMappings.end(); ++i) {
      FieldMap *map = *i;
      type = map->getFieldType( field );
      if ( type != FieldMap::kInvalidFieldType )
         break;
   }

   return type;
}

const char *HttpHeaderBase::getFieldTypeString(
      FieldMap::FieldType type ) const {
   TRACE_BEGIN(LOG_LVL_INFO);

   const char *fieldName = NULL;

   for (std::list<FieldMap*>::const_iterator i = mFieldMappings.begin();
         i != mFieldMappings.end(); ++i) {
      FieldMap *map = *i;
      fieldName = map->getFieldTypeString( type );
      if ( fieldName != NULL )
         break;
   }
   return fieldName;
}

int HttpHeaderBase::parseLine( const CircularBuffer &buffer ) {
   TRACE_BEGIN(LOG_LVL_NOISE);

   enum {
      ParseField, ParseData, ParseCRLF, SkipWS
   } state = ParseField;

   std::string field;
   std::string data;

   bool appendSpace = false;
   int length = 1;
   int c;

   for (int i = 0; i < buffer.getLength(); ++i, ++length) {
      c = buffer.byteAt( i );

      // This shouldn't happen, but make sure...
      if ( c == -1 ) {
         LOG_ERR( "Index %d, Buffer length %d, got -1 from byteAt", i,
               buffer.getLength() );
         return -1;
      }

      // Add characters to the field until we find a ":" at
      // which point we will transition to SkipWS state.
      if ( state == ParseField ) {
         // Check for ":"
         if ( c == ':' ) {
            // Update state to SkipWS
            state = SkipWS;
            appendSpace = false;
         } else {
            // Append character to the field
            field += c;
         }
      }
      // Add characters to the data until we find a CRLF
      else if ( state == ParseData ) {
         // Check for CRLF
         if ( c == '\r' and buffer.byteAt( i + 1 ) == '\n' ) {
            state = ParseCRLF;

            // In the case that this might be a continuation we
            // want a single space appended before the next valid
            // character
            appendSpace = true;
         } else {
            data += c;
         }
      }
      // Someone detected CRLF and we have to parse it appropriately
      else if ( state == ParseCRLF ) {
         // First character when we have CRLF should be the
         // LF.   Determine whether this is the end of the field
         // data or a continuation
         if ( c == '\n' ) {
            // Ah... a continuation... transition to SkipWS
            if ( buffer.byteAt( i + 1 ) == ' '
                  or buffer.byteAt( i + 1 ) == '\t' ) {
               state = SkipWS;
            }
            // Otherwise... this is the end of the field
            else {
               FieldMap::FieldType type = getFieldType( field.c_str() );

               if ( type != FieldMap::kInvalidFieldType ) {
                  mFields[mNumFields].setType( type );
                  mFields[mNumFields].setData( data.c_str() );
                  mNumFields++;
                  LOG("Field %s, data %s", field.c_str(), data.c_str());
               } else {
                  LOG("Unknown field %s, data %s", field.c_str(), data.c_str());
               }
               return length;
            }
         }
      }
      // Skip over white space ' ' and '\t', if we see a
      // CRLF then transition to HaveCRLF
      else if ( state == SkipWS ) {
         // Check for CRLF
         if ( c == '\r' and buffer.byteAt( i + 1 ) == '\n' ) {
            state = ParseCRLF;
         }
         // Otherwise if this is not just normal whitespace (' ' or '\t')
         // then append it to the data
         else if ( c != '\t' and c != ' ' ) {
            // If we are supposed to append a space when we
            // are done skipping white space then do so now
            if ( appendSpace ) {
               data += ' ';
               appendSpace = false;
            }
            data += c;
            state = ParseData;
         }
      } else {
         LOG_ERR( "Parse error, invalid state" );
         return -1;
      }
   }

   // Return an error as we parsed through the entire buffer but
   // were unable to determine the field/data
   LOG_ERR( "Parse error, unable to parse" );
   return -1;
}

const char *HttpHeaderBase::getField( FieldMap::FieldType type ) const {
   for (int i = 0; i < mNumFields; i++) {
      if ( mFields[i].mType == type )
         return mFields[i].mData;
   }

   return NULL;
}

ErrCode HttpHeaderBase::getFieldInt( FieldMap::FieldType type,
      int32_t &value ) const {
   const char *str = getField( type );

   if ( str != NULL ) {
      value = strtol( str, NULL, 0 );
      return kNoError;
   }

   value = 0;
   return kNotFound;
}

ErrCode HttpHeaderBase::getFieldInt64( FieldMap::FieldType type,
      int64_t &value ) const {
   const char *str = getField( type );

   if ( str != NULL ) {
      value = strtoll( str, NULL, 0 );
      return kNoError;
   }

   value = 0;
   return kNotFound;
}

ErrCode HttpHeaderBase::removeField( FieldMap::FieldType type ) {
   TRACE_BEGIN(LOG_LVL_NOISE);

   for (int i = 0; i < mNumFields; i++) {
      if ( mFields[i].mType == type ) {
         if ( mFields[i].mData ) {
            delete[] mFields[i].mData;
         }

         for (int j = i; j < mNumFields - 1; j++) {
            mFields[j] = mFields[j + 1];
         }
         mFields[mNumFields - 1].mData = NULL;
         mNumFields--;
         return kNoError;
      }
   }
   return kNotFound;
}

bool HttpHeaderBase::addField( FieldMap::FieldType type, const char *data ) {
   TRACE_BEGIN(LOG_LVL_NOISE);

   if ( mNumFields >= kMaxFields )
      return false;

   mFields[mNumFields].mType = type;
   mFields[mNumFields].setData( data );
   mNumFields++;

   // Make sure we re-build the header if it has already been built.
   mLen = 0;

   return true;
}

bool HttpHeaderBase::addFieldDecimal( FieldMap::FieldType type, int32_t data ) {
   char buffer[32];

   sprintf( buffer, "%d", data );

   return addField( type, buffer );
}

bool HttpHeaderBase::addFieldHex( FieldMap::FieldType type, uint32_t data ) {
   char buffer[32];

   sprintf( buffer, "%x", data );

   return addField( type, buffer );
}

bool HttpHeaderBase::addFieldDecimal64( FieldMap::FieldType type,
      int64_t data ) {
   char buffer[32];

   sprintf( buffer, "%ld", data );

   return addField( type, buffer );
}

void HttpHeaderBase::buildHeader() const {
   TRACE_BEGIN(LOG_LVL_NOISE);

   const char *buffer = "\r\n";

   int res = buildFirstLine();
   int offset = res;

   if ( res <= 0 )
      return;

   for (int i = 0; i < mNumFields; i++) {
      res = buildLine( offset, &mFields[i] );
      if ( res <= 0 )
         return;

      offset += res;
   }

   memcpy( &mBuffer[offset], buffer, strlen( buffer ) + 1 );

   mLen = offset + strlen( buffer );
}

int HttpHeaderBase::buildLine( int offset, const HeaderField *field ) const {
   TRACE_BEGIN(LOG_LVL_NOISE);

   const char *fieldTypeString = getFieldTypeString( field->mType );

   int lineLength = field->mDataLength + strlen( fieldTypeString ) + 10;
   // 10 extra bytes to cover extra chars in format string and line termination in buildHeader.

   if ( ( offset + lineLength ) > mBufferSize ) {
      int needed = ( offset + lineLength ) - mBufferSize;
      if ( needed < kBufferInitSize )
         needed = kBufferInitSize;
      int newBufferSize = mBufferSize + needed;

      LOG_ERR( "Growing HttpHeaderBase::mBuffer from %d to %d bytes",
            mBufferSize, newBufferSize );

      char *newBuffer = new char[newBufferSize];
      memcpy( newBuffer, mBuffer, mBufferSize );
      delete[] mBuffer;
      mBufferSize = newBufferSize;
      mBuffer = newBuffer;
   }

   return snprintf( ( char* ) &mBuffer[offset], mBufferSize - offset,
         "%s: %s\r\n", fieldTypeString, field->mData );
}

int HttpHeaderBase::searchForLine( const CircularBuffer &buffer ) {
   TRACE_BEGIN(LOG_LVL_NOISE);

   enum {
      SearchCR, HaveCR
   } state = SearchCR;

   // Initial length of 1
   int length = 1;
   int c;

   // Continue until we reach the end of the buffer.  On each
   // iteration of the loop increment the index and the length
   for (int i = 0; i < buffer.getLength(); ++i, ++length) {
      // Fetch the byte and update the parse length
      c = buffer.byteAt( i );

      // This shouldn't happen, but make sure...
      if ( c == -1 ) {
         LOG_ERR( "Index %d, Buffer length %d, got -1 from byteAt", i,
               buffer.getLength() );
         return -1;
      }

      if ( state == SearchCR ) {
         // Check for CR
         if ( c == '\r' ) {
            // Update state to check for LF
            state = HaveCR;
         }
      } else if ( state == HaveCR ) {
         // Check for LF
         if ( c == '\n' ) {
            // Fetch the next byte from the buffer so we
            // can test for continuation.
            c = buffer.byteAt( i + 1 );

            // If the current length is 2 then this
            // is the end of the header
            if ( length == 2 ) {
               return HttpHeaderBase::kEOH;
            }
            // Otherwise fetch the next byte and check
            // to see if it is a continuation.  If it is
            // then reset state to 0
            else if ( c == ' ' or c == '\t' ) {
               state = SearchCR;
            }
            // Finally this wasn't a continuation so we have
            // found the end of the header line, return the length
            else {
               return length;
            }
         }
         // If no LF then search for another CR
         else {
            state = SearchCR;
         }
      }
   }

   // Return an error as we searched the entire buffer and did not
   // find a CRLF yet.
   LOG("Unable to find header field in buffer");
   return -1;
}

//
// HttpHeaderBase::HeaderField functions
//

/**
 * @brief Copy data specified into the HeaderField
 */
void HttpHeaderBase::HeaderField::setData( const char *data ) {
   if ( mData ) {
      delete[] mData;
   }
   mDataLength = strlen( data ) + 1;
   mData = new char[mDataLength];
   strcpy( mData, data );
}
