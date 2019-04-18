/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECPresentationTest.h"
#include <ECPresentation/IECPresentationManager.h>

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Aidas.Vaiksnoras                08/2017
+===============+===============+===============+===============+===============+======*/
struct TestDataSource : IDataSource<ContentSetItemCPtr> 
{
private:
    bvector<ContentSetItemCPtr> m_vector;

protected:
    size_t _GetSize() const override {return m_vector.size();}

    ContentSetItemCPtr _Get(size_t index) const override
        {
        ContentSetItemCPtr item;
        if (m_vector.size() <= 0)
            return nullptr;
        return m_vector[index];
        }
public:
    static RefCountedPtr<TestDataSource> Create() {return new TestDataSource();}
    void AddContentSetItem(ContentSetItemCPtr item) {m_vector.push_back(item);}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2016
+===============+===============+===============+===============+===============+======*/
struct NodesVectorDataSource : IDataSource<NavNodeCPtr>
{
private:
    bvector<NavNodeCPtr> m_vec;
    NodesVectorDataSource(bvector<NavNodeCPtr> vec) : m_vec(vec) {}
protected:
    NavNodeCPtr _Get(size_t index) const override {return m_vec[index];}
    size_t _GetSize() const override {return m_vec.size();}
public:
    static RefCountedPtr<NodesVectorDataSource> Create(bvector<NavNodeCPtr> vec) {return new NodesVectorDataSource(vec);}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2016
+===============+===============+===============+===============+===============+======*/
struct NavNodePtrComparer
    {
    bool operator()(NavNodeCPtr const& lhs, NavNodeCPtr const& rhs) const
        {
        return (lhs.get() < rhs.get());
        }
    };

typedef bmap<NavNodeCPtr, bvector<NavNodeCPtr>, NavNodePtrComparer> Hierarchy;
/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2016
+===============+===============+===============+===============+===============+======*/
struct TestECPresentationManager : IECPresentationManager
{
private:
    Hierarchy m_hierarchy; // parent -> children
    bmap<NavNodeCP, NavNodeCP> m_parentship; // child -> parent
    std::function<bool(IConnectionCR, NavNodeCR parentNode, ECInstanceKeyCR childNodeKey, JsonValueCR)> m_hasChildHandler;
    std::function<bvector<NavNodeCPtr>(IConnectionCR, Utf8CP, JsonValueCR)> m_getFilteredNodesPathsHandler;
    std::function<bvector<ECInstanceChangeResult>(IConnectionCR, bvector<ChangedECInstanceInfo> const&, Utf8CP, ECValueCR, JsonValueCR)> m_saveValueChangeHandler;
    std::function<ContentDescriptorCPtr(IConnectionCR, Utf8CP, KeySetCR, SelectionInfo const*, JsonValueCR)> m_contentDescriptorHandler;
    std::function<ContentCPtr(ContentDescriptorCR, PageOptionsCR)> m_contentHandler;
    std::function<void(IConnectionCR, NavNodeKeyCR, JsonValueCR)> m_onNodeCheckedHandler;
    std::function<void(IConnectionCR, NavNodeKeyCR, JsonValueCR)> m_onNodeUncheckedHandler;
    std::function<void(IConnectionCR, NavNodeKeyCR, JsonValueCR)> m_onNodeExpandedHandler;
    std::function<void(IConnectionCR, NavNodeKeyCR, JsonValueCR)> m_onNodeCollapsedHandler;
    std::function<void(IConnectionCR, JsonValueCR)> m_onAllNodesCollapseHandler;

private:
    DataContainer<NavNodeCPtr> GetNodes(NavNodeCP parent)
        {
        bpair<NavNodeCPtr, bvector<NavNodeCPtr>> pair;
        if (!FindPair(parent, pair))
            return DataContainer<NavNodeCPtr>(*EmptyDataSource<NavNodeCPtr>::Create());

        bvector<NavNodeCPtr> nodes;
        for (NavNodeCPtr node : pair.second)
            nodes.push_back(node.get()->Clone());

        return DataContainer<NavNodeCPtr>(*NodesVectorDataSource::Create(nodes));
        }
    size_t GetNodesCount(NavNodeCP parent)
        {
        bpair<NavNodeCPtr, bvector<NavNodeCPtr>> pair;
        if (!FindPair(parent, pair))
            return 0;
        return pair.second.size();
        }

