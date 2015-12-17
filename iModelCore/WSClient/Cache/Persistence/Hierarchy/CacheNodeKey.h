/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Hierarchy/CacheNodeKey.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/Util/IECDbAdapter.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct CacheNodeKey : public ECInstanceKey
    {
    CacheNodeKey() : ECInstanceKey() {}
    CacheNodeKey(ECClassId ecClassId, ECInstanceId const& ecInstanceId) : ECInstanceKey(ecClassId, ecInstanceId) {}
    };

typedef const CacheNodeKey& CacheNodeKeyCR;
typedef CacheNodeKey& CacheNodeKeyR;

END_BENTLEY_WEBSERVICES_NAMESPACE
