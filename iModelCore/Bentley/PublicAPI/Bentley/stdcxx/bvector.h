/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/stdcxx/bvector.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

// -*- C++ -*-
/***************************************************************************
 *
 * <bvector> - dfefinition of the C++ Standard Library bvector class template
 *
 * $Id: bvector 681836 2008-08-01 21:26:37Z vitek $
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
 **************************************************************************
 *
 * NOTICE: This File contains modifications made by Bentley Systems Inc. where designated.
 *
 *************************************************************************/

#ifndef _RWSTD_VECTOR_INCLUDED
#define _RWSTD_VECTOR_INCLUDED

// Note: Bentley bvector<bool> is just a vector of 1-byte bool values. It is not specialized. 

// *** BENTLEY_CHANGE
//#include <rw/_algobase.h>
//#include <rw/_allocator.h>
//#include <rw/_error.h>
#include <iterator>
//#include <rw/_iterator.h>
#include <Bentley/stdcxx/rw/_defs.h>
#include <Bentley/stdcxx/rw/_select.h>
#include <Bentley/stdcxx/rw/_specialized.h>
#include <Bentley/BentleyAllocator.h>


// *** BENTLEY_CHANGE
NAMESPACE_BENTLEY_BSTDCXX_BEGIN

_EXPORT
template <class _TypeT, class _Allocator = ::BentleyApi::BentleyAllocator<_TypeT> >
class bvector;


//=======================================================================================
/**
A Bentley supplied implementation @c std::vector. This class is used in the Bentley APIs to 
avoid dependencies on compiler-supplied implementations of @c std::vector that sometimes vary
with compiler settings or compiler versions. The @c bvector class does not
suffer from these problems. This makes @c bvector suitable for use in Bentley 
public APIs.
<p>
To understand the @c bvector class, consult the documentation for @c std::vector.
@see http://www.cplusplus.com/reference/vector/vector/
@ingroup BeCollectionsGroup
*/
//=======================================================================================
_EXPORT
template <class _TypeT, class _Allocator >
class bvector
{
public:

    typedef _TypeT                                     value_type;
    typedef _Allocator                                 allocator_type;
    typedef typename allocator_type::size_type        size_type;
    typedef typename allocator_type::difference_type  difference_type;
    typedef typename allocator_type::reference        reference;
    typedef typename allocator_type::const_reference  const_reference;
    typedef typename allocator_type::pointer          pointer;
    typedef typename allocator_type::const_pointer    const_pointer;

public:

#ifndef _RWSTD_NO_DEBUG_ITER

    typedef BC__RW::__rw_debug_iter <bvector, pointer, pointer>  iterator;
    
    typedef BC__RW::__rw_debug_iter <bvector, const_pointer, pointer>
        const_iterator;

    iterator _C_make_iter (pointer __ptr) {
        return iterator (*this, __ptr);
    }

    const_iterator _C_make_iter (pointer __ptr) const {
        return const_iterator (*this, __ptr);
    }

#else   // if defined (_RWSTD_NO_DEBUG_ITER)

    typedef pointer         iterator;
    typedef const_pointer   const_iterator;

  #if !defined (DOCUMENTATION_GENERATOR)
    iterator _C_make_iter (pointer __ptr) {
        return __ptr;
    }

    const_iterator _C_make_iter (const_pointer __ptr) const {
        return __ptr;
    }
  #endif // DOCUMENTATION_GENERATOR

#endif  // _RWSTD_NO_DEBUG_ITER

#ifndef _RWSTD_NO_CLASS_PARTIAL_SPEC 

    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef std::reverse_iterator<iterator>       reverse_iterator;

#else   // if defined (_RWSTD_NO_CLASS_PARTIAL_SPEC)

    typedef std::reverse_iterator<const_iterator, 
        std::random_access_iterator_tag, value_type, 
        const_reference, const_pointer, difference_type>
    const_reverse_iterator;

    typedef std::reverse_iterator<iterator, 
        std::random_access_iterator_tag, value_type,
        reference, pointer, difference_type>
    reverse_iterator;

#endif   // _RWSTD_NO_CLASS_PARTIAL_SPEC 

public:

