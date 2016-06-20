/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Upgrade/UpgraderFromV11ToV12.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UpgraderFromV11ToV12.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UpgraderFromV11ToV12::UpgraderFromV11ToV12(ECDbAdapter& adapter) : UpgraderBase(adapter)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV11ToV12::Upgrade()
    {
    if (SUCCESS != UpgradeCacheSchema(1, 9))
        return ERROR;

    if (SUCCESS != ExecuteStatement("UPDATE ONLY DSC.CachedFileInfo SET UpdateDate = CacheDate"))
        return ERROR;

    return SUCCESS;
    }