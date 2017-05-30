//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/RealityPlatform/WSGServicesTester.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//#ifdef REALITYMODFRAMEWORK_LOCALTEST


#include <Bentley/BeTest.h>
#include <RealityPlatform/RealityPlatformAPI.h>
#include <RealityPlatform/WSGServices.h>


USING_NAMESPACE_BENTLEY_REALITYPLATFORM

using ::testing::NiceMock;
using ::testing::Invoke;
using ::testing::_;

//=====================================================================================
//! @bsimethod                          Remi.Charbonneau                        05/2017
//=====================================================================================
TEST(NavNode, DefaultConstructor)
	{
	auto nodeUnderTest = NavNode();

	EXPECT_STREQ(nodeUnderTest.GetNavString().c_str(), "ROOT");
	}

//=====================================================================================
//! @bsimethod                          Remi.Charbonneau                        05/2017
//=====================================================================================
TEST(NavNode, ParametersConstructor)
	{
	auto nodeUnderTest = NavNode("mySchema", "MyId", "MySystem", "MyClassName");

	
	EXPECT_STREQ(nodeUnderTest.GetSchemaName().c_str(), "mySchema");
	EXPECT_STREQ(nodeUnderTest.GetInstanceId().c_str(), "MyId");
	EXPECT_STREQ(nodeUnderTest.GetRootId().c_str(), "MyId");
	EXPECT_STREQ(nodeUnderTest.GetTypeSystem().c_str(), "MySystem");
	EXPECT_STREQ(nodeUnderTest.GetECClassName().c_str(), "MyClassName");
	EXPECT_STREQ(nodeUnderTest.GetLabel().c_str(), "");

	EXPECT_STREQ(nodeUnderTest.GetRootNode().c_str(), "MySystem--mySchema-RealityData-MyId");
	EXPECT_STREQ(nodeUnderTest.GetNavString().c_str(), "MySystem--mySchema-RealityData-MyId");
	
	}

//=====================================================================================
//! @bsimethod                          Remi.Charbonneau                        05/2017
//=====================================================================================
TEST(NavNode, JSONConstructor)
	{
	Utf8CP nodeJSON = R"(
	
	{
		"instanceId": "myInstance",
		"properties": {
			"Key_TypeSystem": "TypeSystem",
			"Key_SchemaName": "SchemaName",
			"Key_ClassName": "ClassName",
			"Key_InstanceId": "InstanceID",
			"Label": "MyLabel"
		}
	}
	
	)";
	Json::Value instance;
	Json::Reader::Parse(nodeJSON,instance);

	auto nodeUnderTest = NavNode(instance, "myRootNode", "myRootId");

	EXPECT_STREQ(nodeUnderTest.GetRootNode().c_str(), "myRootNode");
	EXPECT_STREQ(nodeUnderTest.GetRootId().c_str(), "myRootId");

	EXPECT_STREQ(nodeUnderTest.GetNavString().c_str(), "myInstance");

	EXPECT_STREQ(nodeUnderTest.GetTypeSystem().c_str(), "TypeSystem");
	EXPECT_STREQ(nodeUnderTest.GetSchemaName().c_str(), "SchemaName");
	EXPECT_STREQ(nodeUnderTest.GetECClassName().c_str(), "ClassName");
	EXPECT_STREQ(nodeUnderTest.GetInstanceId().c_str(), "InstanceID");
	EXPECT_STREQ(nodeUnderTest.GetLabel().c_str(), "MyLabel");
	
	
	}

//=====================================================================================
//! @bsimethod                          Remi.Charbonneau                        05/2017
//=====================================================================================
TEST(NodeNavigator, ParametersConstructor)
	{
	auto nodeUnderTest = NavNode("mySchema", "MyId", "MySystem", "MyClassName");


	EXPECT_STREQ(nodeUnderTest.GetSchemaName().c_str(), "mySchema");
	EXPECT_STREQ(nodeUnderTest.GetInstanceId().c_str(), "MyId");
	EXPECT_STREQ(nodeUnderTest.GetRootId().c_str(), "MyId");
	EXPECT_STREQ(nodeUnderTest.GetTypeSystem().c_str(), "MySystem");
	EXPECT_STREQ(nodeUnderTest.GetECClassName().c_str(), "MyClassName");
	EXPECT_STREQ(nodeUnderTest.GetLabel().c_str(), "");

	EXPECT_STREQ(nodeUnderTest.GetRootNode().c_str(), "MySystem--mySchema-RealityData-MyId");
	EXPECT_STREQ(nodeUnderTest.GetNavString().c_str(), "MySystem--mySchema-RealityData-MyId");

	}


