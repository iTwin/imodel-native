/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/ECPresentationTypes.h>
#include <ECPresentation/RulesetVariables.h>
#include <ECPresentation/Update.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC

//=======================================================================================
// @bsiclass
//=======================================================================================
template<typename TBase>
struct WithPageOptions : TBase
{
private:
    PageOptions m_pageOptions;
public:
    WithPageOptions(TBase const& source, PageOptions pageOptions = PageOptions()) : TBase(source), m_pageOptions(pageOptions) {}
    WithPageOptions(TBase&& source, PageOptions pageOptions = PageOptions()) : TBase(std::move(source)), m_pageOptions(pageOptions) {}
    PageOptionsCR GetPageOptions() const {return m_pageOptions;}
    void SetPageOptions(PageOptions value) {m_pageOptions = value;}
};
template<typename TParams> auto MakePaged(TParams params, PageOptions pageOpts) {return WithPageOptions<TParams>(params, pageOpts);}

//=======================================================================================
// @bsiclass
//=======================================================================================
struct BaseRequestParams
{
private:
    Utf8String m_locale;
    UnitSystem m_unitSystem;
public:
    BaseRequestParams() : m_unitSystem(ECPresentation::UnitSystem::Undefined) {}
    Utf8StringCR GetLocale() const {return m_locale;}
    void SetLocale(Utf8String value) {m_locale = value;}
    UnitSystem GetUnitSystem() const {return m_unitSystem;}
    void SetUnitSystem(UnitSystem value) {m_unitSystem = value;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct RequestWithRulesetParams : BaseRequestParams
{
private:
    Utf8String m_rulesetId;
    RulesetVariables m_rulesetVariables;
public:
    RequestWithRulesetParams(BaseRequestParams const& baseParams, Utf8String rulesetId, RulesetVariables rulesetVariables)
        : BaseRequestParams(baseParams), m_rulesetId(rulesetId), m_rulesetVariables(rulesetVariables)
        {}
    RequestWithRulesetParams(BaseRequestParams&& baseParams, Utf8String rulesetId, RulesetVariables rulesetVariables)
        : BaseRequestParams(std::move(baseParams)), m_rulesetId(rulesetId), m_rulesetVariables(rulesetVariables)
        {}
    RequestWithRulesetParams(Utf8String rulesetId, RulesetVariables rulesetVariables)
        : m_rulesetId(rulesetId), m_rulesetVariables(rulesetVariables)
        {}
    Utf8StringCR GetRulesetId() const {return m_rulesetId;}
    RulesetVariables const& GetRulesetVariables() const {return m_rulesetVariables;}
};

//=======================================================================================
//! Params for a hierarchy level request. When requesting child hierarchy level, either
//! parent node or its key may be provided.
// @bsiclass
//=======================================================================================
struct HierarchyRequestParams : RequestWithRulesetParams
{
private:
    NavNodeKeyCPtr m_parentNodeKey;
    NavNodeCPtr m_parentNode;
    std::shared_ptr<InstanceFilterDefinition const> m_instanceFilter;
public:
    //! Root nodes request
    HierarchyRequestParams(RequestWithRulesetParams const& rulesetParams)
        : RequestWithRulesetParams(rulesetParams)
        {}
    HierarchyRequestParams(RequestWithRulesetParams&& rulesetParams)
        : RequestWithRulesetParams(std::move(rulesetParams))
        {}
    HierarchyRequestParams(Utf8String rulesetId, RulesetVariables rulesetVariables)
        : RequestWithRulesetParams(rulesetId, rulesetVariables)
        {}
    //! Child nodes request with parent node
    HierarchyRequestParams(RequestWithRulesetParams const& rulesetParams, NavNodeCP parentNode)
        : RequestWithRulesetParams(rulesetParams), m_parentNode(parentNode)
        {}
    HierarchyRequestParams(RequestWithRulesetParams&& rulesetParams, NavNodeCP parentNode)
        : RequestWithRulesetParams(std::move(rulesetParams)), m_parentNode(parentNode)
        {}
    HierarchyRequestParams(Utf8String rulesetId, RulesetVariables rulesetVariables, NavNodeCP parentNode)
        : RequestWithRulesetParams(rulesetId, rulesetVariables), m_parentNode(parentNode)
        {}
    //! Child nodes request with parent node key
    HierarchyRequestParams(RequestWithRulesetParams const& rulesetParams, NavNodeKeyCP parentNodeKey)
        : RequestWithRulesetParams(rulesetParams), m_parentNodeKey(parentNodeKey)
        {}
    HierarchyRequestParams(RequestWithRulesetParams&& rulesetParams, NavNodeKeyCP parentNodeKey)
        : RequestWithRulesetParams(std::move(rulesetParams)), m_parentNodeKey(parentNodeKey)
        {}
    HierarchyRequestParams(Utf8String rulesetId, RulesetVariables rulesetVariables, NavNodeKeyCP parentNodeKey)
        : RequestWithRulesetParams(rulesetId, rulesetVariables), m_parentNodeKey(parentNodeKey)
        {}
    NavNodeCP GetParentNode() const {return m_parentNode.get();}
    void SetParentNode(NavNodeCP value) {m_parentNode = value; m_parentNodeKey = nullptr;}
    NavNodeKeyCP GetParentNodeKey() const {return m_parentNodeKey.get();}
    void SetParentNodeKey(NavNodeKeyCP value) {m_parentNodeKey = value; m_parentNode = nullptr;}
    std::shared_ptr<InstanceFilterDefinition const> GetInstanceFilter() const {return m_instanceFilter;}
    void SetInstanceFilter(std::shared_ptr<InstanceFilterDefinition const> value) {m_instanceFilter = value;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct NodeByInstanceKeyRequestParams : HierarchyRequestParams
{
private:
    ECInstanceKey m_key;
public:
    NodeByInstanceKeyRequestParams(HierarchyRequestParams const& hierarchyParams, ECInstanceKey instanceKey)
        : HierarchyRequestParams(hierarchyParams), m_key(instanceKey)
        {}
    NodeByInstanceKeyRequestParams(HierarchyRequestParams&& hierarchyParams, ECInstanceKey instanceKey)
        : HierarchyRequestParams(std::move(hierarchyParams)), m_key(instanceKey)
        {}
    NodeByInstanceKeyRequestParams(Utf8String rulesetId, RulesetVariables rulesetVariables, NavNodeCP parentNode, ECInstanceKey instanceKey)
        : HierarchyRequestParams(rulesetId, rulesetVariables, parentNode), m_key(instanceKey)
        {}
    ECInstanceKeyCR GetInstanceKey() const {return m_key;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct NodeParentRequestParams : RequestWithRulesetParams
{
private:
    NavNodeCPtr m_node;
    std::shared_ptr<InstanceFilterDefinition const> m_instanceFilter;
public:
    NodeParentRequestParams(RequestWithRulesetParams const& rulesetParams, NavNodeCR node)
        : RequestWithRulesetParams(rulesetParams), m_node(&node)
        {}
    NodeParentRequestParams(RequestWithRulesetParams&& rulesetParams, NavNodeCR node)
        : RequestWithRulesetParams(std::move(rulesetParams)), m_node(&node)
        {}
    NodeParentRequestParams(Utf8String rulesetId, RulesetVariables rulesetVariables, NavNodeCR node)
        : RequestWithRulesetParams(rulesetId, rulesetVariables), m_node(&node)
        {}
    NavNodeCR GetNode() const {return *m_node;}
    std::shared_ptr<InstanceFilterDefinition const> GetInstanceFilter() const {return m_instanceFilter;}
    void SetInstanceFilter(std::shared_ptr<InstanceFilterDefinition const> value) {m_instanceFilter = value;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct NodePathFromInstanceKeyPathRequestParams : HierarchyRequestParams
{
private:
    bvector<ECInstanceKey> m_instanceKeyPath;
public:
    NodePathFromInstanceKeyPathRequestParams(HierarchyRequestParams const& hierarchyParams, bvector<ECInstanceKey> instanceKeyPath)
        : HierarchyRequestParams(hierarchyParams), m_instanceKeyPath(instanceKeyPath)
        {}
    NodePathFromInstanceKeyPathRequestParams(HierarchyRequestParams&& hierarchyParams, bvector<ECInstanceKey> instanceKeyPath)
        : HierarchyRequestParams(std::move(hierarchyParams)), m_instanceKeyPath(instanceKeyPath)
        {}
    NodePathFromInstanceKeyPathRequestParams(Utf8String rulesetId, RulesetVariables rulesetVariables, NavNodeCP parentNode, bvector<ECInstanceKey> instanceKeyPath)
        : HierarchyRequestParams(rulesetId, rulesetVariables, parentNode), m_instanceKeyPath(instanceKeyPath)
        {}
    bvector<ECInstanceKey> const& GetInstanceKeyPath() const {return m_instanceKeyPath;}
    bvector<ECInstanceKey>& GetInstanceKeyPath() {return m_instanceKeyPath;}
    void SetInstanceKeyPath(bvector<ECInstanceKey> value) {m_instanceKeyPath = value;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct NodePathsFromInstanceKeyPathsRequestParams : RequestWithRulesetParams
{
private:
    bvector<bvector<ECInstanceKey>> m_instanceKeyPaths;
    Nullable<size_t> m_markedIndex;
public:
    NodePathsFromInstanceKeyPathsRequestParams(RequestWithRulesetParams const& rulesetParams, bvector<bvector<ECInstanceKey>> instanceKeyPaths, Nullable<size_t> markedIndex = nullptr)
        : RequestWithRulesetParams(rulesetParams), m_instanceKeyPaths(instanceKeyPaths), m_markedIndex(markedIndex)
        {}
    NodePathsFromInstanceKeyPathsRequestParams(RequestWithRulesetParams&& rulesetParams, bvector<bvector<ECInstanceKey>> instanceKeyPaths, Nullable<size_t> markedIndex = nullptr)
        : RequestWithRulesetParams(std::move(rulesetParams)), m_instanceKeyPaths(instanceKeyPaths), m_markedIndex(markedIndex)
        {}
    NodePathsFromInstanceKeyPathsRequestParams(Utf8String rulesetId, RulesetVariables rulesetVariables, bvector<bvector<ECInstanceKey>> instanceKeyPaths, Nullable<size_t> markedIndex = nullptr)
        : RequestWithRulesetParams(rulesetId, rulesetVariables), m_instanceKeyPaths(instanceKeyPaths), m_markedIndex(markedIndex)
        {}
    bvector<bvector<ECInstanceKey>> const& GetInstanceKeyPaths() const {return m_instanceKeyPaths;}
    Nullable<size_t> const& GetMarkedIndex() const {return m_markedIndex;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct NodePathsFromFilterTextRequestParams : RequestWithRulesetParams
{
private:
    Utf8String m_filterText;
public:
    NodePathsFromFilterTextRequestParams(RequestWithRulesetParams const& rulesetParams, Utf8String filterText)
        : RequestWithRulesetParams(rulesetParams), m_filterText(filterText)
        {}
    NodePathsFromFilterTextRequestParams(RequestWithRulesetParams&& rulesetParams, Utf8String filterText)
        : RequestWithRulesetParams(std::move(rulesetParams)), m_filterText(filterText)
        {}
    NodePathsFromFilterTextRequestParams(Utf8String rulesetId, RulesetVariables rulesetVariables, Utf8String filterText)
        : RequestWithRulesetParams(rulesetId, rulesetVariables), m_filterText(filterText)
        {}
    Utf8StringCR GetFilterText() const {return m_filterText;}
    void SetFilterText(Utf8String value) {m_filterText = value;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct HierarchyCompareRequestParams : BaseRequestParams
{
private:
    Utf8String m_lhsRulesetId;
    RulesetVariables m_lhsVariables;
    Utf8String m_rhsRulesetId;
    RulesetVariables m_rhsVariables;
    bvector<NavNodeKeyCPtr> m_expandedNodeKeys;
    HierarchyComparePositionPtr m_continuationToken;
    int m_resultSize;
    std::shared_ptr<IHierarchyChangeRecordsHandler> m_recordsHandler;
public:
    HierarchyCompareRequestParams(BaseRequestParams const& baseParams, std::shared_ptr<IHierarchyChangeRecordsHandler> recordsHandler,
        Utf8String lhsRulesetId, RulesetVariables lhsVariables, Utf8String rhsRulesetId, RulesetVariables rhsVariables,
        bvector<NavNodeKeyCPtr> expandedNodeKeys, HierarchyComparePositionPtr continuationToken = nullptr, int resultSize = -1)
        : BaseRequestParams(baseParams), m_recordsHandler(recordsHandler),
        m_lhsRulesetId(lhsRulesetId), m_lhsVariables(lhsVariables), m_rhsRulesetId(rhsRulesetId), m_rhsVariables(rhsVariables),
        m_expandedNodeKeys(expandedNodeKeys), m_continuationToken(continuationToken), m_resultSize(resultSize)
        {}
    HierarchyCompareRequestParams(BaseRequestParams&& baseParams, std::shared_ptr<IHierarchyChangeRecordsHandler> recordsHandler,
        Utf8String lhsRulesetId, RulesetVariables lhsVariables, Utf8String rhsRulesetId, RulesetVariables rhsVariables,
        bvector<NavNodeKeyCPtr> expandedNodeKeys, HierarchyComparePositionPtr continuationToken = nullptr, int resultSize = -1)
        : BaseRequestParams(std::move(baseParams)), m_recordsHandler(recordsHandler),
        m_lhsRulesetId(lhsRulesetId), m_lhsVariables(lhsVariables), m_rhsRulesetId(rhsRulesetId), m_rhsVariables(rhsVariables),
        m_expandedNodeKeys(expandedNodeKeys), m_continuationToken(continuationToken), m_resultSize(resultSize)
        {}
    HierarchyCompareRequestParams(std::shared_ptr<IHierarchyChangeRecordsHandler> recordsHandler,
        Utf8String lhsRulesetId, RulesetVariables lhsVariables, Utf8String rhsRulesetId, RulesetVariables rhsVariables,
        bvector<NavNodeKeyCPtr> expandedNodeKeys, HierarchyComparePositionPtr continuationToken = nullptr, int resultSize = -1)
        : m_recordsHandler(recordsHandler),
        m_lhsRulesetId(lhsRulesetId), m_lhsVariables(lhsVariables), m_rhsRulesetId(rhsRulesetId), m_rhsVariables(rhsVariables),
        m_expandedNodeKeys(expandedNodeKeys), m_continuationToken(continuationToken), m_resultSize(resultSize)
        {}
    std::shared_ptr<IHierarchyChangeRecordsHandler> GetRecordsHandler() const {return m_recordsHandler;}
    Utf8StringCR GetLhsRulesetId() const {return m_lhsRulesetId;}
    RulesetVariables const& GetLhsVariables() const {return m_lhsVariables;}
    Utf8StringCR GetRhsRulesetId() const {return m_rhsRulesetId;}
    RulesetVariables const& GetRhsVariables() const {return m_rhsVariables;}
    bvector<NavNodeKeyCPtr> const& GetExpandedNodeKeys() const {return m_expandedNodeKeys;}
    HierarchyComparePositionPtr GetContinuationToken() const {return m_continuationToken;}
    int GetResultSize() const {return m_resultSize;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ContentMetadataRequestParams : RequestWithRulesetParams
{
private:
    RefCountedPtr<SelectionInfo const> m_selectionInfo;
    Utf8String m_preferredDisplayType;
    int m_contentFlags;
public:
    ContentMetadataRequestParams(RequestWithRulesetParams const& rulesetParams, Utf8String preferredDisplayType, int contentFlags, SelectionInfo const* selectionInfo = nullptr)
        : RequestWithRulesetParams(rulesetParams), m_preferredDisplayType(preferredDisplayType), m_contentFlags(contentFlags), m_selectionInfo(selectionInfo)
        {}
    ContentMetadataRequestParams(RequestWithRulesetParams&& rulesetParams, Utf8String preferredDisplayType, int contentFlags, SelectionInfo const* selectionInfo = nullptr)
        : RequestWithRulesetParams(std::move(rulesetParams)), m_preferredDisplayType(preferredDisplayType), m_contentFlags(contentFlags), m_selectionInfo(selectionInfo)
        {}
    ContentMetadataRequestParams(Utf8String rulesetId, RulesetVariables rulesetVariables, Utf8String preferredDisplayType, int contentFlags, SelectionInfo const* selectionInfo = nullptr)
        : RequestWithRulesetParams(rulesetId, rulesetVariables), m_preferredDisplayType(preferredDisplayType), m_contentFlags(contentFlags), m_selectionInfo(selectionInfo)
        {}
    Utf8StringCR GetPreferredDisplayType() const {return m_preferredDisplayType;}
    void SetPreferredDisplayType(Utf8String value) {m_preferredDisplayType = value;}
    int GetContentFlags() const {return m_contentFlags;}
    void SetContentFlags(int value) {m_contentFlags = value;}
    SelectionInfo const* GetSelectionInfo() const {return m_selectionInfo.get();}
    void SetSelectionInfo(SelectionInfo const* value) {m_selectionInfo = value;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ContentClassesRequestParams : ContentMetadataRequestParams
{
private:
    bvector<ECClassCP> m_inputClasses;
public:
    ContentClassesRequestParams(ContentMetadataRequestParams const& metadataParams, bvector<ECClassCP> const& inputClasses)
        : ContentMetadataRequestParams(metadataParams), m_inputClasses(inputClasses)
        {}
    ContentClassesRequestParams(ContentMetadataRequestParams&& metadataParams, bvector<ECClassCP> const& inputClasses)
        : ContentMetadataRequestParams(std::move(metadataParams)), m_inputClasses(inputClasses)
        {}
    ContentClassesRequestParams(Utf8String rulesetId, RulesetVariables rulesetVariables, Utf8String preferredDisplayType, int contentFlags, bvector<ECClassCP> const& inputClasses)
        : ContentMetadataRequestParams(rulesetId, rulesetVariables, preferredDisplayType, contentFlags), m_inputClasses(inputClasses)
        {}
    bvector<ECClassCP> const& GetInputClasses() const {return m_inputClasses;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ContentDescriptorRequestParams : ContentMetadataRequestParams
{
private:
    KeySetCPtr m_inputKeys;
public:
    ContentDescriptorRequestParams(ContentMetadataRequestParams const& metadataParams, KeySetCR inputKeys)
        : ContentMetadataRequestParams(metadataParams), m_inputKeys(&inputKeys)
        {}
    ContentDescriptorRequestParams(ContentMetadataRequestParams&& metadataParams, KeySetCR inputKeys)
        : ContentMetadataRequestParams(std::move(metadataParams)), m_inputKeys(&inputKeys)
        {}
    ContentDescriptorRequestParams(Utf8String rulesetId, RulesetVariables rulesetVariables, Utf8String preferredDisplayType, int contentFlags, KeySetCR inputKeys, SelectionInfo const* selectionInfo = nullptr)
        : ContentMetadataRequestParams(rulesetId, rulesetVariables, preferredDisplayType, contentFlags, selectionInfo), m_inputKeys(&inputKeys)
        {}
    KeySetCR GetInputKeys() const {return *m_inputKeys;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ContentRequestParams
{
private:
    ContentDescriptorCPtr m_descriptor;
public:
    ContentRequestParams(ContentDescriptorCR descriptor)
        : m_descriptor(&descriptor)
        {}
    ContentDescriptorCR GetContentDescriptor() const {return *m_descriptor;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct DistinctValuesRequestParams : ContentRequestParams
{
private:
    std::shared_ptr<IContentFieldMatcher> m_distinctFieldMatcher;
public:
    DistinctValuesRequestParams(ContentDescriptorCR descriptor, std::shared_ptr<IContentFieldMatcher> distinctFieldMatcher)
        : ContentRequestParams(descriptor), m_distinctFieldMatcher(std::move(distinctFieldMatcher))
        {}
    IContentFieldMatcherCR GetDistinctFieldMatcher() const {return *m_distinctFieldMatcher;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ECInstanceDisplayLabelRequestParams : RequestWithRulesetParams
{
private:
    ECInstanceKey m_key;
public:
    ECInstanceDisplayLabelRequestParams(RequestWithRulesetParams const& rulesetParams, ECInstanceKey key)
        : RequestWithRulesetParams(rulesetParams), m_key(key)
        {}
    ECInstanceDisplayLabelRequestParams(RequestWithRulesetParams&& rulesetParams, ECInstanceKey key)
        : RequestWithRulesetParams(std::move(rulesetParams)), m_key(key)
        {}
    ECInstanceDisplayLabelRequestParams(Utf8String rulesetId, RulesetVariables rulesetVariables, ECInstanceKey key)
        : RequestWithRulesetParams(rulesetId, rulesetVariables), m_key(key)
        {}
    ECInstanceDisplayLabelRequestParams(ECInstanceKey key)
        : RequestWithRulesetParams("", RulesetVariables()), m_key(key)
        {}
    ECInstanceKeyCR GetInstanceKey() const {return m_key;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct KeySetDisplayLabelRequestParams : RequestWithRulesetParams
{
private:
    KeySetCPtr m_keys;
public:
    KeySetDisplayLabelRequestParams(RequestWithRulesetParams const& rulesetParams, KeySetCR keys)
        : RequestWithRulesetParams(rulesetParams), m_keys(&keys)
        {}
    KeySetDisplayLabelRequestParams(RequestWithRulesetParams&& rulesetParams, KeySetCR keys)
        : RequestWithRulesetParams(std::move(rulesetParams)), m_keys(&keys)
        {}
    KeySetDisplayLabelRequestParams(Utf8String rulesetId, RulesetVariables rulesetVariables, KeySetCR keys)
        : RequestWithRulesetParams(rulesetId, rulesetVariables), m_keys(&keys)
        {}
    KeySetDisplayLabelRequestParams(KeySetCR keys)
        : RequestWithRulesetParams("", RulesetVariables()), m_keys(&keys)
        {}
    KeySetCR GetKeys() const {return *m_keys;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
