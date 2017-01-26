/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaPersistenceHelper.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Casey.Mullen      01/2013
//---------------------------------------------------------------------------------------
ECSchemaId ECDbSchemaPersistenceHelper::GetECSchemaId(ECDbCR db, Utf8CP schemaName)
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
// @bsimethod                                 Affan.Khan                    04/2016
//---------------------------------------------------------------------------------------
bool ECDbSchemaPersistenceHelper::TryGetECSchemaKey(SchemaKey& key, ECDbCR ecdb, Utf8CP schemaName)
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
bool ECDbSchemaPersistenceHelper::TryGetECSchemaKeyAndId(SchemaKey& key, ECSchemaId& id, ECDbCR ecdb, Utf8CP schemaName)
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
bool ECDbSchemaPersistenceHelper::ContainsECSchemaWithAlias(ECDbCR db, Utf8CP alias)
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
ECClassId ECDbSchemaPersistenceHelper::GetECClassId(ECDbCR db, ECSchemaId schemaId, Utf8CP className)
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
ECClassId ECDbSchemaPersistenceHelper::GetECClassId(ECDbCR db, Utf8CP schemaNameOrAlias, Utf8CP className, ResolveSchema resolveSchema)
    {
    Utf8CP sql = nullptr;
    switch (resolveSchema)
        {
            case ResolveSchema::BySchemaName:
                sql = "SELECT c.Id FROM ec_Class c JOIN ec_Schema s ON c.SchemaId = s.Id WHERE s.Name=? AND c.Name=?";
                break;

            case ResolveSchema::BySchemaAlias:
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
ECEnumerationId ECDbSchemaPersistenceHelper::GetECEnumerationId(ECDbCR ecdb, Utf8CP schemaName, Utf8CP enumName)
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
KindOfQuantityId ECDbSchemaPersistenceHelper::GetKindOfQuantityId(ECDbCR ecdb, Utf8CP schemaName, Utf8CP koqName)
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
ECPropertyId ECDbSchemaPersistenceHelper::GetECPropertyId(ECDbCR db, ECClassId ecClassId, Utf8CP propertyName)
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
ECPropertyId ECDbSchemaPersistenceHelper::GetECPropertyId(ECDbCR db, Utf8CP schemaName, Utf8CP className, Utf8CP propertyName)
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
BentleyStatus ECDbSchemaPersistenceHelper::SerializeECEnumerationValues(Utf8StringR jsonStr, ECEnumerationCR ecEnum)
    {
    Json::Value enumValuesJson(Json::arrayValue);
    BeAssert(ecEnum.GetEnumeratorCount() > 0);
    for (ECEnumerator const* enumValue : ecEnum.GetEnumerators())
        {
        Json::Value enumValueJson(Json::objectValue);

        Json::Value val;
        if (enumValue->IsInteger())
            enumValueJson[METASCHEMA_ECENUMERATOR_PROPERTY_IntValue] = Json::Value(enumValue->GetInteger());
        else if (enumValue->IsString())
            enumValueJson[METASCHEMA_ECENUMERATOR_PROPERTY_StringValue] = Json::Value(enumValue->GetString().c_str());
        else
            {
            BeAssert(false && "Code needs to be updated as ECEnumeration seems to support types other than int and string.");
            return ERROR;
            }

        if (!enumValue->GetInvariantDisplayLabel().empty())
            enumValueJson[METASCHEMA_ECENUMERATOR_PROPERTY_DisplayLabel] = Json::Value(enumValue->GetInvariantDisplayLabel().c_str());

        enumValuesJson.append(enumValueJson);
        }

    Json::FastWriter writer;
    jsonStr = writer.write(enumValuesJson);
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2016
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSchemaPersistenceHelper::DeserializeECEnumerationValues(ECEnumerationR ecEnum, Utf8CP jsonStr)
    {
    Json::Value enumValuesJson;
    Json::Reader reader;
    if (!reader.Parse(jsonStr, enumValuesJson, false))
        {
        BeAssert(false && "Could not parse ECEnumeration values JSON string.");
        return ERROR;
        }

    BeAssert(enumValuesJson.isArray());

    for (JsonValueCR enumValueJson : enumValuesJson)
        {
        BeAssert(enumValueJson.isObject());
        BeAssert(enumValueJson.isMember(METASCHEMA_ECENUMERATOR_PROPERTY_IntValue) || enumValueJson.isMember(METASCHEMA_ECENUMERATOR_PROPERTY_StringValue));

        ECEnumeratorP enumValue = nullptr;

        if (enumValueJson.isMember(METASCHEMA_ECENUMERATOR_PROPERTY_IntValue))
            {
            JsonValueCR intVal = enumValueJson[METASCHEMA_ECENUMERATOR_PROPERTY_IntValue];
            BeAssert(intVal.isInt());
            if (ECObjectsStatus::Success != ecEnum.CreateEnumerator(enumValue, intVal.asInt()))
                return ERROR;
            }
        else if (enumValueJson.isMember(METASCHEMA_ECENUMERATOR_PROPERTY_StringValue))
            {
            JsonValueCR stringVal = enumValueJson[METASCHEMA_ECENUMERATOR_PROPERTY_StringValue];
            BeAssert(stringVal.isString());
            if (ECObjectsStatus::Success != ecEnum.CreateEnumerator(enumValue, stringVal.asCString()))
                return ERROR;
            }
        else
            {
            BeAssert(false && "Unsupported underlying ECEnumeration type");
            return ERROR;
            }

        if (enumValueJson.isMember(METASCHEMA_ECENUMERATOR_PROPERTY_DisplayLabel))
            {
            Utf8CP displayLabel = enumValueJson[METASCHEMA_ECENUMERATOR_PROPERTY_DisplayLabel].asCString();
            enumValue->SetDisplayLabel(displayLabel);
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  06/2016
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSchemaPersistenceHelper::SerializeKoqAlternativePresentationUnits(Utf8StringR jsonStr, KindOfQuantityCR koq)
    {
    Json::Value altPresUnitsJson(Json::arrayValue);
    BeAssert(!koq.GetAlternativePresentationUnitList().empty());
    for (Utf8StringCR altUnit : koq.GetAlternativePresentationUnitList())
        {
        altPresUnitsJson.append(Json::Value(altUnit.c_str()));
        }

    Json::FastWriter writer;
    jsonStr = writer.write(altPresUnitsJson);
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  06/2016
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSchemaPersistenceHelper::DeserializeKoqAlternativePresentationUnits(KindOfQuantityR koq, Utf8CP jsonStr)
    {
    Json::Value altPresUnitsJson;
    Json::Reader reader;
    if (!reader.Parse(jsonStr, altPresUnitsJson, false))
        {
        BeAssert(false && "Could not parse KindOfQuantity AlternativePresentationUnits values JSON string.");
        return ERROR;
        }

    BeAssert(altPresUnitsJson.isArray());

    for (JsonValueCR altPresUnitJson : altPresUnitsJson)
        {
        BeAssert(altPresUnitJson.isString());
        koq.GetAlternativePresentationUnitListR().push_back(altPresUnitJson.asCString());
        }

    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
