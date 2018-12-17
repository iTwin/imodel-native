/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/ECExpressionsToECSqlConverterTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
* @betest                                       Grigas.Petraitis                05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, LikeOperatorSpecialCase)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("LOWER(Test) ~ \"aaa\" OR LOWER(some_func(Label, 111)) LIKE \"Te\\_st\"");
    ASSERT_STREQ("CAST(LOWER([Test]) AS TEXT) LIKE 'aaa' ESCAPE '\\' OR CAST(LOWER(some_func([Label], 111)) AS TEXT) LIKE 'Te\\_st' ESCAPE \'\\\'", ecsql.c_str());
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
    ASSERT_STREQ("[this].[ECInstanceId] IN ("
        "SELECT [relationship].[SourceECInstanceId] "
        "FROM [TestSchema1].[RelationshipName] relationship, [TestSchema2].[RelatedClassName] related "
        "WHERE [relationship].[TargetECClassId] = [related].[ECClassId] AND [relationship].[TargetECInstanceId] = [related].[ECInstanceId]"
        ")", 
        ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionsToECSqlConverterTests, HasRelatedInstanceSpecialCase_Backward)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("this.HasRelatedInstance(\"TestSchema1:RelationshipName\", \"Backward\", \"TestSchema2:RelatedClassName\")");
    ASSERT_STREQ("[this].[ECInstanceId] IN ("
        "SELECT [relationship].[TargetECInstanceId] "
        "FROM [TestSchema1].[RelationshipName] relationship, [TestSchema2].[RelatedClassName] related "
        "WHERE [relationship].[SourceECClassId] = [related].[ECClassId] AND [relationship].[SourceECInstanceId] = [related].[ECInstanceId]"
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
TEST_F(ECExpressionsToECSqlConverterTests, ValueSetAnyMatchSpecialCase)
    {
    Utf8String ecsql = m_helper.ConvertToECSql("Set(1, 2, 3, \"4\").AnyMatch(x => x = this.SomeProperty.Id)");
    EXPECT_STREQ("[this].[SomeProperty].[Id] IN (1, 2, 3, '4')", ecsql.c_str());

    ecsql = m_helper.ConvertToECSql("Set(1, 2, 3, \"4\").AnyMatch(x => this.SomeProperty.Id = x)");
    EXPECT_STREQ("[this].[SomeProperty].[Id] IN (1, 2, 3, '4')", ecsql.c_str());
    }