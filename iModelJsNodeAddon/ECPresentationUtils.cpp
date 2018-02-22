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
#ifdef wip_serialization
    return mgr.GetNode(ecdb, *NavNodeKey::FromJson(params["nodeKey"]));
#else
    JsonValueCR json = params["nodeKey"];
    Utf8CP type = json["type"].asCString();
    bvector<Utf8String> path;
    for (JsonValueCR pathElement : json["pathFromRoot"])
        path.push_back(pathElement.asString());
    ECClassId classId = ECClassId(BeJsonUtilities::UInt64FromValue(json["classId"]));
    NavNodeKeyCPtr key;
    if (0 == strcmp(NAVNODE_TYPE_ECInstanceNode, type))
        {
        ECInstanceId instanceId = ECInstanceId(BeJsonUtilities::UInt64FromValue(json["instanceId"]));
        key = ECInstanceNodeKey::Create(classId, instanceId, path);
        }
    else
        {
        key = NavNodeKey::Create(type, path, classId);
        }
    return mgr.GetNode(ecdb, *key);
#endif
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
static KeySetPtr GetKeys(JsonValueCR params)
    {
    if (!params.isMember("keys"))
        return KeySet::Create();

    JsonValueCR keysJson = params["keys"];
#ifdef wip_serialization
    if (!keysJson.isObject())
        return KeySet::Create();
    return KeySet::FromJson(keysJson);
#else
    if (!keysJson.isArray())
        return KeySet::Create();
    bvector<ECInstanceKey> instanceKeys;
    for (Json::ArrayIndex i = 0; i < keysJson.size(); ++i)
        {
        ECInstanceKey key(ECClassId(BeJsonUtilities::UInt64FromValue(keysJson[i]["classId"])), ECInstanceId(BeJsonUtilities::UInt64FromValue(keysJson[i]["instanceId"])));
        instanceKeys.push_back(key);
        }
    return KeySet::Create(instanceKeys);
#endif
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

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2018
+===============+===============+===============+===============+===============+======*/
struct DescriptorOverrideHelper
{
private:
    JsonValueCR m_overridesJson;
private:
    Utf8CP GetSortingFieldName() const { return m_overridesJson["sortingFieldName"].asCString(); }
    SortDirection GetSortDirection() const { return (SortDirection)m_overridesJson["sortDirection"].asInt(); }
    int GetContentFlags() const { return m_overridesJson["contentFlags"].asInt(); }
    Utf8CP GetFilterExpression() const { return m_overridesJson["filterExpression"].isNull() ? "" : m_overridesJson["filterExpression"].asCString(); }
    bvector<Utf8String> GetHiddenFieldNames() const
        {
        bvector<Utf8String> names;
        JsonValueCR arr = m_overridesJson["hiddenFieldNames"];
        for (Json::ArrayIndex i = 0; i < arr.size(); ++i)
            names.push_back(arr[i].asCString());
        return names;
        }
public:
    DescriptorOverrideHelper(JsonValueCR json) : m_overridesJson(json) {}
    Utf8CP GetDisplayType() const { return m_overridesJson["displayType"].asCString(); }
    ContentDescriptorCPtr GetOverridenDescriptor(ContentDescriptorCR defaultDescriptor) const
        {
        if (!defaultDescriptor.GetPreferredDisplayType().Equals(GetDisplayType()))
            {
            BeAssert(false);
            return &defaultDescriptor;
            }
        ContentDescriptorPtr descriptorCopy = ContentDescriptor::Create(defaultDescriptor);
        descriptorCopy->SetSortingField(GetSortingFieldName());
        descriptorCopy->SetSortDirection(GetSortDirection());
        descriptorCopy->SetContentFlags(GetContentFlags());
        descriptorCopy->SetFilterExpression(GetFilterExpression());
        bvector<Utf8String> hiddenFieldNames = GetHiddenFieldNames();
        for (Utf8StringCR name : hiddenFieldNames)
            descriptorCopy->RemoveField(name.c_str());
        return descriptorCopy;
        }
};

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
    ContentDescriptorCPtr descriptor = manager.GetContentDescriptor(db, GetDisplayType(params), *GetKeys(params), nullptr, GetManagerOptions(params)).get();
    if (descriptor.IsValid())
        response = descriptor->AsJson();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::GetContent(IECPresentationManagerR manager, ECDbR db, JsonValueCR params, rapidjson::Document& response)
    {
    DescriptorOverrideHelper helper(params["descriptorOverrides"]);
    ContentDescriptorCPtr descriptor = manager.GetContentDescriptor(db, helper.GetDisplayType(), *GetKeys(params), nullptr, GetManagerOptions(params)).get();
    if (descriptor.IsNull())
        return;

    ContentDescriptorCPtr overridenDescriptor = helper.GetOverridenDescriptor(*descriptor);
    ContentCPtr content = manager.GetContent(*overridenDescriptor, GetPageOptions(params)).get();
    response = content->AsJson();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::GetContentSetSize(IECPresentationManagerR manager, ECDbR db, JsonValueCR params, rapidjson::Document& response)
    {
    DescriptorOverrideHelper helper(params["descriptorOverrides"]);
    ContentDescriptorCPtr descriptor = manager.GetContentDescriptor(db, helper.GetDisplayType(), *GetKeys(params), nullptr, GetManagerOptions(params)).get();
    if (descriptor.IsNull())
        return;

    ContentDescriptorCPtr overridenDescriptor = helper.GetOverridenDescriptor(*descriptor);
    size_t size = manager.GetContentSetSize(*overridenDescriptor).get();
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