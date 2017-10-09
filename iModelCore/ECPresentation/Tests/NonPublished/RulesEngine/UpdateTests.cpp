/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/UpdateTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "../../../Source/RulesDriven/RulesEngine/NavNodesCache.h"
#include <UnitTests/BackDoor/ECPresentation/TestRuleSetLocater.h>
#include "ECDbTestProject.h"
#include "TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

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
struct UpdateTests : ::testing::Test
{
    static ECDbTestProject* s_project;
    static TestRuleSetLocaterPtr s_locater;
    static StubLocalState s_localState;
    static RulesDrivenECPresentationManager* s_manager;
    static RefCountedPtr<TestECInstanceChangeEventsSource> s_eventsSource;
    RefCountedPtr<TestUpdateRecordsHandler> m_updateRecordsHandler;
    
    ECSchemaCP m_schema;
    ECClassCP m_widgetClass;
    ECClassCP m_gadgetClass;
    ECClassCP m_sprocketClass;

    public: static void SetUpTestCase();
    public: static void TearDownTestCase();

    void SetUp() override
        {
        s_localState.GetStubMap().clear();

        if (!s_project->GetECDb().GetDefaultTransaction()->IsActive())
            s_project->GetECDb().GetDefaultTransaction()->Begin();

        m_updateRecordsHandler = TestUpdateRecordsHandler::Create();
        s_manager->RegisterUpdateRecordsHandler(m_updateRecordsHandler.get());

        m_schema = s_project->GetECDb().Schemas().GetSchema("RulesEngineTest");
        m_widgetClass = m_schema->GetClassCP("Widget");
        m_gadgetClass = m_schema->GetClassCP("Gadget");
        m_sprocketClass = m_schema->GetClassCP("Sprocket");

        Localization::Init();
        IECPresentationManager::GetManager().GetConnections().NotifyConnectionOpened(s_project->GetECDb());
        }

    void TearDown() override
        {
        IECPresentationManager::GetManager().GetConnections().NotifyConnectionClosed(s_project->GetECDb());
        
        s_manager->SetCategorySupplier(nullptr);
        s_manager->RegisterUpdateRecordsHandler(nullptr);
        s_project->GetECDb().GetDefaultTransaction()->Cancel();
        s_locater->Clear();
        
        Localization::Terminate();
        }

