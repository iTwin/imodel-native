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
    WSGURL(Utf8String url);

    //! Default constructor. All strings must be set. The default protocol is 'Repositories'
    WSGURL();

    //! Creates a WSG URL from the various provided components
    WSGURL(Utf8String server, Utf8String protocol, Utf8String pluginName, Utf8String schema, Utf8String _interface, Utf8String className, Utf8String id, bool objectContent);

    //! Get/Set the server name. The string provided or returned only contains
    //! the server name without trailling or leading slashes and without the communication protocol
    //! for example the server name could be 'dev-contextservices.connect.com'
    //! SetServer() will return an error is the server string provided is invalid
    Utf8StringCR GetServerName() const;

    //! A string containing the protocol. The protocol must of the format number dot number such as '2.4'
    //! otherwise an error will be returned
    //! Note that the method does not validate the support of the specified protocol.
    Utf8StringCR GetProtocol() const;

    
    //! A string containing the plug-in name. A WSG server can host many plug-ins so specification is required.
    Utf8StringCR GetPluginName() const;

    //! The Schema. A plugin can publish many schemas so specification is required.
    //! Note that the method does not validate the publication of the specified schema.
    Utf8StringCR GetSchema() const;

    //! The interface. WSG plugins can implement one on many interfaces. We currently only support two
    //! The Repository interface enables accessing objects related to a schema while the NavNode interface
    //! lists an exposed tree-like structure.
    //! Note that the method does not validate the implementation of the specified interface.
    WSGInterface GetInterface() const;

    //! The class name. 
    //! Note that the method does not validate the class is part of the schema.
    Utf8StringCR GetClassName() const;

    //! The id of the object requested. If the request does not require
    //! an identifier then this field should remain empty
    Utf8StringCR GetId() const;
    StatusInt SetId(Utf8String id);

    //! The object content flag indicates we want or not the content of the file associated with designated object.
    //! The default is false.
    bool GetContentFlag() const

    enum class HttpRequestType
        {
        GET_Request,
        PUT_Request,
        POST_Request
        }

    //! Returns the full http request string
    Utf8StringCR GetHttpRequestString() const 
        {
        if (!m_validRequestString)
            _PrepareHttpRequestString();

        BeAssert(m_validRequestString);
        BeAssert(m_httpRequestString.size() != 0);

        return m_httpRequestString;
        };

    Utf8StringCR GetRequestPayload() const
        {
        if (!m_validRequestString)
            _PrepareHttpRequestString();

        BeAssert(m_validRequestString);

        return m_requestPayload;
        }

    bmap<Utf8String, Utf8String> const & GetRequestHeader() const
        {
        if (!m_validRequestString)
            _PrepareHttpRequestString();

        BeAssert(m_validRequestString);

        return m_requestHeader;
        }

protected:
    // Default constructor
    WSGURL() : m_validRequestString(false), m_requestType(GET_Request){}
    ~WSGURL() {}
    WSGURL(WSGURL const& object);
    WSGURL& operator=(WSGURL const & object);


    StatusInt SetServerName(Utf8String serverName);
    StatusInt SetPluginName(Utf8String pluginName);
    StatusInt SetProtocol(Utf8String protocol);
    StatusInt SetClassName(Utf8String className);
    StatusInt SetSchema(Utf8String schema);
    StatusInt SetInterface(WSGInterface _interface);


    virtual bool _PrepareHttpRequestStringAndPayload() const;

    Utf8String m_serverName;
    Utf8String m_protocol;
    Utf8String m_pluginName;
    Utf8String m_schema;
    Utf8String m_className;
    WSGInterface m_interface;
    Utf8String m_id;
    bool m_objectContent = false;


    mutable bool m_validRequestString;
    mutable Utf8String m_httpRequestString;
    mutable Utf8String m_requestPayload;
    mutable bmap<Utf8String, Utf8String> m_requestHeader; 
    mutable HttpRequestType m_requestType;
    }


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
    WSGNavNodeRequest(Utf8String server, Utf8String protocol, Utf8String pluginName, Utf8String schema, Utf8String nodeId);

    
    }

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! WSGObjectRequest
//! This class represents a URL to a WSG request for a single designated object.

