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

#ifndef INCLUDE_REFCOUNT_H_
#define INCLUDE_REFCOUNT_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/
#include "Mutex.h"

enum ErrorCode {
   kNoError, kNoInterface, kNoClass, kNoFactory, kLoadFailed
};

/**============================================================================
 * Class description
 *
 *
 *============================================================================*/
class RefCount {

public:

   /**=========================================================================
    * @brief Constructor
    *
    *=========================================================================*/
   RefCount() :
            mRefCount( 0 )
   {
   }

   /**=========================================================================
    * @brief Increase reference counting
    *
    *=========================================================================*/
   void AddRef() const
   {
      Mutex::EnterCriticalSection();
      mRefCount++;
      Mutex::ExitCriticalSection();
   }

   /**=========================================================================
    * @brief Increase reference counting
    *
    *=========================================================================*/
   void Release() const
   {
      Mutex::EnterCriticalSection();
      mRefCount--;
      if ( 0 == mRefCount ) {
         Mutex::ExitCriticalSection();
         onRefCountZero();
      }
      else {
         Mutex::ExitCriticalSection();
      }
   }

protected:

   /**=========================================================================
    * @brief Destroy the object when count reaches 0
    *
    *
    *=========================================================================*/
   virtual void onRefCountZero() const
   {
      delete this;
   }

   /**=========================================================================
    * @brief Destructor
    *
    *
    *=========================================================================*/
   virtual ~RefCount()
   {
   }

   /**=========================================================================
    * @brief Method description
    *
    *
    * @param[in]
    * @param[out]
    *
    * @return
    * @retval
    *=========================================================================*/
private:

   /**=========================================================================
    * Description of data
    *
    *=========================================================================*/
   mutable int mRefCount;
};

#endif /* ifndef INCLUDE_REFCOUNT_H_ */
