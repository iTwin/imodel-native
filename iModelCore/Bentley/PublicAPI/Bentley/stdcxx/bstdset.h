/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/stdcxx/bstdset.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

// -*- C++ -*-
/***************************************************************************
 *
 * <bstdset> - definition of the C++ Standard Library set class template
 *
 * $Id: bstdset 681820 2008-08-01 20:51:17Z vitek $
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
 * NOTICE: This File contains modifications made by Bentley Systems Inc. where designated.
 *
 *************************************************************************/

#ifndef _RWSTD_SET_INCLUDED
#define _RWSTD_SET_INCLUDED

#if !defined (DOCUMENTATION_GENERATOR)

// *** BENTLEY_CHANGE
#include <Bentley/BentleyAllocator.h>  // for allocator
#include <Bentley/stdcxx/rw/bc___rb_tree.h>       // for bc___rb_tree
#include <Bentley/stdcxx/rw/bpair.h>
#include <Bentley/stdcxx/rw/_defs.h>
#include <functional>

NAMESPACE_BENTLEY_BC__RW_BEGIN  //  BENTLEY_CHANGE
template <class _TypeT, class _TypeU>
struct __ident: std::unary_function<_TypeT, _TypeU>
{
    const _TypeU& operator() (const _TypeT& __x) const {
        return __x;
    }
};

NAMESPACE_BENTLEY_BC__RW_END  // BENTLEY_CHANGE

#endif // DOCUMENTATION_GENERATOR

// *** BENTLEY_CHANGE
NAMESPACE_BENTLEY_BSTDCXX_BEGIN

//=======================================================================================
/**
A %Bentley supplied implementation @c std::set. This class is used in the %Bentley APIs to 
avoid dependencies on compiler-supplied implementations of @c std::map that sometimes vary
with compiler settings or compiler versions. The @c bstdset class does not
suffer from these problems. This makes @c bstdset suitable for use in %Bentley 
public APIs.

There is also another %Bentley supplied implementation of std::set called bset. bset is generally
the preferable std::set implementation for most use cases, but it is not a 100% compatible 
implementation of std::set (see discussion therein). If you cannot live with bset's restrictions,
you should use bstdset.
<p>
To understand the @c bstdset class, consult the documentation for @c std::set.
@see http://www.cplusplus.com/reference/set/set/
@ingroup BeCollectionsGroup
*/
//=======================================================================================
template <class _Key,
          class _Compare = std::less<_Key>,
          class _Allocator = ::BentleyApi::BentleyAllocator<_Key> >
class bstdset
{
public:
    //
    // Types
    //
    typedef _Key                key_type;
    typedef _Key                value_type;
    typedef _Compare            key_compare;
    typedef _Compare            value_compare;
    typedef _Allocator          allocator_type;

private:
    
    typedef BC__RW::bc___rb_tree<key_type, value_type, 
                           BC__RW::__ident<value_type, key_type>, 
                           key_compare, allocator_type> __rep_type;
    __rep_type _C_rep;

public:
    //
    // Types
    //
        // Note that iterator and reverse_iterator are typedefed to 
        // const iterators. This is intentional, the purpose is to
        // prevent the modification of a bstdset element after it has
        // been inserted.
    typedef typename __rep_type::reference               reference;
    typedef typename __rep_type::const_reference         const_reference;
    typedef typename __rep_type::const_iterator          iterator;
    typedef typename __rep_type::const_iterator          const_iterator;
    typedef typename __rep_type::size_type               size_type;
    typedef typename __rep_type::difference_type         difference_type;
    typedef typename __rep_type::pointer                 pointer;
    typedef typename __rep_type::const_pointer           const_pointer;
    typedef typename __rep_type::const_reverse_iterator  reverse_iterator;
    typedef typename __rep_type::const_reverse_iterator  const_reverse_iterator;

    //
    // construct/copy/destroy
    //
    explicit
    bstdset (const key_compare    &__cmp   = key_compare (),
         const allocator_type &__alloc = allocator_type ())
        : _C_rep (__cmp, __alloc) { }

    template<class _InputIter>
    bstdset (_InputIter __first, _InputIter __last,
         const key_compare& __cmp = key_compare (),
         const allocator_type& __al = allocator_type ())
        : _C_rep (__first, __last, __cmp, __al, false) { }

    bstdset (const bstdset &__x)
        : _C_rep (__x._C_rep) { }

    bstdset& operator= (const bstdset &__x) {
        _C_rep = __x._C_rep; return *this;
    }

    allocator_type get_allocator () const {
        return _C_rep.get_allocator ();
    }

