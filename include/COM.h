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

#ifndef INCLUDE_COM_H_
#define INCLUDE_COM_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/


#define COM_DEFINE_IID( name ) static COM::IID getIID() { return COM::IID( name ); }
#define COM_DEFINE_CID( name ) static COM::CID getCID() { return COM::CID( name ); }
#define COM_GET_IID( _class ) _class::getIID()
#define COM_GET_CID( _class ) _class::getCID()

#define COM_INIT_ISUPPORTS  \
   mRefCnt = 0

namespace COM
{
class ComId {
public:
   ComId( const char *com_id ) :
            mId( com_id )
   {
   }

   bool operator==( const ComId &other ) const;

   const char *toString() const
   {
      return mId;
   }

private:
   const char *mId;
};

typedef ComId CID;
typedef ComId IID;

enum ErrorCode {
   kNoError, kNoInterface, kNoClass, kNoFactory, kLoadFailed
};

class ISupports {
public:
   virtual int AddRef() = 0;
   virtual int Release() = 0;
   virtual ErrorCode QueryInterface( IID iid, void **interface ) = 0;COM_DEFINE_IID( "ISupports" );
protected:
   virtual ~ISupports() {}
};

class IFactory: public ISupports {
public:
   virtual ErrorCode CreateInstance( CID cid, IID iid, void **object ) = 0;COM_DEFINE_IID( "IFactory" );

protected:
   virtual ~IFactory() {}
};

class IComponentManager: public ISupports {
public:
   virtual ErrorCode LoadLibrary( const char *name ) = 0;

   virtual ErrorCode CreateInstance( CID cid, IID iid, void **object ) = 0;
   virtual ErrorCode GetService( CID cid, IID iid, void **object ) = 0;

   virtual ErrorCode AddService( CID cid, ISupports *service ) = 0;
   virtual ErrorCode RemoveService( CID cid ) = 0;

   COM_DEFINE_IID( "IComponentManager" );

protected:
   virtual ~IComponentManager() {}
};

/**
 * Called by anyone to get the pointer to the componentManager.
 */
IComponentManager *getComponentManager();

/**
 * should be calle by your main application just before exiting.
 */
void destroyComponentManager();
}
;

extern "C" COM::ErrorCode COM_RegisterServices( COM::IComponentManager *mgr );

/**
 * Declare the reference count variable and the implementations of the
 * AddRef and QueryInterface methods.
 */

#define COM_DECL_ISUPPORTS                                         \
public:                                                              \
   COM::ErrorCode QueryInterface( COM::IID iid, void** object ); \
   int AddRef();                                                     \
   int Release();                                                    \
protected:                                               \
   int mRefCnt;                                          \
public:

/**
 * Use this macro to implement the AddRef method for a given <i>_class</i>
 * @param _class The name of the class implementing the method
 */

#define COM_IMPL_ADDREF(_class)                                             \
int _class::AddRef()                                                    \
{                                                                    \
   return atomic_increment_val( &mRefCnt );                                   \
}

/**
 * Use this macro to implement the AddRef method for a given <i>_class</i>
 * @param _class The name of the class implementing the method
 */
