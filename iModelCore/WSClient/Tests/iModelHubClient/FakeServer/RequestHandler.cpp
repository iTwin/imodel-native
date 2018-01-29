#include "RequestHandler.h"
#include <Bentley/BeTest.h>
#include "../../../iModelHubClient/Utils.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

RequestHandler::RequestHandler()
    {
    //BeFileName outPath("C:\\BSW\\Bim0200Dev2\\src\\DgnClientSdkTestData\\MockServer");
    ////BeTest::GetHost().GetOutputRoot(outPath);
    //serverPath = outPath.GetNameUtf8();
    BeFileName documentsDir;
    BeTest::GetHost().GetDocumentsRoot(documentsDir);
    documentsDir.AppendToPath(L"ImodelHubTestData");
    documentsDir.AppendToPath(L"iModelHubNativeTests");
    serverPath = documentsDir.GetNameUtf8();
    }

RequestHandler::~RequestHandler()
    {

    }

bvector<Utf8String> ParseUrl(Request req) 
    {
    Utf8String requestUrl = req.GetUrl();
    bvector<Utf8String> tokens;
    Utf8CP url = requestUrl.c_str();
    BeStringUtilities::Split(url, "/", nullptr, tokens);
    return tokens;
    }

Response ProjectSchemaClassiModelWithQuery()
    {
    /*Utf8CP url = "asd";
    HttpResponseContentPtr respContent;
    HttpBodyPtr body;
    respContent->Create(body);
    Response resp(respContent, url, ConnectionStatus::OK, HttpStatus::OK);*/
    Response resp;
    return resp;
    }
Response ProjectSchemaClassiModel()
    {
    Utf8CP url = "asd";
    HttpResponseContentPtr respContent;
    HttpBodyPtr body;
    respContent->Create(body);
    Response resp(respContent, url, ConnectionStatus::OK, HttpStatus::OK);
    //Response resp;
    return resp;
    }

Response GetProjectSchema(bvector<Utf8String> args) 
    {
    if (args.size() > 7)
        return ProjectSchemaClassiModelWithQuery();
    return ProjectSchemaClassiModel();
    }

Utf8String GetInstanceid(Utf8String str)
    {
    bvector<Utf8String> tokens;

    BeStringUtilities::Split(str.c_str(), "-",nullptr,  tokens);
    Utf8String instanceid(tokens[1]);
    instanceid.append("-");
    instanceid.append(tokens[2]);
    instanceid.append("-");
    instanceid.append(tokens[3]);
    instanceid.append("-");
    instanceid.append(tokens[4]);
    instanceid.append("-");
    instanceid.append(tokens[5]);

    return instanceid;
    }

Response RequestHandler::DownloadiModel(bvector<Utf8String> args)
    {
    //download by checking instanceid from db
    Utf8String instanceid = GetInstanceid(args[2]);

    BeFileName dbName("ServerRepo.db");
    BeFileName dbPath(serverPath);
    dbPath.AppendToPath(dbName);
    BentleyB0200::BeSQLite::Db m_db;
    CharCP filetoDownload;
    if (DbResult::BE_SQLITE_OK == m_db.OpenBeSQLiteDb(dbPath, BentleyB0200::BeSQLite::Db::OpenParams(BentleyB0200::BeSQLite::Db::OpenMode::Readonly, DefaultTxn::Yes)))
        {
        Statement st;
        st.Prepare(m_db, "Select Name from instances where instanceid = ?");
        st.BindText(1, instanceid, Statement::MakeCopy::No);

        printf("%d\n", st.Step());
        //printf("Success\n");
        filetoDownload = st.GetValueText(0);
        //printf("%s\n", filetoDownload);


        BeFileName downloadPath;
        BeTest::GetHost().GetOutputRoot(downloadPath);
        //outPath.AppendToPath(L"Server");
        downloadPath.AppendToPath(L"iModelHub");
        /*printf("%ls\n", downloadPath.GetWCharCP());
        printf("%s\n", serverPath.c_str());
        printf("%s\n", filetoDownload);*/
        FakeServer::DownloadiModel(downloadPath, serverPath.c_str(), filetoDownload);
        st.Finalize();
        }

    Response resp;
    return resp;
    }

