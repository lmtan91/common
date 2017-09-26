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

#ifndef INCLUDE_MUTEX_H_
#define INCLUDE_MUTEX_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/
#include <pthread.h>

/**============================================================================
 * Class description
 *
 *
 *============================================================================*/
class Mutex
{

public:

   /**=========================================================================
    * @brief Constructor
    *
    * @param[in]
    *=========================================================================*/
   Mutex();

   /**=========================================================================
    * @brief Destructor
    *
    *
    * @param[in]
    *=========================================================================*/
   virtual ~Mutex();

   /**=========================================================================
    * @brief Method description
    *
    *
    * @param[in]
    * @param[out]
    *=========================================================================*/
private:

   // The name of this lock
   const char *mName;
   /**=========================================================================
    * Is this a recirsive lock? That is, can it be held repeatedly by the same
    * thread, or will grabbing it twice by any thread cause an error?
    *=========================================================================*/
   bool mRecursive;

   /**=========================================================================
    * current lock count for recursive lock. This is used in order to keep
    * track og mLockBy accurately
    *=========================================================================*/
   int mLockCount;

   /**=========================================================================
    * if tracing, this is supposed to be the name of file
    *
    *=========================================================================*/
   const char *mLockFile;

   /**=========================================================================
    * if tracing, this is supposed to be the line number in file
    *
    *=========================================================================*/
   int mLockLine;
   static pthread_mutex_t mCriticalSection;
   static bool mInited;
};

#endif /* ifndef INCLUDE_MUTEX_H_ */
