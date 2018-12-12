/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/stdcxx/basic_string.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

// -*- C++ -*-
/***************************************************************************
 *
 * <string> - definition of the C++ Standard Library basic_string template
 *
 * $Id: string 685863 2008-08-14 11:47:59Z faridz $
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
 * Copyright 1994-2008 Rogue Wave Software, Inc.
 *
 **************************************************************************
 *
 * NOTICE: This File contains modifications made by Bentley Systems Inc. where designated.
 *
 *************************************************************************/

#ifndef _RWSTD_STRING_INCLUDED
#define _RWSTD_STRING_INCLUDED


// *** BENTLEY_CHANGE
#include <iosfwd>
#include <iterator>
#include <string>   // for std::char_traits
#include <Bentley/BentleyAllocator.h>
#include <Bentley/stdcxx/rw/_select.h>
#include <Bentley/stdcxx/rw/_strref.h>
#include <Bentley/stdcxx/rw/_defs.h>

// *** BENTLEY_CHANGE
/** @namespace BentleyApi::Bstdcxx 

string and container templates.

*/

NAMESPACE_BENTLEY_BSTDCXX_BEGIN

#if defined _RWSTD_NO_EXTERN_MEMBER_TEMPLATE
#  define _RWSTD_NO_STRING_OUTLINED_MEMBER_TEMPLATES
#endif   // _RWSTD_NO_STRING_MEMBER_TEMPLATES


#ifdef _RWSTD_NO_STRING_OUTLINED_MEMBER_TEMPLATES

_EXPORT
template <class _CharT, class _Traits, class _Alloc,
          class _StringIter, class _InputIter>
basic_string<_CharT, _Traits, _Alloc>& 
__rw_replace (basic_string<_CharT, _Traits, _Alloc>&, 
              _StringIter, _StringIter, _InputIter, _InputIter);

_EXPORT
template <class _CharT, class _Traits, class _Alloc,
          class _StringIter, class _InputIter>
basic_string<_CharT, _Traits, _Alloc>& 
__rw_replace_aux (basic_string<_CharT, _Traits, _Alloc>&, 
                  _StringIter, _StringIter, _InputIter, _InputIter);

#endif   // _RWSTD_NO_STRING_OUTLINED_MEMBER_TEMPLATES


//! A template that has many of the capabilities of std::basic_string
_EXPORT
template <class _CharT, class _Traits, class _Allocator>
class basic_string: private _Allocator
{
public:
  
    typedef _Traits                               traits_type;
    typedef typename traits_type::char_type      value_type;
    typedef _Allocator                            allocator_type;

#if !defined (DOCUMENTATION_GENERATOR)
    typedef BC__RW::__string_ref<value_type, traits_type, allocator_type>
     _C_string_ref_type;

    typedef  _RWSTD_ALLOC_TYPE(allocator_type, value_type)          
        _C_value_alloc_type;
    typedef  _RWSTD_REBIND(allocator_type, _C_string_ref_type)  
        _C_ref_alloc_type;
#endif

    typedef typename allocator_type::size_type       size_type;
    typedef typename allocator_type::difference_type difference_type;
    typedef typename allocator_type::reference       reference;
    typedef typename allocator_type::const_reference const_reference;
    typedef typename allocator_type::pointer         pointer;
    typedef typename allocator_type::const_pointer   const_pointer;

#ifndef _RWSTD_NO_DEBUG_ITER
#error "we don't want debug iterators ... yet"
    typedef BC__RW::__rw_debug_iter <basic_string, pointer, pointer>
        iterator;
    
    typedef BC__RW::__rw_debug_iter <basic_string, const_pointer, pointer>
        const_iterator;

    iterator _C_make_iter (const pointer& __ptr) {
        return iterator (*this, __ptr);
    }

    const_iterator _C_make_iter (const const_pointer& __ptr) const {
        return const_iterator (*this, __ptr);
    }

#else   // if defined (_RWSTD_NO_DEBUG_ITER)

    typedef pointer                        iterator;
    typedef const_pointer                  const_iterator;

#if !defined (DOCUMENTATION_GENERATOR)
    iterator _C_make_iter (pointer __ptr) {
        return __ptr;
    }

    const_iterator _C_make_iter (const_pointer __ptr) const {
        return __ptr;
    }
#endif  // DOCUMENTATION_GENERATOR

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

#ifndef _RWSTD_NO_STRING_NPOS_TYPE
    _RWSTD_STATIC_CONST (size_type, npos = size_type (_RWSTD_SIZE_MAX));
#else   // if defined (_RWSTD_NO_STRING_NPOS_TYPE)
    // working around an MSVC 7.0 bug (PR #26549)
    // and an HP aCC extern template bug #333
    _RWSTD_STATIC_CONST (_RWSTD_SIZE_T, npos = _RWSTD_SIZE_MAX);
#endif

    // 21.3.1, p2
    explicit basic_string (const allocator_type &__alloc = allocator_type ())
        : allocator_type (__alloc), _C_data (_C_nullref ()->data ()) { }

    // lwg issue #42
    basic_string (const basic_string&);

    // 21.3.1, p3
    basic_string (const basic_string&, size_type, size_type = npos, 
                  const allocator_type& = allocator_type ());

    // 21.3.1, p6
    basic_string (const_pointer, size_type, 
                  const allocator_type& = allocator_type ());

    // 21.3.1, p9
    basic_string (const_pointer, const allocator_type& = allocator_type ());

    // 21.3.1, p12
    basic_string (size_type, value_type,
                  const allocator_type& = allocator_type ());


    // 21.3.1, p15
    template <class _InputIter>
    basic_string (_InputIter __first, _InputIter __last, 
                  const allocator_type &__alloc = allocator_type ())
        : allocator_type (__alloc), _C_data (_C_nullref ()->data ()) {
        replace (_C_make_iter (_C_data), _C_make_iter (_C_data),
                 __first, __last);
    }

    basic_string (const_pointer, const_pointer, const allocator_type&
                  _RWSTD_REDECLARED_DEFAULT (allocator_type ()));

    ~basic_string () {
        _C_unlink (0);
    }

    // 21.3.1, p16
    basic_string& operator= (const basic_string&);

    // 21.3.1, p18
    basic_string& operator= (const_pointer __s) {
        return replace (size_type (), size (), __s, traits_type::length (__s));
    }

    // 21.3.1, p20
    basic_string& operator= (value_type __c) {
        return replace (size_type (), size (), &__c, size_type (1));
    }

