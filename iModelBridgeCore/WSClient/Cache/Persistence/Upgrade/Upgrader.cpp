/*--------------------------------------------------------------------------------------+
 |
 |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 |
 +--------------------------------------------------------------------------------------*/

#include "Upgrader.h"

#include <WebServices/Cache/Util/ECDbHelper.h>
#include <WebServices/Cache/Persistence/DataSourceCache.h>
#include <WebServices/Cache/Util/FileUtil.h>

#include "../Core/CacheSettings.h"
#include "../Files/FileStorage.h"
#include "UpgraderFromV3ToV4.h"
#include "UpgraderFromV4ToV5.h"
#include "UpgraderFromV5ToCurrent.h"
#include "UpgraderFromV7ToV9.h"
#include "UpgraderFromV9ToV10.h"
#include "UpgraderFromV10ToV11.h"
#include "UpgraderFromV11ToV12.h"
#include "UpgraderFromV12ToV13.h"
#include "UpgraderFromV20ToV21.h"
#include "UpgraderFromV21ToV23.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Upgrader::Upgrader(ObservableECDb& db, CacheEnvironmentCR baseEnvironment) :
m_db(db),
m_baseEnvironment(baseEnvironment)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Upgrader::Upgrade(int oldVersion)
    {
    ECDbAdapter adapter (m_db);
    switch (oldVersion)
        {
        // GraphiteXXXX generation
        case 3:
            if (SUCCESS != UpgraderFromV3ToV4(adapter, m_baseEnvironment).Upgrade()) return ERROR;
        case 4:
            if (SUCCESS != UpgraderFromV4ToV5(adapter).Upgrade()) return ERROR;
        case 5:
            if (SUCCESS != UpgraderFromV5ToCurrent(adapter, m_baseEnvironment).Upgrade()) return ERROR;
            return SUCCESS; // Upgraded to latest version, done
        case 6:
            return ERROR; // 5 to 6 handled in previous case
        case 7:
        case 8:
            if (SUCCESS != UpgraderFromV7ToV9(adapter).Upgrade()) return ERROR;
        case 9:
            if (SUCCESS != UpgraderFromV9ToV10(adapter).Upgrade()) return ERROR;
        case 10:
            if (SUCCESS != UpgraderFromV10ToV11(adapter).Upgrade()) return ERROR;
        case 11:
            if (SUCCESS != UpgraderFromV11ToV12(adapter).Upgrade()) return ERROR;
        case 12:
            if (SUCCESS != UpgraderFromV12ToV13(adapter).Upgrade()) return ERROR;

        // DgnDb0601/BimXXXX generation
        case 20:
            if (SUCCESS != UpgraderFromV20ToV21(adapter).Upgrade()) return ERROR;
        case 21:
        case 22:
            if (SUCCESS != UpgraderFromV21ToV23(adapter).Upgrade()) return ERROR;
            // Current version, return
            return SUCCESS;
        default:
            BeAssert(false && "Upgrade to newer version not implemented");
            return ERROR;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName Upgrader::GetNewCachePathForUpgrade(BeFileNameCR oldCachePath, CacheEnvironmentCR baseEnvironment)
    {
    return BeFileName(oldCachePath + L"-upgrade-new");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Upgrader::FinalizeUpgradeIfNeeded(BeFileNameCR oldCachePath, CacheEnvironmentCR baseEnvironment)
    {
    BeFileName newCachePath = GetNewCachePathForUpgrade(oldCachePath, baseEnvironment);
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

    auto oldEnv = FileStorage::CreateCacheEnvironment(oldCachePath, baseEnvironment);
    auto newEnv = FileStorage::CreateCacheEnvironment(newCachePath, baseEnvironment);

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

    if (DbResult::BE_SQLITE_OK != db.OpenBeSQLiteDb(cacheFilePath, Db::OpenParams(ECDb::OpenMode::ReadWrite)) ||
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

    if (DbResult::BE_SQLITE_OK != db.OpenBeSQLiteDb(cacheFilePath, Db::OpenParams(ECDb::OpenMode::Readonly)))
        {
        return false;
        }

    bool upgradeFinished = db.HasProperty(PropertySpec("UpgradeFinished", "Upgrader"));
    db.CloseDb();
    return upgradeFinished;
    }