    //
    // iterators
    //
    iterator                 begin  ()       { return _C_rep.begin ();  }
    const_iterator           begin  () const { return _C_rep.begin ();  }
    iterator                 end    ()       { return _C_rep.end ();    }
    const_iterator           end    () const { return _C_rep.end ();    }
    reverse_iterator         rbegin ()       { return _C_rep.rbegin (); } 
    const_reverse_iterator   rbegin () const { return _C_rep.rbegin (); } 
    reverse_iterator         rend   ()       { return _C_rep.rend ();   }
    const_reverse_iterator   rend   () const { return _C_rep.rend ();   }

    //
    // capacity
    //
    bool        empty    () const { return _C_rep.empty ();    }
    size_type   size     () const { return _C_rep.size ();     }
    size_type   max_size () const { return _C_rep.max_size (); }

    //
    // modifiers
    //
    bpair<iterator, bool> insert (const value_type& __x) {
        const bpair<typename __rep_type::iterator, bool> __p =
            _C_rep.insert (__x, false); 
        return bpair<iterator, bool>(__p.first, __p.second);
    }

#if !defined (_MSC_VER) || _MSC_VER > 1300

    iterator insert (iterator __it, const value_type& __x) {
        return _C_rep.insert (__it, __x, false);
    }

    // BENTLEY_CHANGE
    iterator erase (iterator __it) {
        return _C_rep.erase (__it);
    }

    void erase (iterator __first, iterator __last) {
        _C_rep.erase (__first, __last);
    }

#else

    // working around MSVC bugs
    iterator insert (iterator __it, const value_type& __x) {
        typedef typename __rep_type::iterator _Iterator;
        return _RWSTD_REINTERPRET_CAST (iterator&, _C_rep.insert (
                   _RWSTD_REINTERPRET_CAST (_Iterator&, __it), __x, false));
    }

    // BENTLEY_CHANGE
    iterator erase (iterator __it) {
        typedef typename __rep_type::iterator _Iterator;
        return _C_rep.erase (_RWSTD_REINTERPRET_CAST (_Iterator&, __it));
    }

    void erase (iterator __first, iterator __last) {
        typedef typename __rep_type::iterator _Iterator;
        _C_rep.erase (_RWSTD_REINTERPRET_CAST (_Iterator&, __first),
                      _RWSTD_REINTERPRET_CAST (_Iterator&, __last));
    }

#endif

    template<class _InputIter>
    void insert (_InputIter __first, _InputIter __last) {
        for ( ;__first != __last; ++__first)
            _C_rep.insert (*__first, false);
    }

    size_type erase (const key_type& __x)  {
        return _C_rep.erase (__x); 
    }

    void swap (bstdset& __x) {
        _C_rep.swap (__x._C_rep);
    }

    void clear () {
        _C_rep.clear ();
    }

    key_compare key_comp () const {
        return _C_rep.key_comp ();
    }

    value_compare value_comp () const {
        return _C_rep.key_comp ();
    }

    // follows proposed resolution of lwg issue 214
    iterator find (const key_type& __x) {
        return _C_rep.find (__x);
    }

    const_iterator find (const key_type& __x) const {
        return _C_rep.find (__x);
    }

    size_type count (const key_type& __x) const {
        return _C_rep.count (__x);
    }

    // follows proposed resolution of lwg issue 214
    iterator lower_bound (const key_type& __x) {
        return _C_rep.lower_bound (__x);
    }

    const_iterator lower_bound (const key_type& __x) const {
        return _C_rep.lower_bound (__x);
    }

    // follows proposed resolution of lwg issue 214
    iterator upper_bound (const key_type& __x) {
        return _C_rep.upper_bound (__x);
    }

    const_iterator upper_bound (const key_type& __x) const {
        return _C_rep.upper_bound (__x);
    }

    // follows proposed resolution of lwg issue 214
    bpair<iterator, iterator> equal_range (const key_type& __x) {
        return _RWSTD_CONST_CAST (const bstdset*, this)->equal_range (__x);
    }

    bpair<const_iterator, const_iterator>
    equal_range (const key_type& __x) const {
        return _C_rep.equal_range (__x);
    }

#if defined (_RWSTD_NO_PART_SPEC_OVERLOAD)
    friend void swap (bstdset& __lhs, bstdset& __rhs) {
        __lhs.swap (__rhs);
    }
#endif

};


//! A template that has many of the capabilities of std::multiset
template <class _Key, 
          class _Compare = std::less<_Key>,
          class _Allocator = ::BentleyApi::BentleyAllocator<_Key> >
