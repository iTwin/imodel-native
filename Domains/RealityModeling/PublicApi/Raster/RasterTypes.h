/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/Raster/RasterTypes.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <Raster/RasterCommon.h>
#include <Raster/ExportMacros.h>

//-----------------------------------------------------------------------------------------
// Define standard typedefs (P, CP, R, CR) and RefCountedPtr
//-----------------------------------------------------------------------------------------
RASTER_TYPEDEFS(RasterFile)
RASTER_REF_COUNTED_PTR(RasterFile)

RASTER_REF_COUNTED_PTR(RasterModel)

RASTER_TYPEDEFS(WmsModel)
RASTER_REF_COUNTED_PTR(WmsModel)

RASTER_TYPEDEFS(WmsSource)
RASTER_REF_COUNTED_PTR(WmsSource)

RASTER_TYPEDEFS(RasterFileModel)
RASTER_REF_COUNTED_PTR(RasterFileModel)

RASTER_TYPEDEFS(RasterFileSource)
RASTER_REF_COUNTED_PTR(RasterFileSource)

RASTER_TYPEDEFS(RasterFileTile)
RASTER_REF_COUNTED_PTR(RasterFileTile)

RASTER_TYPEDEFS(RasterClip)
RASTER_REF_COUNTED_PTR(RasterClip)

RASTER_TYPEDEFS(RasterTile)
RASTER_TYPEDEFS(RasterRoot)
RASTER_REF_COUNTED_PTR(RasterTile)
RASTER_REF_COUNTED_PTR(RasterRoot)

