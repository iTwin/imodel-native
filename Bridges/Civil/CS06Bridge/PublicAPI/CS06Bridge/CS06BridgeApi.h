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

#ifdef __CS06BRIDGE_BUILD__
#define CS06BRIDGE_EXPORT EXPORT_ATTRIBUTE
#else
#define CS06BRIDGE_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace DgnDbSync Contains types defined by %Bentley Systems that are used to synchronize between DgnDb and foreign data formats. */
#define CS06BRIDGE_NAMESPACE_NAME CS06_Bridge
#define BEGIN_CS06BRIDGE_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace CS06BRIDGE_NAMESPACE_NAME {
#define END_CS06BRIDGE_NAMESPACE   } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_CS06BRIDGE using namespace BENTLEY_NAMESPACE_NAME::CS06BRIDGE_NAMESPACE_NAME;

//-----------------------------------------------------------------------------------------
// Define standard typedefs (P, CP, R, CR) in the CS06Bridge namespace
//-----------------------------------------------------------------------------------------
#define CS06BRIDGE_TYPEDEFS(_name_) \
    BEGIN_CS06BRIDGE_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_CS06BRIDGE_NAMESPACE

//-----------------------------------------------------------------------------------------
// Define RefCountedPtr and CPtr types
//-----------------------------------------------------------------------------------------
#define CS06BRIDGE_REFCOUNTED_PTR(_name_) \
    BEGIN_CS06BRIDGE_NAMESPACE struct _name_; DEFINE_REF_COUNTED_PTR(_name_) END_CS06BRIDGE_NAMESPACE

// create the BentleyApi.CS06_Bridge namespace
BEGIN_CS06BRIDGE_NAMESPACE
END_CS06BRIDGE_NAMESPACE

//-----------------------------------------------------------------------------------------
// DgnModel names
//-----------------------------------------------------------------------------------------
#define CS06BRIDGE_AlignmentModelName    "ConceptStation-Bridge Alignment Model"
#define CS06BRIDGE_PhysicalModelName     "ConceptStation-Bridge Physical Model"

//-----------------------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------------------
#include "CS06Bridge.h"

