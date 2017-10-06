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
 * conditionTest.cpp
 *
 *  Created on: Oct 6, 2017
 *      Author: lia1hc
 */

#include "Timer.h"
#include "Mutex.h"
#include "Condition.h"
#include <iostream>

using namespace std;
int main( int argc, char* argv[] )
{
   Condition c;
   Mutex m;
   AutoLock l( m );

   cout << "hehe=" << c.Wait( m, 10000 ) << endl;
   return 0;
}
