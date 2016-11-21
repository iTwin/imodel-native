/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Upgrade/Upgrader.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/Util/ECDbAdapter.h>
#include <WebServices/Cache/Persistence/CacheEnvironment.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct Upgrader
    {
    private:
        ObservableECDb& m_db;
        CacheEnvironment m_baseEnvironment;

    private:
        BentleyStatus UpgradeCascade(int oldVersion);

    public:
        Upgrader(ObservableECDb& db, CacheEnvironmentCR baseEnvironment);
        BentleyStatus Upgrade(int oldVersion);

        static BentleyStatus FinalizeUpgradeIfNeeded(BeFileNameCR oldCachePath, CacheEnvironmentCR baseEnvironment);
        static BeFileName GetNewCachePathForUpgrade(BeFileNameCR oldCachePath, CacheEnvironmentCR baseEnvironment);

        static BentleyStatus SetUpgradeFinishedFlag(BeFileNameCR cachePath);
        static bool GetUpgradeFinishedFlag(BeFileNameCR cachePath);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
