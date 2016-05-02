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
// @bsimethod                                                    Krischan.Eberle        05/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3600::_Upgrade(ECDbR ecdb) const
    {
    DbResult stat = UpdateECPropertyKindColumn(ecdb);
    if (BE_SQLITE_OK != stat)
        return stat;

    return AddMissingIndexesOnFKColumns(ecdb);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle        05/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3600::UpdateECPropertyKindColumn(ECDbR ecdb) const
    {
    //ECPropertyKind::Enumeration (value: 4) was removed, as enumeration props are primitive props, 
    //and therefore ECPropertyKind::Navigation now has value 4.
    //So old value 4 is changed to new value 0 (Primitive)
    //And old value 5 (Navigation) is changed to 4 (Navigation)
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "UPDATE ec_Property SET Kind=? WHERE Kind=?"))
        {
        LOG.errorv("ECDb profile upgrade failed: Preparing statement to update column 'Kind' in table 'ec_Property' to change in ECPropertyKind enum failed: %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (BE_SQLITE_OK != stmt.BindInt(1, (int) ECPropertyKind::Primitive) || BE_SQLITE_OK != stmt.BindInt(2, 4) ||
        BE_SQLITE_DONE != stmt.Step())
        {
        LOG.errorv("ECDb profile upgrade failed: Executing statement to update column 'Kind' in table 'ec_Property' to change in ECPropertyKind enum failed: %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    stmt.Reset();
    stmt.ClearBindings();

    if (BE_SQLITE_OK != stmt.BindInt(1, (int) ECPropertyKind::Navigation) || BE_SQLITE_OK != stmt.BindInt(2, 5) ||
        BE_SQLITE_DONE != stmt.Step())
        {
        LOG.errorv("ECDb profile upgrade failed: Executing statement to update column 'Kind' in table 'ec_Property' to change in ECPropertyKind enum failed: %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    LOG.debug("ECDb profile upgrade: Updated column 'Kind' in table 'ec_Property' to change in ECPropertyKind enum.");
    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle        05/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3600::AddMissingIndexesOnFKColumns(ECDbR ecdb) const
    {
    if (BE_SQLITE_OK != ecdb.ExecuteSql("CREATE INDEX ix_ec_SchemaReference_ReferencedSchemaId ON ec_SchemaReference(ReferencedSchemaId);"
                                        "DROP INDEX ix_ec_Class_SchemaId;"
                                        "CREATE UNIQUE INDEX uix_ec_Class_SchemaId_Name ON ec_Class(SchemaId,Name);"
                                        "CREATE INDEX ix_ec_Property_Enumeration ON ec_Property(Enumeration);"
                                        "CREATE INDEX ix_ec_Property_NonPrimitiveType ON ec_Property(NonPrimitiveType);"
                                        "CREATE INDEX ix_ec_CustomAttribute_ClassId ON ec_CustomAttribute(ClassId);"
                                        "CREATE INDEX ix_ec_ClassMap_ParentId ON ec_ClassMap(ParentId);"
                                        "CREATE INDEX ix_ec_PropertyMap_PropertyPathId ON ec_PropertyMap(PropertyPathId);"
                                        //This index is now already added during upgrade to 3.2. However if the file was created
                                        //before the 3.2 upgrader was enhanced, the file wouldn't have the index. So attempt to create it here, too
                                        "CREATE INDEX IF NOT EXISTS ix_ec_PropertyMap_ColumnId ON ec_PropertyMap(ColumnId);"
                                        "CREATE INDEX ix_ec_Index_ClassId ON ec_Index(ClassId);"
                                        "CREATE INDEX ix_ec_IndexColumn_ColumnId ON ec_IndexColumn(ColumnId);"
                                        "CREATE INDEX ix_ec_ForeignKey_TableId ON ec_ForeignKey(TableId);"
                                        "CREATE INDEX ix_ec_ForeignKey_ReferencedTableId ON ec_ForeignKey(ReferencedTableId);"
                                        "CREATE INDEX ix_ec_ForeignKeyColumn_ForeignKeyId ON ec_ForeignKeyColumn(ForeignKeyId);"
                                        "CREATE INDEX ix_ec_ForeignKeyColumn_ColumnId ON ec_ForeignKeyColumn(ColumnId);"
                                        "CREATE INDEX ix_ec_ForeignKeyColumn_ReferencedColumnId ON ec_ForeignKeyColumn(ReferencedColumnId)"))
        {
        LOG.errorv("ECDb profile upgrade failed: Could not create indexes on foreign key columns: %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    LOG.debug("ECDb profile upgrade: Created missing indexes on the foreign key columns of the ec_ tables.");
    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle        04/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3500::_Upgrade(ECDbR ecdb) const
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "SELECT Id, EnumValues FROM ec_Enumeration"))
        {
        LOG.errorv("ECDb profile upgrade failed: Updating JSON format in column 'EnumValues' of table 'ec_Enumeration' failed: %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    //Old: [ [1,"Foo"], [2,"Goo"] ]
    //New: [ {"IntValue":1, "DisplayLabel":"Foo"}, {"IntValue":2, "DisplayLabel":"Goo"}]

    std::vector<std::pair<ECEnumerationId, Utf8String>> newValues;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ECEnumerationId id = stmt.GetValueId<ECEnumerationId>(0);
        Json::Value newJson;
        Utf8CP oldJsonStr = stmt.GetValueText(1);
        Json::Value oldJson;
        Json::Reader reader;
        if (!reader.Parse(oldJsonStr, oldJson, false))
            {
            LOG.errorv("ECDb profile upgrade failed: Updating JSON format in column 'EnumValues' of table 'ec_Enumeration' failed: %s", ecdb.GetLastError().c_str());
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
            }

        BeAssert(oldJson.isArray());
        for (JsonValueCR oldArrayElement : oldJson)
            {
            BeAssert(oldArrayElement.isArray());

            Json::Value newArrayElement;

            JsonValueCR oldVal = oldArrayElement[0];
            if (oldVal.isInt())
                newArrayElement[METASCHEMA_ECENUMERATOR_PROPERTY_IntValue] = oldVal;
            else if (oldVal.isString())
                newArrayElement[METASCHEMA_ECENUMERATOR_PROPERTY_StringValue] = oldVal;
            else
                {
                LOG.errorv("ECDb profile upgrade failed: Updating JSON format in column 'EnumValues' of table 'ec_Enumeration' failed: %s", ecdb.GetLastError().c_str());
                return BE_SQLITE_ERROR_ProfileUpgradeFailed;
                }

            const Json::ArrayIndex size = oldArrayElement.size();
            if (size == 2)
                newArrayElement[METASCHEMA_ECENUMERATOR_PROPERTY_DisplayLabel] = oldArrayElement[1];

            newJson.append(newArrayElement);
            }

        Json::FastWriter writer;
        newValues.push_back(std::make_pair(id, writer.write(newJson)));
        }

    stmt.Finalize();

    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "UPDATE ec_Enumeration SET EnumValues=? WHERE Id=?"))
        {
        LOG.errorv("ECDb profile upgrade failed: Updating JSON format in column 'EnumValues' of table 'ec_Enumeration' failed: %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    for (std::pair<ECEnumerationId, Utf8String> const& newValue : newValues)
        {
        if (BE_SQLITE_OK != stmt.BindText(1, newValue.second.c_str(), Statement::MakeCopy::No) ||
            BE_SQLITE_OK != stmt.BindId(2, newValue.first) ||
            BE_SQLITE_DONE != stmt.Step())
            {
            LOG.errorv("ECDb profile upgrade failed: Updating JSON format in column 'EnumValues' of table 'ec_Enumeration' failed: %s", ecdb.GetLastError().c_str());
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
            }

        stmt.Reset();
        stmt.ClearBindings();
        }

    LOG.debug("ECDb profile upgrade: Updated JSON format in column 'EnumValues' of table 'ec_Enumeration'.");
    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle        04/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3400::_Upgrade(ECDbR ecdb) const
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "SELECT Name, Val FROM be_Local WHERE Name LIKE 'ec_%sequence'"))
        {
        LOG.errorv("ECDb profile upgrade failed: Updating format of ECDb sequences in table 'be_Local' failed: %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    Statement updateStmt;
    if (BE_SQLITE_OK != updateStmt.Prepare(ecdb, "UPDATE be_Local SET Val=? WHERE Name=?"))
        {
        LOG.errorv("ECDb profile upgrade failed: Updating format of ECDb sequences in table 'be_Local' failed: %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    int sequenceCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        Utf8CP sequenceName = stmt.GetValueText(0);

        //the old format was persisted as blob by taking the pointer address of the local integral variable. So we must read out the value that way
        //and then it can be readded using directly SQLite's appropriate bind overload
        BeAssert(stmt.GetColumnBytes(1) <= (int) sizeof(uint64_t));
        uint64_t val = 0;
        void const* blob = stmt.GetValueBlob(1);
        memcpy(&val, blob, sizeof(val));

        if (BE_SQLITE_OK != updateStmt.BindUInt64(1, val) || BE_SQLITE_OK != updateStmt.BindText(2, sequenceName, Statement::MakeCopy::No))
            {
            LOG.errorv("ECDb profile upgrade failed: Updating format of ECDb sequences in table 'be_Local' failed: %s", ecdb.GetLastError().c_str());
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
            }

        if (BE_SQLITE_DONE != updateStmt.Step() || ecdb.GetModifiedRowCount() != 1)
            {
            LOG.error("ECDb profile upgrade failed: Updating format of ECDb sequences in table 'be_Local' failed.");
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
            }

        updateStmt.Reset();
        updateStmt.ClearBindings();
        sequenceCount++;
        }

    if (sequenceCount != 11)
        {
        LOG.errorv("ECDb profile upgrade failed: Unexpected number of ECDb sequences in table 'be_Local' were updated. Expected: 11. Found: %d", sequenceCount);
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    LOG.debug("ECDb profile upgrade: Updated format of ECDb sequences format in table 'be_Local'.");
    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle        04/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3302::_Upgrade(ECDbR ecdb) const
    {
    Utf8String sql;
    sql.Sprintf("UPDATE ec_Class SET Modifier=%d WHERE Name IN ('ExternalFileInfo', 'FileInfoOwnership') AND SchemaId=(SELECT Id FROM ec_Schema WHERE Name = 'ECDb_FileInfo')",
                Enum::ToInt(ECClassModifier::None));

    if (BE_SQLITE_OK != ecdb.ExecuteSql(sql.c_str()))
        {
        LOG.errorv("ECDb profile upgrade failed: Unsealing ECClasses 'ExternalFileInfo' and 'FileInfoOwnership' in ECSchema 'ECDb_FileInfo' failed: %s",
                  ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    LOG.debug("ECDb profile upgrade: Unsealed ECClasses 'ExternalFileInfo' and 'FileInfoOwnership' in ECSchema 'ECDb_FileInfo'.");
    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle        04/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3300::_Upgrade(ECDbR ecdb) const
    {
    if (BE_SQLITE_OK != ecdb.ExecuteSql("ALTER TABLE ec_Class ADD COLUMN CustomAttributeContainerType INTEGER;"))
        {
        LOG.error("ECDb profile upgrade failed: Adding column 'CustomAttributeContainerType' to table 'ec_Class' failed.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    LOG.debug("ECDb profile upgrade: Table 'ec_Class': added column 'CustomAttributeContainerType'.");

    DbResult stat = UpdateGeneralizedCustomContainerTypeInCAInstanceTable(ecdb);
    if (BE_SQLITE_OK != stat)
        return stat;

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "SELECT Id FROM ec_Schema WHERE Name='ECDbMap'"))
        {
        LOG.errorv("ECDb profile upgrade failed: Updating CustomAttribute container type in ECDbMap ECSchema failed. %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (BE_SQLITE_ROW != stmt.Step())
        return BE_SQLITE_OK;

    const ECSchemaId ecdbMapSchemaId = stmt.GetValueId<ECSchemaId>(0);
    stmt.Finalize();

    Utf8String updateSql;
    updateSql.Sprintf("UPDATE ec_Class SET CustomAttributeContainerType=:type WHERE SchemaId=%s AND Name=:name",
                      ecdbMapSchemaId.ToString().c_str());

    if (BE_SQLITE_OK != stmt.Prepare(ecdb, updateSql.c_str()))
        {
        LOG.errorv("ECDb profile upgrade failed: Updating CustomAttribute container type in ECDbMap ECSchema failed. %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    stat = SetCustomContainerType(ecdb, stmt, "SchemaMap", CustomAttributeContainerType::Schema);
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = SetCustomContainerType(ecdb, stmt, "ClassMap", CustomAttributeContainerType::EntityClass | CustomAttributeContainerType::RelationshipClass);
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = SetCustomContainerType(ecdb, stmt, "DisableECInstanceIdAutogeneration", CustomAttributeContainerType::EntityClass);
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = SetCustomContainerType(ecdb, stmt, "PropertyMap", CustomAttributeContainerType::PrimitiveProperty);
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = SetCustomContainerType(ecdb, stmt, "LinkTableRelationshipMap", CustomAttributeContainerType::RelationshipClass);
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = SetCustomContainerType(ecdb, stmt, "ForeignKeyRelationshipMap", CustomAttributeContainerType::RelationshipClass);
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt.Finalize();

    LOG.debug("ECDb profile upgrade: Updated CustomAttributeContainerType in table 'ec_Class' for the CustomAttribute classes in the ECDbMap ECSchema.");
    return BE_SQLITE_OK;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle        04/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileUpgrader_3300::UpdateGeneralizedCustomContainerTypeInCAInstanceTable(ECDbCR ecdb)
    {
    Utf8String sql;
    sql.Sprintf("UPDATE ec_customattribute SET ContainerType = CASE ContainerType "
                "WHEN 1 THEN %d "
                "WHEN 2 THEN %d "
                "WHEN 3 THEN %d "
                "WHEN 4 THEN %d "
                "WHEN 5 THEN %d "
                "ELSE ContainerType END",
                Enum::ToInt(ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Schema),
                Enum::ToInt(ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class),
                Enum::ToInt(ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property),
                Enum::ToInt(ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::SourceRelationshipConstraint),
                Enum::ToInt(ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::TargetRelationshipConstraint));

    if (BE_SQLITE_OK != ecdb.ExecuteSql(sql.c_str()))
        {
        LOG.errorv("ECDb profile upgrade failed: Updating ContainerType values in ec_CustomAttribute to enum GeneralizedCustomAttributeContainerType failed. %s", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    LOG.debug("ECDb profile upgrade: Updated values of column 'ContainerType' in table 'ec_CustomAttribute' to enum GeneralizedCustomAttributeContainerType.");
    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle        04/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileUpgrader_3300::SetCustomContainerType(ECDbCR ecdb, Statement& stmt, Utf8CP caClassName, CustomAttributeContainerType type)
    {
    if (BE_SQLITE_OK != stmt.BindText(stmt.GetParameterIndex(":name"), caClassName, Statement::MakeCopy::No) ||
        BE_SQLITE_OK != stmt.BindInt(stmt.GetParameterIndex(":type"), Enum::ToInt(type)))
        {
        LOG.errorv("ECDb profile upgrade failed: Updating CustomAttribute container type for class 'ECDbMap:%s' failed. %s", caClassName, ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (BE_SQLITE_DONE != stmt.Step() || ecdb.GetModifiedRowCount() != 1)
        {
        LOG.errorv("ECDb profile upgrade failed: Updating CustomAttribute container type for class 'ECDbMap:%s' failed. %s", caClassName, ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    stmt.ClearBindings();
    stmt.Reset();

    CachedStatementPtr validateStmt = ecdb.GetCachedStatement("SELECT ca.rowId, ca.ContainerType FROM ec_CustomAttribute ca, ec_Class c WHERE ca.ClassId=c.Id AND c.Name=?");
    if (validateStmt == nullptr)
        {
        LOG.errorv("ECDb profile upgrade failed: Validating existing %s CustomAttribute instances against container type failed. %s", caClassName, ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (BE_SQLITE_OK != validateStmt->BindText(1, caClassName, Statement::MakeCopy::No))
        {
        LOG.errorv("ECDb profile upgrade failed: Validating existing %s CustomAttribute instances against container type failed. %s", caClassName, ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    while (BE_SQLITE_ROW == validateStmt->Step())
        {
        const ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType actualType = Enum::FromInt<ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType>(validateStmt->GetValueInt(1));
        if (!Enum::Intersects(type, (CustomAttributeContainerType) actualType))
            {
            LOG.errorv("ECDb profile upgrade failed: Found %s custom attribute instance on a container which is not supported for this custom attribute class. (rowid %" PRId64 " in table ec_CustomAttribute)",
                       caClassName, validateStmt->GetValueInt64(0));
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
            }
        }

    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle        03/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3202::_Upgrade(ECDbR ecdb) const
    {
    //don't fail if indexes to drop don't exist, or indexes to create already exist, because there was a bad
    //push that pushed the index stuff when creating a new file but the upgrader wasn't added to the list yet.
    //So files were created with a too old profile version, so that ECDb attempts to upgrade it although it doesn't
    //have to upgrade.
    ecdb.TryExecuteSql("DROP INDEX ix_ec_Property_ClassId; DROP INDEX ix_ec_Column_TableId;");

    if (BE_SQLITE_OK != ecdb.ExecuteSql("CREATE INDEX IF NOT EXISTS ix_ec_BaseClass_ClassId_Ordinal ON ec_BaseClass(ClassId,Ordinal);"
                                        "CREATE INDEX IF NOT EXISTS ix_ec_Property_ClassId_Ordinal ON ec_Property(ClassId,Ordinal);"
                                        "CREATE INDEX IF NOT EXISTS ix_ec_CustomAttribute_ContainerId_ContainerType_Ordinal ON ec_CustomAttribute(ContainerId,ContainerType,Ordinal);"
                                        "CREATE INDEX IF NOT EXISTS ix_ec_Table_BaseTableId ON ec_Table(BaseTableId);"
                                        "CREATE INDEX IF NOT EXISTS ix_ec_Column_TableId_Ordinal ON ec_Column(TableId,Ordinal);"
                                        "CREATE INDEX IF NOT EXISTS ix_ec_IndexColumn_IndexId_Ordinal ON ec_IndexColumn(IndexId,Ordinal);"))
        {
        LOG.errorv("ECDb profile upgrade failed for '%s': Failed to create indexes ix_ec_BaseClass_ClassId_Ordinal, ix_ec_Property_ClassId_Ordinal, "
                   "ix_ec_CustomAttribute_ContainerId_ContainerType_Ordinal, ix_ec_Table_BaseTableId, ix_ec_Column_TableId_Ordinal, ix_ec_IndexColumn_IndexId_Ordinal: %s", ecdb.GetDbFileName(), ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    LOG.debug("ECDb profile upgrade: Dropped indexes ix_ec_Property_ClassId and ix_ec_Column_TableId and created indexes ix_ec_BaseClass_ClassId_Ordinal, ix_ec_Property_ClassId_Ordinal, "
              "ix_ec_CustomAttribute_ContainerId_ContainerType_Ordinal, ix_ec_Table_BaseTableId, ix_ec_Column_TableId_Ordinal, ix_ec_IndexColumn_IndexId_Ordinal.");
    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle        03/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3201::_Upgrade(ECDbR ecdb) const
    {
    if (BE_SQLITE_OK != ecdb.ExecuteSql("DELETE FROM ec_Property WHERE lower(Name) IN ('parentecinstanceid','ecpropertypathid','ecarrayindex') AND "
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

#define OBSOLETE_STRUCTARRAY_TABLETYPE "3"

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle        03/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3200::_Upgrade(ECDbR ecdb) const
    {
    //Delete struct array delete triggers
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "SELECT Name FROM sqlite_master WHERE type='trigger' AND Name LIKE '%_StructArray_Delete'"))
        {
        LOG.errorv("ECDb profile upgrade failed for '%s'. Preparing SQL to find struct array delete triggers failed. SQL: %s. Error: %s",
                   ecdb.GetDbFileName(), stmt.GetSql(), ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    //This index was missing. Without it the below deletes would be very slow.
    if (BE_SQLITE_OK != ecdb.ExecuteSql("CREATE INDEX ix_ec_PropertyMap_ColumnId ON ec_PropertyMap(ColumnId);"))
        {
        LOG.errorv("ECDb profile upgrade failed for '%s'. Failed to create index 'ix_ec_PropertyMap_ColulmnId': %s",
                   ecdb.GetDbFileName(), ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }
        
    std::vector<Utf8String> triggerNames;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        //cache names in a vector as DROP TRIGGER doesn't work if SELECT is still active
        triggerNames.push_back(stmt.GetValueText(0));
        }

    stmt.Finalize();

    Utf8CP dropTriggerSql = "DROP TRIGGER %s";
    Utf8String sql;
    for (Utf8StringCR triggerName : triggerNames)
        {
        sql.Sprintf(dropTriggerSql, triggerName.c_str());
        if (BE_SQLITE_DONE != ecdb.ExecuteSql(sql.c_str()))
            {
            LOG.errorv("ECDb profile upgrade failed for '%s'. Deleting struct array trigger %s failed. Error: %s",
                       ecdb.GetDbFileName(), triggerName.c_str(), ecdb.GetLastError().c_str());
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
            }

        LOG.debugv("ECDb profile upgrade: Dropped trigger %s.", triggerName.c_str());
        }


    //Delete struct array tables
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "SELECT Name FROM ec_Table WHERE Type=" OBSOLETE_STRUCTARRAY_TABLETYPE " AND IsVirtual=0"))
        {
        LOG.errorv("ECDb profile upgrade failed for '%s'. Preparing SQL '%s' failed. Error: %s",
                   ecdb.GetDbFileName(), stmt.GetSql(), ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    std::vector<Utf8String> structArrayTableNames;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        //cache names in a vector as DROP TABLE doesn't work if SELECT is still active
        Utf8CP structArrayTableName = stmt.GetValueText(0);
        if (!ecdb.TableExists(structArrayTableName))
            {
            LOG.warningv("ECDb profile upgrade for '%s': Tried to delete struct array table '%s' but it did not exist although it was listed in the ec_Table.",
                         ecdb.GetDbFileName(), structArrayTableName);
            continue;
            }

        structArrayTableNames.push_back(structArrayTableName);
        }

    stmt.Finalize();

    for (Utf8StringCR structArrayTableName : structArrayTableNames)
        {
        if (BE_SQLITE_OK != ecdb.DropTable(structArrayTableName.c_str()))
            {
            LOG.errorv("ECDb profile upgrade failed for '%s'. Deleting struct array table %s failed. Error: %s",
                       ecdb.GetDbFileName(), structArrayTableName.c_str(), ecdb.GetLastError().c_str());
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
            }

        LOG.debugv("ECDb profile upgrade: Dropped struct array table %s.", structArrayTableName.c_str());
        }

    //delete entries in ec_Table. This also deletes respective entries in ec_Column, ec_PropertyMap and ec_PropertyPath (via FK)
    //Here we have to delete non-virtual tables, too, as we don't even have virtual tables for struct arrays anymore
    StopWatch timer(true);
    if (BE_SQLITE_OK != ecdb.ExecuteSql("DELETE FROM ec_Table WHERE Type=" OBSOLETE_STRUCTARRAY_TABLETYPE))
        {
        LOG.errorv("ECDb profile upgrade failed for '%s'. Deleting struct array entries from ec_Table failed: %s",
                   ecdb.GetDbFileName(), ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    timer.Stop();
    LOG.debugv("ECDb profile upgrade: Deleted struct array entries from ec_Table (%.4f msecs). Number of rows deleted: %d", timer.GetElapsedSeconds() * 1000.0, ecdb.GetModifiedRowCount());
    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle        02/2016
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ECDbProfileUpgrader_3100::_Upgrade(ECDbR ecdb) const
    {
    //shared columns now have a dedicated DbColumn::Kind::SharedDataColumn. So update all sharead column entries in ec_Column.
    //Identifying shared columns in the previous version is done by looking for columns with DbColumn::Kind::DataColumn, Type::Any
    //and a table created by ECDb (-> not an existing table)
    const int dataColKindInt = Enum::ToInt(DbColumn::Kind::DataColumn);

    Utf8String sql;
    sql.Sprintf("UPDATE ec_Column SET ColumnKind=%d WHERE ColumnKind & %d=%d AND Type=%d AND TableId IN (SELECT t.Id FROM ec_Table t WHERE t.Type<>%d)", Enum::ToInt(DbColumn::Kind::SharedDataColumn),
                dataColKindInt, dataColKindInt, Enum::ToInt(DbColumn::Type::Any), Enum::ToInt(DbTable::Type::Existing));

    if (BE_SQLITE_OK != ecdb.ExecuteSql(sql.c_str()))
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
    if (BE_SQLITE_OK != ecdb.ExecuteSql("ALTER TABLE ec_ClassMap ADD COLUMN MapStrategyMinSharedColumnCount INTEGER;"))
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

    vector<Utf8String> createIndexDdlList;
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
        for (auto const& createIndexDdl : createIndexDdlList)
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
DbResult ECDbProfileUpgrader::RetrieveIndexDdlListForTable(vector<Utf8String>& indexDdlList, ECDbCR ecdb, Utf8CP tableName)
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
