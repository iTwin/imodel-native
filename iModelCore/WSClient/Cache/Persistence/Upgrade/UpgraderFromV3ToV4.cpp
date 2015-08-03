/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Upgrade/UpgraderFromV3ToV4.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "UpgraderFromV3ToV4.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
UpgraderFromV3ToV4::UpgraderFromV3ToV4(ECDbAdapter& adapter, CacheEnvironmentCR environment) :
UpgraderBase(adapter), m_environment(environment)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV3ToV4::Upgrade()
    {
    // Schema update
    if (SUCCESS != UpgradeCacheSchema(1, 2))
        {
        return ERROR;
        }

    // Directory change
    BeFileName cachePath(m_adapter.GetECDb().GetDbFileName());
    WString cacheName = BeFileName::GetFileNameAndExtension(cachePath);

    CacheEnvironment envV3 = m_environment;
    envV3.persistentFileCacheDir.AppendToPath((cacheName + L"_FileCache").c_str());
    envV3.temporaryFileCacheDir.AppendToPath((cacheName + L"_FileCache").c_str());

    CacheEnvironment envV4 = m_environment;
    envV4.persistentFileCacheDir.AppendToPath((cacheName + L"f").c_str());
    envV4.temporaryFileCacheDir.AppendToPath((cacheName + L"f").c_str());

    if (envV3.persistentFileCacheDir.DoesPathExist())
        {
        if (BeFileNameStatus::Success != BeFileName::BeMoveFile(envV3.persistentFileCacheDir, envV4.persistentFileCacheDir))
            {
            return ERROR;
            }
        }

    if (envV3.temporaryFileCacheDir.DoesPathExist())
        {
        if (BeFileNameStatus::Success != BeFileName::BeMoveFile(envV3.temporaryFileCacheDir, envV4.temporaryFileCacheDir))
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }

