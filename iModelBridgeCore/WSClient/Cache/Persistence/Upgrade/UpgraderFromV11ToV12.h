/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Upgrade/UpgraderFromV11ToV12.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "UpgraderBase.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
struct UpgraderFromV11ToV12 : private UpgraderBase
    {
    public:
        UpgraderFromV11ToV12(ECDbAdapter& adapter);
        BentleyStatus Upgrade();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
