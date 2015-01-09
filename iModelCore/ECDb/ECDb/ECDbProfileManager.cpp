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
const PropertySpec ECDbProfileManager::PROFILEVERSION_PROPSPEC = PropertySpec ("SchemaVersion", "ec_Db");
//static
std::vector<std::unique_ptr<ECDbProfileUpgrader>> ECDbProfileManager::s_upgraderSequence;

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2012
//+---------------+---------------+---------------+---------------+---------------+-
//static
DbResult ECDbProfileManager::CreateECProfile 
(
ECDbR ecdb
)
    {
    LOG.debugv ("Creating %s profile in %s...", PROFILENAME, ecdb.GetDbFileName ());

    StopWatch timer ("", true);
    ecdb.SaveChanges ();
    // Set up the id sequences as the upgrade steps might add entries to the ec tables and therefore
    // need the sequence.
    // Setting up the sequence just means to reset them to the current repo id
    auto stat = ecdb.GetImplR ().ResetSequences (ecdb);
    if (stat != BE_SQLITE_OK)
        {
        ecdb.AbandonChanges ();
        LOG.errorv ("Failed to create %s profile in file '%s'. Could not initialize id sequences.",
            PROFILENAME, ecdb.GetDbFileName ());
        return stat;
        }

    stat = ProfileCreator::Create (ecdb);
    if (stat != BE_SQLITE_OK)
        {
        ecdb.AbandonChanges ();
        return stat;
        }

    stat = AssignProfileVersion (ecdb);
    if (stat != BE_SQLITE_OK)
        {
        ecdb.AbandonChanges ();
        LOG.errorv ("Failed to create %s profile in file '%s'. Could not assign new profile version. %s",
            PROFILENAME, ecdb.GetDbFileName (), ecdb.GetLastError ());
        return stat;
        }

    ecdb.SaveChanges ();
    timer.Stop();
    LOG.infov ("Created %s profile (in %.4lf seconds) in '%s'.", PROFILENAME, timer.GetElapsedSeconds(), ecdb.GetDbFileName());

    return BE_SQLITE_OK;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2013
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ECDbProfileManager::UpgradeECProfile (ECDbR ecdb, Db::OpenParams const& openParams, Savepoint& defaultTransaction)
    {
    StopWatch timer ("", true);

    SchemaVersion actualProfileVersion (0, 0, 0, 0);
    auto stat = ReadProfileVersion (actualProfileVersion, ecdb, defaultTransaction);
    if (stat != BE_SQLITE_OK)
        //File is no ECDb file, i.e. doesn't have the ECDb profile
        return stat;

    bool profileNeedsUpgrade = false;
    stat = ECDb::CheckProfileVersion (profileNeedsUpgrade, GetExpectedProfileVersion(), actualProfileVersion, GetMinimumAutoUpgradableProfileVersion(), openParams.IsReadonly(), PROFILENAME);
    if (!profileNeedsUpgrade)
        return stat;

    LOG.infov ("Version of file's %s profile is too old. Upgrading '%s' now...", PROFILENAME, ecdb.GetDbFileName ());

    //if ECDb file is readonly, reopen it in read-write mode
    if (!openParams._ReopenForSchemaUpgrade (ecdb))
        {
        LOG.errorv ("Upgrade of file's %s profile failed because file could not be re-opened in read-write mode.", PROFILENAME);
        return BE_SQLITE_ERROR_ProfileUpgradeFailedCannotOpenForWrite;
        }

    BeAssert (!ecdb.IsReadonly ());

    
    //Creating the context performs some preparational steps in the SQLite database required for table modifications (e.g. foreign key
    //enforcement is disabled). When the context goes out of scope its destructor automatically performs the clean-up so that the ECDb file is
    //in the same state as before the upgrade.
    ProfileUpgradeContext context (ecdb, defaultTransaction); //also commits the transaction (if active) right now
    if (context.GetBeginTransError () == BE_SQLITE_BUSY)
        return BE_SQLITE_BUSY;

    //Call upgrader sequence and let upgraders incrementally upgrade the profile
    //to the latest state
    auto upgraderIterator = GetUpgraderSequenceFor (actualProfileVersion);
    BeAssert (upgraderIterator != GetUpgraderSequence ().end () && "Upgrader sequence is not expected to be empty as the profile status is 'requires upgrade'");
    for (;upgraderIterator != GetUpgraderSequence ().end (); ++upgraderIterator)
        {
        auto const& upgrader = *upgraderIterator;
        auto stat = upgrader->Upgrade (ecdb);
        if (stat != BE_SQLITE_OK)
            return BE_SQLITE_ERROR_ProfileUpgradeFailed; //context dtor ensures that changes are rolled back
        }

    stat = ECDbProfileECSchemaUpgrader::ImportProfileSchemas (ecdb, true);
    if (stat != BE_SQLITE_OK)
        return BE_SQLITE_ERROR_ProfileUpgradeFailed; //context dtor ensures that changes are rolled back

    //after upgrade procedure set new profile version in ECDb file
    stat = AssignProfileVersion (ecdb);

    timer.Stop();
    if (stat != BE_SQLITE_OK)
        {
        LOG.errorv ("Failed to upgrade %s profile in file '%s'. Could not assign new profile version. %s",
            PROFILENAME, ecdb.GetDbFileName (), ecdb.GetLastError ());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed; //context dtor ensures that changes are rolled back
        }
    
    context.SetCommitAfterUpgrade (); //change context dtor behavior to commit changes
    if (LOG.isSeverityEnabled (NativeLogging::LOG_INFO))
        {
        const auto expectedProfileVersion = GetExpectedProfileVersion ();
        LOG.infov ("Upgraded %s profile from version %d.%d.%d.%d to version %d.%d.%d.%d (in %.4lf seconds) in file '%s'.",
            PROFILENAME,
            actualProfileVersion.GetMajor (), actualProfileVersion.GetMinor (), actualProfileVersion.GetSub1 (), actualProfileVersion.GetSub2 (),
            expectedProfileVersion.GetMajor (), expectedProfileVersion.GetMinor (), expectedProfileVersion.GetSub1 (), expectedProfileVersion.GetSub2 (),
            timer.GetElapsedSeconds(), ecdb.GetDbFileName());
        }

    return BE_SQLITE_OK; //context dtor ensures that changes are committed
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::AssignProfileVersion (ECDbR ecdb)
    {
    //Save the profile version as string (JSON format)
    const auto profileVersionStr = GetExpectedProfileVersion ().ToJson ();
    return ecdb.SavePropertyString (PROFILEVERSION_PROPSPEC, profileVersionStr);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2013
//---------------+---------------+---------------+---------------+---------------+------
//static
 DbResult ECDbProfileManager::ReadProfileVersion (SchemaVersion& profileVersion, ECDbCR ecdb, Savepoint& defaultTransaction)
    {
    //we always need a transaction to execute SQLite statements. If ECDb was opened in no-default-trans mode, we need to
    //begin a transaction ourselves (just use BeSQLite's default transaction which is always there even in no-default-trans mode,
    //except that in that case, it is not active).
    const bool isDefaultTransactionActive = defaultTransaction.IsActive ();
    if (!isDefaultTransactionActive)
        defaultTransaction.Begin ();

    Utf8String currentVersionString;
    DbResult stat = BE_SQLITE_OK;
    if (BE_SQLITE_ROW == ecdb.QueryProperty (currentVersionString, PROFILEVERSION_PROPSPEC))
        {
        profileVersion.FromJson (currentVersionString.c_str ());
        }
    // version entry does not exist. This either means it is ECDb profile 1.0 (because we did not store
    // a version entry for profile 1.0 or it isn't an ECDb file at all. In order to tell these we need
    // to check for a typical table of the ECDb profile:
    else if (ecdb.TableExists ("ec_Schema"))
        {
        profileVersion = SchemaVersion (1, 0, 0, 0);
        }
    else
        //File is no ECDb file
        stat = BE_SQLITE_ERROR_InvalidProfileVersion;

    //make sure to end default transaction again, if it wasn't active before this call
    if (!isDefaultTransactionActive)
        defaultTransaction.Commit ();

    return stat;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
SchemaVersion ECDbProfileManager::GetExpectedProfileVersion ()
    {
    //Version of latest upgrader is the version currently required by the API
    return GetLatestUpgrader ().GetTargetVersion ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
SchemaVersion ECDbProfileManager::GetMinimumAutoUpgradableProfileVersion()
    {
    //Auto-upgradable back to version 1.0.0.0
    return SchemaVersion (1, 0, 0, 0);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECDbProfileUpgrader const& ECDbProfileManager::GetLatestUpgrader ()
    {
    return *GetUpgraderSequence ().back ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECDbProfileManager::ECDbProfileUpgraderSequence::const_iterator ECDbProfileManager::GetUpgraderSequenceFor (SchemaVersion const& currentProfileVersion)
    {
    auto end = GetUpgraderSequence ().end ();
    for (auto it = GetUpgraderSequence ().begin (); it != end; ++it)
        {
        auto const& upgrader = *it;
        if (currentProfileVersion < upgrader->GetTargetVersion ())
            {
            return it;
            }
        }

    return end;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECDbProfileManager::ECDbProfileUpgraderSequence const& ECDbProfileManager::GetUpgraderSequence()
    {
    if (s_upgraderSequence.empty ())
        {
        s_upgraderSequence.push_back (std::unique_ptr<ECDbProfileUpgrader> (new ECDbProfileUpgrader_1001 ()));
        s_upgraderSequence.push_back (std::unique_ptr<ECDbProfileUpgrader> (new ECDbProfileUpgrader_1002 ()));
        s_upgraderSequence.push_back (std::unique_ptr<ECDbProfileUpgrader> (new ECDbProfileUpgrader_1003 ()));
        s_upgraderSequence.push_back (std::unique_ptr<ECDbProfileUpgrader> (new ECDbProfileUpgrader_1004 ()));
        }

    return s_upgraderSequence;
    }


//*************************************** ECDbProfileManager::ProfileCreator *********************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::Create (ECDbR ecdb)
    {
    auto stat = CreateECProfileTables (ecdb);
    if (stat != BE_SQLITE_OK)
        {
        LOG.errorv ("Failed to create %s profile in %s: %s", PROFILENAME, ecdb.GetDbFileName (), ecdb.GetLastError ());
        return stat;
        }

    return ECDbProfileECSchemaUpgrader::ImportProfileSchemas (ecdb);
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
    StopWatch timer ("", true);
    auto stat = CreateTableECSchema (db); 
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableECClass (db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableBaseClass (db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableECProperty (db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableECRelationshipConstraint (db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableECRelationshipConstraintClass (db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableECRelationshipConstraintClassProperty (db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableCustomAttributes (db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableReferences (db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableECClassMap (db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTableECPropertyMap (db);
    if (stat != BE_SQLITE_OK)
        return stat;

    stat = CreateTablePropertyAlias (db);
    if (stat != BE_SQLITE_OK)
        return stat;

    timer.Stop ();
    LOG.debugv ("Created ECDb profile tables in ECDb file in %.4f msecs.", timer.GetElapsedSeconds () * 1000.0);
    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::CreateTableECSchema (Db& db)
    {
    auto stat = db.ExecuteSql (
        "CREATE TABLE ec_Schema "
        "("
        "ECSchemaId INTEGER NOT NULL PRIMARY KEY, "
        "Name CHAR NOT NULL, "
        "DisplayLabel CHAR, "
        "Description CHAR, "
        "NamespacePrefix CHAR, "
        "VersionMajor SMALLINT, "
        "VersionMinor SMALLINT, "
        "MapVersion SMALLINT, " //deprecated, to be removed once backwards compatibility with 03 apps is no longer needed
        "SchemaAsBlob BLOB, " //deprecated, to be removed once backwards compatibility with 03 apps is no longer needed
        "SchemaType SMALLINT, "
        "IsReadonly BOOL CHECK (IsReadonly IN (0, 1))" //deprecated, to be removed once backwards compatibility with 03 apps is no longer needed
        ");");

    if (stat != BE_SQLITE_OK)
        return stat;

    return db.ExecuteSql ("CREATE UNIQUE INDEX idx_UniqueECSchemaName ON ec_Schema ([Name] COLLATE NOCASE);");
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
    auto stat = db.ExecuteSql (
        "CREATE TABLE ec_Class "
        "("
        "ECClassId INTEGER NOT NULL PRIMARY KEY, "
        "ECSchemaId INTEGER NOT NULL REFERENCES ec_Schema(ECSchemaId) ON DELETE CASCADE, "
        "Name CHAR NOT NULL, "
        "DisplayLabel CHAR, "
        "Description CHAR, "
        "IsDomainClass BOOL NOT NULL CHECK (IsDomainClass IN (0, 1)), "
        "IsStruct BOOL NOT NULL CHECK (IsStruct IN (0, 1)), "
        "IsCustomAttribute BOOL NOT NULL CHECK (IsCustomAttribute IN (0, 1)), "
        "RelationStrength SMALLINT, "
        "RelationStrengthDirection SMALLINT, "
        "IsRelationship BOOL NOT NULL CHECK (IsRelationship IN (0, 1))"
        ");");

    if (stat != BE_SQLITE_OK)
        return stat;

    return db.ExecuteSql ("CREATE INDEX idx_ECClassName ON ec_Class ([Name]);");
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
    return db.ExecuteSql (
        "CREATE TABLE ec_ClassMap "   
        "("
        "ECClassId INTEGER NOT NULL PRIMARY KEY REFERENCES ec_Class(ECClassId) ON DELETE CASCADE, "
        "MapParentECClassId INTEGER REFERENCES ec_Class(ECClassId), "
        "MapStrategy SMALLINT NOT NULL, "
        "MapToDbTable CHAR, "
        "SQLCreateTable CHAR, "  //deprecated, to be removed once backwards compatibility with 03 apps is no longer needed
        "PrimaryKeyColumnName CHAR"
        ");");
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
    auto stat = db.ExecuteSql (
        "CREATE TABLE ec_BaseClass "
        "("
        "ECClassId INTEGER NOT NULL REFERENCES ec_Class(ECClassId) ON DELETE CASCADE, "
        "BaseECClassId INTEGER NOT NULL REFERENCES ec_Class(ECClassId) ON DELETE CASCADE, "
        "ECIndex SMALLINT NOT NULL, /*Location of baseclass in BaseClasses array*/ "
        "PRIMARY KEY (ECClassId, BaseECClassId)"
        ");");

    if (stat != BE_SQLITE_OK)
        return stat;

    //index needed for fast look-ups of derived classes for a given ECClass
    return db.ExecuteSql ("CREATE INDEX idx_ec_BaseClass_BaseECClassId ON ec_BaseClass (BaseECClassId);");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::CreateTablePropertyAlias
(
Db& db
)
    {
    auto stat = db.ExecuteSql (
        "CREATE TABLE ec_PropertyAlias "
        "("
        "AliasECPropertyId INTEGER PRIMARY KEY, " 
        "RootECClassId INTEGER  REFERENCES ec_Class(ECClassId) ON DELETE CASCADE, "
        "RootECPropertyId INTEGER  REFERENCES ec_Property(ECPropertyId) ON DELETE CASCADE, "
        "LeafECPropertyId INTEGER NOT NULL REFERENCES ec_Property(ECPropertyId) ON DELETE CASCADE, "
        "AccessString CHAR NOT NULL " 
        ");" );

    if (stat != BE_SQLITE_OK)
        return stat;

    return db.ExecuteSql ("CREATE UNIQUE INDEX idx_ec_PropertyAliasCIDA ON ec_PropertyAlias (RootECClassId, AccessString);");
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
    auto stat = db.ExecuteSql (
        "CREATE TABLE ec_Property "
        "("
        "ECClassId INTEGER NOT NULL REFERENCES ec_Class(ECClassId) ON DELETE CASCADE, "
        "ECPropertyId INTEGER NOT NULL, "
        "Name CHAR NOT NULL, "
        "DisplayLabel CHAR, "
        "Description CHAR, "
        "ECIndex SMALLINT, "
        "IsArray BOOL NOT NULL CHECK (IsArray IN (0, 1)), "
        "TypeCustom CHAR, "
        "TypeECPrimitive SMALLINT, "
        "TypeGeometry CHAR, "
        "TypeECStruct INTEGER REFERENCES ec_Class(ECClassId) ON DELETE CASCADE, "
        "IsReadOnly BOOL NOT NULL CHECK (IsReadOnly IN (0, 1)), "
        "MinOccurs INTEGER,"
        "MaxOccurs INTEGER,"
        "PRIMARY KEY (ECPropertyId)"
        ");");

    if (stat != BE_SQLITE_OK)
        return stat;

    stat = db.ExecuteSql ("CREATE INDEX idx_ECPropertyName ON ec_Property ([Name]);");
    if (stat != BE_SQLITE_OK)
        return stat;

    return db.ExecuteSql ("CREATE INDEX idx_ECClassId ON ec_Property (ECClassId);");
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
    return db.ExecuteSql (
        "CREATE TABLE ec_PropertyMap "
        "("
        "ECPropertyId INTEGER NOT NULL, "
        "MapColumnName CHAR NOT NULL, "
        "PRIMARY KEY (ECPropertyId), "
        "FOREIGN KEY (ECPropertyId) REFERENCES ec_Property(ECPropertyId) ON DELETE CASCADE"
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
    return db.ExecuteSql (
        "CREATE TABLE ec_RelationshipConstraint "
        "("
        "ECClassId INTEGER NOT NULL REFERENCES ec_Class(ECClassId) ON DELETE CASCADE, "
        "ECRelationshipEnd BOOL NOT NULL CHECK (ECRelationshipEnd IN (0, 1)), "
        "CardinalityLowerLimit SMALLINT, "
        "CardinalityUpperLimit SMALLINT, "
        "RoleLable CHAR, " //Deprecated: To be renamed to RoleLabel once backwards compatibility to 03 apps is not needed anymore
        "IsPolymorphic BOOL NOT NULL CHECK (IsPolymorphic IN (0, 1)), "
        "PRIMARY KEY (ECClassId, ECRelationshipEnd)"
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
    return db.ExecuteSql (
        "CREATE TABLE ec_RelationshipConstraintClass "
        "("
        "ECClassId INTEGER NOT NULL, "
        "ECRelationshipEnd BOOL NOT NULL, "
        "RelationECClassId INTEGER NOT NULL REFERENCES ec_Class(ECClassId) ON DELETE CASCADE, "
        "RelationKeyDbColumnName CHAR, " //deprecated, to be removed once backwards compatibility with 03 apps is no longer needed
        "PRIMARY KEY (ECClassId, ECRelationshipEnd, RelationECClassId), "
        "FOREIGN KEY (ECClassId, ECRelationshipEnd) "
        "REFERENCES ec_RelationshipConstraint(ECClassId, ECRelationshipEnd) ON DELETE CASCADE"
        ");");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::CreateTableECRelationshipConstraintClassProperty 
(
Db& db
)
    {
    return db.ExecuteSql (
        "CREATE TABLE ec_RelationshipConstraintClassProperty "
        "("
        "ECClassId INTEGER NOT NULL, "
        "ECRelationshipEnd BOOL NOT NULL, "
        "RelationECClassId INTEGER NOT NULL, "
        "RelationECPropertyId INTEGER NOT NULL, "
        "PRIMARY KEY (ECClassId, ECRelationshipEnd, RelationECClassId, RelationECPropertyId), "
        "FOREIGN KEY (ECClassId, ECRelationshipEnd, RelationECClassId) "
        "REFERENCES ec_RelationshipConstraintClass (ECClassId, ECRelationshipEnd, RelationECClassId) ON DELETE CASCADE, "
        "FOREIGN KEY (RelationECPropertyId) "
        "REFERENCES ec_Property (ECPropertyId) ON DELETE CASCADE"
        ");");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::CreateTableCustomAttributes 
(
Db& db
)
    {
    return db.ExecuteSql (
        "CREATE TABLE ec_CustomAttribute ("
        "ContainerId INTEGER NOT NULL, "
        "ContainerType SMALLINT NOT NULL, "
        "OverridenByContainerId INTEGER, "
        "[Index] SMALLINT NOT NULL, "
        "ECClassId INTEGER NOT NULL REFERENCES ec_Class(ECClassId), "
        "ECId INTEGER, "
        "Instance TEXT, "
        "PRIMARY KEY (ContainerId, ContainerType, ECClassId)"
        ");");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileManager::ProfileCreator::CreateTableReferences 
(
Db& db
)
    {
    return db.ExecuteSql (
        "CREATE TABLE ec_SchemaReference ("
        "ECSchemaId INTEGER REFERENCES ec_Schema(ECSchemaId) ON DELETE CASCADE, "
        "ReferenceECSchemaId INTEGER REFERENCES ec_Schema(ECSchemaId) ON DELETE CASCADE"
        ");");
    }

//*************************************** ECDbProfileManager::ProfileUpgradeContext *************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECDbProfileManager::ProfileUpgradeContext::ProfileUpgradeContext (ECDbR ecdb, Savepoint& defaultTransaction)
    : m_ecdb (ecdb), m_defaultTransaction (defaultTransaction), m_isDefaultTransOpenMode (defaultTransaction.IsActive ()), m_rollbackOnDestruction (true)
    {
    DisableForeignKeyEnforcement ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECDbProfileManager::ProfileUpgradeContext::~ProfileUpgradeContext ()
    {
    EnableForeignKeyEnforcement ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void ECDbProfileManager::ProfileUpgradeContext::DisableForeignKeyEnforcement ()
    {
    if (m_isDefaultTransOpenMode)
        m_defaultTransaction.Commit ();

    //Need to use TryExecuteSql which calls SQLite directly without any checks (Calling ExecuteSql would
    //check that a transaction is active which we explicity must not have for setting this pragma)
    auto stat = m_ecdb.TryExecuteSql ("PRAGMA foreign_keys = OFF;");
    if (stat != BE_SQLITE_OK)
        {
        LOG.error ("ECDb profile upgrade: Disabling foreign key enforcement in SQLite failed.");
        BeAssert (false);
        }

    if (!m_isDefaultTransOpenMode)
        m_defaultTransaction.SetTxnMode (BeSQLiteTxnMode::Immediate);

    m_beginTransError = m_defaultTransaction.Begin ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void ECDbProfileManager::ProfileUpgradeContext::EnableForeignKeyEnforcement () const
    {
    if (m_rollbackOnDestruction)
        m_defaultTransaction.Cancel ();
    else
        m_defaultTransaction.Commit ();

    //Need to use TryExecuteSql which calls SQLite directly without any checks (Calling ExecuteSql would
    //check that a transaction is active which we explicity must not have for setting this pragma)
    auto stat = m_ecdb.TryExecuteSql ("PRAGMA foreign_keys = ON;");
    if (stat != BE_SQLITE_OK)
        {
        LOG.error ("ECDb profile upgrade: Re-enabling foreign key enforcement in SQLite failed.");
        BeAssert (false);
        }

    if (m_isDefaultTransOpenMode)
        m_defaultTransaction.Begin ();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
