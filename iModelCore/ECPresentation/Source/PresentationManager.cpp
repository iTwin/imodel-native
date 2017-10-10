/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/PresentationManager.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/IECPresentationManager.h>
#include "../Localization/Xliffs/ECPresentation.xliff.h"
#include "ValueHelpers.h"

IECPresentationManager* IECPresentationManager::s_instance = nullptr;
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
DataContainer<NavNodeCPtr> IECPresentationManager::GetRootNodes(ECDbR connection, PageOptionsCR pageOptions, JsonValueCR options)
    {
    return _GetRootNodes(connection, pageOptions, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t IECPresentationManager::GetRootNodesCount(ECDbR connection, JsonValueCR options)
    {
    return _GetRootNodesCount(connection, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DataContainer<NavNodeCPtr> IECPresentationManager::GetChildren(ECDbR connection, NavNodeCR node, PageOptionsCR pageOptions, JsonValueCR options)
    {
    return _GetChildren(connection, node, pageOptions, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t IECPresentationManager::GetChildrenCount(ECDbR connection, NavNodeCR node, JsonValueCR options)
    {
    return _GetChildrenCount(connection, node, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeCPtr IECPresentationManager::GetParent(ECDbR connection, NavNodeCR node, JsonValueCR options)
    {
    return _GetParent(connection, node, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<SelectClassInfo> IECPresentationManager::GetContentClasses(ECDbR connection, Utf8CP preferredDisplayType, bvector<ECClassCP> const& input, JsonValueCR options)
    {
    return _GetContentClasses(connection, preferredDisplayType, input, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorCPtr IECPresentationManager::GetContentDescriptor(ECDbR connection, Utf8CP preferredDisplayType, SelectionInfo const& selectionInfo, JsonValueCR options)
    {
    return _GetContentDescriptor(connection, preferredDisplayType, selectionInfo, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentCPtr IECPresentationManager::GetContent(ECDbR connection, ContentDescriptorCR descriptor, SelectionInfo const& selectionInfo, PageOptionsCR pageOptions, JsonValueCR options)
    {
    return _GetContent(connection, descriptor, selectionInfo, pageOptions, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t IECPresentationManager::GetContentSetSize(ECDbR connection, ContentDescriptorCR descriptor, SelectionInfo const& selectionInfo, JsonValueCR options)
    {
    return _GetContentSetSize(connection, descriptor, selectionInfo, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeCPtr IECPresentationManager::GetNode(ECDbR connection, uint64_t nodeId) {return _GetNode(connection, nodeId);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void IECPresentationManager::NotifyNodeChecked(ECDbR connection, uint64_t nodeId) {return _OnNodeChecked(connection, nodeId);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void IECPresentationManager::NotifyNodeUnchecked(ECDbR connection, uint64_t nodeId) {return _OnNodeUnchecked(connection, nodeId);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void IECPresentationManager::NotifyNodeExpanded(ECDbR connection, uint64_t nodeId) {return _OnNodeExpanded(connection, nodeId);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void IECPresentationManager::NotifyNodeCollapsed(ECDbR connection, uint64_t nodeId) {return _OnNodeCollapsed(connection, nodeId);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECPresentationManager::HasChild(ECDbR connection, NavNodeCR parentNode, NavNodeKeyCR childNodeKey, JsonValueCR extendedOptions) {return _HasChild(connection, parentNode, childNodeKey, extendedOptions);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
NodesPathElement IECPresentationManager::FindNode(ECDbR connection, NavNodeCP parentNode, NavNodeKeyCR lookupKey, JsonValueCR extendedOptions)
    {
    DataContainer<NavNodeCPtr> nodes = (nullptr == parentNode)
        ? GetRootNodes(connection, PageOptions(), extendedOptions)
        : GetChildren(connection, *parentNode, PageOptions(), extendedOptions);
    for (size_t i = 0; i < nodes.GetSize(); ++i)
        {
        NavNodeCPtr node = nodes[i];
        if (0 == node->GetKey().Compare(lookupKey))
            return NodesPathElement(*node, i);
        if (HasChild(connection, *node, lookupKey, extendedOptions))
            return NodesPathElement(*node, i);
        }
    return NodesPathElement();
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
NodesPathElement IECPresentationManager::GetNodesPath(ECDbR connection, NavNodeKeyPath const& keyPath, JsonValueCR extendedOptions)
    {
    NodesPathElement path;
    NodesPathElement* curr = &path;
    NavNodeCP parent = nullptr;
    for (size_t i = 0; i < keyPath.size(); ++i)
        {
        while (true)
            {
            NavNodeKeyCR key = *keyPath[i];
            NodesPathElement el = FindNode(connection, parent, key, extendedOptions);
            if (!el.GetNode().IsValid())
                {
                BeAssert(false && "Provided nodes path doesn't exist in the hierarchy");
                return NodesPathElement();
                }

            parent = el.GetNode().get();
            curr = AddToPath(*curr, std::move(el));

            if (0 == parent->GetKey().Compare(key))
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
    if (lhs.GetNode() == rhs.GetNode())
        {
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
bvector<NodesPathElement> IECPresentationManager::GetNodesPath(ECDbR connection, bvector<NavNodeKeyPath> const& keyPaths, int64_t markedIndex, JsonValueCR extendedOptions)
    {
    bvector<NodesPathElement> paths;
    for (size_t i = 0; i < keyPaths.size(); ++i)
        {
        NavNodeKeyPath const& keyPath = keyPaths[i];
        NodesPathElement path = GetNodesPath(connection, keyPath, extendedOptions);
        if (path.GetNode().IsValid())
            {
            if (markedIndex == (int64_t)i)
                MarkLeaves(path);
            AppendPath(paths, path);
            }
        }
    return paths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceChangeResult IECPresentationManager::SaveValueChange(ECDbR connection, ChangedECInstanceInfo const& instanceInfo, Utf8CP propertyAccessor, ECValueCR value, JsonValueCR extendedOptions)
    {
    bvector<ECInstanceChangeResult> result = SaveValueChange(connection, bvector<ChangedECInstanceInfo>{instanceInfo}, propertyAccessor, value, extendedOptions);
    if (result.empty())
        {
        BeAssert(false);
        return ECInstanceChangeResult::Error(L10N::GetString(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()));
        }
    return result[0];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECInstanceChangeResult> IECPresentationManager::SaveValueChange(ECDbR connection, bvector<ChangedECInstanceInfo> const& instanceInfos, Utf8CP propertyAccessor, ECValueCR value, JsonValueCR extendedOptions)
    {
    return _SaveValueChange(connection, instanceInfos, propertyAccessor, value, extendedOptions);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceChangeResult IECPresentationManager::SaveValueChange(ECDbR connection, ChangedECInstanceInfo const& instanceInfo, Utf8CP propertyAccessor, JsonValueCR value, JsonValueCR extendedOptions)
    {
    bvector<ECInstanceChangeResult> result = SaveValueChange(connection, bvector<ChangedECInstanceInfo>{instanceInfo}, propertyAccessor, value, extendedOptions);
    if (result.empty())
        {
        BeAssert(false);
        return ECInstanceChangeResult::Error(L10N::GetString(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()));
        }
    return result[0];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECInstanceChangeResult> IECPresentationManager::SaveValueChange(ECDbR connection, bvector<ChangedECInstanceInfo> const& instanceInfos, Utf8CP propertyAccessor, JsonValueCR value, JsonValueCR extendedOptions)
    {
    // note: assumes that all changed instances in this field use the same property
    ECClassCR changedClass = instanceInfos.front().GetChangedInstanceClass();
    ECPropertyCP prop = changedClass.GetPropertyP(propertyAccessor);
    if (nullptr == prop || (!prop->GetIsPrimitive() && !prop->GetIsNavigation()))
        {
        BeAssert(false && "Failed to determine the changed property or it's not primitive");
        return bvector<ECInstanceChangeResult>(instanceInfos.size(), 
            ECInstanceChangeResult::Error(L10N::GetString(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::ERROR_General_Unknown())));
        }
    ECValue ecValue = ValueHelpers::GetECValueFromJson(*prop, value);
    return SaveValueChange(connection, instanceInfos, propertyAccessor, ecValue, extendedOptions);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void IECPresentationManager::RegisterImplementation(IECPresentationManager* impl) {s_instance = impl;}