#define COM_IMPL_ADDREF_DEBUG(_class)                                          \
int _class::AddRef()                                                    \
{                                                                    \
   int result = atomic_increment_val( &mRefCnt );                                \
   printf( "Addref %d->%d in %s\n", result-1, result, #_class );                    \
   return result;                                                       \
}

/**
 * Use this macro to implement the AddRef method for a given <i>_class</i>
 * @param _class The name of the class implementing the method
 */
#define COM_IMPL_ADDREF_TEMPLATE(_class,_params)                               \
template<_params>                                                       \
int _class::AddRef()                                                    \
{                                                                    \
   return atomic_increment_val( &mRefCnt );                                   \
}

#define COM_DELETE( obj ) delete obj

/**
 * Use this macro to implement the Release method for a given
 * <i>_class</i>.
 * @param _class The name of the class implementing the method
 */
#define COM_IMPL_RELEASE_WITH_DESTROY(_class, _destroy)                           \
int _class::Release()                                                      \
{                                                                    \
   int result = atomic_decrement_val( &mRefCnt );                                \
   if( result == 0 )                                                    \
      _destroy;                                                         \
   return result;                                                       \
}

/**
 * Use this macro to implement the Release method for a given
 * <i>_class</i>.
 * @param _class The name of the class implementing the method
 */
#define COM_IMPL_RELEASE_DEBUG(_class)                                      \
int _class::Release()                                                      \
{                                                                    \
   int result = atomic_decrement_val( &mRefCnt );                                \
   if( result == 0 )                                                    \
      _destroy;                                                         \
   printf( "Release %d->%d in %s\n", result+1, result, #_class );                   \
   return result;                                                       \
}

/**
 * Use this macro to implement the Release method for a given <i>_class</i>
 * @param _class The name of the class implementing the method
 *
 * A note on the 'stabilization' of the refcnt to one. At that point,
 * the object's refcount will have gone to zero. The object's
 * destructor may trigger code that attempts to QueryInterface() and
 * Release() 'this' again. Doing so will temporarily increment and
 * decrement the refcount. (Only a logic error would make one try to
 * keep a permanent hold on 'this'.)  To prevent re-entering the
 * destructor, we make sure that no balanced refcounting can return
 * the refcount to |0|.
 */
#define COM_IMPL_RELEASE(_class) \
  COM_IMPL_RELEASE_WITH_DESTROY(_class, COM_DELETE(this))

/**
 * Use this macro to implement the Release method for a given
 * <i>_class</i>.
 * @param _class The name of the class implementing the method
 */
#define COM_IMPL_RELEASE_TEMPLATE(_class, _param)                                 \
template<_param>                                                        \
int _class::Release()                                                      \
{                                                                    \
   int result = atomic_decrement_val( &mRefCnt );                                \
   if( result == 0 )                                                    \
      COM_DELETE(this);                                                  \
    return result;                                                         \
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Some convenience macros for implementing QueryInterface
 */

/**
 * This implements query interface with two assumptions: First, the
 * class in question implements ISupports and its own interface and
 * nothing else. Second, the implementation of the class's primary
 * inheritance chain leads to its own interface.
 *
 * @param _class The name of the class implementing the method
 * @param _classiiddef The name of the #define symbol that defines the IID
 * for the class (e.g. COM_ISUPPORTS_IID)
 */

#define COM_IMPL_QUERY_HEAD(_class)                                            \
COM::ErrorCode _class::QueryInterface( COM::IID iid, void** object )      \
{                                                                             \
  COM::ISupports* foundInterface;

#define COM_IMPL_QUERY_HEAD_TEMPLATE(_class,_param)                                            \
template<_param> \
COM::ErrorCode _class::QueryInterface( COM::IID iid, void** object )      \
{                                                                             \
  COM::ISupports* foundInterface;

#define COM_IMPL_QUERY_BODY(_interface)                                        \
  if ( iid == _interface::getIID() )                                  \
    foundInterface = static_cast<_interface*>(this);                       \
  else

#define COM_IMPL_QUERY_BODY_CONDITIONAL(_interface, condition)                 \
  if ( (condition) && iid == _interface::getIID() ))                    \
    foundInterface = static_cast<_interface*>(this);                       \
  else

#define COM_IMPL_QUERY_BODY_AMBIGUOUS(_interface, _implClass)                  \
  if ( iid == _interface::getIID() )                                  \
    foundInterface = static_cast<_interface*>( static_cast<_implClass*> (this));       \
  else

#define COM_IMPL_QUERY_TAIL_GUTS                                               \
    foundInterface = NULL;                                                       \
  COM::ErrorCode status;                                                            \
  if ( !foundInterface )                                                      \
    status = COM::kNoInterface;                                                  \
  else                                                                        \
    {                                                                         \
      foundInterface->AddRef();                                              \
      status = COM::kNoError;                                                         \
    }                                                                         \
  *object = foundInterface;                                             \
  return status;                                                              \
}

#define COM_IMPL_QUERY_TAIL_INHERITING(_baseclass)                             \
    foundInterface = 0;                                                       \
  COM::ErrorCode status;                                                            \
  if ( !foundInterface )                                                      \
    status = _baseclass::QueryInterface(aIID, (void**)&foundInterface);       \
  else                                                                        \
  {                                                                         \
    foundInterface->AddRef();                                               \
    status = COM::kNoError;                                                         \
  }                                                                         \
  *object = foundInterface;                                             \
  return status;                                                              \
}

#define COM_IMPL_QUERY_TAIL(_supports_interface)                               \
  COM_IMPL_QUERY_BODY_AMBIGUOUS(ISupports, _supports_interface)              \
  COM_IMPL_QUERY_TAIL_GUTS

/*
 This is the new scheme.  Using this notation now will allow us to switch to
 a table driven mechanism when it's ready.  Note the difference between this
 and the (currently) underlying COM_IMPL_QUERY_INTERFACE mechanism.  You must
 explicitly mention |ISupports| when using the interface maps.
 */
#define COM_INTERFACE_MAP_BEGIN(_implClass)      COM_IMPL_QUERY_HEAD(_implClass)
#define COM_INTERFACE_MAP_BEGIN_TEMPLATE(_implClass,_template)      COM_IMPL_QUERY_HEAD_TEMPLATE(_implClass, _template)
#define COM_INTERFACE_MAP_ENTRY(_interface)      COM_IMPL_QUERY_BODY(_interface)
#define COM_INTERFACE_MAP_ENTRY_CONDITIONAL(_interface, condition)             \
  COM_IMPL_QUERY_BODY_CONDITIONAL(_interface, condition)
#define COM_INTERFACE_MAP_ENTRY_AGGREGATED(_interface,_aggregate)              \
  COM_IMPL_QUERY_BODY_AGGREGATED(_interface,_aggregate)

#define COM_INTERFACE_MAP_END                    COM_IMPL_QUERY_TAIL_GUTS
#define COM_INTERFACE_MAP_ENTRY_AMBIGUOUS(_interface, _implClass)              \
  COM_IMPL_QUERY_BODY_AMBIGUOUS(_interface, _implClass)
#define COM_INTERFACE_MAP_END_INHERITING(_baseClass)                           \
  COM_IMPL_QUERY_TAIL_INHERITING(_baseClass)
#define COM_INTERFACE_MAP_END_AGGREGATED(_aggregator)                          \
  COM_IMPL_QUERY_TAIL_USING_AGGREGATOR(_aggregator)

#define COM_IMPL_QUERY_INTERFACE0(_class)                                      \
  COM_INTERFACE_MAP_BEGIN(_class)                                              \
    COM_INTERFACE_MAP_ENTRY(COM::ISupports)                                       \
  COM_INTERFACE_MAP_END

#define COM_IMPL_QUERY_INTERFACE1(_class, _i1)                                 \
  COM_INTERFACE_MAP_BEGIN(_class)                                              \
    COM_INTERFACE_MAP_ENTRY(_i1)                                               \
    COM_INTERFACE_MAP_ENTRY_AMBIGUOUS(COM::ISupports, _i1)                        \
  COM_INTERFACE_MAP_END

#define COM_IMPL_QUERY_INTERFACE2(_class, _i1, _i2)                            \
  COM_INTERFACE_MAP_BEGIN(_class)                                              \
    COM_INTERFACE_MAP_ENTRY(_i1)                                               \
    COM_INTERFACE_MAP_ENTRY(_i2)                                               \
    COM_INTERFACE_MAP_ENTRY_AMBIGUOUS(COM::ISupports, _i1)                        \
  COM_INTERFACE_MAP_END

#define COM_IMPL_QUERY_INTERFACE3(_class, _i1, _i2, _i3)                       \
  COM_INTERFACE_MAP_BEGIN(_class)                                              \
    COM_INTERFACE_MAP_ENTRY(_i1)                                               \
    COM_INTERFACE_MAP_ENTRY(_i2)                                               \
    COM_INTERFACE_MAP_ENTRY(_i3)                                               \
    COM_INTERFACE_MAP_ENTRY_AMBIGUOUS(COM::ISupports, _i1)                        \
  COM_INTERFACE_MAP_END

#define COM_IMPL_QUERY_INTERFACE4(_class, _i1, _i2, _i3, _i4)                  \
  COM_INTERFACE_MAP_BEGIN(_class)                                              \
    COM_INTERFACE_MAP_ENTRY(_i1)                                               \
    COM_INTERFACE_MAP_ENTRY(_i2)                                               \
    COM_INTERFACE_MAP_ENTRY(_i3)                                               \
    COM_INTERFACE_MAP_ENTRY(_i4)                                               \
    COM_INTERFACE_MAP_ENTRY_AMBIGUOUS(COM::ISupports, _i1)                        \
  COM_INTERFACE_MAP_END

#define COM_IMPL_QUERY_INTERFACE5(_class, _i1, _i2, _i3, _i4, _i5)             \
  COM_INTERFACE_MAP_BEGIN(_class)                                              \
    COM_INTERFACE_MAP_ENTRY(_i1)                                               \
    COM_INTERFACE_MAP_ENTRY(_i2)                                               \
    COM_INTERFACE_MAP_ENTRY(_i3)                                               \
    COM_INTERFACE_MAP_ENTRY(_i4)                                               \
    COM_INTERFACE_MAP_ENTRY(_i5)                                               \
    COM_INTERFACE_MAP_ENTRY_AMBIGUOUS(COM::ISupports, _i1)                        \
  COM_INTERFACE_MAP_END

#define COM_IMPL_QUERY_INTERFACE6(_class, _i1, _i2, _i3, _i4, _i5, _i6)        \
  COM_INTERFACE_MAP_BEGIN(_class)                                              \
    COM_INTERFACE_MAP_ENTRY(_i1)                                               \
    COM_INTERFACE_MAP_ENTRY(_i2)                                               \
    COM_INTERFACE_MAP_ENTRY(_i3)                                               \
    COM_INTERFACE_MAP_ENTRY(_i4)                                               \
    COM_INTERFACE_MAP_ENTRY(_i5)                                               \
    COM_INTERFACE_MAP_ENTRY(_i6)                                               \
    COM_INTERFACE_MAP_ENTRY_AMBIGUOUS(COM::ISupports, _i1)                        \
  COM_INTERFACE_MAP_END

#define COM_IMPL_QUERY_INTERFACE7(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7)   \
  COM_INTERFACE_MAP_BEGIN(_class)                                              \
    COM_INTERFACE_MAP_ENTRY(_i1)                                               \
    COM_INTERFACE_MAP_ENTRY(_i2)                                               \
    COM_INTERFACE_MAP_ENTRY(_i3)                                               \
    COM_INTERFACE_MAP_ENTRY(_i4)                                               \
    COM_INTERFACE_MAP_ENTRY(_i5)                                               \
    COM_INTERFACE_MAP_ENTRY(_i6)                                               \
    COM_INTERFACE_MAP_ENTRY(_i7)                                               \
    COM_INTERFACE_MAP_ENTRY_AMBIGUOUS(COM::ISupports, _i1)                        \
  COM_INTERFACE_MAP_END

#define COM_IMPL_QUERY_INTERFACE8(_class, _i1, _i2, _i3, _i4, _i5, _i6,        \
                                 _i7, _i8)                                    \
  COM_INTERFACE_MAP_BEGIN(_class)                                              \
    COM_INTERFACE_MAP_ENTRY(_i1)                                               \
    COM_INTERFACE_MAP_ENTRY(_i2)                                               \
    COM_INTERFACE_MAP_ENTRY(_i3)                                               \
    COM_INTERFACE_MAP_ENTRY(_i4)                                               \
    COM_INTERFACE_MAP_ENTRY(_i5)                                               \
    COM_INTERFACE_MAP_ENTRY(_i6)                                               \
    COM_INTERFACE_MAP_ENTRY(_i7)                                               \
    COM_INTERFACE_MAP_ENTRY(_i8)                                               \
    COM_INTERFACE_MAP_ENTRY_AMBIGUOUS(COM::ISupports, _i1)                        \
  COM_INTERFACE_MAP_END

#define COM_IMPL_QUERY_INTERFACE9(_class, _i1, _i2, _i3, _i4, _i5, _i6,        \
                                 _i7, _i8, _i9)                               \
  COM_INTERFACE_MAP_BEGIN(_class)                                              \
    COM_INTERFACE_MAP_ENTRY(_i1)                                               \
    COM_INTERFACE_MAP_ENTRY(_i2)                                               \
    COM_INTERFACE_MAP_ENTRY(_i3)                                               \
    COM_INTERFACE_MAP_ENTRY(_i4)                                               \
    COM_INTERFACE_MAP_ENTRY(_i5)                                               \
    COM_INTERFACE_MAP_ENTRY(_i6)                                               \
    COM_INTERFACE_MAP_ENTRY(_i7)                                               \
    COM_INTERFACE_MAP_ENTRY(_i8)                                               \
    COM_INTERFACE_MAP_ENTRY(_i9)                                               \
    COM_INTERFACE_MAP_ENTRY_AMBIGUOUS(COM::ISupports, _i1)                        \
  COM_INTERFACE_MAP_END

