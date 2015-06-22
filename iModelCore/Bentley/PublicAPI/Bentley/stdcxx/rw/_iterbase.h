/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/stdcxx/rw/_iterbase.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/// @cond BENTLEY_SDK_None

/***************************************************************************
 *
 * _iterbase.h - Definitions of iterator primitives
 *
 * This is an internal header file used to implement the C++ Standard
 * Library. It should never be #included directly by a program.
 *
 * $Id: _iterbase.h 681820 2008-08-01 20:51:17Z vitek $
 *
 ***************************************************************************
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 ***************************************************************************
 *
 * Licensed to the Apache Software  Foundation (ASF) under one or more
 * contributor  license agreements.  See  the NOTICE  file distributed
 * with  this  work  for  additional information  regarding  copyright
 * ownership.   The ASF  licenses this  file to  you under  the Apache
 * License, Version  2.0 (the  "License"); you may  not use  this file
 * except in  compliance with the License.   You may obtain  a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the  License is distributed on an  "AS IS" BASIS,
 * WITHOUT  WARRANTIES OR CONDITIONS  OF ANY  KIND, either  express or
 * implied.   See  the License  for  the  specific language  governing
 * permissions and limitations under the License.
 *
 * Copyright 1994-2007 Rogue Wave Software, Inc.
 * 
 **************************************************************************
 *
 * NOTICE: This File contains modifications made by Bentley Systems Inc. where designated.
 *
 *************************************************************************/

#ifndef _RWSTD_RW_ITERBASE_H_INCLUDED
#define _RWSTD_RW_ITERBASE_H_INCLUDED

#ifndef _RWSTD_RW_DEFS_H_INCLUDED
#  include <Bentley/stdcxx/rw/_defs.h>
#endif   // _RWSTD_RW_DEFS_H_INCLUDED


// *** BENTLEY_CHANGE
NAMESPACE_BENTLEY_BSTDCXX_BEGIN



template <class _Tag>
inline bool bc___is_input_iterator (_Tag)
{
    return false;
}


template <class _Tag>
inline bool bc___is_bidirectional_iterator (_Tag)
{
    return false;
}


template <class _Tag>
inline bool bc___is_random_access_iterator (_Tag)
{
    return false;
}


_RWSTD_SPECIALIZED_FUNCTION 
inline bool bc___is_input_iterator (std::input_iterator_tag)
{
    return true;
}


_RWSTD_SPECIALIZED_FUNCTION 
inline bool bc___is_bidirectional_iterator (std::bidirectional_iterator_tag)
{
    return true;
}


_RWSTD_SPECIALIZED_FUNCTION 
inline bool bc___is_bidirectional_iterator (std::random_access_iterator_tag)
{
    return true;
}


_RWSTD_SPECIALIZED_FUNCTION 
inline bool bc___is_random_access_iterator (std::random_access_iterator_tag)
{
    return true;
}


#ifndef _RWSTD_NO_CLASS_PARTIAL_SPEC

template <class _Iterator>
inline typename std::iterator_traits<_Iterator>::value_type*
bc___value_type (const _Iterator*)
{ 
    return 0;
}

#else   // if defined (_RWSTD_NO_CLASS_PARTIAL_SPEC)

template <class _Category, class _TypeT, class _Distance, 
          class _Pointer, class _Reference>
inline _TypeT*
bc___value_type (const iterator<_Category, _TypeT, _Distance,
                             _Pointer, _Reference>*)
{
    return 0;
}

template <class _TypeT>
inline _TypeT* bc___value_type (const _TypeT* const*)
{
    return 0;
}

#endif   // _RWSTD_NO_CLASS_PARTIAL_SPEC


#ifndef _RWSTD_NO_CLASS_PARTIAL_SPEC

template <class _Iterator>
inline typename std::iterator_traits<_Iterator>::difference_type*
bc___distance_type (_Iterator)
{ 
    return 0;
}

#else   // if defined (_RWSTD_NO_CLASS_PARTIAL_SPEC)

template <class _Category, class _TypeT, class _Distance, 
          class _Pointer, class _Reference>
