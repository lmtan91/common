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

#ifndef INCLUDE_HTTPHEADER_H_
#define INCLUDE_HTTPHEADER_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/
#include "HttpHeaderBase.h"
#include "FieldMap.h"

/**============================================================================
 * @brief This class provides field mappings for standard HTTP 1.1 header fields
 * 
 * 
 *============================================================================*/
class HttpFieldMap: public FieldMap {
public:
   enum HttpFieldType {
      kFieldAccept = FieldMap::kHttpFieldMapStart,
      kFieldAcceptCharset,
      kFieldAcceptEncoding,
      kFieldAcceptLanguage,
      kFieldAcceptRanges,
      kFieldAge,
      kFieldAllow,
      kFieldAuthorization,
      kFieldCacheControl,
      kFieldConnection,
      kFieldContentEncoding,
      kFieldContentLanguage,
      kFieldContentLength,
      kFieldContentLocation,
      kFieldContentMD5,
      kFieldContentRange,
      kFieldContentType,
      kFieldDate,
      kFieldETag,
      kFieldExpect,
      kFieldExpires,
      kFieldFrom,
      kFieldHost,
      kFieldIfMatch,
      kFieldIfModifiedSince,
      kFieldIfNoneMatch,
      kFieldIfRange,
      kFieldIfUnmodifiedSince,
      kFieldLastModified,
      kFieldLocation,
      kFieldMaxForwards,
      kFieldPragma,
      kFieldProxyAuthenticate,
      kFieldProxyAuthorization,
      kFieldRange,
      kFieldReferer,
      kFieldRetryAfter,
      kFieldServer,
      kFieldTE,
      kFieldTrailer,
      kFieldTransferEncoding,
      kFieldUpgrade,
      kFieldUserAgent,
      kFieldVary,
      kFieldVia,
      kFieldWarning,
      kFieldWWWAuthenticate,

      kNumFields
   };
   // Virtual destructor
   virtual ~HttpFieldMap() {
   }
private:

   //
   // FieldMap interface
   //

   //! Return the field ID given the header string
   FieldType getFieldType( const char *field ) const;

   //! Return the header string given the field ID
   const char *getFieldTypeString( FieldType fieldType ) const;
};

/**============================================================================
 * @brief Base HTTP 1.1 header
 *
 * This base class initializes with HTTP 1.1 Field mappings and
 * provides a convenient method to invoke to generate an HTTP 1.1
 * compliant Date field.
 *============================================================================*/
class HttpHeader: public HttpHeaderBase {

public:

   //! Extend HttpHeaderBase with the HttpFieldMap, above
   HttpHeader() {
      addFieldMap( new HttpFieldMap() );
}

   //! Virtual do-nothing destructor
   virtual ~HttpHeader() {
   }

   //! Put the date value in the header
   bool addDateField();
};

#endif /* ifndef INCLUDE_HTTPHEADER_H_ */