class bstdmultiset
{
public:
    //  
    // types
    //
    typedef _Key       key_type;
    typedef _Key       value_type;
    typedef _Compare   key_compare;
    typedef _Compare   value_compare;
    typedef _Allocator allocator_type;
private:
    
    typedef BC__RW::bc___rb_tree<key_type, value_type, 
                           BC__RW::__ident<value_type, key_type>, 
                           key_compare, allocator_type> __rep_type;
    __rep_type _C_rep;

public:
    //
    // types
    //
    typedef typename __rep_type::reference               reference;
    typedef typename __rep_type::const_reference         const_reference;
    typedef typename __rep_type::iterator                iterator;
    typedef typename __rep_type::const_iterator          const_iterator;
    typedef typename __rep_type::size_type               size_type;
    typedef typename __rep_type::difference_type         difference_type;
    typedef typename __rep_type::pointer                 pointer;
    typedef typename __rep_type::const_pointer           const_pointer;
    typedef typename __rep_type::reverse_iterator        reverse_iterator;
    typedef typename __rep_type::const_reverse_iterator  const_reverse_iterator;

    //
    // construct/copy/destroy
    //
    explicit
    bstdmultiset (const key_compare &__cmp = key_compare (),
              const allocator_type &__alloc = allocator_type ())
        : _C_rep (__cmp, __alloc) { }

    template<class _InputIter>
    bstdmultiset (_InputIter __first, _InputIter __last, 
              const key_compare &__cmp = key_compare (),
              const allocator_type &__alloc = allocator_type ())
        : _C_rep (__first, __last, __cmp, __alloc, true) { }

    bstdmultiset (const bstdmultiset &__x)
        : _C_rep (__x._C_rep) { }

    bstdmultiset& operator= (const bstdmultiset &__x) {
        _C_rep = __x._C_rep; return *this;
    }

    allocator_type get_allocator () const {
        return _C_rep.get_allocator ();
    }

    //
    // iterators
    //
    iterator                 begin  ()       { return _C_rep.begin ();  }
    const_iterator           begin  () const { return _C_rep.begin ();  }
    iterator                 end    ()       { return _C_rep.end ();    }
    const_iterator           end    () const { return _C_rep.end ();    }
    reverse_iterator         rbegin ()       { return _C_rep.rbegin (); } 
    const_reverse_iterator   rbegin () const { return _C_rep.rbegin (); } 
    reverse_iterator         rend   ()       { return _C_rep.rend ();   }
    const_reverse_iterator   rend   () const { return _C_rep.rend ();   }

    //
    // capacity
    //
    bool       empty    () const { return _C_rep.empty ();    }
    size_type  size     () const { return _C_rep.size ();     }
    size_type  max_size () const { return _C_rep.max_size (); }

    //
    // modifiers
    //
    iterator insert (const value_type& __x) {
        return _C_rep.insert (__x, true).first;
    }

    iterator insert (iterator __it, const value_type& __x) {
        return _C_rep.insert (__it, __x, true);
    }

