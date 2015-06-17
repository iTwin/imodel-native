/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/stdcxx/bstdmap.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

// -*- C++ -*-
/***************************************************************************
 *
 * <bstdmap> - definition of the C++ Standard Library bstdmap class template
 *
 * $Id: bstdmap 681820 2008-08-01 20:51:17Z vitek $
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
 * Copyright 1994-2006 Rogue Wave Software.
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
 * NOTICE: This File contains modifications made by Bentley Systems where designated.
 *
 *************************************************************************/

#ifndef _RWSTD_MAP_INCLUDED
#define _RWSTD_MAP_INCLUDED


// *** BENTLEY_CHANGE
#include <Bentley/BentleyAllocator.h>  // for allocator
#include <Bentley/stdcxx/rw/bc___rb_tree.h>       // for bc___rb_tree
#include <Bentley/stdcxx/rw/bpair.h>
#include <Bentley/stdcxx/rw/_defs.h>
#include <functional>

#if !defined (DOCUMENTATION_GENERATOR)

BC__RWSTD_NAMESPACE (BC__RW) { 
// This is used in the implementation of bstdmap and bmultimap.
template <class _TypeT, class _TypeU>
struct __select1st : public std::unary_function<_TypeT, _TypeU>
{
    const _TypeU& operator () (const _TypeT& __x) const {
        return __x.first;
    }
};

}   // namespace BC__RW

#endif // DOCUMENTATION_GENERATOR

// *** BENTLEY_CHANGE
NAMESPACE_BENTLEY_BSTDCXX_BEGIN

//=======================================================================================
/**
A %Bentley supplied implementation @c std::map. This class is used in the %Bentley APIs to 
avoid dependencies on compiler-supplied implementations of @c std::map that sometimes vary
with compiler settings or compiler versions. The @c bstdmap class does not
suffer from these problems. This makes @c bstdmap suitable for use in %Bentley 
public APIs.
<p>
There is also another %Bentley supplied implementation of std::map called bmap. bmap is generally
the preferable std::map implementation for most use cases, but it is not a 100% compatible 
implementation of std::map (see discussion therein). If you cannot live with bmap's restrictions,
you should use bstdmap.
<p>
To understand the @c bstdmap class, consult the documentation for @c std::map.
@see http://www.cplusplus.com/reference/map/map/
@ingroup BeCollectionsGroup
*/
//=======================================================================================
template <class _Key, class _TypeT,
          class _Compare = std::less<_Key>,
          class _Allocator = BentleyApi::BentleyAllocator<bpair<const _Key, _TypeT> > >
class bstdmap
{
public:

    typedef _Key                                key_type;
    typedef _TypeT                              mapped_type;
    typedef bpair<const key_type, mapped_type>   value_type;
    typedef _Compare                            key_compare;
    typedef _Allocator                          allocator_type;
    
private:
    
    typedef BC__RW::bc___rb_tree<key_type, value_type,
                           BC__RW::__select1st<value_type, key_type>, 
                           key_compare, allocator_type> _C_repT;
    _C_repT _C_rep;

public:

    typedef typename _C_repT::reference              reference;
    typedef typename _C_repT::const_reference        const_reference;
    typedef typename _C_repT::iterator               iterator;
    typedef typename _C_repT::const_iterator         const_iterator;
    typedef typename _C_repT::size_type              size_type;
    typedef typename _C_repT::difference_type        difference_type;
    typedef typename _C_repT::pointer                pointer;
    typedef typename _C_repT::const_pointer          const_pointer;
    typedef typename _C_repT::reverse_iterator       reverse_iterator;
    typedef typename _C_repT::const_reverse_iterator const_reverse_iterator;

#if !defined (DOCUMENTATION_GENERATOR)
    struct value_compare: std::binary_function<value_type, value_type, bool> {
        // explicitly specified template arg-list below
        // to work around an MSVC 6.0 bug (PR #26908)
        friend class bstdmap<_Key, _TypeT, _Compare, _Allocator>;

