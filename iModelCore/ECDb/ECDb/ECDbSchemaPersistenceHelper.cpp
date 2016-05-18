/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaPersistenceHelper.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <rapidjson/BeRapidJson.h>
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaPersistenceHelper::ContainsECClass(ECDbCR db, ECClassCR ecClass)
    {
    if (ecClass.HasId()) //This is unsafe but since we do not delete ecclass any class that hasId does exist in db
        return true;

    const ECClassId classId = GetECClassId(db, ecClass.GetSchema().GetName().c_str(), ecClass.GetName().c_str(), ResolveSchema::BySchemaName);
    return classId.IsValid();
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                    04/2016
//---------------------------------------------------------------------------------------
bool ECDbSchemaPersistenceHelper::TryGetECSchemaKey(SchemaKey& key, ECDbCR ecdb, Utf8CP schemaName)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != ecdb.GetCachedStatement(stmt, "SELECT Name, VersionDigit1, VersionDigit2, VersionDigit3 FROM ec_Schema WHERE Name=?"))
        return false;

    if (BE_SQLITE_OK != stmt->BindText(1, schemaName, Statement::MakeCopy::No))
        return false;

    if (stmt->Step() != BE_SQLITE_ROW)
        return false;

    key = SchemaKey(stmt->GetValueText(0), stmt->GetValueInt(1), stmt->GetValueInt(2), stmt->GetValueInt(3));
    return true;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaPersistenceHelper::ContainsECSchema(ECDbCR db, ECSchemaId ecSchemaId)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT NULL FROM ec_Schema WHERE Id=?"))
        return false;

    stmt->BindId(1, ecSchemaId);
    return stmt->Step() == BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaPersistenceHelper::ContainsECSchemaWithNamespacePrefix(ECDbCR db, Utf8CP namespacePrefix)
    {
    CachedStatementPtr stmt = nullptr;
    //Although the columns used in the WHERE have COLLATE NOCASE we need to specify it in the WHERE clause again
    //to satisfy older files which were created before column COLLATE NOCASE was added to the ECDb profile tables.
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT NULL FROM ec_Schema WHERE NamespacePrefix=? COLLATE NOCASE"))
        return false;

    stmt->BindText(1, namespacePrefix, Statement::MakeCopy::No);
    return stmt->Step() == BE_SQLITE_ROW;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Casey.Mullen      01/2013
//---------------------------------------------------------------------------------------
ECSchemaId ECDbSchemaPersistenceHelper::GetECSchemaId(ECDbCR db, Utf8CP schemaName)
    {
    CachedStatementPtr stmt = nullptr;
    //Although the columns used in the WHERE have COLLATE NOCASE we need to specify it in the WHERE clause again
    //to satisfy older files which were created before column COLLATE NOCASE was added to the ECDb profile tables.
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT Id FROM ec_Schema WHERE Name=? COLLATE NOCASE"))
        return ECSchemaId();

    stmt->BindText(1, schemaName, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return ECSchemaId();

    return stmt->GetValueId<ECSchemaId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Casey.Mullen      01/2013
//---------------------------------------------------------------------------------------
//static
ECClassId ECDbSchemaPersistenceHelper::GetECClassId(ECDbCR db, Utf8CP schemaNameOrPrefix, Utf8CP className, ResolveSchema resolveSchema)
    {
    Utf8CP sql = nullptr;
    switch (resolveSchema)
        {
        //Although the columns used in the WHERE have COLLATE NOCASE we need to specify it in the WHERE clause again
        //to satisfy older files which were created before column COLLATE NOCASE was added to the ECDb profile tables.
            case ResolveSchema::BySchemaName:
                sql = "SELECT c.Id FROM ec_Class c JOIN ec_Schema s WHERE c.SchemaId = s.Id AND s.Name=? COLLATE NOCASE AND c.Name=? COLLATE NOCASE";
                break;

            case ResolveSchema::BySchemaNamespacePrefix:
                sql = "SELECT c.Id FROM ec_Class c JOIN ec_Schema s WHERE c.SchemaId = s.Id AND s.NamespacePrefix=? COLLATE NOCASE AND c.Name=? COLLATE NOCASE";
                break;

            default:
                sql = "SELECT c.Id FROM ec_Class c JOIN ec_Schema s WHERE c.SchemaId = s.Id AND (s.Name=?1 COLLATE NOCASE OR s.NamespacePrefix=?1 COLLATE NOCASE) AND c.Name=?2 COLLATE NOCASE";
                break;
        }

    CachedStatementPtr stmt = db.GetCachedStatement(sql);
    if (stmt == nullptr)
        return ECClassId();

    stmt->BindText(1, schemaNameOrPrefix, Statement::MakeCopy::No);
    stmt->BindText(2, className, Statement::MakeCopy::No);
    if (BE_SQLITE_ROW != stmt->Step())
        return ECClassId();

    return stmt->GetValueId<ECClassId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Casey.Mullen      01/2013
//---------------------------------------------------------------------------------------
//static
ECEnumerationId ECDbSchemaPersistenceHelper::GetECEnumerationId(ECDbCR ecdb, Utf8CP schemaName, Utf8CP enumName)
    {
    CachedStatementPtr stmt = nullptr;
    //Although the columns used in the WHERE have COLLATE NOCASE we need to specify it in the WHERE clause again
    //to satisfy older files which were created before column COLLATE NOCASE was added to the ECDb profile tables.
    if (BE_SQLITE_OK != ecdb.GetCachedStatement(stmt, "SELECT e.Id FROM ec_Enumeration e, ec_Schema s WHERE e.SchemaId=s.Id AND s.Name=? COLLATE NOCASE AND e.Name=? COLLATE NOCASE"))
        return ECEnumerationId();

    stmt->BindText(1, schemaName, Statement::MakeCopy::No);
    stmt->BindText(2, enumName, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return ECEnumerationId();

    return stmt->GetValueId<ECEnumerationId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      05/2013
//---------------------------------------------------------------------------------------
ECPropertyId ECDbSchemaPersistenceHelper::GetECPropertyId(ECDbCR db, Utf8CP schemaName, Utf8CP className, Utf8CP propertyName)
    {
    CachedStatementPtr stmt = nullptr;
    //Although the columns used in the WHERE have COLLATE NOCASE we need to specify it in the WHERE clause again
    //to satisfy older files which were created before column COLLATE NOCASE was added to the ECDb profile tables.
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT p.Id FROM ec_Property p INNER JOIN ec_Class c ON p.ClassId = c.Id INNER JOIN ec_Schema s WHERE c.SchemaId = s.Id AND s.Name=? COLLATE NOCASE AND c.Name=? COLLATE NOCASE AND p.Name=? COLLATE NOCASE"))
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
BentleyStatus ECDbSchemaPersistenceHelper::SerializeRelationshipKeyProperties(Utf8StringR jsonStr, bvector<Utf8String> const& keyPropNames)
    {
    if (keyPropNames.empty())
        return SUCCESS;

    rapidjson::Document keyPropJson;
    auto& allocator = keyPropJson.GetAllocator();
    keyPropJson.SetArray();
    keyPropJson.Reserve((rapidjson::SizeType) keyPropNames.size(), allocator);

    for (Utf8StringCR keyPropertyName : keyPropNames)
        {
        keyPropJson.PushBack(rapidjson::StringRef(keyPropertyName.c_str()), allocator);
        }

    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    keyPropJson.Accept(writer);

    jsonStr.assign(buf.GetString());
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2016
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSchemaPersistenceHelper::DeserializeRelationshipKeyProperties(ECRelationshipConstraintClassR constraintClass, Utf8CP jsonStr)
    {
    rapidjson::Document d;
    if (d.Parse<0>(jsonStr).HasParseError())
        {
        BeAssert(false && "Could not parse KeyProperty JSON string.");
        return ERROR;
        }

    BeAssert(d.IsArray());
    const rapidjson::SizeType count = d.Size();
    for (rapidjson::SizeType i = 0; i < count; i++)
        {
        BeAssert(d[i].IsString());
        constraintClass.AddKey(d[i].GetString());
        }

    return SUCCESS;
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

        if (enumValue->GetIsDisplayLabelDefined())
            enumValueJson[METASCHEMA_ECENUMERATOR_PROPERTY_DisplayLabel] = Json::Value(enumValue->GetDisplayLabel().c_str());

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

END_BENTLEY_SQLITE_EC_NAMESPACE
