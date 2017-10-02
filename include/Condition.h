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

#ifndef INCLUDE_CONDITION_H_
#define INCLUDE_CONDITION_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/
#include <stdint.h>
#include <pthread.h>

/**============================================================================
 * Condition
 *
 *
 *============================================================================*/
class Mutex;
class Condition {

public:

   /**=========================================================================
    * @brief Constructor
    *
    * @param[in]
    *=========================================================================*/
   Condition();

   /**=========================================================================
    * @brief Destructor
    *
    *
    * @param[in]
    *=========================================================================*/
   virtual ~Condition();

   /**=========================================================================
    * @brief Block the calling thread, waiting for the condition to be signaled
    * or a specified timeout to occur.
    *
    * @note The mutex MUST be locked when this method is called,
    * and WILL be locked when this method returns, otherwise the results are
    * undefined (Deadlocks most likely)
    *
    * @param[out] &mutex   Wait for mutex.
    *
    * @return  void
    *=========================================================================*/
   void Wait( Mutex &mutex );

   /**=========================================================================
    * @brief Block the calling thread, waiting for the condition to be signaled
    * or a specified timeout to occur.
    *
    * @note The mutex MUST be locked when this method is called,
    * and WILL be locked when this method returns, otherwise the results are
    * undefined (Deadlocks most likely)
    *
    * @param[in]  timeoutMs   Wait for timeoutMs to occur.
    * @param[out] &mutex      Wait for mutex.
    *
    * @return  bool
    *=========================================================================*/
   bool Wait( Mutex &mutex, uint32_t timeoutMs );

private:

   /**=========================================================================
    * Condition variable
    *
    *=========================================================================*/
   pthread_cond_t mCond;
};

#endif /* ifndef INCLUDE_CONDITION_H_ */
