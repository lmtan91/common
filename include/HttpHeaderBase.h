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

#ifndef INCLUDE_HTTPHEADERBASE_H_
#define INCLUDE_HTTPHEADERBASE_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/
#include <list>

#include "ErrCode.h"
#include "Socket.h"
#include "FieldMap.h"
#include "CircularBuffer.h"

/**============================================================================
 * @brief Base class for HTTP-style header generation/parsing
 *
 * This class provides a generic means to parse and generate HTTP-style
 * headers.  Users derive from this class and implement the
 * parseFirstLine and buildFirstLine methods to generate and parse the
 * appropriate first line of the HTTP-style header.
 * 
 * Users provide the HttpHeaderBase with field mappings from a
 * FieldMap::FieldType to a string and the inverse and the HttpHeaderBase
 * will parse and generate fields appropriately.  This can be further
 * extended in derived classes, adding additional FieldMaps.
 *
 *============================================================================*/
class IReaderWriter;
class HttpHeaderBase {

public:

   /**=========================================================================
    * @brief Constructor
    *
    *=========================================================================*/
   HttpHeaderBase();

   /**=========================================================================
    * @brief Destructor
    *
    *=========================================================================*/
   virtual ~HttpHeaderBase();

   /**=========================================================================
    * @brief Generate and transmit HTTP-style header.
    * The HttpHeaderBase will generate an HTTP-style header from the
    * list of fields the user has specified (parsed) and write the
    * header to the IReaderWriter specified.
    *
    *=========================================================================*/
   int send( IReaderWriter *writer ) const;

   /**=========================================================================
    * @brief Send header and body in one write
    *
    * The HttpHeaderBase will generate an HTTP-style header from the
    * list of fields the user has specified (parsed) and write the
    * header to the IReaderWriter specified, along with the specified data
    *
    *  @note This requires making a copy of the supplied data!
    *=========================================================================*/
   int send( IReaderWriter *writer, const uint8_t* data, int len ) const;

   /**=========================================================================
    * @brief Generate and transmit HTTP-style header
    *
    * The HttpHeaderBase will generate an HTTP-style header from the
    * list of fields the user has specified (parsed) and write the
    * header to the IReaderWriter specified.
    *=========================================================================*/
   int sendto( Socket *writer, const Socket::Address& addr ) const;

   /**=========================================================================
    * @brief Retrieve string value of a field
    *
    * This method retrives the string value associated with the
    * field specified.
    *
    * @return     Errors:        NULL
    *=========================================================================*/
   const char *getField( FieldMap::FieldType type ) const;

   /**=========================================================================
    * @brief Retrieve int value of a field
    *
    * This method retrieves the int value associated with the
    * field specified.
    *
    * @return     Errors:        kNotFound
    * @return     Success:    kNoError
    *=========================================================================*/
   ErrCode getFieldInt( FieldMap::FieldType type,
         int32_t &value ) const;

   /**=========================================================================
    * @brief Retrieve int64 value of a field
    *
    * This method retrieves the int value associated with the
    * field specified.
    *
    * @return     Errors:        kNotFound
    * @return     Success:    kNoError
    *=========================================================================*/
   ErrCode getFieldInt64( FieldMap::FieldType type,
         int64_t &value ) const;

   /**=========================================================================
    *  @brief Remove a field from a fieldmap
    *
    *  This method removes the first instance of a given field from
    *  the fieldmap.
    *
    *  @return     Errors:         kNotFound
    *  @return     Success:        kNoError
    *=========================================================================*/
   ErrCode removeField( FieldMap::FieldType type );

   /**=========================================================================
    * @brief Add a field to the header (for output)
    *
    * Given the field ID and some data to set as the value of that
    * header, add a new header field.
    *
    * @return  true if successful, false if too many fields
    *=========================================================================*/
   bool addField( FieldMap::FieldType type, const char *data );

   /**=========================================================================
    * @brief Add a field to the header (for output)
    *
    * Given the field ID and an int value for that field, add a new
    * header field.
    *
    * @return  true if successful, false if too many fields
    *=========================================================================*/
   bool addFieldDecimal( FieldMap::FieldType type, int32_t data );

   /**=========================================================================
    * @brief Add a field to the header (for output)
    *
    * Given the field ID and an int64 value for that field, add a new
    * header field.
    *
    * @return  true if successful, false if too many fields
    *=========================================================================*/
   bool addFieldDecimal64( FieldMap::FieldType type, int64_t data );

   /**=========================================================================
    * @brief Add a field to the header (for output)
    *
    * Given the field ID and an int value for that field, add a new
    * header field in hex
    *
    * @return  true if successful, false if too many fields
    *=========================================================================*/
   bool addFieldHex( FieldMap::FieldType type, uint32_t data );

