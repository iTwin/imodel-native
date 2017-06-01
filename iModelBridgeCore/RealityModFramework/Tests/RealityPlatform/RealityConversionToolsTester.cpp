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

using ::testing::Contains;
using ::testing::Key;

//=====================================================================================
//! @bsiclass                          Spencer.Mason                            10/2016
//=====================================================================================
class RealityConversionTestFixture : public testing::Test
    {
    public:
		static Utf8CP s_TwoSpatialEntityJSONString;
        static Utf8CP s_EnterpriseStatJSONString;
        static Utf8CP s_RealityDataJSONString;
		static Utf8CP s_EntityDataSourceJSONString;
		static Utf8CP s_SpatialEntityServerJSONString;
		static Utf8CP s_SpatialEntityMetadataJSONString;

	WCharCP GetDirectory()
	{
		WChar exePath[MAX_PATH];
		GetModuleFileNameW(NULL, exePath, MAX_PATH);

		WString exeDir = exePath;
		size_t pos = exeDir.find_last_of(L"/\\");
		exeDir = exeDir.substr(0, pos + 1);
		return exeDir.c_str();
	}
    };


Utf8CP RealityConversionTestFixture::s_TwoSpatialEntityJSONString = R"(
	{
		"instances": [
			{
				"instanceId": "14812",
				"schemaName": "RealityModeling",
				"className": "SpatialEntityWithDetailsView",
				"properties": {
					"Id": "14812",
					"Footprint": "{ \"points\" : [[-92,39],[-92,38],[-93,38],[-93,39],[-92,39]], \"coordinate_system\" : \"4326\" }",
					"Name": "N38W093",
					"Description": null,
					"ContactInformation": null,
					"Keywords": null,
					"Legal": "Data available from the U.S. Geological Survey",
					"ProcessingDescription": null,
					"DataSourceType": "hgt",
					"Dataset": "someDataSet",
					"AccuracyResolutionDensity": null,
					"Date": "2005-07-05T00:00:00.000",
					"Classification": "Terrain",
					"FileSize": "7066",
					"SpatialDataSourceId": "14812",
					"ResolutionInMeters": "24.2x30.922",
					"AccuracyInMeters": "5x5",
					"DataProvider": "USGS",
					"DataProviderName": "United States Geological Survey",
					"ThumbnailURL": "http://www.example.com/thumbnail.jpg",
					"MetadataURL": null,
					"RawMetadataURL": null,
					"RawMetadataFormat": null,
					"SubAPI": null
				},
				"eTag": "gZIS2SFbXqohdwLlTRXkJOTCHz0="
			},
			{
				"instanceId": "14813",
				"schemaName": "RealityModeling",
				"className": "SpatialEntityWithDetailsView",
				"properties": {
					"Id": "14813",
					"Footprint": "{ \"points\" : [[-92,39],[-92,38],[-93,38],[-93,39],[-92,39]], \"coordinate_system\" : \"4326\" }",
					"Name": "N39W093",
					"Description": null,
					"ContactInformation": null,
					"Keywords": null,
					"Legal": "Data also available from the U.S. Geological Survey",
					"ProcessingDescription": "This is a dummy description that is not Lorem Ipsum based",
					"DataSourceType": "hgt;tif;dem",
					"AccuracyResolutionDensity": null,
					"Date": "2005-07-05T12:12:00",
					"Classification": "Terrain",
					"Dataset": "someDataSet",
					"FileSize": "7066",
					"SpatialDataSourceId": "14812",
					"ResolutionInMeters": "24.2x30.922",
					"DataProvider": "USGS",
					"DataProviderName": "United States Geological Survey",
					"ThumbnailURL": null,
					"MetadataURL": null,
					"RawMetadataURL": null,
					"RawMetadataFormat": null,
					"SubAPI": null
				},
				"eTag": "gZIS2SFbXqohdwLlTRXkJOTCHz0="
			}
		]
	}
        )";

Utf8CP RealityConversionTestFixture::s_EnterpriseStatJSONString = R"(
    {
        "instances": [
            {
                "instanceId": "",
                "schemaName": "S3MX",
                "className": "EnterpriseStat",
                "properties": {
                    "TotalSize": 235892929,
                    "NumberOfRealityData": 1425,
                    "OrganizationId": "e82a584b-9fae-409f-9581-fd154f7b9ef9",
                    "UltimateId": "e82a584b-9fae-409f-9581-fd154f7b9ef9",
                    "UltimateSite": "e82a584b-9fae-409f-9581-fd154f7b9ef9"
                },
                "eTag": "ifZwDzBbgEqb8b0Z62MNmrQTysQ="
            }
        ]
    }
    )";

