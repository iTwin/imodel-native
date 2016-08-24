/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbProfileUpgrader.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*************************************** ECDbProfileUpgrader_XXX *********************************
//=======================================================================================
// @bsiclass                                                 Affan.Khan      08/2016
//+===============+===============+===============+===============+===============+======
DbResult ECDbProfileUpgrader_3701::_Upgrade(ECDbCR ecdb) const
    {
    //Get ECSchemaId of MetaSchema
    DbResult stat = ecdb.ExecuteSql("CREATE TABLE ec_cache_ClassHasTables("
                           "Id INTEGER PRIMARY KEY,"
                           "ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,"
                           "TableId INTEGER NOT NULL REFERENCES ec_Table(Id) ON DELETE CASCADE)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_cache_ClassHasTables_ClassId ON ec_cache_ClassHasTables(ClassId);"
                           "CREATE INDEX ix_ec_cache_ClassHasTables_TableId ON ec_cache_ClassHasTables(TableId);");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_cache_ClassHierarchy
    stat = ecdb.ExecuteSql("CREATE TABLE ec_cache_ClassHierarchy("
                           "Id INTEGER PRIMARY KEY,"
                           "ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,"
                           "BaseClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat= ecdb.ExecuteSql("CREATE INDEX ix_ec_cache_ClassHierarchy_ClassId ON ec_cache_ClassHierarchy(ClassId);"
                           "CREATE INDEX ix_ec_cache_ClassHierarchy_BaseClassId ON ec_cache_ClassHierarchy(BaseClassId);");
    if (BE_SQLITE_OK != stat)
        return stat;


    stat = ECDbSchemaWriter::RepopulateClassHierarchyTable(ecdb);
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ECDbMap::RepopulateClassHasTable(ecdb);
    if (BE_SQLITE_OK != stat)
        return stat;

    return BE_SQLITE_OK;
    }

