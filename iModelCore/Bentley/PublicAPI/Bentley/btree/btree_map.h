/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/btree/btree_map.h $
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
// A bmap<> implements the STL unique sorted associative container
// interface and the pair associative container interface (a.k.a map<>) using a
// btree. A btree_multimap<> implements the STL multiple sorted associative
// container interface and the pair associative container interface (a.k.a
// multimap<>) using a btree. See btree.h for details of the btree
// implementation and caveats.

#ifndef BENTLEY_UTIL_BTREE_bmap_H__
#define BENTLEY_UTIL_BTREE_bmap_H__

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <utility>

#include "btree.h"
#include "btree_container.h"

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================
/**
A %Bentley supplied implementation @c std::map. This class is used in the %Bentley APIs to 
avoid dependencies on compiler-supplied implementations of @c std::map that sometimes vary
with compiler settings or compiler versions. The @c bmap class does not
suffer from these problems. This makes @c bmap suitable for use in %Bentley 
public APIs.
<p>
The bmap implementation is based on the Google cpp-btree (see https://code.google.com/p/cpp-btree/).
It generally has much better performance than the r-b based @c std::map implementations, in terms of
memory usage, creation time, and lookup performance. However, it does suffer from the limitation that
inserts and deletes can invalidate all existing iterators. It also requires that all key types be 
default-constructable (see CAVEATS at Bentley/btree/btree.h).
<p>
There is also another %Bentley supplied implementation of std::map called bstdmap. bmap is generally
the preferable std::map implementation for most use cases. If you cannot live with bmap's restrictions,
you should use bstdmap.
<p>
To understand the @c bmap class, consult the documentation for @c std::map.
@see http://www.cplusplus.com/reference/map/map/
@ingroup BeCollectionsGroup
*/
//=======================================================================================
template <typename Key, typename Value, typename Compare = std::less<Key>, uint16_t EntriesPerNode = 32,
          typename Alloc = BentleyApi::BentleyAllocator<bpair<const Key, Value> > >
class bmap : public bmap_container<
      btree<bmap_params<Key, Value, Compare, Alloc, EntriesPerNode*sizeof(bpair<Key, Value>) > > > {

  typedef bpair<Key,Value> pair_type;
  enum { TargetNodeSize = EntriesPerNode*sizeof(pair_type)};

  typedef bmap<Key, Value, Compare, EntriesPerNode, Alloc> self_type;
  typedef bmap_params<Key, Value, Compare, Alloc, TargetNodeSize> params_type;
  typedef btree<params_type> btree_type;
  typedef bmap_container<btree_type> super_type;

 public:
  typedef typename btree_type::key_compare key_compare;
  typedef typename btree_type::allocator_type allocator_type;
  typedef typename btree<bmap_params<Key, Value, Compare, Alloc, EntriesPerNode*sizeof(bpair<Key, Value>) > >::iterator iterator;

 public:
  // Default constructor.
  bmap(const key_compare &comp = key_compare(),
            const allocator_type &alloc = allocator_type())
      : super_type(comp, alloc) {
  }

  // Copy constructor.
  bmap(const self_type &x)
      : super_type(x) {
  }

  // Range constructor.
  template <class InputIterator>
  bmap(InputIterator b, InputIterator e,
            const key_compare &comp = key_compare(),
            const allocator_type &alloc = allocator_type())
      : super_type(b, e, comp, alloc) {
  }

  // shortcut to insert without having to create pair.  - BENTLEY Extension
  bpair<iterator,bool> Insert(Key k, Value v) {return this->insert(pair_type(k,v));}

  bool operator== (self_type const& __y) const {return this->size() == __y.size() && equal (this->begin(), this->end(), __y.begin());}
  bool operator<  (self_type const& __y) const {return lexicographical_compare (this->begin(), this->end(), __y.begin(), __y.end());}
  bool operator!= (self_type const& __y) const {return !(*this == __y);}
  bool operator>  (self_type const& __y) const {return (__y < *this);}
  bool operator>= (self_type const& __y) const {return !(*this < __y);}
  bool operator<= (self_type const& __y) const {return !(__y < *this);}
};

template <typename K, typename V, typename C, int N, typename A>
inline void swap(bmap<K, V, C, N, A> &x,
                 bmap<K, V, C, N, A> &y) {
  x.swap(y);
}

// The btree_multimap class is needed mainly for its constructors.
template <typename Key, typename Value, typename Compare = std::less<Key>, uint16_t EntriesPerNode = 32,
          typename Alloc = BentleyApi::BentleyAllocator<bpair<const Key, Value> > >
class bmultimap : public btree_multi_container<
  btree<bmap_params<Key, Value, Compare, Alloc, EntriesPerNode*sizeof(bpair<Key, Value>)> > > {

  typedef bpair<Key,Value> pair_type;
  enum { TargetNodeSize = EntriesPerNode*sizeof(pair_type)};

  typedef bmultimap<Key, Value, Compare, EntriesPerNode, Alloc> self_type;
  typedef bmap_params<Key, Value, Compare, Alloc, TargetNodeSize> params_type;
  typedef btree<params_type> btree_type;
  typedef btree_multi_container<btree_type> super_type;

 public:
  typedef typename btree_type::key_compare key_compare;
  typedef typename btree_type::allocator_type allocator_type;
  typedef typename btree_type::data_type data_type;
  typedef typename btree_type::mapped_type mapped_type;
  typedef typename btree<bmap_params<Key, Value, Compare, Alloc, EntriesPerNode*sizeof(bpair<Key, Value>)> >::iterator iterator;

 public:
  // Default constructor.
  bmultimap(const key_compare &comp = key_compare(),
                 const allocator_type &alloc = allocator_type())
      : super_type(comp, alloc) {
  }

  // Copy constructor.
  bmultimap(const self_type &x)
      : super_type(x) {
  }

  // Range constructor.
  template <class InputIterator>
  bmultimap(InputIterator b, InputIterator e,
                 const key_compare &comp = key_compare(),
                 const allocator_type &alloc = allocator_type())
      : super_type(b, e, comp, alloc) {
  }

  // shortcut to insert without having to create pair.  - BENTLEY Extension
  iterator Insert(Key k, Value v) {return this->insert(pair_type(k,v));}

  bool operator== (self_type const& __y) const {return this->size() == __y.size() && equal (this->begin(), this->end(), __y.begin());}
  bool operator<  (self_type const& __y) const {return lexicographical_compare (this->begin(), this->end(), __y.begin(), __y.end());}
  bool operator!= (self_type const& __y) const {return !(*this == __y);}
  bool operator>  (self_type const& __y) const {return (__y < *this);}
  bool operator>= (self_type const& __y) const {return !(*this < __y);}
  bool operator<= (self_type const& __y) const {return !(__y < *this);}
};

template <typename K, typename V, typename C, int N, typename A>
inline void swap(bmultimap<K, V, C, N, A> &x,
                 bmultimap<K, V, C, N, A> &y) {
  x.swap(y);
}
END_BENTLEY_NAMESPACE

#endif  // BENTLEY_UTIL_BTREE_bmap_H__
