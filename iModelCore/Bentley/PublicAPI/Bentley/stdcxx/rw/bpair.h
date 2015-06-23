/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/stdcxx/rw/bpair.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

// -*- C++ -*-
/***************************************************************************
 *
 * _pair.h - definition of std::bpair
 *
 * $Id: _pair.h 538186 2007-05-15 14:23:59Z faridz $
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
 * Copyright 1994-2005 Rogue Wave Software.
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

#ifndef _RWSTD_RW_PAIR_H_INCLUDED
#define _RWSTD_RW_PAIR_H_INCLUDED

// *** BENTLEY_CHANGE
#include <Bentley/stdcxx/rw/_defs.h>

//#ifndef _RWSTD_RW_FUNCBASE_H_INCLUDED
//#  include <rw/_funcbase.h>   // for  less
//#endif   // _RWSTD_RW_FUNCBASE_H_INCLUDED


// *** BENTLEY_CHANGE
NAMESPACE_BENTLEY_BSTDCXX_BEGIN

//! A template that has many of the capabilities of @c std::pair
// 20.2.2
template <class _TypeT, class _TypeU>
struct bpair
{
    typedef _TypeT first_type;
    typedef _TypeU second_type;

    first_type  first;
    second_type second;

    // 20.2.2, p2
    bpair ()
#ifndef _RWSTD_NO_EMPTY_MEM_INITIALIZER
        : first (/* lwg issue 265 */), second () { }
#else
        // assumes types satisfy the CopyConstructible requirements
        : first (first_type ()), second (second_type ()) { }
#endif   // _RWSTD_NO_EMPTY_MEM_INITIALIZER

    // 20.2.2, p3
    bpair (const first_type &__x, const second_type &__y)
        : first (__x), second (__y) { }

    // 20.2.2, p4
    template <class _TypeX, class _TypeY>
    bpair (const bpair <_TypeX, _TypeY> &__rhs) 
        : first (__rhs.first), second (__rhs.second) { }

    template <class _TypeX, class _TypeY,
              class = typename std::enable_if<std::is_convertible<_TypeX, _TypeT>::value && std::is_convertible<_TypeY, _TypeY>::value, void>::type>
    bpair (bpair <_TypeX, _TypeY>&& __rhs) 
        : first(std::forward<_TypeX>(__rhs.first)), second (std::forward<_TypeY>(__rhs.second)) { }

    template<class _TypeX, class _TypeY>
    bpair<_TypeT, _TypeU>& operator=(bpair<_TypeX, _TypeY>&& __rhs)
        {
        first = std::forward<_TypeX>(__rhs.first);
        second = std::forward<_TypeY>(__rhs.second);
        return (*this);
        }

    // lwg issue 353
    template <class _TypeX, class _TypeY>
    bpair<_TypeT, _TypeU>& operator= (const bpair <_TypeX, _TypeY> &__rhs) {
        return first = __rhs.first, second = __rhs.second, *this;
    }

};

// 20.2.2, p5
template <class _TypeT, class _TypeU>
inline bool
operator== (const Bstdcxx::bpair<_TypeT, _TypeU>& __x, const Bstdcxx::bpair<_TypeT, _TypeU>& __y)
{ 
    return __x.first == __y.first && __x.second == __y.second; 
}


template <class _TypeT, class _TypeU>
inline bool
operator!= (const Bstdcxx::bpair<_TypeT, _TypeU>& __x, const Bstdcxx::bpair<_TypeT, _TypeU>& __y)
{ 
    return !(__x == __y);
}


// 20.2.2, p6
template <class _TypeT, class _TypeU>
inline bool
operator< (const Bstdcxx::bpair<_TypeT, _TypeU>& __x, const Bstdcxx::bpair<_TypeT, _TypeU>& __y)
{
    std::less<_TypeT> __lessT;

    // follows resolution of lwg issue 348
    return    __lessT (__x.first, __y.first)
           || (   !__lessT (__y.first, __x.first)
               &&  std::less<_TypeU>()(__x.second, __y.second));
}


template <class _TypeT, class _TypeU>
inline bool
operator> (const Bstdcxx::bpair<_TypeT, _TypeU>& __x, const Bstdcxx::bpair<_TypeT, _TypeU>& __y)
{ 
    return __y < __x;
}


template <class _TypeT, class _TypeU>
inline bool
operator>= (const Bstdcxx::bpair<_TypeT, _TypeU>& __x, const Bstdcxx::bpair<_TypeT, _TypeU>& __y)
{ 
    return !(__x < __y);
}


template <class _TypeT, class _TypeU>
inline bool
operator<= (const Bstdcxx::bpair<_TypeT, _TypeU>& __x, const Bstdcxx::bpair<_TypeT, _TypeU>& __y)
{ 
    return !(__y < __x);
}


// 20.2.2, p7, signature follows lwg issue 181
template <class _TypeT, class _TypeU>
inline Bstdcxx::bpair<_TypeT, _TypeU>
make_bpair (_TypeT __x, _TypeU __y)
{
    return Bstdcxx::bpair<_TypeT, _TypeU>(__x, __y);
}

// *** BENTLEY_CHANGE
NAMESPACE_BENTLEY_BSTDCXX_END

#endif   // _RWSTD_RW_PAIR_H_INCLUDED
