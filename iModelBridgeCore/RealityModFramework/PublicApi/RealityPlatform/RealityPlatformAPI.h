/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <Bentley/Bentley.h>
#include <Bentley/BeFileName.h>
#include <Bentley/bvector.h>
#include <Bentley/WString.h>
#include <Bentley/RefCounted.h>
#include <Geom/GeomApi.h>

#if defined (__REALITYPLATFORM_BUILD__)
#   define REALITYDATAPLATFORM_EXPORT EXPORT_ATTRIBUTE
#else
#   define REALITYDATAPLATFORM_EXPORT IMPORT_ATTRIBUTE
#endif

#define BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE         BEGIN_BENTLEY_NAMESPACE namespace RealityPlatform {
#define END_BENTLEY_REALITYPLATFORM_NAMESPACE           } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_REALITYPLATFORM         using namespace BentleyApi::RealityPlatform;


#define REALITYPLATFORM_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_REALITYPLATFORM_NAMESPACE

#define REALITYPLATFORM_REF_COUNTED_PTR(_sname_) \
    BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE struct _sname_; DEFINE_REF_COUNTED_PTR(_sname_) END_BENTLEY_REALITYPLATFORM_NAMESPACE

REALITYPLATFORM_TYPEDEFS(RealityDataExtract)
REALITYPLATFORM_REF_COUNTED_PTR(RealityDataExtract)

REALITYPLATFORM_TYPEDEFS(RasterData)
REALITYPLATFORM_REF_COUNTED_PTR(RasterData)

REALITYPLATFORM_TYPEDEFS(PointCloudData)
REALITYPLATFORM_REF_COUNTED_PTR(PointCloudData)

REALITYPLATFORM_TYPEDEFS(WmsData)
REALITYPLATFORM_REF_COUNTED_PTR(WmsData)

REALITYPLATFORM_TYPEDEFS(RealityDataDownload)
REALITYPLATFORM_REF_COUNTED_PTR(RealityDataDownload)

REALITYPLATFORM_TYPEDEFS(SpatialEntityDataset)
REALITYPLATFORM_REF_COUNTED_PTR(SpatialEntityDataset)

REALITYPLATFORM_TYPEDEFS(FtpClient)
REALITYPLATFORM_REF_COUNTED_PTR(FtpClient)

REALITYPLATFORM_TYPEDEFS(SpatialEntityRequest)
REALITYPLATFORM_REF_COUNTED_PTR(SpatialEntityRequest)

REALITYPLATFORM_TYPEDEFS(SpatialEntityResponse)
REALITYPLATFORM_REF_COUNTED_PTR(SpatialEntityResponse)

REALITYPLATFORM_TYPEDEFS(SpatialEntityClient)
REALITYPLATFORM_REF_COUNTED_PTR(SpatialEntityClient)

REALITYPLATFORM_TYPEDEFS(SpatialEntity)
REALITYPLATFORM_REF_COUNTED_PTR(SpatialEntity)

REALITYPLATFORM_TYPEDEFS(SpatialEntityDataSource)
REALITYPLATFORM_REF_COUNTED_PTR(SpatialEntityDataSource)

REALITYPLATFORM_TYPEDEFS(SpatialEntityThumbnail)
REALITYPLATFORM_REF_COUNTED_PTR(SpatialEntityThumbnail)

REALITYPLATFORM_TYPEDEFS(SpatialEntityMetadata)
REALITYPLATFORM_REF_COUNTED_PTR(SpatialEntityMetadata)

REALITYPLATFORM_TYPEDEFS(SpatialEntityServer)
REALITYPLATFORM_REF_COUNTED_PTR(SpatialEntityServer)

REALITYPLATFORM_TYPEDEFS(FtpRequest)
REALITYPLATFORM_REF_COUNTED_PTR(FtpRequest)

REALITYPLATFORM_TYPEDEFS(FtpResponse)
REALITYPLATFORM_REF_COUNTED_PTR(FtpResponse)

REALITYPLATFORM_TYPEDEFS(HttpClient)
REALITYPLATFORM_REF_COUNTED_PTR(HttpClient)

REALITYPLATFORM_TYPEDEFS(HttpRequest)
REALITYPLATFORM_REF_COUNTED_PTR(HttpRequest)

REALITYPLATFORM_TYPEDEFS(HttpResponse)
REALITYPLATFORM_REF_COUNTED_PTR(HttpResponse)

REALITYPLATFORM_TYPEDEFS(RealityDataRelationship)
REALITYPLATFORM_REF_COUNTED_PTR(RealityDataRelationship)

REALITYPLATFORM_TYPEDEFS(RealityDataDocument)
REALITYPLATFORM_REF_COUNTED_PTR(RealityDataDocument)

REALITYPLATFORM_TYPEDEFS(RealityDataFolder)
REALITYPLATFORM_REF_COUNTED_PTR(RealityDataFolder)

REALITYPLATFORM_TYPEDEFS(RealityDataBase)
REALITYPLATFORM_REF_COUNTED_PTR(RealityDataBase)

REALITYPLATFORM_TYPEDEFS(RealityData)
REALITYPLATFORM_REF_COUNTED_PTR(RealityData)

REALITYPLATFORM_TYPEDEFS(RealityDataLocation)
REALITYPLATFORM_REF_COUNTED_PTR(RealityDataLocation)

