/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/btree/btree_set.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

// **************************************************************************
// *
// * NOTICE: This File contains modifications made by Bentley Systems where designated.
// *
// *************************************************************************/

// Copyright 2013 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// A btree_set<> implements the STL unique sorted associative container
// interface (a.k.a set<>) using a btree. A btree_multiset<> implements the STL
// multiple sorted associative container interface (a.k.a multiset<>) using a
// btree. See btree.h for details of the btree implementation and caveats.

#ifndef UTIL_BTREE_BTREE_SET_H__
#define UTIL_BTREE_BTREE_SET_H__

#include <functional>
#include <memory>

#include "btree.h"
#include "btree_container.h"

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================
/**
A Bentley supplied implementation @c std::set. This class is used in the Bentley APIs to 
avoid dependencies on compiler-supplied implementations of @c std::set that sometimes vary
with compiler settings or compiler versions. The @c bset class does not
suffer from these problems. This makes @c bset suitable for use in Bentley 
public APIs.
<p>
The bset implementation is based on the Google cpp-btree (see https://code.google.com/p/cpp-btree/).
It generally has much better performance than the r-b based @c std::set implementations, in terms of
memory usage, creation time, and lookup performance. However, it does suffer from the limitation that
inserts and deletes can invalidate all existing iterators. It also requires that all key types be 
default-constructable (see CAVEATS at Bentley/btree/btree.h).
<p>
There is also another Bentley supplied implementation of std::set called bstdset. bset is generally
the preferable std::set implementation for most use cases. If you cannot live with bset's restrictions,
you should use bstdset.
<p>
To understand the @c bset class, consult the documentation for @c std::set.
@see http://www.cplusplus.com/reference/set/set/
@ingroup BeCollectionsGroup
*/
template <typename Key, typename Compare = std::less<Key>, uint16_t EntriesPerNode = 32,
          typename Alloc = BentleyApi::BentleyAllocator<Key> >
class bset : public btree_unique_container<
  btree<btree_set_params<Key, Compare, Alloc, EntriesPerNode*sizeof(Key)> > > {

  enum { TargetNodeSize = EntriesPerNode*sizeof(Key)};
  typedef bset<Key, Compare, EntriesPerNode, Alloc> self_type;
  typedef btree_set_params<Key, Compare, Alloc, TargetNodeSize> params_type;
  typedef btree<params_type> btree_type;
  typedef btree_unique_container<btree_type> super_type;

 public:
  typedef typename btree_type::key_compare key_compare;
  typedef typename btree_type::allocator_type allocator_type;

 public:
  // Default constructor.
  bset(const key_compare &comp = key_compare(),
            const allocator_type &alloc = allocator_type())
      : super_type(comp, alloc) {
  }

  // Copy constructor.
  bset(const self_type &x)
      : super_type(x) {
  }

  // Range constructor.
  template <class InputIterator>
  bset(InputIterator b, InputIterator e,
            const key_compare &comp = key_compare(),
            const allocator_type &alloc = allocator_type())
      : super_type(b, e, comp, alloc) {
  }

  bool operator== (self_type const& __y) const {return this->size() == __y.size() && equal (this->begin(), this->end(), __y.begin());}
  bool operator<  (self_type const& __y) const {return lexicographical_compare (this->begin(), this->end(), __y.begin(), __y.end());}
  bool operator!= (self_type const& __y) const {return !(*this == __y);}
  bool operator>  (self_type const& __y) const {return (__y < *this);}
  bool operator>= (self_type const& __y) const {return !(*this < __y);}
  bool operator<= (self_type const& __y) const {return !(__y < *this);}
};

template <typename K, typename C, int N, typename A>
inline void swap(bset<K, C, N, A> &x, 
                 bset<K, C, N, A> &y) {
  x.swap(y);
}

// The btree_multiset class is needed mainly for its constructors.
template <typename Key, typename Compare = std::less<Key>, uint16_t EntriesPerNode = 32,
          typename Alloc = BentleyApi::BentleyAllocator<Key> >
class bmultiset : public btree_multi_container<
  btree<btree_set_params<Key, Compare, Alloc, EntriesPerNode*sizeof(Key)> > > {

  enum { TargetNodeSize = EntriesPerNode*sizeof(Key)};
  typedef bmultiset<Key, Compare, EntriesPerNode, Alloc> self_type;
  typedef btree_set_params<Key, Compare, Alloc, TargetNodeSize> params_type;
  typedef btree<params_type> btree_type;
  typedef btree_multi_container<btree_type> super_type;

 public:
  typedef typename btree_type::key_compare key_compare;
  typedef typename btree_type::allocator_type allocator_type;

 public:
  // Default constructor.
  bmultiset(const key_compare &comp = key_compare(),
                 const allocator_type &alloc = allocator_type())
      : super_type(comp, alloc) {
  }

  // Copy constructor.
  bmultiset(const self_type &x)
      : super_type(x) {
  }

  // Range constructor.
  template <class InputIterator>
  bmultiset(InputIterator b, InputIterator e,
                 const key_compare &comp = key_compare(),
                 const allocator_type &alloc = allocator_type())
      : super_type(b, e, comp, alloc) {
  }

  bool operator== (self_type const& __y) const {return this->size() == __y.size() && equal (this->begin(), this->end(), __y.begin());}
  bool operator<  (self_type const& __y) const {return lexicographical_compare (this->begin(), this->end(), __y.begin(), __y.end());}
  bool operator!= (self_type const& __y) const {return !(*this == __y);}
  bool operator>  (self_type const& __y) const {return (__y < *this);}
  bool operator>= (self_type const& __y) const {return !(*this < __y);}
  bool operator<= (self_type const& __y) const {return !(__y < *this);}
};

template <typename K, typename C, int N, typename A>
inline void swap(bmultiset<K, C, N, A> &x,
                 bmultiset<K, C, N, A> &y) {
  x.swap(y);
}

END_BENTLEY_NAMESPACE

#endif  // UTIL_BTREE_BTREE_SET_H__