    explicit
    bvector (const allocator_type &__alloc = allocator_type ())
        : _C_alloc (__alloc) { }

    explicit
    bvector (size_type __n, const_reference __x = value_type (),
            const allocator_type &__alloc = allocator_type ())
        : _C_alloc (__alloc) {
        assign (__n, __x);
    }

    template <class _InputIter>
    bvector (_InputIter __first, _InputIter __last,
            const allocator_type &__alloc = allocator_type ())
        : _C_alloc (__alloc) {
        assign (__first, __last);
    }

    bvector (const bvector &__rhs)
        : _C_alloc (__rhs.get_allocator ()) {
        assign (__rhs.begin (), __rhs.end ());
    }
    

// *** BENTLEY_CHANGE
    bvector (bvector&& rhs) : _C_alloc ((allocator_type const&)(rhs._C_alloc))
        {
        *this = std::move (rhs);    // invokes my operator=(&&)
        }

// *** BENTLEY_CHANGE
    bvector& operator=(bvector&& rhs)
        {
        this->swap (rhs);
        return *this;
        }

// *** BENTLEY_CHANGE
// Restrict use of std::initializer_list to VC12 or later.
#if !defined(_MSC_VER) || (_MSC_VER >= 1800)      
    bvector(std::initializer_list<value_type> _Ilist,
                    const allocator_type& _Al = allocator_type())
                    : _C_alloc(_Al)
                    {              // construct from initializer_list
                    insert(begin(), _Ilist.begin(), _Ilist.end());
                    }

// *** BENTLEY_CHANGE
    bvector& operator=(std::initializer_list<value_type> _Ilist)
                    {              // assign initializer_list
                    assign(_Ilist.begin(), _Ilist.end());
                    return (*this);
                    }

// *** BENTLEY_CHANGE
    void assign(std::initializer_list<value_type> _Ilist)
                    {              // assign initializer_list
                    assign(_Ilist.begin(), _Ilist.end());
                    }

// *** BENTLEY_CHANGE
    void insert(iterator _Where,
                    std::initializer_list<value_type> _Ilist)
                    {              // insert initializer_list
                    insert(_Where, _Ilist.begin(), _Ilist.end());
                    }
#endif

    ~bvector () { 
        _C_destroy (begin ());
        _C_alloc.deallocate (_C_alloc._C_begin,
                             _C_alloc._C_bufend - _C_alloc._C_begin);
    }
    
    bvector& operator= (const bvector&);
    
    template <class _InputIter>
    void assign (_InputIter __first, _InputIter __last) {
        // dispatch either to a range assign or to an assign with repetition
        _C_assign (__first, __last, _RWSTD_DISPATCH (_InputIter));
    }

    void assign (size_type __n, const_reference __x) {
        _C_assign_n (__n, __x);
    }

    allocator_type get_allocator () const {
        return _C_alloc;
    }
    
    iterator begin () {
        return _C_make_iter (_C_alloc._C_begin);
    }

    const_iterator begin () const {
        return _C_make_iter (_C_alloc._C_begin);
    }

    iterator end () {
        return _C_make_iter (_C_alloc._C_end);
    }

    const_iterator end () const {
        return _C_make_iter (_C_alloc._C_end);
    }
    
    reverse_iterator rbegin () { 
        return reverse_iterator (end ());
    }
    
    const_reverse_iterator rbegin () const { 
        return const_reverse_iterator (end ());
    }
    
    reverse_iterator rend () { 
        return reverse_iterator (begin ());
    }
    
    const_reverse_iterator rend () const { 
        return const_reverse_iterator (begin ());
    }

    size_type size () const {
        return size_type (_C_alloc._C_end - _C_alloc._C_begin);
    }

    size_type max_size () const {
        return _C_alloc.max_size ();
    }
    
    void resize (size_type, const_reference = value_type ());

    size_type capacity () const {
        return _C_alloc._C_bufend - _C_alloc._C_begin;
    }
    
    bool empty () const {
        return _C_alloc._C_begin == _C_alloc._C_end;
    }
    
    void reserve (size_type);

    reference operator[] (size_type);
  
    const_reference operator[] (size_type) const;
  
    reference at (size_type);

    const_reference at (size_type __n)  const;

