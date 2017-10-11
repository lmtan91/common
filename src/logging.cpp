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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <list>
#include "Thread.h"
#include <sys/syscall.h>

#include "logging.h"

SET_LOG_CAT( LOG_CAT_DEFAULT | LOG_CAT_TRACE );
SET_LOG_LEVEL( LOG_LVL_NOTICE );

#define LOGGING_MALLOC( size )	malloc( (size) )
#define LOGGING_FREE( ptr )		free( (ptr) )

int log_indent = 0;
int log_indent_size = 0;

static char *log_names_buffer = NULL;

static FILE *logging_file = NULL;

static bool use_syslog = false;
static FILE* logging_copy_file = NULL;
static bool logging_sync_mode = false;
static int logging_mark_num = 0;

static int logging_max_preamble = 0;

// This array MUST match in size and entry to the logging levels
// defined in logging.h
const char* log_preamble_syslog[] = { "LOG E: ", // LOG_LVL_ERR
         "LOG E: ",                             // LOG_LVL_ERR_PERROR
         "LOG W: ",                             // LOG_LVL_WARN
         "LOG W: ",                             // LOG_LVL_WARN_PERROR
         "LOG N: ",                             // LOG_LVL_NOTICE
         "LOG I: ",                             // LOG_LVL_INFO
         "LOG D: ",                 // LOG_LVL_NOISE (Preamble is D for clarity)
         NULL, };

// This array MUST match in size and entry to the logging levels
// defined in logging.h
const char* log_preamble_normal[] = { "\e[31mLOG E:\e[0m ",	// LOG_LVL_ERR
         "\e[31mLOG E:\e[0m ",	// LOG_LVL_ERR_PERROR
         "\e[35mLOG W:\e[0m ",	// LOG_LVL_WARN
         "\e[35mLOG W:\e[0m ",	// LOG_LVL_WARN_PERROR
         "LOG N: ",			// LOG_LVL_NOTICE
         "LOG I: ",			// LOG_LVL_INFO
         "LOG D: ",			// LOG_LVL_NOISE (Preamble is D for clarity)
         NULL };

const char* log_level_names[] = { "\e[31mERROR\e[0m, ", "\e[31mERROR\e[0m, ",
         "\e[35mWARNING\e[0m, ", "\e[35mWARNING\e[0m, ", "", "", "", };

struct file_data {
   const char *filename;
   uint32_t *cats;
   int *level;
};

static bool first_register = true;
static std::list<file_data>* file_data_list = NULL;

static const char *global_catagory_names[] = { "LOG_CAT_LOCAL1",
         "LOG_CAT_LOCAL2", "LOG_CAT_LOCAL3", "LOG_CAT_LOCAL4", "LOG_CAT_LOCAL5",
         "LOG_CAT_LOCAL6", "LOG_CAT_LOCAL7", "LOG_CAT_LOCAL8", "LOG_CAT_LOCAL9",
         "LOG_CAT_LOCAL10", "LOG_CAT_LOCAL11", "LOG_CAT_LOCAL12",
         "LOG_CAT_LOCAL13", "LOG_CAT_LOCAL14", "LOG_CAT_LOCAL15",
         "LOG_CAT_LOCAL16", "LOG_CAT_LOCAL17", "LOG_CAT_LOCAL18",
         "LOG_CAT_LOCAL19", "LOG_CAT_LOCAL20", "LOG_CAT_LOCAL21",
         "LOG_CAT_LOCAL22", "LOG_CAT_LOCAL23", "LOG_CAT_LOCAL24",
         "LOG_CAT_LOCAL25", "LOG_CAT_LOCAL26", "LOG_CAT_LOCAL27",
         "LOG_CAT_LOCAL28", "LOG_CAT_LOCAL29", "LOG_CAT_LOCAL30",
         "LOG_CAT_TRACE", "LOG_CAT_DEFAULT", };

static const char *global_level_names[] = { "LOG_LVL_ERR", "LOG_LVL_ERR",
         "LOG_LVL_WARN", "LOG_LVL_WARN", "LOG_LVL_NOTICE", "LOG_LVL_INFO",
         "LOG_LVL_NOISE", };

