//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/RealityPlatform/RealityDataServiceTester.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <Bentley/BeTest.h>
#include <RealityPlatform/RealityDataService.h>
#include "../../RealityPlatform/RealityDataServiceInternal.h"
#include "../Common/RealityModFrameworkTestsCommon.h"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

using ::testing::NiceMock;
using ::testing::_;
using ::testing::Eq;
using ::testing::Invoke;
using ::testing::StrEq;
using ::testing::HasSubstr;
using ::testing::Matcher;
using ::testing::Mock;

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//! RealityDataServiceFixture
//=====================================================================================
class RealityDataServiceFixture : public MockRealityDataServiceFixture
	{
	};

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
	EXPECT_STREQ(s_realityDataService->GetProjectId().c_str(), "myProjectID");
	EXPECT_TRUE(s_realityDataService->AreParametersSet());

    s_realityDataService->SetProjectId("newProjectID");
    EXPECT_STREQ(s_realityDataService->GetProjectId().c_str(), "newProjectID");

    // Reset the project ID for all the other test
    s_realityDataService->SetProjectId("myProjectID");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataUrl)
	{
	RealityDataUrl dataURL{};
	EXPECT_STREQ(dataURL.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema");
	EXPECT_STREQ(dataURL.GetServerName().c_str(), "myserver.com");
	EXPECT_STREQ(dataURL.GetVersion().c_str(), "9.9");
	EXPECT_STREQ(dataURL.GetRepoId().c_str(), "myRepo");
	EXPECT_STREQ(dataURL.GetSchema().c_str(), "mySchema");
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
TEST_F(RealityDataServiceFixture, RealityDataListByOrganizationPagedRequestEmptyID)
	{
	RealityDataListByOrganizationPagedRequest requestUT("",77,75);

	auto filter = RealityDataFilterCreator::FilterByName("MyName");
	requestUT.SetFilter(filter);
	requestUT.SortBy(RealityDataField::AccuracyInMeters, false);
	requestUT.SetQuery("SomeQuery");
	requestUT.SetProject("MyProject");	
	
	auto requestString = requestUT.GetHttpRequestString();
	//e82a584b%2D9fae%2D409f%2D9581%2Dfd154f7b9ef9 <= Bentley OrganizationId
	EXPECT_THAT(requestString.c_str(), HasSubstr("https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityData?$filter=OrganizationId+eq+'e82a584b%2D9fae%2D409f%2D9581%2Dfd154f7b9ef9'"));
	EXPECT_THAT(requestString.c_str(), HasSubstr("+and+Name+eq+'MyName'"));
	EXPECT_THAT(requestString.c_str(), HasSubstr("&$orderby=AccuracyInMeters+desc"));
	EXPECT_THAT(requestString.c_str(), HasSubstr("&$skip=77&$top=75"));
	EXPECT_THAT(requestString.c_str(), HasSubstr("&query=SomeQuery"));
	EXPECT_THAT(requestString.c_str(), HasSubstr("&project=MyProject"));
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataProjectRelationshipByProjectIdPagedRequest)
	{
	RealityDataProjectRelationshipByProjectIdPagedRequest requestUT("MyIdentifier");

	auto filter = RealityDataFilterCreator::FilterByName("MyName");
	requestUT.SetFilter(filter);
	requestUT.SortBy(RealityDataField::Copyright, false);
	requestUT.SetQuery("SomeQuery");
	requestUT.SetProject("MyProject");	

	auto requestString = requestUT.GetHttpRequestString();
	EXPECT_THAT(requestString.c_str(), HasSubstr("https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityDataRelationship?$filter=RelationType+eq+'CONNECT-Project'+and+RelatedId+eq+'MyIdentifier'"));
	EXPECT_THAT(requestString.c_str(), HasSubstr("+and+Name+eq+'MyName'"));
	EXPECT_THAT(requestString.c_str(), HasSubstr("&$orderby=Copyright+desc"));
	EXPECT_THAT(requestString.c_str(), HasSubstr("&$skip=0&$top=25"));
	EXPECT_THAT(requestString.c_str(), HasSubstr("&query=SomeQuery"));
	EXPECT_THAT(requestString.c_str(), HasSubstr("&project=MyProject"));
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataProjectRelationshipByRealityDataIdPagedRequest)
	{
	RealityDataProjectRelationshipByRealityDataIdPagedRequest requestUT("MyIdentifier");

	auto filter = RealityDataFilterCreator::FilterByName("MyName");
	requestUT.SetFilter(filter);
	requestUT.SortBy(RealityDataField::Dataset, false);
	requestUT.SetQuery("SomeQuery");
	requestUT.SetProject("MyProject");	

	auto requestString = requestUT.GetHttpRequestString();

	EXPECT_THAT(requestString.c_str(), HasSubstr("https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityDataRelationship?$filter=RelationType+eq+'CONNECT-Project'+and+RealityDataId+eq+'MyIdentifier'"));
	EXPECT_THAT(requestString.c_str(), HasSubstr("+and+Name+eq+'MyName'"));
	EXPECT_THAT(requestString.c_str(), HasSubstr("&$orderby=Dataset+desc"));
	EXPECT_THAT(requestString.c_str(), HasSubstr("&$skip=0&$top=25"));
	EXPECT_THAT(requestString.c_str(), HasSubstr("&query=SomeQuery"));
	EXPECT_THAT(requestString.c_str(), HasSubstr("&project=MyProject"));
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, AllRealityDataByRootId)
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

	AllRealityDataByRootId requestUT("MyIdentifier");

    // In normal use case, Request will call GetAzureRedirectionRequestUrl() to get the azure server.
    // In our case, we need to manually call it before GetHttpRequestString()
    requestUT.GetAzureRedirectionRequestUrl();

	EXPECT_STREQ(requestUT.GetHttpRequestString().c_str(), "https://redirected.server.com/?myToken&se=2013-03-01T16%3A20%3A00Z&restype=container&comp=list");

    requestUT.SetMarker("SomeMarker");

    EXPECT_STREQ(requestUT.GetHttpRequestString().c_str(), "https://redirected.server.com/?myToken&se=2013-03-01T16%3A20%3A00Z&restype=container&comp=list&marker=SomeMarker");
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, AllRealityDataByRootIdFilter)
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

	AllRealityDataByRootId requestUT("MyIdentifier\\SomeStuffs~2FOtherStuffs");

    // In normal use case, Request will call GetAzureRedirectionRequestUrl() to get the azure server.
    // In our case, we need to manually call it before GetHttpRequestString()
    requestUT.GetAzureRedirectionRequestUrl();

	EXPECT_STREQ(requestUT.GetHttpRequestString().c_str(), "https://redirected.server.com/?myToken&se=2013-03-01T16%3A20%3A00Z&restype=container&comp=list&prefix=SomeStuffs/OtherStuffs");
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataCreateRequestFromString)
	{
	RealityDataCreateRequest requestUT("MyIdentifier", "\"SomeProperty\": \"myProperty\"");

	EXPECT_STREQ(requestUT.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityData");
	Utf8String expectedPayload = R"({"instance":{"instanceId":"MyIdentifier", "className": "RealityData","schemaName":"S3MX", "properties": {"SomeProperty": "myProperty"}}})";

	EXPECT_EQ(requestUT.GetRequestType(), WSGURL::HttpRequestType::POST_Request);

	auto payload = requestUT.GetRequestPayload();
	EXPECT_STREQ(payload.c_str(), expectedPayload.c_str());

	auto headers = requestUT.GetRequestHeader();
	EXPECT_CONTAINS(headers, "Content-Type: application/json");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataCreateRequestFromRealityData)
	{
	auto realityData = RealityData::Create();
	realityData->SetIdentifier("MyIdentifier");
	RealityDataCreateRequest requestUT(*realityData);

	EXPECT_STREQ(requestUT.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityData");
	Utf8String expectedPayload = R"({"instance":{"instanceId":"MyIdentifier", "className": "RealityData","schemaName":"S3MX", "properties": {"Id" : "MyIdentifier"}}})";
	auto payload = requestUT.GetRequestPayload();
	EXPECT_STREQ(payload.c_str(), expectedPayload.c_str());
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataChangeRequestFromString)
	{
	RealityDataChangeRequest requestUT("MyIdentifier", "\"SomeProperty\": \"myProperty\"");

	EXPECT_STREQ(requestUT.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityData/MyIdentifier");
	Utf8String expectedPayload = R"({"instance":{"instanceId":"MyIdentifier", "className": "RealityData","schemaName":"S3MX", "properties": {"SomeProperty": "myProperty"}}})";

	EXPECT_EQ(requestUT.GetRequestType(), WSGURL::HttpRequestType::POST_Request);

	auto payload = requestUT.GetRequestPayload();
	EXPECT_STREQ(payload.c_str(), expectedPayload.c_str());

	auto headers = requestUT.GetRequestHeader();
	EXPECT_CONTAINS(headers, "Content-Type: application/json");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataChangeRequestFromRealityData)
	{
	auto realityData = RealityData::Create();
	realityData->SetIdentifier("MyIdentifier");
	RealityDataChangeRequest requestUT(*realityData);

	EXPECT_STREQ(requestUT.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityData/MyIdentifier");
	Utf8String expectedPayload = R"({"instance":{"instanceId":"MyIdentifier", "className": "RealityData","schemaName":"S3MX", "properties": {"Id" : "MyIdentifier"}}})";
	auto payload = requestUT.GetRequestPayload();
	EXPECT_STREQ(payload.c_str(), expectedPayload.c_str());
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataRelationshipCreateRequest)
	{
	RealityDataRelationshipCreateRequest requestUT("MyIdentifier", "MYProjectID");

	EXPECT_STREQ(requestUT.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityDataRelationship");
	Utf8String expectedPayload = R"({"instance":{"className": "RealityDataRelationship","schemaName":"S3MX", "properties": { "RelationType" : "CONNECT-Project", "RelatedId" : "MYProjectID", "RealityDataId": "MyIdentifier"}}})";

	EXPECT_EQ(requestUT.GetRequestType(), WSGURL::HttpRequestType::POST_Request);

	auto payload = requestUT.GetRequestPayload();
	EXPECT_STREQ(payload.c_str(), expectedPayload.c_str());

	auto headers = requestUT.GetRequestHeader();
	EXPECT_CONTAINS(headers, "Content-Type: application/json");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataRelationshipDelete)
	{
	RealityDataRelationshipDelete requestUT("MyIdentifier", "MYProjectID");

	EXPECT_STREQ(requestUT.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityDataRelationship/MyIdentifier~2FMYProjectID");
	EXPECT_EQ(requestUT.GetRequestType(), WSGURL::HttpRequestType::DELETE_Request);

	auto headers = requestUT.GetRequestHeader();
	EXPECT_CONTAINS(headers, "Content-Type: application/json");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataDelete)
	{
	RealityDataDelete requestUT("RealityDataID");

	EXPECT_STREQ(requestUT.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityData/RealityDataID");
	EXPECT_EQ(requestUT.GetRequestType(), WSGURL::HttpRequestType::DELETE_Request);

	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataDeleteGoodRequest)
	{
    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(0);
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
	    {
	    response.status = ::OK;
	    response.curlCode = CURLE_OK;
	    response.responseCode = 200;
	    response.body = R"(
		    {
		    "changedInstance": 
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

	RealityDataDelete requestUT("RealityDataID");
	RawServerResponse rawResponse{};

	s_realityDataService->Request(requestUT, rawResponse);
	EXPECT_EQ(rawResponse.status, ::OK);
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataDeleteBadRequest)
	{
	EXPECT_CALL(*s_errorClass, errorCallBack(Eq("RealityDataDelete failed with response"), _)).Times(1);
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::BADREQ;
		}));

	RealityDataDelete requestUT("RealityDataID");
	RawServerResponse rawResponse{};

	s_realityDataService->Request(requestUT, rawResponse);
	EXPECT_EQ(rawResponse.status, ::BADREQ);
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataEnterpriseStatRequestBadRequest)
	{
	EXPECT_CALL(*s_errorClass, errorCallBack(Eq("RealityDataEnterpriseStatRequest failed with response"), _)).Times(1);
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::BADREQ;
		}));

	RealityDataEnterpriseStatRequest requestUT("RealityDataID");
	RawServerResponse rawResponse{};

    RealityDataEnterpriseStat stat {};
	s_realityDataService->Request(requestUT, stat, rawResponse);
	EXPECT_EQ(rawResponse.status, ::BADREQ);
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataByIdRequestBadRequest)
	{
	EXPECT_CALL(*s_errorClass, errorCallBack(Eq("RealityDataByIdRequest failed with response"), _)).Times(1);
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::BADREQ;
		}));

	RealityDataByIdRequest requestUT("RealityDataID");
	RawServerResponse rawResponse{};

	s_realityDataService->Request(requestUT, rawResponse);
	EXPECT_EQ(rawResponse.status, ::BADREQ);
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataDocumentByIdRequestBadRequest)
	{
	EXPECT_CALL(*s_errorClass, errorCallBack(Eq("RealityDataDocumentByIdRequest failed with response"), _)).Times(1);
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::BADREQ;
		}));

	RealityDataDocumentByIdRequest requestUT("RealityDataID");
	RawServerResponse rawResponse{};

	s_realityDataService->Request(requestUT, rawResponse);
	EXPECT_EQ(rawResponse.status, ::BADREQ);
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataDocumentContentByIdRequestBadRequest)
	{
	EXPECT_CALL(*s_errorClass, errorCallBack(Eq("RealityDataDocumentContentByIdRequest failed with response"), _)).Times(1);
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::BADREQ;
		}));

	RealityDataDocumentContentByIdRequest requestUT("RealityDataID");
	RawServerResponse rawResponse{};

	s_realityDataService->Request(requestUT,new BeFile() ,rawResponse);
	EXPECT_EQ(rawResponse.status, ::BADREQ);
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataFolderByIdRequestBadRequest)
	{
	EXPECT_CALL(*s_errorClass, errorCallBack(Eq("RealityDataFolderByIdRequest failed with response"), _)).Times(1);
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::BADREQ;
		}));

	RealityDataFolderByIdRequest requestUT("RealityDataID");
	RawServerResponse rawResponse{};

	s_realityDataService->Request(requestUT ,rawResponse);
	EXPECT_EQ(rawResponse.status, ::BADREQ);
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataListByOrganizationPagedRequestBadRequest)
	{
	EXPECT_CALL(*s_errorClass, errorCallBack(Eq("RealityDataListByOrganizationPagedRequest failed with response"), _)).Times(1);
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::BADREQ;
		}));

	RealityDataListByOrganizationPagedRequest requestUT("RealityDataID");
	RawServerResponse rawResponse{};

	auto realityDataVector = s_realityDataService->Request(requestUT ,rawResponse);
	EXPECT_EQ(rawResponse.status, ::BADREQ);
    EXPECT_TRUE(realityDataVector.empty());
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataProjectRelationshipByProjectIdRequestBadRequest)
	{
	EXPECT_CALL(*s_errorClass, errorCallBack(Eq("RealityDataProjectRelationshipRequest failed with response"), _)).Times(1);
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::BADREQ;
		}));

	RealityDataProjectRelationshipByProjectIdRequest requestUT("RealityDataID");
	RawServerResponse rawResponse{};

	auto realityDataVector = s_realityDataService->Request(requestUT ,rawResponse);
	EXPECT_EQ(rawResponse.status, ::BADREQ);
    EXPECT_TRUE(realityDataVector.empty());
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataProjectRelationshipByProjectIdPagedRequestBadRequest)
	{
	EXPECT_CALL(*s_errorClass, errorCallBack(Eq("RealityDataProjectRelationshipPagedRequest failed with response"), _)).Times(1);
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::BADREQ;
		}));

	RealityDataProjectRelationshipByProjectIdPagedRequest requestUT("RealityDataID");
	RawServerResponse rawResponse{};

	auto realityDataVector = s_realityDataService->Request(requestUT ,rawResponse);
	EXPECT_EQ(rawResponse.status, ::BADREQ);
    EXPECT_TRUE(realityDataVector.empty());
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataChangeRequestBadRequest)
	{
	EXPECT_CALL(*s_errorClass, errorCallBack(Eq("RealityDataChangeRequest failed with response"), _)).Times(1);
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::BADREQ;
		}));

	RealityDataChangeRequest requestUT("RealityDataID", "SomeProperties");
	RawServerResponse rawResponse{};

	s_realityDataService->Request(requestUT ,rawResponse);
	EXPECT_EQ(rawResponse.status, ::BADREQ);
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataRelationshipCreateRequestBadRequest)
	{
	EXPECT_CALL(*s_errorClass, errorCallBack(Eq("RealityDataRelationshipCreateRequest failed with response"), _)).Times(1);
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::BADREQ;
		}));

	RealityDataRelationshipCreateRequest requestUT("RealityDataID", "SomeProperties");
	RawServerResponse rawResponse{};

	s_realityDataService->Request(requestUT ,rawResponse);
	EXPECT_EQ(rawResponse.status, ::BADREQ);
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataRelationshipDeleteBadRequest)
	{
	EXPECT_CALL(*s_errorClass, errorCallBack(Eq("RealityDataRelationshipDelete failed with response"), _)).Times(1);
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::BADREQ;
		}));

	RealityDataRelationshipDelete requestUT("RealityDataID", "SomeProperties");
	RawServerResponse rawResponse{};

	s_realityDataService->Request(requestUT ,rawResponse);
	EXPECT_EQ(rawResponse.status, ::BADREQ);
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataCreateRequestBadRequest)
	{
	EXPECT_CALL(*s_errorClass, errorCallBack(Eq("RealityDataCreateRequest failed with response"), _)).Times(1);
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::BADREQ;
		}));

	RealityDataCreateRequest requestUT("RealityDataID", "SomeProperties");
	RawServerResponse rawResponse{};

	s_realityDataService->Request(requestUT, rawResponse);
	EXPECT_EQ(rawResponse.status, ::BADREQ);
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataPagedRequestBadRequest)
	{
	EXPECT_CALL(*s_errorClass, errorCallBack(Eq("RealityDataPagedRequest failed with response"), _)).Times(1);
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::BADREQ;
		}));

	RealityDataPagedRequest requestUT{};
	RawServerResponse rawResponse{};

	auto resultVector = s_realityDataService->Request(requestUT, rawResponse);
	EXPECT_EQ(rawResponse.status, ::BADREQ);
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataPagedRequestGoodRequestLastPage)
	{
	EXPECT_CALL(*s_errorClass, errorCallBack(Eq("RealityDataPagedRequest failed with response"), _)).Times(0);
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::OK;
        response.responseCode = 200;
        response.curlCode = CURLE_OK;
        response.body = RealityModFrameworkTestsUtils::GetJson(L"TestData\\RealityPlatform\\MultipleRealityData.json");
		}));

	RealityDataPagedRequest requestUT{};
	RawServerResponse rawResponse{};

	auto resultVector = s_realityDataService->Request(requestUT, rawResponse);
	EXPECT_EQ(rawResponse.status, RequestStatus::LASTPAGE);
    EXPECT_EQ(resultVector.size(), 2);
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataPagedRequestGoodRequestNotLastPage)
	{
	EXPECT_CALL(*s_errorClass, errorCallBack(Eq("RealityDataPagedRequest failed with response"), _)).Times(0);
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
        auto pagedRequest = static_cast<const RealityDataPagedRequest*>(&wsgRequest);
		response.status = ::OK;
        response.responseCode = 200;
        response.curlCode = CURLE_OK;
        if(pagedRequest->GetStartIndex() == 0)
            {
            response.body = RealityModFrameworkTestsUtils::GetJson(L"TestData\\RealityPlatform\\SingleRealityData-Helsinki.json");
            }
        else
            {
            response.body = RealityModFrameworkTestsUtils::GetJson(L"TestData\\RealityPlatform\\SingleRealityData-Helsinki2.json");
            }
        
		}));

	RealityDataPagedRequest requestUT{};
	RawServerResponse rawResponse{};

    // Only ask for 1 entity so we can get another page
    requestUT.SetPageSize(1);

	auto resultVector = s_realityDataService->Request(requestUT, rawResponse);

	EXPECT_EQ(rawResponse.status, RequestStatus::OK);
    EXPECT_EQ(resultVector.size(), 1);
    EXPECT_EQ(resultVector[0]->GetName(), "Helsinki");


    // The last request called advance page, so we can now get the second entity
    EXPECT_EQ(requestUT.GetStartIndex(), 1);

    resultVector = s_realityDataService->Request(requestUT, rawResponse);

	EXPECT_EQ(rawResponse.status, RequestStatus::OK);
    EXPECT_EQ(resultVector.size(), 1);
    EXPECT_EQ(resultVector[0]->GetName(), "Helsinki2");
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataEnterpriseStatRequestGoodRequest)
	{
	EXPECT_CALL(*s_errorClass, errorCallBack(Eq("RealityDataEnterpriseStatRequest failed with response"), _)).Times(0);
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
        EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/EnterpriseStat/72adad30%2Dc07c%2D465d%2Da1fe%2D2f2dfac950a4");
		response.status = ::OK;
        response.responseCode = 200;
        response.curlCode = CURLE_OK;
        response.body = RealityModFrameworkTestsUtils::GetJson(L"TestData\\RealityPlatform\\EnterpriseStat.json");
		}));

	RealityDataEnterpriseStatRequest requestUT("72adad30-c07c-465d-a1fe-2f2dfac950a4");
	RawServerResponse rawResponse{};
    RealityDataEnterpriseStat stats;

	s_realityDataService->Request(requestUT, stats, rawResponse);
	EXPECT_EQ(rawResponse.status, RequestStatus::OK);
    EXPECT_EQ(stats.GetUltimateId(), "e82a584b-9fae-409f-9581-fd154f7b9ef9");
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, AllRealityDataByRootIdGoodRequest)
	{
    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::OK;
        response.responseCode = 200;
        response.curlCode = CURLE_OK;
        
        auto requestString = wsgRequest.GetHttpRequestString();

        if(requestString.Contains("RealityData/RootID/FileAccess.FileAccessKey?"))
            {
            // First request is from GetAzureRedirectionRequestUrl() so we send a redirection URL
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
            }
		}));

    EXPECT_CALL(*s_mockWSGInstance, PerformAzureRequest(_,_,_,_,_)).Times(2).WillRepeatedly(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        response.status = ::OK;
        response.responseCode = 200;
        response.curlCode = CURLE_OK;
        auto requestString = wsgRequest.GetHttpRequestString();
        if(requestString.Contains("marker=Page2"))
            {
            response.body = RealityModFrameworkTestsUtils::GetJson(L"TestData\\RealityPlatform\\ListAllServerResponseSecondPage.xml");    
            }
        else
            {
            response.body = RealityModFrameworkTestsUtils::GetJson(L"TestData\\RealityPlatform\\ListAllServerResponseFirstPage.xml");        
            }
        
        }));

	AllRealityDataByRootId requestUT("RootID");
	RawServerResponse rawResponse{};

	auto vector = s_realityDataService->Request(requestUT, rawResponse);
    ASSERT_EQ(vector.size(), 4);
	EXPECT_EQ(rawResponse.status, RequestStatus::OK);
    EXPECT_EQ(vector[0].first, L"Folder1/File1.txt");
    EXPECT_EQ(vector[0].second, 0);
    EXPECT_EQ(vector[1].first, L"json.json");
    EXPECT_EQ(vector[1].second, 1024);
    EXPECT_EQ(vector[2].first, L"Folder1/File3.txt");
    EXPECT_EQ(vector[2].second, 5000);
    EXPECT_EQ(vector[3].first, L"json4.json");
    EXPECT_EQ(vector[3].second, 9000);
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataByIdRequestGoodRequest)
	{
    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(0);
    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
        EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityData/72adad30%2Dc07c%2D465d%2Da1fe%2D2f2dfac950a5");
		response.status = ::OK;
        response.responseCode = 200;
        response.curlCode = CURLE_OK;
		response.body = RealityModFrameworkTestsUtils::GetJson(L"TestData\\RealityPlatform\\SingleRealityData-Helsinki.json");  
		}));

	RealityDataByIdRequest requestUT("72adad30-c07c-465d-a1fe-2f2dfac950a5");
	RawServerResponse rawResponse{};

	auto realityData = s_realityDataService->Request(requestUT, rawResponse);
    EXPECT_EQ(realityData->GetName(), "Helsinki");
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataDocumentByIdRequestGoodRequest)
	{
    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(0);
    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
        EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/Document/72adad30%2Dc07c%2D465d%2Da1fe%2D2f2dfac950a7");
		response.status = ::OK;
        response.responseCode = 200;
        response.curlCode = CURLE_OK;
        
        response.body = RealityModFrameworkTestsUtils::GetJson(L"TestData\\RealityPlatform\\RealityDataDocument.json");  
		}));

	RealityDataDocumentByIdRequest requestUT("72adad30-c07c-465d-a1fe-2f2dfac950a7");
	RawServerResponse rawResponse{};

	auto realityDatadocument = s_realityDataService->Request(requestUT, rawResponse);
    EXPECT_EQ(realityDatadocument->GetName(), "Production_Helsinki_3MX_ok.3mx");
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataDocumentContentByIdRequestGoodRequestWithoutRedirect)
	{
    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(0);
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
        EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/Document/72adad30%2Dc07c%2D465d%2Da1fe%2D2f2dfac950a8/$file");
		response.status = ::OK;
        response.responseCode = 200;
        response.curlCode = CURLE_OK;
		}));

	RealityDataDocumentContentByIdRequest requestUT("72adad30-c07c-465d-a1fe-2f2dfac950a8");
	RawServerResponse rawResponse{};

    BeFile file {};
    requestUT.SetAzureRedirectionPossible(false);
    
	s_realityDataService->Request(requestUT, &file , rawResponse);
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataDocumentContentByIdRequestGoodRequestWithRedirect)
	{
    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(0);
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::OK;
        response.responseCode = 200;
        response.curlCode = CURLE_OK;
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

    EXPECT_CALL(*s_mockWSGInstance, PerformAzureRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
        EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://redirected.server.com//72adad30-c07c-465d-a1fe-2f2dfac950a9/RootDocument.s3mx?myToken&se=2013-03-01T16%3A20%3A00Z");
		response.status = ::OK;
        response.responseCode = 200;
        response.curlCode = CURLE_OK;
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

	RealityDataDocumentContentByIdRequest requestUT("72adad30-c07c-465d-a1fe-2f2dfac950a9/RootDocument.s3mx");
	RawServerResponse rawResponse{};

    BeFile file {};
   
    // This request will call PerformRequest once to get the azure token
    // and will call PerformAzureRequest once with the information it got from the first result.
	s_realityDataService->Request(requestUT, &file , rawResponse);
    
	}


//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataFolderByIdRequestGoodRequest)
	{
    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(0);
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
        EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/Folder/72adad30%2Dc07c%2D465d%2Da1fe%2D2f2dfac950a7");
		response.status = ::OK;
        response.responseCode = 200;
        response.curlCode = CURLE_OK;
        response.body = RealityModFrameworkTestsUtils::GetJson(L"TestData\\RealityPlatform\\RealityDataFolder.json");  
		}));

	RealityDataFolderByIdRequest requestUT("72adad30-c07c-465d-a1fe-2f2dfac950a7");
	RawServerResponse rawResponse{};
   
	auto realityDatafolder = s_realityDataService->Request(requestUT, rawResponse);
    EXPECT_EQ(realityDatafolder->GetName(), "Scene123");
    
	}


