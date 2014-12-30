/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/BeThread_Tests.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/BeThread.h>
#include <thread>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Julija.Suboc    09/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeConditionVariable, CreateByPassingNullToConstructor)
    {
    BeConditionVariable  conditionVar(NULL);
    EXPECT_TRUE(conditionVar.GetIsValid())<<"Failed to create BeConditionVariable";
    BeCriticalSection section = conditionVar.GetCriticalSection();
    EXPECT_TRUE(section.GetIsValid())<<"Failed to create BeCriticalSection for BeConditionVariable";
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Julija.Suboc    09/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeConditionVariable, CreateByPassingBeCriticalSection)
    {
    BeCriticalSection section;
    ASSERT_TRUE(section.GetIsValid())<<"Failed to create BeCriticalSection";
    BeConditionVariable  conditionVar(&section);
    ASSERT_TRUE(conditionVar.GetIsValid())<<"Failed to create BeConditionVariable";
    BeCriticalSection sectionReturned = conditionVar.GetCriticalSection();
    EXPECT_TRUE(sectionReturned.GetIsValid())<<"BeConditionVariable returned invalid BeCriticalSection.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/13
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
* @bsimethod                                                    Sam.Wilson      12/13
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