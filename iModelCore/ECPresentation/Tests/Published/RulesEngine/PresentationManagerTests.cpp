﻿/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/RulesEngine/PresentationManagerTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PresentationManagerTests.h"
#include "../../NonPublished/RulesEngine/TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

ECDbTestProject* RulesDrivenECPresentationManagerTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bmap<Utf8String, Utf8String>& GetRegisteredSchemaXmls()
    {
    static bmap<Utf8String, Utf8String> s_registeredSchemaXmls;
    return s_registeredSchemaXmls;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerTests::SetUpTestCase()
    {
    s_project = new ECDbTestProject();
    s_project->Create("RulesDrivenECPresentationManagerTests", "RulesEngineTest.01.00.ecschema.xml");
    
    bvector<ECSchemaPtr> schemas;
    ECSchemaReadContextPtr schemaReadContext = ECSchemaReadContext::CreateContext();
    schemaReadContext->AddSchemaLocater(s_project->GetECDb().GetSchemaLocater());
    for (auto pair : GetRegisteredSchemaXmls())
        {
        ECSchemaPtr schema;
        ECSchema::ReadFromXmlString(schema, pair.second.c_str(), *schemaReadContext);
        if (!schema.IsValid())
            {
            BeAssert(false);
            continue;
            }
        schemas.push_back(schema);
        }

    if (!schemas.empty())
        {
        bvector<ECSchemaCP> importSchemas;
        importSchemas.resize(schemas.size());
        std::transform(schemas.begin(), schemas.end(), importSchemas.begin(), [](ECSchemaPtr const& schema) { return schema.get(); });

        ASSERT_TRUE(SUCCESS == s_project->GetECDb().Schemas().ImportSchemas(importSchemas));
        s_project->GetECDb().SaveChanges();
        }
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerTests::TearDownTestCase()
    {
    DELETE_AND_CLEAR(s_project);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerTests::RegisterSchemaXml(Utf8String name, Utf8String schemaXml) {GetRegisteredSchemaXmls()[name] = schemaXml;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerTests::SetUp()
    {
    Localization::Init();

    m_locater = TestRuleSetLocater::Create();
    m_manager = new RulesDrivenECPresentationManager(RulesEngineTestHelpers::GetPaths(BeTest::GetHost()));
    m_manager->GetLocaters().RegisterLocater(*m_locater);
    IECPresentationManager::RegisterImplementation(m_manager);
    IECPresentationManager::GetManager().GetConnections().NotifyConnectionOpened(s_project->GetECDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerTests::TearDown()
    {
    m_locater = nullptr;
    IECPresentationManager::GetManager().GetConnections().NotifyConnectionClosed(s_project->GetECDb());
    IECPresentationManager::RegisterImplementation(nullptr);
    delete m_manager;
    Localization::Terminate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerTests::InitTestL10N()
    {
    BeFileName sqlangFile;
    BeTest::GetHost().GetDocumentsRoot(sqlangFile);
    sqlangFile.AppendToPath(L"ECPresentationTestData");
    sqlangFile.AppendToPath(L"RulesEngineLocalizedStrings.sqlang.db3");
    BeSQLite::L10N::Shutdown();
    BeSQLite::L10N::Initialize(BeSQLite::L10N::SqlangFiles(sqlangFile));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerTests::ShutDownTestL10N()
    {
    BeSQLite::L10N::Shutdown();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP RulesDrivenECPresentationManagerTests::GetSchema()
    {
    return s_project->GetECDb().Schemas().GetSchema(BeTest::GetNameOfCurrentTest());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP RulesDrivenECPresentationManagerTests::GetClass(Utf8CP schemaName, Utf8CP className)
    {
    return s_project->GetECDb().Schemas().GetClass(schemaName, className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP RulesDrivenECPresentationManagerTests::GetClass(Utf8CP name)
    {
    return GetClass(BeTest::GetNameOfCurrentTest(), name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCP RulesDrivenECPresentationManagerTests::GetRelationshipClass(Utf8CP schemaName, Utf8CP className)
    {
    ECClassCP ecClass = GetClass(schemaName, className);
    return (nullptr == ecClass) ? nullptr : ecClass->GetRelationshipClassCP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCP RulesDrivenECPresentationManagerTests::GetRelationshipClass(Utf8CP name)
    {
    return GetRelationshipClass(BeTest::GetNameOfCurrentTest(), name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String RulesDrivenECPresentationManagerTests::GetClassNamesList(bvector<ECClassCP> const& classes)
    {
    Utf8String list;
    bset<ECSchemaCP> usedSchemas;
    bool firstClass = true;
    for (ECClassCP ecClass : classes)
        {
        if (usedSchemas.end() == usedSchemas.find(&ecClass->GetSchema()))
            {
            if (!usedSchemas.empty())
                list.append(";");
            list.append(ecClass->GetSchema().GetName()).append(":");
            usedSchemas.insert(&ecClass->GetSchema());
            firstClass = true;
            }
        if (!firstClass)
            list.append(",");
        list.append(ecClass->GetName());
        firstClass = false;
        }
    return list;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String RulesDrivenECPresentationManagerTests::GetDisplayLabel(IECInstanceCR instance)
    {
    Utf8String label;
    if (ECObjectsStatus::Success == instance.GetDisplayLabel(label))
        return label;

    return instance.GetClass().GetDisplayLabel();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerTests, InitializesUserSettings)
    {
    StubLocalState localState;
    m_manager->SetLocalState(localState);

    ASSERT_TRUE(m_manager->GetUserSettings("MyRulesetId").GetSettingValue("TestSetting").empty());

    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("MyRulesetId", 1, 0, false, "", "", "", false);
    UserSettingsGroupP settingsGroup = new UserSettingsGroup("Label");
    settingsGroup->GetSettingsItemsR().push_back(new UserSettingsItem("TestSetting", "Label", "StringValue", "DefaultValue"));
    ruleset->AddPresentationRule(*settingsGroup);

    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    m_manager->GetLocaters().RegisterLocater(*locater);
    locater->AddRuleSet(*ruleset);

    ASSERT_STREQ("DefaultValue", m_manager->GetUserSettings("MyRulesetId").GetSettingValue("TestSetting").c_str());
    }

//=======================================================================================
// @bsiclass                                    Grigas.Petraitis                06/2017
//=======================================================================================
struct TestECInstanceChangeHandler : IECInstanceChangeHandler
{
private:
    int m_priority;
    std::function<bool(ECDbCR, ECClassCR)> m_canHandleHandler;
    std::function<ECInstanceChangeResult(ECDbR, ChangedECInstanceInfo const&, Utf8CP, ECValueCR)> m_changeHandler;
private:
    TestECInstanceChangeHandler(int priority) : m_priority(priority) {}
protected:
    int _GetPriority() const override {return m_priority;}
    bool _CanHandle(ECDbCR db, ECClassCR ecClass) const override
        {
        if (nullptr != m_canHandleHandler)
            return m_canHandleHandler(db, ecClass);
        return true;
        }
    ECInstanceChangeResult _Change(ECDbR db, ChangedECInstanceInfo const& changeInfo, Utf8CP propertyAccessor, ECValueCR value) override
        {
        return m_changeHandler(db, changeInfo, propertyAccessor, value);
        }
public:
    static RefCountedPtr<TestECInstanceChangeHandler> Create(int priority = PRIORITY_DEFAULT + 100) {return new TestECInstanceChangeHandler(priority);}
    void SetCanHandleHandler(std::function<bool(ECDbCR, ECClassCR)> handler) {m_canHandleHandler = handler;}
    void SetChangeHandler(std::function<ECInstanceChangeResult(ECDbCR, ChangedECInstanceInfo const&, Utf8CP, ECValueCR)> handler) {m_changeHandler = handler;}
};
typedef RefCountedPtr<TestECInstanceChangeHandler> TestECInstanceChangeHandlerPtr;

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerTests, SaveValueChange_CallsHandlerWithValidParameters)
    {
    ECClassCP widgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECClassCP gadgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Gadget");

    ECInstanceKey widgetKey(widgetClass->GetId(), ECInstanceId((uint64_t)111));
    ECInstanceKey gadgetKey(gadgetClass->GetId(), ECInstanceId((uint64_t)222));

    TestECInstanceChangeHandlerPtr handler = TestECInstanceChangeHandler::Create();
    m_manager->RegisterECInstanceChangeHandler(*handler);

    int canHandleHandler_callCount = 0;
    handler->SetCanHandleHandler([&](ECDbCR db, ECClassCR ecClass) -> bool
        {
        BeAssert(&s_project->GetECDb() == &db);
        BeAssert(widgetClass == &ecClass || gadgetClass == &ecClass);
        ++canHandleHandler_callCount;
        return true;
        });

    int changeHandler_callCount = 0;
    handler->SetChangeHandler([&](ECDbCR db, ChangedECInstanceInfo const& changeInfo, Utf8CP propertyAccessor, ECValueCR value) -> ECInstanceChangeResult
        {
        BeAssert(&s_project->GetECDb() == &db);
        BeAssert(0 == strcmp("test.property.accessor", propertyAccessor));
        BeAssert(ECValue(123) == value);
        BeAssert(widgetClass == &changeInfo.GetPrimaryInstanceClass() || gadgetClass == &changeInfo.GetPrimaryInstanceClass());
        BeAssert(widgetKey.GetInstanceId() == changeInfo.GetPrimaryInstanceId() || gadgetKey.GetInstanceId() == changeInfo.GetPrimaryInstanceId());
        return ECInstanceChangeResult::Success(ECValue(++changeHandler_callCount));
        });

    bvector<ChangedECInstanceInfo> instanceInfos = {
        ChangedECInstanceInfo(*widgetClass, widgetKey.GetInstanceId()), 
        ChangedECInstanceInfo(*gadgetClass, gadgetKey.GetInstanceId())
        };
    bvector<ECInstanceChangeResult> result = m_manager->SaveValueChange(s_project->GetECDb(), instanceInfos, "test.property.accessor", ECValue(123));

    EXPECT_EQ(2, canHandleHandler_callCount);
    EXPECT_EQ(2, changeHandler_callCount);

    ASSERT_EQ(2, result.size());

    EXPECT_EQ(SUCCESS, result[0].GetStatus());
    EXPECT_EQ(ECValue(1), result[0].GetChangedValue());
    
    EXPECT_EQ(SUCCESS, result[1].GetStatus());
    EXPECT_EQ(ECValue(2), result[1].GetChangedValue());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerTests, SaveValueChange_CallsHandlerOnlyWithInstancesItCanHandle)
    {
    ECClassCP widgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECClassCP gadgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Gadget");

    ECInstanceKey widgetKey(widgetClass->GetId(), ECInstanceId((uint64_t)111));
    ECInstanceKey gadgetKey(gadgetClass->GetId(), ECInstanceId((uint64_t)222));

    TestECInstanceChangeHandlerPtr handler = TestECInstanceChangeHandler::Create();
    m_manager->RegisterECInstanceChangeHandler(*handler);

    handler->SetCanHandleHandler([&](ECDbCR, ECClassCR ecClass) -> bool
        {
        return ecClass.GetId() == widgetClass->GetId();
        });

    handler->SetChangeHandler([&](ECDbCR, ChangedECInstanceInfo const& changeInfo, Utf8CP, ECValueCR) -> ECInstanceChangeResult
        {
        BeAssert(widgetClass == &changeInfo.GetPrimaryInstanceClass());
        BeAssert(widgetKey.GetInstanceId() == changeInfo.GetPrimaryInstanceId());
        return ECInstanceChangeResult::Success(ECValue(1));
        });

    bvector<ChangedECInstanceInfo> instanceInfos = {
        ChangedECInstanceInfo(*widgetClass, widgetKey.GetInstanceId()), 
        ChangedECInstanceInfo(*gadgetClass, gadgetKey.GetInstanceId())
        };
    bvector<ECInstanceChangeResult> result = m_manager->SaveValueChange(s_project->GetECDb(), instanceInfos, "test", ECValue());

    ASSERT_EQ(2, result.size());

    EXPECT_EQ(SUCCESS, result[0].GetStatus());
    EXPECT_EQ(ECValue(1), result[0].GetChangedValue());
    
    EXPECT_EQ(ERROR, result[1].GetStatus());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerTests, SaveValueChange_PutsResultsInCorrectOrderWhenUsingMultipleHandlers)
    {
    ECClassCP widgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECClassCP gadgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Gadget");

    ECInstanceKey widgetKey1(widgetClass->GetId(), ECInstanceId((uint64_t)111));
    ECInstanceKey widgetKey2(widgetClass->GetId(), ECInstanceId((uint64_t)222));
    ECInstanceKey gadgetKey(gadgetClass->GetId(), ECInstanceId((uint64_t)333));
    
    int canHandleHandler_callCount = 0;
    int widgetChanges = 0;
    TestECInstanceChangeHandlerPtr widgetsHandler = TestECInstanceChangeHandler::Create();
    widgetsHandler->SetCanHandleHandler([&](ECDbCR, ECClassCR ecClass) -> bool
        {
        if (ecClass.GetId() == widgetClass->GetId())
            {
            ++canHandleHandler_callCount;
            return true;
            }
        return false;
        });
    widgetsHandler->SetChangeHandler([&](ECDbCR, ChangedECInstanceInfo const& changeInfo, Utf8CP, ECValueCR) -> ECInstanceChangeResult
        {
        BeAssert(widgetClass == &changeInfo.GetPrimaryInstanceClass());
        return ECInstanceChangeResult::Success(ECValue(Utf8PrintfString("widget %d", ++widgetChanges).c_str()));
        });
    m_manager->RegisterECInstanceChangeHandler(*widgetsHandler);
    
    TestECInstanceChangeHandlerPtr gadgetsHandler = TestECInstanceChangeHandler::Create();
    gadgetsHandler->SetCanHandleHandler([&](ECDbCR, ECClassCR ecClass) -> bool
        {
        return ecClass.GetId() == gadgetClass->GetId();
        });
    gadgetsHandler->SetChangeHandler([&](ECDbCR, ChangedECInstanceInfo const& changeInfo, Utf8CP, ECValueCR) -> ECInstanceChangeResult
        {
        BeAssert(gadgetClass == &changeInfo.GetPrimaryInstanceClass());
        return ECInstanceChangeResult::Success(ECValue("gadget"));
        });
    m_manager->RegisterECInstanceChangeHandler(*gadgetsHandler);
    
    bvector<ChangedECInstanceInfo> instanceInfos = {
        ChangedECInstanceInfo(*widgetClass, widgetKey1.GetInstanceId()), 
        ChangedECInstanceInfo(*gadgetClass, gadgetKey.GetInstanceId()),
        ChangedECInstanceInfo(*widgetClass, widgetKey2.GetInstanceId())
        };
    bvector<ECInstanceChangeResult> result = m_manager->SaveValueChange(s_project->GetECDb(), instanceInfos, "test", ECValue());
    
    // note - _CanHandle callback should be called only once because the class is the same for both changes
    EXPECT_EQ(1, canHandleHandler_callCount);

    ASSERT_EQ(3, result.size());
    
    EXPECT_EQ(SUCCESS, result[0].GetStatus());
    EXPECT_EQ(ECValue("widget 1"), result[0].GetChangedValue());
    
    EXPECT_EQ(SUCCESS, result[1].GetStatus());
    EXPECT_EQ(ECValue("gadget"), result[1].GetChangedValue());
    
    EXPECT_EQ(SUCCESS, result[2].GetStatus());
    EXPECT_EQ(ECValue("widget 2"), result[2].GetChangedValue());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerTests, OnNodeExpanded_SetsTheFlagAndUpdatesNodeInCache)
    {
    NodesCache& cache = m_manager->GetNodesCache();

    // cache root data source
    DataSourceInfo info(s_project->GetECDb().GetDbGuid(), "RulesDrivenMessagingTest", nullptr, nullptr);
    cache.Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // create the root node
    JsonNavNodePtr node = TestNodesHelper::CreateCustomNode("test type", "test label", "test descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(s_project->GetECDb().GetDbGuid());
    extendedData.SetRulesetId(info.GetRulesetId().c_str());
    cache.Cache(*node, false);

    // verify
    EXPECT_FALSE(node->IsExpanded());

    // tell the node was expanded
    m_manager->NotifyNodeExpanded(s_project->GetECDb(), node->GetNodeId());
    
    // verify
    node = cache.GetNode(node->GetNodeId());
    EXPECT_TRUE(node->IsExpanded());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerTests, OnNodeCollapsed_SetsTheFlagAndUpdatesNodeInCache)
    {
    NodesCache& cache = m_manager->GetNodesCache();

    // cache root data source
    DataSourceInfo info(s_project->GetECDb().GetDbGuid(), "RulesDrivenMessagingTest", nullptr, nullptr);
    cache.Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // create the root node
    JsonNavNodePtr node = TestNodesHelper::CreateCustomNode("test type", "test label", "test descr");
    node->SetIsExpanded(true);
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(s_project->GetECDb().GetDbGuid());
    extendedData.SetRulesetId(info.GetRulesetId().c_str());
    cache.Cache(*node, false);

    // verify
    EXPECT_TRUE(node->IsExpanded());

    // tell the node was expanded
    m_manager->NotifyNodeCollapsed(s_project->GetECDb(), node->GetNodeId());
    
    // verify
    node = cache.GetNode(node->GetNodeId());
    EXPECT_FALSE(node->IsExpanded());
    }
