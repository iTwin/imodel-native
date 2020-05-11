/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/IECPresentationManager.h>
#include <ECPresentation/DefaultECPresentationSerializer.h>
#include "../Localization/Xliffs/ECPresentation.xliff.h"
#include "ValueHelpers.h"

IECPresentationSerializer const* IECPresentationManager::s_serializer = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<DataContainer<NavNodeCPtr>> IECPresentationManager::GetRootNodes(ECDbCR db, PageOptionsCR pageOptions, JsonValueCR options, PresentationRequestContextCR context)
    {
    return _GetRootNodes(db, pageOptions, options, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<size_t> IECPresentationManager::GetRootNodesCount(ECDbCR db, JsonValueCR options, PresentationRequestContextCR context)
    {
    return _GetRootNodesCount(db, options, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<DataContainer<NavNodeCPtr>> IECPresentationManager::GetChildren(ECDbCR db, NavNodeCR node, PageOptionsCR pageOptions, JsonValueCR options, PresentationRequestContextCR context)
    {
    return _GetChildren(db, node, pageOptions, options, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<size_t> IECPresentationManager::GetChildrenCount(ECDbCR db, NavNodeCR node, JsonValueCR options, PresentationRequestContextCR context)
    {
    return _GetChildrenCount(db, node, options, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NavNodeCPtr> IECPresentationManager::GetParent(ECDbCR db, NavNodeCR node, JsonValueCR options, PresentationRequestContextCR context)
    {
    return _GetParent(db, node, options, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bvector<SelectClassInfo>> IECPresentationManager::GetContentClasses(ECDbCR db, Utf8CP preferredDisplayType, int contentFlags, bvector<ECClassCP> const& input, JsonValueCR options, PresentationRequestContextCR context)
    {
    return _GetContentClasses(db, preferredDisplayType, contentFlags, input, options, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ContentDescriptorCPtr> IECPresentationManager::GetContentDescriptor(ECDbCR db, Utf8CP preferredDisplayType, int contentFlags, KeySetCR inputKeys, SelectionInfo const* selectionInfo, JsonValueCR options, PresentationRequestContextCR context)
    {
    return _GetContentDescriptor(db, preferredDisplayType, contentFlags, inputKeys, selectionInfo, options, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ContentCPtr> IECPresentationManager::GetContent(ContentDescriptorCR descriptor, PageOptionsCR pageOptions, PresentationRequestContextCR context)
    {
    return _GetContent(descriptor, pageOptions, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<size_t> IECPresentationManager::GetContentSetSize(ContentDescriptorCR descriptor, PresentationRequestContextCR context)
    {
    return _GetContentSetSize(descriptor, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<LabelDefinitionCPtr> IECPresentationManager::GetDisplayLabel(ECDbCR db, ECInstanceKeyCR key, JsonValueCR extendedOptions, PresentationRequestContextCR context)
    {
    ECClassCP ecClass = db.Schemas().GetClass(key.GetClassId());
    KeySetPtr keys = KeySet::Create({ECClassInstanceKey(ecClass, key.GetInstanceId())});
    return GetDisplayLabel(db, *keys, extendedOptions, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<LabelDefinitionCPtr> IECPresentationManager::GetDisplayLabel(ECDbCR db, KeySetCR keys, JsonValueCR extendedOptions, PresentationRequestContextCR context)
    {
    return _GetDisplayLabel(db, keys, extendedOptions, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<PagedDataContainer<DisplayValueGroupCPtr>> IECPresentationManager::GetDistinctValues(ContentDescriptorCR descriptor, Utf8StringCR fieldName, PageOptionsCR pageOptions, PresentationRequestContextCR context)
    {
    return _GetDistinctValues(descriptor, fieldName, pageOptions, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NavNodeCPtr> IECPresentationManager::GetNode(ECDbCR db, NavNodeKeyCR nodeKey, JsonValueCR options, PresentationRequestContextCR context)
    {
    return _GetNode(db, nodeKey, options, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NodesPathElement> IECPresentationManager::GetNodePath(ECDbCR db, bvector<ECInstanceKey> const& keyPath, JsonValueCR options, PresentationRequestContextCR context)
    {
    return _GetNodePath(db, keyPath, options, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bvector<NodesPathElement>> IECPresentationManager::GetNodePaths(ECDbCR db, bvector<bvector<ECInstanceKey>> const& keyPaths, int64_t markedIndex, JsonValueCR options, PresentationRequestContextCR context)
    {
    return _GetNodePaths(db, keyPaths, markedIndex, options, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bvector<NodesPathElement>> IECPresentationManager::GetFilteredNodePaths(ECDbCR db, Utf8CP filterText, JsonValueCR options, PresentationRequestContextCR context)
    {
    return _GetFilteredNodePaths(db, filterText, options, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bvector<NavNodeCPtr>> IECPresentationManager::GetFilteredNodes(ECDbCR db, Utf8CP filterText, JsonValueCR options, PresentationRequestContextCR context)
    {
    return _GetFilteredNodes(db, filterText, options, context);
    }

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
