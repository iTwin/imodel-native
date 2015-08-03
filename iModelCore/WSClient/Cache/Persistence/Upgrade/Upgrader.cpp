/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Upgrade/Upgrader.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "Upgrader.h"

#include <WebServices/Cache/Util/ECDbHelper.h>
#include <WebServices/Cache/Persistence/DataSourceCache.h>
#include <WebServices/Cache/Util/FileUtil.h>

#include "../Core/CacheSettings.h"
#include "../Files/FileCacheManager.h"
#include "UpgraderFromV3ToV4.h"
#include "UpgraderFromV4ToV5.h"
#include "UpgraderFromV5ToCurrent.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Upgrader::Upgrader(ObservableECDb& db, CacheEnvironmentCR environment) :
m_adapter(db),
m_environment(environment)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Upgrader::Upgrade(int oldVersion)
    {
    switch (oldVersion)
        {
            case 3:
                if (SUCCESS != UpgraderFromV3ToV4(m_adapter, m_environment).Upgrade()) return ERROR;
            case 4:
                if (SUCCESS != UpgraderFromV4ToV5(m_adapter).Upgrade()) return ERROR;
            case 5:
                if (SUCCESS != UpgraderFromV5ToCurrent(m_adapter, m_environment).Upgrade()) return ERROR;
                return SUCCESS;
            case 6:
                // Upgrade not supported
                return ERROR;
            case 7:
                // Current version
                return SUCCESS;
            default:
                return ERROR;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName Upgrader::GetNewCachePathForUpgrade(BeFileNameCR oldCachePath, CacheEnvironmentCR environment)
    {
    return BeFileName(oldCachePath + L"-upgrade-new");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Upgrader::FinalizeUpgradeIfNeeded(BeFileNameCR oldCachePath, CacheEnvironmentCR environment)
    {
    BeFileName newCachePath = GetNewCachePathForUpgrade(oldCachePath, environment);
    if (!newCachePath.DoesPathExist())
        {
        return SUCCESS;
        }

    if (!GetUpgradeFinishedFlag(newCachePath))
        {
        // New db is not yet finalized
        return SUCCESS;
        }

    // Do not delete old cache file to keep this operation atomic - without loosing track if cache db exists
    if (SUCCESS != FileUtil::CopyFileContent(newCachePath, oldCachePath))
        {
        return ERROR;
        }

    auto oldEnv = FileCacheManager::CreateCacheEnvironment(oldCachePath, environment);
    auto newEnv = FileCacheManager::CreateCacheEnvironment(newCachePath, environment);

    if (oldEnv.persistentFileCacheDir.DoesPathExist() &&
        BeFileNameStatus::Success != BeFileName::EmptyAndRemoveDirectory(oldEnv.persistentFileCacheDir))
        {
        return ERROR;
        }

    if (oldEnv.temporaryFileCacheDir.DoesPathExist() &&
        BeFileNameStatus::Success != BeFileName::EmptyAndRemoveDirectory(oldEnv.temporaryFileCacheDir))
        {
        return ERROR;
        }

    if (newEnv.persistentFileCacheDir.DoesPathExist() &&
        BeFileNameStatus::Success != BeFileName::BeMoveFile(newEnv.persistentFileCacheDir, oldEnv.persistentFileCacheDir))
        {
        return ERROR;
        }

    if (newEnv.temporaryFileCacheDir.DoesPathExist() &&
        BeFileNameStatus::Success != BeFileName::BeMoveFile(newEnv.temporaryFileCacheDir, oldEnv.temporaryFileCacheDir))
        {
        return ERROR;
        }

    if (BeFileNameStatus::Success != BeFileName::BeDeleteFile(newCachePath))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Upgrader::SetUpgradeFinishedFlag(BeFileNameCR cacheFilePath)
    {
    Db db;

    if (DbResult::BE_SQLITE_OK != db.OpenBeSQLiteDb(cacheFilePath, Db::OpenParams(ECDb::OpenMode::OPEN_ReadWrite)) ||
        DbResult::BE_SQLITE_OK != db.SavePropertyString(PropertySpec("UpgradeFinished", "Upgrader"), "") ||
        DbResult::BE_SQLITE_OK != db.SaveChanges())
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool Upgrader::GetUpgradeFinishedFlag(BeFileNameCR cacheFilePath)
    {
    Db db;

    if (DbResult::BE_SQLITE_OK != db.OpenBeSQLiteDb(cacheFilePath, Db::OpenParams(ECDb::OpenMode::OPEN_Readonly)))
        {
        return false;
        }

    bool upgradeFinished = db.HasProperty(PropertySpec("UpgradeFinished", "Upgrader"));
    db.CloseDb();
    return upgradeFinished;
    }
