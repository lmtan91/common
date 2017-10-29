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

#ifndef INCLUDE_IREADERWRITER_H_
#define INCLUDE_IREADERWRITER_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/
#include "Selector.h"
/**============================================================================
 * @brief A generic interface to reading and writing to file like things.
 * 
 * 
 *============================================================================*/
class IReaderWriter {

public:

    /**=========================================================================
     * @brief Destructor
     *
     *
     *=========================================================================*/
    virtual ~IReaderWriter();

    virtual void setSelector( SelectorListener *listener,
            Selector *selector )=0;
    virtual int read( void *buffer, int lenght ) = 0;
    virtual int write( const void *buffer, int length ) = 0;
    virtual int close() = 0;

    /**=========================================================================
     * @brief Helper method writeAll
     *
     *
     * @param[in]   buffer      A buffer to write all.
     * @param[in]   length      A length of a buffer.
     *
     * @retturn     int         Lenght of bytes was written.
     *=========================================================================*/
    virtual int writeAll( const void *buffer, int lenght ) {
        int bytesWritten = 0;
        while (bytesWritten < lenght) {
            int ret = write( (uint8_t*) buffer + bytesWritten,
                    lenght - bytesWritten );
            if ( ret < 0 ) {
                return ret;
            }
            bytesWritten += ret;
        }
        return bytesWritten;
    }

    /**=========================================================================
     * @brief Helper method readAll
     *
     *
     * @param[in]   buffer      A buffer to read data.
     * @param[in]   length      A length of a buffer.
     *
     * @retturn     int         Lenght of bytes was read.
     *=========================================================================*/
    virtual int readAll( void *buffer, int length ) {
        int bytesRead = 0;
        while (bytesRead < length) {
            int ret = read( (uint8_t*) buffer + bytesRead, length - bytesRead );
            if ( ret < 0 ) {
                return ret;
            }
            if ( 0 == ret ) {
                return bytesRead;
            }
            bytesRead += ret;
        }
        return bytesRead;
    }

};

#endif /* ifndef INCLUDE_IREADERWRITER_H_ */