        bool operator () (const value_type &__x, const value_type &__y) const {
            return comp (__x.first, __y.first);
        }
#endif

    protected:

        key_compare comp;

        value_compare (key_compare __cmp) : comp (__cmp) { /* empty */ }
    };

    // 23.3.1.1, p1
    explicit
    bstdmap (const key_compare &__cmp   = key_compare (),
         const allocator_type &__alloc = allocator_type ()) 
        : _C_rep (__cmp, __alloc) { }

    // 23.3.1.1, p3
    template <class _InputIter>
    bstdmap (_InputIter __first, _InputIter __last, 
         const _Compare& __cmp = _Compare (),
         const _Allocator& __alloc = _Allocator ())
      : _C_rep (__first, __last, __cmp, __alloc, false) { }

    bstdmap (const bstdmap &__rhs): _C_rep (__rhs._C_rep) { /* empty */ }

    bstdmap& operator= (const bstdmap &__rhs) {
        _C_rep = __rhs._C_rep; return *this; 
    }

    allocator_type get_allocator () const {
        return _C_rep.get_allocator ();
    }

    iterator begin () {
        return _C_rep.begin ();
    }

    const_iterator begin () const {
        return _C_rep.begin ();
    }

    iterator end () {
        return _C_rep.end ();
    }

    const_iterator end () const {
        return _C_rep.end ();
    }

    reverse_iterator rbegin () {
        return _C_rep.rbegin ();
    }

    const_reverse_iterator rbegin () const {
        return _C_rep.rbegin ();
    }

    reverse_iterator rend () {
        return _C_rep.rend ();
    }

    const_reverse_iterator rend () const {
        return _C_rep.rend ();
    }

    bool empty () const {
        return _C_rep.empty ();
    }

    size_type size () const {
        return _C_rep.size ();
    }

    size_type max_size () const {
        return _C_rep.max_size ();
    }

    mapped_type& operator[] (const key_type &__k) {
        // note: temporary is necessary to avoid an xlC 5.0 bug (PR #25040)
        iterator __i = insert (value_type (__k, mapped_type ())).first;
        return (*__i).second;
    }

    bpair<iterator, bool> insert (const value_type &__x) {
        return _C_rep.insert (__x, false);
    }

    iterator insert (iterator __it, const value_type &__x) {
        return _C_rep.insert (__it, __x, false);
    }

    template<class _InputIter>
    void insert (_InputIter __first, _InputIter __last) {
        _C_rep.insert (__first, __last, false);
    }

    // BENTLEY_CHANGE
    iterator erase (iterator __it) {
        return _C_rep.erase (__it);
    }

    size_type erase (const key_type& __x) {
        return _C_rep.erase (__x);
    }

    void  erase (iterator __first, iterator __last) {
        _C_rep.erase (__first,__last);
    }
    void swap (bstdmap &__x) {
        _C_rep.swap (__x._C_rep);
    }

    void clear () {
        erase (begin (),end ());
    }

    key_compare key_comp () const {
        return _C_rep.key_comp ();
    }

    value_compare value_comp () const {
        return value_compare (_C_rep.key_comp ());
    }

    iterator find (const key_type& __x) {
        return _C_rep.find (__x);
    }

    const_iterator find (const key_type& __x) const {
        return _C_rep.find (__x);
    }

    size_type count (const key_type& __x) const {
        return _C_rep.count (__x);
    }

    iterator lower_bound (const key_type& __x) {
        return _C_rep.lower_bound (__x);
    }

    iterator upper_bound (const key_type& __x) {
        return _C_rep.upper_bound (__x);
    }

    const_iterator lower_bound (const key_type& __x) const {
        return _C_rep.lower_bound (__x); 
    }

    const_iterator upper_bound (const key_type& __x) const {
        return _C_rep.upper_bound (__x); 
    }