    Utf8String GetDisplayLabel(IECInstanceCR instance)
        {
        Utf8String label;
        if (ECObjectsStatus::Success == instance.GetDisplayLabel(label))
            return Utf8String(label.c_str());

        return Utf8String(instance.GetClass().GetDisplayLabel().c_str());
        }
};
ECDbTestProject* UpdateTests::s_project = nullptr;
TestRuleSetLocaterPtr UpdateTests::s_locater;
StubLocalState UpdateTests::s_localState;
RulesDrivenECPresentationManager* UpdateTests::s_manager = nullptr;
RefCountedPtr<TestECInstanceChangeEventsSource> UpdateTests::s_eventsSource = TestECInstanceChangeEventsSource::Create();

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateTests::SetUpTestCase()
    {
    BeFileName temporaryDirectory;
    BeTest::GetHost().GetTempDir(temporaryDirectory);

    s_locater = TestRuleSetLocater::Create();

    s_project = new ECDbTestProject();
    s_project->Create("UpdateTests", "RulesEngineTest.01.00.ecschema.xml");
    
    s_manager = new RulesDrivenECPresentationManager(RulesEngineTestHelpers::GetPaths(BeTest::GetHost()));
    s_manager->SetLocalState(s_localState);
    s_manager->GetLocaters().RegisterLocater(*s_locater);
    s_manager->RegisterECInstanceChangeEventSource(*s_eventsSource);
    IECPresentationManager::RegisterImplementation(s_manager);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateTests::TearDownTestCase()
    {
    s_manager->UnregisterECInstanceChangeEventSource(*s_eventsSource);
    s_manager->GetLocaters().UnregisterLocater(*s_locater);
    IECPresentationManager::RegisterImplementation(nullptr);
    delete s_manager;
    s_locater = nullptr;

    delete s_project;
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
        s_manager->GetNodesCache().Update(jsonNode.GetNodeId(), jsonNode);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RemovesECInstanceNodeAfterECInstanceDelete)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RemovesECInstanceNodeAfterECInstanceDelete", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options("RemovesECInstanceNodeAfterECInstanceDelete", TargetTree_Both);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 2 nodes
    ASSERT_EQ(2, nodes.GetSize());
    ASSERT_TRUE(nodes[0].IsValid());
    ASSERT_TRUE(nodes[1].IsValid());
    NavNodeCPtr removedNode = nodes[0];
    NavNodeCPtr retainedNode = nodes[1];

    // delete one of the instances
    RulesEngineTestHelpers::DeleteInstance(*s_project, *widget1);
    s_eventsSource->NotifyECInstanceDeleted(s_project->GetECDbCR(), *widget1);
    
    // expect 1 node
    nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_EQ(retainedNode->GetKey(), nodes[0]->GetKey());
    
    // expect two update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(removedNode->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(nodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesECClassGroupingNodeChildrenAfterECInstanceDeleteWhenInstancesLeft)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesECClassGroupingNodeChildrenAfterECInstanceDeleteWhenInstancesLeft", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, true, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("UpdatesECClassGroupingNodeChildrenAfterECInstanceDeleteWhenInstancesLeft", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 1 class grouping node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECClassGroupingNode));

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // request its children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    
    // expect 2 instance nodes
    ASSERT_EQ(2, childNodes.GetSize());
    ASSERT_TRUE(childNodes[0].IsValid());
    ASSERT_TRUE(childNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_TRUE(childNodes[1].IsValid());
    ASSERT_TRUE(childNodes[1]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    NavNodeCPtr removedNode = childNodes[0];

    // delete one of the instances
    RulesEngineTestHelpers::DeleteInstance(*s_project, *widget1);
    s_eventsSource->NotifyECInstanceDeleted(s_project->GetECDbCR(), *widget1);

    // expect the same one class grouping node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECClassGroupingNode));

    // expect it to have only one instance node
    childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childNodes.GetSize());
    ASSERT_TRUE(childNodes[0].IsValid());
    ASSERT_TRUE(childNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_STREQ(widget2->GetInstanceId().c_str(), childNodes[0]->GetInstance()->GetInstanceId().c_str());

    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(removedNode->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(childNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RemovesECClassGroupingNodeAfterECInstanceDeleteWhenNoECInstancesLeft_WhenChildrenCached)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RemovesECClassGroupingNodeAfterECInstanceDeleteWhenNoECInstancesLeft_WhenChildrenCached", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, true, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    NavNodeCPtr removedClassNode;
    bvector<NavNodeCPtr> removedChildNodes;

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("RemovesECClassGroupingNodeAfterECInstanceDeleteWhenNoECInstancesLeft_WhenChildrenCached", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 1 class grouping node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECClassGroupingNode));
    removedClassNode = rootNodes[0];

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // request its children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    
    // expect 2 instance nodes
    ASSERT_EQ(2, childNodes.GetSize());
    ASSERT_TRUE(childNodes[0].IsValid());
    ASSERT_TRUE(childNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_TRUE(childNodes[1].IsValid());
    ASSERT_TRUE(childNodes[1]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    removedChildNodes.push_back(childNodes[0]);
    removedChildNodes.push_back(childNodes[1]);

    // delete both instances
    ECInstanceDeleter deleter(s_project->GetECDbCR(), *m_widgetClass, nullptr);
    deleter.Delete(*widget1);
    deleter.Delete(*widget2);

    bvector<IECInstanceCP> deletedInstances;
    deletedInstances.push_back(widget1.get());
    deletedInstances.push_back(widget2.get());
    s_eventsSource->NotifyECInstancesDeleted(s_project->GetECDbCR(), deletedInstances);

    // expect the class grouping node to be gone as there're no instances left to group
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(0, rootNodes.GetSize());

    // expect one update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());
    ASSERT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    ASSERT_EQ(removedClassNode->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RemovesECClassGroupingNodeAfterECInstanceDeleteWhenNoECInstancesLeft_WhenChildrenNotCached)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RemovesECClassGroupingNodeAfterECInstanceDeleteWhenNoECInstancesLeft_WhenChildrenNotCached", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, true, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("RemovesECClassGroupingNodeAfterECInstanceDeleteWhenNoECInstancesLeft_WhenChildrenNotCached", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 1 class grouping node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECClassGroupingNode));
    NavNodeCPtr removedNode = rootNodes[0];

    // delete both instances
    ECInstanceDeleter deleter(s_project->GetECDbCR(), *m_widgetClass, nullptr);
    deleter.Delete(*widget1);
    deleter.Delete(*widget2);

    bvector<IECInstanceCP> deletedInstances;
    deletedInstances.push_back(widget1.get());
    deletedInstances.push_back(widget2.get());
    s_eventsSource->NotifyECInstancesDeleted(s_project->GetECDbCR(), deletedInstances);

    // expect the class grouping node to be gone as there're no instances left to group
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(0, rootNodes.GetSize());

    // expect one update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    ASSERT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    ASSERT_EQ(removedNode->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RemovesDisplayLabelGroupingNodeChildrenAfterECInstanceDeleteWhenInstancesLeft)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Other Label"));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RemovesDisplayLabelGroupingNodeChildrenAfterECInstanceDeleteWhenInstancesLeft", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("RemovesDisplayLabelGroupingNodeChildrenAfterECInstanceDeleteWhenInstancesLeft", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 1 display label grouping node & 1 ECInstance node
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode));
    NavNodeCPtr displayLabelGroupingNode = rootNodes[0];
    NavNodeCPtr retainedNode = rootNodes[1];

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // request label grouping nodes children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    
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
    RulesEngineTestHelpers::DeleteInstance(*s_project, *widget1);
    s_eventsSource->NotifyECInstanceDeleted(s_project->GetECDbCR(), *widget1);

    // expect the display label grouping node to be gone (single instance is not grouped under a display label grouping node)
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_TRUE(rootNodes[0]->GetLabel().Equals("My Label"));
    ASSERT_EQ(retainedNode->GetKey(), rootNodes[1]->GetKey());
    
    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());

    ASSERT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    ASSERT_EQ(displayLabelGroupingNode->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    ASSERT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    ASSERT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetNode()->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetPosition());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(rootNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RemovesDisplayLabelGroupingNodeAfterECInstanceDeleteWhenNoInstancesLeft_WhenChildrenCached)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Other Label"));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RemovesDisplayLabelGroupingNodeAfterECInstanceDeleteWhenNoInstancesLeft_WhenChildrenCached", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    NavNodeCPtr removedLabelGroupingNode;
    bvector<NavNodeCPtr> removedInstanceNodes;

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("RemovesDisplayLabelGroupingNodeAfterECInstanceDeleteWhenNoInstancesLeft_WhenChildrenCached", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
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
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    
    // expect 2 instance nodes
    ASSERT_EQ(2, childNodes.GetSize());
    ASSERT_TRUE(childNodes[0].IsValid());
    ASSERT_TRUE(childNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_TRUE(childNodes[1].IsValid());
    ASSERT_TRUE(childNodes[1]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    removedInstanceNodes.push_back(childNodes[0]);
    removedInstanceNodes.push_back(childNodes[1]);

    // delete both instances
    ECInstanceDeleter deleter(s_project->GetECDbCR(), *m_widgetClass, nullptr);
    deleter.Delete(*widget1);
    deleter.Delete(*widget2);

    bvector<IECInstanceCP> deletedInstances;
    deletedInstances.push_back(widget1.get());
    deletedInstances.push_back(widget2.get());
    s_eventsSource->NotifyECInstancesDeleted(s_project->GetECDbCR(), deletedInstances);

    // expect the display label grouping node to be gone as there're no instances left to group
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_TRUE(rootNodes[0]->GetLabel().Equals("Other Label"));

    // expect one update record
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    ASSERT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    ASSERT_EQ(removedLabelGroupingNode->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RemovesDisplayLabelGroupingNodeAfterECInstanceDeleteWhenNoInstancesLeft_WhenChildrenNotCached)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Other Label"));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RemovesDisplayLabelGroupingNodeAfterECInstanceDeleteWhenNoInstancesLeft_WhenChildrenNotCached", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("RemovesDisplayLabelGroupingNodeAfterECInstanceDeleteWhenNoInstancesLeft_WhenChildrenNotCached", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 1 display label grouping node & 1 ECInstance node
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode));
    NavNodeCPtr removedNode = rootNodes[0];

    // delete both instances
    ECInstanceDeleter deleter(s_project->GetECDbCR(), *m_widgetClass, nullptr);
    deleter.Delete(*widget1);
    deleter.Delete(*widget2);

    bvector<IECInstanceCP> deletedInstances;
    deletedInstances.push_back(widget1.get());
    deletedInstances.push_back(widget2.get());
    s_eventsSource->NotifyECInstancesDeleted(s_project->GetECDbCR(), deletedInstances);

    // expect the display label grouping node to be gone as there're no instances left to group
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_TRUE(rootNodes[0]->GetLabel().Equals("Other Label"));
    
    // expect one update record
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    ASSERT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    ASSERT_EQ(removedNode->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RemovesDisplayLabelGroupingNodeAfterECInstanceLabelChange)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Other Label"));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RemovesDisplayLabelGroupingNodeAfterECInstanceLabelChange", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("RemovesDisplayLabelGroupingNodeAfterECInstanceLabelChange", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 1 display label grouping node & 1 ECInstance node
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode));
    NavNodeCPtr displayLabelGroupingNode = rootNodes[0];

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // request label grouping nodes children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    
    // expect 2 instance nodes
    ASSERT_EQ(2, childNodes.GetSize());
    ASSERT_TRUE(childNodes[0].IsValid());
    ASSERT_TRUE(childNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_TRUE(childNodes[1].IsValid());
    ASSERT_TRUE(childNodes[1]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));

    // change the label of one of the instances
    widget1->SetValue("MyID", ECValue("My Other Label"));
    ECInstanceUpdater updater(s_project->GetECDbCR(), *widget1, nullptr);
    updater.Update(*widget1);
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *widget1);

    // expect the display label grouping node to be gone (single instance is not grouped under a display label grouping node)
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
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
    ASSERT_EQ(displayLabelGroupingNode->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    ASSERT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetPosition());

    ASSERT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(rootNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[2].GetPosition());
        
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[3].GetChangeType());
    EXPECT_EQ(rootNodes[2]->GetKey(), m_updateRecordsHandler->GetRecords()[3].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, SetsCorrectInsertPositionWhenMultipleSpecificationsUsedAtTheSameLevel1)
    {    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SetsCorrectInsertPositionWhenMultipleSpecificationsUsedAtTheSameLevel1", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect no nodes
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // create a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *widget);

    // expect one widget instance node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_TRUE(rootNodes[0]->GetLabel().Equals("Widget"));
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, SetsCorrectInsertPositionWhenMultipleSpecificationsUsedAtTheSameLevel2)
    {    
    // insert some gadgets
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SetsCorrectInsertPositionWhenMultipleSpecificationsUsedAtTheSameLevel2", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 2 gadget nodes
    ASSERT_EQ(2, rootNodes.GetSize());
    
    // create a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *widget);

    // expect one additional widget instance node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(3, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[2].IsValid());
    ASSERT_TRUE(rootNodes[2]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode));
    ASSERT_TRUE(rootNodes[2]->GetLabel().Equals("Widget"));
    
    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(rootNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(rootNodes[2]->GetKey(), m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[2].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, CreatesDisplayLabelGroupingNodeAfterECInstanceLabelChange)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 2"));});
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 3"));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("CreatesDisplayLabelGroupingNodeAfterECInstanceLabelChange", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("CreatesDisplayLabelGroupingNodeAfterECInstanceLabelChange", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
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
    ECInstanceUpdater updater(s_project->GetECDbCR(), *widget1, nullptr);
    updater.Update(*widget1);
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *widget1);

    // expect the display label grouping node to be created
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
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
    ASSERT_EQ(deletedNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    ASSERT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    ASSERT_EQ(deletedNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());

    ASSERT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    ASSERT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[2].GetPosition());
    
    ASSERT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[3].GetChangeType());
    ASSERT_EQ(rootNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[3].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, CreatesDisplayLabelGroupingNodeAfterECInstanceInsert)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 2"));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("CreatesDisplayLabelGroupingNodeAfterECInstanceLabelChange", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("CreatesDisplayLabelGroupingNodeAfterECInstanceLabelChange", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 2 ECInstance nodes
    ASSERT_EQ(2, rootNodes.GetSize());
    for (NavNodeCPtr node : rootNodes)
        {
        ASSERT_TRUE(node.IsValid());
        ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
        }

    NavNodeCPtr deletedNode = rootNodes[1];
    
    // change the label of one of the instances
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 2"));});
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *widget3);

    // expect the display label grouping node to be created
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
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
    ASSERT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    ASSERT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    ASSERT_EQ(deletedNode->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());

    ASSERT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    ASSERT_EQ(rootNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[2].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesDisplayLabelGroupingNodeAfterECInstanceInsert)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 2"));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("CreatesDisplayLabelGroupingNodeAfterECInstanceLabelChange", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("CreatesDisplayLabelGroupingNodeAfterECInstanceLabelChange", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(2, rootNodes.GetSize());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // request display label grouping node children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(2, childNodes.GetSize());
    for (NavNodeCPtr const& node : childNodes)
        {
        EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
        EXPECT_STREQ("Label 1", node->GetLabel().c_str());
        }
    
    // change the label of one of the instances
    IECInstancePtr widget4 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *widget4);

    // expect the display label grouping node to have 3 children now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(3, childNodes.GetSize());
    for (NavNodeCPtr const& node : childNodes)
        {
        EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
        EXPECT_STREQ("Label 1", node->GetLabel().c_str());
        }
    
    // expect 5 update records
    ASSERT_EQ(5, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(rootNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(childNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[3].GetChangeType());
    EXPECT_EQ(childNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[3].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[4].GetChangeType());
    EXPECT_EQ(childNodes[2]->GetKey(), m_updateRecordsHandler->GetRecords()[4].GetNode()->GetKey());
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[4].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesVirtualDisplayLabelGroupingNodeAfterECInstanceInsert)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesVirtualDisplayLabelGroupingNodeAfterECInstanceInsert", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("UpdatesVirtualDisplayLabelGroupingNodeAfterECInstanceInsert", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(2, rootNodes.GetSize());
    for (NavNodeCPtr const& node : rootNodes)
        {
        EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
        EXPECT_STREQ("Label 1", node->GetLabel().c_str());
        }
    
    // insert another widget
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *widget3);

    // expect 3 root nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(3, rootNodes.GetSize());
    for (NavNodeCPtr const& node : rootNodes)
        {
        EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
        EXPECT_STREQ("Label 1", node->GetLabel().c_str());
        }
    
    // expect 3 update records
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(rootNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(rootNodes[2]->GetKey(), m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[2].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesVirtualDisplayLabelGroupingNodeAfterECInstanceDelete)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesVirtualDisplayLabelGroupingNodeAfterECInstanceInsert", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("UpdatesVirtualDisplayLabelGroupingNodeAfterECInstanceInsert", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(3, rootNodes.GetSize());
    for (NavNodeCPtr const& node : rootNodes)
        {
        EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
        EXPECT_STREQ("Label 1", node->GetLabel().c_str());
        }
    NavNodeCPtr deletedNode = rootNodes[1];
    
    // delete one of the instances
    RulesEngineTestHelpers::DeleteInstance(*s_project, *widget2);
    s_eventsSource->NotifyECInstanceDeleted(s_project->GetECDbCR(), *widget2);

    // expect 2 root nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(2, rootNodes.GetSize());
    for (NavNodeCPtr const& node : rootNodes)
        {
        EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
        EXPECT_STREQ("Label 1", node->GetLabel().c_str());
        }
    
    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(deletedNode->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
        
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(rootNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesECInstanceNodeAfterECInstanceChange)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesECInstanceNodeAfterECInstanceChange", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("UpdatesECInstanceNodeAfterECInstanceChange", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());
    for (NavNodeCPtr node : rootNodes)
        {
        ASSERT_TRUE(node.IsValid());
        ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
        }

    // change the label of one of the instances
    widget1->SetValue("MyID", ECValue("Label 2"));
    ECInstanceUpdater updater(s_project->GetECDbCR(), *widget1, nullptr);
    updater.Update(*widget1);
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *widget1);

    // expect the label of the node to be changed
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_STREQ("Label 2", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesRootDataSourceAfterECInstanceInsert_AllInstanceNodesSpecification)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("1"));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesRootDataSourceAfterECInstanceInsert_AllInstanceNodesSpecification", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("UpdatesRootDataSourceAfterECInstanceInsert_AllInstanceNodesSpecification", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());

    // insert another instance
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("2"));});
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *widget2);

    // expect 2 nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
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
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(rootNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesRootDataSourceAfterECInstanceInsert_InstancesOfSpecificClassesSpecification)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("1"));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesRootDataSourceAfterECInstanceInsert_InstancesOfSpecificClassesSpecification", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("UpdatesRootDataSourceAfterECInstanceInsert_InstancesOfSpecificClassesSpecification", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());

    // insert another instance
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("2"));});
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *widget2);

    // expect 2 nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
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
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(rootNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
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
    IECInstancePtr instanceE = RulesEngineTestHelpers::InsertInstance(*s_project, *classE);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesRootDataSourceAfterECInstanceInsert_InstancesOfSpecificClassesSpecification_NonPolymorphicMatch", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassE", false));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("UpdatesRootDataSourceAfterECInstanceInsert_InstancesOfSpecificClassesSpecification_NonPolymorphicMatch", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());

    // insert F instance
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(*s_project, *classF);
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *instanceF);

    // still expect 1 node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
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
    IECInstancePtr instanceE = RulesEngineTestHelpers::InsertInstance(*s_project, *classE);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesRootDataSourceAfterECInstanceInsert_InstancesOfSpecificClassesSpecification_NonPolymorphicMatch", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassE", true));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("UpdatesRootDataSourceAfterECInstanceInsert_InstancesOfSpecificClassesSpecification_NonPolymorphicMatch", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());

    // insert F instance
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(*s_project, *classF);
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *instanceF);

    // expect 2 nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(2, rootNodes.GetSize());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(rootNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, DoesntUpdateRootDataSourceAfterECInstanceInsertIfClassDoesntMatch_InstancesOfSpecificClassesSpecification)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("1"));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("DoesntUpdateRootDataSourceAfterECInstanceInsertIfClassDoesntMatch_InstancesOfSpecificClassesSpecification", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("DoesntUpdateRootDataSourceAfterECInstanceInsertIfClassDoesntMatch_InstancesOfSpecificClassesSpecification", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());

    // insert a gadget instance
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *gadget);

    // still expect 1 node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
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
    ECRelationshipClassCP widgetHasGadgetsRelationshipClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();

    // insert the root instance
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesDataSourceAfterECInstanceInsert_RelatedInstancesSpecification", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.IsInstanceNode", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection::RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("UpdatesDataSourceAfterECInstanceInsert_RelatedInstancesSpecification", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // request for children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(0, childNodes.GetSize());

    // insert a gadget instance
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *widgetHasGadgetsRelationshipClass, *widget, *gadget);
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *gadget);

    // expect 1 node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ("Gadget", childNodes[0]->GetLabel().c_str());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(childNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesDataSourceAfterECInstanceInsert_SearchResultInstanceNodesSpecificationWithSingleQuerySpecification)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(12));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(22));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesDataSourceAfterECInstanceInsert_SearchResultInstanceNodesSpecificationWithSingleQuerySpecification", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rules->AddPresentationRule(*rule);

    SearchResultInstanceNodesSpecificationP spec = new SearchResultInstanceNodesSpecification(1, false, false, false, false, false);
    spec->AddQuerySpecification(*new StringQuerySpecification("SELECT [Widget].[MyID] FROM [RulesEngineTest].[Widget] WHERE [Widget].[IntProperty] > 10", "RulesEngineTest", "Widget"));
    rule->AddSpecification(*spec);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("UpdatesDataSourceAfterECInstanceInsert_SearchResultInstanceNodesSpecificationWithSingleQuerySpecification", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 2 ECInstance nodes
    ASSERT_EQ(2, rootNodes.GetSize());

    // insert another instance
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(32));});
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *widget3);
    ECValue v;

    // expect 3 nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(3, rootNodes.GetSize());

    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    rootNodes[0]->GetInstance()->GetValue(v, "IntProperty");
    ASSERT_EQ(12, v.GetInteger());

    ASSERT_TRUE(rootNodes[1].IsValid());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[1]->GetType().c_str());
    rootNodes[1]->GetInstance()->GetValue(v, "IntProperty");
    ASSERT_EQ(22, v.GetInteger());

    ASSERT_TRUE(rootNodes[2].IsValid());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[2]->GetType().c_str());
    rootNodes[2]->GetInstance()->GetValue(v, "IntProperty");
    ASSERT_EQ(32, v.GetInteger());
    
    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(rootNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(rootNodes[2]->GetKey(), m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[2].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesDataSourceAfterECInstanceInsert_SearchResultInstanceNodesSpecificationWithMultipleQuerySpecifications)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(12));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(22));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesDataSourceAfterECInstanceInsert_SearchResultInstanceNodesSpecificationWithMultipleQuerySpecifications", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rules->AddPresentationRule(*rule);

    SearchResultInstanceNodesSpecificationP spec = new SearchResultInstanceNodesSpecification(1, false, false, false, false, false);
    spec->AddQuerySpecification(*new StringQuerySpecification("SELECT [Widget].[MyID] FROM [RulesEngineTest].[Widget] WHERE [Widget].[IntProperty] > 20", "RulesEngineTest", "Widget"));
    spec->AddQuerySpecification(*new StringQuerySpecification("SELECT [Widget].[MyID] FROM [RulesEngineTest].[Widget] WHERE [Widget].[IntProperty] < 15", "RulesEngineTest", "Widget"));
    rule->AddSpecification(*spec);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("UpdatesDataSourceAfterECInstanceInsert_SearchResultInstanceNodesSpecificationWithMultipleQuerySpecifications", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 2 ECInstance nodes
    ASSERT_EQ(2, rootNodes.GetSize());

    // insert another instance
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(32));});
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *widget3);
    ECValue v;
    
    // expect 3 nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(3, rootNodes.GetSize());

    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    rootNodes[0]->GetInstance()->GetValue(v, "IntProperty");
    ASSERT_EQ(22, v.GetInteger());

    ASSERT_TRUE(rootNodes[1].IsValid());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[1]->GetType().c_str());
    rootNodes[1]->GetInstance()->GetValue(v, "IntProperty");
    ASSERT_EQ(32, v.GetInteger());

    ASSERT_TRUE(rootNodes[2].IsValid());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[2]->GetType().c_str());
    rootNodes[2]->GetInstance()->GetValue(v, "IntProperty");
    ASSERT_EQ(12, v.GetInteger());
    
    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(rootNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetPosition());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(rootNodes[2]->GetKey(), m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesDataSourceAfterECInstanceInsert_SearchResultInstanceNodesSpecification_Polymorphic)
    {
    ECClassCP classE = m_schema->GetClassCP("ClassE");
    ECClassCP classF = m_schema->GetClassCP("ClassF");

    // insert some ClassE instances
    IECInstancePtr instanceE = RulesEngineTestHelpers::InsertInstance(*s_project, *classE, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(12));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesDataSourceAfterECInstanceInsert_SearchResultInstanceNodesSpecification_Polymorphic", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rules->AddPresentationRule(*rule);

    SearchResultInstanceNodesSpecificationP spec = new SearchResultInstanceNodesSpecification(1, false, false, false, false, false);
    spec->AddQuerySpecification(*new StringQuerySpecification("SELECT * FROM [RulesEngineTest].[ClassE] WHERE [ClassE].[IntProperty] > 10", "RulesEngineTest", "ClassE"));
    rule->AddSpecification(*spec);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("UpdatesDataSourceAfterECInstanceInsert_SearchResultInstanceNodesSpecification_Polymorphic", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());

    // insert ClassF instance
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(*s_project, *classF, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(32));});
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *instanceF);
    ECValue v;
    
    // expect 2 nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(2, rootNodes.GetSize());

    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    rootNodes[0]->GetInstance()->GetValue(v, "IntProperty");
    ASSERT_EQ(12, v.GetInteger());

    ASSERT_TRUE(rootNodes[1].IsValid());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[1]->GetType().c_str());
    rootNodes[1]->GetInstance()->GetValue(v, "IntProperty");
    ASSERT_EQ(32, v.GetInteger());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(rootNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
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
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("Description", ECValue(s_query1));
        });
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relationship, *widget, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relationship, *widget, *gadget2);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesDataSourceAfterParentECInstanceUpdate_SearchResultInstanceNodesSpecification", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

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
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // request for children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(2, childNodes.GetSize());

    // change the query in widget Description property
    widget->SetValue("Description", ECValue(s_query2));
    ECInstanceUpdater updater(s_project->GetECDb(), *m_widgetClass, nullptr);
    updater.Update(*widget);
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *widget);
    ECValue v;
    
    // expect 1 child node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childNodes.GetSize());
    
    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());
    
    ASSERT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    ASSERT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    ASSERT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    ASSERT_EQ(childNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());

    ASSERT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    ASSERT_STREQ(gadget2->GetInstanceId().c_str(), m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey().AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, CreatesNodeAfterECInstanceInsertWhenPreviouslyNotCreatedDueToHideIfNoChildrenFlag)
    {
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("CreatesNodeAfterECInstanceInsertWhenPreviouslyNotCreatedDueToHideIfNoChildrenFlag", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

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
    RulesDrivenECPresentationManager::NavigationOptions options("CreatesNodeAfterECInstanceInsertWhenPreviouslyNotCreatedDueToHideIfNoChildrenFlag", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 0 nodes
    ASSERT_EQ(0, rootNodes.GetSize());

    // insert a gadget
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *gadget);

    // expect 1 node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, SameLabelInstanceGroupIsCreatedWhenAdditionalInstancesAreInserted)
    {
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SameLabelInstanceGroupIsCreatedWhenAdditionalInstancesAreInserted", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup());
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("SameLabelInstanceGroupIsCreatedWhenAdditionalInstancesAreInserted", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 1 widget
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());

    // insert another widget
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *widget2);

    // still expect 1 node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ("Widget", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_STREQ("Widget", m_updateRecordsHandler->GetRecords()[1].GetNode()->GetLabel().c_str());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, BaseClassGroupIsUpdatedWhenAdditionalInstancesAreInserted)
    {
    ECClassCP classE = m_schema->GetClassCP("ClassE");
    ECClassCP classF = m_schema->GetClassCP("ClassF");

    // insert a base class instance
    RulesEngineTestHelpers::InsertInstance(*s_project, *classE);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("BaseClassGroupIsUpdatedWhenAdditionalInstancesAreInserted", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassE", true));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassE", "", "", "");
    groupingRule->AddGroup(*new ClassGroup("", true, "RulesEngineTest", "ClassE"));
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("BaseClassGroupIsUpdatedWhenAdditionalInstancesAreInserted", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 1 instance
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassE", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // expect it to have 1 child
    ASSERT_EQ(1, IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson()).GetSize());
        
    // insert a derived instance
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(*s_project, *classF);
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *instanceF);

    // still expect 1 ECClassGrouping node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassE", rootNodes[0]->GetLabel().c_str());
    
    // expect it to have 2 children now
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(2, childNodes.GetSize());
    
    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(childNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(childNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[2].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, ValuePropertyGroupIsUpdatedWhenAdditionalInstancesAreInserted)
    {
    // insert a widget
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(8));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("ValuePropertyGroupIsUpdatedWhenAdditionalInstancesAreInserted", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "IntProperty"));
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("ValuePropertyGroupIsUpdatedWhenAdditionalInstancesAreInserted", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 1 property grouping node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("8", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // expect it to have 1 child
    ASSERT_EQ(1, IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson()).GetSize());
        
    // insert another widget
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(8));});
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *widget2);

    // still expect 1 ECProperty grouping node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("8", rootNodes[0]->GetLabel().c_str());
    
    // expect it to have 2 children now
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(2, childNodes.GetSize());
    
    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(childNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(childNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[2].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RangePropertyGroupIsUpdatedWhenAdditionalInstancesAreInserted)
    {
    // insert a widget
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(8));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RangePropertyGroupIsUpdatedWhenAdditionalInstancesAreInserted", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    rules->AddPresentationRule(*groupingRule);

    PropertyGroupP groupingSpec = new PropertyGroup("", "", true, "IntProperty");
    groupingSpec->AddRange(*new PropertyRangeGroupSpecification("5 to 10", "", "5", "10"));
    groupingRule->AddGroup(*groupingSpec);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("RangePropertyGroupIsUpdatedWhenAdditionalInstancesAreInserted", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 1 property grouping node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("5 to 10", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // expect it to have 1 child
    ASSERT_EQ(1, IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson()).GetSize());

    // insert another widget
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(10));});
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *widget2);

    // still expect 1 ECProperty grouping node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("5 to 10", rootNodes[0]->GetLabel().c_str());
    
    // expect it to have 2 children now
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(2, childNodes.GetSize());
    
    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(childNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(childNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[2].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, PropertyGroupIsCreatedWhenInstanceValuesChange)
    {
    // insert some widgets
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(8));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(8));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("ValuePropertyGroupIsCreatedWhenInstanceValuesChange", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "IntProperty"));
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("ValuePropertyGroupIsCreatedWhenInstanceValuesChange", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 1 property grouping node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("8", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // expect it to have 2 children
    ASSERT_EQ(2, IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson()).GetSize());

    // change one of the widgets
    widget2->SetValue("IntProperty", ECValue(9));
    ECInstanceUpdater updater(s_project->GetECDbCR(), *widget2, nullptr);
    updater.Update(*widget2);
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *widget2);

    // expect 2 ECProperty grouping nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("8", rootNodes[0]->GetLabel().c_str());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[1]->GetType().c_str());
    ASSERT_STREQ("9", rootNodes[1]->GetLabel().c_str());
    
    // expect each of them to have 1 child
    DataContainer<NavNodeCPtr> childNodes1 = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childNodes1.GetSize());
    DataContainer<NavNodeCPtr> childNodes2 = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[1], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childNodes2.GetSize());
    
    // expect 4 update records
    ASSERT_EQ(4, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(rootNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetPosition());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(childNodes1[0]->GetKey(), m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[3].GetChangeType());
    EXPECT_EQ(childNodes2[0]->GetKey(), m_updateRecordsHandler->GetRecords()[3].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, PropertyGroupIsCreatedWhenInstanceValuesChangeWithCreateGroupForSingleItemFalse)
    {
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(8));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(9));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("PropertyGroupIsCreatedWhenInstanceValuesChangeWithCreateGroupForSingleItemFalse", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", false, "IntProperty"));
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("PropertyGroupIsCreatedWhenInstanceValuesChangeWithCreateGroupForSingleItemFalse", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect 2 ECInstance nodes
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[1]->GetType().c_str());

    // change one of the widgets
    widget2->SetValue("IntProperty", ECValue(8));
    ECInstanceUpdater updater(s_project->GetECDbCR(), *widget2, nullptr);
    updater.Update(*widget2);
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *widget2);

    // expect 1 ECProperty grouping node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("8", rootNodes[0]->GetLabel().c_str());
    
    // expect it to have 2 children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesCustomNodeWithHideIfNoChildrenFlagWhenNoChanges", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    CustomNodeSpecificationP spec = new CustomNodeSpecification(1, true, "MyType", "MyLabel", "", "MyImageId");
    spec->SetAlwaysReturnsChildren(false);
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("UpdatesCustomNodeWithHideIfNoChildrenFlagWhenNoChanges", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // insert a widget to force custom spec update
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *widget);

    // expect 1 root node now (still no custom node)
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());
    
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesCustomNodeWithHideIfNoChildrenFlagWhenChildrenAdded", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    CustomNodeSpecificationP spec = new CustomNodeSpecification(1, true, "MyType", "MyLabel", "", "MyImageId");
    spec->SetAlwaysReturnsChildren(false);
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type = \"MyType\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("UpdatesCustomNodeWithHideIfNoChildrenFlagWhenChildrenAdded", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // insert a widget to force custom spec update
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *widget);

    // expect 2 root nodes now (including the custom node)
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("MyType", rootNodes[1]->GetType().c_str());
    EXPECT_STREQ("MyLabel", rootNodes[1]->GetLabel().c_str());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(rootNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesCustomNodeWithHideIfNoChildrenFlagWhenChildrenRemoved)
    {
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesCustomNodeWithHideIfNoChildrenFlagWhenChildrenRemoved", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    CustomNodeSpecificationP spec = new CustomNodeSpecification(1, true, "MyType", "MyLabel", "", "MyImageId");
    spec->SetAlwaysReturnsChildren(false);
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type = \"MyType\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("UpdatesCustomNodeWithHideIfNoChildrenFlagWhenChildrenRemoved", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(2, rootNodes.GetSize());
    bvector<NavNodeCPtr> deletedNodes = {rootNodes[0], rootNodes[1]};
    
    // insert a widget to force custom spec update
    RulesEngineTestHelpers::DeleteInstance(*s_project, *widget);
    s_eventsSource->NotifyECInstanceDeleted(s_project->GetECDbCR(), *widget);

    // expect 0 root nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(deletedNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(deletedNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, HidesDisplayLabelGroupingNodeWhenSiblingIsRemovedFromCustomNode)
    {
    // insert 2 widgets and a gadget
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("HidesDisplayLabelGroupingNodeWhenSiblingIsRemovedFromCustomNode", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new CustomNodeSpecification(1, false, "MyCustomType", "MyLabel", "", "MyImageId"));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type = \"MyCustomType\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*childRule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("HidesDisplayLabelGroupingNodeWhenSiblingIsRemovedFromCustomNode", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("MyCustomType", rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("MyLabel", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(2, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("Gadget", childNodes[0]->GetLabel().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, childNodes[1]->GetType().c_str());
    EXPECT_STREQ("Widget", childNodes[1]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*childNodes[1]);

    DataContainer<NavNodeCPtr> widgetChildren = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *childNodes[1], PageOptions(), options.GetJson());
    ASSERT_EQ(2, widgetChildren.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, widgetChildren[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, widgetChildren[1]->GetType().c_str());
    
    // delete the widget
    RulesEngineTestHelpers::DeleteInstance(*s_project, *gadget);
    s_eventsSource->NotifyECInstanceDeleted(s_project->GetECDbCR(), *gadget);

    // expect 2 children now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(2, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("Widget", childNodes[0]->GetLabel().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[1]->GetType().c_str());
    EXPECT_STREQ("Widget", childNodes[1]->GetLabel().c_str());
    
    // expect 4 update records
    ASSERT_EQ(4, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ("Gadget", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, m_updateRecordsHandler->GetRecords()[1].GetNode()->GetType().c_str());
    EXPECT_STREQ("Widget", m_updateRecordsHandler->GetRecords()[1].GetNode()->GetLabel().c_str());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[2].GetNode()->GetType().c_str());
    EXPECT_STREQ("Widget", m_updateRecordsHandler->GetRecords()[2].GetNode()->GetLabel().c_str());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[2].GetPosition());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[3].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[3].GetNode()->GetType().c_str());
    EXPECT_STREQ("Widget", m_updateRecordsHandler->GetRecords()[3].GetNode()->GetLabel().c_str());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[3].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, HidesDisplayLabelGroupingNodeWhenSiblingIsRemovedFromGroupingNode)
    {
    // insert 2 widgets and a gadget
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr differentWidget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("ZZZ"));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("HidesDisplayLabelGroupingNodeWhenSiblingIsRemovedFromGroupingNode", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    GroupingRule* groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "IntProperty", "Default Label"));
    rules->AddPresentationRule(*groupingRule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("HidesDisplayLabelGroupingNodeWhenSiblingIsRemovedFromGroupingNode", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    
    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(2, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("Widget", childNodes[0]->GetLabel().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[1]->GetType().c_str());
    EXPECT_STREQ("ZZZ", childNodes[1]->GetLabel().c_str());
    bvector<NavNodeCPtr> deletedNodes = {childNodes[0], childNodes[1]};

    // expand node
    SetNodeExpanded(*childNodes[0]);

    DataContainer<NavNodeCPtr> widgetChildren = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *childNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(2, widgetChildren.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, widgetChildren[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, widgetChildren[1]->GetType().c_str());
    
    // delete the widget
    RulesEngineTestHelpers::DeleteInstance(*s_project, *differentWidget);
    s_eventsSource->NotifyECInstanceDeleted(s_project->GetECDbCR(), *differentWidget);

    // expect 2 children now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(2, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("Widget", childNodes[0]->GetLabel().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[1]->GetType().c_str());
    EXPECT_STREQ("Widget", childNodes[1]->GetLabel().c_str());
    
    // expect 5 update records
    ASSERT_EQ(5, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(deletedNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(deletedNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[3].GetChangeType());
    EXPECT_EQ(childNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[3].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[3].GetPosition());
    
    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[4].GetChangeType());
    EXPECT_EQ(childNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[4].GetNode()->GetKey());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[4].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RelatedInstanceNodesAreUpdatedAfterOneToManyForwardRelationshipInsert)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RelatedInstanceNodesAreUpdatedAfterOneToManyForwardRelationshipInsert", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childrenRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, 
        "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childrenRule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("RelatedInstanceNodesAreUpdatedAfterOneToManyForwardRelationshipInsert", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(0, childNodes.GetSize());

    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClass, *widget, *gadget);
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *gadget); // gadget is notified because it has a WidgetId property

    // expect 1 child node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("Gadget", childNodes[0]->GetLabel().c_str());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(childNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RelatedInstanceNodesAreUpdatedAfterOneToManyBackwardRelationshipInsert)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RelatedInstanceNodesAreUpdatedAfterOneToManyBackwardRelationshipInsert", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childrenRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, 
        "", RequiredRelationDirection_Backward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*childrenRule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("RelatedInstanceNodesAreUpdatedAfterOneToManyBackwardRelationshipInsert", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Gadget", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(0, childNodes.GetSize());

    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClass, *widget, *gadget);
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *gadget); // gadget is notified because it has a WidgetId property

    // expect 1 child node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("Widget", childNodes[0]->GetLabel().c_str());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(childNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RelatedInstanceNodesAreUpdatedAfterOneToManyForwardRelationshipDelete)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget, relate them
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClass, *widget, *gadget);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RelatedInstanceNodesAreUpdatedAfterOneToManyForwardRelationshipDelete", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childrenRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, 
        "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childrenRule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("RelatedInstanceNodesAreUpdatedAfterOneToManyForwardRelationshipDelete", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("Gadget", childNodes[0]->GetLabel().c_str());
    NavNodeCPtr deletedNode = childNodes[0];

    // unrelate the instances
    RulesEngineTestHelpers::DeleteInstance(*s_project, relationshipKey);
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *gadget); // gadget is notified because it has a WidgetId property

    // expect 0 children now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(0, childNodes.GetSize());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(deletedNode->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RelatedInstanceNodesAreUpdatedAfterOneToManyBackwardRelationshipDelete)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget, relate them
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClass, *widget, *gadget);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RelatedInstanceNodesAreUpdatedAfterOneToManyBackwardRelationshipDelete", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childrenRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, 
        "", RequiredRelationDirection_Backward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*childrenRule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("RelatedInstanceNodesAreUpdatedAfterOneToManyBackwardRelationshipDelete", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Gadget", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("Widget", childNodes[0]->GetLabel().c_str());
    NavNodeCPtr deletedNode = childNodes[0];

    // unrelate the instances
    RulesEngineTestHelpers::DeleteInstance(*s_project, relationshipKey);
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *gadget); // gadget is notified because it has a WidgetId property

    // expect 0 children now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(0, childNodes.GetSize());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(deletedNode->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RelatedInstanceNodesAreUpdatedAfterManyToManyForwardRelationshipInsert)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RelatedInstanceNodesAreUpdatedAfterManyToManyForwardRelationshipInsert", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childrenRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, 
        "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetsHaveGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childrenRule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("RelatedInstanceNodesAreUpdatedAfterManyToManyForwardRelationshipInsert", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(0, childNodes.GetSize());

    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClass, *widget, *gadget);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    s_eventsSource->NotifyECInstancesChanged(s_project->GetECDbCR(), instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 1 child node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("Gadget", childNodes[0]->GetLabel().c_str());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(childNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RelatedInstanceNodesAreUpdatedAfterManyToManyBackwardRelationshipInsert)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RelatedInstanceNodesAreUpdatedAfterManyToManyBackwardRelationshipInsert", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childrenRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, 
        "", RequiredRelationDirection_Backward, "RulesEngineTest", "RulesEngineTest:WidgetsHaveGadgets", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*childrenRule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("RelatedInstanceNodesAreUpdatedAfterManyToManyBackwardRelationshipInsert", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Gadget", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(0, childNodes.GetSize());

    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClass, *widget, *gadget);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    s_eventsSource->NotifyECInstancesChanged(s_project->GetECDbCR(), instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 1 child node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("Widget", childNodes[0]->GetLabel().c_str());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(childNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RelatedInstanceNodesAreUpdatedAfterManyToManyForwardRelationshipDelete)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget, relate them
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClass, *widget, *gadget);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RelatedInstanceNodesAreUpdatedAfterManyToManyForwardRelationshipDelete", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childrenRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, 
        "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetsHaveGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childrenRule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("RelatedInstanceNodesAreUpdatedAfterManyToManyForwardRelationshipDelete", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("Gadget", childNodes[0]->GetLabel().c_str());
    NavNodeCPtr deletedNode = childNodes[0];

    // unrelate the instances
    RulesEngineTestHelpers::DeleteInstance(*s_project, relationshipKey);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    s_eventsSource->NotifyECInstancesChanged(s_project->GetECDbCR(), instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 0 children now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(0, childNodes.GetSize());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(deletedNode->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RelatedInstanceNodesAreUpdatedAfterManyToManyBackwardRelationshipDelete)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget, relate them
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClass, *widget, *gadget);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RelatedInstanceNodesAreUpdatedAfterManyToManyBackwardRelationshipDelete", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childrenRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, 
        "", RequiredRelationDirection_Backward, "RulesEngineTest", "RulesEngineTest:WidgetsHaveGadgets", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*childrenRule);
    
    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("RelatedInstanceNodesAreUpdatedAfterManyToManyBackwardRelationshipDelete", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Gadget", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("Widget", childNodes[0]->GetLabel().c_str());
    NavNodeCPtr deletedNode = childNodes[0];

    // unrelate the instances
    RulesEngineTestHelpers::DeleteInstance(*s_project, relationshipKey);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    s_eventsSource->NotifyECInstancesChanged(s_project->GetECDbCR(), instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 0 children now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(0, childNodes.GetSize());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(deletedNode->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterManyToManyRelationshipInsertWhenBackwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceNodesOfSpecificClassesAreUpdatedAfterRelationshipInsertWhenBackwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.HasRelatedInstance(\"RulesEngineTest:WidgetsHaveGadgets\", \"Backward\", \"RulesEngineTest:Widget\")", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("InstanceNodesOfSpecificClassesAreUpdatedAfterRelationshipInsertWhenBackwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClass, *widget, *gadget);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    s_eventsSource->NotifyECInstancesChanged(s_project->GetECDbCR(), instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 1 root node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("Gadget", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_STREQ("Gadget", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterManyToManyRelationshipInsertWhenForwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceNodesOfSpecificClassesAreUpdatedAfterRelationshipInsertWhenForwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.HasRelatedInstance(\"RulesEngineTest:WidgetsHaveGadgets\", \"Forward\", \"RulesEngineTest:Gadget\")", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("InstanceNodesOfSpecificClassesAreUpdatedAfterRelationshipInsertWhenForwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClass, *widget, *gadget);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    s_eventsSource->NotifyECInstancesChanged(s_project->GetECDbCR(), instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 1 root node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_STREQ("Widget", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterManyToManyRelationshipDeleteWhenBackwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClass, *widget, *gadget);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceNodesOfSpecificClassesAreUpdatedAfterRelationshipInsertWhenBackwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.HasRelatedInstance(\"RulesEngineTest:WidgetsHaveGadgets\", \"Backward\", \"RulesEngineTest:Widget\")", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("InstanceNodesOfSpecificClassesAreUpdatedAfterRelationshipInsertWhenBackwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("Gadget", rootNodes[0]->GetLabel().c_str());
    
    // unrelate the instances
    RulesEngineTestHelpers::DeleteInstance(*s_project, relationshipKey);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    s_eventsSource->NotifyECInstancesChanged(s_project->GetECDbCR(), instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 0 root nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_STREQ("Gadget", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterManyToManyRelationshipDeleteWhenForwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClass, *widget, *gadget);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceNodesOfSpecificClassesAreUpdatedAfterRelationshipInsertWhenForwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.HasRelatedInstance(\"RulesEngineTest:WidgetsHaveGadgets\", \"Forward\", \"RulesEngineTest:Gadget\")", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("InstanceNodesOfSpecificClassesAreUpdatedAfterRelationshipInsertWhenForwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());
    
    // relate the instances
    RulesEngineTestHelpers::DeleteInstance(*s_project, relationshipKey);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    s_eventsSource->NotifyECInstancesChanged(s_project->GetECDbCR(), instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 0 root nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_STREQ("Widget", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterOneToManyRelationshipInsertWhenBackwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceNodesOfSpecificClassesAreUpdatedAfterRelationshipInsertWhenBackwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.HasRelatedInstance(\"RulesEngineTest:WidgetHasGadgets\", \"Backward\", \"RulesEngineTest:Widget\")", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("InstanceNodesOfSpecificClassesAreUpdatedAfterRelationshipInsertWhenBackwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClass, *widget, *gadget);
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *gadget); // gadget is notified because it has a WidgetId property

    // expect 1 root node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("Gadget", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_STREQ("Gadget", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterOneToManyRelationshipInsertWhenForwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceNodesOfSpecificClassesAreUpdatedAfterRelationshipInsertWhenForwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.HasRelatedInstance(\"RulesEngineTest:WidgetHasGadgets\", \"Forward\", \"RulesEngineTest:Gadget\")", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("InstanceNodesOfSpecificClassesAreUpdatedAfterRelationshipInsertWhenForwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClass, *widget, *gadget);
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *gadget); // gadget is notified because it has a WidgetId property

    // expect 1 root node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_STREQ("Widget", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterOneToManyRelationshipDeleteWhenBackwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClass, *widget, *gadget);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceNodesOfSpecificClassesAreUpdatedAfterRelationshipInsertWhenBackwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.HasRelatedInstance(\"RulesEngineTest:WidgetHasGadgets\", \"Backward\", \"RulesEngineTest:Widget\")", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("InstanceNodesOfSpecificClassesAreUpdatedAfterRelationshipInsertWhenBackwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("Gadget", rootNodes[0]->GetLabel().c_str());
    
    // unrelate the instances
    RulesEngineTestHelpers::DeleteInstance(*s_project, relationshipKey);
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *gadget); // gadget is notified because it has a WidgetId property

    // expect 0 root nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_STREQ("Gadget", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterOneToManyRelationshipDeleteWhenForwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClass, *widget, *gadget);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceNodesOfSpecificClassesAreUpdatedAfterRelationshipInsertWhenForwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.HasRelatedInstance(\"RulesEngineTest:WidgetHasGadgets\", \"Forward\", \"RulesEngineTest:Gadget\")", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("InstanceNodesOfSpecificClassesAreUpdatedAfterRelationshipInsertWhenForwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());
    
    // relate the instances
    RulesEngineTestHelpers::DeleteInstance(*s_project, relationshipKey);
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *gadget); // gadget is notified because it has a WidgetId property

    // expect 0 root nodes now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, m_updateRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_STREQ("Widget", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterManyToManyRelationshipRelatedInstanceUpdateWhenGetRelatedValueECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    
    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClass, *widget, *gadget);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceNodesOfSpecificClassesAreUpdatedAfterRelatedInstanceUpdateWhenGetRelatedValueECExpressionIsUsedInInstanceFilter", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "this.GetRelatedValue(\"RulesEngineTest:WidgetsHaveGadgets\", \"Forward\", \"RulesEngineTest:Gadget\", \"MyID\") = \"123\"", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options("InstanceNodesOfSpecificClassesAreUpdatedAfterRelatedInstanceUpdateWhenGetRelatedValueECExpressionIsUsedInInstanceFilter", TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(0, rootNodes.GetSize());
        
    gadget->SetValue("MyID", ECValue("123"));
    ECInstanceUpdater updater(s_project->GetECDbCR(), *gadget, nullptr);
    updater.Update(*gadget);
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *gadget);

    // expect 1 root node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
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
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(*s_project, *m_sprocketClass);
    
    // relate gadget to sprocket
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClassGS, *gadget, *sprocket);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdateAfterSkippedOneToManyRelationshipInsert", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

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
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(0, childNodes.GetSize());
    
    // relate widget to gadget
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClassWG, *widget, *gadget);
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *gadget);

    // expect 1 root node with no children
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(widget->GetInstanceId().c_str(), rootNodes[0]->GetKey().AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
    childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childNodes.GetSize());

    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(childNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterSkippedOneToManyRelationshipDelete)
    {
    ECRelationshipClassCP relationshipClassWG = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipClassGS = m_schema->GetClassCP("GadgetHasSprockets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(*s_project, *m_sprocketClass);
    
    // relate the instances
    ECInstanceKey relWGKey = RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClassWG, *widget, *gadget);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClassGS, *gadget, *sprocket);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdateAfterSkippedOneToManyRelationshipDelete", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

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
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childNodes.GetSize());
    NavNodeCPtr deletedNode = childNodes[0];
    
    RulesEngineTestHelpers::DeleteInstance(*s_project, relWGKey);
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *gadget);

    // expect 1 root node with no children
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(widget->GetInstanceId().c_str(), rootNodes[0]->GetKey().AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
    childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(0, childNodes.GetSize());

    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(deletedNode->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterSkippedManyToManyRelationshipInsert)
    {
    ECRelationshipClassCP relationshipClass1 = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipClass2 = m_schema->GetClassCP("WidgetsHaveGadgets2")->GetRelationshipClassCP();

    // insert some instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(2));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClass1, *widget1, *gadget);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdateAfterSkippedManyToManyRelationshipInsert", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

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
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(widget1->GetInstanceId().c_str(), rootNodes[0]->GetKey().AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(0, childNodes.GetSize());
    
    // insert the relationship
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClass2, *widget2, *gadget);
    s_eventsSource->NotifyECInstancesUpdated(s_project->GetECDbCR(), {widget2.get(), gadget.get()});

    // expect the child node to exist now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(widget1->GetInstanceId().c_str(), rootNodes[0]->GetKey().AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
    childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(widget2->GetInstanceId().c_str(), childNodes[0]->GetKey().AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(childNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterRelatedInstanceInsert)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdateAfterRelatedInstanceInsert", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "", "RulesEngineTest:Gadget", false));
    rule->GetSpecifications()[0]->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "widget"));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new LabelOverride("", 1, "\"Gadget_\" & IIF(NOT IsNull(widget.IntProperty), widget.IntProperty, \"No_Widget\")", ""));

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    
    // verify the label override with related instance property got applied
    EXPECT_STREQ("Gadget_No_Widget", rootNodes[0]->GetLabel().c_str());
    
    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClass, *widget, *gadget);
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *gadget);

    // still expect 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());

    // verify the label has changed
    EXPECT_STREQ("Gadget_1", rootNodes[0]->GetLabel().c_str());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterRelatedInstanceDelete)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    
    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClass, *widget, *gadget);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdateAfterRelatedInstanceDelete", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "", "RulesEngineTest:Gadget", false));
    rule->GetSpecifications()[0]->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "widget"));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new LabelOverride("", 1, "\"Gadget_\" & IIF(NOT IsNull(widget.IntProperty), widget.IntProperty, \"No_Widget\")", ""));

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    
    // verify the label override with related instance property got applied
    EXPECT_STREQ("Gadget_1", rootNodes[0]->GetLabel().c_str());

    RulesEngineTestHelpers::DeleteInstance(*s_project, *widget);
    s_eventsSource->NotifyECInstanceDeleted(s_project->GetECDbCR(), *widget);
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *gadget);

    // still expect 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());

    // verify the label has changed
    EXPECT_STREQ("Gadget_No_Widget", rootNodes[0]->GetLabel().c_str());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterRelatedInstanceUpdate)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    
    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relationshipClass, *widget, *gadget);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdateAfterRelatedInstanceDelete", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, 
        "", "RulesEngineTest:Gadget", false));
    rule->GetSpecifications()[0]->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "widget"));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new LabelOverride("", 1, "\"Gadget_\" & IIF(NOT IsNull(widget.IntProperty), widget.IntProperty, \"No_Widget\")", ""));

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    
    // verify the label override with related instance property got applied
    EXPECT_STREQ("Gadget_1", rootNodes[0]->GetLabel().c_str());

    widget->SetValue("IntProperty", ECValue(2));
    ECInstanceUpdater(s_project->GetECDbCR(), *widget, nullptr).Update(*widget);
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *widget);

    // still expect 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());

    // verify the label has changed
    EXPECT_STREQ("Gadget_2", rootNodes[0]->GetLabel().c_str());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesAllAffectedRootHierarchies)
    {
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("123"));});
    
    // create 3 rulesets
    PresentationRuleSetPtr rules1 = PresentationRuleSet::CreateInstance("UpdatesAllAffectedRootHierarchies_1", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules1);
    RootNodeRule* rule1 = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules1->AddPresentationRule(*rule1);
    rules1->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));
    
    PresentationRuleSetPtr rules2 = PresentationRuleSet::CreateInstance("UpdatesAllAffectedRootHierarchies_2", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules2);
    RootNodeRule* rule2 = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules2->AddPresentationRule(*rule2);
    rules2->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));
    
    PresentationRuleSetPtr rules3 = PresentationRuleSet::CreateInstance("UpdatesAllAffectedRootHierarchies_3", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules3);
    RootNodeRule* rule3 = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule3->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rules3->AddPresentationRule(*rule3);
    rules3->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes1 = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), RulesDrivenECPresentationManager::NavigationOptions("UpdatesAllAffectedRootHierarchies_1", TargetTree_Both).GetJson());
    DataContainer<NavNodeCPtr> rootNodes2 = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), RulesDrivenECPresentationManager::NavigationOptions("UpdatesAllAffectedRootHierarchies_2", TargetTree_Both).GetJson());
    DataContainer<NavNodeCPtr> rootNodes3 = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), RulesDrivenECPresentationManager::NavigationOptions("UpdatesAllAffectedRootHierarchies_3", TargetTree_Both).GetJson());
        
    // verify expected results
    ASSERT_EQ(1, rootNodes1.GetSize());
    EXPECT_STREQ("123", rootNodes1[0]->GetLabel().c_str());
    ASSERT_EQ(1, rootNodes2.GetSize());
    EXPECT_STREQ("123", rootNodes2[0]->GetLabel().c_str());
    ASSERT_EQ(0, rootNodes3.GetSize());

    // update the widget
    widget->SetValue("MyID", ECValue("456"));
    ECInstanceUpdater updater(s_project->GetECDbCR(), *widget, nullptr);
    updater.Update(*widget);
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *widget);

    // verify expected results
    rootNodes1 = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), RulesDrivenECPresentationManager::NavigationOptions("UpdatesAllAffectedRootHierarchies_1", TargetTree_Both).GetJson());
    rootNodes2 = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), RulesDrivenECPresentationManager::NavigationOptions("UpdatesAllAffectedRootHierarchies_2", TargetTree_Both).GetJson());
    rootNodes3 = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), RulesDrivenECPresentationManager::NavigationOptions("UpdatesAllAffectedRootHierarchies_3", TargetTree_Both).GetJson());
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
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("123"));});
    
    // create 3 rulesets
    PresentationRuleSetPtr rules1 = PresentationRuleSet::CreateInstance("UpdatesAllAffectedChildHierarchies_1", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules1);
    rules1->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));
    RootNodeRule* rootRule1 = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule1->AddSpecification(*new CustomNodeSpecification(1, false, "TEST_Type", "Root", "descr", "imageid"));
    rules1->AddPresentationRule(*rootRule1);
    ChildNodeRule* childRule1 = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childRule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules1->AddPresentationRule(*childRule1);
    
    PresentationRuleSetPtr rules2 = PresentationRuleSet::CreateInstance("UpdatesAllAffectedChildHierarchies_2", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules2);
    rules2->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));
    RootNodeRule* rootRule2 = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule2->AddSpecification(*new CustomNodeSpecification(1, false, "TEST_Type", "Root", "descr", "imageid"));
    rules2->AddPresentationRule(*rootRule2);
    ChildNodeRule* childRule2 = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childRule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules2->AddPresentationRule(*childRule2);
    
    PresentationRuleSetPtr rules3 = PresentationRuleSet::CreateInstance("UpdatesAllAffectedChildHierarchies_3", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules3);
    rules3->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));
    RootNodeRule* rootRule3 = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule3->AddSpecification(*new CustomNodeSpecification(1, false, "TEST_Type", "Root", "descr", "imageid"));
    rules3->AddPresentationRule(*rootRule3);
    ChildNodeRule* childRule3 = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childRule3->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rules3->AddPresentationRule(*childRule3);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes1 = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), RulesDrivenECPresentationManager::NavigationOptions("UpdatesAllAffectedChildHierarchies_1", TargetTree_Both).GetJson());
    DataContainer<NavNodeCPtr> rootNodes2 = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), RulesDrivenECPresentationManager::NavigationOptions("UpdatesAllAffectedChildHierarchies_2", TargetTree_Both).GetJson());
    DataContainer<NavNodeCPtr> rootNodes3 = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), RulesDrivenECPresentationManager::NavigationOptions("UpdatesAllAffectedChildHierarchies_3", TargetTree_Both).GetJson());
        
    // verify expected results
    ASSERT_EQ(1, rootNodes1.GetSize());
    EXPECT_STREQ("Root", rootNodes1[0]->GetLabel().c_str());
    // expand node
    SetNodeExpanded(*rootNodes1[0]);
    DataContainer<NavNodeCPtr> childNodes1 = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes1[0], PageOptions(), RulesDrivenECPresentationManager::NavigationOptions("UpdatesAllAffectedChildHierarchies_1", TargetTree_Both).GetJson());
    ASSERT_EQ(1, rootNodes1.GetSize());
    EXPECT_STREQ("123", childNodes1[0]->GetLabel().c_str());

    ASSERT_EQ(1, rootNodes2.GetSize());
    EXPECT_STREQ("Root", rootNodes2[0]->GetLabel().c_str());
    // expand node
    SetNodeExpanded(*rootNodes2[0]);
    DataContainer<NavNodeCPtr> childNodes2 = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes2[0], PageOptions(), RulesDrivenECPresentationManager::NavigationOptions("UpdatesAllAffectedChildHierarchies_2", TargetTree_Both).GetJson());
    ASSERT_EQ(1, childNodes2.GetSize());
    EXPECT_STREQ("123", childNodes2[0]->GetLabel().c_str());

    ASSERT_EQ(1, rootNodes3.GetSize());
    EXPECT_STREQ("Root", rootNodes3[0]->GetLabel().c_str());
    // expand node
    SetNodeExpanded(*rootNodes3[0]);
    DataContainer<NavNodeCPtr> childNodes3 = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes3[0], PageOptions(), RulesDrivenECPresentationManager::NavigationOptions("UpdatesAllAffectedChildHierarchies_3", TargetTree_Both).GetJson());
    ASSERT_EQ(0, childNodes3.GetSize());

    // update the widget
    widget->SetValue("MyID", ECValue("456"));
    ECInstanceUpdater updater(s_project->GetECDbCR(), *widget, nullptr);
    updater.Update(*widget);
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *widget);

    // verify expected results
    rootNodes1 = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), RulesDrivenECPresentationManager::NavigationOptions("UpdatesAllAffectedChildHierarchies_1", TargetTree_Both).GetJson());
    childNodes1 = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes1[0], PageOptions(), RulesDrivenECPresentationManager::NavigationOptions("UpdatesAllAffectedChildHierarchies_1", TargetTree_Both).GetJson());
    rootNodes2 = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), RulesDrivenECPresentationManager::NavigationOptions("UpdatesAllAffectedChildHierarchies_2", TargetTree_Both).GetJson());
    childNodes2 = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes2[0], PageOptions(), RulesDrivenECPresentationManager::NavigationOptions("UpdatesAllAffectedChildHierarchies_2", TargetTree_Both).GetJson());
    rootNodes3 = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), RulesDrivenECPresentationManager::NavigationOptions("UpdatesAllAffectedChildHierarchies_3", TargetTree_Both).GetJson());
    childNodes3 = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes3[0], PageOptions(), RulesDrivenECPresentationManager::NavigationOptions("UpdatesAllAffectedChildHierarchies_3", TargetTree_Both).GetJson());
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesParentsHasChildrenFlagWhenChildNodeIsInserted_WhenParentIsRoot", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, TargetTree_Both, false);
    CustomNodeSpecification* rootSpec = new CustomNodeSpecification(1, false, "custom", "custom", "custom", "custom");
    rootSpec->SetAlwaysReturnsChildren(false);
    rootRule->AddSpecification(*rootSpec);
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type=\"custom\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_FALSE(rootNodes[0]->HasChildren());
    // expand node
    SetNodeExpanded(*rootNodes[0]);
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(0, childNodes.GetSize());
        
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *widget);

    // expect the root node to have a "has children" flag set to "true"
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_TRUE(rootNodes[0]->HasChildren());
    childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childNodes.GetSize());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(childNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* TFS#759626
* @betest                                       Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesParentsHasChildrenFlagWhenChildNodeIsInserted_WhenParentIsNotRoot)
    {
    // set up
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass, [](IECInstanceR g){g.SetValue("Description", ECValue("1"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass, [](IECInstanceR g){g.SetValue("Description", ECValue("2"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesParentsHasChildrenFlagWhenChildNodeIsInserted_WhenParentIsNotRoot", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

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
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_TRUE(rootNodes[0]->HasChildren());
    SetNodeExpanded(*rootNodes[0]);
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_FALSE(childNodes[0]->HasChildren());
    SetNodeExpanded(*childNodes[0]);
    DataContainer<NavNodeCPtr> grandchildNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *childNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(0, grandchildNodes.GetSize());
        
    // insert another gadget
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass, [](IECInstanceR g){g.SetValue("Description", ECValue("3"));});
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *gadget);

    // expect the middle child node to have a "has children" flag set to "true"
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_TRUE(rootNodes[0]->HasChildren());
    childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_TRUE(childNodes[0]->HasChildren());
    grandchildNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *childNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, grandchildNodes.GetSize());
    
    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(childNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(grandchildNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[2].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesParentsHasChildrenFlagWhenChildNodeIsDeleted)
    {
    // set up
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesParentsHasChildrenFlagWhenChildNodeIsDeleted", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, TargetTree_Both, false);
    CustomNodeSpecification* rootSpec = new CustomNodeSpecification(1, false, "custom", "custom", "custom", "custom");
    rootSpec->SetAlwaysReturnsChildren(false);
    rootRule->AddSpecification(*rootSpec);
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type=\"custom\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_TRUE(rootNodes[0]->HasChildren());
    // expand node
    SetNodeExpanded(*rootNodes[0]);
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childNodes.GetSize());
    NavNodeCPtr deletedNode = childNodes[0];
        
    // delete the widget
    RulesEngineTestHelpers::DeleteInstance(*s_project, *widget);
    s_eventsSource->NotifyECInstanceDeleted(s_project->GetECDbCR(), *widget);

    // expect the root node to have a "has children" flag set to "false"
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_FALSE(rootNodes[0]->HasChildren());
    childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(0, childNodes.GetSize());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(deletedNode->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, ShowsParentNodeWithHideIfNoChildrenFlagWhenChildNodeIsInserted)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("ShowsParentNodeWithHideIfNoChildrenFlagWhenChildNodeIsInserted", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, TargetTree_Both, false);
    CustomNodeSpecification* rootSpec = new CustomNodeSpecification(1, true, "custom", "custom", "custom", "custom");
    rootSpec->SetAlwaysReturnsChildren(false);
    rootRule->AddSpecification(*rootSpec);
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type=\"custom\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(0, rootNodes.GetSize());
        
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *widget);

    // expect the root node to get inserted
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_TRUE(rootNodes[0]->HasChildren());
    
    // expect 1 update records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RemovesParentNodeWithHideIfNoChildrenFlagWhenTheLastChildNodeIsDeleted)
    {
    // set up
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RemovesParentNodeWithHideIfNoChildrenFlagWhenTheLastChildNodeIsDeleted", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, TargetTree_Both, false);
    CustomNodeSpecification* rootSpec = new CustomNodeSpecification(1, true, "custom", "custom", "custom", "custom");
    rootSpec->SetAlwaysReturnsChildren(false);
    rootRule->AddSpecification(*rootSpec);
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type=\"custom\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_TRUE(rootNodes[0]->HasChildren());
    NavNodeCPtr deletedRootNode = rootNodes[0];

    // expand node
    SetNodeExpanded(*rootNodes[0]);
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childNodes.GetSize());
    NavNodeCPtr deletedChildNode = childNodes[0];
        
    // delete the widget
    RulesEngineTestHelpers::DeleteInstance(*s_project, *widget);
    s_eventsSource->NotifyECInstanceDeleted(s_project->GetECDbCR(), *widget);

    // expect the root node to be gone
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(deletedChildNode->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(deletedRootNode->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, DoesNotUpdateChildHierarchyIfParentIsRemoved)
    {
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    
    // create the ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("DoesNotUpdateChildHierarchyIfParentIsRemoved", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

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
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
        
    // verify expected results
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());
    NavNodeCPtr deletedRootNode = rootNodes[0];

    // expand node
    SetNodeExpanded(*rootNodes[0]);
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ("Custom label", childNodes[0]->GetLabel().c_str());
    
    // expand node
    SetNodeExpanded(*childNodes[0]);
    DataContainer<NavNodeCPtr> grandchildNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *childNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, grandchildNodes.GetSize());
    EXPECT_STREQ("Widget", grandchildNodes[0]->GetLabel().c_str());

    // delete the widget
    ECInstanceDeleter deleter(s_project->GetECDbCR(), *m_widgetClass, nullptr);
    deleter.Delete(*widget);
    s_eventsSource->NotifyECInstanceDeleted(s_project->GetECDbCR(), *widget);

    // verify expected results
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(deletedRootNode->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, CustomizesInsertedNodes)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesCustomNodeWithHideIfNoChildrenFlagWhenNothingChanges", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // insert a widget 
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *widget);

    // expect 1 root node now 
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(widget->GetInstanceId().c_str(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey().AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
    EXPECT_TRUE(NavNodeExtendedData(*m_updateRecordsHandler->GetRecords()[0].GetNode()).IsCustomized());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesAffectedBranchesWhenUserSettingChanges_UsedInRuleCondition)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesAffectedBranchesWhenUserSettingChanges_UsedInRuleCondition", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("1 = GetSettingIntValue(\"test\")", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // change a setting
    s_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", 1);

    // expect 1 root node now 
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(widget->GetInstanceId().c_str(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey().AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
    EXPECT_TRUE(NavNodeExtendedData(*m_updateRecordsHandler->GetRecords()[0].GetNode()).IsCustomized());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesAffectedBranchesWhenUserSettingChanges_UsedInInstanceFilter)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesAffectedBranchesWhenUserSettingChanges_UsedInInstanceFilter", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "1 = GetSettingIntValue(\"test\")", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(0, rootNodes.GetSize());
    
    // change a setting
    s_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", 1);

    // expect 1 root node now 
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(widget->GetInstanceId().c_str(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey().AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
    EXPECT_TRUE(NavNodeExtendedData(*m_updateRecordsHandler->GetRecords()[0].GetNode()).IsCustomized());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesAffectedBranchesWhenUserSettingChanges_UsedInCustomizationRuleCondition)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesAffectedBranchesWhenUserSettingChanges_UsedInCustomizationRuleCondition", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new LabelOverride("1 = GetSettingIntValue(\"test\")", 1, "\"test\"", "\"test\""));

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());

    // change a setting
    s_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", 1);

    // still expect 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());

    // its label should have changed
    EXPECT_STREQ("test", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(widget->GetInstanceId().c_str(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey().AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
    EXPECT_TRUE(NavNodeExtendedData(*m_updateRecordsHandler->GetRecords()[0].GetNode()).IsCustomized());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesAffectedBranchesWhenUserSettingChanges_UsedInCustomizationRuleExpression)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesAffectedBranchesWhenUserSettingChanges_UsedInCustomizationRuleExpression", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new LabelOverride("", 1, "\"test\" & GetSettingIntValue(\"test\")", "\"test\""));

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("test0", rootNodes[0]->GetLabel().c_str());

    // change a setting
    s_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", 1);

    // still expect 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());

    // its label should have changed
    EXPECT_STREQ("test1", rootNodes[0]->GetLabel().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ(widget->GetInstanceId().c_str(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey().AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
    EXPECT_TRUE(NavNodeExtendedData(*m_updateRecordsHandler->GetRecords()[0].GetNode()).IsCustomized());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesGroupingBranchesUnderHiddenLevelsWhenUserSettingChanges)
    {
    ECRelationshipClassCP widgetHasGadgetsClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(2));});
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *widgetHasGadgetsClass, *widget1, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *widgetHasGadgetsClass, *widget2, *gadget2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesGroupingBranchesWhenUserSettingChanges", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, true, false, false, false, false,
        "this.IntProperty = GetSettingIntValue(\"test\")", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, true, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);

    // set initial setting value
    s_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", 1);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Gadget", rootNodes[0]->GetLabel().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_EQ(*ECInstanceNodeKey::Create(*gadget1), childNodes[0]->GetKey());
    NavNodeCPtr deletedNode = childNodes[0];

    // change a setting
    s_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", 2);

    // expect 1 root node now
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Gadget", rootNodes[0]->GetLabel().c_str());
    
    childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_EQ(*ECInstanceNodeKey::Create(*gadget2), childNodes[0]->GetKey());

    // expect 3 update records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());
    
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    
    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(deletedNode->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(childNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[2].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesLocalizedCustomNodesOnUserSettingChange)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesLocalizedCustomNodesOnUserSettingChange", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new CustomNodeSpecification(1, false, "Custom", "Label", "Description", "ImageId"));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.Type=\"Custom\"", 1, 
        Utf8PrintfString("\"%s\"", RULESENGINE_LOCALIZEDSTRING_Other.c_str()), "GetSettingIntValue(\"counter\")"));

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("0", rootNodes[0]->GetDescription().c_str());

    // change a setting
    s_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("counter", 1);

    // still expect 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());

    // its description should have changed
    EXPECT_STREQ("1", rootNodes[0]->GetDescription().c_str());
    
    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ("Custom", m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey().AsDisplayLabelGroupingNodeKey()->GetType().c_str());
    EXPECT_TRUE(NavNodeExtendedData(*m_updateRecordsHandler->GetRecords()[0].GetNode()).IsCustomized());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, DoesNotUpdateHierarchyWhenNodeRemovedFromCollapsedHierarchy)
    {
    // insert some instances
    ECRelationshipClassCP widgetHasGadgetsClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *widgetHasGadgetsClass, *widget, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *widgetHasGadgetsClass, *widget, *gadget2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("DoesNotUpdateHierarchyWhenNodeRemovedFromCollapsedHierarchy", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());

    // make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ("Gadget", childrenNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("Gadget", childrenNodes[0]->GetLabel().c_str());

    // expect root node to be collapsed
    EXPECT_FALSE(rootNodes[0]->IsExpanded());

    // delete one gadget
    ECInstanceDeleter deleter(s_project->GetECDbCR(), *m_gadgetClass, nullptr);
    deleter.Delete(*gadget2);
    s_eventsSource->NotifyECInstanceDeleted(s_project->GetECDbCR(), *gadget2);

    // expect no updates
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords().size());

    // make sure we still have 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());

    // make sure it now has 1 child node
    childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ("Gadget", childrenNodes[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, DoesNotUpdateHierarchyWhenNodeInsertedIntoCollapsedHierarchy)
    {
    // insert some instances
    ECRelationshipClassCP widgetHasGadgetsClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *widgetHasGadgetsClass, *widget, *gadget1);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("DoesNotUpdateHierarchyWhenNodeInsertedIntoCollapsedHierarchy", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", true));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());

    // make sure it has 1 child node
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ("Gadget", childrenNodes[0]->GetLabel().c_str());

    // expect root node to be collapsed
    EXPECT_FALSE(rootNodes[0]->IsExpanded());

    // relate second gadget
    RulesEngineTestHelpers::InsertRelationship(*s_project, *widgetHasGadgetsClass, *widget, *gadget2);
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *gadget2);

    // expect no updates
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords().size());

    // make sure we have 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());

    // make sure it now has 2 children
    childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ("Gadget", childrenNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("Gadget", childrenNodes[1]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, DoesNotUpdateHierarchyWhenNodeUpdatedInCollapsedHierarchy)
    {
    // insert some instances
    ECRelationshipClassCP widgetHasGadgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *widgetHasGadgetClass, *widget, *gadget);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("DoesNotUpdateHierarchyWhenNodeUpdatedInCollapsedHierarchy", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Gadget\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Backward, "", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Gadget", rootNodes[0]->GetLabel().c_str());

    // make sure it has 1 child node
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ("Widget", childrenNodes[0]->GetLabel().c_str());

    // expect root node to be collapsed
    EXPECT_FALSE(rootNodes[0]->IsExpanded());

    // update widget
    widget->SetValue("MyID", ECValue("New label"));
    ECInstanceUpdater updater(s_project->GetECDbCR(), *widget, nullptr);
    updater.Update(*widget);
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *widget);

    // expect no updates
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords().size());

    // make sure we still have 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Gadget", rootNodes[0]->GetLabel().c_str());

    // make sure it still has 1 child node but label is changed
    childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ("New label", childrenNodes[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, DoesNotUpdateHierarchyWhenAnyParentUpTheHierarchyIsCollapsed)
    {
    // insert some instances and create hierarchy
    ECRelationshipClassCP widgetHasGadgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP();
    ECRelationshipClassCP gadgetHasSprocketsClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    IECInstancePtr sprocket1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_sprocketClass);
    IECInstancePtr sprocket2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_sprocketClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *widgetHasGadgetClass, *widget, *gadget);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *gadgetHasSprocketsClass, *gadget, *sprocket1);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("DoesNotUpdateHierarchyWhenAnyParentUpTheHierarchyIsCollapsed", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

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
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());

    // make sure it has 1 gadget node
    DataContainer<NavNodeCPtr> gadgetNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, gadgetNodes.GetSize());
    EXPECT_STREQ("Gadget", gadgetNodes[0]->GetLabel().c_str());

    // expand gadget node
    SetNodeExpanded(*gadgetNodes[0]);

    // make sure it has 1 sprocket node
    DataContainer<NavNodeCPtr> sprocketNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *gadgetNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, sprocketNodes.GetSize());
    EXPECT_STREQ("Sprocket", sprocketNodes[0]->GetLabel().c_str());

    // expect widget to be collapsed and gadget to be expanded
    EXPECT_FALSE(rootNodes[0]->IsExpanded());
    EXPECT_TRUE(gadgetNodes[0]->IsExpanded());

    // relate second sprocket to gadget
    RulesEngineTestHelpers::InsertRelationship(*s_project, *gadgetHasSprocketsClass, *gadget, *sprocket2);
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *sprocket2);

    // expect no updates, because widget was collapsed
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords().size());

    // make sure we still have 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());

    // make sure it still has 1 gadget node
    gadgetNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, gadgetNodes.GetSize());
    EXPECT_STREQ("Gadget", gadgetNodes[0]->GetLabel().c_str());

    // make sure it now has 2 sprocket nodes
    sprocketNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *gadgetNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(2, sprocketNodes.GetSize());
    EXPECT_STREQ("Sprocket", sprocketNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("Sprocket", sprocketNodes[1]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, UpdateHierarchyWhenLastNodeRemovedFromCollapsedHierarchy)
    {
    // insert some instances
    ECRelationshipClassCP widgetHasGadgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *widgetHasGadgetClass, *widget, *gadget);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdateHierarchyWhenLastNodeRemovedFromCollapsedHierarchy", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());
    EXPECT_TRUE(rootNodes[0]->HasChildren());

    // make sure it has 1 child node
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ("Gadget", childrenNodes[0]->GetLabel().c_str());

    // expect root node to be collapsed
    EXPECT_FALSE(rootNodes[0]->IsExpanded());

    // delete gadget
    ECInstanceDeleter deleter(s_project->GetECDb(), *m_gadgetClass, nullptr);
    deleter.Delete(*gadget);
    s_eventsSource->NotifyECInstanceDeleted(s_project->GetECDb(), *gadget);

    // make sure we still have 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());
    EXPECT_FALSE(rootNodes[0]->HasChildren());

    // make sure now it has 0 children
    childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(0, childrenNodes.GetSize());

    // expect 1 update record
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords().size());
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, UpdateHierarchyWhenNodeInsertedIntoEmptyCollapsedHierarchy)
    {
    // insert some instances
    ECRelationshipClassCP widgetHasGadgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdateHierarchyWhenNodeInsertedIntoEmptyCollapsedHierarchy", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());
    EXPECT_FALSE(rootNodes[0]->HasChildren());

    // make sure it has 0 children
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(0, childrenNodes.GetSize());

    // expect root node to be collapsed
    EXPECT_FALSE(rootNodes[0]->IsExpanded());

    // relate gadget
    RulesEngineTestHelpers::InsertRelationship(*s_project, *widgetHasGadgetClass, *widget, *gadget);
    s_eventsSource->NotifyECInstanceDeleted(s_project->GetECDb(), *gadget);

    // make sure we still have 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());
    EXPECT_TRUE(rootNodes[0]->HasChildren());

    // make sure now it has 1 child node
    childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ("Gadget", childrenNodes[0]->GetLabel().c_str());

    // expect 1 update record
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords().size());
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, UpdateHierarchyWhenLastGroupedNodeDeletedFromCollapsedHierarchy)
    {
    // insert some instances
    ECRelationshipClassCP widgetHasGadgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *widgetHasGadgetClass, *widget, *gadget);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdateHierarchyWhenLastGroupedNodeDeletedFromCollapsedHierarchy", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, true, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());
    EXPECT_TRUE(rootNodes[0]->HasChildren());

    // make sure it has 1 child grouping node
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, childrenNodes[0]->GetType().c_str());

    // make sure it has 1 child gadget node
    DataContainer<NavNodeCPtr> gadgetNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *childrenNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, gadgetNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, gadgetNodes[0]->GetType().c_str());
    EXPECT_STREQ("Gadget", gadgetNodes[0]->GetLabel().c_str());

    // expect root node to be collapsed
    EXPECT_FALSE(rootNodes[0]->IsExpanded());

    // delete gadget
    ECInstanceDeleter deleter(s_project->GetECDb(), *m_gadgetClass, nullptr);
    deleter.Delete(*gadget);
    s_eventsSource->NotifyECInstanceDeleted(s_project->GetECDb(), *gadget);

    // make sure we still have 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());
    EXPECT_FALSE(rootNodes[0]->HasChildren());

    // make sure now it has 0 children
    childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(0, childrenNodes.GetSize());

    // expect 1 update record
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords().size());
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, DoesNotCollapseParentNodeAfterChildNodeIsInserted)
    {
    // insert widget
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("DoesNotCollapseParentNodeAfterChildNodeIsInserted", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    // make sure we have 1 root node
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_TRUE(rootNodes[0]->HasChildren());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // make sure it has 1 child node
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ("Widget", childrenNodes[0]->GetLabel().c_str());

    // expect grouping node to be expanded
    EXPECT_TRUE(rootNodes[0]->IsExpanded());

    // insert second widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDb(), *widget);

    // make sure we still have 1 root node
    rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_TRUE(rootNodes[0]->HasChildren());

    // expect it to be expanded
    EXPECT_TRUE(rootNodes[0]->IsExpanded());

    // make sure now it has 2 children nodes
    childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ("Widget", childrenNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("Widget", childrenNodes[1]->GetLabel().c_str());

    // expect 3 recors
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(rootNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());
    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(childrenNodes[0]->GetKey(), m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[2].GetChangeType());
    EXPECT_EQ(childrenNodes[1]->GetKey(), m_updateRecordsHandler->GetRecords()[2].GetNode()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, DoesNotUpdateFixedHierarchyWhenNodeIsInserted)
    {
    // insert widget
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("DoesNotUpdateFixedHierarchyWhenNodeIsInserted", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    // make sure we have 1 root node and disable updates
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both, true);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());

    // insert second widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDb(), *widget);

    // expect no updates
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, DoesNotUpdateFixedHierarchyWhenNodeIsUpdated)
    {
    // insert widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("DoesNotUpdateFixedHierarchyWhenNodeIsUpdated", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    // make sure we have 1 root node and disable updates
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both, true);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());

    // update widget
    widget->SetValue("MyID", ECValue("New label"));
    ECInstanceUpdater updater(s_project->GetECDbCR(), *widget, nullptr);
    updater.Update(*widget);
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *widget);

    // expect no updates
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, DoesNotUpdateFixedHierarchyWhenChildIsInserted)
    {
    // insert widget
    ECRelationshipClassCP widgetHasGadgetsClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("DoesNotUpdateFixedHierarchyWhenChildIsInserted", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node and disable updates
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both, true);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());

    // make sure it has 0 children
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(0, childrenNodes.GetSize());

    // relate gadget to widget
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *widgetHasGadgetsClass, *widget, *gadget);
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDb(), *gadget);

    // expect no updates
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, DoesNotUpdateFixedHierarchyWhenChildIsDeleted)
    {
    // insert some instances
    ECRelationshipClassCP widgetHasGadgetsClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *widgetHasGadgetsClass, *widget, *gadget);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("DoesNotUpdateFixedHierarchyWhenChildIsDeleted", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node and disable updates
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_Both, true);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());

    // expand root node
    SetNodeExpanded(*rootNodes[0]);

    // make sure it has 1 child
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson());
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ("Gadget", childrenNodes[0]->GetLabel().c_str());

    // delete gadget
    RulesEngineTestHelpers::DeleteInstance(*s_project, *gadget);
    s_eventsSource->NotifyECInstanceDeleted(s_project->GetECDb(), *gadget);

    // expect no updates
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords().size());
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2016
+===============+===============+===============+===============+===============+======*/
struct ContentUpdateTests : UpdateTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentUpdateTests, UpdatesContentAfterECInstanceInsert)
    {
    // insert a widget instance
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesContentAfterECInstanceInsert", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->GetSpecificationsR().push_back(new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // request content
    RulesDrivenECPresentationManager::ContentOptions options("UpdatesContentAfterECInstanceInsert");
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, SelectionInfo(), options.GetJson());
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, SelectionInfo(), PageOptions(), options.GetJson());
    
    // expect 1 record
    ASSERT_EQ(1, content->GetContentSet().GetSize());
    ASSERT_TRUE(content->GetContentSet()[0].IsValid());
    
    // insert one more instance
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    s_eventsSource->NotifyECInstanceInserted(s_project->GetECDbCR(), *widget2);

    // expect 2 records
    content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, SelectionInfo(), PageOptions(), options.GetJson());
    ASSERT_EQ(2, content->GetContentSet().GetSize());
    ASSERT_STREQ(widget1->GetInstanceId().c_str(), content->GetContentSet()[0]->GetKeys()[0].GetInstanceId().ToString().c_str());
    ASSERT_STREQ(widget2->GetInstanceId().c_str(), content->GetContentSet()[1]->GetKeys()[0].GetInstanceId().ToString().c_str());

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
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesContentAfterECInstanceUpdate", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->GetSpecificationsR().push_back(new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // request content
    RulesDrivenECPresentationManager::ContentOptions options("UpdatesContentAfterECInstanceUpdate");
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, SelectionInfo(), options.GetJson());
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, SelectionInfo(), PageOptions(), options.GetJson());
    
    // expect 1 record
    ASSERT_EQ(1, content->GetContentSet().GetSize());
    ASSERT_TRUE(content->GetContentSet()[0].IsValid());
    
    // notify about an update (even though we didn't change anything)
    s_eventsSource->NotifyECInstanceUpdated(s_project->GetECDbCR(), *widget);

    // expect 1 record
    content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, SelectionInfo(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, content->GetContentSet().GetSize());
    ASSERT_STREQ(widget->GetInstanceId().c_str(), content->GetContentSet()[0]->GetKeys()[0].GetInstanceId().ToString().c_str());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    ASSERT_STREQ("UpdatesContentAfterECInstanceUpdate", m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    ASSERT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentUpdateTests, UpdatesContentAfterECInstanceDeleteWhenMoreInstancesExist)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesContentAfterECInstanceDeleteWhenMoreInstancesExist", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->GetSpecificationsR().push_back(new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // request content
    RulesDrivenECPresentationManager::ContentOptions options("UpdatesContentAfterECInstanceDeleteWhenMoreInstancesExist");
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, SelectionInfo(), options.GetJson());
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, SelectionInfo(), PageOptions(), options.GetJson());
    
    // expect 2 records
    ASSERT_EQ(2, content->GetContentSet().GetSize());
    ASSERT_TRUE(content->GetContentSet()[0].IsValid());
    ASSERT_TRUE(content->GetContentSet()[1].IsValid());
    
    // delete one of the instances
    RulesEngineTestHelpers::DeleteInstance(*s_project, *widget1);
    s_eventsSource->NotifyECInstanceDeleted(s_project->GetECDbCR(), *widget1);

    // expect 1 record
    content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, SelectionInfo(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, content->GetContentSet().GetSize());
    ASSERT_STREQ(widget2->GetInstanceId().c_str(), content->GetContentSet()[0]->GetKeys()[0].GetInstanceId().ToString().c_str());

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
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesContentAfterECInstanceDeleteWhenNoMoreInstancesExist", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->GetSpecificationsR().push_back(new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // request content
    RulesDrivenECPresentationManager::ContentOptions options("UpdatesContentAfterECInstanceDeleteWhenNoMoreInstancesExist");
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, SelectionInfo(), options.GetJson());
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, SelectionInfo(), PageOptions(), options.GetJson());
    
    // expect 1 record
    ASSERT_EQ(1, content->GetContentSet().GetSize());
    ASSERT_TRUE(content->GetContentSet()[0].IsValid());
    
    // delete the instance
    RulesEngineTestHelpers::DeleteInstance(*s_project, *widget);
    s_eventsSource->NotifyECInstanceDeleted(s_project->GetECDbCR(), *widget);

    // expect 0 records
    content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, SelectionInfo(), PageOptions(), options.GetJson());
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
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // set up empty selection
    SelectionInfo selection("PROVIDER", false, *NavNodeKeyListContainer::Create());
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesAllContentBasedOnOneRulesetButSendsOnlyOneNotification_WithNoContent", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->GetSpecificationsR().push_back(new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);
    
    // request content and expect none
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor1 = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), ContentDisplayType::Graphics, selection, options.GetJson());
    ASSERT_TRUE(descriptor1.IsNull());
    ContentDescriptorCPtr descriptor2 = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), ContentDisplayType::Grid, selection, options.GetJson());
    ASSERT_TRUE(descriptor2.IsNull());

    // delete the instance
    RulesEngineTestHelpers::DeleteInstance(*s_project, *widget);
    s_eventsSource->NotifyECInstanceDeleted(s_project->GetECDbCR(), *widget);

    // still expect no content
    descriptor1 = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), ContentDisplayType::Graphics, selection, options.GetJson());
    ASSERT_TRUE(descriptor1.IsNull());
    descriptor2 = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), ContentDisplayType::Grid, selection, options.GetJson());
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
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // set up selection
    SelectionInfo selection("PROVIDER", false, *NavNodeKeyListContainer::Create({ECInstanceNodeKey::Create(*widget)}));
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesAllContentBasedOnOneRulesetButSendsOnlyOneNotification_WithContent", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->GetSpecificationsR().push_back(new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);
    
    // request content
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    ContentDescriptorCPtr descriptor1 = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), ContentDisplayType::Graphics, selection, options.GetJson());
    ASSERT_TRUE(descriptor1.IsValid());
    ContentCPtr content1 = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor1, selection, PageOptions(), options.GetJson());
    ASSERT_EQ(1, content1->GetContentSet().GetSize());

    ContentDescriptorCPtr descriptor2 = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), ContentDisplayType::Grid, selection, options.GetJson());
    ASSERT_TRUE(descriptor2.IsValid());
    ContentCPtr content2 = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor2, selection, PageOptions(), options.GetJson());
    ASSERT_EQ(1, content2->GetContentSet().GetSize());

    // delete the instance
    RulesEngineTestHelpers::DeleteInstance(*s_project, *widget);
    s_eventsSource->NotifyECInstanceDeleted(s_project->GetECDbCR(), *widget);
    selection = SelectionInfo("PROVIDER", false, *NavNodeKeyListContainer::Create());

    // expect no content in both cases
    descriptor1 = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), ContentDisplayType::Graphics, selection, options.GetJson());
    ASSERT_TRUE(descriptor1.IsNull());
    descriptor2 = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), ContentDisplayType::Grid, selection, options.GetJson());
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
    s_manager->SetCategorySupplier(&supplier);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("UpdatesContentAfterCategoriesChange", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->GetSpecificationsR().push_back(new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // request content
    RulesDrivenECPresentationManager::ContentOptions options("UpdatesContentAfterCategoriesChange");
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, SelectionInfo(), options.GetJson());
    
    // expect the fields to have supplied category
    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    for (ContentDescriptor::Field const* field : fields)
        {
        EXPECT_TRUE(field->GetCategory() == supplier.GetUsedCategory());
        }
    
    // change supplied category
    supplier.SetUsedCategory(ContentDescriptor::Category::GetFavoriteCategory());
    s_manager->NotifyCategoriesChanged();

    // expect the fields to have the new supplied category
    descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, SelectionInfo(), options.GetJson());
    fields = descriptor->GetVisibleFields();
    for (ContentDescriptor::Field const* field : fields)
        {
        EXPECT_TRUE(field->GetCategory() == supplier.GetUsedCategory());
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
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InvalidatesWhenUserSettingChanges_UsedInRuleCondition", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);
    
    ContentRule* rule = new ContentRule("1 = GetSettingIntValue(\"test\")", 1, false);
    rule->GetSpecificationsR().push_back(new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, SelectionInfo(), options.GetJson());
        
    // expect the descriptor to be null because no rules applied
    ASSERT_TRUE(descriptor.IsNull());
        
    // change a setting
    s_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", 1);
    
    // expect 1 content item
    descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, SelectionInfo(), options.GetJson());
    ASSERT_TRUE(descriptor.IsValid());
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, SelectionInfo(), PageOptions(), options.GetJson());
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
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InvalidatesWhenUserSettingChanges_UsedInInstanceFilter", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);
    
    ContentRule* rule = new ContentRule("", 1, false);
    rule->GetSpecificationsR().push_back(new ContentInstancesOfSpecificClassesSpecification(1, "1 = GetSettingIntValue(\"test\")", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, SelectionInfo(), options.GetJson());
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, SelectionInfo(), PageOptions(), options.GetJson());
    
    // expect 0 records
    ASSERT_EQ(0, content->GetContentSet().GetSize());
        
    // change a setting
    s_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", 1);

    // expect 1 content item
    content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, SelectionInfo(), PageOptions(), options.GetJson());
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
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("DoesNotInvalidateWhenUnusedUserSettingChanges", 1, 0, false, "", "", "", false);
    s_locater->AddRuleSet(*rules);
    
    ContentRule* rule = new ContentRule("", 1, false);
    rule->GetSpecificationsR().push_back(new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, SelectionInfo(), options.GetJson());
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, SelectionInfo(), PageOptions(), options.GetJson());
    
    // expect 1 record
    ASSERT_EQ(1, content->GetContentSet().GetSize());
        
    // change a setting
    s_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", 1);

    // expect the content to be the same
    content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, SelectionInfo(), PageOptions(), options.GetJson());
    ASSERT_EQ(1, content->GetContentSet().GetSize());

    // expect 0 update records
    ASSERT_EQ(0, m_updateRecordsHandler->GetFullUpdateRecords().size());
    }
