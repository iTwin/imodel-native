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
 * A btree implementation of the STL set and map interfaces. A btree is both
 * smaller and faster than STL set/map. The red-black tree implementation of
 * STL set/map has an overhead of 3 pointers (left, right and parent) plus the
 * node color information for each stored value. So a set<int32> consumes 20
 * bytes for each value stored. This btree implementation stores multiple
 * values on fixed size nodes (usually 256 bytes) and doesn't store child
 * pointers for leaf nodes. The result is that a btree::set<int32> may use much
 * less memory per stored value. For the random insertion benchmark in
 * btree_test.cc, a btree::set<int32> with node-size of 256 uses 4.9 bytes per
 * stored value.
 *
 * The packing of multiple values on to each node of a btree has another effect
 * besides better space utilization: better cache locality due to fewer cache
 * lines being accessed. Better cache locality translates into faster
 * operations.
 *
 * CAVEATS
 *
 * Insertions and deletions on a btree can cause splitting, merging or
 * rebalancing of btree nodes. And even without these operations, insertions
 * and deletions on a btree will move values around within a node. In both
 * cases, the result is that insertions and deletions can invalidate iterators
 * pointing to values other than the one being inserted/deleted. This is
 * notably different from STL set/map which takes care to not invalidate
 * iterators on insert/erase except, of course, for iterators pointing to the
 * value being erased.  A partial workaround when erasing is available:
 * erase() returns an iterator pointing to the item just after the one that was
 * erased (or end() if none exists).
 *
 */
#include <Bentley/WString.h>
#include <Bentley/bpair.h>
#include <Bentley/BentleyAllocator.h>

#include <assert.h>
#include <stddef.h>
#include <sys/types.h>

#include <algorithm>
#include <functional>
#include <iterator>
#include <limits>
#include <memory>
#include <new>
#include <string>
#include <type_traits>
#include <utility>

#ifdef max
#undef max
#undef min
#endif

BEGIN_BENTLEY_NAMESPACE

#if defined(_MSC_VER)
typedef intptr_t ssize_t;
#endif

// Inside a btree method, if we just call swap(), it will choose the
// btree::swap method, which we don't want. And we can't say ::swap
// because then MSVC won't pickup any std::swap() implementations. We
// can't just use std::swap() directly because then we don't get the
// specialization for types outside the std namespace. So the solution
// is to have a special swap helper function whose name doesn't
// collide with other swap functions defined by the btree classes.
template <typename T>
inline void btree_swap_helper(T& a, T& b) {
	using std::swap;
	swap(a, b);
}

// Types small_ and big_ are promise that sizeof(small_) < sizeof(big_)
typedef char small_;

struct big_ {
	char dummy[2];
};

// A compile-time assertion.
template <bool>
struct CompileAssert { };

#define COMPILE_ASSERT(expr, msg) \
	typedef CompileAssert<(bool(expr))> msg[bool(expr) ? 1 : -1]

struct btree_extract_key_fail_tag { };
struct btree_extract_key_self_tag { };
struct btree_extract_key_first_tag { };

template <typename ValueType, typename Key,
		  typename RawValueType = typename std::remove_const<typename std::remove_reference<ValueType>::type>::type>
struct btree_can_extract_key
	: std::conditional<std::is_same<RawValueType, Key>::value,
		btree_extract_key_self_tag,
		btree_extract_key_fail_tag>::type { };

template <typename _Pair, typename Key, typename First, typename Second>
struct btree_can_extract_key<_Pair, Key, std::pair<First, Second>>
	: std::conditional<std::is_same<typename std::remove_const<First>::type, Key>::value,
		btree_extract_key_first_tag,
		btree_extract_key_fail_tag>::type { };

// btree_can_extract_map_key uses true_type/false_type instead of the tags.
// It returns true if Key != ContainerValueType (the container is a map not a set)
// and ValueType == Key.
template <typename ValueType, typename Key, typename ContainerValueType,
		  typename RawValueType = typename std::remove_const<typename std::remove_reference<ValueType>::type>::type>
struct btree_can_extract_map_key : std::integral_constant<bool, std::is_same<RawValueType, Key>::value> { };

// This specialization returns btree_extract_key_fail_tag for non-map containers
// because Key == ContainerValueType
template <typename ValueType, typename Key, typename RawValueType>
struct btree_can_extract_map_key<ValueType, Key, Key, RawValueType> : std::false_type { };

template <typename Key, typename Compare, typename Alloc, int TargetNodeSize, int ValueSize>
struct btree_common_params {
	using key_compare = Compare;
	using key_type = Key;
	using size_type = ssize_t;
	using difference_type = ptrdiff_t;
	using allocator_type = Alloc;
	using allocator_traits = std::allocator_traits<allocator_type>;
	using internal_allocator_type = typename allocator_traits::template rebind_alloc<char>;
	using internal_allocator_traits = std::allocator_traits<internal_allocator_type>;

	enum {
		kTargetNodeSize = TargetNodeSize,

		// Available space for values. This is largest for leaf nodes, that have overhead no fewer than two pointers.
		kNodeValueSpace = TargetNodeSize - 2 * sizeof(void*),
	};

	// This is an integral type large enough to hold as many
	// ValueSize-values as will fit a node of TargetNodeSize bytes.
	using node_count_type = std::conditional_t<(kNodeValueSpace / ValueSize) >= 256, uint16_t, uint8_t>;
};

// A parameters structure for holding the type parameters for a map.
template <typename Key, typename Data, typename Compare, typename Alloc, int TargetNodeSize>
struct btree_map_params : public btree_common_params<Key, Compare, Alloc, TargetNodeSize, sizeof(Key) + sizeof(Data)> {
	using data_type = Data;
	using mapped_type = Data;
	using value_type = std::pair<const Key, data_type>;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using reference = value_type&;
	using const_reference = const value_type&;

	enum {
		kValueSize = sizeof(Key) + sizeof(data_type),
	};

	static void swap(value_type& a, value_type& b) {
		BentleyApi::btree_swap_helper(const_cast<Key&>(a.first), const_cast<Key&>(b.first));
		BentleyApi::btree_swap_helper(a.second, b.second);
	}

	static const Key& get_key(const value_type& x) {
		return x.first;
	}
};

// A parameters structure for holding the type parameters for a btree_set.
template <typename Key, typename Compare, typename Alloc, int TargetNodeSize>
struct btree_set_params : public btree_common_params<Key, Compare, Alloc, TargetNodeSize, sizeof(Key)> {
	using data_type = std::false_type;
	using mapped_type = std::false_type;
	using value_type = Key;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using reference = value_type&;
	using const_reference = const value_type&;

	enum {
		kValueSize = sizeof(Key),
	};

	static void swap(value_type& a, value_type& b) {
		BentleyApi::btree_swap_helper(a, b);
	}

	static const Key& get_key(const value_type& x) {
		return x;
	}
};

// Dispatch helper class for using linear search
template <typename K, typename N, typename Compare>
struct btree_linear_search_compare {
	static int lower_bound(const K& k, const N& n, Compare comp)  {
		return n.linear_search_lower(k, 0, n.count(), comp);
	}
	static int upper_bound(const K& k, const N& n, Compare comp)  {
		return n.linear_search_upper(k, 0, n.count(), comp);
	}
};

// Dispatch helper class for using binary search
template <typename K, typename N, typename Compare>
struct btree_binary_search_compare {
	static int lower_bound(const K& k, const N& n, Compare comp)  {
		return n.binary_search_lower(k, 0, n.count(), comp);
	}
	static int upper_bound(const K& k, const N& n, Compare comp)  {
		return n.binary_search_upper(k, 0, n.count(), comp);
	}
};

// A node in the btree holding. The same node type is used for both internal
// and leaf nodes in the btree, though the nodes are allocated in such a way
// that the children array is only valid in internal nodes.
template <typename Params>
class btree_node {
public:
	using params_type = Params;
	using self_type = btree_node<Params>;
	using key_type = typename Params::key_type;
	using data_type = typename Params::data_type;
	using value_type = typename Params::value_type;
	using pointer = typename Params::pointer;
	using const_pointer = typename Params::const_pointer;
	using reference = typename Params::reference;
	using const_reference = typename Params::const_reference;
	using key_compare = typename Params::key_compare;
	using size_type = typename Params::size_type;
	using difference_type = typename Params::difference_type;
	using allocator_type = typename Params::allocator_type;
	using allocator_traits = typename Params::allocator_traits;

	template<typename K> using linear_search_type =  btree_linear_search_compare<K, self_type, key_compare>;
	template<typename K> using binary_search_type = btree_binary_search_compare<K, self_type, key_compare>;

	// If the key is an integral or floating point type, use linear search which
	// is faster than binary search for such types. Might be wise to also
	// configure linear search based on node-size.
	template<typename K> using search_type = std::conditional_t<std::is_integral<key_type>::value || std::is_floating_point<key_type>::value,
		linear_search_type<K>, binary_search_type<K>>;

	struct base_fields {
		using field_type = typename Params::node_count_type;

		// A boolean indicating whether the node is a leaf or not.
		bool leaf;
		// The position of the node in the node's parent.
		field_type position;
		// The maximum number of values the node can hold.
		field_type max_count;
		// The count of the number of values in the node.
		field_type count;
		// A pointer to the node's parent.
		btree_node* parent;
	};

	enum {
		kValueSize = params_type::kValueSize,
		kTargetNodeSize = params_type::kTargetNodeSize,

		// Compute how many values we can fit onto a leaf node.
		kNodeTargetValues = (kTargetNodeSize - sizeof(base_fields)) / kValueSize,
		// We need a minimum of 3 values per internal node in order to perform
		// splitting (1 value for the two nodes involved in the split and 1 value
		// propagated to the parent as the delimiter for the split).
		kNodeValues = kNodeTargetValues >= 3 ? kNodeTargetValues : 3,

		kExactMatch = 1 << 30,
		kMatchMask = kExactMatch - 1,
	};

	struct leaf_fields : public base_fields {
		// The array of values. Only the first count of these values have been
		// constructed and are valid.
		value_type values[kNodeValues];
	};

	struct internal_fields : public leaf_fields {
		// The array of child pointers. The keys in children_[i] are all less than
		// key(i). The keys in children_[i + 1] are all greater than key(i). There
		// are always count + 1 children.
		btree_node* children[kNodeValues + 1];
	};

	struct root_fields : public internal_fields {
		btree_node* rightmost;
		size_type size;
	};

public:
	// Getter/setter for whether this is a leaf node or not. This value doesn't
	// change after the node is created.
	bool is_leaf() const {
		return fields_.leaf;
	}

	// Getter for the position of this node in its parent.
	int position() const {
		return fields_.position;
	}

	// Getter/setter for the number of values stored in this node.
	int count() const {
		return fields_.count;
	}
	void set_count(int v) {
		assert(v >= 0);
		assert(v <= fields_.max_count);
		fields_.count = (typename leaf_fields::field_type) v;
	}
	int max_count() const {
		return fields_.max_count;
	}

	// Getter for the parent of this node.
	btree_node* get_parent() const {
		return fields_.parent;
	}
	// Getter for whether the node is the root of the tree. The parent of the
	// root of the tree is the leftmost node in the tree which is guaranteed to
	// be a leaf.
	bool is_root() const {
		return fields_.parent->is_leaf();
	}
	void make_root() {
		assert(fields_.parent->is_root());
		fields_.parent = fields_.parent->get_parent();
	}

	// Getter for the rightmost root node field. Only valid on the root node.
	btree_node* rightmost() const {
		return fields_.rightmost;
	}
	btree_node*& __rightmost() {
		return fields_.rightmost;
	}

	// Getter for the size root node field. Only valid on the root node.
	size_type size() const {
		return fields_.size;
	}
	size_type& __size() {
		return fields_.size;
	}

	// Getters for the key/value at position i in the node.
	const key_type& key(int i) const {
		return params_type::get_key(fields_.values[i]);
	}
	reference value(int i) {
		return reinterpret_cast<reference>(fields_.values[i]);
	}
	const_reference value(int i) const {
		return reinterpret_cast<const_reference>(fields_.values[i]);
	}

	// Swap value i in this node with value j in node x.
	void swap_value(int i, btree_node* x, int j) {
		assert(x != this || i != j);
		params_type::swap(fields_.values[i], x->fields_.values[j]);
	}

	// Swap value i in this node with value i in node x.
	void swap_value(int i, btree_node* x) {
		assert(x != this);
		params_type::swap(fields_.values[i], x->fields_.values[i]);
	}

	// Swap value i with value j.
	void swap_value(int i, int j) {
		assert(i != j);
		params_type::swap(fields_.values[i], fields_.values[j]);
	}

	// Move value i in this node to value j in node x.
	void move_value(int i, btree_node* x, int j) {
		assert(x != this || i != j);
		x->construct_value(j, std::move(fields_.values[i]));
		destroy_value(i);
	}

	// Move value i in this node to value i in node x.
	void move_value(int i, btree_node* x) {
		assert(x != this);
		x->construct_value(i, std::move(fields_.values[i]));
		destroy_value(i);
	}

	// Move value i to value j.
	void move_value(int i, int j) {
		assert(i != j);
		construct_value(j, std::move(fields_.values[i]));
		destroy_value(i);
	}

	// Getters/setter for the child at position i in the node.
	btree_node* child(int i) const {
		assert(!is_leaf());
		return fields_.children[i];
	}

#ifndef NDEBUG
	void reset_child(int i) {
		assert(!is_leaf());
		fields_.children[i] = nullptr;
	}
#else
	void reset_child(int) {
	}
#endif

	void set_child(int i, btree_node* c) {
		assert(!is_leaf());
		assert(c != nullptr);
		fields_.children[i] = c;
		c->fields_.parent = this;
		c->fields_.position = (typename leaf_fields::field_type) i;
	}

	// Swap child i in this node with child j in node x.
	void swap_child(int i, btree_node* x, int j) {
		assert(x != this || i != j);
		assert(!is_leaf());
		assert(!x->is_leaf());
		assert(fields_.children[i] != nullptr);
		assert(x->fields_.children[j] != nullptr);
		auto& a = fields_.children[i];
		auto& b = x->fields_.children[j];
		BentleyApi::btree_swap_helper(a, b);
		a->fields_.parent = this;
		a->fields_.position = i;
		b->fields_.parent = x;
		b->fields_.position = j;
	}

	// Swap child i in this node with child i in node x.
	void swap_child(int i, btree_node* x) {
		assert(x != this);
		assert(!is_leaf());
		assert(!x->is_leaf());
		assert(fields_.children[i] != nullptr);
		assert(x->fields_.children[i] != nullptr);
		auto& a = fields_.children[i];
		auto& b = x->fields_.children[i];
		BentleyApi::btree_swap_helper(a, b);
		a->fields_.parent = this;
		b->fields_.parent = x;
	}

	// Swap child i with child j.
	void swap_child(int i, int j) {
		assert(i != j);
		assert(!is_leaf());
		assert(fields_.children[i] != nullptr);
		assert(fields_.children[j] != nullptr);
		auto& a = fields_.children[i];
		auto& b = fields_.children[j];
		BentleyApi::btree_swap_helper(a, b);
		a->fields_.position = i;
		b->fields_.position = j;
	}

	// Move child i in this node to child j in node x.
	void move_child(int i, btree_node* x, int j) {
		assert(x != this || i != j);
		assert(!is_leaf());
		assert(!x->is_leaf());
		assert(fields_.children[i] != nullptr);
		auto c = fields_.children[i];
		x->fields_.children[j] = c;
		c->fields_.position = (typename leaf_fields::field_type) j;
		c->fields_.parent = x;
#ifndef NDEBUG
		fields_.children[i] = nullptr;
#endif
	}

	// Move child i in this node to child i in node x.
	void move_child(int i, btree_node* x) {
		assert(x != this);
		assert(!is_leaf());
		assert(!x->is_leaf());
		assert(fields_.children[i] != nullptr);
		auto c = fields_.children[i];
		x->fields_.children[i] = c;
		c->fields_.parent = x;
#ifndef NDEBUG
		fields_.children[i] = nullptr;
#endif
	}

	// Move child i to child j.
	void move_child(int i, int j) {
		assert(i != j);
		assert(!is_leaf());
		assert(fields_.children[i] != nullptr);
		auto c = fields_.children[i];
		fields_.children[j] = c;
		c->fields_.position = (typename leaf_fields::field_type) j;

#ifndef NDEBUG
		fields_.children[i] = nullptr;
#endif
	}

	// Returns the position of the first value whose key is not less than k.
	template <typename Compare, typename K = key_type>
	int lower_bound(const K& k, const Compare& comp) const {
		return search_type<K>::lower_bound(k, *this, comp);
	}
	// Returns the position of the first value whose key is greater than k.
	template <typename Compare, typename K = key_type>
	int upper_bound(const K& k, const Compare& comp) const {
		return search_type<K>::upper_bound(k, *this, comp);
	}

	// Returns the position of the first value whose key is not less than k using linear search
	template <typename Compare, typename K = key_type>
	int linear_search_lower(const K& k, int s, int e, const Compare& comp) const {
		while (s < e) {
			if (!comp(key(s), k)) {
				break;
			}
			++s;
		}
		return s;
	}
	// Returns the position of the last value whose key is less than k using linear search
	template <typename Compare, typename K = key_type>
	int linear_search_upper(const K& k, int s, int e, const Compare& comp) const {
		while (s < e) {
			if (comp(k, key(s))) {
				break;
			}
			++s;
		}
		return s;
	}

	// Returns the position of the first value whose key is not less than k using binary search
	template <typename Compare, typename K = key_type>
	int binary_search_lower(const K& k, int s, int e, const Compare& comp) const {
		while (s != e) {
			int mid = (s + e) / 2;
			if (comp(key(mid), k)) {
				s = mid + 1;
			} else {
				e = mid;
			}
		}
		return s;
	}
	// Returns the position of the last value whose key is less than k using binary search
	template <typename Compare, typename K = key_type>
	int binary_search_upper(const K& k, int s, int e, const Compare& comp) const {
		while (s != e) {
			int mid = (s + e) / 2;
			if (!comp(k, key(mid))) {
				s = mid + 1;
			} else {
				e = mid;
			}
		}
		return s;
	}

	// Inserts the value x at position i, shifting all existing values
	// and children at positions >= i to the right by 1.
	template <typename... Args>
	void insert_value(int i, Args&&... args);

	// Removes the value at position i, shifting all existing values
	// and children at positions > i to the left by 1.
	void remove_value(int i);

	// Rebalances a node with its right sibling.
	void rebalance_right_to_left(btree_node* sibling, int to_move);
	void rebalance_left_to_right(btree_node* sibling, int to_move);

	// Splits a node, moving a portion of the node's values to its right sibling.
	void split(btree_node* sibling, int insert_position);

	// Merges a node with its right sibling, moving all of the values
	// and the delimiting key in the parent node onto itself.
	void merge(btree_node* sibling);

	// Swap the contents of "this" and "src".
	void swap(btree_node* src);

	// Node allocation/deletion routines.
	static btree_node* init_leaf(leaf_fields* f, btree_node* parent, int max_count) {
		btree_node* n = reinterpret_cast<btree_node*>(f);
		f->leaf = 1;
		f->position = 0;
		f->max_count = (typename leaf_fields::field_type) max_count;
		f->count = 0;
		f->parent = parent;
#if defined REMOVED_BY_BENTLEY
		assert(memset(&f->values, 0, max_count * sizeof(value_type)));
#endif
		return n;
	}

	static btree_node* init_internal(internal_fields* f, btree_node* parent) {
		btree_node* n = init_leaf(f, parent, kNodeValues);
		f->leaf = 0;
#if defined REMOVED_BY_BENTLEY
		assert(memset(f->children, 0, sizeof(f->children)));
#endif
		return n;
	}

	static btree_node* init_root(root_fields* f, btree_node* parent) {
		btree_node* n = init_internal(f, parent);
		f->rightmost = parent;
		f->size = parent->count();
		return n;
	}

	void destroy() {
		for (int i = 0; i < count(); ++i) {
			destroy_value(i);
		}
	}

private:
#if defined REMOVED_BY_BENTLEY
	static constexpr const char zero_value[sizeof(value_type)] = {};
#endif

	template <typename... Args>
	void construct_value(value_type* v, Args&&... args) {

#if defined REMOVED_BY_BENTLEY
		assert(memcmp(zero_value, v, sizeof(value_type)) == 0);
#endif
		new (v) value_type(std::forward<Args>(args)...);
		// FIXME: The above should use allocator_traits, but allocator
		// object is not available here at the nodes, is in the tree:
		// allocator_type& alloc = allocator();
		// allocator_traits::construct(alloc, v, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void construct_value(int i, Args&&... args) {
		assert(i >= 0);
		assert(i < fields_.max_count);
		construct_value(&fields_.values[i], std::forward<Args>(args)...);
	}

	void destroy_value(value_type* v) {
		v->~value_type();
		// FIXME: The above should use allocator_traits, but allocator
		// object is not available here at the nodes, is in the tree:
		// allocator_type& alloc = allocator();
		// allocator_traits::destroy(alloc, v);

#if defined REMOVED_BY_BENTLEY
		assert(memcpy(v, zero_value, sizeof(value_type)));
#endif
	}

	void destroy_value(int i) {
		assert(i >= 0);
		assert(i < fields_.max_count);
		destroy_value(&fields_.values[i]);
	}

private:
	root_fields fields_;

private:
	btree_node(const btree_node&);
	void operator=(const btree_node&);
};

template <typename Node, typename Reference, typename Pointer>
class btree_iterator {
public:
	using key_type = typename Node::key_type;
	using size_type = typename Node::size_type;
	using difference_type = typename Node::difference_type;
	using params_type = typename Node::params_type;
	using node_type = Node;
	using normal_node = typename std::remove_const<Node>::type;
	using const_node = const Node;
	using value_type = typename params_type::value_type;
	using normal_pointer = typename params_type::pointer;
	using normal_reference = typename params_type::reference;
	using const_pointer = typename params_type::const_pointer;
	using const_reference = typename params_type::const_reference;
	using pointer = Pointer;
	using reference = Reference;
	using iterator_category = std::bidirectional_iterator_tag;
	using iterator = btree_iterator<normal_node, normal_reference, normal_pointer>;
	using const_iterator = btree_iterator<const_node, const_reference, const_pointer>;
	using self_type = btree_iterator<Node, Reference, Pointer>;

	btree_iterator()
		: node(nullptr),
		  position(-1) {
	}
	btree_iterator(const iterator& x)
		: node(x.node),
		  position(x.position) {
	}

