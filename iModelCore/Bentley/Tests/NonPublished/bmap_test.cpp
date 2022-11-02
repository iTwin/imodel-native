/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/bmap.h>
#include <map>

#define __attribute__(IGNOREDSTUFF)
#define VERIFY(X) ASSERT_TRUE(X)

//  Adapted from unit tests that are part of GCC
// Copyright (C) 1999, 2000, 2001, 2002, 2003, 2009
// Free Software Foundation, Inc.

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST (bmap_test, Simple)
    {
    bmap<int, int> m;
    typedef bpair<int,int> T_IntIntPair;
    m[0] = 0;
    m[1] = 1;
    ASSERT_TRUE(!m.empty() );
    ASSERT_TRUE( m.size() == 2 );
    ASSERT_TRUE( m[0] == 0 );
    ASSERT_TRUE( m[1] == 1 );
    int i = 0;
    FOR_EACH(T_IntIntPair const& it , m)
        {
        ASSERT_TRUE( it.first == i );
        ASSERT_TRUE( it.second == i );
        ++i;
        }
    }

/* */
typedef bmap<int,char>  MyMapType;
typedef bpair<int,char> MyMapPairType;

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (bmap_test, InitBMap)
{
    MyMapType map;

    map.insert (MyMapPairType (1, 'a'));
    map.insert (MyMapPairType (2, 'b'));
    map.insert (MyMapPairType (3, 'c'));

    ASSERT_EQ ('a', map[1]);
    ASSERT_EQ ('b', map[2]);
    ASSERT_EQ ('c', map[3]);
#ifdef BMAP_DOES_NOT_HAVE_AT
    ASSERT_EQ ('a', map.at (1));
    ASSERT_EQ ('b', map.at (2));
    ASSERT_EQ ('c', map.at (3));
#endif
    ASSERT_EQ (1, map.count (1));
    ASSERT_EQ (1, map.count (2));
    ASSERT_EQ (1, map.count (3));
    ASSERT_EQ (3, map.size ());
    ASSERT_FALSE (map.empty ());

    map.erase (1);
    map.erase (2);
    map.erase (3);

    ASSERT_NE ('a', map[1]);
    ASSERT_NE ('b', map[2]);
    ASSERT_NE ('c', map[3]);
    ASSERT_EQ (3, map.size ());
    SUCCEED ();

    MyMapType map2;
    map2.insert(map.begin(), map.end());
    ASSERT_EQ (3, map2.size ());
    ASSERT_TRUE ( map == map2);
}

struct NoCopy {
    NoCopy(NoCopy const&) = delete;
    NoCopy& operator=(NoCopy const&) = delete;

    int m_v;
    NoCopy(int a) : m_v(a) {};
    ~NoCopy() = default;
    bool operator== (NoCopy const& __y) const {return this->m_v == __y.m_v; }
    void swap(NoCopy& rhs) {int tmp = m_v; m_v = rhs.m_v; rhs.m_v = tmp;}
    NoCopy(NoCopy&& rhs) : m_v(std::move(rhs.m_v)){rhs.m_v = -1;}
    NoCopy& operator=(NoCopy&& rhs) {NoCopy(std::move(rhs)).swap(*this); return *this;}
};

