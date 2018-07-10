/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/JsonECSqlSelectAdapter.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
struct ECSqlToJsonConverter final
    {
    private:
        ECSqlStatement const& m_stmt;
        JsonECSqlSelectAdapter::FormatOptions const& m_formatOptions;

        //not copyable
        ECSqlToJsonConverter(ECSqlToJsonConverter const&) = delete;
        ECSqlToJsonConverter& operator=(ECSqlToJsonConverter const&) = delete;

        BentleyStatus PropertyValueToJson(JsonValueR, IECSqlValue const&) const;
        BentleyStatus PrimitiveToJson(JsonValueR, IECSqlValue const&, ECN::PrimitiveType) const;
        BentleyStatus NavigationToJson(JsonValueR, IECSqlValue const&) const;
        BentleyStatus StructToJson(JsonValueR, IECSqlValue const&) const;
        BentleyStatus PrimitiveArrayToJson(JsonValueR, IECSqlValue const&, ECN::PrimitiveType) const;
        BentleyStatus StructArrayToJson(JsonValueR, IECSqlValue const&) const;

        static void FormatMemberName(Utf8String&, JsonECSqlSelectAdapter::MemberNameCasing);
        static bool NeedsMemberNameReformatting(JsonECSqlSelectAdapter::MemberNameCasing);
    public:
        ECSqlToJsonConverter(ECSqlStatement const& stmt, JsonECSqlSelectAdapter::FormatOptions const& options) : m_stmt(stmt), m_formatOptions(options) {}

        BentleyStatus ValidatePreconditions() const;
        ECSqlSystemPropertyInfo const& DetermineTopLevelSystemPropertyInfo(ECSqlColumnInfoCR colInfo);
        BentleyStatus SelectClauseItemToJson(JsonValueR, IECSqlValue const&, ECSqlSystemPropertyInfo const& sysPropInfo) const;

        Utf8String MemberNameFromSelectClauseItem(ECSqlColumnInfoCR, ECSqlSystemPropertyInfo const&) const;
    };

//--------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                   06/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECSqlSelectAdapter::SetStatement(ECSqlStatementCR ecsqlStatement) const
    {
    BeAssert(m_ecsqlStatement != nullptr);
    BeAssert(m_hashCode != 0u);
    BeAssert(ecsqlStatement.GetHashCode() != 0u);
    BeAssert(ecsqlStatement.IsPrepared());
    if (!ecsqlStatement.IsPrepared())
        return ERROR;

    if (ecsqlStatement.GetHashCode() != m_hashCode)
        {
        m_members = nullptr;
        m_hashCode = ecsqlStatement.GetHashCode();
        }

    if (m_ecsqlStatement != &ecsqlStatement)
        m_ecsqlStatement = &ecsqlStatement;

    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                   06/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECSqlSelectAdapter::Init() const
    {
    if (SetStatement(*m_ecsqlStatement) != SUCCESS)
        return ERROR;

    if (m_members != nullptr)
        return SUCCESS;
    
    m_members = std::unique_ptr<bvector<Utf8String>>(new bvector<Utf8String>());
    
    ECSqlToJsonConverter converter(*m_ecsqlStatement, m_formatOptions);
    if (SUCCESS != converter.ValidatePreconditions())
        return ERROR;

    const int count = m_ecsqlStatement->GetColumnCount();
    bmap<Utf8String, int> columnNameCollisions;
    for (int columnIndex = 0; columnIndex < count; columnIndex++)
        {
        IECSqlValue const& ecsqlValue = m_ecsqlStatement->GetValue(columnIndex);       
        ECSqlColumnInfoCR colInfo = ecsqlValue.GetColumnInfo();
        if (colInfo.GetPropertyPath().GetLeafEntry().GetKind() == ECSqlPropertyPath::Entry::Kind::ArrayIndex)
            {
            LOG.errorv("JsonECSqlSelectAdapter does not support array accessors in an ECSQL select clause: %s", m_ecsqlStatement->GetECSql());
            return ERROR;
            }

        BeAssert(colInfo.GetProperty() != nullptr);
        ECSqlSystemPropertyInfo const& sysPropInfo = converter.DetermineTopLevelSystemPropertyInfo(colInfo);
        Utf8String memberName = converter.MemberNameFromSelectClauseItem(colInfo, sysPropInfo);
        if (memberName.empty())
            return ERROR;

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
            suffix.Sprintf("_%d", occurrenceCount - 1);
            memberName.append(suffix);
            }
        
        m_members->push_back(memberName);
        }

    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECSqlSelectAdapter::GetRow(JsonValueR rowJson, bool appendToJson) const
    {
    if (Init() != SUCCESS)
        return ERROR;

    ECSqlToJsonConverter converter(*m_ecsqlStatement, m_formatOptions);
    if (SUCCESS != converter.ValidatePreconditions())
        return ERROR;

    if (appendToJson)
        {
        if (rowJson.isNull() || !rowJson.isObject()) //explicit null check is necessary, as isObject returns true for null as well
            return ERROR;
        }
    else
        rowJson = Json::Value(Json::objectValue);

    const int count = m_ecsqlStatement->GetColumnCount();   
    for (int columnIndex = 0; columnIndex < count; columnIndex++)
        {
        IECSqlValue const& ecsqlValue = m_ecsqlStatement->GetValue(columnIndex);
        if (ecsqlValue.IsNull())
            continue;

        ECSqlColumnInfoCR colInfo = ecsqlValue.GetColumnInfo();
        ECSqlSystemPropertyInfo const& sysPropInfo = converter.DetermineTopLevelSystemPropertyInfo(colInfo);
        Utf8StringCR memberName = m_members->at(columnIndex);
        if (m_thisAdaptorCached)
            {
            if (SUCCESS != converter.SelectClauseItemToJson(rowJson[Json::StaticString(memberName.c_str())], ecsqlValue, sysPropInfo))
                return ERROR;
            }
        else
            {
            if (SUCCESS != converter.SelectClauseItemToJson(rowJson[memberName], ecsqlValue, sysPropInfo))
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 03/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECSqlSelectAdapter::GetRowInstance(JsonValueR rowJson, ECClassId classId) const
    {
    ECSqlToJsonConverter converter(*m_ecsqlStatement, m_formatOptions);
    if (SUCCESS != converter.ValidatePreconditions() || !classId.IsValid())
        return ERROR;

    rowJson = Json::Value(Json::objectValue);

    bool foundMatchingSelectClauseItems = false;
    const int count = m_ecsqlStatement->GetColumnCount();
    for (int columnIndex = 0; columnIndex < count; columnIndex++)
        {
        IECSqlValue const& ecsqlValue = m_ecsqlStatement->GetValue(columnIndex);
        if (ecsqlValue.IsNull())
            continue;

        ECSqlColumnInfoCR colInfo = ecsqlValue.GetColumnInfo();
        ECSqlColumnInfo::RootClass const& rootClass = colInfo.GetRootClass();
        if (colInfo.IsGeneratedProperty() || rootClass.GetClass().GetId() != classId)
            continue;

        if (colInfo.GetPropertyPath().GetLeafEntry().GetKind() == ECSqlPropertyPath::Entry::Kind::ArrayIndex)
            {
            LOG.errorv("JsonECSqlSelectAdapter does not support array accessors in an ECSQL select clause: %s", m_ecsqlStatement->GetECSql());
            return ERROR;
            }

        foundMatchingSelectClauseItems = true;

        BeAssert(colInfo.GetProperty() != nullptr);
        ECSqlSystemPropertyInfo const& sysPropInfo = converter.DetermineTopLevelSystemPropertyInfo(colInfo);
        Utf8String memberName = converter.MemberNameFromSelectClauseItem(colInfo, sysPropInfo);

        if (SUCCESS != converter.SelectClauseItemToJson(rowJson[memberName.c_str()], ecsqlValue, sysPropInfo))
            return ERROR;
        }

    if (!foundMatchingSelectClauseItems)
        {
        LOG.errorv("No properties of ECClass with Id '%s' found in ECSqlStatement '%s' passed to JsonECSqlSelectAdapter.", classId.ToString().c_str(), m_ecsqlStatement->GetECSql());
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
                ECClassCP cls = m_stmt.GetECDb()->Schemas().GetClass(ecsqlValue.GetId<ECClassId>(), ecsqlValue.GetColumnInfo().GetRootClass().GetTableSpace().c_str());
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
                case ECSqlSystemPropertyInfo::Relationship::TargetECInstanceId:
                    return ECJsonUtilities::IdToJson(json, ecsqlValue.GetId<BeInt64Id>());

                case ECSqlSystemPropertyInfo::Relationship::SourceECClassId:
                case ECSqlSystemPropertyInfo::Relationship::TargetECClassId:
                {
                ECClassCP cls = m_stmt.GetECDb()->Schemas().GetClass(ecsqlValue.GetId<ECClassId>(), ecsqlValue.GetColumnInfo().GetRootClass().GetTableSpace().c_str());
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

    ECClassCP relClass = m_stmt.GetECDb()->Schemas().GetClass(relClassIdVal.GetId<ECClassId>(), ecsqlValue.GetColumnInfo().GetRootClass().GetTableSpace().c_str());
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
        Json::Value* memberJson = nullptr;
        if (NeedsMemberNameReformatting(m_formatOptions.GetMemberCasingMode()))
            {
            Utf8String memberPropName(memberProp->GetName());
            FormatMemberName(memberPropName, m_formatOptions.GetMemberCasingMode());
            memberJson = &jsonValue[memberPropName.c_str()];
            }
        else
            memberJson = &jsonValue[memberProp->GetName().c_str()];

        BeAssert(memberJson != nullptr);
        if (SUCCESS != PropertyValueToJson(*memberJson, structMemberValue))
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
            int size = 0;
            Byte const* data = (Byte const*) ecsqlValue.GetBlob(&size);
            ECJsonUtilities::BinaryToJson(jsonValue, data, (size_t) size);
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
                ECJsonUtilities::Int64ToJson(jsonValue, ecsqlValue.GetInt64(), m_formatOptions.GetInt64Format());
                return SUCCESS;

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
ECSqlSystemPropertyInfo const& ECSqlToJsonConverter::DetermineTopLevelSystemPropertyInfo(ECSqlColumnInfoCR colInfo)
    {
    if (colInfo.IsGeneratedProperty())
        return ECSqlSystemPropertyInfo::NoSystemProperty();

    BeAssert(colInfo.GetProperty() != nullptr && "Must be checked before");
    ECSqlSystemPropertyInfo const& sysPropInfo = m_stmt.GetECDb()->Schemas().Main().GetSystemSchemaHelper().GetSystemPropertyInfo(*colInfo.GetProperty());
    if (sysPropInfo.GetType() == ECSqlSystemPropertyInfo::Type::Class || sysPropInfo.GetType() == ECSqlSystemPropertyInfo::Type::Relationship)
        return sysPropInfo;

    return ECSqlSystemPropertyInfo::NoSystemProperty();
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 09/2017
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECSqlToJsonConverter::MemberNameFromSelectClauseItem(ECSqlColumnInfo const& colInfo, ECSqlSystemPropertyInfo const& sysPropInfo) const
    {
    //if property is generated, the display label contains the select clause item as is.
    //The property name in contrast would have encoded special characters of the select clause item.
    //Ex: SELECT MyProp + 4 FROM Foo -> the member name in JSON must be "MyProp + 4"
    if (colInfo.IsGeneratedProperty())
        {
        Utf8String name(colInfo.GetProperty()->GetDisplayLabel());
        if (name.empty())
            name.assign(colInfo.GetProperty()->GetName());

        FormatMemberName(name, m_formatOptions.GetMemberCasingMode());
        return name;
        }

    if (!sysPropInfo.IsSystemProperty())
        {
        Utf8String name(colInfo.GetPropertyPath().ToString());
        BeAssert(!name.empty());
        FormatMemberName(name, m_formatOptions.GetMemberCasingMode());
        return name;
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

//---------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 09/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static 
bool ECSqlToJsonConverter::NeedsMemberNameReformatting(JsonECSqlSelectAdapter::MemberNameCasing casingMode)
    {
    switch (casingMode)
        {
            case JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal:
                return false;

            case JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar:
                return true;

            default:
                BeAssert(false);
                return false;
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 09/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static 
void ECSqlToJsonConverter::FormatMemberName(Utf8String& memberName, JsonECSqlSelectAdapter::MemberNameCasing casingMode)
    {
    switch (casingMode)
        {
            case JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal:
                return;

            case JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar:
                ECJsonUtilities::LowerFirstChar(memberName);
                return;

            default:
                BeAssert(false);
                return;
        }
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