	bool operator==(const const_iterator& x) const {
		return node == x.node && position == x.position;
	}
	bool operator!=(const const_iterator& x) const {
		return node != x.node || position != x.position;
	}

	// Accessors for the key/value the iterator is pointing at.
	const key_type& key() const {
		return node->key(position);
	}
	reference operator*() const {
		return node->value(position);
	}
	pointer operator->() const {
		return &node->value(position);
	}

	self_type& operator++() {
		increment();
		return *this;
	}
	self_type& operator--() {
		decrement();
		return *this;
	}
	self_type operator++(int) {
		self_type tmp =* this;
		++*this;
		return tmp;
	}
	self_type operator--(int) {
		self_type tmp =* this;
		--*this;
		return tmp;
	}
	// Increment/decrement the iterator.
	void increment() {
		if (node->is_leaf() && ++position < node->count()) {
			return;
		}
		increment_slow();
	}
	void decrement() {
		if (node->is_leaf() && --position >= 0) {
			return;
		}
		decrement_slow();
	}

private:
	btree_iterator(Node* n, int p)
		: node(const_cast<normal_node*>(n)),
		  position(p) {
	}

	void increment_by(int count);
	void increment_slow();

	void decrement_slow();

	// The node in the tree the iterator is pointing at.
	normal_node* node;
	// The position within the node of the tree the iterator is pointing at.
	int position;

	template <typename> friend class btree;
	friend iterator;
	friend const_iterator;
};

// Dispatch helper class for using btree::internal_locate with plain compare.
struct btree_internal_locate_compare {
	template <typename K, typename T, typename Iter>
	static std::pair<Iter, int> dispatch(const K& k, const T& t, Iter iter) {
		return t.internal_locate_compare(k, iter);
	}
};

template <typename Params>
class btree : public Params::key_compare {
	using self_type = btree<Params>;
	using node_type = btree_node<Params>;
	using base_fields = typename node_type::base_fields;
	using leaf_fields = typename node_type::leaf_fields;
	using internal_fields = typename node_type::internal_fields;
	using root_fields = typename node_type::root_fields;

	friend struct btree_internal_locate_compare;
	using internal_locate_type = btree_internal_locate_compare;

	enum {
		kNodeValues = node_type::kNodeValues,
		kMinNodeValues = kNodeValues / 2,
		kValueSize = node_type::kValueSize,
		kExactMatch = node_type::kExactMatch,
		kMatchMask = node_type::kMatchMask,
	};

	// A helper class to get the empty base class optimization for 0-size
	// allocators. Base is internal_allocator_type.
	// (e.g. empty_base_handle<internal_allocator_type, node_type*>). If Base is
	// 0-size, the compiler doesn't have to reserve any space for it and
	// sizeof(empty_base_handle) will simply be sizeof(Data). Google [empty base
	// class optimization] for more details.
	template <typename Base, typename Data>
	struct empty_base_handle : public Base {
		empty_base_handle(const Base& b, const Data& d)
			: Base(b),
			  data(d) {
		}
		Data data;
	};

	struct node_stats {
		node_stats(ssize_t l, ssize_t i)
			: leaf_nodes(l),
			  internal_nodes(i) {
		}

		node_stats& operator+=(const node_stats& x) {
			leaf_nodes += x.leaf_nodes;
			internal_nodes += x.internal_nodes;
			return *this;
		}

		ssize_t leaf_nodes;
		ssize_t internal_nodes;
	};

public:
	using params_type = Params;
	using key_type = typename Params::key_type;
	using data_type = typename Params::data_type;
	using mapped_type = typename Params::mapped_type;
	using value_type = typename Params::value_type;
	using key_compare = typename Params::key_compare;
	using pointer = typename Params::pointer;
	using const_pointer = typename Params::const_pointer;
	using reference = typename Params::reference;
	using const_reference = typename Params::const_reference;
	using size_type = typename Params::size_type;
	using difference_type = typename Params::difference_type;
	using iterator = btree_iterator<node_type, reference, pointer>;
	using const_iterator = typename iterator::const_iterator;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using allocator_type = typename Params::allocator_type;
	using allocator_traits = typename Params::allocator_traits;
	using internal_allocator_type = typename Params::internal_allocator_type;
	using internal_allocator_traits = typename Params::internal_allocator_traits;

public:
	// Default constructor.
	btree(const key_compare& comp, const allocator_type& alloc);

	// Copy constructor.
	btree(const self_type& x);

	// Destructor.
	~btree() {
		clear();
	}

	// Iterator routines.
	iterator begin() {
		return iterator(leftmost(), 0);
	}
	const_iterator begin() const {
		return const_iterator(leftmost(), 0);
	}
	const_iterator cbegin() const {
		return const_iterator(leftmost(), 0);
	}
	iterator end() {
		return iterator(rightmost(), rightmost() ? rightmost()->count() : 0);
	}
	const_iterator end() const {
		return const_iterator(rightmost(), rightmost() ? rightmost()->count() : 0);
	}
	const_iterator cend() const {
		return const_iterator(rightmost(), rightmost() ? rightmost()->count() : 0);
	}
	reverse_iterator rbegin() {
		return reverse_iterator(end());
	}
	const_reverse_iterator rbegin() const {
		return const_reverse_iterator(end());
	}
	const_reverse_iterator crbegin() const {
		return const_reverse_iterator(end());
	}
	reverse_iterator rend() {
		return reverse_iterator(begin());
	}
	const_reverse_iterator rend() const {
		return const_reverse_iterator(begin());
	}
	const_reverse_iterator crend() const {
		return const_reverse_iterator(begin());
	}

	// Finds the first element whose key is not less than key.
	iterator lower_bound(const key_type& key) {
		return internal_end(internal_lower_bound(key, iterator(root(), 0)));
	}
	const_iterator lower_bound(const key_type& key) const {
		return internal_end(internal_lower_bound(key, const_iterator(root(), 0)));
	}
	template<class _Other, class _Comp = key_compare, class = typename _Comp::is_transparent>
	iterator lower_bound(const _Other& key) {
		return internal_end(internal_lower_bound(key, iterator(root(), 0)));
	}
	template<class _Other, class _Comp = key_compare, class = typename _Comp::is_transparent>
	const_iterator lower_bound(const _Other& key) const {
		return internal_end(internal_lower_bound(key, const_iterator(root(), 0)));
	}

	// Finds the first element whose key is greater than key.
	iterator upper_bound(const key_type& key) {
		return internal_end(internal_upper_bound(key, iterator(root(), 0))); }
	const_iterator upper_bound(const key_type& key) const {
		return internal_end(internal_upper_bound(key, const_iterator(root(), 0))); }
	template<class _Other, class _Comp = key_compare, class = typename _Comp::is_transparent>
	iterator upper_bound(const _Other& key) {
		return internal_end(internal_upper_bound(key, iterator(root(), 0))); }
	template<class _Other, class _Comp = key_compare, class = typename _Comp::is_transparent>
	const_iterator upper_bound(const _Other& key) const {
		return internal_end(internal_upper_bound(key, const_iterator(root(), 0))); }

	// Finds the range of values which compare equal to key. The first member of
	// the returned pair is equal to lower_bound(key). The second member pair of
	// the pair is equal to upper_bound(key).
	std::pair<iterator, iterator> equal_range(const key_type& key) {
		return std::make_pair(lower_bound(key), upper_bound(key));
	}
	std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const {
		return std::make_pair(lower_bound(key), upper_bound(key));
	}
	template<class _Other, class _Comp = key_compare, class = typename _Comp::is_transparent>
	std::pair<iterator, iterator> equal_range(const _Other& key) {
		return std::make_pair(lower_bound(key), upper_bound(key));
	}
	template<class _Other, class _Comp = key_compare, class = typename _Comp::is_transparent>
	std::pair<const_iterator, const_iterator> equal_range(const _Other& key) const {
		return std::make_pair(lower_bound(key), upper_bound(key));
	}

	// Inserts a value into the btree only if it does not already exist. The
	// boolean return value indicates whether insertion succeeded or failed.
	template <typename... Args>
	std::pair<iterator, bool> emplace_unique_key_args(const key_type& key, Args&&... args);

	template <typename P>
	std::pair<iterator, bool> emplace_unique(P&& x) {
		return emplace_unique_extract_key(std::forward<P>(x), btree_can_extract_key<P, key_type>());
	}

	template <typename First, typename Second>
	typename std::enable_if<
		btree_can_extract_map_key<First, key_type, value_type>::value,
		std::pair<iterator, bool>
	>::type emplace_unique(First&& f, Second&& s) {
		return emplace_unique_key_args(f,
			std::forward<First>(f),
			std::forward<Second>(s));
	}

	// Inserts a value into the btree only if it does not already exist. The
	// boolean return value indicates whether insertion succeeded or failed.
	template <typename... Args>
	std::pair<iterator, bool> emplace_unique(Args&&... args) {
		value_type v(std::forward<Args>(args)...);
		return emplace_unique_key_args(params_type::get_key(v), std::move(v));
	}

	template <class P>
	std::pair<iterator, bool>
	emplace_unique_extract_key(P&& x, btree_extract_key_fail_tag) {
		value_type v(std::forward<P>(x));
		return emplace_unique_key_args(params_type::get_key(v), std::move(v));
	}

	template <class P>
	std::pair<iterator, bool>
	emplace_unique_extract_key(P&& x, btree_extract_key_self_tag) {
		return emplace_unique_key_args(x, std::forward<P>(x));
	}

	template <class P>
	std::pair<iterator, bool>
	emplace_unique_extract_key(P&& x, btree_extract_key_first_tag) {
		return emplace_unique_key_args(x.first, std::forward<P>(x));
	}

	// Insert with hint. Check to see if the value should be placed immediately
	// before position in the tree. If it does, then the insertion will take
	// amortized constant time. If not, the insertion will take amortized
	// logarithmic time as if a call to emplace_unique(v) were made.
	template <typename... Args>
	iterator emplace_hint_unique_key_args(const_iterator hint, const key_type& key, Args&&... args);

	template <typename P>
	std::pair<iterator, bool> emplace_hint_unique(const_iterator hint, P&& x) {
		return emplace_hint_unique_extract_key(hint, std::forward<P>(x), btree_can_extract_key<P, key_type>());
	}

	template <typename First, typename Second>
	typename std::enable_if<
		btree_can_extract_map_key<First, key_type, value_type>::value,
		iterator
	>::type emplace_hint_unique(const_iterator hint, First&& f, Second&& s) {
		return emplace_hint_unique_key_args(hint, f,
			std::forward<First>(f),
			std::forward<Second>(s));
	}

	// Inserts a value into the btree only if it does not already exist. The
	// boolean return value indicates whether insertion succeeded or failed.
	template <typename... Args>
	iterator emplace_hint_unique(const_iterator hint, Args&&... args) {
		value_type v(std::forward<Args>(args)...);
		return emplace_hint_unique_key_args(hint, params_type::get_key(v), std::move(v));
	}

	template <class P>
	iterator
	emplace_hint_unique_extract_key(const_iterator hint, P&& x, btree_extract_key_fail_tag) {
		value_type v(std::forward<P>(x));
		return emplace_hint_unique_key_args(hint, params_type::get_key(v), std::move(v));
	}

	template <class P>
	iterator
	emplace_hint_unique_extract_key(const_iterator hint, P&& x, btree_extract_key_self_tag) {
		return emplace_hint_unique_key_args(hint, x, std::forward<P>(x));
	}

