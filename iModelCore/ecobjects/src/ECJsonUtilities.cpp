/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECJsonUtilities.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <BeJsonCpp/BeJsonUtilities.h>
#include <Bentley/Base64Utilities.h>
#include <GeomSerialization/GeomLibsSerialization.h>
#include <GeomSerialization/GeomLibsJsonSerialization.h>

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
    if (SUCCESS != Base64Utilities::Encode(str, binary, binarySize))
        return ERROR;

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

    Utf8String base64Str = json.asString();
    return Base64Utilities::Decode(binary, base64Str);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECJsonUtilities::Point2DToJson(Json::Value& json, DPoint2d pt)
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
BentleyStatus ECJsonUtilities::JsonToPoint2D(DPoint2d& pt, Json::Value const& json)
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
BentleyStatus ECJsonUtilities::Point3DToJson(Json::Value& json, DPoint3d pt)
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
BentleyStatus ECJsonUtilities::JsonToPoint3D(DPoint3d& pt, Json::Value const& json)
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
            case PRIMITIVETYPE_Point2D:
            {
            DPoint2d point2d;
            if (JsonToPoint2D(point2d, jsonValue))
                return ERROR;

            status = ecValue.SetPoint2D(point2d);
            break;
            }
            case PRIMITIVETYPE_Point3D:
            {
            DPoint3d point3d;
            if (JsonToPoint3D(point3d, jsonValue))
                return ERROR;

            status = ecValue.SetPoint3D(point3d);
            break;
            }
            case PRIMITIVETYPE_Integer:
                if (!EXPECTED_CONDITION(jsonValueType == Json::intValue))
                    return ERROR;
                status = ecValue.SetInteger(jsonValue.asInt());
                break;
            case PRIMITIVETYPE_Long:
                if (!EXPECTED_CONDITION(jsonValueType == Json::stringValue  && "int64_t values need to be serialized as strings to allow use in Javascript"))
                    return ERROR;
                status = ecValue.SetLong(BeJsonUtilities::Int64FromValue(jsonValue));
                break;
            case PRIMITIVETYPE_Double:
                if (!EXPECTED_CONDITION(jsonValueType == Json::realValue))
                    return ERROR;
                status = ecValue.SetDouble(jsonValue.asDouble());
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
    ArrayECPropertyCP arrayProp = property.GetAsArrayProperty();
    NavigationECPropertyCP navProp = property.GetAsNavigationProperty();

    if ((arrayProp == nullptr && navProp == nullptr) || (navProp != nullptr && !navProp->IsMultiple()))
        return ERROR;

    if (!EXPECTED_CONDITION(jsonValue.isArray()))
        return ERROR;

    BentleyStatus r_status = SUCCESS;
    uint32_t length = jsonValue.size();
    if (length == 0)
        return SUCCESS;

    ECObjectsStatus status = instance.AddArrayElements(accessString.c_str(), length);
    POSTCONDITION(ECObjectsStatus::Success == status, ERROR);

    if (arrayProp != nullptr && arrayProp->GetKind() == ARRAYKIND_Struct)
        {
        auto structArrayProperty = arrayProp->GetAsStructArrayProperty();
        if (nullptr == structArrayProperty)
            return ERROR;

        ECClassCP structType = structArrayProperty->GetStructElementType();
        BeAssert(structType != nullptr);
        for (uint32_t ii = 0; ii < length; ii++)
            {
            IECInstancePtr structInstance = structType->GetDefaultStandaloneEnabler()->CreateInstance(0);
            ECInstanceFromJson(*structInstance, jsonValue[ii], *structType, "");
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

    PrimitiveType primType = arrayProp != nullptr ? arrayProp->GetPrimitiveElementType() : navProp->GetType();

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
BentleyStatus ECJsonUtilities::ECInstanceFromJson(IECInstanceR instance, const Json::Value& jsonValue)
    {
    return ECInstanceFromJson(instance, jsonValue, instance.GetClass(), "");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 1/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECJsonUtilities::ECInstanceFromJson(IECInstanceR instance, const Json::Value& jsonValue, ECClassCR currentClass, Utf8StringCR currentAccessString)
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

        ECPropertyP ecProperty = currentClass.GetPropertyP(memberName);
        if (!EXPECTED_CONDITION(ecProperty != nullptr))
            {
            status = ERROR;
            continue;
            }

        Utf8String accessString = (currentAccessString[0] == 0) ? memberName : currentAccessString + "." + memberName;
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
            if (SUCCESS != ECInstanceFromJson(instance, childJsonValue, structProperty->GetType(), accessString))
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
            NavigationECPropertyCP navProp = ecProperty->GetAsNavigationProperty();
            PrimitiveType navPropIdType = navProp->GetType();
            if (!navProp->IsMultiple())
                {
                ECValue ecValue;
                if (SUCCESS != ECPrimitiveValueFromJson(ecValue, childJsonValue, navPropIdType))
                    {
                    status = ERROR;
                    continue;
                    }

                ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), ecValue);
                if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
                    status = ERROR;

                continue;
                }

            status = ECArrayValueFromJson(instance, childJsonValue, *navProp, accessString);
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
    if (SUCCESS != Base64Utilities::Encode(str, binary, binarySize))
        return ERROR;

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

    return Base64Utilities::Decode(binary, json.GetString(), (size_t) json.GetStringLength());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECRapidJsonUtilities::Point2DToJson(RapidJsonValueR json, DPoint2d pt, rapidjson::MemoryPoolAllocator<>& allocator)
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
BentleyStatus ECRapidJsonUtilities::JsonToPoint2D(DPoint2d& pt, RapidJsonValueCR json)
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
BentleyStatus ECRapidJsonUtilities::Point3DToJson(RapidJsonValueR json, DPoint3d pt, rapidjson::MemoryPoolAllocator<>& allocator)
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
BentleyStatus ECRapidJsonUtilities::JsonToPoint3D(DPoint3d& pt, RapidJsonValueCR json)
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
            case PRIMITIVETYPE_Point2D:
            {
            DPoint2d point2d;
            if (SUCCESS != JsonToPoint2D(point2d, jsonValue))
                return ERROR;

            return ecValue.SetPoint2D(point2d);
            }
            case PRIMITIVETYPE_Point3D:
            {
            DPoint3d point3d;
            if (SUCCESS != JsonToPoint3D(point3d, jsonValue))
                return ERROR;

            return ecValue.SetPoint3D(point3d);
            }
            case PRIMITIVETYPE_Integer:
                if (!jsonValue.IsInt())
                    return ERROR;

                return ecValue.SetInteger(jsonValue.GetInt());

            case PRIMITIVETYPE_Long:
                // Int64 values need to be serialized as strings to allow use in JavaScript
                if (!jsonValue.IsString())
                    return ERROR;

                return ecValue.SetLong(Int64FromJson(jsonValue));

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
    ArrayECPropertyCP arrayProp = property.GetAsArrayProperty();
    NavigationECPropertyCP navProp = property.GetAsNavigationProperty();

    if (!jsonValue.IsArray())
        return ERROR;

    if ((arrayProp == nullptr && navProp == nullptr) || (navProp != nullptr && !navProp->IsMultiple()))
        return ERROR;

    rapidjson::SizeType size = jsonValue.Size();
    if (0 == size)
        return SUCCESS;

    if (ECObjectsStatus::Success != instance.AddArrayElements(accessString.c_str(), size))
        return ERROR;

    if (arrayProp != nullptr && arrayProp->GetKind() == ARRAYKIND_Struct)
        {
        StructArrayECPropertyCP structArrayProperty = arrayProp->GetAsStructArrayProperty();
        BeAssert(nullptr != structArrayProperty);

        ECClassCP structType = structArrayProperty->GetStructElementType();
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

    PrimitiveType primType = arrayProp != nullptr ? arrayProp->GetPrimitiveElementType() : navProp->GetType();

    BentleyStatus returnStatus = SUCCESS;
    for (rapidjson::SizeType i = 0; i < size; i++)
        {
        ECValue primitiveValue;
        if (SUCCESS != ECPrimitiveValueFromJson(primitiveValue, jsonValue[i], primType))
            {
            returnStatus = ERROR;
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
            status = ERROR;
            continue;
            }

        Utf8String accessString = (0 == currentAccessString[0]) ? propertyName : currentAccessString + "." + propertyName;
        if (propertyP->GetIsPrimitive())
            {
            ECValue ecValue;
            if (SUCCESS != ECPrimitiveValueFromJson(ecValue, it->value, propertyP->GetAsPrimitiveProperty()->GetType()))
                {
                status = ERROR;
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
            NavigationECPropertyCP navProp = propertyP->GetAsNavigationProperty();
            PrimitiveType navPropIdType = navProp->GetType();
            if (!navProp->IsMultiple())
                {
                ECValue ecValue;
                if (SUCCESS != ECPrimitiveValueFromJson(ecValue, it->value, navPropIdType))
                    {
                    status = ERROR;
                    continue;
                    }

                ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), ecValue);
                if (ECObjectsStatus::Success != ecStatus && ECObjectsStatus::PropertyValueMatchesNoChange != ecStatus)
                    status = ERROR;

                continue;
                }

            if (SUCCESS != ECArrayValueFromJson(instance, it->value, *propertyP, accessString))
                status = ERROR;

            continue;
            }
        }

    return status;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
