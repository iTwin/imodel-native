/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/Persistence/CacheEnvironment.h>
#include "UpgraderBase.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct UpgraderFromV3ToV4 : private UpgraderBase
    {
    private:
        CacheEnvironment m_environment;

    public:
        UpgraderFromV3ToV4(ECDbAdapter& adapter, CacheEnvironmentCR environment);
        BentleyStatus Upgrade();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
