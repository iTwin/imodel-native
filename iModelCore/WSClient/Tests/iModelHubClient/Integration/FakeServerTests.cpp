#include "Helpers.h"
#include "../FakeServer/FakeServer.h"
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <BeHttp/ProxyHttpHandler.h>
#include "IntegrationTestsBase.h"
#include "../FakeServer/MockIMHubHttpHandler.h"
#include <WebServices\iModelHub\Utils.h>
#include <BeXml/BeXml.h>


USING_NAMESPACE_BENTLEY_HTTP

USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_IMODELHUB


class FakeServerFixture : public testing::Test //: public IntegrationTestsBase
    {
    public:

        BeFileName outPath;
        BeFileName m_seed;
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
        void Initialize()
            {
            static bool s_initialized = false;
            if (s_initialized)
                return;

            iModelHubHost& host = iModelHubHost::Instance();
            host.CleanOutputDirectory();
            BeFileName temp = host.GetTempDirectory();
            BeFileName assets = host.GetDgnPlatformAssetsDirectory();
            BeSQLite::BeSQLiteLib::Initialize(temp);
            BeSQLite::EC::ECDb::Initialize(temp, &assets);
            Http::HttpClient::Initialize(assets);
            s_initialized = true;
            }
        void CreateInitialSeedDb()
            {
            BeFileName seedFileName = iModelHubHost::Instance().BuildDbFileName("Test_Seed");
            if (seedFileName.DoesPathExist())
                {
                //s_seed = seedFileName;
                return;
                }
            DgnDbPtr seedDb = iModelHubHost::Instance().CreateTestDb("Test_Seed");
            ASSERT_TRUE(seedDb.IsValid());
            //s_seed = seedDb->GetFileName();
            CreateCategory("DefaultCategory", *seedDb);
            BeSQLite::DbResult result = seedDb->SaveChanges();
            EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, result);
            seedDb->CloseDb();
            }
        virtual void SetUp()
            {
            Initialize();
            CreateInitialSeedDb();


            BeTest::GetHost().GetOutputRoot(outPath);
            BeFileName seedFilePath = outPath;
            //outPath.AppendToPath(L"Server");
            seedFilePath.AppendToPath(L"iModelHub");
            //WCharCP serverPath = outPath.GetWCharCP();
            //BeFileNameStatus stat = CreateFakeServer(serverPath);
            //EXPECT_EQ(stat, BeFileNameStatus::Success);

            //RequestHandler::Initialize(outPath);
            }
        virtual void TearDown()
            {
            /*WCharCP serverPath = outPath.GetWCharCP();
            EXPECT_EQ(BeFileNameStatus::Success, FakeServer::DeleteAlliModels(serverPath));*/
            }
    };

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