Response RequestHandler::PerformGetRequest(Request req)
    {
    Utf8String urlExpected("https://qa-imodelhubapi.bentley.com/v2.0/Plugins");
    //identify the type of request
    // do the specific operation by passing to the concerned function
    // build up the response according to the action taken
    // return response
    bvector<Utf8String> args = ParseUrl(req);
    //get request with Project Schema
    Response resp;
    /*if (args[5] == ServerSchema::Schema::Project)
    resp = GetProjectSchema(args);*/
    if (!req.GetUrl().CompareTo(urlExpected))
        return RequestHandler::PluginRequest(req);
    if (1)//detect imodelhub
        resp = DownloadiModel(args);
    //get request with iModel--Nr
    return resp;
    }
DbResult RequestHandler::Initialize(BeFileName temporaryDir, BeSQLiteLib::LogErrors logSqliteErrors)
    {
    const DbResult stat = BeSQLiteLib::Initialize(temporaryDir, logSqliteErrors);
    return stat;
    }

void CreateTable(Utf8CP tableName, BentleyB0200::BeSQLite::Db& db, Utf8CP ddl) 
    {
    if (!db.TableExists(tableName))
        ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateTable(tableName, ddl));
    }

void RequestHandler::CheckDb() 
    {
    BentleyB0200::BeSQLite::Db m_db;
    BeFileName dbName("ServerRepo.db");
    BeFileName dbPath(serverPath);
    dbPath.AppendToPath(dbName);
    if (dbPath.DoesPathExist())
        {
        ASSERT_EQ(DbResult::BE_SQLITE_OK, m_db.OpenBeSQLiteDb(dbPath, BentleyB0200::BeSQLite::Db::OpenParams(BentleyB0200::BeSQLite::Db::OpenMode::ReadWrite, DefaultTxn::Yes)));
        if(!m_db.TableExists("instances"))
            CreateTable("instances", m_db, "instanceid STRING, className STRING, schemaName STRING, Description STRING, Name STRING, Initialized STRING");
        if(!m_db.TableExists("users"))
            CreateTable("users", m_db, "UserCreated STRING, CreatedDate STRING");
        }
    m_db.CloseDb();
    }

void RequestHandler::Insert(bvector<Utf8String> insertStr) 
    {
    BeFileName dbName("ServerRepo.db");
    BeFileName dbPath(serverPath);
    dbPath.AppendToPath(dbName);
    BentleyB0200::BeSQLite::Db m_db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, m_db.OpenBeSQLiteDb(dbPath, BentleyB0200::BeSQLite::Db::OpenParams(BentleyB0200::BeSQLite::Db::OpenMode::ReadWrite, DefaultTxn::Yes)));
    //ASSERT_EQ(DbResult::BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO instances(instanceid, className, schemaName, Description, Name, Initialized) VALUES ('sa','sa', 'sa','sa','sa','sa')"));
    Statement insertSt;
    insertSt.Prepare(m_db, "INSERT INTO instances(instanceid, className, schemaName, Description, Name) VALUES (?,?,?,?,?)");
    insertSt.BindText(1, insertStr[0], Statement::MakeCopy::No);
    insertSt.BindText(2, insertStr[1], Statement::MakeCopy::No);
    insertSt.BindText(3, insertStr[2], Statement::MakeCopy::No);
    insertSt.BindText(4, insertStr[3], Statement::MakeCopy::No);
    insertSt.BindText(5, insertStr[4], Statement::MakeCopy::No);
    ASSERT_EQ(BE_SQLITE_DONE, insertSt.Step());
    insertSt.Finalize();
/*
    ASSERT_EQ(BE_SQLITE_OK, m_db.DropTable("instances"));
    ASSERT_EQ(BE_SQLITE_OK, m_db.DropTable("users"));*/
    m_db.CloseDb();
    }


Http::Response StubJsonHttpResponse(HttpStatus httpStatus, Utf8CP url, Utf8StringCR body, const bmap<Utf8String, Utf8String>& headers = bmap<Utf8String, Utf8String> ())
    {
    auto newHeaders = headers;
    newHeaders["Content-Type"] = "application/json";
    ConnectionStatus status = ConnectionStatus::OK;
    auto content = HttpResponseContent::Create(HttpStringBody::Create(body));
    for (const auto& header : headers)
        {
        content->GetHeaders().SetValue(header.first, header.second);
        }
    return Http::Response(content, url, status, httpStatus);
    }