inline _Distance* 
bc___distance_type (iterator<_Category, _TypeT, _Distance, _Pointer, _Reference>)
{
    return 0;
}

template <class _TypeT>
inline _RWSTD_PTRDIFF_T* bc___distance_type (const _TypeT*)
{ 
    return 0;
}

#endif   // _RWSTD_NO_CLASS_PARTIAL_SPEC

// 24.3.4 - Iterator operations

template <class _InputIterator, class _Distance>
inline void bc___advance (_InputIterator &__it, _Distance __n, std::input_iterator_tag)
{
    _RWSTD_ASSERT (__n == 0 || __n > 0);

    while (__n > 0) {
        --__n;
        ++__it;
    }
}


template <class _ForwardIterator, class _Distance>
inline void bc___advance (_ForwardIterator &__it, _Distance __n,
                       std::forward_iterator_tag)
{
    bc___advance (__it, __n, std::input_iterator_tag ());
}


template <class _BidirectionalIterator, class _Distance>
inline void bc___advance (_BidirectionalIterator &__it, _Distance __n, 
                       std::bidirectional_iterator_tag)
{
    if (__n > 0)
        bc___advance (__it, __n, std::input_iterator_tag ());
    else
        while (__n) {
            ++__n;
            --__it;
        }
}


template <class _RandomAccessIterator, class _Distance>
inline void bc___advance (_RandomAccessIterator& __it, _Distance __n, 
                       std::random_access_iterator_tag)
{
    __it += __n;
}


template <class _InputIterator, class _Distance>
inline void bc___distance (const _InputIterator &__first,
                        const _InputIterator &__last,
                        _Distance            &__n,
                        std::input_iterator_tag)
{
    for (_InputIterator __it = __first; !(__it == __last); ++__it)
        ++__n;
}


template <class _ForwardIterator, class _Distance>
inline void bc___distance (const _ForwardIterator &__first,
                        const _ForwardIterator &__last,
                        _Distance              &__n,
                        std::forward_iterator_tag)
{
    bc___distance (__first, __last, __n, std::input_iterator_tag ());
}

template <class _BidirectionalIterator, class _Distance>
inline void bc___distance (const _BidirectionalIterator &__first,
                        const _BidirectionalIterator &__last, 
                        _Distance                    &__n,
                        std::bidirectional_iterator_tag)
{
    bc___distance (__first, __last, __n, std::input_iterator_tag ());
}


template <class _RandomAccessIterator, class _Distance>
inline void bc___distance (const _RandomAccessIterator &__first,
                        const _RandomAccessIterator &__last, 
                        _Distance                   &__n,
                        std::random_access_iterator_tag)
{
    __n = __last - __first;
}


//#if    !defined (_RWSTD_NO_EXT_VOID_DISTANCE) \
//    || defined (_RWSTD_NO_CLASS_PARTIAL_SPEC)
//
//template <class _ForwardIterator, class _Distance>
//inline void distance (const _ForwardIterator &__first,
//                      const _ForwardIterator &__last,
//                      _Distance              &__n)
//{
//    bc___distance (__first, __last, __n,
//                _RWSTD_ITERATOR_CATEGORY (_ForwardIterator, __first));
//}
//
//#endif   // !_RWSTD_NO_EXT_VOID_DISTANCE || _RWSTD_NO_CLASS_PARTIAL_SPEC

// *** BENTLEY_CHANGE
NAMESPACE_BENTLEY_BSTDCXX_END


NAMESPACE_BENTLEY_BC__RW_BEGIN  //  BENTLEY_CHANGE

// __rw_distance: Same purpose as 3-parameter distance function, but
// with return value.

template <class _ForwardIterator, class _Distance>
inline _Distance
__rw_distance (const _ForwardIterator &__first,
               const _ForwardIterator &__last,
               _Distance               __n)
{
    BENTLEY_BSTDCXX::bc___distance (__first, __last, __n,
                      _RWSTD_ITERATOR_CATEGORY (_ForwardIterator, __first));
    return __n;
}


NAMESPACE_BENTLEY_BC__RW_END  // BENTLEY_CHANGE


