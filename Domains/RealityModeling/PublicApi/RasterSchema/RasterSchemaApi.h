/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RasterSchema/RasterSchemaApi.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <RasterSchema/ExportMacros.h>
#include <RasterSchema/RasterSchemaTypes.h>
#include <RasterSchema/RasterSchemaCommon.h>
#include <RasterSchema/RasterDomain.h>


//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define RASTER_SCHEMA_NAME  "Raster"
#define RASTER_SCHEMA_FILE  L"Raster.01.00.ecschema.xml"
#define RASTER_SCHEMA(className)   RASTER_SCHEMA_NAME "." className


