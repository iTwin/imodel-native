/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/PresentationManagerImplTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TestHelpers.h"
#include "TestRulesDrivenPresentationManagerImpl.h"
#include <UnitTests/BackDoor/ECPresentation/TestRulesetLocater.h>
#include <UnitTests/BackDoor/ECPresentation/TestSelectionProvider.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerImplTests : ::testing::Test
    {
    static ECDbTestProject* s_project;
    IConnectionPtr m_connection;
    TestConnectionManager m_connections;
    RulesDrivenECPresentationManagerImpl* m_impl;
    PresentationRuleSetPtr m_ruleset;
    TestCategorySupplier m_categorySupplier;
    
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;

    RulesDrivenECPresentationManagerImplTests()
        : m_categorySupplier(ContentDescriptor::Category("cat", "cat", "descr", 1))
        {}

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
    };
ECDbTestProject* RulesDrivenECPresentationManagerImplTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImplTests::SetUpTestCase()
    {
    s_project = new ECDbTestProject();
    s_project->Create("RulesDrivenECPresentationManagerImplTests");
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImplTests::TearDownTestCase()
    {
    DELETE_AND_CLEAR(s_project);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImplTests::SetUp()
    {
    m_impl = new RulesDrivenECPresentationManagerImpl(RulesDrivenECPresentationManagerDependenciesFactory(), m_connections,
        RulesEngineTestHelpers::GetPaths(BeTest::GetHost()), true);
    m_impl->SetCategorySupplier(&m_categorySupplier);
    m_connections.NotifyConnectionOpened(s_project->GetECDb());
    m_connection = m_connections.GetConnection(s_project->GetECDb());
    m_ruleset = CreateRuleSet();

    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    m_impl->GetLocaters().RegisterLocater(*locater);
    locater->AddRuleSet(*m_ruleset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImplTests::TearDown()
    {
    m_connection = nullptr;
    DELETE_AND_CLEAR(m_impl);
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerImplRequestCancelationTests : RulesDrivenECPresentationManagerImplTests
    {
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct CallbackRulesetLocater : RefCounted<RuleSetLocater>
{
private:
    std::function<void()> m_callback;
    void Callback() const {if (m_callback) {m_callback();}}
protected:
    bvector<PresentationRuleSetPtr> _LocateRuleSets(Utf8CP rulesetId) const override {Callback(); return bvector<PresentationRuleSetPtr>();}
    bvector<Utf8String> _GetRuleSetIds() const override {Callback(); return bvector<Utf8String>();}
    int _GetPriority() const override {Callback(); return 1;}
    void _InvalidateCache(Utf8CP rulesetId) override {Callback();}
public:
    static RefCountedPtr<CallbackRulesetLocater> Create() {return new CallbackRulesetLocater();}
    void SetCallback(std::function<void()> callback) {m_callback = callback;}
};

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerImplRequestCancelationTests, AbortsRootNodesCountRequestWhenCanceledWhileLookingForRulesets)
    {
    // add a ruleset locater which cancels the request
    SimpleCancelationTokenPtr token = SimpleCancelationToken::Create();
    RefCountedPtr<CallbackRulesetLocater> locater = CallbackRulesetLocater::Create();
    m_impl->GetLocaters().RegisterLocater(*locater);
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
    RefCountedPtr<CallbackRulesetLocater> locater = CallbackRulesetLocater::Create();
    m_impl->GetLocaters().RegisterLocater(*locater);
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
    RefCountedPtr<CallbackRulesetLocater> locater = CallbackRulesetLocater::Create();
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
    RefCountedPtr<CallbackRulesetLocater> locater = CallbackRulesetLocater::Create();
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
    RefCountedPtr<CallbackRulesetLocater> locater = CallbackRulesetLocater::Create();
    m_impl->GetLocaters().RegisterLocater(*locater);
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
    RefCountedPtr<CallbackRulesetLocater> locater = CallbackRulesetLocater::Create();
    m_impl->GetLocaters().RegisterLocater(*locater);
    locater->SetCallback([&]()
        {
        token->SetCanceled(true);
        });

    // request and verify
    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId().c_str());
    SelectionInfo selection("selection provider", false, *NavNodeKeyListContainer::Create());
    ContentDescriptorCPtr descriptor = m_impl->GetContentDescriptor(*m_connection, nullptr, selection, options, *token);
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
    SelectionInfo selection("selection provider", false, *NavNodeKeyListContainer::Create());
    ContentDescriptorCPtr descriptor = m_impl->GetContentDescriptor(*m_connection, nullptr, selection, options, *token);
        
    // add a ruleset locater which cancels the request
    RefCountedPtr<CallbackRulesetLocater> locater = CallbackRulesetLocater::Create();
    m_impl->GetLocaters().RegisterLocater(*locater);
    m_impl->GetLocaters().InvalidateCache();
    locater->SetCallback([&]()
        {
        token->SetCanceled(true);
        });

    // request and verify
    ContentCPtr content = m_impl->GetContent(*m_connection, *descriptor, selection, PageOptions(), options, *token);
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
    SelectionInfo selection("selection provider", false, *NavNodeKeyListContainer::Create());
    ContentDescriptorCPtr descriptor = m_impl->GetContentDescriptor(*m_connection, nullptr, selection, options, *token);
        
    // add a ruleset locater which cancels the request
    RefCountedPtr<CallbackRulesetLocater> locater = CallbackRulesetLocater::Create();
    m_impl->GetLocaters().RegisterLocater(*locater);
    m_impl->GetLocaters().InvalidateCache();
    locater->SetCallback([&]()
        {
        token->SetCanceled(true);
        });

    // request and verify
    size_t size = m_impl->GetContentSetSize(*m_connection, *descriptor, selection, options, *token);
    EXPECT_EQ(0, size);
    }