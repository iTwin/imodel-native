/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/WSGServices.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <curl/curl.h>

#include <Bentley/Bentley.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatform/WSGServices.h>
#include <BentleyDesktopClient/CCApi/CCPublic.h>
#include <RealityPlatform/RealityPlatformAPI.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Francis Boily                   09/2015
//+---------------+---------------+---------------+---------------+---------------+------*/
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
    ((Utf8String*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Francis Boily                   09/2015
//+---------------+---------------+---------------+---------------+---------------+------*/
static size_t WriteData(void *contents, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = fwrite(contents, size, nmemb, stream);
    return written;
}

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

WSGRequest::WSGRequest() : CurlConstructor()
    {
    s_instance = this;
    }

WSGRequest* WSGRequest::s_instance = nullptr;
WSGRequest& WSGRequest::GetInstance()
    {
    if(nullptr == s_instance)
        s_instance = new WSGRequest();
    return *s_instance;
    }

Utf8String CurlConstructor::GetToken()
    {
    if((std::time(nullptr) - m_tokenRefreshTimer) > (59 * 60)) //refresh required every 60 minutes
        RefreshToken();
    return m_token;
    }

void CurlConstructor::RefreshToken()
    {
    m_tokenRefreshTimer = std::time(nullptr);

    CCAPIHANDLE api = CCApi_InitializeApi(COM_THREADING_Multi);
    CallStatus status = APIERR_SUCCESS;

    bool installed;
    status = CCApi_IsInstalled(api, &installed);
    if (!installed)
        return;
    bool running = false;
    status = CCApi_IsRunning(api, &running);
    if (status != APIERR_SUCCESS || !running)
        return;
    bool loggedIn = false;
    status = CCApi_IsLoggedIn(api, &loggedIn);
    if (status != APIERR_SUCCESS || !loggedIn)
        return;
    bool acceptedEula = false;
    status = CCApi_HasUserAcceptedEULA(api, &acceptedEula);
    if (status != APIERR_SUCCESS || !acceptedEula)
        return;
    bool sessionActive = false;
    status = CCApi_IsUserSessionActive(api, &sessionActive);
    if (status != APIERR_SUCCESS || !sessionActive)
        return;

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
    }

Utf8String WSGRequest::PerformRequest(const WSGURL& wsgRequest, int& result, int verifyPeer, FILE* file, bool retry) const
    {
    result = RequestType::Body;
    return _PerformRequest(wsgRequest, result, verifyPeer, file, retry);
    }

Utf8String WSGRequest::PerformHeaderRequest(const WSGURL& wsgRequest, int& result, int verifyPeer, FILE* file, bool retry) const
    {
    result = RequestType::Header;
    return _PerformRequest(wsgRequest, result, verifyPeer, file, retry);
    }

Utf8String WSGRequest::PerformAzureRequest(const WSGURL& wsgRequest, int& result, int verifyPeer, FILE* file, bool retry) const
    {
    result = RequestType::BodyNoToken;
    return _PerformRequest(wsgRequest, result, verifyPeer, file, retry);
    }

CURL* CurlConstructor::PrepareCurl(const WSGURL& wsgRequest, int& code, int verifyPeer, FILE* file) const
    {
    CURL* curl = curl_easy_init();
    if (nullptr == curl)
        {
        code = CURLcode::CURLE_FAILED_INIT;
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

    bvector<Utf8String> wsgHeaders = wsgRequest.GetRequestHeader();
    for (Utf8String header : wsgHeaders)
        headers = curl_slist_append(headers, header.c_str());
    if(code != RequestType::BodyNoToken)
        headers = curl_slist_append(headers, m_token.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, wsgRequest.GetHttpRequestString());

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, verifyPeer);

    curl_easy_setopt(curl, CURLOPT_CAINFO, m_certificatePath.GetNameUtf8());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_HEADEROPT, CURLHEADER_SEPARATE);

    return curl;
    }

CURL* WSGRequest::PrepareRequest(const WSGURL& wsgRequest, int& result, Utf8StringP returnString, int verifyPeer, FILE* file, bool retry) const
    {
    CURL* curl = PrepareCurl(wsgRequest, result, verifyPeer, file);

    if (file != nullptr)
        {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
        }
    else if (result == RequestType::Header)
        {
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, returnString);
        Utf8StringP dummyString = new Utf8String(); //if you don't handle the body, it gets printed to the output
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, dummyString);
        //delete dummyString;
        }
    else
        {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, returnString);
        }

    return curl;
    }

