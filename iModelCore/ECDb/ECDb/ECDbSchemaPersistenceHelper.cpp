/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaPersistenceHelper.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
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
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT NULL FROM ec_Schema WHERE Id = ?"))
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
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT NULL FROM ec_Schema WHERE lower(NamespacePrefix) = lower(?)"))
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
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT Id FROM ec_Schema WHERE Name LIKE ?"))
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
            case ResolveSchema::BySchemaName:
                sql = "SELECT c.Id FROM ec_Class c JOIN ec_Schema s WHERE c.SchemaId = s.Id AND s.Name LIKE ? AND c.Name LIKE ?";
                break;

            case ResolveSchema::BySchemaNamespacePrefix:
                sql = "SELECT c.Id FROM ec_Class c JOIN ec_Schema s WHERE c.SchemaId = s.Id AND s.NamespacePrefix LIKE ? AND c.Name LIKE ?";
                break;

            default:
                sql = "SELECT c.Id FROM ec_Class c JOIN ec_Schema s WHERE c.SchemaId = s.Id AND (s.Name LIKE ?1 OR s.NamespacePrefix LIKE ?1) AND c.Name LIKE ?2";
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
    if (BE_SQLITE_OK != ecdb.GetCachedStatement(stmt, "SELECT e.Id FROM ec_Enumeration e, ec_Schema s WHERE e.SchemaId=s.Id AND s.Name LIKE ? AND e.Name LIKE ?"))
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
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT p.Id FROM ec_Property p INNER JOIN ec_Class c ON p.ClassId = c.Id INNER JOIN ec_Schema s WHERE c.SchemaId = s.Id AND s.Name LIKE ? AND c.Name LIKE ? AND p.Name LIKE ?"))
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
        keyPropJson.PushBack(keyPropertyName.c_str(), allocator);
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
    rapidjson::Document enumValuesJson;
    auto& allocator = enumValuesJson.GetAllocator();
    enumValuesJson.SetArray();
    enumValuesJson.Reserve((rapidjson::SizeType) ecEnum.GetEnumeratorCount(), allocator);
    BeAssert(ecEnum.GetEnumeratorCount() > 0);
    for (ECEnumerator const* enumValue : ecEnum.GetEnumerators())
        {
        rapidjson::Value enumValueJson(rapidjson::kArrayType);
        if (enumValue->IsInteger())
            enumValueJson.PushBack(enumValue->GetInteger(), allocator);
        else if (enumValue->IsString())
            enumValueJson.PushBack(enumValue->GetString().c_str(), allocator);
        else
            {
            BeAssert(false && "Code needs to be updated as ECEnumeration seems to support types other than int and string.");
            return ERROR;
            }

        if (enumValue->GetIsDisplayLabelDefined())
            enumValueJson.PushBack(enumValue->GetDisplayLabel().c_str(), allocator);

        enumValuesJson.PushBack(enumValueJson, allocator);
        }

    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    enumValuesJson.Accept(writer);
    jsonStr.assign(buf.GetString());
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2016
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSchemaPersistenceHelper::DeserializeECEnumerationValues(ECEnumerationR ecEnum, Utf8CP jsonStr)
    {
    rapidjson::Document enumValuesJson;
    if (enumValuesJson.Parse<0>(jsonStr).HasParseError())
        {
        BeAssert(false && "Could not parse ECEnumeration values JSON string.");
        return ERROR;
        }

    BeAssert(enumValuesJson.IsArray());
    for (auto it = enumValuesJson.Begin(); it != enumValuesJson.End(); ++it)
        {
        rapidjson::Value const& enumValueJson = *it;
        BeAssert(enumValueJson.IsArray());
        const rapidjson::SizeType enumValueMemberCount = enumValueJson.Size();
        BeAssert(enumValueMemberCount == 1 || enumValueMemberCount == 2);

        rapidjson::Value const& val = enumValueJson[(rapidjson::SizeType) 0];

        ECEnumeratorP enumValue = nullptr;

        if (val.IsInt())
            {
            if (ECObjectsStatus::Success != ecEnum.CreateEnumerator(enumValue, val.GetInt()))
                return ERROR;
            }
        else if (val.IsString())
            {
            if (ECObjectsStatus::Success != ecEnum.CreateEnumerator(enumValue, val.GetString()))
                return ERROR;
            }
        else
            {
            BeAssert(false && "Unsupported underlying ECEnumeration type");
            return ERROR;
            }

        if (enumValueMemberCount == 2)
            {
            BeAssert(enumValueJson[1].IsString());
            enumValue->SetDisplayLabel(enumValueJson[1].GetString());
            }
        }

    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