/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FakeServerFixture, CreateiModel)
    {

    Utf8String iModelName("BriefcaseTest");
    Utf8String description("This is a test uploadfile2");
    Json::Value objectCreationJson = iModelCreationJson(iModelName, description);


    Utf8String url("https://qa-imodelhubapi.bentley.com/v2.5/Repositories/Project--1b2b32312-3222-3212-63d3-12312d4rr4/ProjectScope/iModel");
    Utf8String urlUpdate("https://qa-imodelhubapi.bentley.com/v2.5/Repositories/iModel--1b2b32312-3222-3212-63d3-12312d4rr4/iModelScope/SeedFile");
    {
    ObjectId objectId;
    Utf8String schemaName = objectCreationJson["instance"]["schemaName"].asString();
    Utf8String className = objectCreationJson["instance"]["className"].asString();
    Utf8String instanceId = objectCreationJson["instance"]["instanceId"].asString();

    /*url = GetUrl(CreateClassSubPath(schemaName, className));
    if (!instanceId.empty() && objectCreationJson["instance"]["changeState"].asString() != "new")
        url += "/" + instanceId;*/
    }


    Utf8String method = "POST";
    IHttpHandlerPtr handlePtr = std::make_shared<MockIMSHttpHandler>();
    Request request(url, method, handlePtr);
    request.SetRequestBody(HttpStringBody::Create(Json::FastWriter().write(objectCreationJson)));
    Response response = request.PerformAsync()->GetResult();


    HttpResponseContentPtr reqContent = response.GetContent();
    HttpBodyPtr reqBody = reqContent->GetBody();
    char readBuff[1000];
    size_t buffSize = 100000;
    reqBody->Read(readBuff, buffSize);
    Utf8String reqBodyRead(readBuff);

    Json::Reader reader;
    Json::Value settings;
    reader.Parse(reqBodyRead, settings);


    //Now download the iModel
    Utf8String url2("https://imodelhubqasa01.blob.core.windows.net/imodelhub-63849383-f51a-4e4d-b4c9-70640578663a/BriefcaseTestsm-3a55e4a4-9357-48fc-8988-9d61435651b8.bim?sv=2016-05-31&sr=b&sig=1BI8ULlcZoN7WPnjkIfPTbLWZsz");

    Utf8String method2 = "GET";
    Request request2(url2, method2, handlePtr);
    Response response2 = request2.PerformAsync()->GetResult();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FakeServerFixture, DownloadiModel)
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


    Utf8String urlGetBriefcaseId(" https://qa-imodelhubapi.bentley.com/v2.5/Repositories/iModel--c7e9a866-426e-46de-a4cb-acd0d8c3b9a6/iModelScope/Briefcase");

    Utf8String url("https://imodelhubqasa01.blob.core.windows.net/imodelhub-");
    url += iModelId + "/BriefcaseTestsm-3a55e4a4-9357-48fc-8988-9d61435651b8.bim?sv=2016-05-31&sr=b&sig=1BI8ULlcZoN7WPnjkIfPTbLWZsz";
    Request requestDownload(url, method, handlePtr);
    Response response = requestDownload.PerformAsync()->GetResult();
    ASSERT_EQ(HttpStatus::OK, response.GetHttpStatus());
    }
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FakeServerFixture, DeleteiModel)
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

    Utf8String urlDelete("https://qa-imodelhubapi.bentley.com/v2.5/Repositories/Project--7dfb2388-92cf-4ec7-94a7-ff72853466df/ProjectScope/iModel");
    urlDelete += "/" + iModelId;

    Utf8String methodDelete = "DELETE";
    Request requestDelete(urlDelete, methodDelete, handlePtr);
    Response responseDelete = requestDelete.PerformAsync()->GetResult();
    ASSERT_EQ(HttpStatus::OK, responseDelete.GetHttpStatus());
    }

TEST_F(FakeServerFixture, GetiModels)
    {
    Utf8String urlGetInfo("https://qa-imodelhubapi.bentley.com/v2.5/Repositories/Project--7dfb2388-92cf-4ec7-94a7-ff72853466df/ProjectScope/iModel");

    Utf8String method = "GET";
    IHttpHandlerPtr handlePtr = std::make_shared<MockIMSHttpHandler>();
    Request request(urlGetInfo, method, handlePtr);
    Response response = request.PerformAsync()->GetResult();
    ASSERT_EQ(HttpStatus::OK, response.GetHttpStatus());
    HttpResponseContentPtr reqContent = response.GetContent();
    HttpBodyPtr reqBody = reqContent->GetBody();


/*
    Utf8String url2("https://qa-imodelhubapi.bentley.com/v2.5/Repositories/Project--1b2b32312-3222-3212-63d3-12312d4rr4/ProjectScope/iModel");
    Utf8String method2 = "POST";
    Request request2(url2, method2, handlePtr);
    request2.SetRequestBody(reqBody);
    request2.PerformAsync()->GetResult();
*/

    char readBuff[100000] ;
    size_t buffSize = 100000;
    reqBody->Read(readBuff, buffSize);
    Utf8String reqBodyRead(readBuff);

    Json::Reader reader;
    Json::Value settings;
    reader.Parse(reqBodyRead, settings);
    BeGuid projGuid(true);
    Utf8String name = Utf8String(settings["instances"][0]["className"].asString());

    }
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/

