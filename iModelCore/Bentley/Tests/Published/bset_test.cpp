/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/bset_test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/bset.h>
#include <Bentley/bstdset.h>
#include <vector>

#define __attribute__(IGNOREDSTUFF)
#define VERIFY(X) ASSERT_TRUE(X)

//  Adapted from unit tests that are part of GCC 
// Copyright (C) 1999, 2000, 2001, 2002, 2003, 2009
// Free Software Foundation, Inc.

TEST(bset_test, Basic)
    {
    bset<char> s;
    s.insert ('a');
    s.insert ('a');
    s.insert ('a');
    ASSERT_TRUE( s.size() == 1 );
    }

// A few tests for insert with hint, in the occasion of libstdc++/19422
// and libstdc++/19433.
TEST(bset_test, ModifiersInsert01)
{
  // bool test __attribute__((unused)) = true;
  using namespace std;

  bset<int> s0, s1;
  bset<int>::iterator iter1;
  
  s0.insert(1);
  s1.insert(s1.end(), 1);
  VERIFY( s0 == s1 );

  s0.insert(3);
  s1.insert(s1.begin(), 3);
  VERIFY( s0 == s1 );

  s0.insert(4);
  iter1 = s1.insert(s1.end(), 4);
  VERIFY( s0 == s1 );

  s0.insert(6);
  s1.insert(iter1, 6);
  VERIFY( s0 == s1 );

  s0.insert(2);
  s1.insert(s1.begin(), 2);
  VERIFY( s0 == s1 );

  s0.insert(7);
  s1.insert(s1.end(), 7);
  VERIFY( s0 == s1 );

  s0.insert(5);
  s1.insert(s1.find(4), 5);
  VERIFY( s0 == s1 );

  s0.insert(0);
  s1.insert(s1.end(), 0);
  VERIFY( s0 == s1 );

  s0.insert(8);
  s1.insert(s1.find(3), 8);
  VERIFY( s0 == s1 );
  
  s0.insert(9);
  s1.insert(s1.end(), 9);
  VERIFY( s0 == s1 );

  s0.insert(10);
  s1.insert(s1.begin(), 10);
  VERIFY( s0 == s1 );
}

// A few tests for equal_range, in the occasion of libstdc++/29385.
TEST(bset_test, EqualRange)
{
  bset<int> s0;
  typedef bset<int>::iterator iterator;
  typedef bpair<iterator, bool> insert_return_type;
  bpair<iterator, iterator> pp0;

  pp0 = s0.equal_range(1);
  VERIFY( s0.count(1) == 0 );
  VERIFY( pp0.first == s0.end() );
  VERIFY( pp0.second == s0.end() );

  {
  s0.insert(1);
  s0.insert(2);
  insert_return_type irt2 = s0.insert(3);
 
  pp0 = s0.equal_range(2);
  VERIFY( s0.count(2) == 1 );
  VERIFY( *pp0.first == 2 );
  VERIFY( *pp0.second == 3 );
  VERIFY( pp0.second == irt2.first );
  }

  {
  s0.insert(3);
  s0.insert(3);
  insert_return_type irt4 = s0.insert(4);
  
  pp0 = s0.equal_range(3);
  VERIFY( s0.count(3) == 1 );
  VERIFY( *pp0.first == 3 );
  VERIFY( *pp0.second == 4 );
  VERIFY( pp0.second == irt4.first );
  }

  s0.insert(0);
  s0.insert(1);
  s0.insert(1);
  s0.insert(1);

  pp0 = s0.equal_range(1);
  VERIFY( s0.count(1) == 1 );
  VERIFY( *pp0.first == 1 );
  VERIFY( *pp0.second == 2 );

  s0.insert(5);
  s0.insert(5);
  s0.insert(5);

  pp0 = s0.equal_range(5);
  VERIFY( s0.count(5) == 1 );
  VERIFY( *pp0.first == 5 );
  VERIFY( pp0.second == s0.end() );

  s0.insert(4);
  s0.insert(4);
  s0.insert(4);

  pp0 = s0.equal_range(4);
  VERIFY( s0.count(4) == 1 );  
  VERIFY( *pp0.first == 4 );
  VERIFY( *pp0.second == 5 );  
  
  s0.insert(0);
  s0.insert(0);
  s0.insert(1);

  pp0 = s0.equal_range(0);
  VERIFY( s0.count(0) == 1 );  
  VERIFY( *pp0.first == 0 );
  VERIFY( *pp0.second == 1 );  
  VERIFY( pp0.first == s0.begin() );

  pp0 = s0.equal_range(1);
  VERIFY( s0.count(1) == 1 );  
  VERIFY( *pp0.first == 1 );
  VERIFY( *pp0.second == 2 );  
}

