/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define PROFILENAME "ECDb"

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
DbResult ProfileManager::CreateProfile() const
    {
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("Begin CreateECProfile");

    if (!m_ecdb.GetDefaultTransaction()->IsActive())
        {
        //we always need a transaction to execute SQLite statements. If ECDb was opened in no-default-trans mode, we need to
        //begin a transaction ourselves (just use BeSQLite's default transaction which is always there even in no-default-trans mode,
        //except that in that case, it is not active).
        BeAssert(false && "Programmer Error. ECDb expects that BeSqlite::CreateNewDb keeps the default transaction active when it is called to create its profile.");
        return BE_SQLITE_ERROR_NoTxnActive;
        }

    DbResult stat = CreateProfileTables();
    if (stat != BE_SQLITE_OK)
        {
        LOG.errorv("Failed to create " PROFILENAME " profile in %s: %s", m_ecdb.GetDbFileName(), m_ecdb.GetLastError().c_str());
        m_ecdb.AbandonChanges();
        return stat;
        }

    stat = AssignProfileVersion(true);
    if (stat != BE_SQLITE_OK)
        {
        m_ecdb.AbandonChanges();
        LOG.errorv("Failed to create " PROFILENAME " profile in file '%s'. Could not assign new profile version. %s",
                   m_ecdb.GetDbFileName(), m_ecdb.GetLastError().c_str());
        return stat;
        }

    stat = ProfileSchemaUpgrader::ImportProfileSchemas(m_ecdb);
    if (stat != BE_SQLITE_OK)
        {
        m_ecdb.AbandonChanges();
        return stat;
        }
  
    m_ecdb.SaveChanges();
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("End CreateECProfile");
    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
ProfileState ProfileManager::CheckProfileVersion() const
    {
    BeAssert(m_ecdb.GetDefaultTransaction()->IsActive() && "Should have been caught before. Callers must ensure that a transaction is active before calling this.");

    if (BE_SQLITE_OK != ReadProfileVersion())
        return ProfileState::Error();

    return Db::CheckProfileVersion(ECDb::CurrentECDbProfileVersion(), m_profileVersion, ECDb::MinimumUpgradableECDbProfileVersion(), PROFILENAME);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ProfileManager::UpgradeProfile() const
    {
    BeAssert(!m_ecdb.IsReadonly());

    if (!m_ecdb.GetDefaultTransaction()->IsActive())
        {
        BeAssert(false && "Programmer Error. ECDb expects that BeSqlite::OpenBeSQliteDb keeps the default transaction active when it is called to upgrade its profile.");
        return BE_SQLITE_ERROR_NoTxnActive;
        }

    PERFLOG_START("ECDb", "Profile upgrade");

    DbResult stat = ReadProfileVersion();
    if (stat != BE_SQLITE_OK)
        {
        BeAssert(false && "Failed to read ECDb file profile version");
        return stat;
        }

    const ProfileVersion versionBeforeUpgrade(m_profileVersion);

    //set new profile version as otherwise schema imports run by upgraders would not be allowed
    if (BE_SQLITE_OK != AssignProfileVersion(false))
        {
        m_ecdb.AbandonChanges();
        LOG.errorv("Failed to upgrade " PROFILENAME " profile in file '%s'. Could not assign new profile version. %s",
                   m_ecdb.GetDbFileName(), m_ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    //let upgraders incrementally upgrade the profile to the latest state
    if (BE_SQLITE_OK != RunUpgraders(versionBeforeUpgrade))
        {
        m_ecdb.AbandonChanges();
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }


    if (BE_SQLITE_OK != ProfileSchemaUpgrader::ImportProfileSchemas(m_ecdb))
        {
        m_ecdb.AbandonChanges();
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (LOG.isSeverityEnabled(NativeLogging::LOG_INFO))
        {
        LOG.infov("Upgraded " PROFILENAME " profile from version %s to version %s in file '%s'.",
                  versionBeforeUpgrade.ToString().c_str(), m_profileVersion.ToString().c_str(), m_ecdb.GetDbFileName());
        }

    PERFLOG_FINISH("ECDb", "Profile upgrade");
    return BE_SQLITE_OK;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ProfileManager::RunUpgraders(ProfileVersion const& versionBeforeUpgrade) const
    {
    //IMPORTANT: order from low to high version
    //Note: If, for a version there is no upgrader it means just one of the profile ECSchemas needs to be reimported.
    std::vector<std::unique_ptr<ProfileUpgrader>> upgraders;
    if (versionBeforeUpgrade < ProfileVersion(4, 0, 0, 1))
        upgraders.push_back(std::make_unique<ProfileUpgrader_4001>());

    if (versionBeforeUpgrade < ProfileVersion(4, 0, 0, 2))
        upgraders.push_back(std::make_unique<ProfileUpgrader_4002>());

    for (std::unique_ptr<ProfileUpgrader> const& upgrader : upgraders)
        {
        DbResult stat = upgrader->Upgrade(m_ecdb);
        if (BE_SQLITE_OK != stat)
            return stat;
        }

    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ProfileManager::AssignProfileVersion(bool onProfileCreation) const
    {
    m_profileVersion = ECDb::CurrentECDbProfileVersion();

    //Save the profile version as string (JSON format)
    Utf8String profileVersionStr = m_profileVersion.ToJson();

    if (onProfileCreation)
        {
        DbResult stat = m_ecdb.SavePropertyString(GetInitialProfileVersionPropertySpec(), profileVersionStr);
        if (BE_SQLITE_OK != stat)
            return stat;
        }

    return m_ecdb.SavePropertyString(GetProfileVersionPropertySpec(), profileVersionStr);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
DbResult ProfileManager::ReadProfileVersion() const
    {
    if (!m_profileVersion.IsEmpty())
        return BE_SQLITE_OK; // already read before

    Utf8String currentVersionString;
    //if version entry does not exist, this either means it is ECDb profile 1.0 (because we did not store
    // a version entry for profile 1.0 or it isn't an ECDb file at all. In order to tell these we need
    // to check for a typical table of the ECDb profile:
    if (BE_SQLITE_ROW == m_ecdb.QueryProperty(currentVersionString, GetProfileVersionPropertySpec()))
        {
        m_profileVersion.FromJson(currentVersionString.c_str());
        return BE_SQLITE_OK;
        }

    //File is no ECDb file
    return BE_SQLITE_ERROR_InvalidProfileVersion;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ProfileManager::CreateProfileTables() const
    {
    //ec_Schema
    DbResult stat = m_ecdb.ExecuteDdl("CREATE TABLE " TABLE_Schema
                                    "(Id INTEGER PRIMARY KEY,"
                                    "Name TEXT UNIQUE NOT NULL COLLATE NOCASE,"
                                    "DisplayLabel TEXT,"
                                    "Description TEXT,"
                                    "Alias TEXT UNIQUE NOT NULL COLLATE NOCASE,"
                                    "VersionDigit1 INTEGER NOT NULL,"
                                    "VersionDigit2 INTEGER NOT NULL,"
                                    "VersionDigit3 INTEGER NOT NULL,"
                                    "OriginalECXmlVersionMajor INTEGER,"
                                    "OriginalECXmlVersionMinor INTEGER)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_SchemaReference
    stat = m_ecdb.ExecuteDdl("CREATE TABLE " TABLE_SchemaReference
                           "(Id INTEGER PRIMARY KEY,"
                           "SchemaId INTEGER REFERENCES " TABLE_Schema "(Id) ON DELETE CASCADE,"
                           "ReferencedSchemaId INTEGER REFERENCES " TABLE_Schema "(Id) ON DELETE CASCADE)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = m_ecdb.ExecuteDdl("CREATE UNIQUE INDEX uix_ec_SchemaReference_SchemaId_ReferencedSchemaId ON " TABLE_SchemaReference "(SchemaId,ReferencedSchemaId);"
                           "CREATE INDEX ix_ec_SchemaReference_ReferencedSchemaId ON " TABLE_SchemaReference "(ReferencedSchemaId)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Class
    stat = m_ecdb.ExecuteDdl("CREATE TABLE " TABLE_Class
                           "(Id INTEGER PRIMARY KEY,"
                           "SchemaId INTEGER NOT NULL REFERENCES " TABLE_Schema "(Id) ON DELETE CASCADE,"
                           "Name TEXT NOT NULL COLLATE NOCASE,"
                           "DisplayLabel TEXT,"
                           "Description TEXT,"
                           "Type INTEGER NOT NULL,"
                           "Modifier INTEGER NOT NULL,"
                           "RelationshipStrength INTEGER,"
                           "RelationshipStrengthDirection INTEGER,"
                           "CustomAttributeContainerType INTEGER)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = m_ecdb.ExecuteDdl("CREATE INDEX ix_ec_Class_SchemaId_Name ON " TABLE_Class "(SchemaId,Name);"
                           "CREATE INDEX ix_ec_Class_Name ON " TABLE_Class "(Name);");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_ClassHasBaseClasses
    stat = m_ecdb.ExecuteDdl("CREATE TABLE " TABLE_ClassHasBaseClasses
                           "(Id INTEGER PRIMARY KEY,"
                           "ClassId INTEGER NOT NULL REFERENCES " TABLE_Class "(Id) ON DELETE CASCADE,"
                           "BaseClassId INTEGER NOT NULL REFERENCES " TABLE_Class "(Id) ON DELETE CASCADE,"
                           "Ordinal INTEGER NOT NULL)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = m_ecdb.ExecuteDdl("CREATE UNIQUE INDEX uix_ec_ClassHasBaseClasses_ClassId_BaseClassId ON " TABLE_ClassHasBaseClasses "(ClassId,BaseClassId);"
                           "CREATE INDEX ix_ec_ClassHasBaseClasses_BaseClassId ON " TABLE_ClassHasBaseClasses "(BaseClassId);"
                           "CREATE UNIQUE INDEX uix_ec_ClassHasBaseClasses_ClassId_Ordinal ON " TABLE_ClassHasBaseClasses "(ClassId,Ordinal)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Enumeration
    stat = m_ecdb.ExecuteDdl("CREATE TABLE " TABLE_Enumeration
                           "(Id INTEGER PRIMARY KEY,"
                           "SchemaId INTEGER NOT NULL REFERENCES " TABLE_Schema "(Id) ON DELETE CASCADE,"
                           "Name TEXT NOT NULL COLLATE NOCASE,"
                           "DisplayLabel TEXT,"
                           "Description TEXT,"
                           "UnderlyingPrimitiveType INTEGER NOT NULL,"
                           "IsStrict BOOLEAN NOT NULL CHECK(IsStrict IN (" SQLVAL_False "," SQLVAL_True ")),"
                           "EnumValues TEXT NOT NULL)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = m_ecdb.ExecuteDdl("CREATE INDEX ix_ec_Enumeration_SchemaId ON " TABLE_Enumeration "(SchemaId);"
                           "CREATE INDEX ix_ec_Enumeration_Name ON " TABLE_Enumeration "(Name);");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_KindOfQuantity
    stat = m_ecdb.ExecuteDdl("CREATE TABLE " TABLE_KindOfQuantity
                           "(Id INTEGER PRIMARY KEY,"
                           "SchemaId INTEGER NOT NULL REFERENCES " TABLE_Schema "(Id) ON DELETE CASCADE,"
                           "Name TEXT NOT NULL COLLATE NOCASE,"
                           "DisplayLabel TEXT,"
                           "Description TEXT,"
                           "PersistenceUnit TEXT NOT NULL,"
                           "RelativeError REAL NOT NULL,"
                           "PresentationUnits TEXT)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = m_ecdb.ExecuteDdl("CREATE INDEX ix_ec_KindOfQuantity_SchemaId ON " TABLE_KindOfQuantity "(SchemaId);"
                           "CREATE INDEX ix_ec_KindOfQuantity_Name ON " TABLE_KindOfQuantity "(Name);");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_UnitSystem
    stat = m_ecdb.ExecuteDdl(TABLEDDL_UnitSystem);
    if (BE_SQLITE_OK != stat)
        return stat;


    //ec_Phenomenon
    stat = m_ecdb.ExecuteDdl(TABLEDDL_Phenomenon);
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Unit
    stat = m_ecdb.ExecuteDdl(TABLEDDL_Unit);
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Format
    stat = m_ecdb.ExecuteDdl(TABLEDDL_Format);
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_FormatCompositeUnit
    stat = m_ecdb.ExecuteDdl(TABLEDDL_FormatCompositeUnit);
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_PropertyCategory
    stat = m_ecdb.ExecuteDdl("CREATE TABLE " TABLE_PropertyCategory
                           "(Id INTEGER PRIMARY KEY,"
                           "SchemaId INTEGER NOT NULL REFERENCES " TABLE_Schema "(Id) ON DELETE CASCADE,"
                           "Name TEXT NOT NULL COLLATE NOCASE,"
                           "DisplayLabel TEXT,"
                           "Description TEXT,"
                           "Priority INTEGER)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = m_ecdb.ExecuteDdl("CREATE INDEX ix_ec_PropertyCategory_SchemaId ON " TABLE_PropertyCategory "(SchemaId);"
                           "CREATE INDEX ix_ec_PropertyCategory_Name ON " TABLE_PropertyCategory "(Name);");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Property
    stat = m_ecdb.ExecuteDdl("CREATE TABLE " TABLE_Property
                           "(Id INTEGER PRIMARY KEY,"
                           "ClassId INTEGER NOT NULL REFERENCES " TABLE_Class "(Id) ON DELETE CASCADE,"
                           "Name TEXT NOT NULL COLLATE NOCASE,"
                           "DisplayLabel TEXT,"
                           "Description TEXT,"
                           "IsReadonly BOOLEAN NOT NULL CHECK (IsReadonly IN (" SQLVAL_False "," SQLVAL_True ")),"
                           "Priority INTEGER,"
                           "Ordinal INTEGER NOT NULL,"
                           "Kind INTEGER NOT NULL,"
                           "PrimitiveType INTEGER,"
                           "PrimitiveTypeMinLength INTEGER,"
                           "PrimitiveTypeMaxLength INTEGER,"
                           "PrimitiveTypeMinValue NUMERIC,"
                           "PrimitiveTypeMaxValue NUMERIC,"
                           "EnumerationId INTEGER REFERENCES " TABLE_Enumeration "(Id) ON DELETE CASCADE,"
                           "StructClassId INTEGER REFERENCES " TABLE_Class "(Id) ON DELETE CASCADE,"
                           "ExtendedTypeName TEXT,"
                           "KindOfQuantityId INTEGER REFERENCES " TABLE_KindOfQuantity "(Id) ON DELETE CASCADE,"
                           "CategoryId INTEGER REFERENCES " TABLE_PropertyCategory "(Id) ON DELETE CASCADE,"
                           "ArrayMinOccurs INTEGER,"
                           "ArrayMaxOccurs INTEGER,"
                           "NavigationRelationshipClassId INTEGER REFERENCES " TABLE_Class "(Id) ON DELETE CASCADE,"
                           "NavigationDirection INTEGER)");

    if (BE_SQLITE_OK != stat)
        return stat;

    stat = m_ecdb.ExecuteDdl("CREATE UNIQUE INDEX uix_ec_Property_ClassId_Name ON " TABLE_Property "(ClassId,Name);"
                           "CREATE UNIQUE INDEX uix_ec_Property_ClassId_Ordinal ON " TABLE_Property "(ClassId,Ordinal);"
                           "CREATE INDEX ix_ec_Property_Name ON " TABLE_Property "(Name);"
                           "CREATE INDEX ix_ec_Property_EnumerationId ON " TABLE_Property "(EnumerationId);"
                           "CREATE INDEX ix_ec_Property_StructClassId ON " TABLE_Property "(StructClassId);"
                           "CREATE INDEX ix_ec_Property_KindOfQuantityId ON " TABLE_Property "(KindOfQuantityId);"
                           "CREATE INDEX ix_ec_Property_CategoryId ON " TABLE_Property "(CategoryId);"
                           "CREATE INDEX ix_ec_Property_NavigationRelationshipClassId ON " TABLE_Property "(NavigationRelationshipClassId);");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_PropertyPath
    stat = m_ecdb.ExecuteDdl("CREATE Table " TABLE_PropertyPath
                           "(Id INTEGER PRIMARY KEY,"
                           "RootPropertyId INTEGER NOT NULL REFERENCES " TABLE_Property "(Id) ON DELETE CASCADE,"
                           "AccessString TEXT NOT NULL COLLATE NOCASE)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = m_ecdb.ExecuteDdl("CREATE UNIQUE INDEX uix_ec_PropertyPath_RootPropertyId_AccessString ON " TABLE_PropertyPath "(RootPropertyId,AccessString)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_RelationshipConstraint
    stat = m_ecdb.ExecuteDdl("CREATE TABLE " TABLE_RelationshipConstraint
                           "(Id INTEGER PRIMARY KEY,"
                           "RelationshipClassId INTEGER NOT NULL REFERENCES " TABLE_Class "(Id) ON DELETE CASCADE,"
                           "RelationshipEnd INTEGER NOT NULL,"
                           "MultiplicityLowerLimit INTEGER NOT NULL,"
                           "MultiplicityUpperLimit INTEGER,"
                           "IsPolymorphic BOOLEAN NOT NULL CHECK (IsPolymorphic IN (" SQLVAL_False "," SQLVAL_True ")),"
                           "RoleLabel TEXT,"
                           "AbstractConstraintClassId INTEGER REFERENCES " TABLE_Class "(Id) ON DELETE SET NULL)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = m_ecdb.ExecuteDdl("CREATE UNIQUE INDEX uix_ec_RelationshipConstraint_RelationshipClassId_RelationshipEnd ON " TABLE_RelationshipConstraint "(RelationshipClassId,RelationshipEnd);"
                           "CREATE INDEX ix_ec_RelationshipConstraint_AbstractConstraintClassId ON " TABLE_RelationshipConstraint "(AbstractConstraintClassId)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_RelationshipConstraintClass
    stat = m_ecdb.ExecuteDdl("CREATE TABLE " TABLE_RelationshipConstraintClass
                           "(Id INTEGER PRIMARY KEY,"
                           "ConstraintId INTEGER NOT NULL REFERENCES " TABLE_RelationshipConstraint "(Id) ON DELETE CASCADE,"
                           "ClassId INTEGER NOT NULL REFERENCES " TABLE_Class "(Id) ON DELETE CASCADE)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = m_ecdb.ExecuteDdl("CREATE UNIQUE INDEX uix_ec_RelationshipConstraintClass_ConstraintId_ClassId ON " TABLE_RelationshipConstraintClass "(ConstraintId,ClassId);"
                           "CREATE INDEX ix_ec_RelationshipConstraintClass_ClassId ON " TABLE_RelationshipConstraintClass "(ClassId)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_CustomAttribute
    stat = m_ecdb.ExecuteDdl("CREATE TABLE " TABLE_CustomAttribute
                           "(Id INTEGER PRIMARY KEY,"
                           "ClassId INTEGER NOT NULL REFERENCES " TABLE_Class "(Id) ON DELETE CASCADE,"
                           "ContainerId INTEGER NOT NULL,"
                           "ContainerType INTEGER NOT NULL,"
                           "Ordinal INTEGER NOT NULL,"
                           "Instance TEXT NOT NULL)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = m_ecdb.ExecuteDdl("CREATE UNIQUE INDEX uix_ec_CustomAttribute_ContainerId_ContainerType_Ordinal ON " TABLE_CustomAttribute "(ContainerId,ContainerType,Ordinal);"
                           "CREATE UNIQUE INDEX uix_ec_CustomAttribute_ContainerId_ContainerType_ClassId ON " TABLE_CustomAttribute "(ContainerId,ContainerType,ClassId);"
                           "CREATE INDEX ix_ec_CustomAttribute_ClassId ON " TABLE_CustomAttribute "(ClassId)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_ClassMap
    stat = m_ecdb.ExecuteDdl("CREATE Table " TABLE_ClassMap
                           "(ClassId INTEGER PRIMARY KEY REFERENCES " TABLE_Class "(Id) ON DELETE CASCADE,"
                           //resolved map strategy:
                           "MapStrategy INTEGER NOT NULL,"
                           "ShareColumnsMode INTEGER,"
                           "MaxSharedColumnsBeforeOverflow INTEGER,"
                           "JoinedTableInfo INTEGER)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_PropertyMap
    stat = m_ecdb.ExecuteDdl("CREATE Table " TABLE_PropertyMap
                           "(Id INTEGER PRIMARY KEY,"
                           "ClassId INTEGER NOT NULL REFERENCES " TABLE_ClassMap "(ClassId) ON DELETE CASCADE,"
                           "PropertyPathId INTEGER NOT NULL REFERENCES " TABLE_PropertyPath "(Id) ON DELETE CASCADE,"
                           "ColumnId INTEGER NOT NULL REFERENCES " TABLE_Column "(Id) ON DELETE CASCADE)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = m_ecdb.ExecuteDdl("CREATE UNIQUE INDEX uix_ec_PropertyMap_ClassId_PropertyPathId_ColumnId ON " TABLE_PropertyMap "(ClassId,PropertyPathId,ColumnId);"
                           "CREATE INDEX ix_ec_PropertyMap_PropertyPathId ON " TABLE_PropertyMap "(PropertyPathId);"
                           "CREATE INDEX ix_ec_PropertyMap_ColumnId ON " TABLE_PropertyMap "(ColumnId);");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Table
    stat = m_ecdb.ExecuteDdl("CREATE TABLE " TABLE_Table
                           "(Id INTEGER PRIMARY KEY,"
                           "ParentTableId INTEGER REFERENCES " TABLE_Table "(Id) ON DELETE CASCADE,"
                           "Name TEXT UNIQUE NOT NULL COLLATE NOCASE," //ECDb requires table names to be unique even across other db schema names
                           "Type INTEGER NOT NULL,"
                           "ExclusiveRootClassId INTEGER REFERENCES " TABLE_Class "(Id) ON DELETE SET NULL,"
                           "UpdatableViewName TEXT)"); //UpdatableViewName is not used anymore -> WIP_NEXTGEN_DELETE
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = m_ecdb.ExecuteDdl("CREATE INDEX ix_ec_Table_ParentTableId ON " TABLE_Table "(ParentTableId);"
                           "CREATE INDEX ix_ec_Table_ExclusiveRootClassId ON " TABLE_Table "(ExclusiveRootClassId);");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Column
    stat = m_ecdb.ExecuteDdl("CREATE TABLE " TABLE_Column
                           "(Id INTEGER PRIMARY KEY,"
                           "TableId INTEGER NOT NULL REFERENCES " TABLE_Table "(Id) ON DELETE CASCADE,"
                           "Name TEXT NOT NULL COLLATE NOCASE,"
                           "Type INTEGER NOT NULL,"
                           "IsVirtual BOOLEAN NOT NULL CHECK (IsVirtual IN (" SQLVAL_False "," SQLVAL_True ")),"
                           "Ordinal INTEGER NOT NULL,"
                           "NotNullConstraint BOOLEAN NOT NULL CHECK (NotNullConstraint IN (" SQLVAL_False "," SQLVAL_True ")),"
                           "UniqueConstraint BOOLEAN NOT NULL CHECK (UniqueConstraint IN (" SQLVAL_False "," SQLVAL_True ")),"
                           "CheckConstraint TEXT COLLATE NOCASE,"
                           "DefaultConstraint TEXT COLLATE NOCASE,"
                           "CollationConstraint INTEGER NOT NULL,"
                           "OrdinalInPrimaryKey INTEGER,"
                           "ColumnKind INTEGER NOT NULL)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = m_ecdb.ExecuteDdl("CREATE UNIQUE INDEX uix_ec_Column_TableId_Name ON " TABLE_Column "(TableId,Name);"
                           "CREATE UNIQUE INDEX uix_ec_Column_TableId_Ordinal ON " TABLE_Column "(TableId,Ordinal)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Index
    stat = m_ecdb.ExecuteDdl("CREATE TABLE " TABLE_Index
                           "(Id INTEGER PRIMARY KEY,"
                           "Name TEXT UNIQUE NOT NULL COLLATE NOCASE,"
                           "TableId INTEGER NOT NULL REFERENCES " TABLE_Table "(Id) ON DELETE CASCADE,"
                           "IsUnique BOOLEAN NOT NULL CHECK (IsUnique IN (" SQLVAL_False "," SQLVAL_True ")),"
                           "AddNotNullWhereExp BOOLEAN NOT NULL CHECK (AddNotNullWhereExp IN (" SQLVAL_False "," SQLVAL_True ")), "
                           "IsAutoGenerated BOOLEAN NOT NULL CHECK (IsAutoGenerated IN (" SQLVAL_False "," SQLVAL_True ")),"
                           "ClassId INTEGER REFERENCES " TABLE_Class "(Id) ON DELETE CASCADE,"
                           "AppliesToSubclassesIfPartial BOOLEAN NOT NULL CHECK (AppliesToSubclassesIfPartial IN (" SQLVAL_False "," SQLVAL_True ")))");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = m_ecdb.ExecuteDdl("CREATE INDEX ix_ec_Index_TableId ON " TABLE_Index "(TableId);"
                           "CREATE INDEX ix_ec_Index_ClassId ON " TABLE_Index "(ClassId)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_IndexColumn
    stat = m_ecdb.ExecuteDdl("CREATE TABLE " TABLE_IndexColumn
                           "(Id INTEGER PRIMARY KEY,"
                           "IndexId INTEGER NOT NULL REFERENCES " TABLE_Index " (Id) ON DELETE CASCADE,"
                           "ColumnId INTEGER NOT NULL REFERENCES " TABLE_Column " (Id) ON DELETE CASCADE,"
                           "Ordinal INTEGER NOT NULL)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = m_ecdb.ExecuteDdl("CREATE UNIQUE INDEX uix_ec_IndexColumn_IndexId_ColumnId_Ordinal ON " TABLE_IndexColumn "(IndexId,ColumnId,Ordinal);"
                           "CREATE INDEX ix_ec_IndexColumn_IndexId_Ordinal ON " TABLE_IndexColumn "(IndexId,Ordinal);"
                           "CREATE INDEX ix_ec_IndexColumn_ColumnId ON " TABLE_IndexColumn "(ColumnId)");

    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_cache_ClassHasTables
    stat = m_ecdb.ExecuteDdl("CREATE TABLE " TABLE_ClassHasTablesCache "("
                           "Id INTEGER PRIMARY KEY,"
                           "ClassId INTEGER NOT NULL REFERENCES " TABLE_Class "(Id) ON DELETE CASCADE,"
                           "TableId INTEGER NOT NULL REFERENCES " TABLE_Table "(Id) ON DELETE CASCADE)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = m_ecdb.ExecuteDdl("CREATE INDEX ix_ec_cache_ClassHasTables_ClassId_TableId ON " TABLE_ClassHasTablesCache "(ClassId);"
                           "CREATE INDEX ix_ec_cache_ClassHasTables_TableId ON " TABLE_ClassHasTablesCache "(TableId);");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_cache_ClassHierarchy
    stat = m_ecdb.ExecuteDdl("CREATE TABLE " TABLE_ClassHierarchyCache "("
                           "Id INTEGER PRIMARY KEY,"
                           "ClassId INTEGER NOT NULL REFERENCES " TABLE_Class "(Id) ON DELETE CASCADE,"
                           "BaseClassId INTEGER NOT NULL REFERENCES " TABLE_Class "(Id) ON DELETE CASCADE)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = m_ecdb.ExecuteDdl("CREATE INDEX ix_ec_cache_ClassHierarchy_ClassId ON " TABLE_ClassHierarchyCache "(ClassId);"
                           "CREATE INDEX ix_ec_cache_ClassHierarchy_BaseClassId ON " TABLE_ClassHierarchyCache "(BaseClassId);");
    if(BE_SQLITE_OK != stat)
        return stat;

// Product Backlog Item 797894: Use delete trigger for cleaning up custom attributes
// https://tinyurl.com/bdzxrxe8
#ifdef WIP_USE_TRIGGER_FOR_CUSTOMATTRIBUTE_DELETION 
    // trigger to delete custom attributes for ec_Schema
    stat = m_ecdb.ExecuteDdl(R"sql(
        create trigger [ec_delete_custom_attributes_for_schema] before delete on [ec_Schema]
        begin
        delete from
            [ec_CustomAttribute]
        where
            [ContainerId] = [old].[Id] and [containerType] = 1;
        end;
    )sql");
    if(BE_SQLITE_OK != stat)
        return stat;

    // trigger to delete custom attributes for ec_Class
    stat = m_ecdb.ExecuteDdl(R"sql(
        create trigger [ec_delete_custom_attributes_for_class] before delete on [ec_Class]
        begin
        delete from
            [ec_CustomAttribute]
        where
            [ContainerId] = [old].[Id] and [containerType] = 30;
        end;
    )sql");
    if(BE_SQLITE_OK != stat)
        return stat;

    // trigger to delete custom attributes for ec_Property
    stat = m_ecdb.ExecuteDdl(R"sql(
        create trigger [ec_delete_custom_attributes_for_property] before delete on [ec_Property]
        begin
        delete from
            [ec_CustomAttribute]
        where
            [ContainerId] = [old].[Id] and [containerType] = 992;
        end;
    )sql");
    if(BE_SQLITE_OK != stat)
        return stat;

    // trigger to delete custom attributes for ec_RelationshipConstraint source
    stat = m_ecdb.ExecuteDdl(R"sql(
        create trigger [ec_delete_custom_attributes_for_relationship_constraint_source] before delete on [ec_RelationshipConstraint] when [old].[RelationshipEnd] = 0
        begin
        delete from
            [ec_CustomAttribute]
        where
            [ContainerId] = [old].[Id] and [containerType] = 1024;
        end;
    )sql");
    if(BE_SQLITE_OK != stat)
        return stat;

    // trigger to delete custom attributes for ec_RelationshipConstraint target
    stat = m_ecdb.ExecuteDdl(R"sql(
        create trigger [ec_delete_custom_attributes_for_relationship_constraint_target] before delete on [ec_RelationshipConstraint] when [old].[RelationshipEnd] = 1
        begin
        delete from
            [ec_CustomAttribute]
        where
            [ContainerId] = [old].[Id] and [containerType] = 2048;
        end;
    )sql");
    if(BE_SQLITE_OK != stat)
        return stat;
#endif
    return BE_SQLITE_OK;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
