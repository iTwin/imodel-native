#include <Bentley/BeTest.h>
#include <RealityPlatform/WSGServices.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//=====================================================================================
//! @bsimethod                          Remi.Charbonneau                        05/2017
//=====================================================================================
TEST(WSGURLBase, ConstructorWithURL)
    {
    auto wsgUrlToTest = WSGURL("https://myserver.com/watch?=uniqueID");
    EXPECT_TRUE(wsgUrlToTest.GetRequestType() == WSGURL::HttpRequestType::GET_Request);
    EXPECT_STREQ(wsgUrlToTest.GetHttpRequestString().c_str(), "https://myserver.com/watch?=uniqueID");
    EXPECT_TRUE(wsgUrlToTest.GetInterface() == WSGURL::WSGInterface::Repositories);
    EXPECT_STREQ(wsgUrlToTest.GetServerName().c_str(), "");
    EXPECT_STREQ(wsgUrlToTest.GetVersion().c_str(), "");
    EXPECT_STREQ(wsgUrlToTest.GetRepoId().c_str(), "");
    EXPECT_STREQ(wsgUrlToTest.GetSchema().c_str(), "");
    EXPECT_STREQ(wsgUrlToTest.GetECClassName().c_str(), "");
    EXPECT_STREQ(wsgUrlToTest.GetId().c_str(), "");

    EXPECT_FALSE(wsgUrlToTest.GetContentFlag());

    EXPECT_STREQ(wsgUrlToTest.GetRequestHeaders().c_str(), "");
    EXPECT_STREQ(wsgUrlToTest.GetRequestPayload().c_str(), "");
    }


//=====================================================================================
//! @bsimethod                          Remi.Charbonneau                        05/2017
//=====================================================================================
TEST(WSGURLBase, ConstructorWithFullDetails)
    {
    auto wsgUrlToTest = WSGURL("myserver.com", "Version1", "RepoID", "schema", WSGURL::WSGInterface::NavNode, "MyClassName", "MyID", true);

    EXPECT_TRUE(wsgUrlToTest.GetRequestType() == WSGURL::HttpRequestType::GET_Request);
    EXPECT_STREQ(wsgUrlToTest.GetHttpRequestString().c_str(), "https://myserver.com");
    EXPECT_TRUE(wsgUrlToTest.GetInterface() == WSGURL::WSGInterface::NavNode);
    EXPECT_STREQ(wsgUrlToTest.GetServerName().c_str(), "myserver.com");
    EXPECT_STREQ(wsgUrlToTest.GetVersion().c_str(), "Version1");
    EXPECT_STREQ(wsgUrlToTest.GetRepoId().c_str(), "RepoID");
    EXPECT_STREQ(wsgUrlToTest.GetSchema().c_str(), "schema");
    EXPECT_STREQ(wsgUrlToTest.GetECClassName().c_str(), "MyClassName");
    EXPECT_STREQ(wsgUrlToTest.GetId().c_str(), "MyID");

    EXPECT_TRUE(wsgUrlToTest.GetContentFlag());

    EXPECT_STREQ(wsgUrlToTest.GetRequestHeaders().c_str(), "");
    EXPECT_STREQ(wsgUrlToTest.GetRequestPayload().c_str(), "");
    }

struct WSGURLTester: public WSGURL
	{
public:
	WSGURLTester()
		:WSGURL("http://test.com")
		{
		WSGURL::SetServerName("myServer.com");
		//WSGURL::SetPluginName("pluginname");
		WSGURL::SetVersion("myversion");
		WSGURL::SetECClassName("myECClass");
		WSGURL::SetSchema("myschema");
		WSGURL::SetRepoId("myRepo");
		auto interface = WSGInterface::Repositories;
		WSGURL::SetInterface(interface);
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
	EXPECT_EQ(wsgUrlToTest.GetInterface(), WSGURL::WSGInterface::Repositories);
	EXPECT_STREQ(wsgUrlToTest.GetECClassName().c_str(), "myECClass");
	EXPECT_STREQ(wsgUrlToTest.GetRepoId().c_str(), "myRepo");

	wsgUrlToTest.SetId("myNewID");
	EXPECT_STREQ(wsgUrlToTest.GetId().c_str(), "myNewID");
	}

