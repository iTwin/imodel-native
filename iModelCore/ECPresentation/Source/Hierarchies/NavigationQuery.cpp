/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "NavigationQuery.h"
#include "../Shared/ValueHelpers.h"

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NavigationQueryResultParameters::MergeWith(NavigationQueryResultParameters const& other)
    {
    if (other.GetResultType() != NavigationQueryResultType::Invalid)
        {
        DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, (GetResultType() == NavigationQueryResultType::Invalid || GetResultType() == other.GetResultType()),
            Utf8PrintfString("Can only merge query result params with params of invalid or the same type. Source type: '%d', target type: '%d'", (int)other.GetResultType(), (int)GetResultType()));
        SetResultType(other.GetResultType());
        }

    GetNavNodeExtendedDataR().MergeWith(other.GetNavNodeExtendedData());

    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, (nullptr == m_specification || nullptr == other.m_specification || m_specification == other.m_specification),
        "Can only merge query result params with null or the same specification");
    if (nullptr == m_specification)
        m_specification = other.m_specification;

    ContainerHelpers::Push(m_usedRelationshipClasses, other.m_usedRelationshipClasses);
    ContainerHelpers::Push(m_selectInstanceClasses, other.m_selectInstanceClasses);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavigationQueryResultParameters::operator==(NavigationQueryResultParameters const& other) const
    {
    return m_resultType == other.m_resultType
        && m_navNodeExtendedData == other.m_navNodeExtendedData
        && m_specification == other.m_specification
        && m_usedRelationshipClasses == other.m_usedRelationshipClasses
        && m_selectInstanceClasses == other.m_selectInstanceClasses;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NavigationQueryExtendedData::AddRangesData(ECPropertyCR prop, PropertyGroupCR spec)
    {
    if (spec.GetRanges().empty())
        return;

    if (!prop.GetIsPrimitive())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Expecting range data only for primitive properties. Actual property: '%s.%s'", prop.GetClass().GetFullName(), prop.GetName().c_str()));

    rapidjson::Value ranges(rapidjson::kArrayType);
    for (PropertyRangeGroupSpecificationCP rangeSpec : spec.GetRanges())
        {
        rapidjson::Value range(rapidjson::kObjectType);
        PrimitiveType type = prop.GetAsPrimitiveProperty()->GetType();
        range.AddMember("from", ValueHelpers::GetJsonFromString(type, rangeSpec->GetFromValue(), &GetAllocator()), GetAllocator());
        range.AddMember("to", ValueHelpers::GetJsonFromString(type, rangeSpec->GetToValue(), &GetAllocator()), GetAllocator());
        if (!rangeSpec->GetImageId().empty())
            range.AddMember("imageId", rapidjson::StringRef(rangeSpec->GetImageId().c_str()), GetAllocator());
        if (!rangeSpec->GetLabel().empty())
            range.AddMember("label", rapidjson::StringRef(rangeSpec->GetLabel().c_str()), GetAllocator());
        ranges.PushBack(range, GetAllocator());
        }
    AddMember(NAVIGATIONQUERY_EXTENDEDDATA_Ranges, std::move(ranges));
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int NavigationQueryExtendedData::GetRangeIndex(DbValue const& dbValue) const
    {
    if (!HasRangesData())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Requested range index from a query that doesn't have range data");

    RapidJsonValueCR ranges = GetJson()[NAVIGATIONQUERY_EXTENDEDDATA_Ranges];
    for (rapidjson::SizeType i = 0; i < ranges.Size(); i++)
        {
        if (ranges[i].HasMember("from") && ranges[i].HasMember("to"))
            {
            RapidJsonValueCR from = ranges[i]["from"];
            RapidJsonValueCR to = ranges[i]["to"];
            DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, (from.GetType() == to.GetType()), Utf8PrintfString("'from' and 'to' range property types should match. "
                "Actual 'from': % d, 'to': % d", (int)from.GetType(), (int)to.GetType()));
            switch (from.GetType())
                {
                case rapidjson::kFalseType:
                case rapidjson::kTrueType:
                    {
                    bool value = (dbValue.GetValueInt() != 0);
                    if (from.GetBool() <= value && value <= to.GetBool())
                        return i;
                    break;
                    }
                case rapidjson::kNumberType:
                    {
                    if (from.IsUint64() && from.GetUint64() <= (uint64_t)dbValue.GetValueInt64() && (uint64_t)dbValue.GetValueInt64() <= to.GetUint64())
                        return i;
                    if (from.IsInt64() && from.GetInt64() <= dbValue.GetValueInt64() && dbValue.GetValueInt64() <= to.GetInt64())
                        return i;
                    if (from.IsDouble() && from.GetDouble() <= dbValue.GetValueDouble() && dbValue.GetValueDouble() <= to.GetDouble())
                        return i;
                    break;
                    }
                case rapidjson::kStringType:
                    {
                    Utf8CP value = dbValue.GetValueText();
                    if (strcmp(from.GetString(), value) <= 0 && strcmp(value, to.GetString()) <= 0)
                        return i;
                    break;
                    }
                default:
                    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Default, LOG_ERROR, Utf8PrintfString("Unhandled range value type: %d", (int)from.GetType()));
                }
            }
        }
    return -1;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetJsonAsString(RapidJsonValueCR json)
    {
    switch (json.GetType())
        {
        case rapidjson::kNullType:
            return "";
        case rapidjson::kFalseType:
            return "False";
        case rapidjson::kTrueType:
            return "True";
        case rapidjson::kStringType:
            return json.GetString();
        case rapidjson::kNumberType:
            {
            if (json.IsUint64())
                return std::to_string(json.GetUint64()).c_str();
            if (json.IsInt64())
                return std::to_string(json.GetInt64()).c_str();
            if (json.IsDouble())
                return Utf8PrintfString("%.2f", json.GetDouble());
            }
        }
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Default, LOG_ERROR, Utf8PrintfString("Unexpected type of JSON: %d", (int)json.GetType()));
    rapidjson::StringBuffer buf;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buf);
    json.Accept(writer);
    return buf.GetString();
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String NavigationQueryExtendedData::GetRangeLabel(int rangeIndex) const
    {
    RapidJsonValueCR ranges = GetJson()[NAVIGATIONQUERY_EXTENDEDDATA_Ranges];
    if (!ranges.IsArray() || rangeIndex < 0 || rangeIndex >= (int)ranges.Size())
        {
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to get range label for specified index. "
            "Ranges count: %" PRIu64 ", requested index: %d", (uint64_t)ranges.Size(), rangeIndex));
        }

    RapidJsonValueCR range = ranges[rangeIndex];
    if (range.HasMember("label"))
        return range["label"].GetString();

    return GetJsonAsString(range["from"]).append(" - ").append(GetJsonAsString(range["to"]));
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP NavigationQueryExtendedData::GetRangeImageId(int rangeIndex) const
    {
    RapidJsonValueCR ranges = GetJson()[NAVIGATIONQUERY_EXTENDEDDATA_Ranges];
    if (!ranges.IsArray() || rangeIndex < 0 || rangeIndex >= (int)ranges.Size())
        {
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to get range image id for specified index. "
            "Ranges count: %" PRIu64 ", requested index: %d", (uint64_t)ranges.Size(), rangeIndex));
        }

    RapidJsonValueCR range = ranges[rangeIndex];
    return range.HasMember("imageId") ? range["imageId"].GetString() : "";
    }
