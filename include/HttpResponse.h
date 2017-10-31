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

#ifndef INCLUDE_HTTPRESPONSE_H_
#define INCLUDE_HTTPRESPONSE_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/

/**============================================================================
 * Class description
 * 
 * 
 *============================================================================*/
#include "HttpHeader.h"

class HttpResponse: public HttpHeader {
public:
   HttpResponse();
   virtual ~HttpResponse();

   int getResponseCode() const {
      return mResponseCode;
   }
   void setResponseCode( int code ) {
      mResponseCode = code;
   }

   const char *getResponseString() const;

   int parseFirstLine( const CircularBuffer &buf );

private:
   int buildFirstLine() const;

   const char *getResponseString( int code ) const;

   int mResponseCode;
};
#endif /* ifndef INCLUDE_HTTPRESPONSE_H_ */