#define COM_IMPL_QUERY_INTERFACE10(_class, _i1, _i2, _i3, _i4, _i5, _i6,       \
                                  _i7, _i8, _i9, _i10)                        \
  COM_INTERFACE_MAP_BEGIN(_class)                                              \
    COM_INTERFACE_MAP_ENTRY(_i1)                                               \
    COM_INTERFACE_MAP_ENTRY(_i2)                                               \
    COM_INTERFACE_MAP_ENTRY(_i3)                                               \
    COM_INTERFACE_MAP_ENTRY(_i4)                                               \
    COM_INTERFACE_MAP_ENTRY(_i5)                                               \
    COM_INTERFACE_MAP_ENTRY(_i6)                                               \
    COM_INTERFACE_MAP_ENTRY(_i7)                                               \
    COM_INTERFACE_MAP_ENTRY(_i8)                                               \
    COM_INTERFACE_MAP_ENTRY(_i9)                                               \
    COM_INTERFACE_MAP_ENTRY(_i10)                                              \
    COM_INTERFACE_MAP_ENTRY_AMBIGUOUS(COM::ISupports, _i1)                        \
  COM_INTERFACE_MAP_END

#define COM_IMPL_QUERY_INTERFACE11(_class, _i1, _i2, _i3, _i4, _i5, _i6,       \
                                  _i7, _i8, _i9, _i10, _i11)                  \
  COM_INTERFACE_MAP_BEGIN(_class)                                              \
    COM_INTERFACE_MAP_ENTRY(_i1)                                               \
    COM_INTERFACE_MAP_ENTRY(_i2)                                               \
    COM_INTERFACE_MAP_ENTRY(_i3)                                               \
    COM_INTERFACE_MAP_ENTRY(_i4)                                               \
    COM_INTERFACE_MAP_ENTRY(_i5)                                               \
    COM_INTERFACE_MAP_ENTRY(_i6)                                               \
    COM_INTERFACE_MAP_ENTRY(_i7)                                               \
    COM_INTERFACE_MAP_ENTRY(_i8)                                               \
    COM_INTERFACE_MAP_ENTRY(_i9)                                               \
    COM_INTERFACE_MAP_ENTRY(_i10)                                              \
    COM_INTERFACE_MAP_ENTRY(_i11)                                              \
    COM_INTERFACE_MAP_ENTRY_AMBIGUOUS(COM::ISupports, _i1)                        \
  COM_INTERFACE_MAP_END

