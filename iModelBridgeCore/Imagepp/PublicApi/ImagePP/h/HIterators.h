//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/h/HIterators.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HTraits.h"

#include <iterator>

BEGIN_IMAGEPP_NAMESPACE


template <class Category,
         class T,
         bool  IsScalar = false,
         class Diff = ptrdiff_t,
         class Pointer = T*,
         class Reference = T&,
         class ConstPointer = const T*,
         class ConstReference = const T&>
struct BaseIteratorPolicy
    {
    typedef Category        iterator_category;
    typedef T               value_type;
    typedef Diff            difference_type;
    typedef Pointer         pointer;
    typedef Reference       reference;
    typedef ConstPointer    const_pointer;
    typedef ConstReference  const_reference;

    static const bool   is_scalar = IsScalar;
    };


/*---------------------------------------------------------------------------------**//**
* @description  Base iterator class. May provide some algorithms optimizations (such
*               as using memcpy/memmove in algorithms) when descendants specify they
*               are scalar.
*               TDORAY: Provide optimizations
* @see std::iterator
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename IteratorType, typename Policy>
struct BaseIterator : public std::iterator<typename Policy::iterator_category,
        typename Policy::value_type,
        typename Policy::difference_type,
        typename Policy::pointer,
        typename Policy::reference>
    {
public:
    typedef typename Policy::const_reference
    const_reference;
    typedef typename Policy::const_pointer
    const_pointer;

    typedef IteratorType    iterator_t;

protected:

    iterator_t&             _GetIter               ()                                           {
        return static_cast<iterator_t&>(*this);
        }
    const iterator_t&       _GetIter               () const                                     {
        return static_cast<const iterator_t&>(*this);
        }

    const_reference         _Dereference           () const                                     {
        return _GetIter().Dereference();
        }
    typename Policy::reference _Dereference           ()                                        {
        return _GetIter().Dereference();
        }

    iterator_t&             _Increment             ()                                           {
        _GetIter().Increment();
        return _GetIter();
        }
    iterator_t&             _Decrement             ()                                           {
        _GetIter().Decrement();
        return _GetIter();
        }

    iterator_t&             _AdvanceOf             (typename Policy::difference_type         pi_pOffset)         {
        _GetIter().AdvanceOf(pi_pOffset);
        return _GetIter();
        }

    typename Policy::difference_type         _DistanceFrom          (const iterator_t&       pi_rRight) const    {
        return _GetIter().DistanceFrom(pi_rRight);
        }

    bool                    _EqualTo               (const iterator_t&       pi_rRight) const    {
        return _GetIter().EqualTo(pi_rRight);
        }
    bool                    _LessThan              (const iterator_t&       pi_rRight) const    {
        return _GetIter().LessThan(pi_rRight);
        }

    };


/*---------------------------------------------------------------------------------**//**
* @description  Base iterator class. May provide some algorithms optimizations (such
*               as using memcpy/memmove in algorithms) when descendants specify they
*               are scalar.
*               TDORAY: Provide optimizations
* @see std::iterator
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename IteratorType, typename ReverseConstIteratorType, bool IsConstIterator>
struct GetConstIteratorTypeTrait                        
{
    typedef IteratorType value;
};
template <typename IteratorType, typename ReverseConstIteratorType> struct GetConstIteratorTypeTrait<IteratorType, ReverseConstIteratorType, false>     
{
    typedef ReverseConstIteratorType value;
};

template<typename IteratorType, typename ReverseConstIteratorType, bool IsConstIterator>
struct GetNotConstIteratorTypeTrait                     
{
    typedef ReverseConstIteratorType value;
};
template <typename IteratorType, typename ReverseConstIteratorType> 
struct GetNotConstIteratorTypeTrait<IteratorType, ReverseConstIteratorType, false>  
{
    typedef IteratorType value;
};

template<typename IteratorType, typename ReverseConstIteratorType, typename Policy>
struct BaseIteratorWithAutoRerverseConst : public std::iterator<typename Policy::iterator_category,
                                                                typename Policy::value_type,
                                                                typename Policy::difference_type,
                                                                typename Policy::pointer,
                                                                typename Policy::reference>
    {
public:
    typedef typename Policy::const_reference
    const_reference;
    typedef typename Policy::const_pointer
    const_pointer;

    typedef IteratorType                iterator_t;
    typedef ReverseConstIteratorType    rconst_iterator_t;
    typedef typename GetConstIteratorTypeTrait<IteratorType, ReverseConstIteratorType,IsConstTrait<typename Policy::value_type>::value>::value  const_iterator_t;
    typedef typename GetNotConstIteratorTypeTrait<IteratorType, ReverseConstIteratorType,IsConstTrait<typename Policy::value_type>::value>::value nconst_iterator_t;

protected:
    explicit                BaseIteratorWithAutoRerverseConst
    ()
        {
        // Ensure that our const_iterator is really const
//        HSTATICASSERT(IsConstTrait<const_iterator_t::value_type>::value);

        // Ensure that our nconst_iterator is really not const
//        HSTATICASSERT(!IsConstTrait<nconst_iterator_t::value_type>::value);

        // Ensure that one of the iterator points to const data and the other does not
        HSTATICASSERT(IsConstTrait<typename iterator_t::value_type>::value != IsConstTrait<typename rconst_iterator_t::value_type>::value);
        }


    iterator_t&             _GetIter               ()                                               {
        return static_cast<iterator_t&>(*this);
        }
    const iterator_t&       _GetIter               () const                                         {
        return static_cast<const iterator_t&>(*this);
        }

    const_iterator_t        _ConvertToConst        () const                                         {
        return _GetIter().ConvertToConst ();
        }

    const_reference         _Dereference           () const                                         {
        return _GetIter().Dereference();
        }
    typename Policy::reference       _Dereference           ()                                               {
        return _GetIter().Dereference();
        }

    iterator_t&             _Increment             ()                                               {
        _GetIter().Increment();
        return _GetIter();
        }
    iterator_t&             _Decrement             ()                                               {
        _GetIter().Decrement();
        return _GetIter();
        }

    iterator_t&             _AdvanceOf             (typename Policy::difference_type           pi_pOffset)           {
        _GetIter().AdvanceOf(pi_pOffset);
        return _GetIter();
        }
    typename Policy::difference_type         _DistanceFrom          (const iterator_t&           pi_rRight) const    {
        return _GetIter().DistanceFrom(pi_rRight);
        }
    typename Policy::difference_type         _DistanceFrom          (const rconst_iterator_t&    pi_rRight) const    {
        return _GetIter().DistanceFrom(pi_rRight);
        }

    bool                    _EqualTo               (const iterator_t&           pi_rRight) const    {
        return _GetIter().EqualTo(pi_rRight);
        }
    bool                    _EqualTo               (const rconst_iterator_t&    pi_rRight) const    {
        return _GetIter().EqualTo(pi_rRight);
        }

    bool                    _LessThan              (const iterator_t&           pi_rRight) const    {
        return _GetIter().LessThan(pi_rRight);
        }
    bool                    _LessThan              (const rconst_iterator_t&    pi_rRight) const    {
        return _GetIter().LessThan(pi_rRight);
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description  This is a generic template to facilitate implementation of custom
*               forward iterators. Minimizes required methods to implement without
*               any performance cost (that virtual methods call would normally incur).
*
*               IteratorType:       The class that inherits from ForwardIterator.
*               T:                  The type that the iterator iterates on.
*
*
*   E.g. (for an iterator that iterates on "OType"):
*
*   class OType;
*
*   class Iter : public ForwardIterator<Iter, OType>
*   {
*   private: // OPTIONAL: In order to provide more encapsulation (so that required members are not accessible directly through public interface)
*       friend              super_class; // Make required method accessible to super class
*
*
*       // IteratorType requirement: Need to implement the following members:
*
*       const_reference     Dereference    () const;
*       reference           Dereference    ();
*       void                Increment      ();
*       bool                EqualTo        (const iterator_t&) const;
*
*       // IteratorType exemple (will certainly be more complex):
*       friend              SomeContainerClass;
*                           iterator_t     (value_type*);
*       value_type*         m_pObject;
*   };
*
*   typedef Iter            iterator
*
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename IteratorType, typename T, bool IsScalar = false,
         typename BasePolicy = BaseIteratorPolicy<std::forward_iterator_tag, T, IsScalar> >
class ForwardIterator : public BaseIterator<IteratorType, BasePolicy>
    {


public:
    typename BaseIterator<IteratorType, BasePolicy>::reference                   operator*              ()                                               {
        return BaseIterator<IteratorType, BasePolicy>::_Dereference();
        }
    typename BaseIterator<IteratorType, BasePolicy>::const_reference             operator*              () const                                         {
        return BaseIterator<IteratorType, BasePolicy>::_Dereference();
        }

    typename BaseIterator<IteratorType, BasePolicy>::const_pointer               operator->             () const                                         {
        return (&*BaseIterator<IteratorType, BasePolicy>::_GetIter());
        }
    typename BaseIterator<IteratorType, BasePolicy>::pointer                     operator->             ()                                               {
        return (&*BaseIterator<IteratorType, BasePolicy>::_GetIter());
        }

    typename BaseIterator<IteratorType, BasePolicy>::iterator_t&                 operator++             ()                                               {
        return BaseIterator<IteratorType, BasePolicy>::_Increment();
        }
    typename BaseIterator<IteratorType, BasePolicy>::iterator_t                  operator++             (int)                                            {
        typename BasePolicy::iterator_t _Tmp = BaseIterator<IteratorType, BasePolicy>::_GetIter();
        ++BaseIterator<IteratorType, BasePolicy>::_GetIter();
        return (_Tmp);
        }

    bool                        operator==             (const typename BaseIterator<IteratorType, BasePolicy>::iterator_t&           pi_rRight) const    {
        return BaseIterator<IteratorType, BasePolicy>::_EqualTo(pi_rRight);
        }
    bool                        operator!=             (const typename BaseIterator<IteratorType, BasePolicy>::iterator_t&           pi_rRight) const    {
        return (!(BaseIterator<IteratorType, BasePolicy>::_GetIter() == pi_rRight));
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description  This is a generic template to facilitate implementation of custom
*               forward iterators. Avoid code duplication for const version
*               of the iterator and minimizes required methods to implement without
*               any performance cost (that virtual methods call would normally incur).
*
*               IteratorType:       The class that inherits from ForwardIteratorWithAutoReverseConst.
*
*               ReverseConstIteratorType:
*                                   The iterator type that operates on the inverse
*                                   const of the object type pointed by IteratorType.
*
*               T:                  The type that the iterator iterates on.
*
*
*   E.g. (for an iterator that iterates on "OType"):
*
*   class OType;
*
*   template <bool IsConst = false>
*   class Iter : public ForwardIteratorWithAutoReverseConst<Iter<IsConst>,
*                                                           Iter<!IsConst>,
*                                                           typename ConstTriggerTrait<OType, IsConst>::value>
*   {
*   private: // OPTIONAL: In order to provide more encapsulation (so that required members are not accessible directly through public interface)
*       friend              super_class; // Make required method accessible to super class
*
*
*       // IteratorType requirement: Need to implement the following members:
*
*       const_iterator_t    ConvertToConst () const;
*
*       const_reference     Dereference    () const;
*       reference           Dereference    ();
*       void                Increment      ();
*       bool                EqualTo        (const iterator_t&) const;
*
*       //IteratorType optional: May implement the following members to increase
*       //                       performances by avoiding construction of
*       //                       const kind of iterator:
*
*       bool                EqualTo        (const rconst_iterator_t&) const;
*
*       // IteratorType exemple (will certainly be more complex):
*       friend              rconst_iterator_t;
*       friend              SomeContainerClass;
*                           iterator_t     (value_type*);
*       value_type*         m_pObject;
*   };
*
*   typedef Iter<true>      const_iterator
*   typedef Iter<false>     iterator
*
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename IteratorType, typename ReverseConstIteratorType, typename T, bool IsScalar = false,
         typename BasePolicy = BaseIteratorPolicy<std::forward_iterator_tag, T, IsScalar> >
class ForwardIteratorWithAutoReverseConst
    :   public BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>
    {
public:
    operator typename BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>::const_iterator_t
    () const                                         {
        return BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>::_ConvertToConst();
        }


    typename BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>::reference                   operator*              ()                                               {
        return BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>::_Dereference();
        }
    typename BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>::const_reference             operator*              () const                                         {
        return BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>::_Dereference();
        }

    typename BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>::const_pointer               operator->             () const                                         {
        return (&*BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>::_GetIter());
        }
    typename BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>::pointer                     operator->             ()                                               {
        return (&*BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>::_GetIter());
        }

    typename BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>::iterator_t&                 operator++             ()                                               {
        return BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>::_Increment();
        }
    typename BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>::iterator_t                  operator++             (int)                                            {
        typename BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>::iterator_t _Tmp = BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>::_GetIter();
        ++BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>::_GetIter();
        return (_Tmp);
        }

    bool                        operator==             (const typename BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>::iterator_t&           pi_rRight) const    {
        return BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>::_EqualTo(pi_rRight);
        }
    bool                        operator==             (const typename BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>::rconst_iterator_t&    pi_rRight) const    {
        return BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>::_EqualTo(pi_rRight);
        }

    bool                        operator!=             (const typename BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>::iterator_t&           pi_rRight) const    {
        return (!(BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>::_GetIter() == pi_rRight));
        }
    bool                        operator!=             (const typename BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>::rconst_iterator_t&    pi_rRight) const    {
        return (!(BaseIteratorWithAutoRerverseConst<IteratorType, ReverseConstIteratorType, BasePolicy>::_GetIter() == pi_rRight));
        }

    };





/*---------------------------------------------------------------------------------**//**
* @description  This is a generic template to facilitate implementation of custom
*               bidirectional iterators. Minimizes required methods to implement without
*               any performance cost (that virtual methods call would normally incur).
*
*               IteratorType:       The class that inherits from BidirectionalIterator.
*               T:                  The type that the iterator iterates on.
*
*
*   E.g. (for an iterator that iterates on "OType"):
*
*   class OType;
*
*   class Iter : public BidirectionalIterator<Iter, OType>
*   {
*   private: // OPTIONAL: In order to provide more encapsulation (so that required members are not accessible directly through public interface)
*       friend              super_class; // Make required method accessible to super class
*
*
*       // IteratorType requirement: Need to implement the following members:
*
*       const_reference     Dereference    () const;
*       reference           Dereference    ();
*       void                Increment      ();
*       void                Decrement      ();
*       bool                EqualTo        (const iterator_t&) const;
*
*       // IteratorType exemple (will certainly be more complex):
*       friend              SomeContainerClass;
*                           iterator_t     (value_type*);
*       value_type*         m_pObject;
*   };
*
*   typedef Iter            iterator
*
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename IteratorType, typename T, bool IsScalar = false,
         typename BasePolicy = BaseIteratorPolicy<std::bidirectional_iterator_tag, T, IsScalar> >
class BidirectionalIterator : public ForwardIterator<IteratorType, T, IsScalar, BasePolicy>
    {
public:
    typename ForwardIterator<IteratorType, T, IsScalar, BasePolicy>::iterator_t&                 operator--             ()                                               {
        return ForwardIterator<IteratorType, T, IsScalar, BasePolicy>::_Decrement();
        }
    typename ForwardIterator<IteratorType, T, IsScalar, BasePolicy>::iterator_t                  operator--             (int)                                            {
        typename ForwardIterator<IteratorType, T, IsScalar, BasePolicy>::iterator_t _Tmp = ForwardIterator<IteratorType, T, IsScalar, BasePolicy>::_GetIter();
        --ForwardIterator<IteratorType, T, IsScalar, BasePolicy>::_GetIter();
        return (_Tmp);
        }
    };




/*---------------------------------------------------------------------------------**//**
* @description  This is a generic template to facilitate implementation of custom
*               bidirectional iterators. Avoid code duplication for const version
*               of the iterator and minimizes required methods to implement without
*               any performance cost (that virtual methods call would normally incur).
*
*               IteratorType:       The class that inherits from BidirectionalIterator.
*
*               ReverseConstIteratorType:
*                                   The iterator type that operates on the inverse
*                                   const of the object type pointed by IteratorType.
*
*               T:                  The type that the iterator iterates on.
*
*
*   E.g. (for an iterator that iterates on "OType"):
*
*   class OType;
*
*   template <bool IsConst = false>
*   class Iter : public BidirectionalIteratorWithAutoReverseConst<Iter<IsConst>,
*                                                                 Iter<!IsConst>,
*                                                                 typename ConstTriggerTrait<OType, IsConst>::value>
*   {
*   private: // OPTIONAL: In order to provide more encapsulation (so that required members are not accessible directly through public interface)
*       friend              super_class; // Make required method accessible to super class
*
*
*       // IteratorType requirement: Need to implement the following members:
*
*       const_iterator_t    ConvertToConst () const;
*
*       const_reference     Dereference    () const;
*       reference           Dereference    ();
*       void                Increment      ();
*       void                Decrement      ();
*       bool                EqualTo        (const iterator_t&) const;
*
*       //IteratorType optional: May implement the following members to increase
*       //                       performances by avoiding construction of
*       //                       const kind of iterator:
*
*       bool                EqualTo        (const rconst_iterator_t&) const;
*
*       // IteratorType exemple (will certainly be more complex):
*       friend              rconst_iterator_t;
*       friend              SomeContainerClass;
*                           iterator_t     (value_type*);
*       value_type*         m_pObject;
*   };
*
*   typedef Iter<true>      const_iterator
*   typedef Iter<false>     iterator
*
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename IteratorType, typename ReverseConstIteratorType, typename T, bool IsScalar = false,
         typename BasePolicy = BaseIteratorPolicy<std::bidirectional_iterator_tag, T, IsScalar> >
class BidirectionalIteratorWithAutoReverseConst
    :   public ForwardIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>
    {
public:

    typename ForwardIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>::iterator_t&                 operator--             ()                                               {
        return ForwardIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>::_Decrement();
        }
    typename ForwardIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>::iterator_t                  operator--             (int)                                            {
        typename ForwardIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>::iterator_t _Tmp = ForwardIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>::_GetIter();
        --ForwardIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>::_GetIter();
        return (_Tmp);
        }
    };



/*---------------------------------------------------------------------------------**//**
* @description  This is a generic template to facilitate implementation of custom
*               random access iterators. Minimizes required methods to implement without
*               any performance cost (that virtual methods call would normally incur).
*
*               IteratorType:       The class that inherits from RandomAccessIterator.
*               T:                  The type that the iterator iterates on.
*
*
*   E.g. (for an iterator that iterates on "OType"):
*
*   class OType;
*
*   class Iter : public RandomAccessIterator<Iter, OType>
*   {
*   private: // OPTIONAL: In order to provide more encapsulation (so that required members are not accessible directly through public interface)
*       friend              super_class; // Make required method accessible to super class
*
*       // IteratorType requirement: Need to implement the following members:
*
*
*       const_reference     Dereference    () const;
*       reference           Dereference    ();
*       void                Increment      ();
*       void                Decrement      ();
*       void                AdvanceOf      (difference_type);
*       difference_type     DistanceFrom   (const iterator_t&) const;
*       bool                EqualTo        (const iterator_t&) const;
*       bool                LessThan       (const iterator_t&) const;
*
*       // IteratorType exemple (will certainly be more complex):
*       friend              SomeContainerClass;
*                           iterator_t     (value_type*);
*       value_type*         m_pObject;
*   };
*
*   typedef Iter            iterator
*
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename IteratorType, typename T, bool IsScalar = false,
         typename BasePolicy = BaseIteratorPolicy<std::random_access_iterator_tag, T, IsScalar> >
class RandomAccessIterator
    :   public BidirectionalIterator<IteratorType, T, IsScalar, BasePolicy>
    {
public:
    typename BasePolicy::iterator_t&                 operator+=             (typename BasePolicy::difference_type             pi_Offset)          {
        return BidirectionalIterator<IteratorType, T, IsScalar, BasePolicy>::_AdvanceOf(pi_Offset);
        }
    typename BasePolicy::iterator_t&                 operator-=             (typename BasePolicy::difference_type             pi_Offset)          {
        return (BidirectionalIterator<IteratorType, T, IsScalar, BasePolicy>::_GetIter() += -pi_Offset);
        }

    typename BasePolicy::iterator_t                  operator+              (typename BasePolicy::difference_type             pi_Offset) const    {
        typename BasePolicy::iterator_t _Tmp = BidirectionalIterator<IteratorType, T, IsScalar, BasePolicy>::_GetIter();
        return (_Tmp += pi_Offset);
        }
    typename BasePolicy::iterator_t                  operator-              (typename BasePolicy::difference_type             pi_Offset) const    {
        typename BasePolicy::iterator_t _Tmp = BidirectionalIterator<IteratorType, T, IsScalar, BasePolicy>::_GetIter();
        return (_Tmp -= pi_Offset);
        }

    typename BasePolicy::difference_type             operator-              (const typename BasePolicy::iterator_t&           pi_rRight) const    {
        return BidirectionalIterator<IteratorType, T, IsScalar, BasePolicy>::_DistanceFrom(pi_rRight);
        }

    bool                        operator<              (const typename BasePolicy::iterator_t&           pi_rRight) const    {
        return BidirectionalIterator<IteratorType, T, IsScalar, BasePolicy>::_LessThan(pi_rRight);
        }
    bool                        operator>              (const typename BasePolicy::iterator_t&           pi_rRight) const    {
        return (pi_rRight < BidirectionalIterator<IteratorType, T, IsScalar, BasePolicy>::_GetIter());
        }
    bool                        operator<=             (const typename BasePolicy::iterator_t&           pi_rRight) const    {
        return (!(pi_rRight < BidirectionalIterator<IteratorType, T, IsScalar, BasePolicy>::_GetIter()));
        }
    bool                        operator>=             (const typename BasePolicy::iterator_t&           pi_rRight) const    {
        return (!(BidirectionalIterator<IteratorType, T, IsScalar, BasePolicy>::_GetIter() < pi_rRight));
        }

    typename BasePolicy::reference                   operator[]             (typename BasePolicy::difference_type             pi_Offset)          {
        return (*(BidirectionalIterator<IteratorType, T, IsScalar, BasePolicy>::_GetIter() + pi_Offset));
        }
    typename BasePolicy::const_reference             operator[]             (typename BasePolicy::difference_type             pi_Offset) const    {
        return (*(BidirectionalIterator<IteratorType, T, IsScalar, BasePolicy>::_GetIter() + pi_Offset));
        }
    };




/*---------------------------------------------------------------------------------**//**
* @description  This is a generic template to facilitate implementation of custom
*               random access iterators. Avoid code duplication for const version
*               of the iterator and minimizes required methods to implement without
*               any performance cost (that virtual methods call would normally incur).
*
*               IteratorType:       The class that inherits from RandomAccessIterator.
*
*               ReverseConstIteratorType:
*                                   The iterator type that operates on the inverse
*                                   const of the object type pointed by IteratorType.
*
*               T:                  The type that the iterator iterates on.
*
*
*   E.g. (for an iterator that iterates on "OType"):
*
*   class OType;
*
*   template <bool IsConst = false>
*   class Iter : public RandomAccessIteratorWithAutoReverseConst<Iter<IsConst>,
*                                                                Iter<!IsConst>,
*                                                                typename ConstTriggerTrait<OType, IsConst>::value>
*   {
*   private: // OPTIONAL: In order to provide more encapsulation (so that required members are not accessible directly through public interface)
*       friend              super_class; // Make required method accessible to super class
*
*       // IteratorType requirement: Need to implement the following members:
*
*       const_iterator_t    ConvertToConst () const;
*
*       const_reference     Dereference    () const;
*       reference           Dereference    ();
*       void                Increment      ();
*       void                Decrement      ();
*       void                AdvanceOf      (difference_type);
*       difference_type     DistanceFrom   (const iterator_t&) const;
*       bool                EqualTo        (const iterator_t&) const;
*       bool                LessThan       (const iterator_t&) const;
*
*
*       //IteratorType optional: May implement the following members to increase
*       //                       performances by avoiding construction of
*       //                       const kind of iterator:
*
*       difference_type     DistanceFrom   (const rconst_iterator_t&) const;
*       bool                EqualTo        (const rconst_iterator_t&) const;
*       bool                LessThan       (const rconst_iterator_t&) const;
*
*       // IteratorType exemple (will certainly be more complex):
*       friend              rconst_iterator_t;
*       friend              SomeContainerClass;
*                           iterator_t     (value_type*);
*       value_type*         m_pObject;
*   };
*
*   typedef Iter<true>      const_iterator
*   typedef Iter<false>     iterator
*
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename IteratorType, typename ReverseConstIteratorType, typename T, bool IsScalar = false,
         typename BasePolicy = BaseIteratorPolicy<std::random_access_iterator_tag, T, IsScalar> >
class RandomAccessIteratorWithAutoReverseConst
    :   public BidirectionalIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>
    {
public:
typedef BidirectionalIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>
          RandomAccessIteratorWithAutoReverseConst_Type1;  

    typename RandomAccessIteratorWithAutoReverseConst_Type1::iterator_t&                 operator+=             (typename RandomAccessIteratorWithAutoReverseConst_Type1::difference_type             pi_Offset)          {
        return BidirectionalIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>::_AdvanceOf(pi_Offset);
        }
    typename RandomAccessIteratorWithAutoReverseConst_Type1::iterator_t&                 operator-=             (typename RandomAccessIteratorWithAutoReverseConst_Type1::difference_type           pi_Offset)            {
        return (BidirectionalIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>::_GetIter() += -pi_Offset);
        }

    typename RandomAccessIteratorWithAutoReverseConst_Type1::iterator_t                  operator+              (typename RandomAccessIteratorWithAutoReverseConst_Type1::difference_type           pi_Offset) const      {
        typename RandomAccessIteratorWithAutoReverseConst_Type1::iterator_t _Tmp = BidirectionalIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>::_GetIter();
        return (_Tmp += pi_Offset);
        }
    typename RandomAccessIteratorWithAutoReverseConst_Type1::iterator_t                  operator-              (typename RandomAccessIteratorWithAutoReverseConst_Type1::difference_type           pi_Offset) const      {
        typename RandomAccessIteratorWithAutoReverseConst_Type1::iterator_t _Tmp = BidirectionalIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>::_GetIter();
        return (_Tmp -= pi_Offset);
        }
    typename RandomAccessIteratorWithAutoReverseConst_Type1::difference_type             operator-              (const typename RandomAccessIteratorWithAutoReverseConst_Type1::iterator_t&           pi_rRight) const    {
        return BidirectionalIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>::_DistanceFrom(pi_rRight);
        }
    typename RandomAccessIteratorWithAutoReverseConst_Type1::difference_type             operator-              (const typename RandomAccessIteratorWithAutoReverseConst_Type1::rconst_iterator_t&    pi_rRight) const    {
        return BidirectionalIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>::_DistanceFrom(pi_rRight);
        }

    bool                        operator<              (const typename RandomAccessIteratorWithAutoReverseConst_Type1::iterator_t&           pi_rRight) const    {
        return BidirectionalIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>::_LessThan(pi_rRight);
        }
    bool                        operator<              (const typename RandomAccessIteratorWithAutoReverseConst_Type1::rconst_iterator_t&    pi_rRight) const    {
        return BidirectionalIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>::_LessThan(pi_rRight);
        }
    bool                        operator>              (const typename RandomAccessIteratorWithAutoReverseConst_Type1::iterator_t&           pi_rRight) const    {
        return (pi_rRight < BidirectionalIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>::_GetIter());
        }
    bool                        operator>              (const typename RandomAccessIteratorWithAutoReverseConst_Type1::rconst_iterator_t&    pi_rRight) const    {
        return (pi_rRight < BidirectionalIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>::_GetIter());
        }
    bool                        operator<=             (const typename RandomAccessIteratorWithAutoReverseConst_Type1::iterator_t&           pi_rRight) const    {
        return (!(pi_rRight < BidirectionalIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>::_GetIter()));
        }
    bool                        operator<=             (const typename RandomAccessIteratorWithAutoReverseConst_Type1::rconst_iterator_t&    pi_rRight) const    {
        return (!(pi_rRight < BidirectionalIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>::_GetIter()));
        }
    bool                        operator>=             (const typename RandomAccessIteratorWithAutoReverseConst_Type1::iterator_t&           pi_rRight) const    {
        return (!(BidirectionalIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>::_GetIter() < pi_rRight));
        }
    bool                        operator>=             (const typename RandomAccessIteratorWithAutoReverseConst_Type1::rconst_iterator_t&    pi_rRight) const    {
        return (!(BidirectionalIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>::_GetIter() < pi_rRight));
        }

    typename RandomAccessIteratorWithAutoReverseConst_Type1::reference                   operator[]             (typename RandomAccessIteratorWithAutoReverseConst_Type1::difference_type             pi_Offset)          {
        return (*(BidirectionalIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>::_GetIter() + pi_Offset));
        }
    typename RandomAccessIteratorWithAutoReverseConst_Type1::const_reference             operator[]             (typename RandomAccessIteratorWithAutoReverseConst_Type1::difference_type             pi_Offset) const    {
        return (*(BidirectionalIteratorWithAutoReverseConst<IteratorType, ReverseConstIteratorType, T, IsScalar, BasePolicy>::_GetIter() + pi_Offset));
        }

    };


/*---------------------------------------------------------------------------------**//**
* @description  This is an helper iterator template that encapsulate iteration over
*               ranges of pointers. This iterator automatically dereference the
*               wrapped iterator's underlying pointer making iteration over ranges
*               of pointers same as iteration over pointer's pointed values.
*
*               ContainerType:      The container that will create the iterator
*
*               WrappedIteratorType:The wrapped iterator. Must be an iterator on pointers.
*
*               DereferencedElementType
*                                  :The elements pointed by the wrapped iterator's value.
*                                   This is also the type that will be returned when
*                                   this iterator is dereferenced.
*
*
*
*   E.g. (for an iterator that iterates on elements of "MyContainer"):
*
*   class MyContainer{
*   vector<int*> MyIntArray;
*
*   typedef DereferenceForwardIterator<MyContainer, vector<int*>::const_iterator, const int>
*                                                   const_iterator;
*
*   const_iterator Begin () {return const_iterator(MyIntArray.begin());};
*   const_iterator End () {return const_iterator(MyIntArray.end());};
*   };
*
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ContainerType, typename WrappedIteratorType, typename DereferencedElementType>
class DereferenceForwardIterator : public ForwardIterator<DereferenceForwardIterator<ContainerType, WrappedIteratorType, DereferencedElementType>, DereferencedElementType>
    {
public:
typedef ForwardIterator<DereferenceForwardIterator<ContainerType, WrappedIteratorType, DereferencedElementType>, DereferencedElementType> 
            ForwardIterator_type;

    void                        CheckForwardIteratorCompatible
    (std::forward_iterator_tag) const {};
    void                        CheckDereferenceTypeCompatible
    (DereferencedElementType&) const {};

    explicit                    DereferenceForwardIterator
    (WrappedIteratorType&        pi_rWrappedIter)
        :   m_WrappedIter(pi_rWrappedIter)
        {
        // Will fail if wrapped iterator is not forward compatible
        CheckForwardIteratorCompatible(typename iterator_traits<WrappedIteratorType>::iterator_category());
        // Will fail if wrapped iterator's value type does not match specified DereferencedElementType
        CheckDereferenceTypeCompatible(*(typename iterator_traits<WrappedIteratorType>::value_type()));
        }

    const WrappedIteratorType&  GetWrapped             () const
        {
        return m_WrappedIter;
        }

    typename ForwardIterator_type::const_reference             Dereference            () const
        {
        HPRECONDITION(*m_WrappedIter != 0);
        return **m_WrappedIter;
        }

    typename ForwardIterator_type::reference                   Dereference            ()
        {
        HPRECONDITION(*m_WrappedIter != 0);
        return **m_WrappedIter;
        }

    void                        Increment              ()
        {
        ++m_WrappedIter;
        }

    bool                        EqualTo                (const typename ForwardIterator_type::iterator_t&           pi_rRight) const
        {
        return m_WrappedIter == pi_rRight.m_WrappedIter;
        }

    WrappedIteratorType         m_WrappedIter;
    };


/*---------------------------------------------------------------------------------**//**
* @description  This is an helper iterator template that encapsulate iteration over
*               ranges of pointers. This iterator automatically dereference the
*               wrapped iterator's underlying pointer making iteration over ranges
*               of pointers same as iteration over pointer's pointed values. This
*               iterator also supports operations between itself and its reverse const
*               type as specified by STL standard.
*
*               ContainerType:      The container that will create the iterator
*
*               WrappedIteratorType:The wrapped iterator. Must be an iterator on pointers.
*
*               ReverseConstWrappedIteratorType
*                                  :The reverse const iterator of WrappedIteratorType.
*
*               DereferencedElementType
*                                  :The elements pointed by the wrapped iterator's value.
*                                   This is also the type that will be returned when
*                                   this iterator is dereferenced.
*
*
*
*   E.g. (for an iterator that iterates on elements of "MyContainer"):
*
*   class MyContainer{
*   vector<int*> MyIntArray;
*
*   typedef DereferenceForwardIteratorWithAutoReverseConst<MyContainer, vector<int*>::iterator, vector<int*>::const_iterator, int>
*                                                   iterator;
*   typedef DereferenceForwardIteratorWithAutoReverseConst<MyContainer, vector<int*>::const_iterator, vector<int*>::iterator, const int>
*                                                   const_iterator;
*
*   const_iterator Begin () const {return const_iterator(MyIntArray.begin());};
*   const_iterator End () const {return const_iterator(MyIntArray.end());};
*   iterator Begin () {return iterator(MyIntArray.begin());};
*   iterator End () {return iterator(MyIntArray.end());};
*   };
*
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ContainerType, typename WrappedIteratorType, typename ReverseConstWrappedIteratorType, typename DereferencedElementType>
class DereferenceForwardIteratorWithAutoReverseConst
    : public ForwardIteratorWithAutoReverseConst<DereferenceForwardIteratorWithAutoReverseConst<ContainerType,
      WrappedIteratorType,
      ReverseConstWrappedIteratorType,
      DereferencedElementType>,
      DereferenceForwardIteratorWithAutoReverseConst<ContainerType,
      ReverseConstWrappedIteratorType,
      WrappedIteratorType,
      typename ReverseConstTrait<DereferencedElementType>::type>,
      DereferencedElementType>
    {
public:
    typedef ForwardIteratorWithAutoReverseConst<DereferenceForwardIteratorWithAutoReverseConst<ContainerType,
                                                 WrappedIteratorType,
                                                 ReverseConstWrappedIteratorType,
                                                 DereferencedElementType>,
                                                 DereferenceForwardIteratorWithAutoReverseConst<ContainerType,
                                                 ReverseConstWrappedIteratorType,
                                                 WrappedIteratorType,
                                                 typename ReverseConstTrait<DereferencedElementType>::type>,
                                                 DereferencedElementType>  
                        ForwardIteratorWithAutoReverseConst_Type;

    void                        CheckForwardIteratorCompatible
    (std::forward_iterator_tag) const {};
    void                        CheckDereferenceTypeCompatible
    (DereferencedElementType&) const {};

    explicit                    DereferenceForwardIteratorWithAutoReverseConst
    (WrappedIteratorType&        pi_rWrappedIter)
        :   m_WrappedIter(pi_rWrappedIter)
        {
        // Will fail if wrapped iterator is not forward compatible
        CheckForwardIteratorCompatible(typename iterator_traits<WrappedIteratorType>::iterator_category());
        // Will fail if wrapped iterator's value type does not match specified DereferencedElementType
        CheckDereferenceTypeCompatible(*(typename iterator_traits<WrappedIteratorType>::value_type()));
        }

    const WrappedIteratorType&  GetWrapped             () const
        {
        return m_WrappedIter;
        }

    typename ForwardIteratorWithAutoReverseConst_Type::const_iterator_t            ConvertToConst         () const

        {
        return const_iterator_t(m_WrappedIter);
        }

    typename ForwardIteratorWithAutoReverseConst_Type::const_reference             Dereference            () const
        {
        HPRECONDITION(*m_WrappedIter != 0);
        return **m_WrappedIter;
        }

    typename ForwardIteratorWithAutoReverseConst_Type::reference                   Dereference            ()
        {
        HPRECONDITION(*m_WrappedIter != 0);
        return **m_WrappedIter;
        }

    void                        Increment              ()
        {
        ++m_WrappedIter;
        }

    bool                        EqualTo                (const typename ForwardIteratorWithAutoReverseConst_Type::iterator_t&           pi_rRight) const
        {
        return m_WrappedIter == pi_rRight.m_WrappedIter;
        }

    bool                        EqualTo                (const typename ForwardIteratorWithAutoReverseConst_Type::rconst_iterator_t&    pi_rRight) const
        {
        return m_WrappedIter == pi_rRight.m_WrappedIter;
        }

    WrappedIteratorType         m_WrappedIter;
    };


/*---------------------------------------------------------------------------------**//**
* @description  This is an helper iterator template that encapsulate iteration over
*               ranges of pointers. This iterator automatically dereference the
*               wrapped iterator's underlying pointer making iteration over ranges
*               of pointers same as iteration over pointer's pointed values.
*
*               ContainerType:      The container that will create the iterator
*
*               WrappedIteratorType:The wrapped iterator. Must be an iterator on pointers.
*
*               DereferencedElementType
*                                  :The elements pointed by the wrapped iterator's value.
*                                   This is also the type that will be returned when
*                                   this iterator is dereferenced.
*
*
*
*   E.g. (for an iterator that iterates on elements of "MyContainer"):
*
*   class MyContainer{
*   vector<int*> MyIntArray;
*
*   typedef DereferenceRandomAccessIterator<MyContainer, vector<int*>::const_iterator, const int>
*                                                   const_iterator;
*
*   const_iterator Begin () const {return const_iterator(MyIntArray.begin());};
*   const_iterator End () const {return const_iterator(MyIntArray.end());};
*   };
*
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ContainerType, typename WrappedIteratorType,typename DereferencedElementType>
class DereferenceRandomAccessIterator
    : public RandomAccessIterator<DereferenceRandomAccessIterator<ContainerType,
      WrappedIteratorType,
      DereferencedElementType>,
      DereferencedElementType>
    {
public:
    typedef RandomAccessIterator<DereferenceRandomAccessIterator<ContainerType,
                                WrappedIteratorType,
                                DereferencedElementType>,
                                DereferencedElementType>
                RandomAccessIterator_Type;

    explicit                    DereferenceRandomAccessIterator
    ()
        :   m_WrappedIter()
        {
        // Will fail if wrapped iterator is not forward compatible
        CheckForwardIteratorCompatible(typename iterator_traits<WrappedIteratorType>::iterator_category());
        // Will fail if wrapped iterator's value type does not match specified DereferencedElementType
        CheckDereferenceTypeCompatible(*(typename iterator_traits<WrappedIteratorType>::value_type()));
        }

    void                        CheckForwardIteratorCompatible
    (std::random_access_iterator_tag) const {};
    void                        CheckDereferenceTypeCompatible
    (DereferencedElementType&) const {};

    explicit                    DereferenceRandomAccessIterator
    (WrappedIteratorType&        pi_rWrappedIter)
        :   m_WrappedIter(pi_rWrappedIter)
        {
        // Will fail if wrapped iterator is not forward compatible
        CheckForwardIteratorCompatible(typename iterator_traits<WrappedIteratorType>::iterator_category());
        // Will fail if wrapped iterator's value type does not match specified DereferencedElementType
        CheckDereferenceTypeCompatible(*(typename iterator_traits<WrappedIteratorType>::value_type()));
        }

    const WrappedIteratorType&  GetWrapped             () const
        {
        return m_WrappedIter;
        }

    typename RandomAccessIterator_Type::const_reference             Dereference            () const
        {
        HPRECONDITION(*m_WrappedIter != 0);
        return **m_WrappedIter;
        }

    typename RandomAccessIterator_Type::reference                   Dereference            ()
        {
        HPRECONDITION(*m_WrappedIter != 0);
        return **m_WrappedIter;
        }

    void                        Increment              ()
        {
        ++m_WrappedIter;
        }

    void                        Decrement              ()
        {
        --m_WrappedIter;
        }

    void                        AdvanceOf              (typename RandomAccessIterator_Type::difference_type             pi_Offset)
        {
        m_WrappedIter += pi_Offset;
        }

    typename RandomAccessIterator_Type::difference_type             DistanceFrom           (const typename RandomAccessIterator_Type::iterator_t&           pi_rRight) const
        {
        return m_WrappedIter - pi_rRight.m_WrappedIter;
        }

    bool                        EqualTo                (const typename RandomAccessIterator_Type::iterator_t&           pi_rRight) const
        {
        return m_WrappedIter == pi_rRight.m_WrappedIter;
        }

    bool                        LessThan               (const typename RandomAccessIterator_Type::iterator_t&           pi_rRight) const
        {
        return m_WrappedIter < pi_rRight.m_WrappedIter;
        }

           WrappedIteratorType         m_WrappedIter;
    };



/*---------------------------------------------------------------------------------**//**
* @description  This is an helper iterator template that encapsulate iteration over
*               ranges of pointers. This iterator automatically dereference the
*               wrapped iterator's underlying pointer making iteration over ranges
*               of pointers same as iteration over pointer's pointed values. This
*               iterator also supports operations between itself and its reverse const
*               type as specified by STL standard.
*
*               ContainerType:      The container that will create the iterator
*
*               WrappedIteratorType:The wrapped iterator. Must be an iterator on pointers.
*
*               ReverseConstWrappedIteratorType
*                                  :The reverse const iterator of WrappedIteratorType.
*
*               DereferencedElementType
*                                  :The elements pointed by the wrapped iterator's value.
*                                   This is also the type that will be returned when
*                                   this iterator is dereferenced.
*
*
*
*   E.g. (for an iterator that iterates on elements of "MyContainer"):
*
*   class MyContainer{
*   vector<int*> MyIntArray;
*
*   typedef DereferenceRandomAccessIteratorWithAutoReverseConst<MyContainer, vector<int*>::iterator, vector<int*>::const_iterator, int>
*                                                   iterator;
*   typedef DereferenceRandomAccessIteratorWithAutoReverseConst<MyContainer, vector<int*>::const_iterator, vector<int*>::iterator, const int>
*                                                   const_iterator;
*
*   const_iterator Begin () const {return const_iterator(MyIntArray.begin());};
*   const_iterator End () const {return const_iterator(MyIntArray.end());};
*   iterator Begin () {return iterator(MyIntArray.begin());};
*   iterator End () {return iterator(MyIntArray.end());};
*   };
*
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ContainerType, typename WrappedIteratorType, typename ReverseConstWrappedIteratorType, typename DereferencedElementType>
class DereferenceRandomAccessIteratorWithAutoReverseConst
    : public RandomAccessIteratorWithAutoReverseConst<DereferenceRandomAccessIteratorWithAutoReverseConst<ContainerType,
      WrappedIteratorType,
      ReverseConstWrappedIteratorType,
      DereferencedElementType>,
      DereferenceRandomAccessIteratorWithAutoReverseConst<ContainerType,
      ReverseConstWrappedIteratorType,
      WrappedIteratorType,
      typename ReverseConstTrait<DereferencedElementType>::type>,
      DereferencedElementType>
    {
public:
    typedef  RandomAccessIteratorWithAutoReverseConst<DereferenceRandomAccessIteratorWithAutoReverseConst<ContainerType,
                                                    WrappedIteratorType,
                                                    ReverseConstWrappedIteratorType,
                                                    DereferencedElementType>,
                                                    DereferenceRandomAccessIteratorWithAutoReverseConst<ContainerType,
                                                    ReverseConstWrappedIteratorType,
                                                    WrappedIteratorType,
                                                    typename ReverseConstTrait<DereferencedElementType>::type>,
                                                    DereferencedElementType>
                RandomAccessIteratorWithAutoReverseConst_Type;

    explicit                    DereferenceRandomAccessIteratorWithAutoReverseConst
    ()
        :   m_WrappedIter()
        {
        // Will fail if wrapped iterator is not forward compatible
        CheckForwardIteratorCompatible(typename iterator_traits<WrappedIteratorType>::iterator_category());
        // Will fail if wrapped iterator's value type does not match specified DereferencedElementType
        CheckDereferenceTypeCompatible(*(typename iterator_traits<WrappedIteratorType>::value_type()));
        }

    void                        CheckForwardIteratorCompatible
    (std::random_access_iterator_tag) const {};
    void                        CheckDereferenceTypeCompatible
    (DereferencedElementType&) const {};

    explicit                    DereferenceRandomAccessIteratorWithAutoReverseConst
    (const WrappedIteratorType&        pi_rWrappedIter)
        :   m_WrappedIter(pi_rWrappedIter)
        {
        // Will fail if wrapped iterator is not forward compatible
        CheckForwardIteratorCompatible(typename iterator_traits<WrappedIteratorType>::iterator_category());
        // Will fail if wrapped iterator's value type does not match specified DereferencedElementType
        CheckDereferenceTypeCompatible(*(typename iterator_traits<WrappedIteratorType>::value_type()));
        }

    const WrappedIteratorType&  GetWrapped             () const
        {
        return m_WrappedIter;
        }

    typename RandomAccessIteratorWithAutoReverseConst_Type::const_iterator_t            ConvertToConst         () const
        {
        return typename RandomAccessIteratorWithAutoReverseConst_Type::const_iterator_t(m_WrappedIter);
        }

    typename RandomAccessIteratorWithAutoReverseConst_Type::const_reference             Dereference            () const
        {
        HPRECONDITION(*m_WrappedIter != 0);
        return **m_WrappedIter;
        }

    typename RandomAccessIteratorWithAutoReverseConst_Type::reference                   Dereference            ()
        {
        HPRECONDITION(*m_WrappedIter != 0);
        return **m_WrappedIter;
        }

    void                        Increment              ()
        {
        ++m_WrappedIter;
        }

    void                        Decrement              ()
        {
        --m_WrappedIter;
        }

    void                        AdvanceOf              (typename RandomAccessIteratorWithAutoReverseConst_Type::difference_type             pi_Offset)
        {
        m_WrappedIter += pi_Offset;
        }

    typename RandomAccessIteratorWithAutoReverseConst_Type::difference_type             DistanceFrom           (const typename RandomAccessIteratorWithAutoReverseConst_Type::iterator_t&           pi_rRight) const
        {
        return m_WrappedIter - pi_rRight.m_WrappedIter;
        }

    bool                        EqualTo                (const typename RandomAccessIteratorWithAutoReverseConst_Type::iterator_t&           pi_rRight) const
        {
        return m_WrappedIter == pi_rRight.m_WrappedIter;
        }

           bool                        LessThan               (const typename RandomAccessIteratorWithAutoReverseConst_Type::iterator_t&           pi_rRight) const
        {
        return m_WrappedIter < pi_rRight.m_WrappedIter;
        }

           typename RandomAccessIteratorWithAutoReverseConst_Type::difference_type             DistanceFrom           (const typename RandomAccessIteratorWithAutoReverseConst_Type::rconst_iterator_t&    pi_rRight) const
        {
        return m_WrappedIter - pi_rRight.m_WrappedIter;
        }

    bool                        EqualTo                (const typename RandomAccessIteratorWithAutoReverseConst_Type::rconst_iterator_t&    pi_rRight) const
        {
        return m_WrappedIter == pi_rRight.m_WrappedIter;
        }

           bool                        LessThan               (const typename RandomAccessIteratorWithAutoReverseConst_Type::rconst_iterator_t&    pi_rRight) const
        {
        return m_WrappedIter < pi_rRight.m_WrappedIter;
        }


           WrappedIteratorType         m_WrappedIter;
    };




/*---------------------------------------------------------------------------------**//**
* @description  This is a generic template to facilitate implementation of custom
*               random iterators that encapsulate the mechanisms to iterate through
*               elements via indexes. This iterator makes it look like user is iterating
*               directly on elements.
*
*               ElementType:        The type of element this iterator iterates on. This
*                                   is also the type of element that is indexed.
*
*               ElementIteratorType:This is the element iterator type this iterator will
*                                   use to get specific element at indexes specified
*                                   by its current index iterator;
*
*               IndexIteratorType:  This is the index iterator type this iterator will use
*                                   to iterate through indexes that refers to elements
*                                   that this iterator provide as values.
*
*
*   E.g. (for an iterator that iterates on elements of "MyContainer"):
*
*   class MyContainer{
*
*
*   typedef IndexedElementRandomIterator<MyContainer, MyContainer::const_iterator, MyContainer::const_index_iterator, MyContainer::value_type>
*                                                   const_iterator;
*   typedef IndexedElementRandomIterator<MyContainer, MyContainer::iterator, MyContainer::const_index_iterator, MyContainer::value_type>
*                                                   iterator;
*
*   const_iterator Begin () {return const_iterator(Indexes.Begin(), Elements.Begin(), Elements.End(), Indexes.Begin(), Indexes.End())};
*   const_iterator End () {return Begin() + Indexes.size();};
*   };
*
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ContainerType, typename ElementIteratorType, typename IndexIteratorType, typename ElementType>
class IndexedElementRandomIterator : public RandomAccessIteratorWithAutoReverseConst<IndexedElementRandomIterator<ContainerType,
    ElementIteratorType,
    IndexIteratorType,
    ElementType>,
    IndexedElementRandomIterator<ContainerType,
    ElementIteratorType,
    IndexIteratorType,
    typename ReverseConstTrait<ElementType>::type>,
    ElementType>
    {
public:
    typedef RandomAccessIteratorWithAutoReverseConst<IndexedElementRandomIterator<ContainerType,
                                                    ElementIteratorType,
                                                    IndexIteratorType,
                                                    ElementType>,
                                                    IndexedElementRandomIterator<ContainerType,
                                                    ElementIteratorType,
                                                    IndexIteratorType,
                                                    typename ReverseConstTrait<ElementType>::type>,
                                                    ElementType>
                RandomAccessIteratorWithAutoReverseConst_Type;

    explicit                            IndexedElementRandomIterator       ()
        :   m_IndexIter()  {}

    // NOTE: Public interface is STL random access iterators interface.
    //       See RandomAccessIterator.

    struct                              ContainerInfo
        {
        explicit                        ContainerInfo                      (const ElementIteratorType&  pi_ElementBegin,
                                                                            const ElementIteratorType&  pi_ElementEnd,
                                                                            const IndexIteratorType&    pi_IndexBegin,
                                                                            const IndexIteratorType&    pi_IndexEnd)
            :   m_ElementBegin(pi_ElementBegin)
#ifdef __HMR_DEBUG
            , m_ElementEnd(pi_ElementEnd), m_IndexBegin(pi_IndexBegin), m_IndexEnd(pi_IndexEnd)
#endif //__HMR_DEBUG
            {}

        explicit                        ContainerInfo                      () {}

        ElementIteratorType             m_ElementBegin;
#ifdef __HMR_DEBUG
        ElementIteratorType             m_ElementEnd;
        IndexIteratorType               m_IndexBegin;
        IndexIteratorType               m_IndexEnd;
#endif //__HMR_DEBUG
        };

    explicit                            IndexedElementRandomIterator       (const IndexIteratorType&    pi_IndexIter,
                                                                            const ElementIteratorType&  pi_ElementBegin,
                                                                            const ElementIteratorType&  pi_ElementEnd,
                                                                            const IndexIteratorType&    pi_IndexBegin,
                                                                            const IndexIteratorType&    pi_IndexEnd)
        : m_IndexIter(pi_IndexIter),
          m_ContainerInfo(pi_ElementBegin, pi_ElementEnd, pi_IndexBegin, pi_IndexEnd) {}

    IndexIteratorType                   GetIndexIterator                   () const {
        return m_IndexIter;
        }

    typename RandomAccessIteratorWithAutoReverseConst_Type::const_iterator_t                    ConvertToConst                     () const
        {
        return typename RandomAccessIteratorWithAutoReverseConst_Type::rconst_iterator_t(m_IndexIter, m_ContainerInfo.m_ElementBegin, m_ContainerInfo.m_ElementEnd,
                                 m_ContainerInfo.m_IndexBegin, m_ContainerInfo.m_IndexEnd);
        }


    typename RandomAccessIteratorWithAutoReverseConst_Type::const_reference                     Dereference                        () const
        {
        HPRECONDITION(IndexIteratorType() != m_IndexIter);
        return *(m_ContainerInfo.m_ElementBegin + *m_IndexIter);
        }
    typename RandomAccessIteratorWithAutoReverseConst_Type::reference                           Dereference                        ()
        {
        HPRECONDITION(IndexIteratorType() != m_IndexIter);
        return *(m_ContainerInfo.m_ElementBegin + *m_IndexIter);
        }

    void                                Increment  ();
    void                                Decrement  ();
    void                                AdvanceOf  (typename RandomAccessIteratorWithAutoReverseConst_Type::difference_type             pi_Offset)
        {
        HPRECONDITION(IndexIteratorType() != m_IndexIter);
        m_IndexIter += pi_Offset;
        HPOSTCONDITION(m_ContainerInfo.m_IndexBegin <= m_IndexIter &&
            m_ContainerInfo.m_IndexEnd >= m_IndexIter);
        }

    typename RandomAccessIteratorWithAutoReverseConst_Type::difference_type   DistanceFrom (const typename RandomAccessIteratorWithAutoReverseConst_Type::iterator_t&           pi_rRight) const
        {
        HPRECONDITION(m_ContainerInfo.m_IndexBegin == pi_rRight.m_ContainerInfo.m_IndexBegin);

        return distance(pi_rRight.m_IndexIter, m_IndexIter);
        }
    typename RandomAccessIteratorWithAutoReverseConst_Type:: difference_type  DistanceFrom (const typename RandomAccessIteratorWithAutoReverseConst_Type::rconst_iterator_t&    pi_rRight) const
        {
        HPRECONDITION(m_ContainerInfo.m_IndexBegin == pi_rRight.m_ContainerInfo.m_IndexBegin);

        return distance(pi_rRight.m_IndexIter, m_IndexIter);
        }

    bool  EqualTo   (const typename RandomAccessIteratorWithAutoReverseConst_Type::iterator_t&           pi_rRight) const
        {
        HPRECONDITION(m_ContainerInfo.m_IndexBegin == pi_rRight.m_ContainerInfo.m_IndexBegin);
        return m_IndexIter == pi_rRight.m_IndexIter;
        }
    bool  LessThan  (const typename RandomAccessIteratorWithAutoReverseConst_Type::iterator_t&           pi_rRight) const
        {
        HPRECONDITION(m_ContainerInfo.m_IndexBegin == pi_rRight.m_ContainerInfo.m_IndexBegin);
        return m_IndexIter < pi_rRight.m_IndexIter;
        }

    bool  EqualTo   (const typename RandomAccessIteratorWithAutoReverseConst_Type::rconst_iterator_t&    pi_rRight) const
        {
        HPRECONDITION(m_ContainerInfo.m_IndexBegin == pi_rRight.m_ContainerInfo.m_IndexBegin);
        return m_IndexIter == pi_rRight.m_IndexIter;
        }
    bool  LessThan  (const typename RandomAccessIteratorWithAutoReverseConst_Type::rconst_iterator_t&    pi_rRight) const
        {
        HPRECONDITION(m_ContainerInfo.m_IndexBegin == pi_rRight.m_ContainerInfo.m_IndexBegin);
        return m_IndexIter < pi_rRight.m_IndexIter;
        }


    IndexIteratorType                   m_IndexIter;
    ContainerInfo                       m_ContainerInfo;
    };

END_IMAGEPP_NAMESPACE

#include "HIterators.hpp"

