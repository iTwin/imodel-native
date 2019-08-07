/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/ORDBridgeInternal.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#ifndef __ORDBRIDGEINTERNAL_H__
#define __ORDBRIDGEINTERNAL_H__

//  The Vancouver header files will not compile unless the there is a "using namespace Bentley".  Therefore we
//  have to disallow "using namespace BentleyB0200".
#define NO_USING_NAMESPACE_BENTLEY 1

#include <DgnDbSync/DgnV8/DgnV8.h>
#include <Bentley/SHA1.h>
#include <ECDb/ECDbTypes.h>
#include <ECDb/ECDbApi.h>
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnDbSync/DgnDbSync.h>
#include <DgnDbSync/DgnV8/Converter.h>
#include <iModelBridge/iModelBridgeSyncInfoFile.h>
#include <LinearReferencing/LinearReferencingApi.h>
#include <RoadRailAlignment/RoadRailAlignmentApi.h>
#include <RoadRailPhysical/RoadRailPhysicalApi.h>
#include <DgnV8OpenRoadsDesigner/DgnV8OpenRoadsDesignerApi.h>
#include <CifApi/Cif/SDK/Bentley.Cif.SDK.h>
#include <CifApi/Cif/SDK/ConsensusConnection.h>
#include <CifApi/Cif/SDK/CIFGeometryModelSDK.h>
#include <CifApi/Cif/SDK/GeometryModelDgnECDataBinder.h>
#include <CifApi/Cif/SDK/ConsensusDgnECProvider.h>
#include <CifApi/Cif/SDK/CIFGeometryModelECSchema.h>
#include <CifApi/Cif/SDK/PersistentPathBackPointerBuilder.h>
#include <ORDBridge/ORDBridgeApi.h>

/** @namespace DgnDbSync Contains types defined by %Bentley Systems that are used to synchronize between DgnDb and foreign data formats. */
#define ORDBRIDGE_NAMESPACE_NAME ORD_Bridge
#define BEGIN_ORDBRIDGE_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace ORDBRIDGE_NAMESPACE_NAME {
#define END_ORDBRIDGE_NAMESPACE   } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_ORDBRIDGE using namespace BENTLEY_NAMESPACE_NAME::ORDBRIDGE_NAMESPACE_NAME;

// create the BentleyApi.ORD_Bridge namespace
BEGIN_ORDBRIDGE_NAMESPACE
END_ORDBRIDGE_NAMESPACE

#include "ORDConverter.h"
#include "ORDBridge.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_LINEARREFERENCING
DGNV8_USING_NAMESPACE_BENTLEY_CIF_SDK
DGNV8_USING_NAMESPACE_BENTLEY_CIF_GEOMETRYMODEL_SDK

namespace AlignmentBim = BENTLEY_NAMESPACE_NAME::RoadRailAlignment;
namespace RoadRailBim = BENTLEY_NAMESPACE_NAME::RoadRailPhysical;
namespace DgnV8ORDBim = BENTLEY_NAMESPACE_NAME::DgnV8OpenRoadsDesigner;

//-----------------------------------------------------------------------------------------
// Logging macros
//-----------------------------------------------------------------------------------------
#define LOGGER_NAMESPACE_ORDBRIDGE    "ORDBridge"
#if defined (ANDROID)
#include <android/log.h>
#define ORDBRIDGE_LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOGGER_NAMESPACE_ORDBRIDGE, __VA_ARGS__);
#define ORDBRIDGE_LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOGGER_NAMESPACE_ORDBRIDGE, __VA_ARGS__);
#define ORDBRIDGE_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOGGER_NAMESPACE_ORDBRIDGE, __VA_ARGS__);
#else
#include <Logging/BentleyLogging.h>
#define ORDBRIDGE_LOG                 (*NativeLogging::LoggingManager::GetLogger (LOGGER_NAMESPACE_ORDBRIDGE))
#define ORDBRIDGE_LOGD(...)           ORDBRIDGE_LOG.debugv (__VA_ARGS__);
#define ORDBRIDGE_LOGI(...)           ORDBRIDGE_LOG.infov (__VA_ARGS__);
#define ORDBRIDGE_LOGW(...)           ORDBRIDGE_LOG.warningv (__VA_ARGS__);
#define ORDBRIDGE_LOGE(...)           ORDBRIDGE_LOG.errorv (__VA_ARGS__);
#endif

#endif