    bool FindPair(NavNodeCP parent, bpair<NavNodeCPtr, bvector<NavNodeCPtr>>& p)
        {
        for (auto& pair : m_hierarchy)
            {
            if (pair.first == nullptr && parent == nullptr ||
                pair.first != nullptr && parent != nullptr && pair.first.get()->GetKey()->Compare(*parent->GetKey()) == 0)
                {
                p = pair;
                return true;
                }
            }
        return false;
        }
    
protected:
    // Navigation
    folly::Future<DataContainer<NavNodeCPtr>> _GetRootNodes(IConnectionCR, PageOptionsCR, JsonValueCR) override {return GetNodes(nullptr);}
    folly::Future<size_t> _GetRootNodesCount(IConnectionCR, JsonValueCR) override {return GetNodesCount(nullptr);}
    folly::Future<DataContainer<NavNodeCPtr>> _GetChildren(IConnectionCR, NavNodeCR parent, PageOptionsCR, JsonValueCR) override {return GetNodes(&parent);}
    folly::Future<size_t> _GetChildrenCount(IConnectionCR, NavNodeCR parent, JsonValueCR) override {return GetNodesCount(&parent);}
    folly::Future<NavNodeCPtr> _GetParent(IConnectionCR, NavNodeCR node, JsonValueCR) override
        {
        auto iter = m_parentship.find(&node);
        return (m_parentship.end() != iter) ? iter->second : nullptr;
        }
    folly::Future<NavNodeCPtr> _GetNode(IConnectionCR, NavNodeKeyCR nodeKey, JsonValueCR) override
        {
        for (auto pair : m_hierarchy)
            {
            NavNodeCP node = pair.first.get();
            if (nullptr != node && node->GetKey()->GetNodeHash().Equals(nodeKey.GetNodeHash()))
                return node;
            }
        return NavNodeCPtr(nullptr);
        }
    folly::Future<bool> _HasChild(IConnectionCR db, NavNodeCR parentNode, ECInstanceKeyCR childKey, JsonValueCR options) override
        {
        if (nullptr != m_hasChildHandler)
            return m_hasChildHandler(db, parentNode, childKey, options);

        auto iter = m_hierarchy.find(&parentNode);
        if (m_hierarchy.end() == iter)
            return false;

        bvector<NavNodeCPtr> const& children = iter->second;
        return children.end() != std::find_if(children.begin(), children.end(), [&childKey](NavNodeCPtr const& child)
            {
            return (child->GetKey()->AsECInstanceNodeKey() && child->GetKey()->AsECInstanceNodeKey()->GetInstanceKey() == childKey);
            });
        }
    folly::Future<bvector<NavNodeCPtr>> _GetFilteredNodes(IConnectionCR connection, Utf8CP filterText, JsonValueCR options) override 
        {
        if (nullptr != m_getFilteredNodesPathsHandler)
            return m_getFilteredNodesPathsHandler(connection, filterText, options);
        return bvector<NavNodeCPtr>();
        }
    folly::Future<folly::Unit> _OnNodeChecked(IConnectionCR db, NavNodeKeyCR nodeKey, JsonValueCR options) override
        {
        if (nullptr != m_onNodeCheckedHandler) 
            m_onNodeCheckedHandler(db, nodeKey, options);
        return folly::unit;
        }
    folly::Future<folly::Unit> _OnNodeUnchecked(IConnectionCR db, NavNodeKeyCR nodeKey, JsonValueCR options) override
        {
        if (nullptr != m_onNodeUncheckedHandler) 
            m_onNodeUncheckedHandler(db, nodeKey, options);
        return folly::unit;
        }
    folly::Future<folly::Unit> _OnNodeExpanded(IConnectionCR db, NavNodeKeyCR nodeKey, JsonValueCR options) override
        {
        if (nullptr != m_onNodeExpandedHandler) 
            m_onNodeExpandedHandler(db, nodeKey, options);
        return folly::unit;
        }
    folly::Future<folly::Unit> _OnNodeCollapsed(IConnectionCR db, NavNodeKeyCR nodeKey, JsonValueCR options) override
        {
        if (nullptr != m_onNodeCollapsedHandler) 
            m_onNodeCollapsedHandler(db, nodeKey, options);
        return folly::unit;
        }
    folly::Future<folly::Unit> _OnAllNodesCollapsed(IConnectionCR connection, JsonValueCR options) override
        {
        if (nullptr != m_onAllNodesCollapseHandler) 
            m_onAllNodesCollapseHandler(connection, options);
        return folly::unit;
        }

