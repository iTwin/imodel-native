#pragma once

//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <Bentley/SHA1.h>
#include <ECDb/ECDbTypes.h>
#include <ECDb/ECDbApi.h>
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnDb.h>
#include <iModelBridge/iModelBridge.h>
#include <iModelBridge/iModelBridgeSyncInfoFile.h>
#include <LinearReferencing/LinearReferencingApi.h>
#include <RoadRailAlignment/RoadRailAlignmentApi.h>
#include <RoadRailPhysical/RoadRailPhysicalApi.h>

#ifdef __ORDBRIDGE_BUILD__
#define ORDBRIDGE_EXPORT EXPORT_ATTRIBUTE
#else
#define ORDBRIDGE_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace DgnDbSync Contains types defined by %Bentley Systems that are used to synchronize between DgnDb and foreign data formats. */
#define ORDBRIDGE_NAMESPACE_NAME ORD_Bridge
#define BEGIN_ORDBRIDGE_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace ORDBRIDGE_NAMESPACE_NAME {
#define END_ORDBRIDGE_NAMESPACE   } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_ORDBRIDGE using namespace BENTLEY_NAMESPACE_NAME::ORDBRIDGE_NAMESPACE_NAME;

// create the BentleyApi.ORD_Bridge namespace
BEGIN_ORDBRIDGE_NAMESPACE
END_ORDBRIDGE_NAMESPACE

//-----------------------------------------------------------------------------------------
// DgnModel/DgnView names
//-----------------------------------------------------------------------------------------
#define ORDBRIDGE_AlignmentModelName    "ORD-Bridge Alignment Model"
#define ORDBRIDGE_PhysicalModelName     "ORD-Bridge Physical Model"
#define ORDBRIDGE_3dViewName            "ORD-Bridge View-3d"
#define ORDBRIDGE_2dViewName            "ORD-Bridge View-2d"

//-----------------------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------------------
#include "ORDBridge.h"