/**
 * Declare that you're going to inherit from something that already
 * implements ISupports, but also implements an additional interface, thus
 * causing an ambiguity. In this case you don't need another mRefCnt, you
 * just need to forward the definitions to the appropriate superclass. E.g.
 *
 * class Bar : public Foo, public nsIBar {  // both provide ISupports
 * public:
 *   COM_DECL_ISUPPORTS_INHERITED
 *   ...other nsIBar and Bar methods...
 * };
 */
#define COM_DECL_ISUPPORTS_INHERITED                                           \
public:                                                                       \
  COM::ErrorCode QueryInterface(REFNSIID aIID,                                    \
                            void** aInstancePtr);                             \
  int AddRef(void);                                         \
  int Release(void);                                        \

/**
 * These macros can be used in conjunction with COM_DECL_ISUPPORTS_INHERITED
 * to implement the ISupports methods, forwarding the invocations to a
 * superclass that already implements ISupports.
 *
 * Note that I didn't make these inlined because they're virtual methods.
 */

#define COM_IMPL_ADDREF_INHERITED(Class, Super)                                \
int Class::AddRef(void)                                  \
{                                                                             \
  return Super::AddRef();                                                     \
}                                                                             \

#define COM_IMPL_RELEASE_INHERITED(Class, Super)                               \
int Class::Release(void)                                 \
{                                                                             \
  return Super::Release();                                                    \
}                                                                             \

