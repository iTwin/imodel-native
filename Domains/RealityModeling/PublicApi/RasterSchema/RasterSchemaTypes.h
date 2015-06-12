/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RasterSchema/RasterSchemaTypes.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <RasterSchema/RasterSchemaCommon.h>
#include <RasterSchema/ExportMacros.h>

//-----------------------------------------------------------------------------------------
// Define standard typedefs (P, CP, R, CR) and RefCountedPtr
//-----------------------------------------------------------------------------------------
RASTERSCHEMA_TYPEDEFS(RasterQuadTree)
RASTERSCHEMA_REF_COUNTED_PTR(RasterQuadTree)

RASTERSCHEMA_TYPEDEFS(RasterFile)
RASTERSCHEMA_REF_COUNTED_PTR(RasterFile)

RASTERSCHEMA_REF_COUNTED_PTR(RasterModel)

RASTERSCHEMA_REF_COUNTED_PTR(WmsModel)
RASTERSCHEMA_REF_COUNTED_PTR(WmsSource)
RASTERSCHEMA_REF_COUNTED_PTR(RasterFileSource)
RASTERSCHEMA_REF_COUNTED_PTR(RasterFileModel)

RASTERSCHEMA_TYPEDEFS(RasterQuadTree)
RASTERSCHEMA_REF_COUNTED_PTR(RasterQuadTree)

