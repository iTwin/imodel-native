/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

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
    explicit CacheNodeKey(ECInstanceKeyCR key) : CacheNodeKey(key.GetClassId(), key.GetInstanceId()) {}
    };

typedef const CacheNodeKey& CacheNodeKeyCR;
typedef CacheNodeKey& CacheNodeKeyR;

END_BENTLEY_WEBSERVICES_NAMESPACE