Utf8String WSGRequest::_PerformRequest(const WSGURL& wsgRequest, int& result, int verifyPeer, FILE* file, bool retry) const
    {
    Utf8StringP curlString = new Utf8String();
    auto curl = PrepareRequest(wsgRequest, result, curlString, verifyPeer, file, retry);

    result = (int)curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    Utf8String returnString = *curlString;
    //delete curlString;
    if (returnString.Contains("Token is not valid") && retry)
        {
        WSGRequest::GetInstance().RefreshToken();
        return WSGRequest::GetInstance().PerformRequest(wsgRequest, result, verifyPeer, file, false);
        }

    return returnString;
    }

WSGURL::WSGURL(Utf8String url) : 
    m_validRequestString(true), m_requestType(HttpRequestType::GET_Request), m_httpRequestString(url), m_requestHeader(bvector<Utf8String>())
    {}

WSGURL::WSGURL(Utf8String server, Utf8String version, Utf8String repoId, Utf8String pluginName, Utf8String schema, WSGInterface _interface, Utf8String className, Utf8String id, bool objectContent)
    :m_serverName(server), m_version(version), m_repoId(repoId), m_pluginName(pluginName), m_schema(schema), m_interface(_interface), m_className(className), m_id(id), m_objectContent(objectContent),
    m_validRequestString(false), m_requestType(HttpRequestType::GET_Request)
    {}

Utf8StringCR WSGURL::GetServerName() const { return m_serverName; }
void WSGURL::SetServerName(Utf8String serverName) { m_serverName = serverName; }

Utf8StringCR WSGURL::GetVersion() const { return m_version; }
void WSGURL::SetVersion(Utf8String version) { m_version = version; }

Utf8StringCR WSGURL::GetPluginName() const { return m_pluginName; }
void WSGURL::SetPluginName(Utf8String pluginName) { m_pluginName = pluginName; }

Utf8StringCR WSGURL::GetSchema() const { return m_schema; }
void WSGURL::SetSchema(Utf8String schema) { m_schema = schema; }

WSGURL::WSGInterface WSGURL::GetInterface() const { return m_interface; }
void WSGURL::SetInterface(WSGInterface _interface) { m_interface = _interface; }

Utf8StringCR WSGURL::GetClassName() const { return m_className; }
void WSGURL::SetClassName(Utf8String className) { m_className = className; }

Utf8StringCR WSGURL::GetId() const { return m_id; }
void WSGURL::SetId(Utf8String id) { m_id = id; }

bool WSGURL::GetContentFlag() const { return m_objectContent; }
WSGURL::HttpRequestType WSGURL::GetRequestType() const { return m_requestType; }

Utf8StringCR WSGURL::GetRepoId() const { return m_repoId; }
void WSGURL::SetRepoId(Utf8String repoId) { m_repoId = repoId; }

void WSGURL::_PrepareHttpRequestStringAndPayload() const
    {
    m_validRequestString = false;
    //https://localhost/ws/v2.1/
    m_httpRequestString = "https://";
    m_httpRequestString.append(m_serverName);
    
    m_requestHeader = bvector<Utf8String>();
    /*m_requestHeader.push_back("Accept: application / json");
    m_requestHeader.push_back("Content-Type: application/json");
    m_requestHeader.push_back("charsets: utf-8");*/

    m_validRequestString = true;
    }

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
        }
    if(rootNode.length() == 0) //navRoot
        {
        m_rootNode = m_navString;
        m_rootId = m_instanceId;
        }
    }

Utf8String NavNode::GetNavString() { return m_navString; }
Utf8String NavNode::GetTypeSystem() { return m_typeSystem; }
Utf8String NavNode::GetSchemaName() { return m_schemaName; }
Utf8String NavNode::GetClassName() { return m_className; }
Utf8String NavNode::GetInstanceId() { return m_instanceId; }
Utf8String NavNode::GetRootNode() { return m_rootNode; }
Utf8String NavNode::GetRootId() { return m_rootId; }

NodeNavigator* NodeNavigator::s_nnInstance = nullptr;
NodeNavigator& NodeNavigator::GetInstance()
    {
    if (nullptr == s_nnInstance)
        s_nnInstance = new NodeNavigator();
    return *s_nnInstance;
    }

