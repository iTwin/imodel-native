/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__


#include <ctime>

#include <Bentley/DateTime.h>
#include <Bentley/BeFile.h>
#include <RealityPlatform/RealityPlatformAPI.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <WebServices/iModelHub/Client/OidcTokenProvider.h>


BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! Overview:
//! The present classes serve tools common to all WSG related services.
//!
//! The main class provides a small framework to compose WSG related HTTP requests
//! based on the various components.
//!
//! In addition we also define a common interface for accessing the NavNode
//! interface from a WSG-based service.
//!
//=====================================================================================

//=====================================================================================
//! @bsiclass                                   Spencer.Mason              03/2017
//! Determines whether the CC token should be added to the headers of the request
//=====================================================================================
enum ServerType
    {
    WSG = -1,
    Azure = -2
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason            	     8/2016
//=====================================================================================
enum class CONNECTServerType
    {
    DEV,
    QA,
    PERF,
    PROD
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason              03/2017
//=====================================================================================
enum RequestStatus
    {
    OK = 0, // everything seems successful
    BADREQ = 1, // either an http error or a tool error was encountered
    LASTPAGE = 2, // the request returned fewer reponses than the page size of the request
    PARAMSNOTSET = 3, // indicates that the client has not yet set the server info
    UNSENT = 4 // default status, before a response has been received
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason              03/2017
//! RawServerResponse
//! struct used to hold and return all pertinent elements regarding a response.
//=====================================================================================
struct RawServerResponse
    {
public:
    Utf8String header;
    Utf8String body;
    long responseCode;
    int toolCode;
    RequestStatus status;

    RawServerResponse():responseCode(-1), toolCode(ServerType::WSG), status(RequestStatus::UNSENT),
    header(Utf8String()), body(Utf8String()){}
    
    //! Performs a basic check of the response received to determine if the request was successful
    REALITYDATAPLATFORM_EXPORT RequestStatus ValidateResponse();
    
    //! Analyses the json response received to ensure that an error was not received at that
    //! the structure corresponds to the user's expectations (contains keyword)
    REALITYDATAPLATFORM_EXPORT RequestStatus ValidateJSONResponse(Json::Value& instances, Utf8StringCR keyword);

    //! Resets all values to their initial state
    void clear() {header.clear(); body.clear(); responseCode = -1; toolCode=ServerType::WSG; status = RequestStatus::UNSENT;}

    //! Compares a Request Status to the request status imbedded in the response
    bool operator==(RequestStatus compareStatus) {return (compareStatus == status);}
    bool operator!=(RequestStatus compareStatus) {return (compareStatus != status);}
    };

// Operator (left hand request status to rawServerResponse compare)
inline bool operator==(RequestStatus compareStatus, RawServerResponse& response) {return (compareStatus == response.status);}
inline bool operator!=(RequestStatus compareStatus, RawServerResponse& response) {return (compareStatus != response.status);}

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! WSGURL
//! This class represents a URL to a WSG request.
//! Although the class is concrete is is usually meant to be overloaded to represent
//! specific requests to a specific service.
//! There are two different versions of WSGURL provided. One allows accessing
//! classes instances or their content.
//! Another type enables traversing the NavNode structure
//! Finally the last one is specialised to build URL intended to obtain list
//! of objects using pages, sort orders, etc.
//=====================================================================================
struct WSGURL
    {
public:
    //! Creates a WSGURL object from the full unparsed URL. The various components
    //! can be extracted from the URL and modified.
    REALITYDATAPLATFORM_EXPORT WSGURL(Utf8String url, bool validString = true);
    
    //! Creates a WSG URL from the various provided components
    REALITYDATAPLATFORM_EXPORT WSGURL(Utf8String server, Utf8String version, Utf8String repoId, Utf8String schema, Utf8String className, Utf8String id);

    REALITYDATAPLATFORM_EXPORT virtual ~WSGURL() {}

    //! Get the server name. The string provided or returned only contains
    //! the server name without trailling or leading slashes and without the communication version
    //! for example the server name could be 'dev-contextservices.connect.com'
    //! SetServer() will return an error is the server string provided is invalid
    REALITYDATAPLATFORM_EXPORT virtual Utf8StringCR GetServerName() const;

    //! A string containing the version. The version must of the format number dot number such as '2.4'
    //! otherwise an error will be returned
    //! Note that the method does not validate the support of the specified version.
    REALITYDATAPLATFORM_EXPORT virtual Utf8StringCR GetVersion() const;

    //! A string containing the repository id. A WSG server can host many repositories so specification is required.
    REALITYDATAPLATFORM_EXPORT virtual Utf8StringCR GetRepoId() const;

    //! The Schema. A plugin can publish many schemas so specification is required.
    //! Note that the method does not validate the publication of the specified schema.
    REALITYDATAPLATFORM_EXPORT virtual Utf8StringCR GetSchema() const;

    //! The class name. 
    //! Note that the method does not validate the class is part of the schema.
    REALITYDATAPLATFORM_EXPORT virtual Utf8StringCR GetECClassName() const;

    REALITYDATAPLATFORM_EXPORT virtual Utf8StringCR GetUserAgent() const;
    REALITYDATAPLATFORM_EXPORT virtual void SetUserAgent(Utf8StringCR userAgent);


    //! The id of the object requested. If the request does not require
    //! an identifier then this field should remain empty
    REALITYDATAPLATFORM_EXPORT virtual Utf8StringCR GetId() const;
    REALITYDATAPLATFORM_EXPORT virtual void SetId(Utf8String id);

    enum HttpRequestType
        {
        GET_Request,
        PUT_Request,
        POST_Request,
        DELETE_Request
        };

    //! The type of http request (GET, PUT, POST)
    REALITYDATAPLATFORM_EXPORT virtual HttpRequestType GetRequestType() const;

    //! Returns the full http request string
    REALITYDATAPLATFORM_EXPORT virtual Utf8StringCR GetHttpRequestString() const
        {
        if (!m_validRequestString)
            _PrepareHttpRequestStringAndPayload();

        BeAssert(m_validRequestString);
        BeAssert(m_httpRequestString.size() != 0);

        return m_httpRequestString;
        };

    //! Returns the request payload if any
    REALITYDATAPLATFORM_EXPORT virtual Utf8StringCR GetRequestPayload() const
        {
        if (!m_validRequestString)
            _PrepareHttpRequestStringAndPayload();

        BeAssert(m_validRequestString);

        return m_requestPayload;
        }

    //! Returns the http header components
    REALITYDATAPLATFORM_EXPORT virtual bvector<Utf8String> const & GetRequestHeader() const
        {
        if (!m_validRequestString)
            _PrepareHttpRequestStringAndPayload();

        BeAssert(m_validRequestString);

        return m_requestHeader;
        }

    //! Returns the http headers as a single string
    REALITYDATAPLATFORM_EXPORT virtual Utf8String GetRequestHeaders() const
        {
        bvector<Utf8String> const & headers = GetRequestHeader();
        Utf8String headerString = "";
        for(Utf8String header : headers)
            headerString.append(Utf8PrintfString("%s\t", header.c_str()));
        return headerString;
        }

    WSGURL() : m_validRequestString(false), m_requestType(HttpRequestType::GET_Request) {}
    WSGURL(WSGURL const& object);
    REALITYDATAPLATFORM_EXPORT WSGURL& operator=(WSGURL const & object);

protected:
    //! Function to url encode the id, depending on which server is receiving it
    REALITYDATAPLATFORM_EXPORT virtual void EncodeId() const;

    REALITYDATAPLATFORM_EXPORT void SetServerName(Utf8String serverName);
    REALITYDATAPLATFORM_EXPORT void SetPluginName(Utf8String pluginName);
    REALITYDATAPLATFORM_EXPORT void SetVersion(Utf8String version);
    REALITYDATAPLATFORM_EXPORT void SetECClassName(Utf8String className);
    REALITYDATAPLATFORM_EXPORT void SetSchema(Utf8String schema);
    REALITYDATAPLATFORM_EXPORT void SetRepoId(Utf8String repoId);

    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const;

    mutable Utf8String m_serverName;
    Utf8String m_version;
    Utf8String m_schema;
    Utf8String m_className;
    Utf8String m_repoId;
    mutable Utf8String m_userAgent;
    mutable Utf8String m_id;
    mutable Utf8String m_encodedId;

    mutable bool m_validRequestString;
    mutable Utf8String m_httpRequestString;
    mutable Utf8String m_requestPayload;
    mutable bvector<Utf8String> m_requestHeader; 
    mutable HttpRequestType m_requestType;
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason              02/2017
//! NavNode
//! This class represents a single NavNode returned from a NavNode request.
//! The WSG NavNode interface enables to navigate a tree-like structure exposed by a WSG 
//! service.
//=====================================================================================
struct NavNode
    {
public:
    REALITYDATAPLATFORM_EXPORT NavNode(Json::Value jsonObject, Utf8String rootNode = "", Utf8String rootId = "");
    REALITYDATAPLATFORM_EXPORT NavNode();
    REALITYDATAPLATFORM_EXPORT NavNode(Utf8StringCR schema, Utf8String id, Utf8StringCR typeSystem, Utf8StringCR className);

    REALITYDATAPLATFORM_EXPORT Utf8String GetNavString() const;
    REALITYDATAPLATFORM_EXPORT Utf8String GetTypeSystem() const;
    REALITYDATAPLATFORM_EXPORT Utf8String GetSchemaName() const;
    REALITYDATAPLATFORM_EXPORT Utf8String GetECClassName() const;
    REALITYDATAPLATFORM_EXPORT Utf8String GetInstanceId() const;
    REALITYDATAPLATFORM_EXPORT Utf8String GetLabel() const;
    REALITYDATAPLATFORM_EXPORT Utf8String GetRootNode() const;
    REALITYDATAPLATFORM_EXPORT Utf8String GetRootId() const;

protected:
    Utf8String m_navString;
    Utf8String m_typeSystem;
    Utf8String m_schemaName;
    Utf8String m_className;
    Utf8String m_instanceId;
    Utf8String m_label;

    Utf8String m_rootNode;
    Utf8String m_rootId;
    };

struct WSGServer; //forward declaration

//=====================================================================================
//! @bsiclass                                   Spencer.Mason              02/2017
//! NodeNavigator
//! This class encapsulates the NavNode interface. It allows client code to navigate
//! and explore the NavNode tree
//=====================================================================================
struct NodeNavigator
    {
    static NodeNavigator* s_nnInstance;

public:
    REALITYDATAPLATFORM_EXPORT static NodeNavigator& GetInstance();

    //! Returns the NavNodes for all RealityData entries on the server
    REALITYDATAPLATFORM_EXPORT bvector<NavNode> GetRootNodes(Utf8String serverName, Utf8String repoId, RawServerResponse& responseObject);
    REALITYDATAPLATFORM_EXPORT bvector<NavNode> GetRootNodes(WSGServer& server, Utf8String repoId, RawServerResponse& responseObject);

    //! Returns the NavNodes beneath the provided NavNode path
    REALITYDATAPLATFORM_EXPORT bvector<NavNode> GetChildNodes(Utf8String serverName, Utf8String serverVersion, Utf8String repoId, Utf8String nodePath, RawServerResponse& responseObject);
    REALITYDATAPLATFORM_EXPORT bvector<NavNode> GetChildNodes(WSGServer& server, Utf8String repoId, Utf8String nodePath, RawServerResponse& responseObject);
    REALITYDATAPLATFORM_EXPORT bvector<NavNode> GetChildNodes(WSGServer& server, Utf8String repoId, NavNode& parentNode, RawServerResponse& responseObject);

private:    
    NodeNavigator();
    };

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! WSGNavNodeRequest
//! This class represents a URL to a WSG request using the NavNode interface.
//! The WSG NavNode interface enables to navigate a tree-like structure exposed by a WSG 
//! service.
//=====================================================================================
struct WSGNavRootRequest : public WSGURL
    {
public:
    REALITYDATAPLATFORM_EXPORT WSGNavRootRequest(Utf8String server, Utf8String version, Utf8String repoId);
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;
    };

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! WSGNavNodeRequest
//! This class represents a URL to a WSG request using the NavNode interface.
//! The WSG NavNode interface enables to navigate a tree-like structure exposed by a WSG 
//! service.
//=====================================================================================
struct WSGNavNodeRequest: public WSGURL
    {
public:
    REALITYDATAPLATFORM_EXPORT WSGNavNodeRequest(Utf8String server, Utf8String version, Utf8String repoId, Utf8String nodeId);
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;
    };

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! WSGObjectRequest
//! This class represents a URL to a WSG request for a single designated object.
//=====================================================================================
struct WSGObjectRequest: public WSGURL
    {
public:
    REALITYDATAPLATFORM_EXPORT WSGObjectRequest(Utf8String server, Utf8String version, Utf8String repoName, Utf8String schema, Utf8String className, Utf8String objectId);
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;
    };

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! WSGObjectContentRequest
//! This class represents a URL to a WSG request for a single designated object.
//=====================================================================================
struct WSGObjectContentRequest: public WSGURL
    {
public:
    REALITYDATAPLATFORM_EXPORT WSGObjectContentRequest(Utf8String server, Utf8String version, Utf8String repoName, Utf8String schema, Utf8String className, Utf8String objectId);
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;
    };

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! WSGPagedRequest
//! This class represents a request to a WSG Service that results in
//! a list of objects of potential unknown length.
//! This class inherits from the general request class but adds additional 
//! control for paging.
//! Default page size is 25 with, of course a start index of 0
//! To advance to next/previous page simply call AdvancePage(), RewindPage() or
//! GoToPage()
//=====================================================================================
struct WSGPagedRequest : public WSGURL
    {
public:
    
    REALITYDATAPLATFORM_EXPORT WSGPagedRequest() : m_startIndex(0), m_pageSize(25) {}
    REALITYDATAPLATFORM_EXPORT WSGPagedRequest(uint32_t startIndex, uint16_t pageSize) : m_startIndex(startIndex), m_pageSize(pageSize) {BeAssert(m_pageSize >0);}

    REALITYDATAPLATFORM_EXPORT virtual ~WSGPagedRequest() {}
    REALITYDATAPLATFORM_EXPORT WSGPagedRequest(WSGPagedRequest const& object);
    REALITYDATAPLATFORM_EXPORT WSGPagedRequest& operator=(WSGPagedRequest const & object) 
        { 
        if(&object != this)
            {
            WSGURL::operator=(object);  
            m_startIndex = object.m_startIndex; 
            m_pageSize = object.m_pageSize; 
            } 
        return *this;
        }

    //! Get/Set the page size. It must be greater or equal to 1 
    REALITYDATAPLATFORM_EXPORT void SetPageSize(uint16_t pageSize) 
        {
        BeAssert(pageSize > 0); 
        m_validRequestString = false; 
        m_pageSize = pageSize;
        }
    REALITYDATAPLATFORM_EXPORT uint16_t GetPageSize() const {return m_pageSize;}
  
    //! The start index of the request.
    REALITYDATAPLATFORM_EXPORT void SetStartIndex(uint32_t startIndex) 
        {
        m_validRequestString = false; 
        m_startIndex = startIndex;
        } 
    REALITYDATAPLATFORM_EXPORT uint32_t GetStartIndex() const {return m_startIndex;}

    //! Page oriented methods. These methods advance a page, rewind a page or go to specified page.
    //! according to current page size and start index
    REALITYDATAPLATFORM_EXPORT virtual void AdvancePage() const 
        {
        m_validRequestString = false; 
        m_startIndex += m_pageSize;
        }
    REALITYDATAPLATFORM_EXPORT void RewindPage() 
        {
        m_validRequestString = false; 
        m_startIndex = (m_startIndex <= m_pageSize ? 0 : m_startIndex-m_pageSize);
        }
    REALITYDATAPLATFORM_EXPORT void GoToPage(int page) 
        {
        m_validRequestString = false; 
        m_startIndex = (uint32_t)(page * m_pageSize);
        }

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override = 0; //virtual class, not to be used directly

    mutable uint32_t m_startIndex;
    mutable uint16_t m_pageSize;
    };

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! WSGObjectListPagedRequest
//! This class represents a URL to a WSG request for a list of objects.
//! Since the result is a list the request is paged meaning that not all instances
//! will be returned
//=====================================================================================
struct WSGObjectListPagedRequest: public WSGPagedRequest
    {
public:
    REALITYDATAPLATFORM_EXPORT WSGObjectListPagedRequest(Utf8String server, Utf8String version, Utf8String pluginName, Utf8String schema, Utf8String className);
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;
    };

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! WSGServer
//! This class offers the basic functionality common to all WSG servers.
//! For example it may obtain the list of plug-ins on the server or
//! the list of schemas or the list of classes defined in the schema including their
//! definition.
//!
//! It does not offer upper level services which must be provided by aggregation or
//! specialisation
//=====================================================================================
struct WSGServer
    {
public:
    
    REALITYDATAPLATFORM_EXPORT WSGServer(Utf8String serverName, bool verifyPeer = true) : m_serverName(serverName), m_version("") { m_verifyPeer = verifyPeer; }
    
    //! Returns a list plugins installed to the indicated WSG server
    REALITYDATAPLATFORM_EXPORT bvector<Utf8String> GetPlugins(RawServerResponse& responseObject) const;

    //! Returns the version of WSG understood by the WSG server
    REALITYDATAPLATFORM_EXPORT Utf8String GetVersion(RawServerResponse& responseObject) const;

    //! Returns the list of repositories
    REALITYDATAPLATFORM_EXPORT bvector<Utf8String> GetRepositories(RawServerResponse& responseObject) const;

    //! Returns the list of schemas published by the plugin
    REALITYDATAPLATFORM_EXPORT bvector<Utf8String> GetSchemaNames(Utf8String repoName, RawServerResponse& responseObject) const;

    //! Returns the list of classes exposed by the indicated schema for the plugin
    REALITYDATAPLATFORM_EXPORT bvector<Utf8String> GetClassNames(Utf8String repoId, Utf8String schemaName, RawServerResponse& responseObject);

    //! Returns the JSON fragment applying to the class definition of the requested class
    REALITYDATAPLATFORM_EXPORT Utf8String GetJSONClassDefinition(Utf8String repoName, Utf8String schemaName, Utf8String className, RawServerResponse& responseObject);

    REALITYDATAPLATFORM_EXPORT Utf8String GetServerName() { return m_serverName; }

private:
    Utf8String          m_serverName;
    bool                m_verifyPeer;
    mutable Utf8String  m_version;
    };

//=====================================================================================
//! @bsiclass                                  Spencer.Mason               01/2017
//! ProxyManager
//! Centralized class to handle proxy information, in case multiple RequestConstructors
//! are used
//=====================================================================================
struct ProxyManager
    {
    protected:
        REALITYDATAPLATFORM_EXPORT static ProxyManager* s_pmInstance;
        REALITYDATAPLATFORM_EXPORT ProxyManager();
    public:
        REALITYDATAPLATFORM_EXPORT static ProxyManager& GetInstance();

        //! Set proxy informations
        REALITYDATAPLATFORM_EXPORT void SetProxyUrlAndCredentials(Utf8StringCR proxyUrl, Utf8StringCR proxyCreds) 
            { 
            m_proxyUrl = proxyUrl; 
            m_proxyCreds = proxyCreds; 
            }
        REALITYDATAPLATFORM_EXPORT void GetCurrentProxyUrlAndCredentials(Utf8StringR proxyUrl, Utf8StringR proxyCreds) 
            { 
            proxyUrl = m_proxyUrl; 
            proxyCreds = m_proxyCreds; 
            }

    private:
        Utf8String          m_proxyUrl;
        Utf8String          m_proxyCreds;
    };

//! Callback function to retrieve the token.
//! @return If RealityDataDownload_ProgressCallBack returns 0   All downloads continue.
//! @param[out] token       Token string
//! @param[out] timer       Timestamp of when the token was generated.
typedef std::function<void(Utf8StringR token, time_t& timer)> RequestConstructor_TokenCallBack;

//=====================================================================================
//! @bsiclass                                  Spencer.Mason               01/2017
//! ConnectTokenManager
//! Centralized class to handle proxy information, in case multiple RequestConstructors
//! are used
//=====================================================================================
struct ConnectTokenManager
    {
protected:
    REALITYDATAPLATFORM_EXPORT static ConnectTokenManager* s_ctInstance;
    REALITYDATAPLATFORM_EXPORT ConnectTokenManager();
public:
    REALITYDATAPLATFORM_EXPORT static ConnectTokenManager& GetInstance();
    REALITYDATAPLATFORM_EXPORT static void SetTokenProvider(Utf8String callBackurl, Utf8String accessToken);
    REALITYDATAPLATFORM_EXPORT static void SetTokenProvider(WebServices::IConnectTokenProviderPtr);
    //! Set token callback informations
    REALITYDATAPLATFORM_EXPORT void SetTokenCallback(RequestConstructor_TokenCallBack pi_func) { m_tokenCallback = pi_func; }
    
    REALITYDATAPLATFORM_EXPORT Utf8String GetToken() const; 
    REALITYDATAPLATFORM_EXPORT void RefreshToken() const;

private:
    static WebServices::IConnectTokenProviderPtr       m_tokenProvider;
    static bool                                        m_isFromTokenProvider;
    mutable Utf8String                  m_token;
    mutable time_t                      m_tokenRefreshTimer;
    RequestConstructor_TokenCallBack    m_tokenCallback;
    };

//! Callback function to execute a request
//! @return If RealityDataDownload_ProgressCallBack returns 0   All downloads continue.
//! @param[in] wsgRequest       Structure containing the url/header/body of the request
//! @param[out] response        the response returned by the server
//! @param[in] verifyPeer       determines whether the tool should validate the server's certificate
//! @param[out] file            pointer to a file that will receive the body of the response (can be null if you don't need to write to a file)
typedef std::function<void(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file)> RequestConstructor_RequestCallback;

//=====================================================================================
//! @bsiclass                                  Spencer.Mason               01/2017
//! RequestConstructor
//! Base class for all Requestor classes. Defines functions for CC Token management
//! Also Defines basic request setup which will be common to all requests
//=====================================================================================
struct RequestConstructor
    {
public:
    //! Gets the CC Token
    REALITYDATAPLATFORM_EXPORT Utf8String GetToken() const; 
    REALITYDATAPLATFORM_EXPORT void RefreshToken() const;

    //! user defined path for the .pem file used to authenticate the server
    REALITYDATAPLATFORM_EXPORT BeFileName GetCertificatePath() { return m_certificatePath; }
    REALITYDATAPLATFORM_EXPORT void SetCertificatePath(Utf8String certificatePath) { m_certificatePath = BeFileName(certificatePath); }
    REALITYDATAPLATFORM_EXPORT void SetCertificatePath(BeFileNameCR certificatePath) { m_certificatePath = certificatePath; }

    //! Set proxy informations
    REALITYDATAPLATFORM_EXPORT void SetProxyUrlAndCredentials(Utf8StringCR proxyUrl, Utf8StringCR proxyCreds) const;
    REALITYDATAPLATFORM_EXPORT void GetCurrentProxyUrlAndCredentials(Utf8StringR proxyUrl, Utf8StringR proxyCreds) const;

    REALITYDATAPLATFORM_EXPORT void SetRequestCallback(RequestConstructor_RequestCallback pi_func) { m_requestCallback = pi_func; }

    REALITYDATAPLATFORM_EXPORT RequestConstructor();
    REALITYDATAPLATFORM_EXPORT virtual ~RequestConstructor(){}
protected:
    REALITYDATAPLATFORM_EXPORT void* PrepareRequestBase(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file) const;

    BeFileName                         m_certificatePath;
    RequestConstructor_RequestCallback m_requestCallback;
    };

//=====================================================================================
//! @bsiclass                               Spencer.Mason                 01/2017
//! WSGRequest
//! Creates and executes requests as defined by WSGURL objects
//=====================================================================================
struct WSGRequest : public RequestConstructor
    {
protected:
	REALITYDATAPLATFORM_EXPORT static WSGRequest* s_instance;
    virtual void _PerformRequest(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry) const;
public:
    REALITYDATAPLATFORM_EXPORT static WSGRequest& GetInstance();
    REALITYDATAPLATFORM_EXPORT WSGRequest();

    //! General method. Performs a WSG request and returns de result code in result and
    //! the body in the returned string. If a FILE is provided, the result will be written to a file
    REALITYDATAPLATFORM_EXPORT virtual void PerformRequest(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer = true, BeFile* file = nullptr, bool retry = true) const;
    REALITYDATAPLATFORM_EXPORT virtual void PerformAzureRequest(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer = true, BeFile* file = nullptr, bool retry = true) const;
    REALITYDATAPLATFORM_EXPORT virtual void* PrepareRequest(const WSGURL& wsgRequest, RawServerResponse& responseString, bool verifyPeer = true, BeFile* file = nullptr) const;
    };


END_BENTLEY_REALITYPLATFORM_NAMESPACE