int logging_lookup_level( const char *str )
{
   int i;

   TRACE_BEGIN(LOG_LVL_INFO);

   LOG( "level is %s", str );

   // if a single digit number this is the level
   if ( str[ 0 ] >= '0' && str[ 0 ] <= '9' && str[ 1 ] == '\0' ) {
      const int ret = str[ 0 ] - '0';
      if ( ret < 0 || ret > LOG_LVL_NOISE )
         return -1;
      else
         return ret;
   }

   for (i = 0; i < ARRAY_SIZE( global_level_names ); i++) {
      if ( strcmp( global_level_names[ i ], str ) == 0 ) {
         LOG( "found level %d", i );TRACE_END();
         return i;
      }
   }

   TRACE_END();

   return -1;
}

uint32_t logging_lookup_cat( const char *str )
{
   int i;

   TRACE_BEGIN(LOG_LVL_INFO);

   LOG( "cat is %s", str );

   for (i = 0; i < ARRAY_SIZE( global_catagory_names ); i++) {
      if ( strcmp( global_catagory_names[ i ], str ) == 0 ) {
         LOG( "found cat %d", i );TRACE_END();
         return LOG_BIT_VALUE( i );
      }
   }

   TRACE_END();

   return 0;
}

const char* logging_get_names()
{
   uint32_t size = 1;
   char* temp;

   TRACE_BEGIN(LOG_LVL_INFO);

   if ( log_names_buffer != NULL )
      return log_names_buffer;

   // else figure out size, make buffer and copy data into it.

   LOG( "calculating storage size" );
   for (std::list<file_data>::iterator i = file_data_list->begin();
            i != file_data_list->end(); ++i) {
      size += strlen( i->filename ) + 1;
   }

   // if no file in list yet.
   if ( size == 1 )
      return NULL;

   log_names_buffer = (char*) LOGGING_MALLOC( size );
   temp = log_names_buffer;

   for (std::list<file_data>::iterator i = file_data_list->begin();
            i != file_data_list->end(); ++i) {
      strcpy( temp, i->filename );
      temp += strlen( i->filename );
      *temp = ' ';
      temp++;
   }

   temp--;
   *temp = '\0';

   return log_names_buffer;
}

static struct file_data *lookup_file( const char *filename )
{
   TRACE_BEGIN(LOG_LVL_INFO);

   LOG( "file is %s", filename );

   for (std::list<file_data>::iterator i = file_data_list->begin();
            i != file_data_list->end(); ++i) {
      if ( strcmp( i->filename, filename ) == 0 ) {
         TRACE_END();
         return &( *i );
      }
   }

   TRACE_END();

   return NULL;
}

int logging_set_cats( const char *filename, uint32_t cats )
{
   int all_files = ( strcmp( "all", filename ) == 0 );
   int result = 0;

   TRACE_BEGIN(LOG_LVL_INFO);LOG( "changing logging properties on file %s\n", filename );

   for (std::list<file_data>::iterator i = file_data_list->begin();
            i != file_data_list->end(); ++i) {
      if ( all_files == 1 || strcmp( i->filename, filename ) == 0 ) {
         *( i->cats ) = cats;
         result = 1;
      }
   }

   // Seems like everything came out OK
   TRACE_END();
   return result;
}

int logging_set_level( const char *filename, int level )
{
   int all_files = ( strcmp( "all", filename ) == 0 );
   int result = 0;

   TRACE_BEGIN(LOG_LVL_INFO);LOG( "changing logging properties on file %s\n", filename );

   for (std::list<file_data>::iterator i = file_data_list->begin();
            i != file_data_list->end(); ++i) {
      if ( all_files == 1 || strcmp( i->filename, filename ) == 0 ) {
         *( i->level ) = level;
         result = 1;
      }
   }

   // Seems like everything came out OK
   TRACE_END();
   return result;
}

uint32_t logging_get_cats( const char *filename )
{
   struct file_data *node;
   TRACE_BEGIN(LOG_LVL_INFO);LOG( "looking up category on file %s", filename );

   node = lookup_file( filename );

   if ( node == NULL )
      return 0;

   TRACE_END();
   return *node->cats;
}

int logging_get_level( const char *filename )
{
   struct file_data *node;
   TRACE_BEGIN(LOG_LVL_INFO);LOG( "looking up level on file %s", filename );

   node = lookup_file( filename );

   if ( node == NULL )
      return -1;

   TRACE_END();
   return *node->level;
}

void register_logging( const char *filename, uint32_t *cats, int *level )
{
   struct file_data node;

   TRACE_BEGIN(LOG_LVL_INFO);

   if ( first_register )
      logging_init();

   node.filename = filename;
   node.cats = cats;
   node.level = level;

   bool found = false;
   for (std::list<file_data>::iterator i = file_data_list->begin();
            i != file_data_list->end(); ++i) {
      if ( strcasecmp( i->filename, filename ) > 0 ) {
         // insert before current
         file_data_list->insert( i, node );
         found = true;
         break;
      }
   }

   if ( not found ) {
      file_data_list->push_back( node );
   }

   // if we have already built the name buffer then delete it since we just
   //  changed it.
   if ( log_names_buffer != NULL ) {
      LOGGING_FREE( log_names_buffer );
      log_names_buffer = NULL;
   }

   TRACE_END();
}

void logging_init()
{
   TRACE_BEGIN(LOG_LVL_INFO);

   if ( first_register == false )
      return;
   else
      first_register = false;

   file_data_list = new std::list<file_data>();

   logging_file = stdout;

   // Calculate maximum logging preamble length
   unsigned i = 0;
   while ( log_preamble_syslog[ i ] != NULL ) {
      int len = strlen( log_preamble_syslog[ i ] );
      if ( logging_max_preamble < len ) {
         logging_max_preamble = len;
      }
      ++i;
   }
   i = 0;
   while ( log_preamble_normal[ i ] != NULL ) {
      int len = strlen( log_preamble_normal[ i ] );
      if ( logging_max_preamble < len ) {
         logging_max_preamble = len;
      }
      ++i;
   }

   TRACE_END();
}

void logging_cleanup()
{
   TRACE_BEGIN(LOG_LVL_INFO);

   file_data_list->clear();

   TRACE_END();
}

// this is defined in common/src/logging_syslog.c
extern "C" {
void syslog_print( int level, const char *fmt );
}
;

// This is the old method, left for compatibility with the MW.  This function
//  enables syslog and disables normal output.
void set_logging_syslog( bool value )
{
   logging_set_syslog( value );
   logging_set_file( NULL );
}

void logging_set_syslog( bool value )
{
   use_syslog = value;
}

void logging_set_file( FILE *new_file )
{
   logging_file = new_file;
}

void logging_set_copy_file( FILE* copy_file )
{
   logging_copy_file = copy_file;
}

void logging_set_sync_mode( bool mode )
{
   logging_sync_mode = mode;
}

void logging_sync()
{
   if ( logging_file != NULL )
      fflush( logging_file );
   if ( logging_copy_file != NULL )
      fflush( logging_copy_file );
}

void logging_mark( int num )
{
   if ( num >= 0 )
      logging_mark_num = num;

   fprintf( logging_file, "====================\n" );
   fprintf( logging_file, "====================\n" );
   fprintf( logging_file, "==== MARK %05d ====\n", logging_mark_num++ );
   fprintf( logging_file, "====================\n" );
   fprintf( logging_file, "====================\n" );
}

void logging_show_files( FILE *file )
{
   int j;
   int first;

   TRACE_BEGIN(LOG_LVL_INFO);

   for (std::list<file_data>::iterator i = file_data_list->begin();
            i != file_data_list->end(); ++i) {
      //LOG_NOISE( "node %p", node );
      fprintf( file, "%s:%s:", i->filename, global_level_names[ *i->level ] );
      first = 1;

      if ( *i->cats == 0xFFFFFFFF ) {
         fprintf( file, "LOG_CAT_ALL" );
      } else {
         for (j = 0; j < 32; j++) {
            if ( *i->cats & LOG_BIT_VALUE( j ) ) {
               if ( first ) {
                  fprintf( file, "%s", global_catagory_names[ j ] );
                  first = 0;
               } else {
                  fprintf( file, ", %s", global_catagory_names[ j ] );
               }
            }
         }
      }

      fprintf( file, "\n" );
   }

   TRACE_END();
}