Utf8CP RealityConversionTestFixture::s_RealityDataJSONString = R"(
    {
        "instances": [
            {
                "instanceId": "af3c43a9-1797-4765-a7c3-6f1cd6706fa9",
                "schemaName": "S3MX",
                "className": "RealityData",
                "properties": {
                    "Id": "af3c43a9-1797-4765-a7c3-6f1cd6706fa9",
                    "OrganizationId": "e82a584b-9fae-409f-9581-fd154f7b9ef9",
                    "ContainerName": "af3c43a9-1797-4765-a7c3-6f1cd6706fa9",
                    "Name": "Helsinki",
                    "Dataset": "Geogratis",
                    "Group": "TestGroup",
                    "Description": "Helsinki is the capital and largest city of Finland. It is in the region of Uusimaa, in southern Finland, on the shore of the Gulf of Finland. Helsinki has a population of 629,512,[3] an urban population of 1,214,210,[7] and a metropolitan population of over 1.4 million, making it the most populous municipality and urban area in Finland.",
                    "RootDocument": "Helsinki/Scene/Production_Helsinki_3MX_ok.3mx",
                    "Size": "1036657",
                    "Classification": "Terrain",
                    "Streamed": false,
                    "Type": "3mx",
                    "Footprint": {
                        "Coordinates": [
                            {
                                "Long": 24.7828757,
                                "Lat": 59.9224887
                            },
                            {
                                "Long": 25.2544848,
                                "Lat": 59.9224887
                            },
                            {
                                "Long": 25.2544848,
                                "Lat": 60.2978389
                            },
                            {
                                "Long": 24.7828757,
                                "Lat": 60.2978389
                            },
                            {
                                "Long": 24.7828757,
                                "Lat": 59.9224887
                            }
                        ]
                    },
                    "ThumbnailDocument": "Helsinki/thumbnail.jpg",
                    "MetadataUrl": "www.bigTest.com",
                    "UltimateId": "uId",
                    "UltimateSite": "www.bigTest.com/1",
                    "Copyright": "Copyright goes here",
                    "TermsOfUse": "And the terms go here",
                    "AccuracyInMeters": "16.147",
                    "ResolutionInMeters": "13.45x16.34",
                    "Visibility": "PRIVATE",
                    "Listable": true,
                    "ModifiedTimestamp": "2017-02-01T22:26:06.414Z",
                    "CreatedTimestamp": "2017-02-01T22:26:06.414Z",
                    "OwnedBy": "francis.boily@bentley.com"
                },
                "eTag": "bVDdVT+8j6HTmIo7PNaqVcyYyLw="
            },
            {
                "instanceId": "bf3c43a9-1797-4765-a7c3-6f1cd6706fa9",
                "schemaName": "S3MX",
                "className": "RealityData",
                "properties": {
                    "Id": "bf3c43a9-1797-4765-a7c3-6f1cd6706fa9",
                    "OrganizationId": "e82a584b-9fae-409f-9581-fd154f7b9ef9",
                    "ContainerName": "af3c43a9-1797-4765-a7c3-6f1cd6706fa9",
                    "Name": "Helsinki2",
                    "Dataset": "Geogratis2",
                    "Group": "TestGroup",
                    "Description": "Helsinki222 is the capital and largest city of Finland. It is in the region of Uusimaa, in southern Finland, on the shore of the Gulf of Finland. Helsinki has a population of 629,512,[3] an urban population of 1,214,210,[7] and a metropolitan population of over 1.4 million, making it the most populous municipality and urban area in Finland.",
                    "RootDocument": "Helsinki2/Scene/Production_Helsinki_3MX_ok.3mx",
                    "Size": "1036657",
                    "Classification": "Terrain",
                    "Streamed": false,
                    "Type": "3mx",
                    "Footprint": {
                        "Coordinates": [
                            {
                                "Long": 24.7828757,
                                "Lat": 59.9224887
                            },
                            {
                                "Long": 25.2544848,
                                "Lat": 59.9224887
                            },
                            {
                                "Long": 25.2544848,
                                "Lat": 60.2978389
                            },
                            {
                                "Long": 24.7828757,
                                "Lat": 60.2978389
                            },
                            {
                                "Long": 24.7828757,
                                "Lat": 59.9224887
                            }
                        ]
                    },
                    "ThumbnailDocument": "Helsinki/thumbnail.jpg",
                    "MetadataUrl": "www.bigTest.com",
                    "UltimateId": "uId",
                    "UltimateSite": "www.bigTest.com/1",
                    "Copyright": "Copyright goes here",
                    "TermsOfUse": "And the terms go here",
                    "AccuracyInMeters": "16.147",
                    "ResolutionInMeters": "13.45x16.34",
                    "Visibility": "PRIVATE",
                    "Listable": true,
                    "ModifiedTimestamp": "2017-02-01T22:26:06.414Z",
                    "CreatedTimestamp": "2017-02-01T22:26:06.414Z",
                    "OwnedBy": "francis.boily@bentley.com"
                },
                "eTag": "bVDdVT+8j6HTmIo7PNaqVcyYyLw="
            }
        ]
    }
    )";

	Utf8CP RealityConversionTestFixture::s_EntityDataSourceJSONString = 
		R"(
		
		{
		  "instances": [
			{
			  "instanceId": "1",
			  "schemaName": "RealityModeling",
			  "className": "SpatialDataSource",
			  "properties": {
				"Id": "1",
				"MainURL": "http://www.openstreetmap.org/api/0.6/map?",
				"ParameterizedURL": "http://www.example.com",
				"CompoundType": "myCompoundType",
				"LocationInCompound": "myLocationInCompound",
				"DataSourceType": "OSM",
				"SisterFiles": "mySisterfiles",
				"NoDataValue": "myDataValue",
				"FileSize": "9999",
				"CoordinateSystem": "EPSG:9999",
				"Streamed": true,
				"Metadata": "someMetaDATA"
			  },
			  "eTag": "\"5gfZgh6RyYOK1ugIJsUVj6LDp7Y=\""
			},
			{
			  "instanceId": "2",
			  "schemaName": "RealityModeling",
			  "className": "SpatialDataSource",
			  "properties": {
				"Id": "2",
				"MainURL": "http://www.openstreetmap.org/api/0.6/map?",
				"ParameterizedURL": "http://www.example.com",
				"CompoundType": "myCompoundType",
				"LocationInCompound": "myLocationInCompound",
				"DataSourceType": "OSM",
				"SisterFiles": "mySisterfiles",
				"NoDataValue": "myDataValue",
				"FileSize": "9999",
				"CoordinateSystem": "EPSG:4326",
				"Streamed": true,
				"Metadata": "someMetaDATA"
			  },
			  "eTag": "\"5gfZgh6RyYOK1ugIJsUVj6LDp7Y=\""
			}
		  ]
		}

		)";

	Utf8CP RealityConversionTestFixture::s_SpatialEntityServerJSONString =
		R"(
		
		{
			"instances": [
			{
				"instanceId": "1",
				"schemaName": "RealityModeling",
				"className": "Server",
				"properties": {
				"Id": "1",
					"CommunicationProtocol": "ftp",
					"Streamed": true,
					"LoginKey": "myLoginKey",
					"LoginMethod": "myLoginMethod",
					"RegistrationPage": "http://www.example.com/register",
					"OrganisationPage": "http://www.example.com/organisation",
					"Name": "GeoGratis",
					"URL": "ftp://ftp.geogratis.gc.ca/pub/nrcan_rncan/elevation/cdem_mnec/",
					"ServerContactInformation": "mycontact@example.com",
					"Fees": "Over 9000",
					"Legal": "Better Call Saul",
					"AccessConstraints": "No one",
					"Online": true,
					"LastCheck": "2017-02-02T00:28:22",
					"LastTimeOnline": "2017-02-03T00:28:22",
					"Latency": 9000,
					"MeanReachabilityStats": 5,
					"State": "myState",
					"Type": "sftp"
				},
				"eTag": "\"tkErs5pTIlkWlQgzDzW9u0kuBgk=\""
			},
			{
				"instanceId": "2",
				"schemaName": "RealityModeling",
				"className": "Server",
				"properties": {
				"Id": "2",
					"CommunicationProtocol": "ftp",
					"Streamed": null,
					"LoginKey": null,
					"LoginMethod": null,
					"RegistrationPage": null,
					"OrganisationPage": null,
					"Name": "GeoGratis2",
					"URL": "ftp://ftp.geogratis.gc.ca/pub/nrcan_rncan/elevation/cdem_mnec/",
					"ServerContactInformation": null,
					"Fees": null,
					"Legal": null,
					"AccessConstraints": null,
					"Online": true,
					"LastCheck": "2017-02-02T00:28:22",
					"LastTimeOnline": "2017-02-02T00:28:22",
					"Latency": null,
					"MeanReachabilityStats": null,
					"State": null,
					"Type": "ftp"
				},
				"eTag": "\"tkErs5pTIlkWlQgzDzW9u0kuBgk=\""
			}
		]
		}

		)";

	Utf8CP RealityConversionTestFixture::s_SpatialEntityMetadataJSONString = R"(

	{
	  "instances": [
		{
		  "instanceId": "1",
		  "schemaName": "RealityModeling",
		  "className": "Metadata",
		  "properties": {
			"Id": "1",
			"MetadataURL": "http://www.example.com/metadata",
			"DisplayStyle": "myDisplayStyle",
			"Description": "some big description",
			"ContactInformation": "my contact",
			"Keywords": "keyword1;keyword2",
			"Legal": "© OpenStreetMap contributors",
			"TermsOfUse": "Everyone",
			"Lineage": "ok",
			"Provenance": "somewhere"
		  },
		  "eTag": "\"+g+HZYXF8u7jrPDPTI5JJ5Sl2aE=\""
		},
		{
		  "instanceId": "2",
		  "schemaName": "RealityModeling",
		  "className": "Metadata",
		  "properties": {
			"Id": "2",
			"MetadataURL": null,
			"DisplayStyle": null,
			"Description": null,
			"ContactInformation": null,
			"Keywords": null,
			"Legal": "© OpenStreetMap contributors",
			"TermsOfUse": null,
			"Lineage": null,
			"Provenance": null
		  },
		  "eTag": "\"+g+HZYXF8u7jrPDPTI5JJ5Sl2aE=\""
		}
	  ]
	}

    )";

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            10/2016
//-------------------------------------------------------------------------------------
TEST_F(RealityConversionTestFixture, JsonToSpatialEntitybvector)
    {
    bvector<SpatialEntityPtr> spatialVector = bvector<SpatialEntityPtr>();
    StatusInt status = RealityConversionTools::JsonToSpatialEntity(s_TwoSpatialEntityJSONString, &spatialVector);
    ASSERT_EQ(SUCCESS, status);
    ASSERT_EQ(spatialVector.size(), 2);
    SpatialEntityPtr spatialData = spatialVector[0];
    ASSERT_TRUE(spatialData.IsValid());

    ASSERT_EQ(spatialData->GetName(), "N38W093");    
    ASSERT_EQ(spatialData->GetIdentifier(), "14812");
    ASSERT_EQ(spatialData->GetDataType(), "hgt");
    ASSERT_EQ(spatialData->GetProvider(), "USGS");
	ASSERT_EQ(spatialData->GetProviderName(), "United States Geological Survey");
	ASSERT_STREQ(spatialData->GetThumbnailURL().c_str(), "http://www.example.com/thumbnail.jpg");
	ASSERT_STREQ(spatialData->GetAccuracy().c_str(), "5x5");
	ASSERT_NEAR(spatialData->GetAccuracyValue(), sqrt(5*5), 0.000001);
	ASSERT_STREQ(spatialData->GetDataset().c_str(), "someDataSet");

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
    RealityDataEnterpriseStat stat;
    StatusInt status = RealityConversionTools::JsonToEnterpriseStat(s_EnterpriseStatJSONString, stat);
    ASSERT_EQ(SUCCESS, status);
    ASSERT_EQ(stat.GetNbRealityData(), 1425);
    ASSERT_EQ(stat.GetTotalSizeKB(), 235892929);
    ASSERT_EQ(stat.GetOrganizationId(), "e82a584b-9fae-409f-9581-fd154f7b9ef9");
    ASSERT_EQ(stat.GetUltimateId(), "e82a584b-9fae-409f-9581-fd154f7b9ef9");
    ASSERT_EQ(stat.GetUltimateSite(), "e82a584b-9fae-409f-9581-fd154f7b9ef9");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                        Remi.Charbonneau                           05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityConversionTestFixture, BadJsonToEnterpriseStat)
	{
	RealityDataEnterpriseStat stat;
	StatusInt status = RealityConversionTools::JsonToEnterpriseStat("BadlyformatedJSONString", stat);
	ASSERT_EQ(ERROR, status);
	}

