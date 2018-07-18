/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatformTools/WSGServices.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#if defined (BENTLEYCONFIG_OS_WINDOWS)
#include <Windows.h>
#endif
#include <iostream>

#include <Bentley/Bentley.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatformTools/WSGServices.h>
#include <RealityPlatform/RealityPlatformAPI.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason          	    02/2017
//-------------------------------------------------------------------------------------
RequestConstructor::RequestConstructor()
    {
    RefreshToken();
#if defined (BENTLEYCONFIG_OS_WINDOWS)
    WChar exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    WString exeDir = exePath;
    size_t pos = exeDir.find_last_of(L"/\\");
    exeDir = exeDir.substr(0, pos + 1);
    BeFileName caBundlePath = BeFileName(exeDir);
    m_certificatePath = caBundlePath.AppendToPath(L"Assets").AppendToPath(L"http").AppendToPath(L"ContextServices.pem");
    if (!m_certificatePath.DoesPathExist())
        m_certificatePath.clear();
#endif
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
WSGRequest::WSGRequest() : RequestConstructor()
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
Utf8String RequestConstructor::GetToken() const
    {
    return ConnectTokenManager::GetInstance().GetToken();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
void RequestConstructor::RefreshToken() const
    {
    return ConnectTokenManager::GetInstance().RefreshToken();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
void RequestConstructor::SetProxyUrlAndCredentials(Utf8StringCR proxyUrl, Utf8StringCR proxyCreds) const
    {
    ProxyManager::GetInstance().SetProxyUrlAndCredentials(proxyUrl, proxyCreds);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
void RequestConstructor::GetCurrentProxyUrlAndCredentials(Utf8StringR proxyUrl, Utf8StringR proxyCreds) const
    {
    ProxyManager::GetInstance().GetCurrentProxyUrlAndCredentials(proxyUrl, proxyCreds);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
void WSGRequest::PerformRequest(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry) const
    {
    response.clear();
    response.toolCode = ServerType::WSG;
    return _PerformRequest(wsgRequest, response, verifyPeer, file, retry);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
void WSGRequest::PerformAzureRequest(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry) const
    {
    response.clear();
    response.toolCode = ServerType::Azure;
    return _PerformRequest(wsgRequest, response, verifyPeer, file, retry);
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
Utf8String NavNode::GetNavString() const    { return m_navString; }
Utf8String NavNode::GetTypeSystem() const   { return m_typeSystem; }
Utf8String NavNode::GetSchemaName() const   { return m_schemaName; }
Utf8String NavNode::GetECClassName() const  { return m_className; }
Utf8String NavNode::GetInstanceId() const   { return m_instanceId; }
Utf8String NavNode::GetLabel() const        { return m_label; }
Utf8String NavNode::GetRootNode() const     { return m_rootNode; }
Utf8String NavNode::GetRootId() const       { return m_rootId; }

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
bvector<NavNode> NodeNavigator::GetRootNodes(WSGServer& server, Utf8String repoId, RawServerResponse& responseObject)
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
bvector<NavNode> NodeNavigator::GetChildNodes(WSGServer& server, Utf8String repoId, NavNode& parentNode, RawServerResponse& responseObject)
    {
    return GetChildNodes(server, repoId, parentNode.GetNavString(), responseObject);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
bvector<NavNode> NodeNavigator::GetChildNodes(WSGServer& server, Utf8String repoId, Utf8String nodePath, RawServerResponse& responseObject)
    {
    RawServerResponse versionResponse = RawServerResponse();
    Utf8String version = server.GetVersion(versionResponse);
    if (versionResponse.responseCode > 399)
        {
        responseObject = versionResponse;
        return bvector<NavNode>();
        }
    return GetChildNodes(server.GetServerName(), version, repoId, nodePath, responseObject);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                11/2017
//-------------------------------------------------------------------------------------
bvector<NavNode> NodeNavigator::GetChildNodes(Utf8String serverName, Utf8String serverVersion, Utf8String repoId, Utf8String nodePath, RawServerResponse& responseObject)
    {
    nodePath.ReplaceAll("/", "~2F");

    bvector<Utf8String> lines;
    BeStringUtilities::Split(nodePath.c_str(), "~", lines);
    Utf8String rootNode = lines[0];

    Utf8String guidString = rootNode;
    guidString.ReplaceAll("--","-");

    Utf8String rootId = guidString.substr(guidString.length() - 36, rootNode.length()); // 36 = size of GUID

    bvector<NavNode> returnVector;
    WSGNavNodeRequest* navNode = new WSGNavNodeRequest(serverName, serverVersion, repoId, nodePath);

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
    lines.clear();
    BeStringUtilities::Split(m_version.c_str(), ",", lines);
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

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ProxyManager* ProxyManager::s_pmInstance = nullptr;
//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ProxyManager& ProxyManager::GetInstance()
    {
    if (nullptr == s_pmInstance)
        s_pmInstance = new ProxyManager();
    return *s_pmInstance;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ProxyManager::ProxyManager()
    {
    s_pmInstance = this;
    m_proxyUrl = "";
    m_proxyCreds = "";
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectTokenManager* ConnectTokenManager::s_ctInstance = nullptr;
//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectTokenManager& ConnectTokenManager::GetInstance()
    {
    if (nullptr == s_ctInstance)
        s_ctInstance = new ConnectTokenManager();
    return *s_ctInstance;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectTokenManager::ConnectTokenManager() :
    m_token(""), m_tokenRefreshTimer(0)
    {
    s_ctInstance = this;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
Utf8String ConnectTokenManager::GetToken() const
    {
    if ((std::time(nullptr) - m_tokenRefreshTimer) > (59 * 60)) //refresh required every 60 minutes
        RefreshToken();
    return m_token;
    }