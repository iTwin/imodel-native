/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ThreeMxSchema/ThreeMxSchemaApi.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>
#include <ECDb/ECDbTypes.h>
#include <ECDb/ECDbApi.h>
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnPlatformApi.h>

#include <ThreeMxSchema/ThreeMxSchemaCommon.h>

//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define BENTLEY_THREEMX_SCHEMA_NAME                      "BentleyThreeMx"
#define BENTLEY_THREEMX_SCHEMA_PATH                      L"ECSchemas/Domain/BentleyThreeMx.01.00.ecschema.xml"
#define THREEMX_SCHEMA(className)                        BENTLEY_THREEMX_SCHEMA_NAME "." className

//-----------------------------------------------------------------------------------------
// Include from PublicApi
//-----------------------------------------------------------------------------------------
#include "ThreeMxSchemaCommon.h"
#include "ThreeMxDomain.h"

