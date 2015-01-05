/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbProfileUpgrader.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*************************************** ECProfileUpgrader_1100 *********************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle        07/2012
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_1100::_Upgrade (ECDbR ecdb) const
    {
    //****** ec_ClassMap: Drop column
    auto stat = AlterColumns (ecdb, "ec_ClassMap",
        //New DDL body
        "ECClassId INTEGER NOT NULL PRIMARY KEY REFERENCES ec_Class(ECClassId) ON DELETE CASCADE, "
        "MapParentECClassId INTEGER REFERENCES ec_Class(ECClassId), "
        "MapStrategy SMALLINT NOT NULL, "
        "MapToDbTable CHAR, "
        "PrimaryKeyColumnName CHAR",
        true, // recreate indices
        //New column names
        "ECClassId, MapParentECClassId, MapStrategy, MapToDbTable, PrimaryKeyColumnName");

    if (stat == BE_SQLITE_OK)
        LOG.debug ("ECDb profile upgrade: In table 'ec_ClassMap' removed column SQLCreateTable.");
    else
        {
        LOG.error ("ECDb profile upgrade failed: Modifying table 'ec_ClassMap' failed.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    //****** ec_RelationshipConstraint: Rename columns
    stat = AlterColumns (ecdb, "ec_RelationshipConstraint",
        //New DDL body
        "ECClassId INTEGER NOT NULL REFERENCES ec_Class(ECClassId) ON DELETE CASCADE, "
        "ECRelationshipEnd BOOL NOT NULL CHECK (ECRelationshipEnd IN (0, 1)), "
        "CardinalityLowerLimit SMALLINT, "
        "CardinalityUpperLimit SMALLINT, "
        "RoleLabel CHAR, "
        "IsPolymorphic BOOL NOT NULL CHECK (IsPolymorphic IN (0, 1)), "
        "PRIMARY KEY (ECClassId, ECRelationshipEnd)",
        true, // recreate indices
        //New column names
        "ECClassId, ECRelationshipEnd, CardinalityLowerLimit, CardinalityUpperLimit, RoleLabel, IsPolymorphic",
        //Old column names
        "ECClassId, ECRelationshipEnd, CardinalityLowerLimit, CardinalityUpperLimit, RoleLable, IsPolymorphic");
    if (stat == BE_SQLITE_OK)
        LOG.debug ("ECDb profile upgrade: In table 'ec_RelationshipConstraint' renamed column RoleLable to RoleLabel.");
    else
        {
        LOG.error ("ECDb profile upgrade failed: Modifying table 'ec_RelationshipConstraint' failed.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    //****** ec_RelationshipConstraintClass: Drop column
    stat = AlterColumns (ecdb, "ec_RelationshipConstraintClass",
        //New DDL body
        "ECClassId INTEGER NOT NULL, "
        "ECRelationshipEnd BOOL NOT NULL, "
        "RelationECClassId INTEGER NOT NULL REFERENCES ec_Class(ECClassId) ON DELETE CASCADE, "
        "PRIMARY KEY (ECClassId, ECRelationshipEnd, RelationECClassId), "
        "FOREIGN KEY (ECClassId, ECRelationshipEnd) "
        "REFERENCES ec_RelationshipConstraint(ECClassId, ECRelationshipEnd) ON DELETE CASCADE",
        true, // recreate indices
        //New column names
        "ECClassId, ECRelationshipEnd, RelationECClassId");
    if (stat == BE_SQLITE_OK)
        LOG.debug ("ECDb profile upgrade: In table 'ec_RelationshipConstraintClass' dropped column RelationKeyDbColumnName.");
    else
        {
        LOG.error ("ECDb profile upgrade failed: Modifying table 'ec_RelationshipConstraintClass' failed.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    //****** ec_Schema: Drop columns
    stat = AlterColumns (ecdb, "ec_Schema",
        //New DDL body
        "ECSchemaId INTEGER NOT NULL PRIMARY KEY, "
        "Name CHAR NOT NULL, "
        "DisplayLabel CHAR, "
        "Description CHAR, "
        "NamespacePrefix CHAR, "
        "VersionMajor SMALLINT, "
        "VersionMinor SMALLINT, "
        "SchemaType SMALLINT",
        true, // recreate indices
        //New column names
        "ECSchemaId, Name, DisplayLabel, Description, NamespacePrefix, VersionMajor, VersionMinor, SchemaType");

    if (stat == BE_SQLITE_OK)
        {
        LOG.debug ("ECDb profile upgrade: In table 'ec_Schema' dropped columns MapVersion, SchemaAsBlob, IsReadonly.");
        return BE_SQLITE_OK;
        }
    else
        {
        LOG.error ("ECDb profile upgrade failed: Modifying table 'ec_Schema' failed.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }
    }

//*************************************** ECProfileUpgrader_1004 *********************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/2014
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_1004::_Upgrade (ECDbR ecdb) const
    {
    //change namespace prefix of internal ECDbSystem schemas to be more precise, now that ECDb comes with another
    //system schema
    auto stat = ecdb.ExecuteSql ("UPDATE ec_Schema SET NamespacePrefix='ecdbsys' WHERE [Name] = 'ECDbSystem'");
    if (stat == BE_SQLITE_OK && ecdb.GetModifiedRowCount () == 1)
       LOG.debug ("ECDb profile upgrade: Changed namespace prefix of ECDbSystem schema to 'ecdbsys'.");
    else
        {
        LOG.error ("ECDb profile upgrade failed: Changing ECDbSystem schema's namespace prefix to 'ecdbsys' failed.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    //BeSQLite doesn't implicitly upgrade its profile. So we do this here as we know that we need the upgraded BeSQlite profile.
    if (BE_SQLITE_OK != ecdb.UpgradeBeSQLiteProfile ())
        {
        LOG.error ("ECDb profile upgrade failed: Creating/updating table be_EmbedFile failed.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    return BE_SQLITE_OK;
    }

//*************************************** ECProfileUpgrader_1003 *********************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        06/2014
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_1003::_Upgrade (ECDbR ecdb) const
    {
    //Improves performance when retrieving derived classes of a given class
    auto stat = ecdb.ExecuteSql ("CREATE INDEX idx_ec_BaseClass_BaseECClassId ON ec_BaseClass (BaseECClassId);");
    if (stat == BE_SQLITE_OK)
        LOG.debug ("ECDb profile upgrade: Added index on column 'BaseECClassId' in table 'ec_BaseClass'.");
    else
        {
        LOG.error ("ECDb profile upgrade failed: Adding index on column 'BaseECClassId' in table 'ec_BaseClass' failed.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    stat = CreateTablePropertyAlias (ecdb);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = AddECIdColumnToLegacyStructArrayTable (ecdb);
    if (stat != BE_SQLITE_OK)
        return stat;


    return MapTransformationValueMapClass (ecdb);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        08/2014
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_1003::CreateTablePropertyAlias (ECDb& db)
    {
    auto stat = db.ExecuteSql (
        "CREATE TABLE ec_PropertyAlias "
        "("
        "AliasECPropertyId INTEGER PRIMARY KEY, "
        "RootECClassId INTEGER  REFERENCES ec_Class(ECClassId) ON DELETE CASCADE, "
        "RootECPropertyId INTEGER  REFERENCES ec_Property(ECPropertyId) ON DELETE CASCADE, "
        "LeafECPropertyId INTEGER NOT NULL REFERENCES ec_Property(ECPropertyId) ON DELETE CASCADE, "
        "AccessString CHAR NOT NULL "
        ");");

    if (stat == BE_SQLITE_OK)
        LOG.debug ("ECDb profile upgrade: Created table ec_PropertyAlias.");
    else
        {
        LOG.error ("ECDb profile upgrade failed: Creating table ec_PropertyAlias failed.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }


    stat = db.ExecuteSql ("CREATE UNIQUE INDEX idx_ec_PropertyAliasCIDA ON ec_PropertyAlias (RootECClassId, AccessString);");
    if (stat == BE_SQLITE_OK)
        LOG.debug ("ECDb profile upgrade: Added index on columns '(RootECClassId, AccessString)' in table 'ec_PropertyAlias'.");
    else
        {
        LOG.error ("ECDb profile upgrade failed: Adding index on columns '(RootECClassId, AccessString)' in table 'ec_PropertyAlias' failed.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    return stat;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        07/2014
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_1003::AddECIdColumnToLegacyStructArrayTable (ECDbR ecdb)
    {
    auto tableToUpdateQuery = "SELECT DISTINCT MapToDbTable FROM ec_Class JOIN ec_ClassMap USING (ECClassId) JOIN sqlite_master ON sqlite_master.name = MapToDbTable WHERE IsStruct = 1 AND sqlite_master.type = 'table'";
    Statement tableStmt;
    auto status = tableStmt.Prepare (ecdb, tableToUpdateQuery);
    std::vector<Utf8String> tables;
    while (tableStmt.Step () == BE_SQLITE_ROW)
        {
        tables.push_back (tableStmt.GetValueText (0));
        }

    for (auto& tableName : tables)
        {
        Utf8String sql = SqlPrintfString ("ALTER TABLE [%s] ADD COLUMN [ECId] INTEGER", tableName.c_str()).GetUtf8CP ();
        status = ecdb.ExecuteSql (sql.c_str ());
        if (BE_SQLITE_OK != status)
            {
            LOG.errorv ("ECDb profile upgrade failed. Could not execute statement to alter struct array table : %s. SQL: %s.", ecdb.GetLastError (), sql.c_str ());
            BeAssert (false && "ECDb profile upgrade failed. Could not execute statement to alter struct array table.");
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
            }

        sql = SqlPrintfString ("UPDATE [%s] SET [ECId] = [OwnerECId]", tableName.c_str()).GetUtf8CP ();
        status = ecdb.ExecuteSql (sql.c_str ());
        if (BE_SQLITE_OK != status)
            {
            LOG.errorv ("ECDb profile upgrade failed. Could not execute update statement to set ECId =OwnerECId : %s. SQL: %s.", ecdb.GetLastError (), sql.c_str ());
            BeAssert (false && "ECDb profile upgrade failed. Could not execute update statement to set ECId =OwnerECId");
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
            }
        }

    bmap<Utf8String, Utf8String, CompareIUtf8> results;
    auto viewToUpdateQuery = "SELECT MapToDbTable, REPLACE(sql, 'LIMIT 0', ', NULL AS [ECId] LIMIT 0') ViewSql FROM ec_Class JOIN ec_ClassMap USING (ECClassId) JOIN sqlite_master ON sqlite_master.name = MapToDbTable WHERE IsStruct = 1 AND sqlite_master.type = 'view'";
    Statement viewStmt;
    status = viewStmt.Prepare (ecdb, viewToUpdateQuery);
    while (viewStmt.Step () == BE_SQLITE_ROW)
        {
        Utf8String tableName (viewStmt.GetValueText (0));
        if (results.find (tableName) == results.end ())
            {
            results[tableName] = Utf8String (viewStmt.GetValueText (1));
            }
        }

    for (auto const& kvPair : results)
        {
        Utf8CP tableName = kvPair.first.c_str ();
        Utf8CP viewSql = kvPair.second.c_str ();
        Utf8String sql = SqlPrintfString ("DROP VIEW [%s]", tableName).GetUtf8CP ();
        status = ecdb.ExecuteSql (sql.c_str ());
        if (BE_SQLITE_OK != status)
            {
            LOG.errorv ("ECDb profile upgrade failed. Could not execute statement to drop a view : %s. SQL: %s.", ecdb.GetLastError (), sql.c_str ());
            BeAssert (false && "ECDb profile upgrade failed. Could not execute statement to drop a view.");
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
            }

        status = ecdb.ExecuteSql (viewSql);
        if (BE_SQLITE_OK != status)
            {
            LOG.errorv ("ECDb profile upgrade failed. Could not execute create view statement : %s. SQL: %s.", ecdb.GetLastError (), viewSql);
            BeAssert (false && "ECDb profile upgrade failed. Could not execute create view statement.");
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
            }
        }

    return BE_SQLITE_OK;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle        06/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileUpgrader_1003::MapTransformationValueMapClass (ECDbR ecdb)
    {
    ECClassCP tvmECClass = ecdb.GetSchemaManager ().GetECClass ("Bentley_Standard_CustomAttributes", "TransformationValueMap");
    if (tvmECClass == nullptr)
        return BE_SQLITE_OK;

    //validate that the class only has the expected properties as we hard code the CREATE TABLE statement for the class.
    auto nameProp = tvmECClass->GetPropertyP ("Name", false);
    auto valueProp = tvmECClass->GetPropertyP ("Entries", false);
    if (nameProp == nullptr || !nameProp->GetIsPrimitive () ||
        valueProp == nullptr || !valueProp->GetIsArray () ||
        valueProp->GetAsArrayProperty ()->GetKind () != ArrayKind::ARRAYKIND_Struct ||
        tvmECClass->GetPropertyCount (true) != 2)
        {
        LOG.error ("ECDb profile upgrade failed. Unexpected properties in ECClass Bentley_Standard_CustomAttributes:TransformationValueMap.");
        BeAssert (false && "ECDb profile upgrade failed. Unexpected properties in ECClass Bentley_Standard_CustomAttributes:TransformationValueMap.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    //Create table for the class. As it is a struct, the table is of SecondaryTable type (table name ArrayOfXXX, and system columns OwnerECId, ECPropertyId, ECArrayIndex)
    Utf8String tableName;
    tableName.Sprintf ("%s_ArrayOf%s", 
                    Utf8String (tvmECClass->GetSchema ().GetNamespacePrefix ()).c_str (),
                    Utf8String (tvmECClass->GetName ()).c_str ());

    Utf8String createTableSql ("CREATE TABLE IF NOT EXISTS ");
    createTableSql.append (tableName).append (" (ECId INTEGER NOT NULL, OwnerECId INTEGER, ECPropertyId INTEGER, ECArrayIndex INTEGER, Name CHAR, PRIMARY KEY (ECId, OwnerECId, ECPropertyId, ECArrayIndex))");

    auto stat = ecdb.ExecuteSql (createTableSql.c_str ());
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv ("ECDb profile upgrade failed. Table for ECClass Bentley_Standard_CustomAttributes:TransformationValueMap could not be created: %s. SQL: %s", ecdb.GetLastError (), createTableSql.c_str ());
        BeAssert (false && "ECDb profile upgrade failed. Table for ECClass Bentley_Standard_CustomAttributes:TransformationValueMap could not be created.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    //Update the class mapping. Set MapStrategy to 4 (->TableForThisClass) and MapToDbTable to the table name
    //(Note: MapStrategy enum values are 2 less than the values stored in the table, because the enum was changed some time ago
    //but we did not want to change the data in the tables)
    Statement stmt;
    stat = stmt.Prepare (ecdb, "UPDATE ec_ClassMap SET MapStrategy = 4, MapToDbTable = ? WHERE ECClassId = ?");
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv ("ECDb profile upgrade failed. Could not prepare statement to update map strategy for ECClass Bentley_Standard_CustomAttributes:TransformationValueMap: %s. SQL: %s", ecdb.GetLastError (), stmt.GetSql ());
        BeAssert (false && "ECDb profile upgrade failed. Could not prepare statement to update map strategy for ECClass Bentley_Standard_CustomAttributes:TransformationValueMap.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    stmt.BindText (1, tableName, Statement::MakeCopy::No);
    stmt.BindInt64 (2, tvmECClass->GetId ());
    stat = stmt.Step ();
    if (BE_SQLITE_DONE != stat)
        {
        LOG.errorv ("ECDb profile upgrade failed. Could not execute statement to update map strategy for ECClass Bentley_Standard_CustomAttributes:TransformationValueMap: %s. SQL: %s", ecdb.GetLastError (), stmt.GetSql ());
        BeAssert (false && "ECDb profile upgrade failed. Could not execute statement to update map strategy for ECClass Bentley_Standard_CustomAttributes:TransformationValueMap.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    LOG.debugv (L"Changed mapping strategy for ECClass %ls from DoNotMap to TableForThisClass as ECDb as ECDb now supports ECClasses which are custom attributes and domain classes/structs.", tvmECClass->GetFullName ());

    ecdb.ClearCache ();
    return BE_SQLITE_OK;
    }



//*************************************** ECProfileUpgrader_1002 *********************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    02/2014
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_1002::_Upgrade (ECDbR ecdb) const
    {
    //*** ec_ClassMap: MapStrategy 0 and 1 no longer exists. Replace it by 3 (DoNotMap)
    auto stat = ecdb.ExecuteSql ("UPDATE ec_ClassMap SET MapStrategy = 3 WHERE MapStrategy IN (0, 1)");
    if (stat != BE_SQLITE_OK)
        {
        LOG.error ("ECDb profile upgrade failed: Replacing obsolete values 0 and 1 in column 'MapStrategy' in table 'ec_ClassMap' failed.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }
        
    //*** Standard classes AnyClass and InstanceCount are no longer mapped to a table
    return UnmapUnsupportedStandardClasses (ecdb);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    02/2014
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_1002::UnmapUnsupportedStandardClasses (ECDbR ecdb)
    {
    auto const& schemaManager = ecdb.GetSchemaManager ();

    std::map<Utf8String, std::vector<Utf8String>> candidateClassNames;
    candidateClassNames["Bentley_Standard_Classes"] = {"AnyClass", "InstanceCount"};
    for (auto const& kvPair : candidateClassNames)
        {
        Utf8CP schemaName = kvPair.first.c_str ();
        auto const& classNames = kvPair.second;
        for (auto const& className : classNames)
            {
            ECClassCP ecClass = schemaManager.GetECClass (schemaName, className.c_str ());
            if (ecClass == nullptr)
                continue; //was not imported, so no need to change anything

            BeAssert (ecClass != nullptr);

            Utf8String tableName;
            MapStrategy existingStrategy = MapStrategy::DoNotMap;
            if (BE_SQLITE_OK != ReadMapping (existingStrategy, tableName, ecdb, *ecClass))
                {
                LOG.errorv ("ECDb profile upgrade failed: Could not read mapping for class '%s.%s' from table ec_ClassMap.",
                    schemaName, className.c_str ());
                return BE_SQLITE_ERROR_ProfileUpgradeFailed;
                }

            if (ClassMap::IsDoNotMapStrategy (existingStrategy)) 
                continue; //is already unmapped, no need to change anything

            //1) update ec_ClassMap table with DoNotMap map strategy
            auto stat = UpdateMapStrategy (ecdb, *ecClass, MapStrategy::DoNotMap);
            if (stat != BE_SQLITE_OK)
                {
                LOG.errorv ("ECDb profile upgrade failed: Setting map strategy for class '%s.%s' to 'DoNotMap' in table ec_ClassMap failed.",
                    schemaName, className.c_str ());
                return BE_SQLITE_ERROR_ProfileUpgradeFailed;
                }

            //2) drop table to which the class used to be mapped to
            if (ecdb.TableExists (tableName.c_str ()))
                {
                stat = DropTableOrView (ecdb, tableName.c_str ());
                if (stat != BE_SQLITE_OK)
                    {
                    LOG.errorv ("ECDb profile upgrade failed: Deleting table '%s' for class '%s.%s' failed.",
                        tableName.c_str (), schemaName, className.c_str ());
                    return BE_SQLITE_ERROR_ProfileUpgradeFailed;
                    }
                }

            LOG.debugv ("ECDb profile upgrade: ECClass '%s.%s' is no longer mapped to a table.", schemaName, className.c_str ());
            }
        }

    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    05/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileUpgrader_1002::ReadMapping (MapStrategy& mapStrategy, Utf8StringR mappedTableName, ECDbR ecdb, ECN::ECClassCR ecClass)
    {
    DbECClassMapInfo info;
    info.ColsSelect = DbECClassMapInfo::COL_MapStrategy | DbECClassMapInfo::COL_MapToDbTable;
    info.ColsWhere = DbECClassMapInfo::COL_ECClassId;
    info.m_ecClassId = ecClass.GetId ();

    CachedStatementPtr stmt = nullptr;
    auto stat = ECDbSchemaPersistence::FindECClassMapInfo (stmt, ecdb, info);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = ECDbSchemaPersistence::Step (info, *stmt);
    if (stat != BE_SQLITE_ROW) //must find a row. Everything else is an error
        return stat;

    mapStrategy = info.m_mapStrategy;
    mappedTableName = info.m_mapToDbTable;

    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    02/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileUpgrader_1002::UpdateMapStrategy (ECDbR ecdb, ECN::ECClassCR ecClass, MapStrategy mapStrategy)
    {
    DbECClassMapInfo info;
    info.ColsUpdate = DbECClassMapInfo::COL_MapStrategy;
    info.m_mapStrategy = mapStrategy;

    //When updating to "Do Not Map" strategy, also clear out the mapped table name.
    if (ClassMap::IsDoNotMapStrategy (mapStrategy))
        {
        info.ColsUpdate |= DbECClassMapInfo::COL_MapToDbTable;
        info.ColsNull |= DbECClassMapInfo::COL_MapToDbTable;
        }

    info.ColsWhere = DbECClassMapInfo::COL_ECClassId;
    info.m_ecClassId = ecClass.GetId ();

    auto stat = ECDbSchemaPersistence::UpdateECClassMapInfo (ecdb, info);
    if (stat == BE_SQLITE_DONE)
        return BE_SQLITE_OK;
    else
        return stat;
    }


//*************************************** ECProfileUpgrader_1001 *********************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle        07/2012
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_1001::_Upgrade (ECDbR ecdb) const
    {
    //****** ec_Property: Add columns
    auto stat = ecdb.ExecuteSql ("ALTER TABLE ec_Property ADD COLUMN MinOccurs INTEGER;"
        "ALTER TABLE ec_Property ADD COLUMN MaxOccurs INTEGER;");
    if (stat == BE_SQLITE_OK)
        LOG.debug ("ECDb profile upgrade: In table 'ec_Property' added columns MinOccurs and MaxOccurs.");
    else
        {
        LOG.error ("ECDb profile upgrade failed: Modifying table 'ec_Property' failed.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    //Following is to improve performance
    stat = ecdb.ExecuteSql ("CREATE INDEX idx_ECClassId ON ec_Property (ECClassId);");
    if (stat == BE_SQLITE_OK)
        {
        LOG.debug ("ECDb profile upgrade: Added index on column 'ECClassId' in table 'ec_Property'.");
        return BE_SQLITE_OK;
        }
    else
        {
        LOG.error ("ECDb profile upgrade failed: Adding index on column 'ECClassId' in table 'ec_Property' failed.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }
    }


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
SchemaKey ECDbProfileECSchemaUpgrader::s_ecdbfileinfoSchemaKey = SchemaKey (L"ECDb_FileInfo", 1, 0);

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle        07/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileECSchemaUpgrader::ImportProfileSchemas (ECDbR ecdb, bool updateSchema)
    {
    StopWatch timer ("", true);
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
    auto importStat = ecdb.GetSchemaManager ().ImportECSchemas (context->GetCache (), options);

    timer.Stop ();
    if (importStat != SUCCESS)
        {
        LOG.errorv ("Creating / upgrading ECDb file failed because importing / updating the ECDb standard ECSchemas in ECDb '%s' failed.",
            ecdb.GetDbFileName ());
        return BE_SQLITE_ERROR;
        }

    if (LOG.isSeverityEnabled (NativeLogging::LOG_DEBUG))
        {
        LOG.debugv ("Imported / updated ECDb standard ECSchemas in ECDb '%s' in %.4f msecs.",
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
    if (SCHEMA_READ_STATUS_Success != deserializeStat)
        {
        if (SCHEMA_READ_STATUS_ReferencedSchemaNotFound == deserializeStat)
            LOG.errorv ("Creating / upgrading ECDb file %s failed because required standard ECSchemas could not be found.", ecdbFileName);
        else
            {
            //other error codes are considered programmer errors and therefore have an assertion, too
            LOG.errorv ("Creating / upgrading ECDb file %s failed because ECDbSystem ECSchema could not be deserialized. Error code SchemaReadStatus::%d", ecdbFileName, deserializeStat);
            BeAssert (false && "ECDb upgrade: Failed to deserialize ECDbSystem ECSchema");
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
        "<ECSchema schemaName='ECDbSystem' nameSpacePrefix='ecdbsys' version='1.2'  xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'> "
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.06' prefix='bsca' /> "
        "    <ECCustomAttributes> "
        "         <SystemSchema xmlns='Bentley_Standard_CustomAttributes.01.06'/> "
        "    </ECCustomAttributes> "
        "    <ECClass typeName='ArrayOfPrimitives' isDomainClass='False' isStruct='True' > "
        "        <ECCustomAttributes> "
        "            <ECDbClassHint xmlns='Bentley_Standard_CustomAttributes.01.06'> "
        "                <MapStrategy>DoNotMapHierarchy</MapStrategy> "
        "            </ECDbClassHint> "
        "        </ECCustomAttributes> "
        "    </ECClass> "
        "    <ECClass typeName='ArrayOfBinary' isDomainClass='True' isStruct='True' > "
        "        <BaseClass>ArrayOfPrimitives</BaseClass> "
        "        <ECArrayProperty propertyName='PrimitiveArray' typeName='binary'/> "
        "    </ECClass> "
        "    <ECClass typeName='ArrayOfDateTime' isDomainClass='True' isStruct='True' > "
        "        <BaseClass>ArrayOfPrimitives</BaseClass> "
        "        <ECArrayProperty propertyName='PrimitiveArray' typeName='dateTime'/> "
        "    </ECClass> "
        "    <ECClass typeName='ArrayOfDouble' isDomainClass='True' isStruct='True' > "
        "        <BaseClass>ArrayOfPrimitives</BaseClass> "
        "        <ECArrayProperty propertyName='PrimitiveArray' typeName='double'/> "
        "    </ECClass> "
        "    <ECClass typeName='ArrayOfInteger' isDomainClass='True' isStruct='True' > "
        "        <BaseClass>ArrayOfPrimitives</BaseClass> "
        "        <ECArrayProperty propertyName='PrimitiveArray' typeName='int'/> "
        "    </ECClass> "
        "    <ECClass typeName='ArrayOfLong' isDomainClass='True' isStruct='True' > "
        "        <BaseClass>ArrayOfPrimitives</BaseClass> "
        "        <ECArrayProperty propertyName='PrimitiveArray' typeName='long'/> "
        "    </ECClass> "
        "    <ECClass typeName='ArrayOfPoint2d' isDomainClass='True' isStruct='True' > "
        "        <BaseClass>ArrayOfPrimitives</BaseClass> "
        "        <ECArrayProperty propertyName='PrimitiveArray' typeName='point2d'/> "
        "    </ECClass> "
        "    <ECClass typeName='ArrayOfPoint3d' isDomainClass='True' isStruct='True' > "
        "        <BaseClass>ArrayOfPrimitives</BaseClass> "
        "        <ECArrayProperty propertyName='PrimitiveArray' typeName='point3d'/> "
        "    </ECClass> "
        "    <ECClass typeName='ArrayOfString' isDomainClass='True' isStruct='True' > "
        "        <BaseClass>ArrayOfPrimitives</BaseClass> "
        "        <ECArrayProperty propertyName='PrimitiveArray' typeName='string'/> "
        "    </ECClass> "
        "    <ECClass typeName='ArrayOfBoolean' isDomainClass='True' isStruct='True' > "
        "        <BaseClass>ArrayOfPrimitives</BaseClass> "
        "        <ECArrayProperty propertyName='PrimitiveArray' typeName='boolean'/> "
        "    </ECClass> "
        "    <ECClass typeName='ECSqlSystemProperties' isDomainClass='False' isStruct='False' >"
        "        <ECCustomAttributes> "
        "            <ECDbClassHint xmlns='Bentley_Standard_CustomAttributes.01.06'> "
        "                <MapStrategy>DoNotMap</MapStrategy> "
        "            </ECDbClassHint> "
        "        </ECCustomAttributes> "
        "        <ECProperty propertyName='ECInstanceId' typeName='long' description='Represents the ECInstanceId system property used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='SourceECInstanceId' typeName='long' description='Represents the SourceECInstanceId system property of an ECRelationship used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='SourceECClassId' typeName='long' description='Represents the SourceECClassId system property of an ECRelationship used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='TargetECInstanceId' typeName='long' description='Represents the TargetECInstanceId system property of an ECRelationship used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='TargetECClassId' typeName='long' description='Represents the TargetECClassId system property of an ECRelationship used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='OwnerECInstanceId' typeName='long' description='For ECClasses mapped to secondary table storage (e.g. ECStructs), this system property represents the ECInstanceId of the owning (embedding) ECInstance.' />"
        "        <ECProperty propertyName='ECPropertyId' typeName='long' description='For ECClasses mapped to secondary table storage, this system property represents the id of the ECProperty in the owning ECClass to which a row in the secondary table refers to.' />"
        "        <ECProperty propertyName='ECArrayIndex' typeName='int' description='For EC arrays mapped to secondary table storage, this system property represents the index of the array element which a row in the secondary table represents.' />"
        "    </ECClass> "
        "</ECSchema>";
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