//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataListByOrganizationPagedRequestGoodRequest)
	{
    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(0);
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::OK;
        response.responseCode = 200;
        response.curlCode = CURLE_OK;
        response.body = RealityModFrameworkTestsUtils::GetJson(L"TestData\\RealityPlatform\\MultipleRealityData.json");  
		}));

	RealityDataListByOrganizationPagedRequest requestUT("RootID");
	RawServerResponse rawResponse{};
   
	auto vector = s_realityDataService->Request(requestUT, rawResponse);
    EXPECT_EQ(vector.size(), 2);
    EXPECT_EQ(rawResponse.status, ::LASTPAGE);
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataProjectRelationshipByRealityDataIdRequestGoodRequest)
	{
    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(0);
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
        EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityDataRelationship?$filter=RealityDataId+eq+'72adad30%2Dc07c%2D465d%2Da1fe%2D2f2dfac950a6'");
		response.status = ::OK;
        response.responseCode = 200;
        response.curlCode = CURLE_OK;
        response.body = RealityModFrameworkTestsUtils::GetJson(L"TestData\\RealityPlatform\\RealityDataRelationship.json");  
		}));

	RealityDataProjectRelationshipByRealityDataIdRequest requestUT("72adad30-c07c-465d-a1fe-2f2dfac950a6");
	RawServerResponse rawResponse{};
   
	auto vector = s_realityDataService->Request(requestUT, rawResponse);
    EXPECT_EQ(vector.size(), 2);
    EXPECT_EQ(vector[0]->GetRealityDataId(), "f4425509-55c4-4e03-932a-d67b87ace30f");
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataProjectRelationshipByRealityDataIdPagedRequestGoodRequest)
	{
    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(0);
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(2).WillRepeatedly(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::OK;
        response.responseCode = 200;
        response.curlCode = CURLE_OK;
        response.body = RealityModFrameworkTestsUtils::GetJson(L"TestData\\RealityPlatform\\SingleRealityDataRelationship.json");  
		}));

	RealityDataProjectRelationshipByRealityDataIdPagedRequest requestUT("RootID");
	RawServerResponse rawResponse{};
   
	auto vector = s_realityDataService->Request(requestUT, rawResponse);
    EXPECT_EQ(vector.size(), 1);
    EXPECT_EQ(vector[0]->GetRealityDataId(), "f4425509-55c4-4e03-932a-d67b87ace30f");
    EXPECT_EQ(rawResponse.status, ::LASTPAGE);
    
    requestUT.SetPageSize(1);
    requestUT.SetStartIndex(0);

    vector.clear();
    vector = s_realityDataService->Request(requestUT, rawResponse);
    EXPECT_EQ(vector.size(), 1);
    EXPECT_EQ(vector[0]->GetRealityDataId(), "f4425509-55c4-4e03-932a-d67b87ace30f");
    EXPECT_EQ(rawResponse.status, ::OK);
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataChangeRequestGoodRequest)
	{
    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(0);
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::OK;
        response.responseCode = 200;
        response.curlCode = CURLE_OK;
        response.body = R"({"changedInstance": "blabla"})";  
		}));

	RealityDataChangeRequest requestUT("RootID","SomeProperties");
	RawServerResponse rawResponse{};
   
	s_realityDataService->Request(requestUT, rawResponse);
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataCreateRequestGoodRequest)
	{
    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(0);
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::OK;
        response.responseCode = 200;
        response.curlCode = CURLE_OK;
        response.body = R"({"changedInstance": "blabla"})";  
		}));

	RealityDataCreateRequest requestUT("RootID","MoreProperties");
	RawServerResponse rawResponse{};
   
	auto response = s_realityDataService->Request(requestUT, rawResponse);
    EXPECT_EQ(response, R"({"changedInstance": "blabla"})");
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataRelationshipCreateRequestGoodRequest)
	{
    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(0);
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::OK;
        response.responseCode = 200;
        response.curlCode = CURLE_OK;
        response.body = R"({"changedInstance": "blabla"})";  
		}));

	RealityDataRelationshipCreateRequest requestUT("RootID","MoreProperties");
	RawServerResponse rawResponse{};
   
	auto response = s_realityDataService->Request(requestUT, rawResponse);
    EXPECT_EQ(response, R"({"changedInstance": "blabla"})");
    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, RealityDataRelationshipDeleteGoodRequest)
	{
    EXPECT_CALL(*s_errorClass, errorCallBack(Eq("RealityDataRelationshipDelete failed with response"), _)).Times(0);
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::OK;
        response.responseCode = 200;
        response.curlCode = CURLE_OK;
        response.body = R"({"changedInstance": "blabla"})";  
		}));

	RealityDataRelationshipDelete requestUT("RootID","ProjectID");
	RawServerResponse rawResponse{};
   
	auto response = s_realityDataService->Request(requestUT, rawResponse);
    EXPECT_EQ(response, R"({"changedInstance": "blabla"})");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceFixture, AllRealityDataByRootIdBadAzureToken)
	{
    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(0);
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillOnce(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.status = ::OK;
        response.responseCode = 200;
        response.curlCode = CURLE_OK;
        
        auto requestString = wsgRequest.GetHttpRequestString();
		response.body = RealityModFrameworkTestsUtils::GetJson(L"TestData\\RealityPlatform\\ListAllServerResponseSecondPage.xml");  
		}));

	AllRealityDataByRootId requestUT("RootID");
	RawServerResponse rawResponse{};

	auto vector = s_realityDataService->Request(requestUT, rawResponse);
    EXPECT_EQ(vector.size(), 0);
	EXPECT_EQ(rawResponse.status, RequestStatus::BADREQ);    
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(TransferReport, XmlExport)
	{
	TransferResult* result1 = new TransferResult{ CURLE_OK, 100, 5, "MyName", RawServerResponse() };
	RawServerResponse response{};
	response.responseCode = 400;
	response.header = "SomeHeader";
	TransferResult* result2 = new TransferResult{CURLE_COULDNT_CONNECT, 0, 5, "MyName2", response };
	TransferReport transferReport{};

	transferReport.results.push_back(result1);
	transferReport.results.push_back(result2);

	Utf8String report{};
	transferReport.ToXml(report);
	report.ReplaceAll("\n", "");
	EXPECT_THAT(report.c_str(), testing::HasSubstr(R"(<File FileName="MyName" timeSpent="5" CURLcode="0" progress="100"/>)"));
	EXPECT_THAT(report.c_str(), testing::HasSubstr(R"(<File FileName="MyName2" timeSpent="5" CURLcode="7" progress="0">    <Response ResponseCode="400" Header="SomeHeader"/>  </File>)"));
	}