	template <class P>
	iterator
	emplace_hint_unique_extract_key(const_iterator hint, P&& x, btree_extract_key_first_tag) {
		return emplace_hint_unique_key_args(hint, x.first, std::forward<P>(x));
	}

	std::pair<iterator, bool> insert_unique(const value_type& v) {
		return emplace_unique_key_args(params_type::get_key(v), v);
	}
	std::pair<iterator, bool> insert_unique(value_type&& v) {
		return emplace_unique_key_args(params_type::get_key(v), std::move(v));
	}
	iterator insert_unique(const_iterator hint, const value_type& v) {
		return emplace_hint_unique_key_args(hint, params_type::get_key(v), v);
	}
	iterator insert_unique(const_iterator hint, value_type&& v) {
		return emplace_hint_unique_key_args(hint, params_type::get_key(v), std::move(v));
	}
	template <typename P, typename = typename std::enable_if<
		!std::is_same<
			typename std::remove_const<typename std::remove_reference<P>::type>::type,
			value_type
		>::value>::type>
	std::pair<iterator, bool> insert_unique(P&& x) {
		return emplace_unique(std::forward<P>(x));
	}
	template <typename P, typename = typename std::enable_if<
		!std::is_same<
			typename std::remove_const<typename std::remove_reference<P>::type>::type,
			value_type
		>::value>::type>
	iterator insert_unique(const_iterator hint, P&& x) {
		return emplace_hint_unique(hint, std::forward<P>(x));
	}

	// Inserts a value into the btree.
	template <typename... Args>
	iterator emplace_multi_key_args(const key_type& key, Args&&... args);

	// Inserts a value into the btree.
	template <typename... Args>
	iterator emplace_multi(Args&&... args) {
		value_type v(std::forward<Args>(args)...);
		return emplace_multi_key_args(params_type::get_key(v), std::move(v));
	}

	// Insert with hint. Check to see if the value should be placed immediately
	// before hint position in the tree. If it does, then the insertion will take
	// amortized constant time. If not, the insertion will take amortized
	// logarithmic time as if a call to emplace_multi(v) were made.
	template <typename... Args>
	iterator emplace_hint_multi_key_args(const_iterator hint, const key_type& key, Args&&... args);

	// Inserts a value into the btree.
	template <typename... Args>
	iterator emplace_hint_multi(const_iterator hint, Args&&... args) {
		value_type v(std::forward<Args>(args)...);
		return emplace_hint_multi_key_args(hint, params_type::get_key(v), std::move(v));
	}

	iterator insert_multi(const value_type& v) {
		return emplace_multi_key_args(params_type::get_key(v), v);
	}
	iterator insert_multi(value_type&& v) {
		return emplace_multi_key_args(params_type::get_key(v), std::move(v));
	}
	iterator insert_multi(const_iterator hint, const value_type& v) {
		return emplace_hint_multi_key_args(hint, params_type::get_key(v), v);
	}
	iterator insert_multi(const_iterator hint, value_type&& v) {
		return emplace_hint_multi_key_args(hint, params_type::get_key(v), std::move(v));
	}
	template <typename P, typename = typename std::enable_if<
		!std::is_same<
			typename std::remove_const<typename std::remove_reference<P>::type>::type,
			value_type
		>::value>::type>
	iterator insert_multi(P&& x) {
		return emplace_multi(std::forward<P>(x));
	}
	template <typename P, typename = typename std::enable_if<
		!std::is_same<
			typename std::remove_const<typename std::remove_reference<P>::type>::type,
			value_type
		>::value>::type>
	iterator insert_multi(const_iterator hint, P&& x) {
		return emplace_hint_multi(hint, std::forward<P>(x));
	}

	void assign(const self_type& x);

	// Erase the specified iterator from the btree. The iterator must be valid
	// (i.e. not equal to end()).  Return an iterator pointing to the node after
	// the one that was erased (or end() if none exists).
	iterator erase(const_iterator iter);

	// Erases range. Returns the number of keys erased.
	int erase(const_iterator begin, const_iterator end);

	// Erases the specified key from the btree. Returns 1 if an element was
	// erased and 0 otherwise.
	int erase_unique(const key_type& key);

	// Erases all of the entries matching the specified key from the
	// btree. Returns the number of elements erased.
	int erase_multi(const key_type& key);

	// Finds the iterator corresponding to a key or returns end() if the key is
	// not present.
	iterator find_unique(const key_type& key) {
		return internal_end(internal_find_unique(key, iterator(root(), 0)));
	}
	const_iterator find_unique(const key_type& key) const {
		return internal_end(internal_find_unique(key, const_iterator(root(), 0)));
	}
	template<class _Other, class _Comp = key_compare, class = typename _Comp::is_transparent>
	iterator find_unique(const _Other& key) {
		return internal_end(internal_find_unique(key, iterator(root(), 0)));
		}
	template<class _Other, class _Comp = key_compare, class = typename _Comp::is_transparent>
	const_iterator find_unique(const _Other& key) const {
		return internal_end(internal_find_unique(key, const_iterator(root(), 0)));
	}

	iterator find_multi(const key_type& key) {
		return internal_end(internal_find_multi(key, iterator(root(), 0)));
	}
	const_iterator find_multi(const key_type& key) const {
		return internal_end(internal_find_multi(key, const_iterator(root(), 0)));
	}
	template<class _Other, class _Comp = key_compare, class = typename _Comp::is_transparent>
	iterator find_multi(const _Other& key) {
		return internal_end(internal_find_multi(key, iterator(root(), 0)));
	}
	template<class _Other, class _Comp = key_compare, class = typename _Comp::is_transparent>
	const_iterator find_multi(const _Other& key) const {
		return internal_end(internal_find_multi(key, const_iterator(root(), 0)));
	}
	// Returns a count of the number of times the key appears in the btree.
	size_type count_unique(const key_type& key) const {
		const_iterator begin = internal_find_unique(key, const_iterator(root(), 0));
		if (!begin.node) {
			// The key doesn't exist in the tree.
			return 0;
		}
		return 1;
	}
	// Returns a count of the number of times the key appears in the btree.
	size_type count_multi(const key_type& key) const {
		return std::distance(lower_bound(key), upper_bound(key));
	}

	template<class _Other, class _Comp = key_compare, class = typename _Comp::is_transparent>
	size_type count_multi(const _Other& key) const {
		return std::distance(lower_bound(key), upper_bound(key));
	}

	// Clear the btree, deleting all of the values it contains.
	void clear();

	// Swap the contents of* this and x.
	void swap(self_type& x);

	// Assign the contents of x to* this.
	self_type& operator=(const self_type& x) {
		if (&x == this) {
			// Don't copy onto ourselves.
			return *this;
		}
		assign(x);
		return *this;
	}

	key_compare& __key_comp() {
		return *this;
	}
	const key_compare& key_comp() const {
		return *this;
	}
	template<class _Other1, class _Other2, class _Comp = key_compare, class = typename _Comp::is_transparent>
	bool compare_keys(const _Other1& x, const _Other2& y) const {
		return key_comp()(x, y);
	}

	bool compare_keys(const key_type& x, const key_type& y) const {
		return key_comp()(x, y);
	}

	// Verifies the structure of the btree.
	void verify() const;

	// Size routines. Note that empty() is slightly faster than doing size()==0.
	size_type size() const {
		if (empty()) return 0;
		if (root()->is_leaf()) return root()->count();
		return root()->size();
	}

	size_type max_size() const {
		return std::numeric_limits<size_type>::max();
	}

	bool empty() const {
		return root() == nullptr;
	}

	// The height of the btree. An empty tree will have height 0.
	size_type height() const {
		size_type h = 0;
		if (root()) {
			// Count the length of the chain from the leftmost node up to the
			// root. We actually count from the root back around to the level below
			// the root, but the calculation is the same because of the circularity
			// of that traversal.
			const node_type* n = root();
			do {
				++h;
				n = n->get_parent();
			} while (n != root());
		}
		return h;
	}

	// The number of internal, leaf and total nodes used by the btree.
	size_type leaf_nodes() const {
		return internal_stats(root()).leaf_nodes;
	}
	size_type internal_nodes() const {
		return internal_stats(root()).internal_nodes;
	}
	size_type nodes() const {
		node_stats stats = internal_stats(root());
		return stats.leaf_nodes + stats.internal_nodes;
	}

	// The total number of bytes used by the btree.
	size_type bytes_used() const {
		node_stats stats = internal_stats(root());
		if (stats.leaf_nodes == 1 && stats.internal_nodes == 0) {
			return sizeof(*this) +
					sizeof(base_fields) + root()->max_count() * sizeof(value_type);
		} else {
			return sizeof(*this) +
					sizeof(root_fields) - sizeof(internal_fields) +
					stats.leaf_nodes * sizeof(leaf_fields) +
					stats.internal_nodes * sizeof(internal_fields);
		}
	}

	// The average number of bytes used per value stored in the btree.
	static double average_bytes_per_value() {
		// Returns the number of bytes per value on a leaf node that is 75%
		// full. Experimentally, this matches up nicely with the computed number of
		// bytes per value in trees that had their values inserted in random order.
		return sizeof(leaf_fields) / (kNodeValues * 0.75);
	}

	// The fullness of the btree. Computed as the number of elements in the btree
	// divided by the maximum number of elements a tree with the current number
	// of nodes could hold. A value of 1 indicates perfect space
	// utilization. Smaller values indicate space wastage.
	double fullness() const {
		return double(size()) / (nodes() * kNodeValues);
	}
	// The overhead of the btree structure in bytes per node. Computed as the
	// total number of bytes used by the btree minus the number of bytes used for
	// storing elements divided by the number of elements.
	double overhead() const {
		if (empty()) {
			return 0.0;
		}
		return (bytes_used() - size() * kValueSize) / double(size());
	}

private:
	// Internal accessor routines.
	node_type* root() {
		return root_.data;
	}
	const node_type* root() const {
		return root_.data;
	}
	node_type*& __root() {
		return root_.data;
	}

	// The rightmost node is stored in the root node.
	node_type* rightmost() {
		return (!root() || root()->is_leaf()) ? root() : root()->rightmost();
	}
	const node_type* rightmost() const {
		return (!root() || root()->is_leaf()) ? root() : root()->rightmost();
	}
	node_type*& __rightmost() {
		return root()->__rightmost();
	}

	// The leftmost node is stored as the parent of the root node.
	node_type* leftmost() {
		return root() ? root()->get_parent() : nullptr;
	}
	const node_type* leftmost() const {
		return root() ? root()->get_parent() : nullptr;
	}

	// The size of the tree is stored in the root node.
	size_type& __size() {
		return root()->__size();
	}

	// Allocator routines.
	internal_allocator_type& __internal_allocator() noexcept {
		return *static_cast<internal_allocator_type*>(&root_);
	}
	const internal_allocator_type& internal_allocator() const noexcept {
		return *static_cast<const internal_allocator_type*>(&root_);
	}

	allocator_type allocator() const noexcept {
		return allocator_type(internal_allocator());
	}

	// Node creation/deletion routines.
	node_type* new_internal_node(node_type* parent) {
		internal_allocator_type& ia = __internal_allocator();
		internal_fields* p = reinterpret_cast<internal_fields*>(internal_allocator_traits::allocate(ia, sizeof(internal_fields)));
		return node_type::init_internal(p, parent);
	}
	node_type* new_internal_root_node() {
		internal_allocator_type& ia = __internal_allocator();
		root_fields* p = reinterpret_cast<root_fields*>(internal_allocator_traits::allocate(ia, sizeof(root_fields)));
		return node_type::init_root(p, root()->get_parent());
	}
	node_type* new_leaf_node(node_type* parent) {
		internal_allocator_type& ia = __internal_allocator();
		leaf_fields* p = reinterpret_cast<leaf_fields*>(internal_allocator_traits::allocate(ia, sizeof(leaf_fields)));
		return node_type::init_leaf(p, parent, kNodeValues);
	}
	node_type* new_leaf_root_node(int max_count) {
		internal_allocator_type& ia = __internal_allocator();
		leaf_fields* p = reinterpret_cast<leaf_fields*>(internal_allocator_traits::allocate(ia, sizeof(base_fields) + max_count * sizeof(value_type)));
		return node_type::init_leaf(p, reinterpret_cast<node_type*>(p), max_count);
	}
	void delete_internal_node(node_type* node) {
		node->destroy();
		assert(node != root());
		internal_allocator_type& ia = __internal_allocator();
		internal_allocator_traits::deallocate(ia, reinterpret_cast<char*>(node), sizeof(internal_fields));
	}
	void delete_internal_root_node() {
		root()->destroy();
		internal_allocator_type& ia = __internal_allocator();
		internal_allocator_traits::deallocate(ia, reinterpret_cast<char*>(root()), sizeof(root_fields));
	}
	void delete_leaf_node(node_type* node) {
		node->destroy();
		internal_allocator_type& ia = __internal_allocator();
		internal_allocator_traits::deallocate(ia, reinterpret_cast<char*>(node), sizeof(base_fields) + node->max_count() * sizeof(value_type));
	}

	// Rebalances or splits the node iter points to.
	void rebalance_or_split(iterator& iter);

	// Merges the values of left, right and the delimiting key on their parent
	// onto left, removing the delimiting key and deleting right.
	void merge_nodes(node_type* left, node_type* right);

	// Tries to merge node with its left or right sibling, and failing that,
	// rebalance with its left or right sibling. Returns true if a merge
	// occurred, at which point it is no longer valid to access node. Returns
	// false if no merging took place.
	bool try_merge_or_rebalance(const_iterator& iter);

	// Tries to shrink the height of the tree by 1.
	void try_shrink();

	iterator internal_end(const_iterator iter) {
		return iter.node ? iterator(iter.node, iter.position) : end();
	}
	const_iterator internal_end(const_iterator iter) const {
		return iter.node ? iter : end();
	}

	// Inserts a value into the btree immediately before iter. Requires that
	// key(v) <= iter.key() and (--iter).key() <= key(v).
	template <typename... Args>
	iterator internal_emplace(const_iterator hint, Args&&... args);

	// Returns an iterator pointing to the first value >= the value "iter" is
	// pointing at. Note that "iter" might be pointing to an invalid location as
	// iter.position == iter.node->count(). This routine simply moves iter up in
	// the tree to a valid location.
	template <typename IterType>
	static IterType internal_last(IterType iter);

	// Returns an iterator pointing to the leaf position at which key would
	// reside in the tree.
	template <typename IterType, typename K=key_type>
	std::pair<IterType, int> internal_locate(const K& key, IterType iter) const;
	template <typename IterType, typename K=key_type>
	std::pair<IterType, int> internal_locate_compare(const K& key, IterType iter) const;

	// Internal routine which implements lower_bound().
	template <typename IterType, typename K=key_type>
	IterType internal_lower_bound(const K& key, IterType iter) const;

	// Internal routine which implements upper_bound().
	template <typename IterType, typename K=key_type>
	IterType internal_upper_bound(const K& key, IterType iter) const;

	// Internal routine which implements find_unique().
	template <typename IterType, typename K = key_type>
	IterType internal_find_unique(const K& key, IterType iter) const;

	// Internal routine which implements find_multi().
	template <typename IterType>
	IterType internal_find_multi(const key_type& key, IterType iter) const;

	// Deletes a node and all of its children.
	void internal_clear(node_type* node);

	// Verifies the tree structure of node.
	int internal_verify(const node_type* node, const key_type* lo, const key_type* hi) const;

	node_stats internal_stats(const node_type* node) const {
		if (!node) {
			return node_stats(0, 0);
		}
		if (node->is_leaf()) {
			return node_stats(1, 0);
		}
		node_stats res(0, 1);
		for (int i = 0; i <= node->count(); ++i) {
			res += internal_stats(node->child(i));
		}
		return res;
	}

private:
	empty_base_handle<internal_allocator_type, node_type*> root_;

private:
	// A never instantiated helper function that returns big_ if R is bool and small_ otherwise.
	template <typename R>
	static std::conditional_t<
		std::is_same<R, bool>::value,
		big_,
		small_> key_compare_checker(R);

	// A never instantiated helper function that returns the key comparison
	// functor.
	static key_compare key_compare_helper();

	// Note: We insist on kTargetValues, which is computed from
	// Params::kTargetNodeSize, must fit the base_fields::field_type.
	COMPILE_ASSERT((kNodeValues < (1 << (8 * sizeof(typename base_fields::field_type)))),
		target_node_size_too_large);

	// Test the assumption made in setting kNodeValueSpace.
	COMPILE_ASSERT(sizeof(base_fields) >= 2 * sizeof(void*),
		node_space_assumption_incorrect);
};

////
// btree_node methods
template <typename P> template <typename... Args>
inline void btree_node<P>::insert_value(int i, Args&&... args) {
	auto cnt = count();

	assert(i <= cnt);

	// Initialize new value at the end.
	construct_value(cnt, std::forward<Args>(args)...);

	// Move initialized value to the correct position.
	if (cnt > i) {
		if (is_leaf()) {
			for (int j = cnt; j > i; --j) {
				swap_value(j - 1, j);
			}
		} else {
			for (int j = cnt; j > i; --j) {
				swap_value(j - 1, j);
				move_child(j, j + 1);
			}
		}
	}

	// Increase number of items
	set_count(cnt + 1);
}

template <typename P>
inline void btree_node<P>::remove_value(int i) {
	auto cnt = count();

	// Move the value to the end.
	if (is_leaf()) {
		for (int j = i + 1; j < cnt; ++j) {
			swap_value(j, j - 1);
		}
	} else {
		assert(child(i + 1)->count() == 0);
		for (int j = i + 1; j < cnt; ++j) {
			swap_value(j, j - 1);
			move_child(j + 1, j);
		}
		reset_child(cnt);
	}

	// Decrease number of items.
	set_count(--cnt);

	// Finally, destroy value.
	destroy_value(cnt);
}

template <typename P>
void btree_node<P>::rebalance_right_to_left(btree_node* src, int to_move) {
	auto cnt = count();
	auto src_cnt = src->count();

	assert(get_parent() == src->get_parent());
	assert(position() + 1 == src->position());
	assert(src_cnt >= cnt);
	assert(to_move >= 1);
	assert(to_move <= src_cnt);

	// Move the delimiting value to the left node.
	get_parent()->move_value(position(), this, cnt);

	// Move the new delimiting value from the right node.
	src->move_value(to_move - 1, get_parent(), position());

	if (is_leaf()) {
		// Move the values from the right to the left node.
		for (int i = 1; i < to_move; ++i) {
			src->move_value(i - 1, this, cnt + i);
		}
		// Shift the values in the right node to their correct position.
		for (int i = to_move; i < src_cnt; ++i) {
			src->move_value(i, i - to_move);
		}
	} else {
		// Move the values and child pointsts from the right to the left node.
		src->move_child(0, this, 1 + cnt);
		for (int i = 1; i < to_move; ++i) {
			src->move_value(i - 1, this, cnt + i);
			src->move_child(i, this, 1 + cnt + i);
		}
		// Shift the values and child pointsts in the right node to their correct position.
		for (int i = to_move; i < src_cnt; ++i) {
			src->move_value(i, i - to_move);
			src->move_child(i, i - to_move);
		}
		src->move_child(src_cnt, src_cnt - to_move);
	}

	// Fixup the counts on the src and dst nodes.
	set_count(cnt + to_move);
	src->set_count(src_cnt - to_move);
}

template <typename P>
void btree_node<P>::rebalance_left_to_right(btree_node* dst, int to_move) {
	auto cnt = count();
	auto dst_cnt = dst->count();

	assert(get_parent() == dst->get_parent());
	assert(position() + 1 == dst->position());
	assert(cnt >= dst_cnt);
	assert(to_move >= 1);
	assert(to_move <= cnt);

	// Make room in the right node for the new values.
	for (int i = dst_cnt - 1; i >= 0; --i) {
		dst->move_value(i, i + to_move);
	}

	// Move the delimiting value to the right node.
	get_parent()->move_value(position(), dst, to_move - 1);

	// Move the new delimiting value from the left node.
	this->move_value(cnt - to_move, get_parent(), position());

	if (is_leaf()) {
		// Move the values from the left to the right node.
		for (int i = 1; i < to_move; ++i) {
			move_value(cnt - to_move + i, dst, i - 1);
		}
	} else {
		// Move the values and child pointers from the left to the right node.
		for (int i = dst_cnt; i >= 0; --i) {
			dst->move_child(i, i + to_move);
		}
		// Move the values and child pointers from the left to the right node.
		for (int i = 1; i < to_move; ++i) {
			move_value(cnt - to_move + i, dst, i - 1);
			move_child(cnt - to_move + i, dst, i - 1);
		}
		move_child(cnt, dst, to_move - 1);
	}

	// Fixup the counts on the src and dst nodes.
	set_count(cnt - to_move);
	dst->set_count(dst_cnt + to_move);
}

template <typename P>
void btree_node<P>::split(btree_node* dst, int insert_position) {
	auto cnt = count();
	auto dst_cnt = dst->count();

	assert(dst_cnt == 0);

	// We bias the split based on the position being inserted. If we're
	// inserting at the beginning of the left node then bias the split to put
	// more values on the right node. If we're inserting at the end of the
	// right node then bias the split to put more values on the left node.
	if (insert_position == 0) {
		dst_cnt = cnt - 1;
		dst->set_count(dst_cnt);
	} else if (insert_position != max_count()) {
		dst_cnt = cnt / 2;
		dst->set_count(dst_cnt);
	}
	assert(cnt > dst_cnt);
	cnt -= dst_cnt;
	set_count(cnt);

	if (is_leaf()) {
		// Move values from the left sibling to the right sibling.
		for (int i = 0; i < dst_cnt; ++i) {
			move_value(cnt + i, dst, i);
		}
	} else {
		// Move values and child pointers from the left sibling to the right sibling.
		for (int i = 0; i < dst_cnt; ++i) {
			move_value(cnt + i, dst, i);
			move_child(cnt + i, dst, i);
		}
		move_child(cnt + dst_cnt, dst, dst_cnt);
	}

	// The split key is the largest value in the left sibling.
	set_count(--cnt);
	get_parent()->insert_value(position(), std::move(value(cnt)));
	destroy_value(cnt);
	get_parent()->set_child(position() + 1, dst);
}

