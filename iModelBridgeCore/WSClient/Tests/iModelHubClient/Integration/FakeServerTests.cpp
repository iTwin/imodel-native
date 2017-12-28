#include "FakeServer.h"
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include "../BackDoor/PublicAPI/BackDoor/WebServices/iModelHub/iModelHubTests.h"
#include "IntegrationTestsBase.h"
#include "MockIMHubHttpHandler.h"

USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_DGN

class FakeServerFixture : public IntegrationTestsBase
    {
    public:
        BeFileName outPath;
        BeFileName m_seed;
        ScopediModelHubHost *m_pHost;
        ClientPtr    m_client;
        BeFileName GetOutputDirectory()
            {
            BeFileName outputDir;
            BeTest::GetHost().GetOutputRoot(outputDir);
            outputDir.AppendToPath(L"iModelHub");
            return outputDir;
            }
        BeFileNameStatus CreateFakeServer(WCharCP path)
            {
            if (BeFileName::DoesPathExist(path))
                return BeFileNameStatus::AlreadyExists;
            if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(path))
                return BeFileNameStatus::CantCreate;
            if (!BeFileName::IsDirectory(path))
                return BeFileNameStatus::IllegalName;
            return BeFileNameStatus::Success;
            }
        virtual void SetUp()
            {
				//
            /*IntegrationTestsBase::SetUp();
            auto proxy   = ProxyHttpHandler::GetFiddlerProxyIfReachable();
            m_client     = SetUpClient(IntegrationTestSettings::Instance().GetValidAdminCredentials(), proxy);
            m_imodel = CreateNewiModel(*m_client, "BriefcaseTest");*/
            BeTest::GetHost().GetOutputRoot(outPath);
            BeFileName seedFilePath = outPath;
            outPath.AppendToPath(L"Server");
            seedFilePath.AppendToPath(L"E:\\out2");
            WCharCP serverPath = outPath.GetWCharCP();
            BeFileNameStatus stat = CreateFakeServer(serverPath);
            EXPECT_EQ(stat, BeFileNameStatus::Success);
            WCharP seedFile = L"Test_Seed.bim";
            EXPECT_EQ(BeFileNameStatus::Success, FakeServer::CreateiModelFromSeed(L"E:\\out2", serverPath, seedFile));
            }
        virtual void TearDown()
            {
            WCharCP serverPath = outPath.GetWCharCP();
            EXPECT_EQ(BeFileNameStatus::Success, FakeServer::DeleteAlliModels(serverPath));
            }
    };
/*
TEST_F(FakeServerFixture, CreateiModelFromSeed) 
    {
    WCharCP serverPath = outPath.GetWCharCP();
    WCharP downloadPath = L"E:\\out"; 
    WCharP seedFile = L"Test_Seed.bim";
    EXPECT_EQ(BeFileNameStatus::Success, FakeServer::DownloadiModel(downloadPath, serverPath, seedFile));
    DbResult res = DbResult::BE_SQLITE_OK;
    DgnDbPtr m_db = FakeServer::AcquireBriefcase(res, downloadPath, seedFile);
    EXPECT_EQ(DbResult::BE_SQLITE_OK, res);
    EXPECT_TRUE(m_db.IsValid());
    }*/

Utf8String GetUrlWithoutLengthWarning(Utf8StringCR path, Utf8StringCR queryString)
    {
    Utf8String url("https://qa-imodelhubapi.bentley.com/v2.5");

    if (!path.empty())
        {
        url += "/" + path;
        }

    if (!queryString.empty())
        {
        url += "?" + queryString;
        }

    return url;
    }
Utf8String GetUrl(Utf8StringCR path, Utf8StringCR queryString = nullptr)
    {
    Utf8String url = GetUrlWithoutLengthWarning(path, queryString);
    return url;
    }

Utf8String CreateClassSubPath(Utf8StringCR schemaName, Utf8StringCR className)
    {
    return schemaName + "/" + className;
    }
Json::Value iModelCreationJson(Utf8StringCR iModelName, Utf8StringCR description)
    {
    Json::Value iModelCreation(Json::objectValue);
    JsonValueR instance = iModelCreation[ServerSchema::Instance] = Json::objectValue;
    instance[ServerSchema::SchemaName] = ServerSchema::Schema::Project;
    instance[ServerSchema::ClassName] = ServerSchema::Class::iModel;
    JsonValueR properties = instance[ServerSchema::Properties] = Json::objectValue;
    properties[ServerSchema::Property::iModelName] = iModelName;
    properties[ServerSchema::Property::iModelDescription] = description;
    return iModelCreation;
    }

#ifdef __WIP__ 
TEST_F(FakeServerFixture, CreateiModel)
    {
    Utf8String projectId("7ffff-sdsd-wew");
    
    Utf8String iModelName("BriefcaseTest9999");
    Utf8String description("This is a test uploadfile");
    Json::Value objectCreationJson = iModelCreationJson(iModelName, description);

    Utf8String url;
    {
    ObjectId objectId;
    Utf8String schemaName = objectCreationJson["instance"]["schemaName"].asString();
    Utf8String className = objectCreationJson["instance"]["className"].asString();
    Utf8String instanceId = objectCreationJson["instance"]["instanceId"].asString();

    url = GetUrl(CreateClassSubPath(schemaName, className));
    if (!instanceId.empty() && objectCreationJson["instance"]["changeState"].asString() != "new")
        url += "/" + instanceId;
    }

    Utf8String method = "POST";
    IHttpHandlerPtr handlePtr = std::make_shared<MockIMSHttpHandler> ();
    Request request(url, method, handlePtr);
    request.SetRequestBody(HttpStringBody::Create(Json::FastWriter().write(objectCreationJson)));
    Response response = request.PerformAsync ()->GetResult ();

    }

TEST_F(FakeServerFixture, TestMockHandler)
    {
    ClientPtr    m_client;
    iModelInfoPtr m_imodel;
    iModelConnectionPtr m_imodelConnection;

    IHttpHandlerPtr handlePtr = std::make_shared<MockIMSHttpHandler> ();
    auto proxy   = ProxyHttpHandler::GetFiddlerProxyIfReachable(handlePtr);
    }
#endif