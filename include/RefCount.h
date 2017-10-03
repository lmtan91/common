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
/* Standard */
#include <stdio.h>

#include "Alias.h"
#include "Mutex.h"
#include "COM.h"


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

class SmartPtrHelper {
public:
   virtual COM::ErrorCode operator()( const COM::IID&, void** ) const
   {
      return COM::kNoError;
   }
protected:
   virtual ~SmartPtrHelper()
   {
   }
};

template<class T>
class SmartPtr {
public:
   SmartPtr() :
            mObj( NULL )
   {
   }

   SmartPtr( T *obj ) :
            mObj( obj )
   {
      if ( mObj ) {
         mObj->AddRef();
      }
   }

   SmartPtr( const SmartPtr &ptr ) :
            mObj( ptr.mObj )
   {
      if ( mObj ) {
         mObj->AddRef();
      }
   }

   SmartPtr( const SmartPtrHelper &helper ) :
            mObj( NULL )
   {
      helper( COM_GET_IID( T ), Alias<void**, T**>::create( &mObj ) );
   }

   virtual ~SmartPtr()
   {
      if ( mObj ) {
         mObj->Release();
      }
   }

   SmartPtr &operator=( const SmartPtr &ptr )
   {
      // Use SmartPtr &operator=(T *obj) to handle the reference counting
      *this = ptr.mObj;
      return *this;
   }

   SmartPtr &operator=( const SmartPtrHelper &helper )
   {
#if 0
      void *objPtr = reinterpret_cast<void*>( &mObj );
      T *oldObj = reinterpret_cast<T*>( atomic_exchange_acq(
               reinterpret_cast<int*>( objPtr ), NULL ) );

      if ( oldObj != NULL )
         oldObj->Release();
#else
      if ( mObj != NULL )
      mObj->Release();
      mObj = NULL;
#endif
      helper( COM_GET_IID( T ), Alias<void**, T**>::create( &mObj ) );
      return *this;
   }

   SmartPtr &operator=( T *obj )
   {
      if ( obj != NULL ) {
         obj->AddRef();
      }
#if 0
      void *objPtr = reinterpret_cast<void*>( &mObj );
      T *oldObj = reinterpret_cast<T*>( atomic_exchange_acq(
               reinterpret_cast<int*>( objPtr ), (int) obj ) );

      if ( oldObj != NULL )
         oldObj->Release();
#else
      if ( mObj != NULL )
      mObj->Release();
      mObj = obj;
#endif
      return *this;
   }

   bool operator==( const T *obj ) const
   {
      printf( "SmartPtr::operator==( const T *obj ) Enter()\n" );
      return ( mObj == obj );
   }
   bool operator==( const SmartPtr &ptr ) const
   {
      printf( "SmartPtr::operator==( const SmartPtr &ptr ) Enter()\n" );
      return ( mObj == ptr.mObj );
   }
   bool operator!=( const T *obj ) const
   {
      printf( "SmartPtr::operator!=( const T *obj ) Enter()\n" );

      return ( mObj != obj );
   }
   bool operator!=( const SmartPtr &ptr ) const
   {
      printf( "SmartPtr::operator!=( const SmartPtr &ptr ) Enter()\n" );

      return ( mObj != ptr.mObj );
   }

   T* operator->()
   {
      printf( "SmartPtr::operator->() Enter()\n" );

      return mObj;
   }
   const T* operator->() const
   {
      printf( "SmartPtr::operator->() const\n" );

      return mObj;
   }

   operator T*()
   {
      printf( "SmartPtr::operator T*()\n" );

      return mObj;
   }

   operator const T*() const
   {
      printf( "SmartPtr::operator const T*() const\n" );

      return mObj;
   }

   const T* getObjectForDebug() const
   {
      printf( "SmartPtr::getObjectForDebug\n" );

      return mObj;
   }

private:
   T *mObj;
};
#endif /* ifndef INCLUDE_REFCOUNT_H_ */
