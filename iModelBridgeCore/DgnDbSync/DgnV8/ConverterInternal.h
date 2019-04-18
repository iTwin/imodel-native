/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#define NULL 0

//  The Vancouver header files will not compile unless the there is a "using namespace Bentley".  Therefore we
//  have to disallow "using namespace BentleyM0200".
#define NO_USING_NAMESPACE_BENTLEY 1

#ifndef NDEBUG
    #define TEST_EXTERNAL_SOURCE_ASPECT
#endif

#include <DgnDbSync/DgnV8/DgnV8.h> // NB: Must include this first!

#include <VersionedDgnV8Api/DgnPlatform/DgnPlatform.h>
#include <VersionedDgnV8Api/DgnPlatform/IAnnotationHandler.h> // NB! Must be included before CellHeaderHandler.h!
#include <VersionedDgnV8Api/DgnPlatform/CellHeaderHandler.h>
#include <VersionedDgnV8Api/DgnPlatform/DgnAttachment.h>
#include <VersionedDgnV8Api/DgnPlatform/DgnPlatformLib.h>
#include <VersionedDgnV8Api/DgnPlatform/DisplayAttribute.h>
#include <VersionedDgnV8Api/DgnPlatform/DgnDocumentManager.h>
#include <VersionedDgnV8Api/DgnPlatform/DgnECManager.h>
#include <VersionedDgnV8Api/DgnPlatform/DgnECInstance.h>
#include <VersionedDgnV8Api/DgnPlatform/ElementHandle.h>
#include <VersionedDgnV8Api/DgnPlatform/ElementGeometry.h>
#include <VersionedDgnV8Api/DgnPlatform/ElementGraphics.h>
#include <VersionedDgnV8Api/DgnPlatform/ElementProperties.h>
#include <VersionedDgnV8Api/DgnPlatform/ElementUtil.h>
#include <VersionedDgnV8Api/DgnPlatform/LevelTypes.h>
#include <VersionedDgnV8Api/DgnPlatform/LevelCache.h>
#include <VersionedDgnV8Api/DgnPlatform/LsLocal.h>
#include <VersionedDgnV8Api/DgnPlatform/LineStyle.h>
#include <VersionedDgnV8Api/DgnPlatform/LineStyleApi.h>
#include <VersionedDgnV8Api/DgnPlatform/NamedGroup.h>
#include <VersionedDgnV8Api/DgnPlatform/PropertyProcessors.h>
#include <VersionedDgnV8Api/DgnPlatform/SharedCellHandler.h>
#include <VersionedDgnV8Api/DgnPlatform/XGraphics.h>
#include <VersionedDgnV8Api/DgnPlatform/Tools/MdlCnv.h>
#include <VersionedDgnV8Api/DgnPlatform/DgnPlatform.h>
#include <VersionedDgnV8Api/DgnPlatform/DimensionStyle.h>
#include <VersionedDgnV8Api/DgnPlatform/DimensionHandler.h>
#include <VersionedDgnV8Api/DgnPlatform/ComplexHeaderHandler.h>
#include <VersionedDgnV8Api/DgnPlatform/ViewInfo.h>
#include <VersionedDgnV8Api/DgnPlatform/ViewGroup.h>
#include <VersionedDgnV8Api/DgnGeoCoord/DgnGeoCoord.h>
#include <VersionedDgnV8Api/DgnGeoCoord/DgnGeoCoordApi.h>
#include <VersionedDgnV8Api/Bentley/BeTextFile.h>
#include <VersionedDgnV8Api/DgnPlatform/Light.h>
#include <VersionedDgnV8Api/DgnPlatform/LightElementHandlers.h>
#include <VersionedDgnV8Api/DgnPlatform/DgnHandlersAPI.h>
#include <VersionedDgnV8Api/RasterCore/DgnRaster.h>
#include <VersionedDgnV8Api/RasterCore/RasterCoreAdmin.h>
#include <VersionedDgnV8Api/RasterCore/RasterCoreLib.h>
#include <VersionedDgnV8Api/Rastercore/msrastercore.h>
#include <VersionedDgnV8Api/DgnPlatform/ClipUtil.h>
#include <VersionedDgnV8Api/DgnPlatform/DgnPlatform.r.h>
#include <VersionedDgnV8Api/DgnPlatform/CIF/PersistentAppIDs.h>
#include <VersionedDgnV8Api/DgnPlatform/Provenance.h>

#include <iModelBridge/iModelBridgeErrorHandling.h>

#define NO_USING_NAMESPACE_BENTLEY 1

#include <Bentley/bmap.h>
#include <Bentley/bvector.h>
#include <Bentley/bset.h>

#include <DgnDbSync/DgnDbSync.h>
#include <Bentley/Bentley.h>
#include <Bentley/NonCopyableClass.h>

#include <DgnDbSync/DgnV8/Converter.h>
#include <DgnDbSync/DgnV8/LightWeightConverter.h>
#include <DgnDbSync/DgnV8/SyncInfo.h>

#include <Bentley/BeNumerical.h>
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/BeFileListIterator.h>

#include <Logging/bentleylogging.h>

#include <BeSQLite/BeSQLite.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/LsLocal.h>
#include <DgnPlatform/LineStyle.h>
#include <DgnPlatform/DimensionStyle.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/RenderMaterial.h>
#include <DgnPlatform/DgnMaterial.h>
#include <DgnPlatform/GenericDomain.h>
#include <DgnPlatform/FunctionalDomain.h>
#include <DgnPlatform/ModelSpatialClassifier.h>
#include <Raster/RasterApi.h>
#include <PointCloud/PointCloudApi.h>
#include <ThreeMx/ThreeMxApi.h>
#include <ScalableMeshSchema/ScalableMeshSchemaApi.h>
#include "ECSchemaMappings.h"
#include "LinkConverter.h"
#include <DgnPlatform/TileWriter.h>
#include <DgnPlatform/DgnProgressMeter.h>

#define LOG             ConverterLogging::GetLogger (ConverterLogging::Namespace::General)
#define LOG_IS_SEVERITY_ENABLED(sev) ConverterLogging::IsSeverityEnabled (ConverterLogging::Namespace::General,sev)

#define LOG_LEVEL       ConverterLogging::GetLogger (ConverterLogging::Namespace::Level)
#define LOG_LEVEL_IS_SEVERITY_ENABLED(sev) ConverterLogging::IsSeverityEnabled (ConverterLogging::Namespace::Level,sev)

#define LOG_LEVEL_MASK  ConverterLogging::GetLogger (ConverterLogging::Namespace::LevelMask)
#define LOG_LEVEL_MASK_IS_SEVERITY_ENABLED(sev) ConverterLogging::IsSeverityEnabled (ConverterLogging::Namespace::LevelMask,sev)

#define LOG_MODEL       ConverterLogging::GetLogger (ConverterLogging::Namespace::Model)
#define LOG_MODEL_IS_SEVERITY_ENABLED(sev) ConverterLogging::IsSeverityEnabled (ConverterLogging::Namespace::Model,sev)

using namespace BentleyApi::Dgn;
using namespace BentleyApi::BeSQLite;
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_DGNDBSYNC
USING_NAMESPACE_DGNDBSYNC_DGNV8
namespace BECN = BentleyApi::ECN;

    
BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

struct XDomainRegistry
    {
    static bvector<XDomain*> s_xdomains;
    };

END_DGNDBSYNC_DGNV8_NAMESPACE
