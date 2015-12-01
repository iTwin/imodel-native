/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbProfileUpgrader.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*************************************** ECProfileUpgrader *********************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
SchemaVersion ECDbProfileUpgrader::GetTargetVersion () const
    {
    return _GetTargetVersion ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader::Upgrade (ECDbR ecdb) const
    {
    return _Upgrade (ecdb);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileUpgrader::AlterColumns (ECDbR ecdb, Utf8CP tableName, Utf8CP newDdlBody, bool recreateIndices, Utf8CP allColumnNamesAfter,  Utf8CP matchingColumnNamesWithOldNames)
    {
    if (IsView (ecdb, tableName))
        {
        return AlterColumnsInView (ecdb, tableName, allColumnNamesAfter);
        }

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
        {
        matchingColumnNamesWithOldNames = allColumnNamesAfter;
        }

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
    BeStringUtilities::Split(allColumnNamesAfter, ",", NULL, columnNameList);

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
DbResult ECDbProfileECSchemaUpgrader::ImportProfileSchemas (ECDbR ecdb, bool updateSchema)
    {
    StopWatch timer (true);
    auto context = ECSchemaReadContext::CreateContext ();
    context->AddSchemaLocater (ecdb.GetSchemaLocater ());

    BeFileName ecdbStandardSchemasFolder (context->GetHostAssetsDirectory ());
    ecdbStandardSchemasFolder.AppendToPath (L"ECSchemas");
    ecdbStandardSchemasFolder.AppendToPath (L"ECDb");
    context->AddSchemaPath (ecdbStandardSchemasFolder);

    if (BSISUCCESS != ReadECDbSystemSchema (*context, ecdb.GetDbFileName ()))
        return BE_SQLITE_ERROR;

    if (BSISUCCESS != ReadECDbFileInfoSchema (*context, ecdb.GetDbFileName ()))
        return BE_SQLITE_ERROR;

    //import / update if already existing
    ECDbSchemaManager::ImportOptions options (false, updateSchema);
    auto importStat = ecdb.Schemas ().ImportECSchemas (context->GetCache (), options);

    timer.Stop ();
    if (importStat != SUCCESS)
        {
        LOG.errorv ("Creating / upgrading ECDb file failed because importing / updating the ECDb standard ECSchemas in ECDb '%s' failed.",
            ecdb.GetDbFileName ());
        return BE_SQLITE_ERROR;
        }

    if (LOG.isSeverityEnabled (NativeLogging::LOG_DEBUG))
        {
        LOG.debugv ("Imported / updated ECDb system ECSchemas in ECDb '%s' in %.4f msecs.",
            ecdb.GetDbFileName (),
            timer.GetElapsedSeconds () * 1000.0);
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
    auto schema = readContext.LocateSchema (s_ecdbfileinfoSchemaKey, SCHEMAMATCHTYPE_LatestCompatible);
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
        "<ECSchema schemaName='ECDb_System' nameSpacePrefix='ecdbsys' version='3.0'  xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'> "
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca' /> "
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' /> "
        "    <ECCustomAttributes> "
        "         <SystemSchema xmlns='Bentley_Standard_CustomAttributes.01.12'/> "
        "    </ECCustomAttributes> "
        "    <ECClass typeName='PrimitiveArray' isDomainClass='False' isStruct='True' > "
        "        <ECCustomAttributes> "
        "            <ClassMap xmlns='ECDbMap.01.00'> "
        "                <MapStrategy>"
        "                   <Strategy>NotMapped</Strategy>"
        "                   <AppliesToSubclasses>True</AppliesToSubclasses> "
        "                </MapStrategy>"
        "            </ClassMap> "
        "        </ECCustomAttributes> "
        "    </ECClass> "
        "    <ECClass typeName='BinaryArray' isDomainClass='True' isStruct='True' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='binary'/> "
        "    </ECClass> "
        "    <ECClass typeName='BooleanArray' isDomainClass='True' isStruct='True' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='boolean'/> "
        "    </ECClass> "
        "    <ECClass typeName='DateTimeArray' isDomainClass='True' isStruct='True' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='dateTime'/> "
        "    </ECClass> "
        "    <ECClass typeName='DoubleArray' isDomainClass='True' isStruct='True' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='double'/> "
        "    </ECClass> "
        "    <ECClass typeName='IntegerArray' isDomainClass='True' isStruct='True' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='int'/> "
        "    </ECClass> "
        "    <ECClass typeName='GeometryArray' isDomainClass='True' isStruct='True' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='Bentley.Geometry.Common.IGeometry'/> "
        "    </ECClass> "
        "    <ECClass typeName='LongArray' isDomainClass='True' isStruct='True' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='long'/> "
        "    </ECClass> "
        "    <ECClass typeName='Point2dArray' isDomainClass='True' isStruct='True' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='point2d'/> "
        "    </ECClass> "
        "    <ECClass typeName='Point3dArray' isDomainClass='True' isStruct='True' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='point3d'/> "
        "    </ECClass> "
        "    <ECClass typeName='StringArray' isDomainClass='True' isStruct='True' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='string'/> "
        "    </ECClass> "
        "    <ECClass typeName='ECSqlSystemProperties' isDomainClass='False' isStruct='False' >"
        "        <ECCustomAttributes> "
        "            <ClassMap xmlns='ECDbMap.01.00'> "
        "                <MapStrategy>"
        "                   <Strategy>NotMapped</Strategy>"
        "                </MapStrategy>"
        "            </ClassMap> "
        "        </ECCustomAttributes> "
        "        <ECProperty propertyName='ECInstanceId' typeName='long' description='Represents the ECInstanceId system property used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='SourceECInstanceId' typeName='long' description='Represents the SourceECInstanceId system property of an ECRelationship used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='SourceECClassId' typeName='long' description='Represents the SourceECClassId system property of an ECRelationship used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='TargetECInstanceId' typeName='long' description='Represents the TargetECInstanceId system property of an ECRelationship used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='TargetECClassId' typeName='long' description='Represents the TargetECClassId system property of an ECRelationship used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='ParentECInstanceId' typeName='long' description='For ECClasses mapped to secondary table storage (e.g. ECStructs), this system property represents the ECInstanceId of the parent (embedding) ECInstance.' />"
        "        <ECProperty propertyName='ECPropertyPathId' typeName='long' description='For ECClasses mapped to secondary table storage, this system property represents the id of the ECProperty in the owning ECClass to which a row in the secondary table refers to.' />"
        "        <ECProperty propertyName='ECArrayIndex' typeName='int' description='For EC arrays mapped to secondary table storage, this system property represents the index of the array element which a row in the secondary table represents.' />"
        "    </ECClass> "
        "</ECSchema>";
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
