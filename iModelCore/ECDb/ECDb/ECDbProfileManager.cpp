/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbProfileManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define PROFILENAME "ECDb"

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2012
//+---------------+---------------+---------------+---------------+---------------+-
//static
DbResult ECDbProfileManager::CreateECProfile(ECDbR ecdb)
    {
    LOG.debugv("Creating " PROFILENAME " profile in %s...", ecdb.GetDbFileName());

    STATEMENT_DIAGNOSTICS_LOGCOMMENT("Begin CreateECProfile");

    StopWatch timer(true);
    DbResult stat = CreateECProfileTables(ecdb);
    if (stat != BE_SQLITE_OK)
        {
        LOG.errorv("Failed to create " PROFILENAME " profile in %s: %s", ecdb.GetDbFileName(), ecdb.GetLastError().c_str());
        ecdb.AbandonChanges();
        return stat;
        }

    stat = ECDbProfileECSchemaUpgrader::ImportProfileSchemas(ecdb);
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
    timer.Stop();

    if (LOG.isSeverityEnabled(NativeLogging::LOG_INFO))
        LOG.infov("Created " PROFILENAME " profile (in %.4lf msecs) in '%s'.", timer.GetElapsedSeconds() * 1000.0, ecdb.GetDbFileName());

    STATEMENT_DIAGNOSTICS_LOGCOMMENT("End CreateECProfile");

    return BE_SQLITE_OK;
    }

