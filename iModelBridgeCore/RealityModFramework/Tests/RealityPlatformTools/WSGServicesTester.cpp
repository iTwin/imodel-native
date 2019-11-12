//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------

//#ifdef REALITYMODFRAMEWORK_LOCALTEST


#include <Bentley/BeTest.h>
#include <RealityPlatform/RealityPlatformAPI.h>
#include <RealityPlatformTools/WSGServices.h>
#include "../common/RealityModFrameworkTestsCommon.h"


USING_NAMESPACE_BENTLEY_REALITYPLATFORM

using ::testing::NiceMock;
using ::testing::Invoke;
using ::testing::_;
using ::testing::Eq;

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
//! NodeNavigatorFixture
//! Fixture class to host a fake WSGInstance
//! It hold a Mock WSGRequest so that all call to WSGRequest::GetInstance() can be intercepted
//=====================================================================================
class WSGServicesRequestFixture : public MockRealityDataServiceFixture
	{

	};

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServicesRequestFixture, RootNodes)
    {
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.toolCode = CURLE_OK;
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
TEST_F(WSGServicesRequestFixture, RootNodesBadServerResponse)
    {
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.toolCode = CURLE_OK;
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
TEST_F(WSGServicesRequestFixture, RootNodesBadJson)
    {
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.toolCode = CURLE_OK;
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
TEST_F(WSGServicesRequestFixture, ChildNodes)
    {
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.toolCode = CURLE_OK;
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

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServicesRequestFixture, ChildNodesBadJson)
    {
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.toolCode = CURLE_OK;
		response.responseCode = 200;
		response.header = R"(
				Bentley-WebAPI/99.99.99
		
			)";
		response.body = R"(
			{
			"NOTinstances" : 
				[
				]
			}
		
			)";
		}));

	auto rawResponse = RawServerResponse();
	auto nnInstance = NodeNavigator::GetInstance();
	auto wsgServer = WSGServer("myserver.com");
	auto navNode = NavNode("MySchema", "MyID", "MyTypeSystem", "MyClassName");
	bvector<NavNode> nodeVector = nnInstance.GetChildNodes(wsgServer, "MySuperRepo", navNode, rawResponse);

	EXPECT_TRUE(nodeVector.empty());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServicesRequestFixture, ChildNodesBadServerResponse)
    {
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.toolCode = CURLE_OK;
		response.responseCode = 418; // I'm also a teapot
		response.header = R"(
				Bentley-WebAPI/99.99.99
		
			)";
		}));

	auto rawResponse = RawServerResponse();
	auto nnInstance = NodeNavigator::GetInstance();
	auto wsgServer = WSGServer("myserver.com");
	auto navNode = NavNode("MySchema", "MyID", "MyTypeSystem", "MyClassName");
	bvector<NavNode> nodeVector = nnInstance.GetChildNodes(wsgServer, "MySuperRepo", navNode, rawResponse);

	EXPECT_EQ(rawResponse.responseCode, 418);
	EXPECT_TRUE(nodeVector.empty());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServicesRequestFixture, WSGServerVersion)
	{
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.toolCode = CURLE_OK;
		response.responseCode = 200;
		response.header = R"(
				Bentley-WebAPI/99.99.99
		
			)";
		}));

	auto rawResponse = RawServerResponse();
	WSGServer serverUT("myserver.com");

	auto version = serverUT.GetVersion(rawResponse);

	EXPECT_STREQ(version.c_str(), "99.99.99");

	// Ask a second time for the cached version
	auto version2 = serverUT.GetVersion(rawResponse);
	EXPECT_STREQ(version2.c_str(), "99.99.99");
	}

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServicesRequestFixture, WSGServerPluginsBadVersion)
	{
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.toolCode = CURLE_OK;
		response.responseCode = 400;
		response.header = R"(
				Bentley-WebAPI/99.99.99
		
			)";
		}));

	auto rawResponse = RawServerResponse();
	WSGServer serverUT("myserver.com");

	auto plugins = serverUT.GetPlugins(rawResponse);
	EXPECT_EQ(rawResponse.responseCode, 400);
	EXPECT_TRUE(plugins.empty());
	}

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServicesRequestFixture, WSGServerRepositoriesBadVersion)
	{
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.toolCode = CURLE_OK;
		response.responseCode = 400;
		response.header = R"(
				Bentley-WebAPI/99.99.99
		
			)";
		}));

	auto rawResponse = RawServerResponse();
	WSGServer serverUT("myserver.com");

	auto repositories = serverUT.GetRepositories(rawResponse);
	EXPECT_EQ(rawResponse.responseCode, 400);
	EXPECT_TRUE(repositories.empty());
	}

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServicesRequestFixture, WSGServerSchemaNameBadVersion)
	{
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.toolCode = CURLE_OK;
		response.responseCode = 400;
		response.header = R"(
				Bentley-WebAPI/99.99.99
		
			)";
		}));

	auto rawResponse = RawServerResponse();
	WSGServer serverUT("myserver.com");

	auto schemaName = serverUT.GetSchemaNames("myName",rawResponse);
	EXPECT_EQ(rawResponse.responseCode, 400);
	EXPECT_TRUE(schemaName.empty());
	}

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServicesRequestFixture, WSGServerClassNamesBadVersion)
	{
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.toolCode = CURLE_OK;
		response.responseCode = 400;
		response.header = R"(
				Bentley-WebAPI/99.99.99
		
			)";
		}));

	auto rawResponse = RawServerResponse();
	WSGServer serverUT("myserver.com");

	auto classNames = serverUT.GetClassNames("myName","mySchema",rawResponse);
	EXPECT_EQ(rawResponse.responseCode, 400);
	EXPECT_TRUE(classNames.empty());
	}

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServicesRequestFixture, WSGServerJSONClassDefinitionBadVersion)
	{
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.toolCode = CURLE_OK;
		response.responseCode = 400;
		response.header = R"(
				Bentley-WebAPI/99.99.99
		
			)";
		}));

	auto rawResponse = RawServerResponse();
	WSGServer serverUT("myserver.com");

	auto classNames = serverUT.GetJSONClassDefinition("myName","mySchema","myClass",rawResponse);
	EXPECT_EQ(rawResponse.responseCode, 400);
	EXPECT_TRUE(classNames.empty());
	}

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//! WSGServerFixture
//! Fixture class to host the server under test.
//! That way, all future request for version will get the cached version and it will 
//! prevent two calls to the PerformRequest method
//=====================================================================================
class WSGServerFixture : public WSGServicesRequestFixture
	{
	public:
		static WSGServer* s_WSGServer;

		static void SetUpTestCase()
		{
			s_mockWSGInstance = new NiceMock<MockWSGRequest>();
			s_WSGServer = new WSGServer("myserver.com");
			ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
				{
				response.toolCode = CURLE_OK;
				response.responseCode = 200;
				response.header = R"(
						Bentley-WebAPI/99.99.99
		
					)";
				}));
			auto rawResponse = RawServerResponse(); 
			s_WSGServer->GetVersion(rawResponse);
		}

		static void TearDownTestCase()
		{
			delete s_WSGServer;
			s_WSGServer = nullptr;
			delete s_mockWSGInstance;
			s_mockWSGInstance = nullptr;
		}

	};

