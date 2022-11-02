/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeometryCache::Populate(DgnDbR db, BeJsValue out, BeJsConst input, ICancellableR cancel)
    {
    T_HOST.GetBRepGeometryAdmin()._EditGeometryCachePopulate(db, out, input, cancel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ElementGeometryCache::Operation(DgnDbR db, Napi::Object const& input, Napi::Env env)
    {
    return T_HOST.GetBRepGeometryAdmin()._EditGeometryCacheOperation(db, input, env);
    }
