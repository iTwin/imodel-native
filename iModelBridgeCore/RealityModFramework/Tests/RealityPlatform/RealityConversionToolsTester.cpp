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
    Utf8CP jsonString = "{"
                         "\"instances\": [ "
                            "{"
                            " \"instanceId\": \"14812\","
                            " \"schemaName\" : \"RealityModeling\","
                            " \"className\" : \"SpatialEntityWithDetailsView\","
                            " \"properties\" : {"
                                " \"Id\": \"14812\","
                                " \"Footprint\" : \"{ \\\"points\\\" : [[-92,39],[-92,38],[-93,38],[-93,39],[-92,39]], \\\"coordinate_system\\\" : \\\"4326\\\" }\","
                                " \"Name\" : \"N38W093\","
                                " \"Description\" : null,"
                                " \"ContactInformation\" : null,"
                                " \"Keywords\" : null,"
                                " \"Legal\" : \"Data available from the U.S. Geological Survey\","
                                "  \"ProcessingDescription\" : null,"
                                " \"DataSourceType\" : \"hgt\","
                                " \"AccuracyResolutionDensity\" : null,"
                                " \"Date\" : \"2005-07-05T00:00:00\","
                                " \"Classification\" : \"Terrain\","
                                " \"FileSize\" : \"7066\","
                                " \"SpatialDataSourceId\" : \"14812\","
                                " \"ResolutionInMeters\" : \"24.2x30.922\","
                                " \"DataProvider\" : \"USGS\","
                                " \"DataProviderName\" : \"USGS\","
                                " \"ThumbnailURL\" : null,"
                                " \"MetadataURL\" : null,"
                                " \"RawMetadataURL\" : null,"
                                " \"RawMetadataFormat\" : null,"
                                " \"SubAPI\" : null"
                                " },"
                            " \"eTag\": \"gZIS2SFbXqohdwLlTRXkJOTCHz0=\""
                            " }"
                            " ]"
                        " }";

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

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            04/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityConversionTestFixture, JsonToEnterpriseStat)
    {
    Utf8CP jsonString = "{"
                        "\"instances\": ["
                            "{"
                            "\"instanceId\": \"\","
                            "\"schemaName\" : \"S3MX\","
                            "\"className\" : \"EnterpriseStat\","
                            "\"properties\" : {"
                                "\"TotalSize\": 235892929,"
                                "\"NumberOfRealityData\" : 1425,"
                                "\"EnterpriseId\" : \"e82a584b-9fae-409f-9581-fd154f7b9ef9\""
                                "},"
                            "\"eTag\" : \"\\\"ifZwDzBbgEqb8b0Z62MNmrQTysQ=\\\"\""
                            "}"
                        "]"
                        "}";

    uint64_t nbRealityData;
    uint64_t totalSizeKB;
    StatusInt status = RealityConversionTools::JsonToEnterpriseStat(jsonString, &nbRealityData, &totalSizeKB);
    ASSERT_EQ(SUCCESS, status);
    ASSERT_EQ(nbRealityData, 1425);
    ASSERT_EQ(totalSizeKB, 235892929);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            04/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityConversionTestFixture, JsonToRealityData)
    {
    Utf8CP jsonString = "{"
                        "\"instances\": [ "
                            "{"
                            "\"instanceId\": \"af3c43a9-1797-4765-a7c3-6f1cd6706fa9\","
                            "\"schemaName\" : \"S3MX\","
                            "\"className\" : \"RealityData\","
                            "\"properties\" : {"
                                "\"Id\": \"af3c43a9-1797-4765-a7c3-6f1cd6706fa9\","
                                "\"EnterpriseId\" : \"e82a584b-9fae-409f-9581-fd154f7b9ef9\","
                                "\"ContainerName\" : \"af3c43a9-1797-4765-a7c3-6f1cd6706fa9\","
                                "\"Name\" : \"Helsinki\","
                                "\"Dataset\" : \"Geogratis\","
                                "\"Group\" : \"TestGroup\","
                                "\"Description\" : \"Helsinki is the capital and largest city of Finland. It is in the region of Uusimaa, in southern Finland, on the shore of the Gulf of Finland. Helsinki has a population of 629,512,[3] an urban population of 1,214,210,[7] and a metropolitan population of over 1.4 million, making it the most populous municipality and urban area in Finland.\","
                                "\"RootDocument\" : \"Helsinki/Scene/Production_Helsinki_3MX_ok.3mx\","
                                "\"Size\" : 1036657,"
                                "\"Classification\" : \"Terrain\","
                                "\"Streamed\" : false,"
                                "\"Type\" : \"3mx\","
                                "\"Footprint\" : \"{ \\\"points\\\" : [[24.7828757,59.9224887],[25.2544848,59.9224887],[25.2544848,60.2978389],[24.7828757,60.2978389],[24.7828757,59.9224887]], \\\"coordinate_system\\\" : \\\"4326\\\" }\","
                                "\"ThumbnailDocument\" : \"Helsinki/thumbnail.jpg\","
                                "\"MetadataURL\" : \"www.bigTest.com\","
                                "\"Copyright\" : \"Copyright goes here\","
                                "\"TermsOfUse\" : \"And the terms go here\","
                                "\"AccuracyInMeters\" : \"16.147\","
                                "\"ResolutionInMeters\" : \"13.45x16.34\","
                                "\"Visibility\" : \"PRIVATE\","
                                "\"Listable\" : true,"
                                "\"ModifiedTimestamp\" : \"2017-02-01T22:26:06.4138132Z\","
                                "\"CreatedTimestamp\" : \"2017-02-01T22:26:06.4138132Z\","
                                "\"OwnedBy\" : \"francis.boily@bentley.com\""
                                "},"
                            "\"eTag\": \"\\\"bVDdVT+8j6HTmIo7PNaqVcyYyLw=\\\"\""
                            "}"
                        " ]"
                        " }";

    bvector<RealityDataPtr> realityVector = bvector<RealityDataPtr>();
    StatusInt status = RealityConversionTools::JsonToRealityData(jsonString, &realityVector);
    ASSERT_EQ(SUCCESS, status);
    ASSERT_EQ(realityVector.size(), 1);
    RealityDataPtr realityData = realityVector[0];
    ASSERT_TRUE(realityData.IsValid());

    ASSERT_EQ(realityData->GetIdentifier(), "af3c43a9-1797-4765-a7c3-6f1cd6706fa9");
    ASSERT_EQ(realityData->GetEnterpriseId(), "e82a584b-9fae-409f-9581-fd154f7b9ef9");
    ASSERT_EQ(realityData->GetContainerName(), "af3c43a9-1797-4765-a7c3-6f1cd6706fa9");
    ASSERT_EQ(realityData->GetName(), "Helsinki");
    ASSERT_EQ(realityData->GetDataset(), "Geogratis");
    ASSERT_EQ(realityData->GetGroup(), "TestGroup");
    ASSERT_EQ(realityData->GetDescription(), "Helsinki is the capital and largest city of Finland. It is in the region of Uusimaa, in southern Finland, on the shore of the Gulf of Finland. Helsinki has a population of 629,512,[3] an urban population of 1,214,210,[7] and a metropolitan population of over 1.4 million, making it the most populous municipality and urban area in Finland.");
    ASSERT_EQ(realityData->GetRootDocument(), "Helsinki/Scene/Production_Helsinki_3MX_ok.3mx");
    ASSERT_EQ(realityData->GetTotalSize(), 1036657);
    ASSERT_EQ(realityData->GetClassification(), RealityDataBase::Classification::TERRAIN);
    ASSERT_EQ(realityData->GetClassificationTag(), "Terrain");
    ASSERT_EQ(realityData->IsStreamed(), false);
    ASSERT_EQ(realityData->GetRealityDataType(), "3mx");
    ASSERT_EQ(realityData->GetFootprintString(), "{ \"points\" : [[24.7828757,59.9224887],[25.2544848,59.9224887],[25.2544848,60.2978389],[24.7828757,60.2978389],[24.7828757,59.9224887]], \"coordinate_system\" : \"4326\" }");
    ASSERT_EQ(realityData->GetThumbnailDocument(), "Helsinki/thumbnail.jpg");
    ASSERT_EQ(realityData->GetMetadataURL(), "www.bigTest.com");
    ASSERT_EQ(realityData->GetCopyright(), "Copyright goes here");
    ASSERT_EQ(realityData->GetTermsOfUse(), "And the terms go here");
    ASSERT_EQ(realityData->GetAccuracy(), "16.147");
    ASSERT_EQ(realityData->GetAccuracyValue(), 16.147);
    ASSERT_EQ(realityData->GetResolution(), "13.45x16.34");
    ASSERT_EQ(realityData->GetResolutionValue(), sqrt(13.45 * 16.34));
    ASSERT_EQ(realityData->GetVisibility(), RealityDataBase::Visibility::PRIVATE);
    ASSERT_EQ(realityData->GetVisibilityTag(), "PRIVATE");
    ASSERT_EQ(realityData->IsListable(), true);
    ASSERT_EQ(realityData->GetModifiedDateTime().ToString(), "2017-02-01T22:26:06.414Z");
    ASSERT_EQ(realityData->GetCreationDateTime().ToString(), "2017-02-01T22:26:06.414Z");
    ASSERT_EQ(realityData->GetOwner(), "francis.boily@bentley.com");
    DRange2dCR range = realityData->GetFootprintExtent();
    ASSERT_EQ(range.low.x, 24.7828757);
    ASSERT_EQ(range.low.y, 59.9224887);
    ASSERT_EQ(range.high.x, 25.2544848);
    ASSERT_EQ(range.high.y, 60.2978389);
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