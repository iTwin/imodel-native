/*--------------------------------------------------------------------------------------+
|
|     $Source: BeSQLiteProfileManager.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BeSQLiteProfileManager.h"
#include <Logging/bentleylogging.h>

//WIP redundant to BeSQLite.cpp
#define LOG (*NativeLogging::LoggingManager::GetLogger (L"BeSQLite"))

#define PROFILENAME "BeSQLite"

BEGIN_BENTLEY_SQLITE_NAMESPACE

//************************************************************************
// BeSQLiteProfileManager
//************************************************************************
//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                10/2014
//---------------+---------------+---------------+---------------+---------------+------
//static
ProfileState BeSQLiteProfileManager::CheckProfileVersion(DbCR db)
    {
    ProfileVersion fileProfileVersion(0, 0, 0, 0);
    if (BE_SQLITE_OK != ReadProfileVersion(fileProfileVersion, db))
        return ProfileState::Error();

    return Db::CheckProfileVersion(GetExpectedVersion(), fileProfileVersion, GetMinimumSupportedVersion(), PROFILENAME);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                10/2014
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult BeSQLiteProfileManager::UpgradeProfile(DbR db)
    {
    BeAssert(!db.IsReadonly());
    if (!db.GetDefaultTransaction()->IsActive())
        {
        BeAssert(false && "Programmer Error. BeSqlite::OpenBeSQliteDb must keep the default transaction active when it is called to upgrade its profile.");
        return BE_SQLITE_ERROR_NoTxnActive;
        }

    ProfileVersion fileProfileVersion(0, 0, 0, 0);
    DbResult stat = ReadProfileVersion(fileProfileVersion, db);
    if (stat != BE_SQLITE_OK)
        {
        LOG.errorv("Upgrade of " PROFILENAME " profile of file '%s' failed. File is not a BeSQLite file.", db.GetDbFileName());
        return stat;
        }

    //Call upgrader sequence and let upgraders incrementally upgrade the profile
    //to the latest state
    std::vector<std::unique_ptr<BeSQLiteProfileUpgrader>> upgraders;
    GetUpgraderSequence(upgraders, fileProfileVersion);
    for (std::unique_ptr<BeSQLiteProfileUpgrader> const& upgrader : upgraders)
        {
        stat = upgrader->Upgrade(db);
        if (BE_SQLITE_OK != stat)
            {
            db.AbandonChanges();
            return stat;
            }
        }

    //after upgrade procedure set new profile version in BeSQLite file
    stat = AssignProfileVersion(db);
    if (stat != BE_SQLITE_OK)
        {
        LOG.errorv("Upgrade of " PROFILENAME " profile of file '%s' failed. Could not assign new profile version. %s",
                   db.GetDbFileName(), db.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (LOG.isSeverityEnabled(NativeLogging::LOG_INFO))
        {
        LOG.infov("Upgraded " PROFILENAME " profile of file '%s' from version %s to version %s.",
                  db.GetDbFileName(), fileProfileVersion.ToString().c_str(), GetExpectedVersion().ToString().c_str());
        }

    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                10/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult BeSQLiteProfileManager::AssignProfileVersion(DbR db)
    {
    //Save the profile version as string (JSON format)
    Utf8String profileVersionStr = GetExpectedVersion().ToJson();
    return db.SavePropertyString(Properties::ProfileVersion(), profileVersionStr);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                10/2014
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult BeSQLiteProfileManager::ReadProfileVersion(ProfileVersion& profileVersion, DbCR db)
    {
    Utf8String currentVersionString;
    auto stat = db.QueryProperty(currentVersionString, Properties::ProfileVersion());
    if (stat == BE_SQLITE_ROW)
        {
        profileVersion.FromJson(currentVersionString.c_str());
        return BE_SQLITE_OK;
        }

    //File is no BeSQLite file
    return BE_SQLITE_ERROR_InvalidProfileVersion;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    04/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static
void BeSQLiteProfileManager::GetUpgraderSequence(std::vector<std::unique_ptr<BeSQLiteProfileUpgrader>>& upgraders, ProfileVersion const& currentProfileVersion)
    {
    upgraders.clear();
    if (currentProfileVersion < ProfileVersion(3, 1, 0, 2))
        upgraders.push_back(std::unique_ptr<BeSQLiteProfileUpgrader>(new ProfileUpgrader_3102()));
    if (currentProfileVersion < ProfileVersion(3, 1, 0, 3))
        upgraders.push_back(std::unique_ptr<BeSQLiteProfileUpgrader>(new ProfileUpgrader_3103()));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan    1/2018
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ProfileUpgrader_3102::_Upgrade(DbR db) const
    {
    if (db.TableExists("sqlite_stat1") || db.TableExists("sqlite_stat2") || db.TableExists("sqlite_stat3") || db.TableExists("sqlite_stat4"))
        {
        LOG.error("Sqlite file should not have sqlite_stat1, sqlite_stat2, sqlite_stat3 or sqlite_stat4 system tables at the time of upgrade.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    //execute the command to system can create sqlite_stat1 table.
    if (BE_SQLITE_OK != db.ExecuteSql("analyze"))
        {
        LOG.errorv("BeSqlite profile upgrade failed: 'analyze' command failed to execute. failed: %s.", db.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    if (BE_SQLITE_OK != db.ExecuteSql("delete from sqlite_stat1"))
        {
        LOG.errorv("BeSqlite profile upgrade failed: deleting rows from sqlite_stat1 table failed. failed: %s.", db.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    LOG.debug("BeSqlite profile upgrade: Added sqlite_stat1 table using analyze command and truncated it so the table is empty.");
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2019
//---------------+---------------+---------------+---------------+---------------+-------
DbResult ProfileUpgrader_3103::_Upgrade(DbR db) const
    {
    // This should never happen, but better safe than sorry
    Statement stmt;
    stmt.Prepare(db, "SELECT 1 FROM sqlite_master where type='index' AND name='ix_" BEDB_TABLE_Property "_Property'");
    if (BE_SQLITE_ROW == stmt.Step())
        return BE_SQLITE_OK;

    DbResult rc = db.CreateIndex("[ix_" BEDB_TABLE_Property "_Property]", BEDB_TABLE_Property, false, "[Namespace], [Name], [Id]");
    if (BE_SQLITE_OK != rc)
        return rc;

    return BE_SQLITE_OK;
    }
END_BENTLEY_SQLITE_NAMESPACE