NodeNavigator::NodeNavigator()
    {
    s_nnInstance = this;
    }

bvector<NavNode> NodeNavigator::GetRootNodes(Utf8String serverName, Utf8String repoId)
    {
    WSGServer server = WSGServer(serverName, false);
    return GetRootNodes(server, repoId);
    }

bvector<NavNode> NodeNavigator::GetRootNodes(WSGServer server, Utf8String repoId)
    {
    bvector<NavNode> returnVector = bvector<NavNode>();
    WSGNavRootRequest* navRoot = new WSGNavRootRequest(server.GetServerName(), server.GetVersion(), repoId);

    int status = RequestType::Body;
    Utf8String returnJsonString = WSGRequest::GetInstance().PerformRequest(*navRoot, status, 0);

    delete navRoot;
    Json::Value instances(Json::objectValue);
    if((status != CURLE_OK) || !Json::Reader::Parse(returnJsonString, instances) || instances.isMember("errorMessage") || !instances.isMember("instances"))
        return returnVector; 

    for (auto instance : instances["instances"])
        returnVector.push_back(NavNode(instance));

    return returnVector;
    }

bvector<NavNode> NodeNavigator::GetChildNodes(WSGServer server, Utf8String repoId, NavNode& parentNode)
    {
    Utf8String navString = parentNode.GetRootNode();
    if(!navString.Contains(parentNode.GetInstanceId()))
        {
        navString.append("~2F");
        navString.append(parentNode.GetInstanceId());
        }
    return GetChildNodes(server, repoId, navString);
    }

bvector<NavNode> NodeNavigator::GetChildNodes(WSGServer server, Utf8String repoId, Utf8String nodePath)
{
    nodePath.ReplaceAll("/", "~2F");

    bvector<Utf8String> lines;
    BeStringUtilities::Split(nodePath.c_str(), "~", lines);
    Utf8String rootNode = lines[0];

    Utf8String rootId = rootNode.substr(rootNode.length() - 36, rootNode.length()); // 36 = size of GUID

    bvector<NavNode> returnVector = bvector<NavNode>();
    WSGNavNodeRequest* navNode = new WSGNavNodeRequest(server.GetServerName(), server.GetVersion(), repoId, nodePath);
    int status = RequestType::Body;
    Utf8String returnJsonString = WSGRequest::GetInstance().PerformRequest(*navNode, status, 0);

    delete navNode;
    Json::Value instances(Json::objectValue);
    if ((status != CURLE_OK) || !Json::Reader::Parse(returnJsonString, instances) || instances.isMember("errorMessage") || !instances.isMember("instances"))
        return returnVector;

    for (auto instance : instances["instances"])
        returnVector.push_back(NavNode(instance, rootNode, rootId));

    return returnVector;
}

WSGNavRootRequest::WSGNavRootRequest(Utf8String server, Utf8String version, Utf8String repoId)
    {
    m_serverName = server;
    m_version = version;
    m_repoId = repoId;
    m_validRequestString = false;
    m_requestType = HttpRequestType::GET_Request;
    m_interface = WSGInterface::NavNode;
    }

void WSGNavRootRequest::_PrepareHttpRequestStringAndPayload() const
    {
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/v");
    m_httpRequestString.append(m_version);
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(m_repoId);
    m_httpRequestString.append("/Navigation/NavNode/");
    }

WSGNavNodeRequest::WSGNavNodeRequest(Utf8String server, Utf8String version, Utf8String repoId, Utf8String nodeId)
    {
    m_serverName = server;
    m_version = version;
    m_repoId = repoId;
    m_id = nodeId;
    m_validRequestString = false;
    m_requestType = HttpRequestType::GET_Request;
    m_interface = WSGInterface::NavNode;
    }

void WSGNavNodeRequest::_PrepareHttpRequestStringAndPayload() const
    {
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/v");
    m_httpRequestString.append(m_version);
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(m_repoId);
    m_httpRequestString.append("/Navigation/NavNode/");
    m_httpRequestString.append(m_id);
    m_httpRequestString.append("/NavNode");
    }

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

void WSGObjectRequest::_PrepareHttpRequestStringAndPayload() const
    {
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/v");
    m_httpRequestString.append(m_version);
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(m_repoId);
    m_httpRequestString.append("/");
    m_httpRequestString.append(m_schema);
    m_httpRequestString.append("/");
    m_httpRequestString.append(m_className);
    m_httpRequestString.append("/");
    m_httpRequestString.append(m_id);
    }

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

