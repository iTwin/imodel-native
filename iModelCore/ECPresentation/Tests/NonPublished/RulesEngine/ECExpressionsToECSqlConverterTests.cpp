/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "../../../Source/RulesDriven/RulesEngine/ECExpressionContextsProvider.h"
#include "../../../Source/RulesDriven/RulesEngine/CustomFunctions.h"
#include "TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                05/2016
+===============+===============+===============+===============+===============+======*/
struct ECExpressionsToECSqlConverterTests : ECPresentationTest
    {
    ECExpressionsCache m_expressionsCache;
    ECExpressionsHelper m_helper;

    ECExpressionsToECSqlConverterTests()
        : m_helper(m_expressionsCache)
        {}
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, LogicalOperators)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("A And B AndAlso C Or D");
    ASSERT_STREQ("[A] AND [B] AND [C] OR [D]", ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, NotOperator)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("Not True");
    ASSERT_STREQ("NOT TRUE", ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, Parens)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("2 * (3 + 4)");
    ASSERT_STREQ("2 * (3 + 4)", ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, SingleQuotesInStringValues)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("this.Test = \"aaa'\"");
    ASSERT_STREQ("[this].[Test] = 'aaa'''", ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, DoubleQuotesInStringValues)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("this.Test = \"aaa\"\"\"");
    ASSERT_STREQ("[this].[Test] = 'aaa\"'", ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, LikeOperatorSpecialCase)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("this.Test ~ \"aaa\"");
    ASSERT_STREQ("CAST([this].[Test] AS TEXT) LIKE 'aaa' ESCAPE \'\\\'", ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, LikeOperatorSpecialCase_WithFunction)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("LOWER(this.Test) ~ \"aaa\"");
    ASSERT_STREQ("CAST(LOWER([this].[Test]) AS TEXT) LIKE 'aaa' ESCAPE \'\\\'", ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, Concatenation)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("\"str\" & Label");
    ASSERT_STREQ("'str' || [Label]", ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, IsNullCase)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("Label = NULL");
    ASSERT_STREQ("[Label] IS NULL", ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, IsNotNullCase)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("Label <> NULL");
    ASSERT_STREQ("[Label] IS NOT NULL", ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, Functions)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("GetDisplayLabel(param1, \"param2\", 3)");
    ASSERT_STREQ("GetDisplayLabel([param1], 'param2', 3)", ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, Properties)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("this.ParentId = parent.ECInstanceId");
    ASSERT_STREQ("[this].[ParentId] = [parent].[ECInstanceId]", ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, StructProperties)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("this.Code.Value = 1");
    ASSERT_STREQ("[this].[Code].[Value] = 1", ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, DisplayLabelFieldWithSlashes)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("/DisplayLabel/ = \"a\"");
    ASSERT_STREQ("[/DisplayLabel/] = 'a'", ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, PropertiesAsFunctionArguments)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("some_function(this.PropertyName, \"test\")");
    ASSERT_STREQ("some_function([this].[PropertyName], 'test')", ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, IsOfClassFunctionSpecialCase)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("this.IsOfClass(\"ClassName\", \"SchemaName\")");
    ASSERT_STREQ("IsOfClass([this].[ECClassId], 'ClassName', 'SchemaName')", ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, GetECClassIdFunctionSpecialCase)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("this.ECClassId = GetECClassId(\"A\", \"B\") OR GetECClassId(\"C\", \"D\") <> 5");
    ASSERT_STREQ("[this].[ECClassId] = RulesEngine_GetECClassId('A', 'B') OR RulesEngine_GetECClassId('C', 'D') <> 5", ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, HasRelatedInstanceSpecialCase_Forward)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("this.HasRelatedInstance(\"TestSchema1:RelationshipName\", \"Forward\", \"TestSchema2:RelatedClassName\")");
    ASSERT_STREQ(" EXISTS ("
        "SELECT 1 "
        "FROM [TestSchema1].[RelationshipName] relationship "
        "JOIN [TestSchema2].[RelatedClassName] related ON [relationship].[TargetECClassId] = [related].[ECClassId] AND [relationship].[TargetECInstanceId] = [related].[ECInstanceId] "
        "WHERE [relationship].[SourceECInstanceId] = [this].[ECInstanceId]"
        ")",
        ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, HasRelatedInstanceSpecialCase_Backward)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("this.HasRelatedInstance(\"TestSchema1:RelationshipName\", \"Backward\", \"TestSchema2:RelatedClassName\")");
    ASSERT_STREQ(" EXISTS ("
        "SELECT 1 "
        "FROM [TestSchema1].[RelationshipName] relationship "
        "JOIN [TestSchema2].[RelatedClassName] related ON [relationship].[SourceECClassId] = [related].[ECClassId] AND [relationship].[SourceECInstanceId] = [related].[ECInstanceId] "
        "WHERE [relationship].[TargetECInstanceId] = [this].[ECInstanceId]"
        ")",
        ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, GetRelatedInstancesCountSpecialCase_Forward)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("this.GetRelatedInstancesCount(\"TestSchema1:RelationshipName\", \"Forward\", \"TestSchema2:RelatedClassName\")");
    ASSERT_STREQ("("
        "SELECT COUNT(1) "
        "FROM [TestSchema1].[RelationshipName] relationship "
        "JOIN [TestSchema2].[RelatedClassName] related "
        "ON [related].[ECClassId] = [relationship].[TargetECClassId] AND [related].[ECInstanceId] = [relationship].[TargetECInstanceId] "
        "WHERE [relationship].[SourceECClassId] = [this].[ECClassId] AND [relationship].[SourceECInstanceId] = [this].[ECInstanceId]"
        ")",
        ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, GetRelatedInstancesCountSpecialCase_Backward)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("this.GetRelatedInstancesCount(\"TestSchema1:RelationshipName\", \"Backward\", \"TestSchema2:RelatedClassName\")");
    ASSERT_STREQ("("
        "SELECT COUNT(1) "
        "FROM [TestSchema1].[RelationshipName] relationship "
        "JOIN [TestSchema2].[RelatedClassName] related "
        "ON [related].[ECClassId] = [relationship].[SourceECClassId] AND [related].[ECInstanceId] = [relationship].[SourceECInstanceId] "
        "WHERE [relationship].[TargetECClassId] = [this].[ECClassId] AND [relationship].[TargetECInstanceId] = [this].[ECInstanceId]"
        ")",
        ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, GetRelatedValueSpecialCase_Forward)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("this.GetRelatedValue(\"TestSchema1:RelationshipName\", \"Forward\", \"TestSchema2:RelatedClassName\", \"SomePropertyName\")");
    ASSERT_STREQ("("
        "SELECT [related].[SomePropertyName] "
        "FROM [TestSchema1].[RelationshipName] relationship, [TestSchema2].[RelatedClassName] related "
        "WHERE [relationship].[SourceECClassId] = [this].[ECClassId] AND [relationship].[SourceECInstanceId] = [this].[ECInstanceId] "
        "AND [relationship].[TargetECClassId] = [related].[ECClassId] AND [relationship].[TargetECInstanceId] = [related].[ECInstanceId] "
        "LIMIT 1"
        ")",
        ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, GetRelatedValueSpecialCase_Backward)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("this.GetRelatedValue(\"TestSchema1:RelationshipName\", \"Backward\", \"TestSchema2:RelatedClassName\", \"SomePropertyName\")");
    ASSERT_STREQ("("
        "SELECT [related].[SomePropertyName] "
        "FROM [TestSchema1].[RelationshipName] relationship, [TestSchema2].[RelatedClassName] related "
        "WHERE [relationship].[TargetECClassId] = [this].[ECClassId] AND [relationship].[TargetECInstanceId] = [this].[ECInstanceId] "
        "AND [relationship].[SourceECClassId] = [related].[ECClassId] AND [relationship].[SourceECInstanceId] = [related].[ECInstanceId] "
        "LIMIT 1"
        ")",
        ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Bill.Goehrig                    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, GetRelatedValueSpecialCase_StructProperty)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("this.GetRelatedValue(\"TestSchema1:RelationshipName\", \"Forward\", \"TestSchema2:RelatedClassName\", \"Code.Value\")");
    ASSERT_STREQ("("
        "SELECT [related].[Code].[Value] "
        "FROM [TestSchema1].[RelationshipName] relationship, [TestSchema2].[RelatedClassName] related "
        "WHERE [relationship].[SourceECClassId] = [this].[ECClassId] AND [relationship].[SourceECInstanceId] = [this].[ECInstanceId] "
        "AND [relationship].[TargetECClassId] = [related].[ECClassId] AND [relationship].[TargetECInstanceId] = [related].[ECInstanceId] "
        "LIMIT 1"
        ")",
        ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, FunctionNameSubstitution)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("GetSettingValue(\"setting_id\")");
    EXPECT_STREQ(FUNCTION_NAME_GetVariableStringValue "('setting_id')", ecsql.c_str());

    ecsql = m_helper.ConvertToECSql("GetSettingIntValue(\"setting_id\")");
    EXPECT_STREQ(FUNCTION_NAME_GetVariableIntValue "('setting_id')", ecsql.c_str());

    ecsql = m_helper.ConvertToECSql("GetSettingBoolValue(\"setting_id\")");
    EXPECT_STREQ(FUNCTION_NAME_GetVariableBoolValue "('setting_id')", ecsql.c_str());

    ecsql = m_helper.ConvertToECSql("HasSetting(\"setting_id\")");
    EXPECT_STREQ(FUNCTION_NAME_HasVariable "('setting_id')", ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, VariableIntValuesSpecialCase)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("GetSettingIntValues(\"setting_id\").AnyMatch(x => x = this.SomeProperty)");
    EXPECT_STREQ(FUNCTION_NAME_InVariableIntValues "('setting_id', [this].[SomeProperty])", ecsql.c_str());

    ecsql = m_helper.ConvertToECSql("GetSettingIntValues(\"setting_id\").AnyMatch(x => this.SomeProperty = x)");
    EXPECT_STREQ(FUNCTION_NAME_InVariableIntValues "('setting_id', [this].[SomeProperty])", ecsql.c_str());

    ecsql = m_helper.ConvertToECSql("GetVariableIntValues(\"setting_id\").AnyMatch(x => this.SomeProperty = x)");
    EXPECT_STREQ(FUNCTION_NAME_InVariableIntValues "('setting_id', [this].[SomeProperty])", ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, ValueSetAnyMatchSpecialCaseMatchingProperty)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("Set(1, 2, 3, \"4\").AnyMatch(x => x = this.SomeProperty.Id)");
    EXPECT_STREQ("[this].[SomeProperty].[Id] IN (1, 2, 3, '4')", ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, ValueSetAnyMatchSpecialCaseMatchingProperty_Reverse)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("Set(1, 2, 3, \"4\").AnyMatch(x => this.SomeProperty.Id = x)");
    EXPECT_STREQ("[this].[SomeProperty].[Id] IN (1, 2, 3, '4')", ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, ValueSetAnyMatchSpecialCaseMatchingFunctionCall)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("Set(\"a\", \"b\").AnyMatch(x => x = upper(this.SomeProperty.Id))");
    EXPECT_STREQ("upper([this].[SomeProperty].[Id]) IN ('a', 'b')", ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, ValueSetAnyMatchSpecialCaseMatchingFunctionCall_Reverse)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("Set(\"a\", \"b\").AnyMatch(x => upper(this.SomeProperty.Id) = x)");
    EXPECT_STREQ("upper([this].[SomeProperty].[Id]) IN ('a', 'b')", ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, ValueSetWithFunctionCallAnyMatchSpecialCase)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("Set(upper(\"a\")).AnyMatch(x => x = this.SomeProperty)");
    EXPECT_STREQ("[this].[SomeProperty] IN (upper('a'))", ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Haroldas.Vitunskas              01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, ParseValueExpressionAndCreateTree_ExpressionOptimizedWithWhitespaceBeforeParentheses)
    {
    Utf8CP expectedString = "ParentNode.IsOfClass(\"Subject\",\"BisCore\")";
    NodePtr node = m_helper.GetNodeFromExpression("ParentNode.ECInstance.IsOfClass (\"Subject\", \"BisCore\")");
    EXPECT_STREQ(expectedString, node->ToExpressionString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Haroldas.Vitunskas              01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, ParseValueExpressionAndCreateTree_ExpressionOptimizedWithMultipleWhitespacesBeforeParantheses)
    {
    Utf8CP expectedString = "ParentNode.IsOfClass(\"Subject\",\"BisCore\")";
    NodePtr node = m_helper.GetNodeFromExpression("ParentNode.ECInstance.IsOfClass \t\n(\"Subject\", \"BisCore\")");
    EXPECT_STREQ(expectedString, node->ToExpressionString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Haroldas.Vitunskas              01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, ParseValueExpressionAndCreateTree_ExpressionArgumentsNotCorruptedWithWhitespacesInsideQuotation)
    {
    Utf8CP expectedString = "ParentNode.IsOfClass(\"Subject \",\"BisCore\")";
    NodePtr node = m_helper.GetNodeFromExpression("ParentNode.ECInstance.IsOfClass(\"Subject \", \"BisCore\")");
    EXPECT_STREQ(expectedString, node->ToExpressionString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Haroldas.Vitunskas              01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, ParseValueExpressionAndCreateTree_SingleExpressionWhitespaceNotRemovedIfBetweenAlphaNumSymbols)
    {
    Utf8CP expectedString = "ParentNode.IsOfClass(\"Subject \",\"BisCore\")Or ThisNode.IsOfClass(\"Subject \",\"BisCore\")";
    NodePtr node = m_helper.GetNodeFromExpression("ParentNode.ECInstance.IsOfClass(\"Subject \", \"BisCore\") Or ThisNode.IsOfClass(\"Subject \", \"BisCore\")");
    EXPECT_STREQ(expectedString, node->ToExpressionString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Haroldas.Vitunskas              01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, ParseValueExpressionAndCreateTree_MultipleExpressionWhitespacesNotRemovedIfBetweenAlphaNumSymbols)
    {
    Utf8CP expectedString = "ParentNode.IsOfClass(\"Subject \",\"BisCore\")Or ThisNode.IsOfClass(\"Subject \",\"BisCore\")";
    NodePtr node = m_helper.GetNodeFromExpression("ParentNode.ECInstance.IsOfClass(\"Subject \", \"BisCore\") \tOr \n\tThisNode.IsOfClass(\"Subject \", \"BisCore\")");
    EXPECT_STREQ(expectedString, node->ToExpressionString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, CompareDateTimes_WrapsAndComparesArguments)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("CompareDateTimes(this.PropertyName, 2)");
    EXPECT_STREQ("(JULIANDAY([this].[PropertyName]) - JULIANDAY(2))", ecsql.c_str());
    }