    // Content
    folly::Future<bvector<SelectClassInfo>> _GetContentClasses(IConnectionCR db, Utf8CP preferredDisplayType, bvector<ECClassCP> const& classes, JsonValueCR options) override
        {
        return bvector<SelectClassInfo>();
        }
    folly::Future<ContentDescriptorCPtr> _GetContentDescriptor(IConnectionCR db, Utf8CP preferredDisplayType, KeySetCR input, SelectionInfo const* selectionInfo, JsonValueCR options) override 
        {
        if (nullptr != m_contentDescriptorHandler)
            return m_contentDescriptorHandler(db, preferredDisplayType, input, selectionInfo, options);
        return ContentDescriptor::Create(db, options, *NavNodeKeyListContainer::Create());
        }
    folly::Future<ContentCPtr> _GetContent(ContentDescriptorCR descriptor, PageOptionsCR pageOptions) override 
        {
        if (nullptr != m_contentHandler)
            return m_contentHandler(descriptor, pageOptions);
        return Content::Create(descriptor, *TestDataSource::Create());
        }
    folly::Future<size_t> _GetContentSetSize(ContentDescriptorCR) override
        {
        return 0;
        }
    folly::Future<Utf8String> _GetDisplayLabel(IConnectionCR, KeySetCR) override
        {
        return "";
        }

    // Updating
    folly::Future<bvector<ECInstanceChangeResult>> _SaveValueChange(IConnectionCR db, bvector<ChangedECInstanceInfo> const& instancesInfo, Utf8CP propertyAccessor, ECValueCR value, JsonValueCR options) override
        {
        if (nullptr != m_saveValueChangeHandler)
            return m_saveValueChangeHandler(db, instancesInfo, propertyAccessor, value, options);
        return bvector<ECInstanceChangeResult>();
        }

public:
    TestECPresentationManager(IConnectionManagerR connections) : IECPresentationManager(connections) {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetHierarchy(Hierarchy hierarchy)
        {
        m_hierarchy = hierarchy;
        for (auto pair : m_hierarchy)
            {
            NavNodeCP parent = pair.first.get();
            for (NavNodeCPtr child : pair.second)
                m_parentship[child.get()] = parent;
            }
        }
    
    void SetHasChildHandler(std::function<bool(IConnectionCR, NavNodeCR, ECInstanceKeyCR, JsonValueCR)> handler) {m_hasChildHandler = handler;}
    void SetGetFilteredNodesPathsHandler(std::function<bvector<NavNodeCPtr>(IConnectionCR, Utf8CP, JsonValueCR)> handler) {m_getFilteredNodesPathsHandler = handler;}
    void SetSaveValueChangeHandler(std::function<bvector<ECInstanceChangeResult>(IConnectionCR, bvector<ChangedECInstanceInfo> const&, Utf8CP, ECValueCR, JsonValueCR)> handler) {m_saveValueChangeHandler = handler;}
    void SetContentDescriptorHandler(std::function<ContentDescriptorCPtr(IConnectionCR, Utf8CP, KeySetCR, SelectionInfo const*, JsonValueCR)> handler){m_contentDescriptorHandler = handler;}
    void SetContentHandler(std::function<ContentCPtr(ContentDescriptorCR, PageOptionsCR)> handler){m_contentHandler = handler;}
    void SetOnNodeCheckedHandler(std::function<void(IConnectionCR, NavNodeKeyCR, JsonValueCR)> handler) {m_onNodeCheckedHandler = handler;}
    void SetOnNodeUncheckedHandler(std::function<void(IConnectionCR, NavNodeKeyCR, JsonValueCR)> handler) {m_onNodeUncheckedHandler = handler;}
    void SetOnNodeExpandedHandler(std::function<void(IConnectionCR, NavNodeKeyCR, JsonValueCR)> handler) {m_onNodeExpandedHandler = handler;}
    void SetOnNodeCollapsedHandler(std::function<void(IConnectionCR, NavNodeKeyCR, JsonValueCR)> handler) {m_onNodeCollapsedHandler = handler;}
    void SetOnCollapseAllNodesHandler(std::function<void(IConnectionCR, JsonValueCR)> handler) {m_onAllNodesCollapseHandler = handler;}
    };

END_ECPRESENTATIONTESTS_NAMESPACE