// A few tests for equal_range, in the occasion of libstdc++/29385.
TEST(bstdset_test, EqualRange)
{
  bstdset<int> s0;
  typedef bstdset<int>::iterator iterator;
  typedef bpair<iterator, bool> insert_return_type;
  bpair<iterator, iterator> pp0;

  pp0 = s0.equal_range(1);
  VERIFY( s0.count(1) == 0 );
  VERIFY( pp0.first == s0.end() );
  VERIFY( pp0.second == s0.end() );

  insert_return_type irt0 = s0.insert(1);
  insert_return_type irt1 = s0.insert(2);
  insert_return_type irt2 = s0.insert(3);
 
  pp0 = s0.equal_range(2);
  VERIFY( s0.count(2) == 1 );
  VERIFY( *pp0.first == 2 );
  VERIFY( *pp0.second == 3 );
  VERIFY( pp0.first == irt1.first );
  VERIFY( --pp0.first == irt0.first );
  VERIFY( pp0.second == irt2.first );

  s0.insert(3);
  insert_return_type irt3 = s0.insert(3);
  insert_return_type irt4 = s0.insert(4);
  
  pp0 = s0.equal_range(3);
  VERIFY( s0.count(3) == 1 );
  VERIFY( *pp0.first == 3 );
  VERIFY( *pp0.second == 4 );
  VERIFY( pp0.first == irt2.first );
  VERIFY( --pp0.first == irt1.first );  
  VERIFY( pp0.second == irt4.first );

  insert_return_type irt5 = s0.insert(0);
  s0.insert(1);
  s0.insert(1);
  s0.insert(1);

  pp0 = s0.equal_range(1);
  VERIFY( s0.count(1) == 1 );
  VERIFY( *pp0.first == 1 );
  VERIFY( *pp0.second == 2 );
  VERIFY( pp0.first == irt0.first );
  VERIFY( --pp0.first == irt5.first );  
  VERIFY( pp0.second == irt1.first );

  insert_return_type irt6 = s0.insert(5);
  s0.insert(5);
  s0.insert(5);

  pp0 = s0.equal_range(5);
  VERIFY( s0.count(5) == 1 );
  VERIFY( *pp0.first == 5 );
  VERIFY( pp0.first == irt6.first );
  VERIFY( --pp0.first == irt4.first );  
  VERIFY( pp0.second == s0.end() );

  s0.insert(4);
  s0.insert(4);
  s0.insert(4);

  pp0 = s0.equal_range(4);
  VERIFY( s0.count(4) == 1 );  
  VERIFY( *pp0.first == 4 );
  VERIFY( *pp0.second == 5 );  
  VERIFY( pp0.first == irt4.first );
  VERIFY( --pp0.first == irt3.first );  
  VERIFY( pp0.second == irt6.first );
  
  s0.insert(0);
  insert_return_type irt7 = s0.insert(0);
  s0.insert(1);

  pp0 = s0.equal_range(0);
  VERIFY( s0.count(0) == 1 );  
  VERIFY( *pp0.first == 0 );
  VERIFY( *pp0.second == 1 );  
  VERIFY( pp0.first == irt5.first );
  VERIFY( pp0.first == s0.begin() );
  VERIFY( pp0.second == irt0.first );

  pp0 = s0.equal_range(1);
  VERIFY( s0.count(1) == 1 );  
  VERIFY( *pp0.first == 1 );
  VERIFY( *pp0.second == 2 );  
  VERIFY( pp0.first == irt0.first );
  VERIFY( --pp0.first == irt7.first );
  VERIFY( pp0.second == irt1.first );
}

