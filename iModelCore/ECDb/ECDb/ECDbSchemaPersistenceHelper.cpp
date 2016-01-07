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
    if (ecClass.HasId()) //This is unsafe but since we donot delete ecclass any class that hasId does exist in db
        return true;

    const ECClassId classId = GetECClassId(db, Utf8String(ecClass.GetSchema().GetName().c_str()).c_str(),
                                            Utf8String(ecClass.GetName().c_str()).c_str(), ResolveSchema::BySchemaName);

    return classId > ECClass::UNSET_ECCLASSID;
    }
    
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaPersistenceHelper::GetECSchemaKeys(ECSchemaKeys& keys, ECDbCR db)
    {
    keys.clear();
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT Id, Name, VersionMajor, VersionMinor, DisplayLabel FROM ec_Schema ORDER BY Name"))
        return ERROR;

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        keys.push_back(
            ECSchemaKey(
            stmt->GetValueInt64(0),
            stmt->GetValueText(1),
            (uint32_t) stmt->GetValueInt(2),
            (uint32_t) stmt->GetValueInt(3),
            stmt->IsColumnNull(4) ? (Utf8CP)nullptr : stmt->GetValueText(4)));
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaPersistenceHelper::ContainsECSchema(ECDbCR db, ECSchemaId ecSchemaId)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT NULL FROM ec_Schema WHERE Id = ?"))
        return false;

    stmt->BindInt64 (1, ecSchemaId);
    return stmt->Step () == BE_SQLITE_ROW;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Casey.Mullen      01/2013
//---------------------------------------------------------------------------------------
ECSchemaId ECDbSchemaPersistenceHelper::GetECSchemaId(ECDbCR db, Utf8CP schemaName)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT Id FROM ec_Schema WHERE Name = ?"))
        return 0LL;

    stmt->BindText (1, schemaName, Statement::MakeCopy::No);
    
    if (BE_SQLITE_ROW != stmt->Step())
        return 0LL;

    return stmt->GetValueInt64(0);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaPersistenceHelper::GetECClassKeys(ECClassKeys& keys, ECSchemaId schemaId, ECDbCR db) // WIP_FNV: take a name, instead... and modify the where clause
    {
    keys.clear ();
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT Id, Name, DisplayLabel FROM ec_Class WHERE SchemaId = ? ORDER BY Name"))
        return ERROR;

    stmt->BindInt64 (1, schemaId);
    while (stmt->Step () == BE_SQLITE_ROW)
        {
        keys.push_back (
            ECClassKey (
            stmt->GetValueInt64 (0),
            stmt->GetValueText (1),
            (stmt->IsColumnNull (2) ? (Utf8CP)nullptr : stmt->GetValueText (2))));
        }

    return SUCCESS;
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
                sql = "SELECT c.Id FROM ec_Class c JOIN ec_Schema s WHERE c.SchemaId = s.Id AND s.Name = ? AND c.Name = ?";
                break;

            case ResolveSchema::BySchemaNamespacePrefix:
                sql = "SELECT c.Id FROM ec_Class c JOIN ec_Schema s WHERE c.SchemaId = s.Id AND s.NamespacePrefix = ? AND c.Name = ?";
                break;

            default:
                sql = "SELECT c.Id FROM ec_Class c JOIN ec_Schema s WHERE c.SchemaId = s.Id AND (s.Name = ?1 OR s.NamespacePrefix = ?1) AND c.Name = ?2";
                break;
        }

    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, sql))
        return ECClass::UNSET_ECCLASSID;

    stmt->BindText(1, schemaNameOrPrefix, Statement::MakeCopy::No);
    stmt->BindText (2, className, Statement::MakeCopy::No);
    if (BE_SQLITE_ROW != stmt->Step())
        return ECClass::UNSET_ECCLASSID;

    return stmt->GetValueInt64 (0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      05/2013
//---------------------------------------------------------------------------------------
ECPropertyId ECDbSchemaPersistenceHelper::GetECPropertyId(ECDbCR db, Utf8CP schemaName, Utf8CP className, Utf8CP propertyName)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT p.Id FROM ec_Property p INNER JOIN ec_Class c ON p.ClassId = c.Id INNER JOIN ec_Schema s WHERE c.SchemaId = s.Id AND s.Name = ? AND c.Name = ? AND p.Name = ?"))
        return 0LL;

    stmt->BindText (1, schemaName, Statement::MakeCopy::No);
    stmt->BindText (2, className, Statement::MakeCopy::No);
    stmt->BindText (3, propertyName, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return 0LL;

    return stmt->GetValueInt64(0);
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
