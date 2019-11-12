/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <Formatting/FormattingApi.h>
#include <BeRapidJson/BeRapidJson.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE



//****************************************************************************************
//SchemaPersistenceHelper
//****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     06/2017
//---------------------------------------------------------------------------------------
ECSchemaId SchemaPersistenceHelper::GetSchemaId(ECDbCR db, DbTableSpace const& tableSpace, Utf8CP schemaNameOrAlias, SchemaLookupMode mode)
    {
    Utf8String sql;
    if (tableSpace.IsMain())
        sql.assign("SELECT Id FROM main." TABLE_Schema);
    else
        sql.Sprintf("SELECT Id FROM [%s]." TABLE_Schema, tableSpace.GetName().c_str());

    switch (mode)
        {
            case SchemaLookupMode::ByName:
                sql.append(" WHERE Name=?1");
                break;

            case SchemaLookupMode::ByAlias:
                sql.append(" WHERE Alias=?1");
                break;

            default:
                sql.append(" WHERE (Name=?1 OR Alias=?1)");
                break;
        }

    CachedStatementPtr stmt = db.GetImpl().GetCachedSqliteStatement(sql.c_str());
    if (stmt == nullptr)
        return ECSchemaId();

    stmt->BindText(1, schemaNameOrAlias, Statement::MakeCopy::No);
    if (BE_SQLITE_ROW != stmt->Step())
        return ECSchemaId();

    return stmt->GetValueId<ECSchemaId>(0);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     06/2017
//---------------------------------------------------------------------------------------
std::vector<ECSchemaId> SchemaPersistenceHelper::GetSchemaIds(ECDbCR ecdb, DbTableSpace const& tableSpace, Utf8StringVirtualSet const& schemaNames)
    {
    CachedStatementPtr stmt = nullptr;
    if (tableSpace.IsMain())
        stmt = ecdb.GetImpl().GetCachedSqliteStatement("SELECT Id FROM main." TABLE_Schema " WHERE InVirtualSet(?,Name)");
    else
        stmt = ecdb.GetImpl().GetCachedSqliteStatement(Utf8PrintfString("SELECT Id FROM [%s]." TABLE_Schema " WHERE InVirtualSet(?,Name)", tableSpace.GetName().c_str()).c_str());

    if (stmt == nullptr)
        return std::vector<ECSchemaId>();

    if (BE_SQLITE_OK != stmt->BindVirtualSet(1, schemaNames))
        return std::vector<ECSchemaId>();

    std::vector<ECSchemaId> ids;
    while (BE_SQLITE_ROW == stmt->Step())
        {
        ids.push_back(stmt->GetValueId<ECSchemaId>(0));
        }

    return ids;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle      06/2017
//---------------------------------------------------------------------------------------
Utf8String SchemaPersistenceHelper::GetSchemaName(ECDbCR ecdb, DbTableSpace const& tableSpace, ECN::ECSchemaId schemaId)
    {
    CachedStatementPtr stmt = nullptr;
    if (tableSpace.IsMain())
        stmt = ecdb.GetImpl().GetCachedSqliteStatement("SELECT Name FROM main." TABLE_Schema " WHERE Id=?");
    else
        stmt = ecdb.GetImpl().GetCachedSqliteStatement(Utf8PrintfString("SELECT Name FROM [%s]." TABLE_Schema " WHERE Id=?", tableSpace.GetName().c_str()).c_str());

    if (stmt == nullptr)
        return Utf8String();

    stmt->BindId(1, schemaId);

    if (BE_SQLITE_ROW != stmt->Step())
        return Utf8String();

    return Utf8String(stmt->GetValueText(0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                    04/2016
//---------------------------------------------------------------------------------------
bool SchemaPersistenceHelper::TryGetSchemaKey(SchemaKey& key, ECDbCR ecdb, DbTableSpace const& tableSpace, Utf8CP schemaName)
    {
    CachedStatementPtr stmt = nullptr;
    if (tableSpace.IsMain())
        stmt = ecdb.GetImpl().GetCachedSqliteStatement("SELECT Name, VersionDigit1, VersionDigit2, VersionDigit3 FROM main." TABLE_Schema " WHERE Name=?");
    else
        stmt = ecdb.GetImpl().GetCachedSqliteStatement(Utf8PrintfString("SELECT Name, VersionDigit1, VersionDigit2, VersionDigit3 FROM [%s]." TABLE_Schema " WHERE Name=?", tableSpace.GetName().c_str()).c_str());

    if (stmt == nullptr)
        return false;

    if (BE_SQLITE_OK != stmt->BindText(1, schemaName, Statement::MakeCopy::No))
        return false;

    if (stmt->Step() != BE_SQLITE_ROW)
        return false;

    key = SchemaKey(stmt->GetValueText(0), stmt->GetValueInt(1), stmt->GetValueInt(2), stmt->GetValueInt(3));
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   09/2016
//---------------------------------------------------------------------------------------
//static
ECClassId SchemaPersistenceHelper::GetClassId(ECDbCR ecdb, DbTableSpace const& tableSpace, ECSchemaId schemaId, Utf8CP className)
    {
    CachedStatementPtr stmt = nullptr;
    if (tableSpace.IsMain())
        stmt = ecdb.GetImpl().GetCachedSqliteStatement("SELECT Id FROM main." TABLE_Class " WHERE SchemaId=? AND Name=?");
    else
        stmt = ecdb.GetImpl().GetCachedSqliteStatement(Utf8PrintfString("SELECT Id FROM [%s]." TABLE_Class " WHERE SchemaId=? AND Name=?", tableSpace.GetName().c_str()).c_str());

    if (stmt == nullptr)
        return ECClassId();

    if (BE_SQLITE_OK != stmt->BindId(1, schemaId))
        return ECClassId();

    if (BE_SQLITE_OK != stmt->BindText(2, className, Statement::MakeCopy::No))
        return ECClassId();

    if (BE_SQLITE_ROW != stmt->Step())
        return ECClassId();

    return stmt->GetValueId<ECClassId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Casey.Mullen      01/2013
//---------------------------------------------------------------------------------------
//static
ECClassId SchemaPersistenceHelper::GetClassId(ECDbCR ecdb, DbTableSpace const& tableSpace, Utf8CP schemaNameOrAlias, Utf8CP className, SchemaLookupMode lookupMode)
    {
    Utf8String sql;
    if (tableSpace.IsMain())
        sql.assign("SELECT c.Id FROM main." TABLE_Class " c JOIN main." TABLE_Schema " s ON c.SchemaId = s.Id");
    else
        sql.Sprintf("SELECT c.Id FROM [%s]." TABLE_Class " c JOIN [%s]." TABLE_Schema " s ON c.SchemaId = s.Id", tableSpace.GetName().c_str(), tableSpace.GetName().c_str());

    switch (lookupMode)
        {
            case SchemaLookupMode::ByName:
                sql.append(" WHERE s.Name=?1 AND c.Name=?2");
                break;

            case SchemaLookupMode::ByAlias:
                sql.append(" WHERE s.Alias=?1 AND c.Name=?2");
                break;

            default:
                sql.append(" WHERE (s.Name=?1 OR s.Alias=?1) AND c.Name=?2");
                break;
        }

    CachedStatementPtr stmt = ecdb.GetImpl().GetCachedSqliteStatement(sql.c_str());
    if (stmt == nullptr)
        return ECClassId();

    stmt->BindText(1, schemaNameOrAlias, Statement::MakeCopy::No);
    stmt->BindText(2, className, Statement::MakeCopy::No);
    if (BE_SQLITE_ROW != stmt->Step())
        return ECClassId();

    return stmt->GetValueId<ECClassId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   01/2016
//---------------------------------------------------------------------------------------
ECEnumerationId SchemaPersistenceHelper::GetEnumerationId(ECDbCR ecdb, DbTableSpace const& tableSpace, Utf8CP schemaNameOrAlias, Utf8CP enumName, SchemaLookupMode lookupMode)
    {
    Utf8String sql;
    if (tableSpace.IsMain())
        sql.assign("SELECT e.Id FROM main." TABLE_Enumeration " e, main." TABLE_Schema " s");
    else
        sql.Sprintf("SELECT e.Id FROM [%s]." TABLE_Enumeration " e, [%s]." TABLE_Schema " s", tableSpace.GetName().c_str(), tableSpace.GetName().c_str());

    switch (lookupMode)
        {
            case SchemaLookupMode::ByName:
                sql.append(" WHERE e.SchemaId=s.Id AND s.Name=?1 AND e.Name=?2");
                break;

            case SchemaLookupMode::ByAlias:
                sql.append(" WHERE e.SchemaId=s.Id AND s.Alias=?1 AND e.Name=?2");
                break;

            default:
                sql.append(" WHERE e.SchemaId=s.Id AND (s.Name=?1 OR s.Alias=?1) AND e.Name=?2");
                break;
        }

    CachedStatementPtr stmt = ecdb.GetImpl().GetCachedSqliteStatement(sql.c_str());
    if (stmt == nullptr)
        return ECEnumerationId();

    stmt->BindText(1, schemaNameOrAlias, Statement::MakeCopy::No);
    stmt->BindText(2, enumName, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return ECEnumerationId();

    return stmt->GetValueId<ECEnumerationId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 06/2016
//---------------------------------------------------------------------------------------
//static
KindOfQuantityId SchemaPersistenceHelper::GetKindOfQuantityId(ECDbCR ecdb, DbTableSpace const& tableSpace, Utf8CP schemaNameOrAlias, Utf8CP koqName, SchemaLookupMode lookupMode)
    {
    Utf8String sql;
    if (tableSpace.IsMain())
        sql.assign("SELECT koq.Id FROM main." TABLE_KindOfQuantity " koq, main." TABLE_Schema " s");
    else
        sql.Sprintf("SELECT koq.Id FROM [%s]." TABLE_KindOfQuantity " koq, [%s]." TABLE_Schema " s", tableSpace.GetName().c_str(), tableSpace.GetName().c_str());

    switch (lookupMode)
        {
            case SchemaLookupMode::ByName:
                sql.append(" WHERE koq.SchemaId=s.Id AND s.Name=?1 AND koq.Name=?2");
                break;

            case SchemaLookupMode::ByAlias:
                sql.append(" WHERE koq.SchemaId=s.Id AND s.Alias=?1 AND koq.Name=?2");
                break;

            default:
                sql.append(" WHERE koq.SchemaId=s.Id AND (s.Name=?1 OR s.Alias=?1) AND koq.Name=?2");
                break;
        }

    CachedStatementPtr stmt = ecdb.GetImpl().GetCachedSqliteStatement(sql.c_str());
    if (stmt == nullptr)
        return KindOfQuantityId();

    stmt->BindText(1, schemaNameOrAlias, Statement::MakeCopy::No);
    stmt->BindText(2, koqName, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return KindOfQuantityId();

    return stmt->GetValueId<KindOfQuantityId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 06/2017
//---------------------------------------------------------------------------------------
//static
PropertyCategoryId SchemaPersistenceHelper::GetPropertyCategoryId(ECDbCR ecdb, DbTableSpace const& tableSpace, Utf8CP schemaNameOrAlias, Utf8CP catName, SchemaLookupMode lookupMode)
    {
    Utf8String sql;
    if (tableSpace.IsMain())
        sql.assign("SELECT cat.Id FROM main." TABLE_PropertyCategory " cat, main." TABLE_Schema " s");
    else
        sql.Sprintf("SELECT cat.Id FROM [%s]." TABLE_PropertyCategory " cat, [%s]." TABLE_Schema " s", tableSpace.GetName().c_str(), tableSpace.GetName().c_str());

    switch (lookupMode)
        {
            case SchemaLookupMode::ByName:
                sql.append(" WHERE cat.SchemaId=s.Id AND s.Name=?1 AND cat.Name=?2");
                break;

            case SchemaLookupMode::ByAlias:
                sql.append(" WHERE cat.SchemaId=s.Id AND s.Alias=?1 AND cat.Name=?2");
                break;

            default:
                sql.append(" WHERE cat.SchemaId=s.Id AND (s.Name=?1 OR s.Alias=?1) AND cat.Name=?2");
                break;
        }

    CachedStatementPtr stmt = ecdb.GetImpl().GetCachedSqliteStatement(sql.c_str());
    if (stmt == nullptr)
        return PropertyCategoryId();

    stmt->BindText(1, schemaNameOrAlias, Statement::MakeCopy::No);
    stmt->BindText(2, catName, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return PropertyCategoryId();

    return stmt->GetValueId<PropertyCategoryId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 02/2018
//---------------------------------------------------------------------------------------
//static
UnitSystemId SchemaPersistenceHelper::GetUnitSystemId(ECDbCR ecdb, DbTableSpace const& tableSpace, Utf8CP schemaNameOrAlias, Utf8CP unitSystemName, SchemaLookupMode lookupMode)
    {
    Utf8String sql;
    if (tableSpace.IsMain())
        sql.assign("SELECT us.Id FROM main." TABLE_UnitSystem " us, main." TABLE_Schema " s");
    else
        sql.Sprintf("SELECT us.Id FROM [%s]." TABLE_UnitSystem " us, [%s]." TABLE_Schema " s", tableSpace.GetName().c_str(), tableSpace.GetName().c_str());

    switch (lookupMode)
        {
            case SchemaLookupMode::ByName:
                sql.append(" WHERE us.SchemaId=s.Id AND s.Name=?1 AND us.Name=?2");
                break;

            case SchemaLookupMode::ByAlias:
                sql.append(" WHERE us.SchemaId=s.Id AND s.Alias=?1 AND us.Name=?2");
                break;

            default:
                sql.append(" WHERE us.SchemaId=s.Id AND (s.Name=?1 OR s.Alias=?1) AND us.Name=?2");
                break;
        }

    CachedStatementPtr stmt = ecdb.GetImpl().GetCachedSqliteStatement(sql.c_str());
    if (stmt == nullptr)
        return UnitSystemId();

    stmt->BindText(1, schemaNameOrAlias, Statement::MakeCopy::No);
    stmt->BindText(2, unitSystemName, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return UnitSystemId();

    return stmt->GetValueId<UnitSystemId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 02/2018
//---------------------------------------------------------------------------------------
//static
PhenomenonId SchemaPersistenceHelper::GetPhenomenonId(ECDbCR ecdb, DbTableSpace const& tableSpace, Utf8CP schemaNameOrAlias, Utf8CP phenomenonName, SchemaLookupMode lookupMode)
    {
    Utf8String sql;
    if (tableSpace.IsMain())
        sql.assign("SELECT phen.Id FROM main." TABLE_Phenomenon " phen, main." TABLE_Schema " s");
    else
        sql.Sprintf("SELECT phen.Id FROM [%s]." TABLE_Phenomenon " phen, [%s]." TABLE_Schema " s", tableSpace.GetName().c_str(), tableSpace.GetName().c_str());

    switch (lookupMode)
        {
            case SchemaLookupMode::ByName:
                sql.append(" WHERE phen.SchemaId=s.Id AND s.Name=?1 AND phen.Name=?2");
                break;

            case SchemaLookupMode::ByAlias:
                sql.append(" WHERE phen.SchemaId=s.Id AND s.Alias=?1 AND phen.Name=?2");
                break;

            default:
                sql.append(" WHERE phen.SchemaId=s.Id AND (s.Name=?1 OR s.Alias=?1) AND phen.Name=?2");
                break;
        }

    CachedStatementPtr stmt = ecdb.GetImpl().GetCachedSqliteStatement(sql.c_str());
    if (stmt == nullptr)
        return PhenomenonId();

    stmt->BindText(1, schemaNameOrAlias, Statement::MakeCopy::No);
    stmt->BindText(2, phenomenonName, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return PhenomenonId();

    return stmt->GetValueId<PhenomenonId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 02/2018
//---------------------------------------------------------------------------------------
//static
UnitId SchemaPersistenceHelper::GetUnitId(ECDbCR ecdb, DbTableSpace const& tableSpace, Utf8CP schemaNameOrAlias, Utf8CP unitName, SchemaLookupMode lookupMode)
    {
    Utf8String sql;
    if (tableSpace.IsMain())
        sql.assign("SELECT u.Id FROM main." TABLE_Unit " u, main." TABLE_Schema " s");
    else
        sql.Sprintf("SELECT u.Id FROM [%s]." TABLE_Unit " u, [%s]." TABLE_Schema " s", tableSpace.GetName().c_str(), tableSpace.GetName().c_str());

    switch (lookupMode)
        {
            case SchemaLookupMode::ByName:
                sql.append(" WHERE u.SchemaId=s.Id AND s.Name=?1 AND u.Name=?2");
                break;

            case SchemaLookupMode::ByAlias:
                sql.append(" WHERE u.SchemaId=s.Id AND s.Alias=?1 AND u.Name=?2");
                break;

            default:
                sql.append(" WHERE u.SchemaId=s.Id AND (s.Name=?1 OR s.Alias=?1) AND u.Name=?2");
                break;
        }

    CachedStatementPtr stmt = ecdb.GetImpl().GetCachedSqliteStatement(sql.c_str());
    if (stmt == nullptr)
        return UnitId();

    stmt->BindText(1, schemaNameOrAlias, Statement::MakeCopy::No);
    stmt->BindText(2, unitName, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return UnitId();

    return stmt->GetValueId<UnitId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 04/2018
//---------------------------------------------------------------------------------------
//static
FormatId SchemaPersistenceHelper::GetFormatId(ECDbCR ecdb, DbTableSpace const& tableSpace, Utf8CP schemaNameOrAlias, Utf8CP formatName, SchemaLookupMode lookupMode)
    {
    Utf8String sql;
    if (tableSpace.IsMain())
        sql.assign("SELECT f.Id FROM main." TABLE_Format " f, main." TABLE_Schema " s");
    else
        sql.Sprintf("SELECT f.Id FROM [%s]." TABLE_Format " f, [%s]." TABLE_Schema " s", tableSpace.GetName().c_str(), tableSpace.GetName().c_str());

    switch (lookupMode)
        {
            case SchemaLookupMode::ByName:
                sql.append(" WHERE f.SchemaId=s.Id AND s.Name=?1 AND f.Name=?2");
                break;

            case SchemaLookupMode::ByAlias:
                sql.append(" WHERE f.SchemaId=s.Id AND s.Alias=?1 AND f.Name=?2");
                break;

            default:
                sql.append(" WHERE f.SchemaId=s.Id AND (s.Name=?1 OR s.Alias=?1) AND f.Name=?2");
                break;
        }

    CachedStatementPtr stmt = ecdb.GetImpl().GetCachedSqliteStatement(sql.c_str());
    if (stmt == nullptr)
        return FormatId();

    stmt->BindText(1, schemaNameOrAlias, Statement::MakeCopy::No);
    stmt->BindText(2, formatName, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return FormatId();

    return stmt->GetValueId<FormatId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      05/2013
//---------------------------------------------------------------------------------------
ECPropertyId SchemaPersistenceHelper::GetPropertyId(ECDbCR ecdb, DbTableSpace const& tableSpace, ECClassId ecClassId, Utf8CP propertyName)
    {
    CachedStatementPtr stmt = nullptr;
    if (tableSpace.IsMain())
        stmt = ecdb.GetImpl().GetCachedSqliteStatement("SELECT Id FROM main." TABLE_Property " WHERE ClassId=? AND Name=?");
    else
        stmt = ecdb.GetImpl().GetCachedSqliteStatement(Utf8PrintfString("SELECT Id FROM [%s]." TABLE_Property " WHERE ClassId=? AND Name=?", tableSpace.GetName().c_str()).c_str());

    if (stmt == nullptr)
        return ECPropertyId();

    if (BE_SQLITE_OK != stmt->BindId(1, ecClassId))
        return ECPropertyId();
    
    if (BE_SQLITE_OK != stmt->BindText(2, propertyName, Statement::MakeCopy::No))
        return ECPropertyId();

    if (BE_SQLITE_ROW != stmt->Step())
        return ECPropertyId();

    return stmt->GetValueId<ECPropertyId>(0);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      05/2013
//---------------------------------------------------------------------------------------
ECPropertyId SchemaPersistenceHelper::GetPropertyId(ECDbCR ecdb, DbTableSpace const& tableSpace, Utf8CP schemaNameOrAlias, Utf8CP className, Utf8CP propertyName, SchemaLookupMode mode)
    {
    Utf8String sql;
    if (tableSpace.IsMain())
        sql.assign("SELECT p.Id FROM main." TABLE_Property " p JOIN main." TABLE_Class " c ON p.ClassId=c.Id JOIN main." TABLE_Schema " s ON c.SchemaId=s.Id");
    else
        {
        Utf8CP tableSpaceName = tableSpace.GetName().c_str();
        sql.Sprintf("SELECT p.Id FROM [%s]." TABLE_Property " p JOIN [%s]." TABLE_Class " c ON p.ClassId=c.Id JOIN [%s]." TABLE_Schema " s ON c.SchemaId=s.Id", tableSpaceName, tableSpaceName, tableSpaceName);
        }

    switch (mode)
        {
            case SchemaLookupMode::ByName:
                sql.append(" WHERE c.SchemaId = s.Id AND s.Name=?1 AND c.Name=?2 AND p.Name=?3");
                break;

            case SchemaLookupMode::ByAlias:
                sql.append(" WHERE c.SchemaId = s.Id AND s.Alias=?1 AND c.Name=?2 AND p.Name=?3");
                break;

            default:
                sql.append(" WHERE c.SchemaId = s.Id AND (s.Name=?1 OR s.Alias=?1) AND c.Name=?2 AND p.Name=?3");
                break;
        }

    CachedStatementPtr stmt = ecdb.GetImpl().GetCachedSqliteStatement(sql.c_str());
    if (stmt == nullptr)
        return ECPropertyId();

    stmt->BindText(1, schemaNameOrAlias, Statement::MakeCopy::No);
    stmt->BindText(2, className, Statement::MakeCopy::No);
    stmt->BindText(3, propertyName, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return ECPropertyId();

    return stmt->GetValueId<ECPropertyId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2016
//---------------------------------------------------------------------------------------
BentleyStatus SchemaPersistenceHelper::SerializeEnumerationValues(Utf8StringR jsonStr, ECEnumerationCR ecEnum, bool isEC32AvailableInFile)
    {
    BeAssert(ecEnum.GetEnumeratorCount() > 0);
    rapidjson::Document enumValuesJson(rapidjson::kArrayType);
    rapidjson::MemoryPoolAllocator<>& jsonAllocator = enumValuesJson.GetAllocator();

    for (ECEnumerator const* enumValue : ecEnum.GetEnumerators())
        {
        rapidjson::Value enumValueJson(rapidjson::kObjectType);

        if (isEC32AvailableInFile)
            {
            Utf8StringCR enumValueName = enumValue->GetName();
            //don't copy the string into the rapidjson as the json will not outlive the string (it is persisted in the DB before the ECSchema
            //containing the enum name string is released
            enumValueJson.AddMember(ECDBMETA_PROP_ECEnumerator_Name, rapidjson::Value(rapidjson::StringRef(enumValueName.c_str(), enumValueName.size())).Move(), jsonAllocator);
            }
        else
            {
            BeAssert(ecEnum.GetSchema().OriginalECXmlVersionLessThan(ECVersion::V3_2) && "Only EC 3.1 schemas can be imported into a file not supporting EC 3.2 yet");
            }

        if (enumValue->IsInteger())
            enumValueJson.AddMember(ECDBMETA_PROP_ECEnumerator_IntValue, rapidjson::Value(enumValue->GetInteger()).Move(), jsonAllocator);
        else if (enumValue->IsString())
            {
            Utf8StringCR enumValueStr = enumValue->GetString();
            //don't copy the string into the rapidjson as the json will not outlive the string (it is persisted in the DB before the ECSchema
            //containing the enum value string is released
            enumValueJson.AddMember(ECDBMETA_PROP_ECEnumerator_StringValue,
                                    rapidjson::Value(rapidjson::StringRef(enumValueStr.c_str(), enumValueStr.size())).Move(), jsonAllocator);
            }
        else
            {
            BeAssert(false && "Code needs to be updated as ECEnumeration seems to support types other than int and string.");
            return ERROR;
            }

        
        if (enumValue->GetIsDisplayLabelDefined())
            {
            Utf8StringCR displayLabel = enumValue->GetInvariantDisplayLabel();
            //don't copy the string into the rapidjson as the json will not outlive the string (it is persisted in the DB before the ECSchema
            //containing the display label string is released
            enumValueJson.AddMember(ECDBMETA_PROP_ECEnumerator_DisplayLabel,
                                    rapidjson::Value(rapidjson::StringRef(displayLabel.c_str(), displayLabel.size())).Move(), jsonAllocator);
            }

        if (!enumValue->GetInvariantDescription().empty())
            {
            Utf8StringCR description = enumValue->GetInvariantDescription();
            //don't copy the string into the rapidjson as the json will not outlive the string (it is persisted in the DB before the ECSchema
            //containing the display label string is released
            enumValueJson.AddMember(ECDBMETA_PROP_ECEnumerator_Description,
                                    rapidjson::Value(rapidjson::StringRef(description.c_str(), description.size())).Move(), jsonAllocator);

            }

        enumValuesJson.PushBack(enumValueJson.Move(), jsonAllocator);
        }

    rapidjson::StringBuffer jsonStrBuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(jsonStrBuf);
    enumValuesJson.Accept(writer);
    jsonStr.assign(jsonStrBuf.GetString());
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2016
//---------------------------------------------------------------------------------------
BentleyStatus SchemaPersistenceHelper::DeserializeEnumerationValues(ECEnumerationR ecEnum, ECDbCR ecdb, Utf8CP jsonStr)
    {
    rapidjson::Document enumValuesJson;
    if (enumValuesJson.Parse<0>(jsonStr).HasParseError())
        {
        BeAssert(false && "Could not parse ECEnumeration values JSON string.");
        return ERROR;
        }

    BeAssert(enumValuesJson.IsArray());

    const bool isIntEnum = ecEnum.GetType() == PRIMITIVETYPE_Integer;
    BeAssert(isIntEnum || ecEnum.GetType() == PRIMITIVETYPE_String);
    const bool supportsNamedEnumerators = FeatureManager::IsAvailable(ecdb, Feature::NamedEnumerators);

    for (rapidjson::Value const& enumValueJson : enumValuesJson.GetArray())
        {
        BeAssert(enumValueJson.IsObject());
        BeAssert(enumValueJson.HasMember(ECDBMETA_PROP_ECEnumerator_IntValue) || enumValueJson.HasMember(ECDBMETA_PROP_ECEnumerator_StringValue));
        BeAssert(!supportsNamedEnumerators || enumValueJson.HasMember(ECDBMETA_PROP_ECEnumerator_Name));

        ECEnumeratorP enumValue = nullptr;

        int intVal = -1;
        Utf8CP stringVal = nullptr;
        if (enumValueJson.HasMember(ECDBMETA_PROP_ECEnumerator_IntValue))
            {
            BeAssert(isIntEnum);
            rapidjson::Value const& intValJson = enumValueJson[ECDBMETA_PROP_ECEnumerator_IntValue];
            BeAssert(intValJson.IsInt());
            intVal = intValJson.GetInt();
            }
        else if (enumValueJson.HasMember(ECDBMETA_PROP_ECEnumerator_StringValue))
            {
            BeAssert(!isIntEnum);
            rapidjson::Value const& stringValJson = enumValueJson[ECDBMETA_PROP_ECEnumerator_StringValue];
            BeAssert(stringValJson.IsString());
            stringVal = stringValJson.GetString();
            }
        else
            {
            BeAssert(false && "Unsupported underlying ECEnumeration type");
            return ERROR;
            }

        Utf8CP name = nullptr;
        Utf8String generatedName;
        if (supportsNamedEnumerators)
            {
            rapidjson::Value const& nameVal = enumValueJson[ECDBMETA_PROP_ECEnumerator_Name];
            BeAssert(nameVal.IsString());
            name = nameVal.GetString();
            }
        else
            {
            generatedName = ECEnumerator::DetermineName(ecEnum.GetName(), isIntEnum ? nullptr : stringVal, isIntEnum ? &intVal : nullptr);
            name = generatedName.c_str();
            }

        BeAssert(!Utf8String::IsNullOrEmpty(name));
        if (isIntEnum)
            {
            if (ECObjectsStatus::Success != ecEnum.CreateEnumerator(enumValue, name, intVal))
                return ERROR;
            }
        else
            {
            if (ECObjectsStatus::Success != ecEnum.CreateEnumerator(enumValue, name, stringVal))
                return ERROR;
            }
        

        if (enumValueJson.HasMember(ECDBMETA_PROP_ECEnumerator_DisplayLabel))
            {
            Utf8CP displayLabel = enumValueJson[ECDBMETA_PROP_ECEnumerator_DisplayLabel].GetString();
            enumValue->SetDisplayLabel(displayLabel);
            }

        if (enumValueJson.HasMember(ECDBMETA_PROP_ECEnumerator_Description))
            {
            Utf8CP description = enumValueJson[ECDBMETA_PROP_ECEnumerator_Description].GetString();
            enumValue->SetDescription(description);
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  06/2016
//---------------------------------------------------------------------------------------
BentleyStatus SchemaPersistenceHelper::SerializeKoqPresentationFormats(Utf8StringR jsonStr, ECDbCR ecdb, KindOfQuantityCR koq, bool isEC32AvailableInFile)
    {
    BeAssert(!koq.GetPresentationFormats().empty());
    bvector<Utf8String> formatList;
    if (isEC32AvailableInFile)
        {
        for (NamedFormat const& format : koq.GetPresentationFormats())
            {
            if (format.IsProblem())
                {
                ecdb.GetImpl().Issues().ReportV("Failed to import KindOfQuantity '%s'. One of its presentation formats is invalid: %s.", koq.GetFullName().c_str(), format.GetProblemDescription().c_str());
                return ERROR;
                }

            Utf8StringCR formatStr = format.GetQualifiedFormatString(koq.GetSchema());
            if (formatStr.empty())
                {
                BeAssert(!formatStr.empty());
                return ERROR;
                }

            formatList.push_back(formatStr);
            }
        }
    else
        {
        for (Utf8StringCR ec32FusDescriptor : koq.GetDescriptorCache().second)
            {
            formatList.push_back(ec32FusDescriptor);
            }
        }

    return SerializeKoqPresentationFormats(jsonStr, formatList);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  06/2016
//---------------------------------------------------------------------------------------
BentleyStatus SchemaPersistenceHelper::SerializeKoqPresentationFormats(Utf8StringR jsonStr, bvector<Utf8String> const& presentationFormats)
    {
    BeAssert(!presentationFormats.empty());
    rapidjson::Document presFormatsJson(rapidjson::kArrayType);
    rapidjson::MemoryPoolAllocator<>& jsonAllocator = presFormatsJson.GetAllocator();

    for (Utf8StringCR format : presentationFormats)
        {
        if (format.empty())
            {
            BeAssert(!format.empty());
            return ERROR;
            }

        presFormatsJson.PushBack(rapidjson::Value(format.c_str(), (rapidjson::SizeType) format.size(), jsonAllocator).Move(), jsonAllocator);
        }

    rapidjson::StringBuffer jsonStrBuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(jsonStrBuf);
    presFormatsJson.Accept(writer);
    jsonStr.assign(jsonStrBuf.GetString());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  05/2018
//---------------------------------------------------------------------------------------
Utf8String SchemaPersistenceHelper::SerializeNumericSpec(Formatting::NumericFormatSpecCR spec)
    {
    Json::Value json;
    if (!spec.ToJson(json, false))
        return Utf8String();

    return json.ToString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  05/2018
//---------------------------------------------------------------------------------------
Utf8String SchemaPersistenceHelper::SerializeCompositeSpecWithoutUnits(Formatting::CompositeValueSpecCR spec)
    {
    Json::Value json;
    if (!spec.ToJson(json, false, true))
        return Utf8String();

    return json.ToString();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
