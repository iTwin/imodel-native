/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/bmap_test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/bmap.h>
#include <Bentley/bstdmap.h>

#define __attribute__(IGNOREDSTUFF)
#define VERIFY(X) ASSERT_TRUE(X)

//  Adapted from unit tests that are part of GCC 
// Copyright (C) 1999, 2000, 2001, 2002, 2003, 2009
// Free Software Foundation, Inc.

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
// @betest                                      Shaun.Sewall                    08/11
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
}

#ifdef BMAP_DOES_NOT_HAVE_AT
// libstdc++/23578
TEST (bmap_test, ElementAccess01) 
{ 
#ifndef NO_CPP_EXCEPTION_HANDLERS
  typedef bmap<int, double> map_type;

  {
    map_type m;
    m[0] = 1.5;

    double& rd = m.at(0);
    VERIFY( rd == 1.5 );
    try
      {
	m.at(1);
      }
    catch(std::out_of_range& )
      {
	// Expected.
      }
    catch(...)
      {
	// Failed.
	throw;
      }    
  }

  {
    map_type m;
    m[1] = 2.5;
    const map_type cm(m);

    const double& crd = cm.at(1);
    VERIFY( crd == 2.5 );
    try
      {
	cm.at(0);
      }
    catch(std::out_of_range& )
      {
	// Expected.
      }
    catch(...)
      {
	// Failed.
	throw;
      }    
  }
#endif
}
#endif

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


// A few tests for equal_range, in the occasion of libstdc++/29385.
TEST (bstdmap_test, Operations01)
{
  using namespace std;

  bstdmap<int, int> m0;
  typedef bstdmap<int, int>::iterator iterator;
  typedef bpair<iterator, bool> insert_return_type;
  bpair<iterator, iterator> pp0;
  typedef bstdmap<int, int>::value_type value_type;

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