    reference front () {
        _RWSTD_ASSERT (!empty ());
        return *begin ();
    }
    
    const_reference front () const {
        _RWSTD_ASSERT (!empty ());
        return *begin ();
    }
    
// *** BENTLEY_CHANGE
    pointer data() {return &front();}
    const_pointer data() const {return &front();}
    
    reference back () {
        _RWSTD_ASSERT (!empty ());
        return *(end () - 1);
    }
    
    const_reference back () const {
        _RWSTD_ASSERT (!empty ());
        return *(end () - 1);
    }

    void push_back (const_reference);
    
    void pop_back () {
        _RWSTD_ASSERT (!empty ());
        _C_alloc.destroy (_C_alloc._C_end - 1);
        --_C_alloc._C_end;
    }

    iterator insert (iterator, const_reference);

    template <class _InputIter>
    void insert (iterator __it, _InputIter __first, _InputIter __last) {
        _C_insert (__it, __first, __last, _RWSTD_DISPATCH (_InputIter));
    }

    void insert (iterator __it, size_type __n, const_reference __x) {
        _C_insert_n (__it, __n, __x);
    }
    
    iterator erase (iterator);

    iterator erase (iterator, iterator);

    void swap (bvector&);
    
    void clear ();

#ifdef _RWSTD_NO_PART_SPEC_OVERLOAD

    friend void swap (bvector& __lhs, bvector& __rhs) {
        __lhs.swap (__rhs);
    }

#endif   // _RWSTD_NO_PART_SPEC_OVERLOAD

private:

    // implements assign with repetition
    void _C_assign_n (size_type, const_reference);

    // implements a single-element insert
    void _C_insert_1 (const iterator&, const_reference);

    // implements insert with repetition
    void _C_insert_n (const iterator&, size_type, const_reference);

    // implements range insert for ForwardIterators
    template <class _FwdIter>
    void _C_insert_range (iterator, _FwdIter, _FwdIter,
                          std::forward_iterator_tag);

    // implements range insert for InputIterators
    template <class _InputIter>
    void _C_insert_range (iterator, _InputIter, _InputIter,
                          std::input_iterator_tag);

    // implements range assign
    template <class _InputIter>
    void _C_assign (_InputIter __first, _InputIter __last, void*) {
        _RWSTD_ASSERT_RANGE (__first, __last);

        // dispatch to an assign suitable for the category of InputIter
        _RWSTD_ASSIGN_RANGE (__first, __last,
                             _RWSTD_ITERATOR_CATEGORY (_InputIter, __first));
    }

    // implements assign with repetition if value_type == size_type
    template <class _IntType>
    void _C_assign (_IntType __n, _IntType __x, int) {
        _C_assign_n (size_type (__n), __x);
    }

    // implements range insert for ForwardIterators
    template <class _FwdIter>
    void _C_assign_range (_FwdIter, _FwdIter, std::forward_iterator_tag);

    // implements range insert for InputIterators
    template <class _InputIter>
    void _C_assign_range (_InputIter, _InputIter, std::input_iterator_tag);

    // implements range insert
    template <class _InputIter>
    void _C_insert (const iterator &__it,
                   _InputIter __first, _InputIter __last, void*) {
        _RWSTD_ASSERT_RANGE (begin (), __it);
        _RWSTD_ASSERT_RANGE (__first, __last);

        // dispatch to an insert suitable for the category of InputIter
        _RWSTD_INSERT_RANGE (__it, __first, __last,
                             _RWSTD_ITERATOR_CATEGORY (_InputIter, __first));
    }

    // implements insert with repetition if value_type == size_type
    template <class _IntType>
    void _C_insert (const iterator &__it,
                    _IntType __n, _IntType __x, int) {
        _C_insert_n (__it, size_type (__n), __x);
    }

    void _C_realloc (size_type);

    // constructs a copy at the end and grows the size of container
    void _C_push_back (const_reference __x) {
        _RWSTD_ASSERT (_C_alloc._C_end != _C_alloc._C_bufend);
        _C_alloc.construct (_C_alloc._C_end, __x);
        ++_C_alloc._C_end;
    }

    // destroys elements from the iterator to the end of the bvector
    // and resets end() to point to the iterator
    void _C_destroy (iterator);

