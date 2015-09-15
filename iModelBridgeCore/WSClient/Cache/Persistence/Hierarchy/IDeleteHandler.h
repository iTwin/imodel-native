/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Hierarchy/IDeleteHandler.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <ECDb/ECDbApi.h>
#include <WebServices/Cache/Util/ECDbAdapter.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDeleteHandler
    {
    virtual ~IDeleteHandler()
        {};
    virtual BentleyStatus OnBeforeDelete(ECClassCR ecClass, ECInstanceId ecInstanceId) = 0;
    virtual BentleyStatus OnAfterDelete(bset<ECInstanceKey>& instancesToDeleteOut) = 0;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