    // disables reference counting
    iterator begin ();

    const_iterator begin () const {
        return _C_make_iter (_C_data);
    }

    iterator end () {
        // disable reference counting
        return begin () + size ();
    }

    const_iterator end () const {
        return _C_make_iter (_C_data + size ());
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
        return _C_pref ()->size ();
    }

    size_type length () const {
        return size ();
    }

    size_type max_size () const;

    void resize (size_type, value_type);

    void resize (size_type __n) {
        resize (__n, value_type ()); 
    }

    size_type capacity () const {
        return _C_pref ()->capacity ();
    }

    void reserve (size_type = 0);

    void clear () {
        if (_C_pref() == _C_nullref())
           return;
        if (size_type (1) < size_type (_C_pref ()->_C_get_ref ()))
            _C_unlink (_C_nullref ()->data ());
        else {
            traits_type::assign (_C_data [0], value_type ());
            _C_pref ()->_C_size._C_size = 0;
        }
    }

    bool empty () const  {
        return size () == 0;
    }

    const_reference operator[] (size_type) const;
    reference       operator[] (size_type);

    const_reference at (size_type) const;
    reference       at (size_type);

    basic_string& operator+= (const basic_string &__s) {
        return append (__s);
    }

    basic_string& operator+= (const_pointer __s) {
        return append (__s);
    }

    basic_string& operator+= (value_type __c) {
        return push_back (__c), *this;
    }

    basic_string& append (const basic_string&, size_type, size_type);

    basic_string& append (const basic_string &__str) {
        return append (__str.data (), __str.size ());
    }

    basic_string& append (const_pointer, size_type);

    basic_string& append (const_pointer __s) {
        return append (__s, traits_type::length (__s));
    }

    template<class _InputIter>
    basic_string& append (_InputIter __first, _InputIter __last) {
        // resolves to append (size_type, value_type) if _InputIter
        // is any integral type (even not an exact match, such as char)
        // the cast to int is necessary to prevent an exact match
        return append (__first, __last, _RWSTD_DISPATCH (_InputIter));
    }

    template<class _InputIter>
    basic_string& append (_InputIter __first, _InputIter __last, void*) {
        return replace (_C_make_iter (_C_data + size ()),
                        _C_make_iter (_C_data + size ()),
                        __first, __last), *this;
    }

    basic_string& append (size_type __n, value_type __c, int) {
        // unnamed arg is used for overload resolution
        return append (__n, __c);
    }

    basic_string& append (size_type, value_type);

    // lwg issue 7
    void push_back (value_type);

    basic_string& assign (const basic_string &__str) {
        return *this = __str;
    }

    basic_string& assign (const basic_string&, size_type, size_type);

    basic_string& assign (const_pointer __s, size_type __n) {
        return replace (size_type (), size (), __s, __n);
    }

    basic_string& assign (const_pointer __s) {
        return replace (size_type (), size (), __s, traits_type::length (__s));
    }


    template<class _InputIter>
    basic_string& assign (_InputIter __first, _InputIter __last) {
        // resolves to assign (size_type, value_type) if _InputIter
        // is any integral type (even not an exact match, such as char)
        // the cast to int is necessary to prevent an exact match
        return assign (__first, __last, _RWSTD_DISPATCH (_InputIter));
    }

    template<class _InputIter>
    basic_string& assign (_InputIter __first, _InputIter __last, void*) {
        // unnamed arg is used for overload resolution
        // _RWSTD_COMPILE_ASSERT (sizeof (*__first));
        return replace (_C_make_iter (_C_data),
                        _C_make_iter (_C_data + size ()), __first, __last);
    }

    basic_string& assign (size_type __n, value_type __c, int) {
        // unnamed arg is used for overload resolution
        return replace (size_type (), size (), __n, __c);
    }

    basic_string& assign (size_type __n, value_type __c) {
        return replace (size_type (), size (), __n, __c);
    }

    basic_string& insert (size_type, const basic_string&);
    basic_string& insert (size_type, const basic_string&,
                          size_type, size_type);

    basic_string& insert (size_type __pos, const_pointer __s, size_type __n) {
        replace (__pos, size_type (), __s, __n);
        return *this;
    }

    basic_string& insert (size_type __pos, const_pointer __s) {
        return insert (__pos, __s, traits_type::length (__s));
    }

    // 21.3.5.4, p10
    iterator insert (iterator __pos, value_type __c) {
        const size_type __inx = _C_off (__pos);
        return insert (__inx, &__c, 1), begin () + __inx;
    }

    template<class _InputIter>
    void insert (iterator __p, _InputIter __first, _InputIter __last) {
        insert (__p, __first, __last, _RWSTD_DISPATCH (_InputIter));
    }

    template <class _InputIter>
    void insert (iterator __p, _InputIter __first, _InputIter __last, void*) {
        // unnamed arg is used for overload resolution
        // _RWSTD_COMPILE_ASSERT (sizeof (*__first));
        replace (__p, __p, __first, __last);
    }

    void insert (iterator __p, size_type __n, value_type __c, int) {
        // unnamed arg is used for overload resolution
        replace (_C_off (__p), size_type (), __n, __c);
    }

    void insert (iterator __p,
                 const_pointer __first, const_pointer __last, void*) {
        if (__first >= _C_data && __first <= _C_data + size ())
            insert (_C_off (__p), basic_string (__first, __last));
        else
            replace (__p, __p, __first, __last);
    }

#  ifndef _RWSTD_NO_DEBUG_ITER

    void insert (iterator __p,
                 const_iterator __first, const_iterator __last, void*) {
        // unnamed arg is used for overload resolution
        if (!(__first == __last)) {
            const const_pointer __pf = &*__first;
            insert (__p, __pf, __pf + _C_off (__first, __last));
        }
    }

    void insert (iterator __p, iterator __first, iterator __last, void*) {
        // unnamed arg is used for overload resolution
        insert (__p, const_iterator (__first), const_iterator (__last),
                (void*)0);
    }

#  endif   // _RWSTD_NO_DEBUG_ITER

    void insert (iterator __it, size_type __n, value_type __c) {
        replace (_C_off (__it), size_type (), __n, __c);
    }

    basic_string& insert (size_type __pos, size_type __n, value_type __c) {
        return replace (__pos, size_type (), __n, __c);
    }

    basic_string& erase (size_type = 0, size_type = npos);

    iterator erase (iterator __it) {
        _RWSTD_ASSERT (__it != end ());   // verify precondition (Table 67)
        return erase (__it, __it + 1);
    }

