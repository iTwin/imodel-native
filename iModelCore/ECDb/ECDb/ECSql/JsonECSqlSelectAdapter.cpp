/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/JsonECSqlSelectAdapter.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <BeJsonCpp/BeJsonUtilities.h>
#include <GeomSerialization/GeomLibsSerialization.h>
#include <GeomSerialization/GeomLibsJsonSerialization.h>
#include <Bentley/Base64Utilities.h>
#include <ECObjects/ECJsonUtilities.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsIdProperty(Utf8StringCR propertyName)
    {
    return (0 == propertyName.CompareTo(ECDBSYS_PROP_ECInstanceId) || 0 == propertyName.CompareTo(ECDBSYS_PROP_ECClassId) ||
            0 == propertyName.CompareTo(ECDBSYS_PROP_SourceECInstanceId) || 0 == propertyName.CompareTo(ECDBSYS_PROP_TargetECInstanceId) ||
            0 == propertyName.CompareTo(ECDBSYS_PROP_SourceECClassId) || 0 == propertyName.CompareTo(ECDBSYS_PROP_TargetECClassId));
    }

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
    json["$ECClassKey"] = Json::nullValue;

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

    JsonFromClassKey(json["$ECClassKey"], *foundClass);
    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2012
//+---------------+---------------+---------------+---------------+---------------+------
bool JsonECSqlSelectAdapter::GetRow(JsonValueR currentRow) const
    {
    currentRow = Json::Value(Json::arrayValue);

    bmap<ECClassCP, int> classesInSelectClause; // Classes used in the select clause plus their index of their first appearance in the select clause

    int count = m_ecsqlStatement.GetColumnCount();
    for (int columnIndex = 0; columnIndex < count; columnIndex++)
        {
        IECSqlValue const& ecsqlValue = m_ecsqlStatement.GetValue(columnIndex);
        ECClassCR rootClass = ecsqlValue.GetColumnInfo().GetRootClass();

        auto it = classesInSelectClause.find(&rootClass);
        // Setup a new instance (if necessary)
        int instanceIndex = 0;
        if (it == classesInSelectClause.end())
            {
            // Setup a new instance
            instanceIndex = (int) classesInSelectClause.size();
            classesInSelectClause[&rootClass] = instanceIndex;

            Json::Value& currentInstance = currentRow[instanceIndex];
            JsonFromClassKey(currentInstance["$ECClassKey"], rootClass);
            }
        else
            {
            // Update an existing instance
            instanceIndex = it->second;
            }

        // Setup node for the property
        if (SUCCESS != JsonFromCell(currentRow[instanceIndex], ecsqlValue))
            return false;
        }

    return true;
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
        name[0] = (char) tolower(name[0]);

        if (value.IsNull())
            continue; // if the value is null, just skip it

        if (SUCCESS != JsonFromPropertyValue(jsonResult[name], value))
            return false;
        }

    return true;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus JsonECSqlSelectAdapter::JsonFromArray(JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECPropertyCR property) const
    {
    if (!property.GetIsArray())
        return ERROR;

    if (property.GetIsStructArray())
        return JsonFromStructArray(jsonValue, ecsqlValue);

    return JsonFromPrimitiveArray(jsonValue, ecsqlValue, property);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus JsonECSqlSelectAdapter::JsonFromStructArray(JsonValueR jsonValue, IECSqlValue const& ecsqlValue) const
    {
    int ii = 0;
    Json::Value temp(Json::arrayValue);
    for (IECSqlValue const& arrayElementValue : ecsqlValue.GetArrayIterable())
        {
        if (SUCCESS != JsonFromStruct(temp[ii++], arrayElementValue))
            return ERROR;
        }

    jsonValue = temp;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus JsonECSqlSelectAdapter::JsonFromPrimitiveArray(JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECPropertyCR property) const
    {
    int ii = 0;
    jsonValue = Json::Value(Json::arrayValue);
    for (IECSqlValue const& arrayElementValue : ecsqlValue.GetArrayIterable())
        {
        if (SUCCESS != JsonFromPrimitive(jsonValue[ii++], arrayElementValue, property))
            return ERROR;
        }

    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle               06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECSqlSelectAdapter::JsonFromNavigation(JsonValueR jsonValue, IECSqlValue const& ecsqlValue) const
    {
    jsonValue = Json::Value(Json::objectValue);

    if (SUCCESS != JsonFromInstanceId(jsonValue[JSON_NAVIGATION_ID_KEY], ecsqlValue[ECDBSYS_PROP_NavPropId]))
        return ERROR;

    IECSqlValue const& relClassIdVal = ecsqlValue[ECDBSYS_PROP_NavPropRelECClassId];
    ECClassCP relClass = m_ecsqlStatement.GetECDb()->Schemas().GetClass(relClassIdVal.GetId<ECClassId>());
    if (relClass == nullptr)
        return ERROR;

    if (SUCCESS != JsonFromPropertyValue(jsonValue[JSON_NAVIGATION_RELECCLASSID_KEY], relClassIdVal))
        return ERROR;

    JsonFromClassKey(jsonValue["$ECRelClassKey"], *relClass);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus JsonECSqlSelectAdapter::JsonFromStruct(JsonValueR jsonValue, IECSqlValue const& ecsqlValue) const
    {
    jsonValue = Json::Value(Json::objectValue);
    for (IECSqlValue const& structMemberValue : ecsqlValue.GetStructIterable())
        {
        ECPropertyCP ecLeafProperty = structMemberValue.GetColumnInfo().GetProperty();
        BeAssert(ecLeafProperty != nullptr && "TODO: Adjust code as ColumnInfo::GetProperty can be null.");
        if (SUCCESS != JsonFromPropertyValue(jsonValue[ecLeafProperty->GetName().c_str()], structMemberValue))
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus JsonECSqlSelectAdapter::JsonFromPrimitive(JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECPropertyCR ecProperty) const
    {
    ECTypeDescriptor const& typeDescriptor = ecsqlValue.GetColumnInfo().GetDataType();
    BeAssert(typeDescriptor.IsPrimitive());
    switch (typeDescriptor.GetPrimitiveType())
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
            jsonValue = ecsqlValue.GetDateTime().ToString();
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
            IGeometryPtr geometry = ecsqlValue.GetGeometry();
            if (geometry == nullptr)
                {
                BeAssert(false && "Could not read Common Geometry");
                return ERROR;
                }

            return BentleyGeometryJson::TryGeometryToJsonValue(jsonValue, *geometry, false) ? SUCCESS : ERROR;
            }

            default:
                BeAssert(false && "Unknown type");
                return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonECSqlSelectAdapter::JsonFromClassKey(JsonValueR jsonValue, ECClassCR ecClass) const { jsonValue = GetClassKey(ecClass); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus JsonECSqlSelectAdapter::JsonFromInstanceId(JsonValueR jsonValue, IECSqlValue const& ecsqlValue) const
    {
    ECInstanceId ecInstanceId = ecsqlValue.GetId<ECInstanceId>();
    //TODO: If ECInstanceId is invalid, shouldn't something else than 0 be returned?
    const uint64_t ecInstanceIdVal = ecInstanceId.IsValid() ? ecInstanceId.GetValue() : INT64_C(0);
    jsonValue = BeJsonUtilities::StringValueFromInt64(ecInstanceIdVal); // Javascript has issues with holding Int64 values!!!
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus JsonECSqlSelectAdapter::JsonFromPropertyValue(JsonValueR jsonValue, IECSqlValue const& ecsqlValue) const
    {
    if (ecsqlValue.IsNull())
        {
        jsonValue = Json::nullValue;
        return SUCCESS;
        }

    ECN::ECPropertyCP ecProperty = ecsqlValue.GetColumnInfo().GetProperty();
    BeAssert(ecProperty != nullptr && "According to the ECSqlStatement API, this can happen only for array readers, where this method should never have been called");
    if (ecProperty->GetIsPrimitive())
        return JsonFromPrimitive(jsonValue, ecsqlValue, *ecProperty);
    if (ecProperty->GetIsStruct())
        return JsonFromStruct(jsonValue, ecsqlValue);
    if (ecProperty->GetIsNavigation())
        return JsonFromNavigation(jsonValue, ecsqlValue);
    if (ecProperty->GetIsArray())
        return JsonFromArray(jsonValue, ecsqlValue, *ecProperty);

    BeAssert(false && "Unhandled ECProperty type. Adjust the code for this new ECProperty type");
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus JsonECSqlSelectAdapter::JsonFromCell(JsonValueR jsonValue, IECSqlValue const& ecsqlValue) const
    {
    // Create an empty JSON cell (create hierarchy in the case of structs)
    ECSqlColumnInfoCR columnInfo = ecsqlValue.GetColumnInfo();
    ECSqlPropertyPathCR propertyPath = columnInfo.GetPropertyPath();
    size_t pathLength = propertyPath.Size();
    BeAssert(pathLength >= 1 && "Invalid path");
    bool isInstanceIdColumn = false;
    Json::Value* currentCell = &jsonValue;
    for (size_t ii = 0; ii < pathLength; ii++)
        {
        ECPropertyCP ecProperty = propertyPath.At(ii).GetProperty();
        BeAssert(ecProperty != nullptr && "According to the ECSqlStatement API, this can happen only for array readers, where this method should never have been called");
        Utf8String ecPropertyName(ecProperty->GetName());
        if (IsIdProperty(ecPropertyName))
            {
            ecPropertyName = "$" + ecPropertyName;
            isInstanceIdColumn = true;
            BeAssert(pathLength == 1 && "Cannot have a instance id field as a member of a struct");
            }
        else if (ecPropertyName.EqualsIAscii(ECDBSYS_PROP_SourceECClassId) || ecPropertyName.EqualsIAscii(ECDBSYS_PROP_TargetECClassId))
            ecPropertyName = "$" + ecPropertyName;

        currentCell = &((*currentCell)[ecPropertyName.c_str()]);
        }

    if (isInstanceIdColumn)
        return JsonFromInstanceId(*currentCell, ecsqlValue);

    return JsonFromPropertyValue(*currentCell, ecsqlValue);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8String JsonECSqlSelectAdapter::GetClassKey(ECClassCR ecClass)
    {
    Utf8String classKey;
    classKey.Sprintf("%s.%s", ecClass.GetSchema().GetName().c_str(), ecClass.GetName().c_str());
    return classKey;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

