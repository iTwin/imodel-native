/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentation/ECPresentationManager.h>
#include "../../Helpers/TestHelpers.h"
#include "../../Helpers/ECDbTestProject.h"

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct OptimizedExpressionsEvaluationTests : ECPresentationTest
{
    static ECDbTestProject* s_project;
    TestConnectionManager m_connections;
    IConnectionPtr m_connection;
    BeMutex m_mutex;

    ECClassCP m_widgetClass;
    ECClassCP m_gadgetClass;

    static void SetUpTestCase()
        {
        s_project = new ECDbTestProject();
        s_project->Create("ContentProviderTests", "RulesEngineTest.01.00.ecschema.xml");
        }

    static void TearDownTestCase()
        {
        DELETE_AND_CLEAR(s_project);
        }

    void SetUp()
        {
        ECPresentationTest::SetUp();
        m_widgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget");
        m_gadgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Gadget");
        m_connection = m_connections.NotifyConnectionOpened(s_project->GetECDb());
        }
};
ECDbTestProject* OptimizedExpressionsEvaluationTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(OptimizedExpressionsEvaluationTests, EvaluateClassNameOptimizedExpressionWithECPropertyGroupingNodeKey)
    {
    OptimizedExpressionPtr expression = ClassNameOptimizedExpression::Create("Widget", m_mutex);

    NavNodeKeyPtr widgetKey = ECPropertyGroupingNodeKey::Create(*m_widgetClass, "W", rapidjson::Value(rapidjson::kArrayType), "", {"1"}, 1);
    NavNodeKeyPtr gadgetKey = ECPropertyGroupingNodeKey::Create(*m_gadgetClass, "G", rapidjson::Value(rapidjson::kArrayType), "", {"2"}, 1);

    OptimizedExpressionsParameters widgetParams(m_connections, *m_connection, widgetKey.get(), "");
    OptimizedExpressionsParameters gadgetParams(m_connections, *m_connection, gadgetKey.get(), "");
    EXPECT_TRUE(expression->Value(widgetParams));
    EXPECT_FALSE(expression->Value(gadgetParams));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(OptimizedExpressionsEvaluationTests, EvaluateIsOfClassOptimizedExpressionWithECPropertyGroupingNodeKey)
    {
    OptimizedExpressionPtr expression = IsOfClassOptimizedExpression::Create("RulesEngineTest", "Widget", m_mutex);

    NavNodeKeyPtr widgetKey = ECPropertyGroupingNodeKey::Create(*m_widgetClass, "W", rapidjson::Value(rapidjson::kArrayType), "", {"1"}, 1);
    NavNodeKeyPtr gadgetKey = ECPropertyGroupingNodeKey::Create(*m_gadgetClass, "G", rapidjson::Value(rapidjson::kArrayType), "", {"2"}, 1);

    OptimizedExpressionsParameters widgetParams(m_connections, *m_connection, widgetKey.get(), "");
    OptimizedExpressionsParameters gadgetParams(m_connections, *m_connection, gadgetKey.get(), "");
    EXPECT_TRUE(expression->Value(widgetParams));
    EXPECT_FALSE(expression->Value(gadgetParams));
    }
