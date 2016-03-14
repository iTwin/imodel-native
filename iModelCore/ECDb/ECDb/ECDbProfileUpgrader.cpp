/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbProfileUpgrader.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*************************************** ECDbProfileUpgrader_XXX *********************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle        03/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3201::_Upgrade(ECDbR ecdb) const
    {
    if (BE_SQLITE_DONE != ecdb.ExecuteSql("DELETE FROM ec_Property WHERE lower(Name) IN ('parentecinstanceid','ecpropertypathid','ecarrayindex') AND "
                                     "ClassId IN (SELECT c.Id FROM ec_Class c, ec_Schema s "
                                     "WHERE c.SchemaId=s.Id AND lower(s.Name) LIKE 'ecdb_system' AND lower(c.Name) LIKE 'ecsqlsystemproperties')"))
        {
        LOG.errorv("ECDb profile upgrade failed for '%s': Executing SQL to delete obsolete ECProperties 'ParentECInstanceId', 'ECPropertyPathId' and 'ECArrayIndex' "
                   "from ECClass 'ECDb_System.ECSqlSystemProperties' failed. Error: %s", ecdb.GetDbFileName(), ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    LOG.debugv("ECDb profile upgrade: Deleted obsolete ECProperties obsolete ECProperties 'ParentECInstanceId', 'ECPropertyPathId' and 'ECArrayIndex' "
               "from ECClass 'ECDb_System.ECSqlSystemProperties'. %d rows deleted.", ecdb.GetModifiedRowCount());
    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle        03/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3200::_Upgrade(ECDbR ecdb) const
    {
    //Detect struct array properties
/*    Utf8String structArrayPropsSql;
    structArrayPropsSql.Sprintf("SELECT NULL from ec_Property p, ec_ClassMap cm WHERE p.ClassId=cm.ClassId AND cm.MapStrategy<>%d AND p.Kind=%d",
                                Enum::ToInt(ECDbMapStrategy::Strategy::NotMapped), Enum::ToInt(ECPropertyKind::StructArray));
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, structArrayPropsSql.c_str()))
        {
        LOG.errorv("ECDb profile upgrade failed for '%s': Preparing SQL to find struct array properties failed. SQL: %s. Error: %s", ecdb.GetDbFileName(), structArrayPropsSql.c_str(), ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (BE_SQLITE_DONE != stmt.Step())
        { 
        LOG.errorv("Version of ECDb profile of file '%s' is too old: ECDb file has ECSchemas with struct array properties mapped to separate tables. Auto-upgrade not available yet to migrate struct array properties to the JSON based persistence.",
                   ecdb.GetDbFileName());
        return BE_SQLITE_ERROR_ProfileTooOld;
        }

    stmt.Finalize();

    //Delete struct array delete triggers
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "SELECT Name FROM sqlite_master WHERE type='trigger' AND Name LIKE '%_StructArray_Delete'"))
        {
        LOG.errorv("ECDb profile upgrade failed for '%s'. Preparing SQL to find struct array delete triggers failed. SQL: %s. Error: %s", 
                   ecdb.GetDbFileName(), stmt.GetSql(), ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    Utf8CP dropTriggerSql = "DROP TRIGGER %s";
    while (BE_SQLITE_ROW == stmt.Step())
        {
        Utf8CP triggerName = stmt.GetValueText(0);
        Utf8String sql;
        sql.Sprintf(dropTriggerSql, triggerName);
        if (BE_SQLITE_DONE != ecdb.ExecuteSql(sql.c_str()))
            {
            LOG.errorv("ECDb profile upgrade failed for '%s'. Deleting struct array trigger %s failed. Error: %s",
                       ecdb.GetDbFileName(), triggerName, ecdb.GetLastError().c_str());
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
            }

        LOG.debugv("ECDb profile upgrade: Dropped trigger %s.", triggerName);
        }

    stmt.Finalize();

    //Delete struct array tables
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "SELECT Name FROM ec_Table WHERE Type=3"))
        {
        LOG.errorv("ECDb profile upgrade failed for '%s'. Preparing SQL to delete struct array tables failed. SQL: %s. Error: %s",
                   ecdb.GetDbFileName(), stmt.GetSql(), ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    while (BE_SQLITE_ROW == stmt.Step())
        {
        Utf8CP tableName = stmt.GetValueText(0);
        if (BE_SQLITE_OK != ecdb.DropTable(tableName))
            {
            LOG.errorv("ECDb profile upgrade failed for '%s'. Deleting struct array table %s failed. Error: %s",
                       ecdb.GetDbFileName(), tableName, ecdb.GetLastError().c_str());
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
            }

        LOG.debugv("ECDb profile upgrade: Drop struct array table %s.", tableName);
        }

    stmt.Finalize();

    //delete entries in ec_Table. This also deletes respective entries in ec_Column, ec_PropertyMap and ec_PropertyPath (via FK)
    if (BE_SQLITE_DONE != ecdb.ExecuteSql("DELETE FROM ec_Table WHERE Type=3"))
        {
        LOG.errorv("ECDb profile upgrade failed for '%s'. Executing SQL to delete struct array entries in ec_Table failed: %s", 
                   ecdb.GetDbFileName(), ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    LOG.debugv("ECDb profile upgrade: Deleted struct array entries from ec_Table. Rows deleted: %d", ecdb.GetModifiedRowCount());
    return BE_SQLITE_OK;
    */
    LOG.errorv("Version of ECDb profile of file '%s' is too old and an auto-upgrade is not available.", ecdb.GetDbFileName());
    return BE_SQLITE_ERROR_ProfileTooOld;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle        02/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3100::_Upgrade(ECDbR ecdb) const
    {
    //shared columns now have a dedicated ColumnKind::SharedDataColumn. So update all sharead column entries in ec_Column.
    //Identifying shared columns in the previous version is done by looking for columns with ColumnKind::DataColumn, Type::Any
    //and a table created by ECDb (-> not an existing table)
    const int dataColKindInt = Enum::ToInt(ColumnKind::DataColumn);
    
    Utf8String sql;
    sql.Sprintf("UPDATE ec_Column SET ColumnKind=%d WHERE ColumnKind & %d=%d AND Type=%d AND TableId IN (SELECT t.Id FROM ec_Table t WHERE t.Type<>%d)", Enum::ToInt(ColumnKind::SharedDataColumn),
                dataColKindInt, dataColKindInt, Enum::ToInt(ECDbSqlColumn::Type::Any), Enum::ToInt(TableType::Existing));

    Statement stmt;
    DbResult stat = stmt.Prepare(ecdb, sql.c_str());
    if (stat != BE_SQLITE_OK)
        {
        LOG.errorv("ECDb profile upgrade failed: Preparing SQL '%s' failed: %s", sql.c_str(), ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    stat = stmt.Step();
    if (stat != BE_SQLITE_DONE)
        {
        LOG.errorv("ECDb profile upgrade failed: Executing SQL '%s' failed: %s", sql.c_str(), ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    LOG.debugv("ECDb profile upgrade: Table 'ec_Column': changed ColumnKind from 'DataColumn' to 'SharedDataColumn' in %d rows.",
               ecdb.GetModifiedRowCount());
    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle        02/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3001::_Upgrade(ECDbR ecdb) const
    {
    const DbResult stat = ecdb.ExecuteSql("ALTER TABLE ec_ClassMap ADD COLUMN MapStrategyMinSharedColumnCount INTEGER;");
    if (stat != BE_SQLITE_OK)
        {
        LOG.error("ECDb profile upgrade failed: Adding column 'MapStrategyMinSharedColumnCount' to table 'ec_ClassMap' failed.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    LOG.debug("ECDb profile upgrade: Table 'ec_ClassMap': added column 'MapStrategyMinSharedColumnCount'.");
    return BE_SQLITE_OK;
    }


//*************************************** ECProfileUpgrader *********************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileUpgrader::AlterColumns (ECDbR ecdb, Utf8CP tableName, Utf8CP newDdlBody, bool recreateIndices, Utf8CP allColumnNamesAfter,  Utf8CP matchingColumnNamesWithOldNames)
    {
    if (IsView (ecdb, tableName))
        return AlterColumnsInView (ecdb, tableName, allColumnNamesAfter);

    return AlterColumnsInTable (ecdb, tableName, newDdlBody, recreateIndices, allColumnNamesAfter, matchingColumnNamesWithOldNames);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileUpgrader::AlterColumnsInTable (ECDbR ecdb, Utf8CP tableName, Utf8CP newDdlBody, bool recreateIndices, Utf8CP allColumnNamesAfter,  Utf8CP matchingColumnNamesWithOldNames)
    {
    Utf8String tempTableName (tableName);
    tempTableName.append ("_tmp");

    vector<Utf8String> createIndexDdlList;
    if (recreateIndices)
        {
        auto stat = RetrieveIndexDdlListForTable (createIndexDdlList, ecdb, tableName);
        if (stat != BE_SQLITE_OK)
            return stat;
        }

    Utf8String sql;
    sql.Sprintf ("ALTER TABLE %s RENAME TO %s;", tableName, tempTableName.c_str ());
    auto stat = ecdb.ExecuteSql (sql.c_str ());
    if (stat != BE_SQLITE_OK)
        return stat;

    sql.Sprintf ("CREATE TABLE %s (%s);", tableName, newDdlBody);
    stat = ecdb.ExecuteSql (sql.c_str ());
    if (stat != BE_SQLITE_OK)
        return stat;

    //if the old name list is null, the column names have not changed. This is the case
    //when only dropping columns (and not renaming columns)
    if (matchingColumnNamesWithOldNames == nullptr)
        matchingColumnNamesWithOldNames = allColumnNamesAfter;

    sql.Sprintf ("INSERT INTO %s (%s) SELECT %s FROM %s;", tableName, allColumnNamesAfter, matchingColumnNamesWithOldNames, tempTableName.c_str ());
    stat = ecdb.ExecuteSql (sql.c_str ());
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = ecdb.DropTable (tempTableName.c_str ());
    if (stat != BE_SQLITE_OK)
        return stat;

    //re-create indexes after the modification.
    //The old indexes get dropped automatically by dropping the temp table as the table rename
    //updates the index definition to point to the new table name, too.
    if (recreateIndices)
        {
        for (auto const& createIndexDdl : createIndexDdlList)
            {
            stat = ecdb.ExecuteSql (createIndexDdl.c_str ());
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
DbResult ECDbProfileUpgrader::AlterColumnsInView (ECDbR ecdb, Utf8CP viewName, Utf8CP allColumnNamesAfter)
    {
    Utf8String sql ("DROP VIEW ");
    sql.append (viewName);
    auto stat = ecdb.ExecuteSql (sql.c_str ());
    if (stat != BE_SQLITE_OK)
        return stat;

    bvector<Utf8String> columnNameList;
    BeStringUtilities::Split(allColumnNamesAfter, ",", nullptr, columnNameList);

    Utf8String columnsDdl;
    const size_t columnCount = columnNameList.size ();
    for (size_t i = 0; i < columnCount; i++)
        {
        auto& columnName = columnNameList[i];
        columnName.Trim ();
        columnsDdl.append ("NULL AS ");
        columnsDdl.append (columnName.c_str ());
        if (i != columnCount - 1)
            {
            columnsDdl.append (", ");
            }
        }

    sql.Sprintf ("CREATE VIEW %s AS SELECT %s LIMIT 0;", viewName, columnsDdl.c_str ());
    return ecdb.ExecuteSql (sql.c_str ());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileUpgrader::RetrieveIndexDdlListForTable (vector<Utf8String>& indexDdlList, ECDbR ecdb, Utf8CP tableName)
    {
    BeAssert (ecdb.TableExists (tableName));

    CachedStatementPtr stmt = nullptr;
    auto stat = ecdb.GetCachedStatement (stmt, "SELECT sql FROM sqlite_master WHERE tbl_name=? AND type NOT IN ('table', 'view')");
    if (stat != BE_SQLITE_OK)
        return stat;

    stmt->BindText (1, tableName, Statement::MakeCopy::No);

    while (stmt->Step () == BE_SQLITE_ROW)
        {
        indexDdlList.push_back (Utf8String (stmt->GetValueText (0)));
        }

    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    02/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileUpgrader::DropTableOrView (ECDbR ecdb, Utf8CP tableOrViewName)
    {
    Utf8String sql ("DROP ");
    if (IsView (ecdb, tableOrViewName))
        sql.append ("VIEW ");
    else 
        sql.append ("TABLE ");
    
    sql.append (tableOrViewName);

    return ecdb.ExecuteSql (sql.c_str ());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
bool ECDbProfileUpgrader::IsView (ECDbCR ecdb, Utf8CP tableOrViewName)
    {
    BeAssert (ecdb.TableExists (tableOrViewName));
    CachedStatementPtr stmt = nullptr;
    ecdb.GetCachedStatement (stmt, "SELECT NULL FROM sqlite_master WHERE name=? AND type='view' LIMIT 1");

    stmt->BindText (1, tableOrViewName, Statement::MakeCopy::No);
    return stmt->Step () == BE_SQLITE_ROW;
    }

//*************************************** ECDbProfileSchemaUpgrader *********************************
//static
SchemaKey ECDbProfileECSchemaUpgrader::s_ecdbfileinfoSchemaKey = SchemaKey ("ECDb_FileInfo", 2, 0);

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

    if (SUCCESS != ReadECDbFileInfoSchema(*context, ecdb.GetDbFileName()))
        return BE_SQLITE_ERROR;

    //import if already existing
    BentleyStatus importStat = ecdb.Schemas().ImportECSchemas(context->GetCache());
    timer.Stop();
    if (importStat != SUCCESS)
        {
        LOG.errorv("Creating / upgrading ECDb file failed because importing the ECDb standard ECSchemas into the file '%s' failed.",
                   ecdb.GetDbFileName());
        return BE_SQLITE_ERROR;
        }

    if (LOG.isSeverityEnabled(NativeLogging::LOG_DEBUG))
        {
        LOG.debugv("Imported ECDb system ECSchemas into the file '%s' in %.4f msecs.",
                   ecdb.GetDbFileName(),
                   timer.GetElapsedSeconds() * 1000.0);
        }

    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    10/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
BentleyStatus ECDbProfileECSchemaUpgrader::ReadECDbSystemSchema (ECSchemaReadContextR readContext, Utf8CP ecdbFileName)
    {
    ECSchemaPtr ecdbSystemSchema = nullptr;
    auto deserializeStat = ECSchema::ReadFromXmlString (ecdbSystemSchema, GetECDbSystemECSchemaXml (), readContext);
    if (SchemaReadStatus::Success != deserializeStat)
        {
        if (SchemaReadStatus::ReferencedSchemaNotFound == deserializeStat)
            LOG.errorv ("Creating / upgrading ECDb file %s failed because required standard ECSchemas could not be found.", ecdbFileName);
        else
            {
            //other error codes are considered programmer errors and therefore have an assertion, too
            LOG.errorv ("Creating / upgrading ECDb file %s failed because ECDb_System ECSchema could not be deserialized. Error code SchemaReadStatus::%d", ecdbFileName, deserializeStat);
            BeAssert (false && "ECDb upgrade: Failed to deserialize ECDb_System ECSchema");
            }

        return ERROR;
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    10/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
BentleyStatus ECDbProfileECSchemaUpgrader::ReadECDbFileInfoSchema (ECSchemaReadContextR readContext, Utf8CP ecdbFileName)
    {
    auto schema = readContext.LocateSchema (s_ecdbfileinfoSchemaKey, SchemaMatchType::LatestCompatible);
    if (schema == nullptr)
        {
        LOG.errorv ("Creating / upgrading ECDb file %s failed because required ECSchema '%s' could not be found.", ecdbFileName,
                    Utf8String (s_ecdbfileinfoSchemaKey.GetFullSchemaName ().c_str ()).c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2012
//+---------------+---------------+---------------+---------------+---------------+-
//static
Utf8CP ECDbProfileECSchemaUpgrader::GetECDbSystemECSchemaXml ()
    {
    return "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='ECDb_System' nameSpacePrefix='ecdbsys' version='3.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.13' prefix='bsca' /> "
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' /> "
        "    <ECCustomAttributes> "
        "         <SystemSchema xmlns='Bentley_Standard_CustomAttributes.01.13'/> "
        "    </ECCustomAttributes> "
        "    <ECEntityClass typeName='PrimitiveArray' modifier='Abstract'> "
        "        <ECCustomAttributes> "
        "            <ClassMap xmlns='ECDbMap.01.00'> "
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
        "            <ClassMap xmlns='ECDbMap.01.00'>"
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
