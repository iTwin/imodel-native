/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECJsonUtilities.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <BeJsonCpp/BeJsonUtilities.h>
#include <Bentley/Base64Utilities.h>
#include <GeomSerialization/GeomLibsSerialization.h>
#include <GeomSerialization/GeomLibsJsonSerialization.h>
#include <json/value.h>

BEGIN_UNNAMED_NAMESPACE
    BE_JSON_NAME(rawValue)
    BE_JSON_NAME(formattedValue)
    BE_JSON_NAME(fusSpec)
END_UNNAMED_NAMESPACE

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

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
    json[JSON_POINT_X_KEY] = pt.x;
    json[JSON_POINT_Y_KEY] = pt.y;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::JsonToPoint2d(DPoint2d& pt, Json::Value const& json)
    {
    double x = 0.0;
    double y = 0.0;

    if (SUCCESS != PointCoordinateFromJson(x, json, JSON_POINT_X_KEY) ||
        SUCCESS != PointCoordinateFromJson(y, json, JSON_POINT_Y_KEY))
        return ERROR;

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
    json[JSON_POINT_X_KEY] = pt.x;
    json[JSON_POINT_Y_KEY] = pt.y;
    json[JSON_POINT_Z_KEY] = pt.z;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::JsonToPoint3d(DPoint3d& pt, Json::Value const& json)
    {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    if (SUCCESS != PointCoordinateFromJson(x, json, JSON_POINT_X_KEY) ||
        SUCCESS != PointCoordinateFromJson(y, json, JSON_POINT_Y_KEY) ||
        SUCCESS != PointCoordinateFromJson(z, json, JSON_POINT_Z_KEY))
        return ERROR;

    pt = DPoint3d::From(x, y, z);
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::PointCoordinateFromJson(double& coordinate, Json::Value const& json, Utf8CP coordinateKey)
    {
    if (json.isNull() || !json.isObject())
        return ERROR;

    Json::Value const& coordinateJson = json[coordinateKey];
    if (coordinateJson.isNull() || !coordinateJson.isNumeric())
        return ERROR;

    coordinate = coordinateJson.asDouble();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 1/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECJsonUtilities::ECPrimitiveValueFromJson(ECValueR ecValue, const Json::Value& jsonValue, PrimitiveType primitiveType)
    {
    Json::ValueType jsonValueType = jsonValue.type();

    BentleyStatus status = SUCCESS;
    switch (primitiveType)
        {
            case PRIMITIVETYPE_Point2d:
            {
            DPoint2d point2d;
            if (JsonToPoint2d(point2d, jsonValue))
                return ERROR;

            status = ecValue.SetPoint2d(point2d);
            break;
            }
            case PRIMITIVETYPE_Point3d:
            {
            DPoint3d point3d;
            if (JsonToPoint3d(point3d, jsonValue))
                return ERROR;

            status = ecValue.SetPoint3d(point3d);
            break;
            }
            case PRIMITIVETYPE_Integer:
                if (!EXPECTED_CONDITION(jsonValueType == Json::intValue || jsonValueType == Json::stringValue))
                    return ERROR;
                if (jsonValue.isInt())
                    status = ecValue.SetInteger(jsonValue.asInt());
                else if (jsonValue.isString())
                    status = ecValue.SetInteger(std::stoi(jsonValue.asCString()));
                break;
            case PRIMITIVETYPE_Long:
                if (!EXPECTED_CONDITION(jsonValueType == Json::stringValue  && "int64_t values need to be serialized as strings to allow use in Javascript"))
                    return ERROR;
                status = ecValue.SetLong(BeJsonUtilities::Int64FromValue(jsonValue));
                break;
            case PRIMITIVETYPE_Double:
                if (!jsonValue.isConvertibleTo(Json::ValueType::realValue) && !jsonValue.isString())
                    return ERROR;
                if (jsonValue.isDouble())
                    status = ecValue.SetDouble(jsonValue.asDouble());
                else if (jsonValue.isInt())
                    status = ecValue.SetDouble((double)jsonValue.asInt());
                else if (jsonValue.isString())
                    status = ecValue.SetDouble(std::stod(jsonValue.asCString()));
                else
                    {
                    BeAssert(false && "Invalid type to convert to double");
                    return ERROR;
                    }
                break;
            case PRIMITIVETYPE_DateTime:
            {
            if (!EXPECTED_CONDITION(jsonValueType == Json::stringValue))
                return ERROR;
            DateTime dateTime;
            DateTime::FromString(dateTime, jsonValue.asString().c_str());
            status = ecValue.SetDateTime(dateTime);
            break;
            }
            case PRIMITIVETYPE_String:
                if (!EXPECTED_CONDITION(jsonValueType == Json::stringValue))
                    return ERROR;
                status = ecValue.SetUtf8CP(jsonValue.asString().c_str(), true);
                break;
            case PRIMITIVETYPE_Boolean:
                if (!EXPECTED_CONDITION(jsonValueType == Json::booleanValue))
                    return ERROR;
                status = ecValue.SetBoolean(jsonValue.asBool());
                break;
            case PRIMITIVETYPE_Binary:
            {
            bvector<Byte> blob;
            if (SUCCESS != JsonToBinary(blob, jsonValue))
                return ERROR;

            status = ecValue.SetBinary(blob.data(), blob.size(), true);
            break;
            }
            case PRIMITIVETYPE_IGeometry:
            {
            if (!EXPECTED_CONDITION(jsonValueType == Json::objectValue))
                return ERROR;

            if (jsonValue.isNull())
                return SUCCESS;

            bvector<IGeometryPtr> geometry;
            if (!BentleyGeometryJson::TryJsonValueToGeometry(jsonValue, geometry))
                return ERROR;

            BeAssert(geometry.size() == 1);
            return ecValue.SetIGeometry(*(geometry[0]));
            }
            default:
                status = ERROR;
        }

    POSTCONDITION(status == SUCCESS, ERROR);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 1/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECJsonUtilities::ECArrayValueFromJson(IECInstanceR instance, const Json::Value& jsonValue, ECPropertyCR property, Utf8StringCR accessString)
    {
    NavigationECPropertyCP navProp = property.GetAsNavigationProperty();

    if ((!property.GetIsArray() && navProp == nullptr) || (navProp != nullptr && !navProp->IsMultiple()))
        return ERROR;

    if (!EXPECTED_CONDITION(jsonValue.isArray()))
        return ERROR;

    BentleyStatus r_status = SUCCESS;
    uint32_t length = jsonValue.size();
    if (length == 0)
        return SUCCESS;

    ECObjectsStatus status = instance.AddArrayElements(accessString.c_str(), length);
    POSTCONDITION(ECObjectsStatus::Success == status, ERROR);

    if (property.GetIsStructArray())
        {
        auto structArrayProperty = property.GetAsStructArrayProperty();
        if (nullptr == structArrayProperty)
            return ERROR;

        ECClassCR structType = structArrayProperty->GetStructElementType();
        for (uint32_t ii = 0; ii < length; ii++)
            {
            IECInstancePtr structInstance = structType.GetDefaultStandaloneEnabler()->CreateInstance(0);
            ECInstanceFromJson(*structInstance, jsonValue[ii], structType, "");
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

    PrimitiveArrayECPropertyCP arrProp = property.GetAsPrimitiveArrayProperty();
    PrimitiveType primType = arrProp != nullptr ? arrProp->GetPrimitiveElementType() : navProp->GetType();

    for (uint32_t ii = 0; ii < length; ii++)
        {
        ECValue ecPrimitiveValue;
        BentleyStatus status = ECPrimitiveValueFromJson(ecPrimitiveValue, jsonValue[ii], primType);
        if (SUCCESS != status)
            {
            r_status = status;
            continue;
            }
        ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), ecPrimitiveValue, ii);
        if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
            {
            BeAssert(false);
            }
        }

    return r_status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 2/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECJsonUtilities::ECInstanceFromJson(IECInstanceR instance, const Json::Value& jsonValue, IECSchemaRemapperCP remapper)
    {
    return ECInstanceFromJson(instance, jsonValue, instance.GetClass(), "", remapper);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 1/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECJsonUtilities::ECInstanceFromJson(IECInstanceR instance, const Json::Value& jsonValue, ECClassCR currentClass, Utf8StringCR currentAccessString, IECSchemaRemapperCP remapper)
    {
    if (!jsonValue.isObject())
        return ERROR;

    BentleyStatus status = SUCCESS;
    for (Json::Value::iterator iter = jsonValue.begin(); iter != jsonValue.end(); iter++)
        {
        Json::Value& childJsonValue = *iter;
        if (childJsonValue.isNull())
            continue;

        Utf8CP memberName = iter.memberName();
        if (*memberName == '$')
            continue;

        Utf8String remappedMemberName(memberName);
        if (nullptr != remapper)
            remapper->ResolvePropertyName(remappedMemberName, currentClass);
        ECPropertyP ecProperty = currentClass.GetPropertyP(remappedMemberName.c_str());
        if (!EXPECTED_CONDITION(ecProperty != nullptr))
            {
            status = ERROR;
            continue;
            }

        Utf8String accessString = (currentAccessString[0] == 0) ? remappedMemberName.c_str() : currentAccessString + "." + remappedMemberName.c_str();
        if (ecProperty->GetIsPrimitive())
            {
            ECValue ecValue;
            PrimitiveECPropertyCP primitiveProperty = ecProperty->GetAsPrimitiveProperty();
            PrimitiveType primitiveType = primitiveProperty->GetType();
            if (SUCCESS != ECPrimitiveValueFromJson(ecValue, childJsonValue, primitiveType))
                {
                status = ERROR;
                continue;
                }
            ECObjectsStatus ecStatus;
            ecStatus = instance.SetInternalValue(accessString.c_str(), ecValue);
            BeAssert(ecStatus == ECObjectsStatus::Success || ecStatus == ECObjectsStatus::PropertyValueMatchesNoChange);
            continue;
            }
        else if (ecProperty->GetIsStruct())
            {
            StructECPropertyCP structProperty = ecProperty->GetAsStructProperty();
            if (SUCCESS != ECInstanceFromJson(instance, childJsonValue, structProperty->GetType(), accessString, remapper))
                status = ERROR;
            continue;
            }
        else if (ecProperty->GetIsArray())
            {
            if (SUCCESS != ECArrayValueFromJson(instance, childJsonValue, *ecProperty, accessString))
                {
                status = ERROR;
                continue;
                }
            }
        else if (ecProperty->GetIsNavigation())
            {
            //JSON structure for nav props:
            //"<NavPropName>" : {"id":"<Related id>"[, "relECClassId":"<RelECClassId>"]}
            NavigationECPropertyCP navProp = ecProperty->GetAsNavigationProperty();
            if (navProp->IsMultiple())
                {
                LOG.error("NavigationECProperties with IsMultiple == true not supported by ECJsonUtilities");
                status = ERROR;
                continue;
                }

            if (!childJsonValue.isObject() || !childJsonValue.isMember(ECINSTANCE_ID_ATTRIBUTE))
                {
                status = ERROR;
                continue;
                }

            const uint64_t navId = (uint64_t) BeJsonUtilities::Int64FromValue(childJsonValue[ECINSTANCE_ID_ATTRIBUTE], INT64_C(0));
            if (navId == INT64_C(0))
                {
                status = ERROR;
                continue;
                }

            ECValue v;
            if (!childJsonValue.isMember(ECINSTANCE_RELATIONSHIPID_ATTTRIBUTE))
                {
                if (ECObjectsStatus::Success != v.SetNavigationInfo(BeInt64Id(navId)))
                    status = ERROR;
                }
            else
                {
                const uint64_t relClassId = (uint64_t) BeJsonUtilities::Int64FromValue(childJsonValue[ECINSTANCE_RELATIONSHIPID_ATTTRIBUTE], INT64_C(0));
                if (relClassId == INT64_C(0) || ECObjectsStatus::Success != v.SetNavigationInfo(BeInt64Id(navId), ECClassId(relClassId)))
                    status = ERROR;
                }

            if (SUCCESS == status)
                {
                ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), v);
                if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
                    status = ERROR;
                }

            continue;
            }
        }

    return status;
    }

//=======================================================================================
//  ECRapidJsonUtility
//=======================================================================================

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    07/2016
//+---------------+---------------+---------------+---------------+---------------+------
void ECRapidJsonUtilities::Int64ToStringJsonValue(RapidJsonValueR json, int64_t val, rapidjson::MemoryPoolAllocator<>& allocator)
    {
    char str[32];
    const int len = sprintf(str, "%" PRId64, val);
    json.SetString(str, len, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t ECRapidJsonUtilities::Int64FromJson(RapidJsonValueCR value, int64_t defaultOnError)
    {
    if (value.IsNull())
        return defaultOnError;

    if (value.IsNumber())
        return value.GetInt64();

    // strings are used in JavaScript because of UInt64 issues
    if (value.IsString())
        {
        int64_t returnValueInt64 = defaultOnError;
        sscanf(value.GetString(), "%" SCNd64, &returnValueInt64);
        return returnValueInt64;
        }

    return defaultOnError;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECRapidJsonUtilities::BinaryToJson(RapidJsonValueR json, Byte const* binary, size_t binarySize, rapidjson::MemoryPoolAllocator<>& allocator)
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
BentleyStatus ECRapidJsonUtilities::JsonToBinary(bvector<Byte>& binary, RapidJsonValueCR json)
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
BentleyStatus ECRapidJsonUtilities::JsonToBinary(ByteStream& binary, RapidJsonValueCR json)
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
BentleyStatus ECRapidJsonUtilities::Point2dToJson(RapidJsonValueR json, DPoint2d pt, rapidjson::MemoryPoolAllocator<>& allocator)
    {
    json.SetObject();
    rapidjson::Value coordVal(pt.x);
    json.AddMember(JSON_POINT_X_KEY, coordVal, allocator);
    coordVal.SetDouble(pt.y);
    json.AddMember(JSON_POINT_Y_KEY, coordVal, allocator);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECRapidJsonUtilities::JsonToPoint2d(DPoint2d& pt, RapidJsonValueCR json)
    {
    if (json.IsNull() || !json.IsObject())
        return ERROR;

    double x = 0.0;
    double y = 0.0;

    if (SUCCESS != PointCoordinateFromJson(x, json, JSON_POINT_X_KEY) ||
        SUCCESS != PointCoordinateFromJson(y, json, JSON_POINT_Y_KEY))
        return ERROR;

    pt = DPoint2d::From(x, y);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECRapidJsonUtilities::Point3dToJson(RapidJsonValueR json, DPoint3d pt, rapidjson::MemoryPoolAllocator<>& allocator)
    {
    json.SetObject();
    rapidjson::Value coordVal(pt.x);
    json.AddMember(JSON_POINT_X_KEY, coordVal, allocator);
    coordVal.SetDouble(pt.y);
    json.AddMember(JSON_POINT_Y_KEY, coordVal, allocator);
    coordVal.SetDouble(pt.z);
    json.AddMember(JSON_POINT_Z_KEY, coordVal, allocator);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECRapidJsonUtilities::JsonToPoint3d(DPoint3d& pt, RapidJsonValueCR json)
    {
    if (json.IsNull() || !json.IsObject())
        return ERROR;

    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    if (SUCCESS != PointCoordinateFromJson(x, json, JSON_POINT_X_KEY) ||
        SUCCESS != PointCoordinateFromJson(y, json, JSON_POINT_Y_KEY) ||
        SUCCESS != PointCoordinateFromJson(z, json, JSON_POINT_Z_KEY))
        return ERROR;

    pt = DPoint3d::From(x, y, z);
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECRapidJsonUtilities::PointCoordinateFromJson(double& coordinate, RapidJsonValueCR json, Utf8CP coordinateKey)
    {
    auto it = json.FindMember(coordinateKey);
    if (it == json.MemberEnd() || it->value.IsNull() || !it->value.IsNumber())
        return ERROR;

    coordinate = it->value.GetDouble();
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECRapidJsonUtilities::ECPrimitiveValueFromJson(ECValueR ecValue, RapidJsonValueCR jsonValue, PrimitiveType primitiveType)
    {
    switch (primitiveType)
        {
            case PRIMITIVETYPE_Point2d:
            {
            DPoint2d point2d;
            if (SUCCESS != JsonToPoint2d(point2d, jsonValue))
                return ERROR;

            return ecValue.SetPoint2d(point2d);
            }
            case PRIMITIVETYPE_Point3d:
            {
            DPoint3d point3d;
            if (SUCCESS != JsonToPoint3d(point3d, jsonValue))
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
            if (jsonValue.IsInt64())
                return ecValue.SetLong(jsonValue.GetInt64());

            if (jsonValue.IsUint64())
                return ecValue.SetLong((int64_t) jsonValue.GetUint64());

            if (jsonValue.IsInt())
                return ecValue.SetLong(jsonValue.GetInt());

            if (jsonValue.IsUint())
                return ecValue.SetLong(jsonValue.GetUint());

            // Int64 values can be represented in JSON as base64 string (to be compatible with JavaScript)
            if (jsonValue.IsString())
                return ecValue.SetLong(Int64FromJson(jsonValue));

            return ERROR;
            }
            case PRIMITIVETYPE_Double:
                if (!jsonValue.IsNumber())
                    return ERROR;

                return ecValue.SetDouble(jsonValue.GetDouble());

            case PRIMITIVETYPE_DateTime:
            {
            if (!jsonValue.IsString())
                return ERROR;

            DateTime dateTime;
            DateTime::FromString(dateTime, jsonValue.GetString());
            return ecValue.SetDateTime(dateTime);
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
            if (!jsonValue.IsObject())
                return ERROR;

            if (jsonValue.IsNull())
                return SUCCESS;

            rapidjson::StringBuffer stringBuffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);
            jsonValue.Accept(writer);

            Utf8String jsonStr(stringBuffer.GetString());

            bvector<IGeometryPtr> geometry;
            if (!BentleyGeometryJson::TryJsonStringToGeometry(jsonStr, geometry))
                return ERROR;

            BeAssert(geometry.size() == 1);
            return ecValue.SetIGeometry(*(geometry[0]));
            }
            case PRIMITIVETYPE_Binary:
            {
            bvector<Byte> blob;
            if (SUCCESS != JsonToBinary(blob, jsonValue))
                return ERROR;

            return ecValue.SetBinary(blob.data(), blob.size(), true);
            }
            default:
                return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECRapidJsonUtilities::ECArrayValueFromJson(IECInstanceR instance, RapidJsonValueCR jsonValue, ECPropertyCR property, Utf8StringCR accessString)
    {
    NavigationECPropertyCP navProp = property.GetAsNavigationProperty();

    if (!jsonValue.IsArray())
        return ERROR;

    if ((!property.GetIsArray() && navProp == nullptr) || (navProp != nullptr && !navProp->IsMultiple()))
        return ERROR;

    rapidjson::SizeType size = jsonValue.Size();
    if (0 == size)
        return SUCCESS;

    if (ECObjectsStatus::Success != instance.AddArrayElements(accessString.c_str(), size))
        return ERROR;

    if (property.GetIsStructArray())
        {
        StructArrayECPropertyCP structArrayProperty = property.GetAsStructArrayProperty();
        BeAssert(nullptr != structArrayProperty);

        ECClassCP structType = &structArrayProperty->GetStructElementType();
        BeAssert(nullptr != structType);

        for (rapidjson::SizeType i = 0; i < size; i++)
            {
            IECInstancePtr structInstance = structType->GetDefaultStandaloneEnabler()->CreateInstance(0);
            ECInstanceFromJson(*structInstance, jsonValue[i], *structType, "");

            ECValue structValue;
            structValue.SetStruct(structInstance.get());

            ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), structValue, i);
            if ((ECObjectsStatus::Success != ecStatus) && (ECObjectsStatus::PropertyValueMatchesNoChange != ecStatus))
                {
                BeAssert(false);
                }
            }

        return SUCCESS;
        }

    PrimitiveArrayECPropertyCP arrayProp = property.GetAsPrimitiveArrayProperty();
    PrimitiveType primType = arrayProp != nullptr ? arrayProp->GetPrimitiveElementType() : navProp->GetType();

    BentleyStatus returnStatus = SUCCESS;
    for (rapidjson::SizeType i = 0; i < size; i++)
        {
        ECValue primitiveValue;
        if (SUCCESS != ECPrimitiveValueFromJson(primitiveValue, jsonValue[i], primType))
            {
            returnStatus = ERROR;
            LogJsonParseError(jsonValue[i], instance.GetClass(), accessString);
            continue;
            }

        ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), primitiveValue, i);
        if ((ECObjectsStatus::Success != ecStatus) && (ECObjectsStatus::PropertyValueMatchesNoChange != ecStatus))
            {
            BeAssert(false);
            returnStatus = ERROR;
            }
        }

    return returnStatus;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECRapidJsonUtilities::ECInstanceFromJson(IECInstanceR instance, RapidJsonValueCR jsonValue)
    {
    return ECInstanceFromJson(instance, jsonValue, instance.GetClass(), "");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECRapidJsonUtilities::ECInstanceFromJson(ECN::IECInstanceR instance, RapidJsonValueCR jsonValue, ECClassCR currentClass, Utf8StringCR currentAccessString)
    {
    if (!jsonValue.IsObject())
        return ERROR;

    BentleyStatus status = SUCCESS;
    for (rapidjson::Value::ConstMemberIterator it = jsonValue.MemberBegin(); it != jsonValue.MemberEnd(); ++it)
        {
        if (it->value.IsNull())
            continue;

        if ('$' == it->name.GetString()[0])
            continue;

        Utf8CP propertyName = it->name.GetString();
        ECPropertyP propertyP = currentClass.GetPropertyP(propertyName);
        if (nullptr == propertyP)
            {
            LOG.errorv("Failed to create ECInstance from JSON: Property '%s' not found in ECClass '%s'.", propertyName, currentClass.GetFullName());
            status = ERROR;
            continue;
            }

        Utf8String accessString = currentAccessString.empty() ? propertyName : currentAccessString + "." + propertyName;
        if (propertyP->GetIsPrimitive())
            {
            ECValue ecValue;
            if (SUCCESS != ECPrimitiveValueFromJson(ecValue, it->value, propertyP->GetAsPrimitiveProperty()->GetType()))
                {
                status = ERROR;
                LogJsonParseError(it->value, instance.GetClass(), accessString);
                continue;
                }

            ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), ecValue);
            BeAssert((ECObjectsStatus::Success == ecStatus) || (ECObjectsStatus::PropertyValueMatchesNoChange == ecStatus));
            }
        else if (propertyP->GetIsStruct())
            {
            if (SUCCESS != ECInstanceFromJson(instance, it->value, propertyP->GetAsStructProperty()->GetType(), accessString))
                status = ERROR;
            }
        else if (propertyP->GetIsArray())
            {
            if (SUCCESS != ECArrayValueFromJson(instance, it->value, *propertyP, accessString))
                status = ERROR;
            }
        else if (propertyP->GetIsNavigation())
            {
            //JSON structure for nav props:
            //"<NavPropName>" : {"id":"<Related id>"[, "relECClassId":"<RelECClassId>"]}
            NavigationECPropertyCP navProp = propertyP->GetAsNavigationProperty();
            if (navProp->IsMultiple())
                {
                LOG.error("NavigationECProperties with IsMultiple == true not supported by ECRapidJsonUtilities");
                status = ERROR;
                continue;
                }

            RapidJsonValueCR json = it->value;

            if (!json.IsObject() || !json.HasMember(ECINSTANCE_ID_ATTRIBUTE))
                {
                status = ERROR;
                LogJsonParseError(json, instance.GetClass(), accessString);
                continue;
                }

            const uint64_t navId = (uint64_t) Int64FromJson(json[ECINSTANCE_ID_ATTRIBUTE], INT64_C(0));
            if (navId == INT64_C(0))
                {
                status = ERROR;
                LogJsonParseError(json, instance.GetClass(), accessString);
                continue;
                }

            ECValue v;
            if (!json.HasMember(ECINSTANCE_RELATIONSHIPID_ATTTRIBUTE))
                {
                if (ECObjectsStatus::Success != v.SetNavigationInfo(BeInt64Id(navId)))
                    {
                    status = ERROR;
                    LogJsonParseError(json, instance.GetClass(), accessString);
                    }
                }
            else
                {
                const uint64_t relClassId = (uint64_t) Int64FromJson(json[ECINSTANCE_RELATIONSHIPID_ATTTRIBUTE], INT64_C(0));
                if (relClassId == INT64_C(0) || ECObjectsStatus::Success != v.SetNavigationInfo(BeInt64Id(navId), ECClassId(relClassId)))
                    {
                    status = ERROR;
                    LogJsonParseError(json, instance.GetClass(), accessString);
                    }
                }

            if (SUCCESS == status)
                {
                const ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), v);
                if (ECObjectsStatus::Success != ecStatus && ECObjectsStatus::PropertyValueMatchesNoChange != ecStatus)
                    {
                    status = ERROR;
                    LogJsonParseError(json, instance.GetClass(), accessString);
                    }
                }

            continue;
            }
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  05/17
//---------------------------------------------------------------------------------------
void ECRapidJsonUtilities::LogJsonParseError(RapidJsonValueCR json, ECClassCR ecClass, Utf8StringCR propAccessString)
    {
    if (!LOG.isSeverityEnabled(NativeLogging::LOG_ERROR))
        return;

    rapidjson::StringBuffer jsonStrBuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(jsonStrBuf);
    json.Accept(writer);
    LOG.errorv("Failed to convert JSON '%s' to an ECValue for property '%s' in ECClass '%s'.", jsonStrBuf.GetString(), propAccessString.c_str(), ecClass.GetFullName());
    }

/////////////////////////////////////////////////////////////////////////////////////////
// JsonEcInstanceWriter
/////////////////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  02/2016
//---------------------------------------------------------------------------------------
void                 JsonEcInstanceWriter::AppendAccessString(Utf8String& compoundAccessString, Utf8String& baseAccessString, const Utf8String& propertyName)
    {
    compoundAccessString = baseAccessString;
    compoundAccessString.append(propertyName);
    }

//---------------------------------------------------------------------------------------
// TODO: add koq process for all data types
// @bsimethod                                    Bill.Steinbock                  02/2016
//---------------------------------------------------------------------------------------
StatusInt     JsonEcInstanceWriter::WritePrimitiveValue(Json::Value& valueToPopulate, Utf8CP propertyName, ECValueCR ecValue, PrimitiveType propertyType, Utf8CP fusSpec)
    {
    // write the content according to type.
    switch (propertyType)
        {
#if NOT_YET
        case PRIMITIVETYPE_Binary:
            {
            size_t      numBytes;
            const Byte* byteData;
            if (NULL != (byteData = ecValue.GetBinary(numBytes)))
                {
                Utf8String    byteString;
                convertByteArrayToString(byteString, byteData, numBytes);
                m_xmlWriter->WriteRaw(byteString.c_str());
                }
            return BSISUCCESS;
            break;
            }

            case PRIMITIVETYPE_IGeometry:
            {
            bmap<OrderedIGeometryPtr, BeExtendedData> extendedData;
            Utf8String beCgXml;
            BeXmlCGWriter::Write(beCgXml, *(ecValue.GetIGeometry()), &extendedData);
            m_xmlWriter->WriteRaw(beCgXml.c_str());
            //strcpy(outString, beCgXml.c_str());
            return BSISUCCESS;
            break;
            }
#endif
        case PRIMITIVETYPE_Boolean:
            {
            valueToPopulate[propertyName] = ecValue.GetBoolean();
            break;
            }

        case PRIMITIVETYPE_DateTime:
            {
            valueToPopulate[propertyName] = ecValue.ToString();
            break;
            }

        case PRIMITIVETYPE_Double:
            {
            if (fusSpec && *fusSpec)
                {
                // TODO: Quantities -- use fusSpec to get formatted string for value 
                Utf8PrintfString formattedVal("%f", ecValue.GetDouble());

                Json::Value quantityValue(Json::objectValue);
                quantityValue[json_rawValue()] = ecValue.GetDouble();
                quantityValue[json_formattedValue()] = formattedVal;
                quantityValue[json_fusSpec()] = fusSpec;

                valueToPopulate[propertyName] = quantityValue;
                }
            else
                {
                valueToPopulate[propertyName] = ecValue.GetDouble();
                }
            break;
            }

        case PRIMITIVETYPE_Integer:
            {
            if (fusSpec && *fusSpec)
                {
                // TODO: Quantities -- use fusSpec to get formatted string for value 
                Utf8PrintfString formattedVal("%d", ecValue.GetDouble());

                Json::Value quantityValue(Json::objectValue);
                quantityValue[json_rawValue()] = ecValue.GetInteger();
                quantityValue[json_formattedValue()] = formattedVal;
                quantityValue[json_fusSpec()] = fusSpec;

                valueToPopulate[propertyName] = quantityValue;
                }
            else
                {
                valueToPopulate[propertyName] = ecValue.GetInteger();
                }
            break;
            }

        case PRIMITIVETYPE_Long:
            {
            if (fusSpec && *fusSpec)
                {
                // TODO: Quantities -- use fusSpec to get formatted string for value 
                Json::Value quantityValue(Json::objectValue);
                quantityValue[json_rawValue()] = ecValue.GetLong();
                quantityValue[json_formattedValue()] = BeJsonUtilities::StringValueFromInt64(ecValue.GetLong());;
                quantityValue[json_fusSpec()] = fusSpec;

                valueToPopulate[propertyName] = quantityValue;
                }
            else
                {
                valueToPopulate[propertyName] = ecValue.GetLong();
                }

            break;
            }

        case PRIMITIVETYPE_Point2d:
            {
            return ECJsonUtilities::Point2dToJson(valueToPopulate[propertyName], ecValue.GetPoint2d());
            }

        case PRIMITIVETYPE_Point3d:
            {
            return ECJsonUtilities::Point3dToJson(valueToPopulate[propertyName], ecValue.GetPoint3d());
            }

        case PRIMITIVETYPE_String:
            {
            valueToPopulate[propertyName] = ecValue.GetUtf8CP();
            break;
            }

        default:
            {
            BeAssert(false);
            return BSIERROR;
            }
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  05/2017
//---------------------------------------------------------------------------------------
static Utf8String getFusSpec(ECProperty const& primitiveProperty)
    {
    auto koq = primitiveProperty.GetKindOfQuantity();
    if (koq)
        return koq->GetName();

    return "";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  02/2016
//---------------------------------------------------------------------------------------
StatusInt     JsonEcInstanceWriter::WritePrimitivePropertyValue(Json::Value& valueToPopulate, PrimitiveECPropertyR primitiveProperty, IECInstanceCR ecInstance, Utf8String* baseAccessString, bool writeFormattedQuanties)
    {
    ECObjectsStatus     getStatus;
    ECValue             ecValue;
    Utf8StringCR propertyName = primitiveProperty.GetName();

    Utf8String fusSpec;
    if (writeFormattedQuanties)
        fusSpec = getFusSpec(primitiveProperty);

    if (NULL == baseAccessString)
        {
        getStatus = ecInstance.GetValue(ecValue, propertyName.c_str());
        }
    else
        {
        Utf8String compoundAccessString;
        AppendAccessString(compoundAccessString, *baseAccessString, propertyName);
        getStatus = ecInstance.GetValue(ecValue, compoundAccessString.c_str());
        }

    // couldn't get, or NULL value, write nothing.
    if ((ECObjectsStatus::Success != getStatus) || ecValue.IsNull())
        return BSISUCCESS;

    PrimitiveType           propertyType = primitiveProperty.GetType();

    StatusInt status = WritePrimitiveValue(valueToPopulate, propertyName.c_str(), ecValue, propertyType, fusSpec.c_str());
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  02/2016
//---------------------------------------------------------------------------------------
StatusInt     JsonEcInstanceWriter::WritePrimitiveValueForPresentation(Json::Value& valueToPopulate, PrimitiveECPropertyR primitiveProperty, IECInstanceCR ecInstance, Utf8String* baseAccessString)
    {
    return JsonEcInstanceWriter::WritePrimitivePropertyValue(valueToPopulate, primitiveProperty, ecInstance, baseAccessString, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  02/2016
//---------------------------------------------------------------------------------------
StatusInt     JsonEcInstanceWriter::WritePrimitiveValue(Json::Value& valueToPopulate, PrimitiveECPropertyR primitiveProperty, IECInstanceCR ecInstance, Utf8String* baseAccessString)
    {
    return JsonEcInstanceWriter::WritePrimitivePropertyValue(valueToPopulate, primitiveProperty, ecInstance, baseAccessString, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  02/2016
//---------------------------------------------------------------------------------------
StatusInt     JsonEcInstanceWriter::WriteArrayPropertyValue(Json::Value& valueToPopulate, ArrayECPropertyR arrayProperty, IECInstanceCR ecInstance, Utf8String* baseAccessString, bool writeFormattedQuanties)
    {
    ArrayKind       arrayKind = arrayProperty.GetKind();

    Utf8String    accessString;
    if (NULL == baseAccessString)
        accessString = arrayProperty.GetName();
    else
        AppendAccessString(accessString, *baseAccessString, arrayProperty.GetName());

    // no members, don't write anything.
    ECValue         ecValue;
    if (ECObjectsStatus::Success != ecInstance.GetValue(ecValue, accessString.c_str()) || ecValue.IsNull() || ecValue.GetArrayInfo().GetCount() == 0)
        return BSISUCCESS;

    uint32_t nElements = ecValue.GetArrayInfo().GetCount();

    auto& arrayObj = valueToPopulate[arrayProperty.GetName().c_str()] = Json::arrayValue;
    Json::Value entryObj(Json::objectValue);

    StatusInt     ixwStatus;
    if (ARRAYKIND_Primitive == arrayKind)
        {
        ECPropertyCP  primitiveProperty = arrayProperty.GetAsPrimitiveArrayProperty();
        PrimitiveType   memberType = arrayProperty.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();
        Utf8CP          typeString = ECXml::GetPrimitiveTypeName(memberType);

        Utf8String fusSpec;
        if (writeFormattedQuanties)
            fusSpec = getFusSpec(*primitiveProperty);

        for (uint32_t index = 0; index < nElements; index++)
            {
            entryObj.clear();

            if (ECObjectsStatus::Success != ecInstance.GetValue(ecValue, accessString.c_str(), index))
                break;

            // write the primitive value
            if (BSISUCCESS != (ixwStatus = WritePrimitiveValue(entryObj, typeString, ecValue, memberType, fusSpec.c_str())))
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
            if (BSISUCCESS != (iwxStatus = WritePropertyValuesOfClassOrStructArrayMember(entryObj, structClass, *structInstance.get(), nullptr)))
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
StatusInt     JsonEcInstanceWriter::WriteEmbeddedStructPropertyValue(Json::Value& valueToPopulate, StructECPropertyR structProperty, IECInstanceCR ecInstance, Utf8String* baseAccessString, bool writeFormattedQuanties)
    {
    Utf8String    structName = structProperty.GetName();

    auto& structObj = valueToPopulate[structName.c_str()] = Json::objectValue;

    Utf8String    thisAccessString;
    if (NULL != baseAccessString)
        AppendAccessString(thisAccessString, *baseAccessString, structName);
    else
        thisAccessString = structName.c_str();
    thisAccessString.append(".");

    ECClassCR   structClass = structProperty.GetType();
    WritePropertyValuesOfClassOrStructArrayMember(structObj, structClass, ecInstance, &thisAccessString);

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  05/2017
//---------------------------------------------------------------------------------------
StatusInt     JsonEcInstanceWriter::WriteEmbeddedStructValue(Json::Value& valueToPopulate, ECN::StructECPropertyR structProperty, ECN::IECInstanceCR ecInstance, Utf8String* baseAccessString)
    {
    return JsonEcInstanceWriter::WriteEmbeddedStructPropertyValue(valueToPopulate, structProperty, ecInstance, baseAccessString, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  05/2017
//---------------------------------------------------------------------------------------
StatusInt     JsonEcInstanceWriter::WriteEmbeddedStructValueForPresentation(Json::Value& valueToPopulate, ECN::StructECPropertyR structProperty, ECN::IECInstanceCR ecInstance, Utf8String* baseAccessString)
    {
    return JsonEcInstanceWriter::WriteEmbeddedStructPropertyValue(valueToPopulate, structProperty,ecInstance, baseAccessString, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                    12/2016
//---------------------------------------------------------------------------------------
StatusInt     JsonEcInstanceWriter::WriteNavigationPropertyValue(Json::Value& valueToPopulate, NavigationECPropertyR structProperty, IECInstanceCR ecInstance, Utf8String* baseAccessString, bool writeFormattedQuanties)
    {

    return BSIERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  02/2016
//---------------------------------------------------------------------------------------
    StatusInt     JsonEcInstanceWriter::WritePropertyValuesOfClassOrStructArrayMember(Json::Value& valueToPopulate, ECClassCR ecClass, IECInstanceCR ecInstance, Utf8String* baseAccessString, bool writeFormattedQuanties)
    {
    ECPropertyIterableCR    collection = ecClass.GetProperties(true);
    for (ECPropertyP ecProperty : collection)
        {
        PrimitiveECPropertyP    primitiveProperty;
        ArrayECPropertyP        arrayProperty;
        StructECPropertyP       structProperty;
        NavigationECPropertyP   navigationProperty;
        StatusInt               ixwStatus = BSIERROR;

        if (NULL != (primitiveProperty = ecProperty->GetAsPrimitivePropertyP()))
            ixwStatus = WritePrimitivePropertyValue(valueToPopulate, *primitiveProperty, ecInstance, baseAccessString, writeFormattedQuanties);
        else if (NULL != (arrayProperty = ecProperty->GetAsArrayPropertyP()))
            ixwStatus = WriteArrayPropertyValue(valueToPopulate, *arrayProperty, ecInstance, baseAccessString, writeFormattedQuanties);
        else if (NULL != (structProperty = ecProperty->GetAsStructPropertyP()))
            ixwStatus = WriteEmbeddedStructPropertyValue(valueToPopulate, *structProperty, ecInstance, baseAccessString, writeFormattedQuanties);
        else if (NULL != (navigationProperty = ecProperty->GetAsNavigationPropertyP()))
            ixwStatus = WriteNavigationPropertyValue(valueToPopulate, *navigationProperty, ecInstance, baseAccessString, writeFormattedQuanties);

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
StatusInt     JsonEcInstanceWriter::WriteInstanceToJson(Json::Value& valueToPopulate, IECInstanceCR ecInstance, Utf8CP instanceName, bool writeInstanceId)
    {
    ECClassCR   ecClass = ecInstance.GetClass();
    ECSchemaCR  ecSchema = ecClass.GetSchema();
    Utf8String  className = ecClass.GetName();
    Utf8String  fullSchemaName;

    valueToPopulate[ECINSTANCE_CLASS_ATTRIBUTE] = className.c_str();
    fullSchemaName.Sprintf("%s.%02" PRIu32 ".%02" PRIu32, ecSchema.GetName().c_str(), ecSchema.GetVersionRead(), ecSchema.GetVersionMinor());
    valueToPopulate[ECINSTANCE_SCHEMA_ATTRIBUTE] = fullSchemaName.c_str();

    if (writeInstanceId)
        valueToPopulate[ECINSTANCE_INSTANCEID_JSON_ATTRIBUTE] = ecInstance.GetInstanceIdForSerialization().c_str();

    Json::Value instanceObj(Json::objectValue);
    StatusInt status = WritePropertyValuesOfClassOrStructArrayMember(instanceObj, ecClass, ecInstance, nullptr, false);
    if (status != BSISUCCESS)
        return status;

    if (nullptr != instanceName)
        valueToPopulate[instanceName] = instanceObj;
    else
        valueToPopulate[className.c_str()] = instanceObj;

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  02/2016
//---------------------------------------------------------------------------------------
StatusInt     JsonEcInstanceWriter::WriteInstanceToPresentationJson(Json::Value& valueToPopulate, IECInstanceCR ecInstance, Utf8CP instanceName, bool writeInstanceId)
    {
    ECClassCR   ecClass = ecInstance.GetClass();
    ECSchemaCR  ecSchema = ecClass.GetSchema();
    Utf8String  className = ecClass.GetName();
    Utf8String  fullSchemaName;

    valueToPopulate[ECINSTANCE_CLASS_ATTRIBUTE] = className.c_str();
    fullSchemaName.Sprintf("%s.%02" PRIu32 ".%02" PRIu32, ecSchema.GetName().c_str(), ecSchema.GetVersionRead(), ecSchema.GetVersionMinor());
    valueToPopulate[ECINSTANCE_SCHEMA_ATTRIBUTE] = fullSchemaName.c_str();

    if (writeInstanceId)
        valueToPopulate[ECINSTANCE_INSTANCEID_JSON_ATTRIBUTE] = ecInstance.GetInstanceIdForSerialization().c_str();

    Json::Value instanceObj(Json::objectValue);
    StatusInt status = WritePropertyValuesOfClassOrStructArrayMember(instanceObj, ecClass, ecInstance, nullptr, true);
    if (status != BSISUCCESS)
        return status;

    if (nullptr != instanceName)
        valueToPopulate[instanceName] = instanceObj;
    else
        valueToPopulate[className.c_str()] = instanceObj;

    return BSISUCCESS;
    }


END_BENTLEY_ECOBJECT_NAMESPACE
