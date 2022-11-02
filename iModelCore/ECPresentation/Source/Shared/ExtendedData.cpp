/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECPresentationManager.h>
#include "ExtendedData.h"
#include "ValueHelpers.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TResult>
static bvector<TResult> ParseJsonArray(RapidJsonValueCR json, Utf8CP memberName, TResult(rapidjson::Value::*getter)() const)
    {
    bvector<TResult> ints;
    if (json.HasMember(memberName))
        {
        RapidJsonValueCR jsonArr = json[memberName];
        for (rapidjson::SizeType i = 0; i < jsonArr.Size(); ++i)
            ints.push_back((jsonArr[i].*getter)());
        }
    return ints;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TValue>
static void AddValueToJsonArray(RapidJsonValueR json, rapidjson::Document::AllocatorType& allocator, Utf8CP memberName, TValue const& value)
    {
    if (!json.HasMember(memberName))
        json.AddMember(rapidjson::StringRef(memberName), rapidjson::Value(rapidjson::kArrayType), allocator);
    json[memberName].PushBack(value, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TValue>
static void SetValuesToJsonArray(RapidJsonValueR json, rapidjson::Document::AllocatorType& allocator, Utf8CP memberName, bvector<TValue> const& values)
    {
    rapidjson::Value jsonArr(rapidjson::kArrayType);
    for (TValue value : values)
        jsonArr.PushBack(value, allocator);

    rapidjson::Value::MemberIterator iterator = json.FindMember(memberName);
    if (iterator == json.MemberEnd())
        json.AddMember(rapidjson::StringRef(memberName), std::move(jsonArr), allocator);
    else
        iterator->value = std::move(jsonArr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static rapidjson::Value GuidToJson(BeGuidCR guid, rapidjson::Document::AllocatorType& allocator)
    {
    rapidjson::Value doc(rapidjson::kArrayType);
    doc.PushBack(guid.m_guid.u[0], allocator);
    doc.PushBack(guid.m_guid.u[1], allocator);
    return doc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static BeGuid GuidFromJson(RapidJsonValueCR json)
    {
    if (!json.IsArray())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Invalid GUID JSON: '%s'", BeRapidJsonUtilities::ToString(json).c_str()));

    return BeGuid(json[0].GetUint64(), json[1].GetUint64());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<BeGuid> ParseGuidsArray(RapidJsonValueCR json, Utf8CP memberName)
    {
    bvector<BeGuid> guids;
    if (json.HasMember(memberName))
        {
        RapidJsonValueCR jsonArr = json[memberName];
        for (rapidjson::SizeType i = 0; i < jsonArr.Size(); ++i)
            {
            auto guid = GuidFromJson(jsonArr[i]);
            if (guid.IsValid())
                guids.push_back(guid);
            }
        }
    return guids;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AddGuidToJsonArray(RapidJsonValueR json, rapidjson::Document::AllocatorType& allocator, Utf8CP memberName, BeGuidCR guid)
    {
    if (!json.HasMember(memberName))
        json.AddMember(rapidjson::StringRef(memberName), rapidjson::Value(rapidjson::kArrayType), allocator);
    json[memberName].PushBack(GuidToJson(guid, allocator).Move(), allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void SetGuidsToJsonArray(RapidJsonValueR json, rapidjson::Document::AllocatorType& allocator, Utf8CP memberName, bvector<BeGuid> const& values)
    {
    rapidjson::Value jsonArr(rapidjson::kArrayType);
    for (BeGuidCR value : values)
        jsonArr.PushBack(GuidToJson(value, allocator).Move(), allocator);

    rapidjson::Value::MemberIterator iterator = json.FindMember(memberName);
    if (iterator == json.MemberEnd())
        json.AddMember(rapidjson::StringRef(memberName), std::move(jsonArr), allocator);
    else
        iterator->value = std::move(jsonArr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<BeGuid> NavNodeExtendedData::GetVirtualParentIds() const {return ParseGuidsArray(GetJson(), NAVNODE_EXTENDEDDATA_VirtualParentIds);}
void NavNodeExtendedData::AddVirtualParentId(BeGuidCR id) { AddGuidToJsonArray(GetJsonR(), GetAllocator(), NAVNODE_EXTENDEDDATA_VirtualParentIds, id); }
void NavNodeExtendedData::SetVirtualParentIds(bvector<BeGuid> const& ids) { SetGuidsToJsonArray(GetJsonR(), GetAllocator(), NAVNODE_EXTENDEDDATA_VirtualParentIds, ids); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<BeGuid> NavNodeExtendedData::GetMergedNodeIds() const {return ParseGuidsArray(GetJson(), NAVNODE_EXTENDEDDATA_MergedNodeIds);}
void NavNodeExtendedData::AddMergedNodeId(BeGuidCR id) { AddGuidToJsonArray(GetJsonR(), GetAllocator(), NAVNODE_EXTENDEDDATA_MergedNodeIds, id); }
void NavNodeExtendedData::SetMergedNodeIds(bvector<BeGuid> const& ids) { SetGuidsToJsonArray(GetJsonR(), GetAllocator(), NAVNODE_EXTENDEDDATA_MergedNodeIds, ids); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RapidJsonValueCR NavNodeExtendedData::GetPropertyValues() const
    {
    if (!HasPropertyValues())
        {
        static rapidjson::Value s_empty(rapidjson::kArrayType);
        return s_empty;
        }
    return GetJson()[NAVNODE_EXTENDEDDATA_PropertyValues];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodeExtendedData::SetPropertyValues(RapidJsonValueCR value)
    {
    if (!value.IsArray())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Invalid property values JSON format: '%s'", BeRapidJsonUtilities::ToString(value).c_str()));
    AddMember(NAVNODE_EXTENDEDDATA_PropertyValues, rapidjson::Value(value, GetAllocator()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<int> NavNodeExtendedData::GetPropertyValueRangeIndexes() const {return ParseJsonArray<int>(GetJson(), NAVNODE_EXTENDEDDATA_PropertyValueRangeIndexes, &rapidjson::Value::GetInt);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RapidJsonValueCR NavNodeExtendedData::GetPropertyValueRangeIndexesJson() const {return GetJson()[NAVNODE_EXTENDEDDATA_PropertyValueRangeIndexes];}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodeExtendedData::SetPropertyValueRangeIndexes(bvector<int> const& indexes) {SetValuesToJsonArray(GetJsonR(), GetAllocator(), NAVNODE_EXTENDEDDATA_PropertyValueRangeIndexes, indexes);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodeExtendedData::SetPropertyValueRangeIndexes(RapidJsonValueCR value)
    {
    if (!value.IsArray())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Invalid property value range indexes JSON format: '%s'", BeRapidJsonUtilities::ToString(value).c_str()));
    AddMember(NAVNODE_EXTENDEDDATA_PropertyValueRangeIndexes, rapidjson::Value(value, GetAllocator()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ItemExtendedData::RelatedInstanceKey ItemExtendedData::RelatedInstanceKey::FromJson(RapidJsonValueCR json)
    {
    Utf8CP alias = json[ITEM_EXTENDEDDATA_RelatedInstanceKeys_Alias].GetString();
    ECClassId classId(json[ITEM_EXTENDEDDATA_RelatedInstanceKey_ECClassId].GetUint64());
    ECInstanceId instanceId(json[ITEM_EXTENDEDDATA_RelatedInstanceKey_ECInstanceId].GetUint64());
    return RelatedInstanceKey(ECInstanceKey(classId, instanceId), alias);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ItemExtendedData::RelatedInstanceKey> ItemExtendedData::RelatedInstanceKey::FromJsonArray(RapidJsonValueCR json)
    {
    if (!json.IsArray())
        return bvector<RelatedInstanceKey>();

    bvector<RelatedInstanceKey> keys;
    for (rapidjson::SizeType i = 0; i < json.Size(); ++i)
        keys.push_back(RelatedInstanceKey::FromJson(json[i]));
    return keys;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ItemExtendedData::RelatedInstanceKey::ToJson(rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(rapidjson::kObjectType, allocator);
    json.AddMember(ITEM_EXTENDEDDATA_RelatedInstanceKeys_Alias, rapidjson::Value(GetAlias(), json.GetAllocator()), json.GetAllocator());
    json.AddMember(ITEM_EXTENDEDDATA_RelatedInstanceKey_ECClassId, rapidjson::Value(m_instanceKey.GetClassId().GetValueUnchecked()), json.GetAllocator());
    json.AddMember(ITEM_EXTENDEDDATA_RelatedInstanceKey_ECInstanceId, rapidjson::Value(m_instanceKey.GetInstanceId().GetValueUnchecked()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ItemExtendedData::RelatedInstanceKey> ItemExtendedData::GetRelatedInstanceKeys() const
    {
    if (!GetJson().HasMember(ITEM_EXTENDEDDATA_RelatedInstanceKeys))
        return bvector<RelatedInstanceKey>();

    RapidJsonValueCR json = GetJson()[ITEM_EXTENDEDDATA_RelatedInstanceKeys];
    if (!json.IsArray())
        return bvector<RelatedInstanceKey>();

    return RelatedInstanceKey::FromJsonArray(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ItemExtendedData::RelatedInstanceKey> ItemExtendedData::ParseRelatedInstanceKeys(Utf8CP serializedJson)
    {
    if (nullptr == serializedJson || 0 == *serializedJson)
        return bvector<RelatedInstanceKey>();

    rapidjson::Document json;
    json.Parse(serializedJson);
    return RelatedInstanceKey::FromJsonArray(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ItemExtendedData::SetRelatedInstanceKeys(rapidjson::Value&& json)
    {
    if (!json.IsArray())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Invalid related instance keys JSON format: '%s'", BeRapidJsonUtilities::ToString(json).c_str()));
    AddMember(ITEM_EXTENDEDDATA_RelatedInstanceKeys, std::move(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ItemExtendedData::SetRelatedInstanceKeys(bvector<RelatedInstanceKey> const& keys)
    {
    rapidjson::Value json(rapidjson::kArrayType);
    for (RelatedInstanceKey const& key : keys)
        json.PushBack(key.ToJson(&GetAllocator()), GetAllocator());
    AddMember(ITEM_EXTENDEDDATA_RelatedInstanceKeys, std::move(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ItemExtendedData::SetRelatedInstanceKey(RelatedInstanceKey const& key)
    {
    bvector<RelatedInstanceKey> vec;
    vec.push_back(key);
    SetRelatedInstanceKeys(vec);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ItemExtendedData::AddRelatedInstanceKeys(bvector<RelatedInstanceKey> const& keys)
    {
    if (!GetJson().HasMember(ITEM_EXTENDEDDATA_RelatedInstanceKeys))
        SetRelatedInstanceKeys(keys);

    RapidJsonValueR keysArray = GetJsonR()[ITEM_EXTENDEDDATA_RelatedInstanceKeys];
    for (RelatedInstanceKey const& key : keys)
        keysArray.PushBack(key.ToJson(&GetAllocator()), GetAllocator());
    }
