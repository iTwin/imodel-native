/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/RefCounted_test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/RefCounted.h>

namespace {

static int s_baseCount;
static int s_base2Count;
static int s_derivedCount;
static int s_derived2Count;

struct Base : IRefCounted
    {
    Base()  {++s_baseCount;}
    ~Base() {--s_baseCount;}
    };

struct Derived : RefCounted<Base>
    {
    DEFINE_BENTLEY_NEW_DELETE_OPERATORS

    Derived()  {++s_derivedCount;}
    ~Derived() {--s_derivedCount;}
    };


struct Base2 : IRefCounted
    {
    Base2()  {++s_base2Count;}
    ~Base2() {--s_base2Count;}
    };

struct Derived2 : Base, Base2
    {
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS

    Derived2()  {DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT; ++s_derived2Count;}
    ~Derived2() {--s_derived2Count;}

    uint32_t GetRefCount() const {return m_refCount.load();}

    static Derived2* Factory() {return new Derived2;}
    };
}

//---------------------------------------------------------------------------------------
// @betest                                      Sam.Wilson          01/15
//---------------------------------------------------------------------------------------
TEST (RefCountedTests, Test1)
{
    // Basic addref/release
        {
        auto d = new Derived;
        ASSERT_EQ( s_baseCount,     1 );
        ASSERT_EQ( s_derivedCount,  1 );
        ASSERT_EQ( d->GetRefCount(), 0);
        d->AddRef();
        ASSERT_EQ( s_baseCount,     1 );
        ASSERT_EQ( s_derivedCount,  1 );
        ASSERT_EQ( d->GetRefCount(), 1);
        d->Release();
        ASSERT_EQ( s_baseCount,     0 );
        ASSERT_EQ( s_derivedCount,  0 );
        }

    // Basic addref/release, more than once
        {
        ASSERT_EQ( s_baseCount,     0 );
        ASSERT_EQ( s_derivedCount,  0 );
        auto d = new Derived;
        ASSERT_EQ( d->GetRefCount(), 0);
        ASSERT_EQ( s_baseCount,     1 );
        ASSERT_EQ( s_derivedCount,  1 );
        d->AddRef();
        ASSERT_EQ( d->GetRefCount(), 1);
        ASSERT_EQ( s_baseCount,     1 );
        ASSERT_EQ( s_derivedCount,  1 );
        d->AddRef();
        ASSERT_EQ( d->GetRefCount(), 2);
        ASSERT_EQ( s_baseCount,     1 );
        ASSERT_EQ( s_derivedCount,  1 );
        d->Release();
        ASSERT_EQ( d->GetRefCount(), 1);
        ASSERT_EQ( s_baseCount,     1 );
        ASSERT_EQ( s_derivedCount,  1 );
        d->Release();
        ASSERT_EQ( s_baseCount,     0 );
        ASSERT_EQ( s_derivedCount,  0 );
        }

    // RefCountedPtr should do addref/release when working with the object's Derived interface
        {
        auto rcp = RefCountedPtr<Derived> (new Derived);
        ASSERT_EQ( s_baseCount,     1 );
        ASSERT_EQ( s_derivedCount,  1 );
        ASSERT_EQ( rcp->GetRefCount(), 1);
        rcp = nullptr;
        ASSERT_EQ( s_baseCount,     0 );
        ASSERT_EQ( s_derivedCount,  0 );
        }

    // RefCountedPtr should do addref/release, when working with the object's Base interface
        { 
        auto rcp = RefCountedPtr<Base> (new Derived);
        ASSERT_EQ( s_baseCount,     1 );
        ASSERT_EQ( s_derivedCount,  1 );
        rcp = nullptr;
        ASSERT_EQ( s_baseCount,     0 );
        ASSERT_EQ( s_derivedCount,  0 );
        }

    // RefCountedPtr in a bvector
        {
        bvector<RefCountedPtr<Base>> vb;
        auto d = new Derived;
        ASSERT_EQ( s_baseCount,     1 );
        ASSERT_EQ( s_derivedCount,  1 );
        ASSERT_EQ( d->GetRefCount(), 0);
        vb.push_back (d); // AddRef 
        ASSERT_EQ( d->GetRefCount(), 1);
        ASSERT_EQ( s_baseCount,     1 );
        ASSERT_EQ( s_derivedCount,  1 );
        vb.push_back (d); // AddRef 
        ASSERT_EQ( d->GetRefCount(), 2);
        ASSERT_EQ( s_baseCount,     1 );
        ASSERT_EQ( s_derivedCount,  1 );
        vb.pop_back();  // Release
        ASSERT_EQ( d->GetRefCount(), 1);
        ASSERT_EQ( s_baseCount,     1 );
        ASSERT_EQ( s_derivedCount,  1 );
        vb.pop_back();  // Release
        ASSERT_EQ( s_baseCount,     0 );
        ASSERT_EQ( s_derivedCount,  0 );
        }
}

