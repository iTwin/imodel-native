/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "ValueHelpers.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ValueHelpers::GetEnumPropertyDisplayValue(Utf8StringR displayValue, ECPropertyCR prop,
    std::function<int()> getIntEnumId, std::function<Utf8CP()> getStrEnumId)
    {
    if (!prop.GetIsPrimitive() && !prop.GetIsPrimitiveArray())
        return ERROR;

    ECEnumerationCP enumeration = prop.GetIsPrimitive() ? prop.GetAsPrimitiveProperty()->GetEnumeration() : prop.GetAsPrimitiveArrayProperty()->GetEnumeration();
    if (nullptr == enumeration)
        return ERROR;

    ECEnumeratorCP enumerator = nullptr;
    switch (enumeration->GetType())
        {
        case PRIMITIVETYPE_Integer:
            {
            enumerator = enumeration->FindEnumerator(getIntEnumId());
            break;
            }
        case PRIMITIVETYPE_String:
            {
            enumerator = enumeration->FindEnumerator(getStrEnumId());
            break;
            }
        }

    if (nullptr == enumerator)
        {
        BeAssert(false);
        return ERROR;
        }

    displayValue = enumerator->GetDisplayLabel();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ValueHelpers::GetEnumPropertyDisplayValue(Utf8StringR displayValue, ECPropertyCR prop, ECValueCR ecValue)
    {
    if (ecValue.IsNull())
        return ERROR;
    return GetEnumPropertyDisplayValue(displayValue, prop,
        [&ecValue](){BeAssert(ecValue.IsInteger()); return ecValue.GetInteger();},
        [&ecValue](){BeAssert(ecValue.IsUtf8()); return ecValue.GetUtf8CP();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ValueHelpers::GetEnumPropertyDisplayValue(Utf8StringR displayValue, ECPropertyCR prop, DbValue const& dbValue)
    {
    if (dbValue.IsNull())
        return ERROR;
    return GetEnumPropertyDisplayValue(displayValue, prop,
        [&dbValue](){BeAssert(dbValue.GetValueType() == DbValueType::IntegerVal); return dbValue.GetValueInt();},
        [&dbValue](){BeAssert(dbValue.GetValueType() == DbValueType::TextVal); return dbValue.GetValueText();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ValueHelpers::GetEnumPropertyDisplayValue(Utf8StringR displayValue, ECPropertyCR prop, RapidJsonValueCR jsonValue)
    {
    if (jsonValue.IsNull())
        return ERROR;
    return GetEnumPropertyDisplayValue(displayValue, prop,
        [&jsonValue](){BeAssert(jsonValue.IsInt()); return jsonValue.GetInt();},
        [&jsonValue](){BeAssert(jsonValue.IsString()); return jsonValue.GetString();});
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static rapidjson::Document ParseJson(Utf8CP serialized, rapidjson::MemoryPoolAllocator<>* allocator)
    {
    rapidjson::Document json(allocator);
    json.Parse(serialized);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d ValueHelpers::GetPoint2dFromSqlValue(IECSqlValue const& value)
    {
    if (PRIMITIVETYPE_Point2d == value.GetColumnInfo().GetDataType().GetPrimitiveType())
        return value.GetPoint2d();
    // why does this assert when value IS actually a string?
    //BeAssert(PRIMITIVETYPE_String == value.GetColumnInfo().GetDataType().GetPrimitiveType());
    return GetPoint2dFromJsonString(value.GetText());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d ValueHelpers::GetPoint3dFromSqlValue(IECSqlValue const& value)
    {
    if (PRIMITIVETYPE_Point3d == value.GetColumnInfo().GetDataType().GetPrimitiveType())
        return value.GetPoint3d();
    // why does this assert when value IS actually a string?
    // RulesDrivenECPresentationManagerContentTests.ReturnsPointPropertyContent
    //BeAssert(PRIMITIVETYPE_String == value.GetColumnInfo().GetDataType().GetPrimitiveType());
    return GetPoint3dFromJsonString(value.GetText());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d ValueHelpers::GetPoint2dFromJson(JsonValueCR json)
    {
    if (json.isNull() || !json.isObject())
        return DPoint2d();
    return DPoint2d::From(json["x"].asDouble(), json["y"].asDouble());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d ValueHelpers::GetPoint2dFromJson(RapidJsonValueCR json)
    {
    if (json.IsNull() || !json.IsObject())
        return DPoint2d();
    return DPoint2d::From(json["x"].GetDouble(), json["y"].GetDouble());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d ValueHelpers::GetPoint3dFromJson(JsonValueCR json)
    {
    if (json.isNull() || !json.isObject())
        return DPoint3d();
    return DPoint3d::From(json["x"].asDouble(), json["y"].asDouble(), json["z"].asDouble());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d ValueHelpers::GetPoint3dFromJson(RapidJsonValueCR json)
    {
    if (json.IsNull() || !json.IsObject())
        return DPoint3d();
    return DPoint3d::From(json["x"].GetDouble(), json["y"].GetDouble(), json["z"].GetDouble());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Aidas.Vaiksnoras                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d ValueHelpers::GetPoint2dFromJsonString(Utf8CP str)
    {
    rapidjson::Document json = ParseJson(str, nullptr);
    if (json.IsNull() || !json.IsObject())
        return DPoint2d();
    return DPoint2d::From(json["x"].GetDouble(), json["y"].GetDouble());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Aidas.Vaiksnoras                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d ValueHelpers::GetPoint3dFromJsonString(Utf8CP str)
    {
    rapidjson::Document json = ParseJson(str, nullptr);
    if (json.IsNull() || !json.IsObject())
        return DPoint3d();
    return DPoint3d::From(json["x"].GetDouble(), json["y"].GetDouble(), json["z"].GetDouble());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                05/2017
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
    BeAssert(n == values.size());
    return values;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ValueHelpers::GetPoint2dJsonFromString(Utf8StringCR str, rapidjson::MemoryPoolAllocator<>* allocator)
    {
    bvector<double> values = GetNDoublesFromString(str, 2);
    if (2 != values.size())
        {
        BeAssert(false && "Failed to parse Point2d from string");
        return rapidjson::Document(allocator);
        }
    return GetPoint2dJson(DPoint2d::FromArray(&values[0]), allocator);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ValueHelpers::GetPoint3dJsonFromString(Utf8StringCR str, rapidjson::MemoryPoolAllocator<>* allocator)
    {
    bvector<double> values = GetNDoublesFromString(str, 3);
    if (3 != values.size())
        {
        BeAssert(false && "Failed to parse Point3d from string");
        return rapidjson::Document(allocator);
        }
    return GetPoint3dJson(DPoint3d::FromArray(&values[0]), allocator);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                06/2017
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
// @bsimethod                                    Grigas.Petraitis                06/2017
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
// @bsimethod                                    Grigas.Petraitis                04/2016
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
    BeAssert(false);
    return doc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ValueHelpers::GetJsonFromStructValue(ECStructClassCR structClass, IECSqlValue const& sqlValue, rapidjson::MemoryPoolAllocator<>* allocator)
    {
    NULL_VALUE_PRECONDITION(sqlValue);

    BeAssert(sqlValue.GetColumnInfo().GetStructType() == &structClass);
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
* @bsimethod                                    Grigas.Petraitis                06/2017
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
* @bsimethod                                    Grigas.Petraitis                05/2017
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
                BeAssert(false);
            else
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
            doc = GetPoint2dJsonFromString(str, &doc.GetAllocator());
            return doc;
        case PRIMITIVETYPE_Point3d:
            doc = GetPoint3dJsonFromString(str, &doc.GetAllocator());
            return doc;
        }
    BeAssert(false);
    return doc;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue ValueHelpers::GetECValueFromSqlValue(PrimitiveType primitiveType, IECSqlValue const& sqlValue)
    {
    ECValue value;
    if (VALUEKIND_Primitive != sqlValue.GetColumnInfo().GetDataType().GetTypeKind())
        {
        BeAssert(false);
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
            BeAssert(false);
        }
    if (sqlValue.IsNull())
        value.SetIsNull(true);
    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue ValueHelpers::GetECValueFromString(PrimitiveType valueType, Utf8StringCR str)
    {
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
            if (SUCCESS == DateTime::FromString(dt, str.c_str()))
                return ECValue(dt);
            BeAssert(false);
            }
        }

    return ECValue(str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue ValueHelpers::GetECValueFromJson(ECPropertyCR ecProperty, JsonValueCR json)
    {
    ECValue value;
    if (json.isNull())
        {
        value.SetIsNull(true);
        return value;
        }

    if (ecProperty.GetIsNavigation())
        {
        value.SetNavigationInfo(BeInt64Id(json.asUInt64()));
        return value;
        }

    if (!ecProperty.GetIsPrimitive())
        {
        BeAssert(false);
        return value;
        }

    switch (ecProperty.GetAsPrimitiveProperty()->GetType())
        {
        case PRIMITIVETYPE_Boolean:
            value.SetBoolean(json.asBool());
            break;
        case PRIMITIVETYPE_Binary:
            break;
        case PRIMITIVETYPE_DateTime:
            {
            DateTime dt;
            if (json.isDouble())
                DateTime::FromJulianDay(dt, json.asDouble(), DateTime::Info::CreateForDateTime(DateTime::Kind::Utc));
            else
                DateTime::FromString(dt, json.asString().c_str());
            value.SetDateTime(dt);
            break;
            }
        case PRIMITIVETYPE_Double:
            value.SetDouble(json.asDouble());
            break;
        case PRIMITIVETYPE_Integer:
            value.SetInteger(json.asInt());
            break;
        case PRIMITIVETYPE_Long:
            if (json.isString())
                value.SetLong(BeInt64Id::FromString(json.asCString()).GetValueUnchecked());
            else
                value.SetLong(json.asInt64());
            break;
        case PRIMITIVETYPE_String:
            value.SetUtf8CP(json.asCString());
            break;
        case PRIMITIVETYPE_Point2d:
            value.SetPoint2d(GetPoint2dFromJson(json));
            break;
        case PRIMITIVETYPE_Point3d:
            value.SetPoint3d(GetPoint3dFromJson(json));
            break;
        default:
            BeAssert(false);
        }
    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
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
            BeAssert(false);
        }
    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
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
            doc = GetPoint2dJson(ecValue.GetPoint2d(), &doc.GetAllocator());
            return doc;
        case PRIMITIVETYPE_Point3d:
            doc = GetPoint3dJson(ecValue.GetPoint3d(), &doc.GetAllocator());
            return doc;
        }
    BeAssert(false);
    return doc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ValueHelpers::GetJsonAsString(RapidJsonValueCR json)
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
            BeAssert(json.IsUint64() || json.IsInt64() || json.IsDouble());
            if (json.IsUint64())
                return std::to_string(json.GetUint64()).c_str();
            if (json.IsInt64())
                return std::to_string(json.GetInt64()).c_str();
            if (json.IsDouble())
                return Utf8PrintfString("%.2f", json.GetDouble());
            }
        }
    BeAssert(false);
    rapidjson::StringBuffer buf;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buf);
    json.Accept(writer);
    return buf.GetString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECInstanceKey> ValueHelpers::GetECInstanceKeysFromSerializedJson(Utf8CP serializedJson)
    {
    if (nullptr == serializedJson || 0 == *serializedJson)
        {
        BeAssert(false);
        return bvector<ECInstanceKey>();
        }

    rapidjson::Document json;
    json.Parse(serializedJson);

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
        BeAssert(false);
        }    
    return list;
    }
