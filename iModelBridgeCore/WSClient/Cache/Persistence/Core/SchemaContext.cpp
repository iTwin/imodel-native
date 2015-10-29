/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Core/SchemaContext.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "SchemaContext.h"

#include <DgnClientFx/DgnClientFxCommon.h>

USING_NAMESPACE_BENTLEY_DGNCLIENTFX
USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName SchemaContext::GetCacheSchemasDir()
    {
    BeFileName path = DgnClientFxCommon::GetApplicationPaths().GetDgnPlatformAssetsDirectory(); // on iOS schemas are in DgnClientFxAssets.bundle
    path.AppendToPath(L"ECSchemas");
    path.AppendToPath(L"WSClient");
    path.AppendToPath(L"Cache");
    return path;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaReadContextPtr SchemaContext::CreateReadContext()
    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->AddSchemaPath(GetCacheSchemasDir());
    return context;
    }