/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbProfileManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
const SchemaVersion ECDbProfileManager::MINIMUM_SUPPORTED_VERSION = SchemaVersion(2, 5, 0, 0);

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
        ecdb.AbandonChanges();
        LOG.errorv("Failed to create %s profile in file '%s'. Could not initialize id sequences.",
            PROFILENAME, ecdb.GetDbFileName());
        return stat;
        }

    stat = ProfileCreator::Create(ecdb);
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

    stat = ECDbProfileECSchemaUpgrader::ImportProfileSchemas(ecdb, true);
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


//*************************************** ECDbProfileManager::ProfileCreator *********************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::Create(ECDbR ecdb)
    {
    auto stat = CreateECProfileTables(ecdb);
    if (stat != BE_SQLITE_OK)
        {
        LOG.errorv("Failed to create %s profile in %s: %s", PROFILENAME, ecdb.GetDbFileName(), ecdb.GetLastError().c_str());
        return stat;
        }

    return ECDbProfileECSchemaUpgrader::ImportProfileSchemas(ecdb);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::CreateECProfileTables
(
Db& db
)
    {
    StopWatch timer(true);
    auto stat = CreateTableECSchema(db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableECClass(db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableBaseClass(db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableECProperty(db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableECRelationshipConstraint(db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableECRelationshipConstraintClass(db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableECRelationshipConstraintClassKeyProperty(db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableCustomAttribute(db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableSchemaReferences(db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableECClassMap(db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTablePropertyPath(db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableTable(db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableColumn(db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableIndex(db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableIndexColumn(db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableForeignKey(db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableForeignKeyColumn(db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableECPropertyMap(db);
    if (stat != BE_SQLITE_OK)
        return stat;

#ifdef ENABLE_TRIGGER_DEBUGGING
    stat = db.ExecuteSql (
        "CREATE TABLE ec_TriggerLog "
        "("
        "EventId INTEGER PRIMARY KEY AUTOINCREMENT , "
        "EventTime TIMESTAMP DEFAULT CURRENT_TIMESTAMP ,"
        "TriggerId TEXT, "
        "AffectedECInstanceId INTEGER, "
        "AffectedECClassId INTEGER, "
        "Scope TEXT,"
        "Comment TEXT"
        ");");

    if (stat != BE_SQLITE_OK)
        return stat;
#endif

    timer.Stop();
    LOG.debugv("Created ECDb profile tables in ECDb file in %.4f msecs.", timer.GetElapsedSeconds() * 1000.0);
    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::CreateTableECSchema(Db& db)
    {
    auto stat = db.ExecuteSql(
        "CREATE TABLE ec_Schema "
        "("
        "Id INTEGER PRIMARY KEY, "
        "Name TEXT NOT NULL, "
        "DisplayLabel TEXT, "
        "Description TEXT, "
        "NamespacePrefix TEXT, "
        "VersionMajor INTEGER, "
        "VersionMinor INTEGER"
        ");");

    stat = db.ExecuteSql("CREATE UNIQUE INDEX uix_ec_Schema_Name ON ec_Schema(Name COLLATE NOCASE);");
    if (stat != BE_SQLITE_OK)
        return stat;

    return db.ExecuteSql("CREATE UNIQUE INDEX uix_ec_Schema_NamespacePrefix ON ec_Schema(NamespacePrefix COLLATE NOCASE);");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::CreateTableECClass 
(
Db& db
)
    {
    DbResult stat = db.ExecuteSql(
        "CREATE TABLE ec_Class "
        "("
        "Id INTEGER PRIMARY KEY, "
        "SchemaId INTEGER NOT NULL REFERENCES ec_Schema(Id) ON DELETE CASCADE, "
        "Name TEXT NOT NULL, "
        "DisplayLabel TEXT, "
        "Description TEXT, "
        "IsDomainClass BOOL NOT NULL CHECK (IsDomainClass IN (0, 1)), "
        "IsStruct BOOL NOT NULL CHECK (IsStruct IN (0, 1)), "
        "IsCustomAttribute BOOL NOT NULL CHECK (IsCustomAttribute IN (0, 1)), "
        "RelationStrength INTEGER, "
        "RelationStrengthDirection INTEGER, "
        "IsRelationship BOOL NOT NULL CHECK (IsRelationship IN (0, 1))"
        ");");

    if (stat != BE_SQLITE_OK)
        return stat;

    stat = db.ExecuteSql("CREATE INDEX ix_ec_Class_Name ON ec_Class(Name);");
    if (stat != BE_SQLITE_OK)
        return stat;

    return db.ExecuteSql("CREATE INDEX ix_ec_Class_SchemaId ON ec_Class(SchemaId);");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::CreateTableECClassMap 
(
Db& db
)
    {
    auto stat = db.ExecuteSql(
        "CREATE Table ec_ClassMap"
        "("
        "Id INTEGER PRIMARY KEY,"
        "ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,"
        "ParentId INTEGER REFERENCES ec_ClassMap(Id) ON DELETE CASCADE,"
        //resolved map strategy:
        "MapStrategy INTEGER NOT NULL,"
        "MapStrategyOptions INTEGER,"
        "MapStrategyAppliesToSubclasses BOOL NOT NULL CHECK (MapStrategyAppliesToSubclasses IN (0, 1))"
        ");");

    if (stat != BE_SQLITE_OK)
        return stat;

    return db.ExecuteSql("CREATE UNIQUE INDEX uix_ec_ClassMap_ClassId_ParentId ON ec_ClassMap(ClassId, ParentId) WHERE ParentId IS NOT NULL;");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::CreateTableBaseClass 
(
Db& db
)
    {
    auto stat = db.ExecuteSql(
        "CREATE TABLE ec_BaseClass "
        "("
        "ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE, "
        "BaseClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE, "
        "Ordinal INTEGER NOT NULL, /*Location of baseclass in BaseClasses array*/ "
        "PRIMARY KEY (ClassId, BaseClassId)"
        ");");

    if (stat != BE_SQLITE_OK)
        return stat;

    //index needed for fast look-ups of derived classes for a given ECClass
    return db.ExecuteSql("CREATE INDEX ix_ec_BaseClass_BaseClassId ON ec_BaseClass(BaseClassId);");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::CreateTablePropertyPath
(
Db& db
)
    {
    auto stat = db.ExecuteSql(
        "CREATE Table ec_PropertyPath"
        "("
        "Id INTEGER PRIMARY KEY,"
        "RootPropertyId INTEGER NOT NULL REFERENCES ec_Property(Id) ON DELETE CASCADE,"
        "AccessString TEXT NOT NULL"
        ");");


    if (stat != BE_SQLITE_OK)
        return stat;

    return db.ExecuteSql("CREATE UNIQUE INDEX uix_ec_PropertyPath_RootPropertyId_AccessString ON ec_PropertyPath(RootPropertyId, AccessString);");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::CreateTableECProperty 
(
Db& db
)
    {
    auto stat = db.ExecuteSql(
        "CREATE TABLE ec_Property "
        "("
        "Id INTEGER PRIMARY KEY,"
        "ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE, "
        "Name TEXT NOT NULL, "
        "DisplayLabel TEXT, "
        "Description TEXT, "
        "Ordinal INTEGER, "
        "IsArray BOOL NOT NULL CHECK (IsArray IN (0, 1)), "
        "PrimitiveType INTEGER, "
        "StructType INTEGER REFERENCES ec_Class(Id) ON DELETE CASCADE, "
        "IsReadonly BOOL NOT NULL CHECK (IsReadonly IN (0, 1)), "
        "MinOccurs INTEGER,"
        "MaxOccurs INTEGER"
        ");");

    if (stat != BE_SQLITE_OK)
        return stat;

    stat = db.ExecuteSql("CREATE INDEX ix_ec_Property_Name ON ec_Property(Name);");
    if (stat != BE_SQLITE_OK)
        return stat;

    return db.ExecuteSql("CREATE INDEX ix_ec_Property_ClassId ON ec_Property(ClassId);");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::CreateTableECPropertyMap 
(
Db& db
)
    {
    return db.ExecuteSql(
        "CREATE Table ec_PropertyMap"
        "("
        "ClassMapId INTEGER NOT NULL REFERENCES ec_ClassMap(Id) ON DELETE CASCADE,"
        "PropertyPathId INTEGER NOT NULL REFERENCES ec_PropertyPath(Id) ON DELETE CASCADE,"
        "ColumnId INTEGER NOT NULL REFERENCES ec_Column(Id) ON DELETE CASCADE,"
        "PRIMARY KEY (ClassMapId, PropertyPathId, ColumnId)"
        ");");

    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::CreateTableECRelationshipConstraint 
(
Db& db
)
    {
    return db.ExecuteSql(
        "CREATE TABLE ec_RelationshipConstraint "
        "("
        "RelationshipClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE, "
        "RelationshipEnd INTEGER NOT NULL, "
        "CardinalityLowerLimit INTEGER, "
        "CardinalityUpperLimit INTEGER, "
        "RoleLabel TEXT, " 
        "IsPolymorphic BOOL NOT NULL CHECK (IsPolymorphic IN (0, 1)), "
        "PRIMARY KEY (RelationshipClassId, RelationshipEnd)"
        ");");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::CreateTableECRelationshipConstraintClass 
(
Db& db
)
    {
    return db.ExecuteSql(
        "CREATE TABLE ec_RelationshipConstraintClass "
        "("
        "RelationshipClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE, "
        "RelationshipEnd INTEGER NOT NULL, "
        "ClassId INTEGER NOT NULL, "
        "PRIMARY KEY (RelationshipClassId, RelationshipEnd, ClassId), "
        "FOREIGN KEY (RelationshipClassId, RelationshipEnd) "
        "REFERENCES ec_RelationshipConstraint(RelationshipClassId, RelationshipEnd) ON DELETE CASCADE"
        ");");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::CreateTableECRelationshipConstraintClassKeyProperty 
(
Db& db
)
    {
    return db.ExecuteSql(
        "CREATE TABLE ec_RelationshipConstraintClassKeyProperty "
        "("
        "RelationshipClassId INTEGER NOT NULL, "
        "RelationshipEnd INTEGER NOT NULL, "
        "ConstraintClassId INTEGER NOT NULL, "
        "KeyPropertyName TEXT NOT NULL, "
        "PRIMARY KEY (RelationshipClassId,RelationshipEnd,ConstraintClassId,KeyPropertyName), "
        "FOREIGN KEY (RelationshipClassId, RelationshipEnd, ConstraintClassId) REFERENCES ec_RelationshipConstraintClass (RelationshipClassId, RelationshipEnd, ClassId) ON DELETE CASCADE"
        ");");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::CreateTableCustomAttribute 
(
Db& db
)
    {
    return db.ExecuteSql(
        "CREATE TABLE ec_CustomAttribute ("
        "ContainerId INTEGER NOT NULL, "
        "ContainerType INTEGER NOT NULL, "
        "Ordinal INTEGER NOT NULL, "
        "ClassId INTEGER NOT NULL REFERENCES ec_Class(Id), "
        "Instance TEXT, "
        "PRIMARY KEY (ContainerId, ContainerType, ClassId)"
        ");");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::CreateTableSchemaReferences 
(
Db& db
)
    {
    DbResult stat = db.ExecuteSql(
        "CREATE TABLE ec_SchemaReference ("
        "SchemaId INTEGER REFERENCES ec_Schema(Id) ON DELETE CASCADE, "
        "ReferencedSchemaId INTEGER REFERENCES ec_Schema(Id) ON DELETE CASCADE"
        ");");

    if (BE_SQLITE_OK != stat)
        return stat;

    return db.ExecuteSql("CREATE INDEX ix_ec_SchemaReference_SchemaId ON ec_SchemaReference(SchemaId);");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::CreateTableTable
(
Db& db
)
    {
    return db.ExecuteSql(
        "CREATE TABLE ec_Table"
        "("
        "Id INTEGER PRIMARY KEY,"
        "Name TEXT NOT NULL COLLATE NOCASE,"
        "IsOwnedByECDb BOOL NOT NULL CHECK (IsOwnedByECDb IN (0, 1)),"
        "IsVirtual BOOL NOT NULL CHECK (IsVirtual IN (0, 1))"
        ");");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::CreateTableColumn
(
Db& db
)
    {
    DbResult stat = db.ExecuteSql(
        "CREATE TABLE ec_Column"
        "("
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
        "ColumnKind INTEGER"
        ");");

    if (stat != BE_SQLITE_OK)
        return stat;

    return db.ExecuteSql("CREATE INDEX ix_ec_Column_TableId ON ec_Column(TableId);");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::CreateTableIndex
(
Db& db
)
    {
    auto stat = db.ExecuteSql(
        "CREATE TABLE ec_Index"
        "("
        "Id INTEGER PRIMARY KEY,"
        "Name TEXT NOT NULL COLLATE NOCASE,"
        "TableId INTEGER NOT NULL REFERENCES ec_Table(Id) ON DELETE CASCADE,"
        "IsUnique BOOL NOT NULL CHECK (IsUnique IN (0,1)),"
        "PrimaryWhereClause TEXT, "
        "IsAutoGenerated BOOL NOT NULL CHECK (IsAutoGenerated IN (0,1)),"
        "ClassId INTEGER REFERENCES ec_Class(Id) ON DELETE CASCADE,"
        "AppliesToSubclassesIfPartial BOOL NOT NULL CHECK (AppliesToSubclassesIfPartial IN (0,1))"
        ");");

    if (stat != BE_SQLITE_OK)
        return stat;

    return db.ExecuteSql("CREATE UNIQUE INDEX uix_ec_Index_TableId_Name ON ec_Index(TableId, Name);");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::CreateTableIndexColumn
(
Db& db
)
    {
    return db.ExecuteSql(
        "CREATE TABLE ec_IndexColumn"
        "("
        "IndexId INTEGER NOT NULL REFERENCES ec_Index (Id) ON DELETE CASCADE,"
        "ColumnId INTEGER NOT NULL REFERENCES ec_Column (Id) ON DELETE CASCADE,"
        "Ordinal INTEGER NOT NULL,"
        "PRIMARY KEY (IndexId, ColumnId)"
        ");");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::CreateTableForeignKey
(
Db& db
)
    {
    return db.ExecuteSql(
        "CREATE TABLE ec_ForeignKey"
        "("
        "Id INTEGER PRIMARY KEY,"
        "TableId INTEGER NOT NULL REFERENCES ec_Table (Id) ON DELETE CASCADE,"
        "ReferencedTableId INTEGER NOT NULL REFERENCES ec_Table (Id) ON DELETE CASCADE,"
        "Name TEXT COLLATE NOCASE,"
        "OnDelete INTEGER,"
        "OnUpdate INTEGER"
        ");");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::CreateTableForeignKeyColumn(Db& db)
    {
    return db.ExecuteSql(
        "CREATE TABLE ec_ForeignKeyColumn"
        "("
        "ForeignKeyId INTEGER NOT NULL REFERENCES ec_ForeignKey (Id) ON DELETE CASCADE,"
        "ColumnId INTEGER NOT NULL  REFERENCES ec_Column (Id) ON DELETE CASCADE,"
        "ReferencedColumnId INTEGER NOT NULL  REFERENCES ec_Column (Id) ON DELETE CASCADE,"
        "Ordinal INTEGER NOT NULL"
        ");");
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
