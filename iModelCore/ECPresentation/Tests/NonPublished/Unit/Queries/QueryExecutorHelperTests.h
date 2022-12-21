/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "../../Helpers/TestHelpers.h"

USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct QueryExecutorHelperTests : ECPresentationTest
    {
    protected:
        static std::unique_ptr<ECDbTestProject> s_project;
        static std::unique_ptr<PresentationQuery> s_query;

        IConnectionPtr m_connection;

    protected:
        static void SetUpTestCase();
        static void TearDownTestCase();

        void SetUp() override;
        void TearDown() override;

        void InsertInstances(uint32_t numInstances);
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TestQueryResultAccumulator final : IQueryResultAccumulator
    {
    private:
        std::function<QueryResultAccumulatorStatus(int)> m_rowReader;
        int m_numCalls;

        QueryResultAccumulatorStatus _ReadRow(ECSqlStatementCR statement) override
            {
            ++m_numCalls;
            return m_rowReader(m_numCalls);
            }

    public:
        TestQueryResultAccumulator(std::function<QueryResultAccumulatorStatus(int)> rowReader)
            : m_rowReader(rowReader), m_numCalls(0)
            {}
        TestQueryResultAccumulator(QueryResultAccumulatorStatus status)
            : TestQueryResultAccumulator([status] (int) { return status; })
            {}

        int GetNumCalls() const { return m_numCalls; }
    };
