/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#ifndef __DGNCOREINTERNAL_H__
#define __DGNCOREINTERNAL_H__

#ifdef _MSC_VER
// https://studiofreya.com/2018/01/06/visual-studio-2017-with-cpp17-and-boost/#stdbyte-ambiguous-symbol-and-rpcndr.h
#define _HAS_STD_BYTE 0
#endif
#include <Bentley/Bentley.h>
#include <Bentley/BentleyPortable.h>
#include <Bentley/BeThread.h>
#include <Bentley/BeNumerical.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeFileName.h>
#include <Bentley/CatchNonPortable.h>
#include <DgnPlatform/DgnPlatform.h>
#include <Bentley/BeAssert.h>
#include <Bentley/BeDebugLog.h>
#include <Geom/GeoPoint.h>
#include <PlacementOnEarth/Placement.h>
#include <valarray>
#include <math.h>
#include "DgnCore/DgnCoreLog.h"
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeThreadLocalStorage.h>
#include <BeSQLite/IModelDb.h>
#include <DgnPlatform/AutoRestore.h>
#include <DgnPlatform/ClipPrimitive.h>
#include <DgnPlatform/ClipVector.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/DgnCoreAPI.h>
#include <DgnPlatform/DgnDbTables.h>
#include <DgnPlatform/Lighting.h>
#include <DgnPlatform/DgnMaterial.h>
#include <DgnPlatform/PlatformLib.h>
#include <DgnPlatform/RangeIndex.h>
#include <DgnPlatform/DgnTexture.h>
#include <DgnPlatform/ColorBook.h>
#include <DgnPlatform/ViewDefinition.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ElementGeometryCache.h>
#include <DgnPlatform/ElementGraphics.h>
#include <DgnPlatform/GenericDomain.h>
#include <DgnPlatform/FontManager.h>
#include <DgnPlatform/IGeoCoordServices.h>
#include <GeomJsonWireFormat/JsonUtils.h>
#include <DgnPlatform/FenceContext.h>
#include <DgnPlatform/MeasureGeom.h>
#include <DgnPlatform/NullContext.h>
#include <DgnPlatform/RenderMaterial.h>
#include <DgnPlatform/SimplifyGraphic.h>
#include <DgnPlatform/VecMath.h>
#include <DgnPlatform/WebMercator.h>
#include <DgnPlatform/EntityIdsChangeGroup.h>
#include <ECObjects/ECSchema.h>
#include <Units/Units.h>
#include <Bentley/Logging.h>
#include <Napi/napi.h>

#define DGNCORE_RUNONCE_CHECK(VAR) {if (VAR) return; VAR=true;}
#define DGNCORELOG NativeLogging::CategoryLogger("DgnPlatform")

#if defined (ANDROID)
    // show trace messages for DEBUG builds only
    #define DGNPLATFORM_TRACE(msg) BeDebugLog(msg)
#else
    // don't show trace messages for any build
    #define DGNPLATFORM_TRACE(msg)
#endif

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_EC
using namespace std;

extern double const fc_hugeVal;

#if defined (NDEBUG) && !defined (BENTLEY_WINRT)
    #include "DgnCore/DgnCoreDLLInlines.h"     // only included here if were're NOT debugging
#endif

#undef GetMessage

#if !defined (NDEBUG)
    #define DEBUG_LOGGING 1
#endif

#if defined (DEBUG_LOGGING)
#   define DEBUG_PRINTF(...) THREADLOG.debugv(__VA_ARGS__)
#   define WARN_PRINTF(...) THREADLOG.warningv(__VA_ARGS__)
#   define ERROR_PRINTF(...) THREADLOG.errorv(__VA_ARGS__)
#else
#   define DEBUG_PRINTF(...)
#   define WARN_PRINTF(...)
#   define ERROR_PRINTF(...)
#endif

#endif // #ifndef __DGNCOREINTERNAL_H__
