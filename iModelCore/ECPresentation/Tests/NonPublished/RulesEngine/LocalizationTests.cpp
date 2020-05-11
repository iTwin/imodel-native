/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include <BeSQLite/L10N.h>
#include "../../../Source/RulesDriven/RulesEngine/NavNodeProviders.h"
#include "../../../Source/RulesDriven/RulesEngine/LocalizationHelper.h"
#include "TestHelpers.h"
#include "TestLocalizationProvider.h"
#include "NodesProviderTests.h"
#include "QueryExecutorTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

static JsonNavNodesFactory s_nodesFactory;

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2015
+===============+===============+===============+===============+===============+======*/
struct LocalizationHelperTests : ECPresentationTest
    {
    PresentationRuleSetPtr m_ruleset;
    TestLocalizationProvider m_l10nProvider;
    LocalizationHelper m_helper;
    LocalizationHelperTests()
        : m_ruleset(PresentationRuleSet::CreateInstance("", 1, 0, false, "", "", "", false)), m_helper(m_l10nProvider, "test locale", m_ruleset.get()) {}
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LocalizationHelperTests, LocalizeString_PassesValidLocaleAndKeyToLocalizationProvider)
    {
    m_l10nProvider.SetHandler([](Utf8StringCR locale, Utf8StringCR key, Utf8StringR localizedValue)
        {
        EXPECT_STREQ("test locale", locale.c_str());
        EXPECT_STREQ("test key", key.c_str());
        localizedValue = key;
        return true;
        });
    Utf8String str = "@test key@";
    m_helper.LocalizeString(str);
    SUCCEED();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LocalizationHelperTests, LocalizeString_JustLocalizedPart)
    {
    m_l10nProvider.SetHandler([](Utf8StringCR, Utf8StringCR key, Utf8StringR localizedValue){localizedValue = "localized"; return true;});
    Utf8String str = "@Namespace:Id@";
    ASSERT_TRUE(m_helper.LocalizeString(str));
    ASSERT_STREQ("localized", str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LocalizationHelperTests, LocalizeString_MultipleLocalizedParts)
    {
    m_l10nProvider.SetHandler([](Utf8StringCR, Utf8StringCR key, Utf8StringR localizedValue){localizedValue = "localized"; return true;});
    Utf8String str = "@Namespace:Id@@Namespace:Id@";
    ASSERT_TRUE(m_helper.LocalizeString(str));
    ASSERT_STREQ("localizedlocalized", str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LocalizationHelperTests, LocalizeString_MultipleLocalizedPartsWithOtherText)
    {
    m_l10nProvider.SetHandler([](Utf8StringCR, Utf8StringCR key, Utf8StringR localizedValue){localizedValue = "localized"; return true;});
    Utf8String str = "123@Namespace:Id@ @Namespace:Id@ @Namespace:Id";
    ASSERT_TRUE(m_helper.LocalizeString(str));
    ASSERT_STREQ("123localized localized @Namespace:Id", str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LocalizationHelperTests, LocalizeString_NoLocalizedParts)
    {
    m_l10nProvider.SetHandler([](Utf8StringCR, Utf8StringCR key, Utf8StringR localizedValue){localizedValue = "localized"; return true;});
    Utf8String str = "@Namespace:Id";
    ASSERT_TRUE(m_helper.LocalizeString(str));
    ASSERT_STREQ("@Namespace:Id", str.c_str());
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2015
+===============+===============+===============+===============+===============+======*/
struct SQLangLocalizationProviderTests : ECPresentationTest
    {
    SQLangLocalizationProvider m_provider;
    void SetUp() override
        {
        ECPresentationTest::SetUp();
        BeFileName sqlangFile;
        BeTest::GetHost().GetDocumentsRoot(sqlangFile);
        sqlangFile.AppendToPath(L"ECPresentationTestData");
        sqlangFile.AppendToPath(L"RulesEngineLocalizedStrings.sqlang.db3");

        BeFileName temporaryDirectory;
        BeTest::GetHost().GetTempDir(temporaryDirectory);

        BeSQLite::BeSQLiteLib::Initialize(temporaryDirectory, BeSQLite::BeSQLiteLib::LogErrors::Yes);
        BeSQLite::L10N::Shutdown();
        BeSQLite::L10N::Initialize(BeSQLite::L10N::SqlangFiles(sqlangFile));
        }
    void TearDown() override
        {
        BeSQLite::L10N::Shutdown();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SQLangLocalizationProviderTests, ReturnsLocalizedString)
    {
    Utf8String localized;
    ASSERT_TRUE(m_provider.GetString("", "RulesEngine:Test", localized));
    ASSERT_STREQ("T35T", localized.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SQLangLocalizationProviderTests, ReturnsEmptyStringIfNotFound)
    {
    Utf8String localized;
    ASSERT_FALSE(m_provider.GetString("", "RulesEngine:DoesNotExist", localized));
    ASSERT_STREQ("", localized.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SQLangLocalizationProviderTests, ReturnsEmptyStringIfInvalidFormat)
    {
    Utf8String localized;
    ASSERT_FALSE(m_provider.GetString("", "InvalidFormat", localized));
    ASSERT_STREQ("", localized.c_str());
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2016
+===============+===============+===============+===============+===============+======*/
struct RulesEngineLocalizedStringTests : ECPresentationTest
    {
    SQLangLocalizationProvider m_provider;
    void SetUp() override
        {
        ECPresentationTest::SetUp();
        Localization::Init();
        }
    void TearDown() override
        {
        Localization::Terminate();
        }
    };

/*---------------------------------------------------------------------------------**//**
* note: in this test we don't care about the localized values, we just check whether the
* SQLangLocalizationProvider tells it succeeded in localizing the strings
* @bsitest                                      Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesEngineLocalizedStringTests, LocalizesRulesEngineStrings)
    {
    Utf8String str;
    LocalizationHelper helper(m_provider, "");

    str = RULESENGINE_LOCALIZEDSTRING_Other;
    EXPECT_TRUE(helper.LocalizeString(str));

    str = RULESENGINE_LOCALIZEDSTRING_NotSpecified;
    EXPECT_TRUE(helper.LocalizeString(str));
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+===============+===============+===============+===============+===============+======*/
struct CustomNodesProviderLocalizationTests : NodesProviderTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderLocalizationTests, NodesLabelAndDescriptionAreLocalized)
    {
    RootNodeRule rule;
    m_context->SetRootNodeContext(&rule);

    TestLocalizationProvider localizationProvider;
    localizationProvider.SetHandler([](Utf8StringCR, Utf8StringCR, Utf8StringR localizedValue){localizedValue = "localized"; return true;});
    m_context->SetLocalizationContext(localizationProvider);

    JsonNavNodePtr node;
    CustomNodeSpecification spec(1, false, "type", "@Namespace:label@", "@Namespace:description@", "imageId");
    NavNodesProviderPtr provider = CustomNodesProvider::Create(*m_context, spec);
    EXPECT_TRUE(provider->GetNode(node, 0));
    EXPECT_TRUE(node.IsValid());
    EXPECT_STREQ("localized", node->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("localized", node->GetDescription().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderLocalizationTests, NodesOverridenLabelIsLocalized)
    {
    TestLocalizationProvider localizationProvider;
    localizationProvider.SetHandler([](Utf8StringCR, Utf8StringCR, Utf8StringR localizedValue){localizedValue = "localized"; return true;});
    m_context->SetLocalizationContext(localizationProvider);

    m_ruleset->AddPresentationRule(*new LabelOverride("", 1, "\"@Namespace:label@\"", ""));

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(rule);

    JsonNavNodePtr node;
    CustomNodeSpecification spec(1, false, "type", "label", "", "imageId");
    NavNodesProviderPtr provider = CustomNodesProvider::Create(*m_context, spec);
    EXPECT_TRUE(provider->GetNode(node, 0));
    EXPECT_TRUE(node.IsValid());
    EXPECT_STREQ("localized", node->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderLocalizationTests, NodesLabelIsLocalizedWithLocalizationResourceKeyDefinition)
    {
    RootNodeRule rule;
    m_context->SetRootNodeContext(&rule);

    TestLocalizationProvider localizationProvider;
    localizationProvider.SetHandler([](Utf8StringCR, Utf8StringCR key, Utf8StringR localizedValue)
        {
        EXPECT_TRUE(key.Equals("from_definition"));
        localizedValue = "localized";
        return true;
        });
    m_context->SetLocalizationContext(localizationProvider);

    m_ruleset->AddPresentationRule(*new LocalizationResourceKeyDefinition(1, "Namespace:Id", "from_definition", "default_value"));

    JsonNavNodePtr node;
    CustomNodeSpecification spec(1, false, "type", "@Namespace:Id@", "", "imageId");
    NavNodesProviderPtr provider = CustomNodesProvider::Create(*m_context, spec);
    EXPECT_TRUE(provider->GetNode(node, 0));
    EXPECT_TRUE(node.IsValid());
    EXPECT_STREQ("localized", node->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderLocalizationTests, NodesLabelIsLocalizedWithLocalizationResourceKeyDefinitionsDefaultValueWhenLocalizedStringIsNotFound)
    {
    RootNodeRule rule;
    m_context->SetRootNodeContext(&rule);

    TestLocalizationProvider localizationProvider;
    localizationProvider.SetHandler([](Utf8StringCR, Utf8StringCR key, Utf8StringR localizedValue){return false;});
    m_context->SetLocalizationContext(localizationProvider);

    m_ruleset->AddPresentationRule(*new LocalizationResourceKeyDefinition(1, "Namespace:Id", "from_definition", "default_value"));

    JsonNavNodePtr node;
    CustomNodeSpecification spec(1, false, "type", "@Namespace:Id@", "", "imageId");
    NavNodesProviderPtr provider = CustomNodesProvider::Create(*m_context, spec);
    EXPECT_TRUE(provider->GetNode(node, 0));
    EXPECT_TRUE(node.IsValid());
    EXPECT_STREQ("default_value", node->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct NavigationQueryResultsReaderLocalizationTests : QueryExecutorTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavigationQueryResultsReaderLocalizationTests, ECInstanceNodesLabelIsLocalized)
    {
    TestLocalizationProvider localizationProvider;
    localizationProvider.SetHandler([](Utf8StringCR, Utf8StringCR, Utf8StringR localizedValue){localizedValue = "localized"; return true;});

    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("@Namespace:Id@"));});

    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(m_widgetClass);
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->SelectContract(*contract);
    query->From(*m_widgetClass, false);
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::ECInstanceNodes);

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ctx.SetLocalizationProvider(localizationProvider);

    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, "locale", *query->GetContract(),
        query->GetResultParameters().GetResultType(), &query->GetResultParameters().GetNavNodeExtendedData());

    JsonNavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, node->GetType().c_str());
    ASSERT_STREQ("localized", node->GetLabelDefinition().GetDisplayValue().c_str());

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavigationQueryResultsReaderLocalizationTests, ECClassGroupingNodesLabelIsLocalized)
    {
    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.IsClassGroupingNode", 1, "\"@Namespace:Id@\"", ""));

    TestLocalizationProvider localizationProvider;
    localizationProvider.SetHandler([](Utf8StringCR, Utf8StringCR, Utf8StringR localizedValue){localizedValue = "localized"; return true;});

    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create();
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->SelectAll().From(ComplexNavigationQuery::Create()->SelectContract(*contract).From(*m_widgetClass, false));
    query->GroupByContract(*contract);
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::ClassGroupingNodes);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ctx.SetLocalizationProvider(localizationProvider);

    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, "locale", *query->GetContract(),
        query->GetResultParameters().GetResultType(), &query->GetResultParameters().GetNavNodeExtendedData());

    JsonNavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    ASSERT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, node->GetType().c_str());
    ASSERT_STREQ("localized", node->GetLabelDefinition().GetDisplayValue().c_str());

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavigationQueryResultsReaderLocalizationTests, ECPropertyGroupingNodesLabelIsLocalized)
    {
    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.IsPropertyGroupingNode", 1, "\"@Namespace:Id@\"", ""));

    TestLocalizationProvider localizationProvider;
    localizationProvider.SetHandler([](Utf8StringCR, Utf8StringCR, Utf8StringR localizedValue){localizedValue = "localized"; return true;});

    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    PropertyGroup spec("", "", true, "IntProperty", "");
    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(*m_widgetClass, *m_widgetClass->GetPropertyP("IntProperty"), nullptr, spec, nullptr);
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->SelectAll().From(ComplexNavigationQuery::Create()->SelectContract(*contract).From(*m_widgetClass, false));
    query->GroupByContract(*contract);
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ctx.SetLocalizationProvider(localizationProvider);

    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, "locale", *query->GetContract(),
        query->GetResultParameters().GetResultType(), &query->GetResultParameters().GetNavNodeExtendedData());

    JsonNavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, node->GetType().c_str());
    ASSERT_STREQ("localized", node->GetLabelDefinition().GetDisplayValue().c_str());

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavigationQueryResultsReaderLocalizationTests, DisplayLabelGroupingNodesLabelIsLocalized)
    {
    TestLocalizationProvider localizationProvider;
    localizationProvider.SetHandler([](Utf8StringCR, Utf8StringCR, Utf8StringR localizedValue){localizedValue = "localized"; return true;});

    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("@Namespace:Id@"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("@Namespace:Id@"));});

    NavigationQueryContractPtr contract = DisplayLabelGroupingNodesQueryContract::Create(m_widgetClass);
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->SelectAll().From(ComplexNavigationQuery::Create()->SelectContract(*contract).From(*m_widgetClass, false));
    query->GroupByContract(*contract);
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::DisplayLabelGroupingNodes);

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ctx.SetLocalizationProvider(localizationProvider);

    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, "locale", *query->GetContract(),
        query->GetResultParameters().GetResultType(), &query->GetResultParameters().GetNavNodeExtendedData());

    JsonNavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    ASSERT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, node->GetType().c_str());
    ASSERT_STREQ("localized", node->GetLabelDefinition().GetDisplayValue().c_str());

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }
