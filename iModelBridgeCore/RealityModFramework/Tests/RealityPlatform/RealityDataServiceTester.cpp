//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/RealityPlatform/RealityDataServiceTester.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <Bentley/BeTest.h>
#include <RealityPlatform/RealityDataService.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

using ::testing::NiceMock;
using ::testing::_;
using ::testing::Eq;
using ::testing::Invoke;

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//=====================================================================================
struct ErrorClass
	{
	MOCK_CONST_METHOD2(errorCallBack, void(Utf8String basicMessage, const RawServerResponse& rawResponse));

	ErrorClass()
		{

		}
	};

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//=====================================================================================
struct MockWSGRequest : WSGRequest
	{
	MockWSGRequest() : WSGRequest()
	{}

	MOCK_CONST_METHOD1(SetCertificatePath, void(Utf8String certificate));
	MOCK_CONST_METHOD5(PerformAzureRequest, void(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry));
	MOCK_CONST_METHOD4(PrepareRequest, CURL*(const WSGURL& wsgRequest, RawServerResponse& responseString, bool verifyPeer, BeFile* file));
	MOCK_CONST_METHOD5(_PerformRequest, void(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry));
	MOCK_CONST_METHOD5(PerformRequest, void(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry));
	};

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//! RealityDataServiceFixture
//=====================================================================================
class RealityDataServiceFixture : public testing::Test
	{
public:
	static RealityDataService* s_realityDataService;
	static ErrorClass* s_errorClass;
	static NiceMock<MockWSGRequest>* s_mockWSGInstance;

	static void SetUpTestCase()
		{
		s_mockWSGInstance = new NiceMock<MockWSGRequest>();
		s_realityDataService = new RealityDataService();
		s_realityDataService->SetServerComponents("myserver.com", "9.9", "myRepo", "mySchema", "zz:\\mycertificate.pfx", "myProjectID");
		s_errorClass = new ErrorClass();
        s_realityDataService->SetErrorCallback(RealityDataServiceFixture::mockErrorCallBack);
		}

	static void TearDownTestCase()
		{
		delete s_realityDataService;
		s_realityDataService = nullptr;
		delete s_errorClass;
		s_errorClass = nullptr;
		delete s_mockWSGInstance;
		s_mockWSGInstance = nullptr;
		}

	static void mockErrorCallBack(Utf8String basicMessage, const RawServerResponse& rawResponse)
		{
		if (s_errorClass != nullptr)
			{
			s_errorClass->errorCallBack(basicMessage, rawResponse);
			}
		}

	};

