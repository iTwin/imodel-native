/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../../../../Source/Hierarchies/NavNodesDataSource.h"
#include "../../Helpers/TestHelpers.h"
#include "../../Helpers/TestNavNode.h"
#include "../../Helpers/TestNodesProvider.h"
#include "../../Helpers/TestNodesCache.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

static NavNodesFactory s_nodesFactory;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NavNodesDataSourceTests : ECPresentationTest, IECExpressionsCacheProvider
    {
    PresentationRuleSetPtr m_ruleset;
    NavNodesProviderContextPtr m_context;
    TestNodesProviderFactory m_providerFactory;
    RefCountedPtr<TestNodesProvider> m_provider;
    NavNodesDataSourcePtr m_source;
    TestConnectionManager m_connections;
    std::shared_ptr<TestNodesCache> m_nodesCache;
    ECExpressionsCache m_expressionsCache;
    RelatedPathsCache m_relatedPathsCache;

    NavNodesDataSourceTests() {}

    void SetUp() override
        {
        ECPresentationTest::SetUp();
        m_ruleset = PresentationRuleSet::CreateInstance("NavNodesDataSourceTests");
        m_nodesCache = std::make_shared<TestNodesCache>();
        m_context = NavNodesProviderContext::Create(*m_ruleset, TargetTree_Both, nullptr,
            std::make_unique<RulesetVariables>(), m_expressionsCache, m_relatedPathsCache,
            s_nodesFactory, m_nodesCache, m_providerFactory, nullptr);
        m_provider = TestNodesProvider::Create(*m_context);
        m_source = ProviderBasedNodesDataSource::Create(*m_provider);
        }
    virtual ECExpressionsCache& _Get(Utf8CP) override {return m_expressionsCache;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavNodesDataSourceTests, IsEmptyWhenProviderReturnsNothing)
    {
    ASSERT_TRUE(0 == m_source->GetSize());
    ASSERT_TRUE(m_source->Get(0).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavNodesDataSourceTests, ReturnsNodesCountFromProvider)
    {
    m_provider->SetGetNodesCountHandler([](){return CountInfo(2, true);});
    ASSERT_TRUE(2 == m_source->GetSize());
    }
