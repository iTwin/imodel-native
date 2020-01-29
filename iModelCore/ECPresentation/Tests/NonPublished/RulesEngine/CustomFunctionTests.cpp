/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include "../../../Source/RulesDriven/RulesEngine/CustomFunctions.h"
#include "../../../Localization/Xliffs/ECPresentation.xliff.h"
#include "TestHelpers.h"
#include "TestLocalizationProvider.h"
#include <sstream>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2015
+===============+===============+===============+===============+===============+======*/
struct CustomFunctionTests : ECPresentationTest
    {
    static ECDbTestProject* s_project;
    static ECClassP s_widgetClass;

    IECInstancePtr m_widgetInstance;
    ECInstanceId m_widgetInstanceId;
    PresentationRuleSetPtr m_ruleset;
    TestConnectionManager m_connections;
    IConnectionCPtr m_connection;
    CustomFunctionsInjector* m_customFunctionsInjector;
    TestLocalizationProvider m_localizationProvider;
    TestUserSettings m_userSettings;
    JsonNavNodesFactory m_nodesFactory;
    ECSchemaHelper* m_schemaHelper;
    TestPropertyFormatter const* m_propertyFormatter;
    Utf8String m_locale;

    static void SetUpTestCase();
    static void TearDownTestCase();

    void SetUp() override
        {
        ECPresentationTest::SetUp();
        m_connection = m_connections.NotifyConnectionOpened(s_project->GetECDb());
        m_ruleset = PresentationRuleSet::CreateInstance("QueryExecutorTests", 1, 0, false, "", "", "", false);
        m_customFunctionsInjector = new CustomFunctionsInjector(m_connections, *m_connection);
        m_widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *s_widgetClass);
        ECInstanceId::FromString(m_widgetInstanceId, m_widgetInstance->GetInstanceId().c_str());
        m_propertyFormatter = new TestPropertyFormatter();
        m_schemaHelper = new ECSchemaHelper(*m_connection, nullptr, nullptr, nullptr);
        m_locale = "test locale";
        }

    void TearDown() override
        {
        s_project->GetECDb().AbandonChanges();
        delete m_customFunctionsInjector;
        delete m_propertyFormatter;
        delete m_schemaHelper;
        }

    ECDbR GetDb() {return s_project->GetECDb();}
    Utf8String GetDisplayLabelJson(Utf8CP value, Utf8CP displayValue = nullptr) { return LabelDefinition::Create(value, displayValue)->ToJsonString(); }
    Utf8String GetDisplayLabelJson(ECValue value, Utf8CP displayValue = nullptr) { return LabelDefinition::Create(value, displayValue)->ToJsonString(); }

    Utf8String GetJoinOptionallyQuery(Utf8CP separator, bvector<Utf8CP> parts)
        {
        Utf8String query = "SELECT " FUNCTION_NAME_JoinOptionallyRequired;
        query.append("('").append(separator).append("'");
        for (size_t i = 0; i < parts.size(); i += 2)
            {
            Utf8String value = LabelDefinition::Create(parts[i])->ToJsonString();
            Utf8CP required = parts[i + 1];

            query.append(", '").append(value).append("'");
            query.append(", ").append(required);
            }
        query.append(") FROM RET.Widget");
        return query;
        }

    Utf8String GetJoinOptionallyQueryResult(Utf8CP separator, bvector<Utf8CP> parts)
        {
        Utf8String displayLabel;
        bvector<LabelDefinitionCPtr> values;
        for (size_t i = 0; i < parts.size(); ++i)
            {
            LabelDefinitionPtr labelDefinition = LabelDefinition::Create(parts[i]);
            if (!displayLabel.empty())
                displayLabel.append(separator);
            displayLabel.append(parts[i]);
            values.push_back(labelDefinition);
            }

        std::unique_ptr<LabelDefinition::CompositeRawValue> compositeValue = std::make_unique<LabelDefinition::CompositeRawValue>(separator, values);
        return LabelDefinition::Create(displayLabel.c_str(), "composite", std::move(compositeValue))->ToJsonString();
        }
    };
