/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <TerrainModel/TerrainModel.h>

#include <ScalableTerrainModel/Import/Definitions.h>

BEGIN_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE
struct Log;
END_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE

BEGIN_BENTLEY_MRDTM_MEMORY_NAMESPACE
struct MemoryAllocator;
END_BENTLEY_MRDTM_MEMORY_NAMESPACE

BEGIN_BENTLEY_MRDTM_GEOCOORDINATES_NAMESPACE
struct GCSFactory;
struct ReprojectionFactory;
END_BENTLEY_MRDTM_GEOCOORDINATES_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE
struct SourceFactory;
struct ImporterFactory;
END_BENTLEY_MRDTM_IMPORT_NAMESPACE



BEGIN_BENTLEY_MRDTM_NAMESPACE


BENTLEYSTM_EXPORT Foundations::Log&                       GetLog                                 ();

BENTLEYSTM_EXPORT const Memory::MemoryAllocator&          GetMemoryAllocator                     ();

BENTLEYSTM_EXPORT const GeoCoords::GCSFactory&            GetGCSFactory                          ();

BENTLEYSTM_EXPORT const GeoCoords::ReprojectionFactory&   GetReprojectionFactory                 ();


BENTLEYSTM_EXPORT const Import::SourceFactory&            GetSourceFactory                       ();

const Import::ImporterFactory&                      GetImporterFactory                     ();

END_BENTLEY_MRDTM_NAMESPACE