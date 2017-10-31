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

#ifndef INCLUDE_URI_H_
#define INCLUDE_URI_H_

#include <string>
#include <vector>

/**
 Class to parse and generate URI's.  The names of methods follow the naming
 used in RFC 2396 for "generic URIs".   In brief this is
 scheme://authority/path?query#fragment.  The query part of the URI follow
 the format of param1=value&param2=value, with as many params as needed.
 */
class URI {
public:
   URI();
   URI( const char *uri );
   URI( std::string &uri );
   URI( const URI &other );
   ~URI();

   const URI &operator=( const char *uri );
   const URI &operator=( const URI &other );

   bool operator ==( const URI &other ) const;
   bool operator !=( const URI &other ) const;

   const std::string &getScheme() const;
   void setScheme( const std::string &scheme );

   const std::string &getAuthority() const;
   void setAuthority( const std::string &authority );
   std::string getHost() const;
   int getPort() const;

   const std::string &getPath() const;
   void setPath( const std::string &path );

   std::string getPathAndQuery() const;

   const std::string &getQuery() const;
   void setQuery( const std::string &query );
   void appendQueryParam( const std::string &key, const std::string &value );
   void removeQueryParam( const std::string &key );
   const std::string &getQueryParam( const std::string &key ) const;

   const std::string &getFragment() const;
   void setFragment( const std::string &fragment );

   const std::string &getString() const;
   bool setString( const char *uri );
   bool setString( std::string &uri );

   bool isRelative() const {
      return mRelative;
   }

   void clear();
   void unescapeQueryParams();

private:
   bool parseString();
   void buildString() const;
   bool parseQuery();
   void buildQuery() const;

   void copyValues( const URI& other );

   struct QueryParam {
      QueryParam() {
      }

      QueryParam( const std::string& key, const std::string &value ) :
            mKey( key ), mParam( value ) {
      }

      QueryParam( const QueryParam& other ) :
            mKey( other.mKey ), mParam( other.mParam ) {
      }

      const QueryParam& operator=( const QueryParam& other ) {
         mKey = other.mKey;
         mParam = other.mParam;
         return *this;
      }

      std::string mKey;
      std::string mParam;
   };

   int findParam( const std::string &key ) const;

   bool mRelative;
   mutable bool mModified;
   mutable bool mModifiedQuery;
   mutable std::string mFullString;
   mutable std::string mQueryString;
   std::string mScheme;
   std::string mAuthority;
   std::string mPath;
   std::string mFragment;
   std::vector<QueryParam> mQueryParams;
};


#endif /* ifndef INCLUDE_URI_H_ */
