/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../../Helpers/TestHelpers.h"
#include "QueryExecutorTests.h"

ECDbTestProject* QueryExecutorTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryExecutorTests::SetUpTestCase()
    {
    s_project = new ECDbTestProject();
    s_project->Create("QueryExecutorTests", "RulesEngineTest.01.00.ecschema.xml");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryExecutorTests::TearDownTestCase()
    {
    DELETE_AND_CLEAR(s_project);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryExecutorTests, ReadNext_WithResultAsReturnValue_ReturnsValuesFromReaderWhenItReturnsRow)
    {
    // insert a couple of widgets
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create a query to select all widgets
    PresentationQuery query("SELECT ECInstanceId FROM [RET].[Widget]", BoundQueryValuesList());

    // set up query reader
    GenericQueryResultReader<Utf8String> reader([](ECSqlStatementCR stmt)
        {
        return stmt.GetValueText(0);
        });

    // test
    QueryExecutor executor(*m_connection, query);
    EXPECT_FALSE(executor.IsReadStarted());
    EXPECT_FALSE(executor.IsReadFinished());

    EXPECT_STREQ(widget1->GetInstanceId().c_str(), executor.ReadNext(reader).c_str());
    EXPECT_TRUE(executor.IsReadStarted());
    EXPECT_FALSE(executor.IsReadFinished());

    EXPECT_STREQ(widget2->GetInstanceId().c_str(), executor.ReadNext(reader).c_str());
    EXPECT_TRUE(executor.IsReadStarted());
    EXPECT_FALSE(executor.IsReadFinished());

    EXPECT_STREQ("", executor.ReadNext(reader).c_str());
    EXPECT_TRUE(executor.IsReadStarted());
    EXPECT_TRUE(executor.IsReadFinished());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryExecutorTests, ReadNext_WithResultAsReturnValue_ReturnsValuesFromReaderWhenItReturnsRowAndRepeat)
    {
    // insert a widget
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create a query to select all widgets
    PresentationQuery query("SELECT ECInstanceId FROM [RET].[Widget]", BoundQueryValuesList());

    // set up query reader
    int iteration = 0;
    GenericQueryResultReader<int> reader([&iteration](int& result, ECSqlStatementCR stmt)
        {
        result = iteration;
        return (0 == iteration++) ? QueryResultReaderStatus::RowAndRepeat : QueryResultReaderStatus::Row;
        });

    // test
    QueryExecutor executor(*m_connection, query);
    EXPECT_FALSE(executor.IsReadStarted());
    EXPECT_FALSE(executor.IsReadFinished());

    EXPECT_EQ(0, executor.ReadNext(reader));
    EXPECT_TRUE(executor.IsReadStarted());
    EXPECT_FALSE(executor.IsReadFinished());

    EXPECT_EQ(1, executor.ReadNext(reader));
    EXPECT_TRUE(executor.IsReadStarted());
    EXPECT_FALSE(executor.IsReadFinished());

    EXPECT_EQ(0, executor.ReadNext(reader));
    EXPECT_TRUE(executor.IsReadStarted());
    EXPECT_TRUE(executor.IsReadFinished());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryExecutorTests, ReadNext_WithResultAsReturnValue_SkipsValuesUponReaderRequest)
    {
    // insert a couple of widgets
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create a query to select all widgets
    PresentationQuery query("SELECT ECInstanceId FROM [RET].[Widget]", BoundQueryValuesList());

    // set up query reader
    int iteration = 0;
    GenericQueryResultReader<Utf8String> reader([&iteration](Utf8StringR result, ECSqlStatementCR stmt)
        {
        if (0 == iteration++)
            {
            result = "whatever";
            return QueryResultReaderStatus::Skip;
            }
        result = stmt.GetValueText(0);
        return QueryResultReaderStatus::Row;
        });

    // test
    QueryExecutor executor(*m_connection, query);
    EXPECT_FALSE(executor.IsReadStarted());
    EXPECT_FALSE(executor.IsReadFinished());

    EXPECT_STREQ(widget2->GetInstanceId().c_str(), executor.ReadNext(reader).c_str());
    EXPECT_TRUE(executor.IsReadStarted());
    EXPECT_FALSE(executor.IsReadFinished());

    EXPECT_STREQ("", executor.ReadNext(reader).c_str());
    EXPECT_TRUE(executor.IsReadStarted());
    EXPECT_TRUE(executor.IsReadFinished());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryExecutorTests, ReadNext_WithResultAsReturnValue_StopsExecutingUponReaderRequest)
    {
    // insert a couple of widgets
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create a query to select all widgets
    PresentationQuery query("SELECT ECInstanceId FROM [RET].[Widget]", BoundQueryValuesList());

    // set up query reader
    GenericQueryResultReader<Utf8String> reader([](Utf8StringR result, ECSqlStatementCR stmt)
        {
        result = stmt.GetValueText(0);
        return QueryResultReaderStatus::Stop;
        });

    // test
    QueryExecutor executor(*m_connection, query);
    EXPECT_FALSE(executor.IsReadStarted());
    EXPECT_FALSE(executor.IsReadFinished());

    EXPECT_STREQ("", executor.ReadNext(reader).c_str());
    EXPECT_TRUE(executor.IsReadStarted());
    EXPECT_TRUE(executor.IsReadFinished());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryExecutorTests, ReadNext_WithStatusAsReturnValue_ReturnsValuesFromReaderWhenItReturnsRow)
    {
    // insert a couple of widgets
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create a query to select all widgets
    PresentationQuery query("SELECT ECInstanceId FROM [RET].[Widget]", BoundQueryValuesList());

    // set up query reader
    GenericQueryResultReader<Utf8String> reader([](ECSqlStatementCR stmt)
        {
        return stmt.GetValueText(0);
        });

    // test
    Utf8String value;
    QueryExecutor executor(*m_connection, query);
    EXPECT_FALSE(executor.IsReadStarted());
    EXPECT_FALSE(executor.IsReadFinished());

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(value, reader));
    EXPECT_STREQ(widget1->GetInstanceId().c_str(), value.c_str());
    EXPECT_TRUE(executor.IsReadStarted());
    EXPECT_FALSE(executor.IsReadFinished());

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(value, reader));
    EXPECT_STREQ(widget2->GetInstanceId().c_str(), value.c_str());
    EXPECT_TRUE(executor.IsReadStarted());
    EXPECT_FALSE(executor.IsReadFinished());

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(value, reader));
    EXPECT_STREQ("", value.c_str());
    EXPECT_TRUE(executor.IsReadStarted());
    EXPECT_TRUE(executor.IsReadFinished());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryExecutorTests, ReadNext_WithStatusAsReturnValue_ReturnsValuesFromReaderWhenItReturnsRowAndRepeat)
    {
    // insert a widget
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create a query to select all widgets
    PresentationQuery query("SELECT ECInstanceId FROM [RET].[Widget]", BoundQueryValuesList());

    // set up query reader
    int iteration = 0;
    GenericQueryResultReader<int> reader([&iteration](int& result, ECSqlStatementCR stmt)
        {
        result = iteration;
        return (0 == iteration++) ? QueryResultReaderStatus::RowAndRepeat : QueryResultReaderStatus::Row;
        });

    // test
    int value;
    QueryExecutor executor(*m_connection, query);
    EXPECT_FALSE(executor.IsReadStarted());
    EXPECT_FALSE(executor.IsReadFinished());

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(value, reader));
    EXPECT_EQ(0, value);
    EXPECT_TRUE(executor.IsReadStarted());
    EXPECT_FALSE(executor.IsReadFinished());

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(value, reader));
    EXPECT_EQ(1, value);
    EXPECT_TRUE(executor.IsReadStarted());
    EXPECT_FALSE(executor.IsReadFinished());

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(value, reader));
    EXPECT_EQ(0, value);
    EXPECT_TRUE(executor.IsReadStarted());
    EXPECT_TRUE(executor.IsReadFinished());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryExecutorTests, ReadNext_WithStatusAsReturnValue_SkipsValuesUponReaderRequest)
    {
    // insert a couple of widgets
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create a query to select all widgets
    PresentationQuery query("SELECT ECInstanceId FROM [RET].[Widget]", BoundQueryValuesList());

    // set up query reader
    int iteration = 0;
    GenericQueryResultReader<Utf8String> reader([&iteration](Utf8StringR result, ECSqlStatementCR stmt)
        {
        if (0 == iteration++)
            {
            result = "whatever";
            return QueryResultReaderStatus::Skip;
            }
        result = stmt.GetValueText(0);
        return QueryResultReaderStatus::Row;
        });

    // test
    Utf8String value;
    QueryExecutor executor(*m_connection, query);
    EXPECT_FALSE(executor.IsReadStarted());
    EXPECT_FALSE(executor.IsReadFinished());

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(value, reader));
    EXPECT_STREQ(widget2->GetInstanceId().c_str(), value.c_str());
    EXPECT_TRUE(executor.IsReadStarted());
    EXPECT_FALSE(executor.IsReadFinished());

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(value, reader));
    EXPECT_STREQ("", value.c_str());
    EXPECT_TRUE(executor.IsReadStarted());
    EXPECT_TRUE(executor.IsReadFinished());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryExecutorTests, ReadNext_WithStatusAsReturnValue_StopsExecutingUponReaderRequest)
    {
    // insert a couple of widgets
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create a query to select all widgets
    PresentationQuery query("SELECT ECInstanceId FROM [RET].[Widget]", BoundQueryValuesList());

    // set up query reader
    GenericQueryResultReader<Utf8String> reader([](Utf8StringR result, ECSqlStatementCR stmt)
        {
        result = stmt.GetValueText(0);
        return QueryResultReaderStatus::Stop;
        });

    // test
    Utf8String value;
    QueryExecutor executor(*m_connection, query);
    EXPECT_FALSE(executor.IsReadStarted());
    EXPECT_FALSE(executor.IsReadFinished());

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(value, reader));
    EXPECT_TRUE(executor.IsReadStarted());
    EXPECT_TRUE(executor.IsReadFinished());
    }
