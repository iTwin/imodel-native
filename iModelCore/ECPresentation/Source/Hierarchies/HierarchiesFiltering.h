/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentationManager.h>
#include "../Shared/ECSchemaHelper.h"
#include "../RulesEngineTypes.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TraverseHierarchyRulesProps
{
private:
    IHierarchyCacheCR m_nodesCache;
    NavNodesFactoryCR m_nodesFactory;
    IRulesPreprocessorR m_rulesPreprocessor;
    PresentationRuleSetCR m_ruleset;
    ECSchemaHelper const& m_schemaHelper;
public:
    TraverseHierarchyRulesProps(IHierarchyCacheCR nodesCache, NavNodesFactoryCR nodesFactory, IRulesPreprocessorR rulesPreprocessor, PresentationRuleSetCR ruleset, ECSchemaHelper const& schemaHelper)
        : m_nodesCache(nodesCache), m_nodesFactory(nodesFactory), m_rulesPreprocessor(rulesPreprocessor), m_ruleset(ruleset), m_schemaHelper(schemaHelper)
        {}
    IHierarchyCacheCR GetNodesCache() const {return m_nodesCache;}
    NavNodesFactoryCR GetNodesFactory() const {return m_nodesFactory;}
    IRulesPreprocessorR GetRulesPreprocessor() const {return m_rulesPreprocessor;}
    PresentationRuleSetCR GetRuleset() const {return m_ruleset;}
    ECSchemaHelper const& GetSchemaHelper() const {return m_schemaHelper;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct HierarchiesFilteringHelper
    {
    //! Does given node supports filtering its child hierarchy level.
    static bool SupportsFiltering(NavNodeCP, TraverseHierarchyRulesProps const&, bvector<Utf8String>* reasons);

    //! Does given hierarchy specification supports filtering.
    static bool SupportsFiltering(ChildNodeSpecificationCR, bvector<Utf8String>* reasons);

    //! Translate child hierarchy rules into a ruleset with content rules for creating a hierarchy level descriptor.
    static PresentationRuleSetPtr CreateHierarchyLevelDescriptorRuleset(NavNodeCP, TraverseHierarchyRulesProps const&);
    };

#define ENSURE_SUPPORTS_FILTERING(spec) \
    { \
    bvector<Utf8String> reasons; \
    if (!HierarchiesFilteringHelper::SupportsFiltering(spec, &reasons)) \
        throw FilteringNotSupportedException(reasons); \
    }

END_BENTLEY_ECPRESENTATION_NAMESPACE
