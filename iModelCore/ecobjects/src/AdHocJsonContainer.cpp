/*--------------------------------------------------------------------------------------+
|
|     $Source: src/AdHocJsonContainer.cpp $
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
BentleyStatus AdHocJsonContainer::FromString(Utf8CP jsonStr)
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
Utf8String AdHocJsonContainer::ToString()
    {
    if (IsEmpty())
        return "";

    return Json::FastWriter::ToString(m_value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
Json::Value const* AdHocJsonPropertyValue::GetAttribute(Utf8CP attributeName) const
    {
    Utf8CP attributeKey = m_attributeKey.c_str();

    if (!m_value.isMember(attributeKey))
        return nullptr;

    Json::Value const& entry = m_value[attributeKey];
    if (!entry.isObject() || !entry.isMember(attributeName) || entry[attributeName].isNull())
        return nullptr;

    return &entry[attributeName];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
void AdHocJsonPropertyValue::SetType(ECN::PrimitiveType primitiveType)
    {
    SetAttribute<Utf8CP>(TYPE_FIELD_NAME, ECXml::GetPrimitiveTypeName(primitiveType));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
void AdHocJsonPropertyValue::SetValueBoolean(bool value)
    {
    m_value[m_name.c_str()] = value;
    SetType(PRIMITIVETYPE_Boolean);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
void AdHocJsonPropertyValue::SetValueInt(int32_t value, Utf8CP units /*= nullptr*/)
    {
    m_value[m_name.c_str()] = value;
    SetType(PRIMITIVETYPE_Integer);
    if (!Utf8String::IsNullOrEmpty(units))
        SetUnits(units);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
void AdHocJsonPropertyValue::SetValueInt64(int64_t value)
    {
    m_value[m_name.c_str()] = value;
    SetType(PRIMITIVETYPE_Long);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
void AdHocJsonPropertyValue::SetValueDouble(double value, Utf8CP units /*= nullptr*/)
    {
    m_value[m_name.c_str()] = value;
    SetType(PRIMITIVETYPE_Double);
    if (!Utf8String::IsNullOrEmpty(units))
        SetUnits(units);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
void AdHocJsonPropertyValue::SetValueText(Utf8CP value)
    {
    m_value[m_name.c_str()] = value;
    SetType(PRIMITIVETYPE_String);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
void AdHocJsonPropertyValue::SetValuePoint2D(DPoint2dCR value)
    {
    Utf8CP name = m_name.c_str();
    m_value[name] = Json::objectValue;
    m_value[name]["x"] = value.x;
    m_value[name]["y"] = value.y;
    SetType(PRIMITIVETYPE_Point2D);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
void AdHocJsonPropertyValue::SetValuePoint3D(DPoint3dCR value)
    {
    Utf8CP name = m_name.c_str();
    m_value[name] = Json::objectValue;
    m_value[name]["x"] = value.x;
    m_value[name]["y"] = value.y;
    m_value[name]["z"] = value.z;
    SetType(PRIMITIVETYPE_Point3D);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
void AdHocJsonPropertyValue::SetValueDateTime(DateTimeCR value)
    {
    Json::Value jsonValue = value.ToUtf8String();
    m_value[m_name.c_str()] = jsonValue;
    SetType(PRIMITIVETYPE_DateTime);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
BentleyStatus AdHocJsonPropertyValue::SetValueEC(ECN::ECValueCR value)
    {
    if (!value.IsPrimitive())
        {
        BeAssert(false && "Can only set primitive values");
        return ERROR;
        }

    if (value.IsNull())
        {
        SetValueNull();
        return SUCCESS;
        }

    PrimitiveType primitiveType = value.GetPrimitiveType();
    switch (primitiveType)
        {
        case PRIMITIVETYPE_Boolean:
            SetValueBoolean(value.GetBoolean());
            break;
        case PRIMITIVETYPE_DateTime:
            SetValueDateTime(value.GetDateTime());
            break;
        case PRIMITIVETYPE_Double:
            SetValueDouble(value.GetDouble());
            break;
        case PRIMITIVETYPE_Integer:
            SetValueInt(value.GetInteger());
            break;
        case PRIMITIVETYPE_Long:
            SetValueInt64(value.GetLong());
            break;
        case PRIMITIVETYPE_Point2D:
            SetValuePoint2D(value.GetPoint2D());
            break;
        case PRIMITIVETYPE_Point3D:
            SetValuePoint3D(value.GetPoint3D());
            break;
        case PRIMITIVETYPE_String:
            SetValueText(value.GetUtf8CP());
            break;
        default:
            BeAssert(false && "Cannot handle type");
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
void AdHocJsonPropertyValue::SetValueNull()
    {
    m_value[m_name.c_str()] = Json::nullValue;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
void AdHocJsonPropertyValue::RemoveValue()
    {
    if (m_value.isMember(m_name.c_str()))
        m_value.removeMember(m_name.c_str());

    if (m_value.isMember(m_attributeKey.c_str()))
        m_value.removeMember(m_attributeKey.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
Json::Value const* AdHocJsonPropertyValue::GetJsonValue() const
    {
    Utf8CP name = m_name.c_str();
    if (m_value.isMember(name) && !m_value[name].isNull())
        return &m_value[name];

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
bool AdHocJsonPropertyValue::GetValueBoolean() const
    {
    Json::Value const* value = GetJsonValue();
    return value ? value->asBool() : false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
Utf8CP AdHocJsonPropertyValue::GetValueText() const
    {
    Json::Value const* value = GetJsonValue();
    return value ? value->asCString() : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
double AdHocJsonPropertyValue::GetValueDouble() const
    {
    Json::Value const* value = GetJsonValue();
    return value ? value->asDouble() : 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
int AdHocJsonPropertyValue::GetValueInt() const
    {
    Json::Value const* value = GetJsonValue();
    return value ? value->asInt() : 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
int64_t AdHocJsonPropertyValue::GetValueInt64() const
    {
    Json::Value const* value = GetJsonValue();
    return value ? value->asInt64() : 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
DPoint2d AdHocJsonPropertyValue::GetValuePoint2D() const
    {
    Json::Value const* value = GetJsonValue();
    return value ? DPoint2d::From((*value)["x"].asDouble(), (*value)["y"].asDouble()) : DPoint2d::From(0.0, 0.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
DPoint3d AdHocJsonPropertyValue::GetValuePoint3D() const
    {
    Json::Value const* value = GetJsonValue();
    return value ? DPoint3d::From((*value)["x"].asDouble(), (*value)["y"].asDouble(), (*value)["z"].asDouble()) : DPoint3d::From(0.0, 0.0, 0.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
DateTime AdHocJsonPropertyValue::GetValueDateTime() const
    {
    Json::Value const* jsonValue = GetJsonValue();

    DateTime value;
    if (jsonValue)
        DateTime::FromString(value, jsonValue->asCString());

    return value;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
ECValue AdHocJsonPropertyValue::GetValueEC() const
    {
    if (IsValueNull())
        return ECValue();

    PrimitiveType primitiveType;
    if (GetStatus::Found != GetType(primitiveType))
        {
        BeAssert(false && "Could not find the primitive type of the value");
        return ECValue();
        }

    switch (primitiveType)
        {
        case PRIMITIVETYPE_Boolean:
            return ECValue(GetValueBoolean());
        case PRIMITIVETYPE_DateTime:
            return ECValue(GetValueDateTime());
        case PRIMITIVETYPE_Double:
            return ECValue(GetValueDouble());
        case PRIMITIVETYPE_Integer:
            return ECValue(GetValueInt());
        case PRIMITIVETYPE_Long:
            return ECValue(GetValueInt64());
        case PRIMITIVETYPE_Point2D:
            return ECValue(GetValuePoint2D());
        case PRIMITIVETYPE_Point3D:
            return ECValue(GetValuePoint3D());
        case PRIMITIVETYPE_String:
            return ECValue(GetValueText());
        default:
            BeAssert(false);
            return ECValue();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
AdHocJsonPropertyValue::GetStatus AdHocJsonPropertyValue::GetType(PrimitiveType& primitiveType) const
    {
    Json::Value const* jsonValue = GetAttribute(TYPE_FIELD_NAME);
    if (!jsonValue)
        return GetStatus::NotFound;

    return (ECObjectsStatus::Success == ECXml::ParsePrimitiveType(primitiveType, jsonValue->asString())) ? GetStatus::Found : GetStatus::NotFound;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
AdHocJsonPropertyValue::GetStatus AdHocJsonPropertyValue::GetExtendedType(Utf8StringR extendedTypeName) const
    {
    Json::Value const* jsonValue = GetAttribute(EXTENDEDTYPE_FIELD_NAME);
    if (!jsonValue)
        return GetStatus::NotFound;

    extendedTypeName = jsonValue->asString();
    return GetStatus::Found;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
AdHocJsonPropertyValue::GetStatus AdHocJsonPropertyValue::GetUnits(Utf8StringR unitsStr) const
    {
    Json::Value const* jsonValue = GetAttribute(UNITS_FIELD_NAME);
    if (!jsonValue)
        return GetStatus::NotFound;

    unitsStr = jsonValue->asString();
    return GetStatus::Found;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
AdHocJsonPropertyValue::GetStatus AdHocJsonPropertyValue::GetCategory(Utf8StringR categoryLabel) const
    {
    Json::Value const* jsonValue = GetAttribute(CATEGORY_FIELD_NAME);
    if (!jsonValue)
        return GetStatus::NotFound;

    categoryLabel = jsonValue->asString();
    return GetStatus::Found;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
AdHocJsonPropertyValue::GetStatus AdHocJsonPropertyValue::GetHidden(bool& isHidden) const
    {
    Json::Value const* jsonValue = GetAttribute(HIDDEN_FIELD_NAME);
    if (!jsonValue)
        return GetStatus::NotFound;

    isHidden = jsonValue->asBool();
    return GetStatus::Found;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
AdHocJsonPropertyValue::GetStatus AdHocJsonPropertyValue::GetPriority(int& priority) const
    {
    Json::Value const* jsonValue = GetAttribute(PRIORITY_FIELD_NAME);
    if (!jsonValue)
        return GetStatus::NotFound;

    priority = jsonValue->asInt();
    return GetStatus::Found;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2016
//---------------------------------------------------------------------------------------
AdHocJsonPropertyValue::GetStatus AdHocJsonPropertyValue::GetReadOnly(bool& isReadOnly) const
    {
    Json::Value const* jsonValue = GetAttribute(READONLY_FIELD_NAME);
    if (!jsonValue)
        return GetStatus::NotFound;

    isReadOnly = jsonValue->asBool();
    return GetStatus::Found;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
