/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "../../../../Source/Hierarchies/NavNodeProviders.h"
#include "../../../../Source/Hierarchies/NavNodesCache.h"
#include "../../Helpers/ECDbTestProject.h"
#include "../../Helpers/TestNavNode.h"
#include "../../Helpers/TestHelpers.h"
#include "../../Helpers/TestNodesProvider.h"
#include "../../Helpers/TestNodesCache.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodesProviderTests : ECPresentationTest
    {
    static ECDbTestProject* s_project;
    IConnectionPtr m_connection;
    ECExpressionsCache m_expressionsCache;
    RelatedPathsCache m_relatedPathsCache;
    PresentationRuleSetPtr m_ruleset;
    TestConnectionManager m_connections;
    NavNodesProviderContextPtr m_context;
    TestNodesProviderContextFactory m_providerContextFactory;
    TestNodesProviderFactory m_providerFactory;
    TestUserSettings m_settings;
    std::shared_ptr<TestNodesCache> m_nodesCache;
    TestECDbUsedClassesListener m_usedClassesListener;
    bvector<std::unique_ptr<IProvidedNodesPostProcessor>> m_postProcessors;

    uint64_t m_providerIndex;

    NodesProviderTests()
        : m_providerContextFactory(m_connections), m_providerIndex(0)
        {
        m_postProcessors.push_back(std::make_unique<NodesFinalizingPostProcessor>());
        }

    static void SetUpTestCase();
    static void TearDownTestCase();

    virtual void SetUp() override;
    virtual void TearDown() override;

    bvector<IProvidedNodesPostProcessor const*> GetPostProcessors() const
        {
        return ContainerHelpers::TransformContainer<bvector<IProvidedNodesPostProcessor const*>>(m_postProcessors, [](auto const& ptr){return ptr.get();});
        }
    NavNodesProviderCPtr PostProcess(NavNodesProviderCR provider) const {return provider.PostProcess(GetPostProcessors());}
    NavNodesProviderPtr PostProcess(NavNodesProviderR provider) const {return provider.PostProcess(GetPostProcessors());}

    void Cache(NavNodeR node);

    PresentationQueryContractFieldPtr CreateDisplayLabelField(SelectClass<ECClass> const& selectClass, bvector<RelatedClassPath> const& relatedInstancePaths = {},
        bvector<InstanceLabelOverrideValueSpecification const*> const& instanceLabelOverrides = {})
        {
        return RulesEngineTestHelpers::CreateDisplayLabelField(ECSchemaHelper(*m_connection, &m_relatedPathsCache, &m_expressionsCache), selectClass,
            relatedInstancePaths, instanceLabelOverrides);
        }
    };
