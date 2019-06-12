/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "../../../Source/RulesDriven/RulesEngine/PresentationManagerImpl.h"
#include <UnitTests/BackDoor/ECPresentation/TestRuleSetLocater.h>
#include <UnitTests/BackDoor/ECPresentation/TestSelectionProvider.h>
#include "ECDbTestProject.h"
#include "TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

#define DEFINE_SCHEMA(name, schema_xml) DEFINE_REGISTRY_SCHEMA(UpdateTests, name, schema_xml)

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2016
+===============+===============+===============+===============+===============+======*/
struct TestUpdateRecordsHandler : IUpdateRecordsHandler
{
private:
    bvector<UpdateRecord> m_records;
    bvector<FullUpdateRecord> m_fullUpdateRecords;
protected:
    void _Start() override {m_records.clear();m_fullUpdateRecords.clear();}
    void _Accept(UpdateRecord const& record) override {m_records.push_back(record);}
    void _Accept(FullUpdateRecord const& record) override {m_fullUpdateRecords.push_back(record);}
    void _Finish() override {}
public:
    static RefCountedPtr<TestUpdateRecordsHandler> Create() {return new TestUpdateRecordsHandler();}
    bvector<UpdateRecord> const& GetRecords() const {return m_records;}
    bvector<FullUpdateRecord> const& GetFullUpdateRecords() const {return m_fullUpdateRecords;}
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2016
+===============+===============+===============+===============+===============+======*/
struct UpdateTests : ECPresentationTest
{
    static BeFileName s_seedProjectPath;
    static bmap<Utf8String, Utf8String> s_registeredSchemaXmls;
    TestRuleSetLocaterPtr m_locater;
    RuntimeJsonLocalState m_localState;
    ConnectionManager m_connections;
    RulesDrivenECPresentationManager* m_manager;
    RefCountedPtr<TestECInstanceChangeEventsSource> m_eventsSource;
    ECDb m_db;
    RefCountedPtr<TestUpdateRecordsHandler> m_updateRecordsHandler;
    
    ECSchemaCP m_schema;
    ECClassCP m_widgetClass;
    ECClassCP m_gadgetClass;
    ECClassCP m_sprocketClass;

    static void SetUpTestCase();
    
    virtual void SetUp() override
        {
        ECPresentationTest::SetUp();
        m_localState.GetValues().clear();
        
        BeAssert(s_seedProjectPath.DoesPathExist());
        BeFileName projectPath = BeFileName(s_seedProjectPath)
            .PopDir()
            .AppendToPath(WString(Utf8PrintfString("%s_%s", Utf8String(s_seedProjectPath.GetFileNameWithoutExtension()).c_str(), BeTest::GetNameOfCurrentTest()).c_str(), BentleyCharEncoding::Utf8).c_str())
            .AppendExtension(s_seedProjectPath.GetExtension().c_str());
        projectPath.BeDeleteFile();
        BeFileName::BeCopyFile(s_seedProjectPath, projectPath, true);
        m_db.OpenBeSQLiteDb(projectPath, Db::OpenParams(Db::OpenMode::ReadWrite));
        SetUpSchemas();

        m_locater = TestRuleSetLocater::Create();
        m_updateRecordsHandler = TestUpdateRecordsHandler::Create();
        m_eventsSource = TestECInstanceChangeEventsSource::Create();

        m_manager = new RulesDrivenECPresentationManager(m_connections, RulesEngineTestHelpers::GetPaths(BeTest::GetHost()), true);
        m_manager->SetLocalState(&m_localState);
        m_manager->GetLocaters().RegisterLocater(*m_locater);
        m_manager->RegisterECInstanceChangeEventSource(*m_eventsSource);
        m_manager->RegisterUpdateRecordsHandler(*m_updateRecordsHandler);
        IECPresentationManager::RegisterImplementation(m_manager);

        m_connections.NotifyConnectionOpened(m_db);

        m_schema = m_db.Schemas().GetSchema("RulesEngineTest");
        m_widgetClass = m_schema->GetClassCP("Widget");
        m_gadgetClass = m_schema->GetClassCP("Gadget");
        m_sprocketClass = m_schema->GetClassCP("Sprocket");

        Localization::Init();
        }

    void TearDown() override
        {
        IECPresentationManager::RegisterImplementation(nullptr);
        delete m_manager;

        m_locater->Clear();
        m_locater = nullptr;
        
        Localization::Terminate();
        }

    Utf8String GetDisplayLabel(IECInstanceCR instance)
        {
        Utf8String label;
        if (ECObjectsStatus::Success == instance.GetDisplayLabel(label))
            return Utf8String(label.c_str());

        return Utf8String(instance.GetClass().GetDisplayLabel().c_str());
        }

    void Sync()
        {
        folly::via(&m_manager->GetExecutor(), [](){}).wait();
        }

    static void RegisterSchemaXml(Utf8String name, Utf8String schemaXml);
    static bmap<Utf8String, Utf8String>& GetRegisteredSchemaXmls();
    ECSchemaCP GetSchema();
    void SetUpSchemas();
};
BeFileName UpdateTests::s_seedProjectPath;
bmap<Utf8String, Utf8String> UpdateTests::s_registeredSchemaXmls;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateTests::SetUpTestCase()
    {
    BeFileName temporaryDirectory;
    BeTest::GetHost().GetTempDir(temporaryDirectory);

    // Set up for defined schemas
    ECDbTestProject seedProject;
    seedProject.Create("UpdateTests", "RulesEngineTest.01.00.ecschema.xml");
    s_seedProjectPath = BeFileName(seedProject.GetECDbPath());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateTests::RegisterSchemaXml(Utf8String name, Utf8String schemaXml)
    {
    GetRegisteredSchemaXmls()[name] = schemaXml;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateTests::SetUpSchemas()
    {
    bvector<ECSchemaPtr> schemas;
    ECSchemaReadContextPtr schemaReadContext = ECSchemaReadContext::CreateContext();
    schemaReadContext->AddSchemaLocater(m_db.GetSchemaLocater());
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

        ASSERT_TRUE(SUCCESS == m_db.Schemas().ImportSchemas(importSchemas));
        m_db.SaveChanges();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP UpdateTests::GetSchema()
    {
    return m_db.Schemas().GetSchema(BeTest::GetNameOfCurrentTest());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bmap<Utf8String, Utf8String>& UpdateTests::GetRegisteredSchemaXmls()
    {
    
    return s_registeredSchemaXmls;
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2016
+===============+===============+===============+===============+===============+======*/
struct HierarchyUpdateTests : UpdateTests
    {
    void SetNodeExpanded(NavNodeCR node)
        {
        JsonNavNodeCR jsonNode = static_cast<JsonNavNodeCR>(node);
        const_cast<JsonNavNodeR>(jsonNode).SetIsExpanded(true);
        static_cast<RulesDrivenECPresentationManagerImpl&>(m_manager->GetImpl()).GetNodesCache().Update(jsonNode.GetNodeId(), jsonNode);
        }

    virtual void SetUp() override
        {
        UpdateTests::SetUp();
        m_manager->SetLocalizationProvider(new SQLangLocalizationProvider());
        }
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RemovesECInstanceNodeAfterECInstanceDelete)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 2 nodes
    ASSERT_EQ(2, nodes.GetSize());
    ASSERT_TRUE(nodes[0].IsValid());
    ASSERT_TRUE(nodes[1].IsValid());
    NavNodeCPtr removedNode = nodes[0];
    NavNodeCPtr retainedNode = nodes[1];

    // delete one of the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget1, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget1);
    
    // expect 1 node
    nodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_EQ(*retainedNode->GetKey(), *nodes[0]->GetKey());
    
    // expect two update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*removedNode->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*nodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesECClassGroupingNodeChildrenAfterECInstanceDeleteWhenInstancesLeft)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, true, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 class grouping node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECClassGroupingNode));

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // request its children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    
    // expect 2 instance nodes
    ASSERT_EQ(2, childNodes.GetSize());
    ASSERT_TRUE(childNodes[0].IsValid());
    ASSERT_TRUE(childNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_TRUE(childNodes[1].IsValid());
    ASSERT_TRUE(childNodes[1]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    NavNodeCPtr removedNode = childNodes[0];

    // delete one of the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget1, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget1);

    // expect the same one class grouping node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECClassGroupingNode));

    // expect it to have only one instance node
    childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    ASSERT_TRUE(childNodes[0].IsValid());
    ASSERT_TRUE(childNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_EQ(widget2->GetInstanceId(), childNodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString());

    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*removedNode->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(*childNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RemovesECClassGroupingNodeAfterECInstanceDeleteWhenNoECInstancesLeft_WhenChildrenCached)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, true, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    NavNodeCPtr removedClassNode;
    bvector<NavNodeCPtr> removedChildNodes;

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 class grouping node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECClassGroupingNode));
    removedClassNode = rootNodes[0];

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // request its children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    
    // expect 2 instance nodes
    ASSERT_EQ(2, childNodes.GetSize());
    ASSERT_TRUE(childNodes[0].IsValid());
    ASSERT_TRUE(childNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_TRUE(childNodes[1].IsValid());
    ASSERT_TRUE(childNodes[1]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    removedChildNodes.push_back(childNodes[0]);
    removedChildNodes.push_back(childNodes[1]);

    // delete both instances
    ECInstanceDeleter deleter(m_db, *m_widgetClass, nullptr);
    deleter.Delete(*widget1);
    deleter.Delete(*widget2);
    m_db.SaveChanges();

    bvector<IECInstanceCP> deletedInstances;
    deletedInstances.push_back(widget1.get());
    deletedInstances.push_back(widget2.get());
    m_eventsSource->NotifyECInstancesDeleted(m_db, deletedInstances);

    // expect the class grouping node to be gone as there're no instances left to group
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());

    // expect one update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());
    ASSERT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    ASSERT_EQ(*removedClassNode->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RemovesECClassGroupingNodeAfterECInstanceDeleteWhenNoECInstancesLeft_WhenChildrenNotCached)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, true, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 class grouping node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECClassGroupingNode));
    NavNodeCPtr removedNode = rootNodes[0];

    // delete both instances
    ECInstanceDeleter deleter(m_db, *m_widgetClass, nullptr);
    deleter.Delete(*widget1);
    deleter.Delete(*widget2);
    m_db.SaveChanges();

    bvector<IECInstanceCP> deletedInstances;
    deletedInstances.push_back(widget1.get());
    deletedInstances.push_back(widget2.get());
    m_eventsSource->NotifyECInstancesDeleted(m_db, deletedInstances);

    // expect the class grouping node to be gone as there're no instances left to group
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());

    // expect one update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    ASSERT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    ASSERT_EQ(*removedNode->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RemovesDisplayLabelGroupingNodeChildrenAfterECInstanceDeleteWhenInstancesLeft)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Other Label"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 display label grouping node & 1 ECInstance node
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode));
    NavNodeCPtr displayLabelGroupingNode = rootNodes[0];
    NavNodeCPtr retainedNode = rootNodes[1];

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // request label grouping nodes children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    
    // expect 2 instance nodes
    ASSERT_EQ(2, childNodes.GetSize());
    ASSERT_TRUE(childNodes[0].IsValid());
    ASSERT_TRUE(childNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_TRUE(childNodes[1].IsValid());
    ASSERT_TRUE(childNodes[1]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    bvector<NavNodeCPtr> instanceNodes;
    instanceNodes.push_back(childNodes[0]);
    instanceNodes.push_back(childNodes[1]);

    // delete one of the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget1, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget1);

    // expect the display label grouping node to be gone (single instance is not grouped under a display label grouping node)
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_TRUE(rootNodes[0]->GetLabel().Equals("My Label"));
    ASSERT_EQ(*retainedNode->GetKey(), *rootNodes[1]->GetKey());
    
    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());

    ASSERT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    ASSERT_EQ(*displayLabelGroupingNode->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    ASSERT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    ASSERT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetNode()->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetPosition());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(*rootNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RemovesDisplayLabelGroupingNodeAfterECInstanceDeleteWhenNoInstancesLeft_WhenChildrenCached)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Other Label"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    NavNodeCPtr removedLabelGroupingNode;
    bvector<NavNodeCPtr> removedInstanceNodes;

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 display label grouping node & 1 ECInstance node
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode));
    ASSERT_TRUE(rootNodes[1].IsValid());
    ASSERT_TRUE(rootNodes[1]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    removedLabelGroupingNode = rootNodes[0];

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // request label grouping nodes children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    
    // expect 2 instance nodes
    ASSERT_EQ(2, childNodes.GetSize());
    ASSERT_TRUE(childNodes[0].IsValid());
    ASSERT_TRUE(childNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_TRUE(childNodes[1].IsValid());
    ASSERT_TRUE(childNodes[1]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    removedInstanceNodes.push_back(childNodes[0]);
    removedInstanceNodes.push_back(childNodes[1]);

    // delete both instances
    ECInstanceDeleter deleter(m_db, *m_widgetClass, nullptr);
    deleter.Delete(*widget1);
    deleter.Delete(*widget2);
    m_db.SaveChanges();

    bvector<IECInstanceCP> deletedInstances;
    deletedInstances.push_back(widget1.get());
    deletedInstances.push_back(widget2.get());
    m_eventsSource->NotifyECInstancesDeleted(m_db, deletedInstances);

    // expect the display label grouping node to be gone as there're no instances left to group
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_TRUE(rootNodes[0]->GetLabel().Equals("Other Label"));

    // expect one update record
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    ASSERT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    ASSERT_EQ(*removedLabelGroupingNode->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RemovesDisplayLabelGroupingNodeAfterECInstanceDeleteWhenNoInstancesLeft_WhenChildrenNotCached)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Other Label"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 display label grouping node & 1 ECInstance node
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode));
    NavNodeCPtr removedNode = rootNodes[0];

    // delete both instances
    ECInstanceDeleter deleter(m_db, *m_widgetClass, nullptr);
    deleter.Delete(*widget1);
    deleter.Delete(*widget2);
    m_db.SaveChanges();

    bvector<IECInstanceCP> deletedInstances;
    deletedInstances.push_back(widget1.get());
    deletedInstances.push_back(widget2.get());
    m_eventsSource->NotifyECInstancesDeleted(m_db, deletedInstances);

    // expect the display label grouping node to be gone as there're no instances left to group
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_TRUE(rootNodes[0]->GetLabel().Equals("Other Label"));
    
    // expect one update record
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    ASSERT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    ASSERT_EQ(*removedNode->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RemovesDisplayLabelGroupingNodeAfterECInstanceLabelChange)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Other Label"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 display label grouping node & 1 ECInstance node
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode));
    NavNodeCPtr displayLabelGroupingNode = rootNodes[0];

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // request label grouping nodes children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    
    // expect 2 instance nodes
    ASSERT_EQ(2, childNodes.GetSize());
    ASSERT_TRUE(childNodes[0].IsValid());
    ASSERT_TRUE(childNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_TRUE(childNodes[1].IsValid());
    ASSERT_TRUE(childNodes[1]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));

