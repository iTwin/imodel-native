/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/OptimizedExpressionsEvaluationTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "TestHelpers.h"
#include "ECDbTestProject.h"

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                01/2018
+===============+===============+===============+===============+===============+======*/
struct OptimizedExpressionsEvaluationTests : ECPresentationTest
{
    static ECDbTestProject* s_project;
    TestConnectionManager m_connections;
    IConnectionPtr m_connection;

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
* @bsitest                                      Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(OptimizedExpressionsEvaluationTests, EvaluateClassNameOptimizedExpressionWithECPropertyGroupingNodeKey)
    {
    OptimizedExpressionPtr expression = ClassNameOptimizedExpression::Create("Widget");

    NavNodeKeyPtr widgetKey = ECPropertyGroupingNodeKey::Create(*m_widgetClass, "W", nullptr, {"1"}, 1);
    NavNodeKeyPtr gadgetKey = ECPropertyGroupingNodeKey::Create(*m_gadgetClass, "G", nullptr, {"2"}, 1);

    OptimizedExpressionsParameters widgetParams(m_connections, *m_connection, widgetKey.get(), "");
    OptimizedExpressionsParameters gadgetParams(m_connections, *m_connection, gadgetKey.get(), "");
    EXPECT_TRUE(expression->Value(widgetParams));
    EXPECT_FALSE(expression->Value(gadgetParams));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(OptimizedExpressionsEvaluationTests, EvaluateIsOfClassOptimizedExpressionWithECPropertyGroupingNodeKey)
    {
    OptimizedExpressionPtr expression = IsOfClassOptimizedExpression::Create("RulesEngineTest", "Widget");

    NavNodeKeyPtr widgetKey = ECPropertyGroupingNodeKey::Create(*m_widgetClass, "W", nullptr, {"1"}, 1);
    NavNodeKeyPtr gadgetKey = ECPropertyGroupingNodeKey::Create(*m_gadgetClass, "G", nullptr, {"2"}, 1);

    OptimizedExpressionsParameters widgetParams(m_connections, *m_connection, widgetKey.get(), "");
    OptimizedExpressionsParameters gadgetParams(m_connections, *m_connection, gadgetKey.get(), "");
    EXPECT_TRUE(expression->Value(widgetParams));
    EXPECT_FALSE(expression->Value(gadgetParams));
    }