    bpair<iterator, iterator> equal_range (const key_type& __x) {
        return _C_rep.equal_range (__x);
    }

    bpair<const_iterator, const_iterator>
    equal_range (const key_type& __x) const {
        return _C_rep.equal_range (__x);
    }

#if defined (_RWSTD_NO_PART_SPEC_OVERLOAD)
    friend void swap (bstdmap& __lhs, bstdmap& __rhs) {
        __lhs.swap (__rhs);
    }
#endif

};

//! A template that has many of the capabilities of std::multimap
template <class _Key, class _TypeT,
          class _Compare = std::less<_Key>,
          class _Allocator = BentleyApi::BentleyAllocator<bpair<const _Key, _TypeT> > >
class bstdmultimap
{
public:

    typedef _Key                                key_type;
    typedef _TypeT                              mapped_type;
    typedef bpair<const key_type, mapped_type>   value_type;
    typedef _Compare                            key_compare;
    typedef _Allocator                          allocator_type;

private:
    
    typedef BC__RW::bc___rb_tree<key_type, value_type, 
                           BC__RW::__select1st<value_type, key_type>, 
                           key_compare, allocator_type> _C_repT;
    _C_repT _C_rep;

public:

    typedef typename _C_repT::reference              reference;
    typedef typename _C_repT::const_reference        const_reference;
    typedef typename _C_repT::iterator               iterator;
    typedef typename _C_repT::const_iterator         const_iterator; 
    typedef typename _C_repT::size_type              size_type;
    typedef typename _C_repT::difference_type        difference_type;
    typedef typename _C_repT::pointer                pointer;
    typedef typename _C_repT::const_pointer          const_pointer; 
    typedef typename _C_repT::reverse_iterator       reverse_iterator;
    typedef typename _C_repT::const_reverse_iterator const_reverse_iterator;

#if !defined (DOCUMENTATION_GENERATOR)
    struct value_compare: std::binary_function<value_type, value_type, bool> {
        // explicitly specified template arg-list below
        // to work around an MSVC 6.0 bug (PR #26908)
        friend class bstdmultimap<_Key, _TypeT, _Compare, _Allocator>;

        bool operator () (const value_type &__x, const value_type &__y) const {
            return comp (__x.first, __y.first);
        }
#endif

    protected:

        key_compare comp;
        value_compare (key_compare __cmp): comp (__cmp) { /* empty */ }
    };

    explicit
    bstdmultimap (const key_compare &__cmp = key_compare (),
              const allocator_type &__alloc = allocator_type ())
      : _C_rep (__cmp, __alloc) { }

    template<class _InputIter>
    bstdmultimap (_InputIter __first, _InputIter __last, 
              const _Compare& __cmp = _Compare (),
              const _Allocator& __alloc = _Allocator ()) 
      : _C_rep (__first, __last, __cmp, __alloc, true) { }

    bstdmultimap (const bstdmultimap &__rhs)
        : _C_rep (__rhs._C_rep) { }

    bstdmultimap& operator= (const bstdmultimap &__rhs) {
        _C_rep = __rhs._C_rep; return *this; 
    }

    allocator_type get_allocator () const {
        return _C_rep.get_allocator ();
    }

    iterator begin () {
        return _C_rep.begin ();
    }

    const_iterator begin () const {
        return _C_rep.begin ();
    }

    iterator end () {
        return _C_rep.end ();
    }

    const_iterator end () const {
        return _C_rep.end ();
    }

    reverse_iterator rbegin () {
        return _C_rep.rbegin ();
    }

    const_reverse_iterator rbegin () const {
        return _C_rep.rbegin ();
    }

    reverse_iterator rend () {
        return _C_rep.rend ();
    }

    const_reverse_iterator rend () const {
        return _C_rep.rend ();
    }

    bool empty () const {
        return _C_rep.empty ();
    }

    size_type size () const {
        return _C_rep.size ();
    }

