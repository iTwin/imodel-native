/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/WSGServices.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__


#include <Bentley/DateTime.h>
#include <curl/curl.h>
#include <sql.h>
#include <sqlext.h>
#include <RealityPlatform/RealityPlatformAPI.h>

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
    REALITYDATAPLATFORM_EXPORT WSGURL(Utf8String server, Utf8String version, Utf8String repoId, Utf8String pluginName, Utf8String schema, WSGInterface _interface, Utf8String className, Utf8String id, bool objectContent);

    //! Get/Set the server name. The string provided or returned only contains
    //! the server name without trailling or leading slashes and without the communication version
    //! for example the server name could be 'dev-contextservices.connect.com'
    //! SetServer() will return an error is the server string provided is invalid
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetServerName() const;

    //! A string containing the version. The version must of the format number dot number such as '2.4'
    //! otherwise an error will be returned
    //! Note that the method does not validate the support of the specified version.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetVersion() const;

    //! A string containing the plug-in name. A WSG server can host many plug-ins so specification is required.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetPluginName() const;

    //! The Schema. A plugin can publish many schemas so specification is required.
    //! Note that the method does not validate the publication of the specified schema.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetSchema() const;

    //! The interface. WSG plugins can implement one on many interfaces. We currently only support two
    //! The Repository interface enables accessing objects related to a schema while the NavNode interface
    //! lists an exposed tree-like structure.
    //! Note that the method does not validate the implementation of the specified interface.
    REALITYDATAPLATFORM_EXPORT WSGInterface GetInterface() const;

    //! The class name. 
    //! Note that the method does not validate the class is part of the schema.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetClassName() const;

    //! The id of the object requested. If the request does not require
    //! an identifier then this field should remain empty
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetId() const;
    REALITYDATAPLATFORM_EXPORT void SetId(Utf8String id);

    //! The object content flag indicates we want or not the content of the file associated with designated object.
    //! The default is false.
    REALITYDATAPLATFORM_EXPORT bool GetContentFlag() const;

    enum class HttpRequestType
        {
        GET_Request,
        PUT_Request,
        POST_Request
        };

    REALITYDATAPLATFORM_EXPORT HttpRequestType GetRequestType() const;

    REALITYDATAPLATFORM_EXPORT Utf8String GetRepoId() const;

    //! Returns the full http request string
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetHttpRequestString() const
        {
        if (!m_validRequestString)
            _PrepareHttpRequestStringAndPayload();

        BeAssert(m_validRequestString);
        BeAssert(m_httpRequestString.size() != 0);

        return m_httpRequestString;
        };

    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetRequestPayload() const
        {
        if (!m_validRequestString)
            _PrepareHttpRequestStringAndPayload();

        BeAssert(m_validRequestString);

        return m_requestPayload;
        }

    REALITYDATAPLATFORM_EXPORT bvector<Utf8String> const & GetRequestHeader() const
        {
        if (!m_validRequestString)
            _PrepareHttpRequestStringAndPayload();

        BeAssert(m_validRequestString);

        return m_requestHeader;
        }

    WSGURL() : m_validRequestString(false), m_requestType(HttpRequestType::GET_Request){}
    ~WSGURL() {}
    WSGURL(WSGURL const& object);
    WSGURL& operator=(WSGURL const & object);

protected:
    // Default constructor


    void SetServerName(Utf8String serverName);
    void SetPluginName(Utf8String pluginName);
    void SetVersion(Utf8String version);
    void SetClassName(Utf8String className);
    void SetSchema(Utf8String schema);
    void SetInterface(WSGInterface _interface);
    void SetRepoId(Utf8String repoId);


    virtual bool _PrepareHttpRequestStringAndPayload() const;

    Utf8String m_serverName;
    Utf8String m_version;
    Utf8String m_pluginName;
    Utf8String m_schema;
    Utf8String m_className;
    WSGInterface m_interface;
    Utf8String m_repoId;
    Utf8String m_id;
    bool m_objectContent = false;


    mutable bool m_validRequestString;
    mutable Utf8String m_httpRequestString;
    mutable Utf8String m_requestPayload;
    mutable bvector<Utf8String> m_requestHeader; 
    mutable HttpRequestType m_requestType;
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
    REALITYDATAPLATFORM_EXPORT WSGNavNodeRequest(Utf8String server, Utf8String version, Utf8String repoId, Utf8String pluginName, Utf8String schema, Utf8String nodeId);
    };

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! WSGObjectRequest
//! This class represents a URL to a WSG request for a single designated object.
//=====================================================================================
struct WSGObjectRequest: public WSGURL
    {
public:
    REALITYDATAPLATFORM_EXPORT WSGObjectRequest(Utf8String server, Utf8String version, Utf8String pluginName, Utf8String schema, Utf8String className, Utf8String objectId);
    };

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! WSGObjectContentRequest
//! This class represents a URL to a WSG request for a single designated object.
//=====================================================================================
struct WSGObjectContentRequest: public WSGURL
    {
public:
    REALITYDATAPLATFORM_EXPORT WSGObjectContentRequest(Utf8String server, Utf8String version, Utf8String pluginName, Utf8String schema, Utf8String className, Utf8String objectId);
    };

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! WSGPagedRequest
//! This class represents a request to WSG Service that results in
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
    REALITYDATAPLATFORM_EXPORT WSGPagedRequest(uint16_t startIndex, uint8_t pageSize) : m_startIndex(startIndex), m_pageSize(pageSize) {BeAssert(m_pageSize >0);}

    REALITYDATAPLATFORM_EXPORT ~WSGPagedRequest() {}
    REALITYDATAPLATFORM_EXPORT WSGPagedRequest(WSGPagedRequest const& object);
    REALITYDATAPLATFORM_EXPORT WSGPagedRequest& operator=(WSGPagedRequest const & object);


    REALITYDATAPLATFORM_EXPORT void SetPageSize(uint8_t pageSize) {BeAssert(pageSize > 0); m_pageSize = pageSize;}
    REALITYDATAPLATFORM_EXPORT uint8_t GetPageSize() {return m_pageSize;}
  
    REALITYDATAPLATFORM_EXPORT void SetStartIndex(uint16_t startIndex) {m_startIndex = startIndex;} 

    REALITYDATAPLATFORM_EXPORT void AdvancePage() {m_validRequestString = false; m_startIndex += m_pageSize;}
    REALITYDATAPLATFORM_EXPORT void RewindPage() {m_validRequestString = false; m_startIndex = (m_startIndex <= m_pageSize ? 0 : m_startIndex-m_pageSize);}
    REALITYDATAPLATFORM_EXPORT void GoToPage(int page) {m_validRequestString = false; m_startIndex = (uint16_t)(page * m_pageSize);}

protected:
    // Default constructor

    uint16_t m_startIndex;
    uint8_t m_pageSize;
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
    
    REALITYDATAPLATFORM_EXPORT WSGServer(Utf8String serverName): m_serverName(serverName), m_version(""){}
    
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
    mutable Utf8String m_version;
    };

struct WSGRequest
    {
private:
    static WSGRequest* s_instance;
    Utf8String m_token;
    BeFileName m_certificatePath;

public:
    REALITYDATAPLATFORM_EXPORT static WSGRequest& GetInstance();
    REALITYDATAPLATFORM_EXPORT void RefreshToken();
    WSGRequest();

    //! General method. Performs a WSG request and returns de result code in result and
    //! the body in the returned string.
    REALITYDATAPLATFORM_EXPORT  Utf8String PerformRequest(const WSGURL& wsgRequest, int& result) const;
    };


END_BENTLEY_REALITYPLATFORM_NAMESPACE