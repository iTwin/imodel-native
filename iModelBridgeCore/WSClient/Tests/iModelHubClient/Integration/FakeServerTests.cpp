#include <Bentley/BeTest.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <BeXml/BeXml.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <FakeServer/MockIMHubHttpHandler.h>
#include <WebServices/iModelHub/Utils.h>

#include "Helpers.h"
#include "IntegrationTestsBase.h"

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_IMODELHUB

BEGIN_UNNAMED_NAMESPACE
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
END_UNNAMED_NAMESPACE

struct FakeServerFixture : public testing::Test
{
private:
    BeFileName outPath;
    BeFileName m_seed;
    static bool s_initialized;

public:
//---------------------------------------------------------------------------------------
// @bsimethod                                                 Kyle.Abramowitz    06/2018
//---------------+---------------+---------------+---------------+---------------+-------
    Utf8String CreateiModel() 
        {
        Utf8String iModelName = "BriefcaseTest";
        Utf8String description = "This is a test uploadfile2";
        Utf8String url = "https://qa-imodelhubapi.bentley.com/v2.5/Repositories/Project--1b2b32312-3222-3212-63d3-12312d4rr4/ProjectScope/iModel";
        Utf8String method = "POST";

        IHttpHandlerPtr handlePtr = std::make_shared<MockIMSHttpHandler>();
        Request request(url, method, handlePtr);
        request.SetRequestBody(HttpStringBody::Create(Json::FastWriter().write(iModelCreationJson(iModelName, description))));
        Response response = request.PerformAsync()->GetResult();
        EXPECT_EQ(HttpStatus::Created, response.GetHttpStatus());
        Json::Reader reader;
        Json::Value settings;
        reader.Parse(response.GetBody().AsString(), settings);

        Utf8String iModelId = settings[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange][ServerSchema::InstanceId].asString();
        Utf8String urlCreateSeedInstance = "https://qa-imodelhubapi.bentley.com/v2.5/Repositories/iModel--";
        urlCreateSeedInstance += iModelId + "/iModelScope/SeedFile";
        Request requestCreateSeedInstance(urlCreateSeedInstance, method, handlePtr);
        Utf8String seedFileInstanceBody = "{\"instance\":{\"className\":\"SeedFile\",\"properties\":{\"FileDescription\":\"BriefcaseTest is created by iModelHubHost\",\"FileId\":\"dcf8f343-6a5a-4b1f-a583-4c977696afc9\",\"FileName\":\"BriefcaseTest.bim\",\"FileSize\":1114112,\"MergedChangeSetId\":""},\"schemaName\":\"iModelScope\"}}";
        requestCreateSeedInstance.SetRequestBody(HttpStringBody::Create(seedFileInstanceBody));
        Response responseSeedInstanceCreation = requestCreateSeedInstance.PerformAsync()->GetResult();
        EXPECT_EQ(HttpStatus::Created, responseSeedInstanceCreation.GetHttpStatus());

        BeFileName fileToUpload;
        BeTest::GetHost().GetOutputRoot(fileToUpload);
        fileToUpload.AppendToPath(L"iModelHub");
        fileToUpload.AppendToPath(L"BriefcaseTest.bim");
        BeFile file;
        file.Open(fileToUpload, BeFileAccess::Read);
        uint64_t fileSize;
        EXPECT_EQ(BeFileStatus::Success, file.GetSize(fileSize));
        file.Close();
        uint64_t chunkSize = 4 * 1024 * 1024;   // Max 4MB.
        HttpBodyPtr body = HttpFileBody::Create(fileToUpload);
        uint64_t bytesTo = chunkSize - 1; // -1 because ranges are inclusive.
        if (bytesTo >= fileSize)
            bytesTo = fileSize - 1;

        // Update URL
        Utf8String urlUploadSeed = "https://imodelhubqasa01.blob.core.windows.net/imodelhub-";
        urlUploadSeed += iModelId;
        Request reqUploadingSeed(urlUploadSeed, method, handlePtr);
        reqUploadingSeed.GetHeaders().SetValue("x-ms-blob-type", "BlockBlob");
        reqUploadingSeed.SetRequestBody(HttpRangeBody::Create(body, 0, bytesTo));
        response = reqUploadingSeed.PerformAsync()->GetResult();
        EXPECT_EQ(HttpStatus::Created, response.GetHttpStatus());

        return iModelId;
        }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
    void Initialize()
        {
        iModelHubHost& host = iModelHubHost::Instance();
        BeFileName temp = host.GetTempDirectory();
        BeFileName assets = host.GetDgnPlatformAssetsDirectory();
        if (s_initialized)
            {
            Http::HttpClient::Reinitialize();
            RequestHandler handler;
            BeFileName::EmptyDirectory(handler.GetServerPath());
            }
        else
            {
            Http::HttpClient::Initialize(assets);
            BeSQLite::EC::ECDb::Initialize(temp, &assets);
            BeSQLite::BeSQLiteLib::Initialize(temp);
            }
            BeSQLite::Db db;
            RequestHandler handler;
            db.CreateNewDb(handler.GetDbPath());
        s_initialized = true;
        }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
    void CreateInitialSeedDb()
        {
        BeFileName seedFileName = iModelHubHost::Instance().BuildDbFileName("BriefcaseTest");
        if (seedFileName.DoesPathExist())
            {
            return;
            }
        DgnDbPtr seedDb = iModelHubHost::Instance().CreateTestDb("BriefcaseTest");
        ASSERT_TRUE(seedDb.IsValid());
        CreateCategory("DefaultCategory", *seedDb);
        BeSQLite::DbResult result = seedDb->SaveChanges();
        EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, result);
        seedDb->CloseDb();
        }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
    virtual void SetUp()
        {
        Initialize();
        CreateInitialSeedDb();
        }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
    virtual void TearDown()
        {
        Http::HttpClient::Uninitialize();
        }
};
bool FakeServerFixture::s_initialized = false;
//=======================================================================================
//  FakeServerFixture Tests
//=======================================================================================

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FakeServerFixture, CreateiModel)
    {
    Utf8String iModelName = "BriefcaseTest";
    Utf8String description = "This is a test uploadfile2";
    Utf8String url = "https://qa-imodelhubapi.bentley.com/v2.5/Repositories/Project--1b2b32312-3222-3212-63d3-12312d4rr4/ProjectScope/iModel";
    Utf8String method = "POST";

    IHttpHandlerPtr handlePtr = std::make_shared<MockIMSHttpHandler>();
    Request request(url, method, handlePtr);
    request.SetRequestBody(HttpStringBody::Create(Json::FastWriter().write(iModelCreationJson(iModelName, description))));
    Response response = request.PerformAsync()->GetResult();
    ASSERT_EQ(HttpStatus::Created, response.GetHttpStatus());

    Json::Reader reader;
    Json::Value settings;
    reader.Parse(response.GetBody().AsString(), settings);

    ASSERT_EQ(1, settings.getMemberNames().size());
    EXPECT_STRCASEEQ(ServerSchema::ChangedInstance, settings.getMemberNames()[0].c_str());
    EXPECT_STRCASEEQ("Created", settings[ServerSchema::ChangedInstance]["change"].asCString());
    EXPECT_STRCASEEQ("iModel", settings[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange][ServerSchema::ClassName].asCString());
    EXPECT_STRCASEEQ("ProjectScope", settings[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange][ServerSchema::SchemaName].asCString());
    EXPECT_FALSE(settings[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange][ServerSchema::Properties][ServerSchema::Property::Initialized].asBool());
    EXPECT_STRCASEEQ(description.c_str(), settings[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange][ServerSchema::Properties][ServerSchema::Property::Description].asCString());
    EXPECT_STRCASEEQ(iModelName.c_str(), settings[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange][ServerSchema::Properties][ServerSchema::Property::Name].asCString());
    EXPECT_FALSE(settings[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange][ServerSchema::Properties][ServerSchema::Property::UserCreated].asString().empty());
    EXPECT_STRCASEEQ("DummyUserValue", settings[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange][ServerSchema::Properties][ServerSchema::Property::UserCreated].asCString());

    Utf8String iModelId = settings[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange][ServerSchema::InstanceId].asString();
    Utf8String urlCreateSeedInstance = "https://qa-imodelhubapi.bentley.com/v2.5/Repositories/iModel--";
    urlCreateSeedInstance += iModelId + "/iModelScope/SeedFile";
    Request requestCreateSeedInstance(urlCreateSeedInstance, method, handlePtr);
    Utf8String seedFileInstanceBody = "{\"instance\":{\"className\":\"SeedFile\",\"properties\":{\"FileDescription\":\"BriefcaseTest is created by iModelHubHost\",\"FileId\":\"dcf8f343-6a5a-4b1f-a583-4c977696afc9\",\"FileName\":\"BriefcaseTest.bim\",\"FileSize\":1114112,\"MergedChangeSetId\":""},\"schemaName\":\"iModelScope\"}}";
    requestCreateSeedInstance.SetRequestBody(HttpStringBody::Create(seedFileInstanceBody));
    Response responseSeedInstanceCreation = requestCreateSeedInstance.PerformAsync()->GetResult();
    ASSERT_EQ(HttpStatus::Created, responseSeedInstanceCreation.GetHttpStatus());

    BeFileName fileToUpload;
    BeTest::GetHost().GetOutputRoot(fileToUpload);
    fileToUpload.AppendToPath(L"iModelHub");
    fileToUpload.AppendToPath(L"BriefcaseTest.bim");
    BeFile file;
    file.Open(fileToUpload, BeFileAccess::Read);
    uint64_t fileSize;
    ASSERT_EQ(BeFileStatus::Success, file.GetSize(fileSize));
    file.Close();
    uint64_t chunkSize = 4 * 1024 * 1024;   // Max 4MB.
    HttpBodyPtr body = HttpFileBody::Create(fileToUpload);
    uint64_t bytesTo = chunkSize - 1; // -1 because ranges are inclusive.
    if (bytesTo >= fileSize)
        bytesTo = fileSize - 1;

    // Update URL
    Utf8String urlUploadSeed = "https://imodelhubqasa01.blob.core.windows.net/imodelhub-";
    urlUploadSeed += iModelId;
    Request reqUploadingSeed(urlUploadSeed, method, handlePtr);
    reqUploadingSeed.GetHeaders().SetValue("x-ms-blob-type", "BlockBlob");
    reqUploadingSeed.SetRequestBody(HttpRangeBody::Create(body, 0, bytesTo));
    response = reqUploadingSeed.PerformAsync()->GetResult();
    ASSERT_EQ(HttpStatus::Created, response.GetHttpStatus());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Kyle.Abramowitz    06/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FakeServerFixture, CreatingDuplicateNamedModelsReturns409Status)
    {
    Utf8String iModelName = "Duplicate";
    Utf8String description = "This will be a duplicate file";
    Utf8String url = "https://qa-imodelhubapi.bentley.com/v2.5/Repositories/Project--1b2b32312-3222-3212-63d3-12312d4rr4/ProjectScope/iModel";
    Utf8String method = "POST";
    Json::Value objectCreationJson = iModelCreationJson(iModelName, description);
    IHttpHandlerPtr handlePtr = std::make_shared<MockIMSHttpHandler>();
    // Create first request
    Request request(url, method, handlePtr);
    request.SetRequestBody(HttpStringBody::Create(Json::FastWriter().write(objectCreationJson)));
    Response response = request.PerformAsync()->GetResult();
    ASSERT_EQ(HttpStatus::Created, response.GetHttpStatus());
    // Create second request using the same parameters
    Request duplicateRequest(url, method, handlePtr);
    duplicateRequest.SetRequestBody(HttpStringBody::Create(Json::FastWriter().write(objectCreationJson)));
    response = duplicateRequest.PerformAsync()->GetResult();
    ASSERT_EQ(HttpStatus::Conflict, response.GetHttpStatus());
    Json::Reader reader;
    Json::Value settings;
    reader.Parse(response.GetBody().AsString(), settings);
    EXPECT_STRCASEEQ("iModelHub.iModelAlreadyExists", settings["errorId"].asCString());
    EXPECT_STRCASEEQ("iModel already exists.", settings["errorMessage"].asCString());
    EXPECT_STRCASEEQ("", settings["errorDescription"].asCString());
    EXPECT_FALSE(settings["iModelInitialized"].asBool());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FakeServerFixture, GetiModels)
    {
    auto id = FakeServerFixture::CreateiModel();
    Utf8String urlGetInfo = "https://qa-imodelhubapi.bentley.com/v2.5/Repositories/Project--1b2b32312-3222-3212-63d3-12312d4rr4/ProjectScope/iModel?$select=*,HasCreatorInfo-forward-UserInfo.*";
    Utf8String method = "GET";
    IHttpHandlerPtr handlePtr = std::make_shared<MockIMSHttpHandler>();
    Request request(urlGetInfo, method, handlePtr);
    Response response = request.PerformAsync()->GetResult();
    ASSERT_EQ(HttpStatus::OK, response.GetHttpStatus());
    Utf8String body = response.GetBody().AsString();
    Json::Reader reader;
    Json::Value settings;
    reader.Parse(body, settings);
    // TODO assert more fields.
    ASSERT_STRCASEEQ(id.c_str(), settings[ServerSchema::Instances][0][ServerSchema::InstanceId].asCString());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FakeServerFixture, AcquireLocks)
    {
    FakeServerFixture::CreateiModel();
    Utf8String method = "POST";
    Utf8String urlGetInfo = "https://qa-imodelhubapi.bentley.com/v2.5/Repositories/iModel--1b2b32312-3222-3212-63d3-12312d4rr4/$changeset";
    IHttpHandlerPtr handlePtr = std::make_shared<MockIMSHttpHandler>();
    Utf8String seedFileInstanceBody = "{\"instances\":[{\"changeState\":\"modified\",\"className\":\"MultiLock\",\"instanceId\":\"MultiLock\",\"properties\":{\"BriefcaseId\":3,\"LockLevel\":2,\"LockType\":1,\"ObjectIds\":[\"0x30000000001\"],\"QueryOnly\":false,\"ReleasedWithChangeSet\":\"f0300b07214b5a8631e79091b5640fa994b279e8\",\"SeedFileId\":\"3dc56875-dcdf-48cc-92e1-39ed6ae40b09\"},\"schemaName\":\"iModelScope\"},{\"changeState\":\"modified\",\"className\":\"MultiLock\",\"instanceId\":\"MultiLock\",\"properties\":{\"BriefcaseId\":3,\"LockLevel\":2,\"LockType\":2,\"ObjectIds\":[\"0x30000000001\",\"as\"],\"QueryOnly\":false,\"ReleasedWithChangeSet\":\"f0300b07214b5a8631e79091b5640fa994b279e8\",\"SeedFileId\":\"3dc56875-dcdf-48cc-92e1-39ed6ae40b09\"},\"schemaName\":\"iModelScope\"},{\"changeState\":\"new\",\"className\":\"MultiCode\",\"properties\":{\"BriefcaseId\":3,\"CodeScope\":\"0x1\",\"CodeSpecId\":\"0x1d\",\"QueryOnly\":false,\"State\":2,\"Values\":[\"QueryLocksTest1\"]},\"schemaName\":\"iModelScope\"}],\"requestOptions\":{\"CustomOptions\":{\"DetailedError_Codes\":\"false\",\"DetailedError_Locks\":\"false\"},\"ResponseContent\":\"Empty\"}}";
    
    Request request(urlGetInfo, method, handlePtr);
    request.SetRequestBody(HttpStringBody::Create(seedFileInstanceBody));
    RequestHandler reqHandler;
    Response resp = reqHandler.PushAcquiredLocks(request);
    ASSERT_EQ(HttpStatus::OK, resp.GetHttpStatus());
    }

