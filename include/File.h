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

#ifndef INCLUDE_FILE_H_
#define INCLUDE_FILE_H_

/**
 * \file
 *
 * A basic file object, implements the IReaderWriter interface.  Be
 * careful of the open/open2 distinctions.
 */

#include <sys/mman.h>

#include "IReaderWriter.h"
#include "Selector.h"

/**
 * @brief File class
 */
class File: public IReaderWriter {
public:
   //! Default constructor, doesn't really do anything.
   File();

   //! Close it up if needed
   virtual ~File();

   /**=========================================================================
    * Open a file giving the filename.  If you are opening a file
    * that exists this class will check your permission on the file
    * and open it with the least restrictive permissions allowed.
    * You can specify other options through the flags param, these
    * are the same option that go to the system call open.
    *
    *  @note You probably need to read the description on this
    *  carefully if you are having problems with opening files.
    *
    * @param name the name of the file to open or create.
    *
    * @param flags the flags for the open, O_RDWR flags do not need
    * to be set.
    *
    * @return the -errno or 0
    *=========================================================================*/
   virtual int open( const char *name, int flags = 0 );

   /**=========================================================================
    * A straight passthrough to open(2).  Doesn't do the
    * flag-manipulation that File::open does.
    *=========================================================================*/
   virtual int open2( const char *name, int flags = 0 );

   /**=========================================================================
    * Close the file allowing this object to open another file.
    *=========================================================================*/
   virtual int close();

   /**=========================================================================
    * Add a selector listener to the File
    *=========================================================================*/
   void setSelector( SelectorListener *listener, Selector *selector );

   //! Add a listener with the specified event mask
   void setSelector( SelectorListener *listener, Selector *selector,
         short events );

   //! A passthrough for read(2)
   int read( void *buffer, int len );

   //! A passthrough for write(2)
   int write( const void *buffer, int len );

   /**=========================================================================
    * Get the file position (64-bit safe)
    *=========================================================================*/
   virtual __off64_t getPos() const;

   /**=========================================================================
    * Seek end of file.  This returns offset to end of file (64-bit safe)
    *=========================================================================*/
   virtual __off64_t seekEnd();

   /**=========================================================================
    * Set the file position (64-bit safe)
    *=========================================================================*/
   virtual __off64_t setPos( __off64_t offset );

   /**=========================================================================
    * Return the start offset of the file.  For normal files
    * this is always 0
    *=========================================================================*/
   virtual __off64_t getStartOffset() const {
      return ( __off64_t ) 0LL;
   }

   /**=========================================================================
    * Get the file length (64-bit safe).  Used in combination with
    * the start offset this can be used to get the end offset of
    * the file.
    *=========================================================================*/
   virtual __off64_t getLength() const;

   /**=========================================================================
    * mmap a file, see the man page mmap(2)
    *
    * @return pointer to buffer or MAP_FAILED
    *=========================================================================*/
   virtual uint8_t *mmap( off_t offset, size_t length,
         int prot = ( PROT_READ | PROT_WRITE ) );

   //! Unmap the memory mapping for this file
   virtual void munmap();

   //! Make sure everything in the memory mapped buffer gets to disk
   virtual void msync();

   /**=========================================================================
    * copies all data from caches to disk
    *=========================================================================*/
   virtual void fsync();

   //! Helper for "is this a valid file on disk?"
   static bool isFile( const char *name );

   //! Helper for "is this a valid directory name on disk?"
   static bool isDir( const char *name );

   //! Helper for "is there a directory entry of this name?"
   static bool exists( const char *name );

   //! Helper for "how big is this file?"
   static int size( const char *name );

protected:
   /**=========================================================================
    * The file descriptor from open(2)
    * @todo Should this be made private?
    *=========================================================================*/
   int mFd;

   //! Give access to the fd for child classes
   virtual int getFd() {
      return mFd;
   }

private:
   //! The selector we are living on
   Selector *mSelector;

   //! The address we are mmaped onto
   uint8_t *mMapAddress;

   //! The size of the mmapped buffer (needed for munmap)
   size_t mMapLength;
};

#endif /* ifndef INCLUDE_FILE_H_ */
