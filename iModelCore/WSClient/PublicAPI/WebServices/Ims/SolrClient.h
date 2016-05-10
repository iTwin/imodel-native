/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Ims/SolrClient.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Ims/SolrQuery.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

//--------------------------------------------------------------------------------------+
// WebServices Client API for connecting to Ims Search API data source.
//--------------------------------------------------------------------------------------+

typedef std::shared_ptr<struct ISolrClient>     ISolrClientPtr;

typedef AsyncResult<Json::Value, HttpError>    SolrGetResult;


/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct ISolrClient
    {
    public:
        WSCLIENT_EXPORT virtual ~ISolrClient();

        virtual Utf8StringCR GetBaseUrl() const = 0;

        virtual Utf8StringCR GetCollectionPath() const = 0;

        //! Send query request using a SolrQuery. Send an HttpRequest to the Ims Search API Url.
        //! @param[in] query
        virtual AsyncTaskPtr<SolrGetResult> SendGetRequest(SolrQueryCR query = SolrQuery()) const = 0;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct SolrClient : public ISolrClient
    {
    private:
        const Utf8String m_serverUrl;
        Utf8String m_collectionPath;
        std::shared_ptr<HttpClient> m_httpClient;

    private:
        SolrClient(Utf8String serverUrl, Utf8String solrPath, IHttpHeaderProviderPtr defaultHeadersProvider, IHttpHandlerPtr customHandler = nullptr);

    public:
        //! @param[in] serverUrl Address to supported server/site (i.e. qa-waz-search.bentley.com/token)
        //! @param[in] collectionPath The path to the Solr Service (i.e. IMS/User/, GPR/ProductRegistry/, Notification/)
        //! @param[in] clientInfo Client infomation for licensing and other information
        //! @param[in] customHandler [Optional] custom http handler
        WSCLIENT_EXPORT static std::shared_ptr<SolrClient> Create
            (
            Utf8StringCR serverUrl,
            Utf8String collectionPath,
            ClientInfoPtr clientInfo,
            IHttpHandlerPtr customHandler = nullptr
            );

        //! Accessor function for Solr API base url
        //! @return baseUrl Address to supported server/site (i.e. qa-waz-search.bentley.com/token)
        WSCLIENT_EXPORT Utf8StringCR GetBaseUrl() const override;

        //! Accessor function for Solr API Collection Path
        //! @return collectionPath - address to supported service (i.e. IMS/User/, GPR/ProductRegistry/, Notification/)
        WSCLIENT_EXPORT Utf8StringCR GetCollectionPath() const override;

        //! Send query request using a SolrQuery. Send an HttpRequest to the Solr REST Url.
        //! @param[in] [Optional] The SolrQuery used to construct the get request url
        WSCLIENT_EXPORT AsyncTaskPtr<SolrGetResult> SendGetRequest(SolrQueryCR query = SolrQuery()) const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE