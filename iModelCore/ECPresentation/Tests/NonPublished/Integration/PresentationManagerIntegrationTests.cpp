/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PresentationManagerIntegrationTests.h"
#include "../../../Source/PresentationManagerImpl.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

ECDbTestProject* PresentationManagerIntegrationTests::s_project = nullptr;
DEFINE_SCHEMA_REGISTRY(PresentationManagerIntegrationTests);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationManagerIntegrationTests::SetUpTestCase()
    {
    s_project = new ECDbTestProject();
    s_project->Create("PresentationManagerIntegrationTests", "RulesEngineTest.01.00.ecschema.xml");
    INIT_SCHEMA_REGISTRY(s_project->GetECDb());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationManagerIntegrationTests::TearDownTestCase()
    {
    DELETE_AND_CLEAR(s_project);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<IConnectionManager> PresentationManagerIntegrationTests::_CreateConnectionManager()
    {
    return std::make_unique<TestConnectionManager>();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationManagerIntegrationTests::_ConfigureManagerParams(ECPresentationManager::Params& params)
    {
    ECPresentationManager::Params::CachingParams cachingParams;
#ifdef USE_HYBRID_CACHE
    cachingParams.SetCacheMode(ECPresentationManager::Params::CachingParams::Mode::Hybrid);
#else
    cachingParams.SetCacheMode(ECPresentationManager::Params::CachingParams::Mode::Memory);
#endif
    params.SetCachingParams(cachingParams);
    params.SetConnections(m_connectionManager);
    params.SetLocalState(&m_localState);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbR PresentationManagerIntegrationTests::_GetProject()
    {
    return s_project->GetECDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationManagerIntegrationTests::SetUp()
    {
    ECPresentationTest::SetUp();

    m_localState.GetValues().clear();
    m_locater = DelayLoadingRuleSetLocater::Create();
    m_connectionManager = _CreateConnectionManager();
    ReCreatePresentationManager(CreateManagerParams());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationManagerIntegrationTests::TearDown()
    {
    s_project->GetECDb().AbandonChanges();
    m_locater = nullptr;
    DELETE_AND_CLEAR(m_manager);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationManager::Params PresentationManagerIntegrationTests::CreateManagerParams()
    {
    ECPresentationManager::Params params(RulesEngineTestHelpers::GetPaths(BeTest::GetHost()));
    _ConfigureManagerParams(params);
    return params;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationManagerIntegrationTests::ReCreatePresentationManager(ECPresentationManager::Params const& params)
    {
    DELETE_AND_CLEAR(m_manager);
    m_manager = new ECPresentationManager(params);
    m_manager->GetConnections().CreateConnection(_GetProject());
    m_manager->GetLocaters().RegisterLocater(*m_locater);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP PresentationManagerIntegrationTests::GetSchema()
    {
    return s_project->GetECDb().Schemas().GetSchema(BeTest::GetNameOfCurrentTest());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP PresentationManagerIntegrationTests::GetClass(Utf8CP schemaName, Utf8CP className)
    {
    return s_project->GetECDb().Schemas().GetClass(schemaName, className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP PresentationManagerIntegrationTests::GetClass(Utf8CP name)
    {
    return GetClass(BeTest::GetNameOfCurrentTest(), name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCP PresentationManagerIntegrationTests::GetRelationshipClass(Utf8CP schemaName, Utf8CP className)
    {
    ECClassCP ecClass = GetClass(schemaName, className);
    return (nullptr == ecClass) ? nullptr : ecClass->GetRelationshipClassCP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCP PresentationManagerIntegrationTests::GetRelationshipClass(Utf8CP name)
    {
    return GetRelationshipClass(BeTest::GetNameOfCurrentTest(), name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PresentationManagerIntegrationTests::GetDisplayLabel(IECInstanceCR instance)
    {
    Utf8String label;
    if (ECObjectsStatus::Success == instance.GetDisplayLabel(label))
        return label;
    return instance.GetClass().GetDisplayLabel();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetDefaultDisplayLabel(Utf8StringCR className, ECInstanceId id)
    {
    Utf8String label = className;
    label.append("-");
    label.append(CommonTools::ToBase36String(CommonTools::GetBriefcaseId(id)));
    label.append("-");
    label.append(CommonTools::ToBase36String(CommonTools::GetLocalId(id)));
    return label;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PresentationManagerIntegrationTests::GetDefaultDisplayLabel(IECInstanceCR instance)
    {
    ECInstanceId id;
    ECInstanceId::FromString(id, instance.GetInstanceId().c_str());
    return ::GetDefaultDisplayLabel(instance.GetClass().GetDisplayLabel(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationManagerIntegrationTests::SetUpDefaultLabelRule(PresentationRuleSetR rules)
    {
    rules.AddPresentationRule(*new LabelOverride("", -9999, "ThisNode.ClassName & \"-\" & ThisNode.BriefcaseId & \"-\" & ThisNode.LocalId", ""));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationManagerIntegrationTests::VerifyNodeInstances(NavNodeCR node, bvector<RefCountedPtr<IECInstance const>> const& expectedInstances)
    {
    auto connection = m_manager->GetConnections().GetConnection(_GetProject());
    auto instanceKeysProvider = m_manager->GetImpl().CreateNodeInstanceKeysProvider(RequestWithRulesetImplParams::Create(*connection, nullptr, NavNodeExtendedData(node).GetRulesetId(), RulesetVariables()));
    RulesEngineTestHelpers::ValidateNodeInstances(*instanceKeysProvider, node, expectedInstances);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationManagerIntegrationTests::VerifyPropertyGroupingNode(NavNodeCR node, bvector<RefCountedPtr<IECInstance const>> const& groupedInstances, bvector<ECValue> const& groupedValues)
    {
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, node.GetType().c_str());
    ASSERT_TRUE(nullptr != node.GetKey()->AsECPropertyGroupingNodeKey());
    RulesEngineTestHelpers::ValidateNodeGroupedValues(node, groupedValues);
    VerifyNodeInstances(node, groupedInstances);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationManagerIntegrationTests::VerifyPropertyRangeGroupingNode(NavNodeCR node, bvector<RefCountedPtr<IECInstance const>> const& groupedInstances)
    {
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, node.GetType().c_str());
    ASSERT_TRUE(nullptr != node.GetKey()->AsECPropertyGroupingNodeKey());
    VerifyNodeInstances(node, groupedInstances);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationManagerIntegrationTests::VerifyClassGroupingNode(NavNodeCR node, bvector<RefCountedPtr<IECInstance const>> const& groupedInstances, ECClassCP groupingClass, bool isPolymorphicGrouping)
    {
    ASSERT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, node.GetType().c_str());
    ASSERT_TRUE(nullptr != node.GetKey()->AsECClassGroupingNodeKey());
    if (nullptr != groupingClass)
        {
        EXPECT_EQ(groupingClass, &node.GetKey()->AsECClassGroupingNodeKey()->GetECClass());
        EXPECT_EQ(isPolymorphicGrouping, node.GetKey()->AsECClassGroupingNodeKey()->IsPolymorphic());
        }
    VerifyNodeInstances(node, groupedInstances);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationManagerIntegrationTests::VerifyLabelGroupingNode(NavNodeCR node, bvector<RefCountedPtr<IECInstance const>> const& groupedInstances)
    {
    ASSERT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, node.GetType().c_str());
    ASSERT_TRUE(nullptr != node.GetKey()->AsLabelGroupingNodeKey());
    VerifyNodeInstances(node, groupedInstances);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationManagerIntegrationTests::VerifyCustomNode(NavNodeCR node, Utf8StringCR type, Nullable<Utf8String> const& label)
    {
    ASSERT_STREQ(type.c_str(), node.GetType().c_str());
    if (!label.IsNull())
        ASSERT_STREQ(label.Value().c_str(), node.GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<HierarchyDef<>> PresentationManagerIntegrationTests::ValidateHierarchy(AsyncHierarchyRequestParams params, std::function<void(AsyncHierarchyRequestParams&)> const& configureParams, bvector<ExpectedHierarchyDef> const& expectedHierarchy)
    {
    if (configureParams)
        configureParams(params);
    bvector<HierarchyDef<>> h;
    auto nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptions pageOptions) { return m_manager->GetNodes(MakePaged(params, pageOptions)).get(); },
        [&]() { return m_manager->GetNodesCount(params).get(); }
    );
    EXPECT_EQ(expectedHierarchy.size(), nodes.GetSize());
    for (size_t i = 0; i < expectedHierarchy.size() && i < nodes.GetSize(); ++i)
        {
        NavNodeCPtr actualNode = nodes[i];
        h.push_back(actualNode);

        ExpectedHierarchyDef const& expectation = expectedHierarchy[i];
        if (expectation.node)
            expectation.node(*actualNode, params);

        bool expectChildren = expectation.children.size() > 0;
        EXPECT_EQ(expectChildren, actualNode->HasChildren());

        AsyncHierarchyRequestParams childParams(params);
        childParams.SetParentNode(actualNode.get());
        h.back().children = ValidateHierarchy(childParams, configureParams, expectation.children);
        }
    return h;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<HierarchyDef<>> PresentationManagerIntegrationTests::ValidateHierarchy(AsyncHierarchyRequestParams const& params, bvector<ExpectedHierarchyDef> const& expectedHierarchy)
    {
    return ValidateHierarchy(params, nullptr, expectedHierarchy);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::function<void(NavNodeCR, HierarchyRequestParams const&)> PresentationManagerIntegrationTests::CreateInstanceNodeValidator(bvector<RefCountedPtr<IECInstance const>> const& expectedInstances)
    {
    return [this, expectedInstances](NavNodeCR n, HierarchyRequestParams const& requestParams)
        {
        VerifyNodeInstances(n, expectedInstances);
        };
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::function<void(NavNodeCR, HierarchyRequestParams const&)> PresentationManagerIntegrationTests::CreateClassGroupingNodeValidator(ECClassCR ecClass, bool isPolymorphic, bvector<RefCountedPtr<IECInstance const>> const& expectedInstances)
    {
    return [this, &ecClass, isPolymorphic, expectedInstances](NavNodeCR n, HierarchyRequestParams const& requestParams)
        {
        VerifyClassGroupingNode(n, expectedInstances, &ecClass, isPolymorphic);
        };
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::function<void(NavNodeCR, HierarchyRequestParams const&)> PresentationManagerIntegrationTests::CreateLabelGroupingNodeValidator(Utf8StringCR label, bvector<RefCountedPtr<IECInstance const>> const& expectedInstances)
    {
    return [this, label, expectedInstances](NavNodeCR n, HierarchyRequestParams const& requestParams)
        {
        VerifyLabelGroupingNode(n, expectedInstances);
        EXPECT_STREQ(label.c_str(), n.GetLabelDefinition().GetDisplayValue().c_str());
        };
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::function<void(NavNodeCR, HierarchyRequestParams const&)> PresentationManagerIntegrationTests::CreatePropertyGroupingNodeValidator(bvector<RefCountedPtr<IECInstance const>> const& expectedInstances, ValueList const& groupedValues)
    {
    return [this, expectedInstances, groupedValues](NavNodeCR n, HierarchyRequestParams const& requestParams)
        {
        VerifyPropertyGroupingNode(n, expectedInstances, groupedValues);
        };
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::function<void(NavNodeCR, HierarchyRequestParams const&)> PresentationManagerIntegrationTests::CreateCustomNodeValidator(Utf8String type, Utf8String label)
    {
    return [this, type, label](NavNodeCR n, HierarchyRequestParams const& requestParams)
        {
        VerifyCustomNode(n, type, label);
        };
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::function<void(NavNodeCR, HierarchyRequestParams const&)> PresentationManagerIntegrationTests::CreateCustomNodeValidator(Utf8String type)
    {
    return [this, type](NavNodeCR n, HierarchyRequestParams const& requestParams)
        {
        VerifyCustomNode(n, type, nullptr);
        };
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationManagerIntegrationTests, InitializesUserSettings)
    {
    ASSERT_TRUE(m_manager->GetUserSettings("MyRulesetId").GetSettingValue("TestSetting").empty());

    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("MyRulesetId");
    UserSettingsGroupP settingsGroup = new UserSettingsGroup("Label");
    settingsGroup->AddSettingsItem(*new UserSettingsItem("TestSetting", "Label", "StringValue", "DefaultValue"));
    ruleset->AddPresentationRule(*settingsGroup);

    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    m_manager->GetLocaters().RegisterLocater(*locater);
    locater->AddRuleSet(*ruleset);

    ASSERT_STREQ("DefaultValue", m_manager->GetUserSettings("MyRulesetId").GetSettingValue("TestSetting").c_str());
    }

BeFileName UpdateTests::s_seedProjectPath;
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateTests::SetUpTestCase()
    {
    PresentationManagerIntegrationTests::SetUpTestCase();
    s_seedProjectPath = BeFileName(s_project->GetECDbPath());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateTests::SetUp()
    {
    m_updateRecordsHandler = std::make_shared<TestUpdateRecordsHandler>();
    m_eventsSource = std::make_shared<TestECInstanceChangeEventsSource>();

    PresentationManagerIntegrationTests::SetUp();

    m_schema = m_db.Schemas().GetSchema("RulesEngineTest");
    m_widgetClass = m_schema->GetClassCP("Widget");
    m_gadgetClass = m_schema->GetClassCP("Gadget");
    m_sprocketClass = m_schema->GetClassCP("Sprocket");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<IConnectionManager> UpdateTests::_CreateConnectionManager()
    {
    // return no manager to use default
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbR UpdateTests::_GetProject()
    {
    if (!m_db.IsDbOpen())
        {
        BeAssert(s_seedProjectPath.DoesPathExist());
        BeFileName projectPath = BeFileName(s_seedProjectPath)
            .PopDir()
            .AppendToPath(WString(Utf8PrintfString("%s_%s", Utf8String(s_seedProjectPath.GetFileNameWithoutExtension()).c_str(), BeTest::GetNameOfCurrentTest()).c_str(), BentleyCharEncoding::Utf8).c_str())
            .AppendExtension(s_seedProjectPath.GetExtension().c_str());
        projectPath.BeDeleteFile();
        BeFileName::BeCopyFile(s_seedProjectPath, projectPath, true);
        m_db.OpenBeSQLiteDb(projectPath, Db::OpenParams(Db::OpenMode::ReadWrite));
        }
    return m_db;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateTests::_ConfigureManagerParams(ECPresentationManager::Params& params)
    {
    PresentationManagerIntegrationTests::_ConfigureManagerParams(params);
    params.SetECInstanceChangeEventSources({ m_eventsSource });
    params.SetUpdateRecordsHandlers({ m_updateRecordsHandler });
    }