template <typename P>
void btree_node<P>::merge(btree_node* src) {
	auto cnt = count();
	auto src_cnt = src->count();
	auto parent = get_parent();
	auto parent_cnt = parent->count();

	assert(parent == src->get_parent());
	assert(position() + 1 == src->position());

	// Move the delimiting value to the left node.
	parent->move_value(position(), this, cnt);

	// Shift the values in the parent node to their correct position.
	for (int i = position() + 1; i < parent_cnt; ++i) {
		parent->move_value(i, i - 1);
		parent->move_child(i + 1, i);
	}
	parent->reset_child(parent_cnt);

	if (is_leaf()) {
		// Move the values from the right to the left node.
		for (int i = 0; i < src_cnt; ++i) {
			src->move_value(i, this, 1 + cnt + i);
		}
	} else {
		// Move the values and child pointers from the right to the left node.
		for (int i = 0; i < src_cnt; ++i) {
			src->move_value(i, this, 1 + cnt + i);
			src->move_child(i, this, 1 + cnt + i);
		}
		src->move_child(src_cnt, this, 1 + cnt + src_cnt);
	}

	// Fixup the counts on the parent, src and dst nodes.
	parent->set_count(parent_cnt - 1);
	set_count(1 + cnt + src_cnt);
	src->set_count(0);
}

template <typename P>
void btree_node<P>::swap(btree_node* x) {
	auto cnt = count();
	auto x_cnt = x->count();

	int min = std::min(cnt, x_cnt);

	assert(is_leaf() == x->is_leaf());

	if (is_leaf()) {
		// Swap the values.
		for (int i = 0; i < min; ++i) {
			swap_value(i, x);
		}
		for (int i = min; i < x_cnt; ++i) {
			x->move_value(i, this);
		}
		for (int i = min; i < cnt; ++i) {
			move_value(i, x);
		}
	} else {
		// Swap the values and child pointers.
		for (int i = 0; i < min; ++i) {
			swap_value(i, x);
			swap_child(i, x);
		}
		swap_child(min, x);
		for (int i = min; i < x_cnt; ++i) {
			x->move_value(i, this);
			x->move_child(i + 1, this);
		}
		for (int i = min; i < cnt; ++i) {
			move_value(i, x);
			move_child(i + 1, x);
		}
	}

	// Swap the counts.
	BentleyApi::btree_swap_helper(fields_.count, x->fields_.count);
}

////
// btree_iterator methods
template <typename N, typename R, typename P>
void btree_iterator<N, R, P>::increment_slow() {
	if (node->is_leaf()) {
		assert(position >= node->count());
		self_type save(*this);
		while (position == node->count() && !node->is_root()) {
			assert(node->get_parent()->child(node->position()) == node);
			position = node->position();
			node = node->get_parent();
		}
		if (position == node->count()) {
			*this = save;
		}
	} else {
		assert(position < node->count());
		node = node->child(position + 1);
		while (!node->is_leaf()) {
			node = node->child(0);
		}
		position = 0;
	}
}

template <typename N, typename R, typename P>
void btree_iterator<N, R, P>::increment_by(int count) {
	while (count > 0) {
		if (node->is_leaf()) {
			int rest = node->count() - position;
			position += std::min(rest, count);
			count = count - rest;
			if (position < node->count()) {
				return;
			}
		} else {
			--count;
		}
		increment_slow();
	}
}

template <typename N, typename R, typename P>
void btree_iterator<N, R, P>::decrement_slow() {
	if (node->is_leaf()) {
		assert(position <= -1);
		self_type save(*this);
		while (position < 0 && !node->is_root()) {
			assert(node->get_parent()->child(node->position()) == node);
			position = node->position() - 1;
			node = node->get_parent();
		}
		if (position < 0) {
			*this = save;
		}
	} else {
		assert(position >= 0);
		node = node->child(position);
		while (!node->is_leaf()) {
			node = node->child(node->count());
		}
		position = node->count() - 1;
	}
}

////
// btree methods
template <typename P>
btree<P>::btree(const key_compare& comp, const allocator_type& alloc)
	: key_compare(comp),
	  root_(alloc, nullptr) {
}

template <typename P>
btree<P>::btree(const self_type& x)
	: key_compare(x.key_comp()),
	  root_(x.internal_allocator(), nullptr) {
	assign(x);
}

template <typename P> template <typename... Args>
std::pair<typename btree<P>::iterator, bool>
btree<P>::emplace_unique_key_args(const key_type& key, Args&&... args) {
	if (empty()) {
		__root() = new_leaf_root_node(1);
	}

	std::pair<iterator, int> res = internal_locate(key, iterator(root(), 0));
	iterator& iter = res.first;
	if (res.second == kExactMatch) {
		// The key already exists in the tree, do nothing.
		return std::make_pair(internal_last(iter), false);
	} else if (!res.second) {
		iterator last = internal_last(iter);
		if (last.node && !compare_keys(key, last.key())) {
			// The key already exists in the tree, do nothing.
			return std::make_pair(last, false);
		}
	}

	return std::make_pair(internal_emplace(iter, std::forward<Args>(args)...), true);
}

template <typename P> template <typename... Args>
inline typename btree<P>::iterator
btree<P>::emplace_hint_unique_key_args(const_iterator hint, const key_type& key, Args&&... args) {
	if (!empty()) {
		if (hint == end() || compare_keys(key, hint.key())) {
			const_iterator prev = hint;
			if (hint == begin() || compare_keys((--prev).key(), key)) {
				// prev.key() < key < hint.key()
				return internal_emplace(hint, std::forward<Args>(args)...);
			}
		} else if (compare_keys(hint.key(), key)) {
			const_iterator next = hint;
			++next;
			if (next == end() || compare_keys(key, next.key())) {
				// hint.key() < key < next.key()
				return internal_emplace(next, std::forward<Args>(args)...);
			}
		} else {
			// hint.key() == key
			return iterator(hint.node, hint.position);
		}
	}
	return emplace_unique(std::forward<Args>(args)...).first;
}

template <typename P> template <typename... Args>
typename btree<P>::iterator
btree<P>::emplace_multi_key_args(const key_type& key, Args&&... args) {
	if (empty()) {
		__root() = new_leaf_root_node(1);
	}

	iterator iter = internal_upper_bound(key, iterator(root(), 0));
	if (!iter.node) {
		iter = end();
	}
	return internal_emplace(iter, std::forward<Args>(args)...);
}

template <typename P> template <typename... Args>
typename btree<P>::iterator
btree<P>::emplace_hint_multi_key_args(const_iterator hint, const key_type& key, Args&&... args) {
	if (!empty()) {
		if (hint == end() || !compare_keys(hint.key(), key)) {
			const_iterator prev = hint;
			if (hint == begin() || !compare_keys(key, (--prev).key())) {
				// prev.key() <= key <= hint.key()
				return internal_emplace(hint, std::forward<Args>(args)...);
			}
		} else {
			const_iterator next = hint;
			++next;
			if (next == end() || !compare_keys(next.key(), key)) {
				// hint.key() < key <= next.key()
				return internal_emplace(next, std::forward<Args>(args)...);
			}
		}
	}
	return emplace_multi(std::forward<Args>(args)...);
}

template <typename P>
void btree<P>::assign(const self_type& x) {
	clear();

	__key_comp() = x.key_comp();
	__internal_allocator() = x.internal_allocator();

	// Assignment can avoid key comparisons because we know the order of the
	// values is the same order we'll store them in.
	for (const_iterator iter = x.begin(); iter != x.end(); ++iter) {
		if (empty()) {
			emplace_multi(*iter);
		} else {
			// If the btree is not empty, we can just insert the new value at the end
			// of the tree!
			internal_emplace(end(), *iter);
		}
	}
}

template <typename P>
typename btree<P>::iterator btree<P>::erase(const_iterator iter) {
	bool internal_delete = false;
	if (!iter.node->is_leaf()) {
		// Deletion of a value on an internal node. Swap the key with the largest
		// value of our left child. This is easy, we just decrement iter.
		const_iterator tmp_iter(iter--);
		assert(iter.node->is_leaf());
		assert(!compare_keys(tmp_iter.key(), iter.key()));
		iter.node->swap_value(iter.position, tmp_iter.node, tmp_iter.position);
		internal_delete = true;
		--__size();
	} else if (!root()->is_leaf()) {
		--__size();
	}

	// Delete the key from the leaf.
	iter.node->remove_value(iter.position);

	// We want to return the next value after the one we just erased. If we
	// erased from an internal node (internal_delete == true), then the next
	// value is ++(++iter). If we erased from a leaf node (internal_delete ==
	// false) then the next value is ++iter. Note that ++iter may point to an
	// internal node and the value in the internal node may move to a leaf node
	// (iter.node) when rebalancing is performed at the leaf level.

	// Merge/rebalance as we walk back up the tree.
	iterator res(iter.node, iter.position);
	for (;;) {
		if (iter.node == root()) {
			try_shrink();
			if (empty()) {
				return end();
			}
			break;
		}
		if (iter.node->count() >= kMinNodeValues) {
			break;
		}
		bool merged = try_merge_or_rebalance(iter);
		if (iter.node->is_leaf()) {
			res = iterator(iter.node, iter.position);
		}
		if (!merged) {
			break;
		}
		iter.node = iter.node->get_parent();
	}

	// Adjust our return value. If we're pointing at the end of a node, advance
	// the iterator.
	if (res.position == res.node->count()) {
		res.position = res.node->count() - 1;
		++res;
	}
	// If we erased from an internal node, advance the iterator.
	if (internal_delete) {
		++res;
	}
	return res;
}

template <typename P>
int btree<P>::erase(const_iterator begin, const_iterator end) {
	int count = (int) distance(begin, end);
	for (int i = 0; i < count; i++) {
		begin = erase(begin);
	}
	return count;
}

template <typename P>
int btree<P>::erase_unique(const key_type& key) {
	iterator iter = internal_find_unique(key, iterator(root(), 0));
	if (!iter.node) {
		// The key doesn't exist in the tree, return nothing done.
		return 0;
	}
	erase(iter);
	return 1;
}

template <typename P>
int btree<P>::erase_multi(const key_type& key) {
	iterator begin = internal_lower_bound(key, iterator(root(), 0));
	if (!begin.node) {
		// The key doesn't exist in the tree, return nothing done.
		return 0;
	}
	// Delete all of the keys between begin and upper_bound(key).
	iterator end = internal_end(internal_upper_bound(key, iterator(root(), 0)));
	return erase(begin, end);
}

template <typename P>
void btree<P>::clear() {
	if (root() != nullptr) {
		internal_clear(root());
	}
	__root() = nullptr;
}

template <typename P>
void btree<P>::swap(self_type& x) {
	std::swap(static_cast<key_compare&>(*this), static_cast<key_compare&>(x));
	std::swap(root_, x.root_);
}

template <typename P>
void btree<P>::verify() const {
	if (root() != nullptr) {
		assert(size() == internal_verify(root(), nullptr, nullptr));
		assert(leftmost() == (++const_iterator(root(), -1)).node);
		assert(rightmost() == (--const_iterator(root(), root()->count())).node);
		assert(leftmost()->is_leaf());
		assert(rightmost()->is_leaf());
	} else {
		assert(size() == 0);
		assert(leftmost() == nullptr);
		assert(rightmost() == nullptr);
	}
}

