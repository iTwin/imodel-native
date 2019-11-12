/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

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
