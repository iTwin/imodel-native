/*--------------------------------------------------------------------------------------+
|
|     $Source: CivilBaseGeometry/PublicAPI/CivilBaseGeometry/CivilBaseGeometry.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <Geom/GeomApi.h>

// This Dll is not expected to contain any other dependencies other than GeomLibs and the BentleyApi

#ifdef __CIVILBASEGEOMETRY_BUILD__
#define CIVILBASEGEOMETRY_EXPORT EXPORT_ATTRIBUTE
#else
#define CIVILBASEGEOMETRY_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace BentleyApi::CivilBaseGeometry %CivilBaseGeometry data types */
#define BEGIN_BENTLEY_CIVILBASEGEOMETRY_NAMESPACE   BEGIN_BENTLEY_NAMESPACE namespace CivilBaseGeometry {
#define END_BENTLEY_CIVILBASEGEOMETRY_NAMESPACE     } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_CIVILBASEGEOMETRY   using namespace BENTLEY_NAMESPACE_NAME::CivilBaseGeometry;

// create the Bentley.CivilBaseGeometry namespace
BEGIN_BENTLEY_CIVILBASEGEOMETRY_NAMESPACE
END_BENTLEY_CIVILBASEGEOMETRY_NAMESPACE
