/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/RuleSetLocaterTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/RuleSetLocater.h>
#include <ECPresentation/RulesDriven/RuleSetEmbedder.h>
#include <UnitTests/BackDoor/ECPresentation/TestRuleSetLocater.h>
#include "ECDbTestProject.h"
#include "TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Aidas.Kilinskas                05/2018
+===============+===============+===============+===============+===============+======*/
struct RuleSetLocaterManagerTests : ECPresentationTest
    {
    ECDbTestProject* m_project;
    RuleSetLocaterManager* m_manager;
    TestConnectionManager* m_connections;
    void SetUp() override
        {
        ECPresentationTest::SetUp();
        m_connections = new TestConnectionManager();
        m_manager = new RuleSetLocaterManager(*m_connections);
        m_project = new ECDbTestProject();
        }

    void TearDown() override
        {
        ECPresentationTest::TearDown();
        DELETE_AND_CLEAR(m_manager);
        DELETE_AND_CLEAR(m_connections);
        DELETE_AND_CLEAR(m_project);
        }
    };


/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuleSetLocaterManagerTests, LocateSupportedRulesets)
    {
    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    m_manager->RegisterLocater(*locater);
    locater->AddRuleSet(*PresentationRuleSet::CreateInstance("EmptySupportedSchemas", 1, 0, false, "", "", "", false));
    locater->AddRuleSet(*PresentationRuleSet::CreateInstance("SupportedSchema", 1, 0, false, "", "SupportedSchema", "", false));
    locater->AddRuleSet(*PresentationRuleSet::CreateInstance("UnsupportedSchema", 1, 0, false, "", "UnsupportedSchema", "", false));
    locater->AddRuleSet(*PresentationRuleSet::CreateInstance("SupportedAndNotSupportedSchemas", 1, 0, false, "", "SupportedSchema,UnsupportedSchema", "", false));
    locater->AddRuleSet(*PresentationRuleSet::CreateInstance("ExcludedSchemas", 1, 0, false, "", "E:SupportedSchema", "", false));

    Utf8CP xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"                                                                                         \
                 "<ECSchema schemaName=\"SupportedSchema\" alias=\"sup\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">" \
                 "</ECSchema>";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ECSchema::ReadFromXmlString(schema, xml, *context);

    m_project->Create("RulesetLocaterManager_LocateSupportedRulesets");
    m_project->GetECDb().Schemas().ImportSchemas(context->GetCache().GetSchemas());
    IConnectionPtr connection = m_connections->NotifyConnectionOpened(m_project->GetECDb());

    bvector<PresentationRuleSetPtr> rulesets = m_manager->LocateRuleSets(*connection, nullptr);
    ASSERT_EQ(3, rulesets.size());

    // note: the order of rule sets is unknown
    EXPECT_TRUE(rulesets.end() != std::find_if(rulesets.begin(), rulesets.end(), [](PresentationRuleSetPtr const& ruleset){return ruleset->GetRuleSetId().Equals("EmptySupportedSchemas");}));
    EXPECT_TRUE(rulesets.end() != std::find_if(rulesets.begin(), rulesets.end(), [](PresentationRuleSetPtr const& ruleset){return ruleset->GetRuleSetId().Equals("SupportedSchema");}));
    EXPECT_TRUE(rulesets.end() != std::find_if(rulesets.begin(), rulesets.end(), [](PresentationRuleSetPtr const& ruleset){return ruleset->GetRuleSetId().Equals("ExcludedSchemas");}));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuleSetLocaterManagerTests, DisposesCachedRulesetsOnRulesetDispose)
    {
    m_project->Create("DisposesCachedRulesetsOnRulesetDispose");
    IConnectionPtr connection = m_connections->NotifyConnectionOpened(m_project->GetECDb());

    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    locater->AddRuleSet(*PresentationRuleSet::CreateInstance("Test", 1, 0, false, "", "", "", false));
    m_manager->RegisterLocater(*locater);

    RefCountedPtr<TestCallbackRulesetLocater> callbackLocater = TestCallbackRulesetLocater::Create();
    bool locateRuleSetsCalled;
    callbackLocater->SetCallback([&] ()
        {
        locateRuleSetsCalled = true;
        });
    m_manager->RegisterLocater(*callbackLocater);

    // first pass:
    locateRuleSetsCalled = false;
    bvector<PresentationRuleSetPtr> rulesets = m_manager->LocateRuleSets(*connection, nullptr);
    EXPECT_TRUE(locateRuleSetsCalled);
    ASSERT_EQ(1, rulesets.size());

    // verify manager doesnt ask locaters to locate:
    locateRuleSetsCalled = false;
    rulesets = m_manager->LocateRuleSets(*connection, "Test");
    EXPECT_FALSE(locateRuleSetsCalled);
    ASSERT_EQ(1, rulesets.size());

    // tell the ruleset was disposed
    ((IRulesetCallbacksHandler*)m_manager)->_OnRulesetDispose(*locater, *rulesets[0]);
    
    // verify locateRulesets was called:
    locateRuleSetsCalled = false;
    rulesets = m_manager->LocateRuleSets(*connection, "Test");
    EXPECT_TRUE(locateRuleSetsCalled);
    ASSERT_EQ(1, rulesets.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuleSetLocaterManagerTests, DisposesCachedRulesetsOnConnectionClose)
    {    
    m_project->Create("DisposesCachedRulesetsOnConnectionClose");
    IConnectionPtr connection = m_connections->NotifyConnectionOpened(m_project->GetECDb());

    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    locater->SetDesignatedConnection(connection.get());
    locater->AddRuleSet(*PresentationRuleSet::CreateInstance("Test", 1, 0, false, "", "", "", false));
    m_manager->RegisterLocater(*locater);

    RefCountedPtr<TestCallbackRulesetLocater> callbackLocater = TestCallbackRulesetLocater::Create();
    bool locateRuleSetsCalled;
    callbackLocater->SetCallback([&] ()
        {
        locateRuleSetsCalled = true;
        });
    m_manager->RegisterLocater(*callbackLocater);

    // first pass:
    locateRuleSetsCalled = false;
    bvector<PresentationRuleSetPtr> rulesets = m_manager->LocateRuleSets(*connection, nullptr);
    rulesets = m_manager->LocateRuleSets(*connection, "Test");
    EXPECT_TRUE(locateRuleSetsCalled);
    ASSERT_EQ(1, rulesets.size());

    // verify manager doesnt ask locaters to locate:
    locateRuleSetsCalled = false;
    rulesets = m_manager->LocateRuleSets(*connection, "Test");
    EXPECT_FALSE(locateRuleSetsCalled);
    ASSERT_EQ(1, rulesets.size());

    // simulate connection close
    m_connections->NotifyConnectionClosed(*connection);
    m_connections->NotifyConnectionOpened(m_project->GetECDb());
    
    // verify locateRulesets was called:
    locateRuleSetsCalled = false;
    rulesets = m_manager->LocateRuleSets(*connection, "Test");
    EXPECT_TRUE(locateRuleSetsCalled);
    ASSERT_EQ(1, rulesets.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Kilinskas                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuleSetLocaterManagerTests, DisposesCachedRulesetsOnLocaterRegistered)
    {    
    m_project->Create("DisposesCachedRulesetsOnLocaterRegistered");
    IConnectionPtr connection = m_connections->NotifyConnectionOpened(m_project->GetECDb());

    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    locater->AddRuleSet(*PresentationRuleSet::CreateInstance("Test", 1, 0, false, "", "", "", false));
    m_manager->RegisterLocater(*locater);

    RefCountedPtr<TestCallbackRulesetLocater> callbackLocater = TestCallbackRulesetLocater::Create();
    bool locateRuleSetsCalled;
    callbackLocater->SetCallback([&] ()
        {
        locateRuleSetsCalled = true;
        });
    m_manager->RegisterLocater(*callbackLocater);

    // first pass:
    locateRuleSetsCalled = false;
    bvector<PresentationRuleSetPtr> rulesets = m_manager->LocateRuleSets(*connection, nullptr);
    rulesets = m_manager->LocateRuleSets(*connection, "Test");
    EXPECT_TRUE(locateRuleSetsCalled);
    ASSERT_EQ(1, rulesets.size());

    // verify manager doesnt ask locaters to locate:
    locateRuleSetsCalled = false;
    rulesets = m_manager->LocateRuleSets(*connection, "Test");
    EXPECT_FALSE(locateRuleSetsCalled);
    ASSERT_EQ(1, rulesets.size());

    // register new locater
    TestRuleSetLocaterPtr locater1 = TestRuleSetLocater::Create();
    m_manager->RegisterLocater(*locater1);
    
    // verify locateRulesets was called:
    locateRuleSetsCalled = false;
    rulesets = m_manager->LocateRuleSets(*connection, "Test");
    EXPECT_TRUE(locateRuleSetsCalled);
    ASSERT_EQ(1, rulesets.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Kilinskas                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuleSetLocaterManagerTests, DisposesCachedRulesetsOnLocaterUnregistered)
    {    
    IConnectionPtr connection = m_connections->NotifyConnectionOpened(m_project->GetECDb());

    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    locater->AddRuleSet(*PresentationRuleSet::CreateInstance("Test", 1, 0, false, "", "", "", false));
    m_manager->RegisterLocater(*locater);

    RefCountedPtr<TestCallbackRulesetLocater> callbackLocater = TestCallbackRulesetLocater::Create();
    bool locateRuleSetsCalled;
    callbackLocater->SetCallback([&] ()
        {
        locateRuleSetsCalled = true;
        });
    m_manager->RegisterLocater(*callbackLocater);

    // first pass:
    locateRuleSetsCalled = false;
    bvector<PresentationRuleSetPtr> rulesets = m_manager->LocateRuleSets(*connection, nullptr);
    rulesets = m_manager->LocateRuleSets(*connection, "Test");
    EXPECT_TRUE(locateRuleSetsCalled);
    ASSERT_EQ(1, rulesets.size());

    // verify manager doesnt ask locaters to locate:
    locateRuleSetsCalled = false;
    rulesets = m_manager->LocateRuleSets(*connection, "Test");
    EXPECT_FALSE(locateRuleSetsCalled);
    ASSERT_EQ(1, rulesets.size());

    //unregister locater
    m_manager->UnregisterLocater(*locater);
    
    // verify locateRulesets was called:
    locateRuleSetsCalled = false;
    rulesets = m_manager->LocateRuleSets(*connection, "Test");
    EXPECT_TRUE(locateRuleSetsCalled);
    ASSERT_EQ(0, rulesets.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Kililiskas                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuleSetLocaterManagerTests, FindsRulesetsCreatedAfterLocate)
    {
    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    m_manager->RegisterLocater(*locater);

    m_project->Create("UpdatesCacheInOnRuleSetCreated");
    IConnectionPtr connection = m_connections->NotifyConnectionOpened(m_project->GetECDb());
    
    locater->AddRuleSet(*PresentationRuleSet::CreateInstance("Test", 1, 0, false, "", "", "", false));

    bvector<PresentationRuleSetPtr> rulesets = m_manager->LocateRuleSets(*connection, "Test");
    ASSERT_EQ(1, rulesets.size());
    EXPECT_EQ(0, rulesets[0]->GetVersionMinor());

    locater->AddRuleSet(*PresentationRuleSet::CreateInstance("Test", 1, 1, false, "", "", "", false));
    rulesets = m_manager->LocateRuleSets(*connection, "Test");
    ASSERT_EQ(2, rulesets.size());
    EXPECT_EQ(0, rulesets[0]->GetVersionMinor());
    EXPECT_EQ(1, rulesets[1]->GetVersionMinor());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Kililiskas                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuleSetLocaterManagerTests, FindsRulesetWhoseLocatersPriorityIsHigher)
    {
    TestRuleSetLocaterPtr locater1 = TestRuleSetLocater::Create();
    TestRuleSetLocaterPtr locater2 = TestRuleSetLocater::Create();
    TestRuleSetLocaterPtr locater3 = TestRuleSetLocater::Create();
    locater1->SetPriority(1);
    locater2->SetPriority(1);
    locater3->SetPriority(2);

    m_manager->RegisterLocater(*locater1);
    m_manager->RegisterLocater(*locater2);
    m_manager->RegisterLocater(*locater3);

    m_project->Create("FindsRulesetWhoseLocatersPriorityIsHigher");
    IConnectionPtr connection = m_connections->NotifyConnectionOpened(m_project->GetECDb());

    PresentationRuleSetPtr ruleset1 = PresentationRuleSet::CreateInstance("Test", 1, 0, false, "", "", "", false);
    PresentationRuleSetPtr ruleset2 = PresentationRuleSet::CreateInstance("Test", 1, 0, false, "", "", "", false);
    PresentationRuleSetPtr ruleset3 = PresentationRuleSet::CreateInstance("Test", 1, 0, false, "", "", "", false);
    
    locater1->AddRuleSet(*ruleset1);

    bvector<PresentationRuleSetPtr> rulesets = m_manager->LocateRuleSets(*connection, "Test");
    ASSERT_EQ(1, rulesets.size());
    EXPECT_EQ(ruleset1, rulesets[0]);

    locater2->AddRuleSet(*ruleset2);
    rulesets = m_manager->LocateRuleSets(*connection, "Test");
    ASSERT_EQ(1, rulesets.size());
    EXPECT_EQ(ruleset1, rulesets[0]);

    locater3->AddRuleSet(*ruleset3);
    rulesets = m_manager->LocateRuleSets(*connection, "Test");
    ASSERT_EQ(1, rulesets.size());
    EXPECT_EQ(ruleset3, rulesets[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuleSetLocaterManagerTests, InvalidateCacheForwardsCallToRegisteredLocaters)
    {
    TestRuleSetLocaterPtr locater1 = TestRuleSetLocater::Create();
    locater1->AddRuleSet(*PresentationRuleSet::CreateInstance("1", 1, 0, false, "", "", "", false));
    m_manager->RegisterLocater(*locater1);

    TestRuleSetLocaterPtr locater2 = TestRuleSetLocater::Create();
    locater2->AddRuleSet(*PresentationRuleSet::CreateInstance("2", 1, 0, false, "", "", "", false));
    m_manager->RegisterLocater(*locater2);

    EXPECT_FALSE(locater1->GetRuleSetIds().empty());
    EXPECT_FALSE(locater2->GetRuleSetIds().empty());

    m_manager->InvalidateCache();
    EXPECT_TRUE(locater1->GetRuleSetIds().empty());
    EXPECT_TRUE(locater2->GetRuleSetIds().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RuleSetLocater, CallsRulesetCallbacksHandlerWhenHandlerIsSetAfterOnRulesetCreatedCall)
    {
    TestRulesetCallbacksHandler handler;
    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("test", 1, 1, false, "", "", "", false);
    // note: AddRuleSet calls OnRuleSetCreated callback
    locater->AddRuleSet(*ruleset);

    int onCreatedCallbackCallCount = 0;
    handler.SetCreatedHandler([&](PresentationRuleSetCR callbackRuleset)
        {
        EXPECT_EQ(ruleset.get(), &callbackRuleset);
        ++onCreatedCallbackCallCount;
        });

    // make sure the handler got called
    locater->SetRulesetCallbacksHandler(&handler);
    EXPECT_EQ(1, onCreatedCallbackCallCount);

    // reset
    onCreatedCallbackCallCount = 0;
    locater->SetRulesetCallbacksHandler(nullptr);
    EXPECT_EQ(0, onCreatedCallbackCallCount);
    
    // set again to make sure it doesn't get called more than once
    onCreatedCallbackCallCount = 0;
    locater->SetRulesetCallbacksHandler(&handler);
    EXPECT_EQ(0, onCreatedCallbackCallCount);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RuleSetLocater, DoesntCallRulesetCallbacksHandlerWhenHandlerIsSetAfterOnRulesetCreatedAndDisposedCalls)
    {
    TestRulesetCallbacksHandler handler;
    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("test", 1, 1, false, "", "", "", false);
    // note: AddRuleSet calls OnRuleSetCreated callback
    locater->AddRuleSet(*ruleset);
    // note: Clear calls OnRuleSetDisposed callback for each added ruleset
    locater->Clear();

    int onCreatedCallbackCallCount = 0;
    handler.SetCreatedHandler([&](PresentationRuleSetCR)
        {
        ++onCreatedCallbackCallCount;
        });
    int onDisposedCallbackCallCount = 0;
    handler.SetDisposedHandler([&](PresentationRuleSetCR)
        {
        ++onDisposedCallbackCallCount;
        });

    locater->SetRulesetCallbacksHandler(&handler);
    EXPECT_EQ(0, onCreatedCallbackCallCount);
    EXPECT_EQ(0, onDisposedCallbackCallCount);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static BeFileName CreateFileWithContent(Utf8CP content, Utf8CP name, BeFileName const* directory = nullptr)
    {
    BeFileName path;
    if (nullptr != directory)
        path = *directory;
    else
        BeTest::GetHost().GetTempDir(path);

    if (!path.DoesPathExist())
        path.CreateNewDirectory(path.c_str());

    path.AppendToPath(WString(name, true).c_str());
    
    BeFile file;
    file.Create(path, true);
    file.Write(nullptr, content, (uint32_t)strlen(content));
    file.Close();

    return path;
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DirectoryRuleSetLocater, LocateRuleSets_NoDirectories)
    {
    RuleSetLocaterPtr locater = DirectoryRuleSetLocater::Create();
    auto rulesets = locater->LocateRuleSets();
    ASSERT_TRUE(rulesets.empty());
    ASSERT_TRUE(locater->GetRuleSetIds().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DirectoryRuleSetLocater, LocateRuleSets_NonExistingDirectory)
    {
    BeFileName directory;
    BeTest::GetHost().GetTempDir(directory);
    directory.AppendToPath(L"LocateRuleSets_NonExistingDirectory");
    if (directory.DoesPathExist())
        directory.BeDeleteFile();

    Utf8String list(directory.c_str());
    RuleSetLocaterPtr locater = DirectoryRuleSetLocater::Create(list.c_str());
    auto rulesets = locater->LocateRuleSets();
    ASSERT_TRUE(rulesets.empty());
    ASSERT_TRUE(locater->GetRuleSetIds().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DirectoryRuleSetLocater, LocateRuleSets_EmptyDirectory)
    {
    BeFileName directory;
    BeTest::GetHost().GetTempDir(directory);
    directory.AppendToPath(L"LocateRuleSets_EmptyDirectory");
    if (directory.DoesPathExist())
        directory.BeDeleteFile();

    BeFileName::CreateNewDirectory(directory.c_str());

    Utf8String list(directory.c_str());
    RuleSetLocaterPtr locater = DirectoryRuleSetLocater::Create(list.c_str());
    auto rulesets = locater->LocateRuleSets();
    ASSERT_TRUE(rulesets.empty());
    ASSERT_TRUE(locater->GetRuleSetIds().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DirectoryRuleSetLocater, LocateRuleSets_FindsXmlRuleSets)
    {
    BeFileName directory;
    BeTest::GetHost().GetTempDir(directory);
    directory.AppendToPath(WString(BeTest::GetNameOfCurrentTest(), true).c_str());
    if (directory.DoesPathExist())
        directory.BeDeleteFile();

    BeFileName::CreateNewDirectory(directory.c_str());

    Utf8CP ruleSetXmlString1 = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                               "<PresentationRuleSet RuleSetId=\"Items\" VersionMajor=\"5\" VersionMinor=\"3\""
                               "                     xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                               "                     xsi:noNamespaceSchemaLocation=\"PresentationRuleSetSchema.xsd\" />";
    Utf8CP ruleSetXmlString2 = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                               "<PresentationRuleSet RuleSetId=\"Files\" VersionMajor=\"1\" VersionMinor=\"3\""
                               "                     xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                               "                     xsi:noNamespaceSchemaLocation=\"PresentationRuleSetSchema.xsd\" />";

    CreateFileWithContent(ruleSetXmlString1, "1.PresentationRuleSet.xml", &directory);
    CreateFileWithContent(ruleSetXmlString2, "2.PresentationRuleSet.xml", &directory);
    
    Utf8String list(directory.c_str());
    RuleSetLocaterPtr locater = DirectoryRuleSetLocater::Create(list.c_str());
    bvector<PresentationRuleSetPtr> rulesets = locater->LocateRuleSets();
    ASSERT_EQ(2, rulesets.size());
    ASSERT_EQ(2, locater->GetRuleSetIds().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DirectoryRuleSetLocater, LocateRuleSets_FindsJsonRuleSets)
    {
    BeFileName directory;
    BeTest::GetHost().GetTempDir(directory);
    directory.AppendToPath(WString(BeTest::GetNameOfCurrentTest(), true).c_str());
    if (directory.DoesPathExist())
        directory.BeDeleteFile();

    BeFileName::CreateNewDirectory(directory.c_str());

    PresentationRuleSetPtr ruleset1 = PresentationRuleSet::CreateInstance("id_1");
    CreateFileWithContent(ruleset1->WriteToJsonValue().ToString().c_str(), "1.PresentationRuleSet.json", &directory);

    PresentationRuleSetPtr ruleset2 = PresentationRuleSet::CreateInstance("id_2");
    CreateFileWithContent(ruleset2->WriteToJsonValue().ToString().c_str(), "2.PresentationRuleSet.json", &directory);
    
    Utf8String list(directory.c_str());
    RuleSetLocaterPtr locater = DirectoryRuleSetLocater::Create(list.c_str());
    bvector<PresentationRuleSetPtr> rulesets = locater->LocateRuleSets();
    ASSERT_EQ(2, rulesets.size());
    ASSERT_EQ(2, locater->GetRuleSetIds().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DirectoryRuleSetLocater, LocateRuleSets_FindsRuleSetsInSubdirectories)
    {
    BeFileName directory;
    BeTest::GetHost().GetTempDir(directory);
    directory.AppendToPath(L"LocateRuleSets_FindsRuleSetsInSubdirectories");
    if (directory.DoesPathExist())
        directory.BeDeleteFile();

    BeFileName::CreateNewDirectory(directory.c_str());

    Utf8CP ruleSetXmlString1 = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                               "<PresentationRuleSet RuleSetId=\"Items\" VersionMajor=\"5\" VersionMinor=\"3\""
                               "                     xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                               "                     xsi:noNamespaceSchemaLocation=\"PresentationRuleSetSchema.xsd\" />";
    Utf8CP ruleSetXmlString2 = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                               "<PresentationRuleSet RuleSetId=\"Files\" VersionMajor=\"1\" VersionMinor=\"3\""
                               "                     xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                               "                     xsi:noNamespaceSchemaLocation=\"PresentationRuleSetSchema.xsd\" />";
    
    BeFileName ruleset1Path = directory;
    ruleset1Path.AppendToPath(L"subdirectory_1");
    CreateFileWithContent(ruleSetXmlString1, "a.PresentationRuleSet.xml", &ruleset1Path);

    BeFileName ruleset2Path = directory;
    ruleset2Path.AppendToPath(L"subdirectory_2");
    CreateFileWithContent(ruleSetXmlString2, "b.PresentationRuleSet.xml", &ruleset2Path);

    Utf8String list(directory.c_str());
    RuleSetLocaterPtr locater = DirectoryRuleSetLocater::Create(list.c_str());
    bvector<PresentationRuleSetPtr> rulesets = locater->LocateRuleSets();
    ASSERT_EQ(2, rulesets.size());
    ASSERT_EQ(2, locater->GetRuleSetIds().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DirectoryRuleSetLocater, CallsRulesetCallbacksHandlerWhenNecessary)
    {
    BeFileName directory;
    BeTest::GetHost().GetTempDir(directory);
    directory.AppendToPath(L"CallsRulesetCallbacksHandlerWhenNecessary");
    if (directory.DoesPathExist())
        directory.BeDeleteFile();

    BeFileName::CreateNewDirectory(directory.c_str());

    Utf8CP ruleSetXmlString = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                              "<PresentationRuleSet RuleSetId=\"Items\" VersionMajor=\"5\" VersionMinor=\"3\""
                              "                     xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                              "                     xsi:noNamespaceSchemaLocation=\"PresentationRuleSetSchema.xsd\" />";
    CreateFileWithContent(ruleSetXmlString, "ruleset.PresentationRuleSet.xml", &directory);
    
    PresentationRuleSetCP callbackRuleset = nullptr;
    TestRulesetCallbacksHandler handler;
    handler.SetCreatedHandler([&callbackRuleset](PresentationRuleSetCR ruleset){callbackRuleset = &ruleset;});

    RuleSetLocaterPtr locater = DirectoryRuleSetLocater::Create(directory.GetNameUtf8().c_str());
    locater->SetRulesetCallbacksHandler(&handler);
    bvector<PresentationRuleSetPtr> rulesets = locater->LocateRuleSets();
    ASSERT_EQ(callbackRuleset, rulesets[0].get());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DirectoryRuleSetLocater, DisposesAllCachedRulesetsWhenInvalidateRequestedWithNullRulesetId)
    {
    BeFileName directory;
    BeTest::GetHost().GetTempDir(directory);
    directory.AppendToPath(L"DisposesAllCachedRulesetsWhenInvalidateRequestedWithNullRulesetId");
    if (directory.DoesPathExist())
        directory.BeDeleteFile();

    BeFileName::CreateNewDirectory(directory.c_str());
    Utf8CP ruleSetXmlString1 = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                               "<PresentationRuleSet RuleSetId=\"Ruleset1\" VersionMajor=\"1\" VersionMinor=\"0\""
                               "                     xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                               "                     xsi:noNamespaceSchemaLocation=\"PresentationRuleSetSchema.xsd\" />";
    CreateFileWithContent(ruleSetXmlString1, "1.PresentationRuleSet.xml", &directory);
    Utf8CP ruleSetXmlString2 = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                               "<PresentationRuleSet RuleSetId=\"Ruleset2\" VersionMajor=\"1\" VersionMinor=\"0\""
                               "                     xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                               "                     xsi:noNamespaceSchemaLocation=\"PresentationRuleSetSchema.xsd\" />";
    CreateFileWithContent(ruleSetXmlString2, "2.PresentationRuleSet.xml", &directory);
    
    RuleSetLocaterPtr locater = DirectoryRuleSetLocater::Create(directory.GetNameUtf8().c_str());
    bvector<PresentationRuleSetPtr> rulesets = locater->LocateRuleSets();
    ASSERT_EQ(2, rulesets.size());

    size_t disposedRulesetsCount = 0;
    TestRulesetCallbacksHandler handler;
    handler.SetDisposedHandler([&disposedRulesetsCount](PresentationRuleSetCR){disposedRulesetsCount++;});
    locater->SetRulesetCallbacksHandler(&handler);

    locater->InvalidateCache();
    EXPECT_EQ(2, disposedRulesetsCount);
    for (PresentationRuleSetPtr const& ruleset : rulesets)
        EXPECT_EQ(1, (int)ruleset->GetRefCount());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DirectoryRuleSetLocater, DisposesRulesetWithSpecifiedIdWhenInvalidateRequested)
    {
    BeFileName directory;
    BeTest::GetHost().GetTempDir(directory);
    directory.AppendToPath(L"DisposesRulesetWithSpecifiedIdWhenInvalidateRequested");
    if (directory.DoesPathExist())
        directory.BeDeleteFile();

    BeFileName::CreateNewDirectory(directory.c_str());
    Utf8CP ruleSetXmlString1 = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                               "<PresentationRuleSet RuleSetId=\"Ruleset1\" VersionMajor=\"1\" VersionMinor=\"0\""
                               "                     xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                               "                     xsi:noNamespaceSchemaLocation=\"PresentationRuleSetSchema.xsd\" />";
    CreateFileWithContent(ruleSetXmlString1, "1.PresentationRuleSet.xml", &directory);
    Utf8CP ruleSetXmlString2 = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                               "<PresentationRuleSet RuleSetId=\"Ruleset2\" VersionMajor=\"1\" VersionMinor=\"0\""
                               "                     xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                               "                     xsi:noNamespaceSchemaLocation=\"PresentationRuleSetSchema.xsd\" />";
    CreateFileWithContent(ruleSetXmlString2, "2.PresentationRuleSet.xml", &directory);
    
    RuleSetLocaterPtr locater = DirectoryRuleSetLocater::Create(directory.GetNameUtf8().c_str());
    bvector<PresentationRuleSetPtr> rulesets = locater->LocateRuleSets();
    ASSERT_EQ(2, rulesets.size());

    size_t disposedRulesetsCount = 0;
    TestRulesetCallbacksHandler handler;
    handler.SetDisposedHandler([&disposedRulesetsCount](PresentationRuleSetCR){disposedRulesetsCount++;});
    locater->SetRulesetCallbacksHandler(&handler);

    locater->InvalidateCache("Ruleset1");
    EXPECT_EQ(1, disposedRulesetsCount);
    for (PresentationRuleSetPtr const& ruleset : rulesets)
        {
        if (ruleset->GetRuleSetId().Equals("Ruleset1"))
            EXPECT_EQ(1, (int)ruleset->GetRefCount());
        else
            EXPECT_LT(1, (int)ruleset->GetRefCount());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FileRuleSetLocater, LocateRuleSets_NonExistingFile)
    {
    BeFileName path;
    BeTest::GetHost().GetTempDir(path);
    path.AppendToPath(L"LocateRuleSets_NonExistingFile");
    if (path.DoesPathExist())
        path.BeDeleteFile();

    RuleSetLocaterPtr locater = FileRuleSetLocater::Create(path);
    bvector<PresentationRuleSetPtr> rulesets = locater->LocateRuleSets();
    ASSERT_TRUE(rulesets.empty());
    ASSERT_TRUE(locater->GetRuleSetIds().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FileRuleSetLocater, LocateRuleSets_FindsRuleset)
    {
    Utf8CP ruleSetXmlString = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                              "<PresentationRuleSet RuleSetId=\"Items\" VersionMajor=\"5\" VersionMinor=\"3\""
                              "                     xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                              "                     xsi:noNamespaceSchemaLocation=\"PresentationRuleSetSchema.xsd\" />";
    BeFileName path = CreateFileWithContent(ruleSetXmlString, "LocateRuleSets_FindsRuleset.xml");
    
    RuleSetLocaterPtr locater = FileRuleSetLocater::Create(path);
    bvector<PresentationRuleSetPtr> rulesets = locater->LocateRuleSets();
    ASSERT_EQ(1, rulesets.size());
    ASSERT_STREQ("Items", rulesets[0]->GetRuleSetId().c_str());
    ASSERT_STREQ("Items", locater->GetRuleSetIds()[0].c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FileRuleSetLocater, LocateRuleSets_RefreshesRulesetAfterModification)
    {
    Utf8CP ruleSetXmlString1 = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                               "<PresentationRuleSet RuleSetId=\"Items1\" VersionMajor=\"5\" VersionMinor=\"3\""
                               "                     xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                               "                     xsi:noNamespaceSchemaLocation=\"PresentationRuleSetSchema.xsd\" />";
    Utf8CP ruleSetXmlString2 = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                               "<PresentationRuleSet RuleSetId=\"Items2\" VersionMajor=\"5\" VersionMinor=\"3\""
                               "                     xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                               "                     xsi:noNamespaceSchemaLocation=\"PresentationRuleSetSchema.xsd\" />";
    BeFileName path = CreateFileWithContent(ruleSetXmlString1, "LocateRuleSets_RefreshesRulesetAfterModification.xml");
    
    PresentationRuleSetCP createdRuleset = nullptr, 
                          disposedRuleset = nullptr;
    TestRulesetCallbacksHandler handler;
    handler.SetCreatedHandler([&createdRuleset](PresentationRuleSetCR ruleset){createdRuleset = &ruleset;});
    handler.SetDisposedHandler([&disposedRuleset](PresentationRuleSetCR ruleset){disposedRuleset = &ruleset;});

    RuleSetLocaterPtr locater = FileRuleSetLocater::Create(path);
    locater->SetRulesetCallbacksHandler(&handler);

    bvector<PresentationRuleSetPtr> rulesets = locater->LocateRuleSets();
    ASSERT_EQ(1, rulesets.size());
    ASSERT_EQ(createdRuleset, rulesets[0].get());
    ASSERT_STREQ("Items1", createdRuleset->GetRuleSetId().c_str());

    RefCountedPtr<PresentationRuleSet const> previousRuleset = createdRuleset;
    createdRuleset = nullptr;

    BeThreadUtilities::BeSleep(1000); // sleep for 1 second to make sure the "last modified" time differs
    CreateFileWithContent(ruleSetXmlString2, "LocateRuleSets_RefreshesRulesetAfterModification.xml");
    rulesets = locater->LocateRuleSets();
    ASSERT_EQ(1, rulesets.size());
    ASSERT_EQ(disposedRuleset, previousRuleset.get());
    ASSERT_EQ(createdRuleset, rulesets[0].get());
    ASSERT_STREQ("Items2", createdRuleset->GetRuleSetId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (FileRuleSetLocater, DisposesCachedRulesetWhenInvalidateRequestedWithNullRulesetId)
    {
    Utf8CP ruleSetXmlString = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                              "<PresentationRuleSet RuleSetId=\"Ruleset\" VersionMajor=\"1\" VersionMinor=\"0\""
                              "                     xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                              "                     xsi:noNamespaceSchemaLocation=\"PresentationRuleSetSchema.xsd\" />";
    BeFileName rulesetPath = CreateFileWithContent(ruleSetXmlString, "ruleset.PresentationRuleSet.xml");
    
    RuleSetLocaterPtr locater = FileRuleSetLocater::Create(rulesetPath);
    bvector<PresentationRuleSetPtr> rulesets = locater->LocateRuleSets();
    ASSERT_EQ(1, rulesets.size());

    size_t disposedRulesetsCount = 0;
    TestRulesetCallbacksHandler handler;
    handler.SetDisposedHandler([&disposedRulesetsCount](PresentationRuleSetCR){disposedRulesetsCount++;});
    locater->SetRulesetCallbacksHandler(&handler);

    locater->InvalidateCache();
    EXPECT_EQ(1, disposedRulesetsCount);
    EXPECT_EQ(1, rulesets[0]->GetRefCount());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (FileRuleSetLocater, DisposesCachedRulesetWhenInvalidateRequestedWithMatchingRulesetId)
    {
    Utf8CP ruleSetXmlString = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                              "<PresentationRuleSet RuleSetId=\"Ruleset\" VersionMajor=\"1\" VersionMinor=\"0\""
                              "                     xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                              "                     xsi:noNamespaceSchemaLocation=\"PresentationRuleSetSchema.xsd\" />";
    BeFileName rulesetPath = CreateFileWithContent(ruleSetXmlString, "ruleset.PresentationRuleSet.xml");
    
    RuleSetLocaterPtr locater = FileRuleSetLocater::Create(rulesetPath);
    bvector<PresentationRuleSetPtr> rulesets = locater->LocateRuleSets();
    ASSERT_EQ(1, rulesets.size());

    size_t disposedRulesetsCount = 0;
    TestRulesetCallbacksHandler handler;
    handler.SetDisposedHandler([&disposedRulesetsCount](PresentationRuleSetCR){disposedRulesetsCount++;});
    locater->SetRulesetCallbacksHandler(&handler);
    
    locater->InvalidateCache("Ruleset");
    EXPECT_EQ(1, disposedRulesetsCount);
    EXPECT_EQ(1, (int)rulesets[0]->GetRefCount());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (FileRuleSetLocater, DoesntDisposeCachedRulesetIfInvalidateRequestedWithNotMatchingRulesetId)
    {
    Utf8CP ruleSetXmlString = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                              "<PresentationRuleSet RuleSetId=\"Ruleset\" VersionMajor=\"1\" VersionMinor=\"0\""
                              "                     xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                              "                     xsi:noNamespaceSchemaLocation=\"PresentationRuleSetSchema.xsd\" />";
    BeFileName rulesetPath = CreateFileWithContent(ruleSetXmlString, "ruleset.PresentationRuleSet.xml");
    
    RuleSetLocaterPtr locater = FileRuleSetLocater::Create(rulesetPath);
    bvector<PresentationRuleSetPtr> rulesets = locater->LocateRuleSets();
    ASSERT_EQ(1, rulesets.size());

    size_t disposedRulesetsCount = 0;
    TestRulesetCallbacksHandler handler;
    handler.SetDisposedHandler([&disposedRulesetsCount](PresentationRuleSetCR){disposedRulesetsCount++;});
    locater->SetRulesetCallbacksHandler(&handler);
    
    locater->InvalidateCache("SomeNotMatchingId");
    EXPECT_EQ(0, disposedRulesetsCount);
    EXPECT_LT(1, (int)rulesets[0]->GetRefCount());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SupplementalRuleSetLocater, FindsSupplementalRuleSetsAndSetsProvidedId)
    {
    RefCountedPtr<SimpleRuleSetLocater> baseLocater = SimpleRuleSetLocater::Create();

    PresentationRuleSetPtr ruleset1 = PresentationRuleSet::CreateInstance("Ruleset1", 1, 0, true, "", "", "", false);
    ruleset1->AddPresentationRule(*new StyleOverride("", 1, "Red", "Green", ""));
    baseLocater->AddRuleSet(*ruleset1);
    
    PresentationRuleSetPtr ruleset2 = PresentationRuleSet::CreateInstance("Ruleset2", 1, 0, false, "", "", "", false);
    baseLocater->AddRuleSet(*ruleset2);
    
    PresentationRuleSetPtr ruleset3 = PresentationRuleSet::CreateInstance("Ruleset3", 1, 0, true, "", "", "", false);
    ruleset3->AddPresentationRule(*new StyleOverride("", 1, "Blue", "Orange", ""));
    baseLocater->AddRuleSet(*ruleset3);

    RefCountedPtr<SupplementalRuleSetLocater> locater = SupplementalRuleSetLocater::Create(*baseLocater);
    bvector<PresentationRuleSetPtr> rulesets = locater->LocateRuleSets("Ruleset1");
    ASSERT_EQ(2, rulesets.size());

    //Using StyleOverride as ruleset identification, because ruleset id was changed
    ASSERT_STREQ("Ruleset1", rulesets[0]->GetRuleSetId().c_str())
        << "Got: " << rulesets[0]->GetRuleSetId().c_str();
    EXPECT_STREQ("Red", rulesets[0]->GetStyleOverrides()[0]->GetForeColor().c_str())
        << "Got: " << rulesets[0]->GetStyleOverrides()[0]->GetForeColor().c_str();
    EXPECT_STREQ("Green", rulesets[0]->GetStyleOverrides()[0]->GetBackColor().c_str())
        << "Got: " << rulesets[0]->GetStyleOverrides()[0]->GetBackColor().c_str();

    ASSERT_STREQ("Ruleset1", rulesets[1]->GetRuleSetId().c_str())
        << "Got: " << rulesets[1]->GetRuleSetId().c_str();
    EXPECT_STREQ("Blue", rulesets[1]->GetStyleOverrides()[0]->GetForeColor().c_str())
        << "Got: " << rulesets[1]->GetStyleOverrides()[0]->GetForeColor().c_str();
    EXPECT_STREQ("Orange", rulesets[1]->GetStyleOverrides()[0]->GetBackColor().c_str())
        << "Got: " << rulesets[1]->GetStyleOverrides()[0]->GetBackColor().c_str();
    }

#ifdef OldEmbeddedRulesetLocater
/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                10/2017
+===============+===============+===============+===============+===============+======*/
struct EmbeddedRuleSetLocaterTests : ECPresentationTest
{
    static ECDbTestProject* s_project;
    IConnectionPtr m_connection;
    RuleSetEmbedder* m_embedder;

    void SetUp() override
        {
        ECPresentationTest::SetUp();
        m_connection = new TestConnection(s_project->GetECDb());
        m_embedder = new RuleSetEmbedder(m_connection->GetECDb());
        }

    void TearDown() override
        {
        s_project->GetECDb().AbandonChanges();
        DELETE_AND_CLEAR(m_embedder);
        }

    static void SetUpTestCase()
        {
        s_project = new ECDbTestProject();
        s_project->Create("EmbeddedRuleSetLocaterTests");
        }

    static void TearDownTestCase()
        {
        DELETE_AND_CLEAR(s_project);
        }
};
ECDbTestProject* EmbeddedRuleSetLocaterTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(EmbeddedRuleSetLocaterTests, LocateRuleSets_ConnectionWithoutEmbeddedFiles)
    {
    // create ruleset locater
    RuleSetLocaterPtr locater = EmbeddedRuleSetLocater::Create(*m_connection);

    // validate no rulesets are located if connection doesn't have embedded rulesets.
    bvector<PresentationRuleSetPtr> rulesets = locater->LocateRuleSets();
    ASSERT_TRUE(rulesets.empty());
    ASSERT_TRUE(locater->GetRuleSetIds().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(EmbeddedRuleSetLocaterTests, LocateRuleSets_FindsRuleSet)
    {
    // create ruleset
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("LocateRuleSets_FindsRuleSet", 1, 0, false, "", "", "", false);

    // embed ruleset
    m_embedder->Embed(*ruleset);

    // create ruleset locater
    RuleSetLocaterPtr locater = EmbeddedRuleSetLocater::Create(*m_connection);

    // validate 1 ruleset was found in opened connection
    bvector<PresentationRuleSetPtr> rulesets = locater->LocateRuleSets();
    ASSERT_EQ(1, rulesets.size());
    EXPECT_STREQ("LocateRuleSets_FindsRuleSet", rulesets[0]->GetRuleSetId().c_str());
    ASSERT_EQ(1, locater->GetRuleSetIds().size());
    EXPECT_STREQ("LocateRuleSets_FindsRuleSet", locater->GetRuleSetIds()[0].c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(EmbeddedRuleSetLocaterTests, LocateRuleSets_FindsSpecificRuleSetInCache)
    {
    // create rulesets
    PresentationRuleSetPtr ruleset1 = PresentationRuleSet::CreateInstance("LocateRuleSets_FindsRuleSet1", 1, 0, false, "", "", "", false);
    PresentationRuleSetPtr ruleset2 = PresentationRuleSet::CreateInstance("LocateRuleSets_FindsRuleSet2", 1, 0, false, "", "", "", false);
    // embed rulesets
    m_embedder->Embed(*ruleset1);
    m_embedder->Embed(*ruleset2);
    // create ruleset locater
    RuleSetLocaterPtr locater = EmbeddedRuleSetLocater::Create(*m_connection);
    // validate 2 ruleset was found in opened connection
    ASSERT_EQ(2, locater->GetRuleSetIds().size());
    // find specific ruleset
    bvector<PresentationRuleSetPtr> rulesets = locater->LocateRuleSets(ruleset1->GetRuleSetId().c_str());
    ASSERT_EQ(1, rulesets.size());
    EXPECT_STREQ(ruleset1->GetRuleSetId().c_str(), rulesets[0]->GetRuleSetId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(EmbeddedRuleSetLocaterTests, DisposesAllCachedRulesetsWhenInvalidateRequested)
    {
    // create 2 rulesets
    PresentationRuleSetPtr ruleset1 = PresentationRuleSet::CreateInstance("DisposesAllCachedRulesetsWhenInvalidateRequested", 1, 0, false, "", "", "", false);
    PresentationRuleSetPtr ruleset2 = PresentationRuleSet::CreateInstance("DisposesAllCachedRulesetsWhenInvalidateRequested", 2, 0, false, "", "", "", false);

    // embed rulesets
    m_embedder->Embed(*ruleset1);
    m_embedder->Embed(*ruleset2);

    // create ruleset locater
    RuleSetLocaterPtr locater = EmbeddedRuleSetLocater::Create(*m_connection);

    // verify 2 rulesets are located
    bvector<PresentationRuleSetPtr> rulesets = locater->LocateRuleSets();
    ASSERT_EQ(2, rulesets.size());

    size_t disposedRulesetsCount = 0;
    TestRulesetCallbacksHandler handler;
    handler.SetDisposedHandler([&disposedRulesetsCount](PresentationRuleSetCR) {disposedRulesetsCount++; });
    locater->SetRulesetCallbacksHandler(&handler);

    // check if both rulesets were disposed from cache
    locater->InvalidateCache();
    EXPECT_EQ(2, disposedRulesetsCount);
    EXPECT_EQ(1, rulesets[0]->GetRefCount());
    EXPECT_EQ(1, rulesets[1]->GetRefCount());
    }
#endif

/*=================================================================================**//**
* @bsiclass                                     Aidas.Kilinskas                05/2018
+===============+===============+===============+===============+===============+======*/
struct SimpleRuleSetLocaterTests : ECPresentationTest
    {
    const PresentationRuleSetPtr ruleSet1 = PresentationRuleSet::CreateInstance("id1", 0, 0, "", "", "", "", false);
    const PresentationRuleSetPtr ruleSet2 = PresentationRuleSet::CreateInstance("id2", 0, 0, "", "", "", "", false);
    const PresentationRuleSetPtr ruleSet3 = PresentationRuleSet::CreateInstance("id3", 0, 0, "", "", "", "", false);
    RefCountedPtr<SimpleRuleSetLocater> m_ruleSetLocater;
    size_t m_disposedRulesetCount;
    size_t m_createdRulesetCount;

    private:
        TestRulesetCallbacksHandler* m_handler;

    public:
        void SetUp() override
            {
            m_ruleSetLocater = SimpleRuleSetLocater::Create();
            m_createdRulesetCount = 0;
            m_disposedRulesetCount = 0;
            m_handler = new TestRulesetCallbacksHandler();
            m_handler->SetCreatedHandler([this] (PresentationRuleSetCR) { m_createdRulesetCount++; });
            m_handler->SetDisposedHandler([this] (PresentationRuleSetCR) { m_disposedRulesetCount++; });
            m_ruleSetLocater->SetRulesetCallbacksHandler(m_handler);
            }

        void TearDown() override
            {
            delete m_handler;
            }
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Kilinskas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SimpleRuleSetLocaterTests, AddsRuleset)
    {
    m_ruleSetLocater->AddRuleSet(*ruleSet1);
    EXPECT_EQ(1, m_createdRulesetCount);
    EXPECT_EQ(0, m_disposedRulesetCount);
    EXPECT_EQ(1, m_ruleSetLocater->LocateRuleSets().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Kilinskas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SimpleRuleSetLocaterTests, AddRuleSetReplacesRulesetWithSameId)
    {
    m_ruleSetLocater->AddRuleSet(*ruleSet1);
    bvector<PresentationRuleSetPtr> ruleSets = m_ruleSetLocater->LocateRuleSets();
    m_ruleSetLocater->AddRuleSet(*PresentationRuleSet::CreateInstance("id1", 0, 0, "", "", "", "", false));

    EXPECT_EQ(2, m_createdRulesetCount);
    EXPECT_EQ(1, m_disposedRulesetCount);
    EXPECT_EQ(1, ruleSets.size());
    EXPECT_EQ(1, m_ruleSetLocater->LocateRuleSets().size());
    EXPECT_NE(ruleSets[0], m_ruleSetLocater->LocateRuleSets()[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Kilinskas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SimpleRuleSetLocaterTests, LocatesSpecificRuleset)
    {
    m_ruleSetLocater->AddRuleSet(*ruleSet1);
    m_ruleSetLocater->AddRuleSet(*ruleSet2);
    m_ruleSetLocater->AddRuleSet(*ruleSet3);

    bvector<PresentationRuleSetPtr> ruleSets = m_ruleSetLocater->LocateRuleSets("id2");
    EXPECT_EQ(3, m_createdRulesetCount);
    EXPECT_EQ(0, m_disposedRulesetCount);
    EXPECT_EQ(1, ruleSets.size());
    EXPECT_STREQ("id2", ruleSets[0]->GetRuleSetId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Kilinskas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SimpleRuleSetLocaterTests, LocatesAllRulesets)
    {
    m_ruleSetLocater->AddRuleSet(*ruleSet1);
    m_ruleSetLocater->AddRuleSet(*ruleSet2);
    m_ruleSetLocater->AddRuleSet(*ruleSet3);

    bvector<PresentationRuleSetPtr> ruleSets = m_ruleSetLocater->LocateRuleSets();
    EXPECT_EQ(3, m_ruleSetLocater->LocateRuleSets().size());
    EXPECT_EQ(3, m_createdRulesetCount);
    EXPECT_EQ(0, m_disposedRulesetCount);
    EXPECT_STREQ("id1", ruleSets[0]->GetRuleSetId().c_str());
    EXPECT_STREQ("id2", ruleSets[1]->GetRuleSetId().c_str());
    EXPECT_STREQ("id3", ruleSets[2]->GetRuleSetId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Kilinskas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SimpleRuleSetLocaterTests, LocatesNoRulesetsFromEmptyCache)
    {
    bvector<PresentationRuleSetPtr> ruleSets = m_ruleSetLocater->LocateRuleSets();
    EXPECT_EQ(0, m_createdRulesetCount);
    EXPECT_EQ(0, m_disposedRulesetCount);
    EXPECT_EQ(0, m_ruleSetLocater->LocateRuleSets().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Kilinskas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SimpleRuleSetLocaterTests, RemovesSpecificRuleSet)
    {
    m_ruleSetLocater->AddRuleSet(*ruleSet1);
    m_ruleSetLocater->AddRuleSet(*ruleSet2);
    m_ruleSetLocater->AddRuleSet(*ruleSet3);

    m_ruleSetLocater->RemoveRuleSet("id2");
    bvector<PresentationRuleSetPtr> ruleSets = m_ruleSetLocater->LocateRuleSets();

    EXPECT_EQ(2, m_ruleSetLocater->LocateRuleSets().size());
    EXPECT_EQ(3, m_createdRulesetCount);
    EXPECT_EQ(1, m_disposedRulesetCount);
    EXPECT_STREQ("id1", ruleSets[0]->GetRuleSetId().c_str());
    EXPECT_STREQ("id3", ruleSets[1]->GetRuleSetId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Kilinskas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SimpleRuleSetLocaterTests, ClearsRulesets)
    {
    m_ruleSetLocater->AddRuleSet(*ruleSet1);
    m_ruleSetLocater->AddRuleSet(*ruleSet2);
    m_ruleSetLocater->AddRuleSet(*ruleSet3);

    m_ruleSetLocater->Clear();
    bvector<PresentationRuleSetPtr> ruleSets = m_ruleSetLocater->LocateRuleSets();
    EXPECT_EQ(3, m_createdRulesetCount);
    EXPECT_EQ(3, m_disposedRulesetCount);
    EXPECT_EQ(0, m_ruleSetLocater->LocateRuleSets().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Kilinskas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SimpleRuleSetLocaterTests, ClearsEmptyCache)
    {
    m_ruleSetLocater->Clear();
    bvector<PresentationRuleSetPtr> ruleSets = m_ruleSetLocater->LocateRuleSets();
    EXPECT_EQ(0, m_createdRulesetCount);
    EXPECT_EQ(0, m_disposedRulesetCount);
    EXPECT_EQ(0, m_ruleSetLocater->LocateRuleSets().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Kilinskas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SimpleRuleSetLocaterTests, GetsRulesetsIds)
    {
    m_ruleSetLocater->AddRuleSet(*ruleSet1);
    m_ruleSetLocater->AddRuleSet(*ruleSet2);
    m_ruleSetLocater->AddRuleSet(*ruleSet3);

    bvector<Utf8String> ruleSetIds = m_ruleSetLocater->GetRuleSetIds();

    EXPECT_EQ(3, ruleSetIds.size());
    EXPECT_EQ(3, m_createdRulesetCount);
    EXPECT_EQ(0, m_disposedRulesetCount);
    EXPECT_STREQ("id1", ruleSetIds[0].c_str());
    EXPECT_STREQ("id2", ruleSetIds[1].c_str());
    EXPECT_STREQ("id3", ruleSetIds[2].c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Kilinskas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SimpleRuleSetLocaterTests, GetsRulesetsIdsFromEmptyCache)
    {
    bvector<Utf8String> ruleSetIds = m_ruleSetLocater->GetRuleSetIds();

    EXPECT_EQ(0, ruleSetIds.size());
    EXPECT_EQ(0, m_createdRulesetCount);
    EXPECT_EQ(0, m_disposedRulesetCount);
    }
