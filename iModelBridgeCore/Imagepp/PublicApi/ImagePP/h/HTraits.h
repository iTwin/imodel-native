//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/h/HTraits.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

BEGIN_IMAGEPP_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  This is a trait class that enables the addition of the const
*               keyword in front of the specified type when AddConst is
*               true;
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T, bool AddConst = true>
struct ConstTriggerTrait
    {
    typedef const T type;
    };
template <typename T>
struct ConstTriggerTrait<T, false>
    {
    typedef T type;
    };

/*---------------------------------------------------------------------------------**//**
* @description  This is a trait class that enables the addition of the const
*               keyword in front of the specified type.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
struct AddConst
    {
    typedef const T type;
    };


/*---------------------------------------------------------------------------------**//**
* @description  This is a trait class that enables removal of the const
*               keyword in front of the specified type.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
struct RemoveConst
    {
    typedef T type;
    };
template <typename T>
struct RemoveConst<const T>
    {
    typedef T type;
    };

/*---------------------------------------------------------------------------------**//**
* @description  This is a trait class that permits to validate whether a specified
*               type is const or not.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
struct IsConstTrait
    {
    enum {value = 0};
    };
template <typename T>
struct IsConstTrait<const T>
    {
    enum {value = 1};
    };

/*---------------------------------------------------------------------------------**//**
* @description  This is a trait class that permits to validate whether a specified
*               type is a ptr type or not
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
struct IsPointerTrait
    {
    enum {value = 0};
    };
template <typename T>
struct IsPointerTrait<T*>
    {
    enum {value = 1};
    };


/*---------------------------------------------------------------------------------**//**
* @description  This is a trait class that returns the same type as specified but whit
*               inverse constness.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
class ReverseConstTrait
    {
    template <typename ImplT, bool IsConst>
    struct _Impl
        {
        typedef typename RemoveConst<ImplT>::type type;
        };
    template <typename ImplT>
    struct _Impl<ImplT, false>
        {
        typedef typename AddConst<ImplT>::type type;
        };
public:
    typedef typename _Impl<T, IsConstTrait<T>::value>::type type;
    };

/*---------------------------------------------------------------------------------**//**
* @description  This is a trait class that returns same type as T but with a constness
*               that matches ReferenceType.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T, typename ReferenceType>
struct SameConstAsTrait
    {
    typedef typename ConstTriggerTrait< T, IsConstTrait<ReferenceType>::value >::type type;
    };

/*---------------------------------------------------------------------------------**//**
* @description  This is a trait class that returns whether Base is the base of Derived.
*               Will also return true when Derived is of same type as Base.
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename Base, typename Derived>
class IsBaseOfTrait
    {
    typedef int _TRUE_TYPE;
    typedef char _FASE_TYPE;

    static _TRUE_TYPE _TestBase (const Base*);

    static _FASE_TYPE _TestBase (...);

public:
    enum
        {
        value = (sizeof(_TestBase((const Derived*)0)) == sizeof(_TRUE_TYPE))
        };
    };


/*---------------------------------------------------------------------------------**//**
* @description  This is a type trait class that returns T when T is the base of U or
*               U when U is the base of T. In the case of T == U, T will be returned.
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T, typename U, bool IsTBase> struct GetBaseTrait_Impl {
    typedef T type;
};
template <typename T, typename U> struct GetBaseTrait_Impl<T, U, false>
{
    //HSTATICASSERT((typename IsBaseOfTrait<U, T>::value));
    typedef U type;
};

template <typename T, typename U>
class GetBaseTrait
    {
public:
    typedef typename GetBaseTrait_Impl<T, U, IsBaseOfTrait<T, U>::value>::type type;
    };


/*---------------------------------------------------------------------------------**//**
* @description  This is a type trait class that returns T when T is the derived from U or
*               U when U is the derived from T. In the case of T == U, U will be returned.
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T, typename U, bool IsTBase> struct GetDerivedTrait_Impl {
    typedef U type;
};
template <typename T, typename U> struct GetDerivedTrait_Impl<T, U, false>
{
    //HSTATICASSERT((typename IsBaseOfTrait<U, T>::value));
    typedef T type;
};

template <typename T, typename U>
class GetDerivedTrait
    {
public:
    typedef typename GetDerivedTrait_Impl<T, U, IsBaseOfTrait<T, U>::value>::type type;
    };



END_IMAGEPP_NAMESPACE