/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/SelectionTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/SelectionManager.h>
#include <UnitTests/BackDoor/ECPresentation/TestRuleSetLocater.h>
#include "../BackDoor/PublicAPI/BackDoor/ECPresentation/SelectionManager.h"
#include "../NonPublished/RulesEngine/ECDbTestProject.h"
#include "../NonPublished/RulesEngine/TestNavNode.h"
#include "../NonPublished/RulesEngine/TestHelpers.h"

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

static int s_keyIdCounter = 0;
/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2016
+===============+===============+===============+===============+===============+======*/
struct TestNodeKey : DisplayLabelGroupingNodeKey
{
private:
    Utf8String m_label;
private:
    TestNodeKey(Utf8CP type) 
        : DisplayLabelGroupingNodeKey(++s_keyIdCounter, nullptr, type)
        {
        m_label = Utf8PrintfString("Test_%d", s_keyIdCounter);
        SetLabel(m_label.c_str());
        }
public:
    static RefCountedPtr<TestNodeKey> Create(Utf8CP type = "Test") {return new TestNodeKey(type);}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2016
+===============+===============+===============+===============+===============+======*/
struct TestSelectionListener : ISelectionChangesListener
    {
    Utf8String m_sourceName;
    bool m_isSubSelection;
    rapidjson::Document m_extendedData;

    TestSelectionListener() : m_isSubSelection(false) {}

    void _OnSelectionChanged(SelectionChangedEventCR evt) override
        {
        m_sourceName = evt.GetSourceName();
        m_isSubSelection = evt.IsSubSelection();
        m_extendedData.CopyFrom(evt.GetExtendedData(), m_extendedData.GetAllocator());
        }
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2016
+===============+===============+===============+===============+===============+======*/
struct SelectionTests : ::testing::Test
    {
    static ECDbTestProject* s_project;
    TestSelectionListener m_listener;
    SelectionManagerP m_manager;
    
    static void SetUpTestCase()
        {
        s_project = new ECDbTestProject();
        s_project->Create("SelectionTests", "RulesEngineTest.01.00.ecschema.xml");
        }

    static void TearDownTestCase()
        {
        delete s_project;
        s_project = nullptr;
        }

    void SetUp() override
        {
        m_manager = new SelectionManager();
        m_manager->AddListener(m_listener);
        }

    void TearDown() override
        {
        m_manager->RemoveListener(m_listener);
        DELETE_AND_CLEAR(m_manager);
        }

    static void AssertNavNodeKey(JsonValueCR keysJson, NavNodeKeyCR key)
        {
        EXPECT_TRUE(keysJson.isArray());
        EXPECT_EQ(1, keysJson.size());
        NavNodeKeyPtr keyFromJson = NavNodeKey::FromJson(keysJson[0]);
        if (keyFromJson.IsNull())
            {
            FAIL();
            return;
            }
        EXPECT_EQ(0, key.Compare(*keyFromJson));
        }

