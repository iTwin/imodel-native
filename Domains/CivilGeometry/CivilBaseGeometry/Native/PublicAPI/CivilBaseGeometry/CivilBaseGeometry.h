/*--------------------------------------------------------------------------------------+
|
|     $Source: CivilBaseGeometry/Native/PublicAPI/CivilBaseGeometry/CivilBaseGeometry.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <Geom/GeomApi.h>
#include <DgnPlatform/DgnPlatform.r.h> // Only required for Dgn::StandardUnit
#include <DgnPlatform/DgnPlatformApi.h>
#include <GeomSerialization/GeomSerializationApi.h>

// This Dll is not expected to contain any other dependencies other than GeomLibs and the BentleyApi

#ifdef __CIVILBASEGEOMETRY_BUILD__
#define CIVILBASEGEOMETRY_EXPORT EXPORT_ATTRIBUTE
#else
#define CIVILBASEGEOMETRY_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace BentleyApi::CivilBaseGeometry %CivilBaseGeometry data types */
#define BEGIN_BENTLEY_CIVILGEOMETRY_NAMESPACE   BEGIN_BENTLEY_NAMESPACE namespace CivilGeometry {
#define END_BENTLEY_CIVILGEOMETRY_NAMESPACE     } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_CIVILGEOMETRY   using namespace BENTLEY_NAMESPACE_NAME::CivilGeometry;


// create the Bentley.CivilBaseGeometry namespace
BEGIN_BENTLEY_CIVILGEOMETRY_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(AlignmentPair)
DEFINE_POINTER_SUFFIX_TYPEDEFS(AlignmentPairEditor)
DEFINE_POINTER_SUFFIX_TYPEDEFS(AlignmentIntersection)
DEFINE_POINTER_SUFFIX_TYPEDEFS(AlignmentIntersectionInfo)

DEFINE_REF_COUNTED_PTR(AlignmentPair)
DEFINE_REF_COUNTED_PTR(AlignmentPairEditor)
DEFINE_REF_COUNTED_PTR(AlignmentIntersection)

struct AlignmentPairIntersection;
DEFINE_REF_COUNTED_PTR(AlignmentPairIntersection)

#define REPLACEMENT_LOG(...)    (void)0
#define REPLACEMENT_LOGW(...)   (void)0

END_BENTLEY_CIVILGEOMETRY_NAMESPACE