//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//=====================================================================================
struct MockWSGRequest : WSGRequest
	{
	MockWSGRequest() : WSGRequest()
	{}

	MOCK_CONST_METHOD5(PerformAzureRequest, void(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry));
	MOCK_CONST_METHOD4(PrepareRequest, CURL*(const WSGURL& wsgRequest, RawServerResponse& responseString, bool verifyPeer, BeFile* file));
	MOCK_CONST_METHOD5(_PerformRequest, void(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry));
	MOCK_CONST_METHOD5(PerformRequest, void(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry));
	};

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//! NodeNavigatorFixture
//! Fixture class to host a fake WSGInstance
//! It hold a Mock WSGRequest so that all call to WSGRequest::GetInstance() can be intercepted
//=====================================================================================
class NodeNavigatorFixture : public testing::Test
	{
	public:
		static NiceMock<MockWSGRequest>* s_mockWSGInstance;

		static void SetUpTestCase()
		{
			s_mockWSGInstance = new NiceMock<MockWSGRequest>();
		}

		static void TearDownTestCase()
		{
			delete s_mockWSGInstance;
			s_mockWSGInstance = nullptr;
		}

	};

NiceMock<MockWSGRequest>* NodeNavigatorFixture::s_mockWSGInstance = nullptr;


//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(NodeNavigatorFixture, RootNodes)
    {
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.curlCode = CURLE_OK;
		response.responseCode = 200;
		response.header = R"(
				Bentley-WebAPI/99.99.99
		
			)";
		response.body = R"(
			{
			"instances" : 
				[
					{	
					"instanceId": "myInstance",
						"properties": {
							"Key_TypeSystem": "TypeSystem",
							"Key_SchemaName": "SchemaName",
							"Key_ClassName": "ClassName",
							"Key_InstanceId": "InstanceID",
							"Label": "MyLabel"
						}
					},
					{	
					"instanceId": "myInstance2",
						"properties": {
							"Key_TypeSystem": "TypeSystem2",
							"Key_SchemaName": "SchemaName2",
							"Key_ClassName": "ClassName2",
							"Key_InstanceId": "InstanceID2",
							"Label": "MyLabel2"
						}
					}
				]
			}
		
			)";
		}));

	auto rawResponse = RawServerResponse();
	auto nnInstance = NodeNavigator::GetInstance();
	bvector<NavNode> nodeVector = nnInstance.GetRootNodes(Utf8String("myServer"), Utf8String("myRepo"), rawResponse);

	EXPECT_EQ(nodeVector.size(), 2);
	EXPECT_STREQ(nodeVector[0].GetInstanceId().c_str(), "InstanceID");
	EXPECT_STREQ(nodeVector[1].GetInstanceId().c_str(), "InstanceID2");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(NodeNavigatorFixture, RootNodesBadServerResponse)
    {
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.curlCode = CURLE_OK;
		response.responseCode = 418; // I'm a teapot
		response.header = R"(
				Bentley-WebAPI/99.99.99
		
			)";
		}));

	auto rawResponse = RawServerResponse();
	auto nnInstance = NodeNavigator::GetInstance();
	bvector<NavNode> nodeVector = nnInstance.GetRootNodes(Utf8String("myServer"), Utf8String("myRepo"), rawResponse);

	EXPECT_EQ(rawResponse.responseCode, 418);
	EXPECT_TRUE(nodeVector.empty());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(NodeNavigatorFixture, RootNodesBadJson)
    {
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.curlCode = CURLE_OK;
		response.responseCode = 200;
		response.header = R"(
				Bentley-WebAPI/99.99.99
		
			)";
		response.body = R"(
		{
			"badJSON": "nonvalue"
		}
			)";
		}));

	auto rawResponse = RawServerResponse();
	auto nnInstance = NodeNavigator::GetInstance();
	bvector<NavNode> nodeVector = nnInstance.GetRootNodes(Utf8String("myServer"), Utf8String("myRepo"), rawResponse);

	EXPECT_TRUE(nodeVector.empty());
    }


//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(NodeNavigatorFixture, ChildNodes)
    {
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.curlCode = CURLE_OK;
		response.responseCode = 200;
		response.header = R"(
				Bentley-WebAPI/99.99.99
		
			)";
		response.body = R"(
			{
			"instances" : 
				[
					{	
					"instanceId": "myInstance",
						"properties": {
							"Key_TypeSystem": "TypeSystem",
							"Key_SchemaName": "SchemaName",
							"Key_ClassName": "ClassName",
							"Key_InstanceId": "InstanceID",
							"Label": "MyLabel"
						}
					},
					{	
					"instanceId": "myInstance2",
						"properties": {
							"Key_TypeSystem": "TypeSystem2",
							"Key_SchemaName": "SchemaName2",
							"Key_ClassName": "ClassName2",
							"Key_InstanceId": "InstanceID2",
							"Label": "MyLabel2"
						}
					}
				]
			}
		
			)";
		}));

	auto rawResponse = RawServerResponse();
	auto nnInstance = NodeNavigator::GetInstance();
	auto wsgServer = WSGServer("myserver.com");
	auto navNode = NavNode("MySchema", "MyID", "MyTypeSystem", "MyClassName");
	bvector<NavNode> nodeVector = nnInstance.GetChildNodes(wsgServer, "MySuperRepo", navNode, rawResponse);

	EXPECT_EQ(nodeVector.size(), 2);
	EXPECT_STREQ(nodeVector[0].GetInstanceId().c_str(), "InstanceID");
	EXPECT_STREQ(nodeVector[1].GetInstanceId().c_str(), "InstanceID2");
    }