//=======================================================================================
//! Whenever a profile upgrade needs to rename or remove tables/columns
//! a ProfileUpgradeContext is needed which prepares SQLite accordingly, 
//! e.g. foreign key enforcement is disabled during upgrade to not invalidate foreign key
//! constraints when altering tables.
// @bsiclass                                                 Krischan.Eberle      07/2013
//+===============+===============+===============+===============+===============+======
struct ProfileUpgradeContext : NonCopyableClass
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
        void SetCommitAfterUpgrade() { m_rollbackOnDestruction = false; }

        DbResult GetBeginTransError() const { return m_beginTransError; }
    };

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2013
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ECDbProfileManager::UpgradeECProfile(ECDbR ecdb, Db::OpenParams const& openParams)
    {
    StopWatch timer(true);

    SchemaVersion actualProfileVersion(0, 0, 0, 0);
    DbResult stat = ReadProfileVersion(actualProfileVersion, ecdb, *ecdb.GetDefaultTransaction());
    if (stat != BE_SQLITE_OK)
        return stat;       //File is no ECDb file, i.e. doesn't have the ECDb profile

    const SchemaVersion expectedVersion = GetExpectedVersion();
    bool profileNeedsUpgrade = false;
    stat = ECDb::CheckProfileVersion(profileNeedsUpgrade, expectedVersion, actualProfileVersion, GetMinimumSupportedVersion(), openParams.IsReadonly(), PROFILENAME);
    if (!profileNeedsUpgrade)
        return stat;

    //if ECDb file is readonly, reopen it in read-write mode
    if (!openParams._ReopenForSchemaUpgrade(ecdb))
        {
        LOG.errorv("Upgrade of file's " PROFILENAME " profile failed because file could not be re-opened in read-write mode.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailedCannotOpenForWrite;
        }

    BeAssert(!ecdb.IsReadonly());

    //Call upgrader sequence and let upgraders incrementally upgrade the profile
    //to the latest state
    std::vector<std::unique_ptr<ECDbProfileUpgrader>> upgraders;
    GetUpgraderSequence(upgraders, actualProfileVersion);
    for (std::unique_ptr<ECDbProfileUpgrader> const& upgrader : upgraders)
        {
        stat = upgrader->Upgrade(ecdb);
        if (BE_SQLITE_OK != stat)
            {
            ecdb.AbandonChanges();
            return stat;
            }
        }

    if (BE_SQLITE_OK != ECDbProfileECSchemaUpgrader::ImportProfileSchemas(ecdb))
        {
        ecdb.AbandonChanges();
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    //after upgrade procedure set new profile version in ECDb file
    stat = AssignProfileVersion(ecdb, false);

    timer.Stop();
    if (stat != BE_SQLITE_OK)
        {
        ecdb.AbandonChanges();
        LOG.errorv("Failed to upgrade " PROFILENAME " profile in file '%s'. Could not assign new profile version. %s",
                   ecdb.GetDbFileName(), ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (LOG.isSeverityEnabled(NativeLogging::LOG_INFO))
        {
        LOG.infov("Upgraded " PROFILENAME " profile from version %s to version %s (in %.4lf seconds) in file '%s'.",
                  actualProfileVersion.ToString().c_str(), expectedVersion.ToString().c_str(),
                  timer.GetElapsedSeconds(), ecdb.GetDbFileName());
        }

    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::AssignProfileVersion(ECDbR ecdb, bool onProfileCreation)
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
DbResult ECDbProfileManager::ReadProfileVersion(SchemaVersion& profileVersion, ECDbCR ecdb, Savepoint& defaultTransaction)
    {
    //we always need a transaction to execute SQLite statements. If ECDb was opened in no-default-trans mode, we need to
    //begin a transaction ourselves (just use BeSQLite's default transaction which is always there even in no-default-trans mode,
    //except that in that case, it is not active).
    const bool isDefaultTransactionActive = defaultTransaction.IsActive();
    if (!isDefaultTransactionActive)
        defaultTransaction.Begin();

    Utf8String currentVersionString;
    //if version entry does not exist, this either means it is ECDb profile 1.0 (because we did not store
    // a version entry for profile 1.0 or it isn't an ECDb file at all. In order to tell these we need
    // to check for a typical table of the ECDb profile:
    DbResult stat = BE_SQLITE_OK;
    if (BE_SQLITE_ROW == ecdb.QueryProperty(currentVersionString, GetProfileVersionPropertySpec()))
        profileVersion.FromJson(currentVersionString.c_str());
    else if (ecdb.TableExists("ec_Schema"))
        profileVersion = SchemaVersion(1, 0, 0, 0);
    else
        //File is no ECDb file
        stat = BE_SQLITE_ERROR_InvalidProfileVersion;

    //make sure to end default transaction again, if it wasn't active before this call
    if (!isDefaultTransactionActive)
        defaultTransaction.Commit(nullptr);

    return stat;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
void ECDbProfileManager::GetUpgraderSequence(std::vector<std::unique_ptr<ECDbProfileUpgrader>>& upgraders, SchemaVersion const& currentProfileVersion)
    {
    upgraders.clear();

    if (currentProfileVersion < SchemaVersion(3, 0, 0, 1))
        upgraders.push_back(std::unique_ptr<ECDbProfileUpgrader>(new ECDbProfileUpgrader_3001()));

    if (currentProfileVersion < SchemaVersion(3, 1, 0, 0))
        upgraders.push_back(std::unique_ptr<ECDbProfileUpgrader>(new ECDbProfileUpgrader_3100()));

    if (currentProfileVersion < SchemaVersion(3, 2, 0, 0))
        upgraders.push_back(std::unique_ptr<ECDbProfileUpgrader>(new ECDbProfileUpgrader_3200()));

    if (currentProfileVersion < SchemaVersion(3, 2, 0, 1))
        upgraders.push_back(std::unique_ptr<ECDbProfileUpgrader>(new ECDbProfileUpgrader_3201()));

    if (currentProfileVersion < SchemaVersion(3, 2, 0, 2))
        upgraders.push_back(std::unique_ptr<ECDbProfileUpgrader>(new ECDbProfileUpgrader_3202()));

    if (currentProfileVersion < SchemaVersion(3, 3, 0, 0))
        upgraders.push_back(std::unique_ptr<ECDbProfileUpgrader>(new ECDbProfileUpgrader_3300()));

    //No upgrader needed to upgrade to 3.3.0.1 as this step only requires the schemas to be reimported

    if (currentProfileVersion < SchemaVersion(3, 3, 0, 2))
        upgraders.push_back(std::unique_ptr<ECDbProfileUpgrader>(new ECDbProfileUpgrader_3302()));

    //No upgrader needed to upgrade to 3.3.0.3 as this step is not a hornswaggle and would be too much effort to implement

    if (currentProfileVersion < SchemaVersion(3, 4, 0, 0))
        upgraders.push_back(std::unique_ptr<ECDbProfileUpgrader>(new ECDbProfileUpgrader_3400()));

    if (currentProfileVersion < SchemaVersion(3, 5, 0, 0))
        upgraders.push_back(std::unique_ptr<ECDbProfileUpgrader>(new ECDbProfileUpgrader_3500()));

    if (currentProfileVersion < SchemaVersion(3, 6, 0, 0))
        upgraders.push_back(std::unique_ptr<ECDbProfileUpgrader>(new ECDbProfileUpgrader_3600()));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::CreateECProfileTables(ECDbCR ecdb)
    {
    //ec_Schema
    DbResult stat = ecdb.ExecuteSql("CREATE TABLE ec_Schema("
                                    "Id INTEGER PRIMARY KEY,"
                                    "Name TEXT UNIQUE NOT NULL COLLATE NOCASE,"
                                    "DisplayLabel TEXT,"
                                    "Description TEXT,"
                                    "NamespacePrefix TEXT UNIQUE NOT NULL COLLATE NOCASE,"
                                    "VersionDigit1 INTEGER NOT NULL,"
                                    "VersionDigit2 INTEGER NOT NULL,"
                                    "VersionDigit3 INTEGER NOT NULL)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_SchemaReference
    stat = ecdb.ExecuteSql("CREATE TABLE ec_SchemaReference("
                           "SchemaId INTEGER REFERENCES ec_Schema(Id) ON DELETE CASCADE,"
                           "ReferencedSchemaId INTEGER REFERENCES ec_Schema(Id) ON DELETE CASCADE)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_SchemaReference_SchemaId ON ec_SchemaReference(SchemaId);"
                           "CREATE INDEX ix_ec_SchemaReference_ReferencedSchemaId ON ec_SchemaReference(ReferencedSchemaId)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Class
    stat = ecdb.ExecuteSql("CREATE TABLE ec_Class("
                           "Id INTEGER PRIMARY KEY,"
                           "SchemaId INTEGER NOT NULL REFERENCES ec_Schema(Id) ON DELETE CASCADE,"
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

    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_Class_Name ON ec_Class(Name);"
                           "CREATE UNIQUE INDEX uix_ec_Class_SchemaId_Name ON ec_Class(SchemaId,Name)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_BaseClass
    stat = ecdb.ExecuteSql("CREATE TABLE ec_BaseClass("
                           "ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,"
                           "BaseClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,"
                           "Ordinal INTEGER NOT NULL, /*Location of baseclass in BaseClasses array*/"
                           "PRIMARY KEY (ClassId, BaseClassId))");
    if (BE_SQLITE_OK != stat)
        return stat;

    //index needed for fast look-ups of derived classes for a given ECClass
    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_BaseClass_BaseClassId ON ec_BaseClass(BaseClassId);"
                           "CREATE INDEX ix_ec_BaseClass_ClassId_Ordinal ON ec_BaseClass(ClassId,Ordinal)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Enumeration
    stat = ecdb.ExecuteSql("CREATE TABLE ec_Enumeration("
                           "Id INTEGER PRIMARY KEY,"
                           "SchemaId INTEGER NOT NULL REFERENCES ec_Schema(Id) ON DELETE CASCADE,"
                           "Name TEXT NOT NULL COLLATE NOCASE,"
                           "DisplayLabel TEXT,"
                           "Description TEXT,"
                           "UnderlyingPrimitiveType INTEGER NOT NULL,"
                           "IsStrict BOOLEAN NOT NULL CHECK(IsStrict IN (0,1)),"
                           "EnumValues TEXT NOT NULL)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE UNIQUE INDEX uix_ec_Enumeration_SchemaId_Name ON ec_Enumeration(SchemaId,Name)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Property
    stat = ecdb.ExecuteSql("CREATE TABLE ec_Property("
                           "Id INTEGER PRIMARY KEY,"
                           "ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,"
                           "Name TEXT NOT NULL COLLATE NOCASE,"
                           "DisplayLabel TEXT,"
                           "Description TEXT,"
                           "IsReadonly BOOLEAN NOT NULL CHECK (IsReadonly IN (0,1)),"
                           "Kind INTEGER NOT NULL,"
                           "Ordinal INTEGER,"
                           "PrimitiveType INTEGER,"
                           "Enumeration INTEGER REFERENCES ec_Enumeration(Id) ON DELETE CASCADE,"
                           "NonPrimitiveType INTEGER REFERENCES ec_Class(Id) ON DELETE CASCADE,"
                           "ExtendedType TEXT,"
                           "ArrayMinOccurs INTEGER,"
                           "ArrayMaxOccurs INTEGER,"
                           "NavigationPropertyDirection INTEGER)");

    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_Property_Name ON ec_Property(Name);"
                           "CREATE INDEX ix_ec_Property_ClassId_Ordinal ON ec_Property(ClassId,Ordinal);"
                           "CREATE INDEX ix_ec_Property_Enumeration ON ec_Property(Enumeration);"
                           "CREATE INDEX ix_ec_Property_NonPrimitiveType ON ec_Property(NonPrimitiveType);");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_PropertyPath
    stat = ecdb.ExecuteSql("CREATE Table ec_PropertyPath("
                           "Id INTEGER PRIMARY KEY,"
                           "RootPropertyId INTEGER NOT NULL REFERENCES ec_Property(Id) ON DELETE CASCADE,"
                           "AccessString TEXT NOT NULL COLLATE NOCASE)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE UNIQUE INDEX uix_ec_PropertyPath_RootPropertyId_AccessString ON ec_PropertyPath(RootPropertyId,AccessString)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_RelationshipConstraint
    stat = ecdb.ExecuteSql("CREATE TABLE ec_RelationshipConstraint("
                           "RelationshipClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,"
                           "RelationshipEnd INTEGER NOT NULL,"
                           "MultiplicityLowerLimit INTEGER,"
                           "MultiplicityUpperLimit INTEGER,"
                           "RoleLabel TEXT,"
                           "IsPolymorphic BOOLEAN NOT NULL CHECK (IsPolymorphic IN (0,1)),"
                           "PRIMARY KEY (RelationshipClassId, RelationshipEnd))");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_RelationshipConstraintClass
    stat = ecdb.ExecuteSql("CREATE TABLE ec_RelationshipConstraintClass("
                           "RelationshipClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,"
                           "RelationshipEnd INTEGER NOT NULL,"
                           "ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,"
                           "KeyProperties TEXT,"
                           "PRIMARY KEY (RelationshipClassId, RelationshipEnd, ClassId),"
                           "FOREIGN KEY (RelationshipClassId, RelationshipEnd) REFERENCES ec_RelationshipConstraint(RelationshipClassId, RelationshipEnd) ON DELETE CASCADE)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_CustomAttribute
    stat = ecdb.ExecuteSql("CREATE TABLE ec_CustomAttribute("
                           "ContainerId INTEGER NOT NULL,"
                           "ContainerType INTEGER NOT NULL,"
                           "Ordinal INTEGER NOT NULL,"
                           "ClassId INTEGER NOT NULL REFERENCES ec_Class(Id),"
                           "Instance TEXT NOT NULL,"
                           "PRIMARY KEY (ContainerId, ContainerType, ClassId))");
    if (BE_SQLITE_OK != stat)
        return stat;


    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_CustomAttribute_ContainerId_ContainerType_Ordinal ON ec_CustomAttribute(ContainerId,ContainerType,Ordinal);"
                           "CREATE INDEX ix_ec_CustomAttribute_ClassId ON ec_CustomAttribute(ClassId)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_ClassMap
    stat = ecdb.ExecuteSql("CREATE Table ec_ClassMap("
                           "Id INTEGER PRIMARY KEY,"
                           "ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,"
                           "ParentId INTEGER REFERENCES ec_ClassMap(Id) ON DELETE CASCADE,"
                           //resolved map strategy:
                           "MapStrategy INTEGER NOT NULL,"
                           "MapStrategyOptions INTEGER,"
                           "MapStrategyAppliesToSubclasses BOOLEAN NOT NULL CHECK (MapStrategyAppliesToSubclasses IN (0,1)),"
                           "MapStrategyMinSharedColumnCount INTEGER)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE UNIQUE INDEX uix_ec_ClassMap_ClassId_ParentId ON ec_ClassMap(ClassId, ParentId) WHERE ParentId IS NOT NULL;"
                           "CREATE INDEX ix_ec_ClassMap_ParentId ON ec_ClassMap(ParentId)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_PropertyMap
    stat = ecdb.ExecuteSql("CREATE Table ec_PropertyMap("
                           "ClassMapId INTEGER NOT NULL REFERENCES ec_ClassMap(Id) ON DELETE CASCADE,"
                           "PropertyPathId INTEGER NOT NULL REFERENCES ec_PropertyPath(Id) ON DELETE CASCADE,"
                           "ColumnId INTEGER NOT NULL REFERENCES ec_Column(Id) ON DELETE CASCADE,"
                           "PRIMARY KEY (ClassMapId, PropertyPathId, ColumnId))");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_PropertyMap_PropertyPathId ON ec_PropertyMap(PropertyPathId);"
                           "CREATE INDEX ix_ec_PropertyMap_ColumnId ON ec_PropertyMap(ColumnId);");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Table
    stat = ecdb.ExecuteSql("CREATE TABLE ec_Table("
                           "Id INTEGER PRIMARY KEY,"
                           "BaseTableId INTEGER REFERENCES ec_Table(Id) ON DELETE CASCADE,"
                           "Name TEXT UNIQUE NOT NULL COLLATE NOCASE,"
                           "Type INTEGER NOT NULL,"
                           "IsVirtual BOOLEAN NOT NULL CHECK (IsVirtual IN (0,1)))");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_Table_BaseTableId ON ec_Table(BaseTableId)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Column
    stat = ecdb.ExecuteSql("CREATE TABLE ec_Column("
                           "Id INTEGER PRIMARY KEY,"
                           "TableId INTEGER NOT NULL REFERENCES ec_Table(Id) ON DELETE CASCADE,"
                           "Name TEXT NOT NULL COLLATE NOCASE,"
                           "Type INTEGER NOT NULL,"
                           "IsVirtual BOOLEAN NOT NULL CHECK (IsVirtual IN (0,1)),"
                           "Ordinal INTEGER NOT NULL,"
                           "NotNullConstraint BOOLEAN,"
                           "UniqueConstraint BOOLEAN,"
                           "CheckConstraint TEXT COLLATE NOCASE,"
                           "DefaultConstraint TEXT COLLATE NOCASE,"
                           "CollationConstraint INTEGER,"
                           "OrdinalInPrimaryKey INTEGER,"
                           "ColumnKind INTEGER)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_Column_TableId_Ordinal ON ec_Column(TableId,Ordinal)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Index
    stat = ecdb.ExecuteSql("CREATE TABLE ec_Index("
                           "Id INTEGER PRIMARY KEY,"
                           "Name TEXT UNIQUE NOT NULL COLLATE NOCASE,"
                           "TableId INTEGER NOT NULL REFERENCES ec_Table(Id) ON DELETE CASCADE,"
                           "IsUnique BOOLEAN NOT NULL CHECK (IsUnique IN (0,1)),"
                           "AddNotNullWhereExp BOOLEAN NOT NULL CHECK (AddNotNullWhereExp IN (0,1)), "
                           "IsAutoGenerated BOOLEAN NOT NULL CHECK (IsAutoGenerated IN (0,1)),"
                           "ClassId INTEGER REFERENCES ec_Class(Id) ON DELETE CASCADE,"
                           "AppliesToSubclassesIfPartial BOOLEAN NOT NULL CHECK (AppliesToSubclassesIfPartial IN (0,1)))");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_Index_TableId ON ec_Index(TableId);"
                           "CREATE INDEX ix_ec_Index_ClassId ON ec_Index(ClassId)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_IndexColumn
    stat = ecdb.ExecuteSql("CREATE TABLE ec_IndexColumn("
                           "IndexId INTEGER NOT NULL REFERENCES ec_Index (Id) ON DELETE CASCADE,"
                           "ColumnId INTEGER NOT NULL REFERENCES ec_Column (Id) ON DELETE CASCADE,"
                           "Ordinal INTEGER NOT NULL,"
                           "PRIMARY KEY (IndexId, ColumnId))");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_IndexColumn_IndexId_Ordinal ON ec_IndexColumn(IndexId,Ordinal);"
                           "CREATE INDEX ix_ec_IndexColumn_ColumnId ON ec_IndexColumn(ColumnId)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_ForeignKey
    stat = ecdb.ExecuteSql("CREATE TABLE ec_ForeignKey("
                           "Id INTEGER PRIMARY KEY,"
                           "TableId INTEGER NOT NULL REFERENCES ec_Table (Id) ON DELETE CASCADE,"
                           "ReferencedTableId INTEGER NOT NULL REFERENCES ec_Table (Id) ON DELETE CASCADE,"
                           "Name TEXT COLLATE NOCASE,"
                           "OnDelete INTEGER,"
                           "OnUpdate INTEGER)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_ForeignKey_TableId ON ec_ForeignKey(TableId);"
                           "CREATE INDEX ix_ec_ForeignKey_ReferencedTableId ON ec_ForeignKey(ReferencedTableId)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE TABLE ec_ForeignKeyColumn("
                           "ForeignKeyId INTEGER NOT NULL REFERENCES ec_ForeignKey (Id) ON DELETE CASCADE,"
                           "ColumnId INTEGER NOT NULL REFERENCES ec_Column (Id) ON DELETE CASCADE,"
                           "ReferencedColumnId INTEGER NOT NULL REFERENCES ec_Column (Id) ON DELETE CASCADE,"
                           "Ordinal INTEGER NOT NULL)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_ForeignKeyColumn_ForeignKeyId ON ec_ForeignKeyColumn(ForeignKeyId);"
                           "CREATE INDEX ix_ec_ForeignKeyColumn_ColumnId ON ec_ForeignKeyColumn(ColumnId);"
                           "CREATE INDEX ix_ec_ForeignKeyColumn_ReferencedColumnId ON ec_ForeignKeyColumn(ReferencedColumnId)");
    if (BE_SQLITE_OK != stat)
        return stat;

    return BE_SQLITE_OK;
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