    // change the label of one of the instances
    widget1->SetValue("MyID", ECValue("My Other Label"));
    ECInstanceUpdater updater(m_db, *widget1, nullptr);
    updater.Update(*widget1);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget1);

    // expect the display label grouping node to be gone (single instance is not grouped under a display label grouping node)
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(3, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_TRUE(rootNodes[0]->GetLabel().Equals("My Label"));
    ASSERT_TRUE(rootNodes[1].IsValid());
    ASSERT_TRUE(rootNodes[1]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_TRUE(rootNodes[1]->GetLabel().Equals("My Other Label"));
    
    // expect 3 update records
    ASSERT_EQ(4, m_updateRecordsHandler->GetRecords().size());
    
    ASSERT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    ASSERT_EQ(*displayLabelGroupingNode->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    ASSERT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetPosition());

    ASSERT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(*rootNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[2].GetPosition());
        
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[3].GetChangeType());
    EXPECT_EQ(*rootNodes[2]->GetKey(), *m_updateRecordsHandler->GetRecords()[3].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, SetsCorrectInsertPositionWhenMultipleSpecificationsUsedAtTheSameLevel1)
    {    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));  
    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect no nodes
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // create a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget);

    // expect one widget instance node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_TRUE(rootNodes[0]->GetLabel().Equals("WidgetID"));
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, SetsCorrectInsertPositionWhenMultipleSpecificationsUsedAtTheSameLevel2)
    {    
    // insert some gadgets
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 2 gadget nodes
    ASSERT_EQ(2, rootNodes.GetSize());
    
    // create a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget);

    // expect one additional widget instance node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(3, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[2].IsValid());
    ASSERT_TRUE(rootNodes[2]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_TRUE(rootNodes[2]->GetLabel().Equals("WidgetID"));
    
    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*rootNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(*rootNodes[2]->GetKey(), *m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[2].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, CreatesDisplayLabelGroupingNodeAfterECInstanceLabelChange)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 2"));});
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 3"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 3 ECInstance nodes
    ASSERT_EQ(3, rootNodes.GetSize());
    for (NavNodeCPtr node : rootNodes)
        {
        ASSERT_TRUE(node.IsValid());
        ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
        }

    bvector<NavNodeCPtr> deletedNodes;
    deletedNodes.push_back(rootNodes[0]);
    deletedNodes.push_back(rootNodes[1]);

    // change the label of one of the instances
    widget1->SetValue("MyID", ECValue("Label 2"));
    ECInstanceUpdater updater(m_db, *widget1, nullptr);
    updater.Update(*widget1);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget1);

    // expect the display label grouping node to be created
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode));
    ASSERT_TRUE(rootNodes[0]->GetLabel().Equals("Label 2"));
    ASSERT_TRUE(rootNodes[1].IsValid());
    ASSERT_TRUE(rootNodes[1]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_TRUE(rootNodes[1]->GetLabel().Equals("Label 3"));
    
    // expect 4 update records
    ASSERT_EQ(4, m_updateRecordsHandler->GetRecords().size());

    ASSERT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    ASSERT_EQ(*deletedNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    ASSERT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    ASSERT_EQ(*deletedNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());

    ASSERT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    ASSERT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[2].GetPosition());
    
    ASSERT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[3].GetChangeType());
    ASSERT_EQ(*rootNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[3].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, CreatesDisplayLabelGroupingNodeAfterECInstanceInsert)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 2"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 2 ECInstance nodes
    ASSERT_EQ(2, rootNodes.GetSize());
    for (NavNodeCPtr node : rootNodes)
        {
        ASSERT_TRUE(node.IsValid());
        ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
        }

    NavNodeCPtr deletedNode = rootNodes[1];
    
    // change the label of one of the instances
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 2"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget3);

    // expect the display label grouping node to be created
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_TRUE(rootNodes[0]->GetLabel().Equals("Label 1"));
    ASSERT_TRUE(rootNodes[1].IsValid());
    ASSERT_TRUE(rootNodes[1]->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode));
    ASSERT_TRUE(rootNodes[1]->GetLabel().Equals("Label 2"));
        
    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());
    
    ASSERT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    ASSERT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    ASSERT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    ASSERT_EQ(*deletedNode->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());

    ASSERT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    ASSERT_EQ(*rootNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[2].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesDisplayLabelGroupingNodeAfterECInstanceInsert)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 2"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, rootNodes.GetSize());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // request display label grouping node children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, childNodes.GetSize());
    for (NavNodeCPtr const& node : childNodes)
        {
        EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
        EXPECT_STREQ("Label 1", node->GetLabel().c_str());
        }
    
    // change the label of one of the instances
    IECInstancePtr widget4 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget4);

    // expect the display label grouping node to have 3 children now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(3, childNodes.GetSize());
    for (NavNodeCPtr const& node : childNodes)
        {
        EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
        EXPECT_STREQ("Label 1", node->GetLabel().c_str());
        }
    
    // expect 5 update records
    ASSERT_EQ(5, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*rootNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(*childNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[3].GetChangeType());
    EXPECT_EQ(*childNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[3].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[4].GetChangeType());
    EXPECT_EQ(*childNodes[2]->GetKey(), *m_updateRecordsHandler->GetRecords()[4].GetNode()->GetKey());
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[4].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesVirtualDisplayLabelGroupingNodeAfterECInstanceInsert)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    for (NavNodeCPtr const& node : rootNodes)
        {
        EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
        EXPECT_STREQ("Label 1", node->GetLabel().c_str());
        }
    
    // insert another widget
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget3);

    // expect 3 root nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(3, rootNodes.GetSize());
    for (NavNodeCPtr const& node : rootNodes)
        {
        EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
        EXPECT_STREQ("Label 1", node->GetLabel().c_str());
        }
    
    // expect 3 update records
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*rootNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(*rootNodes[2]->GetKey(), *m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[2].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesVirtualDisplayLabelGroupingNodeAfterECInstanceDelete)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(3, rootNodes.GetSize());
    for (NavNodeCPtr const& node : rootNodes)
        {
        EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
        EXPECT_STREQ("Label 1", node->GetLabel().c_str());
        }
    NavNodeCPtr deletedNode = rootNodes[1];
    
    // delete one of the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget2, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget2);

    // expect 2 root nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    for (NavNodeCPtr const& node : rootNodes)
        {
        EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
        EXPECT_STREQ("Label 1", node->GetLabel().c_str());
        }
    
    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*deletedNode->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
        
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(*rootNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesECInstanceNodeAfterECInstanceChange)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());
    for (NavNodeCPtr node : rootNodes)
        {
        ASSERT_TRUE(node.IsValid());
        ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
        }

    // change the label of one of the instances
    widget1->SetValue("MyID", ECValue("Label 2"));
    ECInstanceUpdater updater(m_db, *widget1, nullptr);
    updater.Update(*widget1);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget1);

    // expect the label of the node to be changed
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_STREQ("Label 2", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesRootDataSourceAfterECInstanceInsert_AllInstanceNodesSpecification)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("1"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());

    // insert another instance
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("2"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget2);

    // expect 2 nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("1", rootNodes[0]->GetLabel().c_str());
    ASSERT_TRUE(rootNodes[1].IsValid());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[1]->GetType().c_str());
    ASSERT_STREQ("2", rootNodes[1]->GetLabel().c_str());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*rootNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesRootDataSourceAfterECInstanceInsert_InstancesOfSpecificClassesSpecification)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("1"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());

    // insert another instance
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("2"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget2);

    // expect 2 nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("1", rootNodes[0]->GetLabel().c_str());
    ASSERT_TRUE(rootNodes[1].IsValid());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[1]->GetType().c_str());
    ASSERT_STREQ("2", rootNodes[1]->GetLabel().c_str());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*rootNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesRootDataSourceAfterECInstanceInsert_InstancesOfSpecificClassesSpecification_NonPolymorphicMatch)
    {
    ECClassCP classE = m_schema->GetClassCP("ClassE");
    ECClassCP classF = m_schema->GetClassCP("ClassF");

    // insert some instances
    IECInstancePtr instanceE = RulesEngineTestHelpers::InsertInstance(m_db, *classE, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassE", false));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());

    // insert F instance
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(m_db, *classF, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *instanceF);

    // still expect 1 node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    
    // expect 0 update records
    ASSERT_TRUE(m_updateRecordsHandler->GetRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesRootDataSourceAfterECInstanceInsert_InstancesOfSpecificClassesSpecification_PolymorphicMatch)
    {
    ECClassCP classE = m_schema->GetClassCP("ClassE");
    ECClassCP classF = m_schema->GetClassCP("ClassF");

    // insert some instances
    IECInstancePtr instanceE = RulesEngineTestHelpers::InsertInstance(m_db, *classE, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassE", true));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());

    // insert F instance
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(m_db, *classF, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *instanceF);

    // expect 2 nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*rootNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, DoesntUpdateRootDataSourceAfterECInstanceInsertIfClassDoesntMatch_InstancesOfSpecificClassesSpecification)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("1"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());

    // insert a gadget instance
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *gadget);

    // still expect 1 node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    
    // expect 0 update records
    ASSERT_TRUE(m_updateRecordsHandler->GetRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesDataSourceAfterECInstanceInsert_RelatedInstancesSpecification)
    {
    ECRelationshipClassCP widgetHasGadgetsRelationshipClass = m_db.Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();

    // insert the root instance
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));
    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.IsInstanceNode", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection::RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // request for children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, childNodes.GetSize());

    // insert a gadget instance
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetsRelationshipClass, *widget, *gadget, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *gadget);

    // expect 1 node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ("GadgetID", childNodes[0]->GetLabel().c_str());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*childNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesDataSourceAfterECInstanceInsert_SearchResultInstanceNodesSpecificationWithSingleQuerySpecification)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(12));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(22));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rules->AddPresentationRule(*rule);

    SearchResultInstanceNodesSpecificationP spec = new SearchResultInstanceNodesSpecification(1, false, false, false, false, false);
    spec->AddQuerySpecification(*new StringQuerySpecification("SELECT [Widget].[MyID] FROM [RulesEngineTest].[Widget] WHERE [Widget].[IntProperty] > 10", "RulesEngineTest", "Widget"));
    rule->AddSpecification(*spec);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 2 ECInstance nodes
    ASSERT_EQ(2, rootNodes.GetSize());

    // insert another instance
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(32));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget3);
    ECValue v;

    // expect 3 nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(3, rootNodes.GetSize());
    
    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*rootNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(*rootNodes[2]->GetKey(), *m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[2].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesDataSourceAfterECInstanceInsert_SearchResultInstanceNodesSpecificationWithMultipleQuerySpecifications)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(12));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(22));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName=\"Widget\"", 1, "\"Widget-\" & ThisNode.InstanceId", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rules->AddPresentationRule(*rule);

    SearchResultInstanceNodesSpecificationP spec = new SearchResultInstanceNodesSpecification(1, false, false, false, false, false);
    spec->AddQuerySpecification(*new StringQuerySpecification("SELECT [Widget].[MyID] FROM [RulesEngineTest].[Widget] WHERE [Widget].[IntProperty] > 20", "RulesEngineTest", "Widget"));
    spec->AddQuerySpecification(*new StringQuerySpecification("SELECT [Widget].[MyID] FROM [RulesEngineTest].[Widget] WHERE [Widget].[IntProperty] < 15", "RulesEngineTest", "Widget"));
    rule->AddSpecification(*spec);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 2 ECInstance nodes
    ASSERT_EQ(2, rootNodes.GetSize());

    // insert another instance
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(32));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget3);
    ECValue v;
    
    // expect 3 nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(3, rootNodes.GetSize());
    
    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*rootNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(*rootNodes[2]->GetKey(), *m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[2].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesDataSourceAfterECInstanceInsert_SearchResultInstanceNodesSpecification_Polymorphic)
    {
    ECClassCP classE = m_schema->GetClassCP("ClassE");
    ECClassCP classF = m_schema->GetClassCP("ClassF");

    // insert some ClassE instances
    IECInstancePtr instanceE = RulesEngineTestHelpers::InsertInstance(m_db, *classE, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(12));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rules->AddPresentationRule(*rule);

    SearchResultInstanceNodesSpecificationP spec = new SearchResultInstanceNodesSpecification(1, false, false, false, false, false);
    spec->AddQuerySpecification(*new StringQuerySpecification("SELECT * FROM [RulesEngineTest].[ClassE] WHERE [ClassE].[IntProperty] > 10", "RulesEngineTest", "ClassE"));
    rule->AddSpecification(*spec);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());

    // insert ClassF instance
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(m_db, *classF, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(32));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *instanceF);
    ECValue v;
    
    // expect 2 nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*rootNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesDataSourceAfterParentECInstanceUpdate_SearchResultInstanceNodesSpecification)
    {
    ECRelationshipClassCP relationship = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    static Utf8CP s_query1 = "SELECT * FROM [RulesEngineTest].[Gadget]";
    static Utf8CP s_query2 = "SELECT * FROM [RulesEngineTest].[Gadget] LIMIT 1";

    // insert some widgets and gadgets
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("WidgetID"));
        instance.SetValue("Description", ECValue(s_query1));
        });
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationship, *widget, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationship, *widget, *gadget2, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    rules->AddPresentationRule(*childRule);

    SearchResultInstanceNodesSpecificationP spec = new SearchResultInstanceNodesSpecification(1, false, false, false, false, false);
    spec->AddQuerySpecification(*new ECPropertyValueQuerySpecification("RulesEngineTest", "Gadget", "Description"));
    childRule->AddSpecification(*spec);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // request for children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, childNodes.GetSize());

    // change the query in widget Description property
    widget->SetValue("Description", ECValue(s_query2));
    ECInstanceUpdater updater(m_db, *m_widgetClass, nullptr);
    updater.Update(*widget);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget);
    ECValue v;
    
    // expect 1 child node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    
    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());
    
    ASSERT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    ASSERT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    ASSERT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    ASSERT_EQ(*childNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());

    ASSERT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    ASSERT_STREQ(gadget2->GetInstanceId().c_str(), m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, CreatesNodeAfterECInstanceInsertWhenPreviouslyNotCreatedDueToHideIfNoChildrenFlag)
    {
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rules->AddPresentationRule(*rule);

    InstanceNodesOfSpecificClassesSpecificationP rootSpec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, true, false, false, false,
        "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*rootSpec);

    ChildNodeRuleP childRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    rootSpec->AddNestedRule(*childRule);

    InstanceNodesOfSpecificClassesSpecificationP childSpec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Gadget", false);
    childRule->AddSpecification(*childSpec);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 0 nodes
    ASSERT_EQ(0, rootNodes.GetSize());

    // insert a gadget
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *gadget);

    // expect 1 node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, SameLabelInstanceGroupIsCreatedWhenAdditionalInstancesAreInserted)
    {
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup());
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 widget
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());

    // insert another widget
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget2);

    // still expect 1 node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ("WidgetID", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, BaseClassGroupIsUpdatedWhenAdditionalInstancesAreInserted)
    {
    ECClassCP classE = m_schema->GetClassCP("ClassE");
    ECClassCP classF = m_schema->GetClassCP("ClassF");

    // insert a base class instance
    RulesEngineTestHelpers::InsertInstance(m_db, *classE, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassE", true));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassE", "", "", "");
    groupingRule->AddGroup(*new ClassGroup("", true, "RulesEngineTest", "ClassE"));
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 instance
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassE", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // expect it to have 1 child
    ASSERT_EQ(1, IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get().GetSize());
        
    // insert a derived instance
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(m_db, *classF, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *instanceF);

    // still expect 1 ECClassGrouping node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassE", rootNodes[0]->GetLabel().c_str());
    
    // expect it to have 2 children now
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, childNodes.GetSize());
    
    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*childNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(*childNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[2].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, ValuePropertyGroupIsUpdatedWhenAdditionalInstancesAreInserted)
    {
    // insert a widget
    RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(8));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "IntProperty"));
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 property grouping node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("8", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // expect it to have 1 child
    ASSERT_EQ(1, IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get().GetSize());
        
    // insert another widget
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(8));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget2);

    // still expect 1 ECProperty grouping node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("8", rootNodes[0]->GetLabel().c_str());
    
    // expect it to have 2 children now
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, childNodes.GetSize());
    
    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*childNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(*childNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[2].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RangePropertyGroupIsUpdatedWhenAdditionalInstancesAreInserted)
    {
    // insert a widget
    RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(8));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    rules->AddPresentationRule(*groupingRule);

    PropertyGroupP groupingSpec = new PropertyGroup("", "", true, "IntProperty");
    groupingSpec->AddRange(*new PropertyRangeGroupSpecification("5 to 10", "", "5", "10"));
    groupingRule->AddGroup(*groupingSpec);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 property grouping node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("5 to 10", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // expect it to have 1 child
    ASSERT_EQ(1, IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get().GetSize());

    // insert another widget
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(10));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget2);

    // still expect 1 ECProperty grouping node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("5 to 10", rootNodes[0]->GetLabel().c_str());
    
    // expect it to have 2 children now
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, childNodes.GetSize());
    
    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*childNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(*childNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[2].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, PropertyGroupIsCreatedWhenInstanceValuesChange)
    {
    // insert some widgets
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(8));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(8));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "IntProperty"));
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 property grouping node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("8", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // expect it to have 2 children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, childNodes.GetSize());
    NavNodeKeyCPtr deletedNode = childNodes[1]->GetKey();

    // change one of the widgets
    widget2->SetValue("IntProperty", ECValue(9));
    ECInstanceUpdater updater(m_db, *widget2, nullptr);
    updater.Update(*widget2);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget2);

    // expect 2 ECProperty grouping nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("8", rootNodes[0]->GetLabel().c_str());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[1]->GetType().c_str());
    ASSERT_STREQ("9", rootNodes[1]->GetLabel().c_str());
    
    // expect each of them to have 1 child
    DataContainer<NavNodeCPtr> childNodes1 = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes1.GetSize());
    DataContainer<NavNodeCPtr> childNodes2 = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[1], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes2.GetSize());
    
    // expect 4 update records
    ASSERT_EQ(4, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*rootNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetPosition());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(*childNodes1[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[3].GetChangeType());
    EXPECT_EQ(*deletedNode, *m_updateRecordsHandler->GetRecords()[3].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, PropertyGroupIsCreatedWhenInstanceValuesChangeWithCreateGroupForSingleItemFalse)
    {
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(8));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(9));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", false, "IntProperty"));
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 2 ECInstance nodes
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[1]->GetType().c_str());

    // change one of the widgets
    widget2->SetValue("IntProperty", ECValue(8));
    ECInstanceUpdater updater(m_db, *widget2, nullptr);
    updater.Update(*widget2);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget2);

    // expect 1 ECProperty grouping node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("8", rootNodes[0]->GetLabel().c_str());
    
    // expect it to have 2 children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, childNodes.GetSize());
    
    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    
    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[1].GetNode()->GetType().c_str());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, m_updateRecordsHandler->GetRecords()[2].GetNode()->GetType().c_str());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[2].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesCustomNodeWithHideIfNoChildrenFlagWhenNothingChanges)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    CustomNodeSpecificationP spec = new CustomNodeSpecification(1, true, "MyType", "MyLabel", "", "MyImageId");
    spec->SetHasChildren(ChildrenHint::Unknown);
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // insert a widget to force custom spec update
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget);

    // expect 1 root node now (still no custom node)
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesCustomNodeWithHideIfNoChildrenFlagWhenChildrenAdded)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    CustomNodeSpecificationP spec = new CustomNodeSpecification(1, true, "MyType", "MyLabel", "", "MyImageId");
    spec->SetHasChildren(ChildrenHint::Unknown);
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type = \"MyType\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // insert a widget to force custom spec update
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget);

    // expect 2 root nodes now (including the custom node)
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("MyType", rootNodes[1]->GetType().c_str());
    EXPECT_STREQ("MyLabel", rootNodes[1]->GetLabel().c_str());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*rootNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesCustomNodeWithHideIfNoChildrenFlagWhenChildrenRemoved)
    {
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    CustomNodeSpecificationP spec = new CustomNodeSpecification(1, true, "MyType", "MyLabel", "", "MyImageId");
    spec->SetHasChildren(ChildrenHint::Unknown);
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type = \"MyType\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    bvector<NavNodeCPtr> deletedNodes = {rootNodes[0], rootNodes[1]};
    
    // insert a widget to force custom spec update
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget);

    // expect 0 root nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*deletedNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*deletedNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, HidesDisplayLabelGroupingNodeWhenSiblingIsRemovedFromCustomNode)
    {
    // insert 2 widgets and a gadget
    RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new CustomNodeSpecification(1, false, "MyCustomType", "MyLabel", "", "MyImageId"));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type = \"MyCustomType\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*childRule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("MyCustomType", rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("MyLabel", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", childNodes[0]->GetLabel().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, childNodes[1]->GetType().c_str());
    EXPECT_STREQ("WidgetID", childNodes[1]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*childNodes[1]);

    DataContainer<NavNodeCPtr> widgetChildren = IECPresentationManager::GetManager().GetChildren(m_db, *childNodes[1], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, widgetChildren.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, widgetChildren[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, widgetChildren[1]->GetType().c_str());
    
    // delete the widget
    RulesEngineTestHelpers::DeleteInstance(m_db, *gadget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *gadget);

    // expect 2 children now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", childNodes[0]->GetLabel().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[1]->GetType().c_str());
    EXPECT_STREQ("WidgetID", childNodes[1]->GetLabel().c_str());
    
    // expect 4 update records
    ASSERT_EQ(4, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ("GadgetID", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, m_updateRecordsHandler->GetRecords()[1].GetNode()->GetType().c_str());
    EXPECT_STREQ("WidgetID", m_updateRecordsHandler->GetRecords()[1].GetNode()->GetLabel().c_str());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[2].GetNode()->GetType().c_str());
    EXPECT_STREQ("WidgetID", m_updateRecordsHandler->GetRecords()[2].GetNode()->GetLabel().c_str());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[2].GetPosition());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[3].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[3].GetNode()->GetType().c_str());
    EXPECT_STREQ("WidgetID", m_updateRecordsHandler->GetRecords()[3].GetNode()->GetLabel().c_str());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[3].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, HidesDisplayLabelGroupingNodeWhenSiblingIsRemovedFromGroupingNode)
    {
    // insert 2 widgets and a gadget
    RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr differentWidget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("ZZZ"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "Description,MyID"));
    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    GroupingRule* groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "IntProperty", "Default Label"));
    rules->AddPresentationRule(*groupingRule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    
    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", childNodes[0]->GetLabel().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[1]->GetType().c_str());
    EXPECT_STREQ("ZZZ", childNodes[1]->GetLabel().c_str());
    bvector<NavNodeCPtr> deletedNodes = {childNodes[0], childNodes[1]};

    // expand node
    SetNodeExpanded(*childNodes[0]);

    DataContainer<NavNodeCPtr> widgetChildren = IECPresentationManager::GetManager().GetChildren(m_db, *childNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, widgetChildren.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, widgetChildren[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, widgetChildren[1]->GetType().c_str());
    
    // delete the widget
    RulesEngineTestHelpers::DeleteInstance(m_db, *differentWidget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *differentWidget);

    // expect 2 children now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", childNodes[0]->GetLabel().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[1]->GetType().c_str());
    EXPECT_STREQ("WidgetID", childNodes[1]->GetLabel().c_str());
    
    // expect 5 update records
    ASSERT_EQ(5, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*deletedNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(*deletedNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[3].GetChangeType());
    EXPECT_EQ(*childNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[3].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[3].GetPosition());
    
    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[4].GetChangeType());
    EXPECT_EQ(*childNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[4].GetNode()->GetKey());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[4].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RelatedInstanceNodesAreUpdatedAfterOneToManyForwardRelationshipInsert)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));
    
    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childrenRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, 
        "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childrenRule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, childNodes.GetSize());

    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget); // gadget is notified because it has a WidgetId property

    // expect 1 child node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", childNodes[0]->GetLabel().c_str());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*childNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RelatedInstanceNodesAreUpdatedAfterOneToManyBackwardRelationshipInsert)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childrenRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, 
        "", RequiredRelationDirection_Backward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*childrenRule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("GadgetID", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, childNodes.GetSize());

    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget); // gadget is notified because it has a WidgetId property

    // expect 1 child node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", childNodes[0]->GetLabel().c_str());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*childNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RelatedInstanceNodesAreUpdatedAfterOneToManyForwardRelationshipDelete)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget, relate them
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childrenRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, 
        "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childrenRule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", childNodes[0]->GetLabel().c_str());
    NavNodeCPtr deletedNode = childNodes[0];

    // unrelate the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, relationshipKey, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget); // gadget is notified because it has a WidgetId property

    // expect 0 children now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, childNodes.GetSize());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*deletedNode->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RelatedInstanceNodesAreUpdatedAfterOneToManyBackwardRelationshipDelete)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget, relate them
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childrenRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, 
        "", RequiredRelationDirection_Backward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*childrenRule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("GadgetID", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", childNodes[0]->GetLabel().c_str());
    NavNodeCPtr deletedNode = childNodes[0];

    // unrelate the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, relationshipKey, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget); // gadget is notified because it has a WidgetId property

    // expect 0 children now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, childNodes.GetSize());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*deletedNode->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RelatedInstanceNodesAreUpdatedAfterManyToManyForwardRelationshipInsert)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childrenRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, 
        "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetsHaveGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childrenRule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, childNodes.GetSize());

    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    m_eventsSource->NotifyECInstancesChanged(m_db, instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 1 child node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", childNodes[0]->GetLabel().c_str());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*childNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RelatedInstanceNodesAreUpdatedAfterManyToManyBackwardRelationshipInsert)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childrenRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, 
        "", RequiredRelationDirection_Backward, "RulesEngineTest", "RulesEngineTest:WidgetsHaveGadgets", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*childrenRule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("GadgetID", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, childNodes.GetSize());

    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    m_eventsSource->NotifyECInstancesChanged(m_db, instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 1 child node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", childNodes[0]->GetLabel().c_str());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*childNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RelatedInstanceNodesAreUpdatedAfterManyToManyForwardRelationshipDelete)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget, relate them
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childrenRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, 
        "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetsHaveGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childrenRule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", childNodes[0]->GetLabel().c_str());
    NavNodeCPtr deletedNode = childNodes[0];

    // unrelate the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, relationshipKey, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    m_eventsSource->NotifyECInstancesChanged(m_db, instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 0 children now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, childNodes.GetSize());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*deletedNode->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RelatedInstanceNodesAreUpdatedAfterManyToManyBackwardRelationshipDelete)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget, relate them
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childrenRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, 
        "", RequiredRelationDirection_Backward, "RulesEngineTest", "RulesEngineTest:WidgetsHaveGadgets", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*childrenRule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("GadgetID", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", childNodes[0]->GetLabel().c_str());
    NavNodeCPtr deletedNode = childNodes[0];

    // unrelate the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, relationshipKey, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    m_eventsSource->NotifyECInstancesChanged(m_db, instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 0 children now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, childNodes.GetSize());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*deletedNode->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterManyToManyRelationshipInsertWhenBackwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.HasRelatedInstance(\"RulesEngineTest:WidgetsHaveGadgets\", \"Backward\", \"RulesEngineTest:Widget\")", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    m_eventsSource->NotifyECInstancesChanged(m_db, instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 1 root node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_STREQ("GadgetID", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterManyToManyRelationshipInsertWhenForwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.HasRelatedInstance(\"RulesEngineTest:WidgetsHaveGadgets\", \"Forward\", \"RulesEngineTest:Gadget\")", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    m_eventsSource->NotifyECInstancesChanged(m_db, instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 1 root node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_STREQ("WidgetID", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterManyToManyRelationshipDeleteWhenBackwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.HasRelatedInstance(\"RulesEngineTest:WidgetsHaveGadgets\", \"Backward\", \"RulesEngineTest:Widget\")", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", rootNodes[0]->GetLabel().c_str());
    
    // unrelate the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, relationshipKey, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    m_eventsSource->NotifyECInstancesChanged(m_db, instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 0 root nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_STREQ("GadgetID", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterManyToManyRelationshipDeleteWhenForwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.HasRelatedInstance(\"RulesEngineTest:WidgetsHaveGadgets\", \"Forward\", \"RulesEngineTest:Gadget\")", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());
    
    // relate the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, relationshipKey, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    m_eventsSource->NotifyECInstancesChanged(m_db, instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 0 root nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_STREQ("WidgetID", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterOneToManyRelationshipInsertWhenBackwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.HasRelatedInstance(\"RulesEngineTest:WidgetHasGadgets\", \"Backward\", \"RulesEngineTest:Widget\")", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget); // gadget is notified because it has a WidgetId property

    // expect 1 root node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_STREQ("GadgetID", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterOneToManyRelationshipInsertWhenForwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.HasRelatedInstance(\"RulesEngineTest:WidgetHasGadgets\", \"Forward\", \"RulesEngineTest:Gadget\")", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget); // gadget is notified because it has a WidgetId property

    // expect 1 root node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_STREQ("WidgetID", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterOneToManyRelationshipDeleteWhenBackwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.HasRelatedInstance(\"RulesEngineTest:WidgetHasGadgets\", \"Backward\", \"RulesEngineTest:Widget\")", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", rootNodes[0]->GetLabel().c_str());
    
    // unrelate the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, relationshipKey, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget); // gadget is notified because it has a WidgetId property

    // expect 0 root nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_STREQ("GadgetID", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterOneToManyRelationshipDeleteWhenForwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.HasRelatedInstance(\"RulesEngineTest:WidgetHasGadgets\", \"Forward\", \"RulesEngineTest:Gadget\")", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());
    
    // relate the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, relationshipKey, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget); // gadget is notified because it has a WidgetId property

    // expect 0 root nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_STREQ("WidgetID", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterManyToManyRelationshipInsertWhenBackwardGetRelatedInstancesCountECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.GetRelatedInstancesCount(\"RulesEngineTest:WidgetsHaveGadgets\", \"Backward\", \"RulesEngineTest:Widget\") > 0", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    m_eventsSource->NotifyECInstancesChanged(m_db, instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 1 root node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_STREQ("GadgetID", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterManyToManyRelationshipInsertWhenForwardGetRelatedInstancesCountECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.GetRelatedInstancesCount(\"RulesEngineTest:WidgetsHaveGadgets\", \"Forward\", \"RulesEngineTest:Gadget\") > 0", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    m_eventsSource->NotifyECInstancesChanged(m_db, instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 1 root node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_STREQ("WidgetID", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterManyToManyRelationshipDeleteWhenBackwardGetRelatedInstancesCountECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.GetRelatedInstancesCount(\"RulesEngineTest:WidgetsHaveGadgets\", \"Backward\", \"RulesEngineTest:Widget\") > 0", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", rootNodes[0]->GetLabel().c_str());
    
    // unrelate the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, relationshipKey, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    m_eventsSource->NotifyECInstancesChanged(m_db, instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 0 root nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_STREQ("GadgetID", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterManyToManyRelationshipDeleteWhenForwardGetRelatedInstancesCountInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.GetRelatedInstancesCount(\"RulesEngineTest:WidgetsHaveGadgets\", \"Forward\", \"RulesEngineTest:Gadget\") > 0", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());
    
    // relate the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, relationshipKey, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    m_eventsSource->NotifyECInstancesChanged(m_db, instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 0 root nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_STREQ("WidgetID", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterOneToManyRelationshipInsertWhenBackwardGetRelatedInstancesCountECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.GetRelatedInstancesCount(\"RulesEngineTest:WidgetHasGadgets\", \"Backward\", \"RulesEngineTest:Widget\") > 0", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget); // gadget is notified because it has a WidgetId property

    // expect 1 root node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_STREQ("GadgetID", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterOneToManyRelationshipInsertWhenForwardGetRelatedInstancesCountECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.GetRelatedInstancesCount(\"RulesEngineTest:WidgetHasGadgets\", \"Forward\", \"RulesEngineTest:Gadget\") > 0", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget); // gadget is notified because it has a WidgetId property

    // expect 1 root node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_STREQ("WidgetID", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterOneToManyRelationshipDeleteWhenBackwardGetRelatedInstancesCountECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.GetRelatedInstancesCount(\"RulesEngineTest:WidgetHasGadgets\", \"Backward\", \"RulesEngineTest:Widget\") > 0", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", rootNodes[0]->GetLabel().c_str());
    
    // unrelate the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, relationshipKey, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget); // gadget is notified because it has a WidgetId property

    // expect 0 root nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_STREQ("GadgetID", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterOneToManyRelationshipDeleteWhenForwardGetRelatedInstancesCountECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.GetRelatedInstancesCount(\"RulesEngineTest:WidgetHasGadgets\", \"Forward\", \"RulesEngineTest:Gadget\") > 0", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());
    
    // relate the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, relationshipKey, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget); // gadget is notified because it has a WidgetId property

    // expect 0 root nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_STREQ("WidgetID", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterManyToManyRelationshipRelatedInstanceUpdateWhenGetRelatedValueECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    
    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.GetRelatedValue(\"RulesEngineTest:WidgetsHaveGadgets\", \"Forward\", \"RulesEngineTest:Gadget\", \"MyID\") = \"123\"", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
        
    gadget->SetValue("MyID", ECValue("123"));
    ECInstanceUpdater updater(m_db, *gadget, nullptr);
    updater.Update(*gadget);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget);

    // expect 1 root node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterSkippedOneToManyRelationshipInsert)
    {
    ECRelationshipClassCP relationshipClassWG = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipClassGS = m_schema->GetClassCP("GadgetHasSprockets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(m_db, *m_sprocketClass);
    
    // relate gadget to sprocket
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClassGS, *gadget, *sprocket, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 1, "", 
        RequiredRelationDirection_Forward, "", "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Sprocket"));
    rules->AddPresentationRule(*childRule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, childNodes.GetSize());
    
    // relate widget to gadget
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClassWG, *widget, *gadget, nullptr, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget);

    // expect 1 root node with no children
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(widget->GetInstanceId().c_str(), rootNodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
    childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());

    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*childNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterSkippedOneToManyRelationshipDelete)
    {
    ECRelationshipClassCP relationshipClassWG = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipClassGS = m_schema->GetClassCP("GadgetHasSprockets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(m_db, *m_sprocketClass);
    
    // relate the instances
    ECInstanceKey relWGKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClassWG, *widget, *gadget);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClassGS, *gadget, *sprocket, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 1, "", 
        RequiredRelationDirection_Forward, "", "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Sprocket"));
    rules->AddPresentationRule(*childRule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    NavNodeCPtr deletedNode = childNodes[0];
    
    RulesEngineTestHelpers::DeleteInstance(m_db, relWGKey, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget);

    // expect 1 root node with no children
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(widget->GetInstanceId().c_str(), rootNodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
    childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, childNodes.GetSize());

    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*deletedNode->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterSkippedManyToManyRelationshipInsert)
    {
    ECRelationshipClassCP relationshipClass1 = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipClass2 = m_schema->GetClassCP("WidgetsHaveGadgets2")->GetRelationshipClassCP();

    // insert some instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(2));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass1, *widget1, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.IntProperty = 1", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 1, "", 
        RequiredRelationDirection_Both, "", "RulesEngineTest:WidgetsHaveGadgets2", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*childRule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(widget1->GetInstanceId().c_str(), rootNodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, childNodes.GetSize());
    
    // insert the relationship
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass2, *widget2, *gadget, nullptr, true);
    m_eventsSource->NotifyECInstancesUpdated(m_db, {widget2.get(), gadget.get()});

    // expect the child node to exist now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(widget1->GetInstanceId().c_str(), rootNodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
    childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(widget2->GetInstanceId().c_str(), childNodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*childNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterRelatedInstanceInsert)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "", "RulesEngineTest:Gadget", false));
    rule->GetSpecifications()[0]->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "widget"));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new LabelOverride("", 1, "\"Gadget_\" & IIF(NOT IsNull(widget.IntProperty), widget.IntProperty, \"No_Widget\")", ""));

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    
    // verify the label override with related instance property got applied
    EXPECT_STREQ("Gadget_No_Widget", rootNodes[0]->GetLabel().c_str());
    
    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget);

    // still expect 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());

    // verify the label has changed
    EXPECT_STREQ("Gadget_1", rootNodes[0]->GetLabel().c_str());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterRelatedInstanceDelete)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    
    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "", "RulesEngineTest:Gadget", false));
    rule->GetSpecifications()[0]->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "widget"));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new LabelOverride("", 1, "\"Gadget_\" & IIF(NOT IsNull(widget.IntProperty), widget.IntProperty, \"No_Widget\")", ""));

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    
    // verify the label override with related instance property got applied
    EXPECT_STREQ("Gadget_1", rootNodes[0]->GetLabel().c_str());

    RulesEngineTestHelpers::DeleteInstance(m_db, *widget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget);

    // still expect 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());

    // verify the label has changed
    EXPECT_STREQ("Gadget_No_Widget", rootNodes[0]->GetLabel().c_str());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterRelatedInstanceUpdate)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    
    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "", "RulesEngineTest:Gadget", false));
    rule->GetSpecifications()[0]->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "widget"));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new LabelOverride("", 1, "\"Gadget_\" & IIF(NOT IsNull(widget.IntProperty), widget.IntProperty, \"No_Widget\")", ""));

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    
    // verify the label override with related instance property got applied
    EXPECT_STREQ("Gadget_1", rootNodes[0]->GetLabel().c_str());

    widget->SetValue("IntProperty", ECValue(2));
    ECInstanceUpdater(m_db, *widget, nullptr).Update(*widget);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget);

    // still expect 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());

    // verify the label has changed
    EXPECT_STREQ("Gadget_2", rootNodes[0]->GetLabel().c_str());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesAllAffectedRootHierarchies)
    {
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("123"));}, true);
    
    // create 3 rulesets
    PresentationRuleSetPtr rules1 = PresentationRuleSet::CreateInstance("UpdatesAllAffectedRootHierarchies_1", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules1);
    RootNodeRule* rule1 = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules1->AddPresentationRule(*rule1);
    rules1->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));
    
    PresentationRuleSetPtr rules2 = PresentationRuleSet::CreateInstance("UpdatesAllAffectedRootHierarchies_2", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules2);
    RootNodeRule* rule2 = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules2->AddPresentationRule(*rule2);
    rules2->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));
    
    PresentationRuleSetPtr rules3 = PresentationRuleSet::CreateInstance("UpdatesAllAffectedRootHierarchies_3", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules3);
    RootNodeRule* rule3 = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule3->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rules3->AddPresentationRule(*rule3);
    rules3->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes1 = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), RulesDrivenECPresentationManager::NavigationOptions(rules1->GetRuleSetId().c_str(), TargetTree_Both).GetJson()).get();
    DataContainer<NavNodeCPtr> rootNodes2 = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), RulesDrivenECPresentationManager::NavigationOptions(rules2->GetRuleSetId().c_str(), TargetTree_Both).GetJson()).get();
    DataContainer<NavNodeCPtr> rootNodes3 = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), RulesDrivenECPresentationManager::NavigationOptions(rules3->GetRuleSetId().c_str(), TargetTree_Both).GetJson()).get();
        
    // verify expected results
    ASSERT_EQ(1, rootNodes1.GetSize());
    EXPECT_STREQ("123", rootNodes1[0]->GetLabel().c_str());
    ASSERT_EQ(1, rootNodes2.GetSize());
    EXPECT_STREQ("123", rootNodes2[0]->GetLabel().c_str());
    ASSERT_EQ(0, rootNodes3.GetSize());

    // update the widget
    widget->SetValue("MyID", ECValue("456"));
    ECInstanceUpdater updater(m_db, *widget, nullptr);
    updater.Update(*widget);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget);

    // verify expected results
    rootNodes1 = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), RulesDrivenECPresentationManager::NavigationOptions(rules1->GetRuleSetId().c_str(), TargetTree_Both).GetJson()).get();
    rootNodes2 = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), RulesDrivenECPresentationManager::NavigationOptions(rules2->GetRuleSetId().c_str(), TargetTree_Both).GetJson()).get();
    rootNodes3 = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), RulesDrivenECPresentationManager::NavigationOptions(rules3->GetRuleSetId().c_str(), TargetTree_Both).GetJson()).get();
    ASSERT_EQ(1, rootNodes1.GetSize());
    EXPECT_STREQ("456", rootNodes1[0]->GetLabel().c_str());
    ASSERT_EQ(1, rootNodes2.GetSize());
    EXPECT_STREQ("456", rootNodes2[0]->GetLabel().c_str());
    ASSERT_EQ(0, rootNodes3.GetSize());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ("456", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    EXPECT_STREQ("UpdatesAllAffectedRootHierarchies_1", NavNodeExtendedData(*m_updateRecordsHandler->GetRecords()[0].GetNode()).GetRulesetId());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_STREQ("456", m_updateRecordsHandler->GetRecords()[1].GetNode()->GetLabel().c_str());
    EXPECT_STREQ("UpdatesAllAffectedRootHierarchies_2", NavNodeExtendedData(*m_updateRecordsHandler->GetRecords()[1].GetNode()).GetRulesetId());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesAllAffectedChildHierarchies)
    {
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("123"));}, true);
    
    // create 3 rulesets
    PresentationRuleSetPtr rules1 = PresentationRuleSet::CreateInstance("UpdatesAllAffectedChildHierarchies_1", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules1);
    rules1->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));
    RootNodeRule* rootRule1 = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule1->AddSpecification(*new CustomNodeSpecification(1, false, "TEST_Type", "Root", "descr", "imageid"));
    rules1->AddPresentationRule(*rootRule1);
    ChildNodeRule* childRule1 = new ChildNodeRule("ParentNode.Type = \"TEST_Type\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules1->AddPresentationRule(*childRule1);
    
    PresentationRuleSetPtr rules2 = PresentationRuleSet::CreateInstance("UpdatesAllAffectedChildHierarchies_2", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules2);
    rules2->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));
    RootNodeRule* rootRule2 = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule2->AddSpecification(*new CustomNodeSpecification(1, false, "TEST_Type", "Root", "descr", "imageid"));
    rules2->AddPresentationRule(*rootRule2);
    ChildNodeRule* childRule2 = new ChildNodeRule("ParentNode.Type = \"TEST_Type\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules2->AddPresentationRule(*childRule2);
    
    PresentationRuleSetPtr rules3 = PresentationRuleSet::CreateInstance("UpdatesAllAffectedChildHierarchies_3", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules3);
    rules3->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));
    RootNodeRule* rootRule3 = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule3->AddSpecification(*new CustomNodeSpecification(1, false, "TEST_Type", "Root", "descr", "imageid"));
    rules3->AddPresentationRule(*rootRule3);
    ChildNodeRule* childRule3 = new ChildNodeRule("ParentNode.Type = \"TEST_Type\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule3->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rules3->AddPresentationRule(*childRule3);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes1 = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), RulesDrivenECPresentationManager::NavigationOptions(rules1->GetRuleSetId().c_str(), TargetTree_Both).GetJson()).get();
    DataContainer<NavNodeCPtr> rootNodes2 = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), RulesDrivenECPresentationManager::NavigationOptions(rules2->GetRuleSetId().c_str(), TargetTree_Both).GetJson()).get();
    DataContainer<NavNodeCPtr> rootNodes3 = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), RulesDrivenECPresentationManager::NavigationOptions(rules3->GetRuleSetId().c_str(), TargetTree_Both).GetJson()).get();
        
    // verify expected results
    ASSERT_EQ(1, rootNodes1.GetSize());
    EXPECT_STREQ("Root", rootNodes1[0]->GetLabel().c_str());
    // expand node
    SetNodeExpanded(*rootNodes1[0]);
    DataContainer<NavNodeCPtr> childNodes1 = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes1[0], PageOptions(), RulesDrivenECPresentationManager::NavigationOptions(rules1->GetRuleSetId().c_str(), TargetTree_Both).GetJson()).get();
    ASSERT_EQ(1, rootNodes1.GetSize());
    EXPECT_STREQ("123", childNodes1[0]->GetLabel().c_str());

    ASSERT_EQ(1, rootNodes2.GetSize());
    EXPECT_STREQ("Root", rootNodes2[0]->GetLabel().c_str());
    // expand node
    SetNodeExpanded(*rootNodes2[0]);
    DataContainer<NavNodeCPtr> childNodes2 = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes2[0], PageOptions(), RulesDrivenECPresentationManager::NavigationOptions(rules2->GetRuleSetId().c_str(), TargetTree_Both).GetJson()).get();
    ASSERT_EQ(1, childNodes2.GetSize());
    EXPECT_STREQ("123", childNodes2[0]->GetLabel().c_str());

    ASSERT_EQ(1, rootNodes3.GetSize());
    EXPECT_STREQ("Root", rootNodes3[0]->GetLabel().c_str());
    // expand node
    SetNodeExpanded(*rootNodes3[0]);
    DataContainer<NavNodeCPtr> childNodes3 = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes3[0], PageOptions(), RulesDrivenECPresentationManager::NavigationOptions(rules3->GetRuleSetId().c_str(), TargetTree_Both).GetJson()).get();
    ASSERT_EQ(0, childNodes3.GetSize());

    // update the widget
    widget->SetValue("MyID", ECValue("456"));
    ECInstanceUpdater updater(m_db, *widget, nullptr);
    updater.Update(*widget);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget);

    // verify expected results
    rootNodes1 = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), RulesDrivenECPresentationManager::NavigationOptions(rules1->GetRuleSetId().c_str(), TargetTree_Both).GetJson()).get();
    childNodes1 = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes1[0], PageOptions(), RulesDrivenECPresentationManager::NavigationOptions(rules1->GetRuleSetId().c_str(), TargetTree_Both).GetJson()).get();
    rootNodes2 = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), RulesDrivenECPresentationManager::NavigationOptions(rules2->GetRuleSetId().c_str(), TargetTree_Both).GetJson()).get();
    childNodes2 = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes2[0], PageOptions(), RulesDrivenECPresentationManager::NavigationOptions(rules2->GetRuleSetId().c_str(), TargetTree_Both).GetJson()).get();
    rootNodes3 = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), RulesDrivenECPresentationManager::NavigationOptions(rules3->GetRuleSetId().c_str(), TargetTree_Both).GetJson()).get();
    childNodes3 = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes3[0], PageOptions(), RulesDrivenECPresentationManager::NavigationOptions(rules3->GetRuleSetId().c_str(), TargetTree_Both).GetJson()).get();
    ASSERT_EQ(1, childNodes1.GetSize());
    EXPECT_STREQ("456", childNodes1[0]->GetLabel().c_str());
    ASSERT_EQ(1, childNodes2.GetSize());
    EXPECT_STREQ("456", childNodes2[0]->GetLabel().c_str());
    ASSERT_EQ(0, childNodes3.GetSize());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ("456", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    EXPECT_STREQ("UpdatesAllAffectedChildHierarchies_1", NavNodeExtendedData(*m_updateRecordsHandler->GetRecords()[0].GetNode()).GetRulesetId());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_STREQ("456", m_updateRecordsHandler->GetRecords()[1].GetNode()->GetLabel().c_str());
    EXPECT_STREQ("UpdatesAllAffectedChildHierarchies_2", NavNodeExtendedData(*m_updateRecordsHandler->GetRecords()[1].GetNode()).GetRulesetId());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesParentsHasChildrenFlagWhenChildNodeIsInserted_WhenParentIsRoot)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, TargetTree_Both, false);
    CustomNodeSpecification* rootSpec = new CustomNodeSpecification(1, false, "custom", "custom", "custom", "custom");
    rootSpec->SetHasChildren(ChildrenHint::Unknown);
    rootRule->AddSpecification(*rootSpec);
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type=\"custom\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_FALSE(rootNodes[0]->HasChildren());
    // expand node
    SetNodeExpanded(*rootNodes[0]);
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, childNodes.GetSize());
        
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget);

    // expect the root node to have a "has children" flag set to "true"
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_TRUE(rootNodes[0]->HasChildren());
    childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*childNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* TFS#759626
* @betest                                       Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesParentsHasChildrenFlagWhenChildNodeIsInserted_WhenParentIsNotRoot)
    {
    // set up
    RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR g){g.SetValue("Description", ECValue("1"));});
    RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR g){g.SetValue("Description", ECValue("2"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule1 = new RootNodeRule();
    rule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "this.Description=\"1\"", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule1);
    
    ChildNodeRule* childRule1 = new ChildNodeRule();
    childRule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "this.Description=\"2\"", "RulesEngineTest:Gadget", false));
    rule1->GetSpecifications().front()->AddNestedRule(*childRule1);
    
    ChildNodeRule* childRule2 = new ChildNodeRule();
    childRule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "this.Description=\"3\"", "RulesEngineTest:Gadget", false));
    childRule1->GetSpecifications().front()->AddNestedRule(*childRule2);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_TRUE(rootNodes[0]->HasChildren());
    SetNodeExpanded(*rootNodes[0]);
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_FALSE(childNodes[0]->HasChildren());
    SetNodeExpanded(*childNodes[0]);
    DataContainer<NavNodeCPtr> grandchildNodes = IECPresentationManager::GetManager().GetChildren(m_db, *childNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, grandchildNodes.GetSize());
        
    // insert another gadget
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR g){g.SetValue("Description", ECValue("3"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *gadget);

    // expect the middle child node to have a "has children" flag set to "true"
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_TRUE(rootNodes[0]->HasChildren());
    childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_TRUE(childNodes[0]->HasChildren());
    grandchildNodes = IECPresentationManager::GetManager().GetChildren(m_db, *childNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, grandchildNodes.GetSize());
    
    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*childNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(*grandchildNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[2].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesParentsHasChildrenFlagWhenChildNodeIsDeleted)
    {
    // set up
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, TargetTree_Both, false);
    CustomNodeSpecification* rootSpec = new CustomNodeSpecification(1, false, "custom", "custom", "custom", "custom");
    rootSpec->SetHasChildren(ChildrenHint::Unknown);
    rootRule->AddSpecification(*rootSpec);
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type=\"custom\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_TRUE(rootNodes[0]->HasChildren());
    // expand node
    SetNodeExpanded(*rootNodes[0]);
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    NavNodeCPtr deletedNode = childNodes[0];
        
    // delete the widget
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget);

    // expect the root node to have a "has children" flag set to "false"
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_FALSE(rootNodes[0]->HasChildren());
    childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, childNodes.GetSize());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*deletedNode->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, ShowsParentNodeWithHideIfNoChildrenFlagWhenChildNodeIsInserted)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, TargetTree_Both, false);
    CustomNodeSpecification* rootSpec = new CustomNodeSpecification(1, true, "custom", "custom", "custom", "custom");
    rootSpec->SetHasChildren(ChildrenHint::Unknown);
    rootRule->AddSpecification(*rootSpec);
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type=\"custom\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
        
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget);

    // expect the root node to get inserted
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_TRUE(rootNodes[0]->HasChildren());
    
    // expect 1 update records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RemovesParentNodeWithHideIfNoChildrenFlagWhenTheLastChildNodeIsDeleted)
    {
    // set up
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, TargetTree_Both, false);
    CustomNodeSpecification* rootSpec = new CustomNodeSpecification(1, true, "custom", "custom", "custom", "custom");
    rootSpec->SetHasChildren(ChildrenHint::Unknown);
    rootRule->AddSpecification(*rootSpec);
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type=\"custom\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_TRUE(rootNodes[0]->HasChildren());
    NavNodeCPtr deletedRootNode = rootNodes[0];

    // expand node
    SetNodeExpanded(*rootNodes[0]);
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    NavNodeCPtr deletedChildNode = childNodes[0];
        
    // delete the widget
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget);

    // expect the root node to be gone
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*deletedChildNode->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*deletedRootNode->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, DoesNotUpdateChildHierarchyIfParentIsRemoved)
    {
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);
    
    // create the ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new CustomNodeSpecification(1, false, "TEST_TYPE", "Custom label", "", "ImageId"));
    rootRule->GetSpecifications()[0]->AddNestedRule(*childRule);

    ChildNodeRule* grandchildRule = new ChildNodeRule("ParentNode.Type = \"TEST_TYPE\"", 1, false, RuleTargetTree::TargetTree_Both);
    grandchildRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*grandchildRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
        
    // verify expected results
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());
    NavNodeCPtr deletedRootNode = rootNodes[0];

    // expand node
    SetNodeExpanded(*rootNodes[0]);
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ("Custom label", childNodes[0]->GetLabel().c_str());
    
    // expand node
    SetNodeExpanded(*childNodes[0]);
    DataContainer<NavNodeCPtr> grandchildNodes = IECPresentationManager::GetManager().GetChildren(m_db, *childNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, grandchildNodes.GetSize());
    EXPECT_STREQ("WidgetID", grandchildNodes[0]->GetLabel().c_str());

    // delete the widget
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget);

    // verify expected results
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*deletedRootNode->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, CustomizesInsertedNodes)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // insert a widget 
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget);

    // expect 1 root node now 
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(widget->GetInstanceId().c_str(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
    EXPECT_TRUE(NavNodeExtendedData(*m_updateRecordsHandler->GetRecords()[0].GetNode()).IsCustomized());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesAffectedBranchesWhenUserSettingChanges_UsedInRuleCondition)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("1 = GetSettingIntValue(\"test\")", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // change a setting
    m_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", 1);

    // expect 1 root node now 
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(widget->GetInstanceId().c_str(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
    EXPECT_TRUE(NavNodeExtendedData(*m_updateRecordsHandler->GetRecords()[0].GetNode()).IsCustomized());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesAffectedBranchesWhenUserSettingChanges_UsedInInstanceFilter)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "1 = GetSettingIntValue(\"test\")", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // change a setting
    m_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", 1);

    // expect 1 root node now 
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(widget->GetInstanceId().c_str(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
    EXPECT_TRUE(NavNodeExtendedData(*m_updateRecordsHandler->GetRecords()[0].GetNode()).IsCustomized());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesAffectedBranchesWhenUserSettingChanges_UsedInLabelOverrideRuleCondition)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);
    ECInstanceId widgetId;
    ECInstanceId::FromString(widgetId, widget->GetInstanceId().c_str());
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new LabelOverride("1 = GetSettingIntValue(\"test\")", 1, "\"test\"", "\"test\""));

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_NotSpecified()).c_str(), rootNodes[0]->GetLabel().c_str());

    for (int i = 1; i <= 2; ++i)
        {
        bool shouldLabelChange = (1 == i);

        // change a setting
        m_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", i);

        // still expect 1 root node
        rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
        ASSERT_EQ(1, rootNodes.GetSize());

        // verify label
        EXPECT_STREQ(shouldLabelChange ? "test" : RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_NotSpecified()).c_str(), rootNodes[0]->GetLabel().c_str());

        // expect 1 update record
        ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

        EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
        EXPECT_STREQ(widget->GetInstanceId().c_str(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
        EXPECT_TRUE(NavNodeExtendedData(*m_updateRecordsHandler->GetRecords()[0].GetNode()).IsCustomized());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesAffectedBranchesWhenUserSettingChanges_UsedInLabelOverrideRuleExpression)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new LabelOverride("", 1, "\"test\" & GetSettingIntValue(\"test\")", "\"test\""));

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("test0", rootNodes[0]->GetLabel().c_str());

    for (int i = 1; i <= 2; ++i)
        {
        // change a setting
        m_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", i);

        // still expect 1 root node
        rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
        ASSERT_EQ(1, rootNodes.GetSize());

        // its label should have changed
        EXPECT_STREQ(Utf8PrintfString("test%d", i).c_str(), rootNodes[0]->GetLabel().c_str());

        // expect 1 update record
        ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

        EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
        EXPECT_STREQ(widget->GetInstanceId().c_str(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
        EXPECT_TRUE(NavNodeExtendedData(*m_updateRecordsHandler->GetRecords()[0].GetNode()).IsCustomized());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesAffectedBranchesWhenUserSettingChanges_UsedInCustomizationRuleCondition)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);
    ECInstanceId widgetId;
    ECInstanceId::FromString(widgetId, widget->GetInstanceId().c_str());
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new StyleOverride("1 = GetSettingIntValue(\"test\")", 1, "\"test\"", "\"test\"", "\"test\""));
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("", rootNodes[0]->GetForeColor().c_str());

    for (int i = 1; i <= 2; ++i)
        {
        bool shouldColorChange = (1 == i);

        // change a setting
        m_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", i);

        // still expect 1 root node
        rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
        ASSERT_EQ(1, rootNodes.GetSize());

        // verify label
        EXPECT_STREQ(shouldColorChange ? "test" : "", rootNodes[0]->GetForeColor().c_str());

        // expect 1 update record
        ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

        EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
        EXPECT_STREQ(widget->GetInstanceId().c_str(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
        EXPECT_TRUE(NavNodeExtendedData(*m_updateRecordsHandler->GetRecords()[0].GetNode()).IsCustomized());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesAffectedBranchesWhenUserSettingChanges_UsedInCustomizationRuleExpression)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new StyleOverride("", 1, "\"test\" & GetSettingIntValue(\"test\")", "\"test\"", "\"test\""));

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("test0", rootNodes[0]->GetForeColor().c_str());

    for (int i = 1; i <= 2; ++i)
        {
        // change a setting
        m_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", i);

        // still expect 1 root node
        rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
        ASSERT_EQ(1, rootNodes.GetSize());

        // its label should have changed
        EXPECT_STREQ(Utf8PrintfString("test%d", i).c_str(), rootNodes[0]->GetForeColor().c_str());

        // expect 1 update record
        ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

        EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
        EXPECT_STREQ(widget->GetInstanceId().c_str(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
        EXPECT_TRUE(NavNodeExtendedData(*m_updateRecordsHandler->GetRecords()[0].GetNode()).IsCustomized());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesGroupingBranchesUnderHiddenLevelsWhenUserSettingChanges)
    {
    ECRelationshipClassCP widgetHasGadgetsClass = m_db.Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(2));});
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetsClass, *widget1, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetsClass, *widget2, *gadget2, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, true, false, false, false, false,
        "this.IntProperty = GetSettingIntValue(\"test\")", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, true, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);

    // set initial setting value
    m_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", 1);

    /* hierarchy before update:
    
        widget 1                            -- hidden
            Gadget (class grouping node)    -- root node
                gadget 1                    -- child node
    */

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Gadget", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*gadget1), childNodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());
    NavNodeCPtr deletedNode = childNodes[0];

    // change a setting
    m_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", 2);

    /* hierarchy after update:
    
        widget 2                            -- hidden
            Gadget (class grouping node)    -- root node
                gadget 2                    -- child node
    */

    // expect 1 root node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Gadget", rootNodes[0]->GetLabel().c_str());
    
    childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*gadget2), childNodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());

    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*deletedNode->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(*childNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[2].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesLocalizedCustomNodesOnUserSettingChange)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new CustomNodeSpecification(1, false, "Custom", "Label", "Description", "ImageId"));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.Type=\"Custom\"", 1, 
        Utf8PrintfString("\"%s\"", RULESENGINE_LOCALIZEDSTRING_Other.c_str()), "GetSettingIntValue(\"counter\")"));

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("0", rootNodes[0]->GetDescription().c_str());

    // change a setting
    m_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("counter", 1);

    // still expect 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());

    // its description should have changed
    EXPECT_STREQ("1", rootNodes[0]->GetDescription().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ("Custom", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey()->GetType().c_str());
    EXPECT_TRUE(NavNodeExtendedData(*m_updateRecordsHandler->GetRecords()[0].GetNode()).IsCustomized());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, DoesntUpdateHierarchiesOfClosedConnectionsOnUserSettingChange)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("DoesntUpdateHierarchiesOfClosedConnectionsOnUserSettingChange", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("0 = GetSettingIntValue(\"test\")", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new CustomNodeSpecification(1, false, "Custom", "Label", "Description", "ImageId"));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Label", rootNodes[0]->GetLabel().c_str());

    // close the dataset
    m_db.CloseDb();

    // change a setting
    m_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", 1);

    // expect no update records
    ASSERT_TRUE(m_updateRecordsHandler->GetRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, DoesNotUpdateHierarchyWhenNodeRemovedFromCollapsedHierarchy)
    {
    // insert some instances
    ECRelationshipClassCP widgetHasGadgetsClass = m_db.Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetsClass, *widget, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetsClass, *widget, *gadget2, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());

    // make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ("GadgetID", childrenNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("GadgetID", childrenNodes[0]->GetLabel().c_str());

    // expect root node to be collapsed
    EXPECT_FALSE(rootNodes[0]->IsExpanded());

    // delete one gadget
    RulesEngineTestHelpers::DeleteInstance(m_db, *gadget2, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *gadget2);

    // expect no updates
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords().size());

    // make sure we still have 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());

    // make sure it now has 1 child node
    childrenNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ("GadgetID", childrenNodes[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, DoesNotUpdateHierarchyWhenNodeInsertedIntoCollapsedHierarchy)
    {
    // insert some instances
    ECRelationshipClassCP widgetHasGadgetsClass = m_db.Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetsClass, *widget, *gadget1, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", true));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());

    // make sure it has 1 child node
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ("GadgetID", childrenNodes[0]->GetLabel().c_str());

    // expect root node to be collapsed
    EXPECT_FALSE(rootNodes[0]->IsExpanded());

    // relate second gadget
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetsClass, *widget, *gadget2, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *gadget2);

    // expect no updates
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords().size());

    // make sure we have 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());

    // make sure it now has 2 children
    childrenNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ("GadgetID", childrenNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("GadgetID", childrenNodes[1]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, DoesNotUpdateHierarchyWhenNodeUpdatedInCollapsedHierarchy)
    {
    // insert some instances
    ECRelationshipClassCP widgetHasGadgetClass = m_db.Schemas().GetClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Widget_Label"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Gadget_Label"));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetClass, *widget, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID,Description"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "Description"));

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Gadget\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Backward, "", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Gadget_Label", rootNodes[0]->GetLabel().c_str());

    // make sure it has 1 child node
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ("Widget_Label", childrenNodes[0]->GetLabel().c_str());

    // expect root node to be collapsed
    EXPECT_FALSE(rootNodes[0]->IsExpanded());

    // update widget
    widget->SetValue("MyID", ECValue("New_Widget_Label"));
    ECInstanceUpdater updater(m_db, *widget, nullptr);
    updater.Update(*widget);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget);

    // expect no updates
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords().size());

    // make sure we still have 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Gadget_Label", rootNodes[0]->GetLabel().c_str());

    // make sure it still has 1 child node but label is changed
    childrenNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ("New_Widget_Label", childrenNodes[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, DoesNotUpdateHierarchyWhenAnyParentUpTheHierarchyIsCollapsed)
    {
    // insert some instances and create hierarchy
    ECRelationshipClassCP widgetHasGadgetClass = m_db.Schemas().GetClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP();
    ECRelationshipClassCP gadgetHasSprocketsClass = m_db.Schemas().GetClass("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    IECInstancePtr sprocket1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("SprocketID"));});
    IECInstancePtr sprocket2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("SprocketID"));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetClass, *widget, *gadget);
    RulesEngineTestHelpers::InsertRelationship(m_db, *gadgetHasSprocketsClass, *gadget, *sprocket1, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Sprocket", "MyID"));

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    // gadget rule
    ChildNodeRule* childRule1 = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    childRule1->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule1);

    // sprocket rule
    ChildNodeRule* childRule2 = new ChildNodeRule("ParentNode.ClassName=\"Gadget\"", 1, false, TargetTree_Both);
    childRule2->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Sprocket"));
    rules->AddPresentationRule(*childRule2);

    // make sure we have 1 root node
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());

    // make sure it has 1 gadget node
    DataContainer<NavNodeCPtr> gadgetNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, gadgetNodes.GetSize());
    EXPECT_STREQ("GadgetID", gadgetNodes[0]->GetLabel().c_str());

    // expand gadget node
    SetNodeExpanded(*gadgetNodes[0]);

    // make sure it has 1 sprocket node
    DataContainer<NavNodeCPtr> sprocketNodes = IECPresentationManager::GetManager().GetChildren(m_db, *gadgetNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, sprocketNodes.GetSize());
    EXPECT_STREQ("SprocketID", sprocketNodes[0]->GetLabel().c_str());

    // expect widget to be collapsed and gadget to be expanded
    EXPECT_FALSE(rootNodes[0]->IsExpanded());
    EXPECT_TRUE(gadgetNodes[0]->IsExpanded());

    // relate second sprocket to gadget
    RulesEngineTestHelpers::InsertRelationship(m_db, *gadgetHasSprocketsClass, *gadget, *sprocket2, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *sprocket2);

    // expect no updates, because widget was collapsed
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords().size());

    // make sure we still have 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());

    // make sure it still has 1 gadget node
    gadgetNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, gadgetNodes.GetSize());
    EXPECT_STREQ("GadgetID", gadgetNodes[0]->GetLabel().c_str());

    // make sure it now has 2 sprocket nodes
    sprocketNodes = IECPresentationManager::GetManager().GetChildren(m_db, *gadgetNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, sprocketNodes.GetSize());
    EXPECT_STREQ("SprocketID", sprocketNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("SprocketID", sprocketNodes[1]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, UpdateHierarchyWhenLastNodeRemovedFromCollapsedHierarchy)
    {
    // insert some instances
    ECRelationshipClassCP widgetHasGadgetClass = m_db.Schemas().GetClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetClass, *widget, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());
    EXPECT_TRUE(rootNodes[0]->HasChildren());

    // make sure it has 1 child node
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ("GadgetID", childrenNodes[0]->GetLabel().c_str());

    // expect root node to be collapsed
    EXPECT_FALSE(rootNodes[0]->IsExpanded());

    // delete gadget
    RulesEngineTestHelpers::DeleteInstance(m_db, *gadget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *gadget);

    // make sure we still have 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());
    EXPECT_FALSE(rootNodes[0]->HasChildren());

    // make sure now it has 0 children
    childrenNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, childrenNodes.GetSize());

    // expect 1 update record
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords().size());
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, UpdateHierarchyWhenNodeInsertedIntoEmptyCollapsedHierarchy)
    {
    // insert some instances
    ECRelationshipClassCP widgetHasGadgetClass = m_db.Schemas().GetClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());
    EXPECT_FALSE(rootNodes[0]->HasChildren());

    // make sure it has 0 children
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, childrenNodes.GetSize());

    // expect root node to be collapsed
    EXPECT_FALSE(rootNodes[0]->IsExpanded());

    // relate gadget
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetClass, *widget, *gadget, nullptr, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *gadget);

    // make sure we still have 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());
    EXPECT_TRUE(rootNodes[0]->HasChildren());

    // make sure now it has 1 child node
    childrenNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ("GadgetID", childrenNodes[0]->GetLabel().c_str());

    // expect 1 update record
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords().size());
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, UpdateHierarchyWhenLastGroupedNodeDeletedFromCollapsedHierarchy)
    {
    // insert some instances
    ECRelationshipClassCP widgetHasGadgetClass = m_db.Schemas().GetClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetClass, *widget, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, true, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());
    EXPECT_TRUE(rootNodes[0]->HasChildren());

    // make sure it has 1 child grouping node
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, childrenNodes[0]->GetType().c_str());

    // make sure it has 1 child gadget node
    DataContainer<NavNodeCPtr> gadgetNodes = IECPresentationManager::GetManager().GetChildren(m_db, *childrenNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, gadgetNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, gadgetNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", gadgetNodes[0]->GetLabel().c_str());

    // expect root node to be collapsed
    EXPECT_FALSE(rootNodes[0]->IsExpanded());

    // delete gadget
    RulesEngineTestHelpers::DeleteInstance(m_db, *gadget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *gadget);

    // make sure we still have 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());
    EXPECT_FALSE(rootNodes[0]->HasChildren());

    // make sure now it has 0 children
    childrenNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, childrenNodes.GetSize());

    // expect 1 update record
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords().size());
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, DoesNotCollapseParentNodeAfterChildNodeIsInserted)
    {
    // insert widget
    RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    // make sure we have 1 root node
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_TRUE(rootNodes[0]->HasChildren());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // make sure it has 1 child node
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ("WidgetID", childrenNodes[0]->GetLabel().c_str());

    // expect grouping node to be expanded
    EXPECT_TRUE(rootNodes[0]->IsExpanded());

    // insert second widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget);

    // make sure we still have 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_TRUE(rootNodes[0]->HasChildren());

    // expect it to be expanded
    EXPECT_TRUE(rootNodes[0]->IsExpanded());

    // make sure now it has 2 children nodes
    childrenNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ("WidgetID", childrenNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("WidgetID", childrenNodes[1]->GetLabel().c_str());

    // expect 3 recors
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*childrenNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(*childrenNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, DoesNotUpdateFixedHierarchyWhenNodeIsInserted)
    {
    // insert widget
    RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("WidgetID"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    // make sure we have 1 root node and disable updates
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both, true);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());

    // insert second widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget);

    // expect no updates
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, DoesNotUpdateFixedHierarchyWhenNodeIsUpdated)
    {
    // insert widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("WidgetID"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    // make sure we have 1 root node and disable updates
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both, true);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());

    // update widget
    widget->SetValue("MyID", ECValue("New label"));
    ECInstanceUpdater updater(m_db, *widget, nullptr);
    updater.Update(*widget);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget);

    // expect no updates
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, DoesNotUpdateFixedHierarchyWhenChildIsInserted)
    {
    // insert widget
    ECRelationshipClassCP widgetHasGadgetsClass = m_db.Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("WidgetID"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node and disable updates
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both, true);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());

    // make sure it has 0 children
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(0, childrenNodes.GetSize());

    // relate gadget to widget
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetsClass, *widget, *gadget, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *gadget);

    // expect no updates
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, DoesNotUpdateFixedHierarchyWhenChildIsDeleted)
    {
    // insert some instances
    ECRelationshipClassCP widgetHasGadgetsClass = m_db.Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetsClass, *widget, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node and disable updates
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both, true);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());

    // expand root node
    SetNodeExpanded(*rootNodes[0]);

    // make sure it has 1 child
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ("GadgetID", childrenNodes[0]->GetLabel().c_str());

    // delete gadget
    RulesEngineTestHelpers::DeleteInstance(m_db, *gadget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *gadget);

    // expect no updates
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, DoesNotCollapseNodeIfItWasExpandedAndLastChildrenWasRemoved)
    {
    // insert some widget instances
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    ChildNodeRule* childRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*childRule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 root node which has children
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    EXPECT_TRUE(rootNodes[0]->HasChildren());

    // expand root node
    SetNodeExpanded(*rootNodes[0]);

    // request for children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();

    // expect 1 child
    ASSERT_EQ(1, childNodes.GetSize());

    // delete child
    RulesEngineTestHelpers::DeleteInstance(m_db, *gadget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *gadget);
    
    // expect root node to be expanded but without children
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_TRUE(rootNodes[0]->IsExpanded());
    EXPECT_FALSE(rootNodes[0]->HasChildren());
    }