    size_type max_size () const {
        return _C_rep.max_size ();
    }

    iterator insert (const value_type& __x) {
        return _C_rep.insert (__x, true).first;
    }

    iterator insert (iterator __it, const value_type& __x) {
        return _C_rep.insert (__it, __x, true);
    }

    template<class _InputIter>
    void insert (_InputIter __first, _InputIter __last) {
        _C_rep.insert (__first, __last, true);
    }

    // BENTLEY_CHANGE
    iterator erase (iterator __it) {
        return _C_rep.erase (__it);
    }

    size_type erase (const key_type& __x) {
        return _C_rep.erase (__x);
    }

    void erase (iterator __first, iterator __last) {
        _C_rep.erase (__first, __last);
    }

    void clear () {
        erase (begin (),end ());
    }

    void swap (bstdmultimap &__x) {
        _C_rep.swap (__x._C_rep);
    }

    key_compare  key_comp () const {
        return _C_rep.key_comp ();
    }

    value_compare value_comp () const {
        return value_compare (_C_rep.key_comp ());
    }

    iterator find (const key_type& __x) {
        return _C_rep.find (__x);
    }

    const_iterator find (const key_type& __x) const {
        return _C_rep.find (__x);
    }

    size_type count (const key_type& __x) const {
        return _C_rep.count (__x);
    }

    iterator lower_bound (const key_type& __x) {
        return _C_rep.lower_bound (__x);
    }

    iterator upper_bound (const key_type& __x) {
        return _C_rep.upper_bound (__x);
    }

    const_iterator lower_bound (const key_type& __x) const {
        return _C_rep.lower_bound (__x); 
    }

    const_iterator upper_bound (const key_type& __x) const {
        return _C_rep.upper_bound (__x); 
    }

    bpair<iterator, iterator>
    equal_range (const key_type& __x) {
        return _C_rep.equal_range (__x);
    }

    bpair<const_iterator, const_iterator>
    equal_range (const key_type& __x) const {
        return _C_rep.equal_range (__x);
    }

#if defined (_RWSTD_NO_PART_SPEC_OVERLOAD)
    friend void swap (bstdmultimap& __lhs, bstdmultimap& __rhs) {
        __lhs.swap (__rhs);
    }
#endif

};

// 23.1, p5 - table 65
template <class _Key, class _TypeT, class _Compare, class _Allocator>
inline bool operator== (const bstdmap<_Key, _TypeT, _Compare, _Allocator> &__x,
                        const bstdmap<_Key, _TypeT, _Compare, _Allocator> &__y)
{
    return    __x.size () == __y.size ()
           && equal (__x.begin (), __x.end (), __y.begin ());
}


// 23.1, p5 - table 65
template <class _Key, class _TypeT, class _Compare, class _Allocator>
inline bool operator< (const bstdmap<_Key, _TypeT, _Compare, _Allocator> &__x, 
                       const bstdmap<_Key, _TypeT, _Compare, _Allocator> &__y)
{
    return lexicographical_compare (__x.begin (), __x.end (),
                                    __y.begin (), __y.end ());
}


// 23.1, p5 - table 65
template <class _Key, class _TypeT, class _Compare, class _Allocator>
inline bool operator!= (const bstdmap<_Key, _TypeT, _Compare, _Allocator> &__x, 
                        const bstdmap<_Key, _TypeT, _Compare, _Allocator> &__y)
{
    return !(__x == __y);
}


// 23.1, p5 - table 65
template <class _Key, class _TypeT, class _Compare, class _Allocator>
inline bool operator> (const bstdmap<_Key, _TypeT, _Compare, _Allocator> &__x, 
                       const bstdmap<_Key, _TypeT, _Compare, _Allocator> &__y)
{
    return __y < __x;
}


