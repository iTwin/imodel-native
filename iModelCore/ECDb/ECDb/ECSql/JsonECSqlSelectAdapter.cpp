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

//=======================================================================================
//! Helper class for the JsonECSqlSelectAdapter
// @bsiclass                                                Ramanujam.Raman      10/2012
//+===============+===============+===============+===============+===============+======
struct ECSqlToJsonConverter final : NonCopyableClass
    {
    private:
        ECSqlStatement const& m_stmt;
        JsonECSqlSelectAdapter::FormatOptions m_formatOptions;

        BentleyStatus PropertyValueToJson(JsonValueR, IECSqlValue const&) const;
        BentleyStatus PrimitiveToJson(JsonValueR, IECSqlValue const&, ECN::PrimitiveType) const;
        BentleyStatus NavigationToJson(JsonValueR, IECSqlValue const&) const;
        BentleyStatus StructToJson(JsonValueR, IECSqlValue const&) const;
        BentleyStatus PrimitiveArrayToJson(JsonValueR, IECSqlValue const&, ECN::PrimitiveType) const;
        BentleyStatus StructArrayToJson(JsonValueR, IECSqlValue const&) const;
    public:
        ECSqlToJsonConverter(ECSqlStatement const& stmt, JsonECSqlSelectAdapter::FormatOptions options) : m_stmt(stmt), m_formatOptions(options) {}

        BentleyStatus ValidatePreconditions() const;
        BentleyStatus SelectClauseItemToJson(JsonValueR, IECSqlValue const&, ECSqlSystemPropertyInfo const& sysPropInfo) const;

        ECSqlSystemPropertyInfo const& DetermineSystemPropertyInfo(ECSqlColumnInfoCR colInfo)
            {
            BeAssert(colInfo.GetProperty() != nullptr && "Must be checked before");
            return colInfo.IsGeneratedProperty() ? ECSqlSystemPropertyInfo::NoSystemProperty() :
                m_stmt.GetECDb()->Schemas().GetReader().GetSystemSchemaHelper().GetSystemPropertyInfo(*colInfo.GetProperty());
            }

        static Utf8String MemberNameFromSelectClauseItem(ECSqlColumnInfoCR, ECSqlSystemPropertyInfo const&);
        static void ToJsonMemberName(Utf8StringR str) { str[0] = (char) tolower(str[0]); }
    };

//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECSqlSelectAdapter::GetRow(JsonValueR rowJson) const
    {
    ECSqlToJsonConverter converter(m_ecsqlStatement, m_formatOptions);
    if (SUCCESS != converter.ValidatePreconditions())
        return ERROR;

    rowJson = Json::Value(Json::objectValue);

    const int count = m_ecsqlStatement.GetColumnCount();
    bmap<Utf8String, int> columnNameCollisions;

    for (int columnIndex = 0; columnIndex < count; columnIndex++)
        {
        IECSqlValue const& ecsqlValue = m_ecsqlStatement.GetValue(columnIndex);
        if (ecsqlValue.IsNull())
            continue;

        ECSqlColumnInfoCR colInfo = ecsqlValue.GetColumnInfo();
        if (colInfo.GetPropertyPath().GetLeafEntry().GetKind() == ECSqlPropertyPath::Entry::Kind::ArrayIndex)
            {
            LOG.errorv("JsonECSqlSelectAdapter does not support array accessors in an ECSQL select clause: %s", m_ecsqlStatement.GetECSql());
            return ERROR;
            }

        BeAssert(colInfo.GetProperty() != nullptr);
        ECSqlSystemPropertyInfo const& sysPropInfo = converter.DetermineSystemPropertyInfo(colInfo);
        Utf8String memberName = converter.MemberNameFromSelectClauseItem(colInfo, sysPropInfo);

        int occurrenceCount = 0;
        auto it = columnNameCollisions.find(memberName);
        if (it == columnNameCollisions.end())
            occurrenceCount = 1;
        else
            occurrenceCount = it->second + 1;

        columnNameCollisions[memberName] = occurrenceCount;

        if (occurrenceCount > 1)
            {
            Utf8String suffix;
            suffix.Sprintf("%d", occurrenceCount - 1);
            memberName.append(suffix);
            }

        if (SUCCESS != converter.SelectClauseItemToJson(rowJson[memberName.c_str()], ecsqlValue, sysPropInfo))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 03/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECSqlSelectAdapter::GetRowInstance(JsonValueR rowJson, ECClassId classId) const
    {
    ECSqlToJsonConverter converter(m_ecsqlStatement, m_formatOptions);
    if (SUCCESS != converter.ValidatePreconditions() || !classId.IsValid())
        return ERROR;

    rowJson = Json::Value(Json::objectValue);

    bool foundMatchingSelectClauseItems = false;
    const int count = m_ecsqlStatement.GetColumnCount();
    for (int columnIndex = 0; columnIndex < count; columnIndex++)
        {
        IECSqlValue const& ecsqlValue = m_ecsqlStatement.GetValue(columnIndex);
        if (ecsqlValue.IsNull())
            continue;

        ECSqlColumnInfoCR colInfo = ecsqlValue.GetColumnInfo();
        ECClassCR rootClass = colInfo.GetRootClass();
        if (colInfo.IsGeneratedProperty() || rootClass.GetId() != classId)
            continue;

        if (colInfo.GetPropertyPath().GetLeafEntry().GetKind() == ECSqlPropertyPath::Entry::Kind::ArrayIndex)
            {
            LOG.errorv("JsonECSqlSelectAdapter does not support array accessors in an ECSQL select clause: %s", m_ecsqlStatement.GetECSql());
            return ERROR;
            }

        foundMatchingSelectClauseItems = true;

        BeAssert(colInfo.GetProperty() != nullptr);
        ECSqlSystemPropertyInfo const& sysPropInfo = converter.DetermineSystemPropertyInfo(colInfo);
        Utf8String memberName = converter.MemberNameFromSelectClauseItem(colInfo, sysPropInfo);

        if (SUCCESS != converter.SelectClauseItemToJson(rowJson[memberName.c_str()], ecsqlValue, sysPropInfo))
            return ERROR;
        }

    if (!foundMatchingSelectClauseItems)
        {
        LOG.errorv("No properties of ECClass with Id '%s' found in ECSqlStatement '%s' passed to JsonECSqlSelectAdapter.", classId.ToString().c_str(), m_ecsqlStatement.GetECSql());
        return ERROR;
        }

    return SUCCESS;
    }


//******************************************************************************************
// ECSqlToJsonConverter
//******************************************************************************************
//--------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                 09/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlToJsonConverter::ValidatePreconditions() const
    {
    if (!m_stmt.IsPrepared())
        {
        LOG.error("ECSqlStatement passed to JsonECSqlSelectAdapter is not prepared yet.");
        return ERROR;
        }

    if (m_stmt.GetColumnCount() == 0)
        {
        LOG.errorv("ECSqlStatement '%s' passed to JsonECSqlSelectAdapter has no SELECT clause.", m_stmt.GetECSql());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlToJsonConverter::SelectClauseItemToJson(JsonValueR json, IECSqlValue const& ecsqlValue, ECSqlSystemPropertyInfo const& sysPropInfo) const
    {
    BeAssert(!ecsqlValue.IsNull() && "Should have been caught before");

    if (!sysPropInfo.IsSystemProperty())
        return PropertyValueToJson(json, ecsqlValue);

    BeAssert(ecsqlValue.GetId<BeInt64Id>().IsValid());

    if (sysPropInfo.GetType() == ECSqlSystemPropertyInfo::Type::Class)
        {
        switch (sysPropInfo.GetClass())
            {
                case ECSqlSystemPropertyInfo::Class::ECInstanceId:
                    return ECJsonUtilities::IdToJson(json, ecsqlValue.GetId<BeInt64Id>());

                case ECSqlSystemPropertyInfo::Class::ECClassId:
                {
                ECClassCP cls = m_stmt.GetECDb()->Schemas().GetClass(ecsqlValue.GetId<ECClassId>());
                if (cls == nullptr)
                    return ERROR;

                ECJsonUtilities::ClassNameToJson(json, *cls);
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
                    return ECJsonUtilities::IdToJson(json, ecsqlValue.GetId<BeInt64Id>());

                case ECSqlSystemPropertyInfo::Relationship::SourceECClassId:
                {
                ECClassCP cls = m_stmt.GetECDb()->Schemas().GetClass(ecsqlValue.GetId<ECClassId>());
                if (cls == nullptr)
                    return ERROR;

                ECJsonUtilities::ClassNameToJson(json, *cls);
                return SUCCESS;
                }

                case ECSqlSystemPropertyInfo::Relationship::TargetECInstanceId:
                    return ECJsonUtilities::IdToJson(json, ecsqlValue.GetId<BeInt64Id>());

                case ECSqlSystemPropertyInfo::Relationship::TargetECClassId:
                {
                ECClassCP cls = m_stmt.GetECDb()->Schemas().GetClass(ecsqlValue.GetId<ECClassId>());
                if (cls == nullptr)
                    return ERROR;

                ECJsonUtilities::ClassNameToJson(json, *cls);
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
BentleyStatus ECSqlToJsonConverter::PropertyValueToJson(JsonValueR jsonValue, IECSqlValue const& ecsqlValue) const
    {
    BeAssert(!ecsqlValue.IsNull() && "Should have been caught before to avoid that a null JSON value is created");

    ECN::ECPropertyCP prop = ecsqlValue.GetColumnInfo().GetProperty();
    BeAssert(prop != nullptr);

    if (prop->GetIsPrimitive())
        return PrimitiveToJson(jsonValue, ecsqlValue, prop->GetAsPrimitiveProperty()->GetType());

    if (prop->GetIsStruct())
        return StructToJson(jsonValue, ecsqlValue);

    if (prop->GetIsNavigation())
        return NavigationToJson(jsonValue, ecsqlValue);

    if (prop->GetIsPrimitiveArray())
        return PrimitiveArrayToJson(jsonValue, ecsqlValue, prop->GetAsPrimitiveArrayProperty()->GetPrimitiveElementType());

    if (prop->GetIsStructArray())
        return StructArrayToJson(jsonValue, ecsqlValue);

    BeAssert(false && "Unhandled ECProperty type. Adjust the code for this new ECProperty type");
    return ERROR;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlToJsonConverter::PrimitiveArrayToJson(JsonValueR jsonValue, IECSqlValue const& ecsqlValue, PrimitiveType arrayElementType) const
    {
    int i = 0;
    jsonValue = Json::Value(Json::arrayValue);
    for (IECSqlValue const& arrayElementValue : ecsqlValue.GetArrayIterable())
        {
        if (arrayElementValue.IsNull())
            continue;

        if (SUCCESS != PrimitiveToJson(jsonValue[i], arrayElementValue, arrayElementType))
            return ERROR;

        i++;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlToJsonConverter::StructArrayToJson(JsonValueR jsonValue, IECSqlValue const& ecsqlValue) const
    {
    int i = 0;
    Json::Value temp(Json::arrayValue);
    for (IECSqlValue const& arrayElementValue : ecsqlValue.GetArrayIterable())
        {
        if (arrayElementValue.IsNull())
            continue;

        if (SUCCESS != StructToJson(temp[i], arrayElementValue))
            return ERROR;

        i++;
        }

    jsonValue = temp;
    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle               06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlToJsonConverter::NavigationToJson(JsonValueR jsonValue, IECSqlValue const& ecsqlValue) const
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

    ECClassCP relClass = m_stmt.GetECDb()->Schemas().GetClass(relClassIdVal.GetId<ECClassId>());
    if (relClass == nullptr)
        return ERROR;

    ECJsonUtilities::ClassNameToJson(jsonValue[ECJsonUtilities::json_navRelClassName()], *relClass);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlToJsonConverter::StructToJson(JsonValueR jsonValue, IECSqlValue const& ecsqlValue) const
    {
    for (IECSqlValue const& structMemberValue : ecsqlValue.GetStructIterable())
        {
        if (structMemberValue.IsNull())
            continue;

        ECPropertyCP memberProp = structMemberValue.GetColumnInfo().GetProperty();
        BeAssert(memberProp != nullptr);
        Utf8String memberPropName(memberProp->GetName());
        //ToCamelCase(memberPropName);
        if (SUCCESS != PropertyValueToJson(jsonValue[memberPropName.c_str()], structMemberValue))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlToJsonConverter::PrimitiveToJson(JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECN::PrimitiveType primType) const
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
            if (m_formatOptions == JsonECSqlSelectAdapter::FormatOptions::LongsAreIds)
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

//---------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 09/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static 
Utf8String ECSqlToJsonConverter::MemberNameFromSelectClauseItem(ECSqlColumnInfo const& colInfo, ECSqlSystemPropertyInfo const& sysPropInfo)
    {
    if (!sysPropInfo.IsSystemProperty())
        {
        //if property is generated, the display label contains the select clause item as is.
        //The property name in contrast would have encoded special characters of the select clause item.
        //Ex: SELECT MyProp + 4 FROM Foo -> the member name in JSON must be "MyProp + 4"
        if (colInfo.IsGeneratedProperty())
            return colInfo.GetProperty()->GetDisplayLabel();

        return colInfo.GetPropertyPath().ToString();
        }

    if (sysPropInfo.GetType() == ECSqlSystemPropertyInfo::Type::Class)
        {
        switch (sysPropInfo.GetClass())
            {
                case ECSqlSystemPropertyInfo::Class::ECInstanceId:
                    return ECJsonSystemNames::Id();

                case ECSqlSystemPropertyInfo::Class::ECClassId:
                    return ECJsonSystemNames::ClassName();

                default:
                    BeAssert(false);
                    return Utf8String();
            }
        }

    if (sysPropInfo.GetType() == ECSqlSystemPropertyInfo::Type::Relationship)
        {
        switch (sysPropInfo.GetRelationship())
            {
                case ECSqlSystemPropertyInfo::Relationship::SourceECInstanceId:
                    return ECJsonSystemNames::SourceId();

                case ECSqlSystemPropertyInfo::Relationship::SourceECClassId:
                    return ECJsonSystemNames::SourceClassName();

                case ECSqlSystemPropertyInfo::Relationship::TargetECInstanceId:
                    return ECJsonSystemNames::TargetId();

                case ECSqlSystemPropertyInfo::Relationship::TargetECClassId:
                    return ECJsonSystemNames::TargetClassName();

                default:
                    BeAssert(false);
                    return Utf8String();
            }
        }

    BeAssert(false && "Other system properties should not show up in ECSQL select clause");
    return Utf8String();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