TEST_F(FakeServerFixture, CheckPluginsRequests)
    {
    Utf8String url("https://qa-imodelhubapi.bentley.com/v2.0/Plugins");

    Utf8String method = "GET";
    IHttpHandlerPtr handlePtr = std::make_shared<MockIMSHttpHandler>();
    Request request(url, method, handlePtr);
    Response response = request.PerformAsync()->GetResult();
    /*printf("%s\n", response.GetHeaders().GetValue("Mas-Server"));*/
    }
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FakeServerFixture, UpdateServerFile)
    {
    /*Utf8String url("https://imodelhubqasa01.blob.core.windows.net/imodelhub-3e7ce8fe-aa5e-4ee0-959c-6708d8fc365d/BriefcaseTestsu-bd997d8f-e7f7-4a8b-ad27-785e99f866f0.bim?sv=2016-05-31&sr=b&sig=%2BW0sVgxmBQGzim82S9LwRH4ao");

    Utf8String method = "PUT";
    IHttpHandlerPtr handlePtr = std::make_shared<MockIMSHttpHandler>();
    
    BeFileName filePath("E:\\out\\Test_Seed.bim");
    BeFile file;
    file.Open(filePath, BeFileAccess::Read);

    uint64_t fileSize;
    ASSERT_EQ(BeFileStatus::Success, file.GetSize(fileSize));
    
    file.Close();

    uint64_t chunkSize = 4 * 1024 * 1024;   // Max 4MB.

    HttpBodyPtr body = HttpFileBody::Create(filePath);
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

    Request request(blockUrl, method, handlePtr);

    request.GetHeaders().SetValue("x-ms-blob-type", "BlockBlob");
    request.SetRequestBody(HttpRangeBody::Create(body, chunkSize * chunkNumber, bytesTo));

    Response response = request.PerformAsync()->GetResult();
    HttpBodyPtr fBody = request.GetRequestBody();*/
    BeFileName documentsDir;
    BeTest::GetHost().GetDocumentsRoot(documentsDir);
    documentsDir.AppendToPath(L"ImodelHubTestData");
    documentsDir.AppendToPath(L"iModelHubNativeTests");
    documentsDir.AppendToPath(L"BriefcaseTests.bim");
    HttpBodyPtr fileBody;
    fileBody = HttpFileBody::Create(documentsDir);
    const bmap<Utf8String, Utf8String>& headers = bmap<Utf8String, Utf8String>();
    auto content = HttpResponseContent::Create(fileBody);
    for (const auto& header : headers)
        {
        content->GetHeaders().SetValue(header.first, header.second);
        }
    Utf8String url("https://qa-ims.bentley.com/rest/ActiveSTSService/json/IssueEx");
    Response resp(content, url.c_str(), ConnectionStatus::OK, HttpStatus::OK);
    }
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FakeServerFixture, TestBuddyResponse) 
    {
    Utf8String url("https://qa-ims.bentley.com/rest/ActiveSTSService/json/IssueEx");
    IHttpHandlerPtr handlePtr = std::make_shared<MockIMSHttpHandler>();
    Request request(url, "POST", handlePtr);
    Utf8String url2("Mobile.ImsStsAuth");
    Utf8String url3("iModelHubApi");
    Utf8PrintfString body(R"xml(<?xml version="1.0" encoding="utf-8"?>
<soap:Envelope xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
               xmlns:xsd="http://www.w3.org/2001/XMLSchema"
               xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
    <soap:Body>
        <GetUrl xmlns="http://tempuri.org/">
            <urlName>%s</urlName>
            <region>%u</region>
        </GetUrl>
    </soap:Body>
</soap:Envelope>)xml", url2.c_str(), 102);

    request.SetValidateCertificate(true);
    request.SetRequestBody(HttpStringBody::Create(body));
    request.GetHeaders().SetContentType("text/xml; charset=utf-8");
    Response response = request.PerformAsync()->GetResult();
    printf("%s\n", request.GetRequestBody()->AsString().c_str());
    bvector<Utf8String> tokens;
    BeStringUtilities::Split(request.GetRequestBody()->AsString().c_str(), "<urlName>", tokens);
    if (request.GetRequestBody()->AsString().Contains("Mobile.ImsStsAuth"))
        printf("%s\n", tokens[0].c_str());
    else
        printf("%s\n", tokens[1].c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FakeServerFixture, TestRequestToken) 
    {

    Utf8String responseBody("{\"RequestedSecurityToken\":\"<saml:Assertion MajorVersion>\"}");
    Utf8String responseBody2("{\"RequestedSecurityToken\":\"<saml:Assertion MajorVersion=\\\"1\\\" MinorVersion=\\\"1\\\" AssertionID=\\\"_f0e79a5d-6b8b-4f6c-9dc2-84950140a776\\\" Issuer=\\\"https:\\/\\/qa-ims.bentley.com\\/\\\" IssueInstant=\\\"2018-01-23T06:59:22.817Z\\\" xmlns:saml=\\\"urn:oasis:names:tc:SAML:1.0:assertion\\\"><saml:Conditions NotBefore=\\\"2018-01-23T06:59:22.785Z\\\" NotOnOrAfter=\\\"2018-01-30T06:59:22.785Z\\\"><saml:AudienceRestrictionCondition><saml:Audience>sso:\\/\\/wsfed_desktop\\/1654<\\/saml:Audience><\\/saml:AudienceRestrictionCondition><\\/saml:Conditions><saml:AttributeStatement><saml:Subject><saml:NameIdentifier>87313509-6248-41e0-b43f-62aa4513a3e4<\\/saml:NameIdentifier><saml:SubjectConfirmation><saml:ConfirmationMethod>urn:oasis:names:tc:SAML:1.0:cm:holder-of-key<\\/saml:ConfirmationMethod><KeyInfo xmlns=\\\"http:\\/\\/www.w3.org\\/2000\\/09\\/xmldsig#\\\"><trust:BinarySecret xmlns:trust=\\\"http:\\/\\/docs.oasis-open.org\\/ws-sx\\/ws-trust\\/200512\\\">fS7XRoWI0KsO1u0L8dtk96kaxmf4yxxV74dPz9DlWKE=<\\/trust:BinarySecret><\\/KeyInfo><\\/saml:SubjectConfirmation><\\/saml:Subject><saml:Attribute AttributeName=\\\"name\\\" AttributeNamespace=\\\"http:\\/\\/schemas.xmlsoap.org\\/ws\\/2005\\/05\\/identity\\/claims\\\"><saml:AttributeValue>farhad.kabir@bentley.com<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"givenname\\\" AttributeNamespace=\\\"http:\\/\\/schemas.xmlsoap.org\\/ws\\/2005\\/05\\/identity\\/claims\\\"><saml:AttributeValue>Farhad<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"surname\\\" AttributeNamespace=\\\"http:\\/\\/schemas.xmlsoap.org\\/ws\\/2005\\/05\\/identity\\/claims\\\"><saml:AttributeValue>Kabir<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"emailaddress\\\" AttributeNamespace=\\\"http:\\/\\/schemas.xmlsoap.org\\/ws\\/2005\\/05\\/identity\\/claims\\\"><saml:AttributeValue>farhad.kabir@bentley.com<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"role\\\" AttributeNamespace=\\\"http:\\/\\/schemas.microsoft.com\\/ws\\/2008\\/06\\/identity\\/claims\\\"><saml:AttributeValue>BENTLEY_EMPLOYEE<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"sapbupa\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>1004183475<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"site\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>1004174721<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"ultimatesite\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>1001389117<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"sapentitlement\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>INTERNAL<\\/saml:AttributeValue><saml:AttributeValue>BENTLEY_LEARN<\\/saml:AttributeValue><saml:AttributeValue>SELECT_2006<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"entitlement\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>BENTLEY_EMPLOYEE<\\/saml:AttributeValue><saml:AttributeValue>BENTLEY_LEARN<\\/saml:AttributeValue><saml:AttributeValue>SELECT_2006<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"countryiso\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>PK<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"languageiso\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>EN<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"ismarketingprospect\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>false<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"isbentleyemployee\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>true<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"becommunitiesusername\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>87313509-6248-41E0-B43F-62AA4513A3E4<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"becommunitiesemailaddress\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>Farhad.Kabir@bentley.com<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"userid\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>87313509-6248-41e0-b43f-62aa4513a3e4<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"organization\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>Bentley Systems Inc<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"has_select\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>true<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"organizationid\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>e82a584b-9fae-409f-9581-fd154f7b9ef9<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"ultimateid\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>72adad30-c07c-465d-a1fe-2f2dfac950a4<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"ultimatereferenceid\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>72adad30-c07c-465d-a1fe-2f2dfac950a4<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"usagecountryiso\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>PK<\\/saml:AttributeValue><\\/saml:Attribute><\\/saml:AttributeStatement><ds:Signature xmlns:ds=\\\"http:\\/\\/www.w3.org\\/2000\\/09\\/xmldsig#\\\"><ds:SignedInfo><ds:CanonicalizationMethod Algorithm=\\\"http:\\/\\/www.w3.org\\/2001\\/10\\/xml-exc-c14n#\\\" \\/><ds:SignatureMethod Algorithm=\\\"http:\\/\\/www.w3.org\\/2001\\/04\\/xmldsig-more#rsa-sha256\\\" \\/><ds:Reference URI=\\\"#_f0e79a5d-6b8b-4f6c-9dc2-84950140a776\\\"><ds:Transforms><ds:Transform Algorithm=\\\"http:\\/\\/www.w3.org\\/2000\\/09\\/xmldsig#enveloped-signature\\\" \\/><ds:Transform Algorithm=\\\"http:\\/\\/www.w3.org\\/2001\\/10\\/xml-exc-c14n#\\\" \\/><\\/ds:Transforms><ds:DigestMethod Algorithm=\\\"http:\\/\\/www.w3.org\\/2001\\/04\\/xmlenc#sha256\\\" \\/><ds:DigestValue>d9YJQ24N1Yqgy5t2goeaXxC+e4YKLSVkv3jV3sOu0t4=<\\/ds:DigestValue><\\/ds:Reference><\\/ds:SignedInfo><ds:SignatureValue>LQwKIaerw5fJbtgUdi4nyrXaNS1I+R90z+hBruz9\\/pK3EABOPit4n673yt5LoMStkUY+Pori8Lmi+1xMd6qgTmo3Rlv8owwFXmTpGqZGlCsS67\\/yI3t8+fWjqP5T97\\/FCfOvM9WAu4jiiKkZd4BhaabImGKlSZpsAIFtTrvFnapX8UBRKDnXZIgoYJd8F1Q4Ky\\/qBfVBUOnQfOwoqo8X9xz65tZfN9\\/f6+3hZMcDdyC2r63Cm09g\\/IEvcItIAAQWWExiMzUFdjfg\\/fnYzt\\/QHhjCeKCsy9c94j71NFj\\/sa5fPgp6D+gp130Aqim+gsQ+wj4mgBT6RWH9zej5T2nAeA==<\\/ds:SignatureValue><KeyInfo xmlns=\\\"http:\\/\\/www.w3.org\\/2000\\/09\\/xmldsig#\\\"><X509Data><X509Certificate>MIIFXjCCBEagAwIBAgITXwAAMAAnZoWkOOCfDgAAAAAwADANBgkqhkiG9w0BAQsFADBMMRMwEQYKCZImiZPyLGQBGRYDY29tMRcwFQYKCZImiZPyLGQBGRYHYmVudGxleTEcMBoGA1UEAxMTQmVudGxleS1JbnRlcm5hbC1DQTAeFw0xNzA4MjgxOTQ2MDdaFw0yMjA4MjcxOTQ2MDdaMH0xCzAJBgNVBAYTAlVTMQswCQYDVQQIEwJQQTEOMAwGA1UEBxMFRXh0b24xHDAaBgNVBAoTE0JlbnRsZXkgU3lzdGVtcyBJbmMxCzAJBgNVBAsTAklUMSYwJAYDVQQDEx1pbXMtdG9rZW4tc2lnbmluZy5iZW50bGV5LmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALnpXaPUjWevGxnIkY9bJsdatInIbo2StS3xAmVX3dd9uGUFu7HL4ciJMOlHFlwsASOGreMGdHVQmPnFgL2W5ekghzs\\/Vk\\/asXSYzWtVwQftS2VZZqTcuLrwaYNPznv6vaNPNTTbUI4kXgBCH0S+pA\\/ulhqF03dCopRCB4BR0z\\/9r1WrkxYzUF2fKhKifoyBaX8TqqEnw6ZKAyCMDVRN\\/Dm7ORVEDw\\/\\/iMO0vtXXjFPH3KV2EZn02+K5pdqWpkVzf9TSCfEQZL2JoYAfCVC6Z5gh5Dja+UTIfjJw45lTy4TPD+ivVpPcni6Wiln6i701OCYXMK1WxhwU1vV+eeaQvDUCAwEAAaOCAgYwggICMAsGA1UdDwQEAwIFoDAdBgNVHQ4EFgQUadvHSgG2syhu++t\\/OnkpOawqhOQwKAYDVR0RBCEwH4IdaW1zLXRva2VuLXNpZ25pbmcuYmVudGxleS5jb20wHwYDVR0jBBgwFoAUbjdsNQxJ7tInD1RS39J9x4\\/\\/Zt0wUQYDVR0fBEowSDBGoESgQoZAaHR0cDovL2V4dHByZGNhMDEuYmVudGxleS5jb20vQ2VydEVucm9sbC9CZW50bGV5LUludGVybmFsLUNBLmNybDCBxQYIKwYBBQUHAQEEgbgwgbUwgbIGCCsGAQUFBzAChoGlbGRhcDovLy9DTj1CZW50bGV5LUludGVybmFsLUNBLENOPUFJQSxDTj1QdWJsaWMlMjBLZXklMjBTZXJ2aWNlcyxDTj1TZXJ2aWNlcyxDTj1Db25maWd1cmF0aW9uLERDPWJzaXJvb3QsREM9Y29tP2NBQ2VydGlmaWNhdGU\\/YmFzZT9vYmplY3RDbGFzcz1jZXJ0aWZpY2F0aW9uQXV0aG9yaXR5MDwGCSsGAQQBgjcVBwQvMC0GJSsGAQQBgjcVCIXHrEDf+UuDsZcegeqVQoex+FwxgYOFKoGk0EgCAWQCAQYwEwYDVR0lBAwwCgYIKwYBBQUHAwEwGwYJKwYBBAGCNxUKBA4wDDAKBggrBgEFBQcDATANBgkqhkiG9w0BAQsFAAOCAQEAVyEM1YbcQbtxXpt9qheZ4VIDaCKmhyf1PyyqRQqqzZF9KKpbEnV\\/XRf0qSQNGO4CU6HwOp5zpOpCDX3pKOJYP3NRL6OkvU01jiDg6d9v9EyTd6sqVbEUJ7pKkzmGWkEL1URXPAZY6TiHShpMdkC5+BGLOSIXYcbdp2aMGRMT5Y6e+vWggvy4BUC1Ced9mULAKMSIQeEH76tLYKyLQ44ftqaYep+piGEdtEzah8S9bsS9dcbiIm+yeXiCgyNGvmV1SteaKLn+o2r\\/bU3BAzjA3slKLzZG5u295SeRh6+xRxbm4tOAq\\/s02uN7Jxn22GwXv\\/l+RRhpK4RmgPVnygmbUA==<\\/X509Certificate><\\/X509Data><\\/KeyInfo><\\/ds:Signature><\\/saml:Assertion> \",\"TokenType\":\"\"}");

    Json::Value body = Json::Reader::DoParse(responseBody2);
    printf("%s\n", responseBody2.c_str());

    if (!body["RequestedSecurityToken"].isString())
        {
        printf("Failure\n");
        BeAssert(false && "Token not found in IMS response");
        }
    printf("Success\n");

    printf("%d\n", BeXmlStatus::BEXML_Success);
    }
