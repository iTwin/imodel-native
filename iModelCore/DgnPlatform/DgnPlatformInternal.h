/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnPlatformInternal.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef __DGNCOREINTERNAL_H__
#define __DGNCOREINTERNAL_H__

#include <Bentley/Bentley.h>
#include <Bentley/BentleyPortable.h>
#include <Bentley/BeThread.h>
#include <Bentley/BeNumerical.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeFileName.h>
#include <Bentley/BeFile.h>
#include <Bentley/CatchNonPortable.h>
#include <Bentley/BeFileListIterator.h>
#include <DgnPlatform/DgnPlatform.h>
#include <Bentley/BeAssert.h>
#include <Bentley/BeDebugLog.h>
#include <Vu/VuApi.h>
#include <Mtg/GpaApi.h>
#include <Geom/GeoPoint.h>
#include <iterator>
#include <map>
#include <valarray>
#include <set>
#include <sys/stat.h>
#include <math.h>
#include <DgnPlatform/Tools/BitMask.h>
#include <DgnPlatform/VecMath.h>
#include <DgnPlatform/ECUtils.h>
#include <DgnPlatform/DgnCore/DgnCoreAPI.h>
#include <DgnPlatform/DgnCore/rtypes.r.h>
#include <DgnPlatform/DgnCore/QvElemSet.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatformInternal/DgnCore/DgnCoreL10N.h>
#include <DgnPlatform/DgnCore/NullContext.h>
#include <DgnPlatform/DgnCore/SimplifyViewDrawGeom.h>
#include <DgnPlatform/DgnCore/ClipPrimitive.h>
#include <DgnPlatform/DgnCore/ClipVector.h>
#include <DgnPlatform/DgnCore/SectionClip.h>
#include <DgnPlatform/DgnCore/DgnCoreEvent.h>
#include <DgnPlatform/DgnCore/GPArray.h>
#include <DgnPlatform/DgnCore/DgnRangeTree.h>
#include <DgnPlatform/DgnCore/ITiledRaster.h>
#include <DgnPlatform/DgnCore/MeasureGeom.h>
#include <DgnPlatform/DgnCore/DgnDbTables.h>
#include <DgnPlatform/DgnCore/DgnIModel.h>
#include <DgnPlatform/DgnCore/RealityDataCache.h>
#include <DgnPlatform/DgnCore/WebMercator.h>
#include <DgnPlatform/DgnCore/ElementGeometry.h>
#include <DgnPlatform/MSSmartPtr.h>
#include <Logging/bentleylogging.h>
#include <Bentley/BeStringUtilities.h>
#include <BeXml/BeXml.h>
#include <Bentley/BeThreadLocalStorage.h>
#include <DgnPlatform/DgnCore/DgnProgressMeter.h>
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnCore/TransformClipStack.h>
#include "DgnCore/JsonUtils.h"
#include <DgnPlatform/DgnHandlers/DgnHandlersAPI.h>
#include <DgnPlatform/DgnHandlers/RegionUtil.h>
#include <Mtg/MtgApi.h>
#include <Regions/regionsAPI.h>
#include <Regions/rimsbsAPI.h>
#include <DgnPlatform/DgnHandlers/IEditActionSource.h>
#include <Logging/bentleylogging.h>
#include <DgnPlatform/Tools/ostime.fdf>
#include <DgnPlatform/Tools/stringop.h>
#include <DgnPlatformInternal/DgnCore/ElemRangeCalc.h>
#include <DgnPlatform/DgnCore/IGeoCoordServices.h>
#include <DgnPlatform/DgnCore/DgnMaterial.h>
#include <DgnPlatform/DgnCore/RenderMaterial.h>
#include <DgnPlatform/DgnCore/DgnLight.h>
#include <DgnPlatform/DgnCore/DgnCategory.h>
#include <DgnPlatform/DgnCore/DgnTexture.h>
#include <DgnPlatform/DgnCore/DgnTrueColor.h>
// #include <DgnPlatform/DgnCore/SolarUtility.h>

#include "DgnCore/DgnCoreLog.h"

#define ___DGNPLATFORM_SERIALIZED___ BeSystemMutexHolder ___holdBeSystemMutexInScope___

#define ASSERT_CORRECT_HOST(t)  BeAssert(((t) == DgnPlatformLib::QueryHost()) && "This object cannot be used on this thread")

#define unslong_to_double(ul)  (double)(ul)

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
USING_NAMESPACE_BENTLEY_DGNPLATFORM
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

#endif // #ifndef __DGNCOREINTERNAL_H__
