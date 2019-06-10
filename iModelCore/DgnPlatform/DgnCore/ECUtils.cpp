/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/ECUtils.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
static std::pair<ECN::PrimitiveType, BentleyStatus> jsonTypeToEcPrimitiveType(Json::Value const& jsonValue)
    {
    if (jsonValue.isBool())
        return std::make_pair(ECN::PRIMITIVETYPE_Boolean, BSISUCCESS);
    if (jsonValue.isIntegral())
        return std::make_pair(ECN::PRIMITIVETYPE_Long, BSISUCCESS);
    if (jsonValue.isDouble())
        return std::make_pair(ECN::PRIMITIVETYPE_Double, BSISUCCESS);
    if (jsonValue.isString())
        return std::make_pair(ECN::PRIMITIVETYPE_String, BSISUCCESS);
    
    return std::make_pair(PRIMITIVETYPE_Binary, BSIERROR);
    }e
+---------------+---------------+---------------+---------------+---------------+------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ECUtils::StoreECValueAsJson(Json::Value& json, ECN::ECValueCR ecv)
    {
    if (ECN::VALUEKIND_Primitive != ecv.GetKind())
        return DgnDbStatus::BadArg;
    json["Type"] = ECUtils::ECPrimtiveTypeToString(ecv.GetPrimitiveType());
    json["Value"] = ecv.ToString().c_str();
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ECUtils::LoadECValueFromJson(ECN::ECValueR ecv, Json::Value const& json)
    {
    if (!json.isMember("Value") || json["Value"].isNull())
        return DgnDbStatus::BadArg;
    ecv = ECValue(json["Value"].asCString());
    return ecv.ConvertToPrimitiveType(ECUtils::ECPrimtiveTypeFromString(json["Type"].asCString()))? DgnDbStatus::Success: DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ECUtils::ECPrimtiveTypeToString(ECN::PrimitiveType pt)
    {
    switch(pt)
        {
        case ECN::PRIMITIVETYPE_Boolean:    return "boolean";
        case ECN::PRIMITIVETYPE_DateTime:   return "datetime";
        case ECN::PRIMITIVETYPE_Double:     return "double";
        case ECN::PRIMITIVETYPE_IGeometry:  return "igeometry";
        case ECN::PRIMITIVETYPE_Integer:    return "integer";
        case ECN::PRIMITIVETYPE_Long:       return "long";
        case ECN::PRIMITIVETYPE_String:     return "string";
        case ECN::PRIMITIVETYPE_Point2d:    return "point2d";
        case ECN::PRIMITIVETYPE_Point3d:    return "point3d";
        }
    
    return "binary";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::PrimitiveType ECUtils::ECPrimtiveTypeFromString(Utf8CP str)
    {
    switch(*str)
        {
        case 'b': 
            {
            switch (str[1])
                {
                case 'o': return ECN::PRIMITIVETYPE_Boolean;
                case 'i': return ECN::PRIMITIVETYPE_Binary;
                }
            break;
            }
            
        case 'd': 
            {
            switch (str[1])
                {
                case 'o': return ECN::PRIMITIVETYPE_Double;
                case 'a': return ECN::PRIMITIVETYPE_DateTime;
                }
            break;
            }

        case 'p':
            return (0==BeStringUtilities::Stricmp(str, "dpoint2d"))? ECN::PRIMITIVETYPE_Point2d: ECN::PRIMITIVETYPE_Point3d;
            
        case 'i': 
            {
            switch (str[1])
                {
                case 'n': return ECN::PRIMITIVETYPE_Integer;
                case 'g': return ECN::PRIMITIVETYPE_IGeometry;
                }
            break;
            }

        case 'l': 
            return ECN::PRIMITIVETYPE_Long;

        case 's': 
            return ECN::PRIMITIVETYPE_String;
        }
   
    BeDataAssert(false);
    return ECN::PRIMITIVETYPE_Binary;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECUtils::ConvertECValueToJson(Json::Value& jv, ECN::ECValue const& v)
    {
    if (!v.IsPrimitive())
        return BSIERROR;

    if (v.IsNull())
        {
        jv = Json::nullValue;
        return BSISUCCESS;
        }

    switch(v.GetPrimitiveType())
        {
        case ECN::PRIMITIVETYPE_Boolean:    jv = v.GetBoolean(); break;
        case ECN::PRIMITIVETYPE_Double:     jv = v.GetDouble(); break;
        case ECN::PRIMITIVETYPE_Integer:    jv = v.GetInteger(); break;
        case ECN::PRIMITIVETYPE_Long:       jv = Json::Value(v.GetLong()); break;
        case ECN::PRIMITIVETYPE_String:     jv = v.GetUtf8CP(); break;
        
        case ECN::PRIMITIVETYPE_Point2d:    JsonUtils::DPoint2dToJson(jv, v.GetPoint2d()); break;
        case ECN::PRIMITIVETYPE_Point3d:    JsonUtils::DPoint3dToJson(jv, v.GetPoint3d()); break;

        case ECN::PRIMITIVETYPE_DateTime:   jv = v.GetDateTime().ToString().c_str(); break;

        /* WIP_ECUTILS
        case ECN::PRIMITIVETYPE_IGeometry:  jv = ...
        */

        default:
            return BSIERROR;
        }
    
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECUtils::ConvertJsonToECValue(ECN::ECValue& v, Json::Value const& jsonValue, ECN::PrimitiveType typeRequired)
    {
    if (jsonValue.isBool())
        v = ECN::ECValue(jsonValue.asBool());
    else if (jsonValue.isIntegral())
        v = ECN::ECValue(jsonValue.asInt64());
    else if (jsonValue.isDouble())
        v = ECN::ECValue(jsonValue.asDouble());
    else if (jsonValue.isString())
        {
        if (ECN::PRIMITIVETYPE_DateTime == typeRequired)
            {
            DateTime dt;
            if (BSISUCCESS == DateTime::FromString(dt, jsonValue.asCString()))
                v = ECN::ECValue(dt);
            else
                v.SetIsNull(true);
            }
        else if (ECN::PRIMITIVETYPE_Binary == typeRequired)
            {
            // *** TBD: need extended type to recognize GUID
            // *** TBD: buffer = base64-decode(jsonValue.asCString());
            // *** TBD: v = ECN::ECValue(buffer, buffersize);
            v.SetIsNull(true);
            }
        else
            {
            v = ECN::ECValue(jsonValue.asCString());
            }
        }
    else if (jsonValue.isObject())
        {
        if (ECN::PRIMITIVETYPE_Point3d == typeRequired)
            {
            v = ECN::ECValue(DPoint3d::From(jsonValue["x"].asDouble(), jsonValue["y"].asDouble(), jsonValue["z"].asDouble()));
            }
        else if (ECN::PRIMITIVETYPE_Point2d == typeRequired)
            {
            v = ECN::ECValue(DPoint2d::From(jsonValue["x"].asDouble(), jsonValue["y"].asDouble()));
            }
        else
            {
            v.SetIsNull(true);
            }
        }
    else if (jsonValue.isArray())
        {
        if (ECN::PRIMITIVETYPE_Point3d == typeRequired)
            {
            v = ECN::ECValue(DPoint3d::From(jsonValue[0].asDouble(), jsonValue[1].asDouble(), jsonValue[2].asDouble()));
            }
        else if (ECN::PRIMITIVETYPE_Point2d == typeRequired)
            {
            v = ECN::ECValue(DPoint2d::From(jsonValue[0].asDouble(), jsonValue[1].asDouble()));
            }
        else
            {
            v.SetIsNull(true);
            }
        }
    else
        {
        v.SetIsNull(true);
        }

    if (!v.IsNull() && !v.ConvertToPrimitiveType(typeRequired))
        v.SetIsNull(true);

    return v.IsNull()? BSIERROR: BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECUtils::ToJsonPropertiesFromECProperties(Json::Value& json, ECN::IECInstanceCR ec, Utf8StringCR props)
    {
    size_t offset = 0;
    Utf8String parm;
    while ((offset = props.GetNextToken (parm, ",", offset)) != Utf8String::npos)
        {
        parm.Trim();
        ECN::ECValue ecv;
        ec.GetValue(ecv, parm.c_str());
        if (ConvertECValueToJson(json[parm.c_str()], ecv) != BSISUCCESS)
            return BSIERROR;
        }
    return BSISUCCESS;
    }