    iterator erase (iterator __first, iterator __last) {
        const size_type __pos = _C_off (__first);

        replace (__pos, _C_off (__first, __last), const_pointer(), size_type());

        return _C_make_iter (_C_data + __pos);
    }

private:

    static size_type _C_min (size_type __x, size_type __y) {
        return __x < __y ? __x : __y;
    }

    iterator __replace_aux (size_type __pos1, size_type __n1,
                            const basic_string &__str,
                            size_type __pos2 = 0,
                            size_type __n2 = npos) {
        replace (__pos1, __n1, __str.c_str() + __pos2, __n2);
        return _C_make_iter (_C_data + __pos1);
    }

public:

#if !defined (DOCUMENTATION_GENERATOR)
#ifndef _RWSTD_NO_STRING_OUTLINED_MEMBER_TEMPLATES

    template <class _InputIter>
    basic_string& __replace_aux (iterator, iterator, _InputIter, _InputIter);

#else

    template <class _InputIter>
    basic_string& __replace_aux (iterator __first1, iterator __last1, 
                                 _InputIter __first2, _InputIter __last2) {
        return __rw_replace_aux (*this, __first1, __last1, __first2, __last2);
    }

#endif  // _RWSTD_NO_STRING_OUTLINED_MEMBER_TEMPLATES

    size_type _C_grow (size_type, size_type) const;

#endif  // DOCUMENTATION_GENERATOR

public:
    // 21.3.5.6, p1
    basic_string&
    replace (size_type __pos, size_type __n, const basic_string &__s) {
        return replace (__pos, __n, __s, size_type (), __s.size ());
    }

    // 21.3.5.6, p2
    basic_string&
    replace (size_type, size_type, const basic_string&, size_type, size_type);

    // 21.3.5.6, p7
    basic_string&
    replace (size_type, size_type, const_pointer, size_type);

    // 21.3.5.6, p8
    basic_string&
    replace (size_type __pos, size_type __n, const_pointer __s) {
        return replace (__pos, __n, __s, traits_type::length (__s));
    }

    // 21.3.5.6, p10
    basic_string&
    replace (size_type, size_type, size_type, value_type);

    // 21.3.5.6, p11
    basic_string&
    replace (iterator __first, iterator __last, const basic_string &__str) {
        return replace (_C_off (__first), _C_off (__first,  __last), __str);
    }

    // 21.3.5.6, p15
    basic_string&
    replace (iterator __first, iterator __last,
             const_pointer __s, size_type __n) {
        replace (_C_off (__first), _C_off (__first, __last), __s, __n);
        return *this;
    }

    // 21.3.5.6, p17
    basic_string&
    replace (iterator __first, iterator __last, const_pointer __s) {
        return replace (__first, __last, __s, traits_type::length (__s));
    }

    // 21.3.5.6, p19
    basic_string&
    replace (iterator __first, iterator __last, size_type __n, value_type __c) {
        return replace (_C_off (__first), _C_off (__first, __last), __n, __c);
    }

#ifndef _RWSTD_NO_STRING_OUTLINED_MEMBER_TEMPLATES

    template<class _InputIter>
    basic_string&
    replace (iterator __first1, iterator __last1, 
             _InputIter __first2, _InputIter __last2, void*);

#else   // if defined (_RWSTD_NO_STRING_OUTLINED_MEMBER_TEMPLATES)

    template<class _InputIter>
    basic_string&
    replace (iterator __first1, iterator __last1, 
             _InputIter __first2, _InputIter __last2, void*) {
        return __rw_replace (*this, __first1, __last1, __first2, __last2);
    }

#endif   // _RWSTD_NO_STRING_OUTLINED_MEMBER_TEMPLATES

    basic_string&
    replace (iterator __first, iterator __last,
             size_type __n, value_type __c, int) {
        return replace (_C_off (__first), _C_off (__first, __last), __n, __c);
    }

    // 21.3.5.6, p21
    template<class _InputIter>
    basic_string&
    replace (iterator __first1, iterator __last1,
             _InputIter __first2, _InputIter __last2) {
        return replace (__first1, __last1, __first2, __last2,
                        _RWSTD_DISPATCH (_InputIter));
    }


    // 21.3.5.7, p1
    size_type copy (pointer, size_type, size_type = 0) const;


#ifndef _RWSTD_NO_EXT_DEEP_STRING_COPY

    basic_string copy () const {
        return basic_string (data (), size ());
    }

#endif   //_RWSTD_NO_EXT_DEEP_STRING_COPY

    void swap (basic_string &__s) {
        if (get_allocator () == __s.get_allocator ()) {
            pointer __temp = _C_data;
            _C_data = __s._C_data;
            __s._C_data = __temp;
        }
        else {
            basic_string __tmp = *this;
            *this = __s;
            __s = __tmp;
        }
    }

    //
    // string operations
    //
    const_pointer c_str () const {
        return _C_data;
    }

    const_pointer data () const {
        return _C_data;
    }

    allocator_type get_allocator() const {
        return *this;
    }

    // 21.3.6.1, p1
    size_type find (const basic_string &__str, size_type __pos = 0) const {
        return find (__str.c_str (), __pos, __str.size ());
    }

    // 21.3.6.1, p4
    size_type find (const_pointer, size_type, size_type) const;

    // 21.3.6.1, p5
    size_type find (const_pointer __s, size_type __pos = 0) const {
        return find (__s, __pos, traits_type::length (__s));
    }

    // 21.3.6.1, p7
    size_type find (value_type, size_type = 0) const;

    // 21.3.6.2, p1
    size_type rfind (const basic_string &__str, size_type __pos = npos) const {
        return rfind (__str.c_str (), __pos, __str.size ());
    }

    // 21.3.6.2, p4
    size_type rfind (const_pointer, size_type, size_type) const;

    // 21.3.6.2, p5
    size_type rfind (const_pointer __s, size_type __pos = npos) const {
        return rfind (__s, __pos, traits_type::length (__s));
    }

    // 21.3.6.2, p7
    size_type rfind (value_type __c, size_type __pos = npos) const {
        return rfind (&__c, __pos, 1);
    }    

    // 21.3.6.3, p1
    size_type find_first_of (const basic_string &__str,
                             size_type __pos = 0) const {
        return find_first_of (__str.c_str (), __pos, __str.size ());
    }

    // 21.3.6.3, p4
    size_type find_first_of (const_pointer, size_type, size_type) const;