REALITYPLATFORM_TYPEDEFS(RealityDataEnterpriseStat)
REALITYPLATFORM_REF_COUNTED_PTR(RealityDataEnterpriseStat)

REALITYPLATFORM_TYPEDEFS(RealityDataServiceStat)
REALITYPLATFORM_REF_COUNTED_PTR(RealityDataServiceStat)

REALITYPLATFORM_TYPEDEFS(RealityDataUserStat)
REALITYPLATFORM_REF_COUNTED_PTR(RealityDataUserStat)

REALITYPLATFORM_TYPEDEFS(RealityDataExtended)
REALITYPLATFORM_REF_COUNTED_PTR(RealityDataExtended)

REALITYPLATFORM_TYPEDEFS(ConnectedRealityDataLocation)
REALITYPLATFORM_REF_COUNTED_PTR(ConnectedRealityDataLocation)

REALITYPLATFORM_TYPEDEFS(ConnectedRealityDataEnterpriseStat)
REALITYPLATFORM_REF_COUNTED_PTR(ConnectedRealityDataEnterpriseStat)

REALITYPLATFORM_TYPEDEFS(ConnectedRealityDataServiceStat)
REALITYPLATFORM_REF_COUNTED_PTR(ConnectedRealityDataServiceStat)

REALITYPLATFORM_TYPEDEFS(ConnectedRealityDataUserStat)
REALITYPLATFORM_REF_COUNTED_PTR(ConnectedRealityDataUserStat)

REALITYPLATFORM_TYPEDEFS(ConnectedRealityDataRelationship)
REALITYPLATFORM_REF_COUNTED_PTR(ConnectedRealityDataRelationship)

REALITYPLATFORM_TYPEDEFS(ConnectedRealityDataDocument)
REALITYPLATFORM_REF_COUNTED_PTR(ConnectedRealityDataDocument)

REALITYPLATFORM_TYPEDEFS(ConnectedRealityDataFolder)
REALITYPLATFORM_REF_COUNTED_PTR(ConnectedRealityDataFolder)

REALITYPLATFORM_TYPEDEFS(ConnectedRealityData)
REALITYPLATFORM_REF_COUNTED_PTR(ConnectedRealityData)

// Package
REALITYPLATFORM_TYPEDEFS(RealityDataPackage)
REALITYPLATFORM_REF_COUNTED_PTR(RealityDataPackage)
REALITYPLATFORM_TYPEDEFS(BoundingPolygon)
REALITYPLATFORM_REF_COUNTED_PTR(BoundingPolygon)
REALITYPLATFORM_TYPEDEFS(Uri)
REALITYPLATFORM_REF_COUNTED_PTR(Uri)

// RealiteSources
REALITYPLATFORM_TYPEDEFS(WmsDataSource)
REALITYPLATFORM_REF_COUNTED_PTR(WmsDataSource)
REALITYPLATFORM_TYPEDEFS(WmsMapSettings)
REALITYPLATFORM_REF_COUNTED_PTR(WmsMapSettings)
REALITYPLATFORM_TYPEDEFS(OsmDataSource)
REALITYPLATFORM_REF_COUNTED_PTR(OsmDataSource)
REALITYPLATFORM_TYPEDEFS(OsmResource)
REALITYPLATFORM_REF_COUNTED_PTR(OsmResource)
REALITYPLATFORM_TYPEDEFS(MultiBandSource)
REALITYPLATFORM_REF_COUNTED_PTR(MultiBandSource)

// RealityData
REALITYPLATFORM_TYPEDEFS(PackageRealityData)
REALITYPLATFORM_REF_COUNTED_PTR(PackageRealityData)
REALITYPLATFORM_TYPEDEFS(ImageryData)
REALITYPLATFORM_REF_COUNTED_PTR(ImageryData)
REALITYPLATFORM_TYPEDEFS(ModelData)
REALITYPLATFORM_REF_COUNTED_PTR(ModelData)
REALITYPLATFORM_TYPEDEFS(PinnedData)
REALITYPLATFORM_REF_COUNTED_PTR(PinnedData)
REALITYPLATFORM_TYPEDEFS(TerrainData)
REALITYPLATFORM_REF_COUNTED_PTR(TerrainData)
REALITYPLATFORM_TYPEDEFS(UndefinedData)
REALITYPLATFORM_REF_COUNTED_PTR(UndefinedData)

// RealitySerializers
REALITYPLATFORM_TYPEDEFS(RealityDataSerializerFactory)
REALITYPLATFORM_REF_COUNTED_PTR(RealityDataSerializerFactory)
REALITYPLATFORM_TYPEDEFS(RealityDataSerializer)
REALITYPLATFORM_REF_COUNTED_PTR(RealityDataSerializer)
REALITYPLATFORM_TYPEDEFS(RealityDataSerializerV1)
REALITYPLATFORM_REF_COUNTED_PTR(RealityDataSerializerV1)
REALITYPLATFORM_TYPEDEFS(RealityDataSerializerV2)
REALITYPLATFORM_REF_COUNTED_PTR(RealityDataSerializerV2)


BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! Status codes for RealityPackage operations
//=====================================================================================
enum class RealityPackageStatus
    {
    Success = SUCCESS,  // The operation was successful.
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
                                            UnknownError = ERROR,    // The operation failed with an unspecified error.
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE