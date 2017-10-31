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

#include "HttpResponse.h"
#include "logging.h"

#include <string.h>

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOTICE );

struct ResponseCodeData {
   int code;
   const char *description;
};

ResponseCodeData gResponseCodeList[] = {
   { 100, "Continue" },
   { 100, "Switching Protocols" },

   { 200, "OK" },
   { 201, "Created" },
   { 202, "Accepted" },
   { 203, "Non-Authorative Information" },
   { 204, "No Content" },
   { 205, "Reset Content" },
   { 206, "Partial Content" },

   { 300, "Multiple Choices" },
   { 301, "Moved Permanently" },
   { 302, "Found" },
   { 303, "See Other" },
   { 304, "Not Modified" },
   { 305, "Use Proxy" },
   { 305, "Temporary Redirect" },

   { 400, "Bad Request" },
   { 401, "Unauthorized" },
   { 402, "Payment Required" },
   { 403, "Forbidden" },
   { 404, "Not Found" },
   { 405, "Method Not Allowed" },
   { 406, "Not Acceptable" },
   { 407, "Proxy Authentication Required" },
   { 408, "Request Timeout" },
   { 410, "Gone" },
   { 411, "Length Required" },
   { 412, "Precondition Failed" },
   { 413, "Request Entity Too Large" },
   { 414, "Request-URI Too Long" },
   { 415, "Unsupported Media Type" },
   { 416, "Request Range Not Satisfiable" },
   { 417, "Expectation Failed" },

   { 500, "Internal Server Error" },
   { 501, "Not Implemented" },
   { 502, "Bad Gateway" },
   { 503, "Service Unavailable" },
   { 504, "Gateway Timeout" },
   { 505, "HTTP Version Not Supported" },
};

HttpResponse::HttpResponse() :
      mResponseCode( 200 ) {
}

HttpResponse::~HttpResponse() {
   mResponseCode = -1;
}

const char *HttpResponse::getResponseString() const {
   if ( mResponseCode == -1 ) {
      LOG_ERR_FATAL( "called after destruction with mResponseCode (%d)",
            mResponseCode );
   } else if ( mResponseCode < 100 or mResponseCode > 599 ) {
      LOG_ERR( "unexpected mResponseCode (%d)", mResponseCode );
   }

   return getResponseString( mResponseCode );
}

int HttpResponse::parseFirstLine( const CircularBuffer &buf ) {
   TRACE_BEGIN( LOG_LVL_INFO );
   enum {
      MAX_PROTOCOL_SIZE = 40,
      MAX_STATUS_CODE_SIZE = 20,
      MAX_STATUS_TEXT_SIZE = 512,
   };

   // Phases of parsing.
   enum {
      PHASE_PROTOCOL, PHASE_STATUS_CODE, PHASE_STATUS_TEXT
   };

   // If this is a wrong formatted first line just read it without parsing.
   if ( buf.getLength() < 2 ) {
      LOG_ERR( "Bad first line" );
      return buf.getLength();
   }

   // In the case of having got a 100, we might now get a blank line before the real header.
   if ( buf.byteAt( 0 ) == '\r' and buf.byteAt( 1 ) == '\n' ) {
      return 2;
   }

   int i = 0;
   uint32_t j = 0;
   int c = 0;

   // Read the "HTTP-Version" part of the status line to the protocol buffer.
   // Read the "Status-Code" part of the status line to the status_code buffer.
   // Read the "Status-Text" part of the status line to the status_text buffer.
   char protocol[MAX_PROTOCOL_SIZE];
   char status_code[MAX_STATUS_CODE_SIZE];
   char status_text[MAX_STATUS_TEXT_SIZE];

   memset( protocol, 0, sizeof( protocol ) );
   memset( status_code, 0, sizeof( status_code ) );
   memset( status_text, 0, sizeof( status_text ) );

   int currentPhase = PHASE_PROTOCOL;
   char* currentBuffer = protocol;
   uint32_t currentBufferSize = MAX_PROTOCOL_SIZE;

   while (true) {
      c = buf.byteAt( i );
      if ( c < 0 ) {
         // At this point we have reached the end of buffer.
         break;
      }

      ++i;

      if ( c == '\r' ) {
         // At this point we have reached the end of line.
         break;
      }

      if ( currentPhase != PHASE_STATUS_TEXT && isblank( c ) ) {
         // At this point we have reached the end of the status line's field
         // Switch counters and pointers to the next field's buffer.
         j = 0;
         if ( currentPhase == PHASE_PROTOCOL ) {
            currentPhase = PHASE_STATUS_CODE;
            currentBuffer = status_code;
            currentBufferSize = MAX_STATUS_CODE_SIZE;
         } else {
            currentPhase = PHASE_STATUS_TEXT;
            currentBuffer = status_text;
            currentBufferSize = MAX_STATUS_TEXT_SIZE;
         }

         continue;
      }

      // Do not write past of the buffer's end.
      if ( j < ( currentBufferSize - 2 ) ) {
         currentBuffer[j++] = c;
      }
   }

   // if CRLF move past LF
   if ( c >= 0 && buf.byteAt( i ) == '\n' ) {
      ++i;
   }

   LOG( "Proto %s, Code %s, Text %s", protocol, status_code, status_text );

   mResponseCode = strtol( status_code, NULL, 10 );

   // If we got a 100 and there is enough data in the buffer for the
   // (possible) additional \r\n, then consume those now too.
   if ( mResponseCode == 100 and i + 2 <= buf.getLength() ) {
      if ( buf.byteAt( i ) == '\r' and buf.byteAt( i + 1 ) == '\n' ) {
         i += 2;
      }
   }

   return i;
}

int HttpResponse::buildFirstLine() const {
   return snprintf( ( char* ) mBuffer, mBufferSize, "HTTP/1.1 %d %s\r\n",
         mResponseCode, getResponseString( mResponseCode ) );
}

const char *HttpResponse::getResponseString( int code ) const {
   for (uint32_t i = 0; i < ARRAY_SIZE( gResponseCodeList ); i++) {
      if ( gResponseCodeList[i].code == code )
         return gResponseCodeList[i].description;
   }

   return "Unknown HTTP error";
}