// No way to test those class yet.
#if 0

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//=====================================================================================
struct RealityDataServiceTransfer_callback
	{
	MOCK_CONST_METHOD3(progressCallBack, void(Utf8String filename, double fileProgress, double repoProgress));
	MOCK_CONST_METHOD4(statusCallBack, void(int index, void *pClient, int ErrorCode, const char* pMsg));
	MOCK_CONST_METHOD0(heartbeatCallBack, int());

	RealityDataServiceTransfer_callback()
		{

		}
	};


//=====================================================================================
//! @bsiclass                                    Remi.Charbonneau              06/2017
//=====================================================================================
class RealityDataServiceTransferFixture : public RealityDataServiceFixture
	{
public:
	static RealityDataServiceTransfer_callback* s_transferServiceCallback;
	static RealityDataServiceTransfer* s_dataServiceTransfer;
	RealityDataServiceTransferFixture()
		{
		}

	static void SetUpTestCase()
		{
		s_transferServiceCallback = new RealityDataServiceTransfer_callback();
		s_dataServiceTransfer = new RealityDataServiceTransfer();
		s_dataServiceTransfer->SetProgressCallBack(progressCallBack);
		s_dataServiceTransfer->SetStatusCallBack(statusCallBack);
		s_dataServiceTransfer->SetHeartbeatCallBack(heartBeatCallBack);
		}

	static void TearDownTestCase()
		{
		delete s_transferServiceCallback;
		s_transferServiceCallback = nullptr;
		delete s_dataServiceTransfer;
		s_dataServiceTransfer = nullptr;
		}

	static void progressCallBack(Utf8String filename, double fileProgress, double repoProgress)
		{
		if (s_transferServiceCallback != nullptr)
			{
			s_transferServiceCallback->progressCallBack(filename, fileProgress, repoProgress);
			}
		}

	static void statusCallBack(int index, void *pClient, int ErrorCode, const char* pMsg)
		{
		if (s_transferServiceCallback != nullptr)
			{
			s_transferServiceCallback->statusCallBack(index, pClient, ErrorCode, pMsg);
			}
		}

	static int heartBeatCallBack()
		{
		if (s_transferServiceCallback != nullptr)
			{
			return s_transferServiceCallback->heartbeatCallBack();
			}
		return 1;
		}

	};

RealityDataServiceTransfer_callback* RealityDataServiceTransferFixture::s_transferServiceCallback = nullptr;
RealityDataServiceTransfer* RealityDataServiceTransferFixture::s_dataServiceTransfer = nullptr;


//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceTransferFixture, EmptyFileToTransfer)
	{
	EXPECT_CALL(*s_transferServiceCallback, statusCallBack(Eq(0), Eq(nullptr), Eq(-1), StrEq("No files to transfer, please verify that the previous steps completed without failure"))).Times(1);
	auto report = s_dataServiceTransfer->Perform();
	//EXPECT_EQ(report.results[0]->progress, 0);
	}

#endif

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//! RealityDataServiceBadComponentsFixture
//=====================================================================================
class RealityDataServiceBadComponentsFixture : public testing::Test
	{
public:
	static RealityDataService* s_realityDataService;

	static void SetUpTestCase()
		{
		s_realityDataService = new RealityDataService();
        RealityDataServiceHelper::ResetServerComponents();
		}

	static void TearDownTestCase()
		{
		delete s_realityDataService;
		s_realityDataService = nullptr;
		}

	};

RealityDataService* RealityDataServiceBadComponentsFixture::s_realityDataService = nullptr;

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceBadComponentsFixture, RealityDataDeleteBadComponents)
	{
	RealityDataDelete requestUT("RealityDataID");
	RawServerResponse rawResponse{};

	s_realityDataService->Request(requestUT, rawResponse);
	EXPECT_EQ(rawResponse.status, ::PARAMSNOTSET);
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceBadComponentsFixture, RealityDataEnterpriseStatRequestBadComponents)
	{
	RealityDataEnterpriseStatRequest requestUT("RealityDataID");
	RawServerResponse rawResponse{};

    RealityDataEnterpriseStat stat {};
	s_realityDataService->Request(requestUT, stat, rawResponse);
	EXPECT_EQ(rawResponse.status, ::PARAMSNOTSET);
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceBadComponentsFixture, RealityDataByIdRequestBadComponents)
	{
	RealityDataByIdRequest requestUT("RealityDataID");
	RawServerResponse rawResponse{};

	s_realityDataService->Request(requestUT, rawResponse);
	EXPECT_EQ(rawResponse.status, ::PARAMSNOTSET);
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceBadComponentsFixture, RealityDataDocumentByIdRequestBadComponents)
	{
	RealityDataDocumentByIdRequest requestUT("RealityDataID");
	RawServerResponse rawResponse{};

	s_realityDataService->Request(requestUT, rawResponse);
	EXPECT_EQ(rawResponse.status, ::PARAMSNOTSET);
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceBadComponentsFixture, RealityDataDocumentContentByIdRequestBadComponents)
	{
	RealityDataDocumentContentByIdRequest requestUT("RealityDataID");
	RawServerResponse rawResponse{};

	s_realityDataService->Request(requestUT,new BeFile() ,rawResponse);
	EXPECT_EQ(rawResponse.status, ::PARAMSNOTSET);
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceBadComponentsFixture, RealityDataFolderByIdRequestBadComponents)
	{
	RealityDataFolderByIdRequest requestUT("RealityDataID");
	RawServerResponse rawResponse{};

	s_realityDataService->Request(requestUT ,rawResponse);
	EXPECT_EQ(rawResponse.status, ::PARAMSNOTSET);
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceBadComponentsFixture, RealityDataListByOrganizationPagedRequestBadComponents)
	{
	RealityDataListByOrganizationPagedRequest requestUT("RealityDataID");
	RawServerResponse rawResponse{};

	auto realityDataVector = s_realityDataService->Request(requestUT ,rawResponse);
	EXPECT_EQ(rawResponse.status, ::PARAMSNOTSET);
    EXPECT_TRUE(realityDataVector.empty());
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceBadComponentsFixture, RealityDataProjectRelationshipByProjectIdRequestBadComponents)
	{
	RealityDataProjectRelationshipByProjectIdRequest requestUT("RealityDataID");
	RawServerResponse rawResponse{};

	auto realityDataVector = s_realityDataService->Request(requestUT ,rawResponse);
	EXPECT_EQ(rawResponse.status, ::PARAMSNOTSET);
    EXPECT_TRUE(realityDataVector.empty());
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceBadComponentsFixture, RealityDataProjectRelationshipByProjectIdPagedRequestBadComponents)
	{
	RealityDataProjectRelationshipByProjectIdPagedRequest requestUT("RealityDataID");
	RawServerResponse rawResponse{};

	auto realityDataVector = s_realityDataService->Request(requestUT ,rawResponse);
	EXPECT_EQ(rawResponse.status, ::PARAMSNOTSET);
    EXPECT_TRUE(realityDataVector.empty());
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceBadComponentsFixture, RealityDataChangeRequestBadComponents)
	{
	RealityDataChangeRequest requestUT("RealityDataID", "SomeProperties");
	RawServerResponse rawResponse{};

	s_realityDataService->Request(requestUT ,rawResponse);
	EXPECT_EQ(rawResponse.status, ::PARAMSNOTSET);
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceBadComponentsFixture, RealityDataRelationshipCreateRequestBadComponents)
	{
	RealityDataRelationshipCreateRequest requestUT("RealityDataID", "SomeProperties");
	RawServerResponse rawResponse{};

	s_realityDataService->Request(requestUT ,rawResponse);
	EXPECT_EQ(rawResponse.status, ::PARAMSNOTSET);
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceBadComponentsFixture, RealityDataRelationshipDeleteBadComponents)
	{
	RealityDataRelationshipDelete requestUT("RealityDataID", "SomeProperties");
	RawServerResponse rawResponse{};

	s_realityDataService->Request(requestUT ,rawResponse);
	EXPECT_EQ(rawResponse.status, ::PARAMSNOTSET);
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceBadComponentsFixture, RealityDataCreateRequestBadComponents)
	{
	RealityDataCreateRequest requestUT("RealityDataID", "SomeProperties");
	RawServerResponse rawResponse{};

	s_realityDataService->Request(requestUT ,rawResponse);
	EXPECT_EQ(rawResponse.status, ::PARAMSNOTSET);
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceBadComponentsFixture, RealityDataPagedRequestBadComponents)
	{
	RealityDataPagedRequest requestUT{};
	RawServerResponse rawResponse{};

	s_realityDataService->Request(requestUT ,rawResponse);
	EXPECT_EQ(rawResponse.status, ::PARAMSNOTSET);
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(RealityDataServiceBadComponentsFixture, AllRealityDataByRootIdBadComponents)
	{
	AllRealityDataByRootId requestUT("MyIdentifier");
	RawServerResponse rawResponse{};

	s_realityDataService->Request(requestUT ,rawResponse);
	EXPECT_EQ(rawResponse.status, ::PARAMSNOTSET);
	}