/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <UnitTests/ECPresentation/TestRuleSetLocater.h>
#include "../../../Source/PresentationManagerImpl.h"
#include "../../../Source/Content/ContentCache.h"
#include "../Helpers/TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerImplTests : ECPresentationTest
    {
    static ECDbTestProject* s_project;
    std::shared_ptr<TestConnectionManager> m_connections;
    IConnectionPtr m_connection;
    RulesDrivenECPresentationManagerImpl* m_impl;
    TestCategorySupplier m_categorySupplier;
    TestRuleSetLocaterPtr m_locater;

    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;

    RulesDrivenECPresentationManagerImplTests()
        : m_categorySupplier(ContentDescriptor::Category("cat", "cat", "descr", 1))
        {}
    };
ECDbTestProject* RulesDrivenECPresentationManagerImplTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImplTests::SetUpTestCase()
    {
    s_project = new ECDbTestProject();
    s_project->Create("RulesDrivenECPresentationManagerImplTests");
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImplTests::TearDownTestCase()
    {
    DELETE_AND_CLEAR(s_project);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImplTests::SetUp()
    {
    ECPresentationTest::SetUp();

    m_locater = TestRuleSetLocater::Create();
    m_connections = std::make_shared<TestConnectionManager>();

    RulesDrivenECPresentationManagerImpl::Params::CachingParams cachingParams;
#ifdef USE_HYBRID_CACHE
    cachingParams.SetCacheMode(ECPresentationManager::Params::CachingParams::Mode::Hybrid);
#else
    cachingParams.SetCacheMode(ECPresentationManager::Params::CachingParams::Mode::Memory);
#endif
    RulesDrivenECPresentationManagerImpl::Params params(RulesEngineTestHelpers::GetPaths(BeTest::GetHost()));
    params.SetConnections(m_connections);
    params.SetCachingParams(cachingParams);
    params.SetCategorySupplier(&m_categorySupplier);
    m_impl = new RulesDrivenECPresentationManagerImpl(params);

    m_impl->GetLocaters().RegisterLocater(*m_locater);

    m_impl->GetConnections().CreateConnection(s_project->GetECDb());
    m_connection = m_impl->GetConnections().GetConnection(s_project->GetECDb());

    m_impl->Initialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImplTests::TearDown()
    {
    m_connections->CloseConnections();
    m_connection = nullptr;
    DELETE_AND_CLEAR(m_impl);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplTests, GetsChildrenForNodeWhoseGrandParentIsHiddenAfterNodesCacheIsCleared)
    {
    // create a ruleset
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    ruleset->AddPresentationRule(*new RootNodeRule());
    ruleset->GetRootNodesRules().back()->AddSpecification(*new CustomNodeSpecification(1, false, "T_ROOT", "root", "descr", "imageid"));
    ruleset->GetRootNodesRules().back()->GetSpecifications().back()->SetHideNodesInHierarchy(true);
    ruleset->AddPresentationRule(*new ChildNodeRule("ParentNode.Type=\"T_ROOT\"", 1, false));
    ruleset->GetChildNodesRules().back()->AddSpecification(*new CustomNodeSpecification(1, false, "T_CHILD1", "child1", "descr", "imageid"));
    ruleset->GetChildNodesRules().back()->AddSpecification(*new CustomNodeSpecification(1, false, "T_CHILD2", "child2", "descr", "imageid"));
    ruleset->AddPresentationRule(*new ChildNodeRule("ParentNode.Type=\"T_CHILD2\"", 1, false));
    ruleset->GetChildNodesRules().back()->AddSpecification(*new CustomNodeSpecification(1, false, "T_CHILD2.1", "child2.1", "descr", "imageid"));
    ruleset->AddPresentationRule(*new ChildNodeRule("ParentNode.Type=\"T_CHILD2.1\"", 1, false));
    ruleset->GetChildNodesRules().back()->AddSpecification(*new CustomNodeSpecification(1, false, "T_CHILD2.1.1", "child2.1.1", "descr", "imageid"));
    m_locater->AddRuleSet(*ruleset);

    // request and verify
    INavNodesDataSourcePtr rootNodes = m_impl->GetNodes(HierarchyRequestImplParams::Create(*m_connection, nullptr,
        ruleset->GetRuleSetId(), RulesetVariables()));
    ASSERT_EQ(2, rootNodes->GetSize());
    EXPECT_STREQ("child1", rootNodes->Get(0)->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("child2", rootNodes->Get(1)->GetLabelDefinition().GetDisplayValue().c_str());
    INavNodesDataSourcePtr childNodes = m_impl->GetNodes(HierarchyRequestImplParams::Create(*m_connection, nullptr,
        ruleset->GetRuleSetId(), RulesetVariables(), rootNodes->Get(1).get()));
    ASSERT_EQ(1, childNodes->GetSize());
    EXPECT_STREQ("child2.1", childNodes->Get(0)->GetLabelDefinition().GetDisplayValue().c_str());

    // clear nodes cache and request children for "T_CHILD2.1.1"
    NavNodeKeyCPtr key = childNodes->Get(0)->GetKey();
    m_impl->GetNodesCache(*m_connection)->Clear();

    auto nodes = m_impl->GetNodes(HierarchyRequestImplParams::Create(*m_connection, nullptr,
        ruleset->GetRuleSetId(), RulesetVariables(), key.get()));
    ASSERT_EQ(1, nodes->GetSize());
    EXPECT_STREQ("T_CHILD2.1.1", nodes->Get(0)->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplTests, CreatesSeparateNodesCachesForEachConnection)
    {
    ECDbTestProject secondProject;
    secondProject.Create("RulesDrivenECPresentationManagerImplTests2");
    IConnectionPtr connection2 = m_impl->GetConnections().CreateConnection(secondProject.GetECDb());

    std::shared_ptr<NodesCache> firstCache = m_impl->GetNodesCache(*m_connection);
    ASSERT_TRUE(nullptr != firstCache);
    std::shared_ptr<NodesCache> secondCache = m_impl->GetNodesCache(*connection2);
    ASSERT_TRUE(nullptr != secondCache);

    EXPECT_NE(firstCache, secondCache);
    m_connections->NotifyConnectionClosed(*connection2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplTests, ClosesNodesCacheWhenConnectionsIsClosed)
    {
    ECDbTestProject secondProject;
    secondProject.Create("RulesDrivenECPresentationManagerImplTests2");
    IConnectionPtr connection = m_impl->GetConnections().CreateConnection(secondProject.GetECDb());

    std::shared_ptr<NodesCache> nodesCache = m_impl->GetNodesCache(*connection);
    ASSERT_TRUE(nullptr != nodesCache);

    // simulate connection closing
    m_connections->NotifyConnectionClosed(*connection);
    nodesCache = m_impl->GetNodesCache(*connection);
    ASSERT_TRUE(nullptr == nodesCache);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplTests, UsesCorrectConnectionToGetDescriptorAfterProviderWasCached)
    {
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    ContentRuleP rule = new ContentRule();
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "ECDbMeta:ECEnumerationDef", true, false));
    ruleset->AddPresentationRule(*rule);
    m_locater->AddRuleSet(*ruleset);

    RefCountedPtr<TestConnection> connection1 = m_connections->CreateConnection(s_project->GetECDb());
    IConnectionPtr connection2 = m_connections->CreateSecondaryConnection(*connection1);

    m_impl->GetContentDescriptor(ContentDescriptorRequestImplParams::Create(*connection1, nullptr,
        ruleset->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create()));

    SpecificationContentProviderPtr provider = m_impl->GetContentCache().GetProviders(*connection1).front();
    provider->InvalidateDescriptor();

    // don't expect this connection to be used from now on
    connection1->SetUsageListener([](Utf8StringCR) { FAIL(); });

    m_impl->GetContentDescriptor(ContentDescriptorRequestImplParams::Create(*connection2, nullptr,
        ruleset->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create()));

    connection1->SetUsageListener(nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplTests, UsesCorrectConnectionToGetContentAfterProviderWasCached)
    {
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    ContentRuleP rule = new ContentRule();
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "ECDbMeta:ECEnumerationDef", true, false));
    ruleset->AddPresentationRule(*rule);
    m_locater->AddRuleSet(*ruleset);

    RefCountedPtr<TestConnection> connection1 = m_connections->CreateConnection(s_project->GetECDb());
    IConnectionPtr connection2 = m_connections->CreateSecondaryConnection(*connection1);

    auto descriptor = m_impl->GetContentDescriptor(ContentDescriptorRequestImplParams::Create(*connection1, nullptr,
        ruleset->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create()));
    m_impl->GetContent(ContentRequestImplParams::Create(*connection1, nullptr, *descriptor));

    SpecificationContentProviderPtr provider = m_impl->GetContentCache().GetProviders(*connection1).front();
    provider->InvalidateContent();

    // don't expect this connection to be used from now on
    connection1->SetUsageListener([](Utf8StringCR) { FAIL(); });

    m_impl->GetContent(ContentRequestImplParams::Create(*connection2, nullptr, *descriptor));

    connection1->SetUsageListener(nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplTests, UsesCorrectConnectionToGetContentAfterProviderWasCachedByGettingDescriptor)
    {
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    ContentRuleP rule = new ContentRule();
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "ECDbMeta:ECEnumerationDef", true, false));
    ruleset->AddPresentationRule(*rule);
    m_locater->AddRuleSet(*ruleset);

    RefCountedPtr<TestConnection> connection1 = m_connections->CreateConnection(s_project->GetECDb());
    IConnectionPtr connection2 = m_connections->CreateSecondaryConnection(*connection1);

    auto descriptor = m_impl->GetContentDescriptor(ContentDescriptorRequestImplParams::Create(*connection1, nullptr,
        ruleset->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create()));

    SpecificationContentProviderPtr provider = m_impl->GetContentCache().GetProviders(*connection1).front();
    provider->InvalidateDescriptor();

    // don't expect this connection to be used from now on
    connection1->SetUsageListener([](Utf8StringCR) { FAIL(); });

    m_impl->GetContent(ContentRequestImplParams::Create(*connection2, nullptr, *descriptor));

    connection1->SetUsageListener(nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplTests, UsesCorrectConnectionToGetContentSetSizeAfterProviderWasCached)
    {
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    ContentRuleP rule = new ContentRule();
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "ECDbMeta:ECEnumerationDef", true, false));
    ruleset->AddPresentationRule(*rule);
    m_locater->AddRuleSet(*ruleset);

    RefCountedPtr<TestConnection> connection1 = m_connections->CreateConnection(s_project->GetECDb());
    IConnectionPtr connection2 = m_connections->CreateSecondaryConnection(*connection1);

    auto descriptor = m_impl->GetContentDescriptor(ContentDescriptorRequestImplParams::Create(*connection1, nullptr,
        ruleset->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create()));
    m_impl->GetContentSetSize(ContentRequestImplParams::Create(*connection1, nullptr, *descriptor));

    SpecificationContentProviderPtr provider = m_impl->GetContentCache().GetProviders(*connection1).front();
    provider->InvalidateContent();

    // don't expect this connection to be used from now on
    connection1->SetUsageListener([](Utf8StringCR) { FAIL(); });

    m_impl->GetContentSetSize(ContentRequestImplParams::Create(*connection2, nullptr, *descriptor));

    connection1->SetUsageListener(nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplTests, UsesCorrectConnectionToGetDisplayLabelAfterProviderWasCached)
    {
    ECClassCP ecClass = s_project->GetECDb().Schemas().GetClass("ECDbMeta", "ECEnumerationDef");

    auto keys = KeySet::Create({ECClassInstanceKey(ecClass, ECInstanceId((uint64_t)1))});

    RefCountedPtr<TestConnection> connection1 = m_connections->CreateConnection(s_project->GetECDb());
    IConnectionPtr connection2 = m_connections->CreateSecondaryConnection(*connection1);

    m_impl->GetDisplayLabel(KeySetDisplayLabelRequestImplParams::Create(*connection1, nullptr, "", RulesetVariables(), *keys));

    SpecificationContentProviderPtr provider = m_impl->GetContentCache().GetProviders(*connection1).front();
    provider->InvalidateDescriptor();
    provider->InvalidateContent();

    // don't expect this connection to be used from now on
    connection1->SetUsageListener([](Utf8StringCR) { FAIL(); });

    m_impl->GetDisplayLabel(KeySetDisplayLabelRequestImplParams::Create(*connection2, nullptr, "", RulesetVariables(), *keys));

    connection1->SetUsageListener(nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplTests, UsesCorrectConnectionToGetDistinctValuesAfterProviderWasCached)
    {
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    ContentRuleP rule = new ContentRule();
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "ECDbMeta:ECEnumerationDef", true, false));
    ruleset->AddPresentationRule(*rule);
    m_locater->AddRuleSet(*ruleset);

    RefCountedPtr<TestConnection> connection1 = m_connections->CreateConnection(s_project->GetECDb());
    IConnectionPtr connection2 = m_connections->CreateSecondaryConnection(*connection1);

    auto descriptor = m_impl->GetContentDescriptor(ContentDescriptorRequestImplParams::Create(*connection1, nullptr,
        ruleset->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create()));
    m_impl->GetDistinctValues(DistinctValuesRequestImplParams::Create(*connection1, nullptr,
        *descriptor, std::make_unique<NamedContentFieldMatcher>(descriptor->GetVisibleFields().front()->GetUniqueName())));

    SpecificationContentProviderPtr provider = m_impl->GetContentCache().GetProviders(*connection1).front();
    provider->InvalidateDescriptor();
    provider->InvalidateContent();

    // don't expect this connection to be used from now on
    connection1->SetUsageListener([](Utf8StringCR) { FAIL(); });

    m_impl->GetDistinctValues(DistinctValuesRequestImplParams::Create(*connection2, nullptr,
        *descriptor, std::make_unique<NamedContentFieldMatcher>(descriptor->GetVisibleFields().front()->GetUniqueName())));

    connection1->SetUsageListener(nullptr);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerImplRequestCancelationTests : RulesDrivenECPresentationManagerImplTests
    {
    PresentationRuleSetPtr m_ruleset;

    static PresentationRuleSetPtr CreateRuleSet()
        {
        PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("RulesDrivenECPresentationManagerImplTests");

        RootNodeRuleP rootNodeRule = new RootNodeRule("", 1, false);
        rootNodeRule->AddSpecification(*new CustomNodeSpecification(1, false, "root_type", "label", "descr", "imageid"));
        ruleset->AddPresentationRule(*rootNodeRule);

        ChildNodeRuleP childNodeRule = new ChildNodeRule("", 1, false);
        childNodeRule->AddSpecification(*new CustomNodeSpecification(1, false, "child_type", "label", "descr", "imageid"));
        ruleset->AddPresentationRule(*childNodeRule);

        ContentRuleP contentRule = new ContentRule("", 1, false);
        contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "ECDbMeta:ECClassDef", true, false));
        ruleset->AddPresentationRule(*contentRule);

        return ruleset;
        }

    virtual void SetUp() override
        {
        RulesDrivenECPresentationManagerImplTests::SetUp();
        m_ruleset = CreateRuleSet();
        m_locater->AddRuleSet(*m_ruleset);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplRequestCancelationTests, AbortsNodesCountRequestWhenCanceledWhileLookingForRulesets)
    {
    // add a ruleset locater which cancels the request
    SimpleCancelationTokenPtr token = SimpleCancelationToken::Create();
    RefCountedPtr<TestCallbackRulesetLocater> locater = TestCallbackRulesetLocater::Create();
    m_impl->GetLocaters().RegisterLocater(*locater);
    m_impl->GetLocaters().InvalidateCache();
    locater->SetCallback([&]()
        {
        token->SetCanceled(true);
        });

    // request and verify
    size_t count = m_impl->GetNodesCount(HierarchyRequestImplParams::Create(*m_connection, token.get(), m_ruleset->GetRuleSetId(), RulesetVariables()));
    EXPECT_EQ(0, count);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplRequestCancelationTests, AbortsNodesRequestWhenCanceledWhileLookingForRulesets)
    {
    // add a ruleset locater which cancels the request
    SimpleCancelationTokenPtr token = SimpleCancelationToken::Create();
    RefCountedPtr<TestCallbackRulesetLocater> locater = TestCallbackRulesetLocater::Create();
    m_impl->GetLocaters().RegisterLocater(*locater);
    m_impl->GetLocaters().InvalidateCache();
    locater->SetCallback([&]()
        {
        token->SetCanceled(true);
        });

    // request and verify
    INavNodesDataSourceCPtr nodes = m_impl->GetNodes(HierarchyRequestImplParams::Create(*m_connection, token.get(), m_ruleset->GetRuleSetId(), RulesetVariables()));
    EXPECT_EQ(0, nodes->GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplRequestCancelationTests, AbortsContentClassesRequestWhenCanceledWhileLookingForRulesets)
    {
    // add a ruleset locater which cancels the request
    SimpleCancelationTokenPtr token = SimpleCancelationToken::Create();
    RefCountedPtr<TestCallbackRulesetLocater> locater = TestCallbackRulesetLocater::Create();
    m_impl->GetLocaters().RegisterLocater(*locater);
    m_impl->GetLocaters().InvalidateCache();
    locater->SetCallback([&]()
        {
        token->SetCanceled(true);
        });

    // request and verify
    ECClassCP inputClass = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECClassDef");
    bvector<SelectClassInfo> contentClasses = m_impl->GetContentClasses(ContentClassesRequestImplParams::Create(*m_connection, token.get(),
        m_ruleset->GetRuleSetId(), RulesetVariables(), "", 0, bvector<ECClassCP>{ inputClass }));
    EXPECT_TRUE(contentClasses.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplRequestCancelationTests, AbortsContentDescriptorRequestWhenCanceledWhileLookingForRulesets)
    {
    // add a ruleset locater which cancels the request
    SimpleCancelationTokenPtr token = SimpleCancelationToken::Create();
    RefCountedPtr<TestCallbackRulesetLocater> locater = TestCallbackRulesetLocater::Create();
    m_impl->GetLocaters().RegisterLocater(*locater);
    m_impl->GetLocaters().InvalidateCache();
    locater->SetCallback([&]()
        {
        token->SetCanceled(true);
        });

    // request and verify
    ContentDescriptorCPtr descriptor = m_impl->GetContentDescriptor(ContentDescriptorRequestImplParams::Create(*m_connection, token.get(),
        m_ruleset->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create()));
    EXPECT_TRUE(descriptor.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplRequestCancelationTests, AbortsContentRequestWhenCanceledWhileLookingForRulesets)
    {
    SimpleCancelationTokenPtr token = SimpleCancelationToken::Create();

    // get the descriptor
    ContentDescriptorCPtr descriptor = m_impl->GetContentDescriptor(ContentDescriptorRequestImplParams::Create(*m_connection, token.get(),
        m_ruleset->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create()));

    // add a ruleset locater which cancels the request
    RefCountedPtr<TestCallbackRulesetLocater> locater = TestCallbackRulesetLocater::Create();
    m_impl->GetLocaters().RegisterLocater(*locater);
    m_impl->GetLocaters().InvalidateCache();
    locater->SetCallback([&]()
        {
        token->SetCanceled(true);
        });

    // request and verify
    ContentCPtr content = m_impl->GetContent(ContentRequestImplParams::Create(*m_connection, token.get(), *descriptor));
    EXPECT_TRUE(content.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplRequestCancelationTests, AbortsContentSetSizeRequestWhenCanceledWhileLookingForRulesets)
    {
    SimpleCancelationTokenPtr token = SimpleCancelationToken::Create();

    // get the descriptor
    ContentDescriptorCPtr descriptor = m_impl->GetContentDescriptor(ContentDescriptorRequestImplParams::Create(*m_connection, token.get(),
        m_ruleset->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create()));

    // add a ruleset locater which cancels the request
    RefCountedPtr<TestCallbackRulesetLocater> locater = TestCallbackRulesetLocater::Create();
    m_impl->GetLocaters().RegisterLocater(*locater);
    m_impl->GetLocaters().InvalidateCache();
    locater->SetCallback([&]()
        {
        token->SetCanceled(true);
        });

    // request and verify
    size_t size = m_impl->GetContentSetSize(ContentRequestImplParams::Create(*m_connection, token.get(), *descriptor));
    EXPECT_EQ(0, size);
    }
