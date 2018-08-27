/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaPersistenceHelper.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
BentleyStatus SchemaPersistenceHelper::SerializeEnumerationValues(Utf8StringR jsonStr, ECEnumerationCR ecEnum)
    {
    BeAssert(ecEnum.GetEnumeratorCount() > 0);
    rapidjson::Document enumValuesJson(rapidjson::kArrayType);
    rapidjson::MemoryPoolAllocator<>& jsonAllocator = enumValuesJson.GetAllocator();

    for (ECEnumerator const* enumValue : ecEnum.GetEnumerators())
        {
        rapidjson::Value enumValueJson(rapidjson::kObjectType);

        if (enumValue->IsInteger())
            enumValueJson.AddMember(ECDBMETA_PROP_ECEnumerator_IntValue, rapidjson::Value(enumValue->GetInteger()).Move(), jsonAllocator);
        else if (enumValue->IsString())
            {
            Utf8StringCR enumValueStr = enumValue->GetString();
            enumValueJson.AddMember(ECDBMETA_PROP_ECEnumerator_StringValue,
                                    rapidjson::Value(enumValueStr.c_str(), (rapidjson::SizeType) enumValueStr.size(), jsonAllocator).Move(), 
                                    jsonAllocator);
            }
        else
            {
            BeAssert(false && "Code needs to be updated as ECEnumeration seems to support types other than int and string.");
            return ERROR;
            }

        if (enumValue->GetIsDisplayLabelDefined())
            {
            Utf8StringCR displayLabel = enumValue->GetInvariantDisplayLabel();
            enumValueJson.AddMember(ECDBMETA_PROP_ECEnumerator_DisplayLabel,
                                    rapidjson::Value(displayLabel.c_str(), (rapidjson::SizeType) displayLabel.size(), jsonAllocator).Move(),
                                    jsonAllocator);

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
BentleyStatus SchemaPersistenceHelper::DeserializeEnumerationValues(ECEnumerationR ecEnum, Utf8CP jsonStr)
    {
    rapidjson::Document enumValuesJson;
    if (enumValuesJson.Parse<0>(jsonStr).HasParseError())
        {
        BeAssert(false && "Could not parse ECEnumeration values JSON string.");
        return ERROR;
        }

    BeAssert(enumValuesJson.IsArray());

    for (rapidjson::Value const& enumValueJson : enumValuesJson.GetArray())
        {
        BeAssert(enumValueJson.IsObject());
        BeAssert(enumValueJson.HasMember(ECDBMETA_PROP_ECEnumerator_IntValue) || enumValueJson.HasMember(ECDBMETA_PROP_ECEnumerator_StringValue));

        ECEnumeratorP enumValue = nullptr;

        if (enumValueJson.HasMember(ECDBMETA_PROP_ECEnumerator_IntValue))
            {
            rapidjson::Value const& intVal = enumValueJson[ECDBMETA_PROP_ECEnumerator_IntValue];
            BeAssert(intVal.IsInt());
            if (ECObjectsStatus::Success != ecEnum.CreateEnumerator(enumValue, intVal.GetInt()))
                return ERROR;
            }
        else if (enumValueJson.HasMember(ECDBMETA_PROP_ECEnumerator_StringValue))
            {
            rapidjson::Value const& stringVal = enumValueJson[ECDBMETA_PROP_ECEnumerator_StringValue];
            BeAssert(stringVal.IsString());
            if (ECObjectsStatus::Success != ecEnum.CreateEnumerator(enumValue, stringVal.GetString()))
                return ERROR;
            }
        else
            {
            BeAssert(false && "Unsupported underlying ECEnumeration type");
            return ERROR;
            }

        if (enumValueJson.HasMember(ECDBMETA_PROP_ECEnumerator_DisplayLabel))
            {
            Utf8CP displayLabel = enumValueJson[ECDBMETA_PROP_ECEnumerator_DisplayLabel].GetString();
            enumValue->SetDisplayLabel(displayLabel);
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  06/2016
//---------------------------------------------------------------------------------------
BentleyStatus SchemaPersistenceHelper::SerializeKoqPresentationUnits(Utf8StringR jsonStr, ECDbCR ecdb, KindOfQuantityCR koq)
    {
    BeAssert(!koq.GetPresentationUnitList().empty());
    rapidjson::Document presUnitsJson(rapidjson::kArrayType);
    rapidjson::MemoryPoolAllocator<>& jsonAllocator = presUnitsJson.GetAllocator();

    for (Formatting::FormatUnitSet const& presUnit : koq.GetPresentationUnitList())
        {
        if (presUnit.HasProblem())
            {
            ecdb.GetImpl().Issues().ReportV("Failed to import KindOfQuantity '%s'. One of its presentation units is invalid: %s.", koq.GetFullName().c_str(), presUnit.GetProblemDescription().c_str());
            return ERROR;
            }

        Utf8String presUnitStr = presUnit.ToText(false);
        if (presUnitStr.empty())
            {
            BeAssert(!presUnitStr.empty());
            return ERROR;
            }

        presUnitsJson.PushBack(rapidjson::Value(presUnitStr.c_str(), (rapidjson::SizeType) presUnitStr.size(), jsonAllocator).Move(), jsonAllocator);
        }

    rapidjson::StringBuffer jsonStrBuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(jsonStrBuf);
    presUnitsJson.Accept(writer);
    jsonStr.assign(jsonStrBuf.GetString());
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  06/2016
//---------------------------------------------------------------------------------------
BentleyStatus SchemaPersistenceHelper::DeserializeKoqPresentationUnits(KindOfQuantityR koq, ECDbCR ecdb, Utf8CP jsonStr, bool fileUsesEC32Koqs)
    {
    rapidjson::Document presUnitsJson;
    if (presUnitsJson.Parse<0>(jsonStr).HasParseError())
        {
        BeAssert(false && "Could not parse KindOfQuantity PresentationUnits values JSON string.");
        return ERROR;
        }

    BeAssert(presUnitsJson.IsArray());

    const bool persUnitIsDummy = !koq.GetPersistenceUnit().GetUnit()->IsValid();
    for (rapidjson::Value const& presUnitJson : presUnitsJson.GetArray())
        {
        BeAssert(presUnitJson.IsString() && presUnitJson.GetStringLength() > 0);

        Utf8CP fusDescr = nullptr;
        Utf8String fusDescrStr;
        if (!fileUsesEC32Koqs)
            fusDescr = presUnitJson.GetString();
        else
            {
            //convert EC3.2 format string to FUS descriptor. Ignore errors (resilience contract)
            if (ECObjectsStatus::Success != KindOfQuantity::FormatStringToFUSDescriptor(fusDescrStr, koq, presUnitJson.GetString()))
                continue;

            fusDescr = fusDescrStr.c_str();
            }

        Formatting::FormatUnitSet fus;
        bool hasDummyUnit = false;
        ECObjectsStatus stat = KindOfQuantity::ParseFUSDescriptor(fus, hasDummyUnit, fusDescr, koq, !fileUsesEC32Koqs);
        if (!fileUsesEC32Koqs)
            {
            if (ECObjectsStatus::Success != stat || hasDummyUnit)
                {
                LOG.errorv("Failed to read KindOfQuantity '%s'. Its presentation unit's FormatUnitSet descriptor '%s' could not be parsed.", koq.GetFullName().c_str(), fusDescr);
                return ERROR;
                }
            }
        else
            {
            if (ECObjectsStatus::Success != stat)
                {
                LOG.infov("Dropped presentation unit for KindOfQuantity '%s'. Could not convert EC3.2 format '%s' to a FormatUnitSet.", koq.GetFullName().c_str(), fusDescr);
                continue; // Ignore errors (resilience contract)
                }
            }

        if (!koq.AddPresentationUnit(fus))
            {
            if (!persUnitIsDummy) //if pers unit is dummy, we cannot have presentation units, so ignore errors in that case
                return ERROR;
            }

        BeAssert(!fus.HasProblem() && "KOQ PresentationUnit could not be read. Its format is invalid");
        }

    return SUCCESS;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
