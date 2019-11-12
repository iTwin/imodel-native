/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <FakeServer/RequestHandler.h>
#include <Bentley/BeTest.h>
#include "../iModelHubClient/Utils.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RequestHandler::RequestHandler()
    {
    BeFileName outPath;
    BeTest::GetHost().GetOutputRoot(outPath);
    outPath.AppendToPath(L"iModelHubServer");
    if (!outPath.DoesPathExist())
        BeFileName::CreateNewDirectory(outPath);
    serverPath = outPath.GetNameUtf8();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RequestHandler::~RequestHandler()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void CreateTable(Utf8CP tableName, BentleyM0200::BeSQLite::Db& db, Utf8CP ddl)
    {
    if (!db.TableExists(tableName))
        if (DbResult::BE_SQLITE_OK != db.CreateTable(tableName, ddl))
            return;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String GetInstanceid(Utf8String str)
    {
    bvector<Utf8String> tokens;

    BeStringUtilities::Split(str.c_str(), "-", nullptr, tokens);
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> ParseUrl(Request req, Utf8CP delimiter)
    {
    Utf8String requestUrl = req.GetUrl();
    bvector<Utf8String> tokens;
    Utf8CP url = requestUrl.c_str();
    BeStringUtilities::Split(url, delimiter, nullptr, tokens);
    return tokens;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Response RequestHandler::PluginRequest(Request req)
    {
    const bmap<Utf8String, Utf8String>& headers = bmap<Utf8String, Utf8String>();
    auto newHeaders = headers;
    newHeaders["Content-Type"] = "application/json";
    Utf8String responseBody = "{\"instances\":[{\"instanceId\":\"Project\",\"schemaName\":\"Plugins\",\"className\":\"PluginIdentifier\",\"properties\":{\"ECPluginID\":\"Project\",\"DisplayLabel\":\"Project\"},\"eTag\":\"\\\"XX8D88hmQX54h3Mq4muPnVO0yVQ=\\\"\"},{\"instanceId\":\"iModel\",\"schemaName\":\"Plugins\",\"className\":\"PluginIdentifier\",\"properties\":{\"ECPluginID\":\"iModel\",\"DisplayLabel\":\"iModel\"},\"eTag\":\"\\\"rCWzHX/X9jqKDPqDqq1rSrXMseI=\\\"\"},{\"instanceId\":\"Bentley.ECServices\",\"schemaName\":\"Plugins\",\"className\":\"PluginIdentifier\",\"properties\":{\"ECPluginID\":\"Bentley.ECServices\",\"DisplayLabel\":\"Bentley.ECServices\"},\"eTag\":\"\\\"HqP0PTZyesAOhccoX8fGr3fosBk=\\\"\"}]}";
    auto content = HttpResponseContent::Create(HttpStringBody::Create(responseBody));
    for (const auto& header : headers)
        content->GetHeaders().SetValue(header.first, header.second);
    content->GetHeaders().SetValue(Utf8String("Mas-Server"), Utf8String("Bentley-WSG/02.06.04.04, Bentley-WebAPI/2.7"));
    return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::OK);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
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
        content->GetHeaders().SetValue(header.first, header.second);
    return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::OK);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Response RequestHandler::ImsTokenRequest(Request req)
    {
    const bmap<Utf8String, Utf8String>& headers = bmap<Utf8String, Utf8String>();
    auto newHeaders = headers;
    newHeaders["Content-Type"] = "application/json";
    Utf8String responseBody = "{\"RequestedSecurityToken\":\"<saml:Assertion MajorVersion=\\\"1\\\" MinorVersion=\\\"1\\\" AssertionID=\\\"_f0e79a5d-6b8b-4f6c-9dc2-84950140a776\\\" Issuer=\\\"https:\\/\\/qa-ims.bentley.com\\/\\\" IssueInstant=\\\"2018-01-23T06:59:22.817Z\\\" xmlns:saml=\\\"urn:oasis:names:tc:SAML:1.0:assertion\\\"><saml:Conditions NotBefore=\\\"2018-01-23T06:59:22.785Z\\\" NotOnOrAfter=\\\"2018-01-30T06:59:22.785Z\\\"><saml:AudienceRestrictionCondition><saml:Audience>sso:\\/\\/wsfed_desktop\\/1654<\\/saml:Audience><\\/saml:AudienceRestrictionCondition><\\/saml:Conditions><saml:AttributeStatement><saml:Subject><saml:NameIdentifier>87313509-6248-41e0-b43f-62aa4513a3e4<\\/saml:NameIdentifier><saml:SubjectConfirmation><saml:ConfirmationMethod>urn:oasis:names:tc:SAML:1.0:cm:holder-of-key<\\/saml:ConfirmationMethod><KeyInfo xmlns=\\\"http:\\/\\/www.w3.org\\/2000\\/09\\/xmldsig#\\\"><trust:BinarySecret xmlns:trust=\\\"http:\\/\\/docs.oasis-open.org\\/ws-sx\\/ws-trust\\/200512\\\">fS7XRoWI0KsO1u0L8dtk96kaxmf4yxxV74dPz9DlWKE=<\\/trust:BinarySecret><\\/KeyInfo><\\/saml:SubjectConfirmation><\\/saml:Subject><saml:Attribute AttributeName=\\\"name\\\" AttributeNamespace=\\\"http:\\/\\/schemas.xmlsoap.org\\/ws\\/2005\\/05\\/identity\\/claims\\\"><saml:AttributeValue>farhad.kabir@bentley.com<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"givenname\\\" AttributeNamespace=\\\"http:\\/\\/schemas.xmlsoap.org\\/ws\\/2005\\/05\\/identity\\/claims\\\"><saml:AttributeValue>Farhad<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"surname\\\" AttributeNamespace=\\\"http:\\/\\/schemas.xmlsoap.org\\/ws\\/2005\\/05\\/identity\\/claims\\\"><saml:AttributeValue>Kabir<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"emailaddress\\\" AttributeNamespace=\\\"http:\\/\\/schemas.xmlsoap.org\\/ws\\/2005\\/05\\/identity\\/claims\\\"><saml:AttributeValue>farhad.kabir@bentley.com<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"role\\\" AttributeNamespace=\\\"http:\\/\\/schemas.microsoft.com\\/ws\\/2008\\/06\\/identity\\/claims\\\"><saml:AttributeValue>BENTLEY_EMPLOYEE<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"sapbupa\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>1004183475<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"site\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>1004174721<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"ultimatesite\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>1001389117<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"sapentitlement\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>INTERNAL<\\/saml:AttributeValue><saml:AttributeValue>BENTLEY_LEARN<\\/saml:AttributeValue><saml:AttributeValue>SELECT_2006<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"entitlement\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>BENTLEY_EMPLOYEE<\\/saml:AttributeValue><saml:AttributeValue>BENTLEY_LEARN<\\/saml:AttributeValue><saml:AttributeValue>SELECT_2006<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"countryiso\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>PK<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"languageiso\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>EN<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"ismarketingprospect\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>false<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"isbentleyemployee\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>true<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"becommunitiesusername\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>87313509-6248-41E0-B43F-62AA4513A3E4<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"becommunitiesemailaddress\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>Farhad.Kabir@bentley.com<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"userid\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>87313509-6248-41e0-b43f-62aa4513a3e4<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"organization\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>Bentley Systems Inc<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"has_select\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>true<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"organizationid\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>e82a584b-9fae-409f-9581-fd154f7b9ef9<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"ultimateid\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>72adad30-c07c-465d-a1fe-2f2dfac950a4<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"ultimatereferenceid\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>72adad30-c07c-465d-a1fe-2f2dfac950a4<\\/saml:AttributeValue><\\/saml:Attribute><saml:Attribute AttributeName=\\\"usagecountryiso\\\" AttributeNamespace=\\\"http:\\/\\/schemas.bentley.com\\/ws\\/2011\\/03\\/identity\\/claims\\\"><saml:AttributeValue>PK<\\/saml:AttributeValue><\\/saml:Attribute><\\/saml:AttributeStatement><ds:Signature xmlns:ds=\\\"http:\\/\\/www.w3.org\\/2000\\/09\\/xmldsig#\\\"><ds:SignedInfo><ds:CanonicalizationMethod Algorithm=\\\"http:\\/\\/www.w3.org\\/2001\\/10\\/xml-exc-c14n#\\\" \\/><ds:SignatureMethod Algorithm=\\\"http:\\/\\/www.w3.org\\/2001\\/04\\/xmldsig-more#rsa-sha256\\\" \\/><ds:Reference URI=\\\"#_f0e79a5d-6b8b-4f6c-9dc2-84950140a776\\\"><ds:Transforms><ds:Transform Algorithm=\\\"http:\\/\\/www.w3.org\\/2000\\/09\\/xmldsig#enveloped-signature\\\" \\/><ds:Transform Algorithm=\\\"http:\\/\\/www.w3.org\\/2001\\/10\\/xml-exc-c14n#\\\" \\/><\\/ds:Transforms><ds:DigestMethod Algorithm=\\\"http:\\/\\/www.w3.org\\/2001\\/04\\/xmlenc#sha256\\\" \\/><ds:DigestValue>d9YJQ24N1Yqgy5t2goeaXxC+e4YKLSVkv3jV3sOu0t4=<\\/ds:DigestValue><\\/ds:Reference><\\/ds:SignedInfo><ds:SignatureValue>LQwKIaerw5fJbtgUdi4nyrXaNS1I+R90z+hBruz9\\/pK3EABOPit4n673yt5LoMStkUY+Pori8Lmi+1xMd6qgTmo3Rlv8owwFXmTpGqZGlCsS67\\/yI3t8+fWjqP5T97\\/FCfOvM9WAu4jiiKkZd4BhaabImGKlSZpsAIFtTrvFnapX8UBRKDnXZIgoYJd8F1Q4Ky\\/qBfVBUOnQfOwoqo8X9xz65tZfN9\\/f6+3hZMcDdyC2r63Cm09g\\/IEvcItIAAQWWExiMzUFdjfg\\/fnYzt\\/QHhjCeKCsy9c94j71NFj\\/sa5fPgp6D+gp130Aqim+gsQ+wj4mgBT6RWH9zej5T2nAeA==<\\/ds:SignatureValue><KeyInfo xmlns=\\\"http:\\/\\/www.w3.org\\/2000\\/09\\/xmldsig#\\\"><X509Data><X509Certificate>MIIFXjCCBEagAwIBAgITXwAAMAAnZoWkOOCfDgAAAAAwADANBgkqhkiG9w0BAQsFADBMMRMwEQYKCZImiZPyLGQBGRYDY29tMRcwFQYKCZImiZPyLGQBGRYHYmVudGxleTEcMBoGA1UEAxMTQmVudGxleS1JbnRlcm5hbC1DQTAeFw0xNzA4MjgxOTQ2MDdaFw0yMjA4MjcxOTQ2MDdaMH0xCzAJBgNVBAYTAlVTMQswCQYDVQQIEwJQQTEOMAwGA1UEBxMFRXh0b24xHDAaBgNVBAoTE0JlbnRsZXkgU3lzdGVtcyBJbmMxCzAJBgNVBAsTAklUMSYwJAYDVQQDEx1pbXMtdG9rZW4tc2lnbmluZy5iZW50bGV5LmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALnpXaPUjWevGxnIkY9bJsdatInIbo2StS3xAmVX3dd9uGUFu7HL4ciJMOlHFlwsASOGreMGdHVQmPnFgL2W5ekghzs\\/Vk\\/asXSYzWtVwQftS2VZZqTcuLrwaYNPznv6vaNPNTTbUI4kXgBCH0S+pA\\/ulhqF03dCopRCB4BR0z\\/9r1WrkxYzUF2fKhKifoyBaX8TqqEnw6ZKAyCMDVRN\\/Dm7ORVEDw\\/\\/iMO0vtXXjFPH3KV2EZn02+K5pdqWpkVzf9TSCfEQZL2JoYAfCVC6Z5gh5Dja+UTIfjJw45lTy4TPD+ivVpPcni6Wiln6i701OCYXMK1WxhwU1vV+eeaQvDUCAwEAAaOCAgYwggICMAsGA1UdDwQEAwIFoDAdBgNVHQ4EFgQUadvHSgG2syhu++t\\/OnkpOawqhOQwKAYDVR0RBCEwH4IdaW1zLXRva2VuLXNpZ25pbmcuYmVudGxleS5jb20wHwYDVR0jBBgwFoAUbjdsNQxJ7tInD1RS39J9x4\\/\\/Zt0wUQYDVR0fBEowSDBGoESgQoZAaHR0cDovL2V4dHByZGNhMDEuYmVudGxleS5jb20vQ2VydEVucm9sbC9CZW50bGV5LUludGVybmFsLUNBLmNybDCBxQYIKwYBBQUHAQEEgbgwgbUwgbIGCCsGAQUFBzAChoGlbGRhcDovLy9DTj1CZW50bGV5LUludGVybmFsLUNBLENOPUFJQSxDTj1QdWJsaWMlMjBLZXklMjBTZXJ2aWNlcyxDTj1TZXJ2aWNlcyxDTj1Db25maWd1cmF0aW9uLERDPWJzaXJvb3QsREM9Y29tP2NBQ2VydGlmaWNhdGU\\/YmFzZT9vYmplY3RDbGFzcz1jZXJ0aWZpY2F0aW9uQXV0aG9yaXR5MDwGCSsGAQQBgjcVBwQvMC0GJSsGAQQBgjcVCIXHrEDf+UuDsZcegeqVQoex+FwxgYOFKoGk0EgCAWQCAQYwEwYDVR0lBAwwCgYIKwYBBQUHAwEwGwYJKwYBBAGCNxUKBA4wDDAKBggrBgEFBQcDATANBgkqhkiG9w0BAQsFAAOCAQEAVyEM1YbcQbtxXpt9qheZ4VIDaCKmhyf1PyyqRQqqzZF9KKpbEnV\\/XRf0qSQNGO4CU6HwOp5zpOpCDX3pKOJYP3NRL6OkvU01jiDg6d9v9EyTd6sqVbEUJ7pKkzmGWkEL1URXPAZY6TiHShpMdkC5+BGLOSIXYcbdp2aMGRMT5Y6e+vWggvy4BUC1Ced9mULAKMSIQeEH76tLYKyLQ44ftqaYep+piGEdtEzah8S9bsS9dcbiIm+yeXiCgyNGvmV1SteaKLn+o2r\\/bU3BAzjA3slKLzZG5u295SeRh6+xRxbm4tOAq\\/s02uN7Jxn22GwXv\\/l+RRhpK4RmgPVnygmbUA==<\\/X509Certificate><\\/X509Data><\\/KeyInfo><\\/ds:Signature><\\/saml:Assertion> \",\"TokenType\":\"\"}";
    auto content = HttpResponseContent::Create(HttpStringBody::Create(responseBody));
    for (const auto& header : headers)
        content->GetHeaders().SetValue(header.first, header.second);
    return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::OK);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RequestHandler::CreateTables(BentleyM0200::BeSQLite::Db *m_db)
    {
    if (!m_db->TableExists("Instances"))
        CreateTable("Instances", *m_db, "Id STRING, Name STRING, Description STRING, Briefcases INTEGER DEFAULT 1, UserCreated String, CreatedDate STRING, Initialized STRING");
    if (!m_db->TableExists("Users"))
        CreateTable("Users", *m_db, "UserCreated STRING, CreatedDate STRING");
    if (!m_db->TableExists("ChangeSets"))
        CreateTable("ChangeSets", *m_db, "Id STRING, Description STRING, iModelId STRING, FileSize INTEGER, BriefcaseId INTEGER, ParentId STRING, SeedFileId STRING, IndexNo INTEGER, IsUploaded BOOLEAN, UserCreated STRING, CreatedDate STRING");
    if (!m_db->TableExists("SeedFile"))
        CreateTable("SeedFile", *m_db, "Id STRING, FileName STRING, FileDescription STRING, FileSize INTEGER, iModelId STRING, IsUploaded BOOLEAN, UserUploaded STRING, UploadedDate STRING");
    if (!m_db->TableExists("Locks"))
        CreateTable("Locks", *m_db, "ObjectId STRING, iModelId STRING, LockType INTEGER, LockLevel INTEGER, BriefcaseId INTEGER, ReleasedWithChangeSet STRING, ReleasedWithChangeSetIndex INTEGER, Id String");
    if (!m_db->TableExists("Briefcases"))
        CreateTable("Briefcases", *m_db, "BriefcaseId INTEGER, iModelId STRING, MergedChangeSetId STRING");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RequestHandler::CheckDb()
    {
    BentleyM0200::BeSQLite::Db m_db;
    BeFileName dbPath = GetDbPath();
    if (dbPath.DoesPathExist())
        {
        if (DbResult::BE_SQLITE_OK != m_db.OpenBeSQLiteDb(dbPath, BentleyM0200::BeSQLite::Db::OpenParams(BentleyM0200::BeSQLite::Db::OpenMode::ReadWrite, DefaultTxn::Yes)))
            return;
        CreateTables(&m_db);
        }
    else
        {
        m_db.CreateNewDb(dbPath);
        CreateTables(&m_db);
        }
    m_db.CloseDb();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ParseUrlFilter(Utf8String filter, Utf8CP table = "")
    {
    bvector<Utf8String> tokens, tokens2;
    BeStringUtilities::Split(filter.c_str(), "=", nullptr, tokens);
    if (filter.Contains("$select")) BeStringUtilities::Split(tokens[2].c_str(), "$+", nullptr, tokens2);
    else BeStringUtilities::Split(tokens[1].c_str(), "$+", nullptr, tokens2);
    tokens.clear();
    Utf8String filterQuery = "";
    for (size_t i = 0; i < (tokens2.size() + 1) / 4; i++)
        {
        if (!tokens2[4 * i].Contains("FollowingChangeSet"))
            {
            if (i != 0) if (tokens2[4 * i - 1].Contains("and") || tokens2[4 * i - 1].Contains("or")) filterQuery.append(tokens2[4 * i - 1]);
            filterQuery.append(" ");

            filterQuery.append(tokens2[4 * i]);

            if (tokens2[4 * i + 1].Contains("eq")) filterQuery.append(" = ");
            else if (tokens2[4 * i + 1].Contains("ne")) filterQuery.append(" != ");
            else if (tokens2[4 * i + 1].Contains("gt")) filterQuery.append(" > ");
            else if (tokens2[4 * i + 1].Contains("in")) filterQuery.append(" IN ");

            if (tokens2[4 * i + 2].Contains("%5B"))
                {
                BeStringUtilities::Split(tokens2[4 * i + 2].c_str(), "'", nullptr, tokens);
                filterQuery.append("( Select ");
                filterQuery.append(tokens2[4 * i]);
                filterQuery.append(" from ");
                filterQuery.append(table);
                filterQuery.append(" where ");
                for (size_t j = 1; j < tokens.size() - 1; j++)
                    {
                    if (j % 2 == 0) continue;
                    if (j != 1)filterQuery.append(" or ");
                    filterQuery.append(tokens2[4 * i]);
                    filterQuery.append(" LIKE ");
                    filterQuery.append("'");
                    filterQuery.append(tokens[j]);
                    filterQuery.append("%'");
                    }
                filterQuery.append(")");
                }
            else filterQuery.append(tokens2[4 * i + 2]);
            filterQuery.append(" ");
            }
        else
            {
            if (i != 0) filterQuery.append(tokens2[4 * i - 1]);
            filterQuery.append("IndexNo > (Select IndexNo from ChangeSets where Id = ");
            filterQuery.append(tokens2[4 * i + 2]);
            filterQuery.append(")");
            }
        }
    return filterQuery.c_str();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RequestHandler::Insert(bvector<Utf8String> insertStr)
    {
    BeFileName dbPath = GetDbPath();
    BentleyM0200::BeSQLite::Db m_db;
    if (DbResult::BE_SQLITE_OK != m_db.OpenBeSQLiteDb(dbPath, BentleyM0200::BeSQLite::Db::OpenParams(BentleyM0200::BeSQLite::Db::OpenMode::ReadWrite, DefaultTxn::Yes)))
        return;

    Statement insertSt;
    insertSt.Prepare(m_db, "INSERT INTO Instances(Id, Name, Description) VALUES (?,?,?)");
    insertSt.BindText(1, insertStr[0], Statement::MakeCopy::No);
    insertSt.BindText(2, insertStr[1], Statement::MakeCopy::No);
    insertSt.BindText(3, insertStr[2], Statement::MakeCopy::No);
    if (BE_SQLITE_DONE != insertSt.Step())
        return;
    insertSt.Finalize();
    m_db.CloseDb();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Http::Response StubJsonHttpResponse(HttpStatus httpStatus, Utf8CP url, Utf8StringCR body, const bmap<Utf8String, Utf8String>& headers = bmap<Utf8String, Utf8String>())
    {
    auto newHeaders = headers;
    newHeaders["Content-Type"] = "application/json";
    ConnectionStatus status = ConnectionStatus::OK;
    auto content = HttpResponseContent::Create(HttpStringBody::Create(body));
    for (const auto& header : headers)
        content->GetHeaders().SetValue(header.first, header.second);
    return Http::Response(content, url, status, httpStatus);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value ParsedJson(Request req)
    {
    auto reqBody = req.GetRequestBody()->AsString();
    Json::Reader reader;
    Json::Value settings;
    reader.Parse(reqBody.c_str(), settings);
    return settings;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Response RequestHandler::CreateiModelInstance(Request req)
    {
    BeGuid projGuid(true);
    Json::Value settings = ParsedJson(req);
    bvector<Utf8String> input = 
        {
        projGuid.ToString(),
        settings["instance"]["properties"]["Name"].asString(),
        settings["instance"]["properties"]["Description"].asString() 
        };

    const auto iModelAlreadyExists = [&](Utf8StringCR name)
        {
        BeFileName dbPath = GetDbPath();
        BentleyM0200::BeSQLite::Db m_db;
        if (DbResult::BE_SQLITE_OK == m_db.OpenBeSQLiteDb(dbPath, BentleyM0200::BeSQLite::Db::OpenParams(BentleyM0200::BeSQLite::Db::OpenMode::ReadWrite, DefaultTxn::Yes)))
            {
            Statement st;
            st.Prepare(m_db, "SELECT Name FROM Instances WHERE Name = ?");
            st.BindText(1, name, Statement::MakeCopy::No);
            auto dbResult = st.Step();
            //auto text = st.GetValueText(0);
            if (DbResult::BE_SQLITE_DONE != dbResult)
                return true;
            }
        return false;
        };

    CheckDb();
    if (iModelAlreadyExists(input[1]))
        {
        static Utf8String alreadyExistsError = R"JSON({"errorId": "iModelHub.iModelAlreadyExists",
                                                       "errorMessage": "iModel already exists.",
                                                       "errorDescription": null,
                                                       "iModelInitialized": false})JSON";
        auto content = HttpResponseContent::Create(HttpStringBody::Create(alreadyExistsError));
        return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::Conflict);
        }

    Insert(input);
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
    properties[ServerSchema::Property::UserCreated] = "DummyUserValue";
    properties[ServerSchema::Property::CreatedDate] = DateTime::GetCurrentTimeUtc().ToString();
    properties[ServerSchema::Property::Initialized] = false;
    Utf8String contentToWrite = Json::FastWriter().write(iModelCreation);
    return StubJsonHttpResponse(HttpStatus::Created, req.GetUrl().c_str(), contentToWrite);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Response RequestHandler::CreateSeedFileInstance(Request req)
    {
    Json::Value settings = ParsedJson(req);
    bvector<Utf8String> args = ParseUrl(req, "/");
    Utf8String iModelid = GetInstanceid(args[4]);
    bvector<Utf8String> input = 
        {
        settings[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::FileId].asString(),
        settings[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::FileName].asString(),
        settings[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::FileDescription].asString(),
        iModelid
        };

    BeFileName dbPath = GetDbPath();
    BentleyM0200::BeSQLite::Db m_db;
    if (DbResult::BE_SQLITE_OK == m_db.OpenBeSQLiteDb(dbPath, BentleyM0200::BeSQLite::Db::OpenParams(BentleyM0200::BeSQLite::Db::OpenMode::ReadWrite, DefaultTxn::Yes)))
        {
        Statement insertSt;
        insertSt.Prepare(m_db, "INSERT INTO SeedFile (Id, FileName, FileDescription, FileSize, iModelId) VALUES (?,?,?,?,?)");
        insertSt.BindText(1, input[0], Statement::MakeCopy::No);
        insertSt.BindText(2, input[1], Statement::MakeCopy::No);
        insertSt.BindText(3, input[2], Statement::MakeCopy::No);
        insertSt.BindInt(4, settings[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::FileSize].asInt());
        insertSt.BindText(5, input[3], Statement::MakeCopy::No);
        if (BE_SQLITE_DONE != insertSt.Step())
            return Response();

        Json::Value instancesinfo(Json::objectValue);
        JsonValueR changedInstance = instancesinfo[ServerSchema::ChangedInstance] = Json::objectValue;
        changedInstance["change"] = "Created";
        JsonValueR instance = changedInstance[ServerSchema::InstanceAfterChange] = Json::objectValue;
        instance[ServerSchema::InstanceId] = input[0].c_str();
        instance[ServerSchema::SchemaName] = "iModelScope";
        instance[ServerSchema::ClassName] = "SeedFile";
        JsonValueR properties = instance[ServerSchema::Properties] = Json::objectValue;
        properties[ServerSchema::Property::FileDescription] = input[2].c_str();
        properties[ServerSchema::Property::FileId] = input[0].c_str();
        properties[ServerSchema::Property::FileName] = input[1].c_str();
        properties[ServerSchema::Property::FileSize] = settings[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::FileSize].ToString().c_str();
        properties[ServerSchema::Property::MergedChangeSetId] = "";
        properties[ServerSchema::Property::IsUploaded] = false;
        properties[ServerSchema::Property::Index] = 0;
        JsonValueR relationshipInstances = instance[ServerSchema::RelationshipInstances] = Json::arrayValue;
        JsonValueR relationsArray = relationshipInstances[0] = Json::objectValue;
        relationsArray[ServerSchema::InstanceId] = "";
        relationsArray[ServerSchema::SchemaName] = "iModelScope";
        relationsArray[ServerSchema::ClassName] = "FileAccessKey";
        relationsArray["direction"] = "forward";
        JsonValueR relatedInstance = relationsArray[ServerSchema::RelatedInstance] = Json::objectValue;;
        relatedInstance[ServerSchema::InstanceId] = "";
        relatedInstance[ServerSchema::SchemaName] = "iModelScope";
        relatedInstance[ServerSchema::ClassName] = "AccessKey";
        JsonValueR propertiesl2 = relatedInstance[ServerSchema::Properties] = Json::objectValue;
        Utf8String UploadUrl = "https://imodelhubqasa01.blob.core.windows.net/imodelhub-";
        UploadUrl += iModelid + "/BriefcaseTestsu-" + input[0];
        propertiesl2[ServerSchema::Property::UploadUrl] = UploadUrl.c_str();
        propertiesl2[ServerSchema::Property::DownloadUrl] = NULL;
        Utf8String contentToWrite = Json::FastWriter().write(instancesinfo);

        insertSt.Finalize();
        auto content = HttpResponseContent::Create(HttpStringBody::Create(contentToWrite));
        return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::Created);
        }
    return Response();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Response RequestHandler::UploadSeedFile(Request req)
    {
    bvector<Utf8String> args = ParseUrl(req, "/");
    Utf8String instanceid = GetInstanceid(args[2]);
    HttpBodyPtr body = req.GetRequestBody();
    Utf8String requestBody = body->AsString();
    if (requestBody.Contains("<?xml"))
        {
        Utf8String contentEmp("");
        auto content = HttpResponseContent::Create(HttpStringBody::Create(contentEmp));
        return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::Created);
        }
    auto ptr1 = body.get();
    HttpRangeBody* rangeBody = dynamic_cast<HttpRangeBody*>(ptr1);
    HttpBodyPtr bodyOfRange = rangeBody->GetBody();
    auto ptr2 = bodyOfRange.get();
    HttpFileBody* fileBody = dynamic_cast<HttpFileBody*>(ptr2);
    BeFileName filePath(fileBody->GetFilePath());

    BeFileName dbPath = GetDbPath();
    BentleyM0200::BeSQLite::Db m_db;
    if (DbResult::BE_SQLITE_OK == m_db.OpenBeSQLiteDb(dbPath, BentleyM0200::BeSQLite::Db::OpenParams(BentleyM0200::BeSQLite::Db::OpenMode::Readonly, DefaultTxn::Yes)))
        {
        BeFileName serverFilePath(serverPath);
        serverFilePath.AppendToPath(BeFileName(instanceid).GetWCharCP());

        if (BeFileNameStatus::Success == FakeServer::CreateiModelFromSeed(filePath.GetWCharCP(), serverFilePath.GetWCharCP()))
            {
            Utf8String contentEmp = "";
            auto content = HttpResponseContent::Create(HttpStringBody::Create(contentEmp));
            return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::Created);
            }
        }
    return Response();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Response RequestHandler::FileCreationConfirmation(Request req)
    {
    bvector<Utf8String> args = ParseUrl(req, "/");
    Utf8String iModelid = args[7];
    Json::Value settings = ParsedJson(req);
    bvector<Utf8String> input = 
        {
        settings[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::FileId].asString(),
        settings[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::FileName].asString(),
        settings[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::FileDescription].asString(),
        iModelid
        };

    BeFileName dbPath = GetDbPath();
    BentleyM0200::BeSQLite::Db m_db;
    if (DbResult::BE_SQLITE_OK == m_db.OpenBeSQLiteDb(dbPath, BentleyM0200::BeSQLite::Db::OpenParams(BentleyM0200::BeSQLite::Db::OpenMode::ReadWrite, DefaultTxn::Yes)))
        {
        Statement insertSt;
        insertSt.Prepare(m_db, "Update SeedFile SET IsUploaded = ? where Id = ? AND iModelid = ?");
        insertSt.BindBoolean(1, true);
        insertSt.BindText(2, input[0], Statement::MakeCopy::No);
        insertSt.BindText(3, iModelid, Statement::MakeCopy::No);
        if (BE_SQLITE_DONE != insertSt.Step())
            return Response();

        Json::Value instancesinfo(Json::objectValue);
        JsonValueR changedInstance = instancesinfo[ServerSchema::ChangedInstance] = Json::objectValue;
        changedInstance["change"] = "Modified";
        JsonValueR instance = changedInstance[ServerSchema::InstanceAfterChange] = Json::objectValue;
        instance[ServerSchema::InstanceId] = input[0].c_str();
        instance[ServerSchema::SchemaName] = "iModelScope";
        instance[ServerSchema::ClassName] = "SeedFile";
        JsonValueR properties = instance[ServerSchema::Properties] = Json::objectValue;
        properties[ServerSchema::Property::FileDescription] = input[2].c_str();
        properties[ServerSchema::Property::FileId] = input[0].c_str();
        properties[ServerSchema::Property::FileName] = input[1].c_str();
        properties[ServerSchema::Property::FileSize] = settings[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::FileSize].ToString().c_str();
        properties[ServerSchema::Property::MergedChangeSetId] = "";
        properties[ServerSchema::Property::IsUploaded] = true;
        properties[ServerSchema::Property::Index] = 0;
        Utf8String contentToWrite = Json::FastWriter().write(instancesinfo);

        insertSt.Finalize();
        auto content = HttpResponseContent::Create(HttpStringBody::Create(contentToWrite));
        return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::OK);
        }
    return Response();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Response RequestHandler::GetInitializationState(Request req)
    {
    //https://qa-imodelhubapi.bentley.com/v2.5/Repositories/iModel--0794d7ec-fc1d-4677-9831-dddbf2dbef24/iModelScope/SeedFile?$filter=FileId+eq+'0d45d871-bda7-40fa-b9d3-dfa9904ccaa7'
    bvector<Utf8String> args = ParseUrl(req, "/");
    Utf8String iModelid = GetInstanceid(args[4]);

    BeFileName dbPath = GetDbPath();
    BentleyM0200::BeSQLite::Db m_db;
    if (DbResult::BE_SQLITE_OK == m_db.OpenBeSQLiteDb(dbPath, BentleyM0200::BeSQLite::Db::OpenParams(BentleyM0200::BeSQLite::Db::OpenMode::Readonly, DefaultTxn::Yes)))
        {
        Statement st;
        st.Prepare(m_db, "Select Id, FileName, FileDescription, FileSize from SeedFile where iModelId = ?");
        st.BindText(1, iModelid, Statement::MakeCopy::No);
        st.Step();
        BeFileName name(st.GetValueText(1));
        BeFileName fileName(name.GetFileNameWithoutExtension());

        Json::Value instanceState(Json::objectValue);
        JsonValueR instances = instanceState[ServerSchema::Instances] = Json::arrayValue;
        JsonValueR instance = instances[0] = Json::objectValue;
        instance[ServerSchema::InstanceId] = st.GetValueText(0);
        instance[ServerSchema::SchemaName] = ServerSchema::Schema::iModel;
        instance[ServerSchema::ClassName] = ServerSchema::Class::File;
        JsonValueR properties = instance[ServerSchema::Properties] = Json::objectValue;
        properties[ServerSchema::Property::Index] = 0;
        properties["iModelName"] = fileName.GetNameUtf8().c_str();;
        properties[ServerSchema::Property::FileName] = st.GetValueText(1);
        properties[ServerSchema::Property::FileDescription] = st.GetValueText(2);
        properties[ServerSchema::Property::FileSize] = st.GetValueText(3);
        properties[ServerSchema::Property::FileId] = st.GetValueText(0);
        properties[ServerSchema::Property::MergedChangeSetId] = "";
        properties[ServerSchema::Property::UserUploaded] = "";
        properties[ServerSchema::Property::UserCreated] = "";
        properties[ServerSchema::Property::InitializationState] = 0;

        Utf8String contentToWrite = Json::FastWriter().write(instanceState);
        st.Finalize();
        auto content = HttpResponseContent::Create(HttpStringBody::Create(contentToWrite));
        return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::OK);
        }
    return Response();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Response RequestHandler::UploadNewSeedFile(Request req)
    {
    bvector<Utf8String> args = ParseUrl(req, "/");
    Utf8String instanceid = GetInstanceid(args[2]);

    HttpBodyPtr body = req.GetRequestBody();
    auto ptr1 = body.get();
    HttpRangeBody* rangeBody = dynamic_cast<HttpRangeBody*>(ptr1);
    HttpBodyPtr bodyOfRange = rangeBody->GetBody();
    auto ptr2 = bodyOfRange.get();
    HttpFileBody* fileBody = dynamic_cast<HttpFileBody*>(ptr2);
    BeFileName filePath(fileBody->GetFilePath());
    BeFileName dbPath = GetDbPath();

    BentleyM0200::BeSQLite::Db m_db;
    if (DbResult::BE_SQLITE_OK == m_db.OpenBeSQLiteDb(dbPath, BentleyM0200::BeSQLite::Db::OpenParams(BentleyM0200::BeSQLite::Db::OpenMode::ReadWrite, DefaultTxn::Yes)))
        {
        BeFileName serverFilePath(serverPath);
        serverFilePath.AppendToPath(BeFileName(instanceid).GetWCharCP());

        if (BeFileNameStatus::Success == FakeServer::CreateiModelFromSeed(filePath.GetWCharCP(), serverFilePath.GetWCharCP()))
            {
            Statement st;
            st.Prepare(m_db, "Select Name from Instances where iModelId = ?");
            st.BindText(1, instanceid, Statement::MakeCopy::No);

            st.Step();
            Utf8String fileToDelete = st.GetValueText(0);
            fileToDelete += ".bim";
            BeFileName fileDelete(serverFilePath);
            fileDelete.AppendToPath(BeFileName(fileToDelete));
            fileDelete.BeDeleteFile();
            st.Finalize();

            Statement stUpdate;
            stUpdate.Prepare(m_db, "UPDATE Instances SET Name = ? where iModelId = ?");
            WString fileName = BeFileName(BeFileName::GetFileNameAndExtension(filePath)).GetFileNameWithoutExtension();
            Utf8String name = BeFileName(fileName).GetNameUtf8();
            stUpdate.BindText(1, name, Statement::MakeCopy::No);
            stUpdate.BindText(2, instanceid, Statement::MakeCopy::No);
            stUpdate.Step();
            stUpdate.Finalize();
            Utf8String contentEmpty = "";
            auto content = HttpResponseContent::Create(HttpStringBody::Create(contentEmpty));
            return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::Created);
            }
        }
    return Response();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName RequestHandler::GetDbPath()
    {
    BeFileName dbName("ServerRepo.db");
    BeFileName dbPath(serverPath);
    return dbPath.AppendToPath(dbName);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Response RequestHandler::CreateBriefcaseInstance(Request req)
    {
    bvector<Utf8String> args = ParseUrl(req, "/");
    Utf8String instanceid = GetInstanceid(args[4]);
    BeFileName dbPath = GetDbPath();
    BentleyM0200::BeSQLite::Db m_db;
    if (DbResult::BE_SQLITE_OK == m_db.OpenBeSQLiteDb(dbPath, BentleyM0200::BeSQLite::Db::OpenParams(BentleyM0200::BeSQLite::Db::OpenMode::ReadWrite, DefaultTxn::Yes)))
        {
        Statement st;
        st.Prepare(m_db, "SELECT A.Briefcases, B.Id, B.FileName, B.FileDescription, B.FileSize FROM Instances As A Inner Join SeedFile As B ON A.id = B.iModelid where A.id = ? ");
        st.BindText(1, instanceid, Statement::MakeCopy::No);
        st.Step();
        size_t briefcaseCount = st.GetValueInt(0);
        Utf8CP fileId = st.GetValueText(1);
        Utf8CP fileName = st.GetValueText(2);
        Utf8CP fileDescription = st.GetValueText(3);
        size_t fileSize = st.GetValueInt(4);

        briefcaseCount += 1;
        char buff[50];
        sprintf(buff, "%d", (int)briefcaseCount);

        Json::Value instanceCreation(Json::objectValue);
        JsonValueR changedInstance = instanceCreation[ServerSchema::ChangedInstance] = Json::objectValue;
        changedInstance["change"] = "Created";
        JsonValueR instanceAfterChange = changedInstance[ServerSchema::InstanceAfterChange] = Json::objectValue;
        instanceAfterChange[ServerSchema::InstanceId] = buff;
        instanceAfterChange[ServerSchema::SchemaName] = ServerSchema::Schema::iModel;
        instanceAfterChange[ServerSchema::ClassName] = ServerSchema::Class::Briefcase;
        JsonValueR properties = instanceAfterChange[ServerSchema::Properties] = Json::objectValue;
        properties[ServerSchema::Property::FileName] = fileName;
        properties[ServerSchema::Property::FileDescription] = fileDescription;
        properties[ServerSchema::Property::FileSize] = Json::Value(fileSize);
        properties[ServerSchema::Property::FileId] = fileId;
        properties[ServerSchema::Property::BriefcaseId] = Json::Value(briefcaseCount);
        properties[ServerSchema::Property::MergedChangeSetId] = "";
        properties[ServerSchema::Property::UserOwned] = "";
        properties[ServerSchema::Property::IsReadOnly] = false;
        JsonValueR relationshipInstances = instanceAfterChange[ServerSchema::RelationshipInstances] = Json::arrayValue;
        JsonValueR relationsArray = relationshipInstances[0] = Json::objectValue;
        relationsArray[ServerSchema::InstanceId] = "";
        relationsArray[ServerSchema::SchemaName] = "iModelScope";
        relationsArray[ServerSchema::ClassName] = "FileAccesKey";
        relationsArray["direction"] = "forward";
        JsonValueR relatedInstance = relationsArray[ServerSchema::RelatedInstance] = Json::objectValue;;
        relatedInstance[ServerSchema::InstanceId] = "";
        relatedInstance[ServerSchema::SchemaName] = "iModelScope";
        relatedInstance[ServerSchema::ClassName] = "AccessKey";
        JsonValueR propertiesl2 = relatedInstance[ServerSchema::Properties] = Json::objectValue;
        Utf8String downloadloadUrl = "https://imodelhubqasa01.blob.core.windows.net/imodelhub-";
        downloadloadUrl += instanceid + "/BriefcaseTestsm-" + fileId;
        propertiesl2[ServerSchema::Property::UploadUrl] = NULL;
        propertiesl2[ServerSchema::Property::DownloadUrl] = downloadloadUrl.c_str();
        Utf8String contentToWrite = Json::FastWriter().write(instanceCreation);
        st.Finalize();

        Statement stInsert;
        stInsert.Prepare(m_db, "UPDATE Instances SET Briefcases = ? where id = ? ");
        stInsert.BindInt(1, briefcaseCount);
        stInsert.BindText(2, instanceid, Statement::MakeCopy::No);
        stInsert.Step();
        stInsert.Finalize();
        stInsert.Prepare(m_db, "INSERT INTO Briefcases(BriefcaseId, iModelId) VALUES (?,?)");
        stInsert.BindInt(1, briefcaseCount);
        stInsert.BindText(2, instanceid, Statement::MakeCopy::No);
        stInsert.Step();
        stInsert.Finalize();
        auto content = HttpResponseContent::Create(HttpStringBody::Create(contentToWrite));
        return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::Created);
        }

    return Response();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Response RequestHandler::DeleteBriefcaseInstance(Request req)
    {
    bvector<Utf8String> args = ParseUrl(req, "/");
    Utf8String instanceid = GetInstanceid(args[4]);
    BeFileName dbPath = GetDbPath();
    BentleyM0200::BeSQLite::Db m_db;
    int briefcaseId = 0;
    if (DbResult::BE_SQLITE_OK == m_db.OpenBeSQLiteDb(dbPath, BentleyM0200::BeSQLite::Db::OpenParams(BentleyM0200::BeSQLite::Db::OpenMode::ReadWrite, DefaultTxn::Yes)))
        {
        Statement st;
        st.Prepare(m_db, "DELETE from Briefcases where iModelId = ? AND BriefcaseId = ?");
        st.BindText(1, instanceid, Statement::MakeCopy::No);
        sscanf(args[7].c_str(), "%d", &briefcaseId);
        st.BindInt(2, briefcaseId);
        st.Step();
        st.Finalize();
        st.Prepare(m_db, "DELETE from Locks where iModelId = ? AND BriefcaseId = ?");
        st.BindText(1, instanceid, Statement::MakeCopy::No);
        sscanf(args[7].c_str(), "%d", &briefcaseId);
        st.BindInt(2, briefcaseId);
        st.Step();
        }

    Utf8String contentToWrite = "";
    auto content = HttpResponseContent::Create(HttpStringBody::Create(contentToWrite));
    return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::OK);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Response RequestHandler::GetBriefcaseInfo(Request req)
    {
    bvector<Utf8String> args = ParseUrl(req, "/");
    Utf8String iModelId = GetInstanceid(args[4]);
    BeFileName dbPath = GetDbPath();
    BentleyM0200::BeSQLite::Db m_db;

    int briefcaseId = 0;
    Utf8String sql = "SELECT B.BriefcaseId, S.Id, S.FileName, S.FileDescription, S.FileSize FROM Briefcases As B Inner Join SeedFile As S ON B.iModelid = S.iModelid where B.iModelid = ? ";
    if (args.size() == 8)
        {
        sscanf(args[7].c_str(), "%d", &briefcaseId);
        sql.append(" AND B.BriefcaseId = ");
        sql.append(args[7]);
        }

    if (DbResult::BE_SQLITE_OK == m_db.OpenBeSQLiteDb(dbPath, BentleyM0200::BeSQLite::Db::OpenParams(BentleyM0200::BeSQLite::Db::OpenMode::ReadWrite, DefaultTxn::Yes)))
        {
        Statement st;
        st.Prepare(m_db, sql.c_str());
        st.BindText(1, iModelId, Statement::MakeCopy::No);
        DbResult result = DbResult::BE_SQLITE_ROW;
        result = st.Step();

        Json::Value instancesinfo(Json::objectValue);
        JsonValueR instanceArray = instancesinfo[ServerSchema::Instances] = Json::arrayValue;
        for (int i = 0; result == DbResult::BE_SQLITE_ROW; i++)
            {
            JsonValueR instance = instanceArray[i] = Json::objectValue;
            instance[ServerSchema::InstanceId] = st.GetValueText(0);
            instance[ServerSchema::SchemaName] = ServerSchema::Schema::iModel;
            instance[ServerSchema::ClassName] = ServerSchema::Class::Briefcase;
            JsonValueR properties = instance[ServerSchema::Properties] = Json::objectValue;
            properties[ServerSchema::Property::FileName] = st.GetValueText(2);
            properties[ServerSchema::Property::FileDescription] = st.GetValueText(3);
            properties[ServerSchema::Property::FileSize] = st.GetValueText(4);
            properties[ServerSchema::Property::FileId] = st.GetValueText(1);
            properties[ServerSchema::Property::BriefcaseId] = st.GetValueInt(0);
            properties[ServerSchema::Property::UserOwned] = "";
            properties["AcquiredDate"] = "";
            properties[ServerSchema::Property::IsReadOnly] = false;
            result = st.Step();
            }
        st.Finalize();
        Utf8String contentToWrite = Json::FastWriter().write(instancesinfo);
        auto content = HttpResponseContent::Create(HttpStringBody::Create(contentToWrite));
        return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::OK);
        }
    return Response();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Response RequestHandler::DownloadiModel(Request req)
    {
    bvector<Utf8String> args = ParseUrl(req, "/");

    //download by checking instanceid from db
    Utf8String iModelId = GetInstanceid(args[2]);
    int flag = 0;
    bvector<Utf8String> tokens;
    if (req.GetUrl().Contains(".cs"))
        {
        flag = 1;
        BeStringUtilities::Split(args[3].c_str(), "?", nullptr, tokens);
        }
    BeFileName dbPath = GetDbPath();
    BentleyM0200::BeSQLite::Db m_db;
    CharCP filetoDownload;
    //instanceid = tokens[0];

    HttpBodyPtr body = req.GetResponseBody();
    Utf8String requestBody = body->AsString();
    auto ptr1 = body.get();
    HttpFileBody* fileBody = dynamic_cast<HttpFileBody*>(ptr1);
    BeFileName fileDownloadPath(fileBody->GetFilePath());

    if (DbResult::BE_SQLITE_OK == m_db.OpenBeSQLiteDb(dbPath, BentleyM0200::BeSQLite::Db::OpenParams(BentleyM0200::BeSQLite::Db::OpenMode::Readonly, DefaultTxn::Yes)))
        {
        Statement st;
        if (flag == 0)
            {
            st.Prepare(m_db, "Select FileName from SeedFile where iModelId = ?");
            st.BindText(1, iModelId, Statement::MakeCopy::No);
            st.Step();
            filetoDownload = st.GetValueText(0);
            }
        
        else filetoDownload = tokens[0].c_str();

        BeFileName serverFilePath(serverPath);
        serverFilePath.AppendToPath(BeFileName(iModelId));
        if (FakeServer::DownloadiModel(fileDownloadPath, serverFilePath.GetNameUtf8().c_str(), filetoDownload) == BeFileNameStatus::Success)
            {
            auto content = HttpResponseContent::Create(HttpStringBody::Create(Utf8String("")));
            st.Finalize();
            return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::OK);
            }
        }
    return Response();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Response RequestHandler::GetiModels(Request req)
    {
    bvector<Utf8String> args = ParseUrl(req, "/");
    BeFileName dbPath = GetDbPath();
    BentleyM0200::BeSQLite::Db m_db;

    Utf8String sql = "Select * from Instances";
    if (args[6].Contains("$filter"))
        {
        sql.append(" where ");
        sql.append(ParseUrlFilter(req.GetUrl()));
        }

    if (DbResult::BE_SQLITE_OK == m_db.OpenBeSQLiteDb(dbPath, BentleyM0200::BeSQLite::Db::OpenParams(BentleyM0200::BeSQLite::Db::OpenMode::Readonly, DefaultTxn::Yes)))
        {
        Statement st;
        st.Prepare(m_db, sql.c_str());
        DbResult result = DbResult::BE_SQLITE_ROW;
        result = st.Step();
        Json::Value instancesinfo(Json::objectValue);
        JsonValueR instanceArray = instancesinfo[ServerSchema::Instances] = Json::arrayValue;
        for (int i = 0; result == DbResult::BE_SQLITE_ROW; i++)
            {
            JsonValueR instance = instanceArray[i] = Json::objectValue;
            instance[ServerSchema::InstanceId] = st.GetValueText(0);
            instance[ServerSchema::SchemaName] = ServerSchema::Schema::Project;
            instance[ServerSchema::ClassName] = ServerSchema::Class::iModel;
            JsonValueR properties = instance[ServerSchema::Properties] = Json::objectValue;
            properties[ServerSchema::Property::Description] = st.GetValueText(2);
            properties[ServerSchema::Property::Name] = st.GetValueText(1);
            properties[ServerSchema::Property::UserCreated] = "";
            properties[ServerSchema::Property::CreatedDate] = "";
            properties[ServerSchema::Property::Initialized] = "";
            result = st.Step();
            }
        st.Finalize();
        Utf8String contentToWrite = Json::FastWriter().write(instancesinfo);
        auto content = HttpResponseContent::Create(HttpStringBody::Create(contentToWrite));
        return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::OK);
        }
    return Response();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RequestHandler::DeleteTables(Utf8String tableName)
    {
    BeFileName dbPath = GetDbPath();
    BentleyM0200::BeSQLite::Db m_db;
    if (DbResult::BE_SQLITE_OK != m_db.OpenBeSQLiteDb(dbPath, BentleyM0200::BeSQLite::Db::OpenParams(BentleyM0200::BeSQLite::Db::OpenMode::ReadWrite, DefaultTxn::Yes)))
        {
        Statement stDropTable;
        stDropTable.Prepare(m_db, "DROP table ?");
        stDropTable.BindText(1, tableName, Statement::MakeCopy::No);
        stDropTable.Step();
        stDropTable.Finalize();
        }
    m_db.CloseDb();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Response RequestHandler::DeleteiModels(Request req)
    {
    bvector<Utf8String> args = ParseUrl(req, "/");
    Utf8String iModelId = args[7];
    BeFileName dbPath = GetDbPath();
    BentleyM0200::BeSQLite::Db m_db;

    if (DbResult::BE_SQLITE_OK == m_db.OpenBeSQLiteDb(dbPath, BentleyM0200::BeSQLite::Db::OpenParams(BentleyM0200::BeSQLite::Db::OpenMode::ReadWrite, DefaultTxn::Yes)))
        {
        if (BeFileNameStatus::Success == FakeServer::DeleteiModel(serverPath, iModelId))
            {
            Statement stDelete;
            stDelete.Prepare(m_db, "Delete from Instances where Id = ?");
            stDelete.BindText(1, iModelId, Statement::MakeCopy::No);
            stDelete.Step();
            stDelete.Finalize();

            Json::Value instanceDeletion(Json::objectValue);
            JsonValueR changedInstance = instanceDeletion[ServerSchema::ChangedInstance] = Json::objectValue;
            changedInstance["change"] = "Deleted";
            JsonValueR instanceAfterChange = changedInstance[ServerSchema::InstanceAfterChange] = Json::objectValue;
            instanceAfterChange[ServerSchema::InstanceId] = iModelId;
            instanceAfterChange[ServerSchema::SchemaName] = ServerSchema::Schema::Project;
            instanceAfterChange[ServerSchema::ClassName] = ServerSchema::Class::iModel;

            Utf8String contentToWrite = Json::FastWriter().write(instanceDeletion);
            auto content = HttpResponseContent::Create(HttpStringBody::Create(contentToWrite));
            return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::OK);
            }
        }
    return Response();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Response RequestHandler::PushChangeSetMetadata(Request req)
    {
    bvector<Utf8String> args = ParseUrl(req, "/");
    Utf8String iModelid = GetInstanceid(args[4]);
    Json::Value settings = ParsedJson(req);
    bvector<Utf8String> input =
        {
        settings[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::Id].asString(),
        settings[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::Description].asString(),
        settings[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::ParentId].asString(),
        settings[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::SeedFileId].asString()
        };

    CheckDb();
    BeFileName dbPath = GetDbPath();
    BentleyM0200::BeSQLite::Db m_db;
    if (DbResult::BE_SQLITE_OK == m_db.OpenBeSQLiteDb(dbPath, BentleyM0200::BeSQLite::Db::OpenParams(BentleyM0200::BeSQLite::Db::OpenMode::ReadWrite, DefaultTxn::Yes)))
        {
        Statement st;
        st.Prepare(m_db, "Select IndexNo from ChangeSets where Id = ? AND iModelid = ?");
        st.BindText(1, input[2], Statement::MakeCopy::No);
        st.BindText(2, iModelid, Statement::MakeCopy::No);
        st.Step();
        int index;
        if (settings[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::BriefcaseId].empty())
            index = 1;
        else
            index = st.GetValueInt(0);
        index += 1;

        char buffer[50];
        sprintf(buffer, "%d", index);
        char buffFileSize[50];
        sprintf(buffFileSize, "%d", (int)settings[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::FileSize].asInt());
        Json::Value changesetMetadata(Json::objectValue);
        JsonValueR changedInstance = changesetMetadata[ServerSchema::ChangedInstance] = Json::objectValue;
        changedInstance["change"] = "Created";
        JsonValueR InstanceAfterChange = changedInstance[ServerSchema::InstanceAfterChange] = Json::objectValue;
        InstanceAfterChange[ServerSchema::InstanceId] = input[0];
        InstanceAfterChange[ServerSchema::SchemaName] = ServerSchema::Schema::iModel;
        InstanceAfterChange[ServerSchema::ClassName] = ServerSchema::Class::ChangeSet;
        JsonValueR properties = InstanceAfterChange[ServerSchema::Properties] = Json::objectValue;
        properties[ServerSchema::Property::BriefcaseId] = settings[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::BriefcaseId].asInt();
        properties[ServerSchema::Property::ContainingChanges] = settings[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::ContainingChanges].asInt();
        properties[ServerSchema::Property::Description] = input[1];
        properties[ServerSchema::Property::FileSize] = buffFileSize;
        properties[ServerSchema::Property::Id] = input[0];
        properties[ServerSchema::Property::IsUploaded] = false;
        properties[ServerSchema::Property::ParentId] = input[2];
        properties[ServerSchema::Property::SeedFileId] = input[3];
        properties[ServerSchema::Property::Index] = buffer;
        JsonValueR relationshipInstances = InstanceAfterChange[ServerSchema::RelationshipInstances] = Json::arrayValue;
        JsonValueR relationsArray = relationshipInstances[0] = Json::objectValue;
        relationsArray[ServerSchema::InstanceId] = "";
        relationsArray[ServerSchema::SchemaName] = "iModelScope";
        relationsArray[ServerSchema::ClassName] = "FileAccessKey";
        relationsArray["direction"] = "forward";
        JsonValueR relatedInstance = relationsArray[ServerSchema::RelatedInstance] = Json::objectValue;
        relatedInstance[ServerSchema::InstanceId] = "";
        relatedInstance[ServerSchema::SchemaName] = "iModelScope";
        relatedInstance[ServerSchema::ClassName] = "AccessKey";
        JsonValueR propertiesl1 = relatedInstance[ServerSchema::Properties] = Json::objectValue;
        Utf8String uploadUrl = "https://imodelhubqasa01.blob.core.windows.net/imodelhub-";
        uploadUrl += iModelid + "/" + input[0] + ".cs";
        propertiesl1[ServerSchema::Property::UploadUrl] = uploadUrl.c_str();
        propertiesl1[ServerSchema::Property::DownloadUrl] = NULL;
        st.Finalize();

        st.Prepare(m_db, "INSERT INTO ChangeSets(Id, Description, iModelId, FileSize, BriefcaseId, ParentId, SeedFileId, IndexNo, IsUploaded) VALUES (?,?,?,?,?,?,?,?,?)");
        st.BindText(1, input[0], Statement::MakeCopy::No);
        st.BindText(2, input[1], Statement::MakeCopy::No);
        st.BindText(3, iModelid, Statement::MakeCopy::No);
        st.BindInt(4, settings[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::FileSize].asInt());
        st.BindInt(5, settings[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::BriefcaseId].asInt());
        st.BindText(6, input[2], Statement::MakeCopy::No);
        st.BindText(7, input[3], Statement::MakeCopy::No);
        st.BindInt(8, index);
        st.BindBoolean(9, false);
        st.Step();
        st.Finalize();
        Utf8String contentToWrite = Json::FastWriter().write(changesetMetadata);
        return StubJsonHttpResponse(HttpStatus::Created, req.GetUrl().c_str(), contentToWrite);
        }
    return Response();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Response RequestHandler::GetChangeSetInfo(Request req)
    {
    bvector<Utf8String> args = ParseUrl(req, "/");
    Utf8String iModelId = GetInstanceid(args[4]);
    BeFileName dbPath = GetDbPath();
    BentleyM0200::BeSQLite::Db m_db;

    int fileAccessKey = 0;
    if (args[6].Contains("FileAccessKey"))
        fileAccessKey = 1;

    Utf8String sql("Select * from ChangeSets where iModelId = ?");
    if (args[6].Contains("$filter"))
        {
        sql.append(" AND ");
        sql.append(ParseUrlFilter(req.GetUrl(), "ChangeSets"));
        }
    if (DbResult::BE_SQLITE_OK == m_db.OpenBeSQLiteDb(dbPath, BentleyM0200::BeSQLite::Db::OpenParams(BentleyM0200::BeSQLite::Db::OpenMode::Readonly, DefaultTxn::Yes)))
        {
        Statement st;
        st.Prepare(m_db, sql.c_str());
        st.BindText(1, iModelId, Statement::MakeCopy::No);

        DbResult result = DbResult::BE_SQLITE_ROW;
        result = st.Step();
        Json::Value instancesinfo(Json::objectValue);
        JsonValueR instanceArray = instancesinfo[ServerSchema::Instances] = Json::arrayValue;

        for (int i = 0; result == DbResult::BE_SQLITE_ROW; i++)
            {
            Utf8String fileName(st.GetValueText(0));
            fileName.append(".cs");
            JsonValueR instance = instanceArray[i] = Json::objectValue;
            instance[ServerSchema::InstanceId] = st.GetValueText(0);
            instance[ServerSchema::SchemaName] = ServerSchema::Schema::iModel;
            instance[ServerSchema::ClassName] = ServerSchema::Class::ChangeSet;
            JsonValueR properties = instance[ServerSchema::Properties] = Json::objectValue;
            properties[ServerSchema::Property::FileName] = fileName.c_str();
            properties[ServerSchema::Property::Description] = st.GetValueText(1);
            properties[ServerSchema::Property::FileSize] = st.GetValueText(3);
            properties[ServerSchema::Property::Index] = st.GetValueText(7);
            properties[ServerSchema::Property::Id] = st.GetValueText(0);
            properties[ServerSchema::Property::ParentId] = st.GetValueText(5);
            properties[ServerSchema::Property::SeedFileId] = st.GetValueText(6);
            properties[ServerSchema::Property::BriefcaseId] = st.GetValueInt(4);
            properties[ServerSchema::Property::UserCreated] = "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx";
            properties[ServerSchema::Property::PushDate] = "";
            properties[ServerSchema::Property::ContainingChanges] = 0;
            properties[ServerSchema::Property::IsUploaded] = true;
            if (fileAccessKey)
                {
                JsonValueR relationshipInstancesArray = instance[ServerSchema::RelationshipInstances] = Json::arrayValue;
                JsonValueR relationshipInstances = relationshipInstancesArray[0] = Json::objectValue;
                relationshipInstances[ServerSchema::InstanceId] = "";
                relationshipInstances[ServerSchema::SchemaName] = ServerSchema::Schema::iModel;
                relationshipInstances[ServerSchema::ClassName] = "FileAccessKey";
                relationshipInstances["direction"] = "forward";
                JsonValueR relatedInstance = relationshipInstances[ServerSchema::RelatedInstance] = Json::objectValue;
                relatedInstance[ServerSchema::InstanceId] = "";
                relatedInstance[ServerSchema::SchemaName] = ServerSchema::Schema::iModel;
                relatedInstance[ServerSchema::ClassName] = "AccessKey";
                JsonValueR propertiesl2 = relatedInstance[ServerSchema::Properties] = Json::objectValue;
                Utf8String downloadUrl("https://imodelhubqasa01.blob.core.windows.net/imodelhub-");
                downloadUrl += iModelId + "/" + fileName;
                propertiesl2[ServerSchema::Property::DownloadUrl] = downloadUrl.c_str();
                }
            result = st.Step();
            }
        st.Finalize();
        Utf8String contentToWrite = Json::FastWriter().write(instancesinfo);
        auto content = HttpResponseContent::Create(HttpStringBody::Create(contentToWrite));
        return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::OK);
        }
    return Response();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool CheckConflict(BentleyM0200::BeSQLite::Db* m_db, Json::Value properties, Utf8String iModelid, Utf8String ObjectId)
    {
    Statement st;
    Utf8String sql = "";
    if (properties[ServerSchema::Property::LockLevel].asInt() == 1) sql = "Select * from Locks where iModelId = ? AND ObjectId = ? AND LockType = ? AND LockLevel = 2 AND BriefcaseId != ?";
    else if (properties[ServerSchema::Property::LockLevel].asInt() == 2) sql = "Select * from Locks where iModelId = ? AND ObjectId = ? AND LockType = ? AND LockLevel IN (1,2) AND BriefcaseId != ?";
    st.Prepare(*m_db, sql.c_str());
    st.BindText(1, iModelid, Statement::MakeCopy::No);
    st.BindText(2, ObjectId, Statement::MakeCopy::No);
    st.BindInt(3, properties[ServerSchema::Property::LockType].asInt());
    st.BindInt(4, properties[ServerSchema::Property::BriefcaseId].asInt());
    DbResult res = st.Step();
    st.Finalize();
    if (res == DbResult::BE_SQLITE_ROW) return true;
    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Response RequestHandler::PushAcquiredLocks(Request req)
    {
    bvector<Utf8String> args = ParseUrl(req, "/");
    Utf8String iModelid = GetInstanceid(args[4]);
    Json::Value settings = ParsedJson(req);
    BeFileName dbPath = GetDbPath();
    BentleyM0200::BeSQLite::Db m_db;
    CheckDb();
    if (DbResult::BE_SQLITE_OK == m_db.OpenBeSQLiteDb(dbPath, BentleyM0200::BeSQLite::Db::OpenParams(BentleyM0200::BeSQLite::Db::OpenMode::ReadWrite, DefaultTxn::Yes)))
        {
        Statement st;
        if (settings[ServerSchema::Instances][0][ServerSchema::ClassName] == "ChangeSet")
            {
            st.Prepare(m_db, "Update ChangeSets SET IsUploaded = ? where Id = ?");
            st.BindBoolean(1, true);
            st.BindText(2, settings[ServerSchema::Instances][0][ServerSchema::InstanceId].asString(), Statement::MakeCopy::No);
            if (BE_SQLITE_DONE != st.Step())
                return Response();
            st.Finalize();
            auto content = HttpResponseContent::Create(HttpStringBody::Create(Utf8String()));
            return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::OK);
            }

        int records;
        Json::Value properties;
        for (Json::ArrayIndex i = 0; i < settings[ServerSchema::Instances].size(); i++)
            {
            properties = settings[ServerSchema::Instances][i][ServerSchema::Properties];

            if (settings[ServerSchema::Instances][i][ServerSchema::ClassName] == "MultiLock")
                {
                for (Json::ArrayIndex j = 0; j < properties[ServerSchema::Property::ObjectIds].size(); j++)
                    {
                    if (CheckConflict(&m_db, properties, iModelid, properties[ServerSchema::Property::ObjectIds][j].asString()))
                        {
                        auto content = HttpResponseContent::Create(HttpStringBody::Create(Utf8String("{\"errorId\":\"iModelHub.LockOwnedByAnotherBriefcase\",\"errorMessage\":\"Lock(s) is owned by another briefcase.\",\"errorDescription\":null,\"ConflictingLocks\":null}")));
                        return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::Conflict);
                        }
                    size_t changeSetIndex = 0;
                    if (!Utf8String::IsNullOrEmpty(properties[ServerSchema::Property::ReleasedWithChangeSet].asString().c_str()))
                        {
                        st.Prepare(m_db, "Select IndexNo from ChangeSets where Id = ?");
                        st.BindText(1, properties[ServerSchema::Property::ReleasedWithChangeSet].asString(), Statement::MakeCopy::No);
                        st.Step();
                        changeSetIndex = st.GetValueInt(0);
                        st.Finalize();
                        }

                    st.Prepare(m_db, "Select Count(*) from Locks where iModelId = ? AND ObjectId = ? AND LockType = ? AND BriefcaseId IN (0,?)");
                    st.BindText(1, iModelid, Statement::MakeCopy::No);
                    st.BindText(2, properties[ServerSchema::Property::ObjectIds][j].asString(), Statement::MakeCopy::No);
                    st.BindInt(3, properties[ServerSchema::Property::LockType].asInt());
                    st.BindInt(4, properties[ServerSchema::Property::BriefcaseId].asInt());
                    st.Step();
                    records = st.GetValueInt(0);
                    st.Finalize();
                    char buff[50], buff2[50];

                    sprintf(buff, "%d", properties[ServerSchema::Property::LockType].asInt());
                    Utf8String id = buff;
                    id.append("-");
                    id.append(properties[ServerSchema::Property::ObjectIds][j].asString());
                    if (properties[ServerSchema::Property::LockLevel].asInt() == 0) sprintf(buff2, "%d", 0);
                    else sprintf(buff2, "%d", properties[ServerSchema::Property::BriefcaseId].asInt());
                    id.append("-");
                    id.append(buff2);
                    printf("%s\n", id.c_str());
                    if (records == 0)
                        {
                        st.Prepare(m_db, "INSERT INTO Locks(Id, ObjectId, iModelId, LockType, LockLevel, BriefcaseId, ReleasedWithChangeSet, ReleasedWithChangeSetIndex) VALUES (?,?,?,?,?,?,?,?)");
                        st.BindText(1, id, Statement::MakeCopy::No);
                        st.BindText(2, properties[ServerSchema::Property::ObjectIds][j].asString(), Statement::MakeCopy::No);
                        st.BindText(3, iModelid, Statement::MakeCopy::No);
                        st.BindInt(4, properties[ServerSchema::Property::LockType].asInt());
                        st.BindInt(5, properties[ServerSchema::Property::LockLevel].asInt());
                        st.BindInt(6, properties[ServerSchema::Property::BriefcaseId].asInt());
                        st.BindText(7, properties[ServerSchema::Property::ReleasedWithChangeSet].asString(), Statement::MakeCopy::No);
                        st.BindInt(8, changeSetIndex);
                        st.Step();
                        st.Finalize();
                        }
                    else
                        {
                        if (Utf8String::IsNullOrEmpty(properties[ServerSchema::Property::ReleasedWithChangeSet].asString().c_str()))
                            {
                            st.Prepare(m_db, "UPDATE Locks Set Id = ?, LockLevel = ?, BriefcaseId = ? where iModelId = ? AND ObjectId = ? AND LockType = ? AND BriefcaseId In(0,?)");
                            st.BindText(1, id, Statement::MakeCopy::No);
                            st.BindInt(2, properties[ServerSchema::Property::LockLevel].asInt());
                            if (properties[ServerSchema::Property::LockLevel].asInt() == 0) st.BindInt(3, 0);
                            else st.BindInt(3, properties[ServerSchema::Property::BriefcaseId].asInt());
                            st.BindText(4, iModelid, Statement::MakeCopy::No);
                            st.BindText(5, properties[ServerSchema::Property::ObjectIds][j].asString(), Statement::MakeCopy::No);
                            st.BindInt(6, properties[ServerSchema::Property::LockType].asInt());
                            st.BindInt(7, properties[ServerSchema::Property::BriefcaseId].asInt());
                            st.Step();
                            st.Finalize();
                            }
                        else
                            {
                            st.Prepare(m_db, "UPDATE Locks Set Id = ?, LockLevel = ?, BriefcaseId = ?, ReleasedWithChangeSet = ?, ReleasedWithChangeSetIndex = ? where iModelId = ? AND ObjectId = ? AND LockType = ? AND BriefcaseId In(0,?)");
                            st.BindText(1, id, Statement::MakeCopy::No);
                            st.BindInt(2, properties[ServerSchema::Property::LockLevel].asInt());
                            if (properties[ServerSchema::Property::LockLevel].asInt() == 0) st.BindInt(3, 0);
                            else st.BindInt(3, properties[ServerSchema::Property::BriefcaseId].asInt());
                            st.BindText(4, properties[ServerSchema::Property::ReleasedWithChangeSet].asString(), Statement::MakeCopy::No);
                            st.BindInt(5, changeSetIndex);
                            st.BindText(6, iModelid, Statement::MakeCopy::No);
                            st.BindText(7, properties[ServerSchema::Property::ObjectIds][j].asString(), Statement::MakeCopy::No);
                            st.BindInt(8, properties[ServerSchema::Property::LockType].asInt());
                            st.BindInt(9, properties[ServerSchema::Property::BriefcaseId].asInt());
                            st.Step();
                            st.Finalize();
                            }
                        }
                    }
                }
            else if (settings[ServerSchema::Instances][i][ServerSchema::ClassName] == "Lock" && settings[ServerSchema::Instances][i]["changeState"] == "deleted")
                {
                bvector<Utf8String> tokens;
                BeStringUtilities::Split(settings[ServerSchema::Instances][i][ServerSchema::InstanceId].asString().c_str(), "-", nullptr, tokens);
                st.Prepare(m_db, "Select * from Locks where iModelId = ? AND BriefcaseId = ?");
                st.BindText(1, iModelid, Statement::MakeCopy::No);
                int briefcaseId;
                sscanf(tokens[1].c_str(), "%d", &briefcaseId);
                st.BindInt(2, briefcaseId);
                DbResult result = st.Step();
                Statement updateSt;
                for (int i = 0; result == DbResult::BE_SQLITE_ROW; i++)
                    {
                    Utf8String id(st.GetValueText(2));
                    id.append("-");
                    id.append(st.GetValueText(0));
                    id.append("-");
                    id.append("0");
                    updateSt.Prepare(m_db, "UPDATE Locks Set Id = ?, LockLevel = 0, BriefcaseId = 0 where iModelId = ? AND BriefcaseId = ? AND LockType = ? AND ObjectId = ?");
                    updateSt.BindText(1, id, Statement::MakeCopy::No);
                    updateSt.BindText(2, iModelid, Statement::MakeCopy::No);
                    updateSt.BindInt(3, briefcaseId);
                    updateSt.BindInt(4, st.GetValueInt(2));
                    updateSt.BindText(5, st.GetValueText(0), Statement::MakeCopy::No);
                    updateSt.Step();
                    updateSt.Finalize();
                    result = st.Step();
                    }
                st.Finalize();
                }
            }

        auto content = HttpResponseContent::Create(HttpStringBody::Create(Utf8String()));
        return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::OK);
        }
    return Response();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP GetLockType(int t)
    {
    if (t == 0) return "Db";
    else if (t == 1) return "Model";
    else if (t == 2) return "Element";
    else if (t == 3) return "Schemas";
    else return "CodeSpecs";
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP GetLockLevel(int l)
    {
    if (l == 0) return "None";
    else if (l == 1) return "Shared";
    else return "Exclusive";
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Response RequestHandler::MultiLocksInfo(Request req)
    {
    bvector<Utf8String> args = ParseUrl(req, "/");
    Utf8String iModelid = GetInstanceid(args[4]);
    BeFileName dbPath = GetDbPath();
    BentleyM0200::BeSQLite::Db m_db;

    Utf8String sql = "Select Count(*), LockLevel, LockType, BriefcaseId from Locks where iModelId = ?";
    if (args[6].Contains("filter"))
        {
        sql.append(" AND ");
        sql.append(ParseUrlFilter(req.GetUrl()));
        }

    sql.append(" Group By LockLevel, LockType, BriefcaseId");
    if (DbResult::BE_SQLITE_OK == m_db.OpenBeSQLiteDb(dbPath, BentleyM0200::BeSQLite::Db::OpenParams(BentleyM0200::BeSQLite::Db::OpenMode::Readonly, DefaultTxn::Yes)))
        {
        Statement st;
        st.Prepare(m_db, sql.c_str());
        st.BindText(1, iModelid, Statement::MakeCopy::No);
        DbResult result = DbResult::BE_SQLITE_ROW;
        DbResult result2 = DbResult::BE_SQLITE_ROW;
        result = st.Step();
        Json::Value instancesinfo(Json::objectValue);
        JsonValueR instanceArray = instancesinfo[ServerSchema::Instances] = Json::arrayValue;
        Statement stObjectIds;
        for (int i = 0; result == DbResult::BE_SQLITE_ROW; i++)
            {
            Utf8String instanceId("MultiLock");
            instanceId.append("-");
            instanceId.append(GetLockType(st.GetValueInt(2)));
            instanceId.append("-");
            instanceId.append(GetLockLevel(st.GetValueInt(1)));
            JsonValueR instance = instanceArray[i] = Json::objectValue;
            instance[ServerSchema::InstanceId] = instanceId.c_str();
            instance[ServerSchema::SchemaName] = ServerSchema::Schema::iModel;
            instance[ServerSchema::ClassName] = ServerSchema::Class::MultiLock;
            JsonValueR properties = instance[ServerSchema::Properties] = Json::objectValue;

            stObjectIds.Prepare(m_db, "Select ObjectId from Locks where LockLevel = ? AND LockType = ? AND BriefcaseId = ? AND iModelId = ?");
            stObjectIds.BindInt(1, st.GetValueInt(1));
            stObjectIds.BindInt(2, st.GetValueInt(2));
            stObjectIds.BindInt(3, st.GetValueInt(3));
            stObjectIds.BindText(4, iModelid, Statement::MakeCopy::No);
            result2 = DbResult::BE_SQLITE_ROW;
            result2 = stObjectIds.Step();
            JsonValueR ObjectIdsArray = properties[ServerSchema::Property::ObjectIds] = Json::arrayValue;
            for (int j = 0; result2 == DbResult::BE_SQLITE_ROW; j++)
                {
                ObjectIdsArray[j] = stObjectIds.GetValueText(0);
                result2 = stObjectIds.Step();
                }
            stObjectIds.Finalize();
            properties[ServerSchema::Property::LockLevel] = st.GetValueInt(1);
            properties[ServerSchema::Property::LockType] = st.GetValueInt(2);
            properties[ServerSchema::Property::BriefcaseId] = st.GetValueInt(3);
            result = st.Step();
            }
        st.Finalize();
        Utf8String contentToWrite = Json::FastWriter().write(instancesinfo);
        auto content = HttpResponseContent::Create(HttpStringBody::Create(contentToWrite));
        return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::OK);
        }
    return Response();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Response RequestHandler::LocksInfo(Request req)
    {
    bvector<Utf8String> args = ParseUrl(req, "/");
    Utf8String iModelid = GetInstanceid(args[4]);
    BeFileName dbPath = GetDbPath();
    BentleyM0200::BeSQLite::Db m_db;

    Utf8String sql = "Select * from Locks where iModelId = ?";
    if (args[6].Contains("filter"))
        {
        sql.append(" AND ");
        sql.append(ParseUrlFilter(req.GetUrl(), "Locks"));
        }
    printf("%s\n", sql.c_str());
    if (DbResult::BE_SQLITE_OK == m_db.OpenBeSQLiteDb(dbPath, BentleyM0200::BeSQLite::Db::OpenParams(BentleyM0200::BeSQLite::Db::OpenMode::Readonly, DefaultTxn::Yes)))
        {
        Statement st;
        st.Prepare(m_db, sql.c_str());
        st.BindText(1, iModelid, Statement::MakeCopy::No);
        DbResult result = DbResult::BE_SQLITE_ROW;
        result = st.Step();
        Json::Value instancesinfo(Json::objectValue);
        JsonValueR instanceArray = instancesinfo[ServerSchema::Instances] = Json::arrayValue;

        for (int i = 0; result == DbResult::BE_SQLITE_ROW; i++)
            {
            JsonValueR instance = instanceArray[i] = Json::objectValue;
            instance[ServerSchema::InstanceId] = st.GetValueText(7);
            instance[ServerSchema::SchemaName] = ServerSchema::Schema::iModel;
            instance[ServerSchema::ClassName] = ServerSchema::Class::Lock;
            JsonValueR properties = instance[ServerSchema::Properties] = Json::objectValue;
            properties[ServerSchema::Property::ObjectId] = st.GetValueText(0);
            properties[ServerSchema::Property::LockType] = st.GetValueInt(2);
            properties[ServerSchema::Property::LockLevel] = st.GetValueInt(3);
            properties[ServerSchema::Property::BriefcaseId] = st.GetValueInt(4);
            properties["AcquiredDate"] = "";
            properties[ServerSchema::Property::ReleasedWithChangeSet] = st.GetValueText(5);
            properties[ServerSchema::Property::ReleasedWithChangeSetIndex] = "";
            properties[ServerSchema::Property::QueryOnly] = true;
            result = st.Step();
            }
        Utf8String contentToWrite = Json::FastWriter().write(instancesinfo);
        auto content = HttpResponseContent::Create(HttpStringBody::Create(contentToWrite));
        return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::OK);
        }
    return Response();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Response RequestHandler::CodesInfo(Request req)
    {
    Utf8String contentToWrite = "{\"instances\":[]}";
    auto content = HttpResponseContent::Create(HttpStringBody::Create(contentToWrite));
    return Http::Response(content, req.GetUrl().c_str(), ConnectionStatus::OK, HttpStatus::OK);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Response RequestHandler::PerformGetRequest(Request req)
    {
    Utf8String urlExpected = "https://qa-imodelhubapi.bentley.com/v2.0/Plugins";

    if (!req.GetUrl().CompareTo(urlExpected))
        return RequestHandler::PluginRequest(req);
    if (req.GetUrl().Contains("Repositories/Project") && req.GetUrl().Contains("ProjectScope/iModel"))
        return RequestHandler::GetiModels(req);
    if (req.GetUrl().Contains("https://imodelhubqasa01.blob.core.windows.net/imodelhub"))//detect imodelhub
        return DownloadiModel(req);
    if (req.GetUrl().Contains("Repositories/iModel") && req.GetUrl().Contains("iModelScope/ChangeSet"))
        return RequestHandler::GetChangeSetInfo(req);
    if (req.GetUrl().Contains("Repositories/iModel") && req.GetUrl().Contains("iModelScope/SeedFile"))
        return RequestHandler::GetInitializationState(req);
    if (req.GetUrl().Contains("Repositories/iModel") && req.GetUrl().Contains("iModelScope/Briefcase"))
        return RequestHandler::GetBriefcaseInfo(req);
    if (req.GetUrl().Contains("Repositories/iModel") && req.GetUrl().Contains("iModelScope/Lock"))
        return RequestHandler::LocksInfo(req);
    if (req.GetUrl().Contains("Repositories/iModel") && req.GetUrl().Contains("iModelScope/MultiLock"))
        return RequestHandler::MultiLocksInfo(req);
    if (req.GetUrl().Contains("Repositories/iModel") && (req.GetUrl().Contains("iModelScope/Code") || req.GetUrl().Contains("iModelScope/MultiCode")))
        return RequestHandler::CodesInfo(req);
    return Response();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Response RequestHandler::PerformOtherRequest(Request req)
    {
    Utf8String urlExpected = "https://buddi.bentley.com/discovery.asmx";
    Utf8String urlExpected2 = "https://qa-ims.bentley.com/rest/ActiveSTSService/json/IssueEx";
    Utf8String urlExpected3 = "https://qa-ims.bentley.com/rest/DelegationSTSService/json/IssueEx";

    if (!req.GetUrl().CompareTo(urlExpected))
        return RequestHandler::BuddiRequest(req);
    if (!req.GetUrl().CompareTo(urlExpected2))
        return RequestHandler::ImsTokenRequest(req);
    if (!req.GetUrl().CompareTo(urlExpected3))
        return RequestHandler::ImsTokenRequest(req);

    if (req.GetUrl().Contains("Repositories/Project") && req.GetUrl().Contains("ProjectScope/iModel"))
        {
        if (req.GetMethod().Equals("DELETE"))
            return RequestHandler::DeleteiModels(req);
        return RequestHandler::CreateiModelInstance(req);
        }
    if (req.GetUrl().Contains("Repositories/iModel") && req.GetUrl().Contains("iModelScope/SeedFile"))
        {
        if (ParseUrl(req, "/").size() == 8)
            return RequestHandler::FileCreationConfirmation(req);
        return RequestHandler::CreateSeedFileInstance(req);
        }
    if (req.GetUrl().Contains("https://imodelhubqasa01.blob.core.windows.net"))
        return RequestHandler::UploadSeedFile(req);
    if (req.GetUrl().Contains("Repositories/iModel") && req.GetUrl().Contains("iModelScope/Briefcase"))
        {
        if (req.GetMethod().Equals("DELETE"))
            return RequestHandler::DeleteBriefcaseInstance(req);
        return RequestHandler::CreateBriefcaseInstance(req);
        }
    if (req.GetUrl().Contains("Repositories/iModel") && req.GetUrl().Contains("iModelScope/ChangeSet"))
        return RequestHandler::PushChangeSetMetadata(req);
    if (req.GetUrl().Contains("Repositories/iModel") && req.GetUrl().Contains("$changeset"))
        return RequestHandler::PushAcquiredLocks(req);
    if (req.GetMethod().Equals("PUT"))
        return RequestHandler::UploadNewSeedFile(req);
    return Response();
    }

