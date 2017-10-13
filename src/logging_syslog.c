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
#include <stdbool.h>
#include <stdlib.h>
#include <syslog.h>

static int syslog_levels[ ] =
{
LOG_CRIT,
LOG_CRIT,
LOG_WARNING,
LOG_WARNING,
LOG_NOTICE,
LOG_INFO,
LOG_DEBUG
};

static bool syslog_open = false;
void logging_syslog_open( const char *name )
{
   if ( syslog_open == false ) {
      openlog( name, LOG_PID, LOG_USER );
      syslog_open = true;
   }
}
void syslog_print( int level, const char *fmt )
{
   syslog( syslog_levels[ level ], "%s", fmt );
}