WSGServer* WSGServerFixture::s_WSGServer = nullptr;

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServerFixture, WSGServerPlugins)
	{
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.toolCode = CURLE_OK;
		response.responseCode = 200;

		response.body = R"(
			{
			  "instances": [
				{
				  "instanceId": "S3MXECPlugin",
				  "schemaName": "Plugins",
				  "className": "PluginIdentifier",
				  "properties": {
					"ECPluginID": "S3MXECPlugin",
					"DisplayLabel": "S3MXECPlugin",
					"VersionMajor": 1,
					"VersionMinor": 0
				  },
				  "eTag": "\"lFQPUJLvyi1o0tIm63LPoaY9QXo=\""
				},
				{
				  "instanceId": "Bentley.ECServices",
				  "schemaName": "Plugins",
				  "className": "PluginIdentifier",
				  "properties": {
					"ECPluginID": "Bentley.ECServices",
					"DisplayLabel": "Bentley.ECServices",
					"VersionMajor": 1,
					"VersionMinor": 2
				  },
				  "eTag": "\"IeUKmcizFHusy/Mw5erVclYsyxI=\""
				}
			  ]
			}
		
			)";
		EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v99.99.99/Plugins/");
		}));

	auto rawResponse = RawServerResponse();

	auto plugins = s_WSGServer->GetPlugins(rawResponse);

	EXPECT_EQ(plugins.size(), 2);
	EXPECT_STREQ(plugins[0].c_str(), "S3MXECPlugin");
	EXPECT_STREQ(plugins[1].c_str(), "Bentley.ECServices");
	}

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServerFixture, WSGServerPluginsBadJson)
	{
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.toolCode = CURLE_OK;
		response.responseCode = 200;

		response.body = R"(
			{
			  "NOTinstances": [
			  ]
			}
		
			)";
		}));

	auto rawResponse = RawServerResponse();

	auto plugins = s_WSGServer->GetPlugins(rawResponse);

	EXPECT_TRUE(plugins.empty());
	}

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServerFixture, WSGServerRepositories)
	{
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.toolCode = CURLE_OK;
		response.responseCode = 200;

		response.body = R"(
			{
			  "instances": [
				{
				  "instanceId": "S3MXECPlugin--Server",
				  "schemaName": "Repositories",
				  "className": "RepositoryIdentifier",
				  "properties": {
					"ECPluginID": "S3MXECPlugin",
					"Location": "Server",
					"DisplayLabel": "Server",
					"Description": "Server"
				  },
				  "eTag": "\"pMVuSX/IYPMg7qCsIDT3osAr5b0=\""
				},
				{
				  "instanceId": "S3MXECPlugin--Server222",
				  "schemaName": "Repositories",
				  "className": "RepositoryIdentifier",
				  "properties": {
					"ECPluginID": "S3MXECPlugin",
					"Location": "Server",
					"DisplayLabel": "Server",
					"Description": "Server"
				  },
				  "eTag": "\"pMVuSX/IYPMg7qCsIDT3osAr5b0=\""
				}
			  ]
			}
		
			)";
		EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v99.99.99/Repositories/");
		}));

	auto rawResponse = RawServerResponse();

	auto repositories = s_WSGServer->GetRepositories(rawResponse);

	EXPECT_EQ(repositories.size(), 2);
	EXPECT_STREQ(repositories[0].c_str(), "S3MXECPlugin--Server");
	EXPECT_STREQ(repositories[1].c_str(), "S3MXECPlugin--Server222");
	}

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServerFixture, WSGServerRepositoriesBadJson)
	{
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.toolCode = CURLE_OK;
		response.responseCode = 200;

		response.body = R"(
			{
			  "NOTinstances": [
			  ]
			}
		
			)";
		}));

	auto rawResponse = RawServerResponse();

	auto repositories = s_WSGServer->GetRepositories(rawResponse);

	EXPECT_TRUE(repositories.empty());
	}

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServerFixture, WSGServerSchemaNames)
	{
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.toolCode = CURLE_OK;
		response.responseCode = 200;

		response.body = R"(
			{
			  "instances": [
				{
				  "instanceId": "N~3APolicies.01.01",
				  "schemaName": "MetaSchema",
				  "className": "ECSchemaDef",
				  "properties": {
					"Name": "Policies",
					"DisplayLabel": "Policies",
					"NameSpacePrefix": "rest_pol",
					"Description": null,
					"VersionMajor": 1,
					"VersionMinor": 1
				  },
				  "eTag": "\"TD71Mc1LZR6uAba553UJHMAWktw=\""
				},
				{
				  "instanceId": "N~3ABentley_Standard_CustomAttributes.01.13",
				  "schemaName": "MetaSchema",
				  "className": "ECSchemaDef",
				  "properties": {
					"Name": "Bentley_Standard_CustomAttributes",
					"DisplayLabel": "Bentley Standard Custom Attributes",
					"NameSpacePrefix": "bsca",
					"Description": "Bentley Standard Custom Attributes",
					"VersionMajor": 1,
					"VersionMinor": 13
				  },
				  "eTag": "\"zhfZga8D7RZf0mbqUkJnuzaVc2c=\""
				}
			  ]
			}
		
			)";
		EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v99.99.99/Repositories/MyRepoName/MetaSchema/ECSchemaDef/");
		}));

	auto rawResponse = RawServerResponse();

	auto schemaName = s_WSGServer->GetSchemaNames("MyRepoName" , rawResponse);

	EXPECT_EQ(schemaName.size(), 2);
	EXPECT_STREQ(schemaName[0].c_str(), "Policies");
	EXPECT_STREQ(schemaName[1].c_str(), "Bentley_Standard_CustomAttributes");
	}

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServerFixture, WSGServerSchemaNamesBadJson)
	{
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.toolCode = CURLE_OK;
		response.responseCode = 200;

		response.body = R"(
			{
			  "NOTinstances": [
			  ]
			}
		
			)";
		EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v99.99.99/Repositories/MyRepoName/MetaSchema/ECSchemaDef/");
		}));

	auto rawResponse = RawServerResponse();

	auto schemaName = s_WSGServer->GetSchemaNames("MyRepoName" , rawResponse);

	EXPECT_TRUE(schemaName.empty());
	}

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServerFixture, WSGServerClassNames)
	{
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.toolCode = CURLE_OK;
		response.responseCode = 200;

		response.body = R"(
			{
			  "instances": [
				{
				  "instanceId": "N~3APolicies.01.01~3AAdhocContainer",
				  "schemaName": "MetaSchema",
				  "className": "ECClassDef",
				  "properties": {
					"Name": "AdhocContainer",
					"DisplayLabel": "Adhoc Container",
					"Schema": "Policies.01.01",
					"Description": null,
					"IsStruct": true,
					"IsCustomAttributeClass": false,
					"IsDomainClass": false,
					"HasBaseClasses": false,
					"IsRelationshipClass": false,
					"BaseClasses": []
				  },
				  "eTag": "\"Xvb5bmrPbq5auQG06QwrkaIlq8U=\""
				},
				{
				  "instanceId": "N~3APolicies.01.01~3APolicyAssertion",
				  "schemaName": "MetaSchema",
				  "className": "ECClassDef",
				  "properties": {
					"Name": "PolicyAssertion",
					"DisplayLabel": "Policy Assertion",
					"Schema": "Policies.01.01",
					"Description": null,
					"IsStruct": false,
					"IsCustomAttributeClass": false,
					"IsDomainClass": true,
					"HasBaseClasses": false,
					"IsRelationshipClass": false,
					"BaseClasses": []
				  },
				  "eTag": "\"x5Ww6fQAK3QK+p+sB1Ql7n8MaNw=\""
				}
			  ]
			}
		
			)";
		EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v99.99.99/Repositories/MyRepoName/MetaSchema/ECClassDef?$filter=SchemaHasClass-backward-ECSchemaDef.Name+in+['MySchema']");
		}));

	auto rawResponse = RawServerResponse();

	auto className = s_WSGServer->GetClassNames("MyRepoName","MySchema", rawResponse);

	EXPECT_EQ(className.size(), 2);
	EXPECT_STREQ(className[0].c_str(), "AdhocContainer");
	EXPECT_STREQ(className[1].c_str(), "PolicyAssertion");
	}

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServerFixture, WSGServerClassNamesBadJson)
	{
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.toolCode = CURLE_OK;
		response.responseCode = 200;

		response.body = R"(
			{
			  "NOTinstances": [
			  ]
			}
		
			)";
		EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v99.99.99/Repositories/MyRepoName/MetaSchema/ECClassDef?$filter=SchemaHasClass-backward-ECSchemaDef.Name+in+['MySchema']");
		}));

	auto rawResponse = RawServerResponse();

	auto className = s_WSGServer->GetClassNames("MyRepoName","MySchema", rawResponse);

	EXPECT_TRUE(className.empty());
	}

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServerFixture, WSGServerJsonClassDefinition)
	{
	ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
		{
		response.toolCode = CURLE_OK;
		response.responseCode = 200;

		response.body = R"(
			{
			  "instances": [
				{
				  "instanceId": "N~3APolicies.01.01~3AAdhocContainer",
				  "schemaName": "MetaSchema",
				  "className": "ECClassDef",
				  "properties": {
					"Name": "AdhocContainer",
					"DisplayLabel": "Adhoc Container",
					"Schema": "Policies.01.01",
					"Description": null,
					"IsStruct": true,
					"IsCustomAttributeClass": false,
					"IsDomainClass": false,
					"HasBaseClasses": false,
					"IsRelationshipClass": false,
					"BaseClasses": []
				  },
				  "eTag": "\"Xvb5bmrPbq5auQG06QwrkaIlq8U=\""
				},
				{
				  "instanceId": "N~3APolicies.01.01~3APolicyAssertion",
				  "schemaName": "MetaSchema",
				  "className": "ECClassDef",
				  "properties": {
					"Name": "PolicyAssertion",
					"DisplayLabel": "Policy Assertion",
					"Schema": "Policies.01.01",
					"Description": null,
					"IsStruct": false,
					"IsCustomAttributeClass": false,
					"IsDomainClass": true,
					"HasBaseClasses": false,
					"IsRelationshipClass": false,
					"BaseClasses": []
				  },
				  "eTag": "\"x5Ww6fQAK3QK+p+sB1Ql7n8MaNw=\""
				}
			  ]
			}
		
			)";
		EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v99.99.99/Repositories/MyRepoName/MetaSchema/ECClassDef?$filter=SchemaHasClass-backward-ECSchemaDef.Name+in+['MySchema']");
		}));

	Utf8String expectedJson = R"({"Name": "PolicyAssertion","DisplayLabel": "Policy Assertion","Schema": "Policies.01.01","Description": null,"IsStruct": false,"IsCustomAttributeClass": false,"IsDomainClass": true,"HasBaseClasses": false,"IsRelationshipClass": false,"BaseClasses": []})";

	auto rawResponse = RawServerResponse();

	auto jsonString = s_WSGServer->GetJSONClassDefinition("MyRepoName","MySchema", "PolicyAssertion" ,rawResponse);
	jsonString.ReplaceAll("\t", "");
	jsonString.ReplaceAll("\n", "");
	EXPECT_STREQ(jsonString.c_str(), expectedJson.c_str());

	}

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServicesRequestFixture, WSGNavRootRequestPrepareString)
    {
	WSGNavRootRequest nodeRequest("myserver.com", "99", "repoID");
	auto stringRequest = nodeRequest.GetHttpRequestString();
	EXPECT_STREQ(stringRequest.c_str(), "https://myserver.com/v99/Repositories/repoID/Navigation/NavNode/");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServicesRequestFixture, WSGNavNodeRequestPrepareString)
    {
	WSGNavNodeRequest nodeRequest("myserver.com", "99", "repoID", "myNodeID");
	auto stringRequest = nodeRequest.GetHttpRequestString();
	EXPECT_STREQ(stringRequest.c_str(), "https://myserver.com/v99/Repositories/repoID/Navigation/NavNode/myNodeID/NavNode");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServicesRequestFixture, WSGObjectRequestPrepareString)
	{
	WSGObjectRequest nodeRequest("myserver.com", "99", "repoID", "mySchema", "myClass", "myObjectID");
	auto stringRequest = nodeRequest.GetHttpRequestString();
	EXPECT_STREQ(stringRequest.c_str(), "https://myserver.com/v99/Repositories/repoID/mySchema/myClass/myObjectID");
	}

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServicesRequestFixture, WSGObjectContentRequestPrepareString)
    {
	WSGObjectContentRequest nodeRequest("myserver.com", "99", "repoID", "mySchema" , "myClass", "myObjectID");
	auto stringRequest = nodeRequest.GetHttpRequestString();
	EXPECT_STREQ(stringRequest.c_str(), "https://myserver.com/v99/Repositories/repoID/mySchema/myClass/myObjectID/$file");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServicesRequestFixture, WSGObjectListPagedRequestPrepareString)
    {
	WSGObjectListPagedRequest nodeRequest("myserver.com", "99", "repoID", "mySchema" , "myClass");
	auto stringRequest = nodeRequest.GetHttpRequestString();
	EXPECT_STREQ(stringRequest.c_str(), "https://myserver.com/v99/Repositories/repoID/mySchema/myClass?$skip=0&$top=25");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServicesRequestFixture, ProxyTest)
    {
    ProxyManager pm = ProxyManager::GetInstance();
    pm.SetProxyUrlAndCredentials("test", "test2");
    Utf8String url, cred;
    pm.GetCurrentProxyUrlAndCredentials(url, cred);
    EXPECT_TRUE(url.Equals("test"));
    EXPECT_TRUE(cred.Equals("test2"));
    }