ECDbTestProject* CustomFunctionTests::s_project = nullptr;
ECClassP CustomFunctionTests::s_widgetClass = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomFunctionTests::SetUpTestCase()
    {
    s_project = new ECDbTestProject();
    s_project->Create("CustomFunctionTests", "RulesEngineTest.01.00.ecschema.xml");
    s_widgetClass = const_cast<ECClassP>(CustomFunctionTests::s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomFunctionTests::TearDownTestCase()
    {
    delete CustomFunctionTests::s_project;
    CustomFunctionTests::s_project = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, Registering_DoesNotAttemptToOverrideExistingFunctions)
    {
    DbFunction* func;
    EXPECT_TRUE(GetDb().TryGetSqlFunction(func, FUNCTION_NAME_GetPointAsJsonString, -1));

    // the following injection would fail if it tried to re-register the function when there's
    // a prepared statement
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_GetPointAsJsonString "(1, 2) FROM ECDbMeta.ECClassDef"));
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());

    CustomFunctionsInjector inject(m_connections, *m_connection);
    EXPECT_TRUE(GetDb().TryGetSqlFunction(func, FUNCTION_NAME_GetPointAsJsonString, -1));

    stmt.Reset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetECInstanceDisplayLabel_UsesLabelOverride)
    {
    m_ruleset->AddPresentationRule(*new LabelOverride("", 1, "\"CustomLabel\"", "\"CustomDescription\""));

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT GetECInstanceDisplayLabel(?, ?, '', '') FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(1, s_widgetClass->GetId()));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(2, m_widgetInstanceId));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_STREQ(GetDisplayLabelJson("CustomLabel").c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetECInstanceDisplayLabel_UsesInstanceLabel)
    {
    ECClassCP classJ = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassJ");
    IECInstancePtr instanceJ = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classJ, [](IECInstanceR instance) {instance.SetValue("DisplayLabel", ECValue("CustomLabel"));});
    ECInstanceId instanceJId;
    ECInstanceId::FromString(instanceJId, instanceJ->GetInstanceId().c_str());

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT GetECInstanceDisplayLabel(?, ?, DisplayLabel, '') FROM RET.ClassJ"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(1, classJ->GetId()));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(2, instanceJId));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_STREQ(GetDisplayLabelJson("CustomLabel").c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetECInstanceDisplayLabel_UsesDefaultInstanceDisplayLabel)
    {
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT GetECInstanceDisplayLabel(?, ?, '', '') FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(1, s_widgetClass->GetId()));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(2, m_widgetInstanceId));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_STREQ(GetDisplayLabelJson(RULESENGINE_LOCALIZEDSTRING_NotSpecified.c_str()).c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetECInstanceDisplayLabel_Localizes)
    {
    m_ruleset->AddPresentationRule(*new LabelOverride("", 1, "\"@Namespace:Id@\"", ""));
    m_localizationProvider.SetHandler([](Utf8StringCR, Utf8StringCR key, Utf8StringR localized)
        {
        EXPECT_TRUE(key.Equals("Namespace:Id"));
        localized = "LocalizedLabel";
        return true;
        });

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);
    ctx.SetLocalizationProvider(m_localizationProvider);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT GetECInstanceDisplayLabel(?, ?, '', '') FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(1, s_widgetClass->GetId()));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(2, m_widgetInstanceId));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_STREQ(GetDisplayLabelJson("@Namespace:Id@", "LocalizedLabel").c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetECInstanceDisplayLabel_HandlesEmptyInstanceLabel)
    {
    ECClassCP classJ = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassJ");
    IECInstancePtr instanceJ = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classJ, [](IECInstanceR instance) {instance.SetValue("DisplayLabel", ECValue()); });
    ECInstanceId instanceJId;
    ECInstanceId::FromString(instanceJId, instanceJ->GetInstanceId().c_str());

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT GetECInstanceDisplayLabel(?, ?, DisplayLabel, '') FROM RET.ClassJ"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(1, classJ->GetId()));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(2, instanceJId));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_STREQ(GetDisplayLabelJson(RULESENGINE_LOCALIZEDSTRING_NotSpecified.c_str()).c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetECClassDisplayLabel_UsesLabelOverride)
    {
    m_ruleset->AddPresentationRule(*new LabelOverride("", 1, "\"CustomLabel\"", "\"CustomDescription\""));

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT GetECClassDisplayLabel(?, 0) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(1, s_widgetClass->GetId()));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_STREQ(GetDisplayLabelJson("CustomLabel").c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Tautvydas.Zinys                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, EvaluateECExpression)
    {
    m_localizationProvider.SetHandler([](Utf8StringCR, Utf8StringCR key, Utf8StringR localized)
        {
        EXPECT_TRUE(key.Equals("Namespace:Id"));
        localized = "LocalizedLabel";
        return true;
        });

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);
    ctx.SetLocalizationProvider(m_localizationProvider);

    m_widgetInstance->SetValue("MyID", ECValue("Test"));
    ECInstanceUpdater updater(GetDb(), *m_widgetInstance, nullptr);
    ASSERT_EQ(BE_SQLITE_OK, updater.Update(*m_widgetInstance));

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT EvaluateECExpression(ECClassId, ECInstanceId, ?) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindText(1, "this.MyID & \"Calculated_@Namespace:Id@_\" & 2", IECSqlBinder::MakeCopy::No));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_STREQ("TestCalculated_LocalizedLabel_2", stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetECClassDisplayLabel_UsesClassDisplayLabel)
    {
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT GetECClassDisplayLabel(?, 0) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(1, s_widgetClass->GetId()));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_STREQ(GetDisplayLabelJson(s_widgetClass->GetDisplayLabel().c_str()).c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetECClassDisplayLabel_Localizes)
    {
    m_ruleset->AddPresentationRule(*new LabelOverride("", 1, "\"@Namespace:Id@\"", ""));
    m_localizationProvider.SetHandler([](Utf8StringCR, Utf8StringCR key, Utf8StringR localized)
        {
        EXPECT_TRUE(key.Equals("Namespace:Id"));
        localized = "LocalizedLabel";
        return true;
        });

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);
    ctx.SetLocalizationProvider(m_localizationProvider);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT GetECClassDisplayLabel(?, 0) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(1, s_widgetClass->GetId()));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_STREQ(GetDisplayLabelJson("@Namespace:Id@", "LocalizedLabel").c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetECPropertyDisplayLabel_UsesLabelOverride)
    {
    m_ruleset->AddPresentationRule(*new LabelOverride("", 1, "\"CustomLabel\"", "\"CustomDescription\""));

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT GetECPropertyDisplayLabel(?, ?, ECInstanceId, ?, 0) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(1, s_widgetClass->GetId()));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindText(2, "DoubleProperty", IECSqlBinder::MakeCopy::No));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindDouble(3, 9.99));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_STREQ(GetDisplayLabelJson("CustomLabel").c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetECPropertyDisplayLabel_UsesPropertyValue)
    {
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);
    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT GetECPropertyDisplayLabel(?, ?, ECInstanceId, ?, 0) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(1, s_widgetClass->GetId()));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindText(2, "DoubleProperty", IECSqlBinder::MakeCopy::No));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindDouble(3, 9.99));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_STREQ(GetDisplayLabelJson(ECValue(9.99)).c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetECPropertyDisplayLabel_Formats)
    {
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr, m_propertyFormatter);
    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT GetECPropertyDisplayLabel(?, ?, ECInstanceId, ?, 0) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(1, s_widgetClass->GetId()));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindText(2, "BoolProperty", IECSqlBinder::MakeCopy::No));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindBoolean(3, false));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_STREQ(GetDisplayLabelJson(ECValue(false), "_False_").c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetECPropertyDisplayLabel_Localizes)
    {
    m_ruleset->AddPresentationRule(*new LabelOverride("", 1, "\"@Namespace:Id@\"", ""));
    m_localizationProvider.SetHandler([](Utf8StringCR, Utf8StringCR key, Utf8StringR localized)
        {
        EXPECT_TRUE(key.Equals("Namespace:Id"));
        localized = "LocalizedLabel";
        return true;
        });

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);
    ctx.SetLocalizationProvider(m_localizationProvider);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT GetECPropertyDisplayLabel(?, ?, ECInstanceId, ?, 0) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(1, s_widgetClass->GetId()));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindText(2, "DoubleProperty", IECSqlBinder::MakeCopy::No));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindDouble(3, 9.99));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_STREQ(GetDisplayLabelJson("@Namespace:Id@", "LocalizedLabel").c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetECPropertyDisplayLabel_GroupingByNullValue_LabelIsLocalizedNotSpecified)
    {
    PropertyGroup spec("", "", true, "MyID");
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT GetECPropertyDisplayLabel(?, ?, ECInstanceId, ?, 0) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(1, s_widgetClass->GetId()));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindText(2, "MyID", IECSqlBinder::MakeCopy::No));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindNull(3));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_STREQ(GetDisplayLabelJson(RULESENGINE_LOCALIZEDSTRING_NotSpecified.c_str()).c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetECPropertyDisplayLabel_RangeBased_ReturnsOtherRange)
    {
    PropertyGroup spec("", "", true, "DoubleProperty");
    spec.AddRange(*new PropertyRangeGroupSpecification("One", "", "1", "5"));
    spec.AddRange(*new PropertyRangeGroupSpecification("Two", "", "6", "9"));

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    ECPropertyCR groupingProperty = *s_widgetClass->GetPropertyP("DoubleProperty");
    NavigationQueryExtendedData(*query).AddRangesData(groupingProperty, spec);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT GetECPropertyDisplayLabel(?, ?, ECInstanceId, ?, 0) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(1, s_widgetClass->GetId()));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindText(2, "DoubleProperty", IECSqlBinder::MakeCopy::No));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindDouble(3, 9.99));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_STREQ(GetDisplayLabelJson(RULESENGINE_LOCALIZEDSTRING_Other.c_str()).c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetECPropertyDisplayLabel_RangeBased_ReturnsRangeLabel)
    {
    PropertyGroup spec("", "", true, "DoubleProperty");
    spec.AddRange(*new PropertyRangeGroupSpecification("One", "", "1", "5"));
    spec.AddRange(*new PropertyRangeGroupSpecification("Two", "", "6", "9"));

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    ECPropertyCR groupingProperty = *s_widgetClass->GetPropertyP("DoubleProperty");
    NavigationQueryExtendedData(*query).AddRangesData(groupingProperty, spec);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT GetECPropertyDisplayLabel(?, ?, ECInstanceId, ?, 0) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(1, s_widgetClass->GetId()));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindText(2, "DoubleProperty", IECSqlBinder::MakeCopy::No));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindDouble(3, 6.66));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_STREQ(GetDisplayLabelJson("Two").c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetECPropertyDisplayLabel_RangeBased_ReturnsRangeValues)
    {
    PropertyGroup spec("", "", true, "DoubleProperty");
    spec.AddRange(*new PropertyRangeGroupSpecification("", "", "1", "5"));
    spec.AddRange(*new PropertyRangeGroupSpecification("", "", "6", "9"));

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    ECPropertyCR groupingProperty = *s_widgetClass->GetPropertyP("DoubleProperty");
    NavigationQueryExtendedData(*query).AddRangesData(groupingProperty, spec);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT GetECPropertyDisplayLabel(?, ?, ECInstanceId, ?, 0) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(1, s_widgetClass->GetId()));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindText(2, "DoubleProperty", IECSqlBinder::MakeCopy::No));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindDouble(3, 6.66));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_STREQ(GetDisplayLabelJson("6.00 - 9.00").c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetSortingValue_PadsInteger)
    {
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_GetSortingValue " (?) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindInt(1, 1));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_STREQ("0000000001", stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetSortingValue_PadsDouble)
    {
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_GetSortingValue " (?) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindDouble(1, 1.11));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_STREQ("0000000001.0000000011", stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetSortingValue_PadsText)
    {
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_GetSortingValue " (?) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindText(1, GetDisplayLabelJson("1").c_str(), IECSqlBinder::MakeCopy::Yes));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_STREQ("0000000001", stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetSortingValue_PadsComplexText)
    {
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_GetSortingValue " (?) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindText(1, GetDisplayLabelJson("a*123-b^45_c6d").c_str(), IECSqlBinder::MakeCopy::Yes));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_STREQ("a*0000000123-b^0000000045_c0000000006d", stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetSortingValue_LowerCasesText)
    {
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_GetSortingValue " (?) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindText(1, GetDisplayLabelJson("ABC1 abc2 Abc3 aBC4").c_str(), IECSqlBinder::MakeCopy::Yes));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_STREQ("abc0000000001 abc0000000002 abc0000000003 abc0000000004", stmt.GetValueText(0));
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2015
+===============+===============+===============+===============+===============+======*/
struct CustomAttributeSetter
    {
    ECClassR m_class;
    IECInstanceR m_attribute;
    CustomAttributeSetter(ECClassR ecClass, IECInstanceR attribute) : m_class(ecClass), m_attribute(attribute) {ecClass.SetCustomAttribute(attribute);}
    ~CustomAttributeSetter() {m_class.RemoveCustomAttribute(m_attribute.GetClass());}
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetRangeIndex_PropertyValueDoesntMatchAnyRange)
    {
    PropertyGroup spec("", "", true, "DoubleProperty");
    spec.AddRange(*new PropertyRangeGroupSpecification("One", "", "1", "5"));
    spec.AddRange(*new PropertyRangeGroupSpecification("Two", "", "6", "9"));

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    ECPropertyCR groupingProperty = *s_widgetClass->GetPropertyP("DoubleProperty");
    NavigationQueryExtendedData(*query).AddRangesData(groupingProperty, spec);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT GetRangeIndex(?) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindDouble(1, 0));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_EQ(-1, stmt.GetValueInt(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetRangeIndex_ReturnsValidRangeIndex)
    {
    PropertyGroup spec("", "", true, "DoubleProperty");
    spec.AddRange(*new PropertyRangeGroupSpecification("One", "", "1", "5"));
    spec.AddRange(*new PropertyRangeGroupSpecification("Two", "", "6", "9"));

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    ECPropertyCR groupingProperty = *s_widgetClass->GetPropertyP("DoubleProperty");
    NavigationQueryExtendedData(*query).AddRangesData(groupingProperty, spec);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT GetRangeIndex(?) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindDouble(1, 6));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_EQ(1, stmt.GetValueInt(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetRangeImageId_ReturnsEmptyStringIfValueDoesntMatchAnyRange)
    {
    PropertyGroup spec("", "", true, "DoubleProperty");
    spec.AddRange(*new PropertyRangeGroupSpecification("One", "Image1", "1", "5"));
    spec.AddRange(*new PropertyRangeGroupSpecification("Two", "Image2", "6", "9"));

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    ECPropertyCR groupingProperty = *s_widgetClass->GetPropertyP("DoubleProperty");
    NavigationQueryExtendedData(*query).AddRangesData(groupingProperty, spec);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT GetRangeImageId(?) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindDouble(1, 0));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_STREQ("", stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetRangeImageId_ReturnsRangeImageIdIfValueMatches)
    {
    PropertyGroup spec("", "", true, "DoubleProperty");
    spec.AddRange(*new PropertyRangeGroupSpecification("One", "Image1", "1", "5"));
    spec.AddRange(*new PropertyRangeGroupSpecification("Two", "Image2", "6", "9"));

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    ECPropertyCR groupingProperty = *s_widgetClass->GetPropertyP("DoubleProperty");
    NavigationQueryExtendedData(*query).AddRangesData(groupingProperty, spec);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT GetRangeImageId(?) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindDouble(1, 5));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_STREQ("Image1", stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, IsOfClass)
    {
    ECClassCP classF = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassF");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT IsOfClass(ECClassId, 'ClassF', 'RulesEngineTest') FROM RET.ClassF"));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_TRUE(stmt.GetValueBoolean(0));

    stmt.Finalize();
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT IsOfClass(ECClassId, 'ClassE', 'RulesEngineTest') FROM RET.ClassF"));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_TRUE(stmt.GetValueBoolean(0));

    stmt.Finalize();
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT IsOfClass(ECClassId, 'ClassD', 'RulesEngineTest') FROM RET.ClassF"));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_FALSE(stmt.GetValueBoolean(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetGroupedInstanceIds)
    {
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);
    ECSqlStatement stmt;

    // verify it works with one input value
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_GetGroupedInstanceKeys "(ECClassId, ECInstanceId) FROM RET.Widget"));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    Utf8CP value1 = stmt.GetValue(0).GetText();
    ASSERT_TRUE(nullptr != value1);
    ASSERT_TRUE(0 != *value1);

    rapidjson::Document json1;
    json1.Parse(value1);
    ASSERT_TRUE(json1.IsArray());
    ASSERT_EQ(1, json1.Size());
    ASSERT_EQ(s_widgetClass->GetId().GetValue(), json1[0]["c"].GetInt64());
    ASSERT_EQ(m_widgetInstanceId.GetValue(), json1[0]["i"].GetInt64());

    // verify it works with multiple input values
    IECInstancePtr newWidget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *s_widgetClass);
    ECInstanceId newWidgetId;
    ECInstanceId::FromString(newWidgetId, newWidget->GetInstanceId().c_str());

    stmt.Finalize();
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_GetGroupedInstanceKeys "(ECClassId, ECInstanceId) FROM RET.Widget"));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    Utf8CP value2 = stmt.GetValue(0).GetText();
    ASSERT_TRUE(nullptr != value2);
    ASSERT_TRUE(0 != *value2);

    rapidjson::Document json2;
    json2.Parse(value2);
    ASSERT_TRUE(json2.IsArray());
    ASSERT_EQ(2, json2.Size());
    ASSERT_EQ(s_widgetClass->GetId().GetValue(), json2[1]["c"].GetInt64());
    ASSERT_EQ(newWidgetId.GetValue(), json2[1]["i"].GetInt64());

    // verify it works with null input value
    stmt.Finalize();
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_GetGroupedInstanceKeys "(0) FROM RET.Widget"));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    Utf8CP value3 = stmt.GetValue(0).GetText();
    ASSERT_TRUE(nullptr != value3);
    ASSERT_TRUE(0 != *value3);

    rapidjson::Document json3;
    json3.Parse(value3);
    ASSERT_TRUE(json3.IsArray());
    ASSERT_EQ(0, json3.Size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetVariableStringValue)
    {
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_GetVariableStringValue "('test') FROM RET.Widget"));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_STREQ("", stmt.GetValueText(0));

    m_userSettings.SetSettingValue("test", "test value");
    stmt.Finalize();
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_GetVariableStringValue "('test') FROM RET.Widget"));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_STREQ("test value", stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetVariableIntValue)
    {
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_GetVariableIntValue "('test') FROM RET.Widget"));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_EQ(0, stmt.GetValueInt(0));

    m_userSettings.SetSettingIntValue("test", 999);
    stmt.Finalize();
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_GetVariableIntValue "('test') FROM RET.Widget"));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_EQ(999, stmt.GetValueInt(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetVariableBoolValue)
    {
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_GetVariableBoolValue "('test') FROM RET.Widget"));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_FALSE(stmt.GetValueBoolean(0));

    m_userSettings.SetSettingBoolValue("test", true);
    stmt.Finalize();
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_GetVariableBoolValue "('test') FROM RET.Widget"));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_TRUE(stmt.GetValueBoolean(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, InVariableIntValues)
    {
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_InVariableIntValues "('test', 2) FROM RET.Widget"));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_FALSE(stmt.GetValueBoolean(0));

    m_userSettings.SetSettingIntValues("test", {1, 2, 3});
    ctx._OnSettingChanged(m_ruleset->GetRuleSetId().c_str(), "test");

    stmt.Reset();
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_TRUE(stmt.GetValueBoolean(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, HasVariable)
    {
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_HasVariable "('test') FROM RET.Widget"));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_FALSE(stmt.GetValueBoolean(0));

    m_userSettings.SetSettingBoolValue("test", true);
    stmt.Reset();
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_TRUE(stmt.GetValueBoolean(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetECClassId)
    {
    ECClassCP widgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *widgetClass);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_GetECClassId "('Widget', 'RulesEngineTest') FROM RET.Widget"));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_EQ(widgetClass->GetId().GetValue(), stmt.GetValueInt64(0));

    stmt.Finalize();
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_GetECClassId "('DoesNotExist', 'RulesEngineTest') FROM RET.Widget"));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    ASSERT_EQ(0, stmt.GetValueInt64(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetECClassName)
    {
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), Utf8PrintfString("SELECT " FUNCTION_NAME_GetECClassName "(%" PRIu64 ", FALSE) FROM RET.Widget", s_widgetClass->GetId().GetValue()).c_str()));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_STREQ(GetDisplayLabelJson(s_widgetClass->GetName().c_str()).c_str(), stmt.GetValueText(0));

    stmt.Finalize();
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), Utf8PrintfString("SELECT " FUNCTION_NAME_GetECClassName "(%" PRIu64 ", TRUE) FROM RET.Widget", s_widgetClass->GetId().GetValue()).c_str()));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_STREQ(GetDisplayLabelJson(s_widgetClass->GetFullName()).c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetECClassLabel)
    {
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), Utf8PrintfString("SELECT " FUNCTION_NAME_GetECClassLabel "(%" PRIu64 ") FROM RET.Widget", s_widgetClass->GetId().GetValue()).c_str()));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_STREQ(GetDisplayLabelJson(s_widgetClass->GetDisplayLabel().c_str()).c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetMergedValue)
    {
    ECClassCP widgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue()); // null
        instance.SetValue("Description", ECValue(""));
        instance.SetValue("IntProperty", ECValue(5));
        instance.SetValue("BoolProperty", ECValue(true));
        instance.SetValue("DoubleProperty", ECValue()); // null
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue(""));
        instance.SetValue("Description", ECValue(""));
        instance.SetValue("IntProperty", ECValue(5));
        instance.SetValue("BoolProperty", ECValue(false));
        instance.SetValue("DoubleProperty", ECValue()); // null
        });

    Utf8String defaultVariesStrLocalizationId = PRESENTATION_LOCALIZEDSTRING(RulesEngineL10N::GetNameSpace().m_namespace, RulesEngineL10N::LABEL_General_Varies().m_str);
    Utf8PrintfString formattedVariesStr(CONTENTRECORD_MERGED_VALUE_FORMAT, defaultVariesStrLocalizationId.c_str());

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    Utf8CP query = "SELECT " FUNCTION_NAME_GetMergedValue "(MyID), "
                             FUNCTION_NAME_GetMergedValue "(Description), "
                             FUNCTION_NAME_GetMergedValue "(IntProperty), "
                             FUNCTION_NAME_GetMergedValue "(BoolProperty, 'Test @LocalizationId@'), "
                             FUNCTION_NAME_GetMergedValue "(DoubleProperty) "
                   "  FROM RET.Widget "
                   " WHERE ECInstanceId <> ?";

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(1, m_widgetInstanceId));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());

    EXPECT_STREQ(formattedVariesStr.c_str(), stmt.GetValueText(0));
    EXPECT_STREQ("", stmt.GetValueText(1));
    EXPECT_STREQ("5", stmt.GetValueText(2));
    EXPECT_STREQ("Test @LocalizationId@", stmt.GetValueText(3));
    EXPECT_TRUE(stmt.IsValueNull(4));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, IsRecursivelyRelated_BackwardDirection)
    {
    ECEntityClassCP classN = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassN")->GetEntityClassCP();
    ECRelationshipClassCP relationshipClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassNGroupsClassN")->GetRelationshipClassCP();

    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(2));});
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(3));});
    IECInstancePtr instance4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(4));});
    IECInstancePtr instance5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *instance1, *instance2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *instance2, *instance3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *instance2, *instance4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *instance4, *instance5);

    /*
    Relationship hierarchy:
            1
            |
            2
          /   \
         3     4
               |
               5
    */

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    Utf8CP query = "SELECT e.IntProperty, r.IntProperty "
                   "  FROM RET.ClassN e, RET.ClassN r "
                   " WHERE " FUNCTION_NAME_IsRecursivelyRelated "('RulesEngineTest:ClassNGroupsClassN', 'Backward', "
                                                                " e.ECClassId, e.ECInstanceId, r.ECClassId, r.ECInstanceId)";

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query));

    bmap<int, bvector<int>> actualResults;
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        actualResults[stmt.GetValueInt(0)].push_back(stmt.GetValueInt(1));

    bmap<int, bvector<int>> expectedResults;
    expectedResults[2] = {1};
    expectedResults[3] = {1, 2};
    expectedResults[4] = {1, 2};
    expectedResults[5] = {1, 2, 4};

    EXPECT_EQ(expectedResults, actualResults);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, IsRecursivelyRelated_ForwardDirection)
    {
    ECEntityClassCP classN = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassN")->GetEntityClassCP();
    ECRelationshipClassCP relationshipClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassNGroupsClassN")->GetRelationshipClassCP();

    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(2));});
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(3));});
    IECInstancePtr instance4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(4));});
    IECInstancePtr instance5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *instance1, *instance2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *instance2, *instance3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *instance2, *instance4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *instance4, *instance5);

    /*
    Relationship hierarchy:
            1
            |
            2
          /   \
         3     4
               |
               5
    */

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    Utf8CP query = "SELECT e.IntProperty, r.IntProperty "
                   "  FROM RET.ClassN e, RET.ClassN r "
                   " WHERE " FUNCTION_NAME_IsRecursivelyRelated "('RulesEngineTest:ClassNGroupsClassN', 'Forward', "
                                                                " e.ECClassId, e.ECInstanceId, r.ECClassId, r.ECInstanceId)";

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query));

    bmap<int, bvector<int>> actualResults;
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        actualResults[stmt.GetValueInt(0)].push_back(stmt.GetValueInt(1));

    bmap<int, bvector<int>> expectedResults;
    expectedResults[1] = {2, 3, 4, 5};
    expectedResults[2] = {3, 4, 5};
    expectedResults[4] = {5};

    EXPECT_EQ(expectedResults, actualResults);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, IsRecursivelyRelated_FindsRelatedInstances)
    {
    ECEntityClassCP classN = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassN")->GetEntityClassCP();
    ECRelationshipClassCP relationshipClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassNGroupsClassN")->GetRelationshipClassCP();

    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(2));});
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(3));});
    IECInstancePtr instance4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(4));});
    IECInstancePtr instance5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *instance1, *instance2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *instance2, *instance3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *instance2, *instance4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *instance4, *instance5);

    /*
    Relationship hierarchy:
            1
            |
            2
          /   \
         3     4
               |
               5
    */

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    Utf8CP query = "SELECT e.IntProperty "
                   "  FROM RET.ClassN e, RET.ClassN r "
                   " WHERE " FUNCTION_NAME_IsRecursivelyRelated "('RulesEngineTest:ClassNGroupsClassN', 'Backward', "
                                                                " e.ECClassId, e.ECInstanceId, r.ECClassId, r.ECInstanceId) "
                           "AND r.ECInstanceId = ?";

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindText(1, instance2->GetInstanceId().c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::Yes));

    bvector<int> actualResults;
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        actualResults.push_back(stmt.GetValueInt(0));

    bvector<int> expectedResults = {3, 4, 5};
    EXPECT_EQ(expectedResults, actualResults);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetECEnumerationValue)
    {
    ECClassCP classQ = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassQ");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classQ, [](IECInstanceR instance)
        {
        instance.SetValue("StrEnum", ECValue("Two"));
        instance.SetValue("IntEnum", ECValue(3));
        });

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    Utf8CP query = "SELECT " FUNCTION_NAME_GetECEnumerationValue "('RulesEngineTest', 'StringEnum', StrEnum), "
                             FUNCTION_NAME_GetECEnumerationValue "('RulesEngineTest', 'IntegerEnum', IntEnum) "
                   "  FROM RET.ClassQ ";

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_STREQ(GetDisplayLabelJson("2").c_str(), stmt.GetValueText(0));
    EXPECT_STREQ(GetDisplayLabelJson("A").c_str(), stmt.GetValueText(1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, ECInstanceKeyArray)
    {
    ECClassCP gadgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Gadget");
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *gadgetClass);
    ECInstanceId gadgetId;
    ECInstanceId::FromString(gadgetId, gadget->GetInstanceId().c_str());

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    Utf8CP query = "SELECT " FUNCTION_NAME_ECInstanceKeysArray "(ECClassId, ECInstanceId) "
                   "  FROM RET.Gadget ";

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());

    rapidjson::Document expectedJson;
    expectedJson.SetArray();
    rapidjson::Value gadget1Json;
    gadget1Json.SetObject();
    gadget1Json.AddMember("c", gadgetClass->GetId().GetValue(), expectedJson.GetAllocator());
    gadget1Json.AddMember("i", gadgetId.GetValue(), expectedJson.GetAllocator());
    expectedJson.PushBack(gadget1Json, expectedJson.GetAllocator());

    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    expectedJson.Accept(writer);

    EXPECT_STREQ(buf.GetString(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, AggregateJsonArray)
    {
    ECClassCP gadgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Gadget");
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *gadgetClass);
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *gadgetClass);
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    Utf8CP query = "SELECT " FUNCTION_NAME_AggregateJsonArray "('[{\"ECInstanceId\": ' || CAST(ECInstanceId AS TEXT) || '}]') "
                   "  FROM RET.Gadget ";

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());

    Utf8String expected = Utf8String("[{\"ECInstanceId\": ").append(gadget1->GetInstanceId());
    expected.append("},{\"ECInstanceId\": ").append(gadget2->GetInstanceId()).append("}]");
    EXPECT_STREQ(expected.c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetPointAsJsonString)
    {
    ECClassCP classH = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassH");
    IECInstancePtr instanceH = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance)
        {
        instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.512, 1.512, 1.512)));
        });
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    Utf8CP query = "SELECT " FUNCTION_NAME_GetPointAsJsonString "([PointProperty].x, [PointProperty].y, [PointProperty].z) "
                   "  FROM RET.ClassH ";

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());

    Utf8String expected;
    expected.Sprintf("{\"x\":%g,\"y\":%g,\"z\":%g}", 1.512, 1.512, 1.512);
    EXPECT_STREQ(expected.c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetPropertyDisplayValue_Point2d)
    {
    ECClassCP classH = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassH");
    IECInstancePtr instanceH = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr, m_propertyFormatter);
    Utf8CP query = "SELECT " FUNCTION_NAME_GetPropertyDisplayValue "(?, ?, ?, '{\"x\":1.512,\"y\":1.512}') "
                   "  FROM RET.ClassH h";

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindText(1, classH->GetSchema().GetName().c_str(), IECSqlBinder::MakeCopy::No));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindText(2, classH->GetName().c_str(), IECSqlBinder::MakeCopy::No));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindText(3, "Point2dProperty", IECSqlBinder::MakeCopy::Yes));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_STREQ("_1.512,1.512_", stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetPropertyDisplayValue_Point3d)
    {
    ECClassCP classH = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassH");
    IECInstancePtr instanceH = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr, m_propertyFormatter);
    Utf8CP query = "SELECT " FUNCTION_NAME_GetPropertyDisplayValue "(?, ?, ?, '{\"x\":1.512,\"y\":1.512,\"z\":1.512}') "
                   "  FROM RET.ClassH h";

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindText(1, classH->GetSchema().GetName().c_str(), IECSqlBinder::MakeCopy::No));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindText(2, classH->GetName().c_str(), IECSqlBinder::MakeCopy::No));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindText(3, "PointProperty", IECSqlBinder::MakeCopy::Yes));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_STREQ("_1.512,1.512,1.512_", stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetPropertyDisplayValue_Double)
    {
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr, m_propertyFormatter);
    Utf8CP query = "SELECT " FUNCTION_NAME_GetPropertyDisplayValue "(?, ?, ?, 1.123456789) "
                   "  FROM RET.Widget";

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindText(1, s_widgetClass->GetSchema().GetName().c_str(), IECSqlBinder::MakeCopy::No));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindText(2, s_widgetClass->GetName().c_str(), IECSqlBinder::MakeCopy::No));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindText(3, "DoubleProperty", IECSqlBinder::MakeCopy::Yes));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_STREQ("_1.123456789_", stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, Are3dPointsEqualByValue_ReturnsTrue)
    {
    ECClassCP classH = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassH");
    IECInstancePtr instanceH = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance)
        {
        instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.512, 1.512, 1.512)));
        });

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);
    Utf8CP query = "SELECT " FUNCTION_NAME_ArePointsEqualByValue "('{\"x\":1.512,\"y\":1.512,\"z\":1.512}', [PointProperty].x, [PointProperty].y, [PointProperty].z)"
                   "  FROM RET.ClassH";

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_EQ(1, stmt.GetValueInt(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, Are3dPointsEqualByValue_ReturnsFalse)
    {
    ECClassCP classH = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassH");
    IECInstancePtr instanceH = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance)
        {
        instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.512, 1.512, 1.512)));
        });

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);
    Utf8CP query = "SELECT " FUNCTION_NAME_ArePointsEqualByValue "('{\"x\":1.51200000012,\"y\":1.512,\"z\":1.512}', [PointProperty].x, [PointProperty].y, [PointProperty].z)"
                   "  FROM RET.ClassH";

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_EQ(0, stmt.GetValueInt(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, Are2dPointsEqualByValue_ReturnsTrue)
    {
    ECClassCP classH = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassH");
    IECInstancePtr instanceH = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance)
        {
        instance.SetValue("Point2dProperty", ECValue(DPoint2d::From(1.512, 1.512)));
        });

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);
    Utf8CP query = "SELECT " FUNCTION_NAME_ArePointsEqualByValue "('{\"x\":1.512,\"y\":1.512}', [Point2dProperty].x, [Point2dProperty].y)"
                   "  FROM RET.ClassH";

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_EQ(1, stmt.GetValueInt(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, Are2dPointsEqualByValue_ReturnsFalse)
    {
    ECClassCP classH = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassH");
    IECInstancePtr instanceH = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance)
        {
        instance.SetValue("Point2dProperty", ECValue(DPoint2d::From(1.512, 1.512)));
        });

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);
    Utf8CP query = "SELECT " FUNCTION_NAME_ArePointsEqualByValue "('{\"x\":1.51200000012,\"y\":1.512}', [Point2dProperty].x, [Point2dProperty].y)"
                   "  FROM RET.ClassH";

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_EQ(0, stmt.GetValueInt(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, AreDoublesEqualByValue_ReturnsTrue)
    {
    m_widgetInstance->SetValue("DoubleProperty", ECValue(1.123456789));
    ECInstanceUpdater updater(GetDb(), *m_widgetInstance, nullptr);
    ASSERT_EQ(BE_SQLITE_OK, updater.Update(*m_widgetInstance));

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);
    Utf8CP query = "SELECT " FUNCTION_NAME_AreDoublesEqualByValue "(1.123456789, DoubleProperty)"
                   "  FROM RET.Widget";

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_EQ(1, stmt.GetValueInt(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, AreDoublesEqualByValue_ReturnsFalse)
    {
    m_widgetInstance->SetValue("DoubleProperty", ECValue(1.123456788));
    ECInstanceUpdater updater(GetDb(), *m_widgetInstance, nullptr);
    ASSERT_EQ(BE_SQLITE_OK, updater.Update(*m_widgetInstance));

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);
    Utf8CP query = "SELECT " FUNCTION_NAME_AreDoublesEqualByValue "(1.123456789, DoubleProperty)"
                   "  FROM RET.Widget";

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_EQ(0, stmt.GetValueInt(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetNavigationPropertyLabel_UsesInstanceLabelOverride)
    {
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *s_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("WidgetID"));});

    ECInstanceId widgetInstanceID;
    ECInstanceId::FromString(widgetInstanceID, widgetInstance->GetInstanceId().c_str());
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_GetNavigationPropertyLabel "(?, ?) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(1, s_widgetClass->GetId()));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(2, widgetInstanceID));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_STREQ(GetDisplayLabelJson("WidgetID").c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetNavigationPropertyLabel_UsesLabelOverride)
    {
    m_ruleset->AddPresentationRule(*new LabelOverride("", 1, "this.MyID", ""));

    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *s_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("WidgetID")); });

    ECInstanceId widgetInstanceID;
    ECInstanceId::FromString(widgetInstanceID, widgetInstance->GetInstanceId().c_str());
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_GetNavigationPropertyLabel "(?, ?) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(1, s_widgetClass->GetId()));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(2, widgetInstanceID));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_STREQ(GetDisplayLabelJson("WidgetID").c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetNavigationPropertyLabel_ReturnsNotSpecifiedIfLabelOverridesDoNotApply)
    {
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_GetNavigationPropertyLabel "(?, ?) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(1, s_widgetClass->GetId()));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(2, m_widgetInstanceId));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_STREQ(GetDisplayLabelJson(RULESENGINE_LOCALIZEDSTRING_NotSpecified.c_str()).c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetRelatedDisplayLabel_UsesInstanceLabelOverride)
    {
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *s_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("WidgetID"));});

    ECInstanceId widgetInstanceID;
    ECInstanceId::FromString(widgetInstanceID, widgetInstance->GetInstanceId().c_str());
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_GetRelatedDisplayLabel "(?, ?) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(1, s_widgetClass->GetId()));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(2, widgetInstanceID));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_STREQ(GetDisplayLabelJson("WidgetID").c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetRelatedDisplayLabel_UsesLabelOverride)
    {
    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *s_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("WidgetID"));});

    ECInstanceId widgetInstanceID;
    ECInstanceId::FromString(widgetInstanceID, widgetInstance->GetInstanceId().c_str());
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_GetRelatedDisplayLabel "(?, ?) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(1, s_widgetClass->GetId()));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(2, widgetInstanceID));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_STREQ(GetDisplayLabelJson("WidgetID").c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, GetRelatedDisplayLabel_NoRule_NotSpecifiedStringReturned)
    {
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_GetRelatedDisplayLabel "(?, ?) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(1, s_widgetClass->GetId()));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindId(2, m_widgetInstanceId));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_STREQ(GetDisplayLabelJson(RULESENGINE_LOCALIZEDSTRING_NotSpecified.c_str()).c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, ToBase36)
    {
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_ToBase36 "(?) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindText(1, "{\"DisplayValue\":\"13368\",\"RawValue\":13368,\"TypeName\":\"uint64\"}", IECSqlBinder::MakeCopy::Yes));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_STREQ(GetDisplayLabelJson("ABC").c_str(), stmt.GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, JoinOptionallyRequired)
    {
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);

    ECSqlStatement stmt;
    Utf8String query = GetJoinOptionallyQuery("-", bvector<Utf8CP>());
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query.c_str()));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_TRUE(stmt.IsValueNull(0));

    stmt.Finalize();
    query = GetJoinOptionallyQuery(" ", { "a", "FALSE", "b", "FALSE", "c", "FALSE" });
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query.c_str()));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_STREQ(GetJoinOptionallyQueryResult(" ", {"a", "b", "c"}).c_str(), stmt.GetValueText(0));

    stmt.Finalize();
    query = GetJoinOptionallyQuery("/", { "", "FALSE", "b", "FALSE", "c", "FALSE" });
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query.c_str()));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_STREQ(GetJoinOptionallyQueryResult("/", { "b", "c" }).c_str(), stmt.GetValueText(0));

    stmt.Finalize();
    query = GetJoinOptionallyQuery("_", { "a", "FALSE", "b", "FALSE", "", "False" });
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query.c_str()));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_STREQ(GetJoinOptionallyQueryResult("_", { "a", "b" }).c_str(), stmt.GetValueText(0));

    stmt.Finalize();
    query = GetJoinOptionallyQuery("", { "a", "FALSE", "", "FALSE", "c", "FALSE" });
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query.c_str()));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_STREQ(GetJoinOptionallyQueryResult("", { "a", "c" }).c_str(), stmt.GetValueText(0));

    stmt.Finalize();
    query = GetJoinOptionallyQuery("-", { "a", "TRUE", "b", "TRUE", "c", "TRUE" });
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query.c_str()));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_STREQ(GetJoinOptionallyQueryResult("-", { "a", "b", "c" }).c_str(), stmt.GetValueText(0));

    stmt.Finalize();
    query = GetJoinOptionallyQuery("-", { "", "TRUE", "b", "TRUE", "c", "TRUE" });
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query.c_str()));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_TRUE(stmt.IsValueNull(0));

    stmt.Finalize();
    query = GetJoinOptionallyQuery("-", { "a", "FALSE", "", "TRUE", "c", "FALSE" });
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), query.c_str()));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_TRUE(stmt.IsValueNull(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, CompareDoubles_ComparesPropertyWithStaticValue)
    {
    const double preciseValue = 9.49772474999999616329e+00;
    const double roundedValue = 9.4977247499999961633e+00;

    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *s_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *s_widgetClass, [&](IECInstanceR instance)
        {
        instance.SetValue("DoubleProperty", ECValue(preciseValue));
        });

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);
    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_CompareDoubles "(DoubleProperty, ?) FROM RET.Widget"));
    ASSERT_TRUE(ECSqlStatus::Success == stmt.BindDouble(1, roundedValue));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_EQ(0, stmt.GetValueInt(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionTests, CompareDoubles_ReturnsNon0WhenComparingNullPropertyAnd0)
    {
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);
    ECSqlStatement stmt;
    ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(GetDb(), "SELECT " FUNCTION_NAME_CompareDoubles "(DoubleProperty, 0.0) FROM RET.Widget"));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_NE(0, stmt.GetValueInt(0));
    }

/*=================================================================================**//**
* @bsiclass                                     Mantas.Kontrimas                06/2018
+===============+===============+===============+===============+===============+======*/
struct CustomFunctionsManagerTests : CustomFunctionTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionsManagerTests, ReturnsDifferentContextsOnDifferentThreads)
    {
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);
    ASSERT_EQ(&CustomFunctionsManager::GetManager().GetCurrentContext(), &ctx);

    std::thread([&]()
        {
        ASSERT_TRUE(CustomFunctionsManager::GetManager().IsContextEmpty());

        CustomFunctionsContext ctx2(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, m_locale, m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, nullptr);
        ASSERT_EQ(&CustomFunctionsManager::GetManager().GetCurrentContext(), &ctx2);
        }).join();

    // Check if another thread context is not visible
    ASSERT_EQ(&CustomFunctionsManager::GetManager().GetCurrentContext(), &ctx);
    }

/*=================================================================================**//**
* @bsiclass                                     Mantas.Kontrimas                07/2018
+===============+===============+===============+===============+===============+======*/
struct CustomFunctionsInjectorTests : ECPresentationTest
    {
    static ECDbTestProject* s_project;
    ConnectionManager m_connections;
    CustomFunctionsInjector* m_customFunctionsInjector;

    void SetUp() override
        {
        ECPresentationTest::SetUp();
        m_customFunctionsInjector = new CustomFunctionsInjector(m_connections);
        }

    void TearDown() override
        {
        delete m_customFunctionsInjector;
        }

    static void SetUpTestCase()
        {
        s_project = new ECDbTestProject();
        s_project->Create("CustomFunctionTests", "RulesEngineTest.01.00.ecschema.xml");
        }

    static void TearDownTestCase()
        {
        DELETE_AND_CLEAR(s_project);
        }
    };