/*---------------------------------------------------------------------------------**//**
 * make sure btree can hold objects that cannot be copied
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(bmap_test, AddVector) {
    bmap<int, NoCopy> testMap;

    for (int i=10000; i>0; --i) // add backwards, and enough to ensure tree gets rebalanced often
      testMap.insert(std::pair<int,NoCopy>(i, NoCopy(i)));

    ASSERT_EQ (10000, testMap.size());
    ASSERT_TRUE (testMap.find(1)->second.m_v == 1);
    ASSERT_TRUE (testMap.find(11)->second.m_v == 11);
    ASSERT_TRUE (testMap.find(222)->second.m_v == 222);
    ASSERT_TRUE (testMap.find(555)->second.m_v == 555);
    ASSERT_TRUE (testMap.find(10000)->second.m_v == 10000);
    ASSERT_TRUE (testMap.find(0) == testMap.end());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
// libstdc++/3349 and
// http://gcc.gnu.org/ml/gcc-patches/2001-08/msg01375.html
TEST (bmap_test, ModifiersInsert01)
{
  typedef bmap<int, int>   Map;
  Map             M;
  Map::iterator   hint;

  hint = M.insert(Map::value_type(7, 0)).first;

  M.insert(hint, Map::value_type(8, 1));
  M.insert(M.begin(), Map::value_type(9, 2));

#if 0
  // The tree's __rb_verify() member must be exposed in bmap<> before this
  // will even compile.  It's good test to see that "missing" entries are
  // in fact present in the {bmap,tree}, but in the wrong place.
  if (0)
  {
      Map::iterator  i = M.begin();
      while (i != M.end()) {
          std::cerr << '(' << i->first << ',' << i->second << ")\n";
          ++i;
      }
      std::cerr << "tree internal verify: "
                << std::boolalpha << M.__rb_verify() << "\n";
  }
#endif

  VERIFY ( M.find(7) != M.end() );
  VERIFY ( M.find(8) != M.end() );
  VERIFY ( M.find(9) != M.end() );
}

struct T { int i; };

// T must be LessThanComparable to pass concept-checks
bool operator<(T l, T r) { return l.i < r.i; }

#ifdef BMAP_CANT_DO_THIS_I_GUESS
static int bmap_swap_calls;

namespace bc_stdcxx {
  template<>
    void
    bmap<T, int>::swap(bmap<T, int>&)
    { ++bmap_swap_calls; }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
// Should use bmap specialization for swap.
TEST (bmap_test, ModifiersSwap01)
{
  bmap<T, int> A;
  bmap<T, int> B;
  bmap_swap_calls = 0;
  std::swap(A, B);
  VERIFY(1 == bmap_swap_calls);
}
#endif

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
// A few tests for equal_range of bmaps
TEST (bmap_test, Operations01)
{
  using namespace std;

  bmap<int, int> m0;
  typedef bmap<int, int>::iterator iterator;
  typedef bpair<iterator, bool> insert_return_type;
  bpair<iterator, iterator> pp0;
  typedef bmap<int, int>::value_type value_type;

  pp0 = m0.equal_range(1);
  VERIFY( m0.count(1) == 0 );
  VERIFY( pp0.first == m0.end() );
  VERIFY( pp0.second == m0.end() );

  m0.insert(value_type(1, 1));
  m0.insert(value_type(2, 2));

  {
  insert_return_type irt2 = m0.insert(value_type(3, 3));

  pp0 = m0.equal_range(2);
  VERIFY( m0.count(2) == 1 );
  VERIFY( *pp0.first == value_type(2, 2) );
  VERIFY( *pp0.second == value_type(3, 3) );

  // NOTE: these tests were designed to test the stability of iterators across inserts. With BTREE maps, that is no longer
  // true (iterators are never guaranteed to be stable across inserts/deletes).
#if defined REMOVED_BY_BENTLEY
  VERIFY( pp0.first == irt1.first );
  VERIFY( --pp0.first == irt0.first );
#endif
  VERIFY( pp0.second == irt2.first );
  }

  m0.insert(value_type(3, 4));
  m0.insert(value_type(3, 5));
    {
  insert_return_type irt4 = m0.insert(value_type(4, 6));

  pp0 = m0.equal_range(3);
  VERIFY( m0.count(3) == 1 );
  VERIFY( *pp0.first == value_type(3, 3) );
  VERIFY( *pp0.second == value_type(4, 6) );
#if defined REMOVED_BY_BENTLEY
  VERIFY( pp0.first == irt2.first );
  VERIFY( --pp0.first == irt1.first );
#endif
  VERIFY( pp0.second == irt4.first );
    }

  m0.insert(value_type(0, 7));
  m0.insert(value_type(1, 8));
  m0.insert(value_type(1, 9));
  m0.insert(value_type(1, 10));

  pp0 = m0.equal_range(1);
  VERIFY( m0.count(1) == 1 );
  VERIFY( *pp0.first == value_type(1, 1) );
  VERIFY( *pp0.second == value_type(2, 2) );
#if defined REMOVED_BY_BENTLEY
  VERIFY( pp0.first == irt0.first );
  VERIFY( --pp0.first == irt5.first );
  VERIFY( pp0.second == irt1.first );
#endif

  m0.insert(value_type(5, 11));
  m0.insert(value_type(5, 12));
  m0.insert(value_type(5, 13));

  pp0 = m0.equal_range(5);
  VERIFY( m0.count(5) == 1 );
  VERIFY( *pp0.first == value_type(5, 11) );
#if defined REMOVED_BY_BENTLEY
  VERIFY( pp0.first == irt6.first );
  VERIFY( --pp0.first == irt4.first );
  VERIFY( pp0.second == m0.end() );
#endif

  m0.insert(value_type(4, 14));
  m0.insert(value_type(4, 15));
  m0.insert(value_type(4, 16));

  pp0 = m0.equal_range(4);
  VERIFY( m0.count(4) == 1 );
  VERIFY( *pp0.first == value_type(4, 6) );
  VERIFY( *pp0.second == value_type(5, 11) );
#if defined REMOVED_BY_BENTLEY
  VERIFY( pp0.first == irt4.first );
  VERIFY( --pp0.first == irt3.first );
  VERIFY( pp0.second == irt6.first );
#endif

  m0.insert(value_type(0, 17));
  m0.insert(value_type(0, 18));
  m0.insert(value_type(1, 19));

  pp0 = m0.equal_range(0);
  VERIFY( m0.count(0) == 1 );
  VERIFY( *pp0.first == value_type(0, 7) );
  VERIFY( *pp0.second == value_type(1, 1) );
#if defined REMOVED_BY_BENTLEY
  VERIFY( pp0.first == irt5.first );
#endif
  VERIFY( pp0.first == m0.begin() );
#if defined REMOVED_BY_BENTLEY
  VERIFY( pp0.second == irt0.first );
#endif

  pp0 = m0.equal_range(1);
  VERIFY( m0.count(1) == 1 );
  VERIFY( *pp0.first == value_type(1, 1) );
  VERIFY( *pp0.second == value_type(2, 2) );
#if defined REMOVED_BY_BENTLEY
  VERIFY( pp0.first == irt0.first );
  VERIFY( --pp0.first == irt7.first);
  VERIFY( pp0.second == irt1.first );
#endif
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
// A few tests for equal_range of bmaps
TEST (bmap_test, utf8string)
{
    bmap<Utf8String, int> values;
    bvector<Utf8CP> returned;

    values.Insert("abc", 10);
    values.Insert("def", 20);
    values.Insert("ghi", 30);

    for (auto const&  v: values)
      returned.push_back(v.first.c_str());

    VERIFY(strcmp(returned[0], "abc") == 0);
    VERIFY(strcmp(returned[1], "def") == 0);
    VERIFY(strcmp(returned[2], "ghi") == 0);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
// A few tests for equal_range, in the occasion of libstdc++/29385.
TEST (bstdmap_test, Operations01)
{
  std::map<int, int> m0;
  typedef std::map<int, int>::iterator iterator;
  typedef bpair<iterator, bool> insert_return_type;
  bpair<iterator, iterator> pp0;
  typedef std::map<int, int>::value_type value_type;

  insert_return_type irt0 = m0.insert(value_type(1, 1));
  insert_return_type irt1 = m0.insert(value_type(2, 2));
  insert_return_type irt2 = m0.insert(value_type(3, 3));

  pp0 = m0.equal_range(2);
  VERIFY( m0.count(2) == 1 );
  VERIFY( *pp0.first == value_type(2, 2) );
  VERIFY( *pp0.second == value_type(3, 3) );
  VERIFY( pp0.first == irt1.first );
  VERIFY( --pp0.first == irt0.first );
  VERIFY( pp0.second == irt2.first );

  m0.insert(value_type(3, 4));
  insert_return_type irt3 = m0.insert(value_type(3, 5));
  insert_return_type irt4 = m0.insert(value_type(4, 6));

  pp0 = m0.equal_range(3);
  VERIFY( m0.count(3) == 1 );
  VERIFY( *pp0.first == value_type(3, 3) );
  VERIFY( *pp0.second == value_type(4, 6) );
  VERIFY( pp0.first == irt2.first );
  VERIFY( --pp0.first == irt1.first );
  VERIFY( pp0.second == irt4.first );

  insert_return_type irt5 = m0.insert(value_type(0, 7));
  m0.insert(value_type(1, 8));
  m0.insert(value_type(1, 9));
  m0.insert(value_type(1, 10));

  pp0 = m0.equal_range(1);
  VERIFY( m0.count(1) == 1 );
  VERIFY( *pp0.first == value_type(1, 1) );
  VERIFY( *pp0.second == value_type(2, 2) );
  VERIFY( pp0.first == irt0.first );
  VERIFY( --pp0.first == irt5.first );
  VERIFY( pp0.second == irt1.first );

  insert_return_type irt6 = m0.insert(value_type(5, 11));
  m0.insert(value_type(5, 12));
  m0.insert(value_type(5, 13));

  pp0 = m0.equal_range(5);
  VERIFY( m0.count(5) == 1 );
  VERIFY( *pp0.first == value_type(5, 11) );
  VERIFY( pp0.first == irt6.first );
  VERIFY( --pp0.first == irt4.first );
  VERIFY( pp0.second == m0.end() );

  m0.insert(value_type(4, 14));
  m0.insert(value_type(4, 15));
  m0.insert(value_type(4, 16));

  pp0 = m0.equal_range(4);
  VERIFY( m0.count(4) == 1 );
  VERIFY( *pp0.first == value_type(4, 6) );
  VERIFY( *pp0.second == value_type(5, 11) );
  VERIFY( pp0.first == irt4.first );
  VERIFY( --pp0.first == irt3.first );
  VERIFY( pp0.second == irt6.first );

  m0.insert(value_type(0, 17));
  insert_return_type irt7 = m0.insert(value_type(0, 18));
  m0.insert(value_type(1, 19));

  pp0 = m0.equal_range(0);
  VERIFY( m0.count(0) == 1 );
  VERIFY( *pp0.first == value_type(0, 7) );
  VERIFY( *pp0.second == value_type(1, 1) );
  VERIFY( pp0.first == irt5.first );
  VERIFY( pp0.first == m0.begin() );
  VERIFY( pp0.second == irt0.first );

  pp0 = m0.equal_range(1);
  VERIFY( m0.count(1) == 1 );
  VERIFY( *pp0.first == value_type(1, 1) );
  VERIFY( *pp0.second == value_type(2, 2) );
  VERIFY( pp0.first == irt0.first );
  VERIFY( --pp0.first == irt7.first);
  VERIFY( pp0.second == irt1.first );
}