//=====================================================================================
struct WSGObjectRequest: public WSGURL
    {
public:
    WSGObjectRequest(Utf8String server, Utf8String protocol, Utf8String pluginName, Utf8String schema, Utf8String className, Utf8String objectId);

    
    }

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! WSGObjectContentRequest
//! This class represents a URL to a WSG request for a single designated object.

//=====================================================================================
struct WSGObjectContentRequest: public WSGURL
    {
public:
    WSGObjectContentRequest(Utf8String server, Utf8String protocol, Utf8String pluginName, Utf8String schema, Utf8String className, Utf8String objectId);
   
    }

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
struct WSGPagedRequest : public WSGUrl
    {
public:
    REALITYDATAPLATFORM_EXPORT WSGPagedRequest() : m_startIndex(0), m_pageSize(25) {}
    REALITYDATAPLATFORM_EXPORT WSGPagedRequest(uint16_t startIndex, uint8_t pageSize) : m_startIndex(startIndex), m_pageSize(pageSize) {BeAssert(m_pageSize >0);}

    REALITYDATAPLATFORM_EXPORT ~WSGPagedRequest() {}
    REALITYDATAPLATFORM_EXPORT WSGPagedRequest(WSGPagedRequest const& object);
    REALITYDATAPLATFORM_EXPORT WSGPagedRequest& operator=(WSGPagedRequest const & object);


    REALITYDATAPLATFORM_EXPORT StatusInt SetPageSize(uint8_t pageSize) {BeAssert(pageSize > 0); m_pageSize = pageSize;}
    REALITYDATAPLATFORM_EXPORT uint8_t GetPageSize() {return m_pageSize;}
  
    REALITYDATAPLATFORM_EXPORT StatusInt SetStartIndex(uint16_t startIndex) {m_startIndex = startIndex;} 

    REALITYDATAPLATFORM_EXPORT StatusInt AdvancePage() {m_validRequestString = false; m_startIndex += m_pageSize;}
    REALITYDATAPLATFORM_EXPORT StatusInt RewindPage() {m_validRequestString = false; m_startIndex = (m_startIndex <= m_pageSize ? 0 : m_startIndex-m_pageSize);}
    REALITYDATAPLATFORM_EXPORT StatusInt GoToPage(int page) {m+_validRequestString = false; m_startIndex = (page * m_pageSize);}

protected:
    // Default constructor

    uint16_t m_startIndex;
    uint8_t m_pageSize;

    }

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
    WSGObjectListPagedRequest(Utf8String server, Utf8String protocol, Utf8String pluginName, Utf8String schema, Utf8String className);

    
    }

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
    
    WSGServer();
    
    //! Returns a list plugins installed to the indicated WSG server
    bvector<Utf8String> GetPlugins(Utf8String serverName) const;

    //! Returns the version of WSG understood by the WSG server
    Utf8String GetVersion(Utf8String serverName) const;

    //! Returns the list of schemas published by the plugin
    bvector<Utf8String> GetSchemaNames(Utf8String serverName, Utf8String pluginName) const;

    //! Returns the list of classes exposed by the indicated schema for the plugin
    bvector<Utf8String> GetClassNames(Utf8String pluginName, Utf8String schemaName);

    //! Returns the JSON fragment applying to the class definition of the requested class
    Utf8String GetJSONClassDefinition(Utf8String pluginName, Utf8String schemaName, Utf8String className);

    //! General method. Performs a WSG request and returns de result code in result and
    //! the body in the returned string.
    Utf8String PerformRequest(WSGURL& wsgRequest, int& result);

    }



END_BENTLEY_REALITYPLATFORM_NAMESPACE