#define COM_IMPL_QUERY_INTERFACE_INHERITED0(Class, Super)                      \
  COM_IMPL_QUERY_HEAD(Class)                                                   \
  COM_IMPL_QUERY_TAIL_INHERITING(Super)                                        \

#define COM_IMPL_QUERY_INTERFACE_INHERITED1(Class, Super, i1)                  \
  COM_IMPL_QUERY_HEAD(Class)                                                   \
  COM_IMPL_QUERY_BODY(i1)                                                      \
  COM_IMPL_QUERY_TAIL_INHERITING(Super)                                        \

#define COM_IMPL_QUERY_INTERFACE_INHERITED2(Class, Super, i1, i2)              \
  COM_IMPL_QUERY_HEAD(Class)                                                   \
  COM_IMPL_QUERY_BODY(i1)                                                      \
  COM_IMPL_QUERY_BODY(i2)                                                      \
  COM_IMPL_QUERY_TAIL_INHERITING(Super)                                        \

#define COM_IMPL_QUERY_INTERFACE_INHERITED3(Class, Super, i1, i2, i3)          \
  COM_IMPL_QUERY_HEAD(Class)                                                   \
  COM_IMPL_QUERY_BODY(i1)                                                      \
  COM_IMPL_QUERY_BODY(i2)                                                      \
  COM_IMPL_QUERY_BODY(i3)                                                      \
  COM_IMPL_QUERY_TAIL_INHERITING(Super)                                        \

