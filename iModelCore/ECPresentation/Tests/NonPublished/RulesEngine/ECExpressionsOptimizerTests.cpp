/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "../../../Source/RulesDriven/RulesEngine/ECExpressionContextsProvider.h"
#include "../../../Source/RulesDriven/RulesEngine/CustomFunctions.h"
#include "TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2017
+===============+===============+===============+===============+===============+======*/
struct ECExpressionsOptimizerTests : ECPresentationTest
    {
    ECExpressionsCache m_expressionsCache;
    ECExpressionOptimizer m_optimizer;

    ECExpressionsOptimizerTests()
        : m_optimizer(m_expressionsCache)
        {}
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsOptimizerTests, CachesValidOptimizedExpressions)
    {
    Utf8CP expression = "SelectedNode.IsInstanceNode";
    OptimizedExpressionPtr optimizedExpression = m_optimizer.GetOptimizedExpression(expression);
    ASSERT_TRUE(optimizedExpression.IsValid());

    OptimizedExpressionPtr cachedOptimizedExpression;
    EXPECT_EQ(SUCCESS, m_expressionsCache.Get(cachedOptimizedExpression, expression));
    EXPECT_EQ(optimizedExpression, cachedOptimizedExpression);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsOptimizerTests, CachesInvalidOptimizedExpressions)
    {
    Utf8CP expression = "Something.Invalid";
    OptimizedExpressionPtr optimizedExpression = m_optimizer.GetOptimizedExpression(expression);
    ASSERT_TRUE(optimizedExpression.IsNull());

    OptimizedExpressionPtr cachedOptimizedExpression;
    EXPECT_EQ(SUCCESS, m_expressionsCache.Get(cachedOptimizedExpression, expression));
    EXPECT_TRUE(cachedOptimizedExpression.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsOptimizerTests, OptimizeContentDisplayTypeExpression)
    {
    Utf8CP expression = "ContentDisplayType <> \"PropertyPane\"";
    OptimizedExpressionPtr optimizedExpression = m_optimizer.GetOptimizedExpression(expression);
    ASSERT_TRUE(optimizedExpression.IsValid());

    OptimizedExpressionPtr expected = DisplayTypeOptimizedExpression::Create("PropertyPane", false);
    EXPECT_TRUE(optimizedExpression->IsEqual(*expected));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsOptimizerTests, OptimizeIsOfClassExpression)
    {
    Utf8CP expression = "SelectedNode.ECInstance.IsOfClass(\"ClassName\", \"SchemaName\")";
    OptimizedExpressionPtr optimizedExpression = m_optimizer.GetOptimizedExpression(expression);
    ASSERT_TRUE(optimizedExpression.IsValid());

    OptimizedExpressionPtr expected = IsOfClassOptimizedExpression::Create("SchemaName", "ClassName", m_expressionsCache.GetMutex());
    EXPECT_TRUE(optimizedExpression->IsEqual(*expected));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsOptimizerTests, OptimizeIsInstanceNodeExpression)
    {
    Utf8CP expression = "SelectedNode.IsInstanceNode";
    OptimizedExpressionPtr optimizedExpression = m_optimizer.GetOptimizedExpression(expression);
    ASSERT_TRUE(optimizedExpression.IsValid());

    OptimizedExpressionPtr expected = IsInstanceNodeOptimizedExpression::Create();
    EXPECT_TRUE(optimizedExpression->IsEqual(*expected));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsOptimizerTests, OptimizeIsPropertyGroupingNodeExpression)
    {
    Utf8CP expression = "SelectedNode.IsPropertyGroupingNode";
    OptimizedExpressionPtr optimizedExpression = m_optimizer.GetOptimizedExpression(expression);
    ASSERT_TRUE(optimizedExpression.IsValid());

    OptimizedExpressionPtr expected = IsPropertyGroupingOptimizedExpression::Create();
    EXPECT_TRUE(optimizedExpression->IsEqual(*expected));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsOptimizerTests, OptimizeIsECClassGroupingNodeExpression)
    {
    Utf8CP expression = "SelectedNode.IsClassGroupingNode";
    OptimizedExpressionPtr optimizedExpression = m_optimizer.GetOptimizedExpression(expression);
    ASSERT_TRUE(optimizedExpression.IsValid());

    OptimizedExpressionPtr expected = IsECClassGroupingOptimizedExpression::Create();
    EXPECT_TRUE(optimizedExpression->IsEqual(*expected));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsOptimizerTests, OptimizeClassNameEqualsExpression)
    {
    Utf8CP expression = "SelectedNode.ClassName = \"ClassName\"";
    OptimizedExpressionPtr optimizedExpression = m_optimizer.GetOptimizedExpression(expression);
    ASSERT_TRUE(optimizedExpression.IsValid());

    OptimizedExpressionPtr expected = ClassNameOptimizedExpression::Create("ClassName", m_expressionsCache.GetMutex());
    EXPECT_TRUE(optimizedExpression->IsEqual(*expected));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsOptimizerTests, GetInvalidOptimizedExpressionECExpressionIsUnknown)
    {
    Utf8CP expression = "SelectedNode.Label = \"Label\"";
    OptimizedExpressionPtr optimizedExpression = m_optimizer.GetOptimizedExpression(expression);
    ASSERT_FALSE(optimizedExpression.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsOptimizerTests, GetInvalidOptimizedExpressionIfConditionHasAtleastOneUnknownECExpression)
    {
    Utf8CP expression = "(ContentDisplayType <> \"PropertyPane\" OR SelectedNode.ECInstance.IsOfClass(\"ClassName\", \"SchemaName\")) ANDALSO SelectedNode.Label = \"Label\"";
    OptimizedExpressionPtr optimizedExpression = m_optimizer.GetOptimizedExpression(expression);
    ASSERT_FALSE(optimizedExpression.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsOptimizerTests, OptimizeLogicalAndExpression)
    {
    Utf8CP expression = "ContentDisplayType <> \"PropertyPane\" ANDALSO SelectedNode.ECInstance.IsOfClass(\"ClassName\", \"SchemaName\")";
    OptimizedExpressionPtr optimizedExpression = m_optimizer.GetOptimizedExpression(expression);
    ASSERT_TRUE(optimizedExpression.IsValid());

    OptimizedExpressionPtr expected = LogicalOptimizedExpression::Create(DisplayTypeOptimizedExpression::Create("PropertyPane", false),
        IsOfClassOptimizedExpression::Create("SchemaName", "ClassName", m_expressionsCache.GetMutex()), true);
    EXPECT_TRUE(optimizedExpression->IsEqual(*expected));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsOptimizerTests, OptimizeLogicalORExpression)
    {
    Utf8CP expression = "ContentDisplayType <> \"PropertyPane\" OR SelectedNode.ECInstance.IsOfClass(\"ClassName\", \"SchemaName\")";
    OptimizedExpressionPtr optimizedExpression = m_optimizer.GetOptimizedExpression(expression);
    ASSERT_TRUE(optimizedExpression.IsValid());

    OptimizedExpressionPtr expected = LogicalOptimizedExpression::Create(DisplayTypeOptimizedExpression::Create("PropertyPane", false),
        IsOfClassOptimizedExpression::Create("SchemaName", "ClassName", m_expressionsCache.GetMutex()), false);
    EXPECT_TRUE(optimizedExpression->IsEqual(*expected));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsOptimizerTests, OptimizeLogicalExpressionWithParenthesesAtTheBack)
    {
    Utf8CP expression = "ContentDisplayType <> \"PropertyPane\" ANDALSO (SelectedNode.ECInstance.IsOfClass(\"Class\", \"Schema\")"
        " OR SelectedNode.ECInstance.IsOfClass(\"OtherClass\", \"OtherSchema\"))";
    OptimizedExpressionPtr optimizedExpression = m_optimizer.GetOptimizedExpression(expression);
    ASSERT_TRUE(optimizedExpression.IsValid());

    OptimizedExpressionPtr rightSide = LogicalOptimizedExpression::Create(IsOfClassOptimizedExpression::Create("Schema", "Class", m_expressionsCache.GetMutex()),
        IsOfClassOptimizedExpression::Create("OtherSchema", "OtherClass", m_expressionsCache.GetMutex()), false);
    OptimizedExpressionPtr leftSide = DisplayTypeOptimizedExpression::Create("PropertyPane", false);
    OptimizedExpressionPtr expected = LogicalOptimizedExpression::Create(leftSide, rightSide, true);
    EXPECT_TRUE(optimizedExpression->IsEqual(*expected));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsOptimizerTests, OptimizeLogicalExpressionWithParenthesesAtTheFront)
    {
    Utf8CP expression = "(ContentDisplayType <> \"PropertyPane\" ANDALSO SelectedNode.ECInstance.IsOfClass(\"Class\", \"Schema\"))"
        " OR SelectedNode.ECInstance.IsOfClass(\"OtherClass\", \"OtherSchema\")";
    OptimizedExpressionPtr optimizedExpression = m_optimizer.GetOptimizedExpression(expression);
    ASSERT_TRUE(optimizedExpression.IsValid());

    OptimizedExpressionPtr leftSide = LogicalOptimizedExpression::Create(DisplayTypeOptimizedExpression::Create("PropertyPane", false),
        IsOfClassOptimizedExpression::Create("Schema", "Class", m_expressionsCache.GetMutex()), true);
    OptimizedExpressionPtr rightSide = IsOfClassOptimizedExpression::Create("OtherSchema", "OtherClass", m_expressionsCache.GetMutex());
    OptimizedExpressionPtr expected = LogicalOptimizedExpression::Create(leftSide, rightSide, false);
    EXPECT_TRUE(optimizedExpression->IsEqual(*expected));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsOptimizerTests, OptimizeLogicalExpressionWithNestedParentheses)
    {
    Utf8CP expression = "((ContentDisplayType <> \"PropertyPane\" OR ContentDisplayType = \"OtherPane\") ANDALSO SelectedNode.ECInstance.IsOfClass(\"Class\", \"Schema\"))"
        " OR SelectedNode.ECInstance.IsOfClass(\"OtherClass\", \"OtherSchema\")";
    OptimizedExpressionPtr optimizedExpression = m_optimizer.GetOptimizedExpression(expression);
    ASSERT_TRUE(optimizedExpression.IsValid());

    OptimizedExpressionPtr expected = LogicalOptimizedExpression::Create(DisplayTypeOptimizedExpression::Create("PropertyPane", false),
        DisplayTypeOptimizedExpression::Create("OtherPane", true), false);
    expected = LogicalOptimizedExpression::Create(expected, IsOfClassOptimizedExpression::Create("Schema", "Class", m_expressionsCache.GetMutex()), true);
    expected = LogicalOptimizedExpression::Create(expected, IsOfClassOptimizedExpression::Create("OtherSchema", "OtherClass", m_expressionsCache.GetMutex()), false);
    EXPECT_TRUE(optimizedExpression->IsEqual(*expected));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsOptimizerTests, OptimizeLogicalExpressionWithSeveralOperators)
    {
    Utf8CP expression = "ContentDisplayType <> \"PropertyPane\" OR ContentDisplayType = \"OtherPane\" ANDALSO SelectedNode.ECInstance.IsOfClass(\"Class\", \"Schema\")";
    OptimizedExpressionPtr optimizedExpression = m_optimizer.GetOptimizedExpression(expression);
    ASSERT_TRUE(optimizedExpression.IsValid());

    OptimizedExpressionPtr expected = LogicalOptimizedExpression::Create(DisplayTypeOptimizedExpression::Create("PropertyPane", false),
        DisplayTypeOptimizedExpression::Create("OtherPane", true), false);
    expected = LogicalOptimizedExpression::Create(expected, IsOfClassOptimizedExpression::Create("Schema", "Class", m_expressionsCache.GetMutex()), true);
    EXPECT_TRUE(optimizedExpression->IsEqual(*expected));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsOptimizerTests, OptimizeInstanceIdExpression)
    {
    Utf8CP expression = "ThisNode.InstanceId = \"16\"";
    OptimizedExpressionPtr optimizedExpression = m_optimizer.GetOptimizedExpression(expression);
    ASSERT_TRUE(optimizedExpression.IsValid());

    OptimizedExpressionPtr expected = InstanceIdOptimizedExpression::Create(BeInt64Id(16));
    EXPECT_TRUE(optimizedExpression->IsEqual(*expected));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsOptimizerTests, OptimizeThisNodeIsInstanceNodeExpression)
    {
    Utf8CP expression = "ThisNode.IsInstanceNode";
    OptimizedExpressionPtr optimizedExpression = m_optimizer.GetOptimizedExpression(expression);
    ASSERT_TRUE(optimizedExpression.IsValid());

    OptimizedExpressionPtr expected = IsInstanceNodeOptimizedExpression::Create();
    EXPECT_TRUE(optimizedExpression->IsEqual(*expected));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsOptimizerTests, OptimizeThisIsOfClassExpression)
    {
    Utf8CP expression = "this.IsOfClass(\"DefinitionPartition\", \"BisCore\")";
    OptimizedExpressionPtr optimizedExpression = m_optimizer.GetOptimizedExpression(expression);
    ASSERT_TRUE(optimizedExpression.IsValid());

    OptimizedExpressionPtr expected = IsOfClassOptimizedExpression::Create("BisCore", "DefinitionPartition", m_expressionsCache.GetMutex());
    EXPECT_TRUE(optimizedExpression->IsEqual(*expected));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsOptimizerTests, OptimizeParentIsOfClassExpression)
    {
    Utf8CP expression = "ParentNode.ECInstance.IsOfClass(\"Subject\", \"BisCore\")";
    OptimizedExpressionPtr optimizedExpression = m_optimizer.GetOptimizedExpression(expression);
    ASSERT_TRUE(optimizedExpression.IsValid());

    OptimizedExpressionPtr expected = IsOfClassOptimizedExpression::Create("BisCore", "Subject", m_expressionsCache.GetMutex());
    EXPECT_TRUE(optimizedExpression->IsEqual(*expected));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsOptimizerTests, OptimizeParentIsInstanceNodeExpression)
    {
    Utf8CP expression = "ParentNode.IsInstanceNode";
    OptimizedExpressionPtr optimizedExpression = m_optimizer.GetOptimizedExpression(expression);
    ASSERT_TRUE(optimizedExpression.IsValid());

    OptimizedExpressionPtr expected = IsInstanceNodeOptimizedExpression::Create();
    EXPECT_TRUE(optimizedExpression->IsEqual(*expected));
    }