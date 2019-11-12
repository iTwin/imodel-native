/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "UpgraderFromV20ToV21.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UpgraderFromV20ToV21::UpgraderFromV20ToV21(ECDbAdapter& adapter) : UpgraderBase(adapter)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV20ToV21::Upgrade()
    {
    if (SUCCESS != UpgradeCacheSchema(2, 1))
        return ERROR;

    if (SUCCESS != ExecuteStatement("UPDATE ONLY WSCache.CachedResponsePageInfo SET CacheTag = ''"))
        return ERROR;

    return SUCCESS;
    }
