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
#include <RasterSchema/RasterSchemaCommon.h>

//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define BENTLEY_RASTER_SCHEMA_NAME                          "BentleyRaster"
#define BENTLEY_RASTER_SCHEMA_PATH                          L"ECSchemas/Domain/BentleyRaster.01.00.ecschema.xml"
#define RASTER_SCHEMA(className)                            BENTLEY_RASTER_SCHEMA_NAME "." className

//-----------------------------------------------------------------------------------------
// Define standard typedefs (P, CP, R, CR) and RefCountedPtr
//-----------------------------------------------------------------------------------------
RASTERSCHEMA_TYPEDEFS(RasterQuadTree)
RASTERSCHEMA_REF_COUNTED_PTR(RasterQuadTree)

RASTERSCHEMA_TYPEDEFS(RasterFile)
RASTERSCHEMA_REF_COUNTED_PTR(RasterFile)

//-----------------------------------------------------------------------------------------
// Include from PublicApi
//-----------------------------------------------------------------------------------------
#include "RasterSchemaCommon.h"
#include "ExportMacros.h"
#include "RasterDomain.h"
