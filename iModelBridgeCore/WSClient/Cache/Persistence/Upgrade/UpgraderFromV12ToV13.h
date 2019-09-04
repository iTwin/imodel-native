/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "UpgraderBase.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
struct UpgraderFromV12ToV13 : private UpgraderBase
    {
    public:
        UpgraderFromV12ToV13(ECDbAdapter& adapter);
        BentleyStatus Upgrade();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