void WSGObjectContentRequest::_PrepareHttpRequestStringAndPayload() const
    {
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/v");
    m_httpRequestString.append(m_version);
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(m_repoId);
    m_httpRequestString.append("/");
    m_httpRequestString.append(m_schema);
    m_httpRequestString.append("/");
    m_httpRequestString.append(m_className);
    m_httpRequestString.append("/");
    m_httpRequestString.append(m_id);
    m_httpRequestString.append("/$file");
    }

WSGObjectListPagedRequest::WSGObjectListPagedRequest(Utf8String server, Utf8String version, Utf8String pluginName, Utf8String schema, Utf8String className)
    {
    m_startIndex = 0;
    m_pageSize = 25;
    m_serverName = server;
    m_version = version;
    m_pluginName = pluginName;
    m_schema = schema;
    m_className = className;
    m_validRequestString = false;
    m_requestType = HttpRequestType::GET_Request;
    }

void WSGObjectListPagedRequest::_PrepareHttpRequestStringAndPayload() const
    {
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/v");
    m_httpRequestString.append(m_version);
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(m_repoId);
    m_httpRequestString.append("/");
    m_httpRequestString.append(m_schema);
    m_httpRequestString.append("/");
    m_httpRequestString.append(m_className);
    m_httpRequestString.append("?$skip=");
    Utf8P buf = new Utf8Char();
    BeStringUtilities::FormatUInt64(buf, m_startIndex);
    m_httpRequestString.append(buf);
    m_httpRequestString.append("&$top=");
    BeStringUtilities::FormatUInt64(buf, m_pageSize);
    m_httpRequestString.append(buf);
    delete buf;
    }

bvector<Utf8String> WSGServer::GetPlugins() const
    {
    //https://localhost/v2.4/Plugins 
    Utf8String serverName = m_serverName;
    serverName.append("/v");
    serverName.append(GetVersion());
    serverName.append("/Plugins");
    WSGURL wsgurl = WSGURL(serverName, "", "", "", "", WSGURL::WSGInterface::Repositories, "", "", false);

    bvector<Utf8String> returnVec = bvector<Utf8String>();

    int status = RequestType::Body;
    Utf8String returnJsonString = WSGRequest::GetInstance().PerformRequest(wsgurl, status, m_verifyPeer);

    Json::Value instances(Json::objectValue);
    if ((status != CURLE_OK) || (!Json::Reader::Parse(returnJsonString, instances) || instances.isMember("errorMessage") || !instances.isMember("instances")))
        return returnVec;

    for (auto instance : instances["instances"])
        {
        if (instance.isMember("instanceId"))
            returnVec.push_back(instance["instanceId"].asCString());
        }

    return returnVec;
    }

Utf8String WSGServer::GetVersion() const
    {
    if(m_version.length() > 0)
        return m_version;

    Utf8String serverName = m_serverName;
    serverName.append("/v2.4/Plugins");
    WSGURL wsgurl = WSGURL(serverName, "", "", "", "", WSGURL::WSGInterface::Repositories, "", "", false);

    bvector<Utf8String> returnVec = bvector<Utf8String>();

    int status = RequestType::Header;

    Utf8String returnString = WSGRequest::GetInstance().PerformHeaderRequest(wsgurl, status, m_verifyPeer);

    if (status != CURLE_OK)
        return "";

    const char* charstring = returnString.c_str();
    Utf8String keyword = "Bentley-WebAPI/";
    const char* substringPosition = strstr(charstring, keyword.c_str());
    substringPosition+= keyword.length();
    Utf8String versionString = Utf8String(substringPosition);

    bvector<Utf8String> lines;
    BeStringUtilities::Split(versionString.c_str(), "\n", lines);

    m_version = lines[0];
    return m_version;
    }

bvector<Utf8String> WSGServer::GetRepositories() const
    {
    //https://localhost/ws/v2.1/Repositories 
    Utf8String serverName = m_serverName;
    serverName.append("/v");
    serverName.append(GetVersion());
    serverName.append("/Repositories");
    WSGURL wsgurl = WSGURL(serverName, "", "", "", "", WSGURL::WSGInterface::Repositories, "", "", false);

    bvector<Utf8String> returnVec = bvector<Utf8String>();

    int status = RequestType::Body;
    Utf8String returnJsonString = WSGRequest::GetInstance().PerformRequest(wsgurl, status, m_verifyPeer);
    
    Json::Value instances(Json::objectValue);
    if ((status != CURLE_OK) || (!Json::Reader::Parse(returnJsonString, instances) || instances.isMember("errorMessage") || !instances.isMember("instances")))
        return returnVec;
    
    for (auto instance : instances["instances"])
        {
        if (instance.isMember("instanceId"))
            returnVec.push_back(instance["instanceId"].asCString());
        }

    return returnVec;
    }

