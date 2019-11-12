/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "SchemaContext.h"
#include <WebServices/Cache/CachingDataSource.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES


/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName SchemaContext::GetCacheSchemasDir()
    {
    BeFileName path = CachingDataSource::GetHostAssetsDirectory ();
    path.AppendToPath(L"ECSchemas");
    path.AppendToPath(L"WSClient");
    path.AppendToPath(L"Cache");
    return path;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName SchemaContext::GetSupportV3ConversionDir()
    {
    BeFileName path = GetCacheSchemasDir();
    path.AppendToPath(L"Support");
    path.AppendToPath(L"V3Conversion");
    return path;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName SchemaContext::GetSupportMappingDir()
    {
    BeFileName path = GetCacheSchemasDir();
    path.AppendToPath(L"Support");
    path.AppendToPath(L"Mapping");
    return path;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName SchemaContext::GetECDbSchemasDir()
    {
    BeFileName path = CachingDataSource::GetHostAssetsDirectory ();
    path.AppendToPath(L"ECSchemas");
    path.AppendToPath(L"ECDb");
    return path;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaReadContextPtr SchemaContext::CreateReadContext()
    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->AddSchemaPath(GetCacheSchemasDir());
    context->AddSchemaPath(GetECDbSchemasDir());
    context->AddConversionSchemaPath(GetSupportV3ConversionDir());
    return context;
    }