    // implements swap for objects with unequal allocator
    void _C_unsafe_swap (bvector&);

    struct _C_VectorAlloc: allocator_type {

        _C_VectorAlloc (const allocator_type &__alloc)
            : allocator_type (__alloc), _C_begin (), _C_end (), _C_bufend ()
            { /* empty */}

        pointer _C_begin;
        pointer _C_end;
        pointer _C_bufend;
    } _C_alloc;
};

#if !defined (DOCUMENTATION_GENERATOR)
template <class _TypeT, class _Allocator>
inline typename bvector<_TypeT, _Allocator>::reference
bvector<_TypeT, _Allocator>::
operator[] (size_type __n)
{
#ifdef _RWSTD_BOUNDS_CHECKING

    _RWSTD_REQUIRES (__n < size (),
                     std::out_of_range, (_RWSTD_ERROR_OUT_OF_RANGE, 
                      _RWSTD_FUNC ("bvector::operator[](size_type)"),
                      __n, size ()));

#endif   // _RWSTD_BOUNDS_CHECKING

    return *(begin () + __n);
}
  

template <class _TypeT, class _Allocator>
inline typename bvector<_TypeT, _Allocator>::const_reference
bvector<_TypeT, _Allocator>::
operator[] (size_type __n) const
{
#ifdef _RWSTD_BOUNDS_CHECKING

    _RWSTD_REQUIRES (__n < size (),
                     std::out_of_range, (_RWSTD_ERROR_OUT_OF_RANGE, 
                      _RWSTD_FUNC ("bvector::operator[](size_type) const"),
                      __n, size ()));

#endif   // _RWSTD_BOUNDS_CHECKING

    return *(begin () + __n);
}
  

template <class _TypeT, class _Allocator>
inline typename bvector<_TypeT, _Allocator>::reference
bvector<_TypeT, _Allocator>::
at (size_type __n)
{
    _RWSTD_REQUIRES (__n < size (),
                     std::out_of_range, (_RWSTD_ERROR_OUT_OF_RANGE, 
                      _RWSTD_FUNC ("bvector::at (size_type)"),
                      __n, size ()));
    return *(begin () + __n); 
}
    

template <class _TypeT, class _Allocator>
inline typename bvector<_TypeT, _Allocator>::const_reference
bvector<_TypeT, _Allocator>::
at (size_type __n)  const
{
    _RWSTD_REQUIRES (__n < size (),
                     std::out_of_range, (_RWSTD_ERROR_OUT_OF_RANGE, 
                      _RWSTD_FUNC ("bvector::at(size_type) const"),
                      __n, size ()));
    return *(begin () + __n); 
}


template <class _TypeT, class _Allocator>
inline void
bvector<_TypeT, _Allocator>::
resize (size_type __new_size, const_reference __x /* = value_type () */)
{
    if (size () < __new_size)
        insert (end (), __new_size - size (), __x);
    else if (__new_size < size ())
        erase (begin () + __new_size, end ());
}


template <class _TypeT, class _Allocator>
inline void
bvector<_TypeT, _Allocator>::
reserve (size_type __n)
{
    _RWSTD_REQUIRES (__n <= max_size (),
                     std::length_error, (_RWSTD_ERROR_LENGTH_ERROR,
                      _RWSTD_FUNC ("bvector::reserve(size_type)"),
                      __n, max_size ()));

    if (capacity () < __n)
        _C_realloc (__n);
}


template <class _TypeT, class _Allocator>
inline void
bvector<_TypeT, _Allocator>::
push_back (const_reference __x)
{
    if (_C_alloc._C_end == _C_alloc._C_bufend)
        _C_insert_1 (end (), __x);
    else
        _C_push_back (__x);
}


template <class _TypeT, class _Allocator>
inline typename bvector<_TypeT, _Allocator>::iterator
bvector<_TypeT, _Allocator>::
insert (iterator __it, const_reference __x)
{
    _RWSTD_ASSERT_RANGE (__it, end ());

    const difference_type __off = __it - begin ();

    if (end () == __it)
        push_back (__x);
    else
        _C_insert_1 (__it, __x);

    return begin () + __off;
}