   /**=========================================================================
    * @brief Parse first line of an HTTP-style header
    *
    * This method should be implemented to do something useful by
    * specialized HttpHeaderBase children.   When an Agent receives
    * an HTTP-style request it would call this method to parse the
    * first line of the header and then make additional calls to
    * parseLine() until the end of the header was detected.
    *
    * NOTE:  Circular buffer data is not consumed by this call,
    * the caller is responsible for updating the circular buffer
    * after completing a call to this method by the number of bytes
    * returned.
    *
    * @return Success: Number of bytes parsed to retrieve first line
    *=========================================================================*/
   virtual int parseFirstLine( const CircularBuffer &buf ) {
      ( void ) buf;
      return 0;
   }

   /**=========================================================================
    * @brief Parse field from an HTTP-style header
    *
    * This method is called to parse and retrieve a field from an
    * HTTP-style header.   It will parse a single field and, using the
    * set of FieldMaps that the HttpHeaderBase has available to make
    * sense of it, any unknown fields are dropped on the floor.
    *
    * NOTE:  Circular buffer data is not consumed by this call,
    * the caller is responsible for updating the circular buffer
    * after completing a call to this method by the number of bytes
    * returned.
    *
    * @return     Success:    Number of bytes parsed to retrieve field
    *=========================================================================*/
   virtual int parseLine( const CircularBuffer &buf );

   /**=========================================================================
    * the return value for the end of header condition returned
    * by the searchForLine method
    *=========================================================================*/
   static const int kEOH = 0;

   /**=========================================================================
    * @brief Search for a line in the circular buffer
    *
    * This helper method will search for the end of an HTTP style
    * header line in the circular buffer and return the length of the
    * line if one is found.
    *
    * @return        -1    Indicates no field lines found in buffer
    * @return        kEOH  If a field line of 0 length is found
    * @return        length   Number of bytes parsed in the field line
    *=========================================================================*/
   static int searchForLine( const CircularBuffer &buffer );

   //! Print out this header (to stdout)
   void printHeader();

   //! Get back a copy of the assembled header
   const char *getHeader();

protected:
   /**=========================================================================
    * @brief Add field mapping
    *
    * Add a new field mapping to the HttpHeaderBase.  The new mapping will
    * be used during parse/generation of the HTTP-style header and cleaned
    * up when the HttpHeaderBase is destroyed
    *
    * @see HttpHeader
    *=========================================================================*/
   int addFieldMap( FieldMap *fieldMap );

   //! Return the length of the header
   int getLength() const {
      return mLen;
   }

   /**=========================================================================
    * @brief Generate first line of an HTTP-style header
    *
    * This method should be implemented to do something useful by
    * specialized HttpHeaderBase children.   Data should be affixed
    * to the beginning of the mBuffer member and the number of
    * bytes consumed returned (this includes CR+LF)
    *
    *=========================================================================*/
   virtual int buildFirstLine() const {
      return 0;
   }

   /**=========================================================================
    * The buffer where the HTTP-style header is generated to before
    * being written to the IReaderWriter in the send method.
    *=========================================================================*/
   mutable char *mBuffer;
   mutable int mBufferSize;

   //! The length of mBuffer used by the header
   mutable int mLen;

private:

   struct HeaderField {
      //! Default constructor, doesn't have to do much
      HeaderField() :
            mType( 0 ), mData( NULL ), mDataLength( 0 ) {
      }

      //! Free up the space we've allocated
      virtual ~HeaderField() {
         delete[] mData;
      }

      //! Set the data for this field
      void setData( const char *data );

      //! Set the field type for this field
      void setType( FieldMap::FieldType field ) {
         mType = field;
      }

      FieldMap::FieldType mType;
      char *mData;
      int mDataLength;
   };

   //! Look up a field ID by name
   FieldMap::FieldType getFieldType( const char *field ) const;

   //! Look up a field string by ID
   const char *getFieldTypeString( FieldMap::FieldType type ) const;

   //! Build the header into mBuffer
   void buildHeader() const;

   //! A helper for buildHeader, write one field into (mBuffer + offset)
   int buildLine( int offset, const HeaderField *field ) const;

   //! The maximum size of the message
   static const int kBufferInitSize = 1024;

   //! The maximum number of fields in this header
   static const int kMaxFields = 32;

   //! The fields
   HeaderField mFields[kMaxFields];

   //! How many fields have we used
   int mNumFields;

   //! The mapping from field names to FieldType
   std::list<FieldMap*> mFieldMappings;
};

#endif /* ifndef INCLUDE_HTTPHEADERBASE_H_ */
