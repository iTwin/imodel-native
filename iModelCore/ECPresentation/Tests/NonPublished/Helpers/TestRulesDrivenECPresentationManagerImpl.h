/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <UnitTests/ECPresentation/ECPresentationTest.h>
#include "RulesDrivenECPresentationManagerImplBase.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TestRulesDrivenECPresentationManagerImpl : RulesDrivenECPresentationManagerImplBase
{
    typedef std::function<INavNodesDataSourcePtr(WithPageOptions<HierarchyRequestImplParams> const&)> Handler_GetNodes;
    typedef std::function<size_t(HierarchyRequestImplParams const&)> Handler_GetNodesCount;
    typedef std::function<NavNodeCPtr(NodeParentRequestImplParams const&)> Handler_GetParent;
    typedef std::function<bvector<NavNodeCPtr>(NodePathsFromFilterTextRequestImplParams const&)> Handler_GetFilteredNodes;
    typedef std::function<HierarchyComparePositionPtr(HierarchyCompareRequestImplParams const&)> Handler_CompareHierarchies;

    typedef std::function<bvector<SelectClassInfo>(ContentClassesRequestImplParams const&)> Handler_GetContentClasses;
    typedef std::function<ContentDescriptorCPtr(ContentDescriptorRequestImplParams const&)> Handler_GetContentDescriptor;
    typedef std::function<ContentCPtr(WithPageOptions<ContentRequestImplParams> const&)> Handler_GetContent;
    typedef std::function<size_t(ContentRequestImplParams const&)> Handler_GetContentSetSize;
    typedef std::function<LabelDefinitionCPtr(KeySetDisplayLabelRequestImplParams const&)> Handler_GetDisplayLabel;
    typedef std::function<PagingDataSourcePtr<DisplayValueGroupCPtr>(WithPageOptions<DistinctValuesRequestImplParams> const&)> Handler_GetDistinctValues;

    typedef std::function<void()> Handler_OnUpdateRecordsHandlerChanged;
    typedef std::function<void(ECInstanceChangeEventSource&)> Handler_OnECInstanceChangeEventSourceRegistered;
    typedef std::function<void(ECInstanceChangeEventSource&)> Handler_OnECInstanceChangeEventSourceUnregister;

private:
    Handler_GetNodes m_nodesHandler;
    Handler_GetNodesCount m_nodesCountHandler;
    Handler_GetParent m_getParentHandler;
    Handler_GetFilteredNodes m_filteredNodesHandler;

    Handler_GetContentClasses m_contentClassesHandler;
    Handler_GetContentDescriptor m_contentDescriptorHandler;
    Handler_GetContent m_contentHandler;
    Handler_GetContentSetSize m_contentSetSizeHandler;
    Handler_GetDisplayLabel m_displayLabelHandler;
    Handler_GetDistinctValues m_distinctValuesHandler;

    Handler_CompareHierarchies m_compareHierarchiesHandler;

protected:
    std::unique_ptr<INodeInstanceKeysProvider> _CreateNodeInstanceKeysProvider(NodeInstanceKeysRequestImplParams const&) const override
        {
        return nullptr;
        }
    INavNodesDataSourcePtr _GetNodes(WithPageOptions<HierarchyRequestImplParams> const& params) override
        {
        if (m_nodesHandler)
            return m_nodesHandler(params);
        return nullptr;
        }
    size_t _GetNodesCount(HierarchyRequestImplParams const& params) override
        {
        if (m_nodesCountHandler)
            return m_nodesCountHandler(params);
        return 0;
        }
    NavNodeCPtr _GetParent(NodeParentRequestImplParams const& params) override
        {
        if (m_getParentHandler)
            return m_getParentHandler(params);
        return nullptr;
        }
    bvector<NavNodeCPtr> _GetFilteredNodes(NodePathsFromFilterTextRequestImplParams const& params) override
        {
        if (m_filteredNodesHandler)
            return m_filteredNodesHandler(params);
        return bvector<NavNodeCPtr>();
        }
    HierarchyComparePositionPtr _CompareHierarchies(HierarchyCompareRequestImplParams const& params) override
        {
        if (m_compareHierarchiesHandler)
            return m_compareHierarchiesHandler(params);
        return nullptr;
        }

    bvector<SelectClassInfo> _GetContentClasses(ContentClassesRequestImplParams const& params) override
        {
        if (m_contentClassesHandler)
            return m_contentClassesHandler(params);
        return bvector<SelectClassInfo>();
        }
    ContentDescriptorCPtr _GetContentDescriptor(ContentDescriptorRequestImplParams const& params) override
        {
        if (m_contentDescriptorHandler)
            return m_contentDescriptorHandler(params);
        return nullptr;
        }
    ContentCPtr _GetContent(WithPageOptions<ContentRequestImplParams> const& params) override
        {
        if (m_contentHandler)
            return m_contentHandler(params);
        return nullptr;
        }
    size_t _GetContentSetSize(ContentRequestImplParams const& params) override
        {
        if (m_contentSetSizeHandler)
            return m_contentSetSizeHandler(params);
        return 0;
        }
    LabelDefinitionCPtr _GetDisplayLabel(KeySetDisplayLabelRequestImplParams const& params) override
        {
        if (m_displayLabelHandler)
            return m_displayLabelHandler(params);
        return LabelDefinition::Create();
        }
    PagingDataSourcePtr<DisplayValueGroupCPtr> _GetDistinctValues(WithPageOptions<DistinctValuesRequestImplParams> const& params) override
        {
        if (m_distinctValuesHandler)
            return m_distinctValuesHandler(params);
        return PagingDataSource<DisplayValueGroupCPtr>::Create();
        }

public:
    TestRulesDrivenECPresentationManagerImpl(ECPresentationManager::Impl::Params const& params)
        : RulesDrivenECPresentationManagerImplBase(params)
        {}

    void SetNodesHandler(Handler_GetNodes handler) {m_nodesHandler = handler;}
    void SetNodesCountHandler(Handler_GetNodesCount handler) {m_nodesCountHandler = handler;}
    void SetGetParentHandler(Handler_GetParent handler) {m_getParentHandler = handler;}
    void SetGetFilteredNodesHandler(Handler_GetFilteredNodes handler) {m_filteredNodesHandler = handler;}

    void SetContentClassesHandler(Handler_GetContentClasses handler) {m_contentClassesHandler = handler;}
    void SetContentDescriptorHandler(Handler_GetContentDescriptor handler) {m_contentDescriptorHandler = handler;}
    void SetContentHandler(Handler_GetContent handler) {m_contentHandler = handler;}
    void SetContentSetSizeHandler(Handler_GetContentSetSize handler) {m_contentSetSizeHandler = handler;}
    void SetDisplayLabelHandler(Handler_GetDisplayLabel handler) {m_displayLabelHandler = handler;}
    void SetDistinctValuesHandler(Handler_GetDistinctValues handler) {m_distinctValuesHandler = handler;}

    void SetCompareHierarchiesHandler(Handler_CompareHierarchies handler) {m_compareHierarchiesHandler = handler;}
};

END_ECPRESENTATIONTESTS_NAMESPACE
