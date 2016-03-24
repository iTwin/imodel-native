/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ThreeMxSchema/ThreeMxSchemaApi.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatformApi.h>

#define BEGIN_BENTLEY_THREEMX_SCHEMA_NAMESPACE       BEGIN_BENTLEY_NAMESPACE namespace ThreeMxSchema {
#define END_BENTLEY_THREEMX_SCHEMA_NAMESPACE         } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_THREEMX_SCHEMA       using namespace BentleyApi::ThreeMxSchema;

#define BENTLEY_THREEMX_SCHEMA_NAME "ThreeMx"
#define BENTLEY_THREEMX_SCHEMA_PATH L"ECSchemas/Domain/ThreeMx.01.00.ecschema.xml"
#define THREEMX_SCHEMA(className)   BENTLEY_THREEMX_SCHEMA_NAME "." className