#ifndef _RWSTD_NO_DEBUG_ITER

NAMESPACE_BENTLEY_BC__RW_BEGIN  //  BENTLEY_CHANGE

// __rw_debug_iter - iterator adapter with debugging support
// _Iterator is either iterator or const_iterator; if the latter,
// _MutableIterator should be iterator to allow for implicit
// conversions from non-const (mutable) to const_iterator objects


template <class _Container, class _Iterator, class _MutableIterator>
class __rw_debug_iter
{
    typedef _Container                                container_type;
    typedef _Iterator                                 iterator_type;
    typedef std::iterator_traits<iterator_type>      traits_type;

public:

    typedef typename traits_type::value_type         value_type;
    typedef typename traits_type::difference_type    difference_type;
    typedef typename traits_type::reference          reference;
    typedef typename traits_type::pointer            pointer;
    typedef typename traits_type::iterator_category  iterator_category;

    typedef __rw_debug_iter <container_type, _MutableIterator,
                             _MutableIterator>        _C_mutable_iterator;

    __rw_debug_iter (): _C_cont (0) { }

    __rw_debug_iter (const container_type &__cont, const iterator_type &__it)
        : _C_iter (__it), _C_cont (&__cont) { }

    // no copy ctor other than the one below is defined
    // will use a compiler generated one if _Iterator != _MutableIterator
    __rw_debug_iter (const _C_mutable_iterator &__rhs)
        : _C_iter (__rhs._C_iter), _C_cont (__rhs._C_cont) { }

    __rw_debug_iter& operator= (const __rw_debug_iter &__rhs) {
        _C_iter = __rhs._C_iter;
        _C_cont = __rhs._C_cont;
        return *this;
    }

    reference operator* () const {
        _RWSTD_ASSERT (_C_is_dereferenceable ());
        return *_C_iter;
    }

    reference operator[] (difference_type __n) const {
        _RWSTD_ASSERT ((*this + __n)._C_is_dereferenceable ());
        return _C_iter [__n];
    }

    _RWSTD_OPERATOR_ARROW (pointer operator-> () const);

    __rw_debug_iter& operator++ () {
        _RWSTD_ASSERT (!_C_is_end ());
        return ++_C_iter, *this;
    }

    __rw_debug_iter& operator-- () {
        _RWSTD_ASSERT (!_C_is_begin ());
        return --_C_iter, *this;
    }

    __rw_debug_iter operator++ (int) {
        __rw_debug_iter __tmp = *this;
        return ++*this, __tmp;
    }

    __rw_debug_iter operator-- (int) {
        __rw_debug_iter __tmp = *this;
        return --*this, __tmp;
    }

    __rw_debug_iter& operator+= (difference_type __n) {
        _C_iter += __n;
        _RWSTD_ASSERT (   !(_C_iter < _C_cont->begin ()._C_iter)
                       && !(_C_cont->end ()._C_iter < _C_iter));
        return *this;
    }

    __rw_debug_iter& operator-= (difference_type __n) {
        _C_iter -= __n;
        _RWSTD_ASSERT (   !(_C_iter < _C_cont->begin ()._C_iter)
                       && !(_C_cont->end ()._C_iter < _C_iter));
        return *this;
    }

    __rw_debug_iter operator+ (difference_type __n) const {
        return __rw_debug_iter (*this) += __n;
    }

    __rw_debug_iter operator- (difference_type __n) const {
        return __rw_debug_iter (*this) -= __n;
    }

    bool _C_is_begin () const {
        return _C_cont && _C_cont->begin () == *this;
    }

    bool _C_is_end () const {
        return _C_cont && _C_cont->end () == *this;
    }

    bool _C_is_dereferenceable () const {
        return !_C_is_end ();
    }

    bool _C_valid_range (const __rw_debug_iter &__it) const {
        return _C_cont && _C_cont == __it._C_cont;
    }

    const iterator_type& base () const {
        return _C_iter;
    }

    iterator_type& base () {
        return _C_iter;
    }

#if !defined (__IBMCPP__) || __IBMCPP__ > 502

    // IBM xlC 5.0 fails to find these member template operators,
    // yet it complains about ambiguity if they are defined along
    // with the non-members below...

    // operators are templatized to assure const/non-const symmetry

    template <class _Iter>
    difference_type
    operator- (const __rw_debug_iter<container_type, _Iter,
                                     _MutableIterator> &__rhs) const {
        _RWSTD_ASSERT (_C_cont && _C_cont == __rhs._C_cont);
        return _C_iter - __rhs._C_iter;
    }

    template <class _Iter>
    bool
    operator== (const __rw_debug_iter<container_type, _Iter,
                                      _MutableIterator> &__rhs) const {
        return _C_iter == __rhs._C_iter;
    }
    
    template <class _Iter>
    bool
    operator< (const __rw_debug_iter<container_type, _Iter,
                                     _MutableIterator> &__rhs) const {
        return _C_iter < __rhs._C_iter;
    }

    template <class _Iter>
    bool
    operator!= (const __rw_debug_iter<container_type, _Iter,
                                      _MutableIterator> &__rhs) const {
        return !(_C_iter == __rhs._C_iter);
    }

    template <class _Iter>
    bool
    operator<= (const __rw_debug_iter<container_type, _Iter,
                                      _MutableIterator> &__rhs) const {
        return !(__rhs._C_iter < _C_iter);
    }

    template <class _Iter>
    bool
    operator> (const __rw_debug_iter<container_type, _Iter,
                                     _MutableIterator> &__rhs) const {
        return __rhs._C_iter < _C_iter;
    }

    template <class _Iter>
    bool
    operator>= (const __rw_debug_iter<container_type, _Iter,
                                      _MutableIterator> &__rhs) const {
        return !(_C_iter < __rhs._C_iter);
    }

#endif   // __IBMCPP__ > 502

    iterator_type         _C_iter;   // wrapped iterator
    const container_type *_C_cont;   // associated container
};


NAMESPACE_BENTLEY_BC__RW_END  // BENTLEY_CHANGE


// *** BENTLEY_CHANGE
NAMESPACE_BENTLEY_BSTDCXX_BEGIN


#ifndef _RWSTD_NO_NONDEDUCED_CONTEXT
# define _RWSTD_CONT_DIFF_TYPE typename _Cont::difference_type 
#else
# define _RWSTD_CONT_DIFF_TYPE _RWSTD_PTRDIFF_T 
#endif

template <class _Cont, class _Iter, class _MutIter>
inline BC__RW::__rw_debug_iter<_Cont, _Iter, _MutIter> 
operator+ (_RWSTD_CONT_DIFF_TYPE                               __n,
           const BC__RW::__rw_debug_iter<_Cont, _Iter, _MutIter> &__x)
{
    return __x + __n;
}

#undef _RWSTD_CONT_DIFF_TYPE 


#if defined (__IBMCPP__) && __IBMCPP__ <= 502

// IBM xlC 5.0 fails to find the member template operators
// defined above in the presence of namespaces...

// with no support for member templates namespace-scope (non-member)
// operators must be used - these will cause ambiguities with those
// in std::rel_ops if the latter are found during lookup


// _Iter1 may differ from _Iter2 if the function operands are const
// and non-const iterators, respectively (allows symmetry)

template <class _Cont, class _Iter1, class _Iter2, class _MutIter>
inline typename _Cont::difference_type
operator- (const BC__RW::__rw_debug_iter<_Cont, _Iter1, _MutIter> &__x,
           const BC__RW::__rw_debug_iter<_Cont, _Iter2, _MutIter> &__y)
{
    _RWSTD_ASSERT (__x._C_cont && __x._C_cont == __y._C_cont);
    return __x._C_iter - __y._C_iter;
}
    
template <class _Cont, class _Iter1, class _Iter2, class _MutIter>
inline bool
operator== (const BC__RW::__rw_debug_iter<_Cont, _Iter1, _MutIter> &__x,
            const BC__RW::__rw_debug_iter<_Cont, _Iter2, _MutIter> &__y)
{
    return __x._C_iter == __y._C_iter;
}