//-------------------------------------------------------------------------------------
// @bsimethod                        Remi.Charbonneau                           05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityConversionTestFixture, BadJsonToEnterpriseStat2)
	{
	RealityDataEnterpriseStat stat;
	StatusInt status = RealityConversionTools::JsonToEnterpriseStat(R"({ "notInstances": { "child": "value" }})", stat);
	ASSERT_EQ(ERROR, status);
	}

//-------------------------------------------------------------------------------------
// @bsimethod                        Remi.Charbonneau                           05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityConversionTestFixture, BadJsonToEnterpriseStat3)
	{
	RealityDataEnterpriseStat stat;
	StatusInt status = RealityConversionTools::JsonToEnterpriseStat(R"(
		{
			"instances": [
				{
					"Notproperties": {
						"Id": "myid"
					}
				}
			]
		}
		)", stat);
	ASSERT_EQ(ERROR, status);
	}

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            04/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityConversionTestFixture, JsonToRealityData)
    {
    bvector<RealityDataPtr> realityVector = bvector<RealityDataPtr>();
    StatusInt status = RealityConversionTools::JsonToRealityData(s_RealityDataJSONString, &realityVector);
    ASSERT_EQ(SUCCESS, status);
    ASSERT_EQ(realityVector.size(), 2);
    RealityDataPtr realityData = realityVector[0];
    ASSERT_TRUE(realityData.IsValid());

    ASSERT_EQ(realityData->GetIdentifier(), "af3c43a9-1797-4765-a7c3-6f1cd6706fa9");
    ASSERT_EQ(realityData->GetOrganizationId(), "e82a584b-9fae-409f-9581-fd154f7b9ef9");
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
    ASSERT_EQ(realityData->GetFootprintString(), "{\"Coordinates\": [{\"Long\": 24.782875700, \"Lat\": 59.922488700},{\"Long\": 25.254484800, \"Lat\": 59.922488700},{\"Long\": 25.254484800, \"Lat\": 60.297838900},{\"Long\": 24.782875700, \"Lat\": 60.297838900},{\"Long\": 24.782875700, \"Lat\": 59.922488700}]}");
    ASSERT_EQ(realityData->GetThumbnailDocument(), "Helsinki/thumbnail.jpg");
    ASSERT_EQ(realityData->GetMetadataUrl(), "www.bigTest.com");
    ASSERT_EQ(realityData->GetUltimateId(), "uId");
    ASSERT_EQ(realityData->GetUltimateSite(), "www.bigTest.com/1");
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
    ASSERT_TRUE(std::abs(range.low.x - 24.7828757) < 0.000000001);
    ASSERT_TRUE(std::abs(range.low.y - 59.9224887) < 0.000000001);
    ASSERT_TRUE(std::abs(range.high.x - 25.2544848) < 0.000000001);
    ASSERT_TRUE(std::abs(range.high.y - 60.2978389) < 0.000000001);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityConversionTestFixture, RealityDataToJson)
	{
	bvector<RealityDataPtr> realityVector = bvector<RealityDataPtr>();
	StatusInt status = RealityConversionTools::JsonToRealityData(s_RealityDataJSONString, &realityVector);
	BeAssert(SUCCESS == status);

	Utf8String entityJson("{");
	entityJson.append(RealityConversionTools::RealityDataToJson(*realityVector[0], true, true));
	entityJson.append("}");
	
	Json::Value root(Json::objectValue);
	if(Json::Reader::Parse(s_RealityDataJSONString, root))
		{
		const Json::Value expectedValue = root["instances"][0]["properties"];
		Json::Value valueUnderTest(Json::objectValue);

		bool parseStatus = Json::Reader::Parse(entityJson, valueUnderTest);
		EXPECT_TRUE(parseStatus);

		for (const auto& memberName : expectedValue.getMemberNames())
			{
			//std::cerr << "[          ] memberName = " << memberName << " val: " << expectedValue[memberName].ToString() << " <=> " << valueUnderTest[memberName].ToString() << std::endl;
			EXPECT_TRUE(valueUnderTest.isMember(memberName));
			EXPECT_TRUE(expectedValue[memberName] == valueUnderTest[memberName]);
			}

		}

	}

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityConversionTestFixture, JsonToRealityDatabmap)
	{
	bmap<Utf8String, RealityDataPtr> realityMap = bmap<Utf8String, RealityDataPtr>();
	StatusInt status = RealityConversionTools::JsonToRealityData(s_RealityDataJSONString, &realityMap);
	ASSERT_EQ(SUCCESS, status);

	ASSERT_THAT(realityMap, Contains(Key("Helsinki")));
	ASSERT_THAT(realityMap, Contains(Key("Helsinki2")));
	}


