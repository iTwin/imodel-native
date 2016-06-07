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

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECSchemaId ECDbSchemaPersistenceHelper::GetECSchemaId(ECDbCR db, ECSchemaCR ecSchema)
    {
    if (ecSchema.HasId())
        {
        BeAssert(GetECSchemaId(db, ecSchema.GetName().c_str()).IsValid());
        return ecSchema.GetId();
        }

    const ECSchemaId schemaId = GetECSchemaId(db, ecSchema.GetName().c_str());
    if (schemaId.IsValid())
        {
        const_cast<ECSchemaR>(ecSchema).SetId(schemaId);
        LOG.debugv("ECSchema '%s' exists in the ECDb file, but its ECSchema C++ object wasn't assigned the ECSchemaId. Assigning it now.", ecSchema.GetName().c_str());
        BeAssert(false && "ECSchema exists in the ECDb file, but its ECSchema C++ object wasn't assigned the ECSchemaId. Assigning it now.");
        }

    return schemaId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Casey.Mullen      01/2013
//---------------------------------------------------------------------------------------
ECSchemaId ECDbSchemaPersistenceHelper::GetECSchemaId(ECDbCR db, Utf8CP schemaName)
    {
    //Although the columns used in the WHERE have COLLATE NOCASE we need to specify it in the WHERE clause again
    //to satisfy older files which were created before column COLLATE NOCASE was added to the ECDb profile tables.
    CachedStatementPtr stmt = db.GetCachedStatement("SELECT Id FROM ec_Schema WHERE Name=? COLLATE NOCASE");
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
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECClassId ECDbSchemaPersistenceHelper::GetECClassId(ECDbCR db, ECClassCR ecClass)
    {
    if (ecClass.HasId()) //This is unsafe but since we do not delete ecclass any class that hasId does exist in db
        {
        BeAssert(GetECClassId(db, ecClass.GetSchema().GetName().c_str(), ecClass.GetName().c_str(), ResolveSchema::BySchemaName).IsValid());
        return ecClass.GetId();
        }

    const ECClassId classId = GetECClassId(db, ecClass.GetSchema().GetName().c_str(), ecClass.GetName().c_str(), ResolveSchema::BySchemaName);
    if (classId.IsValid())
        {
        //it is possible that the ECClass was already imported before, but the given C++ object comes from another source.
        //in that case we assign it here on the fly.
        const_cast<ECClassR>(ecClass).SetId(classId);
        }

    return classId;
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
// @bsimethod                                                    Krischan.Eberle   01/2016
//---------------------------------------------------------------------------------------
ECEnumerationId ECDbSchemaPersistenceHelper::GetECEnumerationId(ECDbCR ecdb, ECEnumerationCR ecEnum)
    {
    if (ecEnum.HasId()) //This is unsafe but since we do not delete ecenum any class that hasId does exist in db
        {
        BeAssert(GetECEnumerationId(ecdb, ecEnum.GetSchema().GetName().c_str(), ecEnum.GetName().c_str()).IsValid());
        return ecEnum.GetId();
        }

    const ECEnumerationId id = GetECEnumerationId(ecdb, ecEnum.GetSchema().GetName().c_str(), ecEnum.GetName().c_str());
    if (id.IsValid())
        {
        //it is possible that the ECEnumeration was already imported before, but the given C++ object comes from another source.
        //in that case we assign it here on the fly.
        const_cast<ECEnumerationR>(ecEnum).SetId(id);
        }

    return id;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   01/2016
//---------------------------------------------------------------------------------------
ECEnumerationId ECDbSchemaPersistenceHelper::GetECEnumerationId(ECDbCR ecdb, Utf8CP schemaName, Utf8CP enumName)
    {
    //Although the columns used in the WHERE have COLLATE NOCASE we need to specify it in the WHERE clause again
    //to satisfy older files which were created before column COLLATE NOCASE was added to the ECDb profile tables.
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT e.Id FROM ec_Enumeration e, ec_Schema s WHERE e.SchemaId=s.Id AND s.Name=? COLLATE NOCASE AND e.Name=? COLLATE NOCASE");
    if (stmt == nullptr)
        return ECEnumerationId();

    stmt->BindText(1, schemaName, Statement::MakeCopy::No);
    stmt->BindText(2, enumName, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return ECEnumerationId();

    return stmt->GetValueId<ECEnumerationId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   06/2016
//---------------------------------------------------------------------------------------
KindOfQuantityId ECDbSchemaPersistenceHelper::GetKindOfQuantityId(ECDbCR ecdb, KindOfQuantityCR koq)
    {
    if (koq.HasId()) //This is unsafe but since we do not delete KOQ any class that hasId does exist in db
        {
        BeAssert(GetKindOfQuantityId(ecdb, koq.GetSchema().GetName().c_str(), koq.GetName().c_str()).IsValid());
        return koq.GetId();
        }

    const KindOfQuantityId id = GetKindOfQuantityId(ecdb, koq.GetSchema().GetName().c_str(), koq.GetName().c_str());
    if (id.IsValid())
        {
        //it is possible that the KOQ was already imported before, but the given C++ object comes from another source.
        //in that case we assign it here on the fly.
        const_cast<KindOfQuantityR>(koq).SetId(id);
        }

    return id;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 06/2016
//---------------------------------------------------------------------------------------
//static
KindOfQuantityId ECDbSchemaPersistenceHelper::GetKindOfQuantityId(ECDbCR ecdb, Utf8CP schemaName, Utf8CP koqName)
    {
    //Although the ec_Schema column 'Name' used in the WHERE has COLLATE NOCASE we need to specify it in the WHERE clause again
    //to satisfy older files which were created before column COLLATE NOCASE was added. This is not necessary
    //for the KOQ table which was added later
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT koq.Id FROM ec_KindOfQuantity koq, ec_Schema s WHERE koq.SchemaId=s.Id AND s.Name=? COLLATE NOCASE AND koq.Name=?");
    if (stmt == nullptr)
        return KindOfQuantityId();

    stmt->BindText(1, schemaName, Statement::MakeCopy::No);
    stmt->BindText(2, koqName, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return KindOfQuantityId();

    return stmt->GetValueId<KindOfQuantityId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   06/2016
//---------------------------------------------------------------------------------------
ECPropertyId ECDbSchemaPersistenceHelper::GetECPropertyId(ECDbCR ecdb, ECPropertyCR prop)
    {
    if (prop.HasId()) //This is unsafe but since we do not delete KOQ any class that hasId does exist in db
        {
        BeAssert(GetECPropertyId(ecdb, prop.GetClass().GetSchema().GetName().c_str(), prop.GetClass().GetName().c_str(), prop.GetName().c_str()).IsValid());
        return prop.GetId();
        }

    const ECPropertyId id = GetECPropertyId(ecdb, prop.GetClass().GetSchema().GetName().c_str(), prop.GetClass().GetName().c_str(), prop.GetName().c_str());
    if (id.IsValid())
        {
        const_cast<ECPropertyR>(prop).SetId(id);
        LOG.debugv("ECProperty '%s.%s' exists in the ECDb file, but its ECProperty C++ object wasn't assigned the ECPropertyId. Assigning it now.", prop.GetClass().GetFullName(), prop.GetName().c_str());
        BeAssert(false && "ECProperty exists in the ECDb file, but its C++ object wasn't assigned the ECProperty. Assigning it now.");
        }

    return id;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      05/2013
//---------------------------------------------------------------------------------------
ECPropertyId ECDbSchemaPersistenceHelper::GetECPropertyId(ECDbCR db, Utf8CP schemaName, Utf8CP className, Utf8CP propertyName)
    {
    //Although the columns used in the WHERE have COLLATE NOCASE we need to specify it in the WHERE clause again
    //to satisfy older files which were created before column COLLATE NOCASE was added to the ECDb profile tables.
    CachedStatementPtr stmt = db.GetCachedStatement("SELECT p.Id FROM ec_Property p JOIN ec_Class c ON p.ClassId = c.Id JOIN ec_Schema s WHERE c.SchemaId = s.Id AND s.Name=? COLLATE NOCASE AND c.Name=? COLLATE NOCASE AND p.Name=? COLLATE NOCASE");
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

        if (!enumValue->GetDisplayLabel().empty())
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