    // 21.3.6.3, p5
    size_type find_first_of (const_pointer __s, size_type __pos = 0) const {
        return find_first_of (__s, __pos, traits_type::length (__s));
    }

    // 21.3.6.3, p6
    size_type find_first_of (value_type __c, size_type __pos = 0) const {
        return find (__c, __pos);
    }

    // 21.3.6.4, p1
    size_type find_last_of (const basic_string &__str,
                            size_type __pos = npos) const {
        return find_last_of (__str.c_str (), __pos, __str.size ());
    }

    // 21.3.6.4, p4
    size_type find_last_of (const_pointer, size_type, size_type) const;

    // 21.3.6.4, p5
    size_type find_last_of (const_pointer __s, size_type __pos = npos) const {
        return find_last_of (__s, __pos, traits_type::length (__s));
    }

    // 21.3.6.4, p7
    size_type find_last_of (value_type __c, size_type __pos = npos) const {
        return rfind (__c, __pos);
    }

    // 21.3.6.5, p1
    size_type find_first_not_of (const basic_string &__str, 
                                 size_type __pos = 0) const {
        return find_first_not_of (__str.c_str (), __pos, __str.size ());
    }

    // 21.3.6.5, p4
    size_type find_first_not_of (const_pointer, size_type, size_type) const;

    // 21.3.6.5, p5
    size_type find_first_not_of (const_pointer __s, 
                                 size_type __pos = 0) const {
        return find_first_not_of (__s, __pos, traits_type::length(__s));
    }

    // 21.3.6.5, p7
    size_type find_first_not_of (value_type __c, size_type __pos = 0) const {
        return find_first_not_of (&__c, __pos, size_type (1));
    }

    // 21.3.6.6, p1
    size_type find_last_not_of (const basic_string &__str, 
                                size_type __pos = npos) const {
        return find_last_not_of (__str.c_str (), __pos, __str.size ());
    }

    // 21.3.6.6, p4
    size_type find_last_not_of (const_pointer, size_type, size_type) const;

    // 21.3.6.6, p6
    size_type find_last_not_of (const_pointer __s,
                                size_type __pos = npos) const {
        return find_last_not_of (__s, __pos, traits_type::length (__s));
    }

    // 21.3.6.6, p7
    size_type find_last_not_of (value_type __c, size_type __pos = npos) const {
        return find_last_not_of (&__c, __pos, size_type (1));
    }
  
    // 21.3.6.7
    basic_string substr (size_type = 0, size_type = npos) const;
  
    // 21.3.6.8, p1
    int compare (const basic_string &__str) const;

    // 21.3.6.8, p3
    int compare (size_type __pos, size_type __n,
                 const basic_string &__str) const {
        return compare (__pos, __n, __str.c_str(), __str.size());
    }

    // 21.3.6.8, p4
    int compare (size_type, size_type, const basic_string&, 
                size_type, size_type) const;

    // 21.3.6.8, p5
    int compare (const_pointer __s) const {
        return compare (size_type (), size (), __s, traits_type::length(__s));
    }

    // 21.3.6.8, p6, see also lwg Issue 5
    int compare (size_type __pos, size_type __n, const_pointer __s) const {
        return compare(__pos, __n, __s, traits_type::length (__s));
    }

    // lwg Issue 5
    int compare (size_type, size_type, const_pointer, size_type) const;

#if defined (_RWSTD_NO_PART_SPEC_OVERLOAD)
    friend void swap (basic_string& __lhs, basic_string& __rhs) {
        __lhs.swap (__rhs);
    }
#endif

#ifndef _RWSTD_NO_STRING_OUTLINED_MEMBER_TEMPLATES

private:

#else   // if defined (_RWSTD_NO_STRING_OUTLINED_MEMBER_TEMPLATSE)

public:

#endif   // _RWSTD_NO_STRING_OUTLINED_MEMBER_TEMPLATES

    static size_type
    _C_off (const_iterator __first, const_iterator __last) {
        _RWSTD_ASSERT_RANGE (__first, __last);
        return size_type (__last - __first);
    }

    size_type
    _C_off (const_iterator __it) const {
        _RWSTD_ASSERT_RANGE (__it, _C_make_iter (_C_data));
        return _C_off (_C_make_iter (_C_data), __it);
    }

    void _C_clone (size_type);

    _C_string_ref_type* _C_pref () const {
        // use two static_casts in favor of reinterpret_cast
        // to prevent "increased alignment" warnings
        return _RWSTD_STATIC_CAST (_C_string_ref_type*,
                   _RWSTD_STATIC_CAST (void*, _C_data)) - 1; 
    }

    void _C_unlink (pointer);   

    friend struct BC__RW::__string_ref<value_type, traits_type, allocator_type>;

#ifndef _RWSTD_NO_COLLAPSE_TEMPLATE_STATICS

    static BC__RW::__null_ref<_CharT, _Traits, _Allocator> _C_null_ref;

    static BC__RW::__null_ref<_CharT, _Traits, _Allocator>* _C_nullref () {
        return &_C_null_ref;
    }

#else   // if defined (_RWSTD_NO_COLLAPSE_TEMPLATE_STATICS)

    // BENTLEY_CHANGE
    static BC__RW::__null_ref<_CharT, _Traits, _Allocator>* _C_nullref () {
        typedef BC__RW::__null_ref<_CharT, _Traits, _Allocator> _NullRef;

        return _RWSTD_REINTERPRET_CAST (_NullRef*, bentleyAllocator_getNullRefBuffer());
    }

#endif   // _RWSTD_NO_COLLAPSE_TEMPLATE_STATICS

    _C_string_ref_type * _C_get_rep (size_type, size_type);

    pointer _C_data;
};

#if !defined (DOCUMENTATION_GENERATOR)
#ifdef BENTLEY_CHANGE_REMOVED
typedef basic_string<char, std::char_traits<char>, allocator<char> > string;

#ifndef _RWSTD_NO_WCHAR_T

typedef basic_string<wchar_t, std::char_traits<wchar_t>, allocator<wchar_t> >
wstring;

#endif  // _RWSTD_NO_WCHAR_T
#endif  //def BENTLEY_CHANGE_REMOVED


template <class _CharT, class _Traits , class _Allocator>

#if    !defined (__GNUG__)                  \
    || __GNUG__ > 3 || __GNUC_MINOR__ > 3   \
    || !defined (__ia64__)

// working around a gcc 3.2.3 inliner bug on IA64 (PR #29766)
inline

#endif   // !gcc || gcc > 3.3 || !IA64

