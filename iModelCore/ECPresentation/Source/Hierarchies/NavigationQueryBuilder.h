/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/NavNode.h>
#include "../Shared/UsedClassesListener.h"
#include "../Shared/Queries/QueryBuilder.h"
#include "NavigationQuery.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define SEARCH_QUERY_FIELD_ECInstanceId "/ECInstanceId/"

#define NAVIGATION_QUERY_BUILDER_NAV_CLASS_ALIAS(rel) Utf8PrintfString("nav_%s_%s", (rel).GetSchema().GetAlias().c_str(), (rel).GetName().c_str())

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NavigationQueryBuilderParameters : QueryBuilderParameters
{
private:
    IHierarchyCacheCR m_nodesCache;
    IUsedClassesListener* m_usedClassesListener;
    bool m_useSpecificationIdentifier;
public:
    NavigationQueryBuilderParameters(ECSchemaHelper const& schemaHelper, IConnectionManagerCR connections, IConnectionCR connection, ICancelationTokenCP cancellationToken,
        IRulesPreprocessorR rulesPreprocessor, PresentationRuleSetCR ruleset, RulesetVariables const& rulesetVariables, IUsedRulesetVariablesListener* variablesListener, ECExpressionsCache& ecexpressionsCache,
        IHierarchyCacheCR nodesCache, IJsonLocalState const* localState = nullptr)
        : QueryBuilderParameters(schemaHelper, connections, connection, cancellationToken, rulesPreprocessor, ruleset, rulesetVariables, ecexpressionsCache, variablesListener, localState),
        m_nodesCache(nodesCache), m_usedClassesListener(nullptr), m_useSpecificationIdentifier(true)
        {}
    IHierarchyCacheCR GetNodesCache() const {return m_nodesCache;}
    void SetUsedClassesListener(IUsedClassesListener* listener) {m_usedClassesListener = listener;}
    IUsedClassesListener* GetUsedClassesListener() const {return m_usedClassesListener;}

    // note: this should only be used in query builder tests to avoid using specification identifiers in
    // query contracts to make produced queries easier to verify
    bool ShouldUseSpecificationIdentifierInContracts() const {return m_useSpecificationIdentifier;}
    void SetUseSpecificationIdentifierInContracts(bool value) {m_useSpecificationIdentifier = value;}
};

/*=================================================================================**//**
* Responsible for creating navigation queries based on presentation rules.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NavigationQueryBuilder
{
    struct SpecificationsVisitor;
    struct RelatedPathsCache;

private:
    NavigationQueryBuilderParameters m_params;
    RelatedPathsCache* m_relatedPathsCache;

private:
    bvector<NavigationQueryPtr> GetQueries(NavNodeCP parentNode, AllInstanceNodesSpecification const& specification, ChildNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(NavNodeCP parentNode, InstanceNodesOfSpecificClassesSpecification const& specification, ChildNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(NavNodeCP parentNode, RelatedInstanceNodesSpecification const& specification, Utf8StringCR specificationHash, ChildNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(NavNodeCP parentNode, SearchResultInstanceNodesSpecification const& specification, ChildNodeRuleCR rule) const;

public:
    ECPRESENTATION_EXPORT NavigationQueryBuilder(NavigationQueryBuilderParameters params);
    ECPRESENTATION_EXPORT ~NavigationQueryBuilder();
    NavigationQueryBuilderParameters const& GetParameters() const {return m_params;}
    NavigationQueryBuilderParameters& GetParameters() {return m_params;}
    ECPRESENTATION_EXPORT bvector<NavigationQueryPtr> GetQueries(RootNodeRuleCR, ChildNodeSpecificationCR) const;
    ECPRESENTATION_EXPORT bvector<NavigationQueryPtr> GetQueries(ChildNodeRuleCR, ChildNodeSpecificationCR, NavNodeCR) const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
