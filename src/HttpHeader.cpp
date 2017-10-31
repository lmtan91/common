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

#include "HttpHeader.h"
#include "logging.h"

#include <time.h>
#include <string.h>

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOTICE );

//
// HttpFieldMap functions
//

// All HTTP/1.1 Header Fields
static const char *gFieldTypeList[] = { "Accept", "Accept-Charset",
      "Accept-Encoding", "Accept-Language", "Accept-Ranges", "Age", "Allow",
      "Authorization", "Cache-Control", "Connection", "Content-Encoding",
      "Content-Language", "Content-Length", "Content-Location", "Content-MD5",
      "Content-Range", "Content-Type", "Date", "ETag", "Expect", "Expires",
      "From", "Host", "If-Match", "If-Modified-Since", "If-None-Match",
      "If-Range", "If-Unmodified-Since", "Last-Modified", "Location",
      "Max-Forwards", "Pragma", "Proxy-Authenticate", "Proxy-Authorization",
      "Range", "Referer", "Retry-After", "Server", "TE", "Trailer",
      "Transfer-Encoding", "Upgrade", "User-Agent", "Vary", "Via", "Warning",
      "WWW-Authenticate" };

FieldMap::FieldType HttpFieldMap::getFieldType( const char *field ) const {
   TRACE_BEGIN(LOG_LVL_NOISE);

   for (int i = 0; i < kNumFields - FieldMap::kHttpFieldMapStart; ++i) {
      if ( strcasecmp( field, gFieldTypeList[i] ) == 0 ) {
         LOG("type %d, name %s",
               i+FieldMap::kHttpFieldMapStart, gFieldTypeList[i]);
         return ( FieldMap::FieldType ) i + FieldMap::kHttpFieldMapStart;
      }
   }
   return FieldMap::kInvalidFieldType;
}

const char * HttpFieldMap::getFieldTypeString( FieldType type ) const {
   TRACE_BEGIN(LOG_LVL_NOISE);

   if ( type >= FieldMap::kHttpFieldMapStart and type < kNumFields ) {
      type -= FieldMap::kHttpFieldMapStart;

      LOG("type %d, name %s",
            type, gFieldTypeList[type]);

      return gFieldTypeList[type];
   }
   return NULL;
}


//
// HttpHeader functions
//

bool HttpHeader::addDateField() {
   time_t t = time( NULL );
   struct tm tmp;
   localtime_r( &t, &tmp );

   char buf[64];
   strftime( buf, 64, "%a, %b %e %T %Y", &tmp );
   return addField( HttpFieldMap::kFieldDate, buf );
}
