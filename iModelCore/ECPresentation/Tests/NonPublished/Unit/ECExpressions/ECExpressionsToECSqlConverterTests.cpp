/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentation/ECPresentationManager.h>
#include "../../../../Source/Shared/ECExpressions/ECExpressionContextsProvider.h"
#include "../../../../Source/Shared/Queries/CustomFunctions.h"
#include "../../Helpers/TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, LogicalOperators)
    {
    auto clause = m_helper.ConvertToECSql("A And B AndAlso C Or D", nullptr, nullptr);
    ASSERT_STREQ("[A] AND [B] AND [C] OR [D]", clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, NotOperator)
    {
    auto clause = m_helper.ConvertToECSql("Not True", nullptr, nullptr);
    ASSERT_STREQ("NOT TRUE", clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, Parens)
    {
    auto clause = m_helper.ConvertToECSql("2 * (3 + 4)", nullptr, nullptr);
    ASSERT_STREQ("2 * (3 + 4)", clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, SingleQuotesInStringValues)
    {
    auto clause = m_helper.ConvertToECSql("this.Test = \"aaa'\"", nullptr, nullptr);
    ASSERT_STREQ("[this].[Test] = 'aaa'''", clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, DoubleQuotesInStringValues)
    {
    auto clause = m_helper.ConvertToECSql("this.Test = \"aaa\"\"\"", nullptr, nullptr);
    ASSERT_STREQ("[this].[Test] = 'aaa\"'", clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, HexInteger)
    {
    auto clause = m_helper.ConvertToECSql("this.Test = 0x4D2", nullptr, nullptr);
    ASSERT_STREQ("[this].[Test] = 1234", clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, LikeOperatorSpecialCase)
    {
    auto clause = m_helper.ConvertToECSql("this.Test ~ \"aaa\"", nullptr, nullptr);
    ASSERT_STREQ("CAST([this].[Test] AS TEXT) LIKE 'aaa' ESCAPE \'\\\'", clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, LikeOperatorSpecialCase_WithFunction)
    {
    auto clause = m_helper.ConvertToECSql("LOWER(this.Test) ~ \"aaa\"", nullptr, nullptr);
    ASSERT_STREQ("CAST(LOWER([this].[Test]) AS TEXT) LIKE 'aaa' ESCAPE \'\\\'", clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, Concatenation)
    {
    auto clause = m_helper.ConvertToECSql("\"str\" & Label", nullptr, nullptr);
    ASSERT_STREQ("'str' || [Label]", clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, IsNullCase)
    {
    auto clause = m_helper.ConvertToECSql("Label = NULL", nullptr, nullptr);
    ASSERT_STREQ("[Label] IS NULL", clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, IsNotNullCase)
    {
    auto clause = m_helper.ConvertToECSql("Label <> NULL", nullptr, nullptr);
    ASSERT_STREQ("[Label] IS NOT NULL", clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, Functions)
    {
    auto clause = m_helper.ConvertToECSql("GetDisplayLabel(param1, \"param2\", 3)", nullptr, nullptr);
    ASSERT_STREQ("GetDisplayLabel([param1], 'param2', 3)", clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, Properties)
    {
    auto clause = m_helper.ConvertToECSql("this.ParentId = parent.ECInstanceId", nullptr, nullptr);
    ASSERT_STREQ("[this].[ParentId] = [parent].[ECInstanceId]", clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, StructProperties)
    {
    auto clause = m_helper.ConvertToECSql("this.Code.Value = 1", nullptr, nullptr);
    ASSERT_STREQ("[this].[Code].[Value] = 1", clause.GetClause().c_str());
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TestPresentationQueryFieldTypesProvider : IPresentationQueryFieldTypesProvider
    {
    PresentationQueryFieldType m_fieldType;
    PresentationQueryFieldType _GetFieldType(Utf8StringCR name) const override {return m_fieldType;}
    TestPresentationQueryFieldTypesProvider(PresentationQueryFieldType fieldType)
        : m_fieldType(fieldType)
        {}
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, DisplayLabelFieldWithSlashes)
    {
    TestPresentationQueryFieldTypesProvider types(PresentationQueryFieldType::LabelDefinition);
    auto clause = m_helper.ConvertToECSql("/DisplayLabel/ = \"a\"", &types, nullptr);
    ASSERT_STREQ("GetLabelDisplayValue([/DisplayLabel/]) = 'a'", clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, DisplayLabelFieldWithSlashes_LikeOperator)
    {
    TestPresentationQueryFieldTypesProvider types(PresentationQueryFieldType::LabelDefinition);
    auto clause = m_helper.ConvertToECSql("/DisplayLabel/ LIKE \"a\"", &types, nullptr);
    ASSERT_STREQ("GetLabelDisplayValue([/DisplayLabel/]) LIKE 'a' ESCAPE \'\\\'", clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, NavigationPropertyField)
    {
    TestPresentationQueryFieldTypesProvider types(PresentationQueryFieldType::NavigationPropertyValue);
    auto clause = m_helper.ConvertToECSql("SomeNavigationProperty = \"a\"", &types, nullptr);
    ASSERT_STREQ("GetLabelDisplayValue(json_extract([SomeNavigationProperty], '$.label')) = 'a'", clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, PropertiesAsFunctionArguments)
    {
    auto clause = m_helper.ConvertToECSql("some_function(this.PropertyName, \"test\")", nullptr, nullptr);
    ASSERT_STREQ("some_function([this].[PropertyName], 'test')", clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, IsOfClassFunctionSpecialCase)
    {
    auto clause = m_helper.ConvertToECSql("this.IsOfClass(\"ClassName\", \"SchemaName\")", nullptr, nullptr);
    ASSERT_STREQ("IsOfClass([this].[ECClassId], 'ClassName', 'SchemaName')", clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, GetECClassIdFunctionSpecialCase)
    {
    auto clause = m_helper.ConvertToECSql("this.ECClassId = GetECClassId(\"A\", \"B\") OR GetECClassId(\"C\", \"D\") <> 5", nullptr, nullptr);
    ASSERT_STREQ("[this].[ECClassId] = RulesEngine_GetECClassId('A', 'B') OR RulesEngine_GetECClassId('C', 'D') <> 5", clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, HasRelatedInstanceSpecialCase_Forward)
    {
    auto clause = m_helper.ConvertToECSql("this.HasRelatedInstance(\"TestSchema1:RelationshipName\", \"Forward\", \"TestSchema2:RelatedClassName\")", nullptr, nullptr);
    ASSERT_STREQ(" EXISTS ("
        "SELECT 1 "
        "FROM [TestSchema1].[RelationshipName] relationship "
        "JOIN [TestSchema2].[RelatedClassName] related ON [relationship].[TargetECClassId] = [related].[ECClassId] AND [relationship].[TargetECInstanceId] = [related].[ECInstanceId] "
        "WHERE [relationship].[SourceECInstanceId] = +[this].[ECInstanceId]"
        ")",
        clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, HasRelatedInstanceSpecialCase_Backward)
    {
    auto clause = m_helper.ConvertToECSql("this.HasRelatedInstance(\"TestSchema1:RelationshipName\", \"Backward\", \"TestSchema2:RelatedClassName\")", nullptr, nullptr);
    ASSERT_STREQ(" EXISTS ("
        "SELECT 1 "
        "FROM [TestSchema1].[RelationshipName] relationship "
        "JOIN [TestSchema2].[RelatedClassName] related ON [relationship].[SourceECClassId] = [related].[ECClassId] AND [relationship].[SourceECInstanceId] = [related].[ECInstanceId] "
        "WHERE [relationship].[TargetECInstanceId] = +[this].[ECInstanceId]"
        ")",
        clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, HasRelatedInstanceSpecialCase_RelatedClassAndLambda)
    {
    auto clause = m_helper.ConvertToECSql("this.HasRelatedInstance(\"TestSchema2:RelatedClassName\", r => r.ECClassId = this.ECInstanceId)", nullptr, nullptr);
    ASSERT_STREQ(" EXISTS ("
        "SELECT 1 "
        "FROM [TestSchema2].[RelatedClassName] [r] "
        "WHERE +([r].[ECClassId] = [this].[ECInstanceId])"
        ")",
        clause.GetClause().c_str());

    clause = m_helper.ConvertToECSql("this.HasRelatedInstance(\"TestSchema2:RelatedClassName\", r => r.IsOfClass(this.ECInstanceId))", nullptr, nullptr);
    ASSERT_STREQ(" EXISTS ("
        "SELECT 1 "
        "FROM [TestSchema2].[RelatedClassName] [r] "
        "WHERE +(IsOfClass([r].[ECClassId], [this].[ECInstanceId]))"
        ")",
        clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, GetRelatedInstancesCountSpecialCase_Forward)
    {
    auto clause = m_helper.ConvertToECSql("this.GetRelatedInstancesCount(\"TestSchema1:RelationshipName\", \"Forward\", \"TestSchema2:RelatedClassName\")", nullptr, nullptr);
    ASSERT_STREQ("("
        "SELECT COUNT(1) "
        "FROM [TestSchema1].[RelationshipName] relationship "
        "JOIN [TestSchema2].[RelatedClassName] related "
        "ON [related].[ECClassId] = [relationship].[TargetECClassId] AND [related].[ECInstanceId] = [relationship].[TargetECInstanceId] "
        "WHERE [relationship].[SourceECClassId] = [this].[ECClassId] AND [relationship].[SourceECInstanceId] = [this].[ECInstanceId]"
        ")",
        clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, GetRelatedInstancesCountSpecialCase_Backward)
    {
    auto clause = m_helper.ConvertToECSql("this.GetRelatedInstancesCount(\"TestSchema1:RelationshipName\", \"Backward\", \"TestSchema2:RelatedClassName\")", nullptr, nullptr);
    ASSERT_STREQ("("
        "SELECT COUNT(1) "
        "FROM [TestSchema1].[RelationshipName] relationship "
        "JOIN [TestSchema2].[RelatedClassName] related "
        "ON [related].[ECClassId] = [relationship].[SourceECClassId] AND [related].[ECInstanceId] = [relationship].[SourceECInstanceId] "
        "WHERE [relationship].[TargetECClassId] = [this].[ECClassId] AND [relationship].[TargetECInstanceId] = [this].[ECInstanceId]"
        ")",
        clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, GetRelatedInstancesCountSpecialCase_RelatedClassAndLambda)
    {
    auto clause = m_helper.ConvertToECSql("this.GetRelatedInstancesCount(\"TestSchema2:RelatedClassName\", a => a.ECClassId = this.ECInstanceId)", nullptr, nullptr);
    ASSERT_STREQ("("
        "SELECT COUNT(1) "
        "FROM [TestSchema2].[RelatedClassName] [a] "
        "WHERE [a].[ECClassId] = [this].[ECInstanceId]"
        ")",
        clause.GetClause().c_str());

    clause = m_helper.ConvertToECSql("this.GetRelatedInstancesCount(\"TestSchema2:RelatedClassName\", a => a.IsOfClass(this.ECInstanceId))", nullptr, nullptr);
    ASSERT_STREQ("("
        "SELECT COUNT(1) "
        "FROM [TestSchema2].[RelatedClassName] [a] "
        "WHERE IsOfClass([a].[ECClassId], [this].[ECInstanceId])"
        ")",
        clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, GetRelatedValueSpecialCase_Forward)
    {
    auto clause = m_helper.ConvertToECSql("this.GetRelatedValue(\"TestSchema1:RelationshipName\", \"Forward\", \"TestSchema2:RelatedClassName\", \"SomePropertyName\")", nullptr, nullptr);
    ASSERT_STREQ("("
        "SELECT [related].[SomePropertyName] "
        "FROM [TestSchema1].[RelationshipName] relationship, [TestSchema2].[RelatedClassName] related "
        "WHERE [relationship].[SourceECClassId] = [this].[ECClassId] AND [relationship].[SourceECInstanceId] = [this].[ECInstanceId] "
        "AND [relationship].[TargetECClassId] = [related].[ECClassId] AND [relationship].[TargetECInstanceId] = [related].[ECInstanceId] "
        "LIMIT 1"
        ")",
        clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, GetRelatedValueSpecialCase_Backward)
    {
    auto clause = m_helper.ConvertToECSql("this.GetRelatedValue(\"TestSchema1:RelationshipName\", \"Backward\", \"TestSchema2:RelatedClassName\", \"SomePropertyName\")", nullptr, nullptr);
    ASSERT_STREQ("("
        "SELECT [related].[SomePropertyName] "
        "FROM [TestSchema1].[RelationshipName] relationship, [TestSchema2].[RelatedClassName] related "
        "WHERE [relationship].[TargetECClassId] = [this].[ECClassId] AND [relationship].[TargetECInstanceId] = [this].[ECInstanceId] "
        "AND [relationship].[SourceECClassId] = [related].[ECClassId] AND [relationship].[SourceECInstanceId] = [related].[ECInstanceId] "
        "LIMIT 1"
        ")",
        clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, GetRelatedValueSpecialCase_StructProperty)
    {
    auto clause = m_helper.ConvertToECSql("this.GetRelatedValue(\"TestSchema1:RelationshipName\", \"Forward\", \"TestSchema2:RelatedClassName\", \"Code.Value\")", nullptr, nullptr);
    ASSERT_STREQ("("
        "SELECT [related].[Code].[Value] "
        "FROM [TestSchema1].[RelationshipName] relationship, [TestSchema2].[RelatedClassName] related "
        "WHERE [relationship].[SourceECClassId] = [this].[ECClassId] AND [relationship].[SourceECInstanceId] = [this].[ECInstanceId] "
        "AND [relationship].[TargetECClassId] = [related].[ECClassId] AND [relationship].[TargetECInstanceId] = [related].[ECInstanceId] "
        "LIMIT 1"
        ")",
        clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, GetRelatedValueSpecialCase_RelatedClassAndLambda)
    {
    auto clause = m_helper.ConvertToECSql("this.GetRelatedValue(\"TestSchema2:RelatedClassName\", b => b.ECClassId = this.ECInstanceId, \"SomePropertyName\")", nullptr, nullptr);
    ASSERT_STREQ("("
        "SELECT [b].[SomePropertyName] "
        "FROM [TestSchema2].[RelatedClassName] [b] "
        "WHERE [b].[ECClassId] = [this].[ECInstanceId] "
        "LIMIT 1"
        ")",
        clause.GetClause().c_str());

    clause = m_helper.ConvertToECSql("this.GetRelatedValue(\"TestSchema2:RelatedClassName\", b => b.IsOfClass(this.ECInstanceId), \"SomePropertyName\")", nullptr, nullptr);
    ASSERT_STREQ("("
        "SELECT [b].[SomePropertyName] "
        "FROM [TestSchema2].[RelatedClassName] [b] "
        "WHERE IsOfClass([b].[ECClassId], [this].[ECInstanceId]) "
        "LIMIT 1"
        ")",
        clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, FunctionNameSubstitution)
    {
    auto clause = m_helper.ConvertToECSql("GetSettingValue(\"setting_id\")", nullptr, nullptr);
    EXPECT_STREQ(FUNCTION_NAME_GetVariableStringValue "('setting_id')", clause.GetClause().c_str());

    clause = m_helper.ConvertToECSql("GetSettingIntValue(\"setting_id\")", nullptr, nullptr);
    EXPECT_STREQ(FUNCTION_NAME_GetVariableIntValue "('setting_id')", clause.GetClause().c_str());

    clause = m_helper.ConvertToECSql("GetSettingBoolValue(\"setting_id\")", nullptr, nullptr);
    EXPECT_STREQ(FUNCTION_NAME_GetVariableBoolValue "('setting_id')", clause.GetClause().c_str());

    clause = m_helper.ConvertToECSql("HasSetting(\"setting_id\")", nullptr, nullptr);
    EXPECT_STREQ(FUNCTION_NAME_HasVariable "('setting_id')", clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, VariableIntValuesSpecialCase)
    {
    ECDbTestProject project;
    RefCountedPtr<TestConnection> connection = new TestConnection(project.GetECDb());
    RulesetVariables rulesetVariables;
    rulesetVariables.SetIntValues("setting_id", bvector<int64_t>{ 123, 456 });
    ECExpressionContextsProvider::NodeRulesContextParameters contextParams(nullptr, *connection, rulesetVariables, nullptr);
    auto expressionContext = ECExpressionContextsProvider::GetNodeRulesContext(contextParams);

    auto clause = m_helper.ConvertToECSql("GetSettingIntValues(\"setting_id\").AnyMatches(x => x = this.SomeProperty)", nullptr, expressionContext.get());
    EXPECT_STREQ("[this].[SomeProperty] IN (?,?)", clause.GetClause().c_str());
    EXPECT_EQ(2, clause.GetBindings().size());
    EXPECT_EQ(BoundQueryECValue(ECValue(123)), *clause.GetBindings().at(0));
    EXPECT_EQ(BoundQueryECValue(ECValue(456)), *clause.GetBindings().at(1));

    clause = m_helper.ConvertToECSql("GetSettingIntValues(\"setting_id\").AnyMatches(x => this.SomeProperty = x)", nullptr, expressionContext.get());
    EXPECT_STREQ("[this].[SomeProperty] IN (?,?)", clause.GetClause().c_str());
    EXPECT_EQ(2, clause.GetBindings().size());
    EXPECT_EQ(BoundQueryECValue(ECValue(123)), *clause.GetBindings().at(0));
    EXPECT_EQ(BoundQueryECValue(ECValue(456)), *clause.GetBindings().at(1));

    clause = m_helper.ConvertToECSql("GetVariableIntValues(\"setting_id\").AnyMatches(x => this.SomeProperty = x)", nullptr, expressionContext.get());
    EXPECT_STREQ("[this].[SomeProperty] IN (?,?)", clause.GetClause().c_str());
    EXPECT_EQ(2, clause.GetBindings().size());
    EXPECT_EQ(BoundQueryECValue(ECValue(123)), *clause.GetBindings().at(0));
    EXPECT_EQ(BoundQueryECValue(ECValue(456)), *clause.GetBindings().at(1));

    // for large value lists we should be using InVirtualSet instead of operator IN
    bvector<ECValue> expectedValues;
    bvector<int64_t> ids;
    for (int64_t i = 0; i < 1000; ++i)
        {
        ids.push_back(i + 1);
        expectedValues.push_back(ECValue(i + 1));
        }
    rulesetVariables.SetIntValues("setting_id", ids);

    clause = m_helper.ConvertToECSql("GetVariableIntValues(\"setting_id\").AnyMatches(x => this.SomeProperty = x)", nullptr, expressionContext.get());
    EXPECT_STREQ("InVirtualSet(?, [this].[SomeProperty])", clause.GetClause().c_str());
    EXPECT_EQ(1, clause.GetBindings().size());
    EXPECT_EQ(BoundECValueSet(expectedValues), *clause.GetBindings().at(0));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, ValueSetAnyMatchesSpecialCaseMatchingProperty)
    {
    ECDbTestProject project;
    RefCountedPtr<TestConnection> connection = new TestConnection(project.GetECDb());
    RulesetVariables rulesetVariables;
    rulesetVariables.SetIntValues("setting_id", bvector<int64_t>{ 123, 456 });
    ECExpressionContextsProvider::NodeRulesContextParameters contextParams(nullptr, *connection, rulesetVariables, nullptr);
    auto expressionContext = ECExpressionContextsProvider::GetNodeRulesContext(contextParams);

    auto clause = m_helper.ConvertToECSql("Set(1, \"4\").AnyMatches(x => x = this.SomeProperty.Id)", nullptr, expressionContext.get());
    EXPECT_STREQ("[this].[SomeProperty].[Id] IN (?,?)", clause.GetClause().c_str());
    EXPECT_EQ(2, clause.GetBindings().size());
    EXPECT_EQ(BoundQueryECValue(ECValue(1)), *clause.GetBindings().at(0));
    EXPECT_EQ(BoundQueryECValue(ECValue("4")), *clause.GetBindings().at(1));

    clause = m_helper.ConvertToECSql("Set(1, \"4\").AnyMatches(x => this.SomeProperty.Id = x)", nullptr, expressionContext.get());
    EXPECT_STREQ("[this].[SomeProperty].[Id] IN (?,?)", clause.GetClause().c_str());
    EXPECT_EQ(2, clause.GetBindings().size());
    EXPECT_EQ(BoundQueryECValue(ECValue(1)), *clause.GetBindings().at(0));
    EXPECT_EQ(BoundQueryECValue(ECValue("4")), *clause.GetBindings().at(1));

    clause = m_helper.ConvertToECSql("Set(\"a\", \"b\").AnyMatches(x => x = upper(this.SomeProperty.Id))", nullptr, expressionContext.get());
    EXPECT_STREQ("upper([this].[SomeProperty].[Id]) IN (?,?)", clause.GetClause().c_str());
    EXPECT_EQ(2, clause.GetBindings().size());
    EXPECT_EQ(BoundQueryECValue(ECValue("a")), *clause.GetBindings().at(0));
    EXPECT_EQ(BoundQueryECValue(ECValue("b")), *clause.GetBindings().at(1));

    clause = m_helper.ConvertToECSql("Set(\"a\", \"b\").AnyMatches(x => upper(this.SomeProperty.Id) = x)", nullptr, expressionContext.get());
    EXPECT_STREQ("upper([this].[SomeProperty].[Id]) IN (?,?)", clause.GetClause().c_str());
    EXPECT_EQ(2, clause.GetBindings().size());
    EXPECT_EQ(BoundQueryECValue(ECValue("a")), *clause.GetBindings().at(0));
    EXPECT_EQ(BoundQueryECValue(ECValue("b")), *clause.GetBindings().at(1));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, ParseValueExpressionAndCreateTree_ExpressionOptimizedWithWhitespaceBeforeParentheses)
    {
    Utf8CP expectedString = "ParentNode.IsOfClass(\"Subject\",\"BisCore\")";
    NodePtr node = m_helper.GetNodeFromExpression("ParentNode.ECInstance.IsOfClass (\"Subject\", \"BisCore\")");
    EXPECT_STREQ(expectedString, node->ToExpressionString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, ParseValueExpressionAndCreateTree_ExpressionOptimizedWithMultipleWhitespacesBeforeParantheses)
    {
    Utf8CP expectedString = "ParentNode.IsOfClass(\"Subject\",\"BisCore\")";
    NodePtr node = m_helper.GetNodeFromExpression("ParentNode.ECInstance.IsOfClass \t\n(\"Subject\", \"BisCore\")");
    EXPECT_STREQ(expectedString, node->ToExpressionString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, ParseValueExpressionAndCreateTree_ExpressionArgumentsNotCorruptedWithWhitespacesInsideQuotation)
    {
    Utf8CP expectedString = "ParentNode.IsOfClass(\"Subject \",\"BisCore\")";
    NodePtr node = m_helper.GetNodeFromExpression("ParentNode.ECInstance.IsOfClass(\"Subject \", \"BisCore\")");
    EXPECT_STREQ(expectedString, node->ToExpressionString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, ParseValueExpressionAndCreateTree_SingleExpressionWhitespaceNotRemovedIfBetweenAlphaNumSymbols)
    {
    Utf8CP expectedString = "ParentNode.IsOfClass(\"Subject \",\"BisCore\")Or ThisNode.IsOfClass(\"Subject \",\"BisCore\")";
    NodePtr node = m_helper.GetNodeFromExpression("ParentNode.ECInstance.IsOfClass(\"Subject \", \"BisCore\") Or ThisNode.IsOfClass(\"Subject \", \"BisCore\")");
    EXPECT_STREQ(expectedString, node->ToExpressionString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, ParseValueExpressionAndCreateTree_MultipleExpressionWhitespacesNotRemovedIfBetweenAlphaNumSymbols)
    {
    Utf8CP expectedString = "ParentNode.IsOfClass(\"Subject \",\"BisCore\")Or ThisNode.IsOfClass(\"Subject \",\"BisCore\")";
    NodePtr node = m_helper.GetNodeFromExpression("ParentNode.ECInstance.IsOfClass(\"Subject \", \"BisCore\") \tOr \n\tThisNode.IsOfClass(\"Subject \", \"BisCore\")");
    EXPECT_STREQ(expectedString, node->ToExpressionString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, CompareDateTimes_WrapsAndComparesArguments)
    {
    auto clause = m_helper.ConvertToECSql("CompareDateTimes(this.PropertyName, 2)", nullptr, nullptr);
    EXPECT_STREQ("(JULIANDAY([this].[PropertyName]) - JULIANDAY(2))", clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, HandleParentheses_ConvertsOneCallInsideParentheses)
    {
    auto clause = m_helper.ConvertToECSql("(model.GetRelatedValue(\"BisCore:ModelModelsElement\", \"Forward\", \"BisCore:InformationPartitionElement\", \"Parent.Id\") = 1)", nullptr, nullptr);
    EXPECT_STREQ("("
        "(SELECT [related].[Parent].[Id] "
        "FROM [BisCore].[ModelModelsElement] relationship, [BisCore].[InformationPartitionElement] related "
        "WHERE [relationship].[SourceECClassId] = [model].[ECClassId] AND [relationship].[SourceECInstanceId] = [model].[ECInstanceId] "
        "AND [relationship].[TargetECClassId] = [related].[ECClassId] AND [relationship].[TargetECInstanceId] = [related].[ECInstanceId] "
        "LIMIT 1) = 1"
        ")",
        clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, HandleParentheses_ConvertsMultipleCallsInsideParentheses)
    {
    auto clause = m_helper.ConvertToECSql("(GetVariableBoolValue(\"var_id\") OR model.GetRelatedValue(\"BisCore:ModelModelsElement\", \"Forward\", \"BisCore:InformationPartitionElement\", \"Parent.Id\") = 1)", nullptr, nullptr);
    EXPECT_STREQ("("
        "GetVariableBoolValue('var_id') OR "
        "(SELECT [related].[Parent].[Id] "
        "FROM [BisCore].[ModelModelsElement] relationship, [BisCore].[InformationPartitionElement] related "
        "WHERE [relationship].[SourceECClassId] = [model].[ECClassId] AND [relationship].[SourceECInstanceId] = [model].[ECInstanceId] "
        "AND [relationship].[TargetECClassId] = [related].[ECClassId] AND [relationship].[TargetECInstanceId] = [related].[ECInstanceId] "
        "LIMIT 1) = 1"
        ")",
        clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, HandleParentheses_ConvertsCallsInsideNestedParentheses)
    {
    auto clause = m_helper.ConvertToECSql("((GetVariableBoolValue(\"var_id\") OR model.GetRelatedValue(\"BisCore:ModelModelsElement\", \"Forward\", \"BisCore:InformationPartitionElement\", \"Parent.Id\") = 1) ANDALSO this.ECClassId = parent.ECInstanceId)", nullptr, nullptr);
    EXPECT_STREQ("("
        "(GetVariableBoolValue('var_id') OR "
        "(SELECT [related].[Parent].[Id] "
        "FROM [BisCore].[ModelModelsElement] relationship, [BisCore].[InformationPartitionElement] related "
        "WHERE [relationship].[SourceECClassId] = [model].[ECClassId] AND [relationship].[SourceECInstanceId] = [model].[ECInstanceId] "
        "AND [relationship].[TargetECClassId] = [related].[ECClassId] AND [relationship].[TargetECInstanceId] = [related].[ECInstanceId] "
        "LIMIT 1) = 1) "
        "AND [this].[ECClassId] = [parent].[ECInstanceId]"
        ")",
        clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, GetFormattedValue_ConvertsThisNodeProperty)
    {
    auto clause = m_helper.ConvertToECSql("GetFormattedValue(this.MyValue, \"Metric\")", nullptr, nullptr);
    EXPECT_STREQ("GetFormattedValue([this].[ECClassId], 'MyValue', [this].[MyValue], 'Metric')", clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, GetFormattedValue_ConvertsRelatedNodeProperty)
    {
    auto clause = m_helper.ConvertToECSql("GetFormattedValue(related.MyValue, \"Metric\")", nullptr, nullptr);
    EXPECT_STREQ("GetFormattedValue([related].[ECClassId], 'MyValue', [related].[MyValue], 'Metric')", clause.GetClause().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, ConvertsKnownECExpressionValueToLiteralExpression)
    {
    ECDbTestProject project;
    project.Create(BeTest::GetNameOfCurrentTest(), "RulesEngineTest.01.00.ecschema.xml");
    RefCountedPtr<TestConnection> connection = new TestConnection(project.GetECDb());

    auto node = TestNodesHelper::CreateInstanceNode(*connection, *project.GetECDb().Schemas().GetClass("RulesEngineTest", "Widget"), ECInstanceId((uint64_t)456));
    node->SetLabelDefinition(*LabelDefinition::FromString("MyLabel"));

    RulesetVariables rulesetVariables;
    ECExpressionContextsProvider::NodeRulesContextParameters contextParams(node.get(), *connection, rulesetVariables, nullptr);
    auto expressionContext = ECExpressionContextsProvider::GetNodeRulesContext(contextParams);

    auto clause = m_helper.ConvertToECSql("this.Value = ParentNode.InstanceId", nullptr, expressionContext.get());
    EXPECT_STREQ("[this].[Value] = ?", clause.GetClause().c_str());
    EXPECT_EQ(1, clause.GetBindings().size());
    EXPECT_EQ(BoundQueryECValue(ECValue(456)), *clause.GetBindings().at(0));

    clause = m_helper.ConvertToECSql("this.Value = ParentNode.Label", nullptr, expressionContext.get());
    EXPECT_STREQ("[this].[Value] = ?", clause.GetClause().c_str());
    EXPECT_EQ(1, clause.GetBindings().size());
    EXPECT_EQ(BoundQueryECValue(ECValue("MyLabel")), *clause.GetBindings().at(0));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, ConvertsKnownECExpressionValueListLambdasWithEqualtyExpressionToInClause)
    {
    ECDbTestProject project;
    project.Create(BeTest::GetNameOfCurrentTest(), "RulesEngineTest.01.00.ecschema.xml");
    RefCountedPtr<TestConnection> connection = new TestConnection(project.GetECDb());

    auto nodeKeys = NavNodeKeyListContainer::Create(
        {
        ECInstancesNodeKey::Create(
            {
            ECClassInstanceKey(project.GetECDb().Schemas().GetClass("RulesEngineTest", "Widget"), ECInstanceId((uint64_t)123)),
            }, "", bvector<Utf8String>()),
        ECInstancesNodeKey::Create(
            {
            ECClassInstanceKey(project.GetECDb().Schemas().GetClass("RulesEngineTest", "Gadget"), ECInstanceId((uint64_t)456)),
            ECClassInstanceKey(project.GetECDb().Schemas().GetClass("RulesEngineTest", "Sprocket"), ECInstanceId((uint64_t)789)),
            }, "", bvector<Utf8String>()),
        NavNodeKey::Create("MyType", "", bvector<Utf8String>()),
        });

    RulesetVariables rulesetVariables;
    ECExpressionContextsProvider::ContentSpecificationInstanceFilterContextParameters contextParams(*connection,
        *nodeKeys, rulesetVariables, nullptr);
    auto expressionContext = ECExpressionContextsProvider::GetContentSpecificationInstanceFilterContext(contextParams);

    auto clause = m_helper.ConvertToECSql("SelectedInstanceKeys.AnyMatches(x => x.ECInstanceId = this.Value)", nullptr, expressionContext.get());
    EXPECT_STREQ("[this].[Value] IN (?,?,?)", clause.GetClause().c_str());
    EXPECT_EQ(3, clause.GetBindings().size());
    EXPECT_EQ(BoundQueryECValue(ECValue(123)), *clause.GetBindings().at(0));
    EXPECT_EQ(BoundQueryECValue(ECValue(456)), *clause.GetBindings().at(1));
    EXPECT_EQ(BoundQueryECValue(ECValue(789)), *clause.GetBindings().at(2));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, ConvertsKnownECExpressionValueListLambdasWithUnknownFunctionCall)
    {
    ECDbTestProject project;
    project.Create(BeTest::GetNameOfCurrentTest(), "RulesEngineTest.01.00.ecschema.xml");
    RefCountedPtr<TestConnection> connection = new TestConnection(project.GetECDb());

    auto nodeKeys = NavNodeKeyListContainer::Create(
        {
        ECInstancesNodeKey::Create(
            {
            ECClassInstanceKey(project.GetECDb().Schemas().GetClass("RulesEngineTest", "Widget"), ECInstanceId((uint64_t)123)),
            }, "", bvector<Utf8String>()),
        ECInstancesNodeKey::Create(
            {
            ECClassInstanceKey(project.GetECDb().Schemas().GetClass("RulesEngineTest", "Gadget"), ECInstanceId((uint64_t)456)),
            ECClassInstanceKey(project.GetECDb().Schemas().GetClass("RulesEngineTest", "Sprocket"), ECInstanceId((uint64_t)789)),
            }, "", bvector<Utf8String>()),
        NavNodeKey::Create("MyType", "", bvector<Utf8String>()),
        });

    RulesetVariables rulesetVariables;
    ECExpressionContextsProvider::ContentSpecificationInstanceFilterContextParameters contextParams(*connection,
        *nodeKeys, rulesetVariables, nullptr);
    auto expressionContext = ECExpressionContextsProvider::GetContentSpecificationInstanceFilterContext(contextParams);

    auto clause = m_helper.ConvertToECSql("SelectedInstanceKeys.AnyMatches(x => Eq(this.Value, x.ECInstanceId))", nullptr, expressionContext.get());
    EXPECT_STREQ("(Eq([this].[Value], ?) OR Eq([this].[Value], ?) OR Eq([this].[Value], ?))", clause.GetClause().c_str());
    EXPECT_EQ(3, clause.GetBindings().size());
    EXPECT_EQ(BoundQueryECValue(ECValue(123)), *clause.GetBindings().at(0));
    EXPECT_EQ(BoundQueryECValue(ECValue(456)), *clause.GetBindings().at(1));
    EXPECT_EQ(BoundQueryECValue(ECValue(789)), *clause.GetBindings().at(2));

    clause = m_helper.ConvertToECSql("SelectedInstanceKeys.AnyMatches(x => Eq(x.ECInstanceId, this.Value))", nullptr, expressionContext.get());
    EXPECT_STREQ("(Eq(?, [this].[Value]) OR Eq(?, [this].[Value]) OR Eq(?, [this].[Value]))", clause.GetClause().c_str());
    EXPECT_EQ(3, clause.GetBindings().size());
    EXPECT_EQ(BoundQueryECValue(ECValue(123)), *clause.GetBindings().at(0));
    EXPECT_EQ(BoundQueryECValue(ECValue(456)), *clause.GetBindings().at(1));
    EXPECT_EQ(BoundQueryECValue(ECValue(789)), *clause.GetBindings().at(2));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, ConvertsKnownECExpressionValueListLambdasWithIsOfClassFunctionAll)
    {
    ECDbTestProject project;
    project.Create(BeTest::GetNameOfCurrentTest(), "RulesEngineTest.01.00.ecschema.xml");
    RefCountedPtr<TestConnection> connection = new TestConnection(project.GetECDb());

    auto nodeKeys = NavNodeKeyListContainer::Create(
        {
        ECInstancesNodeKey::Create(
            {
            ECClassInstanceKey(project.GetECDb().Schemas().GetClass("RulesEngineTest", "Widget"), ECInstanceId((uint64_t)123)),
            }, "", bvector<Utf8String>()),
        ECInstancesNodeKey::Create(
            {
            ECClassInstanceKey(project.GetECDb().Schemas().GetClass("RulesEngineTest", "Gadget"), ECInstanceId((uint64_t)456)),
            ECClassInstanceKey(project.GetECDb().Schemas().GetClass("RulesEngineTest", "Sprocket"), ECInstanceId((uint64_t)789)),
            }, "", bvector<Utf8String>()),
        NavNodeKey::Create("MyType", "", bvector<Utf8String>()),
        });

    RulesetVariables rulesetVariables;
    ECExpressionContextsProvider::ContentSpecificationInstanceFilterContextParameters contextParams(*connection,
        *nodeKeys, rulesetVariables, nullptr);
    auto expressionContext = ECExpressionContextsProvider::GetContentSpecificationInstanceFilterContext(contextParams);

    auto clause = m_helper.ConvertToECSql("SelectedInstanceKeys.AnyMatches(x => this.IsOfClass(x.ECInstanceId))", nullptr, expressionContext.get());
    EXPECT_STREQ("(IsOfClass([this].[ECClassId], ?) OR IsOfClass([this].[ECClassId], ?) OR IsOfClass([this].[ECClassId], ?))", clause.GetClause().c_str());
    EXPECT_EQ(3, clause.GetBindings().size());
    EXPECT_EQ(BoundQueryECValue(ECValue(123)), *clause.GetBindings().at(0));
    EXPECT_EQ(BoundQueryECValue(ECValue(456)), *clause.GetBindings().at(1));
    EXPECT_EQ(BoundQueryECValue(ECValue(789)), *clause.GetBindings().at(2));
    }
