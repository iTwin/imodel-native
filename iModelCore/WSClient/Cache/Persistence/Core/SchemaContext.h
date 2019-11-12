/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ECDb/ECDbApi.h>
#include <ECObjects/ECObjects.h>

#include <WebServices/Cache/WebServicesCache.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct SchemaContext
    {
    public:
        static BeFileName GetCacheSchemasDir();
        static BeFileName GetSupportV3ConversionDir();
        static BeFileName GetSupportMappingDir();
        static BeFileName GetECDbSchemasDir();
        static ECSchemaReadContextPtr CreateReadContext();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
