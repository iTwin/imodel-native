/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPackage/RealityPackage.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>
#include <Bentley/WString.h>
#include <Geom/GeomApi.h>

#if defined (__REALITYPACKAGE_BUILD__)
#   define REALITYPACKAGE_EXPORT EXPORT_ATTRIBUTE
#else
#   define REALITYPACKAGE_EXPORT IMPORT_ATTRIBUTE
#endif

#define BEGIN_BENTLEY_REALITYPACKAGE_NAMESPACE              BEGIN_BENTLEY_NAMESPACE namespace RealityPackage {
#define END_BENTLEY_REALITYPACKAGE_NAMESPACE                } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_REALITYPACKAGE              using namespace BentleyApi::RealityPackage;


#define REALITYPACKAGE_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_REALITYPACKAGE_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_REALITYPACKAGE_NAMESPACE

#define REALITYPACKAGE_REF_COUNTED_PTR(_sname_) \
    BEGIN_BENTLEY_REALITYPACKAGE_NAMESPACE struct _sname_; DEFINE_REF_COUNTED_PTR(_sname_) END_BENTLEY_REALITYPACKAGE_NAMESPACE

// Package
REALITYPACKAGE_TYPEDEFS(RealityDataPackage)
REALITYPACKAGE_REF_COUNTED_PTR(RealityDataPackage)
REALITYPACKAGE_TYPEDEFS(BoundingPolygon)
REALITYPACKAGE_REF_COUNTED_PTR(BoundingPolygon)
REALITYPACKAGE_TYPEDEFS(Uri)
REALITYPACKAGE_REF_COUNTED_PTR(Uri)

// RealiteSources
REALITYPACKAGE_TYPEDEFS(RealityDataSource)
REALITYPACKAGE_REF_COUNTED_PTR(RealityDataSource)
REALITYPACKAGE_TYPEDEFS(WmsDataSource)
REALITYPACKAGE_REF_COUNTED_PTR(WmsDataSource)
REALITYPACKAGE_TYPEDEFS(WmsMapSettings)
REALITYPACKAGE_REF_COUNTED_PTR(WmsMapSettings)
REALITYPACKAGE_TYPEDEFS(OsmDataSource)
REALITYPACKAGE_REF_COUNTED_PTR(OsmDataSource)
REALITYPACKAGE_TYPEDEFS(OsmResource)
REALITYPACKAGE_REF_COUNTED_PTR(OsmResource)
REALITYPACKAGE_TYPEDEFS(MultiBandSource)
REALITYPACKAGE_REF_COUNTED_PTR(MultiBandSource)

// RealityData
REALITYPACKAGE_TYPEDEFS(PackageRealityData)
REALITYPACKAGE_REF_COUNTED_PTR(PackageRealityData)
REALITYPACKAGE_TYPEDEFS(ImageryData)
REALITYPACKAGE_REF_COUNTED_PTR(ImageryData)
REALITYPACKAGE_TYPEDEFS(ModelData)
REALITYPACKAGE_REF_COUNTED_PTR(ModelData)
REALITYPACKAGE_TYPEDEFS(PinnedData)
REALITYPACKAGE_REF_COUNTED_PTR(PinnedData)
REALITYPACKAGE_TYPEDEFS(TerrainData)
REALITYPACKAGE_REF_COUNTED_PTR(TerrainData)
REALITYPACKAGE_TYPEDEFS(UndefinedData)
REALITYPACKAGE_REF_COUNTED_PTR(UndefinedData)

// RealitySerializers
REALITYPACKAGE_TYPEDEFS(RealityDataSerializerFactory)
REALITYPACKAGE_REF_COUNTED_PTR(RealityDataSerializerFactory)
REALITYPACKAGE_TYPEDEFS(RealityDataSerializer)
REALITYPACKAGE_REF_COUNTED_PTR(RealityDataSerializer)
REALITYPACKAGE_TYPEDEFS(RealityDataSerializerV1)
REALITYPACKAGE_REF_COUNTED_PTR(RealityDataSerializerV1)
REALITYPACKAGE_TYPEDEFS(RealityDataSerializerV2)
REALITYPACKAGE_REF_COUNTED_PTR(RealityDataSerializerV2)


BEGIN_BENTLEY_REALITYPACKAGE_NAMESPACE

//=====================================================================================
//! Status codes for RealityPackage operations
//=====================================================================================
enum class RealityPackageStatus
    {
    Success                     = SUCCESS,  // The operation was successful.
    UnsupportedVersion,                     // Version is either undefined or we can't handle it.
    XmlReadError,                           // File access error or invalid xml stream.
    PolygonParsingError,                    // Polygon must be a space delimited list of x y double values.
    InvalidDateFormat,                      // The date format is invalid. It must utc time.
    InvalidLongitudeLatitude,               // Longitude[-180, 180] Latitude [-90, 90].
    MissingSourceAttribute,                 // Data source must have an 'uri' and a 'type' attribute.
    MissingDataSource,                      // Each RealityData must have a data source.
    WriteToFileError,                       // Make sure path is valid and you have write permission.
    UnknownElementType,                     // Element type is unknown.
    //*** Add new here.
    UnknownError                = ERROR,    // The operation failed with an unspecified error.
    };

END_BENTLEY_REALITYPACKAGE_NAMESPACE