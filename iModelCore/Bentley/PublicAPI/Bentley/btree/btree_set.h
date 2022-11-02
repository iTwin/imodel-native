/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*
 * Copyright (c) 2019 German Mendez Bravo (Kronuz)
 * Copyright (c) 2013 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
 * A btree::set<> implements the STL unique sorted associative container
 * interface (a.k.a set<>) using a btree. See btree.h for details of the btree
 * implementation and caveats.
 */

#include "btree.h"

BEGIN_BENTLEY_NAMESPACE

// The set class is needed mainly for its constructors.
template <typename Key,
	typename Compare = std::less<Key>,
	uint16_t EntriesPerNode = 32,
	typename Alloc = BentleyApi::BentleyAllocator<Key>>
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
	bset(const key_compare& comp = key_compare(),
		const allocator_type& alloc = allocator_type())
		: super_type(comp, alloc) {
	}

	// Copy constructor.
	bset(const self_type& x)
		: super_type(x) {
	}

	// Range constructor.
	template <class InputIterator>
	bset(InputIterator b, InputIterator e,
		const key_compare& comp = key_compare(),
		const allocator_type& alloc = allocator_type())
		: super_type(b, e, comp, alloc) {
	}

  bool operator== (self_type const& __y) const {return this->size() == __y.size() && std::equal(this->begin(), this->end(), __y.begin());}
  bool operator<  (self_type const& __y) const {return lexicographical_compare (this->begin(), this->end(), __y.begin(), __y.end());}
  bool operator!= (self_type const& __y) const {return !(*this == __y);}
  bool operator>  (self_type const& __y) const {return (__y < *this);}
  bool operator>= (self_type const& __y) const {return !(*this < __y);}
  bool operator<= (self_type const& __y) const {return !(__y < *this);}
};

template <typename K, typename C, int N, typename A>
inline void swap(bset<K, C, N, A>& x, bset<K, C, N, A>& y) {
	x.swap(y);
}

// The multiset class is needed mainly for its constructors.
template <typename Key,
					typename Compare = std::less<Key>,
					int TargetNodeSize = 32,
					typename Alloc = BentleyApi::BentleyAllocator<Key>>
class bmultiset : public btree_multi_container<
	btree<btree_set_params<Key, Compare, Alloc, TargetNodeSize> > > {

	typedef bmultiset<Key, Compare, TargetNodeSize, Alloc> self_type;
	typedef btree_set_params<Key, Compare, Alloc, TargetNodeSize> params_type;
	typedef btree<params_type> btree_type;
	typedef btree_multi_container<btree_type> super_type;

 public:
	typedef typename btree_type::key_compare key_compare;
	typedef typename btree_type::allocator_type allocator_type;

 public:
	// Default constructor.
	bmultiset(const key_compare& comp = key_compare(),
			 const allocator_type& alloc = allocator_type())
		: super_type(comp, alloc) {
	}

	// Copy constructor.
	bmultiset(const self_type& x)
		: super_type(x) {
	}

	// Range constructor.
	template <class InputIterator>
	bmultiset(InputIterator b, InputIterator e,
			 const key_compare& comp = key_compare(),
			 const allocator_type& alloc = allocator_type())
		: super_type(b, e, comp, alloc) {
	}

  bool operator== (self_type const& __y) const {return this->size() == __y.size() && std::equal(this->begin(), this->end(), __y.begin());}
  bool operator<  (self_type const& __y) const {return lexicographical_compare (this->begin(), this->end(), __y.begin(), __y.end());}
  bool operator!= (self_type const& __y) const {return !(*this == __y);}
  bool operator>  (self_type const& __y) const {return (__y < *this);}
  bool operator>= (self_type const& __y) const {return !(*this < __y);}
  bool operator<= (self_type const& __y) const {return !(__y < *this);}
};

template <typename K, typename C, int N, typename A>
inline void swap(bmultiset<K, C, N, A>& x, bmultiset<K, C, N, A>& y) {
	x.swap(y);
}

END_BENTLEY_NAMESPACE