ECDbTestProject* CustomFunctionsInjectorTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomFunctionsInjectorTests, MultipleConnectionsAreStoredWhenMultipleThreadsAreUsed)
    {
    IConnectionCPtr primaryConnection = m_connections.CreateConnection(s_project->GetECDb());
    IConnectionCP firstProxyConnection;
    IConnectionCP secondProxyConnection;
    ASSERT_TRUE(m_customFunctionsInjector->Handles(*primaryConnection));

    std::thread([&]()
        {
        firstProxyConnection = m_connections.GetConnection(primaryConnection->GetId().c_str());
        ASSERT_TRUE(m_customFunctionsInjector->Handles(*firstProxyConnection));
        }).join();

    std::thread([&]()
        {
        secondProxyConnection = m_connections.GetConnection(primaryConnection->GetId().c_str());
        ASSERT_TRUE(m_customFunctionsInjector->Handles(*secondProxyConnection));
        }).join();

    ASSERT_TRUE(m_customFunctionsInjector->Handles(*primaryConnection));
    ASSERT_TRUE(m_customFunctionsInjector->Handles(*firstProxyConnection));
    ASSERT_TRUE(m_customFunctionsInjector->Handles(*secondProxyConnection));

    s_project->GetECDb().CloseDb();

    ASSERT_FALSE(m_customFunctionsInjector->Handles(*primaryConnection));
    ASSERT_FALSE(m_customFunctionsInjector->Handles(*firstProxyConnection));
    ASSERT_FALSE(m_customFunctionsInjector->Handles(*secondProxyConnection));
    }