//*************************************** ECProfileUpgrader *********************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileUpgrader::AlterColumns(ECDbR ecdb, Utf8CP tableName, Utf8CP newDdlBody, bool recreateIndices, Utf8CP allColumnNamesAfter, Utf8CP matchingColumnNamesWithOldNames)
    {
    if (IsView(ecdb, tableName))
        return AlterColumnsInView(ecdb, tableName, allColumnNamesAfter);

    return AlterColumnsInTable(ecdb, tableName, newDdlBody, recreateIndices, allColumnNamesAfter, matchingColumnNamesWithOldNames);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    04/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static
/*DbResult ECDbProfileUpgrader::AlterTable(ECDbR ecdb, Utf8CP alterTableSqlScript)
    {
    Savepoint* defaultTrans = ecdb.GetDefaultTransaction();
    if (defaultTrans == nullptr)
        {
        LOG.error("ECDb profile upgrade failed: No default transaction available for the ECDb connection.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (defaultTrans->IsActive())
        {
        if (BE_SQLITE_OK != defaultTrans->Commit(nullptr))
            {
            LOG.errorv("ECDb profile upgrade failed: Could not commit default transaction before doing DB schema changes: %s", ecdb.GetLastError().c_str());
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
            }
        }
    else
        {
        if (BE_SQLITE_OK != defaultTrans->Begin(BeSQLiteTxnMode::Immediate))
            {
            LOG.errorv("ECDb profile upgrade failed: Could not begin default transaction needed for doing DB schema changes: %s", ecdb.GetLastError().c_str());
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
            }
        }

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "PRAGMA schema_version"))
        {
        LOG.errorv("ECDb profile upgrade failed: Could not read DB schema version: %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (BE_SQLITE_ROW != stmt.Step())
        {
        LOG.errorv("ECDb profile upgrade failed: Could not read DB schema version: %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    const int64_t dbSchemaVersion = stmt.GetValueInt64(0);
    stmt.Finalize();

    if (BE_SQLITE_OK != ecdb.ExecuteSql("PRAGMA writable_schema=1"))
        {
        LOG.errorv("ECDb profile upgrade failed: Could not execute 'PRAGMA writable_schema=1': %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (BE_SQLITE_OK != ecdb.ExecuteSql(alterTableSqlScript))
        {
        LOG.errorv("ECDb profile upgrade failed: Could not execute script '%s': %s", alterTableSqlScript, ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    Utf8String setNewDbSchemaVersionSql;
    setNewDbSchemaVersionSql.Sprintf("PRAGMA schema_version=%" PRId64, (int64_t) (dbSchemaVersion + 1));

    if (BE_SQLITE_OK != ecdb.ExecuteSql(setNewDbSchemaVersionSql.c_str()))
        {
        LOG.errorv("ECDb profile upgrade failed: Could not execute '%s': %s", setNewDbSchemaVersionSql.c_str(), ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (BE_SQLITE_OK != ecdb.ExecuteSql("PRAGMA writable_schema=0"))
        {
        LOG.errorv("ECDb profile upgrade failed: Could not execute 'PRAGMA writable_schema=0': %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (SUCCESS != CheckIntegrity(ecdb))
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;

    if (BE_SQLITE_OK != defaultTrans->Commit(nullptr))
        {
        LOG.errorv("ECDb profile upgrade failed: Could not commit default transaction after doing DB schema changes: %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    04/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static
BentleyStatus ECDbProfileUpgrader::CheckIntegrity(ECDbCR ecdb)
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "PRAGMA integrity_check"))
        {
        LOG.errorv("ECDb profile upgrade failed: Could not execute 'PRAGMA integrity_check': %s", ecdb.GetLastError().c_str());
        return ERROR;
        }

    while (BE_SQLITE_ROW == stmt.Step())
        {
        Utf8CP result = stmt.GetValueText(0);
        if (BeStringUtilities::StricmpAscii(result, "ok") == 0)
            return SUCCESS;

        LOG.errorv("ECDb profile upgrade failed. Integrity check error: %s", result);
        }

    return ERROR;
    }
    */
//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileUpgrader::AlterColumnsInTable(ECDbCR ecdb, Utf8CP tableName, Utf8CP newDdlBody, bool recreateIndices, Utf8CP allColumnNamesAfter, Utf8CP matchingColumnNamesWithOldNames)
    {
    Utf8String tempTableName(tableName);
    tempTableName.append("_tmp");

    std::vector<Utf8String> createIndexDdlList;
    if (recreateIndices)
        {
        auto stat = RetrieveIndexDdlListForTable(createIndexDdlList, ecdb, tableName);
        if (stat != BE_SQLITE_OK)
            return stat;
        }

    Utf8String sql;
    sql.Sprintf("ALTER TABLE %s RENAME TO %s;", tableName, tempTableName.c_str());
    auto stat = ecdb.ExecuteSql(sql.c_str());
    if (stat != BE_SQLITE_OK)
        return stat;

    sql.Sprintf("CREATE TABLE %s (%s);", tableName, newDdlBody);
    stat = ecdb.ExecuteSql(sql.c_str());
    if (stat != BE_SQLITE_OK)
        return stat;

    //if the old name list is null, the column names have not changed. This is the case
    //when only dropping columns (and not renaming columns)
    if (matchingColumnNamesWithOldNames == nullptr)
        matchingColumnNamesWithOldNames = allColumnNamesAfter;

    sql.Sprintf("INSERT INTO %s (%s) SELECT %s FROM %s;", tableName, allColumnNamesAfter, matchingColumnNamesWithOldNames, tempTableName.c_str());
    stat = ecdb.ExecuteSql(sql.c_str());
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = ecdb.DropTable(tempTableName.c_str());
    if (stat != BE_SQLITE_OK)
        return stat;

    //re-create indexes after the modification.
    //The old indexes get dropped automatically by dropping the temp table as the table rename
    //updates the index definition to point to the new table name, too.
    if (recreateIndices)
        {
        for (Utf8StringCR createIndexDdl : createIndexDdlList)
            {
            stat = ecdb.ExecuteSql(createIndexDdl.c_str());
            if (stat != BE_SQLITE_OK)
                return stat;
            }
        }

    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileUpgrader::AlterColumnsInView(ECDbCR ecdb, Utf8CP viewName, Utf8CP allColumnNamesAfter)
    {
    Utf8String sql("DROP VIEW ");
    sql.append(viewName);
    auto stat = ecdb.ExecuteSql(sql.c_str());
    if (stat != BE_SQLITE_OK)
        return stat;

    bvector<Utf8String> columnNameList;
    BeStringUtilities::Split(allColumnNamesAfter, ",", nullptr, columnNameList);

    Utf8String columnsDdl;
    const size_t columnCount = columnNameList.size();
    for (size_t i = 0; i < columnCount; i++)
        {
        auto& columnName = columnNameList[i];
        columnName.Trim();
        columnsDdl.append("NULL AS ");
        columnsDdl.append(columnName.c_str());
        if (i != columnCount - 1)
            {
            columnsDdl.append(", ");
            }
        }

    sql.Sprintf("CREATE VIEW %s AS SELECT %s LIMIT 0;", viewName, columnsDdl.c_str());
    return ecdb.ExecuteSql(sql.c_str());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileUpgrader::RetrieveIndexDdlListForTable(std::vector<Utf8String>& indexDdlList, ECDbCR ecdb, Utf8CP tableName)
    {
    BeAssert(ecdb.TableExists(tableName));

    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT sql FROM sqlite_master WHERE tbl_name=? AND type NOT IN ('table', 'view')");
    if (stmt == nullptr)
        return BE_SQLITE_ERROR;

    stmt->BindText(1, tableName, Statement::MakeCopy::No);

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        indexDdlList.push_back(Utf8String(stmt->GetValueText(0)));
        }

    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    02/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileUpgrader::DropTableOrView(ECDbCR ecdb, Utf8CP tableOrViewName)
    {
    Utf8String sql("DROP ");
    if (IsView(ecdb, tableOrViewName))
        sql.append("VIEW ");
    else
        sql.append("TABLE ");

    sql.append(tableOrViewName);

    return ecdb.ExecuteSql(sql.c_str());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
bool ECDbProfileUpgrader::IsView(ECDbCR ecdb, Utf8CP tableOrViewName)
    {
    BeAssert(ecdb.TableExists(tableOrViewName));
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT NULL FROM sqlite_master WHERE name=? AND type='view' LIMIT 1");

    stmt->BindText(1, tableOrViewName, Statement::MakeCopy::No);
    return stmt->Step() == BE_SQLITE_ROW;
    }

//*************************************** ECDbProfileSchemaUpgrader *********************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle        07/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileECSchemaUpgrader::ImportProfileSchemas(ECDbCR ecdb)
    {
    StopWatch timer(true);
    auto context = ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(ecdb.GetSchemaLocater());

    BeFileName ecdbStandardSchemasFolder(context->GetHostAssetsDirectory());
    ecdbStandardSchemasFolder.AppendToPath(L"ECSchemas");
    ecdbStandardSchemasFolder.AppendToPath(L"ECDb");
    context->AddSchemaPath(ecdbStandardSchemasFolder);

    if (SUCCESS != ReadECDbSystemSchema(*context, ecdb.GetDbFileName()))
        return BE_SQLITE_ERROR;

    SchemaKey schemaKey("ECDb_FileInfo", 2, 0, 0);
    if (SUCCESS != ReadSchemaFromDisk(*context, schemaKey, ecdb.GetDbFileName()))
        return BE_SQLITE_ERROR;

    schemaKey = SchemaKey("MetaSchema", 3, 0, 0);
    if (SUCCESS != ReadSchemaFromDisk(*context, schemaKey, ecdb.GetDbFileName()))
        return BE_SQLITE_ERROR;

    //import if already existing
    BentleyStatus importStat = ecdb.Schemas().ImportECSchemas(context->GetCache());
    timer.Stop();
    if (importStat != SUCCESS)
        {
        LOG.errorv("Creating / upgrading ECDb file failed because importing the ECDb standard ECSchemas and the MetaSchema ECSchema into the file '%s' failed.",
                   ecdb.GetDbFileName());
        return BE_SQLITE_ERROR;
        }

    if (LOG.isSeverityEnabled(NativeLogging::LOG_DEBUG))
        {
        LOG.debugv("Imported ECDb system ECSchemas and MetaSchema ECSchema into the file '%s' in %.4f msecs.",
                   ecdb.GetDbFileName(),
                   timer.GetElapsedSeconds() * 1000.0);
        }

    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    10/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
BentleyStatus ECDbProfileECSchemaUpgrader::ReadECDbSystemSchema(ECSchemaReadContextR readContext, Utf8CP ecdbFileName)
    {
    ECSchemaPtr ecdbSystemSchema = nullptr;
    const SchemaReadStatus deserializeStat = ECSchema::ReadFromXmlString(ecdbSystemSchema, GetECDbSystemECSchemaXml(), readContext);
    if (SchemaReadStatus::Success != deserializeStat)
        {
        if (SchemaReadStatus::ReferencedSchemaNotFound == deserializeStat)
            LOG.errorv("Creating / upgrading ECDb file %s failed because required standard ECSchemas could not be found.", ecdbFileName);
        else
            {
            //other error codes are considered programmer errors and therefore have an assertion, too
            LOG.errorv("Creating / upgrading ECDb file %s failed because ECDb_System ECSchema could not be deserialized. Error code SchemaReadStatus::%d", ecdbFileName, Enum::ToInt(deserializeStat));
            BeAssert(false && "ECDb upgrade: Failed to deserialize ECDb_System ECSchema");
            }

        return ERROR;
        }

    return SUCCESS;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    04/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static
BentleyStatus ECDbProfileECSchemaUpgrader::ReadSchemaFromDisk(ECSchemaReadContextR readContext, SchemaKey& schemaKey, Utf8CP ecdbFileName)
    {
    ECSchemaPtr schema = readContext.LocateSchema(schemaKey, SchemaMatchType::LatestCompatible);
    if (schema == nullptr)
        {
        LOG.errorv("Creating / upgrading ECDb file %s failed because required ECSchema '%s' could not be found.", ecdbFileName,
                   schemaKey.GetFullSchemaName().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2012
//+---------------+---------------+---------------+---------------+---------------+-
//static
Utf8CP ECDbProfileECSchemaUpgrader::GetECDbSystemECSchemaXml()
    {
    return "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='ECDb_System' nameSpacePrefix='ecdbsys' version='3.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.13' prefix='bsca' /> "
        "    <ECSchemaReference name='ECDbMap' version='01.00.01' prefix='ecdbmap' /> "
        "    <ECCustomAttributes> "
        "         <SystemSchema xmlns='Bentley_Standard_CustomAttributes.01.13'/> "
        "    </ECCustomAttributes> "
        "    <ECEntityClass typeName='PrimitiveArray' modifier='Abstract'> "
        "        <ECCustomAttributes> "
        "            <ClassMap xmlns='ECDbMap.01.00.01'> "
        "                <MapStrategy>"
        "                   <Strategy>NotMapped</Strategy>"
        "                   <AppliesToSubclasses>True</AppliesToSubclasses> "
        "                </MapStrategy>"
        "            </ClassMap> "
        "        </ECCustomAttributes> "
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='BinaryArray' modifier='Sealed'> "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='binary'/> "
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='BooleanArray' modifier='Sealed' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='boolean'/> "
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='DateTimeArray' modifier='Sealed' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='dateTime'/> "
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='DoubleArray' modifier='Sealed' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='double'/> "
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='IntegerArray' modifier='Sealed' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='int'/> "
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='GeometryArray' modifier='Sealed' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='Bentley.Geometry.Common.IGeometry'/> "
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='LongArray' modifier='Sealed' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='long'/> "
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='Point2dArray' modifier='Sealed' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='point2d'/> "
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='Point3dArray' modifier='Sealed'> "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='point3d'/> "
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='StringArray' modifier='Sealed' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='string'/> "
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='ECSqlSystemProperties' modifier='Abstract' >"
        "       <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00.01'>"
        "                <MapStrategy>"
        "                   <Strategy>NotMapped</Strategy>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='ECInstanceId' typeName='long' description='Represents the ECInstanceId system property used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='SourceECInstanceId' typeName='long' description='Represents the SourceECInstanceId system property of an ECRelationship used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='SourceECClassId' typeName='long' description='Represents the SourceECClassId system property of an ECRelationship used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='TargetECInstanceId' typeName='long' description='Represents the TargetECInstanceId system property of an ECRelationship used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='TargetECClassId' typeName='long' description='Represents the TargetECClassId system property of an ECRelationship used by the EC->DB Mapping.' />"
        "    </ECEntityClass> "
        "</ECSchema>";
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