int function_name_fixup( const char *pretty_name, char *name, int len )
{
   const char *end = pretty_name + strlen( pretty_name ) - 1;
   const char *pos = end;
   int i = 0;

   // Keep track of the number of <'s and >'s we've encountered.
   int numTemplates = 0;

   while ( pos >= pretty_name ) {
      if ( *pos == ')' )
         numTemplates++;
      else if ( *pos == '(' )
         numTemplates--;

      if ( numTemplates == 0 )
         break;

      pos--;
   }

   end = pos;

   // if parse error return orig string
   if ( pos < pretty_name ) {
      strncpy( name, pretty_name, len );
      name[ len - 1 ] = 0;
      return strlen( name );
   }

   // Read backward until we get to the beginning of the string, but
   // don't bail in the middle of a <> pair (from c++ templates).
   while ( pos >= pretty_name ) {
      if ( *pos == '>' )
         numTemplates++;
      else if ( *pos == '<' )
         numTemplates--;
      else if ( ( isspace( *pos ) && numTemplates == 0 ) )
         break;

      pos--;
   }

   // Now print the string between pos and end, but skip any part that is
   //  of a template.
   pos++;
   numTemplates = 0;
   while ( i < ( len - 1 ) && pos < end ) {
      if ( *pos == '>' )
         numTemplates--;

      if ( numTemplates == 0 )
         name[ i++ ] = *pos;

      if ( *pos == '<' )
         numTemplates++;

      pos++;
   }

   name[ i ] = 0;
   return i;
}

// the last to arguments in the var arg list will be the file and line number
void log_print( int level, const char *function, const char *file, int line,
         const char *fmt, ... )
{
   int len = 0;
   char scratch_buf[ LOG_OUTPUT_BUF_SIZE ];
   char output_buf[ LOG_OUTPUT_BUF_SIZE ];
   va_list params;

   va_start( params, fmt );

   // Start by fixing up the function name in the scratch buffer
   function_name_fixup( function, scratch_buf, LOG_OUTPUT_BUF_SIZE );

   // Now we are going to assemble the output message, leaving a hole
   // at the front of the output_buf large enough to hold the largest
   // preamble message size (syslog or not)
   len = logging_max_preamble;

   // Now setup "[ThreadName:ThreadId]FunctionName:"
   len += snprintf( output_buf + len, LOG_OUTPUT_BUF_SIZE - len, "[%s:%ld]%s:",
            GetThreadName(), syscall( SYS_gettid ),
            /* libc does not provide a wrapper for gettid, so it must
             * be called directly.  This should eventually be
             * abstracted out with perhaps a GetThreadId() in
             * Thread.cpp.
             */
            scratch_buf );

   // Now format the message appropriately.
   if ( len < LOG_OUTPUT_BUF_SIZE ) {
      len += vsnprintf( output_buf + len, LOG_OUTPUT_BUF_SIZE - len, fmt,
               params );
   }

   // Add errno message (if needed)
   if ( errno && len < LOG_OUTPUT_BUF_SIZE
            && ( level == LOG_LVL_ERR_PERROR || level == LOG_LVL_WARN_PERROR ) ) {
      // Reuse the scratch buffer for strerror_r when we add in strerror
      // information
      len += snprintf( output_buf + len, LOG_OUTPUT_BUF_SIZE - len, ": \"%s\"",
               strerror_r( errno, scratch_buf, LOG_OUTPUT_BUF_SIZE ) );
   }

   // Add file and line number
   if ( len < LOG_OUTPUT_BUF_SIZE ) {
      len += snprintf( output_buf + len, LOG_OUTPUT_BUF_SIZE - len, " %s:%d",
               file, line );
   }

   // Add syslog preamble to the buffer
   if ( use_syslog ) {
      // Calculate the preamble length for the current level
      int preamble_len = strlen( log_preamble_syslog[ level ] );

      // Calculate the message start as max preamble - current preamble
      char* msg_start = output_buf + logging_max_preamble - preamble_len;

      // Don't allow us to run off the buffer
      if ( msg_start < output_buf )
         msg_start = output_buf;

      memcpy( msg_start, log_preamble_syslog[ level ], preamble_len );
      syslog_print( level, msg_start );
   }

   // Now add the newline that we need for non-syslog logging
   if ( len < LOG_OUTPUT_BUF_SIZE ) {
      len += snprintf( output_buf + len, LOG_OUTPUT_BUF_SIZE - len, "\n" );
   }

   // If we were truncated make sure we insert a newline at the end of
   // the buffer, and make sure to null terminate
   if ( len >= LOG_OUTPUT_BUF_SIZE ) {
      output_buf[ LOG_OUTPUT_BUF_SIZE - 2 ] = '\n';
      output_buf[ LOG_OUTPUT_BUF_SIZE - 1 ] = '\0';
   }

   // Calculate the preamble length for the current level
   int preamble_len = strlen( log_preamble_normal[ level ] );

   // Calculate the message start as max preamble - current preamble
   char* msg_start = output_buf + logging_max_preamble - preamble_len;

   // Don't allow us to run off the buffer
   if ( msg_start < output_buf )
      msg_start = output_buf;

   memcpy( msg_start, log_preamble_normal[ level ], preamble_len );

   // copy log to the copy file, if set
   if ( logging_copy_file != NULL ) {
      fprintf( logging_copy_file, "%s", msg_start );
      if ( logging_sync_mode )
         fflush( logging_copy_file );
   }

   // if not using syslog, log to the primary file
   if ( logging_file != NULL ) {
      fprintf( logging_file, "%s", msg_start );
      if ( logging_sync_mode )
         fflush( logging_file );
   }

   va_end( params );
}

void k_inc( char *file, int line, char *function )
{
}
void k_dec( char *file, int line, char *function )
{
}

void null_print( int level, const char *fmt, ... )
{
   // dummy function to keep from getting warnings about params not
   // being used.
}

#define GENERIC_PRINT( fmt, args... )        \
do {                                         \
   if ( logging_file != NULL )               \
      fprintf( logging_file, fmt, ## args ); \
} while( 0 )

void print_buffer( const char *str, const void *data, int len )
{
#ifdef VERBOSE_LOGGING
   const uint8_t *buffer = (uint8_t*)data;
   int i, buffer_i = 0;

   while ( len > 0 )
   {
      int cnt = len;
      GENERIC_PRINT( "%s: ", str );

      if ( cnt > 8 )
      cnt = 8;

      for ( i = 0; i < cnt; i++ )
      {
         GENERIC_PRINT( "0x%02X ", buffer[ buffer_i++ ] );
      }

      GENERIC_PRINT( "\n" );
      len -= cnt;
   }

   if ( logging_file != NULL && logging_sync_mode )
   fflush( logging_file );

#endif//VERBOSE_LOGGING
}

void print_buffer2( const char *str, const void *data, int len )
{
#ifdef VERBOSE_LOGGING
   const uint8_t *buffer = (uint8_t*)data;
   int i, buffer_i = 0;
   char line[ 100 ];
   int reset_offset = 0;

   int line_offset = strlen( str );

   if ( line_offset > 9 )
   line_offset = 9;

   memcpy( line, str, line_offset );

   line_offset += sprintf( line + line_offset, ": " );

   reset_offset = line_offset;

   while ( len > 0 )
   {
      int cnt = len;

      if ( cnt > 16 )
      cnt = 16;

      for ( i = 0; i < 16; i++ )
      {
         if ( i == 8 )
         line_offset += sprintf( line + line_offset, " " );

         if ( i < cnt )
         line_offset += sprintf( line + line_offset, "%02X ", buffer[ buffer_i++ ] );
         else
         line_offset += sprintf( line + line_offset, "   " );
      }

      line_offset += sprintf( line + line_offset, ": " );

      buffer_i -= cnt;

      for ( i = 0; i < cnt; i++ )
      {
         if ( i == 8 )
         line_offset += sprintf( line + line_offset, " " );

         if ( buffer[ buffer_i ] >= 32 && buffer[ buffer_i ] < 127 )
         {
            line_offset += sprintf( line + line_offset, "%c", buffer[ buffer_i ] );
            // if this is a % char then print a second on so that it is escaped.
            if ( buffer[ buffer_i ] == '%' )
            line_offset += sprintf( line + line_offset, "%c", buffer[ buffer_i ] );
         }
         else
         line_offset += sprintf( line + line_offset, "." );

         buffer_i++;
      }

      line_offset += sprintf( line + line_offset, "\n" );

      len -= cnt;

      GENERIC_PRINT( "%s", line );
      line_offset = reset_offset;
   }

   if ( logging_file != NULL && logging_sync_mode )
   fflush( logging_file );

#endif//VERBOSE_LOGGING
}

