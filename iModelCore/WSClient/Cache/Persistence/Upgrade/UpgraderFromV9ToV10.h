/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Upgrade/UpgraderFromV9ToV10.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include "UpgraderBase.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct UpgraderFromV9ToV10 : private UpgraderBase
    {
    private:
        struct Response
            {
            ECInstanceKey key;
            Utf8String cacheTag;
            DateTime cacheDate;
            };

    private:
        BentleyStatus RelateResponseResultToPage
            (
            ECInstanceKeyCR responseKey,
            ECInstanceKeyCR pageKey,
            ECRelationshipClassCP responseRelClass,
            ECRelationshipClassCP pageRelClass
            );

    public:
        UpgraderFromV9ToV10(ECDbAdapter& adapter);
        BentleyStatus Upgrade();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
