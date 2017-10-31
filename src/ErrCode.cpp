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


#include "ErrCode.h"

#include "logging.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

static const char *gErrorStrings[] = { "No Error", "Not Found",
      "Connection Failed", "Permission Denied", "Resource Busy",
      "Resource Temporarily Busy, Try Again", "Operation In Progress",
      "Resource Already Requested", "Operation Timed Out",
      "Operation Not Implemented", "Open of resource failed",
      "Invalid Parameter in request", "Resource not initialized", "Read Failed",
      "Write Failed",

      // keep in sync with JetHead.h
      "Max Error String", };

const char *getErrorString( ErrCode err ) {
   // protect against enum bounds
   if ( err >= kMaxErrorString )
      return NULL;

   return gErrorStrings[( int ) err];
}
