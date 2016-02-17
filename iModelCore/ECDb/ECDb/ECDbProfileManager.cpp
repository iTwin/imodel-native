/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbProfileManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2013
//---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP const ECDbProfileManager::PROFILENAME = "ECDb";
//static
const PropertySpec ECDbProfileManager::PROFILEVERSION_PROPSPEC = PropertySpec("SchemaVersion", "ec_Db");
//static
const SchemaVersion ECDbProfileManager::MINIMUM_SUPPORTED_VERSION = SchemaVersion(2, 8, 0, 0);

//static
std::vector<std::unique_ptr<ECDbProfileUpgrader>> ECDbProfileManager::s_upgraderSequence;

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2012
//+---------------+---------------+---------------+---------------+---------------+-
//static
DbResult ECDbProfileManager::CreateECProfile(ECDbR ecdb)
    {
    LOG.debugv("Creating %s profile in %s...", PROFILENAME, ecdb.GetDbFileName());

    StopWatch timer(true);
    ecdb.SaveChanges();
    // Set up the id sequences as the upgrade steps might add entries to the ec tables and therefore
    // need the sequence.
    // Setting up the sequence just means to reset them to the current repo id
    auto stat = ecdb.GetECDbImplR().ResetSequences();
    if (stat != BE_SQLITE_OK)
        {
        LOG.errorv("Failed to create %s profile in file '%s'. Could not initialize id sequences.",
            PROFILENAME, ecdb.GetDbFileName());
        ecdb.AbandonChanges();
        return stat;
        }

    stat = CreateECProfileTables(ecdb);
    if (stat != BE_SQLITE_OK)
        {
        LOG.errorv("Failed to create %s profile in %s: %s", PROFILENAME, ecdb.GetDbFileName(), ecdb.GetLastError().c_str());
        ecdb.AbandonChanges();
        return stat;
        }

    stat = ECDbProfileECSchemaUpgrader::ImportProfileSchemas(ecdb);
    if (stat != BE_SQLITE_OK)
        {
        ecdb.AbandonChanges();
        return stat;
        }

    stat = AssignProfileVersion(ecdb);
    if (stat != BE_SQLITE_OK)
        {
        ecdb.AbandonChanges();
        LOG.errorv("Failed to create %s profile in file '%s'. Could not assign new profile version. %s",
            PROFILENAME, ecdb.GetDbFileName(), ecdb.GetLastError().c_str());
        return stat;
        }

    ecdb.SaveChanges();
    timer.Stop();
    
    if (LOG.isSeverityEnabled (NativeLogging::LOG_INFO))
        LOG.infov("Created %s profile (in %.4lf msecs) in '%s'.", PROFILENAME, timer.GetElapsedSeconds() * 1000.0, ecdb.GetDbFileName());

    return BE_SQLITE_OK;
    }

