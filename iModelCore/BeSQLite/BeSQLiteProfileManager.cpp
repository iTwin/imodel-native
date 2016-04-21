/*--------------------------------------------------------------------------------------+
|
|     $Source: BeSQLiteProfileManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
DbResult BeSQLiteProfileManager::UpgradeProfile(DbR db)
    {
    if (db.IsReadonly())
        {
        LOG.errorv("Upgrade of " PROFILENAME " profile of file '%s' failed. File must be opened in readwrite mode.", db.GetDbFileName());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    SchemaVersion actualProfileVersion(0, 0, 0, 0);
    auto stat = ReadProfileVersion(actualProfileVersion, db);
    if (stat != BE_SQLITE_OK)
        {
        LOG.errorv("Upgrade of " PROFILENAME " profile of file '%s' failed. File is no BeSQLite file.", db.GetDbFileName());
        return stat;
        }

    const SchemaVersion expectedVersion = GetExpectedVersion();

    bool profileNeedsUpgrade = false;
    stat = Db::CheckProfileVersion(profileNeedsUpgrade, expectedVersion, actualProfileVersion, GetMinimumSupportedVersion(), false, PROFILENAME);
    if (!profileNeedsUpgrade)
        return stat;

    LOG.infov("Version of file's " PROFILENAME " profile is too old. Upgrading '%s' now...", db.GetDbFileName());

    //Call upgrader sequence and let upgraders incrementally upgrade the profile
    //to the latest state
    std::vector<std::unique_ptr<BeSQLiteProfileUpgrader>> upgraders;
    GetUpgraderSequence(upgraders, actualProfileVersion);
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
                  db.GetDbFileName(), actualProfileVersion.ToString().c_str(), expectedVersion.ToString().c_str());
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
    return db.SavePropertyString(Properties::SchemaVersion(), profileVersionStr);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                10/2014
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult BeSQLiteProfileManager::ReadProfileVersion(SchemaVersion& profileVersion, DbR db)
    {
    Utf8String currentVersionString;
    auto stat = db.QueryProperty(currentVersionString, Properties::SchemaVersion());
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
void BeSQLiteProfileManager::GetUpgraderSequence(std::vector<std::unique_ptr<BeSQLiteProfileUpgrader>>& upgraders, SchemaVersion const& currentProfileVersion)
    {
    upgraders.clear();
    }

END_BENTLEY_SQLITE_NAMESPACE
