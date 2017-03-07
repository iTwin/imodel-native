//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/RealityPlatform/RealityConversionToolsTester.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//#ifdef REALITYMODFRAMEWORK_LOCALTEST


#include <Bentley/BeTest.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatform/RealityConversionTools.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//=====================================================================================
//! @bsiclass                          Spencer.Mason                            10/2016
//=====================================================================================
class RealityConversionTestFixture : public testing::Test
    {
    public:
    
    };

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            10/2016
//-------------------------------------------------------------------------------------
TEST_F(RealityConversionTestFixture, JsonToSpatialEntity1)
    {
    Utf8CP jsonString = "{\n \"instances\": [ \n{\n \"instanceId\": \"14812\",\n \"schemaName\" : \"RealityModeling\",\n \"className\" : \"SpatialEntityWithDetailsView\",\n \"properties\" : {\n \"Id\": \"14812\",\n \"Footprint\" : \"{ \\\"points\\\" : [[-92,39],[-92,38],[-93,38],[-93,39],[-92,39]], \\\"coordinate_system\\\" : \\\"4326\\\" }\",\n \"Name\" : \"N38W093\",\n \"Description\" : null,\n \"ContactInformation\" : null,\n \"Keywords\" : null,\n \"Legal\" : \"Data available from the U.S. Geological Survey\",\n  \"ProcessingDescription\" : null,\n \"DataSourceType\" : \"hgt\",\n \"AccuracyResolutionDensity\" : null,\n \"Date\" : \"2005-07-05T00:00:00\",\n \"Classification\" : \"Terrain\",\n \"FileSize\" : \"7066\",\n \"SpatialDataSourceId\" : \"14812\",\n \"ResolutionInMeters\" : \"24.2x30.922\",\n \"DataProvider\" : \"USGS\",\n \"DataProviderName\" : \"USGS\",\n \"ThumbnailURL\" : null,\n \"MetadataURL\" : null,\n \"RawMetadataURL\" : null,\n \"RawMetadataFormat\" : null,\n \"SubAPI\" : null\n },\n \"eTag\": \"gZIS2SFbXqohdwLlTRXkJOTCHz0=\"\n }\n ]\n }";

    bvector<SpatialEntityPtr> spatialVector = bvector<SpatialEntityPtr>();
    StatusInt status = RealityConversionTools::JsonToSpatialEntity(jsonString, &spatialVector);
    ASSERT_EQ(SUCCESS, status);
    ASSERT_EQ(spatialVector.size(), 1);
    SpatialEntityPtr spatialData = spatialVector[0];
    ASSERT_TRUE(spatialData.IsValid());

    ASSERT_EQ(spatialData->GetName(), "N38W093");    
    ASSERT_EQ(spatialData->GetIdentifier(), "14812");
    ASSERT_EQ(spatialData->GetDataType(), "hgt");
    ASSERT_EQ(spatialData->GetProvider(), "USGS");
    DateTime date;
    DateTime::FromString(date, "2005-07-05T00:00:00");
    ASSERT_EQ(spatialData->GetDate(), date);
    ASSERT_EQ(spatialData->GetApproximateFileSize(), 7066);
    ASSERT_EQ(spatialData->GetResolution(), "24.2x30.922");
    bvector<GeoPoint2d> footprint = bvector<GeoPoint2d>();
    footprint.push_back(GeoPoint2d::From(-92, 39));
    footprint.push_back(GeoPoint2d::From(-92, 38));
    footprint.push_back(GeoPoint2d::From(-93, 38));
    footprint.push_back(GeoPoint2d::From(-93, 39));
    footprint.push_back(GeoPoint2d::From(-92, 39));

    for (int i = 0 ; i != footprint.size(); i++)
        {
        ASSERT_EQ(spatialData->GetFootprint()[i].longitude, footprint[i].longitude);
        ASSERT_EQ(spatialData->GetFootprint()[i].latitude, footprint[i].latitude);
        }
    }

