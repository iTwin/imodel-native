/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/bvector_test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/bvector.h>
#include <vector>

//  Adapted from unit tests that are part of GCC 
// Copyright (C) 1999, 2000, 2001, 2002, 2003, 2009
// Free Software Foundation, Inc.

template<typename T>
struct A {T m_value; };

struct B { int m_int;};


  // A (static) class for counting copy constructors and possibly throwing an
  // exception on a desired count.
  class copy_constructor
  {
  public:
    static unsigned int
    count() { return count_; }
    
    static void
    mark_call()
    {
      count_++;
      if (count_ == throw_on_)
    throw std::runtime_error("copy_constructor::mark_call");
    }
      
    static void
    reset()
    {
      count_ = 0;
      throw_on_ = 0;
    }
      
    static void
    throw_on(unsigned int count) { throw_on_ = count; }

  private:
    static unsigned int count_;
    static unsigned int throw_on_;
  };

  // A (static) class for counting assignment operator calls and
  // possibly throwing an exception on a desired count.
  class assignment_operator
  {
  public:
    static unsigned int
    count() { return count_; }
    
    static void
    mark_call()
    {
      count_++;
      if (count_ == throw_on_)
    throw std::runtime_error("assignment_operator::mark_call");
    }

    static void
    reset()
    {
      count_ = 0;
      throw_on_ = 0;
    }

    static void
    throw_on(unsigned int count) { throw_on_ = count; }

  private:
    static unsigned int count_;
    static unsigned int throw_on_;
  };
  
  // A (static) class for tracking calls to an object's destructor.
  class destructor
  {
  public:
    static unsigned int
    count() { return _M_count; }
    
    static void
    mark_call() { _M_count++; }

    static void
    reset() { _M_count = 0; }

  private:
    static unsigned int _M_count;
  };
  

  // An class of objects that can be used for validating various
  // behaviours and guarantees of containers and algorithms defined in
  // the standard library.
  class copy_tracker
  {
  public:
    // Creates a copy-tracking object with the given ID number.  If
    // "throw_on_copy" is set, an exception will be thrown if an
    // attempt is made to copy this object.
    copy_tracker(int id = next_id_--, bool throw_on_copy = false)
    : id_(id) , throw_on_copy_(throw_on_copy) { }

    // Copy-constructs the object, marking a call to the copy
    // constructor and forcing an exception if indicated.
    copy_tracker(const copy_tracker& rhs)
    : id_(rhs.id()), throw_on_copy_(rhs.throw_on_copy_)
    {
      if (throw_on_copy_)
    copy_constructor::throw_on(copy_constructor::count() + 1);
      copy_constructor::mark_call();
    }

    // Assigns the value of another object to this one, tracking the
    // number of times this member function has been called and if the
    // other object is supposed to throw an exception when it is
    // copied, well, make it so.
    copy_tracker&
    operator=(const copy_tracker& rhs)
    { 
      id_ = rhs.id();
      if (rhs.throw_on_copy_)
        assignment_operator::throw_on(assignment_operator::count() + 1);
      assignment_operator::mark_call();
      return *this;
    }

    ~copy_tracker()
    { destructor::mark_call(); }

    int
    id() const { return id_; }

    static void
    reset()
    {
      copy_constructor::reset();
      assignment_operator::reset();
      destructor::reset();
    }

  private:
    int   id_;
    const bool  throw_on_copy_;
    static int next_id_;
  };

  inline bool
  operator==(const copy_tracker& lhs, const copy_tracker& rhs)
  { return lhs.id() == rhs.id(); }

  inline bool
  operator<(const copy_tracker& lhs, const copy_tracker& rhs)
  { return lhs.id() < rhs.id(); }

  class tracker_allocator_counter
  {
  public:
    typedef std::size_t    size_type; 
    
    static void*
    allocate(size_type blocksize)
    {
      allocationCount_ += blocksize;
      return ::operator new(blocksize);
    }
    
    static void
    construct() { constructCount_++; }

    static void
    destroy() { destructCount_++; }

    static void
    deallocate(void* p, size_type blocksize)
    {
      ::operator delete(p);
      deallocationCount_ += blocksize;
    }
    
    static size_type
    get_allocation_count() { return allocationCount_; }
    
    static size_type
    get_deallocation_count() { return deallocationCount_; }
    
    static int
    get_construct_count() { return constructCount_; }

    static int
    get_destruct_count() { return destructCount_; }
    
    static void
    reset()
    {
      allocationCount_ = 0;
      deallocationCount_ = 0;
      constructCount_ = 0;
      destructCount_ = 0;
    }

 private:
    static size_type  allocationCount_;
    static size_type  deallocationCount_;
    static int        constructCount_;
    static int        destructCount_;
  };
  // A simple basic allocator that just forwards to the
  // tracker_allocator_counter to fulfill memory requests.  This class
  // is templated on the target object type, but tracker isn't.
  template<class T>
  class tracker_allocator
  {
  private:
    typedef tracker_allocator_counter counter_type;

  public:
    typedef T              value_type;
    typedef T*             pointer;
    typedef const T*       const_pointer;
    typedef T&             reference;
    typedef const T&       const_reference;
    typedef std::size_t    size_type; 
    typedef std::ptrdiff_t difference_type; 
    
    template<class U> struct rebind { typedef tracker_allocator<U> other; };
    
    pointer
    address(reference value) const
    { return &value; }
    
    const_pointer
    address(const_reference value) const
    { return &value; }
    
    tracker_allocator() throw()
    { }

    tracker_allocator(const tracker_allocator&) throw()
    { }

    template<class U>
      tracker_allocator(const tracker_allocator<U>&) throw()
      { }

    ~tracker_allocator() throw()
    { }

    size_type
    max_size() const throw()
    { return size_type(-1) / sizeof(T); }

    pointer
    allocate(size_type n, const void* = 0)
    { return static_cast<pointer>(counter_type::allocate(n * sizeof(T))); }

    void
    construct(pointer p, const T& value)
    {
      ::new ((void *)p) T(value);
      counter_type::construct();
    }

#ifdef __GXX_EXPERIMENTAL_CXX0X__BENTLEY_REMOVED
      template<typename... Args>
        void
        construct(pointer p, Args&&... args) 
    {
      ::new((void *)p) T(std::forward<Args>(args)...);
      counter_type::construct();
    }
#endif

    void
    destroy(pointer p)
    {
      p->~T();
      counter_type::destroy();
    }

    void
    deallocate(pointer p, size_type num)
    { counter_type::deallocate(p, num * sizeof(T)); }
  };

  template<class T1, class T2>
    bool
    operator==(const tracker_allocator<T1>&, 
           const tracker_allocator<T2>&) throw()
    { return true; }

  template<class T1, class T2>
    bool
    operator!=(const tracker_allocator<T1>&, 
           const tracker_allocator<T2>&) throw()
    { return false; }

  //object_counter::size_type  object_counter::count = 0;
  unsigned int copy_constructor::count_ = 0;
  unsigned int copy_constructor::throw_on_ = 0;
  unsigned int assignment_operator::count_ = 0;
  unsigned int assignment_operator::throw_on_ = 0;
  unsigned int destructor::_M_count = 0;
  int copy_tracker::next_id_ = 0;

  tracker_allocator_counter::size_type 
  tracker_allocator_counter::allocationCount_ = 0;
  
  tracker_allocator_counter::size_type 
  tracker_allocator_counter::deallocationCount_ = 0;

  int tracker_allocator_counter::constructCount_ = 0;
  int tracker_allocator_counter::destructCount_ = 0;

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct bvector_test : ::testing::Test
{

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    sam.wilson    03/10
+---------------+---------------+---------------+---------------+---------------+------*/
bvector_test ()
    {
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    sam.wilson    03/10
+---------------+---------------+---------------+---------------+---------------+------*/
// *** FAILS - the C runtime must throw an exception in order for this work, but we compile with that turned off
//TEST_F (bvector_test, Capacity_1)
//    {
//      bvector<int> v;
//      try
//        {
//          v.resize(v.max_size());  
//          v[v.max_size() - 1] = 2002;
//        }
//      catch (const std::bad_alloc&)
//        {
//          test = true;
//        }
//      catch (...)
//        {
//          test = false;
//        }
//      ASSERT_TRUE( test );
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (bvector_test, Capacity_2)
{
  // non POD types
  bvector< A<B> > vec01;
  typedef bvector< A<B> >::size_type size_type;

  size_type sz01 = vec01.capacity();
  vec01.reserve(100);
  size_type sz02 = vec01.capacity();
  ASSERT_TRUE( sz02 >= sz01 );
  
  sz01 = vec01.size() + 5;
  vec01.resize(sz01);
  sz02 = vec01.size();
  ASSERT_TRUE( sz01 == sz02 );

  sz01 = vec01.size() - 5;
  vec01.resize(sz01);
  sz02 = vec01.size();
  ASSERT_TRUE( sz01 == sz02 );
}

// Verifies basic functionality of reserve() with forced reallocation.
TEST_F (bvector_test, test_reserve)
{
  typedef copy_tracker T;
  typedef bvector<T, tracker_allocator<T> > X;

  tracker_allocator_counter::reset();
  {
    X a(3);
    const X::size_type old_size     = a.size();
    const X::size_type old_capacity = a.capacity();
    const X::size_type new_capacity = old_capacity + 10;
    T::reset();
    
    a.reserve(new_capacity);

    // [23.2.4.1 (2)]
    ASSERT_TRUE(new_capacity <= a.capacity());
    // [23.2.4.1 (3)]
    ASSERT_TRUE(old_size == a.size());
    ASSERT_TRUE(copy_constructor::count() <= old_size);
    ASSERT_TRUE(destructor::count() <= old_size);
  }
  // check for memory leaks
  ASSERT_TRUE(tracker_allocator_counter::get_allocation_count() == tracker_allocator_counter::get_deallocation_count());
}

#ifndef NO_CPP_EXCEPTION_HANDLERS

// Verifies that reserve() with reallocation offers the strong
// exception guarantee.
TEST_F (bvector_test, test_reserve_exception_guarantee)
{
  typedef copy_tracker T;
  typedef bvector<T, tracker_allocator<T> > X;

  tracker_allocator_counter::reset();
  {
    X a(7);
    a.size();
    const X::size_type old_capacity = a.capacity();
    const X::size_type new_capacity = old_capacity + 10;
    T::reset();
    copy_constructor::throw_on(3);
    
    try
    {
      a.reserve(new_capacity);
      ASSERT_TRUE(false);
    }
    catch (...)
    {
    }

    ASSERT_TRUE(old_capacity == a.capacity());
    ASSERT_TRUE(copy_constructor::count() == destructor::count()+1);
  }
  ASSERT_TRUE(tracker_allocator_counter::get_allocation_count() == tracker_allocator_counter::get_deallocation_count());
}

#endif

TEST_F (bvector_test, S29134)
{
  bvector<int> v;

  ASSERT_TRUE( v.max_size() == v.get_allocator().max_size() );
}

//#ifdef __GXX_EXPERIMENTAL_CXX0X__
//TEST_F (bvector_test, S42573)
//{
//  bvector<int> v(100);
//  v.push_back(1);
//  v.push_back(1);
//  ASSERT_TRUE( v.size() < v.capacity() );
//  v.shrink_to_fit();
//  ASSERT_TRUE( v.size() == v.capacity() );
//}
//#endif

TEST_F (bvector_test, ConstructorCompilation)
{
  // 1
  bvector< A<B> > vec01;
  bvector< A<B> > vec02(5);
  typedef bvector< A<B> >::size_type size_type;

  vec01 = vec02;
}

// 2
#if !defined (__clang__) // error: explicit instantiation ... must occur in namespace 'bc_stdcxx'
template class bvector<double>;
template class bvector< A<B> >;
#endif

TEST_F (bvector_test, ConstructorCompilation102)
{
  bvector<int> v1;
  bvector<int> v2(v1);
}

// test range constructors and range-fill constructor
TEST_F (bvector_test, Range_constructors)
{
  const int A[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
  const int B[] = {7, 7, 7, 7, 7};
  const int N = sizeof(A) / sizeof(int);
  const int M = sizeof(B) / sizeof(int);
  
  bvector<int> v3(A, A + N);
  ASSERT_TRUE(std::equal(v3.begin(), v3.end(), A));
  
  bvector<int> v4(v3.begin(), v3.end());
  ASSERT_TRUE(std::equal(v4.begin(), v4.end(), A));
  
  bvector<int> v5(M, 7);
  ASSERT_TRUE(std::equal(v5.begin(), v5.end(), B));
  ASSERT_TRUE(std::equal(B, B + M, v5.begin()));
}

// operator=()
//
// case 1: lhs.size() > rhs.size()
// case 2: lhs.size() < rhs.size() < lhs.capacity()
// case 3: lhs.capacity() < rhs.size()
//
TEST_F (bvector_test, test_assignment_operator_1)
{
  // setup
  typedef copy_tracker T;
  typedef bvector<T, tracker_allocator<T> > X;

  X r(9);
  X a(r.size() - 2);
  copy_tracker::reset();
  tracker_allocator_counter::reset();

  // preconditions
  ASSERT_TRUE(r.size() > a.size());

  // run test
  r = a;

  // assert postconditions
  ASSERT_TRUE(r == a);
  ASSERT_TRUE(tracker_allocator_counter::get_allocation_count() == 0);

  // teardown
  copy_tracker::reset();
  tracker_allocator_counter::reset();
}

TEST_F (bvector_test, test_assignment_operator_2)
{
  // setup
  typedef copy_tracker T;
  typedef bvector<T, tracker_allocator<T> > X;

  X r(1);
  r.reserve(17);
  X a(r.size() + 7);
  copy_tracker::reset();
  tracker_allocator_counter::reset();

  // preconditions
  ASSERT_TRUE(r.size() < a.size());
  ASSERT_TRUE(a.size() < r.capacity());

  // run test
  r = a;

  // assert postconditions
  ASSERT_TRUE(r == a);
  ASSERT_TRUE(tracker_allocator_counter::get_allocation_count() == 0);

  // teardown
  copy_tracker::reset();
  tracker_allocator_counter::reset();
}

TEST_F (bvector_test, test_assignment_operator_3)
{
  // setup
  typedef copy_tracker T;
  typedef bvector<T, tracker_allocator<T> > X;

  tracker_allocator_counter::reset();
  {
    X r(1);
    X a(r.capacity() + 7);
    copy_tracker::reset();

    // preconditions
    ASSERT_TRUE(r.capacity() < a.size());

    // run test
    r = a;

    // assert postconditions
    ASSERT_TRUE(r == a);
  }
  ASSERT_TRUE(tracker_allocator_counter::get_allocation_count() == tracker_allocator_counter::get_deallocation_count());

  // teardown
  copy_tracker::reset();
  tracker_allocator_counter::reset();
}

// fill assign()
//
// case 1: [23.2.4.1 (3)] n <= size()
// case 2: [23.2.4.1 (3)] size() < n <= capacity()
// case 3: [23.2.4.1 (3)] n > capacity()
// case 4: [23.2.4.1 (3)] n > capacity(), exception guarantees
// case 5: [23.1.1 (9)] fill assign disguised as a range assign
//
TEST_F (bvector_test, test_fill_assign_1)
{
  // setup
  typedef copy_tracker T;
  typedef bvector<T, tracker_allocator<T> > X;

  X a(7);
  X::size_type old_size = a.size();
  X::size_type new_size = old_size - 2;
  const T t;

  copy_tracker::reset();
  tracker_allocator_counter::reset();

  // run test
  a.assign(new_size, t);

  // assert postconditions
  ASSERT_TRUE(a.size() == new_size);
  ASSERT_TRUE(tracker_allocator_counter::get_allocation_count() == 0);

  // teardown
  copy_tracker::reset();
  tracker_allocator_counter::reset();
}

TEST_F (bvector_test, test_fill_assign_2)
{
  // setup
  typedef copy_tracker T;
  typedef bvector<T, tracker_allocator<T> > X;

  X a(7);
  a.reserve(11);
  X::size_type old_size     = a.size();
  X::size_type old_capacity = a.capacity();
  X::size_type new_size     = old_size + 2;
  const T t;

  copy_tracker::reset();
  tracker_allocator_counter::reset();

  // assert preconditions
  ASSERT_TRUE(old_size < new_size);
  ASSERT_TRUE(new_size <= old_capacity);

  // run test
  a.assign(new_size, t);

  // assert postconditions
  ASSERT_TRUE(a.size() == new_size);
  ASSERT_TRUE(tracker_allocator_counter::get_allocation_count() == 0);

  // teardown
  copy_tracker::reset();
  tracker_allocator_counter::reset();
}

TEST_F (bvector_test, test_fill_assign_3)
{
  // setup
  typedef copy_tracker T;
  typedef bvector<T, tracker_allocator<T> > X;

  tracker_allocator_counter::reset();
  {
    X a(7);
    X::size_type old_capacity = a.capacity();
    X::size_type new_size     = old_capacity + 4;
    const T t;

    copy_tracker::reset();

    // assert preconditions
    ASSERT_TRUE(new_size > old_capacity);

    // run test
    a.assign(new_size, t);

    // assert postconditions
    ASSERT_TRUE(a.size() == new_size);
  }

  ASSERT_TRUE(tracker_allocator_counter::get_allocation_count() > 0);
  ASSERT_TRUE(tracker_allocator_counter::get_allocation_count() == tracker_allocator_counter::get_deallocation_count());

  // teardown
  copy_tracker::reset();
  tracker_allocator_counter::reset();
}

TEST_F (bvector_test, test_fill_assign_4)
{
  // setup
  typedef copy_tracker T;
  typedef bvector<T, tracker_allocator<T> > X;

  X a(7);
  X::size_type old_size  = a.size();
  X::size_type new_size  = old_size - 2;
  X::value_type new_value = 117;

  copy_tracker::reset();
  tracker_allocator_counter::reset();

  // run test
  a.assign(new_size, new_value);

  // assert postconditions
  ASSERT_TRUE(a.size() == new_size);
  ASSERT_TRUE(tracker_allocator_counter::get_allocation_count() == 0);

  // teardown
  copy_tracker::reset();
  tracker_allocator_counter::reset();
}

TEST_F (bvector_test, test_range_assign_2)
{
  // setup
  typedef copy_tracker T;
  typedef bvector<T, tracker_allocator<T> > X;

  X a(7);
  X b(3);

  copy_tracker::reset();
  tracker_allocator_counter::reset();

  // assert preconditions
  ASSERT_TRUE(b.size() < a.capacity());

  // run test
  a.assign(b.begin(), b.end());

  // assert postconditions
  ASSERT_TRUE(a.size() == b.size());
  ASSERT_TRUE(a == b);
  ASSERT_TRUE(tracker_allocator_counter::get_allocation_count() == 0);

  // teardown
  copy_tracker::reset();
  tracker_allocator_counter::reset();
}

TEST_F (bvector_test, test_range_assign_3)
{
  // setup
  typedef copy_tracker T;
  typedef bvector<T, tracker_allocator<T> > X;

  X a(7);
  a.reserve(a.size() + 7);
  X b(a.size() + 3);

  copy_tracker::reset();
  tracker_allocator_counter::reset();

  // assert preconditions
  ASSERT_TRUE(a.size() < b.size());
  ASSERT_TRUE(b.size() < a.capacity());

  // run test
  a.assign(b.begin(), b.end());

  // assert postconditions
  ASSERT_TRUE(a.size() == b.size());
  ASSERT_TRUE(a == b);
  ASSERT_TRUE(tracker_allocator_counter::get_allocation_count() == 0);

  // teardown
  copy_tracker::reset();
  tracker_allocator_counter::reset();
}

TEST_F (bvector_test, test_range_assign_4)
{
  // setup
  typedef copy_tracker T;
  typedef bvector<T, tracker_allocator<T> > X;

  tracker_allocator_counter::reset();
  {
    X a(7);
    X b(a.capacity() + 7);

    copy_tracker::reset();

    // assert preconditions
    ASSERT_TRUE(b.size() > a.capacity());

    // run test
    a.assign(b.begin(), b.end());

    // assert postconditions
    ASSERT_TRUE(a.size() == b.size());
    ASSERT_TRUE(a == b);
  }
  ASSERT_TRUE(tracker_allocator_counter::get_allocation_count() > 0);
  ASSERT_TRUE(tracker_allocator_counter::get_allocation_count() == tracker_allocator_counter::get_deallocation_count());

  // teardown
  copy_tracker::reset();
  tracker_allocator_counter::reset();
}


TEST_F (bvector_test, S23578)
{ 
  typedef bvector<int> vector_type;

  {
    const int A[] = { 0, 1, 2, 3, 4 };    
    vector_type v(A, A + 5);
    ASSERT_TRUE( v.data() == &v.front() );
    int* pi = v.data();
    ASSERT_TRUE( *pi == 0 );
  }

  {
    const int A[] = { 4, 3, 2, 1, 0 };    
    const vector_type cv(A, A + 5);
    ASSERT_TRUE( cv.data() == &cv.front() );
    const int* pci = cv.data();
    ASSERT_TRUE( *pci == 4 );
  }
}

TEST_F (bvector_test, InsertCompilation)
{
  // POD types
  typedef bvector<int> vec_POD;
  vec_POD    vec01;
  int   i01 = 5;
  int* pi01 = &i01;
  vec01.insert(vec01.begin(), pi01, pi01 + 1);

  // non POD types
  typedef bvector< A<B> > vec_nonPOD;
  vec_nonPOD vec02;
  A<B>  np01;
  A<B>*  pnp01 = &np01;
  vec02.insert(vec02.begin(), pnp01, pnp01 + 1);
}

TEST_F (bvector_test, Assign)
{
  const int K = 417;
  const int A[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
  const int B[] = {K, K, K, K, K};
  const std::size_t N = sizeof(A) / sizeof(int);
  const std::size_t M = sizeof(B) / sizeof(int);

  // assign from pointer range
  bvector<int> v3;
  v3.assign(A, A + N);
  ASSERT_TRUE(std::equal(v3.begin(), v3.end(), A));
  ASSERT_TRUE(v3.size() == N);

  // assign from iterator range
  bvector<int> v4;
  v4.assign(v3.begin(), v3.end());
  ASSERT_TRUE(std::equal(v4.begin(), v4.end(), A));
  ASSERT_TRUE(std::equal(A, A + N, v4.begin()));

  // assign from initializer range with resize
  v4.assign(M, K);
  ASSERT_TRUE(std::equal(v4.begin(), v4.end(), B));
  ASSERT_TRUE(std::equal(B, B + M, v4.begin()));
  ASSERT_TRUE((v4.size() == M) && (M != N));
}

namespace EraseTest 
    {
    const int  A[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    const int A1[] = {0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    const int A2[] = {0, 2, 3, 4, 10, 11, 12, 13, 14, 15};
    const int A3[] = {0, 2, 3, 4, 10, 11};
    const int A4[] = {4, 10, 11};
    const int A5[] = {4, 10};
    const unsigned int  N = sizeof(A)  / sizeof(int);
    const unsigned int N1 = sizeof(A1) / sizeof(int);
    const unsigned int N2 = sizeof(A2) / sizeof(int);
    const unsigned int N3 = sizeof(A3) / sizeof(int);
    const unsigned int N4 = sizeof(A4) / sizeof(int);
    const unsigned int N5 = sizeof(A5) / sizeof(int);
    };

TEST_F (bvector_test, Erase1)
{
  typedef bvector<int>   vec_type;
  typedef vec_type::iterator iterator_type;

  vec_type v(EraseTest::A, EraseTest::A + EraseTest::N);

  iterator_type it1 = v.erase(v.begin() + 1);
  ASSERT_TRUE( it1 == v.begin() + 1 );
  ASSERT_TRUE( v.size() == EraseTest::N1 );
  ASSERT_TRUE( std::equal(v.begin(), v.end(), EraseTest::A1) );
  
  iterator_type it2 = v.erase(v.begin() + 4, v.begin() + 9);
  ASSERT_TRUE( it2 == v.begin() + 4 );
  ASSERT_TRUE( v.size() == EraseTest::N2 );
  ASSERT_TRUE( std::equal(v.begin(), v.end(), EraseTest::A2) );
  
  iterator_type it3 = v.erase(v.begin() + 6, v.end());
  ASSERT_TRUE( it3 == v.begin() + 6 );
  ASSERT_TRUE( v.size() == EraseTest::N3 );
  ASSERT_TRUE( std::equal(v.begin(), v.end(), EraseTest::A3) );

  iterator_type it4 = v.erase(v.begin(), v.begin() + 3);
  ASSERT_TRUE( it4 == v.begin() );
  ASSERT_TRUE( v.size() == EraseTest::N4 );
  ASSERT_TRUE( std::equal(v.begin(), v.end(), EraseTest::A4) );

  iterator_type it5 = v.erase(v.begin() + 2);
  ASSERT_TRUE( it5 == v.begin() + 2 );
  ASSERT_TRUE( v.size() == EraseTest::N5 );
  ASSERT_TRUE( std::equal(v.begin(), v.end(), EraseTest::A5) );

  iterator_type it6 = v.erase(v.begin(), v.end());
  ASSERT_TRUE( it6 == v.begin() );
  ASSERT_TRUE( v.empty() );
}

TEST_F (bvector_test, Erase2)
{
  typedef bvector<bvector<int> >   vec_type;
  typedef vec_type::iterator          iterator_type;

  vec_type v, v1, v2, v3, v4, v5;
  for (unsigned int i = 0; i < EraseTest::N; ++i)
    v.push_back(bvector<int>(1, EraseTest::A[i]));
  for (unsigned int i = 0; i < EraseTest::N1; ++i)
    v1.push_back(bvector<int>(1, EraseTest::A1[i]));
  for (unsigned int i = 0; i < EraseTest::N2; ++i)
    v2.push_back(bvector<int>(1, EraseTest::A2[i]));
  for (unsigned int i = 0; i < EraseTest::N3; ++i)
    v3.push_back(bvector<int>(1, EraseTest::A3[i]));
  for (unsigned int i = 0; i < EraseTest::N4; ++i)
    v4.push_back(bvector<int>(1, EraseTest::A4[i]));
  for (unsigned int i = 0; i < EraseTest::N5; ++i)
    v5.push_back(bvector<int>(1, EraseTest::A5[i]));
  
  iterator_type it1 = v.erase(v.begin() + 1);
  ASSERT_TRUE( it1 == v.begin() + 1 );
  ASSERT_TRUE( v.size() == EraseTest::N1 );
  ASSERT_TRUE( std::equal(v.begin(), v.end(), v1.begin()) );
  
  iterator_type it2 = v.erase(v.begin() + 4, v.begin() + 9);
  ASSERT_TRUE( it2 == v.begin() + 4 );
  ASSERT_TRUE( v.size() == EraseTest::N2 );
  ASSERT_TRUE( std::equal(v.begin(), v.end(), v2.begin()) );
  
  iterator_type it3 = v.erase(v.begin() + 6, v.end());
  ASSERT_TRUE( it3 == v.begin() + 6 );
  ASSERT_TRUE( v.size() == EraseTest::N3 );
  ASSERT_TRUE( std::equal(v.begin(), v.end(), v3.begin()) );

  iterator_type it4 = v.erase(v.begin(), v.begin() + 3);
  ASSERT_TRUE( it4 == v.begin() );
  ASSERT_TRUE( v.size() == EraseTest::N4 );
  ASSERT_TRUE( std::equal(v.begin(), v.end(), v4.begin()) );

  iterator_type it5 = v.erase(v.begin() + 2);
  ASSERT_TRUE( it5 == v.begin() + 2 );
  ASSERT_TRUE( v.size() == EraseTest::N5 );
  ASSERT_TRUE( std::equal(v.begin(), v.end(), v5.begin()) );

  iterator_type it6 = v.erase(v.begin(), v.end());
  ASSERT_TRUE( it6 == v.begin() );
  ASSERT_TRUE( v.empty() );
}

struct T { int i; };

static int swap_calls;

// provide specialization of std::swap to handle bvectors
namespace std
{
  template<> 
    void swap(BentleyApi::bvector<T>&, BentleyApi::bvector<T>&) 
    { ++swap_calls; }
}

// Should use swap function specialization above for swap.
TEST_F (bvector_test, Swap01)
{
  bvector<T> A;
  bvector<T> B;
  swap_calls = 0;
  std::swap(A, B);
  ASSERT_TRUE(1 == swap_calls);
}

// Should not use swap function specialization for swap.
TEST_F (bvector_test, Swap02)
{
  using namespace std;
  vector<T> A;
  vector<T> B;
  swap_calls = 0;
  swap(A, B);
  ASSERT_TRUE(0 == swap_calls);
}

#ifdef LIBCXX_NOT_NOW
namespace InsertTypes
{
    struct X { int m_int; };

  template<typename T>
    X operator+(T, std::size_t)
    { return X(); }

  template<typename T>
    X operator-(T, T)
    { return X(); }
}

TEST_F (bvector_test, InsertTypesCompilationTest)
{
  bvector<InsertTypes::X> v(5);
  const bvector<InsertTypes::X> w(1);

  v[0];
  w[0];
  v.size();
  v.capacity();
  v.resize(1);
  v.insert(v.begin(), InsertTypes::X());
  v.insert(v.begin(), 1, InsertTypes::X());
  v.insert(v.begin(), w.begin(), w.end());
  v = w;
}
#endif

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    08/11
//---------------------------------------------------------------------------------------
TEST (BVectorTests, InitBVector)
{
    bvector<int> vector;
    
    vector.push_back (0);
    vector.push_back (1);
    vector.push_back (2);
    
    ASSERT_EQ (0, vector[0]);
    ASSERT_EQ (1, vector[1]);
    ASSERT_EQ (2, vector[2]);
    ASSERT_EQ (3, vector.size ());
    
    vector.pop_back ();
    vector.pop_back ();
    vector.pop_back ();
    
    ASSERT_EQ(0, vector.size ());
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
TEST (BVectorTests, InitializerList)
    {
    bvector<int> numbers = { 2, 4, 6 };
    ASSERT_TRUE(3 == numbers.size());
    EXPECT_TRUE(2 == numbers[0]);
    EXPECT_TRUE(4 == numbers[1]);
    EXPECT_TRUE(6 == numbers[2]);

    numbers = { 10 };
    ASSERT_TRUE(1 == numbers.size());
    EXPECT_TRUE(10 == numbers[0]);

    numbers.assign({ 1, 3, 5 });
    ASSERT_TRUE(3 == numbers.size());
    EXPECT_TRUE(1 == numbers[0]);
    EXPECT_TRUE(3 == numbers[1]);
    EXPECT_TRUE(5 == numbers[2]);

    numbers.insert(numbers.begin() + 1, { 7, 9 });
    ASSERT_TRUE(5 == numbers.size());
    EXPECT_TRUE(1 == numbers[0]);
    EXPECT_TRUE(7 == numbers[1]);
    EXPECT_TRUE(9 == numbers[2]);
    EXPECT_TRUE(3 == numbers[3]);
    EXPECT_TRUE(5 == numbers[4]);
    }
