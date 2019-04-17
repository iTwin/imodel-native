/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "TestHelpers.h"
#include "TestRulesDrivenPresentationManagerImpl.h"
#include <UnitTests/BackDoor/ECPresentation/TestRuleSetLocater.h>
#include <UnitTests/BackDoor/ECPresentation/TestSelectionProvider.h>

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
    IConnectionPtr m_connection;
    TestConnectionManager m_connections;
    RulesDrivenECPresentationManagerImpl* m_impl;
    TestCategorySupplier m_categorySupplier;
    TestRuleSetLocaterPtr m_locater;
    ILocalizationProvider* m_localizationProvider;
    
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
    RulesDrivenECPresentationManagerImpl::Params params(m_connections, RulesEngineTestHelpers::GetPaths(BeTest::GetHost()));
    m_localizationProvider = new SQLangLocalizationProvider();
    params.SetDisableDiskCache(true);
    m_impl = new RulesDrivenECPresentationManagerImpl(RulesDrivenECPresentationManagerDependenciesFactory(), params);
    m_impl->SetCategorySupplier(&m_categorySupplier);
    m_impl->GetLocaters().RegisterLocater(*m_locater);
    //m_impl->SetLocalizationProvider(m_localizationProvider);
    m_connections.NotifyConnectionOpened(s_project->GetECDb());
    m_connection = m_connections.GetConnection(s_project->GetECDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImplTests::TearDown()
    {
    m_connection = nullptr;
    DELETE_AND_CLEAR(m_impl);
    DELETE_AND_CLEAR(m_localizationProvider);
    }

struct NeverCanceledToken : ICancelationToken
    {
    bool _IsCanceled() const override {return false;}
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
    ruleset->AddPresentationRule(*new ChildNodeRule("ParentNode.Type=\"T_ROOT\"", 1, false, TargetTree_Both));
    ruleset->GetChildNodesRules().back()->AddSpecification(*new CustomNodeSpecification(1, false, "T_CHILD1", "child1", "descr", "imageid"));
    ruleset->GetChildNodesRules().back()->AddSpecification(*new CustomNodeSpecification(1, false, "T_CHILD2", "child2", "descr", "imageid"));
    ruleset->AddPresentationRule(*new ChildNodeRule("ParentNode.Type=\"T_CHILD2\"", 1, false, TargetTree_Both));
    ruleset->GetChildNodesRules().back()->AddSpecification(*new CustomNodeSpecification(1, false, "T_CHILD2.1", "child2.1", "descr", "imageid"));
    m_locater->AddRuleSet(*ruleset);

    // request and verify
    NeverCanceledToken cancelationToken;
    RulesDrivenECPresentationManager::NavigationOptions options(ruleset->GetRuleSetId().c_str(), TargetTree_Both);
    INavNodesDataSourcePtr rootNodes = m_impl->GetRootNodes(*m_connection, PageOptions(), options, cancelationToken);
    ASSERT_EQ(2, rootNodes->GetSize());
    EXPECT_STREQ("child1", rootNodes->GetNode(0)->GetLabel().c_str());
    EXPECT_STREQ("child2", rootNodes->GetNode(1)->GetLabel().c_str());
    INavNodesDataSourcePtr childNodes = m_impl->GetChildren(*m_connection, *rootNodes->GetNode(1), PageOptions(), options, cancelationToken);
    ASSERT_EQ(1, childNodes->GetSize());
    EXPECT_STREQ("child2.1", childNodes->GetNode(0)->GetLabel().c_str());

    // clear nodes cache and try to locate the node by its key
    NavNodeKeyCPtr key = childNodes->GetNode(0)->GetKey();
    m_impl->GetNodesCache().Clear();
    NavNodeCPtr locatedNode = m_impl->GetNode(*m_connection, *key, options, cancelationToken);
    ASSERT_TRUE(locatedNode.IsValid());
    EXPECT_STREQ("child2.1", locatedNode->GetLabel().c_str());
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

        RootNodeRuleP rootNodeRule = new RootNodeRule("", 1, false, TargetTree_Both, false);
        rootNodeRule->AddSpecification(*new CustomNodeSpecification(1, false, "root_type", "label", "descr", "imageid"));
        ruleset->AddPresentationRule(*rootNodeRule);

        ChildNodeRuleP childNodeRule = new ChildNodeRule("", 1, false, TargetTree_Both);
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
        m_impl->SetLocalizationProvider(new SQLangLocalizationProvider());
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
    RulesDrivenECPresentationManager::NavigationOptions options(m_ruleset->GetRuleSetId().c_str(), TargetTree_Both);
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
    RulesDrivenECPresentationManager::NavigationOptions options(m_ruleset->GetRuleSetId().c_str(), TargetTree_Both);
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
    RulesDrivenECPresentationManager::NavigationOptions options(m_ruleset->GetRuleSetId().c_str(), TargetTree_Both);
    INavNodesDataSourceCPtr rootNodes = m_impl->GetRootNodes(*m_connection, PageOptions(), options, *token);
    NavNodeCPtr rootNode = rootNodes->GetNode(0);
    ASSERT_TRUE(rootNode.IsValid());

    // add a ruleset locater which cancels the request
    RefCountedPtr<TestCallbackRulesetLocater> locater = TestCallbackRulesetLocater::Create();
    m_impl->GetLocaters().RegisterLocater(*locater);
    m_impl->GetLocaters().InvalidateCache();
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
    RulesDrivenECPresentationManager::NavigationOptions options(m_ruleset->GetRuleSetId().c_str(), TargetTree_Both);
    INavNodesDataSourceCPtr rootNodes = m_impl->GetRootNodes(*m_connection, PageOptions(), options, *token);
    NavNodeCPtr rootNode = rootNodes->GetNode(0);
    ASSERT_TRUE(rootNode.IsValid());

    // add a ruleset locater which cancels the request
    RefCountedPtr<TestCallbackRulesetLocater> locater = TestCallbackRulesetLocater::Create();
    m_impl->GetLocaters().RegisterLocater(*locater);
    m_impl->GetLocaters().InvalidateCache();
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
    bvector<SelectClassInfo> contentClasses = m_impl->GetContentClasses(*m_connection, nullptr, {inputClass}, options, *token);
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
    ContentDescriptorCPtr descriptor = m_impl->GetContentDescriptor(*m_connection, nullptr, *KeySet::Create(), nullptr, options, *token);
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
    ContentDescriptorCPtr descriptor = m_impl->GetContentDescriptor(*m_connection, nullptr, *KeySet::Create(), nullptr, options, *token);
        
    // add a ruleset locater which cancels the request
    RefCountedPtr<TestCallbackRulesetLocater> locater = TestCallbackRulesetLocater::Create();
    m_impl->GetLocaters().RegisterLocater(*locater);
    m_impl->GetLocaters().InvalidateCache();
    locater->SetCallback([&]()
        {
        token->SetCanceled(true);
        });

    // request and verify
    ContentCPtr content = m_impl->GetContent(*descriptor, PageOptions(), *token);
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
    ContentDescriptorCPtr descriptor = m_impl->GetContentDescriptor(*m_connection, nullptr, *KeySet::Create(), nullptr, options, *token);
        
    // add a ruleset locater which cancels the request
    RefCountedPtr<TestCallbackRulesetLocater> locater = TestCallbackRulesetLocater::Create();
    m_impl->GetLocaters().RegisterLocater(*locater);
    m_impl->GetLocaters().InvalidateCache();
    locater->SetCallback([&]()
        {
        token->SetCanceled(true);
        });

    // request and verify
    size_t size = m_impl->GetContentSetSize(*descriptor, *token);
    EXPECT_EQ(0, size);
    }