void basic_string<_CharT, _Traits, _Allocator>::
_C_unlink (pointer __ptr)
{
    _RWSTD_ASSERT (0 != _C_data);

    if (0 >= _C_pref ()->_C_dec_ref ()) {
        // positive result of the decrement means the string body
        // is shared by two or more basic_string objects (handles)
        // result of 0 means that this object (handle) is the only
        // owner of the string body
        // negative result means that this object is the only owner
        // of the string body which has reference counting disabled

        // must pass same size to deallocate as allocate (see string.cc)
        // note that we cannot call capacity() after the destroy() call
        const size_type __size =
           capacity () + sizeof (_C_string_ref_type) / sizeof (value_type) + 2;

        _C_pref ()->_C_destroy ();

        _C_ref_alloc_type (*this).destroy (_C_pref ());
        _RWSTD_VALUE_ALLOC (_C_value_alloc_type, *this,
                            deallocate (_RWSTD_REINTERPRET_CAST (pointer,
                                                                 _C_pref ()),
                                        __size));
    }

    _C_data = __ptr;
}


template <class _CharT, class _Traits, class _Alloc>
inline typename basic_string<_CharT, _Traits, _Alloc>::size_type
basic_string<_CharT, _Traits, _Alloc>::
_C_grow (size_type __from, size_type __to) const
{
    const size_type __cap = _RWSTD_NEW_CAPACITY (basic_string, this, __from);

    return __cap < __to ? __to : __cap;
}

template <class _CharT, class _Traits , class _Allocator>
inline basic_string<_CharT, _Traits, _Allocator>::
basic_string (const basic_string<_CharT, _Traits, _Allocator> &__s)
    : allocator_type (__s.get_allocator ())
{
    if (size_type (0) < size_type (__s._C_pref ()->_C_get_ref ())) {
        _C_data = __s._C_data;
        _C_pref ()->_C_inc_ref ();
    }
    else {
        const size_type __n = __s.size ();
        _C_data = _C_get_rep (__n, __n)->data ();
        traits_type::copy (_C_data, __s.data (), __n);
    }

    _RWSTD_ASSERT (0 != _C_data);
}

template <class _CharT, class _Traits , class _Allocator>
inline typename basic_string<_CharT, _Traits, _Allocator>::iterator
basic_string<_CharT, _Traits, _Allocator>::
begin ()
{
    if (size_type (1) < size_type (_C_pref ()->_C_get_ref ()))
        _C_clone (size ());

    // not thread safe: there is exactly one body pointed to by
    // this->_C_pref() at this point and the caller is responsible
    // for synchronizing accesses to the same object from multiple
    // threads
    _C_pref ()->_C_unref ();

    return _C_make_iter (_C_data);
}


template <class _CharT, class _Traits , class _Allocator>
inline typename basic_string<_CharT, _Traits, _Allocator>::size_type
basic_string<_CharT, _Traits, _Allocator>::
max_size () const
{
    const size_type __max_chars = allocator_type::max_size ();
    const size_type __ref_size  =
        1U + sizeof (_C_string_ref_type) / sizeof (_CharT);

    return __max_chars < __ref_size ? 0 : __max_chars - __ref_size;
}


template <class _CharT, class _Traits , class _Allocator>
inline basic_string<_CharT, _Traits, _Allocator>&
basic_string<_CharT, _Traits, _Allocator>::
erase (size_type __pos, size_type __n)
{
    _RWSTD_REQUIRES (__pos <= size (),
                     std::out_of_range, (_RWSTD_ERROR_OUT_OF_RANGE, 
                     _RWSTD_FUNC ("basic_string::erase(size_type, size_type)"),
                      __pos, size ()));

    const value_type __tmp =  value_type () ;
    size_type __len = size () - __pos;
    return replace (__pos, __n < __len ? __n : __len, &__tmp, size_type ());
}


template <class _CharT, class _Traits , class _Allocator>
inline typename basic_string<_CharT, _Traits, _Allocator>::const_reference 
basic_string<_CharT, _Traits, _Allocator>::
operator[] (size_type __pos) const
{
#ifdef _RWSTD_BOUNDS_CHECKING

    _RWSTD_REQUIRES (__pos <= size (),
                     std::out_of_range, (_RWSTD_ERROR_OUT_OF_RANGE, 
                     _RWSTD_FUNC ("basic_string::operator[](size_type) const"),
                     __pos, size ()));

#endif   // _RWSTD_BOUNDS_CHECKING

    // reference counting still enabled
    return _C_data [__pos];
}


template <class _CharT, class _Traits , class _Allocator>
inline typename basic_string<_CharT, _Traits, _Allocator>::reference
basic_string<_CharT, _Traits, _Allocator>::
operator[] (size_type __pos)
{
#ifdef _RWSTD_BOUNDS_CHECKING

    // 21.3.4, p1 - behavior is undefined if __pos == size ()
    _RWSTD_REQUIRES (__pos < size (),
                     std::out_of_range, (_RWSTD_ERROR_OUT_OF_RANGE, 
                     _RWSTD_FUNC ("basic_string::operator[](size_type)"),
                     __pos, size ()));

#endif   // _RWSTD_BOUNDS_CHECKING

    // prevent reference counting
    return begin ()[__pos];
}


template <class _CharT, class _Traits , class _Allocator>
inline typename basic_string<_CharT, _Traits, _Allocator>::const_reference
basic_string<_CharT, _Traits, _Allocator>::
at (size_type __pos) const
{
    _RWSTD_REQUIRES (__pos < size (),
                     std::out_of_range, (_RWSTD_ERROR_OUT_OF_RANGE, 
                     _RWSTD_FUNC ("basic_string::at (size_type) const"),
                     __pos, size ()));

    // reference counting still enabled
    return _C_data [__pos];
}


template <class _CharT, class _Traits , class _Allocator>
inline typename basic_string<_CharT, _Traits, _Allocator>::reference
basic_string<_CharT, _Traits, _Allocator>::
at (size_type __pos)
{
    _RWSTD_REQUIRES (__pos < size (),
                     std::out_of_range, (_RWSTD_ERROR_OUT_OF_RANGE, 
                     _RWSTD_FUNC ("basic_string::at (size_type)"),
                     __pos, size ()));

    // prevent reference counting
    return begin ()[__pos];
}


