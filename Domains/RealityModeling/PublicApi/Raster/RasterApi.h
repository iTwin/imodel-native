/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <Raster/ExportMacros.h>

//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define RASTER_SCHEMA_NAME  "Raster"
#define RASTER_SCHEMA_FILE  L"Raster.ecschema.xml"
#define RASTER_SCHEMA(className)   RASTER_SCHEMA_NAME "." className

#include <Raster/RasterTypes.h>
#include <Raster/RasterCommon.h>
#include <Raster/RasterDomain.h>