    template<class _InputIter>
    void insert (_InputIter __first, _InputIter __last) {
        for ( ;__first != __last; ++__first)
            _C_rep.insert (*__first, true);
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

    void swap (bstdmultiset &__x) {
        _C_rep.swap (__x._C_rep);
    }

    void clear () {
        _C_rep.clear ();
    }

    key_compare   key_comp   () const {
        return _C_rep.key_comp ();
    }

    value_compare value_comp () const {
        return _C_rep.key_comp ();
    }

      // follows proposed resolution of lwg issue 214
    iterator find (const key_type& __x) {
        return _C_rep.find (__x);
    }

    const_iterator find (const key_type& __x) const {
        return _C_rep.find (__x);
    }

    size_type count (const key_type& __x) const {
        return _C_rep.count (__x);
    }

      // follows proposed resolution of lwg issue 214
    iterator lower_bound (const key_type& __x) {
        return _C_rep.lower_bound (__x);
    }

    const_iterator lower_bound (const key_type& __x) const {
        return _C_rep.lower_bound (__x);
    }

      // follows proposed resolution of lwg issue 214
    iterator upper_bound (const key_type& __x) {
        return _C_rep.upper_bound (__x); 
    }

    const_iterator upper_bound (const key_type& __x) const {
        return _C_rep.upper_bound (__x); 
    }

      // follows proposed resolution of lwg issue 214
    bpair<iterator, iterator>
    equal_range (const key_type& __x) {
        return _C_rep.equal_range (__x);
    }

    bpair<const_iterator,const_iterator>
    equal_range (const key_type& __x) const {
        return _C_rep.equal_range (__x);
    }

#if defined (_RWSTD_NO_PART_SPEC_OVERLOAD)
    friend void swap (bstdmultiset& __lhs, bstdmultiset& __rhs) {
        __lhs.swap (__rhs);
    }
#endif

};

template <class _Key, class _Compare, class _Allocator>
inline bool operator== (const bstdset<_Key, _Compare, _Allocator>& __x, 
                        const bstdset<_Key, _Compare, _Allocator>& __y)
{
    return    __x.size () == __y.size ()
           && equal (__x.begin (), __x.end (), __y.begin ());
}


template <class _Key, class _Compare, class _Allocator>
inline bool operator< (const bstdset<_Key, _Compare, _Allocator>& __x, 
                       const bstdset<_Key, _Compare, _Allocator>& __y)
{
    return lexicographical_compare (__x.begin (), __x.end (),
                                    __y.begin (), __y.end ());
}


template <class _Key, class _Compare, class _Allocator>
inline bool operator!= (const bstdset<_Key,_Compare,_Allocator>& __x, 
                        const bstdset<_Key,_Compare,_Allocator>& __y)
{
    return !(__x == __y);
}


template <class _Key, class _Compare, class _Allocator>
inline bool operator> (const bstdset<_Key,_Compare,_Allocator>& __x, 
                       const bstdset<_Key,_Compare,_Allocator>& __y)
{
    return __y < __x;
}


template <class _Key, class _Compare, class _Allocator>
inline bool operator>= (const bstdset<_Key,_Compare,_Allocator>& __x, 
                        const bstdset<_Key,_Compare,_Allocator>& __y)
{
    return !(__x < __y);
}


template <class _Key, class _Compare, class _Allocator>
inline bool operator<= (const bstdset<_Key,_Compare,_Allocator>& __x, 
                        const bstdset<_Key,_Compare,_Allocator>& __y)
{
    return !(__y <  __x);
}


#ifndef _RWSTD_NO_PART_SPEC_OVERLOAD

template <class _Key, class _Compare, class _Allocator>
void swap (bstdset<_Key,_Compare,_Allocator>& __a, 
           bstdset<_Key,_Compare,_Allocator>& __b)
{
    __a.swap (__b);
}

#endif   // _RWSTD_NO_PART_SPEC_OVERLOAD


template <class _Key, class _Compare, class _Allocator>
inline bool operator== (const bstdmultiset<_Key, _Compare, _Allocator>& __x, 
                        const bstdmultiset<_Key, _Compare, _Allocator>& __y)
{
    return    __x.size () == __y.size ()
           && equal (__x.begin (), __x.end (), __y.begin ());
}


template <class _Key, class _Compare, class _Allocator>
inline bool operator< (const bstdmultiset<_Key, _Compare, _Allocator>& __x, 
                       const bstdmultiset<_Key, _Compare, _Allocator>& __y)
{
    return lexicographical_compare (__x.begin (), __x.end (),
                                    __y.begin (), __y.end ());
}


template <class _Key, class _Compare, class _Allocator>
inline bool operator!= (const bstdmultiset<_Key,_Compare,_Allocator>& __x, 
                        const bstdmultiset<_Key,_Compare,_Allocator>& __y)
{
    return !(__x == __y);
}


template <class _Key, class _Compare, class _Allocator>
inline bool operator> (const bstdmultiset<_Key,_Compare,_Allocator>& __x, 
                       const bstdmultiset<_Key,_Compare,_Allocator>& __y)
{
    return __y < __x;
}


template <class _Key, class _Compare, class _Allocator>
inline bool operator>= (const bstdmultiset<_Key,_Compare,_Allocator>& __x, 
                        const bstdmultiset<_Key,_Compare,_Allocator>& __y)
{
    return !(__x < __y);
}


template <class _Key, class _Compare, class _Allocator>
inline bool operator<= (const bstdmultiset<_Key,_Compare,_Allocator>& __x, 
                        const bstdmultiset<_Key,_Compare,_Allocator>& __y)
{
    return !(__y <  __x);
}

#ifndef _RWSTD_NO_PART_SPEC_OVERLOAD

template <class _Key, class _Compare,  class _Allocator>
void swap (bstdmultiset<_Key,_Compare,_Allocator>& __a, 
           bstdmultiset<_Key,_Compare,_Allocator>& __b)
{
    __a.swap (__b); 
}

#endif   // _RWSTD_NO_PART_SPEC_OVERLOAD

// *** BENTLEY_CHANGE
NAMESPACE_BENTLEY_BSTDCXX_END


#endif   // _RWSTD_SET_INCLUDED
