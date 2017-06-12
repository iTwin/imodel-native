/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/WSGServices.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <curl/curl.h>
#include <iostream>

#include <Bentley/Bentley.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatform/WSGServices.h>
#include <CCApi/CCPublic.h>
#include <RealityPlatform/RealityPlatformAPI.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Francis Boily                   09/2015
//* writes the body of a curl reponse to a Utf8String
//+---------------+---------------+---------------+---------------+---------------+------*/
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
    ((Utf8String*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   03/2017
//* writes the body of a curl reponse to a file
//+---------------+---------------+---------------+---------------+---------------+------*/
static size_t WriteFileCallback(void *contents, size_t size, size_t nmemb, BeFile *stream)
    {
    uint32_t bytesWritten = 0;

    if (stream->Write(&bytesWritten, contents, (uint32_t)(size*nmemb)) != BeFileStatus::Success)
        bytesWritten = 0;

    return bytesWritten;
    }

RequestStatus RawServerResponse::ValidateResponse()
    {
    if ((curlCode != CURLE_OK) || (responseCode > 399))
        status = RequestStatus::BADREQ;
    else
        status = RequestStatus::OK;

    return status;
    }

RequestStatus RawServerResponse::ValidateJSONResponse(Json::Value& instances, Utf8StringCR keyword)
    {
    if ((curlCode != CURLE_OK) || (responseCode > 399) || !Json::Reader::Parse(body, instances) || instances.isMember("errorMessage") || !instances.isMember(keyword))
        status = RequestStatus::BADREQ;
    else
        status = RequestStatus::OK;

    return status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Francis Boily         	    02/2017
//-------------------------------------------------------------------------------------
CurlConstructor::CurlConstructor()
    {
    RefreshToken();

    WChar exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    WString exeDir = exePath;
    size_t pos = exeDir.find_last_of(L"/\\");
    exeDir = exeDir.substr(0, pos + 1);
    BeFileName caBundlePath = BeFileName(exeDir);
    m_certificatePath = caBundlePath.AppendToPath(L"Assets").AppendToPath(L"http").AppendToPath(L"ContextServices.pem");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
WSGRequest::WSGRequest() : CurlConstructor()
    {
    s_instance = this;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
WSGRequest* WSGRequest::s_instance = nullptr;

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
WSGRequest& WSGRequest::GetInstance()
    {
    if(nullptr == s_instance)
        s_instance = new WSGRequest();
    return *s_instance;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
Utf8String CurlConstructor::GetToken()
    {
    if((std::time(nullptr) - m_tokenRefreshTimer) > (59 * 60)) //refresh required every 60 minutes
        RefreshToken();
    return m_token;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
void CurlConstructor::RefreshToken()
    {
    m_tokenRefreshTimer = std::time(nullptr);

    CCAPIHANDLE api = CCApi_InitializeApi(COM_THREADING_Multi);
    CallStatus status = APIERR_SUCCESS;

    bool installed;
    status = CCApi_IsInstalled(api, &installed);
    if (!installed)
        {
        std::cout << "Connection client does not seem to be installed" << std::endl;
        return;
        }
    bool running = false;
    status = CCApi_IsRunning(api, &running);
    if (status != APIERR_SUCCESS || !running)
        {
        std::cout << "Connection client does not seem to be running" << std::endl;
        return;
        }
    bool loggedIn = false;
    status = CCApi_IsLoggedIn(api, &loggedIn);
    if (status != APIERR_SUCCESS || !loggedIn)
        {
        std::cout << "Connection client does not seem to be logged in" << std::endl;
        return;
        }
    bool acceptedEula = false;
    status = CCApi_HasUserAcceptedEULA(api, &acceptedEula);
    if (status != APIERR_SUCCESS || !acceptedEula)
        {
        std::cout << "Connection client user does not seem to have accepted EULA" << std::endl;
        return;
        }
    bool sessionActive = false;
    status = CCApi_IsUserSessionActive(api, &sessionActive);
    if (status != APIERR_SUCCESS || !sessionActive)
        {
        std::cout << "Connection client does not seem to have an active session" << std::endl;
        return;
        }

    LPCWSTR relyingParty = L"https://connect-wsg20.bentley.com";//;L"https:://qa-ims.bentley.com"
    UINT32 maxTokenLength = 16384;
    LPWSTR lpwstrToken = new WCHAR[maxTokenLength];

    status = CCApi_GetSerializedDelegateSecurityToken(api, relyingParty, lpwstrToken, maxTokenLength);
    if (status != APIERR_SUCCESS)
        return;

    char* charToken = new char[maxTokenLength];
    wcstombs(charToken, lpwstrToken, maxTokenLength);

    m_token = "Authorization: Token ";
    m_token.append(charToken);
    
    delete lpwstrToken;
    delete charToken;
    CCApi_FreeApi(api);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
void WSGRequest::PerformRequest(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry) const
    {
    response.clear();
    response.curlCode = ServerType::WSG;
    return _PerformRequest(wsgRequest, response, verifyPeer, file, retry);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
void WSGRequest::PerformAzureRequest(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry) const
    {
    response.clear();
    response.curlCode = ServerType::Azure;
    return _PerformRequest(wsgRequest, response, verifyPeer, file, retry);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
CURL* CurlConstructor::PrepareCurl(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file) const
    {
    CURL* curl = curl_easy_init();
    if (nullptr == curl)
        {
        response.curlCode = CURLcode::CURLE_FAILED_INIT;
        return curl;
        }

    //Adjusting headers for the POST method
    struct curl_slist *headers = NULL;
    if (wsgRequest.GetRequestType() == WSGURL::HttpRequestType::POST_Request)
        {
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, wsgRequest.GetRequestPayload().c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, wsgRequest.GetRequestPayload().length());
        }
    else if (wsgRequest.GetRequestType() == WSGURL::HttpRequestType::DELETE_Request)
        {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, wsgRequest.GetRequestPayload().c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, wsgRequest.GetRequestPayload().length());
        }

    if (!m_proxyUrl.empty())
        {
        curl_easy_setopt(curl, CURLOPT_PROXY, m_proxyUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
        if (!m_proxyCreds.empty())
            {
            curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, m_proxyCreds.c_str());
            }
        }

    bvector<Utf8String> wsgHeaders = wsgRequest.GetRequestHeader();
    for (Utf8String header : wsgHeaders)
        headers = curl_slist_append(headers, header.c_str());
    if(response.curlCode != ServerType::Azure)
        headers = curl_slist_append(headers, m_token.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, wsgRequest.GetHttpRequestString());

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, (verifyPeer ? 1: 0));

    curl_easy_setopt(curl, CURLOPT_CAINFO, m_certificatePath.GetNameUtf8());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_HEADEROPT, CURLHEADER_SEPARATE);

    return curl;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
CURL* WSGRequest::PrepareRequest(const WSGURL& wsgRequest, RawServerResponse& responseObject, bool verifyPeer, BeFile* file) const
    {
    CURL* curl = PrepareCurl(wsgRequest, responseObject, verifyPeer, file);

    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &(responseObject.header));

    if (file != nullptr && wsgRequest.GetRequestType() != WSGURL::HttpRequestType::PUT_Request)
        {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
        }
    else
        {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &(responseObject.body));
        }

    return curl;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
void WSGRequest::_PerformRequest(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry) const
    {
    auto curl = PrepareRequest(wsgRequest, response, verifyPeer, file);

    if(response.curlCode == CURLcode::CURLE_FAILED_INIT)
        return;

    response.curlCode = (int)curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &(response.responseCode));
    curl_easy_cleanup(curl);

    if (response.body.Contains("Token is not valid") && retry)
        {
        WSGRequest::GetInstance().RefreshToken();
        response = RawServerResponse();
        return _PerformRequest(wsgRequest, response, verifyPeer, file, false);
        }

    if (response.curlCode != CURLE_OK)
        response.status = BADREQ; // Default error ... may be modified by caller based on curlCode
    else
        response.status = OK;

    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
WSGURL::WSGURL(Utf8String url, bool validString) : 
    m_validRequestString(validString), m_requestType(HttpRequestType::GET_Request), m_requestHeader(bvector<Utf8String>())
    {
    if (validString)
        m_httpRequestString = url;
    else
        m_serverName = url;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
WSGURL::WSGURL(Utf8String server, Utf8String version, Utf8String repoId, Utf8String schema, Utf8String className, Utf8String id)
    :m_serverName(server), m_version(version), m_repoId(repoId), m_schema(schema), m_className(className), m_id(id),
    m_validRequestString(false), m_requestType(HttpRequestType::GET_Request)
    {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
Utf8StringCR WSGURL::GetServerName() const { return m_serverName; }
void WSGURL::SetServerName(Utf8String serverName) { m_serverName = serverName; }

Utf8StringCR WSGURL::GetVersion() const { return m_version; }
void WSGURL::SetVersion(Utf8String version) { m_version = version; }

Utf8StringCR WSGURL::GetSchema() const { return m_schema; }
void WSGURL::SetSchema(Utf8String schema) { m_schema = schema; }

Utf8StringCR WSGURL::GetECClassName() const { return m_className; }
void WSGURL::SetECClassName(Utf8String className) { m_className = className; }

Utf8StringCR WSGURL::GetId() const { return m_id; }
void WSGURL::SetId(Utf8String id) { m_id = id; }

WSGURL::HttpRequestType WSGURL::GetRequestType() const { return m_requestType; }

Utf8StringCR WSGURL::GetRepoId() const { return m_repoId; }
void WSGURL::SetRepoId(Utf8String repoId) { m_repoId = repoId; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
void WSGURL::_PrepareHttpRequestStringAndPayload() const
    {
    m_validRequestString = false;
    //https://localhost/ws/v2.1/
    m_httpRequestString = "";
    if(!m_serverName.StartsWith("https://"))
        m_httpRequestString = "https://";
    m_httpRequestString.append(m_serverName);

    if (!m_httpRequestString.EndsWith("/") && !m_httpRequestString.EndsWith("]"))
        m_httpRequestString.append("/");

    m_requestHeader.clear();

    EncodeId();

    m_validRequestString = true;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
void WSGURL::EncodeId() const
    {
    m_encodedId = BeStringUtilities::UriEncode(m_id.c_str());
    }

WSGURL& WSGURL::operator=(WSGURL const & object)
    {
    if(&object != this)
        {
        m_serverName = object.m_serverName;
        m_version = object.m_version;
        m_schema = object.m_schema;
        m_className = object.m_className;
        m_repoId = object.m_repoId;
        m_id = object.m_id;

        m_validRequestString = object.m_validRequestString;
        m_httpRequestString = object.m_httpRequestString;
        m_requestPayload = object.m_requestPayload;
        m_requestHeader = object.m_requestHeader;
        m_requestType = object.m_requestType;
        }
    return *this;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
NavNode::NavNode() : m_navString("ROOT")
    {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
NavNode::NavNode(Json::Value jsonObject, Utf8String rootNode, Utf8String rootId)
    {
    m_rootNode = rootNode;
    m_rootId = rootId;
    if(jsonObject.isMember("instanceId"))
        m_navString = jsonObject["instanceId"].asCString();
    if(jsonObject.isMember("properties"))
        {
        if (jsonObject["properties"].isMember("Key_TypeSystem") && !jsonObject["properties"]["Key_TypeSystem"].isNull())
            m_typeSystem = jsonObject["properties"]["Key_TypeSystem"].asCString();
        if (jsonObject["properties"].isMember("Key_SchemaName") && !jsonObject["properties"]["Key_SchemaName"].isNull())
            m_schemaName = jsonObject["properties"]["Key_SchemaName"].asCString();
        if (jsonObject["properties"].isMember("Key_ClassName") && !jsonObject["properties"]["Key_ClassName"].isNull())
            m_className = jsonObject["properties"]["Key_ClassName"].asCString();
        if (jsonObject["properties"].isMember("Key_InstanceId") && !jsonObject["properties"]["Key_InstanceId"].isNull())
            m_instanceId = jsonObject["properties"]["Key_InstanceId"].asCString();
        if (jsonObject["properties"].isMember("Label") && !jsonObject["properties"]["Label"].isNull())
            m_label = jsonObject["properties"]["Label"].asCString();
        }
    if(rootNode.empty()) //navRoot
        {
        m_rootNode = m_navString;
        m_rootId = m_instanceId;
        }
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                03/2017
//-------------------------------------------------------------------------------------
NavNode::NavNode(Utf8StringCR schema, Utf8String id, Utf8StringCR typeSystem, Utf8StringCR className)
    : m_typeSystem(typeSystem), m_schemaName(schema), m_rootId(id), m_instanceId(id), m_className(className)
    {
    id.ReplaceAll("-","--");
    m_rootNode = m_navString = Utf8PrintfString("%s--%s-RealityData-%s", m_typeSystem, m_schemaName, id);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
Utf8String NavNode::GetNavString()  { return m_navString; }
Utf8String NavNode::GetTypeSystem() { return m_typeSystem; }
Utf8String NavNode::GetSchemaName() { return m_schemaName; }
Utf8String NavNode::GetECClassName()  { return m_className; }
Utf8String NavNode::GetInstanceId() { return m_instanceId; }
Utf8String NavNode::GetLabel()      { return m_label; }
Utf8String NavNode::GetRootNode()   { return m_rootNode; }
Utf8String NavNode::GetRootId()     { return m_rootId; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
NodeNavigator* NodeNavigator::s_nnInstance = nullptr;
//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
NodeNavigator& NodeNavigator::GetInstance()
    {
    if (nullptr == s_nnInstance)
        s_nnInstance = new NodeNavigator();
    return *s_nnInstance;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
NodeNavigator::NodeNavigator()
    {
    s_nnInstance = this;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
bvector<NavNode> NodeNavigator::GetRootNodes(Utf8String serverName, Utf8String repoId, RawServerResponse& responseObject)
    {
    WSGServer server = WSGServer(serverName, false);
    return GetRootNodes(server, repoId, responseObject);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
bvector<NavNode> NodeNavigator::GetRootNodes(WSGServer server, Utf8String repoId, RawServerResponse& responseObject)
    {
    bvector<NavNode> returnVector;
    RawServerResponse versionResponse = RawServerResponse();
    WSGNavRootRequest* navRoot = new WSGNavRootRequest(server.GetServerName(), server.GetVersion(versionResponse), repoId);
    if(versionResponse.responseCode > 399)
        {
        responseObject = versionResponse;
        return returnVector;
        }

    WSGRequest::GetInstance().PerformRequest(*navRoot, responseObject, 0);

    delete navRoot;
    Json::Value instances(Json::objectValue);
    if(responseObject.ValidateJSONResponse(instances, "instances") != RequestStatus::OK)
        return returnVector; 

    for (auto instance : instances["instances"])
        returnVector.push_back(NavNode(instance));

    return returnVector;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
bvector<NavNode> NodeNavigator::GetChildNodes(WSGServer server, Utf8String repoId, NavNode& parentNode, RawServerResponse& responseObject)
    {
    return GetChildNodes(server, repoId, parentNode.GetNavString(), responseObject);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
bvector<NavNode> NodeNavigator::GetChildNodes(WSGServer server, Utf8String repoId, Utf8String nodePath, RawServerResponse& responseObject)
    {
    nodePath.ReplaceAll("/", "~2F");

    bvector<Utf8String> lines;
    BeStringUtilities::Split(nodePath.c_str(), "~", lines);
    Utf8String rootNode = lines[0];

    Utf8String guidString = rootNode;
    guidString.ReplaceAll("--","-");

    Utf8String rootId = guidString.substr(guidString.length() - 36, rootNode.length()); // 36 = size of GUID

    bvector<NavNode> returnVector;
    RawServerResponse versionResponse = RawServerResponse();
    WSGNavNodeRequest* navNode = new WSGNavNodeRequest(server.GetServerName(), server.GetVersion(versionResponse), repoId, nodePath);
    if (versionResponse.responseCode > 399)
        {
        responseObject = versionResponse;
        return returnVector;
        }

    WSGRequest::GetInstance().PerformRequest(*navNode, responseObject, 0);

    delete navNode;
    Json::Value instances(Json::objectValue);
    if (responseObject.ValidateJSONResponse(instances, "instances") != RequestStatus::OK)
        return returnVector;

    for (auto instance : instances["instances"])
        returnVector.push_back(NavNode(instance, rootNode, rootId));

    return returnVector;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
WSGNavRootRequest::WSGNavRootRequest(Utf8String server, Utf8String version, Utf8String repoId)
    {
    m_serverName = server;
    m_version = version;
    m_repoId = repoId;
    m_validRequestString = false;
    m_requestType = HttpRequestType::GET_Request;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
void WSGNavRootRequest::_PrepareHttpRequestStringAndPayload() const
    {
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("v");
    m_httpRequestString.append(m_version);
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(m_repoId);
    m_httpRequestString.append("/Navigation/NavNode/");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
WSGNavNodeRequest::WSGNavNodeRequest(Utf8String server, Utf8String version, Utf8String repoId, Utf8String nodeId)
    {
    m_serverName = server;
    m_version = version;
    m_repoId = repoId;
    m_id = nodeId;
    m_validRequestString = false;
    m_requestType = HttpRequestType::GET_Request;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
void WSGNavNodeRequest::_PrepareHttpRequestStringAndPayload() const
    {
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("v");
    m_httpRequestString.append(m_version);
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(m_repoId);
    m_httpRequestString.append("/Navigation/NavNode/");
    m_httpRequestString.append(m_encodedId);
    m_httpRequestString.append("/NavNode");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
WSGObjectRequest::WSGObjectRequest(Utf8String server, Utf8String version, Utf8String repoName, Utf8String schema, Utf8String className, Utf8String objectId)
    {
    m_serverName = server;
    m_version = version;
    m_repoId = repoName;
    m_schema = schema;
    m_className = className;
    m_id = objectId;
    m_validRequestString = false;
    m_requestType = HttpRequestType::GET_Request;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
void WSGObjectRequest::_PrepareHttpRequestStringAndPayload() const
    {
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("v");
    m_httpRequestString.append(m_version);
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(m_repoId);
    m_httpRequestString.append("/");
    m_httpRequestString.append(m_schema);
    m_httpRequestString.append("/");
    m_httpRequestString.append(m_className);
    m_httpRequestString.append("/");
    m_httpRequestString.append(m_encodedId);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
WSGObjectContentRequest::WSGObjectContentRequest(Utf8String server, Utf8String version, Utf8String repoName, Utf8String schema, Utf8String className, Utf8String objectId)
    {
    m_serverName = server;
    m_version = version;
    m_repoId = repoName;
    m_schema = schema;
    m_className = className;
    m_id = objectId;
    m_validRequestString = false;
    m_requestType = HttpRequestType::GET_Request;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
void WSGObjectContentRequest::_PrepareHttpRequestStringAndPayload() const
    {
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("v");
    m_httpRequestString.append(m_version);
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(m_repoId);
    m_httpRequestString.append("/");
    m_httpRequestString.append(m_schema);
    m_httpRequestString.append("/");
    m_httpRequestString.append(m_className);
    m_httpRequestString.append("/");
    m_httpRequestString.append(m_encodedId);
    m_httpRequestString.append("/$file");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
WSGObjectListPagedRequest::WSGObjectListPagedRequest(Utf8String server, Utf8String version, Utf8String repoId, Utf8String schema, Utf8String className)
    {
    m_startIndex = 0;
    m_pageSize = 25;
    m_serverName = server;
    m_version = version;
    m_repoId = repoId;
    m_schema = schema;
    m_className = className;
    m_validRequestString = false;
    m_requestType = HttpRequestType::GET_Request;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
void WSGObjectListPagedRequest::_PrepareHttpRequestStringAndPayload() const
    {
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("v");
    m_httpRequestString.append(m_version);
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(m_repoId);
    m_httpRequestString.append("/");
    m_httpRequestString.append(m_schema);
    m_httpRequestString.append("/");
    m_httpRequestString.append(m_className);
    m_httpRequestString.append("?$skip=");
    m_httpRequestString += Utf8PrintfString("%u", m_startIndex);
    m_httpRequestString.append("&$top=");
    m_httpRequestString += Utf8PrintfString("%u", m_pageSize);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
bvector<Utf8String> WSGServer::GetPlugins(RawServerResponse& responseObject) const
    {
    bvector<Utf8String> returnVec;
    Utf8String serverName = m_serverName;
    if (!serverName.EndsWith("/"))
        serverName.append("/");
    serverName.append("v");
    RawServerResponse versionResponse = RawServerResponse();
    serverName.append(GetVersion(versionResponse));
    if (versionResponse.responseCode > 399)
        {
        responseObject = versionResponse;
        return returnVec;
        }

    serverName.append("/Plugins");
    WSGURL wsgurl = WSGURL(serverName, false);

    WSGRequest::GetInstance().PerformRequest(wsgurl, responseObject, m_verifyPeer);

    Json::Value instances(Json::objectValue);
    if (responseObject.ValidateJSONResponse(instances, "instances") != RequestStatus::OK)
        return returnVec;

    for (auto instance : instances["instances"])
        {
        if (instance.isMember("instanceId"))
            returnVec.push_back(instance["instanceId"].asCString());
        }

    return returnVec;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
Utf8String WSGServer::GetVersion(RawServerResponse& responseObject) const
    {
    if(m_version.length() > 0)
        return m_version;

    Utf8String serverName = m_serverName;
    if (!serverName.EndsWith("/"))
        serverName.append("/");
    serverName.append("v2.5/Plugins");
    WSGURL wsgurl = WSGURL(serverName, false);

    WSGRequest::GetInstance().PerformRequest(wsgurl, responseObject, m_verifyPeer);

    if (responseObject.ValidateResponse() != RequestStatus::OK)
        return "";

    const char* charstring = responseObject.header.c_str();
    Utf8String keyword = "Bentley-WebAPI/";
    const char* substringPosition = strstr(charstring, keyword.c_str());
    substringPosition+= keyword.length();
    Utf8String versionString = Utf8String(substringPosition);

    bvector<Utf8String> lines;
    BeStringUtilities::Split(versionString.c_str(), "\n", lines);
    m_version = lines[0];
    m_version.Trim();

    return m_version;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
bvector<Utf8String> WSGServer::GetRepositories(RawServerResponse& responseObject) const
    {
    bvector<Utf8String> returnVec;
    Utf8String serverName = m_serverName;
    if (!serverName.EndsWith("/"))
        serverName.append("/");
    serverName.append("v");
    RawServerResponse versionResponse = RawServerResponse();
    serverName.append(GetVersion(versionResponse));
    if (versionResponse.responseCode > 399)
        {
        responseObject = versionResponse;
        return returnVec;
        }

    serverName.append("/Repositories");
    WSGURL wsgurl = WSGURL(serverName, false);

    WSGRequest::GetInstance().PerformRequest(wsgurl, responseObject, m_verifyPeer);
    
    Json::Value instances(Json::objectValue);
    if (responseObject.ValidateJSONResponse(instances, "instances") != RequestStatus::OK)
        return returnVec;
    
    for (auto instance : instances["instances"])
        {
        if (instance.isMember("instanceId"))
            returnVec.push_back(instance["instanceId"].asCString());
        }

    return returnVec;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
bvector<Utf8String> WSGServer::GetSchemaNames(Utf8String repoName, RawServerResponse& responseObject) const
    {
    bvector<Utf8String> returnVec;
    Utf8String serverName = m_serverName;
    if (!serverName.EndsWith("/"))
        serverName.append("/");
    serverName.append("v");
    RawServerResponse versionResponse = RawServerResponse();
    serverName.append(GetVersion(versionResponse));
    if (versionResponse.responseCode > 399)
        {
        responseObject = versionResponse;
        return returnVec;
        }

    serverName.append("/Repositories/");
    serverName.append(repoName);
    serverName.append("/MetaSchema/ECSchemaDef");

    WSGURL wsgurl = WSGURL(serverName, false);

    WSGRequest::GetInstance().PerformRequest(wsgurl, responseObject, m_verifyPeer);

    Json::Value instances(Json::objectValue);
    if (responseObject.ValidateJSONResponse(instances, "instances") != RequestStatus::OK)
        return returnVec;
    
    for (auto instance : instances["instances"])
        {
        if (instance.isMember("properties") && instance["properties"].isMember("Name"))
            returnVec.push_back(instance["properties"]["Name"].asCString());
        }

    return returnVec;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
bvector<Utf8String> WSGServer::GetClassNames(Utf8String repoId, Utf8String schemaName, RawServerResponse& responseObject)
    {
    bvector<Utf8String> returnVec;
    Utf8String serverName = m_serverName;
    if (!serverName.EndsWith("/"))
        serverName.append("/");
    serverName.append("v");
    RawServerResponse versionResponse = RawServerResponse();
    serverName.append(GetVersion(versionResponse));
    if (versionResponse.responseCode > 399)
        {
        responseObject = versionResponse;
        return returnVec;
        }

    serverName.append("/Repositories/");
    serverName.append(repoId);
    serverName.append("/MetaSchema/ECClassDef?$filter=SchemaHasClass-backward-ECSchemaDef.Name+in+['");
    serverName.append(schemaName);
    serverName.append("']");
    WSGURL wsgurl = WSGURL(serverName, false);

    WSGRequest::GetInstance().PerformRequest(wsgurl, responseObject, m_verifyPeer);

    Json::Value instances(Json::objectValue);
    if(responseObject.ValidateJSONResponse(instances, "instances") != RequestStatus::OK)
        return returnVec;

    for(auto instance : instances["instances"])
        {
        if (instance.isMember("properties") && instance["properties"].isMember("Name"))
            returnVec.push_back(instance["properties"]["Name"].asCString());
        }

    return returnVec;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
Utf8String WSGServer::GetJSONClassDefinition(Utf8String repoName, Utf8String schemaName, Utf8String className, RawServerResponse& responseObject)
    {
    Utf8String returnString = "";
    Utf8String serverName = m_serverName;
    if (!serverName.EndsWith("/"))
        serverName.append("/");
    serverName.append("v");
    RawServerResponse versionResponse = RawServerResponse();
    serverName.append(GetVersion(versionResponse));
    if (versionResponse.responseCode > 399)
        {
        responseObject = versionResponse;
        return returnString;
        }

    serverName.append("/Repositories/");
    serverName.append(repoName);
    serverName.append("/MetaSchema/ECClassDef?$filter=SchemaHasClass-backward-ECSchemaDef.Name+in+['");
    serverName.append(schemaName);
    serverName.append("']");
    WSGURL wsgurl = WSGURL(serverName, false);

    WSGRequest::GetInstance().PerformRequest(wsgurl, responseObject, m_verifyPeer);

    bvector<Utf8String> props;
    BeStringUtilities::Split(responseObject.body.c_str(), "{}", props);

    for(Utf8String prop : props)
        {
        if(prop.Contains(className))
            {
            returnString = "{";
            returnString.append(prop);
            returnString.append("}");
            }
        }

    return returnString;
    }