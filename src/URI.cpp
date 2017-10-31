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

#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>

#include "URI.h"
#include <vector>

#include "logging.h"
#include <sstream>

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOTICE );

using namespace std;

string gNullString;

URI::URI() :
      mRelative( false ), mModified( false ), mModifiedQuery( false ) {
}


URI::URI( const char *uri ) :
      mRelative( false ), mModified( false ), mModifiedQuery( false ) {
   mFullString = uri;
   parseString();
}

URI::URI( string &uri ) :
      mRelative( false ), mModified( false ), mModifiedQuery( false ) {
   mFullString = uri;
   parseString();
}

URI::URI( const URI &other ) :
      mRelative( false ), mModified( false ), mModifiedQuery( false ) {
   copyValues( other );
}

URI::~URI() {
}

const URI &URI::operator=( const char *uri ) {
   clear();
   mFullString = uri;
   parseString();
   return *this;
}

bool URI::operator ==( const URI& other ) const {
   // Flush changes in both URI objects
   other.buildString();
   buildString();

   // Now compare the full string
   return mFullString == other.mFullString;
}

bool URI::operator !=( const URI& other ) const {
   return !( *this == other );
}

const URI &URI::operator=( const URI &other ) {
   copyValues( other );
   return *this;
}

void URI::copyValues( const URI &other ) {
   // Flush modifications in source URI
   other.buildString();

   // Now copy all of the values from the source
   mRelative = other.mRelative;

   // Source was just flushed, so we are starting with no modifications
   mModified = false;
   mModifiedQuery = false;

   mFullString = other.mFullString;
   mQueryString = other.mQueryString;
   mScheme = other.mScheme;
   mAuthority = other.mAuthority;
   mPath = other.mPath;
   mFragment = other.mFragment;
   mQueryParams = other.mQueryParams;
}

void URI::clear() {
   mRelative = false;
   mModified = false;
   mModifiedQuery = false;
   mFullString.clear();
   mQueryString.clear();
   mScheme.clear();
   mAuthority.clear();
   mPath.clear();
   mFragment.clear();
   mQueryParams.clear();
}

const string &URI::getScheme() const {
   return mScheme;
}

void URI::setScheme( const string &scheme ) {
   mModified = true;
   mScheme = scheme;
}

const string &URI::getAuthority() const {
   return mAuthority;
}

void URI::setAuthority( const std::string &authority ) {
   mModified = true;
   mAuthority = authority;
}

string URI::getHost() const {
   string::size_type offset = mAuthority.find( ":" );

   return mAuthority.substr( 0, offset );
}

int getSchemePort( const string &str ) {
   struct servent *ent = getservbyname( str.c_str(), "tcp" );

   if ( ent != NULL )
      return htons( ent->s_port );
   else
      return 0;
}

int URI::getPort() const {
   string::size_type offset = mAuthority.find( ":" );

   if ( offset == string::npos )
      return getSchemePort( getScheme() );
   else {
      int port_num = 0;
      string port = mAuthority.substr( offset + 1 );
      stringstream strm( port );
      strm >> port_num;
      return port_num;
   }
}

const string &URI::getPath() const {
   return mPath;
}

void URI::setPath( const std::string &path ) {
   mModified = true;
   if ( path[0] != '/' ) {
      mPath = "/";
      mPath.append( path );
   } else {
      mPath = path;
   }
}

const string &URI::getFragment() const {
   return mFragment;
}

void URI::setFragment( const std::string &fragment ) {
   mModified = true;
   mFragment = fragment;
}

const string &URI::getQuery() const {
   buildQuery();
   return mQueryString;
}

void URI::setQuery( const string &query ) {
   mQueryString = query;
   parseQuery();
}

void URI::appendQueryParam( const string &key, const string &value ) {
   QueryParam parms( key, value );
   mQueryParams.push_back( parms );
   mModifiedQuery = true;
   mModified = true;
}

void URI::removeQueryParam( const string &key ) {
   int i = findParam( key );

   if ( i != -1 ) {
//      mQueryParams.erase( i );
      mQueryParams.erase( mQueryParams.begin() + i );
   }
   mModifiedQuery = true;
   mModified = true;
}


const string &URI::getQueryParam( const string &key ) const {
   int i = findParam( key );

   if ( i != -1 ) {
      return mQueryParams[i].mParam;
   } else {
      return gNullString;
   }
}

int URI::findParam( const string &key ) const {
   for (unsigned i = 0; i < mQueryParams.size(); i++) {
      if ( mQueryParams[i].mKey == key ) {
         return i;
      }
   }

   return -1;
}

// copied from extremedb
unsigned char mcohs_hexC2V( unsigned char c ) {
   if ( c >= '0' && c <= '9' )
      return c - '0';
   if ( c >= 'A' && c <= 'F' )
      return c - 'A' + 10;
   if ( c >= 'a' && c <= 'f' )
      return c - 'a' + 10;
   return 0;
}

// copied from extremedb
unsigned int mcohs_unescape( char* str, unsigned int len ) {

   unsigned int rsz = 0;
   char * p = str, *v = str;

   while (len > 0) {
      if ( *p == '+' ) {
         *v = ' ';
      } else if ( *p == '%' ) {
         *v = mcohs_hexC2V( p[1] ) * 0x10 + mcohs_hexC2V( p[2] );
         p += 2;
         len -= 2;
      } else {
         *v = *p;
      };

      p++;
      v++;
      rsz++;
      len--;
   };
   return rsz;
}

// convenience function to unescape string
void unescapeString( std::string& str ) {
   char* cStr = const_cast<char*>( str.c_str() );
   unsigned int len = str.size();
   unsigned int new_len = mcohs_unescape( cStr, len );
   for (unsigned int i = new_len; i < len; i++) {
      cStr[i] = 0;
   }
   str = cStr;
}

void URI::unescapeQueryParams() {
   for (unsigned i = 0; i < mQueryParams.size(); i++) {
      unescapeString( mQueryParams[i].mParam );
   }
}

const std::string &URI::getString() const {
   buildString();
   return mFullString;
}

bool URI::setString( const char *uri ) {
   clear();
   mFullString = uri;
   return parseString();
}

bool URI::setString( std::string &uri ) {
   clear();
   mFullString = uri;
   return parseString();
}

bool URI::parseString() {
   mModified = false;
   mRelative = false;

   string::size_type start = 0;
   string::size_type off = mFullString.find( ":" );

   if ( off != string::npos ) {
      mScheme = mFullString.substr( 0, off );
      LOG_NOISE( "scheme is %s", mScheme.c_str() );
      off++;
      start = off;
   }

   bool hasAuth = false;

   // if the next chars are // or if they are not a / and we have a scheme
   //  then parse auth, otherwise if no / is found and we don't have a schem 
   //  this is a relative URI. 
   if ( mFullString.find( "//", start ) == start ) {
      hasAuth = true;
      start += 2;
   } else if ( mFullString.find( "/", start ) != start ) {
      if ( mScheme.empty() ) {
         mRelative = true;
         hasAuth = false;
      } else {
         hasAuth = true;
      }
   }

   if ( hasAuth ) {
      off = mFullString.find( "/", start );

      mAuthority = mFullString.substr( start, off - start );
      LOG_NOISE( "auth is %s", mAuthority.c_str() );

      start = off;
   }

   if ( start < mFullString.size() ) {
      off = mFullString.find( "?", start );

      mPath = mFullString.substr( start, off - start );

      LOG_NOISE( "path is %s", mPath.c_str() );

      if ( off != string::npos ) {
         start = off + 1;
         off = mFullString.find( "#", start );

         mQueryString = mFullString.substr( start, off - start );
         LOG_NOISE( "query is %s", mQueryString.c_str() );

         if ( parseQuery() == false )
            return false;

         if ( off != string::npos ) {
            mFragment = mFullString.substr( off + 1 );
            LOG_NOISE( "fragment is %s", mFragment.c_str() );
         }
      }
   }

   return true;
}

void URI::buildString() const {
   if ( mModified == false )
      return;

   stringstream strm( stringstream::in | stringstream::out );

   strm << mScheme;
   strm << ":";

   if ( not mAuthority.empty() ) {
      strm << "//";
      strm << mAuthority;
   }

   strm << mPath;

   buildQuery();

   if ( not mQueryString.empty() ) {
      strm << "?";
      strm << mQueryString;
   }

   if ( not mFragment.empty() ) {
      strm << "#";
      strm << mFragment;
   }

   mFullString = strm.str();
   mModified = false;
}

bool URI::parseQuery() {
   mModifiedQuery = false;
   QueryParam parms;

   if ( mQueryString.empty() ) {
      mQueryParams.clear();
      return true;
   }

   string::size_type start = 0;
   string::size_type off = 0;

   while (off != string::npos) {
      off = mQueryString.find( "=", start );

      if ( off == string::npos )
         return false;

      parms.mKey = mQueryString.substr( start, off - start );

      start = off + 1;
      off = mQueryString.find( "&", start );

      if ( off == string::npos ) {
         parms.mParam = mQueryString.substr( start );
      } else {
         parms.mParam = mQueryString.substr( start, off - start );
         start = off + 1;
         off = mQueryString.find( "=", start );
      }

      mQueryParams.push_back( parms );
   }

   return true;
}

void URI::buildQuery() const {
   if ( mModifiedQuery == false )
      return;

   stringstream strm( stringstream::in | stringstream::out );

   for (unsigned i = 0; i < mQueryParams.size(); i++) {
      if ( i != 0 )
         strm << "&";

      strm << mQueryParams[i].mKey;
      strm << "=";
      strm << mQueryParams[i].mParam;
   }

   mQueryString = strm.str();
   mModifiedQuery = false;
}

std::string URI::getPathAndQuery() const {
   if ( mQueryString.empty() )
      return mPath;
   std::string ret = mPath + "?" + mQueryString;
   return ret;
}
