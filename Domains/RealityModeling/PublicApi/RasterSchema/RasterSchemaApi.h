/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RasterSchema/RasterSchemaApi.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <BentleyApi/BentleyApi.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <RasterSchema/ExportMacros.h>
#include <RasterSchema/RasterSchemaTypes.h>
#include <RasterSchema/RasterSchemaCommon.h>
#include <RasterSchema/RasterDomain.h>


//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define BENTLEY_RASTER_SCHEMA_NAME                          "BentleyRaster"
#define BENTLEY_RASTER_SCHEMA_PATH                          L"ECSchemas/Domain/BentleyRaster.01.00.ecschema.xml"
#define RASTER_SCHEMA(className)                            BENTLEY_RASTER_SCHEMA_NAME "." className


