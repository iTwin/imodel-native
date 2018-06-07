/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/PresentationManager.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/IECPresentationManager.h>
#include <ECPresentation/DefaultECPresentationSerializer.h>
#include "../Localization/Xliffs/ECPresentation.xliff.h"
#include "ValueHelpers.h"

IECPresentationManager* IECPresentationManager::s_instance = nullptr;
IECPresentationSerializer const* IECPresentationManager::s_serializer = nullptr;
ILocalizationProvider const* IECPresentationManager::s_localizationProvider = nullptr;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECPresentationManager::IsActive() {return nullptr != s_instance;}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IECPresentationManagerR IECPresentationManager::GetManager() {BeAssert(nullptr != s_instance); return *s_instance;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<DataContainer<NavNodeCPtr>> IECPresentationManager::GetRootNodes(ECDbCR db, PageOptionsCR pageOptions, JsonValueCR options)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return DataContainer<NavNodeCPtr>();
        }
    return _GetRootNodes(*connection, pageOptions, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<size_t> IECPresentationManager::GetRootNodesCount(ECDbCR db, JsonValueCR options)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return 0;
        }
    return _GetRootNodesCount(*connection, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<DataContainer<NavNodeCPtr>> IECPresentationManager::GetChildren(ECDbCR db, NavNodeCR node, PageOptionsCR pageOptions, JsonValueCR options)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return DataContainer<NavNodeCPtr>();
        }
    return _GetChildren(*connection, node, pageOptions, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<size_t> IECPresentationManager::GetChildrenCount(ECDbCR db, NavNodeCR node, JsonValueCR options)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return 0;
        }
    return _GetChildrenCount(*connection, node, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NavNodeCPtr> IECPresentationManager::GetParent(ECDbCR db, NavNodeCR node, JsonValueCR options)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return folly::makeFuture(NavNodeCPtr(nullptr));
        }
    return _GetParent(*connection, node, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bvector<SelectClassInfo>> IECPresentationManager::GetContentClasses(ECDbCR db, Utf8CP preferredDisplayType, bvector<ECClassCP> const& input, JsonValueCR options)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return bvector<SelectClassInfo>();
        }
    return _GetContentClasses(*connection, preferredDisplayType, input, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ContentDescriptorCPtr> IECPresentationManager::GetContentDescriptor(ECDbCR db, Utf8CP preferredDisplayType, KeySetCR inputKeys, SelectionInfo const* selectionInfo, JsonValueCR options)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return folly::makeFuture(ContentDescriptorCPtr(nullptr));
        }
    return _GetContentDescriptor(*connection, preferredDisplayType, inputKeys, selectionInfo, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ContentCPtr> IECPresentationManager::GetContent(ContentDescriptorCR descriptor, PageOptionsCR pageOptions)
    {
    return _GetContent(descriptor, pageOptions);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<size_t> IECPresentationManager::GetContentSetSize(ContentDescriptorCR descriptor)
    {

    return _GetContentSetSize(descriptor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NavNodeCPtr> IECPresentationManager::GetNode(ECDbCR db, NavNodeKeyCR nodeKey, JsonValueCR options)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return folly::makeFuture(NavNodeCPtr(nullptr));
        }
    return _GetNode(*connection, nodeKey, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> IECPresentationManager::NotifyNodeChecked(ECDbCR db, NavNodeKeyCR nodeKey, JsonValueCR options)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return folly::unit;
        }
    return _OnNodeChecked(*connection, nodeKey, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> IECPresentationManager::NotifyNodeUnchecked(ECDbCR db, NavNodeKeyCR nodeKey, JsonValueCR options)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return folly::unit;
        }
    return _OnNodeUnchecked(*connection, nodeKey, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> IECPresentationManager::NotifyNodeExpanded(ECDbCR db, NavNodeKeyCR nodeKey, JsonValueCR options)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return folly::unit;
        }
    return _OnNodeExpanded(*connection, nodeKey, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> IECPresentationManager::NotifyNodeCollapsed(ECDbCR db, NavNodeKeyCR nodeKey, JsonValueCR options)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return folly::unit;
        }
    return _OnNodeCollapsed(*connection, nodeKey, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> IECPresentationManager::NotifyAllNodesCollapsed(ECDbCR db, JsonValueCR options) 
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return folly::unit;
        }
    return _OnAllNodesCollapsed(*connection, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NodesPathElement> IECPresentationManager::FindNode(ECDbCR db, NavNodeCP parentNode, ECInstanceKeyCR lookupKey, JsonValueCR extendedOptions)
    {
    folly::Future<DataContainer<NavNodeCPtr>> nodesFuture = (nullptr == parentNode)
        ? GetRootNodes(db, PageOptions(), extendedOptions)
        : GetChildren(db, *parentNode, PageOptions(), extendedOptions);
    return nodesFuture.then([this, &db, lookupKey, extendedOptions](DataContainer<NavNodeCPtr> nodes) -> NodesPathElement
        {
        IConnectionCPtr connection = GetConnections().GetConnection(db);
        for (size_t i = 0; i < nodes.GetSize(); ++i)
            {
            NavNodeCPtr node = nodes[i];
            if (node->GetKey()->AsECInstanceNodeKey() && node->GetKey()->AsECInstanceNodeKey()->GetInstanceKey() == lookupKey)
                return NodesPathElement(*node, i);
            if (_HasChild(*connection, *node, lookupKey, extendedOptions).get())
                return NodesPathElement(*node, i);
            }
        return NodesPathElement();
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static NodesPathElement* AddToPath(NodesPathElement& path, NodesPathElement&& el)
    {
    if (path.GetNode().IsNull())
        {
        path = std::move(el);
        return &path;
        }
    
    path.GetChildren().push_back(std::move(el));
    return &path.GetChildren().back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NodesPathElement> IECPresentationManager::GetNodesPath(ECDbCR db, bvector<ECInstanceKey> const& keyPath, JsonValueCR extendedOptions)
    {
    NodesPathElement path;
    NodesPathElement* curr = &path;
    NavNodeCP parent = nullptr;
    for (size_t i = 0; i < keyPath.size(); ++i)
        {
        while (true)
            {
            ECInstanceKeyCR key = keyPath[i];
            NodesPathElement el = FindNode(db, parent, key, extendedOptions).get();
            if (!el.GetNode().IsValid())
                {
                BeAssert(false && "Provided nodes path doesn't exist in the hierarchy");
                return NodesPathElement();
                }

            parent = el.GetNode().get();
            curr = AddToPath(*curr, std::move(el));

            if (parent->GetKey()->AsECInstanceNodeKey() && parent->GetKey()->AsECInstanceNodeKey()->GetInstanceKey() == key)
                break;
            }
        }
    return path;
    }

static bool TryAppendPath(NodesPathElement& lhs, NodesPathElement const& rhs);
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void AppendPath(bvector<NodesPathElement>& lhsList, NodesPathElement const& rhs)
    {
    bool didAppend = false;
    for (NodesPathElement& lhs : lhsList)
        {
        if (true == (didAppend = TryAppendPath(lhs, rhs)))
            break;
        }
    if (!didAppend)
        lhsList.push_back(rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bool TryAppendPath(NodesPathElement& lhs, NodesPathElement const& rhs)
    {
    if (lhs.GetNode()->GetKey()->Compare(*rhs.GetNode()->GetKey()) == 0)
        {
        if (rhs.IsMarked())
            lhs.SetIsMarked(true);
        for (NodesPathElement const& rhsChild : rhs.GetChildren())
            AppendPath(lhs.GetChildren(), rhsChild);
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void MarkLeaves(NodesPathElement& path)
    {
    if (path.GetChildren().empty())
        {
        path.SetIsMarked(true);
        }
    else
        {
        for (NodesPathElement& child : path.GetChildren())
            MarkLeaves(child);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bvector<NodesPathElement>> IECPresentationManager::GetNodesPath(ECDbCR db, bvector<bvector<ECInstanceKey>> const& keyPaths, int64_t markedIndex, JsonValueCR extendedOptions)
    {
    bvector<folly::Future<NodesPathElement>> pathFutures;
    for (size_t i = 0; i < keyPaths.size(); ++i)
        {
        bvector<ECInstanceKey> const& keyPath = keyPaths[i];
        pathFutures.push_back(GetNodesPath(db, keyPath, extendedOptions));
        }
    return folly::collect(pathFutures).then([markedIndex](std::vector<NodesPathElement> paths) -> bvector<NodesPathElement>
        {
        bvector<NodesPathElement> mergedPaths;
        for (size_t i = 0; i < paths.size(); ++i)
            {
            NodesPathElement& path = paths[i];
            if (path.GetNode().IsValid())
                {
                if (markedIndex == (int64_t)i)
                    MarkLeaves(path);
                AppendPath(mergedPaths, path);
                }
            }
        return mergedPaths;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECInstanceChangeResult> IECPresentationManager::SaveValueChange(ECDbCR db, ChangedECInstanceInfo const& instanceInfo, Utf8CP propertyAccessor, ECValueCR value, JsonValueCR extendedOptions)
    {
    return SaveValueChange(db, bvector<ChangedECInstanceInfo>{instanceInfo}, propertyAccessor, value, extendedOptions).then([](bvector<ECInstanceChangeResult> result) -> ECInstanceChangeResult
        {
        if (result.empty())
            {
            BeAssert(false);
            return ECInstanceChangeResult::Error(IECPresentationManager::GetLocalizationProvider().GetString(bvector<Utf8CP>{ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()}));
            }
        return result[0];
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bvector<ECInstanceChangeResult>> IECPresentationManager::SaveValueChange(ECDbCR db, bvector<ChangedECInstanceInfo> const& instanceInfos, Utf8CP propertyAccessor, ECValueCR value, JsonValueCR extendedOptions)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return bvector<ECInstanceChangeResult>();
        }
    return _SaveValueChange(*connection, instanceInfos, propertyAccessor, value, extendedOptions);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECInstanceChangeResult> IECPresentationManager::SaveValueChange(ECDbCR db, ChangedECInstanceInfo const& instanceInfo, Utf8CP propertyAccessor, JsonValueCR value, JsonValueCR extendedOptions)
    {
    return SaveValueChange(db, bvector<ChangedECInstanceInfo>{instanceInfo}, propertyAccessor, value, extendedOptions).then([](bvector<ECInstanceChangeResult> result) -> ECInstanceChangeResult
        {
        if (result.empty())
            {
            BeAssert(false);
            return ECInstanceChangeResult::Error(IECPresentationManager::GetLocalizationProvider().GetString(bvector<Utf8CP>{ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()}));
            }
        return result[0];
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bvector<ECInstanceChangeResult>> IECPresentationManager::SaveValueChange(ECDbCR db, bvector<ChangedECInstanceInfo> const& instanceInfos, Utf8CP propertyAccessor, JsonValueCR value, JsonValueCR extendedOptions)
    {
    if (instanceInfos.empty())
        return bvector<ECInstanceChangeResult>();

    // note: assumes that all changed instances in this field use the same property
    ECClassCR changedClass = instanceInfos.front().GetChangedInstanceClass();
    ECPropertyCP prop = changedClass.GetPropertyP(propertyAccessor);
    if (nullptr == prop || (!prop->GetIsPrimitive() && !prop->GetIsNavigation()))
        {
        BeAssert(false && "Failed to determine the changed property or it's not primitive");
        return bvector<ECInstanceChangeResult>(instanceInfos.size(), 
            ECInstanceChangeResult::Error(IECPresentationManager::GetLocalizationProvider().GetString(bvector<Utf8CP>{ECPresentationL10N::GetNameSpace(), ECPresentationL10N::ERROR_General_Unknown()})));
        }
    ECValue ecValue = ValueHelpers::GetECValueFromJson(*prop, value);
    return SaveValueChange(db, instanceInfos, propertyAccessor, ecValue, extendedOptions);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bmap<uint64_t, bvector<NavNodeCPtr>>::iterator CreateHierarchy(IECPresentationManager& mgr, ECDbCR db, NavNodeCR node, bmap<uint64_t, bvector<NavNodeCPtr>>& hierarchy, bvector<NavNodeCPtr>& roots)
    {
    auto parentIter = hierarchy.end();
    if (0 == node.GetParentNodeId())
        {
        // this node has no parent
        roots.push_back(&node);
        }
    else
        {
        // see if parent is already in the hierarchy
        parentIter = hierarchy.find(node.GetParentNodeId());
        if (parentIter == hierarchy.end())
            {
            // get the parent and put it into the hierarchy
            NavNodeCPtr parent = mgr.GetParent(db, node).get();
            if (parent.IsValid())
                parentIter = CreateHierarchy(mgr, db, *parent, hierarchy, roots);
            }
        }
    // see if this node is already in the hierarchy
    auto iter = hierarchy.find(node.GetNodeId());
    if (iter == hierarchy.end())
        {
        if (parentIter != hierarchy.end())
            {
            // if this node has parent, add this as parent's child
            parentIter->second.push_back(&node);
            }
        // insert this node into the hierarchy
        return hierarchy.Insert(node.GetNodeId(), bvector<NavNodeCPtr>()).first;
        }
    return iter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static NodesPathElement GetPath(NavNodeCR root, bmap<uint64_t, bvector<NavNodeCPtr>> const& hierarchy, size_t index)
    {
    NodesPathElement node(root, index);
    auto iter = hierarchy.find(root.GetNodeId());
    for (size_t i = 0; i < iter->second.size(); i++)
        node.GetChildren().push_back(GetPath(*iter->second[i], hierarchy, i));
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bvector<NodesPathElement>> IECPresentationManager::GetFilteredNodesPaths(ECDbCR db, Utf8CP filterText, JsonValueCR options)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return bvector<NodesPathElement>();
        }
    return _GetFilteredNodes(*connection, filterText, options).then([&](bvector<NavNodeCPtr> filteredNodes)
        {
        bvector<NavNodeCPtr> roots;
        bmap<uint64_t, bvector<NavNodeCPtr>> hierarchy;
        for (NavNodeCPtr node : filteredNodes)
            CreateHierarchy(*this, db, *node, hierarchy, roots);

        size_t index = 0;
        bvector<NodesPathElement> paths;
        for (NavNodeCPtr const& root : roots)
            paths.push_back(GetPath(*root, hierarchy, index++));

        return paths;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void IECPresentationManager::RegisterImplementation(IECPresentationManager* impl) {s_instance = impl;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IECPresentationManager::SetSerializer(IECPresentationSerializer const* serializer)
    {
    DELETE_AND_CLEAR(s_serializer);
    s_serializer = serializer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IECPresentationSerializer const& IECPresentationManager::GetSerializer()
    {
    if (nullptr == s_serializer)
        {
        BeAssert(false);
        SetSerializer(new DefaultECPresentationSerializer());
        }
    return *s_serializer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IECPresentationManager::SetLocalizationProvider(ILocalizationProvider const* provider)
    {
    DELETE_AND_CLEAR(s_localizationProvider);
    s_localizationProvider = provider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ILocalizationProvider const& IECPresentationManager::GetLocalizationProvider()
    {
    if (nullptr == s_localizationProvider)
        {
        BeAssert(false);
        SetLocalizationProvider(new SQLangLocalizationProvider());
        }
    return *s_localizationProvider;
    }