//#if defined (NOT_NOW_USED_DGNPLATFORM_TEST_UTILITIES)
//
//#ifdef NDEBUG   // only in an optimized build (otherwise, it's too expensive)
//
//template<typename _SetType>
//static void insertAndErase (_SetType& s, wchar_t const* title, std::vector<typename _SetType::value_type>const & values)
//    {
//    StopWatch sw(L"", true);
//
//    //  Insert one by one
//    FOR_EACH(_SetType::value_type const& i , values)
//        s.insert (i);
//
//    VERIFY( s.size() == values.size() );
//
//    //  erase some values
//    for (size_t j = 0; j < values.size(); j += 100)
//        {
//        typename _SetType::iterator it = s.find (values[j]);
//        s.erase (it);
//        }
//
//    //  erase all (remaining) values
//    s.clear ();
//
//    //  Insert a range all at once
//    s.insert (values.begin(), values.end());
//
//    //  erase all values at once
//    s.clear ();
//
//    sw.Stop();
//    int msec = int(sw.GetElapsedSeconds()*1000.0);
//    wprintf (L"    %s = %d\n", title, msec);
//    }
//
//TEST(bset_test, AllocatorPerformance)
//    {
//    int nitems = 1000;
//    for (int iter=0; iter<3; ++iter)
//        {
//        if (true)
//            {
//            bset<int, std::less<int>, std::allocator<int> >             sstd;
//            bset<int, std::less<int>, Memory::HeapzoneAllocator<int> >  shz;
//            bset<int>                                                   sbentley;
//
//            std::vector<int> values;        // values to be inserted into each set.
//            for (int i=0; i<nitems; ++i)      // since we are only testing allocator performance, 
//                values.push_back (i);       //  it doesn't matter what set of values we use.
//
//            wprintf (L"bset_test:AllocatorPerformance - %d ints\n", nitems);
//            insertAndErase (sstd,      L"std allocator", values);
//            insertAndErase (shz,       L"heapzone allocator", values);
//            insertAndErase (sbentley,  L"bentley allocator", values);
//            wprintf (L"\n");
//            }
//
//        if (true)
//            {
//            bset<std::string, std::less<std::string>, std::allocator<std::string> >             ssstd;
//            bset<std::string, std::less<std::string>, Memory::HeapzoneAllocator<std::string> >  sshz;
//            bset<std::string>                                                   ssbentley;
//
//            std::vector<std::string> svalues;        // values to be inserted into each set.
//            char buf[32];
//            for (int i=0; i<nitems; ++i)              // since we are only testing allocator performance, 
//                svalues.push_back (itoa(i,buf,10));         //  it doesn't matter what set of values we use.
//
//            wprintf (L"bset_test:AllocatorPerformance - %d std::strings\n", nitems);
//            insertAndErase (ssstd,      L"std allocator", svalues);
//            insertAndErase (sshz,       L"heapzone allocator", svalues);
//            insertAndErase (ssbentley,  L"bentley allocator", svalues);
//            wprintf (L"\n");
//            }
//
//        nitems *= 10;
//        }
//    }
//
//template<typename _SetType>
//static void findAll (_SetType& s, std::vector<typename _SetType::value_type>const & values)
//    {
//    FOR_EACH(typename _SetType::value_type const& i , values)
//        {
//        VERIFY( s.find(i) != s.end() );
//        }
//    }
//
//template<typename _SetType>
//static void exerciseSet (_SetType& s, wchar_t const* title, std::vector<typename _SetType::value_type>const & values)
//    {
//    std::vector<typename _SetType::value_type> selected_values;
//    for (size_t ivalue=0; ivalue<values.size(); ivalue += 10)
//        selected_values.push_back (values[ivalue]);
//
//    StopWatch sw(L"", true);
//
//    // 1. insert in order
//    FOR_EACH(_SetType::value_type const& i , values)
//        s.insert (i);
//
//    findAll (s, values);
//
//    s.clear ();
//
//    s.insert (values.begin(), values.end());
//    s.clear ();
//
//    // 2. insert in reverse order
//    for (std::vector<_SetType::value_type>::const_reverse_iterator rit = values.rbegin(); rit != values.rend(); ++rit)
//        s.insert (*rit);
//
//    findAll (s, values);
//
//    s.clear ();
//
//    // 3. insert a selection of values over and over
//    for (size_t repeat=0; repeat < 100; ++repeat)
//        {
//        FOR_EACH(_SetType::value_type const& i , selected_values)
//            s.insert (i);
//        }
//
//    findAll (s, selected_values);
//    s.clear ();
//
//    s.insert (selected_values.begin(), selected_values.end());
//    findAll (s, selected_values);
//    s.clear ();
//
//    //  Do some erasing
//    s.insert (values.begin(), values.end());
//    for (size_t j = 0; j < values.size(); j += 100)
//        {
//        typename _SetType::iterator it = s.find (values[j]);
//        s.erase (it);
//        }
//
//    s.clear ();
//
//    sw.Stop();
//    int msec = int(sw.GetElapsedSeconds()*1000.0);
//    wprintf (L"    %s = %d\n", title, msec);
//    }
//
//TEST(bset_test, ImplPerformance)
//    {
//    int nitems = 1000;
//    for (int iter=0; iter<3; ++iter)
//        {
//        if (true)
//            {
//            bset    <int, std::less<int>, std::allocator<int> > sbentley;
//            std::set<int, std::less<int>, std::allocator<int> > sstd;
//
//            std::vector<int> values;        
//            for (int i=0; i<nitems; ++i)     
//                values.push_back (i);       
//
//            wprintf (L"bset_test:ImplPerformance - %d ints\n", nitems);
//            exerciseSet (sstd,      L"std::set", values);
//            exerciseSet (sbentley,  L"bset", values);
//            wprintf (L"\n");
//            }
//
//        if (true)
//            {
//            bset    <std::string, std::less<std::string>, std::allocator<std::string> > sbentley;
//            std::set<std::string, std::less<std::string>, std::allocator<std::string> > sstd;
//
//            std::vector<std::string> values;        
//            char buf[32];
//            for (int i=0; i<nitems; ++i)     
//                values.push_back (itoa(i,buf,10));       
//
//            wprintf (L"bset_test:ImplPerformance - %d std::strings\n", nitems);
//            exerciseSet (sstd,      L"std::set", values);
//            exerciseSet (sbentley,  L"bset", values);
//            wprintf (L"\n");
//            }
//
//        nitems *= 10;
//        }
//    }
//
//#endif // NDEBUG
//
//#endif // defined (NOT_NOW_USED_DGNPLATFORM_TEST_UTILITIES)
