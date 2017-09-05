/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/TestHelpers/TestECPresentationManager.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECPresentationTest.h"
#include <ECPresentation/IECPresentationManager.h>

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS
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
    bvector<NavNodeCPtr> const& m_vec;
    NodesVectorDataSource(bvector<NavNodeCPtr> const& vec) : m_vec(vec) {}
protected:
    NavNodeCPtr _Get(size_t index) const override {return m_vec[index];}
    size_t _GetSize() const override {return m_vec.size();}
public:
    static RefCountedPtr<NodesVectorDataSource> Create(bvector<NavNodeCPtr> const& vec) {return new NodesVectorDataSource(vec);}
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
    Hierarchy m_hierarchy;
    bmap<NavNodeCP, NavNodeCP> m_parentship;
    std::function<bool(ECDbR, NavNodeCR parentNode, NavNodeKeyCR childNodeKey, JsonValueCR)> m_hasChildHandler;
    std::function<bvector<ECInstanceChangeResult>(ECDbR, bvector<ChangedECInstanceInfo> const&, Utf8CP, ECValueCR, JsonValueCR)> m_saveValueChangeHandler;
    std::function<ContentDescriptorCPtr(ECDbR, Utf8CP, SelectionInfo const&, JsonValueCR)> m_contentDescriptorHandler;
    std::function<ContentCPtr(ECDbR, ContentDescriptorCR, SelectionInfo const&, PageOptionsCR, JsonValueCR)> m_contentHandler;
    std::function<void(ECDbR, uint64_t)> m_onNodeCheckedHandler;
    std::function<void(ECDbR, uint64_t)> m_onNodeUncheckedHandler;
    std::function<void(ECDbR, uint64_t)> m_onNodeExpandedHandler;
    std::function<void(ECDbR, uint64_t)> m_onNodeCollapsedHandler;

private:
    DataContainer<NavNodeCPtr> GetNodes(NavNodeCP parent)
        {
        auto iter = m_hierarchy.find(parent);
        if (m_hierarchy.end() == iter)
            return DataContainer<NavNodeCPtr>(*EmptyDataSource<NavNodeCPtr>::Create());
        return DataContainer<NavNodeCPtr>(*NodesVectorDataSource::Create(iter->second));
        }
    size_t GetNodesCount(NavNodeCP parent)
        {
        auto iter = m_hierarchy.find(parent);
        return (m_hierarchy.end() == iter) ? 0 : iter->second.size();
        }
    
protected:
    // Navigation
    DataContainer<NavNodeCPtr> _GetRootNodes(ECDbR, PageOptionsCR, JsonValueCR) override {return GetNodes(nullptr);}
    size_t _GetRootNodesCount(ECDbR, JsonValueCR) override {return GetNodesCount(nullptr);}
    DataContainer<NavNodeCPtr> _GetChildren(ECDbR, NavNodeCR parent, PageOptionsCR, JsonValueCR) override {return GetNodes(&parent);}
    size_t _GetChildrenCount(ECDbR, NavNodeCR parent, JsonValueCR) override {return GetNodesCount(&parent);}
    NavNodeCPtr _GetParent(ECDbR, NavNodeCR node, JsonValueCR) override
        {
        auto iter = m_parentship.find(&node);
        return (m_parentship.end() != iter) ? iter->second : nullptr;
        }
    NavNodeCPtr _GetNode(ECDbR, uint64_t id) override
        {
        for (auto pair : m_hierarchy)
            {
            NavNodeCP node = pair.first.get();
            if (node->GetNodeId() == id)
                return node;
            }
        return nullptr;
        }
    bool _HasChild(ECDbR db, NavNodeCR parentNode, NavNodeKeyCR childNodeKey, JsonValueCR options) override
        {
        if (nullptr != m_hasChildHandler)
            return m_hasChildHandler(db, parentNode, childNodeKey, options);

        auto iter = m_parentship.find(&parentNode);
        if (m_parentship.end() == iter)
            return false;
        return 0 == iter->second->GetKey().Compare(childNodeKey);
        }
    void _OnNodeChecked(ECDbR db, uint64_t nodeId) override {if (nullptr != m_onNodeCheckedHandler) m_onNodeCheckedHandler(db, nodeId);}
    void _OnNodeUnchecked(ECDbR db, uint64_t nodeId) override {if (nullptr != m_onNodeUncheckedHandler) m_onNodeUncheckedHandler(db, nodeId);}
    void _OnNodeExpanded(ECDbR db, uint64_t nodeId) override {if (nullptr != m_onNodeExpandedHandler) m_onNodeExpandedHandler(db, nodeId);}
    void _OnNodeCollapsed(ECDbR db, uint64_t nodeId) override {if (nullptr != m_onNodeCollapsedHandler) m_onNodeCollapsedHandler(db, nodeId);}

    // Content
    ContentDescriptorCPtr _GetContentDescriptor(ECDbR db, Utf8CP preferredDisplayType, SelectionInfo const& selectionInfo, JsonValueCR options) override 
        {
        if (nullptr != m_contentDescriptorHandler)
            return m_contentDescriptorHandler(db, preferredDisplayType, selectionInfo, options);
        return ContentDescriptor::Create();
        }

    ContentCPtr _GetContent(ECDbR db, ContentDescriptorCR descriptor, SelectionInfo const& selectionInfo, PageOptionsCR pageOptions, JsonValueCR options) override 
        {
        if (nullptr != m_contentHandler)
            return m_contentHandler(db, descriptor, selectionInfo, pageOptions, options);
        return Content::Create(descriptor, *TestDataSource::Create());
        }

    size_t _GetContentSetSize(ECDbR, ContentDescriptorCR, SelectionInfo const&, JsonValueCR) override {return 0;}

    // Updating
    bvector<ECInstanceChangeResult> _SaveValueChange(ECDbR db, bvector<ChangedECInstanceInfo> const& instancesInfo, Utf8CP propertyAccessor, ECValueCR value, JsonValueCR options) override
        {
        if (nullptr != m_saveValueChangeHandler)
            return m_saveValueChangeHandler(db, instancesInfo, propertyAccessor, value, options);
        return bvector<ECInstanceChangeResult>();
        }

public:
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
                m_parentship[parent] = child.get();
            }
        }
    
    void SetHasChildHandler(std::function<bool(ECDbR, NavNodeCR, NavNodeKeyCR, JsonValueCR)> handler) {m_hasChildHandler = handler;}
    void SetSaveValueChangeHandler(std::function<bvector<ECInstanceChangeResult>(ECDbR, bvector<ChangedECInstanceInfo> const&, Utf8CP, ECValueCR, JsonValueCR)> handler) {m_saveValueChangeHandler = handler;}
    void SetContentDescriptorHandler(std::function<ContentDescriptorCPtr(ECDbR, Utf8CP, SelectionInfo const&, JsonValueCR)> handler){m_contentDescriptorHandler = handler;}
    void SetContentHandler(std::function<ContentCPtr(ECDbR, ContentDescriptorCR, SelectionInfo const&, PageOptionsCR, JsonValueCR)> handler){m_contentHandler = handler;}
    void SetOnNodeCheckedHandler(std::function<void(ECDbR, uint64_t)> handler) {m_onNodeCheckedHandler = handler;}
    void SetOnNodeUncheckedHandler(std::function<void(ECDbR, uint64_t)> handler) {m_onNodeUncheckedHandler = handler;}
    void SetOnNodeExpandedHandler(std::function<void(ECDbR, uint64_t)> handler) {m_onNodeExpandedHandler = handler;}
    void SetOnNodeCollapsedHandler(std::function<void(ECDbR, uint64_t)> handler) {m_onNodeCollapsedHandler = handler;}
    };
