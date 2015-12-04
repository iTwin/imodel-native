/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Upgrade/UpgraderBase.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/Util/ECDbAdapter.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct UpgraderBase
    {
    protected:
        ECDbAdapter& m_adapter;

    protected:
        UpgraderBase(ECDbAdapter& adapter);
        BentleyStatus UpgradeCacheSchema(int versionMajor, int versionMinor);
        BentleyStatus ExecuteStatement(Utf8CP ecSql);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
