/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "QueryExecutorHelperTests.h"

std::unique_ptr<ECDbTestProject> QueryExecutorHelperTests::s_project;
GenericQueryPtr QueryExecutorHelperTests::s_query;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryExecutorHelperTests::SetUpTestCase()
    {
    s_project = std::make_unique<ECDbTestProject>();
    s_project->Create("QueryExecutorHelperTests", "RulesEngineTest.01.00.ecschema.xml");
    s_query = StringGenericQuery::Create("SELECT ECInstanceId FROM [RET].[Widget]", {});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryExecutorHelperTests::TearDownTestCase()
    {
    s_query = nullptr;
    s_project.reset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryExecutorHelperTests::SetUp()
    {
    ECPresentationTest::SetUp();
    m_connection = TestConnectionManager().NotifyConnectionOpened(s_project->GetECDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryExecutorHelperTests::TearDown()
    {
    s_project->GetECDb().AbandonChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryExecutorHelperTests::InsertInstances(uint32_t numInstances)
    {
    ECClassCP widgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget");
    for (uint32_t i=0; i<numInstances; ++i)
        RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *widgetClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryExecutorHelperTests, ExecuteQuery_NoResults_ReturnsDone)
    {
    TestQueryResultAccumulator accumulator(QueryResultAccumulatorStatus::Continue);
    QueryExecutorStatus result = QueryExecutorHelper::ExecuteQuery(*m_connection, *s_query, accumulator, TestCancelationToken([] { return false; }));

    EXPECT_EQ(QueryExecutorStatus::Done, result);
    EXPECT_EQ(0, accumulator.GetNumCalls());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryExecutorHelperTests, ExecuteQuery_MultipleResults_ReturnsDone)
    {
    InsertInstances(2);

    TestQueryResultAccumulator accumulator(QueryResultAccumulatorStatus::Continue);
    QueryExecutorStatus result = QueryExecutorHelper::ExecuteQuery(*m_connection, *s_query, accumulator, TestCancelationToken([] { return false; }));

    EXPECT_EQ(QueryExecutorStatus::Done, result);
    EXPECT_EQ(2, accumulator.GetNumCalls());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryExecutorHelperTests, ExecuteQuery_AccumulatorStops_StopsReadingAndReturnsDone)
    {
    InsertInstances(2);

    TestQueryResultAccumulator accumulator([] (int numCalls)
        {
        return numCalls == 1 ? QueryResultAccumulatorStatus::Stop : QueryResultAccumulatorStatus::Continue;
        });
    QueryExecutorStatus result = QueryExecutorHelper::ExecuteQuery(*m_connection, *s_query, accumulator, TestCancelationToken([] { return false; }));

    EXPECT_EQ(QueryExecutorStatus::Done, result);
    EXPECT_EQ(1, accumulator.GetNumCalls());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryExecutorHelperTests, ExecuteQuery_CancellationTriggered_StopsReadingAndReturnsCancelled)
    {
    InsertInstances(2);

    bool wasCancelled = false;
    TestQueryResultAccumulator accumulator(QueryResultAccumulatorStatus::Continue);
    try
        {
        QueryExecutorHelper::ExecuteQuery(*m_connection, *s_query, accumulator,
            TestCancelationToken([&accumulator]{return accumulator.GetNumCalls() == 1;}));
        }
    catch (CancellationException const&)
        {
        wasCancelled = true;
        }
    EXPECT_TRUE(wasCancelled);
    EXPECT_EQ(1, accumulator.GetNumCalls());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryExecutorHelperTests, ExecuteQuery_CancellationTriggeredImmediately_DoesNotForwardResultRowToAccumulator)
    {
    InsertInstances(2);

    TestQueryResultAccumulator accumulator(QueryResultAccumulatorStatus::Continue);
    bool wasCancelled = false;
    try
        {
        QueryExecutorHelper::ExecuteQuery(*m_connection, *s_query, accumulator, TestCancelationToken([]{return true;}));
        }
    catch (CancellationException const&)
        {
        wasCancelled = true;
        }
    EXPECT_TRUE(wasCancelled);
    EXPECT_EQ(0, accumulator.GetNumCalls());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryExecutorHelperTests, ExecuteQuery_ThrowsWhenStatementFailsToPrepare)
    {
    // create an invalid query
    GenericQueryPtr query = StringGenericQuery::Create("SELECT a FROM b", BoundQueryValuesList());

    // create an accumulator (we don't expect it to be called)
    TestQueryResultAccumulator accumulator(QueryResultAccumulatorStatus::Continue);

    // test
    try
        {
        QueryExecutorHelper::ExecuteQuery(*m_connection, *query, accumulator, TestCancelationToken([] { return false; }));
        FAIL();
        }
    catch (InternalError const&)
        {
        SUCCEED();
        }
    }