Response RequestHandler::CreateiModel(Request req)
    {

    
    HttpBodyPtr reqBody = req.GetRequestBody();
    char readBuff[1000] ;
    size_t buffSize = 100000;
    reqBody->Read(readBuff, buffSize);
    Utf8String reqBodyRead(readBuff);

    Json::Reader reader;
    Json::Value settings;
    reader.Parse(reqBodyRead, settings);
    BeGuid projGuid(true);
    printf("%s\n", projGuid.ToString().c_str());
    bvector<Utf8String> input = {   projGuid.ToString(),  
        settings["instance"]["className"].asString(),
        settings["instance"]["schemaName"].asString(),
        settings["instance"]["properties"]["Description"].asString(), 
        settings["instance"]["properties"]["Name"].asString()};

    Utf8String fileName(settings["instance"]["properties"]["Name"].asString());
    BeFileName fileToCreate(fileName);
    BeFileName servPath(serverPath);
    if (BeFileNameStatus::Success == FakeServer::CreateiModel(servPath, fileToCreate.GetWCharCP()))
        {
        CheckDb();
        Insert(input);

        }


    Json::Value iModelCreation(Json::objectValue);
    JsonValueR changedInstance = iModelCreation[ServerSchema::ChangedInstance] = Json::objectValue;
    changedInstance["change"] = "Created";
    JsonValueR InstanceAfterChange = changedInstance[ServerSchema::InstanceAfterChange] = Json::objectValue;
    InstanceAfterChange[ServerSchema::InstanceId] = projGuid.ToString();
    InstanceAfterChange[ServerSchema::SchemaName] = ServerSchema::Schema::Project;
    InstanceAfterChange[ServerSchema::ClassName] = ServerSchema::Class::iModel;
    JsonValueR properties = InstanceAfterChange[ServerSchema::Properties] = Json::objectValue;
    properties[ServerSchema::Property::Description] = settings["instance"]["properties"]["Description"].asString();
    properties[ServerSchema::Property::Name] = settings["instance"]["properties"]["Name"].asString();
    properties[ServerSchema::Property::UserCreated] = "";


    Utf8String contentToWrite(Json::FastWriter().write(iModelCreation));

    //Response resp(m_content, req.GetUrl().c_str(),ConnectionStatus::OK, HttpStatus::Created);

    return StubJsonHttpResponse(HttpStatus::Created, req.GetUrl().c_str(), contentToWrite);
    }

Response RequestHandler::PluginRequest(Request req)
    {
    const bmap<Utf8String, Utf8String>& headers = bmap<Utf8String, Utf8String>();
    auto newHeaders = headers;
    newHeaders["Content-Type"] = "application/json";
    Utf8String responseBody("{\"instances\":[{\"instanceId\":\"Project\",\"schemaName\":\"Plugins\",\"className\":\"PluginIdentifier\",\"properties\":{\"ECPluginID\":\"Project\",\"DisplayLabel\":\"Project\"},\"eTag\":\"\\\"XX8D88hmQX54h3Mq4muPnVO0yVQ=\\\"\"},{\"instanceId\":\"iModel\",\"schemaName\":\"Plugins\",\"className\":\"PluginIdentifier\",\"properties\":{\"ECPluginID\":\"iModel\",\"DisplayLabel\":\"iModel\"},\"eTag\":\"\\\"rCWzHX/X9jqKDPqDqq1rSrXMseI=\\\"\"},{\"instanceId\":\"Bentley.ECServices\",\"schemaName\":\"Plugins\",\"className\":\"PluginIdentifier\",\"properties\":{\"ECPluginID\":\"Bentley.ECServices\",\"DisplayLabel\":\"Bentley.ECServices\"},\"eTag\":\"\\\"HqP0PTZyesAOhccoX8fGr3fosBk=\\\"\"}]}");
    auto content = HttpResponseContent::Create(HttpStringBody::Create(responseBody));
    for (const auto& header : headers)
        {
        content->GetHeaders().SetValue(header.first, header.second);
        }
    content->GetHeaders().SetValue(Utf8String("Mas-Server"), Utf8String("Bentley-WSG/02.06.04.04, Bentley-WebAPI/2.7"));
    return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::OK);
    }

