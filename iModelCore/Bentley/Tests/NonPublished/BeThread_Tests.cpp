/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/BeThread.h>
#include <Bentley/BeThreadLocalStorage.h>
#include <thread>

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeThreadUtilities, Atomic1)
    {
    BeAtomic<uint32_t> v1;

    //  set
    v1.store(2);
    ASSERT_TRUE (v1.load()==2);
    v1.store(0);
    ASSERT_TRUE (v1.load()==0);
    v1.store(0);
    ASSERT_TRUE (v1.load()==0);

    //  preinc/predec
    v1.store(0);
    ASSERT_TRUE (++v1 == 1); //returns new value
    ASSERT_TRUE (++v1 == 2);
    ASSERT_TRUE (--v1 == 1);
    ASSERT_TRUE (--v1 == 0);
    ASSERT_TRUE (--v1 == UINT32_MAX);
    ASSERT_TRUE (++v1 == 0);

    //  postinc/postdec
    v1.store(0);
    ASSERT_TRUE (v1++ == 0); //returns old value
    ASSERT_TRUE (v1++ == 1);
    ASSERT_TRUE (v1-- == 2);
    ASSERT_TRUE (v1-- == 1);
    ASSERT_TRUE (v1-- == 0);
    ASSERT_TRUE (v1++ == UINT32_MAX);

    //  add
    v1.store(0);
    v1 += 10;
    ASSERT_TRUE (v1.load() == 10);
    v1 -= 10;
    ASSERT_TRUE (v1.load() == 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static BeAtomic<uint32_t> s_sharedInt;
#define INC_SHARED_INT_TIMES    10000

static void incrementSharedInt ()
    {
    for (int i=0; i<INC_SHARED_INT_TIMES; ++i)
        {
        if (0 == (i % 1000))
            std::this_thread::yield();
        s_sharedInt++;
        }
    }

static void decrementSharedInt ()
    {
    for (int i=0; i<INC_SHARED_INT_TIMES; ++i)
        {
        if (0 == (i % 1000))
            std::this_thread::yield();
        s_sharedInt--;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeThreadUtilities, IncrementSharedInt)
    {
    std::thread t1 = std::thread (incrementSharedInt);
    std::thread t2 = std::thread (incrementSharedInt);

    t1.join();
    t2.join();

    ASSERT_TRUE( s_sharedInt.load() == 2*INC_SHARED_INT_TIMES );

    std::thread t3 = std::thread (decrementSharedInt);
    std::thread t4 = std::thread (decrementSharedInt);

    t3.join();
    t4.join();

    ASSERT_TRUE( s_sharedInt.load() == 0 );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void destroyTlsEntry(void* value)
    {
    auto pInt = reinterpret_cast<BeAtomic<int>*>(value);
    pInt->IncrementAtomicPre();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeThreadLocalStorage, Destructor)
    {
    BeThreadLocalStorage tls(destroyTlsEntry);

    // Using atomic instead of int to prevent compiler from eliding read at end of test.
    BeAtomic<int> v0(0);
    tls.SetValueAsPointer(&v0);

    BeAtomic<int> v1(1);
    std::thread t1([&]() { tls.SetValueAsPointer(&v1); });

    BeAtomic<int> v2(2);
    std::thread t2([&]() { tls.SetValueAsPointer(&v2); });

    t1.join();
    t2.join();

    EXPECT_EQ(v0, 0);
    EXPECT_EQ(v1, 2);
    EXPECT_EQ(v2, 3);
    }
