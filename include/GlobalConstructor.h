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

#ifndef INCLUDE_GLOBALCONSTRUCTOR_H_
#define INCLUDE_GLOBALCONSTRUCTOR_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/

/**============================================================================
 * This helper class to make initializing code at start. A class with some
 * static data that need to be initialized at startup
 *
 *============================================================================*/
class GlobalConstructor
{

public:

   /**=========================================================================
    * @brief Functor as function pointer to assign init function at startup
    *
    *=========================================================================*/
   typedef void (*GlobalConstructorFunc)();

   /**=========================================================================
    * @brief Constructor
    *
    * @param[in] init      init function at startup
    *=========================================================================*/
   GlobalConstructor( GlobalConstructorFunc init ) :
         mFunc( NULL )
   {
      init();
   }

   /**=========================================================================
    * @brief Constructor
    *
    * @param[in] init      init function at startup
    *=========================================================================*/
   GlobalConstructor( GlobalConstructorFunc init,
         GlobalConstructorFunc destroy ) :
         mFunc( destroy )
   {
      init();
   }

   /**=========================================================================
    * @brief Destructor
    *
    *
    *=========================================================================*/
   virtual ~GlobalConstructor()
   {
      if (mFunc != NULL) {
         mFunc();
      }
   }

private:

   /**=========================================================================
    * This function called by destructor
    *
    *=========================================================================*/
   GlobalConstructorFunc mFunc;
};

#define GLOBAL_CONSTRUCT( construct ) static GlobalConstructor gcFunc( construct )
#define GLOBAL_CONSTRUCT_DESTRUCT( construct, destruct ) static GlobalConstructor gcFunc( construct, destruct )
#endif /* ifndef INCLUDE_GLOBALCONSTRUCTOR_H_ */