// 23.1, p5 - table 65
template <class _Key, class _TypeT, class _Compare, class _Allocator>
inline bool operator>= (const bstdmap<_Key, _TypeT, _Compare, _Allocator> &__x, 
                        const bstdmap<_Key, _TypeT, _Compare, _Allocator> &__y)
{
    return !(__x < __y);
}


// 23.1, p5 - table 65
template <class _Key, class _TypeT, class _Compare, class _Allocator>
inline bool operator<= (const bstdmap<_Key, _TypeT, _Compare, _Allocator> &__x, 
                        const bstdmap<_Key, _TypeT, _Compare, _Allocator> &__y)
{
    return !(__y <  __x);
}


#ifndef _RWSTD_NO_PART_SPEC_OVERLOAD

// 23.3.1.4
template <class _Key, class _TypeT, class _Compare, class _Allocator>
inline void swap (bstdmap<_Key, _TypeT, _Compare, _Allocator> &__y, 
                  bstdmap<_Key, _TypeT, _Compare, _Allocator> &__x)
{
    __x.swap (__y);
}

#endif   // _RWSTD_NO_PART_SPEC_OVERLOAD


// 23.1, p5 - table 65
template <class _Key, class _TypeT, class _Compare, class _Allocator>
inline bool
operator== (const bstdmultimap<_Key, _TypeT, _Compare, _Allocator> &__x, 
            const bstdmultimap<_Key, _TypeT, _Compare, _Allocator> &__y)
{
    return    __x.size () == __y.size ()
           && equal (__x.begin (), __x.end (), __y.begin ());
}


// 23.1, p5 - table 65
template <class _Key, class _TypeT, class _Compare, class _Allocator>
inline bool
operator< (const bstdmultimap<_Key, _TypeT, _Compare, _Allocator> &__x, 
           const bstdmultimap<_Key, _TypeT, _Compare, _Allocator> &__y)
{
    return lexicographical_compare (__x.begin (), __x.end (),
                                    __y.begin (), __y.end ());
}


// 23.1, p5 - table 65
template <class _Key, class _TypeT, class _Compare, class _Allocator>
inline bool
operator!= (const bstdmultimap<_Key, _TypeT, _Compare, _Allocator> &__x, 
            const bstdmultimap<_Key, _TypeT, _Compare, _Allocator> &__y)
{
    return !(__x == __y);
}


// 23.1, p5 - table 65
template <class _Key, class _TypeT, class _Compare, class _Allocator>
inline bool
operator> (const bstdmultimap<_Key, _TypeT, _Compare, _Allocator> &__x, 
           const bstdmultimap<_Key, _TypeT, _Compare, _Allocator> &__y)
{
    return __y < __x;
}


// 23.1, p5 - table 65
template <class _Key, class _TypeT, class _Compare, class _Allocator>
inline bool
operator>= (const bstdmultimap<_Key, _TypeT, _Compare, _Allocator> &__x, 
            const bstdmultimap<_Key, _TypeT, _Compare, _Allocator> &__y)
{
    return !(__x < __y);
}


// 23.1, p5 - table 65
template <class _Key, class _TypeT, class _Compare, class _Allocator>
inline bool
operator<= (const bstdmultimap<_Key, _TypeT, _Compare, _Allocator> &__x, 
            const bstdmultimap<_Key, _TypeT, _Compare, _Allocator> &__y)
{
    return !(__y <  __x);
}


#ifndef _RWSTD_NO_PART_SPEC_OVERLOAD

// 23.3.2.3
template <class _Key, class _TypeT, class _Compare, class _Allocator>
void swap (bstdmultimap<_Key, _TypeT, _Compare, _Allocator> &__x, 
           bstdmultimap<_Key, _TypeT, _Compare, _Allocator> &__y)
{
    __x.swap (__y);
}

#endif   // _RWSTD_NO_PART_SPEC_OVERLOAD

// *** BENTLEY_CHANGE
NAMESPACE_BENTLEY_BSTDCXX_END

#endif   // _RWSTD_MAP_INCLUDED