template <class _CharT, class _Traits , class _Allocator>
inline void
basic_string<_CharT, _Traits, _Allocator>::
resize (size_type __n, value_type __c)
{
    _RWSTD_REQUIRES (__n <= max_size (),
                     std::length_error, (_RWSTD_ERROR_LENGTH_ERROR,
                      _RWSTD_FUNC ("basic_string::resize(size_type, "
                                   "value_type)"), __n, max_size ()));

    if (__n < size())
        erase (__n, size () - __n);
    else
        replace (size (), size_type (), __n - size (), __c);
}


template <class _CharT, class _Traits , class _Allocator>
inline void
basic_string<_CharT, _Traits, _Allocator>::
push_back (value_type __c)
{
    const size_type __size0 = size ();
    const _RWSTD_SIZE_T __size1 = __size0 + 1;

    if (   capacity () < __size1
        || size_type (1) < size_type (_C_pref ()->_C_get_ref ())) {
         replace (size (), size_type (), 1, __c);
    }
    else {
        traits_type::assign (_C_data [__size0], __c);
        traits_type::assign (_C_data [__size1], value_type ());
        _C_pref ()->_C_size._C_size = __size1;
    }
}


template <class _CharT, class _Traits , class _Allocator>
inline void basic_string<_CharT, _Traits, _Allocator>::
reserve (size_type __cap)
{
    _RWSTD_REQUIRES (__cap <= max_size (),
                     std::length_error, (_RWSTD_ERROR_LENGTH_ERROR,
                      _RWSTD_FUNC ("basic_string::reserve(size_type)"),
                      __cap, max_size ()));

    if (__cap > capacity ())
        _C_clone (__cap);
}


template <class _CharT, class _Traits , class _Allocator>
inline basic_string<_CharT, _Traits, _Allocator>&
basic_string<_CharT, _Traits, _Allocator>::
replace (size_type __pos1, size_type __n1, const basic_string &__str,
         size_type __pos2, size_type __n2)
{
    _RWSTD_REQUIRES (__pos1 <= size (),
                     std::out_of_range, (_RWSTD_ERROR_OUT_OF_RANGE,  
                      _RWSTD_FUNC ("basic_string::replace(size_type, "
                                   "size_type, const basic_string&, "
                                   "size_type, size_type)"),
                      __pos1, size ()));

    _RWSTD_REQUIRES (__pos2 <= __str.size (),
                     std::out_of_range, (_RWSTD_ERROR_OUT_OF_RANGE,  
                      _RWSTD_FUNC ("basic_string::replace(size_type, "
                                   "size_type, const basic_string&, "
                                   "size_type, size_type)"),
                      __pos2, __str.size ()));

    const size_type __rem = __str.size () - __pos2;

    replace (__pos1, __n1, __str.data () + __pos2, __n2 < __rem ? __n2 : __rem);

    return *this;
}


template <class _CharT, class _Traits , class _Allocator>
inline typename basic_string<_CharT, _Traits, _Allocator>::size_type
basic_string<_CharT, _Traits, _Allocator>::
find (value_type __c, size_type __pos) const
{
    if (__pos > size ())
        return npos;

    const const_pointer __where =
        traits_type::find (_C_data + __pos, size () - __pos, __c);

    return __where ? __where  - _C_data : npos;
}


template <class _CharT, class _Traits, class _Allocator>
inline void
basic_string<_CharT, _Traits, _Allocator>::
_C_clone (size_type __cap)
{
    const size_type __size = size ();

    _C_string_ref_type* const __temp =
        _C_get_rep (__cap, __size > __cap ? __cap : __size);

    traits_type::copy (__temp->data (), _C_data, __size);

    _C_unlink (__temp->data ());
}


template <class _CharT, class _Traits, class _Allocator>
inline int
basic_string<_CharT, _Traits, _Allocator>::
compare (const basic_string &__str) const
{
    const size_type __n1    = size ();
    const size_type __n2    = __str.size ();
    const bool      __first = __n1 < __n2;

    const int __res =
        traits_type::compare (data (), __str.data (), __first ? __n1 : __n2);

    return __res ? __res : __first ? -1 : int (__n1 != __n2);
}


template <class _CharT, class _Traits, class _Allocator>
inline basic_string<_CharT, _Traits, _Allocator>&
basic_string<_CharT, _Traits, _Allocator>::
append (const basic_string &__str, size_type __pos, size_type __n)
{
    _RWSTD_REQUIRES (__pos <= __str.size (),
                     std::out_of_range, (_RWSTD_ERROR_OUT_OF_RANGE, 
                      _RWSTD_FUNC ("basic_string::append(const basic_string&,"
                                   " size_type, size_type)"),
                      __pos, __str.size ()));

    const size_type __rlen = _C_min (__str.size() - __pos, __n);

    return append (__str.data () + __pos, __rlen);
}


template <class _CharT, class _Traits, class _Allocator>
inline basic_string<_CharT, _Traits, _Allocator>&
basic_string<_CharT, _Traits, _Allocator>::
append (const_pointer __s, size_type __n)
{
    const size_type __size0 = size ();
    const _RWSTD_SIZE_T __size1 = __size0 + __n;

    if (   capacity () <= __size1
        || size_type (1) < size_type (_C_pref ()->_C_get_ref ()))
        return replace (size (), size_type (), __s, __n);

    traits_type::copy (_C_data + __size0, __s, __n);
    traits_type::assign (_C_data [__size1], value_type ());
    _C_pref ()->_C_size._C_size = __size1;

    return *this;
}


template <class _CharT, class _Traits, class _Allocator>
inline basic_string<_CharT, _Traits, _Allocator>&
basic_string<_CharT, _Traits, _Allocator>::
append (size_type __n, value_type __c)
{
    const size_type __size0 = size ();
    const _RWSTD_SIZE_T __size1 = __size0 + __n;

    if (   capacity () < __size1
        || size_type (1) < size_type (_C_pref ()->_C_get_ref ()))
        return replace (size (), size_type (), __n, __c);

    traits_type::assign (_C_data + __size0, __n, __c);
    traits_type::assign (_C_data [__size1], value_type ());
    _C_pref ()->_C_size._C_size = __size1;

    return *this;
}

#endif

// 21.3.7.1, p1
template <class _CharT, class _Traits , class _Allocator>
inline basic_string<_CharT, _Traits, _Allocator>
operator+ (const basic_string<_CharT, _Traits, _Allocator> &__lhs, 
           const basic_string<_CharT, _Traits, _Allocator> &__rhs)
{
    typedef basic_string<_CharT, _Traits, _Allocator> string_type;

    // prevent reference counting while creating a copy of lhs
    return string_type (__lhs.data (), __lhs.size ()) += __rhs;
}


