/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/SelectionTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
struct TestNodeKey : NavNodeKey
{
private:
    Utf8String m_label;
private:
    TestNodeKey(Utf8CP type) : NavNodeKey(type, {std::to_string(++s_keyIdCounter).c_str()}) {}
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
    std::function<folly::Future<folly::Unit>()> m_futureFactory;

    TestSelectionListener() : m_isSubSelection(false), m_futureFactory([](){return folly::makeFuture();}) {}

    folly::Future<folly::Unit> _OnSelectionChanged(SelectionChangedEventCR evt) override
        {
        m_sourceName = evt.GetSourceName();
        m_isSubSelection = evt.IsSubSelection();
        m_extendedData.CopyFrom(evt.GetExtendedData(), m_extendedData.GetAllocator());
        return m_futureFactory();
        }
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2016
+===============+===============+===============+===============+===============+======*/
struct SelectionTests : ECPresentationTest
    {
    static ECDbTestProject* s_project;
    TestConnectionManager m_connections;
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
        ECPresentationTest::SetUp();
        m_manager = new SelectionManager(m_connections);
        m_manager->AddListener(m_listener);
        m_connections.NotifyConnectionOpened(s_project->GetECDb());
        }

    void TearDown() override
        {
        m_manager->RemoveListener(m_listener);
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
    ASSERT_TRUE(m_manager->GetSelection(s_project->GetECDb())->Contains(*key));

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
    ASSERT_TRUE(m_manager->GetSelection(s_project->GetECDb())->Contains(*key2));
    
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
* @bsiclass                                     Tautvydas.Zinys                    09/2016
+===============+===============+===============+===============+===============+======*/
struct MultipleECDbSelectionTest : ECPresentationTest
    {
    static ECDbTestProject* s_project1;
    static ECDbTestProject* s_project2;
    TestConnectionManager m_connections;
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
        ECPresentationTest::SetUp();
        m_manager = new SelectionManager(m_connections);
        m_connections.NotifyConnectionOpened(s_project1->GetECDb());
        m_connections.NotifyConnectionOpened(s_project2->GetECDb());
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
    ASSERT_TRUE(m_manager->GetSelection(s_project1->GetECDb())->Contains(*key));

    ASSERT_EQ(0, m_manager->GetSelection(s_project2->GetECDb())->size());

    m_manager->AddToSelection(s_project2->GetECDb(), "TestSource", false, *key, CreateExtendedData());

    ASSERT_EQ(1, m_manager->GetSelection(s_project1->GetECDb())->size());
    ASSERT_TRUE(m_manager->GetSelection(s_project1->GetECDb())->Contains(*key));

    ASSERT_EQ(1, m_manager->GetSelection(s_project2->GetECDb())->size());
    ASSERT_TRUE(m_manager->GetSelection(s_project2->GetECDb())->Contains(*key));
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

    ASSERT_TRUE(m_manager->GetSelection(s_project1->GetECDb())->Contains(*key2));
    ASSERT_FALSE(m_manager->GetSelection(s_project2->GetECDb())->Contains(*key2));
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

/*=================================================================================**//**
* @bsiclass                                     Gerardas.Butkevicius            11/2018
+===============+===============+===============+===============+===============+======*/
struct AsyncSelectionTest : SelectionTests
    {
    TestSelectionListener m_listener1;

    void SetUp() override
        {
        SelectionTests::SetUp();
        m_manager->AddListener(m_listener1);
        }

    void TearDown() override
        {
        m_manager->RemoveListener(m_listener1);
        SelectionTests::TearDown();
        }
    };

//---------------------------------------------------------------------------------------
// @betest                                      Gerardas.Butkevicius            11/2018
//---------------------------------------------------------------------------------------
TEST_F(AsyncSelectionTest, AddToSelectionShouldResolveWhenAllListenersAreResolved)
    {
    NavNodeKeyPtr key = TestNodeKey::Create();
    folly::Promise<folly::Unit> promise1;
    folly::Promise<folly::Unit> promise2;
    m_listener.m_futureFactory = [&promise1]()
        {
        return promise1.getFuture();
        };
    m_listener1.m_futureFactory = [&promise2]()
        {
        return promise2.getFuture();
        };
    auto addToSelection = m_manager->AddToSelection(s_project->GetECDb(), "TestSource", false, *key, CreateExtendedData());

    ASSERT_FALSE(addToSelection.isReady());

    promise1.setValue();

    ASSERT_FALSE(addToSelection.isReady());

    promise2.setValue();

    ASSERT_TRUE(addToSelection.isReady());
    ASSERT_TRUE(addToSelection.hasValue());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Gerardas.Butkevicius            11/2018
//---------------------------------------------------------------------------------------
TEST_F(AsyncSelectionTest, RemoveFromSelectionShouldResolveWhenAllListenersAreResolved)
    {
    NavNodeKeyPtr key = TestNodeKey::Create();
    m_manager->AddToSelection(s_project->GetECDb(), "TestSource", false, *key);

    folly::Promise<folly::Unit> promise1;
    folly::Promise<folly::Unit> promise2;
    m_listener.m_futureFactory = [&promise1]()
        {
        return promise1.getFuture();
        };
    m_listener1.m_futureFactory = [&promise2]()
        {
        return promise2.getFuture();
        };
    auto removeFromSelection = m_manager->RemoveFromSelection(s_project->GetECDb(), "TestSource2", false, *key, CreateExtendedData());

    ASSERT_FALSE(removeFromSelection.isReady());

    promise1.setValue();

    ASSERT_FALSE(removeFromSelection.isReady());

    promise2.setValue();

    ASSERT_TRUE(removeFromSelection.isReady());
    ASSERT_TRUE(removeFromSelection.hasValue());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Gerardas.Butkevicius            11/2018
//---------------------------------------------------------------------------------------
TEST_F(AsyncSelectionTest, ChangeSelectionShouldResolveWhenAllListenersAreResolved)
    {
    NavNodeKeyPtr key1 = TestNodeKey::Create("1");
    m_manager->AddToSelection(s_project->GetECDb(), "TestSource", false, *key1);
    ASSERT_EQ(1, m_manager->GetSelection(s_project->GetECDb())->size());

    NavNodeKeyPtr key2 = TestNodeKey::Create("2");
    folly::Promise<folly::Unit> promise1;
    folly::Promise<folly::Unit> promise2;
    m_listener.m_futureFactory = [&promise1]()
        {
        return promise1.getFuture();
        };
    m_listener1.m_futureFactory = [&promise2]()
        {
        return promise2.getFuture();
        };
    auto changeSelection = m_manager->ChangeSelection(s_project->GetECDb(), "TestSource2", false, *key2, CreateExtendedData());

    ASSERT_FALSE(changeSelection.isReady());

    promise1.setValue();

    ASSERT_FALSE(changeSelection.isReady());

    promise2.setValue();

    ASSERT_TRUE(changeSelection.isReady());
    ASSERT_TRUE(changeSelection.hasValue());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Gerardas.Butkevicius            11/2018
//---------------------------------------------------------------------------------------
TEST_F(AsyncSelectionTest, ClearSelectionShouldResolveWhenAllListenersAreResolved)
    {
    NavNodeKeyPtr key = TestNodeKey::Create();
    m_manager->AddToSelection(s_project->GetECDb(), "TestSource", false, *key);
    ASSERT_EQ(1, m_manager->GetSelection(s_project->GetECDb())->size());

    folly::Promise<folly::Unit> promise1;
    folly::Promise<folly::Unit> promise2;
    m_listener.m_futureFactory = [&promise1]()
        {
        return promise1.getFuture();
        };
    m_listener1.m_futureFactory = [&promise2]()
        {
        return promise2.getFuture();
        };
    auto clearSelection = m_manager->ClearSelection(s_project->GetECDb(), "TestSource2", false, CreateExtendedData());

    ASSERT_FALSE(clearSelection.isReady());

    promise1.setValue();

    ASSERT_FALSE(clearSelection.isReady());

    promise2.setValue();

    ASSERT_TRUE(clearSelection.isReady());
    ASSERT_TRUE(clearSelection.hasValue());
    }
