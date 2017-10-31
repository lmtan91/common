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

#include "HttpRequest.h"
#include "logging.h"

#include <string.h>


SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOTICE );

static const unsigned MAX_METHOD_SIZE = 128;
static const unsigned MAX_URI_SIZE = 2048;
static const unsigned MAX_PROTO_SIZE = 64;

// This must match the enumeration in HttpRequest.h
static const char *gMethodTypeList[] = { "OPTIONS", "GET", "HEAD", "POST",
      "PUT", "DELETE", "TRACE", "CONNECT", };

HttpRequest::HttpRequest() :
      mMethod( kMethodUnknown ), mMajor( -1 ), mMinor( -1 ) {
}

HttpRequest::HttpRequest( const URI &uri, MethodType type ) :
      mMethod( type ), mUri( uri ) {
   char buf[16];
   snprintf( buf, 15, "%d", uri.getPort() );
   std::string host = uri.getHost();
   host += ":";
   host += buf;

   addField( HttpFieldMap::kFieldHost, host.c_str() );
   addDateField();
}

HttpRequest::~HttpRequest() {
}

int HttpRequest::parseFirstLine( const CircularBuffer &buf ) {
   TRACE_BEGIN( LOG_LVL_INFO );

   std::string method;
   method.reserve( MAX_METHOD_SIZE );
   std::string uri;
   uri.reserve( MAX_URI_SIZE );
   std::string protocol;
   protocol.reserve( MAX_PROTO_SIZE );
   int i = 0;
   int c;

   while (1) {
      c = buf.byteAt( i++ );

      // If we requested a byte beyond the line size . . .
      if ( c == -1 or i > buf.getLength() ) {
         return -1;
      }

      if ( c == ' ' ) {
         break;
      }

      method += c;
      if ( method.size() >= MAX_METHOD_SIZE )
         return -1;
   }

   while (( c = buf.byteAt( i ) == ' ' )) {
      i++;

      // If we requested a byte beyond the line size . . .
      if ( c == -1 or i > buf.getLength() ) {
         return -1;
      }
   }

   while (1) {
      c = buf.byteAt( i++ );

      // If we requested a byte beyond the line size . . .
      if ( c == -1 or i > buf.getLength() ) {
         return -1;
      }

      if ( c == ' ' )
         break;

      uri += c;
      if ( uri.size() >= MAX_URI_SIZE )
         return -1;
   }

   while (1) {
      c = buf.byteAt( i++ );

      // If we requested a byte beyond the line size . . .
      if ( c == -1 or i > buf.getLength() ) {
         return -1;
      }

      if ( c == '\r' )
         break;

      protocol += c;
      if ( protocol.size() >= MAX_PROTO_SIZE )
         return -1;
   }

   // if CRLF move past LF
   if ( buf.byteAt( i ) == '\n' )
      i++;

   LOG( "Method %s, URI %s, Proto %s",
         method.c_str(), uri.c_str(), protocol.c_str() );

   parseProtocol( protocol.c_str() );
   parseMethod( method.c_str() );
   mUri = uri.c_str();

   return i;
}

int HttpRequest::buildFirstLine() const {
   if ( mUri.getQuery().empty() ) {
      return snprintf( ( char* ) mBuffer, mBufferSize, "%s %s HTTP/1.1\r\n",
            gMethodTypeList[mMethod], mUri.getPath().c_str() );
   } else {
      return snprintf( ( char* ) mBuffer, mBufferSize, "%s %s?%s HTTP/1.1\r\n",
            gMethodTypeList[mMethod], mUri.getPath().c_str(),
            mUri.getQuery().c_str() );
   }
}

void HttpRequest::parseMethod( const char *method ) {
   TRACE_BEGIN( LOG_LVL_INFO );

   for (uint32_t i = 0; i < ARRAY_SIZE( gMethodTypeList ); i++) {
      //LOG_NOISE( "matching %s", gMethodTypeList[ i ] );
      if ( strcmp( method, gMethodTypeList[i] ) == 0 ) {
         mMethod = ( MethodType ) i;
         LOG( "Found method %s", gMethodTypeList[ mMethod ] );
         return;
      }
   }

   LOG( "Failed to match method \"%s\"", method );
   mMethod = kMethodUnknown;
}

void HttpRequest::parseProtocol( const char *protocol ) {
   TRACE_BEGIN( LOG_LVL_INFO );
   sscanf( protocol, "HTTP/%d.%d", &mMajor, &mMinor );
   LOG("Major proto %d minor proto %d", mMajor, mMinor);
}

void HttpRequest::getVersion( int& major, int& minor ) {
   major = mMajor;
   minor = mMinor;
}