// 21.3.7.1, p2
template <class _CharT, class _Traits , class _Allocator>
inline basic_string<_CharT, _Traits, _Allocator>
operator+ (const _CharT*                                    __lhs, 
           const basic_string<_CharT, _Traits, _Allocator>& __rhs)
{
    return basic_string<_CharT, _Traits, _Allocator>(__lhs) += __rhs;
}


// 21.3.7.1, p4
template <class _CharT, class _Traits , class _Allocator>
inline basic_string<_CharT, _Traits, _Allocator>
operator+ (_CharT                                           __lhs,
           const basic_string<_CharT, _Traits, _Allocator>& __rhs)
{
    return basic_string<_CharT, _Traits, _Allocator>(1, __lhs) += __rhs;
}


// 21.3.7.1, p5
template <class _CharT, class _Traits , class _Allocator>
inline basic_string<_CharT, _Traits, _Allocator>
operator+ (const basic_string<_CharT, _Traits, _Allocator>& __lhs, 
           const _CharT*                                    __rhs)
{
    typedef basic_string<_CharT, _Traits, _Allocator> string_type;

    // prevent reference counting while creating a copy of lhs
    return string_type (__lhs.data (), __lhs.size ()) += __rhs;
}


// 21.3.7.1, p7
template <class _CharT, class _Traits , class _Allocator>
inline basic_string<_CharT, _Traits, _Allocator>
operator+ (const basic_string<_CharT, _Traits, _Allocator>& __lhs, 
           _CharT                                           __rhs)
{
    typedef basic_string<_CharT, _Traits, _Allocator> string_type;

    // prevent reference counting while creating a copy of lhs
    return string_type (__lhs.data (), __lhs.size ()) += __rhs;
}


// 21.3.7.2, p1
template <class _CharT, class _Traits , class _Allocator>
inline bool
operator== (const basic_string<_CharT, _Traits, _Allocator>& __lhs, 
            const basic_string<_CharT, _Traits, _Allocator>& __rhs)
{
    // avoid using basic_string<>::compare() for efficiency
    return    __lhs.size () == __rhs.size ()
           && !_Traits::compare (__lhs.data (), __rhs.data (), __lhs.size ());
}


// 21.3.7.2, p2
template <class _CharT, class _Traits , class _Allocator>
inline bool
operator== (const _CharT*                                    __lhs, 
            const basic_string<_CharT, _Traits, _Allocator>& __rhs)
{
    const typename basic_string<_CharT, _Traits, _Allocator>::size_type
        __n = _Traits::length (__lhs);

    // avoid using basic_string<>::compare() for efficiency
    return    __rhs.size () == __n
           && !_Traits::compare (__lhs, __rhs.data (), __n);
}


// 21.3.7.2, p3
template <class _CharT, class _Traits , class _Allocator>
inline bool
operator== (const basic_string<_CharT, _Traits, _Allocator>& __lhs, 
            const _CharT*                                    __rhs)
{
    return __rhs == __lhs;
}


// 21.3.7.4, p1
template <class _CharT, class _Traits , class _Allocator>
inline bool
operator< (const basic_string<_CharT, _Traits, _Allocator>& __lhs, 
           const basic_string<_CharT, _Traits, _Allocator>& __rhs)
{
    return 0 > __lhs.compare (__rhs);
}


// 21.3.7.4, p2
template <class _CharT, class _Traits , class _Allocator>
inline bool
operator< (const _CharT*                                    __lhs, 
           const basic_string<_CharT, _Traits, _Allocator>& __rhs)
{
    return 0 < __rhs.compare (__lhs);
}


// 21.3.7.4, p3
template <class _CharT, class _Traits , class _Allocator>
inline bool
operator< (const basic_string<_CharT, _Traits, _Allocator>& __lhs,
           const _CharT*                                    __rhs)
{
    return 0 > __lhs.compare (__rhs);
}


// 21.3.7.3, p1
template <class _CharT, class _Traits , class _Allocator>
inline bool
operator!= (const basic_string<_CharT, _Traits, _Allocator>& __lhs, 
            const basic_string<_CharT, _Traits, _Allocator>& __rhs)
{
    return !(__lhs == __rhs);
}


// 21.3.7.5, p1
template <class _CharT, class _Traits , class _Allocator>
inline bool
operator> (const basic_string<_CharT, _Traits, _Allocator>& __lhs, 
           const basic_string<_CharT, _Traits, _Allocator>& __rhs)
{
    return __rhs < __lhs;
}


// 21.3.7.6, p1
template <class _CharT, class _Traits , class _Allocator>
inline bool
operator<= (const basic_string<_CharT, _Traits, _Allocator>& __lhs, 
            const basic_string<_CharT, _Traits, _Allocator>& __rhs)
{
    return !(__rhs < __lhs);
}


// 21.3.7.7, p1
template <class _CharT, class _Traits , class _Allocator>
inline bool
operator>= (const basic_string<_CharT, _Traits, _Allocator>& __lhs, 
            const basic_string<_CharT, _Traits, _Allocator>& __rhs)
{
    return !(__lhs < __rhs);
}

// 21.3.7.8, p1
#ifndef _RWSTD_NO_PART_SPEC_OVERLOAD

template <class _CharT, class _Traits, class _Allocator>
inline void swap (basic_string<_CharT, _Traits, _Allocator>& __a, 
                  basic_string<_CharT, _Traits, _Allocator>& __b)
{
    __a.swap (__b);
}

#endif   // _RWSTD_NO_PART_SPEC_OVERLOAD


// 21.3.7.3, p2
template <class _CharT, class _Traits , class _Allocator>
inline bool
operator!= (const _CharT*                                    __lhs, 
            const basic_string<_CharT, _Traits, _Allocator>& __rhs)
{
    return !(__lhs == __rhs);
}


// 21.3.7.3, p3
template <class _CharT, class _Traits , class _Allocator>
inline bool
operator!= (const basic_string<_CharT, _Traits, _Allocator>& __lhs, 
            const _CharT*                                    __rhs)
{
    return !(__lhs == __rhs);
}


// 21.3.7.5, p2
template <class _CharT, class _Traits , class _Allocator>
inline bool
operator> (const _CharT*                                    __lhs, 
           const basic_string<_CharT, _Traits, _Allocator>& __rhs)
{
    return __rhs < __lhs;
}


