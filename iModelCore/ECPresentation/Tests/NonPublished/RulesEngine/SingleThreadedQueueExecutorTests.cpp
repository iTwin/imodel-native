/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/SingleThreadedQueueExecutorTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include "../../../Source/RulesDriven/RulesEngine/SingleThreadQueueExecutor.h"
#include "TestHelpers.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION

#define DEBUG_MULTITHREADING 0
#if (DEBUG_MULTITHREADING)
    #define WAIT_TIMEOUT BeConditionVariable::Infinite
#else
    #define WAIT_TIMEOUT 500
#endif

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct SingleThreadQueueExecutorTests : ECPresentationTest
    {
    BeConditionVariable m_cv;
    SingleThreadedQueueExecutor m_executor;
    SingleThreadQueueExecutorTests() : m_executor("SingleThreadQueueExecutorTests-Executor") {}
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct InlineConditionVariablePredicate : IConditionVariablePredicate
    {
    std::function<bool()> m_testFunc;
    InlineConditionVariablePredicate(std::function<bool()> testFunc) : m_testFunc(testFunc) {}
    bool _TestCondition(BeConditionVariable &cv) override {return m_testFunc();}
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleThreadQueueExecutorTests, ExecutesQueuedFunctions)
    {
    int executesCount = 0;
    for (int i = 0; i < 5; ++i)
        {
        m_executor.add([&]()
            {
            BeMutexHolder lock(m_cv.GetMutex());
            executesCount++;
            m_cv.notify_all();
            });
        InlineConditionVariablePredicate pred([&](){return executesCount == (i + 1);});
        m_cv.WaitOnCondition(&pred, WAIT_TIMEOUT);
        }
    EXPECT_EQ(5, executesCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleThreadQueueExecutorTests, ExecutesFunctionsInlineIfAlreadyOnTargetThread)
    {
    bvector<int> executionSequence;
    m_executor.add([&]()
        {
        BeMutexHolder lock(m_cv.GetMutex());
        executionSequence.push_back(1);
        m_executor.add([&]()
            {
            BeMutexHolder lock(m_cv.GetMutex());
            executionSequence.push_back(2);
            m_cv.notify_all();
            });
        executionSequence.push_back(3);
        m_cv.notify_all();
        });
    

    InlineConditionVariablePredicate pred([&](){return executionSequence.size() == 3;});
    EXPECT_TRUE(m_cv.WaitOnCondition(&pred, WAIT_TIMEOUT));

    ASSERT_EQ(3, executionSequence.size());
    for (int i = 0; i < 3; ++i)
        EXPECT_EQ(i + 1, executionSequence[i]);
    }