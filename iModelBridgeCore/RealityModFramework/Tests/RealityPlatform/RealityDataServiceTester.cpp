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

	EXPECT_FALSE(documentUT.IsAzureRedirectionPossible());

	documentUT.SetAzureRedirectionPossible(true);
	EXPECT_TRUE(documentUT.IsAzureRedirectionPossible());
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

/*
TEST_F(RealityDataServiceFixture, RealityDataDocumentContentByIdRequestHttpRequestWithRedirect)
	{
	auto documentUT = RealityDataDocumentContentByIdRequest("MyIdentifier");
	documentUT.SetAzureRedirectionPossible(true);
	auto requestString = documentUT.GetHttpRequestString();

	EXPECT_STREQ(requestString.c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/Document/MyIdentifier/$file");
	}

*/