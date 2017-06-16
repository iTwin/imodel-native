/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaPersistenceHelper.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <Formatting/FormattingApi.h>
#include <rapidjson/BeRapidJson.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Casey.Mullen      01/2013
//---------------------------------------------------------------------------------------
ECSchemaId SchemaPersistenceHelper::GetSchemaId(ECDbCR db, Utf8CP schemaName)
    {
    CachedStatementPtr stmt = db.GetCachedStatement("SELECT Id FROM ec_Schema WHERE Name=?");
    if (stmt == nullptr)
        return ECSchemaId();

    stmt->BindText(1, schemaName, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return ECSchemaId();

    return stmt->GetValueId<ECSchemaId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle      06/2017
//---------------------------------------------------------------------------------------
Utf8String SchemaPersistenceHelper::GetSchemaName(ECDbCR ecdb, ECN::ECSchemaId schemaId)
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT Name FROM ec_Schema WHERE Id=?");
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
bool SchemaPersistenceHelper::TryGetSchemaKey(SchemaKey& key, ECDbCR ecdb, Utf8CP schemaName)
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT Name, VersionDigit1, VersionDigit2, VersionDigit3 FROM ec_Schema WHERE Name=?");
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
// @bsimethod                                 Affan.Khan                    04/2016
//---------------------------------------------------------------------------------------
bool SchemaPersistenceHelper::TryGetSchemaKeyAndId(SchemaKey& key, ECSchemaId& id, ECDbCR ecdb, Utf8CP schemaName)
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT Name, VersionDigit1, VersionDigit2, VersionDigit3, Id FROM ec_Schema WHERE Name=?");
    if (stmt == nullptr)
        return false;

    if (BE_SQLITE_OK != stmt->BindText(1, schemaName, Statement::MakeCopy::No))
        return false;

    if (stmt->Step() != BE_SQLITE_ROW)
        return false;

    key = SchemaKey(stmt->GetValueText(0), stmt->GetValueInt(1), stmt->GetValueInt(2), stmt->GetValueInt(3));
    id = stmt->GetValueId<ECSchemaId>(4);
    return true;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchemaPersistenceHelper::ContainsSchemaWithAlias(ECDbCR db, Utf8CP alias)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT NULL FROM ec_Schema WHERE Alias=?"))
        return false;

    stmt->BindText(1, alias, Statement::MakeCopy::No);
    return stmt->Step() == BE_SQLITE_ROW;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   09/2016
//---------------------------------------------------------------------------------------
//static
ECClassId SchemaPersistenceHelper::GetClassId(ECDbCR db, ECSchemaId schemaId, Utf8CP className)
    {
    CachedStatementPtr stmt = db.GetCachedStatement("SELECT Id FROM ec_Class WHERE SchemaId=? AND Name=?");
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
ECClassId SchemaPersistenceHelper::GetClassId(ECDbCR db, Utf8CP schemaNameOrAlias, Utf8CP className, SchemaLookupMode lookupMode)
    {
    Utf8CP sql = nullptr;
    switch (lookupMode)
        {
            case SchemaLookupMode::ByName:
                sql = "SELECT c.Id FROM ec_Class c JOIN ec_Schema s ON c.SchemaId = s.Id WHERE s.Name=? AND c.Name=?";
                break;

            case SchemaLookupMode::ByAlias:
                sql = "SELECT c.Id FROM ec_Class c JOIN ec_Schema s ON c.SchemaId = s.Id WHERE s.Alias=? AND c.Name=?";
                break;

            default:
                sql = "SELECT c.Id FROM ec_Class c JOIN ec_Schema s ON c.SchemaId = s.Id WHERE (s.Name=?1 OR s.Alias=?1) AND c.Name=?2";
                break;
        }

    CachedStatementPtr stmt = db.GetCachedStatement(sql);
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
ECEnumerationId SchemaPersistenceHelper::GetEnumerationId(ECDbCR ecdb, Utf8CP schemaName, Utf8CP enumName)
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT e.Id FROM ec_Enumeration e, ec_Schema s WHERE e.SchemaId=s.Id AND s.Name=? AND e.Name=?");
    if (stmt == nullptr)
        return ECEnumerationId();

    stmt->BindText(1, schemaName, Statement::MakeCopy::No);
    stmt->BindText(2, enumName, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return ECEnumerationId();

    return stmt->GetValueId<ECEnumerationId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 06/2016
//---------------------------------------------------------------------------------------
//static
KindOfQuantityId SchemaPersistenceHelper::GetKindOfQuantityId(ECDbCR ecdb, Utf8CP schemaName, Utf8CP koqName)
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT koq.Id FROM ec_KindOfQuantity koq, ec_Schema s WHERE koq.SchemaId=s.Id AND s.Name=? AND koq.Name=?");
    if (stmt == nullptr)
        return KindOfQuantityId();

    stmt->BindText(1, schemaName, Statement::MakeCopy::No);
    stmt->BindText(2, koqName, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return KindOfQuantityId();

    return stmt->GetValueId<KindOfQuantityId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      05/2013
//---------------------------------------------------------------------------------------
ECPropertyId SchemaPersistenceHelper::GetPropertyId(ECDbCR db, ECClassId ecClassId, Utf8CP propertyName)
    {
    CachedStatementPtr stmt = db.GetCachedStatement("SELECT Id FROM ec_Property WHERE ClassId=? AND Name=?");
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
ECPropertyId SchemaPersistenceHelper::GetPropertyId(ECDbCR db, Utf8CP schemaName, Utf8CP className, Utf8CP propertyName)
    {
    CachedStatementPtr stmt = db.GetCachedStatement("SELECT p.Id FROM ec_Property p JOIN ec_Class c ON p.ClassId = c.Id JOIN ec_Schema s WHERE c.SchemaId = s.Id AND s.Name=? AND c.Name=? AND p.Name=?");
    if (stmt == nullptr)
        return ECPropertyId();

    stmt->BindText(1, schemaName, Statement::MakeCopy::No);
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

        if (!enumValue->GetInvariantDisplayLabel().empty())
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
            ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to import KindOfQuantity '%s'. One of its presentation units is invalid: %s.", koq.GetFullName().c_str(), presUnit.GetProblemDescription().c_str());
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
BentleyStatus SchemaPersistenceHelper::DeserializeKoqPresentationUnits(KindOfQuantityR koq, Utf8CP jsonStr)
    {
    rapidjson::Document presUnitsJson;
    if (presUnitsJson.Parse<0>(jsonStr).HasParseError())
        {
        BeAssert(false && "Could not parse KindOfQuantity PresentationUnits values JSON string.");
        return ERROR;
        }

    BeAssert(presUnitsJson.IsArray());

    for (rapidjson::Value const& presUnitJson : presUnitsJson.GetArray())
        {
        BeAssert(presUnitJson.IsString() && presUnitJson.GetStringLength() > 0);
        koq.GetPresentationUnitListR().push_back(Formatting::FormatUnitSet(presUnitJson.GetString()));
        BeAssert(!koq.GetPresentationUnitList().back().HasProblem() && "KOQ PresentationUnit could not be read. Its format is invalid");
        }

    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