template <typename P>
void btree<P>::rebalance_or_split(iterator& iter) {
	node_type*& node = iter.node;
	int& insert_position = iter.position;
	assert(node->count() == node->max_count());

	// First try to make room on the node by rebalancing.
	node_type* parent = node->get_parent();
	if (node != root()) {
		if (node->position() > 0) {
			// Try rebalancing with our left sibling.
			node_type* left = parent->child(node->position() - 1);
			if (left->count() < left->max_count()) {
				// We bias rebalancing based on the position being inserted. If we're
				// inserting at the end of the right node then we bias rebalancing to
				// fill up the left node.
				int to_move = (left->max_count() - left->count()) /
						(1 + (insert_position < left->max_count()));
				to_move = std::max(1, to_move);

				if (((insert_position - to_move) >= 0) ||
						((left->count() + to_move) < left->max_count())) {
					left->rebalance_right_to_left(node, to_move);

					assert(node->max_count() - node->count() == to_move);
					insert_position = insert_position - to_move;
					if (insert_position < 0) {
						insert_position = insert_position + left->count() + 1;
						node = left;
					}

					assert(node->count() < node->max_count());
					return;
				}
			}
		}

		if (node->position() < parent->count()) {
			// Try rebalancing with our right sibling.
			node_type* right = parent->child(node->position() + 1);
			if (right->count() < right->max_count()) {
				// We bias rebalancing based on the position being inserted. If we're
				// inserting at the beginning of the left node then we bias rebalancing
				// to fill up the right node.
				int to_move = (right->max_count() - right->count()) /
						(1 + (insert_position > 0));
				to_move = std::max(1, to_move);

				if ((insert_position <= (node->count() - to_move)) ||
						((right->count() + to_move) < right->max_count())) {
					node->rebalance_left_to_right(right, to_move);

					if (insert_position > node->count()) {
						insert_position = insert_position - node->count() - 1;
						node = right;
					}

					assert(node->count() < node->max_count());
					return;
				}
			}
		}

		// Rebalancing failed, make sure there is room on the parent node for a new
		// value.
		if (parent->count() == parent->max_count()) {
			iterator parent_iter(node->get_parent(), node->position());
			rebalance_or_split(parent_iter);
		}
	} else {
		// Rebalancing not possible because this is the root node.
		if (root()->is_leaf()) {
			// The root node is currently a leaf node: create a new root node and set
			// the current root node as the child of the new root.
			parent = new_internal_root_node();
			parent->set_child(0, root());
			__root() = parent;
			assert(__rightmost() == parent->child(0));
		} else {
			// The root node is an internal node. We do not want to create a new root
			// node because the root node is special and holds the size of the tree
			// and a pointer to the rightmost node. So we create a new internal node
			// and move all of the items on the current root into the new node.
			parent = new_internal_node(parent);
			parent->set_child(0, parent);
			parent->swap(root());
			node = parent;
		}
	}

	// Split the node.
	node_type* split_node;
	if (node->is_leaf()) {
		split_node = new_leaf_node(parent);
		node->split(split_node, insert_position);
		if (rightmost() == node) {
			__rightmost() = split_node;
		}
	} else {
		split_node = new_internal_node(parent);
		node->split(split_node, insert_position);
	}

	if (insert_position > node->count()) {
		insert_position = insert_position - node->count() - 1;
		node = split_node;
	}
}

template <typename P>
void btree<P>::merge_nodes(node_type* left, node_type* right) {
	left->merge(right);
	if (right->is_leaf()) {
		if (rightmost() == right) {
			__rightmost() = left;
		}
		delete_leaf_node(right);
	} else {
		delete_internal_node(right);
	}
}

template <typename P>
bool btree<P>::try_merge_or_rebalance(const_iterator& iter) {
	node_type* parent = iter.node->get_parent();
	if (iter.node->position() > 0) {
		// Try merging with our left sibling.
		node_type* left = parent->child(iter.node->position() - 1);
		if ((1 + left->count() + iter.node->count()) <= left->max_count()) {
			iter.position += 1 + left->count();
			merge_nodes(left, iter.node);
			iter.node = left;
			return true;
		}
	}
	if (iter.node->position() < parent->count()) {
		// Try merging with our right sibling.
		node_type* right = parent->child(iter.node->position() + 1);
		if ((1 + iter.node->count() + right->count()) <= right->max_count()) {
			merge_nodes(iter.node, right);
			return true;
		}
		// Try rebalancing with our right sibling. We don't perform rebalancing if
		// we deleted the first element from iter.node and the node is not
		// empty. This is a small optimization for the common pattern of deleting
		// from the front of the tree.
		if ((right->count() > kMinNodeValues) && ((iter.node->count() == 0) || (iter.position > 0))) {
			int to_move = (right->count() - iter.node->count()) / 2;
			to_move = std::min(to_move, right->count() - 1);
			iter.node->rebalance_right_to_left(right, to_move);
			return false;
		}
	}
	if (iter.node->position() > 0) {
		// Try rebalancing with our left sibling. We don't perform rebalancing if
		// we deleted the last element from iter.node and the node is not
		// empty. This is a small optimization for the common pattern of deleting
		// from the back of the tree.
		node_type* left = parent->child(iter.node->position() - 1);
		if ((left->count() > kMinNodeValues) && ((iter.node->count() == 0) || (iter.position < iter.node->count()))) {
			int to_move = (left->count() - iter.node->count()) / 2;
			to_move = std::min(to_move, left->count() - 1);
			left->rebalance_left_to_right(iter.node, to_move);
			iter.position += to_move;
			return false;
		}
	}
	return false;
}

template <typename P>
void btree<P>::try_shrink() {
	if (root()->count() > 0) {
		return;
	}
	// Deleted the last item on the root node, shrink the height of the tree.
	if (root()->is_leaf()) {
		assert(size() == 0);
		delete_leaf_node(root());
		__root() = nullptr;
	} else {
		node_type* child = root()->child(0);
		if (child->is_leaf()) {
			// The child is a leaf node so simply make it the root node in the tree.
			child->make_root();
			delete_internal_root_node();
			__root() = child;
		} else {
			// The child is an internal node. We want to keep the existing root node
			// so we move all of the values from the child node into the existing
			// (empty) root node.
			child->swap(root());
			delete_internal_node(child);
		}
	}
}

template <typename P> template <typename IterType>
inline IterType btree<P>::internal_last(IterType iter) {
	while (iter.node && iter.position == iter.node->count()) {
		iter.position = iter.node->position();
		iter.node = iter.node->get_parent();
		if (iter.node->is_leaf()) {
			iter.node = nullptr;
		}
	}
	return iter;
}

template <typename P> template <typename... Args>
inline typename btree<P>::iterator
btree<P>::internal_emplace(const_iterator hint, Args&&... args) {
	iterator iter(hint.node, hint.position);
	if (!iter.node->is_leaf()) {
		// We can't insert on an internal node. Instead, we'll insert after the
		// previous value which is guaranteed to be on a leaf node.
		--iter;
		++iter.position;
	}
	if (iter.node->count() == iter.node->max_count()) {
		// Make room in the leaf for the new item.
		if (iter.node->max_count() < kNodeValues) {
			// Insertion into the root where the root is smaller that the full node
			// size. Simply grow the size of the root node.
			assert(iter.node == root());
			iter.node = new_leaf_root_node(std::min<int>(kNodeValues, 2 * iter.node->max_count()));
			iter.node->swap(root());
			delete_leaf_node(root());
			__root() = iter.node;
		} else {
			rebalance_or_split(iter);
			++__size();
		}
	} else if (!root()->is_leaf()) {
		++__size();
	}
	iter.node->insert_value(iter.position, std::forward<Args>(args)...);
	return iter;
}

template <typename P> template <typename IterType, typename K>
inline std::pair<IterType, int> btree<P>::internal_locate(const K& key, IterType iter) const {
	return internal_locate_type::dispatch(key, *this, iter);
}

template <typename P> template <typename IterType, typename K>
inline std::pair<IterType, int> btree<P>::internal_locate_compare(const K& key, IterType iter) const {
	for (;;) {
		iter.position = iter.node->lower_bound(key, key_comp());
		if (iter.node->is_leaf()) {
			break;
		}
		iter.node = iter.node->child(iter.position);
	}
	return std::make_pair(iter, 0);
}

template <typename P> template <typename IterType, typename K>
IterType btree<P>::internal_lower_bound(const K& key, IterType iter) const {
	if (iter.node) {
		for (;;) {
			iter.position =
					iter.node->lower_bound(key, key_comp())&  kMatchMask;
			if (iter.node->is_leaf()) {
				break;
			}
			iter.node = iter.node->child(iter.position);
		}
		iter = internal_last(iter);
	}
	return iter;
}

template <typename P> template <typename IterType, typename K>
IterType btree<P>::internal_upper_bound(const K& key, IterType iter) const {
	if (iter.node) {
		for (;;) {
			iter.position = iter.node->upper_bound(key, key_comp());
			if (iter.node->is_leaf()) {
				break;
			}
			iter.node = iter.node->child(iter.position);
		}
		iter = internal_last(iter);
	}
	return iter;
}

template <typename P> template <typename IterType, typename K>
IterType btree<P>::internal_find_unique(const K& key, IterType iter) const {
	if (iter.node) {
		std::pair<IterType, int> res = internal_locate(key, iter);
		if (res.second == kExactMatch) {
			return res.first;
		}
		if (!res.second) {
			iter = internal_last(res.first);
			if (iter.node && !compare_keys(key, iter.key())) {
				return iter;
			}
		}
	}
	return IterType(nullptr, 0);
}

template <typename P> template <typename IterType>
IterType btree<P>::internal_find_multi(const key_type& key, IterType iter) const {
	if (iter.node) {
		iter = internal_lower_bound(key, iter);
		if (iter.node) {
			iter = internal_last(iter);
			if (iter.node && !compare_keys(key, iter.key())) {
				return iter;
			}
		}
	}
	return IterType(nullptr, 0);
}

template <typename P>
void btree<P>::internal_clear(node_type* node) {
	if (!node->is_leaf()) {
		for (int i = 0; i <= node->count(); ++i) {
			internal_clear(node->child(i));
		}
		if (node == root()) {
			delete_internal_root_node();
		} else {
			delete_internal_node(node);
		}
	} else {
		delete_leaf_node(node);
	}
}

template <typename P>
int btree<P>::internal_verify(const node_type* node, const key_type* lo, const key_type* hi) const {
	assert(node->count() > 0);
	assert(node->count() <= node->max_count());
	if (lo) {
		assert(!compare_keys(node->key(0), *lo));
	}
	if (hi) {
		assert(!compare_keys(*hi, node->key(node->count() - 1)));
	}
	for (int i = 1; i < node->count(); ++i) {
		assert(!compare_keys(node->key(i), node->key(i - 1)));
	}
	int count = node->count();
	if (!node->is_leaf()) {
		for (int i = 0; i <= node->count(); ++i) {
			assert(node->child(i) != nullptr);
			assert(node->child(i)->get_parent() == node);
			assert(node->child(i)->position() == i);
			count += internal_verify(node->child(i),
				(i == 0) ? lo : &node->key(i - 1),
				(i == node->count()) ? hi : &node->key(i));
		}
		for (int i = node->count() + 1; i <= node->max_count(); ++i) {
			assert(node->child(i) == nullptr);
		}
	}
	return count;
}