#define COM_IMPL_QUERY_INTERFACE_INHERITED4(Class, Super, i1, i2, i3, i4)      \
  COM_IMPL_QUERY_HEAD(Class)                                                   \
  COM_IMPL_QUERY_BODY(i1)                                                      \
  COM_IMPL_QUERY_BODY(i2)                                                      \
  COM_IMPL_QUERY_BODY(i3)                                                      \
  COM_IMPL_QUERY_BODY(i4)                                                      \
  COM_IMPL_QUERY_TAIL_INHERITING(Super)                                        \

#define COM_IMPL_QUERY_INTERFACE_INHERITED5(Class,Super,i1,i2,i3,i4,i5)        \
  COM_IMPL_QUERY_HEAD(Class)                                                   \
  COM_IMPL_QUERY_BODY(i1)                                                      \
  COM_IMPL_QUERY_BODY(i2)                                                      \
  COM_IMPL_QUERY_BODY(i3)                                                      \
  COM_IMPL_QUERY_BODY(i4)                                                      \
  COM_IMPL_QUERY_BODY(i5)                                                      \
  COM_IMPL_QUERY_TAIL_INHERITING(Super)                                        \

#define COM_IMPL_QUERY_INTERFACE_INHERITED6(Class,Super,i1,i2,i3,i4,i5,i6)     \
  COM_IMPL_QUERY_HEAD(Class)                                                   \
  COM_IMPL_QUERY_BODY(i1)                                                      \
  COM_IMPL_QUERY_BODY(i2)                                                      \
  COM_IMPL_QUERY_BODY(i3)                                                      \
  COM_IMPL_QUERY_BODY(i4)                                                      \
  COM_IMPL_QUERY_BODY(i5)                                                      \
  COM_IMPL_QUERY_BODY(i6)                                                      \
  COM_IMPL_QUERY_TAIL_INHERITING(Super)                                        \

#define COM_IMPL_QUERY_INTERFACE_TEMPLATE1(_class, _param, _i1)                                 \
  COM_INTERFACE_MAP_BEGIN_TEMPLATE(_class,_param)                                              \
    COM_INTERFACE_MAP_ENTRY(_i1)                                               \
    COM_INTERFACE_MAP_ENTRY_AMBIGUOUS(COM::ISupports, _i1)                        \
  COM_INTERFACE_MAP_END