#if (0)
//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            10/2016
//-------------------------------------------------------------------------------------
TEST_F(RealityConversionTestFixture, JsonToSpatialEntity2)
    {
    Utf8CP jsonString = "{"
                          "instances\": [" 
                            "{"
                            "\"instanceId\": \"14812\","
                            "\"schemaName\" : \"RealityModeling\","
                            "\"className\" : \"SpatialEntityWithDetailsView\","
                            "\"properties\" : {"
                                  "\"Id\": \"14812\","
                                  "\"Footprint\" : \"{ \\\"points\\\" : [[-92,39],[-92,38],[-93,38],[-93,39],[-92,39]], \\\"coordinate_system\\\" : \\\"4326\\\" }\","
                                  "\"Name\" : \"N38W093\","
                                  "\"Description\" : null," 
                                  "\"ContactInformation\" : null,"
                                  "\"Keywords\" : null,"
                                  "\"Legal\" : \"Data available from the U.S. Geological Survey\","
                                  "\"ProcessingDescription\" : null,"
                                  "\"DataSourceType\" : \"hgt\","
                                  "\"AccuracyResolutionDensity\" : null,"
                                  "\"Date\" : \"2005-07-05T00:00:00\","
                                  "\"Classification\" : \"Terrain\","
                                  "\"FileSize\" : \"7066\","
                                  "\"SpatialDataSourceId\" : \"14812\","
                                  "\"ResolutionInMeters\" : \"24.2x30.922\","
                                  "\"DataProvider\" : \"USGS\","
                                  "\"DataProviderName\" : \"United States Geological Survey\","
                                  "\"ThumbnailURL\" : null,"
                                  "\"MetadataURL\" : null,"
                                  "\"RawMetadataURL\" : null,"
                                  "\"RawMetadataFormat\" : null,"
                                  " \"SubAPI\" : null"
                               " },"
                            " \"eTag\": \"gZIS2SFbXqohdwLlTRXkJOTCHz0=\""
                           "},"
                            "{"
                            "\"instanceId\": \"14813\","
                            "\"schemaName\" : \"RealityModeling\","
                            "\"className\" : \"SpatialEntityWithDetailsView\","
                            "\"properties\" : {"
                                  "\"Id\": \"14813\","
                                  "\"Footprint\" : \"{ \\\"points\\\" : [[-92,39],[-92,38],[-93,38],[-93,39],[-92,39]], \\\"coordinate_system\\\" : \\\"4326\\\" }\","
                                  "\"Name\" : \"N39W093\","
                                  "\"Description\" : null," 
                                  "\"ContactInformation\" : null,"
                                  "\"Keywords\" : null,"
                                  "\"Legal\" : \"Data also available from the U.S. Geological Survey\","
                                  "\"ProcessingDescription\" : \"This is a dummy description that is not Lorem Ipsum based\","
                                  "\"DataSourceType\" : \"hgt;tif;dem\","
                                  "\"AccuracyResolutionDensity\" : null,"
                                  "\"Date\" : \"2005-07-05T12:12:00\","
                                  "\"Classification\" : \"Terrain\","
                                  "\"FileSize\" : \"7066\","
                                  "\"SpatialDataSourceId\" : \"14812\","
                                  "\"ResolutionInMeters\" : \"24.2x30.922\","
                                  "\"DataProvider\" : \"USGS\","
                                  "\"DataProviderName\" : \"United States Geological Survey\","
                                  "\"ThumbnailURL\" : null,"
                                  "\"MetadataURL\" : null,"
                                  "\"RawMetadataURL\" : null,"
                                  "\"RawMetadataFormat\" : null,"
                                  " \"SubAPI\" : null"
                               " },"
                            " \"eTag\": \"gZIS2SFbXqohdwLlTRXkJOTCHz0=\""
                           "}"
                       " ]"
                     " }";

    bmap<Utf8String, SpatialEntityPtr> spatialMap;

    StatusInt status = RealityConversionTools::JsonToSpatialEntity(jsonString, &spatialMap);

    ASSERT_EQ(SUCCESS, status);
    ASSERT_EQ(spatialMap.size(), 2);
    SpatialEntityPtr spatialData = spatialMap[0];
    ASSERT_TRUE(spatialData.IsValid());

    ASSERT_EQ(spatialData->GetName(), "N38W093");    
    ASSERT_EQ(spatialData->GetIdentifier(), "14812");
    ASSERT_EQ(spatialData->GetDataType(), "hgt");
    ASSERT_EQ(spatialData->GetProvider(), "USGS");
    DateTime date;
    DateTime::FromString(date, "2005-07-05T00:00:00");
    ASSERT_EQ(spatialData->GetDate(), date);
    ASSERT_EQ(spatialData->GetApproximateFileSize(), 7066);
    ASSERT_EQ(spatialData->GetResolution(), "24.2x30.922");
    bvector<GeoPoint2d> footprint = bvector<GeoPoint2d>();
    footprint.push_back(GeoPoint2d::From(-92, 39));
    footprint.push_back(GeoPoint2d::From(-92, 38));
    footprint.push_back(GeoPoint2d::From(-93, 38));
    footprint.push_back(GeoPoint2d::From(-93, 39));
    footprint.push_back(GeoPoint2d::From(-92, 39));

    for (int i = 0 ; i != footprint.size(); i++)
        {
        ASSERT_EQ(spatialData->GetFootprint()[i].longitude, footprint[i].longitude);
        ASSERT_EQ(spatialData->GetFootprint()[i].latitude, footprint[i].latitude);
        }
    }
//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            02/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityConversionTestFixture, JsonToEnterpriseStatTest)
    {
    Utf8CP jsonString = "";

    uint64_t nbRealityData;
    uint64_t totalSize;
    StatusInt status = RealityConversionTools::JsonToEnterpriseStat(jsonString, &nbRealityData, &totalSize);

    EXPECT_EQ(nbRealityData, 1234567);
    EXPECT_EQ(totalSize, 54673407607);

    // Test that the conversion can be given null params without craching
    status = RealityConversionTools::JsonToEnterpriseStat(jsonString, NULL, &totalSize);
    status = RealityConversionTools::JsonToEnterpriseStat(jsonString, &nbRealityData, NULL);
   

    }

#endif