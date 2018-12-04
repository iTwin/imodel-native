/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Upgrade/UpgraderFromV21ToV23.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UpgraderFromV21ToV23.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UpgraderFromV21ToV23::UpgraderFromV21ToV23(ECDbAdapter& adapter) : UpgraderBase(adapter)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV21ToV23::Upgrade()
    {
    if (SUCCESS != UpgradeCacheSchema(2, 2))
        return ERROR;

    return SUCCESS;
    }
