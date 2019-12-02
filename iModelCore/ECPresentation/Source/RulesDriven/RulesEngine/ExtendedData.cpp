/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "ExtendedData.h"
#include "../../ValueHelpers.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodeExtendedData::SetChildrenArtifacts(bvector<NodeArtifacts> allArtifacts)
    {
    if (allArtifacts.empty() && !GetJson().HasMember(NAVNODE_EXTENDEDDATA_ChildrenArtifacts))
        return false;

    rapidjson::Value allArtifactsJson(rapidjson::kArrayType);
    for (NodeArtifacts const& nodeArtifacts : allArtifacts)
        {
        rapidjson::Value nodeArtifactsJson(rapidjson::kObjectType);
        for (auto const& entry : nodeArtifacts)
            {
            rapidjson::Value nodeArtifactJson(rapidjson::kObjectType);
            nodeArtifactJson.AddMember("type", entry.second.GetPrimitiveType(), GetAllocator());
            nodeArtifactJson.AddMember("value", ValueHelpers::GetJsonFromECValue(entry.second, &GetAllocator()), GetAllocator());
            nodeArtifactsJson.AddMember(rapidjson::Value(entry.first.c_str(), GetAllocator()), std::move(nodeArtifactJson), GetAllocator());
            }
        allArtifactsJson.PushBack(std::move(nodeArtifactsJson), GetAllocator());
        }
    AddMember(NAVNODE_EXTENDEDDATA_ChildrenArtifacts, std::move(allArtifactsJson));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NodeArtifacts> NavNodeExtendedData::GetChildrenArtifacts() const
    {
    bvector<NodeArtifacts> allArtifacts;
    if (!GetJson().HasMember(NAVNODE_EXTENDEDDATA_ChildrenArtifacts))
        return allArtifacts;

    RapidJsonValueCR allArtifactsJson = GetJson()[NAVNODE_EXTENDEDDATA_ChildrenArtifacts];
    if (!allArtifactsJson.IsArray())
        return allArtifacts;

    for (rapidjson::SizeType i = 0; i < allArtifactsJson.Size(); ++i)
        {
        RapidJsonValueCR nodeArtifactsJson = allArtifactsJson[i];
        if (!nodeArtifactsJson.IsObject())
            continue;

        NodeArtifacts nodeArtifacts;
        for (auto iter = nodeArtifactsJson.MemberBegin(); iter != nodeArtifactsJson.MemberEnd(); ++iter)
            {
            RapidJsonValueCR nodeArtifactJson = iter->value;
            if (!nodeArtifactJson.IsObject())
                continue;

            PrimitiveType type = (PrimitiveType)nodeArtifactJson["type"].GetInt();
            ECValue value = ValueHelpers::GetECValueFromJson(type, nodeArtifactJson["value"]);
            nodeArtifacts.Insert(iter->name.GetString(), value);
            }

        allArtifacts.push_back(nodeArtifacts);
        }
    return allArtifacts;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ItemExtendedData::RelatedInstanceKey> ItemExtendedData::GetRelatedInstanceKeys() const
    {
    bvector<RelatedInstanceKey> keys;
    if (!GetJson().HasMember(ITEM_EXTENDEDDATA_RelatedInstanceKeys))
        return keys;

    RapidJsonValueCR json = GetJson()[ITEM_EXTENDEDDATA_RelatedInstanceKeys];
    if (!json.IsArray())
        return keys;

    for (Json::ArrayIndex i = 0; i < json.Size(); i++)
        {
        RapidJsonValueCR jsonKey = json[i];
        Utf8CP alias = jsonKey[ITEM_EXTENDEDDATA_RelatedInstanceKeys_Alias].GetString();
        ECClassId classId(jsonKey[ITEM_EXTENDEDDATA_RelatedInstanceKey_ECClassId].GetUint64());
        ECInstanceId instanceId(jsonKey[ITEM_EXTENDEDDATA_RelatedInstanceKey_ECInstanceId].GetUint64());
        keys.push_back(RelatedInstanceKey(ECInstanceKey(classId, instanceId), alias));
        }
    return keys;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ItemExtendedData::SetRelatedInstanceKeys(rapidjson::Value&& json)
    {
    BeAssert(json.IsArray());
    AddMember(ITEM_EXTENDEDDATA_RelatedInstanceKeys, std::move(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ItemExtendedData::SetRelatedInstanceKeys(Utf8CP serializedJson)
    {
    if (nullptr == serializedJson || 0 == *serializedJson)
        return;

    rapidjson::Document json(&GetAllocator());
    json.Parse(serializedJson);
    if (json.IsArray())
        SetRelatedInstanceKeys(std::move(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ItemExtendedData::SetRelatedInstanceKeys(bvector<RelatedInstanceKey> const& keys)
    {
    rapidjson::Value json(rapidjson::kArrayType);
    for (RelatedInstanceKey const& key : keys)
        {
        rapidjson::Value jsonKey(rapidjson::kObjectType);
        jsonKey.AddMember(ITEM_EXTENDEDDATA_RelatedInstanceKeys_Alias, rapidjson::Value(key.GetAlias(), GetAllocator()), GetAllocator());
        jsonKey.AddMember(ITEM_EXTENDEDDATA_RelatedInstanceKey_ECClassId, key.GetInstanceKey().GetClassId().GetValue(), GetAllocator());
        jsonKey.AddMember(ITEM_EXTENDEDDATA_RelatedInstanceKey_ECInstanceId, key.GetInstanceKey().GetInstanceId().GetValue(), GetAllocator());
        json.PushBack(std::move(jsonKey), GetAllocator());
        }
    AddMember(ITEM_EXTENDEDDATA_RelatedInstanceKeys, std::move(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ItemExtendedData::SetRelatedInstanceKey(RelatedInstanceKey const& key)
    {
    bvector<RelatedInstanceKey> vec;
    vec.push_back(key);
    SetRelatedInstanceKeys(vec);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::IdSet<ECInstanceId> NavNodeExtendedData::GetInstanceIds() const
    {
    BeSQLite::IdSet<ECInstanceId> ids;
    if (!GetJson().HasMember(NAVNODE_EXTENDEDDATA_ECInstanceKeys))
        return ids;

    RapidJsonValueCR json = GetJson()[NAVNODE_EXTENDEDDATA_ECInstanceKeys];
    if (!json.IsArray())
        return ids;

    for (Json::ArrayIndex i = 0; i < json.Size(); i++)
        {
        RapidJsonValueCR jsonKey = json[i];
        ids.insert(ECInstanceId(jsonKey[NAVNODE_EXTENDEDDATA_InstanceKey_ECInstanceId].GetUint64()));
        }
    return ids;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECInstanceKey> NavNodeExtendedData::GetInstanceKeys() const
    {
    bvector<ECInstanceKey> keys;
    if (!GetJson().HasMember(NAVNODE_EXTENDEDDATA_ECInstanceKeys))
        return keys;

    RapidJsonValueCR json = GetJson()[NAVNODE_EXTENDEDDATA_ECInstanceKeys];
    if (!json.IsArray())
        return keys;

    for (Json::ArrayIndex i = 0; i < json.Size(); i++)
        {
        RapidJsonValueCR jsonKey = json[i];
        ECClassId classId(jsonKey[NAVNODE_EXTENDEDDATA_InstanceKey_ECClassId].GetUint64());
        ECInstanceId instanceId(jsonKey[NAVNODE_EXTENDEDDATA_InstanceKey_ECInstanceId].GetUint64());
        keys.push_back(ECInstanceKey(classId, instanceId));
        }
    return keys;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::SizeType NavNodeExtendedData::GetInstanceKeysCount() const
    {
    if (!GetJson().HasMember(NAVNODE_EXTENDEDDATA_ECInstanceKeys))
        return 0;

    RapidJsonValueCR json = GetJson()[NAVNODE_EXTENDEDDATA_ECInstanceKeys];
    if (!json.IsArray())
        return 0;

    return json.Size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodeExtendedData::SetInstanceKeys(bvector<ECInstanceKey> const& keys)
    {
    rapidjson::Value json(rapidjson::kArrayType);
    for (ECInstanceKey const& key : keys)
        {
        rapidjson::Value jsonKey(rapidjson::kObjectType);
        jsonKey.AddMember(NAVNODE_EXTENDEDDATA_InstanceKey_ECClassId, key.GetClassId().GetValueUnchecked(), GetAllocator());
        jsonKey.AddMember(NAVNODE_EXTENDEDDATA_InstanceKey_ECInstanceId, key.GetInstanceId().GetValueUnchecked(), GetAllocator());
        json.PushBack(std::move(jsonKey), GetAllocator());
        }
    AddMember(NAVNODE_EXTENDEDDATA_ECInstanceKeys, std::move(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodeExtendedData::SetInstanceKey(ECInstanceKey const& key)
    {
    bvector<ECInstanceKey> vec;
    vec.push_back(key);
    SetInstanceKeys(vec);
    }

#define NAVNODE_EXTENDEDDATA_SkippedInstanceKeys                "SkippedInstanceKeys"
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECInstanceKey> NavNodeExtendedData::GetSkippedInstanceKeys() const
    {
    bvector<ECInstanceKey> keys;
    if (!GetJson().HasMember(NAVNODE_EXTENDEDDATA_SkippedInstanceKeys))
        return keys;

    RapidJsonValueCR json = GetJson()[NAVNODE_EXTENDEDDATA_SkippedInstanceKeys];
    if (!json.IsArray())
        return keys;

    for (Json::ArrayIndex i = 0; i < json.Size(); i++)
        {
        RapidJsonValueCR jsonKey = json[i];
        ECClassId classId(jsonKey[NAVNODE_EXTENDEDDATA_InstanceKey_ECClassId].GetUint64());
        ECInstanceId instanceId(jsonKey[NAVNODE_EXTENDEDDATA_InstanceKey_ECInstanceId].GetUint64());
        keys.push_back(ECInstanceKey(classId, instanceId));
        }
    return keys;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodeExtendedData::SetSkippedInstanceKeys(rapidjson::Value&& json)
    {
    BeAssert(json.IsArray());
    AddMember(NAVNODE_EXTENDEDDATA_SkippedInstanceKeys, std::move(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodeExtendedData::SetSkippedInstanceKeys(bvector<ECInstanceKey> const& keys)
    {
    rapidjson::Value json(rapidjson::kArrayType);
    for (ECInstanceKey const& key : keys)
        {
        rapidjson::Value jsonKey(rapidjson::kObjectType);
        jsonKey.AddMember(NAVNODE_EXTENDEDDATA_InstanceKey_ECClassId, key.GetClassId().GetValueUnchecked(), GetAllocator());
        jsonKey.AddMember(NAVNODE_EXTENDEDDATA_InstanceKey_ECInstanceId, key.GetInstanceId().GetValueUnchecked(), GetAllocator());
        json.PushBack(std::move(jsonKey), GetAllocator());
        }
    AddMember(NAVNODE_EXTENDEDDATA_SkippedInstanceKeys, std::move(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodeExtendedData::SetSkippedInstanceKey(ECInstanceKey const& key)
    {
    bvector<ECInstanceKey> vec;
    vec.push_back(key);
    SetSkippedInstanceKeys(vec);
    }
