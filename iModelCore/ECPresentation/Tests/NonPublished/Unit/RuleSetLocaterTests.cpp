/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentation/RuleSetLocater.h>
#include <UnitTests/ECPresentation/TestRuleSetLocater.h>
#include "../Helpers/ECDbTestProject.h"
#include "../Helpers/TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuleSetLocaterManagerTests, DisposesCachedRulesetsOnRulesetDispose)
    {
    m_project->Create(BeTest::GetNameOfCurrentTest());
    IConnectionPtr connection = m_connections->NotifyConnectionOpened(m_project->GetECDb());

    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    locater->AddRuleSet(*PresentationRuleSet::CreateInstance("Test"));
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuleSetLocaterManagerTests, DisposesCachedRulesetsOnConnectionClose)
    {
    m_project->Create(BeTest::GetNameOfCurrentTest());
    IConnectionPtr connection = m_connections->NotifyConnectionOpened(m_project->GetECDb());

    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    locater->SetDesignatedConnection(connection.get());
    locater->AddRuleSet(*PresentationRuleSet::CreateInstance("Test"));
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuleSetLocaterManagerTests, DisposesCachedRulesetsOnLocaterRegistered)
    {
    m_project->Create(BeTest::GetNameOfCurrentTest());
    IConnectionPtr connection = m_connections->NotifyConnectionOpened(m_project->GetECDb());

    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    locater->AddRuleSet(*PresentationRuleSet::CreateInstance("Test"));
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuleSetLocaterManagerTests, DisposesCachedRulesetsOnLocaterUnregistered)
    {
    IConnectionPtr connection = m_connections->NotifyConnectionOpened(m_project->GetECDb());

    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    locater->AddRuleSet(*PresentationRuleSet::CreateInstance("Test"));
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuleSetLocaterManagerTests, FindsRulesetsCreatedAfterLocate)
    {
    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    m_manager->RegisterLocater(*locater);

    m_project->Create(BeTest::GetNameOfCurrentTest());
    IConnectionPtr connection = m_connections->NotifyConnectionOpened(m_project->GetECDb());

    auto rulesetVer0 = PresentationRuleSet::CreateInstance("Test");
    rulesetVer0->SetRulesetVersion(Version(1, 0, 0));
    locater->AddRuleSet(*rulesetVer0);

    bvector<PresentationRuleSetPtr> rulesets = m_manager->LocateRuleSets(*connection, "Test");
    ASSERT_EQ(1, rulesets.size());
    EXPECT_EQ(0, rulesets[0]->GetRulesetVersion().Value().GetMinor());

    auto rulesetVer1 = PresentationRuleSet::CreateInstance("Test");
    rulesetVer1->SetRulesetVersion(Version(1, 1, 0));
    locater->AddRuleSet(*rulesetVer1);

    rulesets = m_manager->LocateRuleSets(*connection, "Test");
    ASSERT_EQ(2, rulesets.size());
    EXPECT_EQ(0, rulesets[0]->GetRulesetVersion().Value().GetMinor());
    EXPECT_EQ(1, rulesets[1]->GetRulesetVersion().Value().GetMinor());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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

    m_project->Create(BeTest::GetNameOfCurrentTest());
    IConnectionPtr connection = m_connections->NotifyConnectionOpened(m_project->GetECDb());

    PresentationRuleSetPtr ruleset1 = PresentationRuleSet::CreateInstance("Test");
    PresentationRuleSetPtr ruleset2 = PresentationRuleSet::CreateInstance("Test");
    PresentationRuleSetPtr ruleset3 = PresentationRuleSet::CreateInstance("Test");

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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuleSetLocaterManagerTests, InvalidateCacheForwardsCallToRegisteredLocaters)
    {
    TestRuleSetLocaterPtr locater1 = TestRuleSetLocater::Create();
    locater1->AddRuleSet(*PresentationRuleSet::CreateInstance("1"));
    m_manager->RegisterLocater(*locater1);

    TestRuleSetLocaterPtr locater2 = TestRuleSetLocater::Create();
    locater2->AddRuleSet(*PresentationRuleSet::CreateInstance("2"));
    m_manager->RegisterLocater(*locater2);

    EXPECT_FALSE(locater1->GetRuleSetIds().empty());
    EXPECT_FALSE(locater2->GetRuleSetIds().empty());

    m_manager->InvalidateCache();
    EXPECT_TRUE(locater1->GetRuleSetIds().empty());
    EXPECT_TRUE(locater2->GetRuleSetIds().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuleSetLocaterManagerTests, FindsSupplementalRulesetsAfterReregisteringRuleset)
    {
    m_project->Create(BeTest::GetNameOfCurrentTest());
    IConnectionPtr connection = m_connections->NotifyConnectionOpened(m_project->GetECDb());

    TestRuleSetLocaterPtr supplementalsProvider = TestRuleSetLocater::Create();
    auto supplementalRuleset = PresentationRuleSet::CreateInstance("");
    supplementalRuleset->SetSupplementationPurpose("supplemental");
    supplementalsProvider->AddRuleSet(*supplementalRuleset);
    RefCountedPtr<SupplementalRuleSetLocater> supplementalsLocater = SupplementalRuleSetLocater::Create(*supplementalsProvider);
    m_manager->RegisterLocater(*supplementalsLocater);

    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    locater->AddRuleSet(*PresentationRuleSet::CreateInstance("1"));
    m_manager->RegisterLocater(*locater);

    bvector<PresentationRuleSetPtr> locatedRulesets1 = m_manager->LocateRuleSets(*connection, "1");
    ASSERT_EQ(2, locatedRulesets1.size());

    locater->Clear();
    locater->AddRuleSet(*PresentationRuleSet::CreateInstance("2"));

    bvector<PresentationRuleSetPtr> locatedRulesets2 = m_manager->LocateRuleSets(*connection, "2");
    ASSERT_EQ(2, locatedRulesets2.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RuleSetLocater, CallsRulesetCallbacksHandlerWhenHandlerIsSetAfterOnRulesetCreatedCall)
    {
    TestRulesetCallbacksHandler handler;
    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("test");
    // note: AddRuleSet calls OnRuleSetCreated callback
    locater->AddRuleSet(*ruleset);

    int onCreatedCallbackCallCount = 0;
    handler.SetCreatedHandler([&](PresentationRuleSetCR callbackRuleset)
        {
        EXPECT_EQ(ruleset.get(), &callbackRuleset);
        ++onCreatedCallbackCallCount;
        });

    // make sure the handler got called
    locater->AddRulesetCallbacksHandler(handler);
    EXPECT_EQ(1, onCreatedCallbackCallCount);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RuleSetLocater, DoesntCallRulesetCallbacksHandlerWhenHandlerIsSetAfterOnRulesetCreatedAndDisposedCalls)
    {
    TestRulesetCallbacksHandler handler;
    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("test");
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

    locater->AddRulesetCallbacksHandler(handler);
    EXPECT_EQ(0, onCreatedCallbackCallCount);
    EXPECT_EQ(0, onDisposedCallbackCallCount);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
static BeFileName CreateFileWithContent(Utf8CP content, Utf8CP name, BeFileName const* directory = nullptr)
    {
    BeFileName path;
    if (nullptr != directory)
        path = *directory;
    else
        BeTest::GetHost().GetTempDir(path);

    if (!path.DoesPathExist())
        EXPECT_EQ(BeFileNameStatus::Success, path.CreateNewDirectory(path.c_str()));

    path.AppendToPath(WString(name, true).c_str());

    BeFile file;
    EXPECT_EQ(BeFileStatus::Success, file.Create(path, true));
    EXPECT_EQ(BeFileStatus::Success, file.Write(nullptr, content, (uint32_t)strlen(content)));
    EXPECT_EQ(BeFileStatus::Success, file.Close());

    return path;
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DirectoryRuleSetLocater, LocateRuleSets_NoDirectories)
    {
    RuleSetLocaterPtr locater = DirectoryRuleSetLocater::Create();
    auto rulesets = locater->LocateRuleSets();
    ASSERT_TRUE(rulesets.empty());
    ASSERT_TRUE(locater->GetRuleSetIds().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
* @betest
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
* @betest
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
* @betest
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
* @betest
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
* @betest
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
    locater->AddRulesetCallbacksHandler(handler);
    bvector<PresentationRuleSetPtr> rulesets = locater->LocateRuleSets();
    ASSERT_EQ(callbackRuleset, rulesets[0].get());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    locater->AddRulesetCallbacksHandler(handler);

    locater->InvalidateCache();
    EXPECT_EQ(2, disposedRulesetsCount);
    for (PresentationRuleSetPtr const& ruleset : rulesets)
        EXPECT_EQ(1, (int)ruleset->GetRefCount());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    locater->AddRulesetCallbacksHandler(handler);

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
* @betest
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
* @betest
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
* @betest
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
    locater->AddRulesetCallbacksHandler(handler);

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
* @betest
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
    locater->AddRulesetCallbacksHandler(handler);

    locater->InvalidateCache();
    EXPECT_EQ(1, disposedRulesetsCount);
    EXPECT_EQ(1, rulesets[0]->GetRefCount());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    locater->AddRulesetCallbacksHandler(handler);

    locater->InvalidateCache("Ruleset");
    EXPECT_EQ(1, disposedRulesetsCount);
    EXPECT_EQ(1, (int)rulesets[0]->GetRefCount());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    locater->AddRulesetCallbacksHandler(handler);

    locater->InvalidateCache("SomeNotMatchingId");
    EXPECT_EQ(0, disposedRulesetsCount);
    EXPECT_LT(1, (int)rulesets[0]->GetRefCount());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SupplementalRuleSetLocater, FindsSupplementalRuleSetsAndSetsProvidedId)
    {
    RefCountedPtr<SimpleRuleSetLocater> baseLocater = SimpleRuleSetLocater::Create();

    PresentationRuleSetPtr ruleset1 = PresentationRuleSet::CreateInstance("Ruleset1");
    ruleset1->SetIsSupplemental(true);
    ruleset1->AddPresentationRule(*new StyleOverride("", 1, "Red", "Green", ""));
    baseLocater->AddRuleSet(*ruleset1);

    PresentationRuleSetPtr ruleset2 = PresentationRuleSet::CreateInstance("Ruleset2");
    baseLocater->AddRuleSet(*ruleset2);

    PresentationRuleSetPtr ruleset3 = PresentationRuleSet::CreateInstance("Ruleset3");
    ruleset3->SetIsSupplemental(true);
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

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SupplementalRuleSetLocater, ForwardsRulesetCreatedAndDisposedCallbacks)
    {
    RefCountedPtr<SimpleRuleSetLocater> baseLocater = SimpleRuleSetLocater::Create();
    RefCountedPtr<SupplementalRuleSetLocater> locater = SupplementalRuleSetLocater::Create(*baseLocater);

    TestRulesetCallbacksHandler handler;
    int createdRulesetCount = 0;
    int disposedRulesetCount = 0;
    handler.SetCreatedHandler([&](PresentationRuleSetCR) { createdRulesetCount++; });
    handler.SetDisposedHandler([&](PresentationRuleSetCR) { disposedRulesetCount++; });
    locater->AddRulesetCallbacksHandler(handler);

    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("ruleset");
    baseLocater->AddRuleSet(*ruleset);
    EXPECT_EQ(1, createdRulesetCount);

    baseLocater->RemoveRuleSet(ruleset->GetRuleSetId());
    EXPECT_EQ(1, disposedRulesetCount);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SimpleRuleSetLocaterTests : ECPresentationTest
    {
    const PresentationRuleSetPtr ruleSet1 = PresentationRuleSet::CreateInstance("id1");
    const PresentationRuleSetPtr ruleSet2 = PresentationRuleSet::CreateInstance("id2");
    const PresentationRuleSetPtr ruleSet3 = PresentationRuleSet::CreateInstance("id3");
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
            m_ruleSetLocater->AddRulesetCallbacksHandler(*m_handler);
            }

        void TearDown() override
            {
            delete m_handler;
            }
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SimpleRuleSetLocaterTests, AddsRuleset)
    {
    m_ruleSetLocater->AddRuleSet(*ruleSet1);
    EXPECT_EQ(1, m_createdRulesetCount);
    EXPECT_EQ(0, m_disposedRulesetCount);
    EXPECT_EQ(1, m_ruleSetLocater->LocateRuleSets().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SimpleRuleSetLocaterTests, AddRuleSetReplacesRulesetWithSameId)
    {
    m_ruleSetLocater->AddRuleSet(*ruleSet1);
    bvector<PresentationRuleSetPtr> ruleSets = m_ruleSetLocater->LocateRuleSets();

    auto ruleset = PresentationRuleSet::CreateInstance("id1");
    ruleset->AddRequiredSchemaSpecification(*new RequiredSchemaSpecification("test schema"));
    m_ruleSetLocater->AddRuleSet(*ruleset);

    EXPECT_EQ(2, m_createdRulesetCount);
    EXPECT_EQ(1, m_disposedRulesetCount);
    EXPECT_EQ(1, ruleSets.size());
    EXPECT_EQ(1, m_ruleSetLocater->LocateRuleSets().size());
    EXPECT_NE(ruleSets[0], m_ruleSetLocater->LocateRuleSets()[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
* @betest
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SimpleRuleSetLocaterTests, LocatesNoRulesetsFromEmptyCache)
    {
    bvector<PresentationRuleSetPtr> ruleSets = m_ruleSetLocater->LocateRuleSets();
    EXPECT_EQ(0, m_createdRulesetCount);
    EXPECT_EQ(0, m_disposedRulesetCount);
    EXPECT_EQ(0, m_ruleSetLocater->LocateRuleSets().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
* @betest
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
* @betest
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
* @betest
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SimpleRuleSetLocaterTests, GetsRulesetsIdsFromEmptyCache)
    {
    bvector<Utf8String> ruleSetIds = m_ruleSetLocater->GetRuleSetIds();

    EXPECT_EQ(0, ruleSetIds.size());
    EXPECT_EQ(0, m_createdRulesetCount);
    EXPECT_EQ(0, m_disposedRulesetCount);
    }
