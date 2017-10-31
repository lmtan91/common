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

#ifndef INCLUDE_ERRCODE_H_
#define INCLUDE_ERRCODE_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/

enum ErrCode {
   kNoError = 0, kNotFound,        // object not found
   kConnectionFailed,   // connection attempt failed
   kPermissionDenied,   // not sufficent persmission for operation
   kBusy,            // resource busy, action needed to free.
   kTemporarilyBusy, // resource busy, but will soon be free.
   kInProgress,      // request in progress
   kAlreadyRequested,   // resource already added/requested.
   kTimedOut,        // operation timed out.
   kNotImplemented,  // operation not implemented.
   kOpenFailed,      // open of a file or device failed.
   kInvalidRequest,  // the parameters specificed in the request are not valid.
   kNotInitialized,  // the device or resource has not been initialized yet
   kReadFailed,      // an in progress read failed
   kWriteFailed,       // a write failed

   kMaxErrorString,

   kFirstUserErrorCode = 1000
};

/**
 * Get a easy to read message associate with an error code.
 */
const char *getErrorString( ErrCode );

#endif /* ifndef INCLUDE_ERRCODE_H_ */