/**
 * Convenience macros for implementing all ISupports methods for
 * a simple class.
 * @param _class The name of the class implementing the method
 * @param _classiiddef The name of the #define symbol that defines the IID
 * for the class (e.g. COM_ISUPPORTS_IID)
 */

#define COM_IMPL_ISUPPORTS0_DEBUG(_class)                                            \
  COM_IMPL_ADDREF_DEBUG(_class)                                                      \
  COM_IMPL_RELEASE_DEBUG(_class)                                                     \
  COM_IMPL_QUERY_INTERFACE0(_class)

#define COM_IMPL_ISUPPORTS0(_class)                                            \
  COM_IMPL_ADDREF(_class)                                                      \
  COM_IMPL_RELEASE(_class)                                                     \
  COM_IMPL_QUERY_INTERFACE0(_class)

#define COM_IMPL_ISUPPORTS1_DEBUG(_class, _interface)                                \
  COM_IMPL_ADDREF_DEBUG(_class)                                                      \
  COM_IMPL_RELEASE_DEBUG(_class)                                                     \
  COM_IMPL_QUERY_INTERFACE1(_class, _interface)

#define COM_IMPL_ISUPPORTS1(_class, _interface)                                \
  COM_IMPL_ADDREF(_class)                                                      \
  COM_IMPL_RELEASE(_class)                                                     \
  COM_IMPL_QUERY_INTERFACE1(_class, _interface)

#define COM_IMPL_ISUPPORTS2_DEBUG(_class, _i1, _i2)                                  \
  COM_IMPL_ADDREF_DEBUG(_class)                                                      \
  COM_IMPL_RELEASE_DEBUG(_class)                                                     \
  COM_IMPL_QUERY_INTERFACE2(_class, _i1, _i2)

#define COM_IMPL_ISUPPORTS2(_class, _i1, _i2)                                  \
  COM_IMPL_ADDREF(_class)                                                      \
  COM_IMPL_RELEASE(_class)                                                     \
  COM_IMPL_QUERY_INTERFACE2(_class, _i1, _i2)

#define COM_IMPL_ISUPPORTS3(_class, _i1, _i2, _i3)                             \
  COM_IMPL_ADDREF(_class)                                                      \
  COM_IMPL_RELEASE(_class)                                                     \
  COM_IMPL_QUERY_INTERFACE3(_class, _i1, _i2, _i3)

#define COM_IMPL_ISUPPORTS4(_class, _i1, _i2, _i3, _i4)                        \
  COM_IMPL_ADDREF(_class)                                                      \
  COM_IMPL_RELEASE(_class)                                                     \
  COM_IMPL_QUERY_INTERFACE4(_class, _i1, _i2, _i3, _i4)

#define COM_IMPL_ISUPPORTS5(_class, _i1, _i2, _i3, _i4, _i5)                   \
  COM_IMPL_ADDREF(_class)                                                      \
  COM_IMPL_RELEASE(_class)                                                     \
  COM_IMPL_QUERY_INTERFACE5(_class, _i1, _i2, _i3, _i4, _i5)

#define COM_IMPL_ISUPPORTS6(_class, _i1, _i2, _i3, _i4, _i5, _i6)              \
  COM_IMPL_ADDREF(_class)                                                      \
  COM_IMPL_RELEASE(_class)                                                     \
  COM_IMPL_QUERY_INTERFACE6(_class, _i1, _i2, _i3, _i4, _i5, _i6)

#define COM_IMPL_ISUPPORTS7(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7)         \
  COM_IMPL_ADDREF(_class)                                                      \
  COM_IMPL_RELEASE(_class)                                                     \
  COM_IMPL_QUERY_INTERFACE7(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7)

#define COM_IMPL_ISUPPORTS8(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8)    \
  COM_IMPL_ADDREF(_class)                                                      \
  COM_IMPL_RELEASE(_class)                                                     \
  COM_IMPL_QUERY_INTERFACE8(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8)

#define COM_IMPL_ISUPPORTS9(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8,    \
                           _i9)                                               \
  COM_IMPL_ADDREF(_class)                                                      \
  COM_IMPL_RELEASE(_class)                                                     \
  COM_IMPL_QUERY_INTERFACE9(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8, _i9)

