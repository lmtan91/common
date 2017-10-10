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

/*
 * listTest.cpp
 *
 *  Created on: Oct 7, 2017
 *      Author: lmtan91
 */

#include <list>
#include <iostream>

using namespace std;

list<int> listTest;
void show() {
   for (list<int>::iterator i = listTest.begin(); i != listTest.end(); ++i) {
      cout << *i << " ";
   }
   cout << endl;
}
int main( int argc, char* argv[] ) {

   listTest.push_back( 17 );
   listTest.push_back( 12 );
   listTest.push_back( 1991 );

   show();

   for (list<int>::iterator i = listTest.begin(); i != listTest.end(); ++i) {
      if ( *i == 12 ) {
         listTest.insert( i, 13 );
      }
   }
   show();

   listTest.push_back( 13 );
   listTest.push_back( 13 );
   show();

   list<int>::iterator i = listTest.begin();
   while (i != listTest.end()) {
      if ( 13 == *i ) {
         i = listTest.erase( i );
         cout << "i=" << *i << endl;
         continue;
      }
      ++i;
   }
   show();
}
