/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <UnitTests/ECPresentation/ECPresentationTest.h>
#include "../../../Source/Hierarchies/NavNodeProviders.h"

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TestNodesProvider : NavNodesProvider
{
    typedef std::function<bool()> HasNodesHandler;
    typedef std::function<CountInfo()> GetNodesCountHandler;

private:
    HasNodesHandler m_hasNodesHandler;
    GetNodesCountHandler m_getNodesCountHandler;

private:
    TestNodesProvider(NavNodesProviderContextR context) : NavNodesProvider(context) {}

protected:
    Utf8CP _GetName() const override {return "Test provider";}
    bool _HasNodes() const override {return m_hasNodesHandler ? m_hasNodesHandler() : false;}
    CountInfo _GetTotalNodesCount() const override {return m_getNodesCountHandler ? m_getNodesCountHandler() : CountInfo(0, true);}
    Iterator _CreateFrontIterator() const override {return Iterator(std::make_unique<EmptyIteratorImpl<NavNodePtr>>());}
    Iterator _CreateBackIterator() const override {return Iterator(std::make_unique<EmptyIteratorImpl<NavNodePtr>>());}

public:
    static RefCountedPtr<TestNodesProvider> Create(NavNodesProviderContextR context) {return new TestNodesProvider(context);}
    void SetHasNodesHandler(HasNodesHandler const& handler) {m_hasNodesHandler = handler;}
    void SetGetNodesCountHandler(GetNodesCountHandler const& handler) {m_getNodesCountHandler = handler;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TestNodesProviderFactory : INodesProviderFactory
{
private:
    std::vector<std::shared_ptr<IProvidedNodesPostProcessor>> m_postProcessors;
private:
    NavNodesProviderPtr CreateProvider(NavNodesProviderContextR context) const
        {
        NavNodesProviderPtr provider;
        NavNodeCPtr parent = context.GetVirtualParentNode();
        if (parent.IsNull())
            {
            IRulesPreprocessor::RootNodeRuleParameters params(TargetTree_MainTree);
            RootNodeRuleSpecificationsList specs = context.GetRulesPreprocessor().GetRootNodeSpecifications(params);
            provider = MultiSpecificationNodesProvider::Create(context, specs);
            }
        else
            {
            IRulesPreprocessor::ChildNodeRuleParameters params(*parent, TargetTree_MainTree);
            ChildNodeRuleSpecificationsList specs = context.GetRulesPreprocessor().GetChildNodeSpecifications(params);
            provider = MultiSpecificationNodesProvider::Create(context, specs, *parent);
            }
        return provider;
        }

protected:
    bvector<IProvidedNodesPostProcessor const*> _GetPostProcessors() const override
        {
        return ContainerHelpers::TransformContainer<bvector<IProvidedNodesPostProcessor const*>>(m_postProcessors, [](auto const& ptr){return ptr.get();});
        }
    NavNodesProviderPtr _Create(NavNodesProviderContextR context) const override
        {
        return CreateProvider(context);
        }
public:
    TestNodesProviderFactory() {}
    void SetPostProcessors(bvector<std::shared_ptr<IProvidedNodesPostProcessor>> postProcessors) {m_postProcessors = postProcessors;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TestNodesProviderContextFactory : INodesProviderContextFactory
{
private:
    IConnectionManagerCR m_connections;
    NavNodesFactory m_nodesFactory;
    TestNodesProviderFactory m_providerFactory;
    PresentationRuleSetCPtr m_ruleset;
    IECDbUsedClassesListener* m_usedClassesListener;
    mutable ECExpressionsCache m_ecexpressionsCache;
    mutable RelatedPathsCache m_relatedPathsCache;
    mutable CustomFunctionsInjector m_customFunctions;
    std::shared_ptr<INavNodesCache> m_nodesCache;

protected:
    NavNodesProviderContextPtr _Create(IConnectionCR connection, Utf8CP rulesetId,
        NavNodeCP parentNode, std::shared_ptr<INavNodesCache> cache, ICancelationTokenCP cancelationToken, RulesetVariables const& variables) const override
        {
        PresentationRuleSetCPtr ruleset = m_ruleset;
        if (ruleset.IsNull())
            ruleset = PresentationRuleSet::CreateInstance(rulesetId);
        NavNodesProviderContextPtr context = NavNodesProviderContext::Create(*ruleset, TargetTree_MainTree, parentNode,
            std::make_unique<RulesetVariables>(variables), m_ecexpressionsCache, m_relatedPathsCache, m_nodesFactory,
            nullptr == m_nodesCache ? cache : m_nodesCache, m_providerFactory, nullptr);
        context->SetQueryContext(m_connections, connection, m_usedClassesListener);
        context->SetCancelationToken(cancelationToken);
        return context;
        }

public:
    TestNodesProviderContextFactory(IConnectionManagerCR connections)
        : m_connections(connections), m_nodesCache(nullptr), m_customFunctions(connections), m_usedClassesListener(nullptr)
        {}
    void SetNodesCache(std::shared_ptr<INavNodesCache> cache) {m_nodesCache = cache;}
    void SetRuleset(PresentationRuleSetCP ruleset) {m_ruleset = ruleset;}
    void SetUsedClassesListener(IECDbUsedClassesListener* listener) {m_usedClassesListener = listener;}
};

END_ECPRESENTATIONTESTS_NAMESPACE