// A common base class for btree::set, map, btree_multiset and
// btree_multimap.
template <typename Tree>
class btree_container {
	using self_type = btree_container<Tree>;

public:
	using params_type = typename Tree::params_type;
	using key_type = typename Tree::key_type;
	using value_type = typename Tree::value_type;
	using key_compare = typename Tree::key_compare;
	using allocator_type = typename Tree::allocator_type;
	using pointer = typename Tree::pointer;
	using const_pointer = typename Tree::const_pointer;
	using reference = typename Tree::reference;
	using const_reference = typename Tree::const_reference;
	using size_type = typename Tree::size_type;
	using difference_type = typename Tree::difference_type;
	using iterator = typename Tree::iterator;
	using const_iterator = typename Tree::const_iterator;
	using reverse_iterator = typename Tree::reverse_iterator;
	using const_reverse_iterator = typename Tree::const_reverse_iterator;

public:
	// Default constructor.
	btree_container(const key_compare& comp, const allocator_type& alloc)
		: tree_(comp, alloc) {
	}

	// Copy constructor.
	btree_container(const self_type& x)
		: tree_(x.tree_) {
	}

	// Iterator routines.
	iterator begin() {
		return tree_.begin();
	}
	const_iterator begin() const {
		return tree_.begin();
	}
	const_iterator cbegin() const {
		return tree_.cbegin();
	}
	iterator end() {
		return tree_.end();
	}
	const_iterator end() const {
		return tree_.end();
	}
	const_iterator cend() const {
		return tree_.cend();
	}
	reverse_iterator rbegin() {
		return tree_.rbegin();
	}
	const_reverse_iterator rbegin() const {
		return tree_.rbegin();
	}
	const_reverse_iterator crbegin() const {
		return tree_.crbegin();
	}
	reverse_iterator rend() {
		return tree_.rend();
	}
	const_reverse_iterator rend() const {
		return tree_.rend();
	}
	const_reverse_iterator crend() const {
		return tree_.crend();
	}

	// Lookup routines.
	iterator lower_bound(const key_type& key) {
		return tree_.lower_bound(key);
	}
	const_iterator lower_bound(const key_type& key) const {
		return tree_.lower_bound(key);
	}
	iterator upper_bound(const key_type& key) {
		return tree_.upper_bound(key);
	}
	const_iterator upper_bound(const key_type& key) const {
		return tree_.upper_bound(key);
	}
	std::pair<iterator,iterator> equal_range(const key_type& key) {
		return tree_.equal_range(key);
	}
	std::pair<const_iterator,const_iterator> equal_range(const key_type& key) const {
		return tree_.equal_range(key);
	}

	template<class _Other, class _Comp = key_compare, class = typename _Comp::is_transparent>
	iterator lower_bound(const _Other& key) {
		return tree_.lower_bound(key);
	}
	template<class _Other, class _Comp = key_compare, class = typename _Comp::is_transparent>
	const_iterator lower_bound(const _Other& key) const {
		return tree_.lower_bound(key);
	}
	template<class _Other, class _Comp = key_compare, class = typename _Comp::is_transparent>
	iterator upper_bound(const _Other& key) {
		return tree_.upper_bound(key);
	}
	template<class _Other, class _Comp = key_compare, class = typename _Comp::is_transparent>
	const_iterator upper_bound(const _Other& key) const {
		return tree_.upper_bound(key);
	}
	template<class _Other, class _Comp = key_compare, class = typename _Comp::is_transparent>
	std::pair<iterator,iterator> equal_range(const _Other& key) {
		return tree_.equal_range(key);
	}
	template<class _Other, class _Comp = key_compare, class = typename _Comp::is_transparent>
	std::pair<const_iterator,const_iterator> equal_range(const _Other& key) const {
		return tree_.equal_range(key);
	}

	allocator_type get_allocator() const noexcept {
		return allocator_type(tree_.allocator());
	}

	// Utility routines.
	void clear() {
		tree_.clear();
	}
	void swap(self_type& x) {
		tree_.swap(x.tree_);
	}

	void verify() const {
		tree_.verify();
	}

	// Size routines.
	size_type size() const {
		return tree_.size();
	}
	size_type max_size() const {
		return tree_.max_size();
	}
	bool empty() const {
		return tree_.empty();
	}

	size_type height() const {
		return tree_.height();
	}
	size_type internal_nodes() const {
		return tree_.internal_nodes();
	}
	size_type leaf_nodes() const {
		return tree_.leaf_nodes();
	}
	size_type nodes() const {
		return tree_.nodes();
	}
	size_type bytes_used() const {
		return tree_.bytes_used();
	}
	static double average_bytes_per_value() {
		return Tree::average_bytes_per_value();
	}
	double fullness() const {
		return tree_.fullness();
	}
	double overhead() const {
		return tree_.overhead();
	}

	bool operator==(const self_type& x) const {
		if (size() != x.size()) {
			return false;
		}
		for (const_iterator i = begin(), xi = x.begin(); i != end(); ++i, ++xi) {
			if (*i !=* xi) {
				return false;
			}
		}
		return true;
	}

	bool operator!=(const self_type& other) const {
		return !operator==(other);
	}


 protected:
	Tree tree_;
};

// A common base class for btree::map and btree::set.
template <typename Tree>
class btree_unique_container : public btree_container<Tree> {
	using self_type = btree_unique_container<Tree>;
	using super_type = btree_container<Tree>;

public:
	using key_type = typename Tree::key_type;
	using value_type = typename Tree::value_type;
	using size_type = typename Tree::size_type;
	using key_compare = typename Tree::key_compare;
	using allocator_type = typename Tree::allocator_type;
	using iterator = typename Tree::iterator;
	using const_iterator = typename Tree::const_iterator;

public:
	// Default constructor.
	btree_unique_container(const key_compare& comp = key_compare(),
						   const allocator_type& alloc = allocator_type())
		: super_type(comp, alloc) {
	}

	// Copy constructor.
	btree_unique_container(const self_type& x)
		: super_type(x) {
	}

	// Range constructor.
	template <class InputIterator>
	btree_unique_container(InputIterator b, InputIterator e,
						   const key_compare& comp = key_compare(),
						   const allocator_type& alloc = allocator_type())
		: super_type(comp, alloc) {
		insert(b, e);
	}

	// Lookup routines.
	iterator find(const key_type& key) {
		return this->tree_.find_unique(key);
	}
	const_iterator find(const key_type& key) const {
		return this->tree_.find_unique(key);
	}

	template<class _Other, class _Comp = key_compare, class = typename _Comp::is_transparent>
	iterator find(const _Other& _Keyval) {
		return this->tree_.find_unique(_Keyval);
	}
	template<class _Other, class _Comp = key_compare, class = typename _Comp::is_transparent>
	const_iterator find(const _Other& key) const {
		return this->tree_.find_unique(key);
	}

	size_type count(const key_type& key) const {
		return this->tree_.count_unique(key);
	}

	// Insertion routines.
	std::pair<iterator, bool> insert(const value_type& x) {
		return this->tree_.insert_unique(x);
	}
	std::pair<iterator, bool> insert(value_type&& x) {
		return this->tree_.insert_unique(std::move(x));
	}
	iterator insert(const_iterator hint, const value_type& x) {
		return this->tree_.insert_unique(hint, x);
	}

	iterator insert(const_iterator hint, value_type&& x) {
		return this->tree_.insert_unique(hint, std::move(x));
	}
	void insert(std::initializer_list<value_type> il) {
		insert(il.begin(), il.end());
	}
	template <typename InputIterator>
	void insert(InputIterator f, InputIterator l) {
		for (; f != l; ++f) {
			insert(*f);
		}
	}

	template <typename... Args>
	std::pair<iterator, bool> emplace(Args&&... args) {
		return this->tree_.emplace_unique(std::forward<Args>(args)...);
	}

	template <typename... Args>
	iterator emplace_hint(const_iterator hint, Args&&... args) {
		return this->tree_.emplace_hint_unique(hint, std::forward<Args>(args)...);
	}

	// Deletion routines.
	int erase(const key_type& key) {
		return this->tree_.erase_unique(key);
	}
	// Erase the specified iterator from the btree. The iterator must be valid
	// (i.e. not equal to end()).  Return an iterator pointing to the node after
	// the one that was erased (or end() if none exists).
	iterator erase(const iterator& iter) {
		return this->tree_.erase(iter);
	}
	void erase(const iterator& first, const iterator& last) {
		this->tree_.erase(first, last);
	}
};

// A common base class for btree::multimap and btree::multiset.
template <typename Tree>
class btree_multi_container : public btree_container<Tree> {
	using self_type = btree_multi_container<Tree>;
	using super_type = btree_container<Tree>;

 public:
	using key_type = typename Tree::key_type;
	using value_type = typename Tree::value_type;
	using size_type = typename Tree::size_type;
	using key_compare = typename Tree::key_compare;
	using allocator_type = typename Tree::allocator_type;
	using iterator = typename Tree::iterator;
	using const_iterator = typename Tree::const_iterator;

 public:
	// Default constructor.
	btree_multi_container(const key_compare& comp = key_compare(),
						  const allocator_type& alloc = allocator_type())
		: super_type(comp, alloc) {
	}

	// Copy constructor.
	btree_multi_container(const self_type& x)
		: super_type(x) {
	}

	// Range constructor.
	template <class InputIterator>
	btree_multi_container(InputIterator b, InputIterator e,
						  const key_compare& comp = key_compare(),
						  const allocator_type& alloc = allocator_type())
		: super_type(comp, alloc) {
		insert(b, e);
	}

	// Lookup routines.
	iterator find(const key_type& key) {
		return this->tree_.find_multi(key);
	}
	const_iterator find(const key_type& key) const {
		return this->tree_.find_multi(key);
	}
	template<class _Other, class _Comp = key_compare, class = typename _Comp::is_transparent>
	iterator find(const _Other& key) {
		return this->tree_.find_multi(key);
	}
	template<class _Other, class _Comp = key_compare, class = typename _Comp::is_transparent>
	const_iterator find(const _Other& key) const {
		return this->tree_.find_multi(key);
	}
	size_type count(const key_type& key) const {
		return this->tree_.count_multi(key);
	}
	template<class _Other, class _Comp = key_compare, class = typename _Comp::is_transparent>
	size_type count(const _Other& key) const {
		return this->tree_.count_multi(key);
	}

	// Insertion routines.
	iterator insert(const value_type& x) {
		return this->tree_.insert_multi(x);
	}
	iterator insert(value_type&& x) {
		return this->tree_.insert_multi(std::move(x));
	}
	iterator insert(const_iterator hint, const value_type& x) {
		return this->tree_.insert_multi(hint, x);
	}
	iterator insert(const_iterator hint, value_type&& x) {
		return this->tree_.insert_multi(hint, std::move(x));
	}
	template <typename InputIterator>
	void insert(InputIterator f, InputIterator l) {
		for (; f != l; ++f) {
			insert(*f);
		}
	}
	void insert(std::initializer_list<value_type> il) {
		insert(il.begin(), il.end());
	}

	template <typename... Args>
	std::pair<iterator, bool> emplace(Args&&... args) {
		return this->tree_.emplace_multi(std::forward<Args>(args)...);
	}

	template <typename... Args>
	iterator emplace_hint(const_iterator hint, Args&&... args) {
		return this->tree_.emplace_hint_multi(hint, std::forward<Args>(args)...);
	}

	// Deletion routines.
	int erase(const key_type& key) {
		return this->tree_.erase_multi(key);
	}
	// Erase the specified iterator from the btree. The iterator must be valid
	// (i.e. not equal to end()).  Return an iterator pointing to the node after
	// the one that was erased (or end() if none exists).
	iterator erase(const iterator& iter) {
		return this->tree_.erase(iter);
	}
	void erase(const iterator& first, const iterator& last) {
		this->tree_.erase(first, last);
	}
};

END_BENTLEY_NAMESPACE
