/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/RealityConversionTools.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatform/RealityPlatformAPI.h>

#include <RealityPlatform/SpatialEntity.h>
#include <RealityPackage/RealityDataPackage.h>
#include <RealityPlatform/RealityDataDownload.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

struct RealityConversionTools
    {
public:
    //! Fills a bvector with SpatialEntity objects, created with data extracted from JSON from the GeoCoordination Service
    //! Refer to the GeoCoordination Service documentation for details about the JSON format.
    //! They take the whole string from the JSON response from the servie with multiple spatial entities with details views.
    //! Since we are not dealing with complete spatial entity classes (with adjoining SpatialDataSource, SpatialDataServer)
    //! The result is a partial reconstitution of the original Spatial Enity. This skeleton can be completed
    //! Upon request from other components from the service.
    //! Fills a bvector with SpatialEntity objects, created with data extracted from JSON
    //! Currently parses JSON for Name, DataType, Classification, Provider, Date, Size, 
    //! Resolution and Footprint. 
    //! None of these fields are guaranteed, and should be validated by the user. 
    //! Remember that an empty value can result from a NULL or absent property in the JSON 
    //!
    //! The first version provides a simple list of extracted spatial entities while the second version
    //! fills a map for which the spatial entity name is the key.
    //&&AR There can be Spatial Entities with the same name (see SRTM1, SRTM3)
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToSpatialEntity(Utf8CP data, bvector<SpatialEntityPtr>* outData);
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToSpatialEntity(Utf8CP data, bmap<Utf8String, SpatialEntityPtr>* outData);

    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToRealityData(Utf8CP data, bvector<RealityDataPtr>* outData);
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToRealityData(Utf8CP data, bmap<Utf8String, RealityDataPtr>* outData);

    //! To retreive EnterpriseStat 
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToEnterpriseStat(Utf8CP data, uint64_t* pNbRealityData, uint64_t* pTotalSizeKB);

    //! Utility function to extract a SpatialEntity from a SpatialEntityWithDetailsView JSON fragment.
    //! This one takes the JSON value as input and created the SpatialEntity object.    
    static SpatialEntityPtr JsonToSpatialEntity(Json::Value properties);    
    static RealityDataPtr JsonToRealityData(Json::Value properties);

    REALITYDATAPLATFORM_EXPORT static RealityDataDownload::Link_File_wMirrors_wSisters PackageStringToDownloadOrder(Utf8CP source, WStringP pParseError = NULL);
    REALITYDATAPLATFORM_EXPORT static RealityDataDownload::Link_File_wMirrors_wSisters PackageFileToDownloadOrder(BeFileNameCR filename, WStringP pParseError = NULL);
    REALITYDATAPLATFORM_EXPORT static RealityDataDownload::Link_File_wMirrors_wSisters PackageToDownloadOrder(RealityPackage::RealityDataPackagePtr package);

private:
    static RealityDataDownload::sisterFileVector RealityDataToSisterVector(RealityPackage::RealityDataSourceCP dataSource);
    static RealityDataDownload::mirrorWSistersVector RealityConversionTools::RealityDataToMirrorVector(RealityPackage::RealityDataPtr realityData);
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE