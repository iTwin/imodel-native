/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnPlatformInternal.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef __DGNCOREINTERNAL_H__
#define __DGNCOREINTERNAL_H__

#include <BentleyApi/BentleyApi.h>
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
#include <ECObjects/ECInstanceIterable.h>
#include <DgnPlatform/VecMath.h>
#include <DgnPlatform/DgnCore/DgnCoreAPI.h>
#include <DgnPlatform/DgnCore/rtypes.r.h>
#include <DgnPlatform/DgnCore/Linkage.h>
#include <DgnPlatform/DgnCore/Linkage1.h>
#include <DgnPlatform/DgnCore/QvElemSet.h>
#include <DgnPlatform/DgnCore/DgnLinkage.h>
#include <DgnPlatform/DgnCore/DgnShxFont.h>
#include <DgnPlatform/DgnCore/DgnTrueTypeFont.h>
#include <DgnPlatformInternal/DgnCore/MaterialTokens.h>
#include <DgnPlatformInternal/DgnCore/RasterDb.h>
#include <DgnPlatform/DgnCore/LsLocal.h>
#include <DgnPlatformInternal/DgnCore/DgnCoreL10N.h>
#include <DgnPlatform/DgnCore/DisplayPriority.h>
#include <DgnPlatform/DgnCore/NullContext.h>
#include <DgnPlatform/DgnCore/BSISerializable.h>
#include <DgnPlatform/DgnCore/SimplifyViewDrawGeom.h>
#include <DgnPlatform/DgnCore/AnnotationScale.h>
#include <DgnPlatform/DgnCore/ClipPrimitive.h>
#include <DgnPlatform/DgnCore/ClipVector.h>
#include <DgnPlatform/DgnCore/SectionClip.h>
#include <DgnPlatform/DgnCore/CompressedXAttribute.h>
#include <DgnPlatform/DgnCore/DgnCoreEvent.h>
#include <DgnPlatform/DgnCore/DisplayFilter.h>
#include <DgnPlatform/DgnCore/DisplayFilterManager.h>
#include <DgnPlatform/DgnCore/EldscrFuncs.h>
#include <DgnPlatform/DgnCore/GPArray.h>
#include <DgnPlatform/DgnCore/GradientSettings.h>
#include <DgnPlatform/DgnCore/LineStyleApi.h>
#include <DgnPlatform/DgnCore/LinkageHolderElement.h>
#include <DgnPlatform/DgnCore/DgnRscFont.h>
#include <DgnPlatform/DgnCore/IDrawRasterAttachment.h>
#include <DgnPlatform/DgnCore/Undo.h>
#include <DgnPlatform/DgnCore/LineStyleApi.h>
#include <DgnPlatform/DgnCore/DgnRangeTree.h>
#include <DgnPlatform/DgnCore/ITiledRaster.h>
#include <DgnPlatform/DgnCore/MeasureGeom.h>
#include <DgnPlatform/ProxyDisplayCore.h>
#include <DgnPlatform/CVEHandler.h>
#include <DgnPlatform/DgnCore/DgnProjectTables.h>
#include <DgnPlatform/DgnCore/RealityDataCache.h>
#include <DgnPlatform/DgnCore/WebMercator.h>
#include <Mtg/MtgApi.h>
#include <RmgrTools/Tools/DataExternalizer.h>
#include <DgnPlatform/Tools/stringop.h>
#include <DgnPlatform/Tools/varichar.h>
#include <DgnPlatform/Tools/ostime.fdf>
#include <RmgrTools/Tools/toolsubsStdio.h>
#include <DgnPlatform/Tools/MultiStateMask.h>
#include <Logging/bentleylogging.h>
#include <DgnPlatform/DgnCore/MSElementDescr.h>
#include <Bentley/BeStringUtilities.h>
#include <BeXml/BeXml.h>
#include <RmgrTools/Tools/UglyStrings.h>
#include <Bentley/BeThreadLocalStorage.h>
#include <DgnPlatform/DgnCore/DgnProgressMeter.h>
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnCore/XGraphicsCache.h>
#include <DgnPlatform/DgnCore/TransformClipStack.h>
#include <DgnPlatform/MSSmartPtr.h>
#include "DgnCore/JsonUtils.h"
#include <DgnPlatform/DgnCore/ZipStream.h>
#include <DgnPlatform/DgnCore/XAttributeHandler.h>
#include <DgnPlatform/DgnHandlers/DgnHandlersAPI.h>
#include <DgnPlatform/DgnHandlers/AssocGeom.h>
#include <DgnPlatform/DgnHandlers/dgnole.h>
#include <DgnPlatform/DgnHandlers/ElementUtil.h>
#include <DgnPlatform/DgnHandlers/RegionUtil.h>
#include <DgnPlatformInternal/DgnHandlers/MultilineStyle.h>
#include <DgnPlatformInternal/DgnHandlers/ECXDataTreeSchemasLocater.h>
#include <DgnPlatform/DgnHandlers/Note.h>
#include <DgnPlatform/DgnHandlers/MdlTextInternal.h>
#include <DgnPlatformInternal/DgnHandlers/BitMaskLinkage.h>
#include <DgnPlatformInternal/DgnHandlers/Dimension.h>
#include <Mtg/MtgApi.h>
#include <Regions/regionsAPI.h>
#include <Regions/rimsbsAPI.h>
#include <DgnPlatform/DgnHandlers/TextBlock/AlongTextDependency.h>
#include <DgnPlatform/DgnHandlers/TextBlock/IDwgContext.h>
#include <DgnPlatform/DgnHandlers/TextBlock/TextBlockUtilities.h>
#include <DgnPlatform/DgnHandlers/TextBlock/TextContext.h>                    
#include <DgnPlatform/DgnHandlers/TextBlock/TextParamWideAndScale.h>
#include <DgnPlatform/DgnHandlers/XMLFragment.h>
#include <DgnPlatform/DgnHandlers/ContentAreaHandler.h>
#include <DgnPlatform/DgnHandlers/DetailingSymbol/DetailingSymbolHandlers.h>
#include <DgnPlatform/DgnHandlers/DgnStoreHandlers.h>
#include <DgnPlatform/DgnHandlers/IEditActionSource.h>
#include <DgnPlatform/DgnHandlers/RasterResolutionSolver.h>
#include <DgnPlatform/DgnHandlers/FlatteningCollection.h>
#include <DgnPlatform/DgnHandlers/thematicdisplay.h>
#include <DgnPlatform/DgnHandlers/IRenderDependency.h>
#include <DgnPlatformInternal/DgnHandlers/RenderHandlers.h>
#include <DgnPlatformInternal/DgnHandlers/MarkupPlacemarkHandler.h>
#include <Logging/bentleylogging.h>
#include "DgnHandlers/Icons.h"
#include <DgnPlatformInternal/DgnHandlers/DgnHandlersMessage.h>
#include "DgnHandlers/Dimension/DimensionInternal.h"
#include "DgnHandlers/Dimension/NoteInternal.h"
#include <RmgrTools/Tools/msavltre.h>
#include <DgnPlatform/Tools/MdlCnv.h>
#include <DgnPlatform/Tools/varichar.h>
#include <DgnPlatform/Tools/MultiStateMask.h>
#include <DgnPlatform/Tools/ostime.fdf>
#include <DgnPlatform/Tools/stringop.h>
#include <RmgrTools/Tools/toolsubsStdio.h>
#include <RmgrTools/Tools/DataExternalizer.h>
#include <RmgrTools/Tools/UglyStrings.h>
#include <DgnPlatform/DgnHandlers/DgnECTypes.h>
#include <DgnPlatform/MSSmartPtr.h>
#include <DgnPlatformInternal/DgnCore/ElemRangeCalc.h>

