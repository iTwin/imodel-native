/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnPlatformInternal.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef __DGNCOREINTERNAL_H__
#define __DGNCOREINTERNAL_H__

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
#include <Mtg/GpaApi.h>
#include <Geom/GeoPoint.h>
#include <valarray>
#include <math.h>
#include "DgnCore/DgnCoreLog.h"
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeThreadLocalStorage.h>
#include <DgnPlatform/AutoRestore.h>
#include <DgnPlatform/ClipPrimitive.h>
#include <DgnPlatform/ClipVector.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/DgnCoreAPI.h>
#include <DgnPlatform/DgnCoreEvent.h>
#include <DgnPlatform/DgnDbTables.h>
#include <DgnPlatform/DgnHandlersAPI.h>
#include <DgnPlatform/DgnIModel.h>
#include <DgnPlatform/DgnLight.h>
#include <DgnPlatform/DgnMaterial.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnProgressMeter.h>
#include <DgnPlatform/DgnRangeTree.h>
#include <DgnPlatform/DgnTexture.h>
#include <DgnPlatform/DgnTrueColor.h>
#include <DgnPlatform/DgnView.h>
#include <DgnPlatform/ECUtils.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ElementGraphics.h>
#include <DgnPlatform/GPArray.h>
#include <DgnPlatform/GenericDomain.h>
#include <DgnPlatform/IGeoCoordServices.h>
#include <DgnPlatform/JsonUtils.h>
#include <DgnPlatform/MeasureGeom.h>
#include <DgnPlatform/NullContext.h>
#include <DgnPlatform/RealityDataCache.h>
#include <DgnPlatform/RegionUtil.h>
#include <DgnPlatform/RenderMaterial.h>
#include <DgnPlatform/SectionClip.h>
#include <DgnPlatform/SimplifyGraphic.h>
#include <DgnPlatform/TileTree.h>
#include <DgnPlatform/VecMath.h>
#include <DgnPlatform/WebMercator.h>
#include <DgnPlatformInternal/DgnCore/DgnCoreL10N.h>
#include <ECObjects/ECSchema.h>
#include <Logging/bentleylogging.h>

#define ___DGNPLATFORM_SERIALIZED___ BeSystemMutexHolder ___holdBeSystemMutexInScope___

#define DGNCORE_RUNONCE_CHECK(VAR) {if (VAR) return; VAR=true;}
#define DGNCORELOG NativeLogging::LoggingManager::GetLogger(L"DgnPlatform")

// Invoke this macro in your .cpp file to define a function that gets and caches a logger
#define DPILOG_DEFINE(LOGGER)\
ILogger& LOGGER ## _getLogger()\
    {\
    static ILogger* s_logger;\
    if (NULL == s_logger)\
        s_logger = LoggingManager::GetLogger (#LOGGER);\
    return *s_logger;\
    }

// Then define this macro in your .cpp file, replacing LOGGER with whatever you passed to DPLOG_DEFINE above
// #define LOG(sev,...) {if (<LOGGER>_getLogger().isSeverityEnabled(sev)) { <LOGGER>_getLogger().messagev (sev, __VA_ARGS__); }}

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

#define DIMLABEL_MASTUNIT               STRING_LINKAGE_KEY_MastUnitLabel
#define DIMLABEL_SUBUNIT                STRING_LINKAGE_KEY_SubUnitLabel
#define DIMLABEL_SECONDARY_MASTUNIT     STRING_LINKAGE_KEY_SecondaryMastUnitLabel
#define DIMLABEL_SECONDARY_SUBUNIT      STRING_LINKAGE_KEY_SecondarySubUnitLabel

#if !defined (NDEBUG)
    #define DEBUG_LOGGING 1
#endif

#if defined (DEBUG_LOGGING)
#   define DEBUG_PRINTF THREADLOG.debugv
#   define WARN_PRINTF THREADLOG.warningv
#   define ERROR_PRINTF THREADLOG.errorv
#else
#   define DEBUG_PRINTF(...)
#   define WARN_PRINTF(...)
#   define ERROR_PRINTF(...)
#endif

#endif // #ifndef __DGNCOREINTERNAL_H__
