/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Instances/NavigationBaseManager.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <ECDb/ECDbApi.h>
#include <WebServices/Cache/Util/ECDbAdapter.h>
#include <WebServices/Cache/Util/ECSqlStatementCache.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    01/2015
* NavigationBase object represents empty ObjectId(). It is used to assign navigation
* hierarchy to specific cache root.
+---------------+---------------+---------------+---------------+---------------+------*/
struct NavigationBaseManager
    {
    private:
        ECDbAdapter* m_dbAdapter;
        ECSqlStatementCache* m_statementCache;

        ECClassCP m_navigationBaseClass;

    public:
        NavigationBaseManager
            (
            ECDbAdapterR dbAdapter,
            ECSqlStatementCache& statementCache
            );

        ECInstanceKey FindNavigationBase();
        ECInstanceKey FindOrCreateNavigationBase();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
