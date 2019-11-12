/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <Geom/GeomApi.h>
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

DEFINE_REF_COUNTED_PTR(AlignmentPair)
DEFINE_REF_COUNTED_PTR(AlignmentPairEditor)

END_BENTLEY_CIVILGEOMETRY_NAMESPACE