Response RequestHandler::BuddiRequest(Request req) 
    {
    const bmap<Utf8String, Utf8String>& headers = bmap<Utf8String, Utf8String>();
    auto newHeaders = headers;
    newHeaders["Content-Type"] = "application/json";
    Utf8String responseBody;
    if (req.GetRequestBody()->AsString().Contains("Mobile.ImsStsAuth"))
        responseBody = "<?xml version=\"1.0\" encoding=\"utf-8\"?><soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"><soap:Body><GetUrlResponse xmlns=\"http://tempuri.org/\"><GetUrlResult>https://qa-ims.bentley.com/rest/ActiveSTSService/json/IssueEx</GetUrlResult></GetUrlResponse></soap:Body></soap:Envelope>";
    if (req.GetRequestBody()->AsString().Contains("iModelHubApi"))
        responseBody = "<?xml version=\"1.0\" encoding=\"utf-8\"?><soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"><soap:Body><GetUrlResponse xmlns=\"http://tempuri.org/\"><GetUrlResult>https://qa-imodelhubapi.bentley.com</GetUrlResult></GetUrlResponse></soap:Body></soap:Envelope>";

    auto content = HttpResponseContent::Create(HttpStringBody::Create(responseBody));
    for (const auto& header : headers)
        {
        content->GetHeaders().SetValue(header.first, header.second);
        }
    return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::OK);
    }

Response RequestHandler::ImsTokenRequest(Request req) 
    {
    
    const bmap<Utf8String, Utf8String>& headers = bmap<Utf8String, Utf8String>();
    auto newHeaders = headers;
    newHeaders["Content-Type"] = "application/json";
    Utf8String responseBody("{\"RequestedSecurityToken\":\"<saml:Assertion MajorVersion=\\\"1\\\" MinorVersion=\\\"1\\\" AssertionID=\\\"_f0e79a5d-6b8b-4f6c-9dc2-84950140a776\\\" Issuer=\\\"https:\\/\\/qa-ims.bentley.com\\/\\\" IssueInstant=\\\"2018-01-23T06:59:22.817Z\\\" xmlns:saml=\\\"urn:oasis:names:tc:SAML:1.0:assertion\\\"><saml:Conditions NotBefore=\\\"2018-01-23T06:59:22.785Z\\\" NotOnOrAfter=\\\"2018-01-30T06:59:22.785Z\\\"><saml:AudienceRestrictionCondition><saml:Audience>sso:\\/\\/wsfed_desktop\\/1654<\\/saml:Audience><\\/saml:AudienceRestrictionCondition><\\/saml:Conditions><saml:AttributeStatement><saml:Subject><saml:NameIdentifier>87313509-6248-41e0-b43f-62aa4513a3e4<\\/saml:NameIdentifier><saml:SubjectConfirmation><saml:ConfirmationMethod>urn:oasis:names:tc:SAML:1.0:cm:holder-of-key<\\/saml:ConfirmationMethod><KeyInfo xmlns=\\\"http:\\/\\/www.w3.org\\/2000\\/09\\/xmldsig#\\\"><trust:BinarySecret xmlns:trust=\\\"http:\\/\\/docs.oasis-open.org\\/ws-sx\\/ws-trust\\/200512\\\">fS7XRoWI0KsO1u0L8dtk96kaxmf4yxxV74dPz9DlWKE=<\\/trust:BinarySecret><\\/KeyInfo><\\/saml:SubjectConfirmation><\\/saml:Subject><saml:Attribute AttributeName=\\\"name\\\" AttributeNamespace=\\\"http:\\/\\/schemas.xmlsoap.org\\/ws\\/2005\\/05\\/identity\\/claims\\\"><saml:AttributeValue>farhad.kabir@bentley.com<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"givenname\\\" AttributeNamespace=\\\"http:\\/\\/schemas.xmlsoap.org\\/ws\\/2005\\/05\\/identity\\/claims\\\"><saml:AttributeValue>Farhad<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"surname\\\" AttributeNamespace=\\\"http:\\/\\/schemas.xmlsoap.org\\/ws\\/2005\\/05\\/identity\\/claims\\\"><saml:AttributeValue>Kabir<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"emailaddress\\\" AttributeNamespace=\\\"http:\\/\\/schemas.xmlsoap.org\\/ws\\/2005\\/05\\/identity\\/claims\\\"><saml:AttributeValue>farhad.kabir@bentley.com<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"role\\\" AttributeNamespace=\\\"http:\\/\\/schemas.microsoft.com\\/ws\\/2008\\/06\\/identity\\/claims\\\"><saml:AttributeValue>BENTLEY_EMPLOYEE<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"sapbupa\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>1004183475<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"site\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>1004174721<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"ultimatesite\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>1001389117<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"sapentitlement\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>INTERNAL<\\/saml:AttributeValue><saml:AttributeValue>BENTLEY_LEARN<\\/saml:AttributeValue><saml:AttributeValue>SELECT_2006<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"entitlement\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>BENTLEY_EMPLOYEE<\\/saml:AttributeValue><saml:AttributeValue>BENTLEY_LEARN<\\/saml:AttributeValue><saml:AttributeValue>SELECT_2006<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"countryiso\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>PK<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"languageiso\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>EN<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"ismarketingprospect\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>false<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"isbentleyemployee\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>true<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"becommunitiesusername\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>87313509-6248-41E0-B43F-62AA4513A3E4<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"becommunitiesemailaddress\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>Farhad.Kabir@bentley.com<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"userid\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>87313509-6248-41e0-b43f-62aa4513a3e4<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"organization\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>Bentley Systems Inc<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"has_select\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>true<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"organizationid\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>e82a584b-9fae-409f-9581-fd154f7b9ef9<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"ultimateid\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>72adad30-c07c-465d-a1fe-2f2dfac950a4<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"ultimatereferenceid\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>72adad30-c07c-465d-a1fe-2f2dfac950a4<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"usagecountryiso\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>PK<\\/saml:AttributeValue><\\/saml:Attribute><\\/saml:AttributeStatement><ds:Signature xmlns:ds=\\\"http:\\/\\/www.w3.org\\/2000\\/09\\/xmldsig#\\\"><ds:SignedInfo><ds:CanonicalizationMethod Algorithm=\\\"http:\\/\\/www.w3.org\\/2001\\/10\\/xml-exc-c14n#\\\" \\/><ds:SignatureMethod Algorithm=\\\"http:\\/\\/www.w3.org\\/2001\\/04\\/xmldsig-more#rsa-sha256\\\" \\/><ds:Reference URI=\\\"#_f0e79a5d-6b8b-4f6c-9dc2-84950140a776\\\"><ds:Transforms><ds:Transform Algorithm=\\\"http:\\/\\/www.w3.org\\/2000\\/09\\/xmldsig#enveloped-signature\\\" \\/><ds:Transform Algorithm=\\\"http:\\/\\/www.w3.org\\/2001\\/10\\/xml-exc-c14n#\\\" \\/><\\/ds:Transforms><ds:DigestMethod Algorithm=\\\"http:\\/\\/www.w3.org\\/2001\\/04\\/xmlenc#sha256\\\" \\/><ds:DigestValue>d9YJQ24N1Yqgy5t2goeaXxC+e4YKLSVkv3jV3sOu0t4=<\\/ds:DigestValue><\\/ds:Reference><\\/ds:SignedInfo><ds:SignatureValue>LQwKIaerw5fJbtgUdi4nyrXaNS1I+R90z+hBruz9\\/pK3EABOPit4n673yt5LoMStkUY+Pori8Lmi+1xMd6qgTmo3Rlv8owwFXmTpGqZGlCsS67\\/yI3t8+fWjqP5T97\\/FCfOvM9WAu4jiiKkZd4BhaabImGKlSZpsAIFtTrvFnapX8UBRKDnXZIgoYJd8F1Q4Ky\\/qBfVBUOnQfOwoqo8X9xz65tZfN9\\/f6+3hZMcDdyC2r63Cm09g\\/IEvcItIAAQWWExiMzUFdjfg\\/fnYzt\\/QHhjCeKCsy9c94j71NFj\\/sa5fPgp6D+gp130Aqim+gsQ+wj4mgBT6RWH9zej5T2nAeA==<\\/ds:SignatureValue><KeyInfo xmlns=\\\"http:\\/\\/www.w3.org\\/2000\\/09\\/xmldsig#\\\"><X509Data><X509Certificate>MIIFXjCCBEagAwIBAgITXwAAMAAnZoWkOOCfDgAAAAAwADANBgkqhkiG9w0BAQsFADBMMRMwEQYKCZImiZPyLGQBGRYDY29tMRcwFQYKCZImiZPyLGQBGRYHYmVudGxleTEcMBoGA1UEAxMTQmVudGxleS1JbnRlcm5hbC1DQTAeFw0xNzA4MjgxOTQ2MDdaFw0yMjA4MjcxOTQ2MDdaMH0xCzAJBgNVBAYTAlVTMQswCQYDVQQIEwJQQTEOMAwGA1UEBxMFRXh0b24xHDAaBgNVBAoTE0JlbnRsZXkgU3lzdGVtcyBJbmMxCzAJBgNVBAsTAklUMSYwJAYDVQQDEx1pbXMtdG9rZW4tc2lnbmluZy5iZW50bGV5LmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALnpXaPUjWevGxnIkY9bJsdatInIbo2StS3xAmVX3dd9uGUFu7HL4ciJMOlHFlwsASOGreMGdHVQmPnFgL2W5ekghzs\\/Vk\\/asXSYzWtVwQftS2VZZqTcuLrwaYNPznv6vaNPNTTbUI4kXgBCH0S+pA\\/ulhqF03dCopRCB4BR0z\\/9r1WrkxYzUF2fKhKifoyBaX8TqqEnw6ZKAyCMDVRN\\/Dm7ORVEDw\\/\\/iMO0vtXXjFPH3KV2EZn02+K5pdqWpkVzf9TSCfEQZL2JoYAfCVC6Z5gh5Dja+UTIfjJw45lTy4TPD+ivVpPcni6Wiln6i701OCYXMK1WxhwU1vV+eeaQvDUCAwEAAaOCAgYwggICMAsGA1UdDwQEAwIFoDAdBgNVHQ4EFgQUadvHSgG2syhu++t\\/OnkpOawqhOQwKAYDVR0RBCEwH4IdaW1zLXRva2VuLXNpZ25pbmcuYmVudGxleS5jb20wHwYDVR0jBBgwFoAUbjdsNQxJ7tInD1RS39J9x4\\/\\/Zt0wUQYDVR0fBEowSDBGoESgQoZAaHR0cDovL2V4dHByZGNhMDEuYmVudGxleS5jb20vQ2VydEVucm9sbC9CZW50bGV5LUludGVybmFsLUNBLmNybDCBxQYIKwYBBQUHAQEEgbgwgbUwgbIGCCsGAQUFBzAChoGlbGRhcDovLy9DTj1CZW50bGV5LUludGVybmFsLUNBLENOPUFJQSxDTj1QdWJsaWMlMjBLZXklMjBTZXJ2aWNlcyxDTj1TZXJ2aWNlcyxDTj1Db25maWd1cmF0aW9uLERDPWJzaXJvb3QsREM9Y29tP2NBQ2VydGlmaWNhdGU\\/YmFzZT9vYmplY3RDbGFzcz1jZXJ0aWZpY2F0aW9uQXV0aG9yaXR5MDwGCSsGAQQBgjcVBwQvMC0GJSsGAQQBgjcVCIXHrEDf+UuDsZcegeqVQoex+FwxgYOFKoGk0EgCAWQCAQYwEwYDVR0lBAwwCgYIKwYBBQUHAwEwGwYJKwYBBAGCNxUKBA4wDDAKBggrBgEFBQcDATANBgkqhkiG9w0BAQsFAAOCAQEAVyEM1YbcQbtxXpt9qheZ4VIDaCKmhyf1PyyqRQqqzZF9KKpbEnV\\/XRf0qSQNGO4CU6HwOp5zpOpCDX3pKOJYP3NRL6OkvU01jiDg6d9v9EyTd6sqVbEUJ7pKkzmGWkEL1URXPAZY6TiHShpMdkC5+BGLOSIXYcbdp2aMGRMT5Y6e+vWggvy4BUC1Ced9mULAKMSIQeEH76tLYKyLQ44ftqaYep+piGEdtEzah8S9bsS9dcbiIm+yeXiCgyNGvmV1SteaKLn+o2r\\/bU3BAzjA3slKLzZG5u295SeRh6+xRxbm4tOAq\\/s02uN7Jxn22GwXv\\/l+RRhpK4RmgPVnygmbUA==<\\/X509Certificate><\\/X509Data><\\/KeyInfo><\\/ds:Signature><\\/saml:Assertion> \",\"TokenType\":\"\"}");
    auto content = HttpResponseContent::Create(HttpStringBody::Create(responseBody));
    for (const auto& header : headers)
        {
        content->GetHeaders().SetValue(header.first, header.second);
        }
    return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::OK);
    }

Response RequestHandler::PerformOtherRequest(Request req)
    {
    printf("%s\n", req.GetUrl().c_str());
    Utf8String urlExpected("https://buddi.bentley.com/discovery.asmx");
    Utf8String urlExpected2("https://qa-ims.bentley.com/rest/ActiveSTSService/json/IssueEx");
    Utf8String urlExpected3("https://qa-ims.bentley.com/rest/DelegationSTSService/json/IssueEx");
    if (!req.GetUrl().CompareTo(urlExpected))
        return RequestHandler::BuddiRequest(req);
    if (!req.GetUrl().CompareTo(urlExpected2))
        return RequestHandler::ImsTokenRequest(req);
    if (!req.GetUrl().CompareTo(urlExpected3))
        return RequestHandler::ImsTokenRequest(req);
    return RequestHandler::CreateiModel(req);
    }