/*---------------------------------------------------------------------------------**//**
* TFS#887406
* @betest                                       Grigas.Petraitis                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesDataSourceAfterInsertWhenItAlreadyHasManyToManyRelatedInstanceNodeGroupedUnderVirtualPropertyGroupingNode)
    {
    ECRelationshipClassCP widgetHasGadgetsClass = m_db.Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCP widgetsHaveGadgetsClass = m_db.Schemas().GetClass("RulesEngineTest", "WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert some instances
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("1"));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetsHaveGadgetsClass, *widget, *gadget, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    GroupingRule* groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Gadget", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", false, "MyID"));
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "this.Widget.Id = parent.ECInstanceId", "RulesEngineTest:Gadget", false));
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, 
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:WidgetsHaveGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 root node 
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());

    // expand root node
    SetNodeExpanded(*rootNodes[0]);

    // request for children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();

    // expect 1 child
    ASSERT_EQ(1, childNodes.GetSize());

    // add a new gadget 
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("2"));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetsClass, *widget, *gadget2, nullptr, true);    
    m_eventsSource->NotifyECInstanceInserted(m_db, *gadget2);
    
    // make sure we still have 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    
    // make sure now it has 2 children nodes
    childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, childNodes.GetSize());
    EXPECT_STREQ(gadget2->GetInstanceId().c_str(), childNodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
    EXPECT_STREQ(gadget->GetInstanceId().c_str(), childNodes[1]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());

    // expect 2 records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*childNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*childNodes[1]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* VSTS#28901
* @betest                                       Haroldas.Vitunskas              09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UserSettingsTrackedWhenCustomizingChildNodesDuringHierarchyUpdate, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementProperty" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="ElementA">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ElementB">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
)*");
TEST_F(HierarchyUpdateTests, UserSettingsTrackedWhenCustomizingChildNodesDuringHierarchyUpdate)
    {
    // prepare the dataset
    ECRelationshipClassCP rel = dynamic_cast<ECRelationshipClass const *>(GetSchema()->GetClassCP("ElementOwnsChildElements"));
    ECClassCP ecClassA = GetSchema()->GetClassCP("ElementA");
    ECClassCP ecClassB = GetSchema()->GetClassCP("ElementB");
    ASSERT_NE(nullptr, rel);
    
    IECInstancePtr rootElement = RulesEngineTestHelpers::InsertInstance(m_db, *ecClassA);
    IECInstancePtr childElement = RulesEngineTestHelpers::InsertInstance(m_db, *ecClassB);
    RulesEngineTestHelpers::InsertRelationship(m_db, *rel, *rootElement, *childElement, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    
    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", ecClassA->GetFullName(), false));

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"ElementA\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", ecClassB->GetFullName(), false));

    rules->AddPresentationRule(*rootRule);
    rules->AddPresentationRule(*childRule);
    rules->AddPresentationRule(*new StyleOverride("ThisNode.ClassName=\"ElementB\"", 1, "GetSettingIntValue(\"custom\")", "", ""));

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    NavNodeCPtr rootNode = rootNodes[0];
    SetNodeExpanded(*rootNode);
    
    // get a child node
    ASSERT_TRUE(rootNode->HasChildren());
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNode, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    NavNodeCPtr childNode = childNodes[0];

    // modify and update child
    childElement->SetValue("ElementProperty", ECValue(10));
    ECInstanceUpdater updater(m_db, *childElement, nullptr);
    updater.Update(*childElement);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *childElement);
    
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());

    // expect user settings have been tracked
    bvector<HierarchyLevelInfo> hierarchies = static_cast<RulesDrivenECPresentationManagerImpl&>(m_manager->GetImpl()).GetNodesCache().GetRelatedHierarchyLevels(rules->GetRuleSetId().c_str(), "custom");
    ASSERT_EQ(1, hierarchies.size());
    ASSERT_EQ(m_connections.GetConnection(m_db)->GetId(), hierarchies[0].GetConnectionId());
    ASSERT_EQ(rules->GetRuleSetId(), hierarchies[0].GetRulesetId());
    ASSERT_EQ(rootNodes[0]->GetNodeId(), *hierarchies[0].GetPhysicalParentNodeId());
    }

/*---------------------------------------------------------------------------------**//**
* VSTS#28901
* @betest                                       Haroldas.Vitunskas              09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UserSettingsTrackedWhenCustomizingChildNodesWithVirtualParentDuringHierarchyUpdate, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementProperty" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="ElementA">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ElementB">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ElementC">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
)*");
TEST_F(HierarchyUpdateTests, UserSettingsTrackedWhenCustomizingChildNodesWithVirtualParentDuringHierarchyUpdate)
    {
    // prepare the dataset
    ECRelationshipClassCP rel = dynamic_cast<ECRelationshipClass const *>(GetSchema()->GetClassCP("ElementOwnsChildElements"));
    ECClassCP ecClassA = GetSchema()->GetClassCP("ElementA");
    ECClassCP ecClassB = GetSchema()->GetClassCP("ElementB");
    ECClassCP ecClassC = GetSchema()->GetClassCP("ElementC");
    ASSERT_NE(nullptr, rel);

    IECInstancePtr rootElement = RulesEngineTestHelpers::InsertInstance(m_db, *ecClassA);
    IECInstancePtr virtualElement = RulesEngineTestHelpers::InsertInstance(m_db, *ecClassB);
    IECInstancePtr childElement = RulesEngineTestHelpers::InsertInstance(m_db, *ecClassC);
    RulesEngineTestHelpers::InsertRelationship(m_db, *rel, *rootElement, *virtualElement, nullptr, false);
    RulesEngineTestHelpers::InsertRelationship(m_db, *rel, *virtualElement, *childElement, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "", ecClassA->GetFullName(), false));

    ChildNodeRule* virtualRule = new ChildNodeRule("ParentNode.ClassName=\"ElementA\"", 1, false, RuleTargetTree::TargetTree_Both);
    virtualRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, true, false, false, false, false,
        "", ecClassB->GetFullName(), false));

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"ElementB\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", ecClassC->GetFullName(), false));

    rules->AddPresentationRule(*rootRule);
    rules->AddPresentationRule(*virtualRule);
    rules->AddPresentationRule(*childRule);
    rules->AddPresentationRule(*new StyleOverride("ThisNode.ClassName=\"ElementC\"", 1, "GetSettingIntValue(\"custom\")", "", ""));

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    NavNodeCPtr rootNode = rootNodes[0];
    SetNodeExpanded(*rootNode);

    // get a child node
    ASSERT_TRUE(rootNode->HasChildren());
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(m_db, *rootNode, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    NavNodeCPtr childNode = childNodes[0];

    // modify and update child
    childElement->SetValue("ElementProperty", ECValue(10));
    ECInstanceUpdater updater(m_db, *childElement, nullptr);
    updater.Update(*childElement);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *childElement);

    rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());

    // expect user settings have been tracked
    bvector<HierarchyLevelInfo> hierarchies = static_cast<RulesDrivenECPresentationManagerImpl&>(m_manager->GetImpl()).GetNodesCache().GetRelatedHierarchyLevels(rules->GetRuleSetId().c_str(), "custom");
    ASSERT_EQ(1, hierarchies.size());
    ASSERT_EQ(m_connections.GetConnection(m_db)->GetId(), hierarchies[0].GetConnectionId());
    ASSERT_EQ(rules->GetRuleSetId(), hierarchies[0].GetRulesetId());
    ASSERT_EQ(rootNodes[0]->GetNodeId(), *hierarchies[0].GetPhysicalParentNodeId());
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2016
+===============+===============+===============+===============+===============+======*/
struct ContentUpdateTests : UpdateTests
    {
    virtual void SetUp() override
        {
        UpdateTests::SetUp();
        m_manager->SetLocalizationProvider(new SQLangLocalizationProvider());
        }
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentUpdateTests, UpdatesContentAfterECInstanceInsert)
    {
    // insert a widget instance
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // request content
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(m_db, nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(*descriptor, PageOptions()).get();
    
    // expect 1 record
    ASSERT_EQ(1, content->GetContentSet().GetSize());
    ASSERT_TRUE(content->GetContentSet()[0].IsValid());
    
    // insert one more instance
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget2);

    // expect 2 records
    content = IECPresentationManager::GetManager().GetContent(*descriptor, PageOptions()).get();
    ASSERT_EQ(2, content->GetContentSet().GetSize());
    ASSERT_STREQ(widget1->GetInstanceId().c_str(), content->GetContentSet()[0]->GetKeys()[0].GetId().ToString().c_str());
    ASSERT_STREQ(widget2->GetInstanceId().c_str(), content->GetContentSet()[1]->GetKeys()[0].GetId().ToString().c_str());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    ASSERT_STREQ("UpdatesContentAfterECInstanceInsert", m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    ASSERT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentUpdateTests, UpdatesContentAfterECInstanceUpdate)
    {
    // insert a widget instance
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // request content
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(m_db, nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(*descriptor, PageOptions()).get();
    
    // expect 1 record
    ASSERT_EQ(1, content->GetContentSet().GetSize());
    ASSERT_TRUE(content->GetContentSet()[0].IsValid());
    
    // notify about an update (even though we didn't change anything)
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget);

    // expect 1 record
    content = IECPresentationManager::GetManager().GetContent(*descriptor, PageOptions()).get();
    ASSERT_EQ(1, content->GetContentSet().GetSize());
    ASSERT_STREQ(widget->GetInstanceId().c_str(), content->GetContentSet()[0]->GetKeys()[0].GetId().ToString().c_str());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    ASSERT_STREQ("UpdatesContentAfterECInstanceUpdate", m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    ASSERT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentUpdateTests, UpdatesContentAfterECInstanceDeleteWhenMoreInstancesExist)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request content
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(m_db, nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(*descriptor, PageOptions()).get();
    
    // expect 2 records
    ASSERT_EQ(2, content->GetContentSet().GetSize());
    ASSERT_TRUE(content->GetContentSet()[0].IsValid());
    ASSERT_TRUE(content->GetContentSet()[1].IsValid());
    
    // delete one of the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget1, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget1);

    // expect 1 record
    content = IECPresentationManager::GetManager().GetContent(*descriptor, PageOptions()).get();
    ASSERT_EQ(1, content->GetContentSet().GetSize());
    ASSERT_STREQ(widget2->GetInstanceId().c_str(), content->GetContentSet()[0]->GetKeys()[0].GetId().ToString().c_str());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    ASSERT_STREQ("UpdatesContentAfterECInstanceDeleteWhenMoreInstancesExist", m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    ASSERT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentUpdateTests, UpdatesContentAfterECInstanceDeleteWhenNoMoreInstancesExist)
    {
    // insert some widget instances
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // request content
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(m_db, nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(*descriptor, PageOptions()).get();
    
    // expect 1 record
    ASSERT_EQ(1, content->GetContentSet().GetSize());
    ASSERT_TRUE(content->GetContentSet()[0].IsValid());
    
    // delete the instance
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget);

    // expect 0 records
    content = IECPresentationManager::GetManager().GetContent(*descriptor, PageOptions()).get();
    ASSERT_EQ(0, content->GetContentSet().GetSize());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    ASSERT_STREQ("UpdatesContentAfterECInstanceDeleteWhenNoMoreInstancesExist", m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    ASSERT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentUpdateTests, UpdatesAllContentBasedOnOneRulesetButSendsOnlyOneNotification_WithNoContent)
    {
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);
    
    // request content and expect none
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor1 = IECPresentationManager::GetManager().GetContentDescriptor(m_db, ContentDisplayType::Graphics, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor1.IsNull());
    ContentDescriptorCPtr descriptor2 = IECPresentationManager::GetManager().GetContentDescriptor(m_db, ContentDisplayType::Grid, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor2.IsNull());

    // delete the instance
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget);

    // still expect no content
    descriptor1 = IECPresentationManager::GetManager().GetContentDescriptor(m_db, ContentDisplayType::Graphics, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor1.IsNull());
    descriptor2 = IECPresentationManager::GetManager().GetContentDescriptor(m_db, ContentDisplayType::Grid, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor2.IsNull());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    ASSERT_STREQ(rules->GetRuleSetId().c_str(), m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    ASSERT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentUpdateTests, UpdatesAllContentBasedOnOneRulesetButSendsOnlyOneNotification_WithContent)
    {
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // set up selection
    KeySetPtr inputKeys = KeySet::Create({RulesEngineTestHelpers::GetInstanceKey(*widget)});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);
    
    // request content
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    ContentDescriptorCPtr descriptor1 = IECPresentationManager::GetManager().GetContentDescriptor(m_db, ContentDisplayType::Graphics, 0, *inputKeys, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor1.IsValid());
    ContentCPtr content1 = IECPresentationManager::GetManager().GetContent(*descriptor1, PageOptions()).get();
    ASSERT_EQ(1, content1->GetContentSet().GetSize());

    ContentDescriptorCPtr descriptor2 = IECPresentationManager::GetManager().GetContentDescriptor(m_db, ContentDisplayType::Grid, 0, *inputKeys, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor2.IsValid());
    ContentCPtr content2 = IECPresentationManager::GetManager().GetContent(*descriptor2, PageOptions()).get();
    ASSERT_EQ(1, content2->GetContentSet().GetSize());

    // delete the instance
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget);
    inputKeys = KeySet::Create();

    // expect no content in both cases
    descriptor1 = IECPresentationManager::GetManager().GetContentDescriptor(m_db, ContentDisplayType::Graphics, 0, *inputKeys, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor1.IsNull());
    descriptor2 = IECPresentationManager::GetManager().GetContentDescriptor(m_db, ContentDisplayType::Grid, 0, *inputKeys, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor2.IsNull());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    ASSERT_STREQ(rules->GetRuleSetId().c_str(), m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    ASSERT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentUpdateTests, UpdatesContentAfterCategoriesChange)
    {
    TestCategorySupplier supplier;
    m_manager->SetCategorySupplier(&supplier);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // request content
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(m_db, nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    
    // expect the fields to have supplied category
    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    for (ContentDescriptor::Field const* field : fields)
        {
        EXPECT_STREQ(field->GetCategory().GetName().c_str(), supplier.GetUsedCategory().GetName().c_str());
        }
    
    // change supplied category
    supplier.SetUsedCategory(ContentDescriptor::Category::GetFavoriteCategory());
    m_manager->NotifyCategoriesChanged();

    // expect the fields to have the new supplied category
    descriptor = IECPresentationManager::GetManager().GetContentDescriptor(m_db, nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    fields = descriptor->GetVisibleFields();
    for (ContentDescriptor::Field const* field : fields)
        {
        EXPECT_STREQ(field->GetCategory().GetName().c_str(), supplier.GetUsedCategory().GetName().c_str());
        }

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    ASSERT_TRUE(m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().empty());
    ASSERT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentUpdateTests, InvalidatesWhenUserSettingChanges_UsedInRuleCondition)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    
    ContentRule* rule = new ContentRule("1 = GetSettingIntValue(\"test\")", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(m_db, nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
        
    // expect the descriptor to be null because no rules applied
    ASSERT_TRUE(descriptor.IsNull());
        
    // change a setting
    m_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", 1);
    
    // expect 1 content item
    descriptor = IECPresentationManager::GetManager().GetContentDescriptor(m_db, nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(*descriptor, PageOptions()).get();
    ASSERT_EQ(1, content->GetContentSet().GetSize());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    EXPECT_STREQ(rules->GetRuleSetId().c_str(), m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    EXPECT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentUpdateTests, InvalidatesWhenUserSettingChanges_UsedInInstanceFilter)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    
    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "1 = GetSettingIntValue(\"test\")", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(m_db, nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(*descriptor, PageOptions()).get();
    
    // expect 0 records
    ASSERT_EQ(0, content->GetContentSet().GetSize());
        
    // change a setting
    m_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", 1);

    // expect 1 content item
    content = IECPresentationManager::GetManager().GetContent(*descriptor, PageOptions()).get();
    ASSERT_EQ(1, content->GetContentSet().GetSize());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    EXPECT_STREQ(rules->GetRuleSetId().c_str(), m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    EXPECT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentUpdateTests, DoesNotInvalidateWhenUnusedUserSettingChanges)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    
    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(m_db, nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(*descriptor, PageOptions()).get();
    
    // expect 1 record
    ASSERT_EQ(1, content->GetContentSet().GetSize());
        
    // change a setting
    m_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", 1);

    // expect the content to be the same
    content = IECPresentationManager::GetManager().GetContent(*descriptor, PageOptions()).get();
    ASSERT_EQ(1, content->GetContentSet().GetSize());

    // expect 0 update records
    ASSERT_EQ(0, m_updateRecordsHandler->GetFullUpdateRecords().size());
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2018
+===============+===============+===============+===============+===============+======*/
struct SelectionUpdateTests : UpdateTests
    {
    TestSelectionManager m_selectionManager;
    RefCountedPtr<SelectionUpdateRecordsHandler> m_updateHandler;

    SelectionUpdateTests(): m_selectionManager(m_connections) {}

    void SetUp() override
        {
        UpdateTests::SetUp();
        m_updateHandler = SelectionUpdateRecordsHandler::Create(m_connections, m_selectionManager);
        m_manager->RegisterUpdateRecordsHandler(*m_updateHandler);
        m_manager->SetLocalizationProvider(new SQLangLocalizationProvider());
        }
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SelectionUpdateTests, RemovesECInstanceNodeKeyFromSelection)
    {
    // insert some widget instances
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    
    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 node
    ASSERT_EQ(1, rootNodes.GetSize());
    NavNodeKeyCPtr key = rootNodes[0]->GetKey();

    // add the node to selection
    m_selectionManager.AddToSelection(m_db, "", false, *key);
    EXPECT_TRUE(m_selectionManager.GetSelection(m_db)->Contains(*key));

    // send update notification
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget);
    Sync();

    // expect the key to *not* be found in the selection
    EXPECT_FALSE(m_selectionManager.GetSelection(m_db)->Contains(*key));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SelectionUpdateTests, RemovesECClassGroupingNodeKeyFromSelection)
    {
    // insert some widget instances
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    
    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, true, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 node
    ASSERT_EQ(1, rootNodes.GetSize());
    NavNodeKeyCPtr key = rootNodes[0]->GetKey();

    // add the node to selection
    m_selectionManager.AddToSelection(m_db, "", false, *key);
    EXPECT_TRUE(m_selectionManager.GetSelection(m_db)->Contains(*key));

    // send update notification
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget);
    Sync();

    // expect the key to *not* be found in the selection
    EXPECT_FALSE(m_selectionManager.GetSelection(m_db)->Contains(*key));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SelectionUpdateTests, RemovesECPropertyGroupingNodeKeyFromSelection)
    {
    // insert some widget instances
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    
    rules->AddPresentationRule(*new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", ""));
    PropertyGroupP groupSpec = new PropertyGroup("", "", true, "Description");
    groupSpec->SetCreateGroupForUnspecifiedValues(true);
    rules->GetGroupingRules().back()->AddGroup(*groupSpec);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 1 node
    ASSERT_EQ(1, rootNodes.GetSize());
    NavNodeKeyCPtr key = rootNodes[0]->GetKey();

    // add the node to selection
    m_selectionManager.AddToSelection(m_db, "", false, *key);
    EXPECT_TRUE(m_selectionManager.GetSelection(m_db)->Contains(*key));

    // send update notification
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget);
    Sync();

    // expect the key to be found in the selection
    EXPECT_FALSE(m_selectionManager.GetSelection(m_db)->Contains(*key));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SelectionUpdateTests, RemovesDisplayLabelGroupingNodeKeyFromSelection)
    {
    // insert some widget instances
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Other Label"));}, true);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName=\"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    
    // expect 2 grouping nodes
    ASSERT_EQ(2, rootNodes.GetSize());
    NavNodeKeyCPtr key = rootNodes[0]->GetKey();

    // add the node to selection
    m_selectionManager.AddToSelection(m_db, "", false, *key);
    EXPECT_TRUE(m_selectionManager.GetSelection(m_db)->Contains(*key));

    // send update notification
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget);
    Sync();

    // expect the key to be found in the selection
    EXPECT_FALSE(m_selectionManager.GetSelection(m_db)->Contains(*key));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SelectionUpdateTests, RefreshesSelectionWhenAffectedECInstanceNodeChanges)
    {
    bvector<SelectionChangedEventCPtr> selectionChangeEvents;
    TestSelectionChangesListener listener([&](SelectionChangedEventCR evt){selectionChangeEvents.push_back(&evt);});
    m_selectionManager.AddListener(listener);

    // insert some widget instances
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    rootRule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rootRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();

    // expect 1 node
    ASSERT_EQ(1, rootNodes.GetSize());
    NavNodeKeyCPtr key = rootNodes[0]->GetKey();

    // add node to selection
    m_selectionManager.AddToSelection(m_db, "", false, *key);
    EXPECT_TRUE(m_selectionManager.GetSelection(m_db)->Contains(*key));
    selectionChangeEvents.clear();

    // update instance
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget);
    Sync();

    // expect a selection change event to be fired
    ASSERT_EQ(1, selectionChangeEvents.size());
    EXPECT_EQ(SelectionChangeType::Replace, selectionChangeEvents[0]->GetChangeType());
    EXPECT_EQ(1, selectionChangeEvents[0]->GetSelectedKeys().size());
    EXPECT_TRUE(selectionChangeEvents[0]->GetSelectedKeys().Contains(*key));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SelectionUpdateTests, DoesntRefreshSelectionWhenUnaffectedECInstanceNodeChanges)
    {
    bvector<SelectionChangedEventCPtr> selectionChangeEvents;
    TestSelectionChangesListener listener([&](SelectionChangedEventCR evt){selectionChangeEvents.push_back(&evt);});
    m_selectionManager.AddListener(listener);

    // insert some instances
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    rootRule->AddSpecification(*new CustomNodeSpecification(1, false, "custom1", "custom", "custom", "custom"));
    rootRule->AddSpecification(*new CustomNodeSpecification(1, false, "custom2", "custom", "custom", "custom"));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRuleP childRule1 = new ChildNodeRule("ParentNode.Type = \"custom1\"", 1, false, RuleTargetTree::TargetTree_MainTree);
    childRule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false,
        "", m_widgetClass->GetFullName(), false));
    rules->AddPresentationRule(*childRule1);

    ChildNodeRuleP childRule2 = new ChildNodeRule("ParentNode.Type = \"custom2\"", 1, false, RuleTargetTree::TargetTree_MainTree);
    childRule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false,
        "", m_gadgetClass->GetFullName(), false));
    rules->AddPresentationRule(*childRule2);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, rootNodes.GetSize());

    // request child nodes
    DataContainer<NavNodeCPtr> childNodes1 = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    DataContainer<NavNodeCPtr> childNodes2 = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[1], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes2.GetSize());
    NavNodeKeyCPtr key = childNodes2[0]->GetKey();

    // add node to selection
    m_selectionManager.AddToSelection(m_db, "", false, *key);
    EXPECT_TRUE(m_selectionManager.GetSelection(m_db)->Contains(*key));
    selectionChangeEvents.clear();

    // update instance
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget);
    Sync();

    // expect no selection change events
    EXPECT_EQ(0, selectionChangeEvents.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SelectionUpdateTests, RefreshesSelectionWhenAffectedGroupingNodeChanges)
    {
    bvector<SelectionChangedEventCPtr> selectionChangeEvents;
    TestSelectionChangesListener listener([&](SelectionChangedEventCR evt){selectionChangeEvents.push_back(&evt);});
    m_selectionManager.AddListener(listener);

    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("IntProperty", ECValue(2)); });
    RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("IntProperty", ECValue(2)); }, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    GroupingRuleP grouping = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    grouping->AddGroup(*new PropertyGroup("", "", false, "IntProperty"));
    rootRule->AddCustomizationRule(*grouping);

    rootRule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rootRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();

    // expect 1 grouping node
    ASSERT_EQ(1, rootNodes.GetSize());
    NavNodeKeyCPtr key = rootNodes[0]->GetKey();

    // add node to selection
    m_selectionManager.AddToSelection(m_db, "", false, *key);
    EXPECT_TRUE(m_selectionManager.GetSelection(m_db)->Contains(*key));
    selectionChangeEvents.clear();

    // insert new instance
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("IntProperty", ECValue(2));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget);
    Sync();

    // expect a selection change event to be fired
    ASSERT_EQ(1, selectionChangeEvents.size());
    EXPECT_EQ(SelectionChangeType::Replace, selectionChangeEvents[0]->GetChangeType());
    EXPECT_EQ(1, selectionChangeEvents[0]->GetSelectedKeys().size());
    EXPECT_TRUE(selectionChangeEvents[0]->GetSelectedKeys().Contains(*key));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SelectionUpdateTests, DoesntRefreshSelectionWhenUnaffectedGroupingNodeChanges)
    {
    bvector<SelectionChangedEventCPtr> selectionChangeEvents;
    TestSelectionChangesListener listener([&](SelectionChangedEventCR evt){selectionChangeEvents.push_back(&evt);});
    m_selectionManager.AddListener(listener);

    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    rootRule->AddSpecification(*new CustomNodeSpecification(1, false, "custom1", "custom", "custom", "custom"));
    rootRule->AddSpecification(*new CustomNodeSpecification(1, false, "custom2", "custom", "custom", "custom"));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRuleP childRule1 = new ChildNodeRule("ParentNode.Type = \"custom1\"", 1, false, RuleTargetTree::TargetTree_MainTree);
    childRule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, true, false, false,
        "", m_widgetClass->GetFullName(), false));
    rules->AddPresentationRule(*childRule1);

    ChildNodeRuleP childRule2 = new ChildNodeRule("ParentNode.Type = \"custom2\"", 1, false, RuleTargetTree::TargetTree_MainTree);
    childRule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, true, false, false,
        "", m_gadgetClass->GetFullName(), false));
    rules->AddPresentationRule(*childRule2);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(m_db, PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, rootNodes.GetSize());

    // request child nodes
    DataContainer<NavNodeCPtr> childNodes1 = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    DataContainer<NavNodeCPtr> childNodes2 = IECPresentationManager::GetManager().GetChildren(m_db, *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes2.GetSize());
    NavNodeKeyCPtr key = childNodes2[0]->GetKey();

    // add node to selection
    m_selectionManager.AddToSelection(m_db, "", false, *key);
    EXPECT_TRUE(m_selectionManager.GetSelection(m_db)->Contains(*key));
    selectionChangeEvents.clear();

    // insert a new gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget);
    Sync();

    // expect no selection change events
    EXPECT_EQ(0, selectionChangeEvents.size());
    }