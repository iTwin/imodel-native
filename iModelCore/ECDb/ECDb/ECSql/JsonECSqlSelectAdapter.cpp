/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/JsonECSqlSelectAdapter.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECObjects/ECJsonUtilities.h>
#include <GeomSerialization/GeomSerializationApi.h>
#include <Bentley/Base64Utilities.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 07/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool JsonECSqlSelectAdapter::GetRowInstance(JsonValueR json) const
    {
    if (!m_ecsqlStatement.IsPrepared())
        {
        LOG.error("ECSqlStatement passed to JsonECSqlSelectAdapter is not prepared yet.");
        return false;
        }

    if (m_ecsqlStatement.GetColumnCount() == 0)
        {
        LOG.errorv("ECSqlStatement '%s' passed to JsonECSqlSelectAdapter has no SELECT clause.", m_ecsqlStatement.GetECSql());
        return false;
        }

    // Pick the first column's class to get the instance
    IECSqlValue const& ecsqlValue = m_ecsqlStatement.GetValue(0);
    ECClassCR rootClass = ecsqlValue.GetColumnInfo().GetRootClass();
    return GetRowInstance(json, rootClass.GetId());
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 03/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool JsonECSqlSelectAdapter::GetRowInstance(JsonValueR json, ECClassId classId) const
    {
    if (!m_ecsqlStatement.IsPrepared())
        {
        LOG.error("ECSqlStatement passed to JsonECSqlSelectAdapter is not prepared yet.");
        return false;
        }

    if (m_ecsqlStatement.GetColumnCount() == 0)
        {
        LOG.errorv("ECSqlStatement '%s' passed to JsonECSqlSelectAdapter has no SELECT clause.", m_ecsqlStatement.GetECSql());
        return false;
        }

    json = Json::Value(Json::objectValue);
    ECClassCP foundClass = nullptr;
    const int count = m_ecsqlStatement.GetColumnCount();
    for (int columnIndex = 0; columnIndex < count; columnIndex++)
        {
        IECSqlValue const& ecsqlValue = m_ecsqlStatement.GetValue(columnIndex);
        ECSqlColumnInfo const& columnInfo = ecsqlValue.GetColumnInfo();
        ECClassCR rootClass = columnInfo.GetRootClass();
        if (columnInfo.IsGeneratedProperty() || rootClass.GetId() != classId)
            continue;

        foundClass = &rootClass;
        if (SUCCESS != JsonFromCell(json, ecsqlValue))
            return false;
        }

    if (foundClass == nullptr)
        {
        LOG.errorv("No properties of ECClass with Id '%s' found in ECSqlStatement '%s' passed to JsonECSqlSelectAdapter.", classId.ToString().c_str(), m_ecsqlStatement.GetECSql());
        return false;
        }

    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECSqlSelectAdapter::GetRow(JsonValueR json) const
    {
    json = Json::Value(Json::objectValue);

    const int count = m_ecsqlStatement.GetColumnCount();
    for (int columnIndex = 0; columnIndex < count; columnIndex++)
        {
        IECSqlValue const& ecsqlValue = m_ecsqlStatement.GetValue(columnIndex);
        if (SUCCESS != JsonFromCell(json, ecsqlValue))
            return ERROR;
        }

    return SUCCESS;
    }
    
//--------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      09/2017
//+---------------+---------------+---------------+---------------+---------------+------
bool JsonECSqlSelectAdapter::GetRowForImodelJs(JsonValueR jsonResult)
    {
    if (m_formatOptions != FormatOptions::LongsAreIds)
        return false;

    const int colCount = m_ecsqlStatement.GetColumnCount();
    for (int i = 0; i < colCount; ++i)
        {
        IECSqlValue const& value = m_ecsqlStatement.GetValue(i);
        ECSqlColumnInfoCR info = value.GetColumnInfo();
        BeAssert(info.IsValid());

        Utf8String name = info.GetProperty()->GetName();
        ToJsonMemberName(name);

        if (value.IsNull())
            continue; // if the value is null, just skip it

        if (SUCCESS != JsonFromPropertyValue(jsonResult[name], value))
            return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECSqlSelectAdapter::JsonFromCell(JsonValueR parentJsonValue, IECSqlValue const& ecsqlValue) const
    {
    if (ecsqlValue.IsNull())
        return SUCCESS;

    if (ecsqlValue.GetColumnInfo().GetPropertyPath().GetLeafEntry().GetKind() == ECSqlPropertyPath::Entry::Kind::ArrayIndex)
        {
        LOG.errorv("JsonECSqlSelectAdapter does not support array accessors in an ECSQL select clause: %s", m_ecsqlStatement.GetECSql());
        return ERROR;
        }

    BeAssert(ecsqlValue.GetColumnInfo().GetProperty() != nullptr);
    ECSqlSystemPropertyInfo const& sysPropInfo = m_ecsqlStatement.GetECDb()->Schemas().GetReader().GetSystemSchemaHelper().GetSystemPropertyInfo(*ecsqlValue.GetColumnInfo().GetProperty());

    if (!sysPropInfo.IsSystemProperty())
        {
        Utf8String propPath = ecsqlValue.GetColumnInfo().GetPropertyPath().ToString();
        //ToCamelCase(propPath);
        return JsonFromPropertyValue(parentJsonValue[propPath.c_str()], ecsqlValue);
        }

    BeAssert(ecsqlValue.GetId<BeInt64Id>().IsValid());

    if (sysPropInfo.GetType() == ECSqlSystemPropertyInfo::Type::Class)
        {
        switch (sysPropInfo.GetClass())
            {
                case ECSqlSystemPropertyInfo::Class::ECInstanceId:
                    return ECJsonUtilities::IdToJson(parentJsonValue[ECJsonUtilities::json_id()], ecsqlValue.GetId<BeInt64Id>());

                case ECSqlSystemPropertyInfo::Class::ECClassId:
                {
                ECClassCP cls = m_ecsqlStatement.GetECDb()->Schemas().GetClass(ecsqlValue.GetId<ECClassId>());
                if (cls == nullptr)
                    return ERROR;

                ECJsonUtilities::ClassNameToJson(parentJsonValue[ECJsonUtilities::json_className()], *cls);
                return SUCCESS;
                }

                default:
                    BeAssert(false);
                    return ERROR;
            }
        }

    if (sysPropInfo.GetType() == ECSqlSystemPropertyInfo::Type::Relationship)
        {
        switch (sysPropInfo.GetRelationship())
            {
                case ECSqlSystemPropertyInfo::Relationship::SourceECInstanceId:
                    return ECJsonUtilities::IdToJson(parentJsonValue[ECJsonUtilities::json_sourceId()], ecsqlValue.GetId<BeInt64Id>());

                case ECSqlSystemPropertyInfo::Relationship::SourceECClassId:
                {
                ECClassCP cls = m_ecsqlStatement.GetECDb()->Schemas().GetClass(ecsqlValue.GetId<ECClassId>());
                if (cls == nullptr)
                    return ERROR;

                ECJsonUtilities::ClassNameToJson(parentJsonValue[ECJsonUtilities::json_sourceClassName()], *cls);
                return SUCCESS;
                }

                case ECSqlSystemPropertyInfo::Relationship::TargetECInstanceId:
                    return ECJsonUtilities::IdToJson(parentJsonValue[ECJsonUtilities::json_targetId()], ecsqlValue.GetId<BeInt64Id>());

                case ECSqlSystemPropertyInfo::Relationship::TargetECClassId:
                {
                ECClassCP cls = m_ecsqlStatement.GetECDb()->Schemas().GetClass(ecsqlValue.GetId<ECClassId>());
                if (cls == nullptr)
                    return ERROR;

                ECJsonUtilities::ClassNameToJson(parentJsonValue[ECJsonUtilities::json_targetClassName()], *cls);
                return SUCCESS;
                }

                default:
                    BeAssert(false);
                    return ERROR;
            }
        }

    BeAssert(false && "Other system properties are not expected to show up as top-level props in an ECSQL select clause");
    return ERROR;
    }
        
//---------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECSqlSelectAdapter::JsonFromPropertyValue(JsonValueR jsonValue, IECSqlValue const& ecsqlValue) const
    {
    BeAssert(!ecsqlValue.IsNull() && "Should have been caught before to avoid that a null JSON value is created");

    ECN::ECPropertyCP prop = ecsqlValue.GetColumnInfo().GetProperty();
    BeAssert(prop != nullptr);

    if (prop->GetIsPrimitive())
        return JsonFromPrimitive(jsonValue, ecsqlValue, prop->GetAsPrimitiveProperty()->GetType());

    if (prop->GetIsStruct())
        return JsonFromStruct(jsonValue, ecsqlValue);

    if (prop->GetIsNavigation())
        return JsonFromNavigation(jsonValue, ecsqlValue);

    if (prop->GetIsPrimitiveArray())
        return JsonFromPrimitiveArray(jsonValue, ecsqlValue, prop->GetAsPrimitiveArrayProperty()->GetPrimitiveElementType());

    if (prop->GetIsStructArray())
        return JsonFromStructArray(jsonValue, ecsqlValue);

    BeAssert(false && "Unhandled ECProperty type. Adjust the code for this new ECProperty type");
    return ERROR;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECSqlSelectAdapter::JsonFromPrimitiveArray(JsonValueR jsonValue, IECSqlValue const& ecsqlValue, PrimitiveType arrayElementType) const
    {
    int i = 0;
    jsonValue = Json::Value(Json::arrayValue);
    for (IECSqlValue const& arrayElementValue : ecsqlValue.GetArrayIterable())
        {
        if (arrayElementValue.IsNull())
            continue;

        if (SUCCESS != JsonFromPrimitive(jsonValue[i], arrayElementValue, arrayElementType))
            return ERROR;

        i++;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECSqlSelectAdapter::JsonFromStructArray(JsonValueR jsonValue, IECSqlValue const& ecsqlValue) const
    {
    int i = 0;
    Json::Value temp(Json::arrayValue);
    for (IECSqlValue const& arrayElementValue : ecsqlValue.GetArrayIterable())
        {
        if (arrayElementValue.IsNull())
            continue;

        if (SUCCESS != JsonFromStruct(temp[i], arrayElementValue))
            return ERROR;

        i++;
        }

    jsonValue = temp;
    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle               06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECSqlSelectAdapter::JsonFromNavigation(JsonValueR jsonValue, IECSqlValue const& ecsqlValue) const
    {
    IECSqlValue const& navIdVal = ecsqlValue[ECDBSYS_PROP_NavPropId];
    if (navIdVal.IsNull())
        return SUCCESS;

    jsonValue = Json::Value(Json::objectValue);
    if (SUCCESS != ECJsonUtilities::IdToJson(jsonValue[ECJsonUtilities::json_navId()], navIdVal.GetId<ECInstanceId>()))
        return ERROR;

    IECSqlValue const& relClassIdVal = ecsqlValue[ECDBSYS_PROP_NavPropRelECClassId];
    if (relClassIdVal.IsNull())
        return SUCCESS;

    ECClassCP relClass = m_ecsqlStatement.GetECDb()->Schemas().GetClass(relClassIdVal.GetId<ECClassId>());
    if (relClass == nullptr)
        return ERROR;

    ECJsonUtilities::ClassNameToJson(jsonValue[ECJsonUtilities::json_navRelClassName()], *relClass);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECSqlSelectAdapter::JsonFromStruct(JsonValueR jsonValue, IECSqlValue const& ecsqlValue) const
    {
    for (IECSqlValue const& structMemberValue : ecsqlValue.GetStructIterable())
        {
        if (structMemberValue.IsNull())
            continue;

        ECPropertyCP memberProp = structMemberValue.GetColumnInfo().GetProperty();
        BeAssert(memberProp != nullptr);
        Utf8String memberPropName(memberProp->GetName());
        //ToCamelCase(memberPropName);
        if (SUCCESS != JsonFromPropertyValue(jsonValue[memberPropName.c_str()], structMemberValue))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECSqlSelectAdapter::JsonFromPrimitive(JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECN::PrimitiveType primType) const
    {
    BeAssert(!ecsqlValue.IsNull());
    switch (primType)
        {
            case PRIMITIVETYPE_Binary:
            {
            int size = -1;
            Byte const* data = (Byte const*) ecsqlValue.GetBlob(&size);
            Utf8String base64Str;
            Base64Utilities::Encode(base64Str, data, size);
            jsonValue = base64Str;
            return SUCCESS;
            }
            case PRIMITIVETYPE_Boolean:
            {
            jsonValue = ecsqlValue.GetBoolean();
            return SUCCESS;
            }

            case PRIMITIVETYPE_DateTime:
            {
            ECJsonUtilities::DateTimeToJson(jsonValue, ecsqlValue.GetDateTime());
            return SUCCESS;
            }
            case PRIMITIVETYPE_Double:
            {
            jsonValue = ecsqlValue.GetDouble();
            return SUCCESS;
            }
            case PRIMITIVETYPE_Integer:
            {
            jsonValue = ecsqlValue.GetInt();
            return SUCCESS;
            }
            case PRIMITIVETYPE_Long:
            {
            if (m_formatOptions == FormatOptions::LongsAreIds)
                jsonValue = ecsqlValue.GetId<BeInt64Id>().ToHexStr();
            else
                jsonValue = BeJsonUtilities::StringValueFromInt64(ecsqlValue.GetInt64()); // Javascript has issues with holding Int64 values!!!

            return SUCCESS;
            }
            case PRIMITIVETYPE_Point2d:
                return ECJsonUtilities::Point2dToJson(jsonValue, ecsqlValue.GetPoint2d());

            case PRIMITIVETYPE_Point3d:
                return ECJsonUtilities::Point3dToJson(jsonValue, ecsqlValue.GetPoint3d());

            case PRIMITIVETYPE_String:
            {
            jsonValue = ecsqlValue.GetText();
            return SUCCESS;
            }

            case PRIMITIVETYPE_IGeometry:
            {
            IGeometryPtr geom = ecsqlValue.GetGeometry();
            if (geom == nullptr)
                return ERROR;

            return ECJsonUtilities::IGeometryToJson(jsonValue, *geom);
            }
            default:
                BeAssert(false && "Unknown type");
                return ERROR;
        }
    }


END_BENTLEY_SQLITE_EC_NAMESPACE

