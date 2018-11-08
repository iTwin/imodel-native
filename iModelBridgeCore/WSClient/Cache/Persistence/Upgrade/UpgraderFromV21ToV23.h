/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Upgrade/UpgraderFromV21ToV23.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "UpgraderBase.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
struct UpgraderFromV21ToV23 : private UpgraderBase
    {
    public:
        UpgraderFromV21ToV23(ECDbAdapter& adapter);
        BentleyStatus Upgrade();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
