/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <RealityPlatformTools/WSGServices.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//=====================================================================================
//! @bsimethod                          Remi.Charbonneau                        05/2017
//=====================================================================================
TEST(WSGURLBase, ConstructorWithURL)
    {
    auto wsgUrlToTest = WSGURL("https://myserver.com/watch?=uniqueID");
    EXPECT_TRUE(wsgUrlToTest.GetRequestType() == WSGURL::HttpRequestType::GET_Request);
    EXPECT_STREQ(wsgUrlToTest.GetHttpRequestString().c_str(), "https://myserver.com/watch?=uniqueID");
    EXPECT_STREQ(wsgUrlToTest.GetServerName().c_str(), "");
    EXPECT_STREQ(wsgUrlToTest.GetVersion().c_str(), "");
    EXPECT_STREQ(wsgUrlToTest.GetRepoId().c_str(), "");
    EXPECT_STREQ(wsgUrlToTest.GetSchema().c_str(), "");
    EXPECT_STREQ(wsgUrlToTest.GetECClassName().c_str(), "");
    EXPECT_STREQ(wsgUrlToTest.GetId().c_str(), "");

    EXPECT_STREQ(wsgUrlToTest.GetRequestHeaders().c_str(), "");
    EXPECT_STREQ(wsgUrlToTest.GetRequestPayload().c_str(), "");
    }


//=====================================================================================
//! @bsimethod                          Remi.Charbonneau                        05/2017
//=====================================================================================
TEST(WSGURLBase, ConstructorWithFullDetails)
    {
    auto wsgUrlToTest = WSGURL("myserver.com", "Version1", "RepoID", "schema", "MyClassName", "MyID");

    EXPECT_TRUE(wsgUrlToTest.GetRequestType() == WSGURL::HttpRequestType::GET_Request);
    EXPECT_STREQ(wsgUrlToTest.GetHttpRequestString().c_str(), "https://myserver.com/");
    EXPECT_STREQ(wsgUrlToTest.GetServerName().c_str(), "myserver.com");
    EXPECT_STREQ(wsgUrlToTest.GetVersion().c_str(), "Version1");
    EXPECT_STREQ(wsgUrlToTest.GetRepoId().c_str(), "RepoID");
    EXPECT_STREQ(wsgUrlToTest.GetSchema().c_str(), "schema");
    EXPECT_STREQ(wsgUrlToTest.GetECClassName().c_str(), "MyClassName");
    EXPECT_STREQ(wsgUrlToTest.GetId().c_str(), "MyID");

    EXPECT_STREQ(wsgUrlToTest.GetRequestHeaders().c_str(), "");
    EXPECT_STREQ(wsgUrlToTest.GetRequestPayload().c_str(), "");
    }

//=====================================================================================
//! @bsiclass                         Remi.Charbonneau                        05/2017
//=====================================================================================
struct WSGURLTester: public WSGURL
	{
public:
	WSGURLTester()
		:WSGURL("http://test.com", true)
		{
		WSGURL::SetServerName("myServer.com");
		//WSGURL::SetPluginName("pluginname");
		WSGURL::SetVersion("myversion");
		WSGURL::SetECClassName("myECClass");
		WSGURL::SetSchema("myschema");
		WSGURL::SetRepoId("myRepo");
		}

	};

//=====================================================================================
//! @bsimethod                          Remi.Charbonneau                        05/2017
//=====================================================================================
TEST(WSGURLBase, Setter)
	{
	auto wsgUrlToTest = WSGURLTester();

	EXPECT_STREQ(wsgUrlToTest.GetServerName().c_str(), "myServer.com");
	EXPECT_STREQ(wsgUrlToTest.GetVersion().c_str(), "myversion");
	EXPECT_STREQ(wsgUrlToTest.GetSchema().c_str(), "myschema");
	EXPECT_STREQ(wsgUrlToTest.GetECClassName().c_str(), "myECClass");
	EXPECT_STREQ(wsgUrlToTest.GetRepoId().c_str(), "myRepo");

	wsgUrlToTest.SetId("myNewID");
	EXPECT_STREQ(wsgUrlToTest.GetId().c_str(), "myNewID");
	}

//=====================================================================================
//! @bsimethod                          Remi.Charbonneau                        05/2017
//=====================================================================================
TEST(WSGURLBase, AssignmentOperator)
	{
	auto basedURL = WSGURL("myserver.com", "Version1", "RepoID", "schema", "MyClassName", "MyID");

	WSGURL wsgUrlToTest;
	
	wsgUrlToTest = basedURL;

	EXPECT_TRUE(wsgUrlToTest.GetRequestType() == WSGURL::HttpRequestType::GET_Request);
    EXPECT_STREQ(wsgUrlToTest.GetHttpRequestString().c_str(), "https://myserver.com/");
    EXPECT_STREQ(wsgUrlToTest.GetServerName().c_str(), "myserver.com");
    EXPECT_STREQ(wsgUrlToTest.GetVersion().c_str(), "Version1");
    EXPECT_STREQ(wsgUrlToTest.GetRepoId().c_str(), "RepoID");
    EXPECT_STREQ(wsgUrlToTest.GetSchema().c_str(), "schema");
    EXPECT_STREQ(wsgUrlToTest.GetECClassName().c_str(), "MyClassName");
    EXPECT_STREQ(wsgUrlToTest.GetId().c_str(), "MyID");

    EXPECT_STREQ(wsgUrlToTest.GetRequestHeaders().c_str(), "");
    EXPECT_STREQ(wsgUrlToTest.GetRequestPayload().c_str(), "");

	}