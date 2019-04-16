/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PresentationManagerTests.h"
#include "../../NonPublished/RulesEngine/TestHelpers.h"
#include "../../BackDoor/PublicAPI/BackDoor/ECPresentation/BackDoor.h"

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
* @bsimethod                                    Grigas.Petraitis                11/2017
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
    ECPresentationTest::SetUp();
    Localization::Init();

    m_manager = new RulesDrivenECPresentationManager(m_connections, RulesEngineTestHelpers::GetPaths(BeTest::GetHost()), true);
    IECPresentationManager::RegisterImplementation(m_manager);
    m_manager->SetLocalizationProvider(new SQLangLocalizationProvider());

    m_locater = TestRuleSetLocater::Create();
    m_manager->GetLocaters().RegisterLocater(*m_locater);
    
    m_connections.NotifyConnectionOpened(s_project->GetECDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerTests::TearDown()
    {
    s_project->GetECDb().AbandonChanges();
    m_locater = nullptr;
    DELETE_AND_CLEAR(m_manager);
    IECPresentationManager::RegisterImplementation(nullptr);
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
    RuntimeJsonLocalState localState;
    m_manager->SetLocalState(&localState);

    ASSERT_TRUE(m_manager->GetUserSettings("MyRulesetId").GetSettingValue("TestSetting").empty());

    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("MyRulesetId", 1, 0, false, "", "", "", false);
    UserSettingsGroupP settingsGroup = new UserSettingsGroup("Label");
    settingsGroup->AddSettingsItem(*new UserSettingsItem("TestSetting", "Label", "StringValue", "DefaultValue"));
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
    bvector<ECInstanceChangeResult> result = m_manager->SaveValueChange(s_project->GetECDb(), instanceInfos, "test.property.accessor", ECValue(123)).get();

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
    bvector<ECInstanceChangeResult> result = m_manager->SaveValueChange(s_project->GetECDb(), instanceInfos, "test", ECValue()).get();

    ASSERT_EQ(2, result.size());

    EXPECT_EQ(SUCCESS, result[0].GetStatus());
    EXPECT_EQ(ECValue(1), result[0].GetChangedValue());
    
    EXPECT_EQ(ERROR, result[1].GetStatus());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerTests, SaveValueChange_OverloadCallsHandlerWithValidECValueParameter)
    {
    ECClassCP widgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECInstanceKey widgetKey(widgetClass->GetId(), ECInstanceId((uint64_t)111));

    TestECInstanceChangeHandlerPtr handler = TestECInstanceChangeHandler::Create();
    m_manager->RegisterECInstanceChangeHandler(*handler);

    handler->SetCanHandleHandler([&](ECDbCR, ECClassCR ecClass) -> bool {return true;});
    size_t count = 0;
    handler->SetChangeHandler([&](ECDbCR, ChangedECInstanceInfo const& changeInfo, Utf8CP, ECValueCR) -> ECInstanceChangeResult
        {
        BeAssert(widgetClass == &changeInfo.GetPrimaryInstanceClass());
        BeAssert(widgetKey.GetInstanceId() == changeInfo.GetPrimaryInstanceId());
        count++;
        return ECInstanceChangeResult::Success(ECValue(1));
        });

    ChangedECInstanceInfo info(*widgetClass, widgetKey.GetInstanceId());
    ECInstanceChangeResult result = m_manager->SaveValueChange(s_project->GetECDb(), info, "test", ECValue()).get();

    EXPECT_EQ(1, count);
    EXPECT_EQ(SUCCESS, result.GetStatus());
    EXPECT_EQ(ECValue(1), result.GetChangedValue());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerTests, SaveValueChange_OverloadCallsHandlerWithValidJsonValueParameter)
    {
    ECClassCP widgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECInstanceKey widgetKey(widgetClass->GetId(), ECInstanceId((uint64_t)111));

    TestECInstanceChangeHandlerPtr handler = TestECInstanceChangeHandler::Create();
    m_manager->RegisterECInstanceChangeHandler(*handler);

    handler->SetCanHandleHandler([&](ECDbCR, ECClassCR ecClass) -> bool {return true;});
    size_t count = 0;
    handler->SetChangeHandler([&](ECDbCR, ChangedECInstanceInfo const& changeInfo, Utf8CP, ECValueCR value) -> ECInstanceChangeResult
        {
        BeAssert(widgetClass == &changeInfo.GetPrimaryInstanceClass());
        BeAssert(widgetKey.GetInstanceId() == changeInfo.GetPrimaryInstanceId());
        BeAssert(ECValue(123) == value);
        count++;
        return ECInstanceChangeResult::Success(ECValue(1));
        });

    ChangedECInstanceInfo info(*widgetClass, widgetKey.GetInstanceId());
    ECInstanceChangeResult result = m_manager->SaveValueChange(s_project->GetECDb(), info, "IntProperty", Json::Value(123)).get();

    EXPECT_EQ(1, count);
    EXPECT_EQ(SUCCESS, result.GetStatus());
    EXPECT_EQ(ECValue(1), result.GetChangedValue());
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
    bvector<ECInstanceChangeResult> result = m_manager->SaveValueChange(s_project->GetECDb(), instanceInfos, "test", ECValue()).get();
    
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
TEST_F(RulesDrivenECPresentationManagerTests, NotifyNodeExpanded_NotifyNodeCollapsed_SetsTheFlagAndUpdatesNodeInCache)
    {
    // create a ruleset with which returns 1 custom root node
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*ruleset);
    RootNodeRuleP rule = new RootNodeRule();
    ruleset->AddPresentationRule(*rule);
    rule->AddSpecification(*new CustomNodeSpecification(1, false, "T", "L", "D", "I"));

    // get the node
    RulesDrivenECPresentationManager::NavigationOptions options("test", TargetTree_Both);
    NavNodesContainer nodes = m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get();
    NavNodeCPtr node = nodes[0];
    EXPECT_FALSE(node->IsExpanded());

    // tell the node was expanded
    m_manager->NotifyNodeExpanded(s_project->GetECDb(), *node->GetKey(), options.GetJson()).wait();

    // verify
    node = m_manager->GetNode(s_project->GetECDb(), *node->GetKey(), options.GetJson()).get();
    EXPECT_TRUE(node->IsExpanded());

    // tell the node was collapsed
    m_manager->NotifyNodeCollapsed(s_project->GetECDb(), *node->GetKey(), options.GetJson()).wait();
    
    // verify
    node = m_manager->GetNode(s_project->GetECDb(), *node->GetKey(), options.GetJson()).get();
    EXPECT_FALSE(node->IsExpanded());
    }
