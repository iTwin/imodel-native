/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "ValueHelpers.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ValueHelpers::GetEnumPropertyDisplayValue(Utf8StringR displayValue, ECPropertyCR prop,
    std::function<int()> getIntEnumId, std::function<Utf8CP()> getStrEnumId)
    {
    if (!prop.GetIsPrimitive() && !prop.GetIsPrimitiveArray())
        return ERROR;

    ECEnumerationCP enumeration = prop.GetIsPrimitive() ? prop.GetAsPrimitiveProperty()->GetEnumeration() : prop.GetAsPrimitiveArrayProperty()->GetEnumeration();
    if (nullptr == enumeration)
        return ERROR;

    Json::Value rawValue;
    ECEnumeratorCP enumerator = nullptr;
    switch (enumeration->GetType())
        {
        case PRIMITIVETYPE_Integer:
            {
            int enumId = getIntEnumId();
            rawValue = enumId;
            enumerator = enumeration->FindEnumerator(enumId);
            break;
            }
        case PRIMITIVETYPE_String:
            {
            Utf8CP enumId = getStrEnumId();
            rawValue = enumId;
            enumerator = enumeration->FindEnumerator(enumId);
            break;
            }
        }

    if (nullptr == enumerator)
        {
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to determine enumerator for `%s`, value `%s`",
            enumeration->GetFullName().c_str(), rawValue.ToString().c_str()));
        }

    displayValue = enumerator->GetDisplayLabel();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ValueHelpers::GetEnumPropertyDisplayValue(Utf8StringR displayValue, ECPropertyCR prop, ECValueCR ecValue)
    {
    if (ecValue.IsNull())
        return ERROR;
    return GetEnumPropertyDisplayValue(displayValue, prop,
        [&ecValue](){return ecValue.GetInteger();},
        [&ecValue](){return ecValue.GetUtf8CP();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ValueHelpers::GetEnumPropertyDisplayValue(Utf8StringR displayValue, ECPropertyCR prop, DbValue const& dbValue)
    {
    if (dbValue.IsNull())
        return ERROR;
    return GetEnumPropertyDisplayValue(displayValue, prop,
        [&dbValue](){return dbValue.GetValueInt();},
        [&dbValue](){return dbValue.GetValueText();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ValueHelpers::GetEnumPropertyDisplayValue(Utf8StringR displayValue, ECPropertyCR prop, RapidJsonValueCR jsonValue)
    {
    if (jsonValue.IsNull())
        return ERROR;
    return GetEnumPropertyDisplayValue(displayValue, prop,
        [&jsonValue](){return jsonValue.GetInt();},
        [&jsonValue](){return jsonValue.GetString();});
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static rapidjson::Document ParseJson(Utf8CP serialized, rapidjson::MemoryPoolAllocator<>* allocator)
    {
    rapidjson::Document json(allocator);
    json.Parse(serialized);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d ValueHelpers::GetPoint2dFromSqlValue(IECSqlValue const& value)
    {
    if (PRIMITIVETYPE_Point2d == value.GetColumnInfo().GetDataType().GetPrimitiveType())
        return value.GetPoint2d();
    return GetPoint2dFromJsonString(value.GetText());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d ValueHelpers::GetPoint3dFromSqlValue(IECSqlValue const& value)
    {
    if (PRIMITIVETYPE_Point3d == value.GetColumnInfo().GetDataType().GetPrimitiveType())
        return value.GetPoint3d();
    return GetPoint3dFromJsonString(value.GetText());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d ValueHelpers::GetPoint2dFromJson(JsonValueCR json)
    {
    if (json.isNull() || !json.isObject())
        return DPoint2d();
    return DPoint2d::From(json["x"].asDouble(), json["y"].asDouble());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d ValueHelpers::GetPoint2dFromJson(RapidJsonValueCR json)
    {
    if (json.IsNull() || !json.IsObject())
        return DPoint2d();
    return DPoint2d::From(json["x"].GetDouble(), json["y"].GetDouble());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d ValueHelpers::GetPoint3dFromJson(JsonValueCR json)
    {
    if (json.isNull() || !json.isObject())
        return DPoint3d();
    return DPoint3d::From(json["x"].asDouble(), json["y"].asDouble(), json["z"].asDouble());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d ValueHelpers::GetPoint3dFromJson(RapidJsonValueCR json)
    {
    if (json.IsNull() || !json.IsObject())
        return DPoint3d();
    return DPoint3d::From(json["x"].GetDouble(), json["y"].GetDouble(), json["z"].GetDouble());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d ValueHelpers::GetPoint2dFromJsonString(Utf8CP str)
    {
    rapidjson::Document json = ParseJson(str, nullptr);
    if (json.IsNull() || !json.IsObject())
        return DPoint2d();
    return DPoint2d::From(json["x"].GetDouble(), json["y"].GetDouble());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d ValueHelpers::GetPoint3dFromJsonString(Utf8CP str)
    {
    rapidjson::Document json = ParseJson(str, nullptr);
    if (json.IsNull() || !json.IsObject())
        return DPoint3d();
    return DPoint3d::From(json["x"].GetDouble(), json["y"].GetDouble(), json["z"].GetDouble());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<double> GetNDoublesFromString(Utf8StringCR str, size_t n)
    {
    Utf8String trimmed = str;
    trimmed.Trim("{[ ");
    bvector<Utf8String> tokens;
    BeStringUtilities::Split(trimmed.c_str(), ",; ", tokens);
    bvector<double> values;
    for (Utf8StringCR token : tokens)
        {
        if (!token.empty())
            values.push_back(std::stod(token.c_str()));
        }
    return values;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ValueHelpers::GetPoint2dJsonFromString(Utf8StringCR str, rapidjson::MemoryPoolAllocator<>* allocator)
    {
    bvector<double> values = GetNDoublesFromString(str, 2);
    if (2 != values.size())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to parse Point2d from '%s'", str.c_str()));

    return GetPoint2dJson(DPoint2d::FromArray(&values[0]), allocator);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ValueHelpers::GetPoint3dJsonFromString(Utf8StringCR str, rapidjson::MemoryPoolAllocator<>* allocator)
    {
    bvector<double> values = GetNDoublesFromString(str, 3);
    if (3 != values.size())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to parse Point3d from '%s'", str.c_str()));

    return GetPoint3dJson(DPoint3d::FromArray(&values[0]), allocator);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ValueHelpers::GetPoint2dJson(DPoint2dCR pt, rapidjson::MemoryPoolAllocator<>* allocator)
    {
    rapidjson::Document doc(allocator);
    doc.SetObject();
    doc.AddMember("x", pt.x, doc.GetAllocator());
    doc.AddMember("y", pt.y, doc.GetAllocator());
    return doc;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ValueHelpers::GetPoint3dJson(DPoint3dCR pt, rapidjson::MemoryPoolAllocator<>* allocator)
    {
    rapidjson::Document doc(allocator);
    doc.SetObject();
    doc.AddMember("x", pt.x, doc.GetAllocator());
    doc.AddMember("y", pt.y, doc.GetAllocator());
    doc.AddMember("z", pt.z, doc.GetAllocator());
    return doc;
    }

#define NULL_VALUE_PRECONDITION(sqlValue) \
    if (sqlValue.IsNull()) \
        return rapidjson::Document();

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ValueHelpers::GetJsonFromPrimitiveValue(PrimitiveType primitiveType, IECSqlValue const& value, rapidjson::MemoryPoolAllocator<>* allocator)
    {
    NULL_VALUE_PRECONDITION(value);

    rapidjson::Document doc(allocator);
    switch (primitiveType)
        {
        case PRIMITIVETYPE_Boolean:
            doc.SetBool(value.GetBoolean());
            return doc;
        case PRIMITIVETYPE_DateTime:
            doc.SetString(value.GetDateTime().ToString().c_str(), doc.GetAllocator());
            return doc;
        case PRIMITIVETYPE_Double:
            doc.SetDouble(value.GetDouble());
            return doc;
        case PRIMITIVETYPE_Integer:
            doc.SetInt(value.GetInt());
            return doc;
        case PRIMITIVETYPE_Long:
            doc.SetInt64(value.GetInt64());
            return doc;
        case PRIMITIVETYPE_String:
            doc.SetString(value.GetText(), doc.GetAllocator());
            return doc;
        case PRIMITIVETYPE_Point2d:
            return GetPoint2dJson(GetPoint2dFromSqlValue(value), allocator);
        case PRIMITIVETYPE_Point3d:
            return GetPoint3dJson(GetPoint3dFromSqlValue(value), allocator);
        case PRIMITIVETYPE_Binary:
        case PRIMITIVETYPE_IGeometry:
            return doc;
        }
    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Unrecognized primitive property type: %d", (int)primitiveType));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ValueHelpers::GetJsonFromStructValue(ECStructClassCR structClass, IECSqlValue const& sqlValue, rapidjson::MemoryPoolAllocator<>* allocator)
    {
    NULL_VALUE_PRECONDITION(sqlValue);

    if (sqlValue.GetColumnInfo().GetStructType() != &structClass)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Unexpected struct stored in IECSqlValue");

    rapidjson::Document doc(allocator);
    doc.SetObject();
    for (IECSqlValue const& v : sqlValue.GetStructIterable())
        {
        Utf8CP propertyName = v.GetColumnInfo().GetProperty()->GetName().c_str();
        rapidjson::Value propertyNameJson(propertyName, doc.GetAllocator());
        switch (v.GetColumnInfo().GetDataType().GetTypeKind())
            {
            case ValueKind::VALUEKIND_Struct:
                doc.AddMember(propertyNameJson, GetJsonFromStructValue(*v.GetColumnInfo().GetStructType(), v, &doc.GetAllocator()), doc.GetAllocator());
                break;
            case ValueKind::VALUEKIND_Array:
                doc.AddMember(propertyNameJson, GetJsonFromArrayValue(v, &doc.GetAllocator()), doc.GetAllocator());
                break;
            case ValueKind::VALUEKIND_Primitive:
                doc.AddMember(propertyNameJson, GetJsonFromPrimitiveValue(v.GetColumnInfo().GetDataType().GetPrimitiveType(), v, &doc.GetAllocator()), doc.GetAllocator());
                break;
            }
        }
    return doc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ValueHelpers::GetJsonFromArrayValue(IECSqlValue const& sqlValue, rapidjson::MemoryPoolAllocator<>* allocator)
    {
    NULL_VALUE_PRECONDITION(sqlValue);

    rapidjson::Document doc(allocator);
    doc.SetArray();
    for (IECSqlValue const& v : sqlValue.GetArrayIterable())
        {
        switch (v.GetColumnInfo().GetDataType().GetTypeKind())
            {
            case ValueKind::VALUEKIND_Struct:
                doc.PushBack(GetJsonFromStructValue(*v.GetColumnInfo().GetStructType(), v, &doc.GetAllocator()), doc.GetAllocator());
                break;
            case ValueKind::VALUEKIND_Array:
                doc.PushBack(GetJsonFromArrayValue(v, &doc.GetAllocator()), doc.GetAllocator());
                break;
            case ValueKind::VALUEKIND_Primitive:
                doc.PushBack(GetJsonFromPrimitiveValue(v.GetColumnInfo().GetDataType().GetPrimitiveType(), v, &doc.GetAllocator()), doc.GetAllocator());
                break;
            }
        }
    return doc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ValueHelpers::GetJsonFromString(PrimitiveType primitiveType, Utf8StringCR str, rapidjson::MemoryPoolAllocator<>* allocator)
    {
    rapidjson::Document doc(allocator);
    switch (primitiveType)
        {
        case PRIMITIVETYPE_Boolean:
            doc.SetBool(str.EqualsI("true") || str.Equals("1"));
            return doc;
        case PRIMITIVETYPE_Binary:
            return doc;
        case PRIMITIVETYPE_DateTime:
            {
            DateTime dt;
            double julianDays;
            if (SUCCESS != DateTime::FromString(dt, str.c_str()) || SUCCESS != dt.ToJulianDay(julianDays))
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to parse DateTime from '%s'", str.c_str()))
            doc.SetDouble(julianDays);
            return doc;
            }
        case PRIMITIVETYPE_Double:
            doc.SetDouble(std::stod(str.c_str()));
            return doc;
        case PRIMITIVETYPE_Integer:
            doc.SetInt(std::stoi(str.c_str()));
            return doc;
        case PRIMITIVETYPE_Long:
            doc.SetInt64(std::stoll(str.c_str()));
            return doc;
        case PRIMITIVETYPE_String:
            doc.SetString(str.c_str(), doc.GetAllocator());
            return doc;
        case PRIMITIVETYPE_Point2d:
            return GetPoint2dJsonFromString(str, allocator);
        case PRIMITIVETYPE_Point3d:
            return GetPoint3dJsonFromString(str, allocator);
        }
    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Unrecognized primitive property type: %d", (int)primitiveType));
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue ValueHelpers::GetECValueFromSqlValue(PrimitiveType primitiveType, DbValue const& sqlValue)
    {
    ECValue value;
    if (sqlValue.IsNull())
        {
        value.SetIsNull(true);
        return value;
        }
    switch (primitiveType)
        {
        case PRIMITIVETYPE_Boolean:
            value.SetBoolean(0 != sqlValue.GetValueInt());
            break;
        case PRIMITIVETYPE_DateTime:
            {
            double julianDay;
            if (DbValueType::TextVal == sqlValue.GetValueType())
                julianDay = std::stod(sqlValue.GetValueText());
            else
                julianDay = sqlValue.GetValueDouble();
            DateTime dt;
            DateTime::FromJulianDay(dt, julianDay, DateTime::Info::CreateForDateTime(DateTime::Kind::Utc));
            value.SetDateTime(dt);
            break;
            }
        case PRIMITIVETYPE_Double:
            value.SetDouble(sqlValue.GetValueDouble());
            break;
        case PRIMITIVETYPE_Integer:
            value.SetInteger(sqlValue.GetValueInt());
            break;
        case PRIMITIVETYPE_Long:
            value.SetLong(sqlValue.GetValueInt64());
            break;
        case PRIMITIVETYPE_String:
            value.SetUtf8CP(sqlValue.GetValueText());
            break;
        case PRIMITIVETYPE_Point2d:
            value.SetPoint2d(GetPoint2dFromJsonString(sqlValue.GetValueText()));
            break;
        case PRIMITIVETYPE_Point3d:
            value.SetPoint3d(GetPoint3dFromJsonString(sqlValue.GetValueText()));
            break;
        case PRIMITIVETYPE_Binary:
        case PRIMITIVETYPE_IGeometry:
            break;
        default:
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Unrecognized primitive property type: %d", (int)primitiveType));
        }
    return value;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue ValueHelpers::GetECValueFromSqlValue(PrimitiveType primitiveType, IECSqlValue const& sqlValue)
    {
    if (VALUEKIND_Primitive != sqlValue.GetColumnInfo().GetDataType().GetTypeKind())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to convert IECSqlValue to ECValue - value is not primitive. Actual type: %d", (int)sqlValue.GetColumnInfo().GetDataType().GetTypeKind()));

    ECValue value;
    if (sqlValue.IsNull())
        {
        value.SetIsNull(true);
        return value;
        }
    switch (primitiveType)
        {
        case PRIMITIVETYPE_Boolean:
            value.SetBoolean(sqlValue.GetBoolean());
            break;
        case PRIMITIVETYPE_DateTime:
            {
            double julianDay;
            if (PRIMITIVETYPE_String == sqlValue.GetColumnInfo().GetDataType().GetPrimitiveType())
                julianDay = std::stod(sqlValue.GetText());
            else
                julianDay = sqlValue.GetDouble();
            DateTime dt;
            DateTime::FromJulianDay(dt, julianDay, DateTime::Info::CreateForDateTime(DateTime::Kind::Utc));
            value.SetDateTime(dt);
            break;
            }
        case PRIMITIVETYPE_Double:
            value.SetDouble(sqlValue.GetDouble());
            break;
        case PRIMITIVETYPE_Integer:
            value.SetInteger(sqlValue.GetInt());
            break;
        case PRIMITIVETYPE_Long:
            value.SetLong(sqlValue.GetInt64());
            break;
        case PRIMITIVETYPE_String:
            value.SetUtf8CP(sqlValue.GetText());
            break;
        case PRIMITIVETYPE_Point2d:
            value.SetPoint2d(GetPoint2dFromSqlValue(sqlValue));
            break;
        case PRIMITIVETYPE_Point3d:
            value.SetPoint3d(GetPoint3dFromSqlValue(sqlValue));
            break;
        case PRIMITIVETYPE_Binary:
        case PRIMITIVETYPE_IGeometry:
            break;
        default:
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Unrecognized primitive property type: %d", (int)primitiveType));
        }
    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue ValueHelpers::GetECValueFromString(PrimitiveType valueType, Utf8StringCR str)
    {
    if (str.empty())
        {
        ECValue nullValue;
        nullValue.SetToNull();
        return nullValue;
        }

    switch (valueType)
        {
        case PRIMITIVETYPE_Boolean:
            return ECValue(str.EqualsI("true") || str.EqualsI("1") ? true : false);
        case PRIMITIVETYPE_Double:
            return ECValue(std::stod(str.c_str()));
        case PRIMITIVETYPE_Integer:
            return ECValue(std::stoi(str.c_str()));
        case PRIMITIVETYPE_Long:
            return ECValue((int64_t)std::stoll(str.c_str()));
        case PRIMITIVETYPE_DateTime:
            {
            DateTime dt;
            if (SUCCESS != DateTime::FromString(dt, str.c_str()))
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to parse DateTime from '%s'", str.c_str()));
            return ECValue(dt);
            }
        }

    return ECValue(str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue ValueHelpers::GetECValueFromJson(PrimitiveType type, RapidJsonValueCR json)
    {
    ECValue value;
    if (json.IsNull())
        {
        value.SetIsNull(true);
        return value;
        }

    switch (type)
        {
        case PRIMITIVETYPE_Boolean:
            value.SetBoolean(json.GetBool());
            break;
        case PRIMITIVETYPE_Binary:
            break;
        case PRIMITIVETYPE_DateTime:
            {
            DateTime dt;
            if (json.IsDouble())
                DateTime::FromJulianDay(dt, json.GetDouble(), DateTime::Info::CreateForDateTime(DateTime::Kind::Utc));
            else
                DateTime::FromString(dt, json.GetString());
            value.SetDateTime(dt);
            break;
            }
        case PRIMITIVETYPE_Double:
            value.SetDouble(json.GetDouble());
            break;
        case PRIMITIVETYPE_Integer:
            value.SetInteger(json.GetInt());
            break;
        case PRIMITIVETYPE_Long:
            if (json.IsString())
                value.SetLong(BeInt64Id::FromString(json.GetString()).GetValueUnchecked());
            else
                value.SetLong(json.GetInt64());
            break;
        case PRIMITIVETYPE_String:
            value.SetUtf8CP(json.GetString());
            break;
        case PRIMITIVETYPE_Point2d:
            value.SetPoint2d(GetPoint2dFromJson(json));
            break;
        case PRIMITIVETYPE_Point3d:
            value.SetPoint3d(GetPoint3dFromJson(json));
            break;
        default:
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Unrecognized primitive property type: %d", (int)type));
        }
    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECValue> ValueHelpers::GetECValueSetFromJson(PrimitiveType type, RapidJsonValueCR json)
    {
    bvector<ECValue> ecValues;
    for (rapidjson::SizeType i = 0; i < json.Size(); i++)
        {
        ECValue value = GetECValueFromJson(type, json[i]);
        if (value.IsNull())
            ecValues.push_back(ECValue(type));
        else
            ecValues.push_back(value);
        
        }
    return ecValues;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ValueHelpers::GetJsonFromECValue(ECValueCR ecValue, rapidjson::MemoryPoolAllocator<>* allocator)
    {
    rapidjson::Document doc(allocator);
    if (ecValue.IsUninitialized() || ecValue.IsNull())
        return doc;

    if (ecValue.IsNavigation())
        {
        doc.SetString(ecValue.GetNavigationInfo().GetId<BeInt64Id>().ToHexStr().c_str(), doc.GetAllocator());
        return doc;
        }
    switch (ecValue.GetPrimitiveType())
        {
        case PRIMITIVETYPE_Boolean:
            doc.SetBool(ecValue.GetBoolean());
            return doc;
        case PRIMITIVETYPE_Binary:
            return doc;
        case PRIMITIVETYPE_DateTime:
            doc.SetString(ecValue.GetDateTime().ToString().c_str(), doc.GetAllocator());
            return doc;
        case PRIMITIVETYPE_Double:
            doc.SetDouble(ecValue.GetDouble());
            return doc;
        case PRIMITIVETYPE_Integer:
            doc.SetInt(ecValue.GetInteger());
            return doc;
        case PRIMITIVETYPE_Long:
            doc.SetString(BeInt64Id(ecValue.GetLong()).ToHexStr().c_str(), doc.GetAllocator());
            return doc;
        case PRIMITIVETYPE_String:
            doc.SetString(ecValue.GetUtf8CP(), doc.GetAllocator());
            return doc;
        case PRIMITIVETYPE_Point2d:
            return GetPoint2dJson(ecValue.GetPoint2d(), allocator);
        case PRIMITIVETYPE_Point3d:
            return GetPoint3dJson(ecValue.GetPoint3d(), allocator);
        }
    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Unrecognized primitive property type: %d", (int)ecValue.GetPrimitiveType()));
    }

/*---------------------------------------------------------------------------------**//**
* Taken from BeStringUtilities::ParseUInt64, but with less checks and most importantly,
* returns the number instead of 0 when a non-digit character is found at the end.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static uint64_t ParseUInt64FromString(Utf8CP string)
    {
    uint64_t value = 0;
    for (; *string; ++string)
        {
        char c = *string;
        if (!isdigit(c))
            break;

        uint64_t digit = c - '0';
        value *= 10;
        value += digit;
        }
    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECInstanceKey GetECInstanceKeyFromJsonString(Utf8CP begin, Utf8CP end)
    {
    // format example:
    // {"c":207,"i":2199023261626}

    CharCP commaPtr = nullptr;
    for (auto c = begin; c < end; ++c)
        {
        if (*c == ',')
            {
            commaPtr = c;
            break;
            }
        }
    if (!commaPtr)
        return ECInstanceKey();

    // note: ParseUInt64FromString reads from given Utf8CP until it finds a non-digit, so in our case
    // it's fine to just give the starting pointer. In case of class id it's going to stop at comma,
    // in case of instance id it's going to stop at closing brace.
    ECClassId classId(ParseUInt64FromString(begin + 5));
    ECInstanceId instanceId(ParseUInt64FromString(commaPtr + 5));
    return ECInstanceKey(classId, instanceId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ValueHelpers::GetECInstanceKeyAsJson(ECInstanceKeyCR key, rapidjson::MemoryPoolAllocator<>* allocator)
    {
    rapidjson::Document json(rapidjson::kObjectType, allocator);
    json.AddMember("c", rapidjson::Value(key.GetClassId().GetValueUnchecked()), json.GetAllocator());
    json.AddMember("i", rapidjson::Value(key.GetInstanceId().GetValueUnchecked()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ValueHelpers::GetECInstanceKeyAsJsonString(ECInstanceKeyCR key)
    {
    // format example:
    // {"c":207,"i":2199023261626}
    return Utf8String("{\"c\":")
        .append(key.GetClassId().ToString())
        .append(",\"i\":")
        .append(key.GetInstanceId().ToString())
        .append("}");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey ValueHelpers::GetECInstanceKeyFromJson(RapidJsonValueCR json)
    {
    if (!json.IsObject())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to convert JSON to ECInstanceKey as it's not an object. Actual JSON type: %d", (int)json.GetType()));

    return ECInstanceKey(ECClassId(json["c"].GetUint64()), ECInstanceId(json["i"].GetUint64()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey ValueHelpers::GetECInstanceKeyFromJsonString(Utf8CP serializedJson)
    {
    if (nullptr == serializedJson || 0 == *serializedJson)
        return ECInstanceKey();

    return ::GetECInstanceKeyFromJsonString(serializedJson, serializedJson + strlen(serializedJson));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECInstanceKey> ValueHelpers::GetECInstanceKeysFromJson(RapidJsonValueCR json)
    {
    bvector<ECInstanceKey> list;
    if (json.IsArray())
        {
        for (rapidjson::SizeType i = 0; i < json.Size(); ++i)
            list.push_back(ECInstanceKey(ECClassId(json[i]["c"].GetUint64()), ECInstanceId(json[i]["i"].GetUint64())));
        }
    else if (json.IsObject())
        {
        list.push_back(ECInstanceKey(ECClassId(json["c"].GetUint64()), ECInstanceId(json["i"].GetUint64())));
        }
    else
        {
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to convert JSON to a list of ECInstanceKey as it's not an an array or object. Actual JSON type: %d", (int)json.GetType()));
        }
    return list;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECInstanceKey> ValueHelpers::GetECInstanceKeysFromJsonString(Utf8CP serializedJson)
    {
    bvector<ECInstanceKey> keys;
    if (nullptr == serializedJson || 0 == *serializedJson)
        return keys;

    if (serializedJson[0] == '{')
        {
        auto key = GetECInstanceKeyFromJsonString(serializedJson);
        if (key.IsValid())
            keys.push_back(key);
        }
    else if (serializedJson[0] == '[')
        {
        auto len = strlen(serializedJson);
        Utf8CP keyStart = nullptr;
        for (Utf8CP curr = serializedJson + 1; curr < serializedJson + len; ++curr)
            {
            if (*curr == '{')
                keyStart = curr;
            else if (*curr == '}' && keyStart)
                {
                auto key = ::GetECInstanceKeyFromJsonString(keyStart, curr);
                if (key.IsValid())
                    keys.push_back(key);
                }
            }
        }
    else
        {
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to convert serialized JSON to a list of ECInstanceKey as it's not an an array or object. Value: `%s`", serializedJson));
        }
    return keys;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassInstanceKey ValueHelpers::GetECClassInstanceKey(SchemaManagerCR schemas, ECInstanceKeyCR key)
    {
    return ECClassInstanceKey(schemas.GetClass(key.GetClassId()), key.GetInstanceId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ValueHelpers::PrimitiveTypeAsString(PrimitiveType type)
    {
    switch (type)
        {
        case PRIMITIVETYPE_Binary:
            return "binary";
        case PRIMITIVETYPE_Boolean:
            return "boolean";
        case PRIMITIVETYPE_DateTime:
            return "dateTime";
        case PRIMITIVETYPE_Double:
            return "double";
        case PRIMITIVETYPE_Integer:
            return "int";
        case PRIMITIVETYPE_Long:
            return "long";
        case PRIMITIVETYPE_Point2d:
            return "point2d";
        case PRIMITIVETYPE_Point3d:
            return "point3d";
        case PRIMITIVETYPE_String:
            return "string";
        default:
            return "";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsInteger(int c) {return '0' <= c && c <= '9';}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
#define PADDING 10
static Utf8String GetPaddedNumber(Utf8CP chars, int length)
    {
    Utf8String padded;
    padded.reserve(PADDING);
    for (int i = length; i < PADDING; i++)
        padded.append("0");
    padded.append(chars, length);
    return padded;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ValueHelpers::PadNumbersInString(Utf8StringCR inputStr)
    {
    Utf8CP inputP = inputStr.c_str();
    Utf8CP numberBegin = nullptr;
    Utf8String output;
    output.reserve(strlen(inputP));
    while (nullptr != inputP && 0 != *inputP)
        {
        if (IsInteger(*inputP))
            {
            if (nullptr == numberBegin)
                {
                numberBegin = inputP;
                output.reserve(output.size() + PADDING + strlen(inputP));
                }
            }
        else
            {
            if (nullptr != numberBegin)
                {
                output.append(GetPaddedNumber(numberBegin, (int)(inputP - numberBegin)));
                numberBegin = nullptr;
                }
            Utf8Char c = *inputP;
            output.append(1, (Utf8Char)std::tolower(c));
            }
        inputP++;
        }
    if (nullptr != numberBegin)
        output.append(GetPaddedNumber(numberBegin, (int)(inputP - numberBegin)));
    return output;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ValueHelpers::GuidToString(BeGuidCR guid)
    {
    static Utf8String s_default = "00000000-0000-0000-0000-000000000000";
    return guid.IsValid() ? guid.ToString() : s_default;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<Utf8String> const& GetUnitSystemGroupNames(ECPresentation::UnitSystem group)
    {
    static bvector<Utf8String> s_metricUnitSystems{ "SI", "METRIC", "INTERNATIONAL", "FINANCE" };
    static bvector<Utf8String> s_britishImperialUnitSystems{ "IMPERIAL", "USCUSTOM", "INTERNATIONAL", "FINANCE" };
    static bvector<Utf8String> s_usCustomaryUnitSystems{ "USCUSTOM", "INTERNATIONAL", "FINANCE" };
    static bvector<Utf8String> s_usSurveyUnitSystems{ "USSURVEY", "USCUSTOM", "INTERNATIONAL", "FINANCE" };
    static bvector<Utf8String> s_empty;
    switch (group)
        {
        case ECPresentation::UnitSystem::Metric: return s_metricUnitSystems;
        case ECPresentation::UnitSystem::BritishImperial: return s_britishImperialUnitSystems;
        case ECPresentation::UnitSystem::UsCustomary: return s_usCustomaryUnitSystems;
        case ECPresentation::UnitSystem::UsSurvey: return s_usSurveyUnitSystems;
        }
    return s_empty;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Formatting::Format const* ValueHelpers::GetPresentationFormat(KindOfQuantityCR koq, ECPresentation::UnitSystem unitSystemGroup, std::map<std::pair<Utf8String, ECPresentation::UnitSystem>, std::shared_ptr<Formatting::Format>> const& defaultFormats)
    {
    Formatting::Format const* format = nullptr;
    for (Utf8StringCR unitSystemName : GetUnitSystemGroupNames(unitSystemGroup))
        {
        // find the first presentation format that uses one of the unit systems in the group
        auto formatIter = std::find_if(koq.GetPresentationFormats().begin(), koq.GetPresentationFormats().end(), [&unitSystemName](NamedFormatCR f)
            {
            return f.HasCompositeMajorUnit()
                && f.GetCompositeMajorUnit()->GetUnitSystem()
                && f.GetCompositeMajorUnit()->GetUnitSystem()->GetName().EqualsI(unitSystemName);
            });
        if (koq.GetPresentationFormats().end() != formatIter)
            {
            format = &*formatIter;
            break;
            }
        }
    if (!format)
        {
        Formatting::Format const* defaultFormat = koq.GetDefaultPresentationFormat();
        if (defaultFormat->HasCompositeMajorUnit())
            {
            std::pair<Utf8String, UnitSystem> pair = std::make_pair(defaultFormat->GetCompositeMajorUnit()->GetPhenomenon()->GetName(), unitSystemGroup);
            auto it = defaultFormats.find(pair);
            if (it != defaultFormats.end())
                format = it->second.get();
            }
        // if format based on requested unit systems group was not found, use default
        if (!format)
            format = defaultFormat;
        }

    return format;
    }


// TODO: This method should only be used while the transition from RapidJson/JsonValue to BeJsConst isn't finished. It should be deleted afterwards.
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ValueHelpers::ToRapidJson(BeJsConst json)
    {
    rapidjson::Document doc;
    doc.Parse(json.Stringify().c_str());
    return doc;
    }
