/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/WSGServices.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__


#include <ctime>

#include <Bentley/DateTime.h>
#include <curl/curl.h>
#include <sql.h>
#include <sqlext.h>
#include <RealityPlatform/RealityPlatformAPI.h>
#include <BeJsonCpp/BeJsonUtilities.h>

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

    // enumeration for WSG interfaces
    enum class WSGInterface
        {
        Repositories,
        NavNode
        };

    //! Creates a WSGURL object from the full unparsed URL. The various components
    //! can be extracted from the URL and modified.
    REALITYDATAPLATFORM_EXPORT WSGURL(Utf8String url);
    
    //! Creates a WSG URL from the various provided components
    REALITYDATAPLATFORM_EXPORT WSGURL(Utf8String server, Utf8String version, Utf8String repoId, Utf8String schema, WSGInterface _interface, Utf8String className, Utf8String id, bool objectContent);

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

    //! The interface. WSG plugins can implement one on many interfaces. We currently only support two
    //! The Repository interface enables accessing objects related to a schema while the NavNode interface
    //! lists an exposed tree-like structure.
    //! Note that the method does not validate the implementation of the specified interface.
    REALITYDATAPLATFORM_EXPORT virtual WSGInterface GetInterface() const;

    //! The class name. 
    //! Note that the method does not validate the class is part of the schema.
    REALITYDATAPLATFORM_EXPORT virtual Utf8StringCR GetClassName() const;

    //! The id of the object requested. If the request does not require
    //! an identifier then this field should remain empty
    REALITYDATAPLATFORM_EXPORT virtual Utf8StringCR GetId() const;
    REALITYDATAPLATFORM_EXPORT virtual void SetId(Utf8String id);

    //! The object content flag indicates we want or not the content of the file associated with designated object.
    //! The default is false.
    REALITYDATAPLATFORM_EXPORT virtual bool GetContentFlag() const;

    enum HttpRequestType
        {
        GET_Request,
        PUT_Request,
        POST_Request
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

    WSGURL() : m_validRequestString(false), m_requestType(HttpRequestType::GET_Request), m_interface(WSGInterface::Repositories), m_objectContent(false) {}
    ~WSGURL() {}
    WSGURL(WSGURL const& object);
    WSGURL& operator=(WSGURL const & object);

protected:
    void SetServerName(Utf8String serverName);
    void SetPluginName(Utf8String pluginName);
    void SetVersion(Utf8String version);
    void SetClassName(Utf8String className);
    void SetSchema(Utf8String schema);
    void SetInterface(WSGInterface _interface);
    void SetRepoId(Utf8String repoId);

    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const;

    mutable Utf8String m_serverName;
    Utf8String m_version;
    Utf8String m_schema;
    Utf8String m_className;
    WSGInterface m_interface = WSGInterface::Repositories;
    Utf8String m_repoId;
    mutable Utf8String m_id;
    bool m_objectContent = false;


    mutable bool m_validRequestString;
    mutable Utf8String m_httpRequestString;
    mutable Utf8String m_requestPayload;
    mutable bvector<Utf8String> m_requestHeader; 
    mutable HttpRequestType m_requestType;
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason              02/2017
//=====================================================================================
enum RequestType
    {
    Body = 0,
    Header = 1,
    BodyNoToken = 2
    };

enum RequestStatus
    {
    SUCCESS = 0,
    ERROR = 1,
    NOMOREPAGES = 2
    };

struct NavNode
    {
public:
    REALITYDATAPLATFORM_EXPORT NavNode(Json::Value jsonObject, Utf8String rootNode = "", Utf8String rootId = "");
    REALITYDATAPLATFORM_EXPORT NavNode();
    REALITYDATAPLATFORM_EXPORT NavNode(Utf8String schema, Utf8String id);

    REALITYDATAPLATFORM_EXPORT Utf8String GetNavString();
    REALITYDATAPLATFORM_EXPORT Utf8String GetTypeSystem();
    REALITYDATAPLATFORM_EXPORT Utf8String GetSchemaName();
    REALITYDATAPLATFORM_EXPORT Utf8String GetClassName();
    REALITYDATAPLATFORM_EXPORT Utf8String GetInstanceId();
    REALITYDATAPLATFORM_EXPORT Utf8String GetLabel();
    REALITYDATAPLATFORM_EXPORT Utf8String GetRootNode();
    REALITYDATAPLATFORM_EXPORT Utf8String GetRootId();

private:
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

struct NodeNavigator
    {
    static NodeNavigator* s_nnInstance;

public:
    REALITYDATAPLATFORM_EXPORT static NodeNavigator& GetInstance();

    REALITYDATAPLATFORM_EXPORT bvector<NavNode> GetRootNodes(Utf8String serverName, Utf8String repoId);
    REALITYDATAPLATFORM_EXPORT bvector<NavNode> GetRootNodes(WSGServer server, Utf8String repoId);
    REALITYDATAPLATFORM_EXPORT bvector<NavNode> GetChildNodes(WSGServer server, Utf8String repoId, Utf8String nodePath);
    REALITYDATAPLATFORM_EXPORT bvector<NavNode> GetChildNodes(WSGServer server, Utf8String repoId, NavNode& parentNode);

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
//! To advance to next/previous page simply call AdvancePage(), RewingPage() or
//! GoToPage()
//=====================================================================================
struct WSGPagedRequest : public WSGURL
    {
public:
    
    REALITYDATAPLATFORM_EXPORT WSGPagedRequest() : m_startIndex(0), m_pageSize(25) {}
    REALITYDATAPLATFORM_EXPORT WSGPagedRequest(uint32_t startIndex, uint16_t pageSize) : m_startIndex(startIndex), m_pageSize(pageSize) {BeAssert(m_pageSize >0);}

    REALITYDATAPLATFORM_EXPORT ~WSGPagedRequest() {}
    REALITYDATAPLATFORM_EXPORT WSGPagedRequest(WSGPagedRequest const& object);
    REALITYDATAPLATFORM_EXPORT WSGPagedRequest& operator=(WSGPagedRequest const & object);

    //! Get/Set the page size. It must be greater or equal to 1 
    REALITYDATAPLATFORM_EXPORT void SetPageSize(uint16_t pageSize) {BeAssert(pageSize > 0); m_pageSize = pageSize;}
    REALITYDATAPLATFORM_EXPORT uint16_t GetPageSize() const {return m_pageSize;}
  
    //! The start index of the request.
    REALITYDATAPLATFORM_EXPORT void SetStartIndex(uint32_t startIndex) {m_startIndex = startIndex;} 
    REALITYDATAPLATFORM_EXPORT uint32_t GetStartIndex() const {return m_startIndex;}

    //! Page oriented methods. These methods advance a page, rewind a page or go to specified page.
    //! according to current page size and start index
    REALITYDATAPLATFORM_EXPORT virtual void AdvancePage() {m_validRequestString = false; m_startIndex += m_pageSize;}
    REALITYDATAPLATFORM_EXPORT void RewindPage() {m_validRequestString = false; m_startIndex = (m_startIndex <= m_pageSize ? 0 : m_startIndex-m_pageSize);}
    REALITYDATAPLATFORM_EXPORT void GoToPage(int page) {m_validRequestString = false; m_startIndex = (uint32_t)(page * m_pageSize);}

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override = 0; //virtual class, not to be used directly

    mutable uint32_t m_startIndex;
    uint16_t m_pageSize;
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
    REALITYDATAPLATFORM_EXPORT bvector<Utf8String> GetPlugins() const;

    //! Returns the version of WSG understood by the WSG server
    REALITYDATAPLATFORM_EXPORT Utf8String GetVersion() const;

    //! Returns the list of repositories
    REALITYDATAPLATFORM_EXPORT bvector<Utf8String> GetRepositories() const;

    //! Returns the list of schemas published by the plugin
    REALITYDATAPLATFORM_EXPORT bvector<Utf8String> GetSchemaNames(Utf8String repoName) const;

    //! Returns the list of classes exposed by the indicated schema for the plugin
    REALITYDATAPLATFORM_EXPORT bvector<Utf8String> GetClassNames(Utf8String repoId, Utf8String schemaName);

    //! Returns the JSON fragment applying to the class definition of the requested class
    REALITYDATAPLATFORM_EXPORT Utf8String GetJSONClassDefinition(Utf8String repoName, Utf8String schemaName, Utf8String className);

    REALITYDATAPLATFORM_EXPORT Utf8String GetServerName() { return m_serverName; }

private:
    Utf8String m_serverName;
    bool m_verifyPeer;
    mutable Utf8String m_version;
    };

struct CurlConstructor
    {
public:
    REALITYDATAPLATFORM_EXPORT Utf8String GetToken(); 
    REALITYDATAPLATFORM_EXPORT void RefreshToken();
    REALITYDATAPLATFORM_EXPORT BeFileName GetCertificatePath() { return m_certificatePath; }
    REALITYDATAPLATFORM_EXPORT void SetCertificatePath(Utf8String certificatePath) { m_certificatePath = BeFileName(certificatePath); }

    REALITYDATAPLATFORM_EXPORT CurlConstructor();
protected:
    REALITYDATAPLATFORM_EXPORT CURL* PrepareCurl(const WSGURL& wsgRequest, int& code, bool verifyPeer, FILE* file) const;

    Utf8String          m_token;
    BeFileName          m_certificatePath;
    time_t              m_tokenRefreshTimer;
    };

struct WSGRequest : public CurlConstructor
    {
private:
    static WSGRequest* s_instance;

    Utf8String _PerformRequest(const WSGURL& wsgRequest, int& code, bool verifyPeer, FILE* file, bool retry) const;
public:
    REALITYDATAPLATFORM_EXPORT static WSGRequest& GetInstance();
    WSGRequest();

    //! General method. Performs a WSG request and returns de result code in result and
    //! the body in the returned string. If a FILE is provided, the result will be written to a file
    REALITYDATAPLATFORM_EXPORT Utf8String PerformRequest(const WSGURL& wsgRequest, int& result, bool verifyPeer = true, FILE* file = nullptr, bool retry = true) const;
    REALITYDATAPLATFORM_EXPORT Utf8String PerformHeaderRequest(const WSGURL& wsgRequest, int& result, bool verifyPeer = true, FILE* file = nullptr, bool retry = true) const;
    REALITYDATAPLATFORM_EXPORT Utf8String PerformAzureRequest(const WSGURL& wsgRequest, int& result, bool verifyPeer = true, FILE* file = nullptr, bool retry = true) const;
    REALITYDATAPLATFORM_EXPORT CURL* PrepareRequest(const WSGURL& wsgRequest, int& result, Utf8StringP returnString, bool verifyPeer = true, FILE* file = nullptr, bool retry = true) const;
    };


END_BENTLEY_REALITYPLATFORM_NAMESPACE