bvector<Utf8String> ParseUrl(Utf8String requestUrl, Utf8CP delimiter)
    {
    bvector<Utf8String> tokens;
    Utf8CP url = requestUrl.c_str();
    BeStringUtilities::Split(url, delimiter, nullptr, tokens);
    return tokens;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FakeServerFixture, MultiLocks) 
    {
    FakeServerFixture::CreateiModel();
    Utf8String urlMultiLocksInfo = "https://qa-imodelhubapi.bentley.com/v2.5/Repositories/iModel--1b2b32312-3222-3212-63d3-12312d4rr4/iModelScope/MultiLock";
    Utf8String method = "GET";
    IHttpHandlerPtr handlePtr = std::make_shared<MockIMSHttpHandler>();
    RequestHandler reqHandler;
    Request request(urlMultiLocksInfo, method, handlePtr);
    Response response = reqHandler.MultiLocksInfo(request);
    HttpResponseContentPtr respContent = response.GetContent();
    HttpBodyPtr respBody = respContent->GetBody();
    printf("reponse is %s\n", respBody->AsString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FakeServerFixture, Locks) 
    {
    FakeServerFixture::CreateiModel();
    Utf8String urlMultiLocksInfo = "https://qa-imodelhubapi.bentley.com/v2.5/Repositories/iModel--1b2b32312-3222-3212-63d3-12312d4rr4/iModelScope/Lock?$filter=BriefcaseId+ne+2+and+(LockLevel+gt+0+and+ReleasedWithChangeSetIndex+gt+2)";
    Utf8String method = "GET";
    IHttpHandlerPtr handlePtr = std::make_shared<MockIMSHttpHandler>();
    RequestHandler reqHandler;
    Request request(urlMultiLocksInfo, method, handlePtr);
    Response response = reqHandler.LocksInfo(request);
    HttpResponseContentPtr respContent = response.GetContent();
    HttpBodyPtr respBody = respContent->GetBody();
    printf("reponse is %s\n", respBody->AsString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FakeServerFixture, DeleteLocks) 
    {
    FakeServerFixture::CreateiModel();
    Utf8String urlDeleteLocks = "https://qa-imodelhubapi.bentley.com/v2.5/Repositories/iModel--1b2b32312-3222-3212-63d3-12312d4rr4/$changeset";
    Utf8String method = "POST";
    IHttpHandlerPtr handlePtr = std::make_shared<MockIMSHttpHandler>();
    Request request(urlDeleteLocks, method, handlePtr);
    request.SetRequestBody(HttpStringBody::Create("{\"instances\":[{\"changeState\":\"new\",\"className\":\"MultiCode\",\"properties\":{\"BriefcaseId\":2,\"CodeScope\":\"0x1\",\"CodeSpecId\":\"0x1d\",\"QueryOnly\":false,\"State\":2,\"Values\":[\"PushToEmptyiModel\"]},\"schemaName\":\"iModelScope\"},{\"changeState\":\"deleted\",\"className\":\"Lock\",\"instanceId\":\"DeleteAll-2\",\"schemaName\":\"iModelScope\"},{\"changeState\":\"deleted\",\"className\":\"Code\",\"instanceId\":\"DiscardReservedCodes-2\",\"schemaName\":\"iModelScope\"}],\"requestOptions\":{\"CustomOptions\":{\"DetailedError_Codes\":\"false\",\"DetailedError_Locks\":\"false\"},\"ResponseContent\":\"Empty\"}}"));
    Response response = request.PerformAsync()->GetResult();
    HttpResponseContentPtr respContent = response.GetContent();
    HttpBodyPtr respBody = respContent->GetBody();
    printf("reponse is %s\n", respBody->AsString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FakeServerFixture, GetBriefcaseInfo) 
    {
    FakeServerFixture::CreateiModel();
    Utf8String urlBriefcaseInfo = "https://qa-imodelhubapi.bentley.com/v2.5/Repositories/iModel--1b2b32312-3222-3212-63d3-12312d4rr4/iModelScope/Briefcase/3";
    Utf8String method = "GET";
    IHttpHandlerPtr handlePtr = std::make_shared<MockIMSHttpHandler>();
    Request request(urlBriefcaseInfo, method, handlePtr);
    Response response = request.PerformAsync()->GetResult();
    HttpResponseContentPtr respContent = response.GetContent();
    HttpBodyPtr respBody = respContent->GetBody();
    printf("reponse is %s\n", respBody->AsString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FakeServerFixture, GetChangeSetInfo)
    {
    FakeServerFixture::CreateiModel();
    //Utf8String urlChangeSet("https://qa-imodelhubapi.bentley.com/v2.5/Repositories/iModel--1b2b32312-3222-3212-63d3-12312d4rr4/iModelScope/ChangeSet");
    Utf8String method = "POST";
    //Utf8String body("{\"instance\":{\"className\":\"ChangeSet\",\"properties\":{\"BriefcaseId\":2,\"ContainingChanges\":0,\"Description\":\"Test ChangeSet\",\"FileSize\":323,\"Id\":\"206755703a1d9f7cb05de482dc89a0aa80abac28\",\"SeedFileId\":\"f14595ba-ee80-4669-b246-97c3eea43b89\",\"IsUploaded\":false,\"ParentId\":""},\"schemaName\":\"iModelScope\"}}");
    
    IHttpHandlerPtr handlePtr = std::make_shared<MockIMSHttpHandler>();
    /*Request requestCreateChangeSet(urlChangeSet, method, handlePtr);
    requestCreateChangeSet.SetRequestBody(HttpStringBody::Create(body));*/
    RequestHandler reqHandler;
    /*Response responseChangeSetCreated = reqHandler.PushChangeSetMetadata(requestCreateChangeSet);
    ASSERT_EQ(HttpStatus::Created, responseChangeSetCreated.GetHttpStatus());*/

    Utf8String urlGetLocksInfo = "https://qa-imodelhubapi.bentley.com/v2.5/Repositories/iModel--1b2b32312-3222-3212-63d3-12312d4rr4/iModelScope/ChangeSet?$select=Id,Index,ParentId,SeedFileId,FileAccessKey-forward-AccessKey.DownloadURl&$filter=SeedFileId+eq+'1ed549cd-eff3-46c8-a21d-d41f685e0844'";
    method = "GET";
    Request request(urlGetLocksInfo, method, handlePtr);
    Response response = reqHandler.GetChangeSetInfo(request);
    //ASSERT_EQ(HttpStatus::OK, response.GetHttpStatus());
    HttpResponseContentPtr respContent = response.GetContent();
    HttpBodyPtr respBody = respContent->GetBody();
    printf("reponse is %s\n", respBody->AsString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
/*TEST_F(FakeServerFixture, UpdateServerFile)
    {

    Utf8String urlGetInfo("https://qa-imodelhubapi.bentley.com/v2.5/Repositories/Project--7dfb2388-92cf-4ec7-94a7-ff72853466df/ProjectScope/iModel");

    Utf8String method = "GET";
    IHttpHandlerPtr handlePtr = std::make_shared<MockIMSHttpHandler>();
    Request request(urlGetInfo, method, handlePtr);
    Response responseInfo = request.PerformAsync()->GetResult();
    HttpResponseContentPtr reqContent = responseInfo.GetContent();
    HttpBodyPtr reqBody = reqContent->GetBody();

    char readBuff[100000] ;
    size_t buffSize = 100000;
    reqBody->Read(readBuff, buffSize);
    Utf8String reqBodyRead(readBuff);

    Json::Reader reader;
    Json::Value settings;
    reader.Parse(reqBodyRead, settings);
    BeGuid projGuid(true);
    Utf8String iModelId = Utf8String(settings["instances"][0]["instanceId"].asString());


    Utf8String url("https://imodelhubqasa01.blob.core.windows.net/imodelhub-");
    url += iModelId + "/BriefcaseTestu-bd997d8f-e7f7-4a8b-ad27-785e99f866f0.bim?sv=2016-05-31&sr=b&sig=%2BW0sVgxmBQGzim82S9LwRH4ao";

    Utf8String methodPut = "PUT";

    BeFileName fileToUpload;
    BeTest::GetHost().GetOutputRoot(fileToUpload);
    fileToUpload.AppendToPath(L"iModelHub");
    fileToUpload.AppendToPath(L"UploadFile.bim");
    CreateDgnDbParams params("MockServerFile");
    DbResult res;
    DgnDb::CreateDgnDb(&res, fileToUpload, params);
    ASSERT_EQ(res, DbResult::BE_SQLITE_OK);

    BeFile file;
    file.Open(fileToUpload, BeFileAccess::Read);

    uint64_t fileSize;
    ASSERT_EQ(BeFileStatus::Success, file.GetSize(fileSize));

    file.Close();
    uint64_t chunkSize = 4 * 1024 * 1024;   // Max 4MB.
    HttpBodyPtr body = HttpFileBody::Create(fileToUpload);
    Utf8String blockIds = "";
    int chunkNumber = 0;
    uint64_t bytesTo = chunkSize * chunkNumber + chunkSize - 1; // -1 because ranges are inclusive.
    if (bytesTo >= fileSize)
        bytesTo = fileSize - 1;

    std::stringstream blockIdStream;
    blockIdStream << std::setw(5) << std::setfill('0') << chunkNumber;
    std::string blockId = blockIdStream.str();
    Utf8String encodedBlockId = Base64Utilities::Encode(blockId.c_str()).c_str();
    blockIds += Utf8PrintfString("<Latest>%s</Latest>", encodedBlockId.c_str());

    // Update URL
    Utf8String blockUrl = Utf8PrintfString("%s&comp=block&blockid=%s", url.c_str(), encodedBlockId.c_str());

    Request requestUpload(blockUrl, methodPut, handlePtr);

    requestUpload.GetHeaders().SetValue("x-ms-blob-type", "BlockBlob");
    requestUpload.SetRequestBody(HttpRangeBody::Create(body, chunkSize * chunkNumber, bytesTo));

    Response responseUpload = requestUpload.PerformAsync()->GetResult();
    ASSERT_EQ(HttpStatus::Created ,  responseUpload.GetHttpStatus());
    }*/

Json::Value GetJsonResponse(Response response) 
    {
    HttpResponseContentPtr reqContent = response.GetContent();
    HttpBodyPtr reqBody = reqContent->GetBody();
    char readBuff[100000] ;
    size_t buffSize = 100000;
    reqBody->Read(readBuff, buffSize);
    Utf8String reqBodyRead(readBuff);

    Json::Reader reader;
    Json::Value settings;
    reader.Parse(reqBodyRead, settings);
    return settings;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FakeServerFixture, DownloadiModel)
    {
    FakeServerFixture::CreateiModel();
    Utf8String urlGetInfo = "https://qa-imodelhubapi.bentley.com/v2.5/Repositories/Project--1b2b32312-3222-3212-63d3-12312d4rr4/ProjectScope/iModel";
    Utf8String method = "GET";
    IHttpHandlerPtr handlePtr = std::make_shared<MockIMSHttpHandler>();
    Request request(urlGetInfo, method, handlePtr);
    Response responseInfo = request.PerformAsync()->GetResult();
    ASSERT_EQ(HttpStatus::OK, responseInfo.GetHttpStatus());
    Json::Value json;
    json = GetJsonResponse(responseInfo);

    Utf8String iModelId = json[ServerSchema::Instances][0][ServerSchema::InstanceId].asString();
    Utf8String urlGetBriefcaseId = "https://qa-imodelhubapi.bentley.com/v2.5/Repositories/iModel--";
    urlGetBriefcaseId += iModelId + "/iModelScope/Briefcase";
    method = "POST";
    Request requestBriefcaseId(urlGetBriefcaseId, method, handlePtr);
    Response responseBriefcaseId = requestBriefcaseId.PerformAsync()->GetResult();
    ASSERT_EQ(HttpStatus::Created, responseBriefcaseId.GetHttpStatus());

    json = GetJsonResponse(responseBriefcaseId);
    Utf8String fileId = json[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange][ServerSchema::Properties][ServerSchema::Property::FileId].asString();
    Utf8String briefcaseId = json[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange][ServerSchema::InstanceId].asString();
    Utf8String fileName = json[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange][ServerSchema::Properties][ServerSchema::Property::FileName].asString();
    method = "GET";
    Utf8String urlDownloadBriefcase = "https://imodelhubqasa01.blob.core.windows.net/imodelhub-";
    urlDownloadBriefcase += iModelId + "/BriefcaseTestm-" + fileId +".bim?sv=2016-05-31&sr=b&sig=1BI8ULlcZoN7WPnjkIfPTbLWZsz";
    Request requestDownload(urlDownloadBriefcase, method, handlePtr);
    BeFileName fileDownloadPath;
    BeTest::GetHost().GetOutputRoot(fileDownloadPath);
    fileDownloadPath.AppendToPath(L"BriefcaseTest");
    fileDownloadPath.AppendA("\\");
    fileDownloadPath.AppendA(iModelId.c_str());
    fileDownloadPath.AppendA("\\");
    fileDownloadPath.AppendA(briefcaseId.c_str());
    fileDownloadPath.AppendA("\\");
    fileDownloadPath.AppendA(fileName.c_str());

    requestDownload.SetResponseBody(HttpFileBody::Create(fileDownloadPath));
    Response response = requestDownload.PerformAsync()->GetResult();
    ASSERT_EQ(HttpStatus::OK, response.GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FakeServerFixture, DeleteiModel)
    {
    FakeServerFixture::CreateiModel();
    Utf8String urlGetInfo = "https://qa-imodelhubapi.bentley.com/v2.5/Repositories/Project--1b2b32312-3222-3212-63d3-12312d4rr4/ProjectScope/iModel";
    Utf8String method = "GET";
    IHttpHandlerPtr handlePtr = std::make_shared<MockIMSHttpHandler>();
    Request request(urlGetInfo, method, handlePtr);
    Response responseInfo = request.PerformAsync()->GetResult();
    HttpResponseContentPtr reqContent = responseInfo.GetContent();
    HttpBodyPtr reqBody = reqContent->GetBody();
    Utf8String reqBodyRead = reqContent->GetBody()->AsString();

    Json::Reader reader;
    Json::Value settings;
    reader.Parse(reqBodyRead, settings);
    BeGuid projGuid(true);

    Utf8String iModelId = Utf8String(settings[ServerSchema::Instances][0][ServerSchema::InstanceId].asString());
    Utf8String urlDelete = "https://qa-imodelhubapi.bentley.com/v2.5/Repositories/Project--1b2b32312-3222-3212-63d3-12312d4rr4/ProjectScope/iModel";
    urlDelete += "/" + iModelId;
    Utf8String methodDelete = "DELETE";
    Request requestDelete(urlDelete, methodDelete, handlePtr);
    Response responseDelete = requestDelete.PerformAsync()->GetResult();
    ASSERT_EQ(HttpStatus::OK, responseDelete.GetHttpStatus());
    }

