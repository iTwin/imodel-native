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
//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3730::_Upgrade(ECDbCR ecdb) const
    {
    //Get ECSchemaId of MetaSchema
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "SELECT Id FROM ec_Schema WHERE Name='MetaSchema'"))
        {
        LOG.errorv("ECDb profile upgrade failed: Select ECSchemaId for MetaSchema ECSchema failed. %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (BE_SQLITE_ROW != stmt.Step())
        {
        LOG.error("ECDb profile upgrade failed: Did not find MetaSchema ECSchema in the table ec_Schema.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    const ECSchemaId metaSchemaId = stmt.GetValueId<ECSchemaId>(0);
    Utf8String metaSchemaIdStr = metaSchemaId.ToString();

    if (BE_SQLITE_DONE != stmt.Step())
        {
        LOG.error("ECDb profile upgrade failed: Found more than one entries for the MetaSchema ECSchema in ec_Schema.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    stmt.Finalize();

    //Change ECEnumeration MetaSchema.ECCustomAttributeContainerType from non-strict to strict
    Utf8String sql;
    sql.Sprintf("UPDATE ec_Enumeration SET IsStrict=%d WHERE Name='ECCustomAttributeContainerType' AND SchemaId=%s",
                DbSchemaPersistenceManager::BoolToSqlInt(true), metaSchemaIdStr.c_str());

    if (BE_SQLITE_OK != stmt.Prepare(ecdb, sql.c_str()) ||
        BE_SQLITE_DONE != stmt.Step())
        {
        LOG.errorv("ECDb profile upgrade failed: Updating ECEnumeration 'MetaSchema.ECCustomAttributeContainerType' from non-strict to strict failed. %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (ecdb.GetModifiedRowCount() != 1)
        {
        LOG.error("ECDb profile upgrade failed: Updating ECEnumeration 'MetaSchema.ECCustomAttributeContainerType' from non-strict to strict failed. SQL statement didn't affect a single row.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    stmt.Finalize();

    //Change ECRelationshipClassConstraint MetaSchema.ClassOwnsLocalProperties - Target from polymorphic to non-polymorphic
    sql.Sprintf("UPDATE ec_RelationshipConstraint SET IsPolymorphic=%d WHERE RelationshipEnd=%d AND RelationshipClassId IN "
                 "(SELECT Id FROM ec_Class WHERE Name='ClassOwnsLocalProperties' AND SchemaId=%s)",
                DbSchemaPersistenceManager::BoolToSqlInt(false), (int) ECRelationshipEnd_Target, metaSchemaIdStr.c_str());

    if (BE_SQLITE_OK != stmt.Prepare(ecdb, sql.c_str()) ||
        BE_SQLITE_DONE != stmt.Step())
        {
        LOG.errorv("ECDb profile upgrade failed: Updating ECRelationshipClassConstraint 'MetaSchema.ClassOwnsLocalProperties [Target]' from polymorphic to non-polymorphic failed. %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (ecdb.GetModifiedRowCount() != 1)
        {
        LOG.error("ECDb profile upgrade failed: Updating ECRelationshipClassConstraint 'MetaSchema.ClassOwnsLocalProperties [Target]' from polymorphic to non-polymorphic failed. SQL statement didn't affect a single row.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    LOG.debug("ECDb profile upgrade: Updated ECEnumeration 'MetaSchema.ECCustomAttributeContainerType' from non-strict to strict and "
             "ECRelationshipClassConstraint 'MetaSchema.ClassOwnsLocalProperties [Target]' from polymorphic to non-polymorphic.");
    return BE_SQLITE_OK;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        06/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3720::AddVirtualECClassIdToTableWhichDoesNotHaveIt(ECDbCR ecdb)
    {
    Statement stmt;
    if (stmt.Prepare(ecdb, "SELECT ec_Table.Id, (SELECT Max(Ordinal) + 1 FROM ec_Column WHERE TableId = ec_Table.Id) NewColumnOrdinal FROM ec_Table WHERE ec_Table.Name != 'ECDbNotMapped' AND ec_Table.Id NOT IN (SELECT ec_Table.Id FROM ec_Table INNER JOIN ec_Column ON ec_Column.TableId = ec_Table.Id AND(ec_Column.ColumnKind & 2)) ORDER BY ec_Table.Id;") != BE_SQLITE_OK)
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;

    std::vector<std::pair<DbTableId, int>> tableWithNoECClassIds;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        DbTableId id = stmt.GetValueId<DbTableId>(0);
        int newOrdinalForColumn = stmt.GetValueInt(1);

        tableWithNoECClassIds.push_back(std::make_pair(id, newOrdinalForColumn));
        }

    //! INSERT virtual ECClassId column in table that does not have ------------------------------
    stmt.Finalize();
    
    if (stmt.Prepare(ecdb, SqlPrintfString("INSERT INTO ec_Column(Id, TableId, Name, [Type], IsVirtual, Ordinal, NotNullConstraint, UniqueConstraint, CheckConstraint, DefaultConstraint, CollationConstraint, OrdinalInPrimaryKey, ColumnKind) "
                                           " VALUES(?, ?, '" ECDB_COL_ECClassId "', %d, 1, ?, 1, 0, null, null ,0 ,null ,%d);",
                                           Enum::ToInt(DbColumn::Type::Integer), Enum::ToInt(DbColumn::Kind::ECClassId)).GetUtf8CP()) != BE_SQLITE_OK)
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;

    for (std::pair<DbTableId, int> const& kp : tableWithNoECClassIds)
        {
        int columnOrdinal = kp.second;
        DbTableId tableId = kp.first;
        DbColumnId columnId;
        if (ecdb.GetECDbImplR().GetColumnIdSequence().GetNextValue(columnId) != BE_SQLITE_OK)
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;

        stmt.Reset();
        stmt.ClearBindings();
        stmt.BindId(1, columnId); //Id
        stmt.BindId(2, tableId); //TableId
        stmt.BindInt(3, columnOrdinal); //ColumnId
        if (stmt.Step() != BE_SQLITE_DONE)
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        06/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3720::AddECClassIdPropertyPath(PropertyPathId& ecclassIdPropertyPathId, ECDbCR ecdb)
    {
    Statement stmt;    
    if (stmt.Prepare(ecdb,
                     "SELECT ec_Class.Id, (SELECT MAX(Ordinal) + 1 FROM ec_Property WHERE ec_Property.ClassId = ec_Class.Id) "
                     "FROM ec_Class INNER JOIN ec_Schema ON ec_Class.SchemaId = ec_Schema.Id WHERE ec_Class.Name = 'ECSqlSystemProperties' AND ec_Schema.Name = 'ECDb_System' ") != BE_SQLITE_OK)
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;

    if (stmt.Step() != BE_SQLITE_ROW)
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;

    ECClassId ecSqlSystemPropertiesECClassId = stmt.GetValueId<ECClassId>(0);
    int newOrdinalPropertyId = stmt.GetValueInt(1);

    //! Insert ECProperty
    stmt.Finalize();
    if (stmt.Prepare(ecdb, SqlPrintfString("INSERT INTO ec_Property(Id, ClassId, Name, DisplayLabel, Description, IsReadonly, Kind, Ordinal, PrimitiveType)"
                                           "VALUES(?, ?, '%s', '%s', 'Represents the ECClassId system property used by the EC->DB Mapping.', 1, %d, ?, %d)",
                                           ECDbSystemSchemaHelper::ECCLASSID_PROPNAME,
                                           ECDbSystemSchemaHelper::ECCLASSID_PROPNAME,
                                           Enum::ToInt(ECPropertyKind::Primitive),
                                           PrimitiveType::PRIMITIVETYPE_Long).GetUtf8CP()) != BE_SQLITE_OK)
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;

    ECPropertyId ecClassIdPropertyId;
    if (ecdb.GetECDbImplR().GetECPropertyIdSequence().GetNextValue(ecClassIdPropertyId) != BE_SQLITE_OK)
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;

    stmt.BindId(1, ecClassIdPropertyId);
    stmt.BindId(2, ecSqlSystemPropertiesECClassId);
    stmt.BindInt(3, newOrdinalPropertyId);
    if (stmt.Step() != BE_SQLITE_DONE)
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;

    //! Find PropertyPath Id for ECClassId ------------------------------------------------------
    stmt.Finalize();
    if (stmt.Prepare(ecdb, SqlPrintfString("INSERT INTO ec_PropertyPath(Id, RootPropertyId, AccessString) VALUES (?, ?, '%s')", ECDbSystemSchemaHelper::ECCLASSID_PROPNAME).GetUtf8CP()) != BE_SQLITE_OK)
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;

    if (ecdb.GetECDbImplR().GetPropertyPathIdSequence().GetNextValue(ecclassIdPropertyPathId) != BE_SQLITE_OK)
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;

    stmt.BindId(1, ecclassIdPropertyPathId);
    stmt.BindId(2, ecClassIdPropertyId);
    if (stmt.Step() != BE_SQLITE_DONE)
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;

    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        06/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3720::GetECClassIdByTable(std::map<DbTableId, DbColumnId>& tableByECClassId, ECDbCR ecdb)
    {
    Statement stmt;
    if (stmt.Prepare(ecdb, "SELECT ec_Column.Id, ec_Column.TableId FROM ec_Column WHERE ec_Column.ColumnKind & 2") != BE_SQLITE_OK)
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        DbColumnId columnId = stmt.GetValueId<DbColumnId>(0);
        DbTableId tableId = stmt.GetValueId<DbTableId>(1);
        tableByECClassId[tableId] = columnId;
        }

    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        06/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3720::GetECClassMapByTableId(std::map<ClassMapId, DbTableId>& classMapByTable, ECDbCR ecdb)
    {
    Statement stmt;
    if (stmt.Prepare(ecdb,
                     "SELECT ec_ClassMap.Id, ec_Table.Id "
                     "FROM ec_PropertyMap "
                     "    JOIN ec_Column ON ec_Column.Id = ec_PropertyMap.ColumnId "
                     "    JOIN ec_PropertyPath ON ec_PropertyPath.Id = ec_PropertyMap.PropertyPathId "
                     "    JOIN ec_ClassMap ON ec_ClassMap.Id = ec_PropertyMap.ClassMapId "
                     "    JOIN ec_Class ON ec_Class.Id = ec_ClassMap.ClassId "
                     "    JOIN ec_Table ON ec_Table.Id = ec_Column.TableId "
                     "    WHERE ec_ClassMap.MapStrategy <> 100 AND ec_ClassMap.MapStrategy <> 101 "
                     "    GROUP BY ec_ClassMap.Id, ec_Table.Id") != BE_SQLITE_OK)
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ClassMapId classMapId = stmt.GetValueId<ClassMapId>(0);
        DbTableId tableId = stmt.GetValueId<DbTableId>(1);
        classMapByTable[classMapId] = tableId;
        }

    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        06/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3720::AddECClassIdPropertyMap(PropertyPathId ecclassIdPropertyPathId, std::map<ClassMapId, DbTableId>& classMapByTable, std::map<DbTableId, DbColumnId>& tableByECClassId, ECDbCR ecdb)
    {
    Statement stmt;
    if (stmt.Prepare(ecdb, "INSERT INTO ec_PropertyMap (ClassMapId, PropertyPathId, ColumnId) VALUES (?,?,?)") != BE_SQLITE_OK)
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;

    for (auto const& kp : classMapByTable)
        {
        ClassMapId classMapId = kp.first;
        DbTableId tableId = kp.second;
        DbColumnId columnId = tableByECClassId[tableId];
        stmt.Reset();
        stmt.ClearBindings();
        stmt.BindId(1, classMapId);
        stmt.BindId(2, ecclassIdPropertyPathId);
        stmt.BindId(3, columnId);
        if (stmt.Step() != BE_SQLITE_DONE)
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        06/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3720::_Upgrade(ECDbCR ecdb) const
    {
    if (AddVirtualECClassIdToTableWhichDoesNotHaveIt(ecdb) != BE_SQLITE_OK)
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;

    PropertyPathId ecclassIdPropertyPathId;
    if (AddECClassIdPropertyPath(ecclassIdPropertyPathId, ecdb) != BE_SQLITE_OK)
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;

    std::map<DbTableId, DbColumnId> tableByECClassId;
    if (GetECClassIdByTable(tableByECClassId, ecdb) != BE_SQLITE_OK)
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;

    std::map<ClassMapId, DbTableId> classMapByTable;
    if (GetECClassMapByTableId(classMapByTable, ecdb) != BE_SQLITE_OK)
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;

    if (AddECClassIdPropertyMap(ecclassIdPropertyPathId, classMapByTable, tableByECClassId, ecdb) != BE_SQLITE_OK)
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;

    Statement stmt;
    if (stmt.Prepare(ecdb,
                     "WITH "
                     "MapStruct (ColumnId, ClassMapId, TableId, MapStrategy, AccessString, ColumnName, ClassName, NamespacePrefix) AS ( "
                     "SELECT "
                     "       ec_Column.[Id], "
                     "       ec_ClassMap.[Id], "
                     "       ec_Table.[Id], "
                     "       ec_ClassMap.MapStrategy, "
                     "       ec_PropertyPath.AccessString, "
                     "       ec_Column.Name, "
                     "       ec_Class.[Name], "
                     "       ec_Schema.[NamespacePrefix] "
                     "FROM ec_PropertyMap "
                     "    JOIN ec_Column ON ec_Column.Id = ec_PropertyMap.ColumnId "
                     "    JOIN ec_PropertyPath ON ec_PropertyPath.Id = ec_PropertyMap.PropertyPathId "
                     "    JOIN ec_ClassMap ON ec_ClassMap.Id = ec_PropertyMap.ClassMapId "
                     "    JOIN ec_Class ON ec_Class.Id = ec_ClassMap.ClassId "
                     "    JOIN ec_Schema ON ec_Schema.Id = ec_Class.SchemaId "
                     "    JOIN ec_Table ON ec_Table.Id = ec_Column.TableId) "
                     "SELECT "
                     "  A.[ClassMapId], A.[TableId], "
                     "  (SELECT MAX(ec_Column.[Ordinal]) + 1 FROM ec_Column WHERE ec_Column.[TableId] = A.[TableId]) NewColumnOrdinal, "
                     "  (CASE WHEN A.MapStrategy = 100 THEN "
                     "        (CASE WHEN SUBSTR(A.ColumnName, LENGTH(A.ColumnName) - 1, 2) = 'Id' AND A.ColumnName != 'Id' THEN "
                     "              SUBSTR(A.ColumnName, 1, LENGTH(A.ColumnName) - 2) || 'RelECClassId' "
                     "         ELSE "
                     "             A.[NamespacePrefix] || '_' || A.ClassName || '_RelECClassId' "
                     "         END) "
                     "  ELSE "
                     "        (CASE WHEN SUBSTR(B.ColumnName, LENGTH(B.ColumnName) - 1, 2) = 'Id' AND B.ColumnName != 'Id' THEN "
                     "              SUBSTR(B.ColumnName, 1, LENGTH(B.ColumnName) - 2) || 'RelECClassId' "
                     "         ELSE "
                     "             B.[NamespacePrefix] || '_' || B.ClassName || '_RelECClassId' "
                     "         END) "
                     "  END) RelECClassIdColumnName, "
                     " (A.ColumnId = B.ColumnId) IsSelf "
                     "FROM MapStruct A INNER JOIN MapStruct B ON A.[ClassMapId] = B.ClassMapId  AND B.AccessString ='TargetECInstanceId' "
                     "WHERE A.MapStrategy IN (100, 101) AND A.AccessString ='SourceECInstanceId'") != BE_SQLITE_OK)
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
    //ClassMapId, TableId, ordinal, columnName, IsSelf

    std::vector<std::tuple<DbColumnId, DbTableId, ClassMapId, Utf8String, int>> tupleList;
    std::map<DbTableId, int> ordinals;
    std::map<DbTableId, std::pair<DbColumnId, bool>> selfRelClassIdColum;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ClassMapId classMapId = stmt.GetValueId<ClassMapId>(0);
        DbTableId tableId = stmt.GetValueId<DbTableId>(1);
        int newColumnOrdinalId = stmt.GetValueInt(2);
        Utf8CP newColumnName = stmt.GetValueText(3);
        bool isSelf = stmt.GetValueInt(4) == 1;
        DbColumnId newColumnId;
        if (isSelf && selfRelClassIdColum.find(tableId) != selfRelClassIdColum.end())
            {
            newColumnId = selfRelClassIdColum[tableId].first;
            }
        else
            {
            if (ecdb.GetECDbImplR().GetColumnIdSequence().GetNextValue(newColumnId) != BE_SQLITE_OK)
                return BE_SQLITE_ERROR_ProfileUpgradeFailed;

            if (ordinals.find(tableId) != ordinals.end())
                {
                newColumnOrdinalId = ordinals[tableId] + 1;
                ordinals[tableId] = newColumnOrdinalId;
                }
            else
                ordinals[tableId] = newColumnOrdinalId;

            if (isSelf)
                selfRelClassIdColum[tableId] = std::make_pair(newColumnId, false);
            }

        tupleList.push_back(std::tuple<DbColumnId, DbTableId, ClassMapId, Utf8String, int >(newColumnId, tableId, classMapId, newColumnName, newColumnOrdinalId));
        }

    stmt.Finalize();
    if (stmt.Prepare(ecdb, SqlPrintfString("INSERT INTO ec_Column(Id, TableId, Name, [Type], IsVirtual, Ordinal, NotNullConstraint, UniqueConstraint, CheckConstraint, DefaultConstraint, CollationConstraint, OrdinalInPrimaryKey, ColumnKind) "
                                           " VALUES(?, ?, ?, %d, 1, ?, 0, 0, null, null ,0 ,null ,%d);",
                                           Enum::ToInt(DbColumn::Type::Integer), Enum::ToInt(DbColumn::Kind::RelECClassId)).GetUtf8CP()) != BE_SQLITE_OK)
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;

    for (auto const& tuple : tupleList)
        { 
        stmt.Reset();
        stmt.ClearBindings();
        auto itor = selfRelClassIdColum.find(std::get<1>(tuple));
        if (itor != selfRelClassIdColum.end())
            {
            if (itor->second.second)
                continue;

            itor->second.second = true;
            }

        stmt.BindId(1, std::get<0>(tuple)); //Id
        stmt.BindId(2, std::get<1>(tuple)); //TableId
        stmt.BindText(3, std::get<3>(tuple).c_str(), Statement::MakeCopy::No); //ColumnName
        stmt.BindInt(4, std::get<4>(tuple)); //Ordinal
        DbResult r = stmt.Step();
        
        if (r != BE_SQLITE_DONE)
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    stmt.Finalize();
    if (stmt.Prepare(ecdb, "INSERT INTO ec_PropertyMap (ClassMapId, PropertyPathId, ColumnId) VALUES (?,?,?)") != BE_SQLITE_OK)
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;

    for (auto const& tuple : tupleList)
        {
        stmt.Reset();
        stmt.ClearBindings();
        stmt.BindId(1, std::get<2>(tuple));
        stmt.BindId(2, ecclassIdPropertyPathId);
        stmt.BindId(3, std::get<0>(tuple));
        if (stmt.Step() != BE_SQLITE_DONE)
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    06/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3717::_Upgrade(ECDbCR ecdb) const
    {
    StopWatch timer(true);
    Utf8String sql;
    sql.Sprintf("SELECT t.Id, t.Name, c.Name, c.Id FROM ec_Column c, ec_Table t "
                "WHERE c.TableId=t.Id AND t.Type<>%d AND t.IsVirtual=%d AND ((c.ColumnKind & %d) <> 0 OR (c.ColumnKind & %d) <> 0) "
                "AND c.NotNullConstraint=%d order by t.Id",
                DbTable::Type::Existing, DbSchemaPersistenceManager::BoolToSqlInt(false),
                Enum::ToInt(DbColumn::Kind::SourceECInstanceId), Enum::ToInt(DbColumn::Kind::TargetECInstanceId),
                DbSchemaPersistenceManager::BoolToSqlInt(false));

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, sql.c_str()))
        {
        LOG.errorv("ECDb profile upgrade failed. %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }


    IdSet<DbColumnId> colsToUpdate;
    DbTableId currentTableId;
    Utf8String currentTableName;
    bmap<Utf8String, DbColumnId, CompareIUtf8Ascii> cols;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        DbTableId tableId = stmt.GetValueId<DbTableId>(0);
        if (currentTableId.IsValid() && currentTableId != tableId)
            {
            //Next table found. Process columns of previous table now
            Utf8String tableInfoSql;
            tableInfoSql.Sprintf("pragma table_info('%s')", currentTableName.c_str());

            Statement tableInfoStmt;
            if (BE_SQLITE_OK != tableInfoStmt.Prepare(ecdb, tableInfoSql.c_str()))
                {
                LOG.errorv("ECDb profile upgrade failed. %s", ecdb.GetLastError().c_str());
                return BE_SQLITE_ERROR_ProfileUpgradeFailed;
                }

            while (BE_SQLITE_ROW == tableInfoStmt.Step())
                {
                Utf8CP colName = tableInfoStmt.GetValueText(1);
                auto it = cols.find(colName);
                if (it == cols.end())
                    continue;

                if (DbSchemaPersistenceManager::IsTrue(tableInfoStmt.GetValueInt(3)))
                    colsToUpdate.insert(it->second);
                }

            cols.clear();
            }

        currentTableId = tableId;
        currentTableName.assign(stmt.GetValueText(1));
        cols[stmt.GetValueText(2)] = stmt.GetValueId<DbColumnId>(3);
        }

    stmt.Finalize();

    if (colsToUpdate.empty())
        return BE_SQLITE_OK;

    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "UPDATE ec_Column SET NotNullConstraint=? WHERE InVirtualSet(?,Id)") ||
        BE_SQLITE_OK != stmt.BindInt(1, DbSchemaPersistenceManager::BoolToSqlInt(true)) ||
        BE_SQLITE_OK != stmt.BindVirtualSet(2, colsToUpdate) ||
        BE_SQLITE_DONE != stmt.Step())
        {
        LOG.errorv("ECDb profile upgrade failed. %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    BeAssert((int) colsToUpdate.size() == ecdb.GetModifiedRowCount());

    timer.Stop();
    LOG.debugv("ECDb profile upgrade: Updated NotNullConstraint column in table 'ec_Column' for "
              "%" PRIu64 " Foreign Key columns. [%.4f ms]", (uint64_t) colsToUpdate.size(), timer.GetElapsedSeconds() * 1000.0);
    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    06/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3715::_Upgrade(ECDbCR ecdb) const
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "SELECT c.Id FROM ec_Class c, ec_Schema s WHERE c.SchemaId=s.Id AND s.Name='ECDb_System' AND c.Name='ECSqlSystemProperties'") ||
        BE_SQLITE_ROW != stmt.Step())
        {
        LOG.errorv("ECDb profile upgrade failed: Retrieving ECClassId of ECClass 'ECDb_System:ECSqlSystemProperties' failed. %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    Utf8String ecsqlSystemPropertiesClassIdStr = stmt.GetValueId<ECClassId>(0).ToString();

    if (BE_SQLITE_ROW == stmt.Step())
        {
        LOG.error("ECDb profile upgrade failed: More than one ECClass with name 'ECDb_System:ECSqlSystemProperties' found unexpectedly.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    stmt.Finalize();

    Utf8String updateClassMapSql;
    updateClassMapSql.Sprintf("UPDATE ec_ClassMap SET MapStrategyAppliesToSubclasses=%d WHERE ClassId=%s", 
                              DbSchemaPersistenceManager::BoolToSqlInt(true), ecsqlSystemPropertiesClassIdStr.c_str());

    if (BE_SQLITE_OK != ecdb.ExecuteSql(updateClassMapSql.c_str()))
        {
        LOG.errorv("ECDb profile upgrade failed: Updating MapStrategyAppliesToSubclasses to 1 in table ec_ClassMap for ECClass 'ECSqlSystemProperties' failed: %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (ecdb.GetModifiedRowCount() != 1)
        {
        LOG.error("ECDb profile upgrade failed: Updating MapStrategyAppliesToSubclasses to 1 in table ec_ClassMap for ECClass 'ECSqlSystemProperties' should have affected a row. It didn't affect a row though.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    Utf8String updateCASql;
    updateCASql.Sprintf("UPDATE ec_CustomAttribute SET Instance=? WHERE ContainerId=%s AND ContainerType=%d AND "
                        "ClassId IN (SELECT c.Id FROM ec_Class c, ec_Schema s WHERE c.SchemaId = s.Id AND s.Name = 'ECDbMap' AND c.Name = 'ClassMap')", 
                        ecsqlSystemPropertiesClassIdStr.c_str(), Enum::ToInt(ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class));

    if (BE_SQLITE_OK != stmt.Prepare(ecdb, updateCASql.c_str()) ||
        BE_SQLITE_OK != stmt.BindText(1, "<ClassMap xmlns=\"ECDbMap.01.01\">"
                                         "  <MapStrategy>"
                                         "    <Strategy>NotMapped</Strategy>"
                                         "    <AppliesToSubclasses>True</AppliesToSubclasses>"
                                         "  </MapStrategy>"
                                         "</ClassMap>", Statement::MakeCopy::No) ||
        BE_SQLITE_DONE != stmt.Step())
        {
        LOG.errorv("ECDb profile upgrade failed: Updating the respective ClassMap custom attribute entry for ECSqlSystemProperties in the table 'ec_CustomAttribute' failed. %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (ecdb.GetModifiedRowCount() != 1)
        {
        LOG.error("ECDb profile upgrade failed: Updating the respective ClassMap custom attribute entry for ECSqlSystemProperties in the table 'ec_CustomAttribute' should have affected a row. It didn't affect a row though.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    LOG.debug("ECDb profile upgrade: Updated the ClassMap custom attribute on the ECClass 'ECDb_System:ECSqlSystemProperties': ECProperty 'AppliesToSubclasses' was set to True.");
    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    06/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3712::_Upgrade(ECDbCR ecdb) const
    {
    if (BE_SQLITE_OK != ecdb.ExecuteSql("CREATE TABLE ec_KindOfQuantity("
                                        "Id INTEGER PRIMARY KEY,"
                                        "SchemaId INTEGER NOT NULL REFERENCES ec_Schema(Id) ON DELETE CASCADE,"
                                        "Name TEXT NOT NULL COLLATE NOCASE,"
                                        "DisplayLabel TEXT,"
                                        "Description TEXT,"
                                        "PersistenceUnit TEXT NOT NULL,"
                                        "PersistencePrecision INTEGER NOT NULL,"
                                        "DefaultPresentationUnit TEXT,"
                                        "AlternativePresentationUnits TEXT)"))
        {
        LOG.errorv("ECDb profile upgrade failed: Creating table ec_KindOfQuantity failed: %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (BE_SQLITE_OK != ecdb.ExecuteSql("CREATE INDEX ix_ec_KindOfQuantity_SchemaId ON ec_KindOfQuantity(SchemaId);"
                                        "CREATE INDEX ix_ec_KindOfQuantity_Name ON ec_KindOfQuantity(Name);"))
        {
        LOG.errorv("ECDb profile upgrade failed: Creating indexes on table ec_KindOfQuantity failed: %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (BE_SQLITE_OK != ecdb.ExecuteSql("ALTER TABLE ec_Property ADD COLUMN "
                                        "KindOfQuantityId INTEGER REFERENCES ec_KindOfQuantity(Id) ON DELETE CASCADE"))
        {
        LOG.errorv("ECDb profile upgrade failed: Adding column 'KindOfQuantityId' to table 'ec_KindOfQuantity' failed: %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    LOG.debug("ECDb profile upgrade: Added table 'ec_KindOfQuantity' and indexes. Added column 'KindOfQuantityId' to table 'ec_Property'.");
    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        06/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3711::_Upgrade(ECDbCR ecdb) const
    {
    if (BE_SQLITE_OK != ecdb.ExecuteSql("CREATE TABLE ec_ClassHierarchy("
                                        "Id INTEGER PRIMARY KEY,"
                                        "ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,"
                                        "BaseClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,"
                                        "Level INTEGER NOT NULL)"))
        {
        LOG.errorv("ECDb profile upgrade failed: Creating table ec_ClassHierarchy failed: %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (BE_SQLITE_OK != ecdb.ExecuteSql("CREATE UNIQUE INDEX uix_ec_ClassHierarchy_ClassId_BaseClassId ON ec_ClassHierarchy(ClassId,BaseClassId);"
                           "CREATE INDEX ix_ec_ClassHierarchy_BaseClassId ON ec_ClassHierarchy(BaseClassId);"))
        {
        LOG.errorv("ECDb profile upgrade failed: Creating indexes on table ec_ClassHierarchy failed: %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (BE_SQLITE_OK != ECDbSchemaWriter::RepopulateClassHierarchyTable(ecdb))
        {
        LOG.errorv("ECDb profile upgrade failed: Repopulating the ec_ClassHierarchy table failed: %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    LOG.debug("ECDb profile upgrade: Added and populated table 'ec_ClassHierarchy'.");
    return BE_SQLITE_OK;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle        05/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3710::_Upgrade(ECDbCR ecdb) const
    {
    const int ecinstanceIdColKindInt = Enum::ToInt(DbColumn::Kind::ECInstanceId);
    Utf8String sql;
    sql.Sprintf("UPDATE ec_Column SET NotNullConstraint=0 WHERE ColumnKind & %d = %d", ecinstanceIdColKindInt, ecinstanceIdColKindInt);
    const DbResult stat = ecdb.ExecuteSql(sql.c_str());
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv("ECDb profile upgrade failed: Updating column 'NotNullConstraint' in table 'ec_Column' for ECInstanceId columns failed: %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    LOG.debug("ECDb profile upgrade: Updated column 'NotNullConstraint' in table 'ec_Column' for ECInstanceId columns.");
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

    schemaKey = SchemaKey("MetaSchema", 3, 0, 1);
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
        "<ECSchema schemaName='ECDb_System' nameSpacePrefix='ecdbsys' version='3.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='01.00.01' prefix='ecdbmap' /> "
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
        "                   <AppliesToSubclasses>True</AppliesToSubclasses> "
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='ECInstanceId' typeName='long' description='Represents the ECInstanceId system property used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='ECClassId' typeName='long' readOnly='True' description='Represents the ECClassId system property used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='SourceECInstanceId' typeName='long' description='Represents the SourceECInstanceId system property of an ECRelationship used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='SourceECClassId' typeName='long' description='Represents the SourceECClassId system property of an ECRelationship used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='TargetECInstanceId' typeName='long' description='Represents the TargetECInstanceId system property of an ECRelationship used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='TargetECClassId' typeName='long' description='Represents the TargetECClassId system property of an ECRelationship used by the EC->DB Mapping.' />"
        "    </ECEntityClass> "
        "</ECSchema>";
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