//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityConversionTestFixture, JsonToSpatialEntitybmap)
	{
	bmap<Utf8String, SpatialEntityPtr> spatialMap;

	StatusInt status = RealityConversionTools::JsonToSpatialEntity(s_TwoSpatialEntityJSONString, &spatialMap);

	ASSERT_EQ(SUCCESS, status);

	ASSERT_THAT(spatialMap, Contains(Key("N38W093")));
	ASSERT_THAT(spatialMap, Contains(Key("N39W093")));

	}

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityConversionTestFixture, BadJsonToSpatialEntitybmap)
	{
	bmap<Utf8String, SpatialEntityPtr> spatialMap;

	StatusInt status = RealityConversionTools::JsonToSpatialEntity("BADJSON", &spatialMap);

	ASSERT_EQ(ERROR, status);

	}

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityConversionTestFixture, BadJsonToSpatialEntityDataSource)
	{
	bvector<SpatialEntityDataSourcePtr> dataSourceVector;

	StatusInt status = RealityConversionTools::JsonToSpatialEntityDataSource("BADJSON", &dataSourceVector);

	ASSERT_EQ(ERROR, status);
	}

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityConversionTestFixture, JsonToSpatialEntityServer)
	{
	bvector<SpatialEntityServerPtr> entityServersVector;

	StatusInt status = RealityConversionTools::JsonToSpatialEntityServer(s_SpatialEntityServerJSONString, &entityServersVector);

	ASSERT_EQ(SUCCESS, status);

	// Verify both SpatialEntityServer were parsed
	ASSERT_EQ(entityServersVector.size(), 2);
	ASSERT_STREQ(entityServersVector[0]->GetId().c_str(), "1");
	ASSERT_STREQ(entityServersVector[1]->GetId().c_str(), "2");

	// Only verify the first one was correctly parsed
	auto entityServerUnderTest = entityServersVector[0];
	ASSERT_STREQ(entityServerUnderTest->GetProtocol().c_str(), "ftp");
	ASSERT_TRUE(entityServerUnderTest->IsStreamed());
	ASSERT_STREQ(entityServerUnderTest->GetLoginKey().c_str(), "myLoginKey");
	ASSERT_STREQ(entityServerUnderTest->GetLoginMethod().c_str(), "myLoginMethod");
	ASSERT_STREQ(entityServerUnderTest->GetRegistrationPage().c_str(), "http://www.example.com/register");
	ASSERT_STREQ(entityServerUnderTest->GetOrganisationPage().c_str(), "http://www.example.com/organisation");
	ASSERT_STREQ(entityServerUnderTest->GetName().c_str(), "GeoGratis");
	ASSERT_STREQ(entityServerUnderTest->GetUrl().c_str(), "ftp://ftp.geogratis.gc.ca/pub/nrcan_rncan/elevation/cdem_mnec/");
	ASSERT_STREQ(entityServerUnderTest->GetContactInfo().c_str(), "mycontact@example.com");
	ASSERT_STREQ(entityServerUnderTest->GetFees().c_str(), "Over 9000");
	ASSERT_STREQ(entityServerUnderTest->GetLegal().c_str(), "Better Call Saul");
	ASSERT_STREQ(entityServerUnderTest->GetAccessConstraints().c_str(), "No one");
	ASSERT_TRUE(entityServerUnderTest->IsOnline());
	ASSERT_STREQ(entityServerUnderTest->GetLastCheck().ToString().c_str(), "2017-02-02T00:28:22.000");
	ASSERT_STREQ(entityServerUnderTest->GetLastTimeOnline().ToString().c_str(), "2017-02-03T00:28:22.000");
	ASSERT_EQ(entityServerUnderTest->GetLatency(), 9000);
	ASSERT_EQ(entityServerUnderTest->GetMeanReachabilityStats(), 5);
	ASSERT_STREQ(entityServerUnderTest->GetState().c_str(), "myState");
	ASSERT_STREQ(entityServerUnderTest->GetType().c_str(), "sftp");

	}

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityConversionTestFixture, BadJsonToSpatialEntityServer)
	{
	bvector<SpatialEntityServerPtr> entityServersVector;

	StatusInt status = RealityConversionTools::JsonToSpatialEntityServer("BADJSON", &entityServersVector);

	ASSERT_EQ(ERROR, status);
	}

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityConversionTestFixture, JsonToSpatialEntityMetadata)
	{
	bvector<SpatialEntityMetadataPtr> entityMetadataVector;

	StatusInt status = RealityConversionTools::JsonToSpatialEntityMetadata(s_SpatialEntityMetadataJSONString, &entityMetadataVector);

	ASSERT_EQ(SUCCESS, status);

	// Verify both SpatialEntityMetadata were parsed
	ASSERT_EQ(entityMetadataVector.size(), 2);
	ASSERT_STREQ(entityMetadataVector[0]->GetId().c_str(), "1");
	ASSERT_STREQ(entityMetadataVector[1]->GetId().c_str(), "2");

	auto metadataUnderTest = entityMetadataVector[0];

	ASSERT_STREQ(metadataUnderTest->GetMetadataUrl().c_str(), "http://www.example.com/metadata");
	ASSERT_STREQ(metadataUnderTest->GetDisplayStyle().c_str(), "myDisplayStyle");
	ASSERT_STREQ(metadataUnderTest->GetDescription().c_str(), "some big description");
	ASSERT_STREQ(metadataUnderTest->GetContactInfo().c_str(), "my contact");
	ASSERT_STREQ(metadataUnderTest->GetKeywords().c_str(), "keyword1;keyword2");
	ASSERT_STREQ(metadataUnderTest->GetLegal().c_str(), "© OpenStreetMap contributors");
	ASSERT_STREQ(metadataUnderTest->GetTermsOfUse().c_str(), "Everyone");
	ASSERT_STREQ(metadataUnderTest->GetLineage().c_str(), "ok");
	ASSERT_STREQ(metadataUnderTest->GetProvenance().c_str(), "somewhere");

	}

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityConversionTestFixture, BadJsonToSpatialEntityMetadata)
	{
	bvector<SpatialEntityMetadataPtr> entityMetadataVector;

	StatusInt status = RealityConversionTools::JsonToSpatialEntityMetadata("BADJSON", &entityMetadataVector);

	ASSERT_EQ(ERROR, status);
	}

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityConversionTestFixture, PackageFileToDownloadOrder)
	{
	WString parseError;
	
	Utf8String samplePath(GetDirectory());
	samplePath.append(".\\TestData\\RealityPlatform\\RealityDataPackageSample.xml");
	BeFileName fileName(samplePath);
	auto linkFile = RealityConversionTools::PackageFileToDownloadOrder(fileName, &parseError);

	//for (RealityDataDownload::mirrorWSistersVector link : linkFile)
	//	{
	//	std::wcerr << "[          ] " << "First Level" << "\n";
	//	for (RealityDataDownload::sisterFileVector sister : link)
	//		{
	//		std::wcerr << "[          ] " << "Second Level" << "\n";
	//		for (RealityDataDownload::url_file_pair filePair : sister)
	//			{
	//			WString urlLink = L"";
	//			BeStringUtilities::CurrentLocaleCharToWChar(urlLink, filePair.first.c_str());
	//			std::wcerr << "[          ] " << urlLink << "     " << filePair.second << "\n";
	//			}
	//		}
	//	}


	// Imagery Group
	// Imagery Data 1
	EXPECT_EQ(linkFile[0][0][0].m_url, "http://uri1.com/");
	EXPECT_EQ(linkFile[0][0][1].m_url, "http://uri1.com/url1.html");
	EXPECT_EQ(linkFile[0][0][2].m_url, "http://uri1.com/url2.html");
	EXPECT_EQ(linkFile[0][0][3].m_url, "http://uri1.com/url3.html");

	EXPECT_EQ(linkFile[0][0][0].m_filePath, L"uri1.com");
	EXPECT_EQ(linkFile[0][0][1].m_filePath, L"url1.html");
	EXPECT_EQ(linkFile[0][0][2].m_filePath, L"url2.html");
	EXPECT_EQ(linkFile[0][0][3].m_filePath, L"url3.html");

	// Imagery data 2 Multiband test
	EXPECT_EQ(linkFile[1][0][0].m_url, "https://s3-us-west-2.amazonaws.com/landsat-pds/L8/013/028/LC80130282015007LGN00/LC80130282015007LGN00_B4.TIF");
	EXPECT_EQ(linkFile[1][0][1].m_url, "https://s3-us-west-2.amazonaws.com/landsat-pds/L8/013/028/LC80130282015007LGN00/LC80130282015007LGN00_B2.TIF");
	EXPECT_EQ(linkFile[1][0][2].m_url, "https://s3-us-west-2.amazonaws.com/landsat-pds/L8/013/028/LC80130282015007LGN00/LC80130282015007LGN00_B3.TIF");
	EXPECT_EQ(linkFile[1][0][3].m_url, "https://s3-us-west-2.amazonaws.com/landsat-pds/L8/013/028/LC80130282015007LGN00/LC80130282015007LGN00_B8.TIF");

	EXPECT_EQ(linkFile[1][0][0].m_filePath, L"LC80130282015007LGN00_B4.TIF");
	EXPECT_EQ(linkFile[1][0][1].m_filePath, L"LC80130282015007LGN00_B2.TIF");
	EXPECT_EQ(linkFile[1][0][2].m_filePath, L"LC80130282015007LGN00_B3.TIF");
	EXPECT_EQ(linkFile[1][0][3].m_filePath, L"LC80130282015007LGN00_B8.TIF");

	// Terrain Group
	// Terrain data 1
	EXPECT_EQ(linkFile[2][0][0].m_url, "http://uri28");
	EXPECT_EQ(linkFile[2][0][1].m_url, "http://uri82");

	EXPECT_EQ(linkFile[2][0][0].m_filePath, L"uri28");
	EXPECT_EQ(linkFile[2][0][1].m_filePath, L"uri82");

	// Model group
	// Model data 1
	EXPECT_EQ(linkFile[3][0][0].m_url, "http://uri10");
	EXPECT_EQ(linkFile[3][0][1].m_url, "http://uri28");

	EXPECT_EQ(linkFile[3][0][0].m_filePath, L"uri10");
	EXPECT_EQ(linkFile[3][0][1].m_filePath, L"uri28");

	// Pinned group
	// Pinned data 1
	EXPECT_EQ(linkFile[4][0][0].m_url, "http://uri19");
	EXPECT_EQ(linkFile[4][0][1].m_url, "http://uri55");
	EXPECT_EQ(linkFile[4][0][2].m_url, "http://uri56");
	EXPECT_EQ(linkFile[4][0][3].m_url, "http://uri57");

	EXPECT_EQ(linkFile[4][0][0].m_filePath, L"uri19");
	EXPECT_EQ(linkFile[4][0][1].m_filePath, L"uri55");
	EXPECT_EQ(linkFile[4][0][2].m_filePath, L"uri56");
	EXPECT_EQ(linkFile[4][0][3].m_filePath, L"uri57");

	// Pinned data 2 source 1
	EXPECT_EQ(linkFile[5][0][0].m_url, "http://uri22");
	EXPECT_EQ(linkFile[5][0][0].m_filePath, L"uri22");

	// Pinned data 2 source 2
	EXPECT_EQ(linkFile[5][1][0].m_url, "http://uri23");
	EXPECT_EQ(linkFile[5][1][1].m_url, "http://uri67");

	EXPECT_EQ(linkFile[5][1][0].m_filePath, L"uri23");
	EXPECT_EQ(linkFile[5][1][1].m_filePath, L"uri67");
	
	// Pinned data 2 source 3
	EXPECT_EQ(linkFile[5][2][0].m_url, "http://uri24");
	EXPECT_EQ(linkFile[5][2][1].m_url, "http://uri70");
	EXPECT_EQ(linkFile[5][2][2].m_url, "http://uri71");
	EXPECT_EQ(linkFile[5][2][3].m_url, "http://uri72");

	EXPECT_EQ(linkFile[5][2][0].m_filePath, L"uri24");
	EXPECT_EQ(linkFile[5][2][1].m_filePath, L"uri70");
	EXPECT_EQ(linkFile[5][2][2].m_filePath, L"uri71");
	EXPECT_EQ(linkFile[5][2][3].m_filePath, L"uri72");

	// Undefined Group
	// Undefined data 1
	EXPECT_EQ(linkFile[6][0][0].m_url, "http://uri37");
	EXPECT_EQ(linkFile[6][0][0].m_filePath, L"uri37");
	}


#if 0

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityConversionTestFixture, PackageToDownloadOrder)
	{
	auto dataPackage = RealityPackage::RealityDataPackage::Create("MyDataOrder");
	auto linkFile = RealityConversionTools::PackageToDownloadOrder(dataPackage);


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