//---------------------------------------------------------------------------------------
// @betest                                      Sam.Wilson          01/15
//---------------------------------------------------------------------------------------
TEST (RefCountedTests, Test2)
{
    // Basic addref/release
        {
        auto d = Derived2::Factory();
        ASSERT_EQ( d->GetRefCount(), 0);
        ASSERT_EQ( s_baseCount,     1 );
        ASSERT_EQ( s_base2Count,    1 ) << L"Both super classes should have been constructed";
        ASSERT_EQ( s_derived2Count, 1 );
        d->AddRef();
        ASSERT_EQ( d->GetRefCount(), 1);
        ASSERT_EQ( s_baseCount,     1 );
        ASSERT_EQ( s_base2Count,    1 );
        ASSERT_EQ( s_derived2Count, 1 );
        d->Release();
        ASSERT_EQ( s_baseCount,     0 );
        ASSERT_EQ( s_base2Count,    0 ) << L"Both super classes should have been destructed";
        ASSERT_EQ( s_derived2Count, 0 );
        }

    // RefCountedPtr should do basic addref/release
        {
        auto rcp = RefCountedPtr<Base> (Derived2::Factory());
        ASSERT_EQ( s_baseCount,     1 );
        ASSERT_EQ( s_base2Count,    1 ) << L"Both super classes should have been constructed";
        ASSERT_EQ( s_derived2Count, 1 );
        rcp = nullptr;
        ASSERT_EQ( s_baseCount,     0 );
        ASSERT_EQ( s_base2Count,    0 ) << L"Both super classes should have been destructed";
        ASSERT_EQ( s_derived2Count, 0 );
        }

    // All interfaces to d should lead to the one reference count 
        {
        auto d = Derived2::Factory();
        ASSERT_EQ( d->GetRefCount(), 0 );
        auto rcp_d = RefCountedPtr<Derived2> (d);
        auto rcp_b1 = RefCountedPtr<Base> (d);
        auto rcp_b2 = RefCountedPtr<Base2> (d);
        ASSERT_EQ( d->GetRefCount(), 3 );
        ASSERT_EQ( s_baseCount,     1 );
        ASSERT_EQ( s_base2Count,    1 );
        ASSERT_EQ( s_derived2Count, 1 );
        rcp_d = nullptr;
        rcp_b1 = nullptr;
        rcp_b2 = nullptr;
        ASSERT_EQ( s_baseCount,     0 );
        ASSERT_EQ( s_base2Count,    0 );
        ASSERT_EQ( s_derived2Count, 0 );
        }

    if (true)
        {
        //  Create 2 references to Derived2, once on each of its 2 superclass pointers
        auto d2 = Derived2::Factory();

        ASSERT_EQ( d2->GetRefCount(), 0 );
        ASSERT_EQ( s_baseCount,     1 );
        ASSERT_EQ( s_base2Count,    1 );
        ASSERT_EQ( s_derived2Count, 1 );

        bvector<RefCountedPtr<Base>> vb1;
        vb1.push_back (d2); // AddRef
        ASSERT_EQ( d2->GetRefCount(), 1 );
        ASSERT_EQ( s_baseCount,     1 );
        ASSERT_EQ( s_base2Count,    1 );
        ASSERT_EQ( s_derived2Count, 1 );

        bvector<RefCountedPtr<Base2>> vb2;
        vb2.push_back (d2); // AddRef
        ASSERT_EQ( d2->GetRefCount(), 2 );
        ASSERT_EQ( s_baseCount,     1 );
        ASSERT_EQ( s_base2Count,    1 );
        ASSERT_EQ( s_derived2Count, 1 );

        vb1.pop_back(); // Release
        ASSERT_EQ( d2->GetRefCount(), 1 );
        ASSERT_EQ( s_baseCount,     1 );
        ASSERT_EQ( s_base2Count,    1 );
        ASSERT_EQ( s_derived2Count, 1 );

        vb2.pop_back(); // Release
        ASSERT_EQ( s_baseCount,     0 );
        ASSERT_EQ( s_base2Count,    0 );
        ASSERT_EQ( s_derived2Count, 0 );
        }
}
