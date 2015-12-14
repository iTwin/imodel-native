/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECJsonUtility.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <BeJsonCpp/BeJsonUtilities.h>
#include <GeomSerialization/GeomLibsSerialization.h>
#include <GeomSerialization/GeomLibsJsonSerialization.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 1/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ECJsonCppUtility::ECPrimitiveValueFromJsonValue (ECValueR ecValue, const Json::Value& jsonValue, PrimitiveType primitiveType)
    {
    Json::ValueType jsonValueType = jsonValue.type();

    StatusInt status = SUCCESS;
    switch (primitiveType)
        {
        case PRIMITIVETYPE_Point2D:
            if (!EXPECTED_CONDITION (jsonValueType == Json::objectValue))
                return ERROR;
            if (!EXPECTED_CONDITION (jsonValue["x"].isDouble() && jsonValue["y"].isDouble()))
                return ERROR;
            DPoint2d point2d;
            point2d.Init (jsonValue["x"].asDouble(), jsonValue["y"].asDouble());
            status = ecValue.SetPoint2D (point2d);
            break;
        case PRIMITIVETYPE_Point3D:
            if (!EXPECTED_CONDITION (jsonValueType == Json::objectValue))
                return ERROR;
            if (!EXPECTED_CONDITION (jsonValue["x"].isDouble() && jsonValue["y"].isDouble() && jsonValue["z"].isDouble()))
                return ERROR;
            DPoint3d point3d;
            point3d.Init (jsonValue["x"].asDouble(), jsonValue["y"].asDouble(), jsonValue["z"].asDouble());
            status = ecValue.SetPoint3D (point3d);
            break;
        case PRIMITIVETYPE_Integer: 
            if (!EXPECTED_CONDITION (jsonValueType == Json::intValue))
                return ERROR;
            status = ecValue.SetInteger (jsonValue.asInt());
            break;
        case PRIMITIVETYPE_Long:
            if (!EXPECTED_CONDITION (jsonValueType == Json::stringValue  && "int64_t values need to be serialized as strings to allow use in Javascript"))
                return ERROR;
            status = ecValue.SetLong (BeJsonUtilities::Int64FromValue (jsonValue));
            break;
        case PRIMITIVETYPE_Double:
            if (!EXPECTED_CONDITION (jsonValueType == Json::realValue))
                return ERROR;
            status = ecValue.SetDouble (jsonValue.asDouble());
            break;
        case PRIMITIVETYPE_DateTime:
            {
            if (!EXPECTED_CONDITION (jsonValueType == Json::stringValue))
                return ERROR;
            DateTime dateTime;
            DateTime::FromString (dateTime, jsonValue.asString().c_str());
            status = ecValue.SetDateTime (dateTime);
            break;
            }
        case PRIMITIVETYPE_String:
            if (!EXPECTED_CONDITION (jsonValueType == Json::stringValue))
                return ERROR;
            status = ecValue.SetUtf8CP (jsonValue.asString().c_str(), true);
            break;
        case PRIMITIVETYPE_Boolean:
            if (!EXPECTED_CONDITION (jsonValueType == Json::booleanValue))
                return ERROR;
            status = ecValue.SetBoolean (jsonValue.asBool());
            break;
        case PRIMITIVETYPE_Binary:
            if (!EXPECTED_CONDITION (jsonValueType == Json::stringValue))
                return ERROR;
            // TODO: Do Base64 conversions of binary/text here!!!
            break;
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

    POSTCONDITION (status == SUCCESS, ERROR);
    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 1/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ECJsonCppUtility::ECArrayValueFromJsonValue (IECInstanceR instance, const Json::Value& jsonValue, ArrayECPropertyCR arrayProperty, Utf8StringCR accessString)
    {
    if (!EXPECTED_CONDITION (jsonValue.isArray()))
        return ERROR;

    StatusInt r_status = SUCCESS;
    uint32_t length = jsonValue.size();
    if (length == 0)
        return SUCCESS;

    ECObjectsStatus status = instance.AddArrayElements (accessString.c_str(), length);
    POSTCONDITION (ECObjectsStatus::Success == status, ERROR);

    if (arrayProperty.GetKind() == ARRAYKIND_Primitive)
        {
        PrimitiveType primitiveType = arrayProperty.GetPrimitiveElementType();
        for (uint32_t ii=0; ii<length; ii++)
            {
            ECValue ecPrimitiveValue;
            StatusInt status = ECPrimitiveValueFromJsonValue (ecPrimitiveValue, jsonValue[ii], primitiveType);
            if (SUCCESS != status)
                {
                r_status = status;
                continue;
                }
            ECObjectsStatus ecStatus = instance.SetInternalValue (accessString.c_str(), ecPrimitiveValue, ii);
            if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
                { BeAssert(false); }
            }
        }
    else /* if (arrayProperty.GetKind() == ARRAYKIND_Struct) */
        {
        auto structArrayProperty = arrayProperty.GetAsStructArrayProperty();
        if (nullptr == structArrayProperty)
            return ERROR;

        ECClassCP structType = structArrayProperty->GetStructElementType();
        BeAssert (structType != nullptr);
        for (uint32_t ii=0; ii<length; ii++)
            {
            IECInstancePtr structInstance = structType->GetDefaultStandaloneEnabler()->CreateInstance (0);
            ECInstanceFromJsonValue (*structInstance, jsonValue[ii], *structType, "");
            ECValue ecStructValue;
            ecStructValue.SetStruct (structInstance.get());
            ECObjectsStatus ecStatus = instance.SetInternalValue (accessString.c_str(), ecStructValue, ii);
            if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
                { BeAssert(false); }
            }
        }

    return r_status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 2/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ECJsonCppUtility::ECInstanceFromJsonValue (IECInstanceR instance, const Json::Value& jsonValue)
    {
    return ECInstanceFromJsonValue (instance, jsonValue, instance.GetClass(), "");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 1/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ECJsonCppUtility::ECInstanceFromJsonValue (IECInstanceR instance, const Json::Value& jsonValue, ECClassCR currentClass, Utf8StringCR currentAccessString)
    {
    if (!jsonValue.isObject())
        return ERROR;

    StatusInt status = SUCCESS;
    for (Json::Value::iterator iter = jsonValue.begin(); iter != jsonValue.end(); iter++)
        {
        Json::Value& childJsonValue = *iter;
        if (childJsonValue.isNull())
            continue;

        Utf8CP memberName = iter.memberName();
        if (*memberName == '$')
            continue;

        ECPropertyP ecProperty = currentClass.GetPropertyP (memberName);
        if (!EXPECTED_CONDITION (ecProperty != nullptr))
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
            if (SUCCESS != ECPrimitiveValueFromJsonValue (ecValue, childJsonValue, primitiveType))
                {
                status = ERROR;
                continue;
                }
            ECObjectsStatus ecStatus;
            ecStatus = instance.SetInternalValue (accessString.c_str(), ecValue);
            BeAssert (ecStatus == ECObjectsStatus::Success || ecStatus == ECObjectsStatus::PropertyValueMatchesNoChange);
            continue;
            }
        else if (ecProperty->GetIsStruct())
            {
            StructECPropertyCP structProperty = ecProperty->GetAsStructProperty();
            if (SUCCESS != ECInstanceFromJsonValue (instance, childJsonValue, structProperty->GetType(), accessString))
                status = ERROR;
            continue;
            }
        else if (ecProperty->GetIsArray())
            {
            ArrayECPropertyCP arrayProperty = ecProperty->GetAsArrayProperty();
            if (SUCCESS != ECArrayValueFromJsonValue (instance, childJsonValue, *arrayProperty, accessString))
                {
                status = ERROR;
                continue;
                }
            }
        }

    return status;
    }

//=======================================================================================
//  ECRapidJsonUtility
//=======================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t ECRapidJsonUtility::Int64FromValue (RapidJsonValueCR value, int64_t defaultOnError)
    {
    if (value.IsNull())
        return defaultOnError;

    if (value.IsNumber())
        return value.GetInt64();

    // strings are used in JavaScript because of UInt64 issues
    if (value.IsString())
        {
        int64_t returnValueInt64 = defaultOnError;
        sscanf (value.GetString(), "%" PRId64, &returnValueInt64);
        return returnValueInt64;
        }

    return defaultOnError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ECRapidJsonUtility::ECPrimitiveValueFromJsonValue (ECValueR ecValue, RapidJsonValueCR jsonValue, PrimitiveType primitiveType)
    {
    switch (primitiveType)
        {
        case PRIMITIVETYPE_Point2D:
            if (!jsonValue.IsObject())
                return ERROR;

            if (!jsonValue["x"].IsDouble() || !jsonValue["y"].IsDouble())
                return ERROR;

            DPoint2d point2d;
            point2d.Init (jsonValue["x"].GetDouble(), jsonValue["y"].GetDouble());
            return ecValue.SetPoint2D (point2d);

        case PRIMITIVETYPE_Point3D:
            if (!jsonValue.IsObject())
                return ERROR;

            if (!jsonValue["x"].IsDouble() || !jsonValue["y"].IsDouble() || !jsonValue["z"].IsDouble())
                return ERROR;

            DPoint3d point3d;
            point3d.Init (jsonValue["x"].GetDouble(), jsonValue["y"].GetDouble(), jsonValue["z"].GetDouble());
            return ecValue.SetPoint3D (point3d);

        case PRIMITIVETYPE_Integer: 
            if (!jsonValue.IsInt())
                return ERROR;

            return ecValue.SetInteger (jsonValue.GetInt());

        case PRIMITIVETYPE_Long:
            // Int64 values need to be serialized as strings to allow use in JavaScript
            if (!jsonValue.IsString())
                return ERROR;

            return ecValue.SetLong (Int64FromValue (jsonValue));

        case PRIMITIVETYPE_Double:
            if (!jsonValue.IsNumber())
                return ERROR;

            return ecValue.SetDouble (jsonValue.GetDouble());

        case PRIMITIVETYPE_DateTime:
            {
            if (!jsonValue.IsString())
                return ERROR;

            DateTime dateTime;
            DateTime::FromString (dateTime, jsonValue.GetString());
            return ecValue.SetDateTime (dateTime);
            }

        case PRIMITIVETYPE_String:
            if (!jsonValue.IsString())
                return ERROR;

            return ecValue.SetUtf8CP (jsonValue.GetString(), true);

        case PRIMITIVETYPE_Boolean:
            if (!jsonValue.IsBool())
                return ERROR;

            return ecValue.SetBoolean (jsonValue.GetBool());

        case PRIMITIVETYPE_IGeometry:
            {
            if (!jsonValue.IsObject())
                return ERROR;

            if (jsonValue.IsNull())
                return SUCCESS;

            rapidjson::StringBuffer stringBuffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);
            jsonValue.Accept(writer);

            Utf8String jsonStr = stringBuffer.GetString();

            bvector<IGeometryPtr> geometry;
            if (!BentleyGeometryJson::TryJsonStringToGeometry(jsonStr, geometry))
                return ERROR;

            BeAssert(geometry.size() == 1);
            return ecValue.SetIGeometry(*(geometry[0]));
            }
        case PRIMITIVETYPE_Binary:
        default:
            return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ECRapidJsonUtility::ECArrayValueFromJsonValue (IECInstanceR instance, RapidJsonValueCR jsonValue, ArrayECPropertyCR arrayProperty, Utf8StringCR accessString)
    {
    if (!jsonValue.IsArray())
        return ERROR;

    rapidjson::SizeType size = jsonValue.Size();
    if (0 == size)
        return SUCCESS;

    if (ECObjectsStatus::Success != instance.AddArrayElements (accessString.c_str(), size))
        return ERROR;

    switch (arrayProperty.GetKind())
        {
        case ARRAYKIND_Primitive:
            {
            StatusInt returnStatus = SUCCESS;

            for (rapidjson::SizeType i=0; i<size; i++)
                {
                ECValue primitiveValue;
                if (SUCCESS != ECPrimitiveValueFromJsonValue (primitiveValue, jsonValue[i], arrayProperty.GetPrimitiveElementType()))
                    {
                    returnStatus = ERROR;
                    continue;
                    }

                ECObjectsStatus ecStatus = instance.SetInternalValue (accessString.c_str(), primitiveValue, i);
                if ((ECObjectsStatus::Success != ecStatus) && (ECObjectsStatus::PropertyValueMatchesNoChange != ecStatus))
                    { 
                    BeAssert (false);
                    returnStatus = ERROR;
                    }
                }

            return returnStatus;
            }

        case ARRAYKIND_Struct:
            {
            StructArrayECPropertyCP structArrayProperty = arrayProperty.GetAsStructArrayProperty();
            BeAssert(nullptr != structArrayProperty);

            ECClassCP structType = structArrayProperty->GetStructElementType();
            BeAssert (nullptr != structType);

            for (rapidjson::SizeType i=0; i<size; i++)
                {
                IECInstancePtr structInstance = structType->GetDefaultStandaloneEnabler()->CreateInstance (0);
                ECInstanceFromJsonValue (*structInstance, jsonValue[i], *structType, "");

                ECValue structValue;
                structValue.SetStruct (structInstance.get());
                
                ECObjectsStatus ecStatus = instance.SetInternalValue (accessString.c_str(), structValue, i);
                if ((ECObjectsStatus::Success != ecStatus) && (ECObjectsStatus::PropertyValueMatchesNoChange != ecStatus))
                    { BeAssert(false); }
                }

            return SUCCESS;
            }

        default:
            BeAssert (false);
            return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ECRapidJsonUtility::ECInstanceFromJsonValue (IECInstanceR instance, RapidJsonValueCR jsonValue)
    {
    return ECInstanceFromJsonValue (instance, jsonValue, instance.GetClass(), "");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ECRapidJsonUtility::ECInstanceFromJsonValue (ECN::IECInstanceR instance, RapidJsonValueCR jsonValue, ECClassCR currentClass, Utf8StringCR currentAccessString)
    {
    if (!jsonValue.IsObject())
        return ERROR;
    
    StatusInt status = SUCCESS;
    for (rapidjson::Value::ConstMemberIterator it = jsonValue.MemberBegin(); it != jsonValue.MemberEnd(); ++it)
        {
        if (it->value.IsNull())
            continue;

        if ('$' == it->name.GetString()[0])
            continue;

        Utf8String propertyName (it->name.GetString());
        ECPropertyP propertyP = currentClass.GetPropertyP (propertyName.c_str());
        if (nullptr == propertyP)
            {
            status = ERROR;
            continue;
            }

        Utf8String accessString = (0 == currentAccessString[0]) ? propertyName : currentAccessString + "." + propertyName;
        if (propertyP->GetIsPrimitive())
            {
            ECValue ecValue;
            if (SUCCESS != ECPrimitiveValueFromJsonValue (ecValue, it->value, propertyP->GetAsPrimitiveProperty()->GetType()))
                {
                status = ERROR;
                continue;
                }

            ECObjectsStatus ecStatus = instance.SetInternalValue (accessString.c_str(), ecValue);
            BeAssert ((ECObjectsStatus::Success == ecStatus) || (ECObjectsStatus::PropertyValueMatchesNoChange == ecStatus)); (void) ecStatus;
            }
        else if (propertyP->GetIsStruct())
            {
            if (SUCCESS != ECInstanceFromJsonValue (instance, it->value, propertyP->GetAsStructProperty()->GetType(), accessString))
                status = ERROR;
            }
        else if (propertyP->GetIsArray())
            {
            if (SUCCESS != ECArrayValueFromJsonValue (instance, it->value, *propertyP->GetAsArrayProperty(), accessString))
                status = ERROR;
            }
        }

    return status;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