template <class _TypeT, class _Allocator>
inline typename bvector<_TypeT, _Allocator>::iterator
bvector<_TypeT, _Allocator>::
erase (iterator __it)
{
    _RWSTD_ASSERT_RANGE (__it, end ());
    _RWSTD_ASSERT (__it < end ());   // `it' must be dereferenceable

    const iterator __next = __it + 1;

    if (__next != end ()) 
        std::copy (__next, end (), __it);

    _C_alloc.destroy (_C_alloc._C_end - 1);
    --_C_alloc._C_end;

    return __it;
}


template <class _TypeT, class _Allocator>
inline typename bvector<_TypeT, _Allocator>::iterator
bvector<_TypeT, _Allocator>::
erase (iterator __first, iterator __last)
{
    _RWSTD_ASSERT_RANGE (__first, __last);
    _RWSTD_ASSERT_RANGE (begin (), __first);

    _C_destroy (std::copy (__last, end (), __first));

    return __first;
}


template <class _TypeT, class _Allocator>
inline void
bvector<_TypeT, _Allocator>::
clear ()
{
    if (!empty ())
        _C_destroy (begin ());
}


template <class _TypeT, class _Allocator>
inline void
bvector<_TypeT, _Allocator>::
swap (bvector &__other)
{
    if (get_allocator () == __other.get_allocator ()) {
        pointer __tmp             = _C_alloc._C_begin;
        _C_alloc._C_begin         = __other._C_alloc._C_begin;
        __other._C_alloc._C_begin = __tmp;
        __tmp                     = _C_alloc._C_end;
        _C_alloc._C_end           = __other._C_alloc._C_end;
        __other._C_alloc._C_end   = __tmp;
        __tmp                     = _C_alloc._C_bufend;
        _C_alloc._C_bufend        = __other._C_alloc._C_bufend;
        __other._C_alloc._C_bufend = __tmp;
    }
    else {
        // not exception-safe
        _C_unsafe_swap (__other);
    }
}


template <class _TypeT, class _Allocator>
inline bool
operator== (const bvector<_TypeT, _Allocator> &__x,
            const bvector<_TypeT, _Allocator> &__y)
{
    return __x.size () == __y.size ()
        && std::equal(__x.begin (), __x.end (), __y.begin ());
}

template <class _TypeT, class _Allocator>
inline bool
operator< (const bvector<_TypeT, _Allocator> &__x,
           const bvector<_TypeT, _Allocator> &__y)
{
    return std::lexicographical_compare (__x.begin (), __x.end (),
                                          __y.begin (), __y.end ());
}

template <class _TypeT, class _Allocator>
inline bool
operator!= (const bvector<_TypeT, _Allocator> &__x,
            const bvector<_TypeT, _Allocator> &__y)
{
    return !(__x == __y);
}

template <class _TypeT, class _Allocator>
inline bool
operator> (const bvector<_TypeT, _Allocator> &__x,
           const bvector<_TypeT, _Allocator> &__y)
{
    return __y < __x;
}

template <class _TypeT, class _Allocator>
inline bool
operator>= (const bvector<_TypeT, _Allocator> &__x,
            const bvector<_TypeT, _Allocator> &__y)
{
    return !(__x < __y);
}

template <class _TypeT, class _Allocator>
inline bool
operator<= (const bvector<_TypeT, _Allocator> &__x,
            const bvector<_TypeT, _Allocator> &__y)
{
    return !(__y <  __x);
}

#ifndef _RWSTD_NO_PART_SPEC_OVERLOAD
template <class _TypeT, class _Allocator>
inline void
swap (bvector<_TypeT, _Allocator> &__x, bvector<_TypeT, _Allocator> &__y)
{
    __x.swap (__y);
}
#endif  // _RWSTD_NO_PART_SPEC_OVERLOAD

#endif  // DOCUMENTATION_GENERATOR

// *** BENTLEY_CHANGE
NAMESPACE_BENTLEY_BSTDCXX_END

#if !defined (DOCUMENTATION_GENERATOR)
    #if defined (_RWSTD_NO_IMPLICIT_INCLUSION)
        #include "bvector_cc.h"
    #endif
#endif

#endif   // _RWSTD_VECTOR_INCLUDED
