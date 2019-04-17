/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ECObjects/ECQuantityFormatting.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <Bentley/Base64Utilities.h>
#include <GeomSerialization/GeomSerializationApi.h>
#include <GeomSerialization/GeomLibsSerialization.h>
#include <GeomSerialization/GeomLibsJsonSerialization.h>
#include <json/value.h>

BEGIN_UNNAMED_NAMESPACE
    BE_JSON_NAME(rawValue)
    BE_JSON_NAME(formattedValue)
    BE_JSON_NAME(currentUnit)
END_UNNAMED_NAMESPACE

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2017
//---------------------------------------------------------------------------------------
//static
ECClassCP ECJsonUtilities::GetClassFromClassNameJson(JsonValueCR json, IECClassLocaterR classLocater)
    {
    if (!json.isString())
        return nullptr;

    //support both delimiters (The colon is common in ECObjects. The dot is common elsewhere)
    bvector<Utf8String> tokens;
    BeStringUtilities::Split(json.asCString(), ".:", tokens);
    if (tokens.size() != 2)
        return nullptr;

    return classLocater.LocateClass(tokens[0].c_str(), tokens[1].c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2017
//---------------------------------------------------------------------------------------
//static
ECClassId ECJsonUtilities::GetClassIdFromClassNameJson(JsonValueCR json, IECClassLocaterR classLocater)
    {
    if (!json.isString())
        return ECClassId();

    //support both delimiters (The colon is common in ECObjects. The dot is common elsewhere)
    bvector<Utf8String> tokens;
    BeStringUtilities::Split(json.asCString(), ".:", tokens);
    if (tokens.size() != 2)
        return ECClassId();

    return classLocater.LocateClassId(tokens[0].c_str(), tokens[1].c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::IdToJson(Json::Value& json, BeInt64Id id)
    {
    if (!id.IsValid())
        return ERROR;

    json = id.ToHexStr();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2017
//---------------------------------------------------------------------------------------
//static
void ECJsonUtilities::Int64ToJson(Json::Value& json, int64_t int64Val, ECJsonInt64Format int64Format)
    {
    switch (int64Format)
        {
        case ECJsonInt64Format::AsNumber:
            json = Json::Value(int64Val);
            return;

        case ECJsonInt64Format::AsDecimalString:
            json = BeJsonUtilities::StringValueFromInt64(int64Val);
            return;

        case ECJsonInt64Format::AsHexadecimalString:
            {
            BeInt64Id id(int64Val);
            json = id.ToHexStr();
            return;
            }

        default:
            break;
        }

    BeAssert(false && "Unhandled ECJsonFormatOptions type");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::JsonToInt64(int64_t& int64Val, JsonValueCR json)
    {
    if (json.isNull())
        return ERROR;

    if (json.isString())
        {
        Utf8CP strVal = json.asCString();
        if (BeStringUtilities::HasHexPrefix(strVal))
            {
            BentleyStatus hexParseStat = SUCCESS;
            int64Val = (int64_t) BeStringUtilities::ParseHex(strVal, &hexParseStat);
            return hexParseStat;
            }

        sscanf(strVal, "%" SCNi64, &int64Val);
        return SUCCESS;
        }

    if (json.isNumeric())
        {
        if (json.isUInt())
            int64Val = json.asUInt64();
        else
            int64Val = json.asInt64();

        return SUCCESS;
        }

    return ERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::JsonToDateTime(DateTime& dateTime, JsonValueCR json)
    {
    if (!json.isString())
        return ERROR;

    return DateTime::FromString(dateTime, json.asCString());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::BinaryToJson(Json::Value& json, Byte const* binary, size_t binarySize)
    {
    if (binarySize == 0)
        {
        json = Json::nullValue;
        return SUCCESS;
        }

    Utf8String str;
    Base64Utilities::Encode(str, binary, binarySize);
    json = Json::Value(str);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::JsonToBinary(bvector<Byte>& binary, Json::Value const& json)
    {
    binary.clear();

    if (json.isNull())
        return SUCCESS;

    if (!json.isString())
        return ERROR;

    Base64Utilities::Decode(binary, json.asString());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::JsonToBinary(ByteStream& byteStream, Json::Value const& json)
    {
    if (json.isNull())
        {
        byteStream.clear();
        return SUCCESS;
        }

    if (!json.isString())
        return ERROR;

    Utf8CP base64Str = json.asCString();
    Base64Utilities::Decode(byteStream, base64Str, strlen(base64Str));
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::Point2dToJson(Json::Value& json, DPoint2d pt)
    {
    json = Json::Value(Json::objectValue);
    json[json_x()] = pt.x;
    json[json_y()] = pt.y;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::JsonToPoint2d(DPoint2d& pt, Json::Value const& json)
    {
    if (json.isNull())
        return ERROR;

    double x = 0.0;
    double y = 0.0;

    switch (json.type())
        {
        case Json::ValueType::objectValue:
            {
            if (SUCCESS != PointCoordinateFromJson(x, json, json_x()) ||
                SUCCESS != PointCoordinateFromJson(y, json, json_y()))
                return ERROR;
            }
            break;
        case Json::ValueType::arrayValue:
            {
            // Must be equal to the length of a valid DPoint2d array.
            if (2 != json.size())
                return ERROR;
            x = json[0u].asDouble();
            y = json[1u].asDouble();
            }
            break;
        default:
            return ERROR;
        }

    pt = DPoint2d::From(x, y);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::Point3dToJson(Json::Value& json, DPoint3d pt)
    {
    json = Json::Value(Json::objectValue);
    json[json_x()] = pt.x;
    json[json_y()] = pt.y;
    json[json_z()] = pt.z;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::JsonToPoint3d(DPoint3d& pt, Json::Value const& json)
    {
    if (json.isNull())
        return ERROR;

    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    switch (json.type())
        {
        case Json::ValueType::objectValue:
            {
            if (SUCCESS != PointCoordinateFromJson(x, json, json_x()) ||
                SUCCESS != PointCoordinateFromJson(y, json, json_y()) ||
                SUCCESS != PointCoordinateFromJson(z, json, json_z()))
                return ERROR;
            }
            break;
        case Json::ValueType::arrayValue:
            {
            // Must be equal to the length of a valid DPoint3d array.
            if (3 != json.size())
                return ERROR;
            x = json[0u].asDouble();
            y = json[1u].asDouble();
            z = json[2u].asDouble();
            }
            break;
        default:
            return ERROR;
        }

    pt = DPoint3d::From(x, y, z);
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::PointCoordinateFromJson(double& coordinate, Json::Value const& json, Json::StaticString const& coordinateKey)
    {
    if (json.isNull() || !json.isObject())
        return ERROR;

    Json::Value const& coordinateJson = json[coordinateKey];
    if (coordinateJson.isNull() || !coordinateJson.isNumeric())
        return ERROR;

    coordinate = coordinateJson.asDouble();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::IGeometryToJson(Json::Value& json, IGeometryCR geom)
    {
    return BentleyGeometryJson::TryGeometryToJsonValue(json, geom, false) ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2017
//---------------------------------------------------------------------------------------
//static
IGeometryPtr ECJsonUtilities::JsonToIGeometry(JsonValueCR json)
    {
    bvector<IGeometryPtr> geometry;
    if (!BentleyGeometryJson::TryJsonValueToGeometry(json, geometry) || geometry.empty())
        return nullptr;

    return geometry[0];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::IdToJson(RapidJsonValueR json, BeInt64Id id, rapidjson::MemoryPoolAllocator<>& allocator)
    {
    if (!id.IsValid())
        return ERROR;

    Utf8String hexStr = id.ToHexStr();
    json.SetString(hexStr.c_str(), (rapidjson::SizeType) hexStr.size(), allocator);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2017
//---------------------------------------------------------------------------------------
//static
void ECJsonUtilities::ClassToJson(RapidJsonValueR json, ECClassCR ecClass, rapidjson::MemoryPoolAllocator<>& allocator)
    {
    Utf8String fullName = ECJsonUtilities::FormatClassName(ecClass);
    json.SetString(fullName.c_str(), (rapidjson::SizeType) fullName.size(), allocator);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2017
//---------------------------------------------------------------------------------------
//static
ECClassCP ECJsonUtilities::GetClassFromClassNameJson(RapidJsonValueCR json, IECClassLocaterR classLocater)
    {
    if (!json.IsString())
        return nullptr;

    //support both delimiters (The colon is common in ECObjects. The dot is common elsewhere)
    bvector<Utf8String> tokens;
    BeStringUtilities::Split(json.GetString(), ".:", tokens);
    if (tokens.size() != 2)
        return nullptr;

    return classLocater.LocateClass(tokens[0].c_str(), tokens[1].c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2017
//---------------------------------------------------------------------------------------
//static
ECClassId ECJsonUtilities::GetClassIdFromClassNameJson(RapidJsonValueCR json, IECClassLocaterR classLocater)
    {
    if (!json.IsString())
        return ECClassId();

    //support both delimiters (The colon is common in ECObjects. The dot is common elsewhere)
    bvector<Utf8String> tokens;
    BeStringUtilities::Split(json.GetString(), ".:", tokens);
    if (tokens.size() != 2)
        return ECClassId();

    return classLocater.LocateClassId(tokens[0].c_str(), tokens[1].c_str());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    07/2016
//+---------------+---------------+---------------+---------------+---------------+------
void ECJsonUtilities::Int64ToJson(RapidJsonValueR json, int64_t val, rapidjson::MemoryPoolAllocator<>& allocator, ECJsonInt64Format int64Format)
    {
    switch (int64Format)
        {
            case ECJsonInt64Format::AsNumber:
                json.SetInt64(val);
                return;

            case ECJsonInt64Format::AsDecimalString:
            {
            char str[32];
            const int len = sprintf(str, "%" PRIi64, val);
            json.SetString(str, len, allocator);
            return;
            }

            case ECJsonInt64Format::AsHexadecimalString:
            {
            BeInt64Id id(val);
            Utf8String hexStr = id.ToHexStr();
            json.SetString(hexStr.c_str(), (rapidjson::SizeType) hexStr.size(), allocator);
            return;
            }

            default:
                break;
        }

    BeAssert(false && "Unhandled ECJsonInt64Format");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECJsonUtilities::JsonToInt64(int64_t& val, RapidJsonValueCR json)
    {
    if (json.IsNull())
        return ERROR;

    if (json.IsNumber())
        {
        if (json.IsInt() || json.IsInt64())
            val = json.GetInt64();
        else if (json.IsUint() || json.IsUint64())
            val = json.GetUint64();
        else if (json.IsFloat() || json.IsDouble())
            val = (int64_t) json.GetDouble();
        else
            return ERROR;

        return SUCCESS;
        }

    if (json.IsString())
        {
        Utf8CP strVal = json.GetString();
        if (BeStringUtilities::HasHexPrefix(strVal))
            {
            BentleyStatus hexParseStat = SUCCESS;
            val = (int64_t) BeStringUtilities::ParseHex(strVal, &hexParseStat);
            return hexParseStat;
            }

        sscanf(strVal, "%" SCNi64, &val);
        return SUCCESS;
        }

    return ERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::JsonToDateTime(DateTime& dateTime, RapidJsonValueCR json)
    {
    if (!json.IsString())
        return ERROR;

    return DateTime::FromString(dateTime, json.GetString());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::BinaryToJson(RapidJsonValueR json, Byte const* binary, size_t binarySize, rapidjson::MemoryPoolAllocator<>& allocator)
    {
    if (binarySize == 0)
        {
        json.SetNull();
        return SUCCESS;
        }

    Utf8String str;
    Base64Utilities::Encode(str, binary, binarySize);

    json.SetString(str.c_str(), (rapidjson::SizeType) str.size(), allocator);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::JsonToBinary(bvector<Byte>& binary, RapidJsonValueCR json)
    {
    if (!json.IsString())
        return ERROR;

    binary.clear();

    if (json.IsNull())
        return SUCCESS;

    Base64Utilities::Decode(binary, json.GetString(), (size_t) json.GetStringLength());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::JsonToBinary(ByteStream& binary, RapidJsonValueCR json)
    {
    if (!json.IsString())
        return ERROR;

    if (json.IsNull())
        {
        binary.Clear();
        return SUCCESS;
        }

    Base64Utilities::Decode(binary, json.GetString(), (size_t) json.GetStringLength());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::Point2dToJson(RapidJsonValueR json, DPoint2d pt, rapidjson::MemoryPoolAllocator<>& allocator)
    {
    json.SetObject();
    rapidjson::Value coordVal(pt.x);
    json.AddMember(rapidjson::StringRef(ECJsonSystemNames::Point::X()), coordVal, allocator);
    coordVal.SetDouble(pt.y);
    json.AddMember(rapidjson::StringRef(ECJsonSystemNames::Point::Y()), coordVal, allocator);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::JsonToPoint2d(DPoint2d& pt, RapidJsonValueCR json)
    {
    if (json.IsNull())
        return ERROR;

    double x = 0.0;
    double y = 0.0;

    switch (json.GetType())
        {
        case rapidjson::Type::kObjectType:
            {
            if (SUCCESS != PointCoordinateFromJson(x, json, ECJsonSystemNames::Point::X()) ||
                SUCCESS != PointCoordinateFromJson(y, json, ECJsonSystemNames::Point::Y()))
                return ERROR;
            }
            break;
        case rapidjson::Type::kArrayType:
            {
            // Must be equal to the length of a valid DPoint2d array.
            if (2 != json.Size())
                return ERROR;
            x = json.GetArray()[0u].GetDouble();
            y = json.GetArray()[1u].GetDouble();
            }
            break;
        default:
            return ERROR;
        }

    pt = DPoint2d::From(x, y);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::Point3dToJson(RapidJsonValueR json, DPoint3d pt, rapidjson::MemoryPoolAllocator<>& allocator)
    {
    json.SetObject();
    rapidjson::Value coordVal(pt.x);
    json.AddMember(rapidjson::StringRef(ECJsonSystemNames::Point::X()), coordVal, allocator);
    coordVal.SetDouble(pt.y);
    json.AddMember(rapidjson::StringRef(ECJsonSystemNames::Point::Y()), coordVal, allocator);
    coordVal.SetDouble(pt.z);
    json.AddMember(rapidjson::StringRef(ECJsonSystemNames::Point::Z()), coordVal, allocator);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::JsonToPoint3d(DPoint3d& pt, RapidJsonValueCR json)
    {
    if (json.IsNull())
        return ERROR;

    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    switch (json.GetType())
        {
        case rapidjson::Type::kObjectType:
            {
            if (SUCCESS != PointCoordinateFromJson(x, json, ECJsonSystemNames::Point::X()) ||
                SUCCESS != PointCoordinateFromJson(y, json, ECJsonSystemNames::Point::Y()) ||
                SUCCESS != PointCoordinateFromJson(z, json, ECJsonSystemNames::Point::Z()))
                return ERROR;
            }
            break;
        case rapidjson::Type::kArrayType:
            {
            // Must be equal to the length of a valid DPoint3d array.
            if (3 != json.Size())
                return ERROR;
            x = json.GetArray()[0u].GetDouble();
            y = json.GetArray()[1u].GetDouble();
            z = json.GetArray()[2u].GetDouble();
            }
            break;
        default:
            return ERROR;
        }

    pt = DPoint3d::From(x, y, z);
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::PointCoordinateFromJson(double& coordinate, RapidJsonValueCR json, Utf8CP coordinateKey)
    {
    auto it = json.FindMember(coordinateKey);
    if (it == json.MemberEnd() || it->value.IsNull() || !it->value.IsNumber())
        return ERROR;

    coordinate = it->value.GetDouble();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::IGeometryToJson(RapidJsonValueR json, IGeometryCR geom, rapidjson::MemoryPoolAllocator<>& allocator)
    {
    Utf8String jsonStr;
    if (!BentleyGeometryJson::TryGeometryToJsonString(jsonStr, geom, false) || jsonStr.empty())
        return ERROR;

    rapidjson::Document jsonDoc;
    if (jsonDoc.Parse<0>(jsonStr.c_str()).HasParseError())
        return ERROR;

    json.CopyFrom(jsonDoc, allocator);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2017
//---------------------------------------------------------------------------------------
//static
IGeometryPtr ECJsonUtilities::JsonToIGeometry(RapidJsonValueCR json)
    {
    if (!json.IsObject())
        return nullptr;

    rapidjson::StringBuffer jsonStr;
    rapidjson::Writer<rapidjson::StringBuffer> writer(jsonStr);
    json.Accept(writer);

    bvector<IGeometryPtr> geometry;
    if (!BentleyGeometryJson::TryJsonStringToGeometry(jsonStr.GetString(), geometry) || geometry.empty())
        return nullptr;

    return geometry[0];
    }


//*************************************************************************************
// JsonECInstanceConverter
//*************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 2/2013
//---------------------------------------------------------------------------------------
BentleyStatus JsonECInstanceConverter::JsonToECInstance(IECInstanceR instance, Json::Value const& jsonValue, IECClassLocaterR classLocater, bool ignoreUnknownProperties, IECSchemaRemapperCP remapper)
    {
    return JsonToECInstance(instance, jsonValue, instance.GetClass(), Utf8String(), classLocater, ignoreUnknownProperties, remapper);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                02/2018
//---------------------------------------------------------------------------------------
BentleyStatus JsonECInstanceConverter::JsonToECInstance(ECN::IECInstanceR instance, Json::Value const& jsonValue, IECClassLocaterR classLocater, std::function<bool(Utf8CP)> shouldSerializeProperty)
    {
    return JsonToECInstance(instance, jsonValue, instance.GetClass(), Utf8String(), classLocater, false, nullptr, shouldSerializeProperty);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 2/2013
//---------------------------------------------------------------------------------------
BentleyStatus JsonECInstanceConverter::JsonToECInstance(IECInstanceR instance, Json::Value const& jsonValue, ECClassCR currentClass, Utf8StringCR currentAccessString, IECClassLocaterR classLocater, bool ignoreUnknownProperties, IECSchemaRemapperCP remapper, std::function<bool(Utf8CP)> shouldSerializeProperty)
    {
    if (!jsonValue.isObject())
        return ERROR;

    bool checkShouldSerializeProperty = shouldSerializeProperty != nullptr;
    for (Json::Value::iterator iter = jsonValue.begin(); iter != jsonValue.end(); iter++)
        {
        Json::Value const& childJsonValue = *iter;
        Utf8CP memberName = iter.memberName();
        if (ECJsonSystemNames::IsTopLevelSystemMember(memberName))
            continue;
        if (checkShouldSerializeProperty && !shouldSerializeProperty(memberName))
            continue;

        Utf8String remappedMemberName(memberName);
        if (nullptr != remapper)
            remapper->ResolvePropertyName(remappedMemberName, currentClass);
        ECPropertyP ecProperty = currentClass.GetPropertyP(remappedMemberName.c_str());
        if (ecProperty == nullptr)
            {
            if (ignoreUnknownProperties)
                continue;
            return ERROR;
            }

        Utf8String accessString = currentAccessString.empty() ? remappedMemberName : currentAccessString + "." + remappedMemberName;
        if (ecProperty->GetIsPrimitive())
            {
            ECValue ecValue;
            if (SUCCESS != JsonToPrimitiveECValue(ecValue, childJsonValue, ecProperty->GetAsPrimitiveProperty()->GetType()))
                return ERROR;

            ECObjectsStatus ecStatus;
            ecStatus = instance.SetInternalValue(accessString.c_str(), ecValue);
            BeAssert(ecStatus == ECObjectsStatus::Success || ecStatus == ECObjectsStatus::PropertyValueMatchesNoChange);
            continue;
            }
        else if (ecProperty->GetIsStruct())
            {
            if (SUCCESS != JsonToECInstance(instance, childJsonValue, ecProperty->GetAsStructProperty()->GetType(), accessString, classLocater, ignoreUnknownProperties, remapper))
                return ERROR;

            continue;
            }
        else if (ecProperty->GetIsArray())
            {
            if (SUCCESS != JsonToArrayECValue(instance, childJsonValue, *ecProperty->GetAsArrayProperty(), accessString, classLocater))
                return ERROR;
            }
        else if (ecProperty->GetIsNavigation())
            {
            NavigationECPropertyCP navProp = ecProperty->GetAsNavigationProperty();
            if (navProp->IsMultiple())
                {
                LOG.error("NavigationECProperties with IsMultiple == true not supported by ECJsonUtilities");
                return ERROR;
                }

            if (!childJsonValue.isNull() && !childJsonValue.isObject())
                return ERROR;

            //the existence of a nav prop JSON object means we will not ignore it regardless of whether its member exist or not
            ECValue v;
            v.SetToNull();
            if (childJsonValue.isNull())
                {
                ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), v);
                if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
                    return ERROR;

                continue;
                }

            if (!childJsonValue.isMember(ECJsonUtilities::json_navId()))
                return ERROR; //nav prop JSON requires the id member to be present

            JsonValueCR navIdJson = childJsonValue[ECJsonUtilities::json_navId()];
            if (navIdJson.isNull())
                {
                ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), v);
                if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
                    return ERROR;

                continue;
                }

            BeInt64Id navId = ECJsonUtilities::JsonToId<BeInt64Id>(navIdJson);
            if (!navId.IsValid())
                return ERROR; //wrong format

            if (!childJsonValue.isMember(ECJsonUtilities::json_navRelClassName()))
                {
                if (ECObjectsStatus::Success != v.SetNavigationInfo(navId))
                    return ERROR;

                ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), v);
                if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
                    return ERROR;

                continue;
                }

            JsonValueCR navRelClassNameJson = childJsonValue[ECJsonUtilities::json_navRelClassName()];
            if (navRelClassNameJson.isNull())
                {
                if (ECObjectsStatus::Success != v.SetNavigationInfo(navId))
                    return ERROR;

                ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), v);
                if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
                    return ERROR;

                continue;
                }

            ECClassId relClassId = ECJsonUtilities::GetClassIdFromClassNameJson(navRelClassNameJson, classLocater);
            if (!relClassId.IsValid())
                return ERROR;

            if (ECObjectsStatus::Success != v.SetNavigationInfo(navId, relClassId))
                return ERROR;

            ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), v);
            if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 2/2013
//---------------------------------------------------------------------------------------
BentleyStatus JsonECInstanceConverter::JsonToPrimitiveECValue(ECValueR ecValue, Json::Value const& jsonValue, PrimitiveType primitiveType)
    {
    if (jsonValue.isNull())
        {
        ecValue.SetToNull();
        return SUCCESS;
        }

    Json::ValueType jsonValueType = jsonValue.type();
    switch (primitiveType)
        {
        case PRIMITIVETYPE_Point2d:
            {
            DPoint2d point2d;
            if (ECJsonUtilities::JsonToPoint2d(point2d, jsonValue))
                return ERROR;

            return ecValue.SetPoint2d(point2d);
            }
        case PRIMITIVETYPE_Point3d:
            {
            DPoint3d point3d;
            if (ECJsonUtilities::JsonToPoint3d(point3d, jsonValue))
                return ERROR;

            return ecValue.SetPoint3d(point3d);
            }
        case PRIMITIVETYPE_Integer:
            {
            if (jsonValueType != Json::intValue && jsonValueType != Json::stringValue)
                return ERROR;

            if (jsonValue.isInt())
                return ecValue.SetInteger(jsonValue.asInt());

            return ecValue.SetInteger(std::stoi(jsonValue.asCString()));
            }
        case PRIMITIVETYPE_Long:
            {
            int64_t val;
            if (SUCCESS != ECJsonUtilities::JsonToInt64(val, jsonValue))
                return ERROR;

            return ecValue.SetLong(val);
            }
         case PRIMITIVETYPE_Double:
            {
            // FIXME: This seems extremely dangerous - there's no guarantee that a string represents a valid double value...
            if (jsonValue.isString())
                return ecValue.SetDouble(std::stod(jsonValue.asCString()));

            if (jsonValue.isConvertibleTo(Json::ValueType::realValue))
                return ecValue.SetDouble(jsonValue.asDouble());

            BeAssert(false && "Invalid type to convert to double");
            return ERROR;
            }
        case PRIMITIVETYPE_DateTime:
            {
            if (jsonValueType != Json::stringValue)
                return ERROR;

            DateTime dateTime;
            if (SUCCESS != ECJsonUtilities::JsonToDateTime(dateTime, jsonValue))
                return ERROR;

            return ecValue.SetDateTime(dateTime);
            }
        case PRIMITIVETYPE_String:
            {
            if (jsonValueType != Json::stringValue)
                return ERROR;

            return ecValue.SetUtf8CP(jsonValue.asCString(), true);
            }
        case PRIMITIVETYPE_Boolean:
            {
            if (jsonValueType != Json::booleanValue)
                return ERROR;

            return ecValue.SetBoolean(jsonValue.asBool());
            }
        case PRIMITIVETYPE_Binary:
            {
            bvector<Byte> blob;
            if (SUCCESS != ECJsonUtilities::JsonToBinary(blob, jsonValue))
                return ERROR;

            return ecValue.SetBinary(blob.data(), blob.size(), true);
            }
        case PRIMITIVETYPE_IGeometry:
            {
            if (jsonValueType == Json::objectValue)
                {
                IGeometryPtr geom = ECJsonUtilities::JsonToIGeometry(jsonValue);
                if (geom == nullptr)
                    return ERROR;

                return ecValue.SetIGeometry(*geom);
                }

            if (jsonValueType == Json::stringValue)
                {
                bvector<Byte> geometryFbBlob;
                if (SUCCESS != ECJsonUtilities::JsonToBinary(geometryFbBlob, jsonValue))
                    return ERROR;

                return ecValue.SetIGeometry(geometryFbBlob.data(), geometryFbBlob.size(), true);
                }

            return ERROR;
            }
        default:
            BeAssert(false);
            return ERROR;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 2/2013
//---------------------------------------------------------------------------------------
BentleyStatus JsonECInstanceConverter::JsonToArrayECValue(IECInstanceR instance, const Json::Value& jsonValue, ArrayECPropertyCR property, Utf8StringCR accessString, IECClassLocaterR classLocater)
{
    if (!jsonValue.isArray())
        return ERROR;

    const uint32_t length = jsonValue.size();

    if (ECObjectsStatus::Success != instance.AddArrayElements(accessString.c_str(), length))
        return ERROR;

    if (property.GetIsStructArray())
        {
        StructArrayECPropertyCP structArrayProperty = property.GetAsStructArrayProperty();
        if (nullptr == structArrayProperty)
            return ERROR;

        ECClassCR structType = structArrayProperty->GetStructElementType();
        for (uint32_t ii = 0; ii < length; ii++)
            {
            IECInstancePtr structInstance = structType.GetDefaultStandaloneEnabler()->CreateInstance(0);
            JsonToECInstance(*structInstance, jsonValue[ii], structType, Utf8String(), classLocater);
            ECValue ecStructValue;
            ecStructValue.SetStruct(structInstance.get());
            ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), ecStructValue, ii);
            if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
                {
                BeAssert(false);
                }
            }

        return SUCCESS;
        }

    BeAssert(property.GetIsPrimitiveArray());
    const PrimitiveType primType = property.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();

    for (uint32_t ii = 0; ii < length; ii++)
        {
        ECValue ecPrimitiveValue;
        if (SUCCESS != JsonToPrimitiveECValue(ecPrimitiveValue, jsonValue[ii], primType))
            return ERROR;

        ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), ecPrimitiveValue, ii);
        if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
            {
            BeAssert(false);
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Shaun.Sewall                    01/2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECInstanceConverter::JsonToECInstance(IECInstanceR instance, RapidJsonValueCR jsonValue, IECClassLocaterR classLocater)
    {
    return JsonToECInstance(instance, jsonValue, instance.GetClass(), Utf8String(), classLocater);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Shaun.Sewall                    01/2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECInstanceConverter::JsonToECInstance(ECN::IECInstanceR instance, RapidJsonValueCR jsonValue, ECClassCR currentClass, Utf8StringCR currentAccessString, IECClassLocaterR classLocater)
    {
    if (!jsonValue.IsObject())
        return ERROR;

    for (rapidjson::Value::ConstMemberIterator it = jsonValue.MemberBegin(); it != jsonValue.MemberEnd(); ++it)
        {
        if (ECJsonSystemNames::IsTopLevelSystemMember(it->name.GetString()))
            continue;

        Utf8CP propertyName = it->name.GetString();
        ECPropertyP prop = currentClass.GetPropertyP(propertyName);
        if (nullptr == prop)
            {
            LOG.errorv("Failed to create ECInstance from JSON: Property '%s' not found in ECClass '%s'.", propertyName, currentClass.GetFullName());
            return ERROR;
            }

        Utf8String accessString = currentAccessString.empty() ? propertyName : currentAccessString + "." + propertyName;

        RapidJsonValueCR childJsonValue = it->value;

        if (prop->GetIsPrimitive())
            {
            ECValue ecValue;
            if (SUCCESS != JsonToPrimitiveECValue(ecValue, childJsonValue, prop->GetAsPrimitiveProperty()->GetType()))
                return ERROR;

            ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), ecValue);
            if (ECObjectsStatus::Success != ecStatus && ECObjectsStatus::PropertyValueMatchesNoChange != ecStatus)
                return ERROR;

            continue;
            }

        if (prop->GetIsStruct())
            {
            if (SUCCESS != JsonToECInstance(instance, childJsonValue, prop->GetAsStructProperty()->GetType(), accessString, classLocater))
                return ERROR;

            continue;
            }

        if (prop->GetIsArray())
            {
            if (SUCCESS != JsonToArrayECValue(instance, childJsonValue, *prop->GetAsArrayProperty(), accessString, classLocater))
                return ERROR;

            continue;
            }

        if (prop->GetIsNavigation())
            {
            NavigationECPropertyCP navProp = prop->GetAsNavigationProperty();
            if (navProp->IsMultiple())
                {
                LOG.error("NavigationECProperties with IsMultiple == true not supported by JsonECInstanceConverter");
                return ERROR;
                }


            if (!childJsonValue.IsNull() && !childJsonValue.IsObject())
                return ERROR;

            //the existence of a nav prop JSON object means we will not ignore it regardless of whether its member exist or not
            ECValue v;
            v.SetToNull();
            if (childJsonValue.IsNull())
                {
                ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), v);
                if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
                    return ERROR;

                continue;
                }

            if (!childJsonValue.HasMember(ECJsonSystemNames::Navigation::Id()))
                return ERROR; //nav prop JSON requires the id member to be present

            RapidJsonValueCR navIdJson = childJsonValue[ECJsonSystemNames::Navigation::Id()];
            if (navIdJson.IsNull())
                {
                ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), v);
                if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
                    return ERROR;

                continue;
                }

            BeInt64Id navId = ECJsonUtilities::JsonToId<BeInt64Id>(navIdJson);
            if (!navId.IsValid())
                return ERROR; //wrong format

            if (!childJsonValue.HasMember(ECJsonSystemNames::Navigation::RelClassName()))
                {
                if (ECObjectsStatus::Success != v.SetNavigationInfo(navId))
                    return ERROR;

                ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), v);
                if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
                    return ERROR;

                continue;
                }

            RapidJsonValueCR navRelClassNameJson = childJsonValue[ECJsonSystemNames::Navigation::RelClassName()];
            if (navRelClassNameJson.IsNull())
                {
                if (ECObjectsStatus::Success != v.SetNavigationInfo(navId))
                    return ERROR;

                ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), v);
                if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
                    return ERROR;

                continue;
                }

            ECClassId relClassId = ECJsonUtilities::GetClassIdFromClassNameJson(navRelClassNameJson, classLocater);
            if (!relClassId.IsValid())
                return ERROR;

            if (ECObjectsStatus::Success != v.SetNavigationInfo(navId, relClassId))
                return ERROR;

            ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), v);
            if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Shaun.Sewall                    01/2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECInstanceConverter::JsonToPrimitiveECValue(ECValueR ecValue, RapidJsonValueCR jsonValue, PrimitiveType primitiveType)
    {
    if (jsonValue.IsNull())
        {
        ecValue.SetToNull();
        return SUCCESS;
        }

    switch (primitiveType)
        {
        case PRIMITIVETYPE_Point2d:
            {
            DPoint2d point2d;
            if (SUCCESS != ECJsonUtilities::JsonToPoint2d(point2d, jsonValue))
                return ERROR;

            return ecValue.SetPoint2d(point2d);
            }
        case PRIMITIVETYPE_Point3d:
            {
            DPoint3d point3d;
            if (SUCCESS != ECJsonUtilities::JsonToPoint3d(point3d, jsonValue))
                return ERROR;

            return ecValue.SetPoint3d(point3d);
            }
        case PRIMITIVETYPE_Integer:
            {
            if (jsonValue.IsInt())
                return ecValue.SetInteger(jsonValue.GetInt());

            if (jsonValue.IsUint())
                return ecValue.SetInteger((int) jsonValue.GetUint());

            return ERROR;
            }

        case PRIMITIVETYPE_Long:
            {
            int64_t val;
            if (SUCCESS != ECJsonUtilities::JsonToInt64(val, jsonValue))
                return ERROR;

            return ecValue.SetLong(val);
            }
        case PRIMITIVETYPE_Double:
            if (!jsonValue.IsNumber())
                return ERROR;

            return ecValue.SetDouble(jsonValue.GetDouble());

        case PRIMITIVETYPE_DateTime:
            {
            if (!jsonValue.IsString())
                return ERROR;

            DateTime dt;
            if (SUCCESS != ECJsonUtilities::JsonToDateTime(dt, jsonValue))
                return ERROR;

            return ecValue.SetDateTime(dt);
            }

        case PRIMITIVETYPE_String:
            if (!jsonValue.IsString())
                return ERROR;

            return ecValue.SetUtf8CP(jsonValue.GetString(), true);

        case PRIMITIVETYPE_Boolean:
            if (!jsonValue.IsBool())
                return ERROR;

            return ecValue.SetBoolean(jsonValue.GetBool());

        case PRIMITIVETYPE_IGeometry:
            {
            if (jsonValue.IsObject())
                {
                IGeometryPtr geom = ECJsonUtilities::JsonToIGeometry(jsonValue);
                if (geom == nullptr)
                    return ERROR;

                return ecValue.SetIGeometry(*geom);
                }

            if (jsonValue.IsString())
                {
                bvector<Byte> flatBufferBlob;
                if (SUCCESS != ECJsonUtilities::JsonToBinary(flatBufferBlob, jsonValue))
                    return ERROR;

                return ecValue.SetBinary(flatBufferBlob.data(), flatBufferBlob.size(), true);
                }

            return ERROR;
            }
        case PRIMITIVETYPE_Binary:
            {
            bvector<Byte> blob;
            if (SUCCESS != ECJsonUtilities::JsonToBinary(blob, jsonValue))
                return ERROR;

            return ecValue.SetBinary(blob.data(), blob.size(), true);
            }
        default:
            return ERROR;
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Shaun.Sewall                    01/2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECInstanceConverter::JsonToArrayECValue(IECInstanceR instance, RapidJsonValueCR jsonValue, ArrayECPropertyCR property, Utf8StringCR accessString, IECClassLocaterR classLocater)
    {
    if (!jsonValue.IsArray())
        return ERROR;

    rapidjson::SizeType size = jsonValue.Size();
    if (0 == size)
        return SUCCESS;

    if (ECObjectsStatus::Success != instance.AddArrayElements(accessString.c_str(), size))
        return ERROR;

    if (property.GetIsStructArray())
        {
        ECClassCP structType = &property.GetAsStructArrayProperty()->GetStructElementType();
        BeAssert(nullptr != structType);
        for (rapidjson::SizeType i = 0; i < size; i++)
            {
            IECInstancePtr structInstance = structType->GetDefaultStandaloneEnabler()->CreateInstance(0);
            if (SUCCESS != JsonToECInstance(*structInstance, jsonValue[i], *structType, Utf8String(), classLocater))
                return ERROR;

            ECValue structValue;
            structValue.SetStruct(structInstance.get());

            ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), structValue, i);
            if ((ECObjectsStatus::Success != ecStatus) && (ECObjectsStatus::PropertyValueMatchesNoChange != ecStatus))
                return ERROR;
            }

        return SUCCESS;
        }

    BeAssert(property.GetIsPrimitiveArray());
    PrimitiveType primType = property.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();

    for (rapidjson::SizeType i = 0; i < size; i++)
        {
        ECValue primitiveValue;
        if (SUCCESS != JsonToPrimitiveECValue(primitiveValue, jsonValue[i], primType))
            return ERROR;

        ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), primitiveValue, i);
        if ((ECObjectsStatus::Success != ecStatus) && (ECObjectsStatus::PropertyValueMatchesNoChange != ecStatus))
            return ERROR;
        }

    return SUCCESS;
    }


/////////////////////////////////////////////////////////////////////////////////////////
// JsonEcInstanceWriter
/////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  02/2016
//---------------------------------------------------------------------------------------
void JsonEcInstanceWriter::AppendAccessString(Utf8StringR compoundAccessString, Utf8CP baseAccessString, Utf8StringCR propertyName)
    {
    compoundAccessString = baseAccessString;
    if (!compoundAccessString.EndsWith("."))
        compoundAccessString.append(".");
    
    compoundAccessString.append(propertyName);
    }

//---------------------------------------------------------------------------------------
// TODO: add koq process for all data types
// @bsimethod                                    Bill.Steinbock                  02/2016
//---------------------------------------------------------------------------------------
StatusInt JsonEcInstanceWriter::WritePrimitiveValue(Json::Value& valueToPopulate, Utf8CP propertyName, ECValueCR ecValue, PrimitiveType propertyType, KindOfQuantityCP koq)
    {
    ECQuantityFormattingStatus status = ECQuantityFormattingStatus::InvalidKOQ;

    // write the content according to type.
    switch (propertyType)
        {
        case PRIMITIVETYPE_Binary:
            {
            if (koq)
                BeAssert(false && "KOQ not yet support for this type");

            size_t      numBytes;
            const Byte* byteData;
            if (NULL != (byteData = ecValue.GetBinary(numBytes)))
                {
                Utf8String    byteString;
                Base64Utilities::Encode(byteString, byteData, numBytes);
                valueToPopulate[propertyName] = byteString.c_str();
                }
            return BSISUCCESS;
            }

        case PRIMITIVETYPE_IGeometry:
            {
            if (koq)
                BeAssert(false && "KOQ not yet support for this type");

            bmap<OrderedIGeometryPtr, BeExtendedData> extendedData;
            Utf8String beCgXml;
            BeXmlCGWriter::Write(beCgXml, *(ecValue.GetIGeometry()), &extendedData);
            valueToPopulate[propertyName] = beCgXml.c_str();
            return BSISUCCESS;
            }

        case PRIMITIVETYPE_Boolean:
            {
            if (koq)
                BeAssert(false && "KOQ not yet support for this type");

            valueToPopulate[propertyName] = ecValue.GetBoolean();
            return BSISUCCESS;
            }

        case PRIMITIVETYPE_DateTime:
            {
            if (koq)
                BeAssert(false && "KOQ not yet support for this type");

            valueToPopulate[propertyName] = ecValue.ToString();
            return BSISUCCESS;
            }

        case PRIMITIVETYPE_Double:
            {
            if (koq)
                {
                Utf8String formattedVal = ECQuantityFormatting::FormatPersistedValue(ecValue.GetDouble(), koq, &status);
                if (ECQuantityFormattingStatus::Success == status)
                    {
                    Json::Value quantityValue(Json::objectValue);

                    quantityValue[json_rawValue()] = ecValue.GetDouble();
                    quantityValue[json_formattedValue()] = formattedVal;
                    quantityValue[json_currentUnit()] = koq->GetDefaultPresentationFormat()->GetName();
                    valueToPopulate[propertyName] = quantityValue;
                    return BSISUCCESS;
                    }
                }

            valueToPopulate[propertyName] = ecValue.GetDouble();
            return BSISUCCESS;
            }

        case PRIMITIVETYPE_Integer:
            {
            if (koq)
                BeAssert(false && "KOQ not yet support for this type");

            valueToPopulate[propertyName] = ecValue.GetInteger();
            return BSISUCCESS;
            }

        case PRIMITIVETYPE_Long:
            {
            if (koq)
                BeAssert(false && "KOQ not yet support for this type");

            valueToPopulate[propertyName] = Json::Value(ecValue.GetLong());
            return BSISUCCESS;
            }

        case PRIMITIVETYPE_Point2d:
            {
            if (koq)
                BeAssert(false && "KOQ not yet support for this type");

            return ECJsonUtilities::Point2dToJson(valueToPopulate[propertyName], ecValue.GetPoint2d());
            }

        case PRIMITIVETYPE_Point3d:
            {
            if (koq)
                BeAssert(false && "KOQ not yet support for this type");

            return ECJsonUtilities::Point3dToJson(valueToPopulate[propertyName], ecValue.GetPoint3d());
            }

        case PRIMITIVETYPE_String:
            {
            if (koq)
                BeAssert(false && "KOQ not yet support for this type");
            
            valueToPopulate[propertyName] = ecValue.GetUtf8CP();
            return BSISUCCESS;
            }

        default:
            {
            BeAssert(false);
            return BSIERROR;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  02/2016
//---------------------------------------------------------------------------------------
StatusInt JsonEcInstanceWriter::WritePrimitivePropertyValue(Json::Value& valueToPopulate, PrimitiveECPropertyR primitiveProperty, IECInstanceCR ecInstance, Utf8CP baseAccessString, bool writeFormattedQuantities, bool serializeNullValues)
    {
    ECObjectsStatus     getStatus;
    ECValue             ecValue;
    Utf8StringCR propertyName = primitiveProperty.GetName();

    KindOfQuantityCP koq = nullptr;
    if (writeFormattedQuantities)
        koq = primitiveProperty.GetKindOfQuantity();

    if (Utf8String::IsNullOrEmpty(baseAccessString))
        {
        getStatus = ecInstance.GetValue(ecValue, propertyName.c_str());
        }
    else
        {
        Utf8String compoundAccessString;
        AppendAccessString(compoundAccessString, baseAccessString, propertyName);
        getStatus = ecInstance.GetValue(ecValue, compoundAccessString.c_str());
        }

    if (ECObjectsStatus::Success != getStatus)
        return BSISUCCESS;

    if (ecValue.IsNull())
        {
        if (serializeNullValues)
            valueToPopulate[propertyName.c_str()] = Json::nullValue;
        return BSISUCCESS;
        }

    PrimitiveType propertyType = primitiveProperty.GetType();

    StatusInt status = WritePrimitiveValue(valueToPopulate, propertyName.c_str(), ecValue, propertyType, koq);
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  02/2016
//---------------------------------------------------------------------------------------
StatusInt JsonEcInstanceWriter::WritePrimitiveValueForPresentation(Json::Value& valueToPopulate, PrimitiveECPropertyR primitiveProperty, IECInstanceCR ecInstance, Utf8String* baseAccessString)
    {
    auto const baseAccessStringP = nullptr == baseAccessString ? nullptr : baseAccessString->c_str();
    return WritePrimitivePropertyValue(valueToPopulate, primitiveProperty, ecInstance, baseAccessStringP, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  02/2016
//---------------------------------------------------------------------------------------
StatusInt JsonEcInstanceWriter::WritePrimitiveValue(Json::Value& valueToPopulate, PrimitiveECPropertyR primitiveProperty, IECInstanceCR ecInstance, Utf8String* baseAccessString)
    {
    auto const baseAccessStringP = nullptr == baseAccessString ? nullptr : baseAccessString->c_str();
    return WritePrimitivePropertyValue(valueToPopulate, primitiveProperty, ecInstance, baseAccessStringP, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  02/2016
//---------------------------------------------------------------------------------------
StatusInt JsonEcInstanceWriter::WriteArrayPropertyValue(Json::Value& valueToPopulate, ArrayECPropertyR arrayProperty, IECInstanceCR ecInstance, Utf8CP baseAccessString, ECClassLocatorByClassIdCP classLocator, bool writeFormattedQuantities, bool serializeNullValues)
    {
    ArrayKind arrayKind = arrayProperty.GetKind();

    Utf8String    accessString;
    if (Utf8String::IsNullOrEmpty(baseAccessString))
        accessString = arrayProperty.GetName();
    else
        AppendAccessString(accessString, baseAccessString, arrayProperty.GetName());

    // no members, don't write anything.
    ECValue         ecValue;
    if (ECObjectsStatus::Success != ecInstance.GetValue(ecValue, accessString.c_str()))
        return BSISUCCESS;

    if (ecValue.IsNull())
        {
        if (serializeNullValues)
            valueToPopulate[arrayProperty.GetName().c_str()] = Json::nullValue;
        return BSISUCCESS;
        }

    uint32_t nElements = ecValue.GetArrayInfo().GetCount();

    if (nElements == 0 && !serializeNullValues)
        return BSISUCCESS;

    auto& arrayObj = valueToPopulate[arrayProperty.GetName().c_str()] = Json::arrayValue;

    // Serialize an empty array
    if (nElements == 0)
        return BSISUCCESS;

    Json::Value entryObj(Json::objectValue);

    StatusInt     ixwStatus;
    if (ARRAYKIND_Primitive == arrayKind)
        {
        ECPropertyCP  primitiveProperty = arrayProperty.GetAsPrimitiveArrayProperty();
        PrimitiveType   memberType = arrayProperty.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();
        Utf8CP          typeString = SchemaParseUtils::PrimitiveTypeToString(memberType);

        KindOfQuantityCP koq = nullptr;
        if (writeFormattedQuantities)
            koq = primitiveProperty->GetKindOfQuantity();

        for (uint32_t index = 0; index < nElements; index++)
            {
            entryObj.clear();

            if (ECObjectsStatus::Success != ecInstance.GetValue(ecValue, accessString.c_str(), index))
                break;

            // write the primitive value
            if (BSISUCCESS != (ixwStatus = WritePrimitiveValue(entryObj, typeString, ecValue, memberType, koq)))
                {
                BeAssert(false);
                return ixwStatus;
                }
            arrayObj.append(entryObj[typeString]);
            }
        }
    else if (ARRAYKIND_Struct == arrayKind)
        {
        IECInstancePtr  structInstance;

        for (uint32_t index = 0; index < nElements; index++)
            {
            entryObj.clear();
            if (ECObjectsStatus::Success != ecInstance.GetValue(ecValue, accessString.c_str(), index))
                break;

            // the XML element tag is the struct type.
            BeAssert(ecValue.IsStruct());

            structInstance = ecValue.GetStruct();
            if (!structInstance.IsValid())
                {
                // ###TODO: It is valid to have null struct array instances....
                BeAssert(false);
                break;
                }

            ECClassCR   structClass = structInstance->GetClass();
            StatusInt iwxStatus;
            if (BSISUCCESS != (iwxStatus = WritePropertyValuesOfClassOrStructArrayMember(entryObj, structClass, *structInstance.get(), nullptr, classLocator, writeFormattedQuantities, serializeNullValues)))
                {
                BeAssert(false);
                return iwxStatus;
                }

            arrayObj.append(entryObj);
            }
        }
    else
        {
        // unexpected arrayKind - should never happen.
        BeAssert(false);
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  02/2016
//---------------------------------------------------------------------------------------
StatusInt JsonEcInstanceWriter::WriteEmbeddedStructPropertyValue(Json::Value& valueToPopulate, StructECPropertyR structProperty, IECInstanceCR ecInstance, Utf8CP baseAccessString, bool writeFormattedQuantities, bool serializeNullValues)
    {
    Utf8String    structName = structProperty.GetName();

    Json::Value structObj = Json::objectValue;

    Utf8String    thisAccessString;
    if (Utf8String::IsNullOrEmpty(baseAccessString))
        thisAccessString = structName.c_str();
    else
        AppendAccessString(thisAccessString, baseAccessString, structName);
    thisAccessString.append(".");

    ECClassCR   structClass = structProperty.GetType();
    WritePropertyValuesOfClassOrStructArrayMember(structObj, structClass, ecInstance, thisAccessString.c_str(), nullptr, writeFormattedQuantities, serializeNullValues);

    if (!structObj.empty() || serializeNullValues)
        valueToPopulate[structName.c_str()] = structObj;

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  05/2017
//---------------------------------------------------------------------------------------
StatusInt JsonEcInstanceWriter::WriteEmbeddedStructValue(Json::Value& valueToPopulate, StructECPropertyR structProperty, IECInstanceCR ecInstance, Utf8String* baseAccessString)
    {
    auto const baseAccessStringP = nullptr == baseAccessString ? nullptr : baseAccessString->c_str();
    return JsonEcInstanceWriter::WriteEmbeddedStructPropertyValue(valueToPopulate, structProperty, ecInstance, baseAccessStringP, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  05/2017
//---------------------------------------------------------------------------------------
StatusInt JsonEcInstanceWriter::WriteEmbeddedStructValueForPresentation(Json::Value& valueToPopulate, ECN::StructECPropertyR structProperty, ECN::IECInstanceCR ecInstance, Utf8String* baseAccessString)
    {
    auto const baseAccessStringP = nullptr == baseAccessString ? nullptr : baseAccessString->c_str();
    return JsonEcInstanceWriter::WriteEmbeddedStructPropertyValue(valueToPopulate, structProperty, ecInstance, baseAccessStringP, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                    08/2017
//---------------------------------------------------------------------------------------
StatusInt JsonEcInstanceWriter::WriteNavigationPropertyValue(Json::Value& valueToPopulate, NavigationECPropertyR navigationProperty, IECInstanceCR ecInstance, Utf8CP baseAccessString, ECClassLocatorByClassIdCP classLocator, bool writeFormattedQuantities, bool serializeNullValues)
    {
    Utf8String navName = navigationProperty.GetName();

    Utf8String thisAccessString;
    if (Utf8String::IsNullOrEmpty(baseAccessString))
        thisAccessString = navName.c_str();
    else
        AppendAccessString(thisAccessString, baseAccessString, navName);

    ECValue value;
    if (ECObjectsStatus::Success != ecInstance.GetValue(value, thisAccessString.c_str()))
        return BSIERROR;

    if (!value.IsNavigation())
        BeAssert(false);

    if (value.IsNull())
        {
        if (serializeNullValues)
            valueToPopulate[navName.c_str()] = Json::nullValue;
        return BSISUCCESS;
        }

    auto& navObj = valueToPopulate[navName.c_str()] = Json::objectValue;

    ECValue::NavigationInfo const& navInfo = value.GetNavigationInfo();

    navObj[ECJsonUtilities::json_navId()] = navInfo.GetId<BeInt64Id>().ToHexStr();

    if (ECClassModifier::Sealed != navigationProperty.GetRelationshipClass()->GetClassModifier())
        {
        ECClassCP relationshipClass = navInfo.GetRelationshipClass();
        if (nullptr == relationshipClass)
            {
            if (!navInfo.GetRelationshipClassId().IsValid())
                return BSISUCCESS;

            if (nullptr == classLocator)
                return LOG.error("ECClassLocatorByClassId was not provided to locate ECClass by ECClassId."), BSIERROR;

            auto const relationshipClassId = navInfo.GetRelationshipClassId();
            relationshipClass = classLocator->LocateClass(relationshipClassId);

            if (nullptr == relationshipClass)
                return LOG.error("Failed to locate ECClass with ECClassLocatorByClassId (maybe you forgot to pass schema in it?)."), BSIERROR;
            }
        
        ECJsonUtilities::ClassNameToJson(navObj[ECJsonUtilities::json_navRelClassName()], *relationshipClass);
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  02/2016
//---------------------------------------------------------------------------------------
StatusInt JsonEcInstanceWriter::WritePropertyValuesOfClassOrStructArrayMember(Json::Value& valueToPopulate, ECClassCR ecClass, IECInstanceCR ecInstance, Utf8CP baseAccessString, ECClassLocatorByClassIdCP classLocator, bool writeFormattedQuantities, bool serializeNullValues)
    {
    ECPropertyIterableCR    collection = ecClass.GetProperties(true);
    for (ECPropertyP ecProperty : collection)
        {
        PrimitiveECPropertyP    primitiveProperty;
        ArrayECPropertyP        arrayProperty;
        StructECPropertyP       structProperty;
        NavigationECPropertyP   navigationProperty;
        StatusInt               ixwStatus = BSIERROR;

        if (nullptr != (primitiveProperty = ecProperty->GetAsPrimitivePropertyP()))
            ixwStatus = WritePrimitivePropertyValue(valueToPopulate, *primitiveProperty, ecInstance, baseAccessString, writeFormattedQuantities, serializeNullValues);
        else if (nullptr != (arrayProperty = ecProperty->GetAsArrayPropertyP()))
            ixwStatus = WriteArrayPropertyValue(valueToPopulate, *arrayProperty, ecInstance, baseAccessString, classLocator, writeFormattedQuantities, serializeNullValues);
        else if (nullptr != (structProperty = ecProperty->GetAsStructPropertyP()))
            ixwStatus = WriteEmbeddedStructPropertyValue(valueToPopulate, *structProperty, ecInstance, baseAccessString, writeFormattedQuantities, serializeNullValues);
        else if (nullptr != (navigationProperty = ecProperty->GetAsNavigationPropertyP()))
            ixwStatus = WriteNavigationPropertyValue(valueToPopulate, *navigationProperty, ecInstance, baseAccessString, classLocator, writeFormattedQuantities, serializeNullValues);

        if (BSISUCCESS != ixwStatus)
            {
            BeAssert(false);
            return ixwStatus;
            }
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  02/2016
//---------------------------------------------------------------------------------------
StatusInt JsonEcInstanceWriter::WriteInstanceToJson(Json::Value& valueToPopulate, IECInstanceCR ecInstance, Utf8CP instanceName, bool writeInstanceId, bool serializeNullValues, ECClassLocatorByClassIdCP classLocator)
    {
    ECClassCR   ecClass = ecInstance.GetClass();
    ECSchemaCR  ecSchema = ecClass.GetSchema();
    Utf8String  className = ecClass.GetName();
    Utf8String  fullSchemaName;

    valueToPopulate[ECINSTANCE_CLASS_ATTRIBUTE] = className.c_str();
    fullSchemaName.Sprintf("%s.%02" PRIu32 ".%02" PRIu32, ecSchema.GetName().c_str(), ecSchema.GetVersionRead(), ecSchema.GetVersionMinor());
    valueToPopulate[ECINSTANCE_SCHEMA_ATTRIBUTE] = fullSchemaName.c_str();

    if (writeInstanceId)
        valueToPopulate[ECJSON_ECINSTANCE_INSTANCEID_ATTRIBUTE] = ecInstance.GetInstanceIdForSerialization().c_str();

    Json::Value instanceObj(Json::objectValue);
    StatusInt status = WritePropertyValuesOfClassOrStructArrayMember(instanceObj, ecClass, ecInstance, nullptr, classLocator, false, serializeNullValues);
    if (status != BSISUCCESS)
        return status;

    if (nullptr != instanceName)
        valueToPopulate[instanceName] = instanceObj;
    else
        valueToPopulate[className.c_str()] = instanceObj;

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
ECOBJECTS_EXPORT StatusInt JsonEcInstanceWriter::WriteInstanceToSchemaJson(Json::Value& valueToPopulate, ECN::IECInstanceCR ecInstance, ECClassLocatorByClassIdCP classLocator)
    {
    ECClassCR ecClass = ecInstance.GetClass();
    StatusInt status = WritePropertyValuesOfClassOrStructArrayMember(valueToPopulate, ecClass, ecInstance, nullptr, classLocator, true);
    if (BSISUCCESS != status)
        return status;
    valueToPopulate[ECJsonSystemNames::ClassName()] = ECJsonUtilities::FormatClassName(ecClass);
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  02/2016
//---------------------------------------------------------------------------------------
StatusInt     JsonEcInstanceWriter::WriteInstanceToPresentationJson(Json::Value& valueToPopulate, IECInstanceCR ecInstance, Utf8CP instanceName, bool writeInstanceId, ECClassLocatorByClassIdCP classLocator)
    {
    ECClassCR   ecClass = ecInstance.GetClass();
    ECSchemaCR  ecSchema = ecClass.GetSchema();
    Utf8String  className = ecClass.GetName();
    Utf8String  fullSchemaName;

    valueToPopulate[ECINSTANCE_CLASS_ATTRIBUTE] = className.c_str();
    fullSchemaName.Sprintf("%s.%02" PRIu32 ".%02" PRIu32, ecSchema.GetName().c_str(), ecSchema.GetVersionRead(), ecSchema.GetVersionMinor());
    valueToPopulate[ECINSTANCE_SCHEMA_ATTRIBUTE] = fullSchemaName.c_str();

    if (writeInstanceId)
        valueToPopulate[ECJSON_ECINSTANCE_INSTANCEID_ATTRIBUTE] = ecInstance.GetInstanceIdForSerialization().c_str();

    Json::Value instanceObj(Json::objectValue);
    StatusInt status = WritePropertyValuesOfClassOrStructArrayMember(instanceObj, ecClass, ecInstance, nullptr, classLocator, true);
    if (status != BSISUCCESS)
        return status;

    if (nullptr != instanceName)
        valueToPopulate[instanceName] = instanceObj;
    else
        valueToPopulate[className.c_str()] = instanceObj;

    return BSISUCCESS;
    }


END_BENTLEY_ECOBJECT_NAMESPACE