RealityDataService* RealityDataServiceFixture::s_realityDataService = nullptr;
ErrorClass* RealityDataServiceFixture::s_errorClass = nullptr;
NiceMock<MockWSGRequest>* RealityDataServiceFixture::s_mockWSGInstance = nullptr;

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, SetServerComponents)
	{
	// Test the SetServerComponents called in SetUpTestcase
	EXPECT_STREQ(s_realityDataService->GetServerName().c_str(), "myserver.com");
	EXPECT_STREQ(s_realityDataService->GetWSGProtocol().c_str(), "9.9");
	EXPECT_STREQ(s_realityDataService->GetRepoName().c_str(), "myRepo");
	EXPECT_STREQ(s_realityDataService->GetSchemaName().c_str(), "mySchema");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataUrl)
	{
	RealityDataUrl dataURL{};
	EXPECT_STREQ(dataURL.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataEnterpriseStatRequest)
	{
	RealityDataEnterpriseStatRequest dataRequest("myUltimateID");
	EXPECT_STREQ(dataRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/EnterpriseStat/myUltimateID");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataByIdRequest)
	{
	RealityDataByIdRequest dataRequest("myIdentifierID");
	EXPECT_STREQ(dataRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityData/myIdentifierID");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataProjectRelationshipByProjectIdRequest)
	{
	RealityDataProjectRelationshipByProjectIdRequest dataRequest("myIdentifierID");
	EXPECT_STREQ(dataRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityDataRelationship?$filter=ProjectId+eq+'myIdentifierID'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataProjectRelationshipByRealityDataIdRequest)
	{
	RealityDataProjectRelationshipByRealityDataIdRequest dataRequest("myIdentifierID");
	EXPECT_STREQ(dataRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityDataRelationship?$filter=RealityDataId+eq+'myIdentifierID'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataFolderByIdRequest)
	{
	RealityDataFolderByIdRequest dataRequest("myIdentifierID");
	EXPECT_STREQ(dataRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/Folder/myIdentifierID");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataDocumentByIdRequest)
	{
	RealityDataDocumentByIdRequest dataRequest("myIdentifierID");
	EXPECT_STREQ(dataRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/Document/myIdentifierID");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, AzureHandshakeDefaultConstructor)
	{
	AzureHandshake azureHandshake{};
	azureHandshake.SetId("mySuperID");
	EXPECT_STREQ(azureHandshake.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityData/mySuperID/FileAccess.FileAccessKey?$filter=Permissions+eq+'Read'&api.singleurlperinstance=true");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, AzureHandshakeConstructorWithParameters)
	{
	AzureHandshake azureHandshake("mySuperID~2Fmysourcepath", true);
	EXPECT_STREQ(azureHandshake.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityData/mySuperID/FileAccess.FileAccessKey?$filter=Permissions+eq+'Write'&api.singleurlperinstance=true");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, AzureParseResponse)
	{
	Utf8String azureJSON = R"(
		{
		"instances": 
			[
				{
					"properties": 
						{
							"Name": "myName",
							"Url": "https://myserver.com/?myToken&se=2013-03-01T16%3A20%3A00Z"
						}
				}
			]
		}
	
	)";

	AzureHandshake azureHandshake("mySuperID~2Fmysourcepath", true);
	Utf8String azureServer;
	Utf8String azureToken;
	int64_t tokenTimer;

	DateTime tokenExpiry;
	int64_t expectedTimer;
	// 10 mins before the time we specified in the JSON
	DateTime::FromString(tokenExpiry,"2013-03-01T16:10:00Z");
	tokenExpiry.ToUnixMilliseconds(expectedTimer);

	BentleyStatus status = azureHandshake.ParseResponse(azureJSON, azureServer, azureToken, tokenTimer);
	EXPECT_EQ(status, SUCCESS);
	EXPECT_EQ(tokenTimer, expectedTimer);
	EXPECT_STREQ(azureToken.c_str(), "myToken&se=2013-03-01T16%3A20%3A00Z");
	EXPECT_STREQ(azureServer.c_str(), "https://myserver.com/");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, AzureParseBadResponse)
	{
	Utf8String azureJSON = R"(
		{
		"errormessage": "this is an error"
		}
	
	)";

	AzureHandshake azureHandshake("mySuperID~2Fmysourcepath", true);
	Utf8String azureServer;
	Utf8String azureToken;
	int64_t tokenTimer;

	BentleyStatus status = azureHandshake.ParseResponse(azureJSON, azureServer, azureToken, tokenTimer);
	EXPECT_EQ(status, ERROR);
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataDocumentContentByIdRequestBasic)
	{
	auto documentUT = RealityDataDocumentContentByIdRequest("MyIdentifier");

	EXPECT_STREQ(documentUT.GetId().c_str(), "MyIdentifier");

	documentUT.ChangeInstanceId("MyNewID");
	EXPECT_STREQ(documentUT.GetId().c_str(), "MyNewID");

	EXPECT_TRUE(documentUT.IsAzureRedirectionPossible());

	documentUT.SetAzureRedirectionPossible(false);
	EXPECT_FALSE(documentUT.IsAzureRedirectionPossible());

	EXPECT_FALSE(documentUT.IsAzureBlobRedirected());
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataDocumentContentByIdRequestHttpRequestWithoutRedirect)
	{
	auto documentUT = RealityDataDocumentContentByIdRequest("MyIdentifier");
	auto requestString = documentUT.GetHttpRequestString();

	EXPECT_STREQ(requestString.c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/Document/MyIdentifier/$file");

	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataDocumentContentByIdRequestHttpRequestWithRedirect)
	{

	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::OK;
		response.curlCode = CURLE_OK;
		response.responseCode = 200;
		response.body = R"(
			{
			"instances": 
				[
					{
						"properties": 
							{
								"Name": "myName",
								"Url": "https://redirected.server.com/?myToken&se=2013-03-01T16%3A20%3A00Z"
							}
					}
				]
			}
	
			)";
		}));


	auto documentUT = RealityDataDocumentContentByIdRequest("MyIdentifier/RootDocument.s3mx");
	documentUT.GetAzureRedirectionRequestUrl();
	auto requestString = documentUT.GetHttpRequestString();


	EXPECT_TRUE(documentUT.IsAzureBlobRedirected());
	EXPECT_STREQ(requestString.c_str(), "https://redirected.server.com//MyIdentifier/RootDocument.s3mx?myToken&se=2013-03-01T16%3A20%3A00Z");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataPagedRequestBasic)
	{
	RealityDataPagedRequest requestUT{};
	EXPECT_STREQ(requestUT.GetServerName().c_str(), "myserver.com");
	EXPECT_STREQ(requestUT.GetVersion().c_str(), "9.9");
	EXPECT_STREQ(requestUT.GetRepoId().c_str(), "myRepo");
	EXPECT_STREQ(requestUT.GetSchema().c_str(), "mySchema");

	EXPECT_STREQ(requestUT.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityData?$skip=0&$top=25");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataPagedRequestSetFilter)
	{
	RealityDataPagedRequest requestUT{};
	auto filter = RealityDataFilterCreator::FilterByName("MyName");
	requestUT.SetFilter(filter);

	EXPECT_STREQ(requestUT.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityData?$filter=Name+eq+'MyName'&$skip=0&$top=25");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataPagedRequestSetOrder)
	{
	RealityDataPagedRequest requestUT{};
	requestUT.SortBy(RealityDataField::Classification, true);

	EXPECT_STREQ(requestUT.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityData?$orderby=Classification+asc&$skip=0&$top=25");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataPagedRequestSetQuery)
	{
	RealityDataPagedRequest requestUT{};
	requestUT.SetQuery("someQuery");

	EXPECT_STREQ(requestUT.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityData?$skip=0&$top=25&query=someQuery");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataPagedRequestSetProject)
	{
	RealityDataPagedRequest requestUT{};
	requestUT.SetProject("ImportantProject");

	EXPECT_STREQ(requestUT.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityData?$skip=0&$top=25&project=ImportantProject");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataListByOrganizationPagedRequest)
	{
	RealityDataListByOrganizationPagedRequest requestUT("MyIdentifier",77,75);

	EXPECT_STREQ(requestUT.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityData?$filter=OrganizationId+eq+'MyIdentifier'&$skip=77&$top=75");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataProjectRelationshipByProjectIdPagedRequest)
	{
	RealityDataProjectRelationshipByProjectIdPagedRequest requestUT("MyIdentifier");

	EXPECT_STREQ(requestUT.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityDataRelationship?$filter=ProjectId+eq+'MyIdentifier'&$skip=0&$top=25");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataProjectRelationshipByRealityDataIdPagedRequest)
	{
	RealityDataProjectRelationshipByRealityDataIdPagedRequest requestUT("MyIdentifier");

	EXPECT_STREQ(requestUT.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityDataRelationship?$filter=RealityDataId+eq+'MyIdentifier'&$skip=0&$top=25");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
#if 0
TEST_F(RealityDataServiceFixture, AllRealityDataByRootId)
	{
	AllRealityDataByRootId requestUT("MyIdentifier");

	EXPECT_STREQ(requestUT.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityDataRelationship?$filter=RealityDataId+eq+'MyIdentifier'&$skip=0&$top=25");
	}
#endif

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterByName)
	{
	RDSFilter filter = RealityDataFilterCreator::FilterByName("MyName");
	EXPECT_STREQ(filter.ToString().c_str(), "Name+eq+'MyName'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterByClassification)
	{
	RDSFilter filter = RealityDataFilterCreator::FilterByClassification(RealityDataBase::Classification::MODEL);
	EXPECT_STREQ(filter.ToString().c_str(), "Classification+eq+'Model'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterBySize)
	{
	RDSFilter filter = RealityDataFilterCreator::FilterBySize(0, UINT64_MAX);
	EXPECT_STREQ(filter.ToString().c_str(), "Size+ge+0+and+Size+le+4294967295");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterSpatial)
	{
	auto footprint = bvector<GeoPoint2d>();
    footprint.emplace_back(GeoPoint2d::From(-92, 39));
    footprint.emplace_back(GeoPoint2d::From(-92, 38));
    footprint.emplace_back(GeoPoint2d::From(-93, 38));
    footprint.emplace_back(GeoPoint2d::From(-93, 39));
    footprint.emplace_back(GeoPoint2d::From(-92, 39));

	auto expectedFootprint = R"(polygon={\"points\":[[-92.000000,39.000000],[-92.000000,38.000000],[-93.000000,38.000000],[-93.000000,39.000000],[-92.000000,39.000000]], \"coordinate_system\":\"1555\"})";
	RDSFilter filter = RealityDataFilterCreator::FilterSpatial(footprint, 1555);
	EXPECT_STREQ(filter.ToString().c_str(), expectedFootprint);
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterByOwner)
	{
	RDSFilter filter = RealityDataFilterCreator::FilterByOwner("importantOwner@example.com");
	EXPECT_STREQ(filter.ToString().c_str(), "OwnedBy+eq+'importantOwner%40example%2Ecom'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterByCreationDate)
	{
	DateTime minDate(DateTime::Kind::Utc, 2000, 01, 01, 12, 00, 25, 200);
	DateTime maxDate(DateTime::Kind::Utc, 2016, 12, 31, 23, 59, 59, 999);

	RDSFilter filter = RealityDataFilterCreator::FilterByCreationDate(minDate, maxDate);
	EXPECT_STREQ(filter.ToString().c_str(), "CreatedTimestamp+ge+'2000-01-01T12:00:25.200Z'+and+CreatedTimestamp+le+'2016-12-31T23:59:59.999Z'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterByModificationDate)
	{
	DateTime minDate(DateTime::Kind::Utc, 2000, 01, 01, 12, 00, 25, 200);
	DateTime maxDate(DateTime::Kind::Utc, 2016, 12, 31, 23, 59, 59, 999);

	RDSFilter filter = RealityDataFilterCreator::FilterByModificationDate(minDate, maxDate);
	EXPECT_STREQ(filter.ToString().c_str(), "ModifiedTimestamp+ge+'2000-01-01T12:00:25.200Z'+and+ModifiedTimestamp+le+'2016-12-31T23:59:59.999Z'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterByVisibility)
	{
	RDSFilter filter = RealityDataFilterCreator::FilterVisibility(RealityDataBase::Visibility::PERMISSION);
	EXPECT_STREQ(filter.ToString().c_str(), "Visibility+eq+'PERMISSION'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterByResolution)
	{
	RDSFilter filter = RealityDataFilterCreator::FilterByResolution(0, 200, true);
	EXPECT_STREQ(filter.ToString().c_str(), "ResolutionInMeters+ge+'0.000000'+and+ResolutionInMeters+le+'200.000000'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterByAccuracy)
	{
	RDSFilter filter = RealityDataFilterCreator::FilterByAccuracy(0, 200, true);
	EXPECT_STREQ(filter.ToString().c_str(), "AccuracyInMeters+ge+'0.000000'+and+AccuracyInMeters+le+'200.000000'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterByType)
	{
	RDSFilter filter = RealityDataFilterCreator::FilterByType("MyType");
	EXPECT_STREQ(filter.ToString().c_str(), "Type+eq+'MyType'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterByDataset)
	{
	RDSFilter filter = RealityDataFilterCreator::FilterByDataset("SomeDataset");
	EXPECT_STREQ(filter.ToString().c_str(), "Dataset+eq+'SomeDataset'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterByGroup)
	{
	RDSFilter filter = RealityDataFilterCreator::FilterByGroup("YourGroup");
	EXPECT_STREQ(filter.ToString().c_str(), "Group+eq+'YourGroup'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterRelationshipByRealityDataId)
	{
	RDSFilter filter = RealityDataFilterCreator::FilterRelationshipByRealityDataId("MyRealityDataID");
	EXPECT_STREQ(filter.ToString().c_str(), "RealityDataId+eq+'MyRealityDataID'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterRelationshipByProjectId)
	{
	RDSFilter filter = RealityDataFilterCreator::FilterRelationshipByProjectId("MyProjectID");
	EXPECT_STREQ(filter.ToString().c_str(), "ProjectId+eq+'MyProjectID'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, GroupFiltersAND)
	{
	bvector<RDSFilter> filtersVector;
	filtersVector.emplace_back(RealityDataFilterCreator::FilterRelationshipByRealityDataId("MyRealityDataID"));
	filtersVector.emplace_back(RealityDataFilterCreator::FilterRelationshipByProjectId("MyProjectID"));
	auto filter = RealityDataFilterCreator::GroupFiltersAND(filtersVector);
	EXPECT_STREQ(filter.ToString().c_str(), "RealityDataId+eq+'MyRealityDataID'+and+ProjectId+eq+'MyProjectID'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, GroupFiltersOR)
	{
	bvector<RDSFilter> filtersVector;
	filtersVector.emplace_back(RealityDataFilterCreator::FilterRelationshipByRealityDataId("MyRealityDataID"));
	filtersVector.emplace_back(RealityDataFilterCreator::FilterRelationshipByProjectId("MyProjectID"));
	auto filter = RealityDataFilterCreator::GroupFiltersOR(filtersVector);
	EXPECT_STREQ(filter.ToString().c_str(), "RealityDataId+eq+'MyRealityDataID'+or+ProjectId+eq+'MyProjectID'");
	}