// 21.3.7.5, p3
template <class _CharT, class _Traits , class _Allocator>
inline bool
operator> (const basic_string<_CharT, _Traits, _Allocator>& __lhs, 
           const _CharT*                                    __rhs)
{
    return __rhs < __lhs;
}


// 21.3.7.6, p2
template <class _CharT, class _Traits , class _Allocator>
inline bool
operator<= (const _CharT*                                    __lhs, 
            const basic_string<_CharT, _Traits, _Allocator>& __rhs)
{
    return !(__rhs < __lhs);
}


// 21.3.7.6, p3
template <class _CharT, class _Traits , class _Allocator>
inline bool
operator<= (const basic_string<_CharT, _Traits, _Allocator>& __lhs, 
            const _CharT*                                    __rhs)
{
    return !(__rhs < __lhs);
}


// 21.3.7.7, p2
template <class _CharT, class _Traits , class _Allocator>
inline bool
operator>= (const _CharT*                                    __lhs, 
            const basic_string<_CharT, _Traits, _Allocator>& __rhs)
{
    return !(__lhs < __rhs);
}


// 21.3.7.7, p3
template <class _CharT, class _Traits , class _Allocator>
inline bool
operator>= (const basic_string<_CharT, _Traits, _Allocator>& __lhs, 
            const _CharT*                                    __rhs)
{
    return !(__lhs < __rhs);
}


// *** BENTLEY_CHANGE
template <class _CharT, class _Traits , class _Allocator>
inline std::basic_ostream<_CharT, _Traits>&
operator<< (std::basic_ostream<_CharT, _Traits>&                  __os,
            const basic_string<_CharT, _Traits, _Allocator>& __str)
{
    __os << __str.c_str(); 
    return __os;
}

// *** BENTLEY_CHANGE
NAMESPACE_BENTLEY_BSTDCXX_END

#if !defined (DOCUMENTATION_GENERATOR)
#ifdef BENTLEY_CHANGE_REMOVED
#ifndef _RWSTD_RW_STRINGIO_H_INCLUDED
#  include <rw/_stringio.h>
#endif   // _RWSTD_RW_STRINGIO_H_INCLUDED
#endif

NAMESPACE_BENTLEY_BC__RW_BEGIN  //  BENTLEY_CHANGE

#ifndef _RWSTD_NO_FUNC_PARTIAL_SPEC

#  ifndef _RWSTD_NO_NONDEDUCED_CONTEXT
#    define _RWSTD_STRING_SIZE_TYPE \
            typename BENTLEY_BSTDCXX::basic_string<_CharT, _Traits, _Allocator>::size_type
#  else
#    define _RWSTD_STRING_SIZE_TYPE _RWSTD_SIZE_T
#  endif   // _RWSTD_NO_NONDEDUCED_CONTEXT

// more specialized version for basic_string<>; may be further specialized
// in user code for example on a user-defined allocator

template <class _CharT, class _Traits, class _Allocator>
inline _RWSTD_STRING_SIZE_TYPE
__rw_new_capacity (_RWSTD_STRING_SIZE_TYPE __size,
                   const BENTLEY_BSTDCXX::basic_string<_CharT, _Traits, _Allocator>*)
{
    typedef _RWSTD_STRING_SIZE_TYPE _RWSizeT;

    const _RWSizeT __ratio = _RWSizeT (  (_RWSTD_STRING_CAPACITY_RATIO << 10)
                                       / _RWSTD_RATIO_DIVIDER);

    const _RWSizeT __cap =   (__size >> 10) * __ratio
                           + (((__size & 0x3ff) * __ratio) >> 10);

    return (__size += _RWSTD_MINIMUM_STRING_CAPACITY) > __cap ? __size : __cap;
}                     

#else   // if defined (_RWSTD_NO_FUNC_PARTIAL_SPEC)

#  ifndef _RWSTD_NO_NONDEDUCED_CONTEXT
#    define _RWSTD_STRING_SIZE_TYPE(type)   type::size_type
#  else
#    define _RWSTD_STRING_SIZE_TYPE(ignore) _RWSTD_SIZE_T
#  endif   // _RWSTD_NO_NONDEDUCED_CONTEXT


// the following specializations of the __rw_new_capacity<> function template
// are provided for char and wchar_t; the general case is given in <memory>

_RWSTD_SPECIALIZED_FUNCTION
inline _RWSTD_STRING_SIZE_TYPE (BENTLEY_BSTDCXX::string)
__rw_new_capacity (_RWSTD_STRING_SIZE_TYPE (BENTLEY_BSTDCXX::string) __size,
                   const BENTLEY_BSTDCXX::string*)
{
    typedef _RWSTD_STRING_SIZE_TYPE (BENTLEY_BSTDCXX::string) _RWSizeT;

    const _RWSizeT __ratio = _RWSizeT (  (_RWSTD_STRING_CAPACITY_RATIO << 10)
                                       / _RWSTD_RATIO_DIVIDER);

    const _RWSizeT __cap =   (__size >> 10) * __ratio
                           + (((__size & 0x3ff) * __ratio) >> 10);

    return (__size += _RWSTD_MINIMUM_STRING_CAPACITY) > __cap ? __size : __cap;
}                     

_RWSTD_SPECIALIZED_FUNCTION
inline _RWSTD_STRING_SIZE_TYPE (BENTLEY_BSTDCXX::wstring)
__rw_new_capacity (_RWSTD_STRING_SIZE_TYPE (BENTLEY_BSTDCXX::wstring) __size,
                   const BENTLEY_BSTDCXX::wstring*)
{
    typedef _RWSTD_STRING_SIZE_TYPE (BENTLEY_BSTDCXX::wstring) _RWSizeT;

    const _RWSizeT __ratio = _RWSizeT (  (_RWSTD_STRING_CAPACITY_RATIO << 10)
                                       / _RWSTD_RATIO_DIVIDER);

    const _RWSizeT __cap =   (__size >> 10) * __ratio
                           + (((__size & 0x3ff) * __ratio) >> 10);

    return (__size += _RWSTD_MINIMUM_STRING_CAPACITY) > __cap  ? __size : __cap;
}                     

#endif   // _RWSTD_NO_FUNC_PARTIAL_SPEC

// clean up
#undef _RWSTD_STRING_SIZE_TYPE

NAMESPACE_BENTLEY_BC__RW_END  // BENTLEY_CHANGE

#endif  // DOCUMENTATION_GENERATOR

#include "basic_string_cc.h"

#endif  // _RWSTD_STRING_INCLUDED