void tokenCallback(Utf8StringR token, time_t& timer)
    {
    token = "here's a token";
    timer = 700;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                           01/2018
//-------------------------------------------------------------------------------------
TEST_F(WSGServicesRequestFixture, ConnectTokenManagerTest)
    {
    ConnectTokenManager& ctm = ConnectTokenManager::GetInstance();

    EXPECT_TRUE(!ctm.GetToken().empty());

    ctm.SetTokenCallback(tokenCallback);
    ctm.RefreshToken();

    EXPECT_TRUE(ctm.GetToken().ContainsI("here's a token"));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServicesRequestFixture, CertificateTest)
    {
    BeFileName cert = BeFileName(L"C:/CertificateHere/cert.pem");
    WSGRequest::GetInstance().SetCertificatePath(cert);
    EXPECT_TRUE(cert.Equals(WSGRequest::GetInstance().GetCertificatePath()));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(WSGServicesRequestFixture, PagedRequestOperations)
    {
    WSGObjectListPagedRequest pReq = WSGObjectListPagedRequest("myserver.com", "99", "repoID", "mySchema", "myClass");

    pReq.SetPageSize(20);
    EXPECT_TRUE(pReq.GetPageSize() == 20);

    pReq.SetStartIndex(5);
    EXPECT_TRUE(pReq.GetStartIndex() == 5);
   
    pReq.RewindPage();
    EXPECT_TRUE(pReq.GetStartIndex() == 0);

    pReq.GoToPage(50);
    EXPECT_TRUE(pReq.GetStartIndex() == (50 * pReq.GetPageSize()));

    /*WSGObjectListPagedRequest pReqCopy = pReq;
    EXPECT_TRUE(pReqCopy.GetPageSize() == 20);
    EXPECT_TRUE(pReqCopy.GetStartIndex() == 50);*/
    }