#define COM_IMPL_ISUPPORTS10(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8,   \
                            _i9, _i10)                                        \
  COM_IMPL_ADDREF(_class)                                                      \
  COM_IMPL_RELEASE(_class)                                                     \
  COM_IMPL_QUERY_INTERFACE10(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8,   \
                            _i9, _i10)

#define COM_IMPL_ISUPPORTS11(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8,   \
                            _i9, _i10, _i11)                                  \
  COM_IMPL_ADDREF(_class)                                                      \
  COM_IMPL_RELEASE(_class)                                                     \
  COM_IMPL_QUERY_INTERFACE11(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8,   \
                            _i9, _i10, _i11)

#define COM_IMPL_ISUPPORTS_INHERITED0(Class, Super)                            \
    COM_IMPL_QUERY_INTERFACE_INHERITED0(Class, Super)                          \
    COM_IMPL_ADDREF_INHERITED(Class, Super)                                    \
    COM_IMPL_RELEASE_INHERITED(Class, Super)                                   \

#define COM_IMPL_ISUPPORTS_INHERITED1(Class, Super, i1)                        \
    COM_IMPL_QUERY_INTERFACE_INHERITED1(Class, Super, i1)                      \
    COM_IMPL_ADDREF_INHERITED(Class, Super)                                    \
    COM_IMPL_RELEASE_INHERITED(Class, Super)                                   \

#define COM_IMPL_ISUPPORTS_INHERITED2(Class, Super, i1, i2)                    \
    COM_IMPL_QUERY_INTERFACE_INHERITED2(Class, Super, i1, i2)                  \
    COM_IMPL_ADDREF_INHERITED(Class, Super)                                    \
    COM_IMPL_RELEASE_INHERITED(Class, Super)                                   \

#define COM_IMPL_ISUPPORTS_INHERITED3(Class, Super, i1, i2, i3)                \
    COM_IMPL_QUERY_INTERFACE_INHERITED3(Class, Super, i1, i2, i3)              \
    COM_IMPL_ADDREF_INHERITED(Class, Super)                                    \
    COM_IMPL_RELEASE_INHERITED(Class, Super)                                   \

#define COM_IMPL_ISUPPORTS_INHERITED4(Class, Super, i1, i2, i3, i4)            \
    COM_IMPL_QUERY_INTERFACE_INHERITED4(Class, Super, i1, i2, i3, i4)          \
    COM_IMPL_ADDREF_INHERITED(Class, Super)                                    \
    COM_IMPL_RELEASE_INHERITED(Class, Super)                                   \

#define COM_IMPL_ISUPPORTS_INHERITED5(Class, Super, i1, i2, i3, i4, i5)        \
    COM_IMPL_QUERY_INTERFACE_INHERITED5(Class, Super, i1, i2, i3, i4, i5)      \
    COM_IMPL_ADDREF_INHERITED(Class, Super)                                    \
    COM_IMPL_RELEASE_INHERITED(Class, Super)                                   \

#define COM_IMPL_ISUPPORTS_INHERITED6(Class, Super, i1, i2, i3, i4, i5, i6)    \
    COM_IMPL_QUERY_INTERFACE_INHERITED6(Class, Super, i1, i2, i3, i4, i5, i6)  \
    COM_IMPL_ADDREF_INHERITED(Class, Super)                                    \
    COM_IMPL_RELEASE_INHERITED(Class, Super)                                   \

/**
 * This is for when you have a templated class implementing a COM interface.
 *  This will create template friendly version of QuertInterface, AddRef and
 *  Release.
 *
 * Usage is:
 *  COM_IMPL_QUERY_INTERFACE_TEMPLATE1( Foo<T>, class T, IFoo )
 */

#define COM_IMPL_ISUPPORTS_TEMPLATE1(Class, Template, i1)                        \
    COM_IMPL_QUERY_INTERFACE_TEMPLATE1(Class, Template, i1)                      \
    COM_IMPL_ADDREF_TEMPLATE(Class, Template)                                    \
    COM_IMPL_RELEASE_TEMPLATE(Class, Template)                                   \



#endif /* ifndef INCLUDE_COM_H_ */
