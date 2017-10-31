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

#ifndef INCLUDE_FIELDMAP_H_
#define INCLUDE_FIELDMAP_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/

/**============================================================================
 * @brief Field mapping base interface
 *
 * The FieldMap interface is used by the HttpHeaderBase class to parse
 * and generate HTTP-style header fields.   Many open standards use HTTP
 * as their basis and define custom fields to communicate information
 * between the client and server.  Parsing and generation of these
 * custom header fields is all general and can be handled in a single
 * place (HttpHeaderBase), the FieldMap interface allows users to
 * provide the HttpHeaderBase with knowledge about the fields it cares
 * to parse and/or generate.
 * 
 *============================================================================*/
class FieldMap {

public:

   /**=========================================================================
    * @brief Constructor
    *
    * @param[in]
    *=========================================================================*/
   FieldMap() {
   }

   /**=========================================================================
    * @brief Destructor
    *
    *
    * @param[in]
    *=========================================================================*/
   virtual ~FieldMap() {
   }

   typedef int FieldType;

   static const int kInvalidFieldType = -1;

   static const int kRtspFieldMapStart = 100;
   static const int kGenaFieldMapStart = 200;
   static const int kHttpFieldMapStart = 300;

   virtual FieldType getFieldType( const char *field ) const=0;
   virtual const char *getFieldTypeString( FieldType type ) const =0;
};

#endif /* ifndef INCLUDE_FIELDMAP_H_ */
