/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "UpgraderFromV12ToV13.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UpgraderFromV12ToV13::UpgraderFromV12ToV13(ECDbAdapter& adapter) : UpgraderBase(adapter)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV12ToV13::Upgrade()
    {
    if (SUCCESS != UpgradeCacheSchema(1, 10))
        return ERROR;

    if (SUCCESS != ExecuteStatement("UPDATE ONLY DSC.CachedResponsePageInfo SET CacheTag = ''"))
        return ERROR;

    return SUCCESS;
    }
