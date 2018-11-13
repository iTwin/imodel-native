/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Foundations/Definitions.h $
|    $RCSfile: Definitions.h,v $
|   $Revision: 1.6 $
|       $Date: 2011/12/20 16:23:45 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <Bentley/RefCounted.h>
#include <Bentley/Bentley.h>
#include <utility>
#ifdef LINUX_SCALABLEMESH_BUILD 
using namespace std;
#endif
// NTERAY: See if Bentley.h's forward declaration may suffice.
#include <Bentley/WString.h> 
#include <ImagePP/h/HmrMacro.h>
#include <ImagePP/h/HNumeric.h>

#ifdef FOUNDATIONS_DLLE
//    #error "Export name conflict with another definition of the same name"
#endif

#if _WIN32
#ifdef __BENTLEYSTM_BUILD__ //BENTLEY_SCALABLEMESH_FOUNDATIONS_EXPORTS
    #define FOUNDATIONS_DLLE __declspec(dllexport)
#else
    #define FOUNDATIONS_DLLE __declspec(dllimport)
#endif
#else
 #define FOUNDATIONS_DLLE
#endif



#ifndef BEGIN_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE
#define BEGIN_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE namespace BENTLEY_NAMESPACE_NAME { namespace ScalableMesh { namespace Foundations {
    #define END_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE   }}}
    #define USING_NAMESPACE_BENTLEY_SCALABLEMESH_FOUNDATIONS using namespace BENTLEY_NAMESPACE_NAME::ScalableMesh::Foundations;
#endif //!BEGIN_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE


#ifndef BEGIN_BENTLEY_SCALABLEMESH_FOUNDATIONS_ITERATOR_NAMESPACE
    #define BEGIN_BENTLEY_SCALABLEMESH_FOUNDATIONS_ITERATOR_NAMESPACE BEGIN_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE namespace Iterator {
    #define END_BENTLEY_SCALABLEMESH_FOUNDATIONS_ITERATOR_NAMESPACE   END_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE }
    #define USING_NAMESPACE_BENTLEY_SCALABLEMESH_FOUNDATIONS_ITERATOR using namespace BENTLEY_NAMESPACE_NAME::ScalableMesh::Foundations::Iterator;
#endif //!BEGIN_BENTLEY_SCALABLEMESH_FOUNDATIONS_ITERATOR_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  Validate a compile time condition.
*               The compiler will fail compiling the line of the static assertion 
*               if "static_expr" is false. Could be placed at any scope
*               (function/class/global).
*
*  e.g.: static_assert(sizeof(m_Data) == sizeof(m_Data2), "")
*          -> Will not compile if members differ in size
* TDORAY: Replace with standard C++0x static assert
* @bsimacro                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
/*#ifndef static_assert
#define _static_assertln_exp(expr, line) static const int static_assert_ln##line [(expr)? 1 : -1];
#define _static_assertln(expr, line) _static_assertln_exp((expr), line)
#define static_assert(expr, msg) _static_assertln((expr), __LINE__)
#endif //!static_assert*/


BEGIN_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE

// Make this namespace synonymous to Bentley. This has the same effect as if this namespace was part of Bentley.
using namespace BENTLEY_NAMESPACE_NAME;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
struct ShareableObjectTypeTrait 
    {
    class RefCountedBase : public BENTLEY_NAMESPACE_NAME::IRefCounted
        {
    private:
        mutable uint32_t  m_refCount;

    protected:
        virtual         ~RefCountedBase() {}         // force virtual destructor for all subclasses

    public:  // OPERATOR_NEW_KLUDGE  >>> BEIJING_WIP_STM look for OPERATOR_NEW_KLUDGE and fix them
        void*   operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
        void    operator delete(void* rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
        void*   operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
        void    operator delete [] (void* rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

        RefCountedBase() : m_refCount(0) {}

        bool            IsShared () const {return m_refCount > 1;}

        virtual uint32_t  AddRef   () const {return ++m_refCount;}
        virtual uint32_t  Release  () const
            {
            if (1 < m_refCount--)
                return  m_refCount;

            delete this;
            return  0;
            }
        };

    typedef RefCountedBase type;
    };

template <typename T>
struct SharedPtrTypeTrait {typedef RefCountedPtr<T> type;};
template <typename T>
struct SharedPtrTypeTrait<const T> {typedef RefCountedPtr<T> type;};



/*---------------------------------------------------------------------------------**//**
* @description  Utility class used to facilitate deactivation of  
*               assignment operator. Classes inheriting from HUncopyable become themselves
*               non-assignable by thwarting any compiler's attempt to generate default
*               version of assignment operator.
*    
* Example:
* class HTest : private Unassignable
* {
*  ...
* };
*
* NOTE: Private inheritance is preferable because we do not want that users
*       manipulate derived objects via polymorphism.
*
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class Unassignable
{
    protected:
        // Allow construction and destruction of derived classes
        Unassignable () {};
        ~Unassignable() {};

    private:
        // Prevent assignment of derived classes
        Unassignable& operator=(const Unassignable&);
};



/*---------------------------------------------------------------------------------**//**
* @description  Utility class used to facilitate deactivation of copy constructor and 
*               assignment operator. Classes inheriting from HUncopyable become themselves
*               non-copyable by thwarting any compiler's attempt to generate default
*               version of copy constructor / assignment operator.
*    
* Example:
* class HTest : private Uncopyable
* {
*  ...
* };
*
* NOTE: Private inheritance is preferable because we do not want that users
*       manipulate derived objects via polymorphism.
*
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class Uncopyable
{
    protected:
        // Allow construction and destruction of derived classes
        Uncopyable () {};
        ~Uncopyable() {};

    private:
        // Prevent copying derived classes
        Uncopyable(const Uncopyable&);
        Uncopyable& operator=(const Uncopyable&);
};



#ifndef LINUX_SCALABLEMESH_BUILD 
/*---------------------------------------------------------------------------------**//**
* @description  This is a trait class that returns whether a specified type is
*               plain old data (POD).
*    
* TDORAY: Replace with std::is_pod or std::tr1::is_pod when available.
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
struct is_pod
    {
    enum {value = (__is_pod(T)|| __is_enum(T) || __is_union(T))};
    };
template <typename T> struct is_pod<const T> {enum {value = is_pod<T>::value};};
template <> struct is_pod<char> {enum {value = 1};};
template <> struct is_pod<unsigned char> {enum {value = 1};};
template <> struct is_pod<short> {enum {value = 1};};
template <> struct is_pod<unsigned short> {enum {value = 1};};
template <> struct is_pod<int> {enum {value = 1};};
template <> struct is_pod<unsigned int> {enum {value = 1};};

#ifndef _WIN32
template <> struct is_pod<int64_t> {enum {value = 1};};
template <> struct is_pod<unsigned int64_t> {enum {value = 1};};
#endif
template <> struct is_pod<long> {enum {value = 1};};
template <> struct is_pod<unsigned long> {enum {value = 1};};
template <> struct is_pod<float> {enum {value = 1};};
template <> struct is_pod<double> {enum {value = 1};};
#endif

/*---------------------------------------------------------------------------------**//**
* @description  Help setting specified bits to a boolean value
* @bsimethod                                                Raymond.Gauthier   5/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename BitsetT, typename BitselectT>
void                                SetBitsTo                          (BitsetT&    bitset, 
                                                                        BitselectT  bitSelectionMask, 
                                                                        bool        value)
    { bitset = (value ? (bitset | bitSelectionMask) : (bitset & (~bitSelectionMask))); }

/*---------------------------------------------------------------------------------**//**
* @description  Help setting specified bits to On
* @bsimethod                                                Raymond.Gauthier   5/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename BitsetT, typename BitselectT>
void                                SetBitsOn                          (BitsetT&    bitset, 
                                                                        BitselectT  bitSelectionMask)
    { bitset |= bitSelectionMask; }

/*---------------------------------------------------------------------------------**//**
* @description  Help setting specified bits to Off
* @bsimethod                                                Raymond.Gauthier   5/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename BitsetT, typename BitselectT>
void                                SetBitsOff                         (BitsetT&    bitset, 
                                                                        BitselectT  bitSelectionMask)
    { bitset &= (~bitSelectionMask); }

/*---------------------------------------------------------------------------------**//**
* @description  Help checking whether specified bits are On
* @bsimethod                                                Raymond.Gauthier   5/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename BitsetT, typename BitselectT>
bool                                HasBitsOn                          (BitsetT     bitset, 
                                                                        BitselectT  bitSelectionMask)
    { return (0 != (bitset & bitSelectionMask)); }


END_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE
