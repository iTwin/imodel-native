/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ECObjects/ECQuantityFormatting.h>
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
// An alternative implementation of BeGuid that only implements FromString. This was copied from BeSQLite.h and BeSQLite.cpp.
// iModel.js stores GUIDs as strings, but the ECSchema standard is to declare them as binary with an extendedTypeName of BeGuid.
// The problem was that ECObjects does not depend on BeSQLite so the real BeGuid was not available.
//---------------------------------------------------------------------------------------
struct BeGuidAlt
{
    union{uint64_t u[2]; uint32_t i[4]; uint8_t b[16];} m_guid;
    BeGuidAlt() {Invalidate();}
    void Invalidate() {m_guid.u[0] = m_guid.u[1] = 0;}
    bool IsValid() const {return 0!=m_guid.u[0] && 0!=m_guid.u[1];}
    BentleyStatus FromString(Utf8CP uuid_str)
        {
        if (uuid_str == nullptr)
            return ERROR;

        for (int i = 0; i < 36; ++i)
            {
            char c = uuid_str[i];
            if (c == 0 || !isxdigit(c) && !(c == '-' && (i == 8 || i == 13 || i == 18 || i == 23)))
                return ERROR; /* ### need a better value */
            }

        if (uuid_str[36] != '\0')
            return ERROR; /* ### need a better value */

        m_guid.b[0] = parse_hexpair(&uuid_str[0]);
        m_guid.b[1] = parse_hexpair(&uuid_str[2]);
        m_guid.b[2] = parse_hexpair(&uuid_str[4]);
        m_guid.b[3] = parse_hexpair(&uuid_str[6]);
        m_guid.b[4] = parse_hexpair(&uuid_str[9]);
        m_guid.b[5] = parse_hexpair(&uuid_str[11]);
        m_guid.b[6] = parse_hexpair(&uuid_str[14]);
        m_guid.b[7] = parse_hexpair(&uuid_str[16]);
        m_guid.b[8] = parse_hexpair(&uuid_str[19]);
        m_guid.b[9] = parse_hexpair(&uuid_str[21]);

        for (int i = 6; i--;)
            m_guid.b[10 + i] = parse_hexpair(&uuid_str[i * 2 + 24]);

        return SUCCESS;
        }
    static unsigned char parse_hexpair(Utf8CP s)
        {
        int result = s[0] - '0';
        if (result > 48)
            result = (result - 39) << 4;
        else if (result > 16)
            result = (result - 7) << 4;
        else
            result = result << 4;

        int temp = s[1] - '0';
        if (temp > 48)
            result |= temp - 39;
        else if (temp > 16)
            result |= temp - 7;
        else
            result |= temp;

        return (unsigned char)result;
        }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECClassCP ECJsonUtilities::GetClassFromClassNameJson(BeJsConst json, IECClassLocaterR classLocater)
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
// @bsimethod
//---------------------------------------------------------------------------------------
ECClassId ECJsonUtilities::GetClassIdFromClassNameJson(BeJsConst json, IECClassLocaterR classLocater)
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
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECJsonUtilities::IdToJson(BeJsValue json, BeInt64Id id)
    {
    if (!id.IsValid())
        return ERROR;

    json = id;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ECJsonUtilities::Int64ToJson(BeJsValue json, int64_t int64Val, ECJsonInt64Format int64Format)
    {
    switch (int64Format)
        {
        case ECJsonInt64Format::AsNumber:
            if (int64Val < Json::Value::maxInt())
                json = (Json::Int) int64Val;
            else
                json = (double) int64Val;
            return;

        case ECJsonInt64Format::AsDecimalString: {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%" PRId64, int64Val);
            json = buffer;
            return;
        }

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
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECJsonUtilities::JsonToInt64(int64_t& int64Val, BeJsConst json)
    {
    if (json.isNull())
        return ERROR;

    int64Val = json.asInt64();
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECJsonUtilities::JsonToDateTime(DateTime& dateTime, BeJsConst json)
    {
    return DateTime::FromString(dateTime, json.ToJsonString().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECJsonUtilities::Point2dToJson(BeJsValue json, DPoint2d pt)
    {
    json[json_x()] = pt.x;
    json[json_y()] = pt.y;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECJsonUtilities::JsonToPoint2d(DPoint2d& pt, BeJsConst json) {
    if (json.isNull())
        return ERROR;

    double x = 0.0;
    double y = 0.0;

    if (json.isObject()) {
        if (SUCCESS != PointCoordinateFromJson(x, json, json_x()) ||
            SUCCESS != PointCoordinateFromJson(y, json, json_y()))
            return ERROR;
    } else if (json.isArray()) {
        // Must be equal to the length of a valid DPoint2d array.
        if (2 != json.size())
            return ERROR;
        x = json[0u].asDouble();
        y = json[1u].asDouble();
    } else {
        return ERROR;
    }

    pt = DPoint2d::From(x, y);
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECJsonUtilities::Point3dToJson(BeJsValue json, DPoint3d pt)
    {
    json[json_x()] = pt.x;
    json[json_y()] = pt.y;
    json[json_z()] = pt.z;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECJsonUtilities::JsonToPoint3d(DPoint3d& pt, BeJsConst json) {
    if (json.isNull())
        return ERROR;

    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    if (json.isObject()) {
        if (SUCCESS != PointCoordinateFromJson(x, json, json_x()) ||
            SUCCESS != PointCoordinateFromJson(y, json, json_y()) ||
            SUCCESS != PointCoordinateFromJson(z, json, json_z()))
            return ERROR;
    } else if (json.isArray()) {
        // Must be equal to the length of a valid DPoint3d array.
        if (3 != json.size())
            return ERROR;
        x = json[0u].asDouble();
        y = json[1u].asDouble();
        z = json[2u].asDouble();
    } else {
        return ERROR;
    }

    pt = DPoint3d::From(x, y, z);
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECJsonUtilities::PointCoordinateFromJson(double& coordinate, BeJsConst json, Json::StaticString const& coordinateKey) {
    if (!json.isObject())
        return ERROR;

    auto coordinateJson = json[coordinateKey];
    if (coordinateJson.isNull() || !coordinateJson.isNumeric())
        return ERROR;

    coordinate = coordinateJson.asDouble();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECJsonUtilities::IGeometryToJson(JsonValueR json, IGeometryCR geom)
    {
    return BentleyGeometryJson::TryGeometryToJsonValue(json, geom, false) ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECJsonUtilities::IGeometryToIModelJson(BeJsValue json, IGeometryCR geom)
    {
    return IModelJson::TryGeometryToIModelJsonValue(json, geom) ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IGeometryPtr ECJsonUtilities::JsonToIGeometry(JsonValueCR json)
    {
    bvector<IGeometryPtr> geometry;
    if (!BentleyGeometryJson::TryJsonValueToGeometry(json, geometry) || geometry.empty())
        return nullptr;

    return geometry[0];
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECJsonUtilities::IdToJson(RapidJsonValueR json, BeInt64Id id, rapidjson::MemoryPoolAllocator<>& allocator)
    {
    if (!id.IsValid())
        return ERROR;

    Utf8String hexStr = id.ToHexStr();
    json.SetString(hexStr.c_str(), (rapidjson::SizeType) hexStr.size(), allocator);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ECJsonUtilities::ClassToJson(RapidJsonValueR json, ECClassCR ecClass, rapidjson::MemoryPoolAllocator<>& allocator)
    {
    Utf8String fullName = ECJsonUtilities::FormatClassName(ecClass);
    json.SetString(fullName.c_str(), (rapidjson::SizeType) fullName.size(), allocator);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
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
// @bsimethod
//---------------------------------------------------------------------------------------
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
// @bsimethod
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
            const int len = snprintf(str, sizeof(str), "%" PRIi64, val);
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
* @bsimethod
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

        Utf8String::Sscanf_safe(strVal, "%" SCNi64, &val);
        return SUCCESS;
        }

    return ERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::JsonToDateTime(DateTime& dateTime, RapidJsonValueCR json)
    {
    if (!json.IsString())
        return ERROR;

    return DateTime::FromString(dateTime, json.GetString());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::BinaryToJson(RapidJsonValueR json, Byte const* binary, size_t binarySize, rapidjson::MemoryPoolAllocator<>& allocator, Utf8CP header)
    {
    if (binarySize == 0)
        {
        json.SetNull();
        return SUCCESS;
        }

    Utf8String str;
    Base64Utilities::Encode(str, binary, binarySize, header);

    json.SetString(str.c_str(), (rapidjson::SizeType) str.size(), allocator);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
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
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::IGeometryToIModelJson(RapidJsonValueR json, IGeometryCR geom, rapidjson::MemoryPoolAllocator<>& allocator)
    {
    Utf8String jsonStr;
    if (!IModelJson::TryGeometryToIModelJsonString(jsonStr, geom) || jsonStr.empty())
        return ERROR;

    rapidjson::Document jsonDoc;
    if (jsonDoc.Parse<0>(jsonStr.c_str()).HasParseError())
        return ERROR;

    json.CopyFrom(jsonDoc, allocator);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus JsonECInstanceConverter::JsonToECInstance(IECInstanceR instance, BeJsConst jsonValue, IECClassLocaterR classLocater, bool ignoreUnknownProperties, IECSchemaRemapperCP remapper)
    {
    return JsonToECInstance(instance, jsonValue, instance.GetClass(), Utf8String(), classLocater, ignoreUnknownProperties, remapper);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus JsonECInstanceConverter::JsonToECInstance(ECN::IECInstanceR instance, BeJsConst jsonValue, IECClassLocaterR classLocater, std::function<bool(Utf8CP)> shouldSerializeProperty)
    {
    return JsonToECInstance(instance, jsonValue, instance.GetClass(), Utf8String(), classLocater, false, nullptr, shouldSerializeProperty);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus JsonECInstanceConverter::JsonToECInstance(IECInstanceR instance, BeJsConst jsonValue, ECClassCR currentClass, Utf8StringCR currentAccessString,
    IECClassLocaterR classLocater, bool ignoreUnknownProperties, IECSchemaRemapperCP remapper, std::function<bool(Utf8CP)> shouldSerializeProperty)
    {
    if (!jsonValue.isObject())
        return ERROR;

    bool checkShouldSerializeProperty = shouldSerializeProperty != nullptr;
    BentleyStatus stat = SUCCESS;
    jsonValue.ForEachProperty([&](Utf8CP memberName, BeJsConst childJsonValue) {

        auto convertOne = [&]() {
        if (ECJsonSystemNames::IsTopLevelSystemMember(memberName))
            return false;
        if (checkShouldSerializeProperty && !shouldSerializeProperty(memberName))
            return false;

        Utf8String remappedMemberName(memberName);
        if (nullptr != remapper)
            remapper->ResolvePropertyName(remappedMemberName, currentClass);
        ECPropertyP ecProperty = currentClass.GetPropertyP(remappedMemberName.c_str());
        if (ecProperty == nullptr)
            {
            if (ignoreUnknownProperties)
                return false;
            return true;
            }

        Utf8String accessString = currentAccessString.empty() ? remappedMemberName : currentAccessString + "." + remappedMemberName;
        if (ecProperty->GetIsPrimitive())
            {
            ECValue ecValue;
            if (SUCCESS != JsonToPrimitiveECValue(ecValue, childJsonValue, ecProperty->GetAsPrimitiveProperty()->GetType(), ecProperty->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str()))
                return true;

            ECObjectsStatus ecStatus;
            ecStatus = instance.SetInternalValue(accessString.c_str(), ecValue);
            UNUSED_VARIABLE(ecStatus);
            BeAssert(ecStatus == ECObjectsStatus::Success || ecStatus == ECObjectsStatus::PropertyValueMatchesNoChange);
            return false;
            }
        else if (ecProperty->GetIsStruct())
            {
            if (childJsonValue.isNull())
                return false;

            if (SUCCESS != JsonToECInstance(instance, childJsonValue, ecProperty->GetAsStructProperty()->GetType(), accessString, classLocater, ignoreUnknownProperties, remapper))
                return true;

            return false;
            }
        else if (ecProperty->GetIsArray())
            {
            if (childJsonValue.isNull())
                return false;

            if (SUCCESS != JsonToArrayECValue(instance, childJsonValue, *ecProperty->GetAsArrayProperty(), accessString, classLocater))
                return true;
            }
        else if (ecProperty->GetIsNavigation())
            {
            NavigationECPropertyCP navProp = ecProperty->GetAsNavigationProperty();
            if (navProp->IsMultiple())
                {
                LOG.error("NavigationECProperties with IsMultiple == true not supported by ECJsonUtilities");
                return true;
                }

            if (!childJsonValue.isObject() && !childJsonValue.isNull())
                return true;

            //the existence of a nav prop JSON object means we will not ignore it regardless of whether its member exist or not
            ECValue v;
            v.SetToNull();
            if (childJsonValue.isNull())
                {
                ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), v);
                if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
                    return true;

                return false;
                }

            if (!childJsonValue.isMember(ECJsonUtilities::json_navId()))
                return true; //nav prop JSON requires the id member to be present

            auto navIdJson = childJsonValue[ECJsonUtilities::json_navId()];
            if (navIdJson.isNull())
                {
                ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), v);
                if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
                    return true;

                return false;
                }

            BeInt64Id navId = navIdJson.GetId64<BeInt64Id>();
            if (!navId.IsValid()) // invalid Id means value should be cleared
                {
                if (ECObjectsStatus::Success != v.SetNavigationInfo())
                    return true;

                ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), v);
                if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
                    return true;

                return false;
                }

            if (!childJsonValue.isMember(ECJsonUtilities::json_navRelClassName()))
                {
                if (ECObjectsStatus::Success != v.SetNavigationInfo(navId))
                    return true;

                ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), v);
                if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
                    return true;

                return false;
                }

            auto navRelClassNameJson = childJsonValue[ECJsonUtilities::json_navRelClassName()];
            if (navRelClassNameJson.isNull())
                {
                if (ECObjectsStatus::Success != v.SetNavigationInfo(navId))
                    return true;

                ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), v);
                if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
                    return true;

                return false;
                }

            ECClassId relClassId = ECJsonUtilities::GetClassIdFromClassNameJson(navRelClassNameJson, classLocater);
            if (!relClassId.IsValid())
                return true;

            if (ECObjectsStatus::Success != v.SetNavigationInfo(navId, relClassId))
                return true;

            ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), v);
            if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
                return true;
            }
            return false;
        };

        if (convertOne())
            stat = ERROR; // set this to error if any conversion fails

        return false; // but don't stop on failure
        });

    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus JsonECInstanceConverter::JsonToPrimitiveECValue(ECValueR ecValue, BeJsConst jsonValue, PrimitiveType primitiveType, Utf8CP extendedTypeName)
    {
    if (jsonValue.isNull())
        {
        ecValue.SetToNull();
        return SUCCESS;
        }

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
            if (!jsonValue.isNull())
                return ecValue.SetInteger(jsonValue.asInt());

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
            {
            if (jsonValue.isNumeric())
                return ecValue.SetDouble(jsonValue.asDouble());

            BeAssert(false && "Invalid type to convert to double");
            return ERROR;
            }
        case PRIMITIVETYPE_DateTime:
            {
            DateTime dateTime;
            if (SUCCESS != ECJsonUtilities::JsonToDateTime(dateTime, jsonValue))
                return ERROR;

            return ecValue.SetDateTime(dateTime);
            }
        case PRIMITIVETYPE_String:
            {
            if (!jsonValue.isString())
                return ERROR;

            return ecValue.SetUtf8CP(jsonValue.asCString(), true);
            }
        case PRIMITIVETYPE_Boolean:
            {
            if (!jsonValue.isBool())
                return ERROR;

            return ecValue.SetBoolean(jsonValue.asBool());
            }
        case PRIMITIVETYPE_Binary:
            {
            if (extendedTypeName && (0 == BeStringUtilities::Stricmp(extendedTypeName, "BeGuid")))
                {
                BeGuidAlt guid;
                if (BentleyStatus::SUCCESS == guid.FromString(jsonValue.asCString()))
                    ecValue.SetBinary((Byte*)&guid, sizeof(guid), true);
                else
                    ecValue.SetIsNull(true);
                return SUCCESS;
                }

            bvector<Byte> blob;
            if (SUCCESS != jsonValue.GetBinary(blob))
                return ERROR;
            return ecValue.SetBinary(blob.data(), blob.size(), true);
            }
        case PRIMITIVETYPE_IGeometry:
            {
            if (jsonValue.isObject())
                {
                Json::Value tmp;
                jsonValue.SaveTo(tmp);
                IGeometryPtr geom = ECJsonUtilities::JsonToIGeometry(tmp);
                if (geom == nullptr)
                    return ERROR;

                return ecValue.SetIGeometry(*geom);
                }

            if (jsonValue.isString())
                {
                bvector<Byte> geometryFbBlob;
                // try reading it as a base64 encoded binary blob
                BentleyStatus status = jsonValue.GetBinary(geometryFbBlob);
                if (status == SUCCESS)
                    return ecValue.SetIGeometry(geometryFbBlob.data(), geometryFbBlob.size(), true);
                Utf8String strVal = jsonValue.ToUtf8CP();
                constexpr auto maxCharsShown = 100;
                LOG.errorv(
                    "Failed to read geometry from json string '%s%s'.",
                    strVal.substr(0, maxCharsShown).c_str(),
                    strVal.size() > maxCharsShown ? "... (only first 100 characters shown)" : ""
                );
                return status;
                }

            return ERROR;
            }
        default:
            BeAssert(false);
            return ERROR;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus JsonECInstanceConverter::JsonToArrayECValue(IECInstanceR instance, BeJsConst jsonValue, ArrayECPropertyCR property, Utf8StringCR accessString, IECClassLocaterR classLocater)
{
    if (!jsonValue.isArray())
        return ERROR;

    const uint32_t length = jsonValue.size();

    ECValue arrayValue;
    instance.GetValue(arrayValue, accessString.c_str());
    uint32_t currentLength = arrayValue.IsNull()? 0: arrayValue.GetArrayInfo().GetCount();
    if (length < currentLength)
        {
        // We need to shorten the array. Start by emptying it out.
        if (ECObjectsStatus::Success != instance.ClearArray(accessString.c_str()))
            return ERROR;
        currentLength = 0;
        // Now make the array the size we need
        }
    if (length > currentLength)
        {
        uint32_t xlength = length - currentLength;
        if (ECObjectsStatus::Success != instance.AddArrayElements(accessString.c_str(), xlength))
            return ERROR;
        }

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
        if (SUCCESS != JsonToPrimitiveECValue(ecPrimitiveValue, jsonValue[ii], primType, nullptr))
            return ERROR;

        ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), ecPrimitiveValue, ii);
        if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
            {
            BeAssert(false);
            }
        }

    return SUCCESS;
    }


/////////////////////////////////////////////////////////////////////////////////////////
// JsonEcInstanceWriter
/////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void JsonEcInstanceWriter::AppendAccessString(Utf8StringR compoundAccessString, Utf8CP baseAccessString, Utf8StringCR propertyName)
    {
    compoundAccessString = baseAccessString;
    if (!compoundAccessString.EndsWith("."))
        compoundAccessString.append(".");

    compoundAccessString.append(propertyName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String JsonEcInstanceWriter::FormatMemberName(Utf8StringCR propertyName, MemberNameCasing casing)
    {
    if (MemberNameCasing::KeepOriginal == casing)
        return propertyName;

    Utf8String memberName(propertyName);
    ECJsonUtilities::LowerFirstChar(memberName);
    return memberName;
    }

//---------------------------------------------------------------------------------------
// TODO: add koq process for all data types
// @bsimethod
//---------------------------------------------------------------------------------------
StatusInt JsonEcInstanceWriter::WritePrimitiveValue(BeJsValue valueToPopulate, Utf8CP propertyName, ECValueCR ecValue, PrimitiveType propertyType, KindOfQuantityCP koq, MemberNameCasing casing)
    {
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
                valueToPopulate[FormatMemberName(propertyName, casing)].SetBinary(byteData, numBytes);

            return BSISUCCESS;
            }

        case PRIMITIVETYPE_IGeometry:
            {
            if (koq)
                BeAssert(false && "KOQ not yet support for this type");

            Json::Value tmp;
            auto status =  ECJsonUtilities::IGeometryToIModelJson(tmp, *ecValue.GetIGeometry());
            if (status != SUCCESS)
                return status;
            valueToPopulate[FormatMemberName(propertyName, casing)].From(tmp);
            return BSISUCCESS;
            }

        case PRIMITIVETYPE_Boolean:
            {
            if (koq)
                BeAssert(false && "KOQ not yet support for this type");

            valueToPopulate[FormatMemberName(propertyName, casing)] = ecValue.GetBoolean();
            return BSISUCCESS;
            }

        case PRIMITIVETYPE_DateTime:
            {
            if (koq)
                BeAssert(false && "KOQ not yet support for this type");

            valueToPopulate[FormatMemberName(propertyName, casing)] = ecValue.ToString();
            return BSISUCCESS;
            }

        case PRIMITIVETYPE_Double:
            {
            if (koq)
                {
                ECQuantityFormattingStatus status = ECQuantityFormattingStatus::InvalidKOQ;

                Utf8String formattedVal = ECQuantityFormatting::FormatPersistedValue(ecValue.GetDouble(), koq, &status);
                if (ECQuantityFormattingStatus::Success == status)
                    {
                    auto quantityValue = valueToPopulate[FormatMemberName(propertyName, casing)];
                    quantityValue[json_rawValue()] = ecValue.GetDouble();
                    quantityValue[json_formattedValue()] = formattedVal;
                    quantityValue[json_currentUnit()] = koq->GetDefaultPresentationFormat()->GetName();
                    return BSISUCCESS;
                    }
                }

            valueToPopulate[FormatMemberName(propertyName, casing)] = ecValue.GetDouble();
            return BSISUCCESS;
            }

        case PRIMITIVETYPE_Integer:
            {
            if (koq)
                BeAssert(false && "KOQ not yet support for this type");

            valueToPopulate[FormatMemberName(propertyName, casing)] = ecValue.GetInteger();
            return BSISUCCESS;
            }

        case PRIMITIVETYPE_Long:
            {
            if (koq)
                BeAssert(false && "KOQ not yet support for this type");

            valueToPopulate[FormatMemberName(propertyName, casing)] = (double) ecValue.GetLong();
            return BSISUCCESS;
            }

        case PRIMITIVETYPE_Point2d:
            {
            if (koq)
                BeAssert(false && "KOQ not yet support for this type");

            return ECJsonUtilities::Point2dToJson(valueToPopulate[FormatMemberName(propertyName, casing)], ecValue.GetPoint2d());
            }

        case PRIMITIVETYPE_Point3d:
            {
            if (koq)
                BeAssert(false && "KOQ not yet support for this type");

            return ECJsonUtilities::Point3dToJson(valueToPopulate[FormatMemberName(propertyName, casing)], ecValue.GetPoint3d());
            }

        case PRIMITIVETYPE_String:
            {
            if (koq)
                BeAssert(false && "KOQ not yet support for this type");

            valueToPopulate[FormatMemberName(propertyName, casing)] = ecValue.GetUtf8CP();
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
// @bsimethod
//---------------------------------------------------------------------------------------
StatusInt JsonEcInstanceWriter::WritePrimitivePropertyValue(BeJsValue valueToPopulate, PrimitiveECPropertyCR primitiveProperty, IECInstanceCR ecInstance, Utf8CP baseAccessString, bool writeFormattedQuantities, bool serializeNullValues, MemberNameCasing casing)
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
            valueToPopulate[FormatMemberName(propertyName, casing)].SetNull();
        return BSISUCCESS;
        }

    PrimitiveType propertyType = primitiveProperty.GetType();

    StatusInt status = WritePrimitiveValue(valueToPopulate, propertyName.c_str(), ecValue, propertyType, koq, casing);
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
StatusInt JsonEcInstanceWriter::WritePrimitiveValueForPresentation(BeJsValue valueToPopulate, PrimitiveECPropertyCR primitiveProperty, IECInstanceCR ecInstance, Utf8CP baseAccessString)
    {
    return WritePrimitivePropertyValue(valueToPopulate, primitiveProperty, ecInstance, baseAccessString, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
StatusInt JsonEcInstanceWriter::WritePrimitiveValue(BeJsValue valueToPopulate, PrimitiveECPropertyCR primitiveProperty, IECInstanceCR ecInstance, Utf8CP baseAccessString)
    {
    return WritePrimitivePropertyValue(valueToPopulate, primitiveProperty, ecInstance, baseAccessString, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
StatusInt JsonEcInstanceWriter::WriteArrayPropertyValue(BeJsValue valueToPopulate, ArrayECPropertyCR arrayProperty, IECInstanceCR ecInstance, Utf8CP baseAccessString, IECClassLocaterP classLocator, bool writeFormattedQuantities, bool serializeNullValues, MemberNameCasing casing)
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
            valueToPopulate[FormatMemberName(arrayProperty.GetName(), casing)].SetNull();
        return BSISUCCESS;
        }

    uint32_t nElements = ecValue.GetArrayInfo().GetCount();

    if (nElements == 0 && !serializeNullValues)
        return BSISUCCESS;

    auto arrayObj = valueToPopulate[FormatMemberName(arrayProperty.GetName(), casing)];
    arrayObj.toArray();

    // Serialize an empty array
    if (nElements == 0)
        return BSISUCCESS;

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
            if (ECObjectsStatus::Success != ecInstance.GetValue(ecValue, accessString.c_str(), index))
                break;

            // write the primitive value
            Json::Value val; // tricky - we have to use Json::Value because WritePrimitive creates an object but we only want one member
            if (BSISUCCESS != (ixwStatus = WritePrimitiveValue(BeJsValue(val), typeString, ecValue, memberType, koq, casing)))
                {
                BeAssert(false);
                return ixwStatus;
                }
            arrayObj.appendValue().From(val[typeString]); // must call From here!
            }
        }
    else if (ARRAYKIND_Struct == arrayKind)
        {
        IECInstancePtr  structInstance;

        for (uint32_t index = 0; index < nElements; index++)
            {
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
            if (BSISUCCESS != (iwxStatus = WritePropertyValuesOfClassOrStructArrayMember(arrayObj.appendValue(), structClass, *structInstance.get(), nullptr, classLocator, writeFormattedQuantities, serializeNullValues, casing)))
                {
                BeAssert(false);
                return iwxStatus;
                }
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
// @bsimethod
//---------------------------------------------------------------------------------------
StatusInt JsonEcInstanceWriter::WriteEmbeddedStructPropertyValue(BeJsValue valueToPopulate, StructECPropertyCR structProperty, IECInstanceCR ecInstance, Utf8CP baseAccessString, bool writeFormattedQuantities, bool serializeNullValues, MemberNameCasing casing)
    {
    Utf8String    structName = structProperty.GetName();

    Utf8String    thisAccessString;
    if (Utf8String::IsNullOrEmpty(baseAccessString))
        thisAccessString = structName.c_str();
    else
        AppendAccessString(thisAccessString, baseAccessString, structName);
    thisAccessString.append(".");

    ECClassCR   structClass = structProperty.GetType();
    auto name = FormatMemberName(structName, casing);
    auto structObj = valueToPopulate[name];
    WritePropertyValuesOfClassOrStructArrayMember(structObj, structClass, ecInstance, thisAccessString.c_str(), nullptr, writeFormattedQuantities, serializeNullValues, casing);
    if (structObj.empty() && !serializeNullValues)
        valueToPopulate.removeMember(name);

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
StatusInt JsonEcInstanceWriter::WriteEmbeddedStructValue(BeJsValue valueToPopulate, StructECPropertyCR structProperty, IECInstanceCR ecInstance, Utf8CP baseAccessString)
    {
    return JsonEcInstanceWriter::WriteEmbeddedStructPropertyValue(valueToPopulate, structProperty, ecInstance, baseAccessString, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
StatusInt JsonEcInstanceWriter::WriteEmbeddedStructValueForPresentation(BeJsValue valueToPopulate, ECN::StructECPropertyCR structProperty, ECN::IECInstanceCR ecInstance, Utf8CP baseAccessString)
    {
    return JsonEcInstanceWriter::WriteEmbeddedStructPropertyValue(valueToPopulate, structProperty, ecInstance, baseAccessString, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
StatusInt JsonEcInstanceWriter::WriteNavigationPropertyValue(BeJsValue valueToPopulate, NavigationECPropertyCR navigationProperty, IECInstanceCR ecInstance, Utf8CP baseAccessString, IECClassLocaterP classLocator, bool writeFormattedQuantities, bool serializeNullValues, MemberNameCasing casing)
    {
    Utf8String navName = navigationProperty.GetName();

    Utf8String thisAccessString = navName;
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
            valueToPopulate[FormatMemberName(navName, casing)].SetNull();
        return BSISUCCESS;
        }

    auto navObj = valueToPopulate[FormatMemberName(navName, casing)];

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
                {
                LOG.errorv("Failed to locate the ECRelationshipClass with Id %" PRIu64 ".", navInfo.GetRelationshipClassId().GetValueUnchecked());
                return BSIERROR;
                }

            ECClassId test = navInfo.GetRelationshipClassId();
            relationshipClass = classLocator->LocateClass(test);

            if (nullptr == relationshipClass)
                {
                LOG.errorv("Failed to locate the ECRelationshipClass with Id %" PRIu64 ".", navInfo.GetRelationshipClassId().GetValueUnchecked());
                return BSIERROR;
                }
            }

        ECJsonUtilities::ClassNameToJson(navObj[ECJsonUtilities::json_navRelClassName()], *relationshipClass);
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
StatusInt JsonEcInstanceWriter::WritePropertyValuesOfClassOrStructArrayMember(BeJsValue valueToPopulate, ECClassCR ecClass, IECInstanceCR ecInstance, Utf8CP baseAccessString, IECClassLocaterP classLocator, bool writeFormattedQuantities, bool serializeNullValues, MemberNameCasing casing, std::function<bool(Utf8CP)> shouldWriteProperty)
    {
    valueToPopulate.toObject();
    ECPropertyIterableCR    collection = ecClass.GetProperties(true);
    for (ECPropertyP ecProperty : collection)
        {
        PrimitiveECPropertyP    primitiveProperty;
        ArrayECPropertyP        arrayProperty;
        StructECPropertyP       structProperty;
        NavigationECPropertyP   navigationProperty;
        StatusInt               ixwStatus = BSIERROR;

        if ((nullptr != shouldWriteProperty) && !shouldWriteProperty(ecProperty->GetName().c_str()))
            continue;

        if (nullptr != (primitiveProperty = ecProperty->GetAsPrimitivePropertyP()))
            ixwStatus = WritePrimitivePropertyValue(valueToPopulate, *primitiveProperty, ecInstance, baseAccessString, writeFormattedQuantities, serializeNullValues, casing);
        else if (nullptr != (arrayProperty = ecProperty->GetAsArrayPropertyP()))
            ixwStatus = WriteArrayPropertyValue(valueToPopulate, *arrayProperty, ecInstance, baseAccessString, classLocator, writeFormattedQuantities, serializeNullValues, casing);
        else if (nullptr != (structProperty = ecProperty->GetAsStructPropertyP()))
            ixwStatus = WriteEmbeddedStructPropertyValue(valueToPopulate, *structProperty, ecInstance, baseAccessString, writeFormattedQuantities, serializeNullValues, casing);
        else if (nullptr != (navigationProperty = ecProperty->GetAsNavigationPropertyP()))
            ixwStatus = WriteNavigationPropertyValue(valueToPopulate, *navigationProperty, ecInstance, baseAccessString, classLocator, writeFormattedQuantities, serializeNullValues, casing);

        if (BSISUCCESS != ixwStatus)
            {
            BeAssert(false);
            return ixwStatus;
            }
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
StatusInt JsonEcInstanceWriter::WriteInstanceToJson(BeJsValue valueToPopulate, IECInstanceCR ecInstance, Utf8CP instanceName, bool writeInstanceId, bool serializeNullValues, IECClassLocaterP classLocator)
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

    auto name = (nullptr != instanceName) ? instanceName : className.c_str();
    auto instanceObj = valueToPopulate[name];
    auto status = WritePropertyValuesOfClassOrStructArrayMember(instanceObj, ecClass, ecInstance, nullptr, classLocator, false, serializeNullValues);
    if (status != BSISUCCESS)
        valueToPopulate.removeMember(name);

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
StatusInt JsonEcInstanceWriter::WritePartialInstanceToJson(BeJsValue jsonValue, IECInstanceCR ecInstance, MemberNameCasing casing, std::function<bool(Utf8CP)> shouldWriteProperty, IECClassLocaterP classLocator)
    {
    return WritePropertyValuesOfClassOrStructArrayMember(jsonValue, ecInstance.GetClass(), ecInstance, nullptr, classLocator, false, false, casing, shouldWriteProperty);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
StatusInt JsonEcInstanceWriter::WriteInstanceToSchemaJson(BeJsValue valueToPopulate, ECN::IECInstanceCR ecInstance, IECClassLocaterP classLocator)
    {
    ECClassCR ecClass = ecInstance.GetClass();
    StatusInt status = WritePropertyValuesOfClassOrStructArrayMember(valueToPopulate, ecClass, ecInstance, nullptr, classLocator, true);
    if (BSISUCCESS != status)
        return status;
    valueToPopulate[ECJsonSystemNames::ClassName()] = ECJsonUtilities::FormatClassName(ecClass);
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
StatusInt JsonEcInstanceWriter::WriteInstanceToPresentationJson(BeJsValue valueToPopulate, IECInstanceCR ecInstance, Utf8CP instanceName, bool writeInstanceId, IECClassLocaterP classLocator)
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

    auto name = (nullptr != instanceName) ? instanceName : className.c_str();
    auto instanceObj = valueToPopulate[name];
    auto status =  WritePropertyValuesOfClassOrStructArrayMember(instanceObj, ecClass, ecInstance, nullptr, classLocator, true);
    if (status != BSISUCCESS)
        valueToPopulate.removeMember(name);
    return status;

    }

END_BENTLEY_ECOBJECT_NAMESPACE
