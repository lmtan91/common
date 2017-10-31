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

#ifndef INCLUDE_HTTPREQUEST_H_
#define INCLUDE_HTTPREQUEST_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/
#include "HttpHeader.h"
/**============================================================================
 * Class description
 * 
 * 
 *============================================================================*/
class HttpRequest: public HttpHeader {
public:
   // This must match the listings in HttpRequest.cpp AND
   // WebServer.cpp
   enum MethodType {
      kMethodOptions,
      kMethodGet,
      kMethodHead,
      kMethodPost,
      kMethodPut,
      kMethodDelete,
      kMethodTrace,
      kMethodConnect,
      kMethodUnknown,
   };

   HttpRequest();
   HttpRequest( const URI &uri, MethodType type = kMethodGet );
   virtual ~HttpRequest();

   MethodType getMethod() const {
      return mMethod;
   }
   const URI &getURI() const {
      return mUri;
   }
   void getVersion( int& major, int& minor );

   void setMethod( MethodType type ) {
      mMethod = type;
   }
   void setURI( const URI &uri ) {
      mUri = uri;
   }

   int parseFirstLine( const CircularBuffer &buf );

protected:
   virtual int buildFirstLine() const;

   void parseMethod( const char *method );
   void parseProtocol( const char *protocol );

private:
   MethodType mMethod;
   URI mUri;

   int mMajor;
   int mMinor;
};


#endif /* ifndef INCLUDE_HTTPREQUEST_H_ */