    static rapidjson::Document CreateExtendedData()
        {
        rapidjson::Document json;
        json.SetObject();
        json.AddMember("TestExtendedDataProperty", rapidjson::StringRef("SelectionTests"), json.GetAllocator());
        return json;
        }
    };
ECDbTestProject* SelectionTests::s_project = nullptr;

//---------------------------------------------------------------------------------------
// @betest                                      Grigas.Petraitis                08/2016
//---------------------------------------------------------------------------------------
TEST_F(SelectionTests, AddToSelection)
    {
    NavNodeKeyPtr key = TestNodeKey::Create();
    m_manager->AddToSelection(s_project->GetECDb(), "TestSource", false, *key, CreateExtendedData());

    // verify the selection is valid in the selection manager
    ASSERT_EQ(1, m_manager->GetSelection(s_project->GetECDb())->size());
    ASSERT_TRUE(m_manager->GetSelection(s_project->GetECDb())->end() != m_manager->GetSelection(s_project->GetECDb())->find(key));

    // verify listeners were notified
    ASSERT_STREQ("TestSource", m_listener.m_sourceName.c_str());
    ASSERT_FALSE(m_listener.m_isSubSelection);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Grigas.Petraitis                08/2016
//---------------------------------------------------------------------------------------
TEST_F(SelectionTests, RemoveFromSelection)
    {
    NavNodeKeyPtr key = TestNodeKey::Create();
    m_manager->AddToSelection(s_project->GetECDb(), "TestSource", false, *key);
    ASSERT_EQ(1, m_manager->GetSelection(s_project->GetECDb())->size());

    m_manager->RemoveFromSelection(s_project->GetECDb(), "TestSource2", false, *key, CreateExtendedData());

    // verify the selection is valid in the selection manager
    ASSERT_TRUE(m_manager->GetSelection(s_project->GetECDb())->empty());
    
    // verify listeners were notified
    ASSERT_STREQ("TestSource2", m_listener.m_sourceName.c_str());
    ASSERT_FALSE(m_listener.m_isSubSelection);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Grigas.Petraitis                08/2016
//---------------------------------------------------------------------------------------
TEST_F(SelectionTests, ChangeSelection)
    {
    NavNodeKeyPtr key1 = TestNodeKey::Create("1");
    m_manager->AddToSelection(s_project->GetECDb(), "TestSource", false, *key1);
    ASSERT_EQ(1, m_manager->GetSelection(s_project->GetECDb())->size());
    
    NavNodeKeyPtr key2 = TestNodeKey::Create("2");
    m_manager->ChangeSelection(s_project->GetECDb(), "TestSource2", false, *key2, CreateExtendedData());
    
    // verify the selection is valid in the selection manager
    ASSERT_EQ(1, m_manager->GetSelection(s_project->GetECDb())->size());
    ASSERT_TRUE(m_manager->GetSelection(s_project->GetECDb())->end() != m_manager->GetSelection(s_project->GetECDb())->find(key2));
    
    // verify listeners were notified
    ASSERT_STREQ("TestSource2", m_listener.m_sourceName.c_str());
    ASSERT_FALSE(m_listener.m_isSubSelection);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Grigas.Petraitis                08/2016
//---------------------------------------------------------------------------------------
TEST_F(SelectionTests, ClearSelection)
    {
    NavNodeKeyPtr key = TestNodeKey::Create();
    m_manager->AddToSelection(s_project->GetECDb(), "TestSource", false, *key);
    ASSERT_EQ(1, m_manager->GetSelection(s_project->GetECDb())->size());

    m_manager->ClearSelection(s_project->GetECDb(), "TestSource2", false, CreateExtendedData());

    // verify the selection is valid in the selection manager
    ASSERT_TRUE(m_manager->GetSelection(s_project->GetECDb())->empty());
    
    // verify listeners were notified
    ASSERT_STREQ("TestSource2", m_listener.m_sourceName.c_str());
    ASSERT_FALSE(m_listener.m_isSubSelection);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Grigas.Petraitis                11/2016
//---------------------------------------------------------------------------------------
TEST_F(SelectionTests, ClearsSelectionWhenConnectionIsClosed)
    {
    NavNodeKeyPtr key = TestNodeKey::Create();
    m_manager->AddToSelection(s_project->GetECDb(), "TestSource", false, *key);
    ASSERT_EQ(1, m_manager->GetSelection(s_project->GetECDb())->size());
    
    // reopening should clear the selection
    BeFileName path(s_project->GetECDb().GetDbFileName());
    s_project->GetECDb().CloseDb();
    s_project->GetECDb().OpenBeSQLiteDb(path, BeSQLite::Db::OpenParams(BeSQLite::Db::OpenMode::ReadWrite));

    ASSERT_EQ(0, m_manager->GetSelection(s_project->GetECDb())->size());
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2016
+===============+===============+===============+===============+===============+======*/
struct RemapTestSyncHandler : SelectionSyncHandler
    {
    std::function<void(bmap<uint64_t, uint64_t> const&)> m_onSelectedNodesRemapped;
    void SetOnSelectedNodesRemappedHandler(std::function<void(bmap<uint64_t, uint64_t> const&)> handler) {m_onSelectedNodesRemapped = handler;}

    rapidjson::Document _CreateSelectionEventExtendedData() const override {return rapidjson::Document();}
    SyncDirection _GetSyncDirection() const override {return SyncDirection::Both;}
    void _OnSelectedNodesRemapped(bmap<uint64_t, uint64_t> const& remapInfo) override
        {
        if (nullptr != m_onSelectedNodesRemapped)
            m_onSelectedNodesRemapped(remapInfo);
        }
    };

//---------------------------------------------------------------------------------------
// @betest                                      Grigas.Petraitis                10/2017
//---------------------------------------------------------------------------------------
TEST_F(SelectionTests, RemappingNodeIdsCallsSyncHandlersWithRemappedIds)
    {
    // create selection of 3 node keys
    NavNodeKeyList keys = {
        TestNodeKey::Create("type 1"),
        ECInstanceNodeKey::Create(ECClassId((uint64_t)1), ECInstanceId((uint64_t)2)),
        TestNodeKey::Create("type 2")
        };
    for (NavNodeKeyCPtr key : keys)
        m_manager->AddToSelection(s_project->GetECDb(), "", false, *NavNodeKeyListContainer::Create(keys));
    ASSERT_EQ(3, m_manager->GetSelection(s_project->GetECDb())->size());

    // create remap info for one key
    uint64_t keyBefore = keys[0]->AsDisplayLabelGroupingNodeKey()->GetNodeId();
    uint64_t keyAfter = s_keyIdCounter++;
    bmap<uint64_t, uint64_t> remapInfo;
    remapInfo[keyBefore] = keyAfter; // simulate remapping selected key
    remapInfo[s_keyIdCounter++] = s_keyIdCounter++; // simulate remapping unselected key

    // set up the selection handler
    bool handlerCalled = false;
    RefCountedPtr<RemapTestSyncHandler> syncHandler = new RemapTestSyncHandler();
    syncHandler->SetOnSelectedNodesRemappedHandler([&](bmap<uint64_t, uint64_t> const& remapInfo)
        {
        ASSERT_EQ(1, remapInfo.size());
        EXPECT_EQ(keyBefore, remapInfo.begin()->first);
        EXPECT_EQ(keyAfter, remapInfo.begin()->second);
        handlerCalled = true;
        });
    m_manager->AddSyncHandler(*syncHandler);

    // do remap
    BackDoor::SelectionManager::RemapNodeIds(*m_manager, remapInfo);

    // verify remap succeeded by making sure the keys[0] changed to the right value
    EXPECT_EQ(keyAfter, keys[0]->AsDisplayLabelGroupingNodeKey()->GetNodeId());

    // verify sync handler was called
    EXPECT_TRUE(handlerCalled);
    }

/*=================================================================================**//**
* @bsiclass                                     Tautvydas.Zinys                    09/2016
+===============+===============+===============+===============+===============+======*/
struct MultipleECDbSelectionTest : ::testing::Test
    {
    static ECDbTestProject* s_project1;
    static ECDbTestProject* s_project2;
    SelectionManagerP m_manager;

    static void SetUpTestCase()
        {
        s_project1 = new ECDbTestProject();
        s_project2 = new ECDbTestProject();
        s_project1->Create("RulesDrivenECPresentationManagerContentTests1", "RulesEngineTest.01.00.ecschema.xml");
        s_project2->Create("RulesDrivenECPresentationManagerContentTests2", "RulesEngineTest.01.00.ecschema.xml");
        }

    static void TearDownTestCase()
        {
        delete s_project1;
        delete s_project2;
        s_project1 = nullptr;
        s_project2 = nullptr;
        }

    void SetUp() override
        {
        m_manager = new SelectionManager();
        }

    void TearDown() override
        {
        DELETE_AND_CLEAR(m_manager);
        }

    static rapidjson::Document CreateExtendedData()
        {
        rapidjson::Document json;
        json.SetObject();
        json.AddMember("TestExtendedDataProperty", rapidjson::StringRef("SelectionTests"), json.GetAllocator());
        return json;
        }
    };
ECDbTestProject* MultipleECDbSelectionTest::s_project1 = nullptr;
ECDbTestProject* MultipleECDbSelectionTest::s_project2 = nullptr;

//---------------------------------------------------------------------------------------
// @betest                                      Tautvydas.Zinys                09/2016
//---------------------------------------------------------------------------------------
TEST_F(MultipleECDbSelectionTest, AddToSelection)
    {
    NavNodeKeyPtr key = TestNodeKey::Create();
    m_manager->AddToSelection(s_project1->GetECDb(), "TestSource", false, *key, CreateExtendedData());

    // verify the selection is valid in the selection manager for s_project1 and s_project2
    ASSERT_EQ(1, m_manager->GetSelection(s_project1->GetECDb())->size());
    ASSERT_TRUE(m_manager->GetSelection(s_project1->GetECDb())->end() != m_manager->GetSelection(s_project1->GetECDb())->find(key));

    ASSERT_EQ(0, m_manager->GetSelection(s_project2->GetECDb())->size());

    m_manager->AddToSelection(s_project2->GetECDb(), "TestSource", false, *key, CreateExtendedData());

    ASSERT_EQ(1, m_manager->GetSelection(s_project1->GetECDb())->size());
    ASSERT_TRUE(m_manager->GetSelection(s_project1->GetECDb())->end() != m_manager->GetSelection(s_project1->GetECDb())->find(key));

    ASSERT_EQ(1, m_manager->GetSelection(s_project2->GetECDb())->size());
    ASSERT_TRUE(m_manager->GetSelection(s_project2->GetECDb())->end() != m_manager->GetSelection(s_project2->GetECDb())->find(key));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Tautvydas.Zinys                09/2016
//---------------------------------------------------------------------------------------
TEST_F(MultipleECDbSelectionTest, RemoveFromSelection)
    {
    NavNodeKeyPtr key = TestNodeKey::Create();
    m_manager->AddToSelection(s_project1->GetECDb(), "TestSource", false, *key);
    m_manager->AddToSelection(s_project2->GetECDb(), "TestSource", false, *key);

    m_manager->RemoveFromSelection(s_project1->GetECDb(), "TestSource2", false, *key, CreateExtendedData());

    ASSERT_TRUE(m_manager->GetSelection(s_project1->GetECDb())->empty());
    ASSERT_FALSE(m_manager->GetSelection(s_project2->GetECDb())->empty());

    m_manager->RemoveFromSelection(s_project2->GetECDb(), "TestSource2", false, *key, CreateExtendedData());
    ASSERT_TRUE(m_manager->GetSelection(s_project2->GetECDb())->empty());
    ASSERT_TRUE(m_manager->GetSelection(s_project1->GetECDb())->empty());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Tautvydas.Zinys                09/2016
//---------------------------------------------------------------------------------------
TEST_F(MultipleECDbSelectionTest, ChangeSelection)
    {
    NavNodeKeyPtr key1 = TestNodeKey::Create("1");
    m_manager->AddToSelection(s_project1->GetECDb(), "TestSource", false, *key1);
    m_manager->AddToSelection(s_project2->GetECDb(), "TestSource", false, *key1);

    NavNodeKeyPtr key2 = TestNodeKey::Create("2");
    m_manager->ChangeSelection(s_project1->GetECDb(), "TestSource2", false, *key2, CreateExtendedData());

    ASSERT_TRUE(m_manager->GetSelection(s_project1->GetECDb())->end() != m_manager->GetSelection(s_project1->GetECDb())->find(key2));
    ASSERT_TRUE(m_manager->GetSelection(s_project2->GetECDb())->end() == m_manager->GetSelection(s_project2->GetECDb())->find(key2));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Tautvydas.Zinys                09/2016
//---------------------------------------------------------------------------------------
TEST_F(MultipleECDbSelectionTest, ClearSelection)
    {
    NavNodeKeyPtr key = TestNodeKey::Create();
    m_manager->AddToSelection(s_project1->GetECDb(), "TestSource", false, *key);
    m_manager->AddToSelection(s_project2->GetECDb(), "TestSource", false, *key);
    
    m_manager->ClearSelection(s_project1->GetECDb(), "TestSource2", false, CreateExtendedData());

    ASSERT_TRUE(m_manager->GetSelection(s_project1->GetECDb())->empty());
    ASSERT_FALSE(m_manager->GetSelection(s_project2->GetECDb())->empty());

    m_manager->ClearSelection(s_project2->GetECDb(), "TestSource2", false, CreateExtendedData());

    ASSERT_TRUE(m_manager->GetSelection(s_project1->GetECDb())->empty());
    ASSERT_TRUE(m_manager->GetSelection(s_project2->GetECDb())->empty());
    }