//=======================================================================================
//! Whenever a profile upgrade is performed a context is created which prepares SQLite
//! for the performing table and column alterations.
//! E.g. foreign key enforcement is disabled during upgrade to not invalidate foreign key
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
    void SetCommitAfterUpgrade() {m_rollbackOnDestruction = false;}

    DbResult GetBeginTransError() const {return m_beginTransError;}
    };

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2013
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ECDbProfileManager::UpgradeECProfile(ECDbR ecdb, Db::OpenParams const& openParams)
    {
    StopWatch timer(true);

    SchemaVersion actualProfileVersion(0, 0, 0, 0);
    auto stat = ReadProfileVersion(actualProfileVersion, ecdb, *ecdb.GetDefaultTransaction());
    if (stat != BE_SQLITE_OK)
        return stat;       //File is no ECDb file, i.e. doesn't have the ECDb profile

    bool profileNeedsUpgrade = false;
    stat = ECDb::CheckProfileVersion(profileNeedsUpgrade, GetExpectedProfileVersion(), actualProfileVersion, GetMinimumAutoUpgradableProfileVersion(), openParams.IsReadonly(), PROFILENAME);
    if (!profileNeedsUpgrade)
        return stat;

    LOG.infov("Version of file's %s profile is too old. Upgrading '%s' now...", PROFILENAME, ecdb.GetDbFileName());

    //if ECDb file is readonly, reopen it in read-write mode
    if (!openParams._ReopenForSchemaUpgrade(ecdb))
        {
        LOG.errorv("Upgrade of file's %s profile failed because file could not be re-opened in read-write mode.", PROFILENAME);
        return BE_SQLITE_ERROR_ProfileUpgradeFailedCannotOpenForWrite;
        }

    BeAssert(!ecdb.IsReadonly());

    //Creating the context performs some preparational steps in the SQLite database required for table modifications (e.g. foreign key
    //enforcement is disabled). When the context goes out of scope its destructor automatically performs the clean-up so that the ECDb file is
    //in the same state as before the upgrade.
    ProfileUpgradeContext context(ecdb, *ecdb.GetDefaultTransaction()); //also commits the transaction (if active) right now
    if (context.GetBeginTransError() == BE_SQLITE_BUSY)
        return BE_SQLITE_BUSY;

    //Call upgrader sequence and let upgraders incrementally upgrade the profile
    //to the latest state
    auto upgraderIterator = GetUpgraderSequenceFor(actualProfileVersion);
    BeAssert(upgraderIterator != GetUpgraderSequence().end() && "Upgrader sequence is not expected to be empty as the profile status is 'requires upgrade'");
    for (;upgraderIterator != GetUpgraderSequence().end(); ++upgraderIterator)
        {
        auto const& upgrader = *upgraderIterator;
        auto stat = upgrader->Upgrade(ecdb);
        if (stat != BE_SQLITE_OK)
            return BE_SQLITE_ERROR_ProfileUpgradeFailed; //context dtor ensures that changes are rolled back
        }

    stat = ECDbProfileECSchemaUpgrader::ImportProfileSchemas(ecdb);
    if (stat != BE_SQLITE_OK)
        return BE_SQLITE_ERROR_ProfileUpgradeFailed; //context dtor ensures that changes are rolled back

    //after upgrade procedure set new profile version in ECDb file
    stat = AssignProfileVersion(ecdb);

    timer.Stop();
    if (stat != BE_SQLITE_OK)
        {
        LOG.errorv("Failed to upgrade %s profile in file '%s'. Could not assign new profile version. %s",
            PROFILENAME, ecdb.GetDbFileName(), ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed; //context dtor ensures that changes are rolled back
        }
    
    context.SetCommitAfterUpgrade(); //change context dtor behavior to commit changes
    if (LOG.isSeverityEnabled(NativeLogging::LOG_INFO))
        {
        const auto expectedProfileVersion = GetExpectedProfileVersion();
        LOG.infov("Upgraded %s profile from version %d.%d.%d.%d to version %d.%d.%d.%d (in %.4lf seconds) in file '%s'.",
            PROFILENAME,
            actualProfileVersion.GetMajor(), actualProfileVersion.GetMinor(), actualProfileVersion.GetSub1(), actualProfileVersion.GetSub2(),
            expectedProfileVersion.GetMajor(), expectedProfileVersion.GetMinor(), expectedProfileVersion.GetSub1(), expectedProfileVersion.GetSub2(),
            timer.GetElapsedSeconds(), ecdb.GetDbFileName());
        }

    return BE_SQLITE_OK; //context dtor ensures that changes are committed
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::AssignProfileVersion(ECDbR ecdb)
    {
    //Save the profile version as string (JSON format)
    const auto profileVersionStr = GetExpectedProfileVersion().ToJson();
    return ecdb.SavePropertyString(PROFILEVERSION_PROPSPEC, profileVersionStr);
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
    DbResult stat = BE_SQLITE_OK;
    if (BE_SQLITE_ROW == ecdb.QueryProperty(currentVersionString, PROFILEVERSION_PROPSPEC))
        {
        profileVersion.FromJson(currentVersionString.c_str());
        }
    // version entry does not exist. This either means it is ECDb profile 1.0 (because we did not store
    // a version entry for profile 1.0 or it isn't an ECDb file at all. In order to tell these we need
    // to check for a typical table of the ECDb profile:
    else if (ecdb.TableExists("ec_Schema"))
        {
        profileVersion = SchemaVersion(1, 0, 0, 0);
        }
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
SchemaVersion ECDbProfileManager::GetExpectedProfileVersion()
    {
    //if there are no upgraders yet, the current version is the minimally supported version
    if (GetUpgraderSequence().empty())
        return GetMinimumAutoUpgradableProfileVersion();

    //Version of latest upgrader is the version currently required by the API
    return GetLatestUpgrader().GetTargetVersion();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
SchemaVersion ECDbProfileManager::GetMinimumAutoUpgradableProfileVersion()
    {
    //Auto-upgradable back to this version
    return MINIMUM_SUPPORTED_VERSION;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECDbProfileUpgrader const& ECDbProfileManager::GetLatestUpgrader()
    {
    return *GetUpgraderSequence().back();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECDbProfileManager::ECDbProfileUpgraderSequence::const_iterator ECDbProfileManager::GetUpgraderSequenceFor(SchemaVersion const& currentProfileVersion)
    {
    auto end = GetUpgraderSequence().end();
    for (auto it = GetUpgraderSequence().begin(); it != end; ++it)
        {
        auto const& upgrader = *it;
        if (currentProfileVersion < upgrader->GetTargetVersion())
            return it;
        }

    return end;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECDbProfileManager::ECDbProfileUpgraderSequence const& ECDbProfileManager::GetUpgraderSequence()
    {
    if (s_upgraderSequence.empty())
        {
        //no upgraders on top of the minimally supported version for this version of ECDb yet
        }

    return s_upgraderSequence;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::CreateECProfileTables(ECDbR ecdb)
    {
    //ec_Schema
    DbResult stat = ecdb.ExecuteSql("CREATE TABLE ec_Schema("
                                    "Id INTEGER PRIMARY KEY,"
                                    "Name TEXT NOT NULL,"
                                    "DisplayLabel TEXT,"
                                    "Description TEXT,"
                                    "NamespacePrefix TEXT,"
                                    "VersionMajor INTEGER NOT NULL,"
                                    "VersionMiddle INTEGER NOT NULL,"
                                    "VersionMinor INTEGER NOT NULL)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE UNIQUE INDEX uix_ec_Schema_Name ON ec_Schema(Name COLLATE NOCASE)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE UNIQUE INDEX uix_ec_Schema_NamespacePrefix ON ec_Schema(NamespacePrefix COLLATE NOCASE)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_SchemaReference
    stat = ecdb.ExecuteSql("CREATE TABLE ec_SchemaReference("
                           "SchemaId INTEGER REFERENCES ec_Schema(Id) ON DELETE CASCADE,"
                           "ReferencedSchemaId INTEGER REFERENCES ec_Schema(Id) ON DELETE CASCADE)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_SchemaReference_SchemaId ON ec_SchemaReference(SchemaId)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Class
    stat = ecdb.ExecuteSql("CREATE TABLE ec_Class("
                           "Id INTEGER PRIMARY KEY,"
                           "SchemaId INTEGER NOT NULL REFERENCES ec_Schema(Id) ON DELETE CASCADE,"
                           "Name TEXT NOT NULL,"
                           "DisplayLabel TEXT,"
                           "Description TEXT,"
                           "Type INTEGER NOT NULL,"
                           "Modifier INTEGER NOT NULL,"
                           "RelationshipStrength INTEGER,"
                           "RelationshipStrengthDirection INTEGER)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_Class_Name ON ec_Class(Name)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_Class_SchemaId ON ec_Class(SchemaId)");
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
    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_BaseClass_BaseClassId ON ec_BaseClass(BaseClassId)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Enumeration
    stat = ecdb.ExecuteSql("CREATE TABLE ec_Enumeration("
                           "Id INTEGER PRIMARY KEY,"
                           "SchemaId INTEGER NOT NULL REFERENCES ec_Schema(Id) ON DELETE CASCADE,"
                           "Name TEXT NOT NULL,"
                           "DisplayLabel TEXT,"
                           "Description TEXT,"
                           "UnderlyingPrimitiveType INTEGER NOT NULL,"
                           "IsStrict BOOL NOT NULL CHECK(IsStrict IN (0,1)),"
                           "EnumValues JSON NOT NULL)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE UNIQUE INDEX uix_ec_Enumeration_SchemaId_Name ON ec_Enumeration(SchemaId,Name)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Property
    stat = ecdb.ExecuteSql("CREATE TABLE ec_Property("
                           "Id INTEGER PRIMARY KEY,"
                           "ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,"
                           "Name TEXT NOT NULL,"
                           "DisplayLabel TEXT,"
                           "Description TEXT,"
                           "IsReadonly BOOL NOT NULL CHECK (IsReadonly IN (0, 1)),"
                           "Kind INTEGER NOT NULL,"
                           "Ordinal INTEGER,"
                           "PrimitiveType INTEGER,"
                           "NonPrimitiveType INTEGER REFERENCES ec_Class(Id) ON DELETE CASCADE,"
                           "ExtendedType TEXT,"
                           "Enumeration INTEGER REFERENCES ec_Enumeration(Id) ON DELETE CASCADE,"
                           "ArrayMinOccurs INTEGER,"
                           "ArrayMaxOccurs INTEGER,"
                           "NavigationPropertyDirection INTEGER)");

    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_Property_Name ON ec_Property(Name)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_Property_ClassId ON ec_Property(ClassId)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_PropertyPath
    stat = ecdb.ExecuteSql("CREATE Table ec_PropertyPath("
                           "Id INTEGER PRIMARY KEY,"
                           "RootPropertyId INTEGER NOT NULL REFERENCES ec_Property(Id) ON DELETE CASCADE,"
                           "AccessString TEXT NOT NULL)");
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
                           "IsPolymorphic BOOL NOT NULL CHECK (IsPolymorphic IN (0, 1)),"
                           "PRIMARY KEY (RelationshipClassId, RelationshipEnd))");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_RelationshipConstraintClass
    stat = ecdb.ExecuteSql("CREATE TABLE ec_RelationshipConstraintClass("
                           "RelationshipClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,"
                           "RelationshipEnd INTEGER NOT NULL,"
                           "ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,"
                           "KeyProperties JSON,"
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


    //ec_ClassMap
    stat = ecdb.ExecuteSql("CREATE Table ec_ClassMap("
                           "Id INTEGER PRIMARY KEY,"
                           "ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,"
                           "ParentId INTEGER REFERENCES ec_ClassMap(Id) ON DELETE CASCADE,"
                           //resolved map strategy:
                           "MapStrategy INTEGER NOT NULL,"
                           "MapStrategyOptions INTEGER,"
                           "MapStrategyAppliesToSubclasses BOOL NOT NULL CHECK (MapStrategyAppliesToSubclasses IN (0, 1)))");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE UNIQUE INDEX uix_ec_ClassMap_ClassId_ParentId ON ec_ClassMap(ClassId, ParentId) WHERE ParentId IS NOT NULL");
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

    //ec_Table
    stat = ecdb.ExecuteSql("CREATE TABLE ec_Table("
                           "Id INTEGER PRIMARY KEY,"
                           "BaseTableId INTEGER REFERENCES ec_Table(Id) ON DELETE CASCADE,"
                           "Name TEXT NOT NULL COLLATE NOCASE,"
                           "Type INTEGER NOT NULL,"
                           "IsVirtual BOOL NOT NULL CHECK (IsVirtual IN (0, 1)))");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Column
    stat = ecdb.ExecuteSql("CREATE TABLE ec_Column("
                           "Id INTEGER PRIMARY KEY,"
                           "TableId INTEGER NOT NULL REFERENCES ec_Table(Id) ON DELETE CASCADE,"
                           "Name TEXT NOT NULL COLLATE NOCASE,"
                           "Type INTEGER NOT NULL,"
                           "IsVirtual BOOL NOT NULL CHECK (IsVirtual IN (0, 1)),"
                           "Ordinal INTEGER NOT NULL,"
                           "NotNullConstraint BOOLEAN,"
                           "UniqueConstraint BOOLEAN,"
                           "CheckConstraint TEXT,"
                           "DefaultConstraint TEXT,"
                           "CollationConstraint INTEGER,"
                           "OrdinalInPrimaryKey INTEGER,"
                           "ColumnKind INTEGER)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE INDEX ix_ec_Column_TableId ON ec_Column(TableId)");
    if (BE_SQLITE_OK != stat)
        return stat;

    //ec_Index
    stat = ecdb.ExecuteSql("CREATE TABLE ec_Index("
                           "Id INTEGER PRIMARY KEY,"
                           "Name TEXT NOT NULL COLLATE NOCASE,"
                           "TableId INTEGER NOT NULL REFERENCES ec_Table(Id) ON DELETE CASCADE,"
                           "IsUnique BOOL NOT NULL CHECK (IsUnique IN (0,1)),"
                           "PrimaryWhereClause TEXT, "
                           "IsAutoGenerated BOOL NOT NULL CHECK (IsAutoGenerated IN (0,1)),"
                           "ClassId INTEGER REFERENCES ec_Class(Id) ON DELETE CASCADE,"
                           "AppliesToSubclassesIfPartial BOOL NOT NULL CHECK (AppliesToSubclassesIfPartial IN (0,1)))");
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = ecdb.ExecuteSql("CREATE UNIQUE INDEX uix_ec_Index_TableId_Name ON ec_Index(TableId,Name)");
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

    stat = ecdb.ExecuteSql("CREATE TABLE ec_ForeignKeyColumn("
        "ForeignKeyId INTEGER NOT NULL REFERENCES ec_ForeignKey (Id) ON DELETE CASCADE,"
        "ColumnId INTEGER NOT NULL  REFERENCES ec_Column (Id) ON DELETE CASCADE,"
        "ReferencedColumnId INTEGER NOT NULL  REFERENCES ec_Column (Id) ON DELETE CASCADE,"
        "Ordinal INTEGER NOT NULL)");
    if (BE_SQLITE_OK != stat)
        return stat;

#ifdef ENABLE_TRIGGER_DEBUGGING
    stat = ecdb.ExecuteSql (
        "CREATE TABLE ec_TriggerLog("
        "EventId INTEGER PRIMARY KEY AUTOINCREMENT,"
        "EventTime TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "TriggerId TEXT,"
        "AffectedECInstanceId INTEGER,"
        "AffectedECClassId INTEGER,"
        "Scope TEXT,"
        "Comment TEXT)");

    if (stat != BE_SQLITE_OK)
        return stat;
#endif
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
    auto stat = m_ecdb.TryExecuteSql("PRAGMA foreign_keys = OFF;");
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
    auto stat = m_ecdb.TryExecuteSql("PRAGMA foreign_keys = ON;");
    if (stat != BE_SQLITE_OK)
        {
        LOG.error("ECDb profile upgrade: Re-enabling foreign key enforcement in SQLite failed.");
        BeAssert(false);
        }

    if (m_isDefaultTransOpen)
        m_defaultTransaction.Begin();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
