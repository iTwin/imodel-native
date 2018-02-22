/*--------------------------------------------------------------------------------------+
|
|     $Source: ECPresentationUtils.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECPresentationUtils.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenECPresentationManager* ECPresentationUtils::CreatePresentationManager(IConnectionManagerR connections, Dgn::DgnPlatformLib::Host::IKnownLocationsAdmin& locations)
    {
    BeFileName assetsDir = locations.GetDgnPlatformAssetsDirectory();
    BeFileName tempDir = locations.GetLocalTempDirectoryBaseName();
    RulesDrivenECPresentationManager::Paths paths(assetsDir, tempDir);
    RulesDrivenECPresentationManager* manager = new RulesDrivenECPresentationManager(connections, paths);

    BeFileName supplementalsDirectory = BeFileName(assetsDir).AppendToPath(L"PresentationRules");
    manager->GetLocaters().RegisterLocater(*SupplementalRuleSetLocater::Create(*DirectoryRuleSetLocater::Create(supplementalsDirectory.GetNameUtf8().c_str())));

    return manager;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::SetupRulesetDirectories(RulesDrivenECPresentationManager& manager, bvector<Utf8String> const& directories)
    {
    Utf8String joinedDirectories = BeStringUtilities::Join(directories, ";");
    manager.GetLocaters().RegisterLocater(*DirectoryRuleSetLocater::Create(joinedDirectories.c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static JsonValueCR GetManagerOptions(JsonValueCR params)
    {
    if (!params.isMember("options") || !params["options"].isObject())
        {
        static Json::Value s_default;
        return s_default;
        }
    return params["options"];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static PageOptions GetPageOptions(JsonValueCR params)
    {
    PageOptions pageOptions;
    if (params.isMember("pageOptions"))
        {
        pageOptions.SetPageStart((size_t)params["pageOptions"]["pageStart"].asUInt64());
        pageOptions.SetPageSize((size_t)params["pageOptions"]["pageSize"].asUInt64());
        }
    return pageOptions;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static folly::Future<NavNodeCPtr> GetNode(IECPresentationManager& mgr, ECDbR ecdb, JsonValueCR params)
    {
    return mgr.GetNode(ecdb, BeJsonUtilities::UInt64FromValue(params["nodeId"]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8CP GetDisplayType(JsonValueCR params)
    {
    return params.isMember("displayType") ? params["displayType"].asCString() : ContentDisplayType::Undefined;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static INavNodeKeysContainerCPtr GetKeys(JsonValueCR params)
    {
    if (!params.isMember("keys") || !params["keys"].isArray())
        return NavNodeKeyListContainer::Create();

    JsonValueCR keysJson = params["keys"];
    NavNodeKeyList keys;
    for (Json::ArrayIndex i = 0; i < keysJson.size(); ++i)
        {
        ECClassId classId(BeJsonUtilities::UInt64FromValue(keysJson[i]["classId"]));
        ECInstanceId instanceId(BeJsonUtilities::UInt64FromValue(keysJson[i]["instanceId"]));
        keys.push_back(ECInstanceNodeKey::Create(classId, instanceId));
        }
    return NavNodeKeyListContainer::Create(keys);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::GetRootNodesCount(IECPresentationManagerR manager, ECDbR db, JsonValueCR params, rapidjson::Document& response)
    {
    size_t count = manager.GetRootNodesCount(db, GetManagerOptions(params)).get();
    response.SetInt64((int64_t)count);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::GetRootNodes(IECPresentationManagerR manager, ECDbR db, JsonValueCR params, rapidjson::Document& response)
    {
    response.SetArray();
    DataContainer<NavNodeCPtr> nodes = manager.GetRootNodes(db, GetPageOptions(params), GetManagerOptions(params)).get();
    for (NavNodeCPtr const& node : nodes)
        response.PushBack(node->AsJson(&response.GetAllocator()), response.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::GetChildrenCount(IECPresentationManagerR manager, ECDbR db, JsonValueCR params, rapidjson::Document& response)
    {
    NavNodeCPtr parentNode = GetNode(manager, db, params).get();
    size_t count = manager.GetChildrenCount(db, *parentNode, GetManagerOptions(params)).get();
    response.SetInt64((int64_t)count);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::GetChildren(IECPresentationManagerR manager, ECDbR db, JsonValueCR params, rapidjson::Document& response)
    {
    NavNodeCPtr parentNode = GetNode(manager, db, params).get();
    response.SetArray();
    DataContainer<NavNodeCPtr> nodes = manager.GetChildren(db, *parentNode, GetPageOptions(params), GetManagerOptions(params)).get();
    for (NavNodeCPtr const& node : nodes)
        response.PushBack(node->AsJson(&response.GetAllocator()), response.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::GetNodePaths(IECPresentationManagerR manager, ECDbR db, JsonValueCR params, rapidjson::Document& response)
    {
    BeAssert(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::GetFilteredNodesPaths(IECPresentationManagerR manager, ECDbR db, JsonValueCR params, rapidjson::Document& response)
    {
    BeAssert(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::GetContentDescriptor(IECPresentationManagerR manager, ECDbR db, JsonValueCR params, rapidjson::Document& response)
    {
    SelectionInfo selection("Undefined", false, *GetKeys(params));
    ContentDescriptorCPtr descriptor = manager.GetContentDescriptor(db, GetDisplayType(params), selection, GetManagerOptions(params)).get();
    if (descriptor.IsValid())
        response = descriptor->AsJson();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::GetContent(IECPresentationManagerR manager, ECDbR db, JsonValueCR params, rapidjson::Document& response)
    {
    SelectionInfo selection("Undefined", false, *GetKeys(params));
    ContentDescriptorCPtr descriptor = manager.GetContentDescriptor(db, GetDisplayType(params), selection, GetManagerOptions(params)).get();
    if (descriptor.IsNull())
        return;

    ContentCPtr content = manager.GetContent(db, *descriptor, selection, GetPageOptions(params), GetManagerOptions(params)).get();
    response = content->AsJson();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::GetContentSetSize(IECPresentationManagerR manager, ECDbR db, JsonValueCR params, rapidjson::Document& response)
    {
    SelectionInfo selection("Undefined", false, *GetKeys(params));
    ContentDescriptorCPtr descriptor = manager.GetContentDescriptor(db, GetDisplayType(params), selection, GetManagerOptions(params)).get();
    if (descriptor.IsNull())
        return;

    size_t size = manager.GetContentSetSize(db, *descriptor, selection, GetManagerOptions(params)).get();
    response.SetUint64(size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::GetDistinctValues(IECPresentationManagerR manager, ECDbR db, JsonValueCR params, rapidjson::Document& response)
    {
    BeAssert(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::SaveValueChange(IECPresentationManagerR manager, ECDbR db, JsonValueCR params, rapidjson::Document& response)
    {
    BeAssert(false);
    }