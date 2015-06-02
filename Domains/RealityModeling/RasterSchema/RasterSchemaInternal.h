/*--------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterSchemaInternal.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef __RASTERSCHEMAINTERNAL_H__
#define __RASTERSCHEMAINTERNAL_H__

#include <RasterSchema/RasterSchemaApi.h>

#include <Bentley/Bentley.h>
#include <DgnPlatform/DgnCore/DgnDomain.h>
#include <DgnPlatform/DgnCore/RasterBaseModel.h>
#include <DgnPlatform/DgnGeoCoord.h>

#include <RasterSchema/RasterSchemaTypes.h>
#include <RasterSchema/RasterSchemaCommon.h>
#include <RasterSchema/ExportMacros.h>
#include <RasterSchema/RasterHandler.h>
#include <RasterSchema/RasterDomain.h>

RASTERSCHEMA_TYPEDEFS(RasterTile)
RASTERSCHEMA_REF_COUNTED_PTR(RasterTile)

RASTERSCHEMA_TYPEDEFS(RasterSource)
RASTERSCHEMA_REF_COUNTED_PTR(RasterSource)

RASTERSCHEMA_TYPEDEFS(DisplayTile)
RASTERSCHEMA_REF_COUNTED_PTR(DisplayTile)

RASTERSCHEMA_TYPEDEFS(Bitmap)
RASTERSCHEMA_REF_COUNTED_PTR(Bitmap)

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_RASTERSCHEMA

#endif // __RASTERSCHEMAINTERNAL_H__