template <class _Cont, class _Iter1, class _Iter2, class _MutIter>
inline bool
operator< (const BC__RW::__rw_debug_iter<_Cont, _Iter1, _MutIter> &__x,
           const BC__RW::__rw_debug_iter<_Cont, _Iter2, _MutIter> &__y)
{
    _RWSTD_ASSERT (__x._C_cont && __x._C_cont == __y._C_cont);
    return __x._C_iter < __y._C_iter;
}

template <class _Cont, class _Iter1, class _Iter2, class _MutIter>
inline bool
operator!= (const BC__RW::__rw_debug_iter<_Cont, _Iter1, _MutIter> &__x,
            const BC__RW::__rw_debug_iter<_Cont, _Iter2, _MutIter> &__y)
{
    return !(__x == __y);
}

template <class _Cont, class _Iter1, class _Iter2, class _MutIter>
inline bool
operator<= (const BC__RW::__rw_debug_iter<_Cont, _Iter1, _MutIter> &__x,
            const BC__RW::__rw_debug_iter<_Cont, _Iter2, _MutIter> &__y)
{
    return !(__y < __x);
}

template <class _Cont, class _Iter1, class _Iter2, class _MutIter>
inline bool
operator>= (const BC__RW::__rw_debug_iter<_Cont, _Iter1, _MutIter> &__x,
            const BC__RW::__rw_debug_iter<_Cont, _Iter2, _MutIter> &__y)
{
    return !(__x < __y);
}

template <class _Cont, class _Iter1, class _Iter2, class _MutIter>
inline bool
operator> (const BC__RW::__rw_debug_iter<_Cont, _Iter1, _MutIter> &__x,
           const BC__RW::__rw_debug_iter<_Cont, _Iter2, _MutIter> &__y)
{
    return __y < __x;
}

#endif   // __IBMCPP__ <= 502

// *** BENTLEY_CHANGE
NAMESPACE_BENTLEY_BSTDCXX_END


NAMESPACE_BENTLEY_BC__RW_BEGIN  //  BENTLEY_CHANGE


#define _RWSTD_DEBUG_ITER(cont, it, mutit) __rw_debug_iter< cont, it, mutit >


template <class _Cont, class _Iter, class _MutIter>
inline bool
__rw_valid_range (const _RWSTD_DEBUG_ITER(_Cont, _Iter, _MutIter) &__first,
                  const _RWSTD_DEBUG_ITER(_Cont, _Iter, _MutIter) &__last)
{
    return __first._C_cont && __first._C_cont == __last._C_cont;
}


template <class _Iterator>
inline bool
__rw_valid_range (const _Iterator &, const _Iterator &)
{
    return true;
}


template <class _Cont, class _Iter, class _MutIter>
inline bool
__rw_in_range (const _RWSTD_DEBUG_ITER(_Cont, _Iter, _MutIter) &__it,
               const _RWSTD_DEBUG_ITER(_Cont, _Iter, _MutIter) &__first,
               const _RWSTD_DEBUG_ITER(_Cont, _Iter, _MutIter) &__last)
{
    return    __rw_valid_range (__first, __it)
           && __rw_valid_range (__it, __last);
}


template <class _Iterator>
inline bool
__rw_in_range (const _Iterator&, const _Iterator&, const _Iterator&)
{
    return true;
}


template <class _Cont, class _Iter, class _MutIter>
inline bool
__rw_dereferenceable (const _RWSTD_DEBUG_ITER(_Cont, _Iter, _MutIter) &__it)
{
    return __it._C_is_dereferenceable ();
}


template <class _Iterator>
inline bool
__rw_dereferenceable (const _Iterator&)
{
    return true;
}


template <class _TypeT>
inline bool
__rw_dereferenceable (const _TypeT *__ptr)
{
    return 0 != __ptr;
}

NAMESPACE_BENTLEY_BC__RW_END  // BENTLEY_CHANGE

#undef _RWSTD_DEBUG_ITER

#endif   // _RWSTD_NO_DEBUG_ITER


#endif   // _RWSTD_RW_ITERBASE_H_INCLUDED


/// @endcond