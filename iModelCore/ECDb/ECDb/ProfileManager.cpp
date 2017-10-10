/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ProfileManager.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define PROFILENAME "ECDb"

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2012
//+---------------+---------------+---------------+---------------+---------------+-
//static
DbResult ProfileManager::CreateProfile(ECDbR ecdb)
    {
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("Begin CreateECProfile");

    if (!ecdb.GetDefaultTransaction()->IsActive())
        {
        //we always need a transaction to execute SQLite statements. If ECDb was opened in no-default-trans mode, we need to
        //begin a transaction ourselves (just use BeSQLite's default transaction which is always there even in no-default-trans mode,
        //except that in that case, it is not active).
        BeAssert(false && "Programmer Error. ECDb expects that BeSqlite::CreateNewDb keeps the default transaction active when it is called to create its profile.");
        return BE_SQLITE_ERROR_NoTxnActive;
        }

    DbResult stat = CreateProfileTables(ecdb);
    if (stat != BE_SQLITE_OK)
        {
        LOG.errorv("Failed to create " PROFILENAME " profile in %s: %s", ecdb.GetDbFileName(), ecdb.GetLastError().c_str());
        ecdb.AbandonChanges();
        return stat;
        }

    stat = ProfileSchemaUpgrader::ImportProfileSchemas(ecdb);
    if (stat != BE_SQLITE_OK)
        {
        ecdb.AbandonChanges();
        return stat;
        }

    stat = AssignProfileVersion(ecdb, true);
    if (stat != BE_SQLITE_OK)
        {
        ecdb.AbandonChanges();
        LOG.errorv("Failed to create " PROFILENAME " profile in file '%s'. Could not assign new profile version. %s",
                   ecdb.GetDbFileName(), ecdb.GetLastError().c_str());
        return stat;
        }

    ecdb.SaveChanges();
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("End CreateECProfile");
    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle  01/2017
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ProfileManager::CheckProfileVersion(bool& fileIsAutoUpgradable, ProfileVersion& actualProfileVersion, ECDbCR ecdb, bool openModeIsReadOnly)
    {
    BeAssert(ecdb.GetDefaultTransaction()->IsActive() && "Should have been caught before. Callers must ensure that a transaction is active before calling this.");

    fileIsAutoUpgradable = false;
    const DbResult stat = ReadProfileVersion(actualProfileVersion, ecdb);
    if (BE_SQLITE_OK != stat)
        return stat; //File is no ECDb file, i.e. doesn't have the ECDb profile

    return ECDb::CheckProfileVersion(fileIsAutoUpgradable, GetExpectedVersion(), actualProfileVersion, GetMinimumSupportedVersion(), openModeIsReadOnly, PROFILENAME);
    }

//=======================================================================================
//! Whenever a profile upgrade needs to rename or remove tables/columns
//! a ProfileUpgradeContext is needed which prepares SQLite accordingly, 
//! e.g. foreign key enforcement is disabled during upgrade to not invalidate foreign key
//! constraints when altering tables.
// @bsiclass                                                 Krischan.Eberle      07/2013
//+===============+===============+===============+===============+===============+======
struct ProfileUpgradeContext final: NonCopyableClass
    {
    private:
        ECDbR m_ecdb;
        Savepoint& m_defaultTransaction;
        bool m_isDefaultTransOpen;
        bool m_rollbackOnDestruction;
        DbResult m_beginTransError;

        void DisableForeignKeyEnforcement();
        void EnableForeignKeyEnforcement() const;
    public:
        ProfileUpgradeContext(ECDbR ecdb, Savepoint& defaultTransaction);
        ~ProfileUpgradeContext();

        //! Be default the upgrade transaction is rolled back when the context is destroyed.
        //! Be calling this method, the transaction is committed when the context is destroyed.
       // void SetCommitAfterUpgrade() { m_rollbackOnDestruction = false; }

     //   DbResult GetBeginTransError() const { return m_beginTransError; }
    };

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2013
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ProfileManager::UpgradeProfile(ECDbR ecdb, Db::OpenParams const& openParams)
    {
    if (!ecdb.GetDefaultTransaction()->IsActive())
        {
        BeAssert(false && "Programmer Error. ECDb expects that BeSqlite::OpenBeSQliteDb keeps the default transaction active when it is called to upgrade its profile.");
        return BE_SQLITE_ERROR_NoTxnActive;
        }

    bool runProfileUpgrade = false;
    ProfileVersion actualProfileVersion(0, 0, 0, 0);
    DbResult stat = CheckProfileVersion(runProfileUpgrade, actualProfileVersion, ecdb, openParams.IsReadonly());
    if (!runProfileUpgrade)
        return stat;

    PERFLOG_START("ECDb", "Profile upgrade");

    //if ECDb file is readonly, reopen it in read-write mode
    if (!openParams._ReopenForProfileUpgrade(ecdb))
        {
        LOG.errorv("Upgrade of file's " PROFILENAME " profile failed because file could not be re-opened in read-write mode.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailedCannotOpenForWrite;
        }

    BeAssert(!ecdb.IsReadonly());

    //let upgraders incrementally upgrade the profile
    //to the latest state
    if (BE_SQLITE_OK != RunUpgraders(ecdb, actualProfileVersion))
        {
        ecdb.AbandonChanges();
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (BE_SQLITE_OK != ProfileSchemaUpgrader::ImportProfileSchemas(ecdb))
        {
        ecdb.AbandonChanges();
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    //after upgrade procedure set new profile version in ECDb file
    stat = AssignProfileVersion(ecdb, false);

    if (stat != BE_SQLITE_OK)
        {
        ecdb.AbandonChanges();
        LOG.errorv("Failed to upgrade " PROFILENAME " profile in file '%s'. Could not assign new profile version. %s",
                   ecdb.GetDbFileName(), ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (LOG.isSeverityEnabled(NativeLogging::LOG_INFO))
        {
        LOG.infov("Upgraded " PROFILENAME " profile from version %s to version %s in file '%s'.",
                  actualProfileVersion.ToString().c_str(), GetExpectedVersion().ToString().c_str(), ecdb.GetDbFileName());
        }

    PERFLOG_FINISH("ECDb", "Profile upgrade");
    return BE_SQLITE_OK;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle  06/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ProfileManager::RunUpgraders(ECDbCR ecdb, ProfileVersion const& currentProfileVersion)
    {
    //IMPORTANT: order from low to high version
    //Note: If, for a version there is no upgrader it means just one of the profile ECSchemas needs to be reimported.
    std::vector<std::unique_ptr<ProfileUpgrader>> upgraders;
    
    for (std::unique_ptr<ProfileUpgrader> const& upgrader : upgraders)
        {
        DbResult stat = upgrader->Upgrade(ecdb);
        if (BE_SQLITE_OK != stat)
            return stat;
        }

    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ProfileManager::AssignProfileVersion(ECDbR ecdb, bool onProfileCreation)
    {
    //Save the profile version as string (JSON format)
    Utf8String profileVersionStr = GetExpectedVersion().ToJson();

    if (onProfileCreation)
        {
        DbResult stat = ecdb.SavePropertyString(GetInitialProfileVersionPropertySpec(), profileVersionStr);
        if (BE_SQLITE_OK != stat)
            return stat;
        }

    return ecdb.SavePropertyString(GetProfileVersionPropertySpec(), profileVersionStr);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2013
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ProfileManager::ReadProfileVersion(ProfileVersion& profileVersion, ECDbCR ecdb)
    {
    Utf8String currentVersionString;
    //if version entry does not exist, this either means it is ECDb profile 1.0 (because we did not store
    // a version entry for profile 1.0 or it isn't an ECDb file at all. In order to tell these we need
    // to check for a typical table of the ECDb profile:
    if (BE_SQLITE_ROW == ecdb.QueryProperty(currentVersionString, GetProfileVersionPropertySpec()))
        {
        profileVersion.FromJson(currentVersionString.c_str());
        return BE_SQLITE_OK;
        }

    if (ecdb.TableExists("ec_Schema"))
        {
        profileVersion = ProfileVersion(1, 0, 0, 0);
        return BE_SQLITE_OK;
        }

    //File is no ECDb file
    return BE_SQLITE_ERROR_InvalidProfileVersion;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ProfileManager::CreateProfileTables(ECDbCR ecdb)
    {
    //ec_Schema
    DbResult stat = ecdb.ExecuteSql("CREATE TABLE " TABLE_Schema
                                    "(Id INTEGER PRIMARY KEY,"
                                    "Name TEXT UNIQUE NOT NULL COLLATE NOCASE,"
                                    "DisplayLabel TEXT,"
                                    "Description TEXT,"
                                    "Alias TEXT UNIQUE NOT NULL COLLATE NOCASE,"
                                    "VersionDigit1 INTEGER NOT NULL,"
                                    "VersionDigit2 INTEGER NOT NULL,"
                                    "VersionDigit3 INTEGER NOT NULL)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_SchemaReference
    stat = ecdb.ExecuteSql("CREATE TABLE " TABLE_SchemaReference
                           "(Id INTEGER PRIMARY KEY,"
                           "SchemaId INTEGER REFERENCES " TABLE_Schema "(Id) ON DELETE CASCADE,"
                           "ReferencedSchemaId INTEGER REFERENCES " TABLE_Schema "(Id) ON DELETE CASCADE)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE UNIQUE INDEX uix_ec_SchemaReference_SchemaId_ReferencedSchemaId ON " TABLE_SchemaReference "(SchemaId,ReferencedSchemaId);"
                           "CREATE INDEX ix_ec_SchemaReference_ReferencedSchemaId ON " TABLE_SchemaReference "(ReferencedSchemaId)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Class
    stat = ecdb.ExecuteSql("CREATE TABLE " TABLE_Class
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

    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_Class_SchemaId_Name ON " TABLE_Class "(SchemaId,Name);"
                           "CREATE INDEX ix_ec_Class_Name ON " TABLE_Class "(Name);");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_ClassHasBaseClasses
    stat = ecdb.ExecuteSql("CREATE TABLE " TABLE_ClassHasBaseClasses
                           "(Id INTEGER PRIMARY KEY,"
                           "ClassId INTEGER NOT NULL REFERENCES " TABLE_Class "(Id) ON DELETE CASCADE,"
                           "BaseClassId INTEGER NOT NULL REFERENCES " TABLE_Class "(Id) ON DELETE CASCADE,"
                           "Ordinal INTEGER NOT NULL)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE UNIQUE INDEX uix_ec_ClassHasBaseClasses_ClassId_BaseClassId ON " TABLE_ClassHasBaseClasses "(ClassId,BaseClassId);"
                           "CREATE INDEX ix_ec_ClassHasBaseClasses_BaseClassId ON " TABLE_ClassHasBaseClasses "(BaseClassId);"
                           "CREATE UNIQUE INDEX uix_ec_ClassHasBaseClasses_ClassId_Ordinal ON " TABLE_ClassHasBaseClasses "(ClassId,Ordinal)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Enumeration
    stat = ecdb.ExecuteSql("CREATE TABLE " TABLE_Enumeration
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

    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_Enumeration_SchemaId ON " TABLE_Enumeration "(SchemaId);"
                           "CREATE INDEX ix_ec_Enumeration_Name ON " TABLE_Enumeration "(Name);");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_KindOfQuantity
    stat = ecdb.ExecuteSql("CREATE TABLE " TABLE_KindOfQuantity
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

    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_KindOfQuantity_SchemaId ON " TABLE_KindOfQuantity "(SchemaId);"
                           "CREATE INDEX ix_ec_KindOfQuantity_Name ON " TABLE_KindOfQuantity "(Name);");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_PropertyCategory
    stat = ecdb.ExecuteSql("CREATE TABLE " TABLE_PropertyCategory
                           "(Id INTEGER PRIMARY KEY,"
                           "SchemaId INTEGER NOT NULL REFERENCES " TABLE_Schema "(Id) ON DELETE CASCADE,"
                           "Name TEXT NOT NULL COLLATE NOCASE,"
                           "DisplayLabel TEXT,"
                           "Description TEXT,"
                           "Priority INTEGER)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_PropertyCategory_SchemaId ON " TABLE_PropertyCategory "(SchemaId);"
                           "CREATE INDEX ix_ec_PropertyCategory_Name ON " TABLE_PropertyCategory "(Name);");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Property
    stat = ecdb.ExecuteSql("CREATE TABLE " TABLE_Property
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

    stat = ecdb.ExecuteSql("CREATE UNIQUE INDEX uix_ec_Property_ClassId_Name ON " TABLE_Property "(ClassId,Name);"
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
    stat = ecdb.ExecuteSql("CREATE Table " TABLE_PropertyPath
                           "(Id INTEGER PRIMARY KEY,"
                           "RootPropertyId INTEGER NOT NULL REFERENCES " TABLE_Property "(Id) ON DELETE CASCADE,"
                           "AccessString TEXT NOT NULL COLLATE NOCASE)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE UNIQUE INDEX uix_ec_PropertyPath_RootPropertyId_AccessString ON " TABLE_PropertyPath "(RootPropertyId,AccessString)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_RelationshipConstraint
    stat = ecdb.ExecuteSql("CREATE TABLE " TABLE_RelationshipConstraint
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

    stat = ecdb.ExecuteSql("CREATE UNIQUE INDEX uix_ec_RelationshipConstraint_RelationshipClassId_RelationshipEnd ON " TABLE_RelationshipConstraint "(RelationshipClassId,RelationshipEnd);"
                           "CREATE INDEX ix_ec_RelationshipConstraint_AbstractConstraintClassId ON " TABLE_RelationshipConstraint "(AbstractConstraintClassId)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_RelationshipConstraintClass
    stat = ecdb.ExecuteSql("CREATE TABLE " TABLE_RelationshipConstraintClass
                           "(Id INTEGER PRIMARY KEY,"
                           "ConstraintId INTEGER NOT NULL REFERENCES " TABLE_RelationshipConstraint "(Id) ON DELETE CASCADE,"
                           "ClassId INTEGER NOT NULL REFERENCES " TABLE_Class "(Id) ON DELETE CASCADE)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE UNIQUE INDEX uix_ec_RelationshipConstraintClass_ConstraintId_ClassId ON " TABLE_RelationshipConstraintClass "(ConstraintId,ClassId);"
                           "CREATE INDEX ix_ec_RelationshipConstraintClass_ClassId ON " TABLE_RelationshipConstraintClass "(ClassId)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_CustomAttribute
    stat = ecdb.ExecuteSql("CREATE TABLE " TABLE_CustomAttribute
                           "(Id INTEGER PRIMARY KEY,"
                           "ClassId INTEGER NOT NULL REFERENCES " TABLE_Class "(Id) ON DELETE CASCADE,"
                           "ContainerId INTEGER NOT NULL,"
                           "ContainerType INTEGER NOT NULL,"
                           "Ordinal INTEGER NOT NULL,"
                           "Instance TEXT NOT NULL)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE UNIQUE INDEX uix_ec_CustomAttribute_ContainerId_ContainerType_Ordinal ON " TABLE_CustomAttribute "(ContainerId,ContainerType,Ordinal);"
                           "CREATE UNIQUE INDEX uix_ec_CustomAttribute_ContainerId_ContainerType_ClassId ON " TABLE_CustomAttribute "(ContainerId,ContainerType,ClassId);"
                           "CREATE INDEX ix_ec_CustomAttribute_ClassId ON " TABLE_CustomAttribute "(ClassId)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_ClassMap
    stat = ecdb.ExecuteSql("CREATE Table " TABLE_ClassMap
                           "(ClassId INTEGER PRIMARY KEY REFERENCES " TABLE_Class "(Id) ON DELETE CASCADE,"
                           //resolved map strategy:
                           "MapStrategy INTEGER NOT NULL,"
                           "ShareColumnsMode INTEGER,"
                           "MaxSharedColumnsBeforeOverflow INTEGER,"
                           "JoinedTableInfo INTEGER)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_PropertyMap
    stat = ecdb.ExecuteSql("CREATE Table " TABLE_PropertyMap
                           "(Id INTEGER PRIMARY KEY,"
                           "ClassId INTEGER NOT NULL REFERENCES " TABLE_ClassMap "(ClassId) ON DELETE CASCADE,"
                           "PropertyPathId INTEGER NOT NULL REFERENCES " TABLE_PropertyPath "(Id) ON DELETE CASCADE,"
                           "ColumnId INTEGER NOT NULL REFERENCES " TABLE_Column "(Id) ON DELETE CASCADE)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE UNIQUE INDEX uix_ec_PropertyMap_ClassId_PropertyPathId_ColumnId ON " TABLE_PropertyMap "(ClassId,PropertyPathId,ColumnId);"
                           "CREATE INDEX ix_ec_PropertyMap_PropertyPathId ON " TABLE_PropertyMap "(PropertyPathId);"
                           "CREATE INDEX ix_ec_PropertyMap_ColumnId ON " TABLE_PropertyMap "(ColumnId);");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Table
    stat = ecdb.ExecuteSql("CREATE TABLE " TABLE_Table
                           "(Id INTEGER PRIMARY KEY,"
                           "ParentTableId INTEGER REFERENCES " TABLE_Table "(Id) ON DELETE CASCADE,"
                           "Name TEXT UNIQUE NOT NULL COLLATE NOCASE,"
                           "Type INTEGER NOT NULL,"
                           "ExclusiveRootClassId INTEGER REFERENCES " TABLE_Class "(Id) ON DELETE SET NULL,"
                           "UpdatableViewName TEXT)"); //UpdatableViewName is not used right now
if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_Table_ParentTableId ON " TABLE_Table "(ParentTableId);"
                           "CREATE INDEX ix_ec_Table_ExclusiveRootClassId ON " TABLE_Table "(ExclusiveRootClassId);");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Column
    stat = ecdb.ExecuteSql("CREATE TABLE " TABLE_Column
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

    stat = ecdb.ExecuteSql("CREATE UNIQUE INDEX uix_ec_Column_TableId_Name ON " TABLE_Column "(TableId,Name);"
                           "CREATE UNIQUE INDEX uix_ec_Column_TableId_Ordinal ON " TABLE_Column "(TableId,Ordinal)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Index
    stat = ecdb.ExecuteSql("CREATE TABLE " TABLE_Index
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

    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_Index_TableId ON " TABLE_Index "(TableId);"
                           "CREATE INDEX ix_ec_Index_ClassId ON " TABLE_Index "(ClassId)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_IndexColumn
    stat = ecdb.ExecuteSql("CREATE TABLE " TABLE_IndexColumn
                           "(Id INTEGER PRIMARY KEY,"
                           "IndexId INTEGER NOT NULL REFERENCES " TABLE_Index " (Id) ON DELETE CASCADE,"
                           "ColumnId INTEGER NOT NULL REFERENCES " TABLE_Column " (Id) ON DELETE CASCADE,"
                           "Ordinal INTEGER NOT NULL)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE UNIQUE INDEX uix_ec_IndexColumn_IndexId_ColumnId_Ordinal ON " TABLE_IndexColumn "(IndexId,ColumnId,Ordinal);"
                           "CREATE INDEX ix_ec_IndexColumn_IndexId_Ordinal ON " TABLE_IndexColumn "(IndexId,Ordinal);"
                           "CREATE INDEX ix_ec_IndexColumn_ColumnId ON " TABLE_IndexColumn "(ColumnId)");

    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_cache_ClassHasTables
    stat = ecdb.ExecuteSql("CREATE TABLE " TABLE_ClassHasTablesCache "("
                           "Id INTEGER PRIMARY KEY,"
                           "ClassId INTEGER NOT NULL REFERENCES " TABLE_Class "(Id) ON DELETE CASCADE,"
                           "TableId INTEGER NOT NULL REFERENCES " TABLE_Table "(Id) ON DELETE CASCADE)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_cache_ClassHasTables_ClassId_TableId ON " TABLE_ClassHasTablesCache "(ClassId);"
                           "CREATE INDEX ix_ec_cache_ClassHasTables_TableId ON " TABLE_ClassHasTablesCache "(TableId);");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_cache_ClassHierarchy
    stat = ecdb.ExecuteSql("CREATE TABLE " TABLE_ClassHierarchyCache "("
                           "Id INTEGER PRIMARY KEY,"
                           "ClassId INTEGER NOT NULL REFERENCES " TABLE_Class "(Id) ON DELETE CASCADE,"
                           "BaseClassId INTEGER NOT NULL REFERENCES " TABLE_Class "(Id) ON DELETE CASCADE)");
    if (BE_SQLITE_OK != stat)
        return stat;

    return ecdb.ExecuteSql("CREATE INDEX ix_ec_cache_ClassHierarchy_ClassId ON " TABLE_ClassHierarchyCache "(ClassId);"
                           "CREATE INDEX ix_ec_cache_ClassHierarchy_BaseClassId ON " TABLE_ClassHierarchyCache "(BaseClassId);");
    }


//*************************************** ECDbProfileManager::ProfileUpgradeContext *************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ProfileUpgradeContext::ProfileUpgradeContext(ECDbR ecdb, Savepoint& defaultTransaction)
    : m_ecdb(ecdb), m_defaultTransaction(defaultTransaction), m_isDefaultTransOpen(defaultTransaction.IsActive()), m_rollbackOnDestruction(true)
    {
    DisableForeignKeyEnforcement();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ProfileUpgradeContext::~ProfileUpgradeContext()
    {
    EnableForeignKeyEnforcement();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void ProfileUpgradeContext::DisableForeignKeyEnforcement()
    {
    if (m_isDefaultTransOpen)
        m_defaultTransaction.Commit(nullptr);

    //Need to use TryExecuteSql which calls SQLite directly without any checks (Calling ExecuteSql would
    //check that a transaction is active which we explicity must not have for setting this pragma)
    auto stat = m_ecdb.TryExecuteSql("PRAGMA foreign_keys=OFF;");
    if (stat != BE_SQLITE_OK)
        {
        LOG.error("ECDb profile upgrade: Disabling foreign key enforcement in SQLite failed.");
        BeAssert(false);
        }

    // Start a transaction in immediate mode
    m_beginTransError = m_defaultTransaction.Begin(BeSQLiteTxnMode::Immediate);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void ProfileUpgradeContext::EnableForeignKeyEnforcement() const
    {
    if (m_rollbackOnDestruction)
        m_defaultTransaction.Cancel();
    else
        m_defaultTransaction.Commit(nullptr);

    //Need to use TryExecuteSql which calls SQLite directly without any checks (Calling ExecuteSql would
    //check that a transaction is active which we explicity must not have for setting this pragma)
    auto stat = m_ecdb.TryExecuteSql("PRAGMA foreign_keys=ON;");
    if (stat != BE_SQLITE_OK)
        {
        LOG.error("ECDb profile upgrade: Re-enabling foreign key enforcement in SQLite failed.");
        BeAssert(false);
        }

    if (m_isDefaultTransOpen)
        m_defaultTransaction.Begin();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
