/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Upgrade/UpgraderFromV10ToV11.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "UpgraderFromV10ToV11.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
UpgraderFromV10ToV11::UpgraderFromV10ToV11(ECDbAdapter& adapter) : UpgraderBase(adapter) {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV10ToV11::Upgrade()
    {
    return UpgradeCacheSchema(1, 8);
    }