/*--------------------------------------------------------------------------------------+
|
|     $Source: Formats/PublicAPI/Formats.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__

#pragma once

#include <TerrainModel/TerrainModel.h>
#include <GeoCoord/BaseGeoCoord.h>
#include <Bentley/WString.h>

#if defined (CREATE_STATIC_LIBRARIES) || defined (TERRAINMODEL_STATICLIB)
  #define BENTLEYDTMFORMATS_EXPORT
#elif defined (__BENTLEYTMFORMATS_BUILD__)
  #define BENTLEYDTMFORMATS_EXPORT EXPORT_ATTRIBUTE
#else
  #define BENTLEYDTMFORMATS_EXPORT IMPORT_ATTRIBUTE
#endif

TERRAINMODEL_TYPEDEFS (TerrainImporter)
TERRAINMODEL_TYPEDEFS (TerrainExporter)
ADD_BENTLEY_TYPEDEFS (Bentley::TerrainModel, TerrainImporter);
ADD_BENTLEY_TYPEDEFS (Bentley::TerrainModel, TerrainExporter);

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

typedef RefCountedPtr<TerrainImporter> TerrainImporterPtr;
typedef RefCountedPtr<TerrainExporter> TerrainExporterPtr;

/* units */
// These numbers match the ones in the GCS units. But there is no enum for it.
enum class FileUnit
    {

    Unknown = -1,

    // Metric units
    Millimeter = 30, Centimeter = 10, Meter = 0, Kilometer = 11,

    // Imperial units
    Inch = 2, Foot = 3, USSurveyFoot = 1, Mile = 22,

    // Other
//    Degree = 58,
    Custom = -2,
    };

END_BENTLEY_TERRAINMODEL_NAMESPACE
