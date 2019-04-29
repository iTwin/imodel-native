/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatform/RealityPlatformAPI.h>

#include <RealityPlatform/SpatialEntity.h>
#include <RealityPlatform/RealityDataObjects.h>
#include <RealityPlatform/RealityDataPackage.h>
#include <RealityPlatformTools/RealityDataDownload.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! @bsiclass                                   Spencer.Mason              01/2017
//! RealityConversionTools
//! This class provides multiple static methods for the conversion of JSON
//! fragments originating from the GeoCoordination Service or the RealityData Services.
//! They will return SpatialEntity or RealityData objects or a list of those, created with data extracted from JSON from 
//! the GeoCoordination Service or the RealityData Service.
//! Refer to the GeoCoordination Service and Reality Data Service documentation for details about the JSON format.
//! They take the whole string from the JSON response from the service with multiple spatial entities or reality data or sub-objects.
//! Since we are not always dealing with complete spatial entity classes (with adjoining SpatialDataSource, SpatialDataServer)
//! The result may be a partial reconstitution of the original Spatial Entity or Reality Data. This skeleton can be completed
//! Upon request from other components from the service.
//! None of these fields are guaranteed, and should be validated by the user. 
//! Remember that an empty value can result from a NULL or absent property in the JSON 
//!
//! It also provides static methods to convert a package into a download order.
//=====================================================================================
struct RealityConversionTools
    {
public:
    //! The first version provides a simple list of extracted spatial entities while the second version
    //! fills a map for which the spatial entity name is the key.
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToSpatialEntity(Utf8CP data, bvector<SpatialEntityPtr>* outData);
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToSpatialEntity(Utf8CP data, bmap<Utf8String, SpatialEntityPtr>* outData);

    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToSpatialEntityDataSource(Utf8CP data, bvector<SpatialEntityDataSourcePtr>* outData);
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToSpatialEntityServer(Utf8CP data, bvector<SpatialEntityServerPtr>* outData);
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToSpatialEntityMetadata(Utf8CP data, bvector<SpatialEntityMetadataPtr>* outData);

#if (1)
    //! Converts a reality data instance to a JSON fragment appropriate for the RealityData Service
    //! the two bool control the insertion of properties not always required
    //! The includeUnsetProps indicates if empty properties must be added to the JSON fragment.
    //! The includeROProps indicates that properties that cannot be set but are computed by the service be included or not.
    //! The includeIds indicates if the RealityData's Id should be included (it should not being included for change requests)
    REALITYDATAPLATFORM_EXPORT static Utf8String RealityDataToJson(RealityDataCR realityData, bool includeUnsetProps = false, bool includeROProps = false, bool includeIds = true);

    //! Converts a reality data instance to a JSON fragment appropriate for the RealityData Service. Only properties listed
    //! are inserted.
    REALITYDATAPLATFORM_EXPORT static Utf8String RealityDataToJson(RealityDataCR realityData, bvector<RealityDataField> properties);
#endif

    //! The first version provides a simple list of extracted Reality Data while the second version
    //! fills a map for which the spatial entity name is the key.
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToRealityData(Utf8CP data, bvector<RealityDataPtr>* outData);
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToRealityData(Utf8CP data, bmap<Utf8String, RealityDataPtr>* outData);

    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToRealityDataExtended(Utf8CP data, bvector<RealityDataExtendedPtr>* outData);

    //! To retrieve Data Location from a JSON fragment 
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToDataLocation(Utf8CP data, RealityDataLocation& dataLocationObject);
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToDataLocation(Json::Value properties, RealityDataLocation& datLocationObject);

    //! To retrieve many Data Location from a JSON fragment 
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToDataLocations(Utf8CP data, bvector<RealityDataLocation>& dataLocationObjects);

    //! To retrieve public keys from a JSON fragment 
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToPublicKey(Utf8CP data, RealityDataPublicKey& publicKeyObject);
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToPublicKey(Json::Value properties, RealityDataPublicKey& publicKeyObject);

    //! To retrieve many public keys from a JSON fragment 
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToPublicKeys(Utf8CP data, bvector<RealityDataPublicKey>& publicKeyObjects);

    //! To retrieve EnterpriseStat from a JSON fragment 
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToEnterpriseStat(Utf8CP data, RealityDataEnterpriseStat& statObject);
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToEnterpriseStat(Json::Value properties, RealityDataEnterpriseStat& statObject);

    //! To retrieve many EnterpriseStat from a JSON fragment 
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToEnterpriseStats(Utf8CP data, bvector<RealityDataEnterpriseStat>& statObjects);

    //! To retrieve ServiceStat from a JSON fragment 
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToServiceStat(Utf8CP data, RealityDataServiceStat& statObject);
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToServiceStat(Json::Value properties, RealityDataServiceStat& statObject);

    //! To retrieve many EnterpriseStat from a JSON fragment 
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToServiceStats(Utf8CP data, bvector<RealityDataServiceStat>& statObjects);

    //! To retrieve EnterpriseStat from a JSON fragment 
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToUserStat(Utf8CP data, RealityDataUserStat& statObject);
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToUserStat(Json::Value properties, RealityDataUserStat& statObject);

    //! To retrieve many UserStat from a JSON fragment 
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToUserStats(Utf8CP data, bvector<RealityDataUserStat>& statObjects);

    //! Utility function to extract a SpatialEntity from a SpatialEntityWithDetailsView JSON fragment.
    //! They take the JSON value as input and creates the SpatialEntity or Reality Data object.    
    REALITYDATAPLATFORM_EXPORT static SpatialEntityPtr JsonToSpatialEntity(Json::Value properties);

    REALITYDATAPLATFORM_EXPORT static void JsonToRealityData(RealityDataPtr realityData, Json::Value properties);

    REALITYDATAPLATFORM_EXPORT static SpatialEntityDataSourcePtr JsonToSpatialEntityDataSource(Json::Value properties);
    REALITYDATAPLATFORM_EXPORT static SpatialEntityServerPtr JsonToSpatialEntityServer(Json::Value properties);
    REALITYDATAPLATFORM_EXPORT static SpatialEntityMetadataPtr JsonToSpatialEntityMetadata(Json::Value properties);

    //! Utility methods that convert a file containing a GeoCoordinationService package, the package itself or a string containing the package
    //! and produces a download list according tot he package content including alternate data sources as mirror sites and so on.
    REALITYDATAPLATFORM_EXPORT static RealityDataDownload::Link_File_wMirrors_wSisters PackageStringToDownloadOrder(Utf8CP source, WStringP pParseError = NULL, BeFileNameCR destinationFolder = BeFileName(), bool skipStreams = true);
    REALITYDATAPLATFORM_EXPORT static RealityDataDownload::Link_File_wMirrors_wSisters PackageFileToDownloadOrder(BeFileNameCR filename, WStringP pParseError = NULL, BeFileNameCR destinationFolder = BeFileName(), bool skipStreams = true);
    REALITYDATAPLATFORM_EXPORT static RealityDataDownload::Link_File_wMirrors_wSisters PackageToDownloadOrder(RealityPlatform::RealityDataPackagePtr package, BeFileNameCR destinationFolder = BeFileName(), bool skipStreams = true);

    REALITYDATAPLATFORM_EXPORT static RealityDataDownload::sisterFileVector SpatialEntityDataSourceToSisterVector(RealityPlatform::SpatialEntityDataSourceCR dataSource, BeFileNameCR destinationFolder);

private:
    static RealityDataDownload::mirrorWSistersVector RealityDataToMirrorVector(const RealityPlatform::PackageRealityData& realityData, BeFileNameCR destinationFolder, bool skipStreams = true);
    static StatusInt JsonToObjectBase(Utf8CP data, Json::Value& json);
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE