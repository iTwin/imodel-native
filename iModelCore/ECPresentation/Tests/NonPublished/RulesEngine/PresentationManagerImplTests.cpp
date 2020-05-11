/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "TestHelpers.h"
#include <UnitTests/BackDoor/ECPresentation/TestRuleSetLocater.h>
#include "../../../Source/RulesDriven/RulesEngine/PresentationManagerImpl.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerImplTests : ECPresentationTest
    {
    static ECDbTestProject* s_project;
    std::shared_ptr<TestConnectionManager> m_connections;
    IConnectionPtr m_connection;
    RulesDrivenECPresentationManagerImpl* m_impl;
    TestCategorySupplier m_categorySupplier;
    TestRuleSetLocaterPtr m_locater;
    SQLangLocalizationProvider m_localizationProvider;

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
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImplTests::SetUpTestCase()
    {
    Localization::Init();
    s_project = new ECDbTestProject();
    s_project->Create("RulesDrivenECPresentationManagerImplTests");
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImplTests::TearDownTestCase()
    {
    DELETE_AND_CLEAR(s_project);
    Localization::Terminate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImplTests::SetUp()
    {
    ECPresentationTest::SetUp();

    m_locater = TestRuleSetLocater::Create();
    m_connections = std::make_shared<TestConnectionManager>();

    RulesDrivenECPresentationManagerImpl::Params::CachingParams cachingParams;
    cachingParams.SetDisableDiskCache(true);
    RulesDrivenECPresentationManagerImpl::Params params(RulesEngineTestHelpers::GetPaths(BeTest::GetHost()));
    params.SetConnections(m_connections);
    params.SetCachingParams(cachingParams);
    params.SetCategorySupplier(&m_categorySupplier);
    params.SetLocalizationProvider(&m_localizationProvider);
    m_impl = new RulesDrivenECPresentationManagerImpl(params);

    m_impl->GetLocaters().RegisterLocater(*m_locater);

    m_impl->GetConnections().CreateConnection(s_project->GetECDb());
    m_connection = m_impl->GetConnections().GetConnection(s_project->GetECDb());

    m_impl->Initialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImplTests::TearDown()
    {
    m_connection = nullptr;
    DELETE_AND_CLEAR(m_impl);
    }

struct NeverCanceledToken : ICancelationToken
{
private:
    NeverCanceledToken() {}
protected:
    bool _IsCanceled() const override {return false;}
public:
    static RefCountedPtr<NeverCanceledToken> Create() {return new NeverCanceledToken();}
};

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplTests, LocatesChildNodeWhoseGrandParentIsHiddenAfterNodesCacheIsCleared)
    {
    // create a ruleset
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    ruleset->AddPresentationRule(*new RootNodeRule());
    ruleset->GetRootNodesRules().back()->AddSpecification(*new CustomNodeSpecification(1, false, "T_ROOT", "root", "descr", "imageid"));
    ruleset->GetRootNodesRules().back()->GetSpecifications().back()->SetHideNodesInHierarchy(true);
    ruleset->AddPresentationRule(*new ChildNodeRule("ParentNode.Type=\"T_ROOT\"", 1, false));
    ruleset->GetChildNodesRules().back()->AddSpecification(*new CustomNodeSpecification(1, false, "T_CHILD1", "child1", "descr", "imageid"));
    ruleset->GetChildNodesRules().back()->AddSpecification(*new CustomNodeSpecification(1, false, "T_CHILD2", "child2", "descr", "imageid"));
    ruleset->AddPresentationRule(*new ChildNodeRule("ParentNode.Type=\"T_CHILD2\"", 1, false));
    ruleset->GetChildNodesRules().back()->AddSpecification(*new CustomNodeSpecification(1, false, "T_CHILD2.1", "child2.1", "descr", "imageid"));
    m_locater->AddRuleSet(*ruleset);

    // request and verify
    auto cancelationToken = NeverCanceledToken::Create();
    RulesDrivenECPresentationManager::NavigationOptions options(ruleset->GetRuleSetId().c_str());
    INavNodesDataSourcePtr rootNodes = m_impl->GetRootNodes(*m_connection, PageOptions(), options, *cancelationToken);
    ASSERT_EQ(2, rootNodes->GetSize());
    EXPECT_STREQ("child1", rootNodes->Get(0)->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("child2", rootNodes->Get(1)->GetLabelDefinition().GetDisplayValue().c_str());
    INavNodesDataSourcePtr childNodes = m_impl->GetChildren(*m_connection, *rootNodes->Get(1), PageOptions(), options, *cancelationToken);
    ASSERT_EQ(1, childNodes->GetSize());
    EXPECT_STREQ("child2.1", childNodes->Get(0)->GetLabelDefinition().GetDisplayValue().c_str());

    // clear nodes cache and try to locate the node by its key
    NavNodeKeyCPtr key = childNodes->Get(0)->GetKey();
    m_impl->GetNodesCache(*m_connection)->Clear();
    NavNodeCPtr locatedNode = m_impl->GetNode(*m_connection, *key, options, *cancelationToken);
    ASSERT_TRUE(locatedNode.IsValid());
    EXPECT_STREQ("child2.1", locatedNode->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplTests, CreatesSeparateNodesCachesForEachConnection)
    {
    ECDbTestProject secondProject;
    secondProject.Create("RulesDrivenECPresentationManagerImplTests2");
    IConnectionPtr connection2 = m_impl->GetConnections().CreateConnection(secondProject.GetECDb());

    NodesCache* firstCache = m_impl->GetNodesCache(*m_connection);
    ASSERT_TRUE(nullptr != firstCache);
    NodesCache* secondCache = m_impl->GetNodesCache(*connection2);
    ASSERT_TRUE(nullptr != secondCache);

    EXPECT_NE(firstCache, secondCache);
    m_connections->NotifyConnectionClosed(*connection2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplTests, ClosesNodesCacheWhenConnectionsIsClosed)
    {
    ECDbTestProject secondProject;
    secondProject.Create("RulesDrivenECPresentationManagerImplTests2");
    IConnectionPtr connection = m_impl->GetConnections().CreateConnection(secondProject.GetECDb());

    NodesCache* nodesCache = m_impl->GetNodesCache(*connection);
    ASSERT_TRUE(nullptr != nodesCache);

    // simulate connection closing
    m_connections->NotifyConnectionClosed(*connection);
    nodesCache = m_impl->GetNodesCache(*connection);
    ASSERT_TRUE(nullptr == nodesCache);
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerImplRequestCancelationTests : RulesDrivenECPresentationManagerImplTests
    {
    PresentationRuleSetPtr m_ruleset;

    static PresentationRuleSetPtr CreateRuleSet()
        {
        PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("RulesDrivenECPresentationManagerImplTests", 1, 0, false, "", "", "", false);

        RootNodeRuleP rootNodeRule = new RootNodeRule("", 1, false);
        rootNodeRule->AddSpecification(*new CustomNodeSpecification(1, false, "root_type", "label", "descr", "imageid"));
        ruleset->AddPresentationRule(*rootNodeRule);

        ChildNodeRuleP childNodeRule = new ChildNodeRule("", 1, false);
        childNodeRule->AddSpecification(*new CustomNodeSpecification(1, false, "child_type", "label", "descr", "imageid"));
        ruleset->AddPresentationRule(*childNodeRule);

        ContentRuleP contentRule = new ContentRule("", 1, false);
        contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "ECDbMeta:ECClassDef", true));
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
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplRequestCancelationTests, AbortsRootNodesCountRequestWhenCanceledWhileLookingForRulesets)
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
    RulesDrivenECPresentationManager::NavigationOptions options(m_ruleset->GetRuleSetId().c_str());
    size_t count = m_impl->GetRootNodesCount(*m_connection, options, *token);
    EXPECT_EQ(0, count);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplRequestCancelationTests, AbortsRootNodesRequestWhenCanceledWhileLookingForRulesets)
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
    RulesDrivenECPresentationManager::NavigationOptions options(m_ruleset->GetRuleSetId().c_str());
    INavNodesDataSourceCPtr nodes = m_impl->GetRootNodes(*m_connection, PageOptions(), options, *token);
    EXPECT_EQ(0, nodes->GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplRequestCancelationTests, AbortsChildNodesCountRequestWhenCanceledWhileLookingForRulesets)
    {
    SimpleCancelationTokenPtr token = SimpleCancelationToken::Create();

    // get the parent node
    RulesDrivenECPresentationManager::NavigationOptions options(m_ruleset->GetRuleSetId().c_str());
    INavNodesDataSourceCPtr rootNodes = m_impl->GetRootNodes(*m_connection, PageOptions(), options, *token);
    NavNodeCPtr rootNode = rootNodes->Get(0);
    ASSERT_TRUE(rootNode.IsValid());

    // add a ruleset locater which cancels the request
    RefCountedPtr<TestCallbackRulesetLocater> locater = TestCallbackRulesetLocater::Create();
    m_impl->GetLocaters().RegisterLocater(*locater);
    locater->SetCallback([&]()
        {
        token->SetCanceled(true);
        });

    // request and verify
    size_t count = m_impl->GetChildrenCount(*m_connection, *rootNode, options, *token);
    EXPECT_EQ(0, count);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplRequestCancelationTests, AbortsChildNodesRequestWhenCanceledWhileLookingForRulesets)
    {
    SimpleCancelationTokenPtr token = SimpleCancelationToken::Create();

    // get the parent node
    RulesDrivenECPresentationManager::NavigationOptions options(m_ruleset->GetRuleSetId().c_str());
    INavNodesDataSourceCPtr rootNodes = m_impl->GetRootNodes(*m_connection, PageOptions(), options, *token);
    NavNodeCPtr rootNode = rootNodes->Get(0);
    ASSERT_TRUE(rootNode.IsValid());

    // add a ruleset locater which cancels the request
    RefCountedPtr<TestCallbackRulesetLocater> locater = TestCallbackRulesetLocater::Create();
    m_impl->GetLocaters().RegisterLocater(*locater);
    locater->SetCallback([&]()
        {
        token->SetCanceled(true);
        });

    // request and verify
    INavNodesDataSourceCPtr nodes = m_impl->GetChildren(*m_connection, *rootNode, PageOptions(), options, *token);
    EXPECT_EQ(0, nodes->GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
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
    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId().c_str());
    ECClassCP inputClass = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECClassDef");
    bvector<SelectClassInfo> contentClasses = m_impl->GetContentClasses(*m_connection, nullptr, 0, {inputClass}, options, *token);
    EXPECT_TRUE(contentClasses.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
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
    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_impl->GetContentDescriptor(*m_connection, nullptr, 0, *KeySet::Create(), nullptr, options, *token);
    EXPECT_TRUE(descriptor.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplRequestCancelationTests, AbortsContentRequestWhenCanceledWhileLookingForRulesets)
    {
    SimpleCancelationTokenPtr token = SimpleCancelationToken::Create();

    // get the descriptor
    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_impl->GetContentDescriptor(*m_connection, nullptr, 0, *KeySet::Create(), nullptr, options, *token);

    // add a ruleset locater which cancels the request
    RefCountedPtr<TestCallbackRulesetLocater> locater = TestCallbackRulesetLocater::Create();
    m_impl->GetLocaters().RegisterLocater(*locater);
    m_impl->GetLocaters().InvalidateCache();
    locater->SetCallback([&]()
        {
        token->SetCanceled(true);
        });

    // request and verify
    ContentCPtr content = m_impl->GetContent(*m_connection, *descriptor, PageOptions(), *token);
    EXPECT_TRUE(content.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplRequestCancelationTests, AbortsContentSetSizeRequestWhenCanceledWhileLookingForRulesets)
    {
    SimpleCancelationTokenPtr token = SimpleCancelationToken::Create();

    // get the descriptor
    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_impl->GetContentDescriptor(*m_connection, nullptr, 0, *KeySet::Create(), nullptr, options, *token);

    // add a ruleset locater which cancels the request
    RefCountedPtr<TestCallbackRulesetLocater> locater = TestCallbackRulesetLocater::Create();
    m_impl->GetLocaters().RegisterLocater(*locater);
    m_impl->GetLocaters().InvalidateCache();
    locater->SetCallback([&]()
        {
        token->SetCanceled(true);
        });

    // request and verify
    size_t size = m_impl->GetContentSetSize(*m_connection, *descriptor, *token);
    EXPECT_EQ(0, size);
    }
