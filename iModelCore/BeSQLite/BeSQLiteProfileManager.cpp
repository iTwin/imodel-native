/*--------------------------------------------------------------------------------------+
|
|     $Source: BeSQLiteProfileManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BeSQLiteProfileManager.h"
#include <Logging/bentleylogging.h>

//WIP redundant to BeSQLite.cpp
#define LOG (*NativeLogging::LoggingManager::GetLogger (L"BeSQLite"))

BEGIN_BENTLEY_SQLITE_NAMESPACE

//************************************************************************
// BeSQLiteProfileManager
//************************************************************************

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                10/2014
//---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP const BeSQLiteProfileManager::PROFILENAME = "BeSQLite";
const SchemaVersion BeSQLiteProfileManager::s_expectedProfileVersion = SchemaVersion (BEDB_CURRENT_VERSION_Major, BEDB_CURRENT_VERSION_Minor, BEDB_CURRENT_VERSION_Sub1, BEDB_CURRENT_VERSION_Sub2);
const SchemaVersion BeSQLiteProfileManager::s_minimumAutoUpgradableProfileVersion = SchemaVersion (BEDB_SUPPORTED_VERSION_Major, BEDB_SUPPORTED_VERSION_Minor, BEDB_SUPPORTED_VERSION_Sub1, BEDB_SUPPORTED_VERSION_Sub2);
std::vector<std::unique_ptr<BeSQLiteProfileUpgrader>> BeSQLiteProfileManager::s_upgraderSequence;

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                10/2014
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult BeSQLiteProfileManager::UpgradeProfile (DbR db)
    {
    if (db.IsReadonly ())
        {
        LOG.errorv ("Upgrade of %s profile of file '%s' failed. File must be opened in readwrite mode.", PROFILENAME, db.GetDbFileName ());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    SchemaVersion actualProfileVersion (0, 0, 0, 0);
    auto stat = ReadProfileVersion (actualProfileVersion, db);
    if (stat != BE_SQLITE_OK)
        {
        LOG.errorv ("Upgrade of %s profile of file '%s' failed. File is no BeSQLite file.", PROFILENAME, db.GetDbFileName ());
        return stat;
        }

    bool profileNeedsUpgrade = false;
    stat = Db::CheckProfileVersion (profileNeedsUpgrade, s_expectedProfileVersion, actualProfileVersion, s_minimumAutoUpgradableProfileVersion, false, PROFILENAME);
    if (!profileNeedsUpgrade)
        return stat;

    LOG.infov ("Version of file's %s profile is too old. Upgrading '%s' now...", PROFILENAME, db.GetDbFileName ());

    //Call upgrader sequence and let upgraders incrementally upgrade the profile
    //to the latest state
    auto upgraderIterator = GetUpgraderSequenceFor (actualProfileVersion);
    BeAssert (upgraderIterator != GetUpgraderSequence ().end () && "Upgrader sequence is not expected to be empty as the profile status is 'requires upgrade'");
    for (;upgraderIterator != GetUpgraderSequence ().end (); ++upgraderIterator)
        {
        auto const& upgrader = *upgraderIterator;
        auto stat = upgrader->Upgrade (db);
        if (stat != BE_SQLITE_OK)
            return BE_SQLITE_ERROR_ProfileUpgradeFailed; //context dtor ensures that changes are rolled back
        }

    //after upgrade procedure set new profile version in BeSQLite file
    stat = AssignProfileVersion (db);

    if (stat != BE_SQLITE_OK)
        {
        LOG.errorv ("Upgrade of %s profile of file '%s' failed. Could not assign new profile version. %s",
                    PROFILENAME, db.GetDbFileName (), db.GetLastError ().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed; 
        }
    
    if (LOG.isSeverityEnabled (NativeLogging::LOG_INFO))
        {
        LOG.infov ("Upgraded %s profile of file '%s' from version %d.%d.%d.%d to version %d.%d.%d.%d.",
                   PROFILENAME, db.GetDbFileName (),
            actualProfileVersion.GetMajor (), actualProfileVersion.GetMinor (), actualProfileVersion.GetSub1 (), actualProfileVersion.GetSub2 (),
            s_expectedProfileVersion.GetMajor (), s_expectedProfileVersion.GetMinor (), s_expectedProfileVersion.GetSub1 (), s_expectedProfileVersion.GetSub2 ());
        }

    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                10/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult BeSQLiteProfileManager::AssignProfileVersion (DbR db)
    {
    //Save the profile version as string (JSON format)
    const auto profileVersionStr = s_expectedProfileVersion.ToJson ();
    const auto stat = db.SavePropertyString (Properties::SchemaVersion (), profileVersionStr);
    if (BE_SQLITE_OK != stat)
        LOG.errorv ("Failed to create %s profile in file '%s'. Could not assign new profile version. %s",
                    PROFILENAME, db.GetDbFileName (), db.GetLastError ().c_str());

    return stat;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                10/2014
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult BeSQLiteProfileManager::ReadProfileVersion (SchemaVersion& profileVersion, DbR db)
    {
    Utf8String currentVersionString;
    auto stat = db.QueryProperty (currentVersionString, Properties::SchemaVersion ());
    if (stat == BE_SQLITE_ROW)
        {
        profileVersion.FromJson (currentVersionString.c_str ());
        return BE_SQLITE_OK;
        }
    
    //File is no BeSQLite file
    return BE_SQLITE_ERROR_InvalidProfileVersion;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                10/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
BeSQLiteProfileManager::UpgraderSequence::const_iterator BeSQLiteProfileManager::GetUpgraderSequenceFor (SchemaVersion const& currentProfileVersion)
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
// @bsimethod                                Krischan.Eberle                10/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
BeSQLiteProfileManager::UpgraderSequence const& BeSQLiteProfileManager::GetUpgraderSequence ()
    {
    if (s_upgraderSequence.empty ())
        {
        s_upgraderSequence.push_back (std::unique_ptr<BeSQLiteProfileUpgrader> (new BeSQLiteProfileUpgrader_3101 ()));
        }

    BeAssert (s_upgraderSequence.back ()->GetTargetVersion () == s_expectedProfileVersion);
    return s_upgraderSequence;
    }

//************************************************************************
// BeSQLiteProfileUpgrader 
//************************************************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                10/2014
//+---------------+---------------+---------------+---------------+---------------+--------
SchemaVersion BeSQLiteProfileUpgrader::GetTargetVersion () const
    {
    return _GetTargetVersion ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                10/2014
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult BeSQLiteProfileUpgrader::Upgrade (DbR db) const
    {
    return _Upgrade (db);
    }

//************************************************************************
// BeSQLiteProfileUpgrader_3101 
//************************************************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                10/2014
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult BeSQLiteProfileUpgrader_3101::_Upgrade (DbR db) const
    {
    if (db.TableExists (BEDB_TABLE_EmbeddedFile))
        {
        auto stat = db.ExecuteSql ("ALTER TABLE " BEDB_TABLE_EmbeddedFile " ADD COLUMN LastModified TimeStamp;");
        if (stat == BE_SQLITE_OK)
            LOG.debug ("BeSQLite profile upgrade: Added column 'LastModified' to table '" BEDB_TABLE_EmbeddedFile "'.");
        else
            {
            LOG.error ("BeSQLite profile upgrade failed: Adding column 'LastModified' to table '" BEDB_TABLE_EmbeddedFile "' failed.");
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
            }
        }
    else
        {
        auto stat = db.EmbeddedFiles ().CreateTable ();
        if (stat == BE_SQLITE_OK)
            LOG.debug ("BeSQLite profile upgrade: Created table '" BEDB_TABLE_EmbeddedFile "'.");
        else
            {
            LOG.error ("BeSQLite profile upgrade failed: Creating table '" BEDB_TABLE_EmbeddedFile "' failed.");
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
            }
        }

    return BE_SQLITE_OK;
    }

END_BENTLEY_SQLITE_NAMESPACE