bvector<Utf8String> WSGServer::GetSchemaNames(Utf8String repoName) const
    {
    //GET https://localhost/ws/v2.1/Repositories/{repoId}/MetaSchema/ECSchemaDef
    Utf8String serverName = m_serverName;
    serverName.append("/v");
    serverName.append(GetVersion());
    serverName.append("/Repositories/");
    serverName.append(repoName);
    serverName.append("/MetaSchema/ECSchemaDef");

    WSGURL wsgurl = WSGURL(serverName, "", "", "", "", WSGURL::WSGInterface::Repositories, "", "", false);

    bvector<Utf8String> returnVec = bvector<Utf8String>();

    int status = RequestType::Body;
    Utf8String returnJsonString = WSGRequest::GetInstance().PerformRequest(wsgurl, status, m_verifyPeer);

    Json::Value instances(Json::objectValue);
    if ((status != CURLE_OK) || (!Json::Reader::Parse(returnJsonString, instances) || instances.isMember("errorMessage") || !instances.isMember("instances")))
        return returnVec;
    
    for (auto instance : instances["instances"])
        {
        if (instance.isMember("properties") && instance["properties"].isMember("Name"))
            returnVec.push_back(instance["properties"]["Name"].asCString());
        }

    return returnVec;
    }

bvector<Utf8String> WSGServer::GetClassNames(Utf8String repoId, Utf8String schemaName)
    {
    //https://localhost/ws/v2.1/Repositories/{repoId}/MetaSchema/ECClassDef?$filter=SchemaHasClass-backward-ECSchemaDef.Name+in+[‘{schemaName}’]
    Utf8String serverName = m_serverName;
    serverName.append("/v");
    serverName.append(GetVersion());
    serverName.append("/Repositories/");
    serverName.append(repoId);
    serverName.append("/MetaSchema/ECClassDef?$filter=SchemaHasClass-backward-ECSchemaDef.Name+in+['");
    serverName.append(schemaName);
    serverName.append("']");
    WSGURL wsgurl = WSGURL(serverName, "", "", "", "", WSGURL::WSGInterface::Repositories, "", "", false);

    bvector<Utf8String> returnVec = bvector<Utf8String>();

    int status = RequestType::Body;
    Utf8String returnJsonString = WSGRequest::GetInstance().PerformRequest(wsgurl, status, m_verifyPeer);

    Json::Value instances(Json::objectValue);
    if((status != CURLE_OK) || (!Json::Reader::Parse(returnJsonString, instances) || instances.isMember("errorMessage") || !instances.isMember("instances")))
        return returnVec;

    for(auto instance : instances["instances"])
        {
        if (instance.isMember("properties") && instance["properties"].isMember("Name"))
            returnVec.push_back(instance["properties"]["Name"].asCString());
        }

    return returnVec;
    }

Utf8String WSGServer::GetJSONClassDefinition(Utf8String repoName, Utf8String schemaName, Utf8String className)
    {
    //https://localhost/ws/v2.1/Repositories/{repoId}/MetaSchema/ECClassDef?$filter=SchemaHasClass-backward-ECSchemaDef.Name+in+[‘{schemaName}’]
    Utf8String serverName = m_serverName;
    serverName.append("/v");
    serverName.append(GetVersion());
    serverName.append("/Repositories/");
    serverName.append(repoName);
    serverName.append("/MetaSchema/ECClassDef?$filter=SchemaHasClass-backward-ECSchemaDef.Name+in+['");
    serverName.append(schemaName);
    serverName.append("']");
    WSGURL wsgurl = WSGURL(serverName, "", "", "", "", WSGURL::WSGInterface::Repositories, "", "", false);

    int status = RequestType::Body;
    Utf8String returnString = WSGRequest::GetInstance().PerformRequest(wsgurl, status, m_verifyPeer);

    bvector<Utf8String> props;
    BeStringUtilities::Split(returnString.c_str(), "{}", props);

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