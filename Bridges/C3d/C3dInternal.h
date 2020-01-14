/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/DgnCoreAPI.h>
#include <DgnPlatform/GenericDomain.h>

#include <LinearReferencing/LinearReferencingApi.h>
#include <RoadRailPhysical/RoadRailPhysicalApi.h>

#include <ECPresentation/RulesDriven/RuleSetEmbedder.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

#include <Dwg/DwgImporter.h>
#include <Dwg/DwgHelper.h>
#include <Dwg/DwgBridge.h>

#include <Teigha/Civil/DbEntity/AECCDbAlignment.h>
#include <Teigha/Civil/DbEntity/AECCDbVAlignment.h>
#include <Teigha/Civil/DbEntity/AECCDbCorridor.h>
#include <Teigha/Civil/DbEntity/AECCDbFeatureLine.h>
#include <Teigha/Civil/DbObject/AECCDbRoadwayStyleSet.h>
#include <Teigha/Civil/DbObject/AECCDbRoadwayLinkStyle.h>
#include <Teigha/Civil/DbObject/AECCDbRoadwayShapeStyle.h>
#include <Teigha/Civil/DbObject/AECCDbFeatureLineStyle.h>
#include <Teigha/Civil/DbLabeling/AECCDbAlignmentStationLabeling.h>
#include <Teigha/Civil/DbLabeling/AECCDbAlignmentMinorStationLabeling.h>
#include <Teigha/Civil/DbLabeling/AECCDbVAlignmentStationLabeling.h>
#include <Teigha/Civil/DbLabeling/AECCDbVAlignmentMinorStationLabeling.h>
#include <Teigha/Civil/DbLabeling/AECCDbAlignmentGeomPointLabeling.h>

#include <Teigha/Drawing/Include/GeometryFromProxy.h>

#include <RoadRailAlignment/RoadRailAlignmentApi.h>

#include "C3dImporter.h"
#include "C3dHelper.h"

#define LOG (*NativeLogging::LoggingManager::GetLogger(L"C3dImporter"))

#include "C3dProtocolExtensions.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG
USING_NAMESPACE_BENTLEY_LINEARREFERENCING
USING_NAMESPACE_BENTLEY_ROADRAILALIGNMENT

