/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/NavNodesDataSourceTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include "../../../Source/RulesDriven/RulesEngine/NavNodesDataSource.h"
#include "TestNodesProvider.h"
#include "TestHelpers.h"
#include "TestNavNode.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

static JsonNavNodesFactory s_nodesFactory;

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2015
+===============+===============+===============+===============+===============+======*/
struct NavNodesDataSourceTests : ECPresentationTest, IECExpressionsCacheProvider
    {
    PresentationRuleSetPtr m_ruleset;
    NavNodesProviderContextPtr m_context;
    TestNodesProviderFactory m_providerFactory;
    RefCountedPtr<TestNodesProvider> m_provider;
    NavNodesDataSourcePtr m_source;
    TestUserSettings m_settings;
    TestConnectionManager m_connections;
    TestNodesCache m_nodesCache;
    ECExpressionsCache m_expressionsCache;
    RelatedPathsCache m_relatedPathsCache;
    PolymorphicallyRelatedClassesCache m_polymorphicallyRelatedClassesCache;
    
    NavNodesDataSourceTests() : m_nodesCache(m_connections) {}

    void SetUp() override
        {
        ECPresentationTest::SetUp();
        m_ruleset = PresentationRuleSet::CreateInstance("NavNodesDataSourceTests", 1, 0, false, "", "", "", false);
        m_context = NavNodesProviderContext::Create(*m_ruleset, true, TargetTree_Both, "locale", 0, 
            m_settings, m_expressionsCache, m_relatedPathsCache, m_polymorphicallyRelatedClassesCache, 
            s_nodesFactory, m_nodesCache, m_providerFactory, nullptr);
        m_provider = TestNodesProvider::Create(*m_context);
        m_source = NavNodesDataSource::Create(*m_provider);
        }
    virtual ECExpressionsCache& _Get(Utf8CP) override {return m_expressionsCache;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavNodesDataSourceTests, IsEmptyWhenProviderReturnsNothing)
    {
    ASSERT_TRUE(0 == m_source->GetSize());
    ASSERT_TRUE(m_source->Get(0).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavNodesDataSourceTests, ReturnsNodesCountFromProvider)
    {
    m_provider->SetGetNodesCountHandler([](){return 2;});
    ASSERT_TRUE(2 == m_source->GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavNodesDataSourceTests, ReturnsNodesFromProvider)
    {
    m_provider->SetGetNodeHandler([](JsonNavNodePtr& node, size_t index)
        {
        node = s_nodesFactory.CreateCustomNode("some connection id", "locale", "label", "description", "imageId", "type");
        return true;
        });
    ASSERT_TRUE(m_source->Get(0).IsValid());
    }
