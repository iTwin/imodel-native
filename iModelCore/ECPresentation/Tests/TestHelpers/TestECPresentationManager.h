/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECPresentationTest.h"
#include <ECPresentation/IECPresentationManager.h>
#include "TestConnectionCache.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Aidas.Vaiksnoras                08/2017
+===============+===============+===============+===============+===============+======*/
struct TestContentDataSource : IDataSource<ContentSetItemCPtr>
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
    static RefCountedPtr<TestContentDataSource> Create() {return new TestContentDataSource();}
    void AddContentSetItem(ContentSetItemCPtr item) {m_vector.push_back(item);}
};

/*=================================================================================**//**
* @bsiclass                                     Aidas.Vaiksnoras                08/2017
+===============+===============+===============+===============+===============+======*/
struct TestNodesDataSource : IDataSource<NavNodeCPtr>
{
private:
    bvector<NavNodeCPtr> m_vec;
    TestNodesDataSource(bvector<NavNodeCPtr> vec) : m_vec(vec) {}
protected:
    NavNodeCPtr _Get(size_t index) const override { return m_vec[index]; }
    size_t _GetSize() const override { return m_vec.size(); }
public:
    static RefCountedPtr<TestNodesDataSource> Create(bvector<NavNodeCPtr> vec) { return new TestNodesDataSource(vec); }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2016
+===============+===============+===============+===============+===============+======*/
struct TestECPresentationManager : IECPresentationManager
{
    typedef bmap<NavNodeCPtr, bvector<NavNodeCPtr>> Hierarchy;
private:
    Hierarchy m_hierarchy; // parent -> children
    bmap<NavNodeCP, NavNodeCP> m_parentship; // child -> parent
    std::function<bvector<NavNodeCPtr>(ECDbCR, Utf8CP, JsonValueCR)> m_getFilteredNodesPathsHandler;
    std::function<ContentDescriptorCPtr(ECDbCR, Utf8CP, int, KeySetCR, SelectionInfo const*, JsonValueCR)> m_contentDescriptorHandler;
    std::function<ContentCPtr(ContentDescriptorCR, PageOptionsCR)> m_contentHandler;

private:
    DataContainer<NavNodeCPtr> GetNodes(NavNodeCP parent)
        {
        bpair<NavNodeCPtr, bvector<NavNodeCPtr>> pair;
        if (!FindPair(parent, pair))
            return DataContainer<NavNodeCPtr>(*EmptyDataSource<NavNodeCPtr>::Create());

        bvector<NavNodeCPtr> nodes;
        for (NavNodeCPtr node : pair.second)
            nodes.push_back(node.get()->Clone());

        return DataContainer<NavNodeCPtr>(*TestNodesDataSource::Create(nodes));
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
    folly::Future<DataContainer<NavNodeCPtr>> _GetRootNodes(ECDbCR, PageOptionsCR, JsonValueCR, PresentationRequestContextCR) override {return GetNodes(nullptr);}
    folly::Future<size_t> _GetRootNodesCount(ECDbCR, JsonValueCR, PresentationRequestContextCR) override {return GetNodesCount(nullptr);}
    folly::Future<DataContainer<NavNodeCPtr>> _GetChildren(ECDbCR, NavNodeCR parent, PageOptionsCR, JsonValueCR, PresentationRequestContextCR) override {return GetNodes(&parent);}
    folly::Future<size_t> _GetChildrenCount(ECDbCR, NavNodeCR parent, JsonValueCR, PresentationRequestContextCR) override {return GetNodesCount(&parent);}
    folly::Future<NavNodeCPtr> _GetParent(ECDbCR, NavNodeCR node, JsonValueCR, PresentationRequestContextCR) override
        {
        auto iter = m_parentship.find(&node);
        return (m_parentship.end() != iter) ? iter->second : nullptr;
        }
    folly::Future<NavNodeCPtr> _GetNode(ECDbCR, NavNodeKeyCR nodeKey, JsonValueCR, PresentationRequestContextCR) override
        {
        for (auto pair : m_hierarchy)
            {
            NavNodeCP node = pair.first.get();
            if (nullptr != node && node->GetKey()->GetHash().Equals(nodeKey.GetHash()))
                return node;
            }
        return NavNodeCPtr(nullptr);
        }
    folly::Future<bvector<NavNodeCPtr>> _GetFilteredNodes(ECDbCR db, Utf8CP filterText, JsonValueCR options, PresentationRequestContextCR) override
        {
        if (nullptr != m_getFilteredNodesPathsHandler)
            return m_getFilteredNodesPathsHandler(db, filterText, options);
        return bvector<NavNodeCPtr>();
        }

    // Content
    folly::Future<bvector<SelectClassInfo>> _GetContentClasses(ECDbCR db, Utf8CP preferredDisplayType, int contentFlags, bvector<ECClassCP> const& classes, JsonValueCR options, PresentationRequestContextCR) override
        {
        return bvector<SelectClassInfo>();
        }
    folly::Future<ContentDescriptorCPtr> _GetContentDescriptor(ECDbCR db, Utf8CP preferredDisplayType, int contentFlags, KeySetCR input, SelectionInfo const* selectionInfo, JsonValueCR options, PresentationRequestContextCR) override
        {
        if (nullptr != m_contentDescriptorHandler)
            return m_contentDescriptorHandler(db, preferredDisplayType, contentFlags, input, selectionInfo, options);
        return ContentDescriptor::Create(*new TestConnection(const_cast<ECDbR>(db)), options, *NavNodeKeyListContainer::Create());
        }
    folly::Future<ContentCPtr> _GetContent(ContentDescriptorCR descriptor, PageOptionsCR pageOptions, PresentationRequestContextCR) override
        {
        if (nullptr != m_contentHandler)
            return m_contentHandler(descriptor, pageOptions);
        return Content::Create(descriptor, *TestContentDataSource::Create());
        }
    folly::Future<size_t> _GetContentSetSize(ContentDescriptorCR, PresentationRequestContextCR) override
        {
        return 0;
        }
    folly::Future<Utf8String> _GetDisplayLabel(ECDbCR, KeySetCR, JsonValueCR, PresentationRequestContextCR) override
        {
        return "";
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
                m_parentship[child.get()] = parent;
            }
        }

    void SetGetFilteredNodesPathsHandler(std::function<bvector<NavNodeCPtr>(ECDbCR, Utf8CP, JsonValueCR)> handler) {m_getFilteredNodesPathsHandler = handler;}
    void SetContentDescriptorHandler(std::function<ContentDescriptorCPtr(ECDbCR, Utf8CP, int, KeySetCR, SelectionInfo const*, JsonValueCR)> handler){m_contentDescriptorHandler = handler;}
    void SetContentHandler(std::function<ContentCPtr(ContentDescriptorCR, PageOptionsCR)> handler){m_contentHandler = handler;}
    };

END_ECPRESENTATIONTESTS_NAMESPACE
