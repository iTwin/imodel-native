/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/ORDBridgeInternal.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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

#define ORD_SCHEMA_NAME                              "DgnV8OpenRoadsDesigner"
#define ORD_SCHEMA_FILE                              L"DgnV8OpenRoadsDesigner.ecschema.xml"
#define ORD_SCHEMA_LOCATION                          L"ECSchemas/Application/"

#define ORD_CLASS_CorridorSurfaceAspect              "CorridorSurfaceAspect"
#define ORD_CLASS_FeatureAspect                      "FeatureAspect"

#define ORD_PROP_CorridorSurfaceAspect_IsBottomMesh  "IsBottomMesh"
#define ORD_PROP_CorridorSurfaceAspect_IsTopMesh     "IsTopMesh"
#define ORD_PROP_FeatureAspect_DefinitionName        "DefinitionName"
#define ORD_PROP_FeatureAspect_Name                  "Name"

#endif