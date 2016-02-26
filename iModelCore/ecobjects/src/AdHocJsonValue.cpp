/*--------------------------------------------------------------------------------------+
|
|     $Source: src/AdHocJsonValue.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <json/json.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
BentleyStatus AdHocJsonValue::FromString(Utf8CP jsonStr)
    {
    if (Utf8String::IsNullOrEmpty(jsonStr))
        {
        Clear();
        return SUCCESS;
        }

    if (!Json::Reader::Parse(jsonStr, m_value))
        {
        BeAssert(false && "Expected user properties to be in JSON format");
        return ERROR;
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
Utf8String AdHocJsonValue::ToString()
    {
    if (IsEmpty())
        return "";

    return Json::FastWriter::ToString(m_value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
Json::Value const* AdHocJsonValue::GetMetaData(Utf8CP name, Utf8CP metaDataName) const
    {
    Utf8String metaDataKey = Utf8PrintfString("%s%s", META_DATA_KEY_PREFIX, name);

    if (!m_value.isMember(metaDataKey.c_str()))
        return nullptr;

    Json::Value const& entry = m_value[metaDataKey.c_str()];
    if (!entry.isObject() || !entry.isMember(metaDataName) || entry[metaDataName].isNull())
        return nullptr;

    return &entry[metaDataName];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
void AdHocJsonValue::SetType(Utf8CP name, ECN::PrimitiveType primitiveType)
    {
    SetMetaData<Utf8CP>(name, TYPE_FIELD_NAME, ECXml::GetPrimitiveTypeName(primitiveType));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
void AdHocJsonValue::SetValueBoolean(Utf8CP name, bool value)
    {
    m_value[name] = value;
    SetType(name, PRIMITIVETYPE_Boolean);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
void AdHocJsonValue::SetValueInt(Utf8CP name, int32_t value, Utf8CP units /*= nullptr*/)
    {
    m_value[name] = value;
    SetType(name, PRIMITIVETYPE_Integer);
    if (!Utf8String::IsNullOrEmpty(units))
        SetUnits(name, units);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
void AdHocJsonValue::SetValueInt64(Utf8CP name, int64_t value)
    {
    m_value[name] = value;
    SetType(name, PRIMITIVETYPE_Long);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
void AdHocJsonValue::SetValueDouble(Utf8CP name, double value, Utf8CP units /*= nullptr*/)
    {
    m_value[name] = value;
    SetType(name, PRIMITIVETYPE_Double);
    if (!Utf8String::IsNullOrEmpty(units))
        SetUnits(name, units);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
void AdHocJsonValue::SetValueText(Utf8CP name, Utf8CP value)
    {
    m_value[name] = value;
    SetType(name, PRIMITIVETYPE_String);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
void AdHocJsonValue::SetValuePoint2D(Utf8CP name, DPoint2dCR value)
    {
    m_value[name] = Json::objectValue;
    m_value[name]["x"] = value.x;
    m_value[name]["y"] = value.y;
    SetType(name, PRIMITIVETYPE_Point2D);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
void AdHocJsonValue::SetValuePoint3D(Utf8CP name, DPoint3dCR value)
    {
    m_value[name] = Json::objectValue;
    m_value[name]["x"] = value.x;
    m_value[name]["y"] = value.y;
    m_value[name]["z"] = value.z;
    SetType(name, PRIMITIVETYPE_Point3D);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
void AdHocJsonValue::SetValueDateTime(Utf8CP name, DateTimeCR value)
    {
    Json::Value jsonValue = value.ToUtf8String();
    m_value[name] = jsonValue;
    SetType(name, PRIMITIVETYPE_DateTime);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
bool AdHocJsonValue::SetValueEC(Utf8CP name, ECN::ECValueCR value)
    {
    if (!value.IsPrimitive())
        {
        BeAssert(false && "Can only set primitive values");
        return false;
        }

    if (value.IsNull())
        {
        SetValueNull(name);
        return true;
        }

    PrimitiveType primitiveType = value.GetPrimitiveType();
    switch (primitiveType)
        {
        case PRIMITIVETYPE_Boolean:
            SetValueBoolean(name, value.GetBoolean());
            return true;
        case PRIMITIVETYPE_DateTime:
            SetValueDateTime(name, value.GetDateTime());
            return true;
        case PRIMITIVETYPE_Double:
            SetValueDouble(name, value.GetDouble());
            return true;
        case PRIMITIVETYPE_Integer:
            SetValueInt(name, value.GetInteger());
            return true;
        case PRIMITIVETYPE_Long:
            SetValueInt64(name, value.GetLong());
            return true;
        case PRIMITIVETYPE_Point2D:
            SetValuePoint2D(name, value.GetPoint2D());
            return true;
        case PRIMITIVETYPE_Point3D:
            SetValuePoint3D(name, value.GetPoint3D());
            return true;
        case PRIMITIVETYPE_String:
            SetValueText(name, value.GetUtf8CP());
            return true;
        default:
            BeAssert(false && "Cannot handle type");
            return false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
void AdHocJsonValue::SetValueNull(Utf8CP name)
    {
    m_value[name] = Json::nullValue;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
void AdHocJsonValue::RemoveValue(Utf8CP name)
    {
    if (m_value.isMember(name))
        m_value.removeMember(name);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
Json::Value const* AdHocJsonValue::GetValue(Utf8CP name) const
    {
    if (m_value.isMember(name) && !m_value[name].isNull())
        return &m_value[name];

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
bool AdHocJsonValue::GetValueBoolean(Utf8CP name) const
    {
    Json::Value const* value = GetValue(name);
    return value ? value->asBool() : false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
Utf8CP AdHocJsonValue::GetValueText(Utf8CP name) const
    {
    Json::Value const* value = GetValue(name);
    return value ? value->asCString() : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
double AdHocJsonValue::GetValueDouble(Utf8CP name) const
    {
    Json::Value const* value = GetValue(name);
    return value ? value->asDouble() : 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
int AdHocJsonValue::GetValueInt(Utf8CP name) const
    {
    Json::Value const* value = GetValue(name);
    return value ? value->asInt() : 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
int64_t AdHocJsonValue::GetValueInt64(Utf8CP name) const
    {
    Json::Value const* value = GetValue(name);
    return value ? value->asInt64() : 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
DPoint2d AdHocJsonValue::GetValuePoint2D(Utf8CP name) const
    {
    Json::Value const* value = GetValue(name);
    return value ? DPoint2d::From((*value)["x"].asDouble(), (*value)["y"].asDouble()) : DPoint2d::From(0.0, 0.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
DPoint3d AdHocJsonValue::GetValuePoint3D(Utf8CP name) const
    {
    Json::Value const* value = GetValue(name);
    return value ? DPoint3d::From((*value)["x"].asDouble(), (*value)["y"].asDouble(), (*value)["z"].asDouble()) : DPoint3d::From(0.0, 0.0, 0.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
DateTime AdHocJsonValue::GetValueDateTime(Utf8CP name) const
    {
    Json::Value const* jsonValue = GetValue(name);

    DateTime value;
    if (jsonValue)
        DateTime::FromString(value, jsonValue->asCString());

    return value;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
ECValue AdHocJsonValue::GetValueEC(Utf8CP name) const
    {
    Json::Value const* jsonValue = GetValue(name);
    if (!jsonValue)
        return ECValue();

    PrimitiveType primitiveType;
    if (!GetType(primitiveType, name))
        {
        BeAssert(false && "Could not find the primitive type of the value");
        return ECValue();
        }

    switch (primitiveType)
        {
        case PRIMITIVETYPE_Boolean:
            return ECValue(GetValueBoolean(name));
        case PRIMITIVETYPE_DateTime:
            return ECValue(GetValueDateTime(name));
        case PRIMITIVETYPE_Double:
            return ECValue(GetValueDouble(name));
        case PRIMITIVETYPE_Integer:
            return ECValue(GetValueInt(name));
        case PRIMITIVETYPE_Long:
            return ECValue(GetValueInt64(name));
        case PRIMITIVETYPE_Point2D:
            return ECValue(GetValuePoint2D(name));
        case PRIMITIVETYPE_Point3D:
            return ECValue(GetValuePoint3D(name));
        case PRIMITIVETYPE_String:
            return ECValue(GetValueText(name));
        default:
            BeAssert(false);
            return ECValue();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
bool AdHocJsonValue::GetType(PrimitiveType& primitiveType, Utf8CP name) const
    {
    Json::Value const* jsonValue = GetMetaData(name, TYPE_FIELD_NAME);
    if (!jsonValue)
        return false;

    return (ECObjectsStatus::Success == ECXml::ParsePrimitiveType(primitiveType, jsonValue->asString()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
bool AdHocJsonValue::GetExtendedType(Utf8StringR extendedTypeName, Utf8CP name) const
    {
    Json::Value const* jsonValue = GetMetaData(name, EXTENDEDTYPE_FIELD_NAME);
    if (!jsonValue)
        return false;

    extendedTypeName = jsonValue->asString();
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
bool AdHocJsonValue::GetUnits(Utf8StringR unitsStr, Utf8CP name) const
    {
    Json::Value const* jsonValue = GetMetaData(name, UNITS_FIELD_NAME);
    if (!jsonValue)
        return false;

    unitsStr = jsonValue->asString();
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
bool AdHocJsonValue::GetCategory(Utf8StringR categoryLabel, Utf8CP name) const
    {
    Json::Value const* jsonValue = GetMetaData(name, CATEGORY_FIELD_NAME);
    if (!jsonValue)
        return false;

    categoryLabel = jsonValue->asString();
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
bool AdHocJsonValue::GetHidden(bool& isHidden, Utf8CP name) const
    {
    Json::Value const* jsonValue = GetMetaData(name, HIDDEN_FIELD_NAME);
    if (!jsonValue)
        return false;

    isHidden = jsonValue->asBool();
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
bool AdHocJsonValue::GetPriority(int& priority, Utf8CP name) const
    {
    Json::Value const* jsonValue = GetMetaData(name, PRIORITY_FIELD_NAME);
    if (!jsonValue)
        return false;

    priority = jsonValue->asInt();
    return true;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
bool AdHocJsonValue::GetReadOnly(bool& isReadOnly, Utf8CP name) const
    {
    Json::Value const* jsonValue = GetMetaData(name, READONLY_FIELD_NAME);
    if (!jsonValue)
        return false;

    isReadOnly = jsonValue->asBool();
    return true;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