#define ___DGNPLATFORM_SERIALIZED___ BeSystemCriticalSectionHolder ___holdBeSystemCriticalSectionInScope___

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

#undef MASTERFILE
#undef ACTIVEMODEL

#define unslong_to_double(ul)  (double)(ul)
#define double_to_unslong(d)   (unsigned long)(d)

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
extern void DrawSheetBorder (ViewContextP);
extern void depcallback_staticInitialize ();
extern void assocRegion_staticInitialize ();
extern void mdlNote_staticInitialize ();
extern void textstyle_staticInitialize ();
extern void dimension_staticInitialize ();
extern void rasterDependency_initialize();

extern void textField_staticInitialize();

#if defined (NDEBUG) && !defined (BENTLEY_WINRT)
    #include "DgnCore/DgnCoreDLLInlines.h"     // only included here if were're NOT debugging
    #include "DgnHandlers/DgnHandlersDLLInlines.h"      // only included here if were're NOT debugging
#endif

#define SemiPrivate
#undef GetMessage

#define DIMLABEL_MASTUNIT               STRING_LINKAGE_KEY_MastUnitLabel
#define DIMLABEL_SUBUNIT                STRING_LINKAGE_KEY_SubUnitLabel
#define DIMLABEL_SECONDARY_MASTUNIT     STRING_LINKAGE_KEY_SecondaryMastUnitLabel
#define DIMLABEL_SECONDARY_SUBUNIT      STRING_LINKAGE_KEY_SecondarySubUnitLabel

#endif // #ifndef __DGNCOREINTERNAL_H__
