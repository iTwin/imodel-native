/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/IECPresentationManager.h>
#include <ECPresentation/DefaultECPresentationSerializer.h>
#include "../Localization/Xliffs/ECPresentation.xliff.h"
#include "ValueHelpers.h"

IECPresentationManager* IECPresentationManager::s_instance = nullptr;
IECPresentationSerializer const* IECPresentationManager::s_serializer = nullptr;
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
folly::Future<DataContainer<NavNodeCPtr>> IECPresentationManager::GetRootNodes(ECDbCR db, PageOptionsCR pageOptions, JsonValueCR options, PresentationTaskNotificationsContextCR notificationsContext)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return DataContainer<NavNodeCPtr>();
        }
    return _GetRootNodes(*connection, pageOptions, options, notificationsContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<size_t> IECPresentationManager::GetRootNodesCount(ECDbCR db, JsonValueCR options, PresentationTaskNotificationsContextCR notificationsContext)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return 0;
        }
    return _GetRootNodesCount(*connection, options, notificationsContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<DataContainer<NavNodeCPtr>> IECPresentationManager::GetChildren(ECDbCR db, NavNodeCR node, PageOptionsCR pageOptions, JsonValueCR options, PresentationTaskNotificationsContextCR notificationsContext)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return DataContainer<NavNodeCPtr>();
        }
    return _GetChildren(*connection, node, pageOptions, options, notificationsContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<size_t> IECPresentationManager::GetChildrenCount(ECDbCR db, NavNodeCR node, JsonValueCR options, PresentationTaskNotificationsContextCR notificationsContext)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return 0;
        }
    return _GetChildrenCount(*connection, node, options, notificationsContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NavNodeCPtr> IECPresentationManager::GetParent(ECDbCR db, NavNodeCR node, JsonValueCR options, PresentationTaskNotificationsContextCR notificationsContext)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return folly::makeFuture(NavNodeCPtr(nullptr));
        }
    return _GetParent(*connection, node, options, notificationsContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bvector<SelectClassInfo>> IECPresentationManager::GetContentClasses(ECDbCR db, Utf8CP preferredDisplayType, int contentFlags, bvector<ECClassCP> const& input, JsonValueCR options, PresentationTaskNotificationsContextCR notificationsContext)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return bvector<SelectClassInfo>();
        }
    return _GetContentClasses(*connection, preferredDisplayType, contentFlags, input, options, notificationsContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ContentDescriptorCPtr> IECPresentationManager::GetContentDescriptor(ECDbCR db, Utf8CP preferredDisplayType, int contentFlags, KeySetCR inputKeys, SelectionInfo const* selectionInfo, JsonValueCR options, PresentationTaskNotificationsContextCR notificationsContext)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return folly::makeFuture(ContentDescriptorCPtr(nullptr));
        }
    return _GetContentDescriptor(*connection, preferredDisplayType, contentFlags, inputKeys, selectionInfo, options, notificationsContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ContentCPtr> IECPresentationManager::GetContent(ContentDescriptorCR descriptor, PageOptionsCR pageOptions, PresentationTaskNotificationsContextCR notificationsContext)
    {
    return _GetContent(descriptor, pageOptions, notificationsContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<size_t> IECPresentationManager::GetContentSetSize(ContentDescriptorCR descriptor, PresentationTaskNotificationsContextCR notificationsContext)
    {
    return _GetContentSetSize(descriptor, notificationsContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<Utf8String> IECPresentationManager::GetDisplayLabel(ECDbCR db, ECInstanceKeyCR key, PresentationTaskNotificationsContextCR notificationsContext)
    {
    ECClassCP ecClass = db.Schemas().GetClass(key.GetClassId());
    KeySetPtr keys = KeySet::Create({ECClassInstanceKey(ecClass, key.GetInstanceId())});
    return GetDisplayLabel(db, *keys, notificationsContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<Utf8String> IECPresentationManager::GetDisplayLabel(ECDbCR db, KeySetCR keys, PresentationTaskNotificationsContextCR notificationsContext)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return folly::makeFuture(Utf8String());
        }
    return _GetDisplayLabel(*connection, keys, notificationsContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NavNodeCPtr> IECPresentationManager::GetNode(ECDbCR db, NavNodeKeyCR nodeKey, JsonValueCR options, PresentationTaskNotificationsContextCR notificationsContext)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return folly::makeFuture(NavNodeCPtr(nullptr));
        }
    return _GetNode(*connection, nodeKey, options, notificationsContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> IECPresentationManager::NotifyNodeChecked(ECDbCR db, NavNodeKeyCR nodeKey, JsonValueCR options, PresentationTaskNotificationsContextCR notificationsContext)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return folly::unit;
        }
    return _OnNodeChecked(*connection, nodeKey, options, notificationsContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> IECPresentationManager::NotifyNodeUnchecked(ECDbCR db, NavNodeKeyCR nodeKey, JsonValueCR options, PresentationTaskNotificationsContextCR notificationsContext)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return folly::unit;
        }
    return _OnNodeUnchecked(*connection, nodeKey, options, notificationsContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> IECPresentationManager::NotifyNodeExpanded(ECDbCR db, NavNodeKeyCR nodeKey, JsonValueCR options, PresentationTaskNotificationsContextCR notificationsContext)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return folly::unit;
        }
    return _OnNodeExpanded(*connection, nodeKey, options, notificationsContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> IECPresentationManager::NotifyNodeCollapsed(ECDbCR db, NavNodeKeyCR nodeKey, JsonValueCR options, PresentationTaskNotificationsContextCR notificationsContext)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return folly::unit;
        }
    return _OnNodeCollapsed(*connection, nodeKey, options, notificationsContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> IECPresentationManager::NotifyAllNodesCollapsed(ECDbCR db, JsonValueCR options, PresentationTaskNotificationsContextCR notificationsContext)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return folly::unit;
        }
    return _OnAllNodesCollapsed(*connection, options, notificationsContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NodesPathElement> IECPresentationManager::FindNode(ECDbCR db, NavNodeCP parentNode, ECInstanceKeyCR lookupKey, JsonValueCR extendedOptions, PresentationTaskNotificationsContextCR notificationsContext)
    {
    folly::Future<DataContainer<NavNodeCPtr>> nodesFuture = (nullptr == parentNode)
        ? GetRootNodes(db, PageOptions(), extendedOptions, notificationsContext)
        : GetChildren(db, *parentNode, PageOptions(), extendedOptions, notificationsContext);
    return nodesFuture.then([this, &db, lookupKey, extendedOptions, notificationsContext](DataContainer<NavNodeCPtr> nodes) -> NodesPathElement
        {
        notificationsContext.OnTaskStart();
        IConnectionCPtr connection = GetConnections().GetConnection(db);
        for (size_t i = 0; i < nodes.GetSize(); ++i)
            {
            NavNodeCPtr node = nodes[i];
            if (node->GetKey()->AsECInstanceNodeKey() && node->GetKey()->AsECInstanceNodeKey()->GetInstanceKey() == lookupKey)
                return NodesPathElement(*node, i);
            if (_HasChild(*connection, *node, lookupKey, extendedOptions, notificationsContext).get())
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
folly::Future<NodesPathElement> IECPresentationManager::GetNodesPath(ECDbCR db, bvector<ECInstanceKey> const& keyPath, JsonValueCR extendedOptions, PresentationTaskNotificationsContextCR notificationsContext)
    {
    NodesPathElement path;
    NodesPathElement* curr = &path;
    NavNodeCP parent = nullptr;
    for (size_t i = 0; i < keyPath.size(); ++i)
        {
        while (true)
            {
            ECInstanceKeyCR key = keyPath[i];
            NodesPathElement el = FindNode(db, parent, key, extendedOptions, notificationsContext).get();
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
folly::Future<bvector<NodesPathElement>> IECPresentationManager::GetNodesPath(ECDbCR db, bvector<bvector<ECInstanceKey>> const& keyPaths, int64_t markedIndex, JsonValueCR extendedOptions, PresentationTaskNotificationsContextCR notificationsContext)
    {
    bvector<folly::Future<NodesPathElement>> pathFutures;
    for (size_t i = 0; i < keyPaths.size(); ++i)
        {
        bvector<ECInstanceKey> const& keyPath = keyPaths[i];
        pathFutures.push_back(GetNodesPath(db, keyPath, extendedOptions, notificationsContext));
        }
    return folly::collect(pathFutures).then([markedIndex, notificationsContext](std::vector<NodesPathElement> paths) -> bvector<NodesPathElement>
        {
        notificationsContext.OnTaskStart();
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
folly::Future<ECInstanceChangeResult> IECPresentationManager::SaveValueChange(ECDbCR db, ChangedECInstanceInfo const& instanceInfo, Utf8CP propertyAccessor, ECValueCR value, JsonValueCR extendedOptions, PresentationTaskNotificationsContextCR notificationsContext)
    {
    return SaveValueChange(db, bvector<ChangedECInstanceInfo>{instanceInfo}, propertyAccessor, value, extendedOptions, notificationsContext)
        .then([this](bvector<ECInstanceChangeResult> result) -> ECInstanceChangeResult
        {
        if (result.empty())
            {
            BeAssert(false);
            return ECInstanceChangeResult::Error(GetLocalizationProvider()->GetString("", bvector<Utf8CP>{ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()}));
            }
        return result[0];
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bvector<ECInstanceChangeResult>> IECPresentationManager::SaveValueChange(ECDbCR db, bvector<ChangedECInstanceInfo> const& instanceInfos, Utf8CP propertyAccessor, ECValueCR value, JsonValueCR extendedOptions, PresentationTaskNotificationsContextCR notificationsContext)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return bvector<ECInstanceChangeResult>();
        }
    return _SaveValueChange(*connection, instanceInfos, propertyAccessor, value, extendedOptions, notificationsContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECInstanceChangeResult> IECPresentationManager::SaveValueChange(ECDbCR db, ChangedECInstanceInfo const& instanceInfo, Utf8CP propertyAccessor, JsonValueCR value, JsonValueCR extendedOptions, PresentationTaskNotificationsContextCR notificationsContext)
    {
    return SaveValueChange(db, bvector<ChangedECInstanceInfo>{instanceInfo}, propertyAccessor, value, extendedOptions, notificationsContext)
        .then([this](bvector<ECInstanceChangeResult> result) -> ECInstanceChangeResult
        {
        if (result.empty())
            {
            BeAssert(false);
            return ECInstanceChangeResult::Error(GetLocalizationProvider()->GetString("", bvector<Utf8CP>{ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()}));
            }
        return result[0];
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bvector<ECInstanceChangeResult>> IECPresentationManager::SaveValueChange(ECDbCR db, bvector<ChangedECInstanceInfo> const& instanceInfos, Utf8CP propertyAccessor, JsonValueCR value, JsonValueCR extendedOptions, PresentationTaskNotificationsContextCR notificationsContext)
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
            ECInstanceChangeResult::Error(IECPresentationManager::GetLocalizationProvider()->GetString("", bvector<Utf8CP>{ECPresentationL10N::GetNameSpace(), ECPresentationL10N::ERROR_General_Unknown()})));
        }
    ECValue ecValue = ValueHelpers::GetECValueFromJson(*prop, value);
    return SaveValueChange(db, instanceInfos, propertyAccessor, ecValue, extendedOptions, notificationsContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bmap<uint64_t, bvector<NavNodeCPtr>>::iterator CreateHierarchy(IECPresentationManager& mgr, ECDbCR db, NavNodeCR node, bmap<uint64_t, bvector<NavNodeCPtr>>& hierarchy, bvector<NavNodeCPtr>& roots, JsonValueCR jsonOptions)
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
            NavNodeCPtr parent = mgr.GetParent(db, node, jsonOptions).get();
            if (parent.IsValid())
                parentIter = CreateHierarchy(mgr, db, *parent, hierarchy, roots, jsonOptions);
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
* @bsimethod                                    Elonas.Seviakovas               09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static uint16_t CountFilterTextOccurances(NavNodeCR node, Utf8CP lowerFilterText)
    {
    if (Utf8String::IsNullOrEmpty(lowerFilterText))
        return 0;

    uint16_t occurances = 0;
    Utf8String lowerLabel(node.GetLabel().ToLower());
    size_t position = lowerLabel.find(lowerFilterText, 0);
    while (position != Utf8String::npos)
        {
        occurances++;
        position = lowerLabel.find(lowerFilterText, position + 1);
        }
    return occurances;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static NodesPathElement GetPath(NavNodeCR root, bmap<uint64_t, bvector<NavNodeCPtr>> const& hierarchy, size_t index, Utf8CP lowerFilterText, uint64_t& totalFilterOccurances)
    {
    uint16_t currentNodeOccurances = CountFilterTextOccurances(root, lowerFilterText);
    uint64_t totalChildrenOccurances = 0;

    NodesPathElement node(root, index);
    auto iter = hierarchy.find(root.GetNodeId());
    for (size_t i = 0; i < iter->second.size(); i++)
        {
        uint64_t branchChildrenOccurances = 0;
        node.GetChildren().push_back(GetPath(*iter->second[i], hierarchy, i, lowerFilterText, branchChildrenOccurances));
        totalChildrenOccurances += branchChildrenOccurances;
        }

    node.GetFilteringData().SetOccurances(currentNodeOccurances);
    node.GetFilteringData().SetChildrenOccurances(totalChildrenOccurances);

    totalFilterOccurances = totalChildrenOccurances + currentNodeOccurances;
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bvector<NodesPathElement>> IECPresentationManager::GetFilteredNodesPaths(ECDbCR db, Utf8CP filterText, JsonValueCR options, PresentationTaskNotificationsContextCR notificationsContext)
    {
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false && "Unknown connection");
        return bvector<NodesPathElement>();
        }

    Utf8String escapedString(filterText);
    escapedString.ReplaceAll("\\", "\\\\");
    escapedString.ReplaceAll("%", "\\%");
    escapedString.ReplaceAll("_", "\\_");

    return _GetFilteredNodes(*connection, escapedString.c_str(), options, notificationsContext)
        .then([&, notificationsContext, filterText = Utf8String(filterText).ToLower(), options](bvector<NavNodeCPtr> filteredNodes)
        {
        notificationsContext.OnTaskStart();

        bvector<NavNodeCPtr> roots;
        bmap<uint64_t, bvector<NavNodeCPtr>> hierarchy;
        for (NavNodeCPtr node : filteredNodes)
            CreateHierarchy(*this, db, *node, hierarchy, roots, options);

        size_t index = 0;
        bvector<NodesPathElement> paths;
        for (NavNodeCPtr const& root : roots)
            {
            uint64_t totalOccurances = 0;
            paths.push_back(GetPath(*root, hierarchy, index++, filterText.c_str(), totalOccurances));
            }

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
    DELETE_AND_CLEAR(m_localizationProvider);
    m_localizationProvider = provider;
    _OnLocalizationProviderChanged();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ILocalizationProvider const* IECPresentationManager::GetLocalizationProvider()
    {
    return m_localizationProvider;
    }
