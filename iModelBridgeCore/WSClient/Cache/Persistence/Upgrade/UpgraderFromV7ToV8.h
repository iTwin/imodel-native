/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Upgrade/UpgraderFromV7ToV8.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include "UpgraderBase.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct UpgraderFromV7ToV8 : private UpgraderBase
    {
    public:
        UpgraderFromV7ToV8(ECDbAdapter& adapter);
        BentleyStatus Upgrade();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
