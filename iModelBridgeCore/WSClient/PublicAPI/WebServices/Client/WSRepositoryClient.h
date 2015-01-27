/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Client/WSRepositoryClient.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>

#include <MobileDgn/Utils/Threading/AsyncResult.h>
#include <WebServices/Client/ObjectId.h>
#include <WebServices/Client/Response/WSCreateObjectResponse.h>
#include <WebServices/Client/Response/WSFileResponse.h>
#include <WebServices/Client/Response/WSObjectsResponse.h>
#include <WebServices/Client/WSClient.h>
#include <WebServices/Client/WSError.h>
#include <WebServices/Client/WSQuery.h>

#include <Bentley/bset.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

//--------------------------------------------------------------------------------------+
// WebServices Client API for connecting to BWSG server data source. 
//--------------------------------------------------------------------------------------+

typedef std::shared_ptr<struct IWSRepositoryClient>     IWSRepositoryClientPtr;

typedef MobileDgn::Utils::AsyncResult<WSObjectsResponse, WSError>         WSObjectsResult;
typedef MobileDgn::Utils::AsyncResult<WSFileResponse, WSError>            WSFileResult;
typedef MobileDgn::Utils::AsyncResult<WSCreateObjectResponse, WSError>    WSCreateObjectResult;
typedef MobileDgn::Utils::AsyncResult<void, WSError>                      WSUpdateObjectResult;
typedef MobileDgn::Utils::AsyncResult<void, WSError>                      WSDeleteObjectResult;
typedef MobileDgn::Utils::AsyncResult<void, WSError>                      WSUpdateFileResult;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct IWSRepositoryClient
    {
    public:
        WSCLIENT_EXPORT virtual ~IWSRepositoryClient ();

        virtual IWSClientPtr GetWSClient () const = 0;
        virtual Utf8StringCR GetRepositoryId () const = 0;

        virtual void SetCredentials (MobileDgn::Utils::Credentials credentials) = 0;

        //! Checks if supplied credentials are valid for this repository.
        //! @param[in] cancellationToken
        //! @return success if credentials are valid for given repository, else error that occurred
        virtual std::shared_ptr<MobileDgn::Utils::PackagedAsyncTask<MobileDgn::Utils::AsyncResult<void, WSError>>> VerifyAccess
            (
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSObjectsResult> SendGetObjectRequest
            (
            ObjectIdCR objectId,
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequest
            (
            ObjectIdCR parentObjectId,
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequest
            (
            ObjectIdCR parentObjectId,
            const bset<Utf8String>& propertiesToSelect,
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSFileResult> SendGetFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::HttpRequest::ProgressCallbackCR downloadProgressCallback = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSObjectsResult> SendGetSchemasRequest
            (
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSObjectsResult> SendQueryRequest
            (
            WSQueryCR query,
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        //! Create object with any relationships or related objects. Optionally attach file.
        //! Parameter objectCreationJson must follow WSG 2.0 format for creating objects.
        //! NOTES for different server versions:
        //!     WSG 2.0: format is fully supported.
        //!     WSG 1.x: objectCreationJson can have only one relationship to existing object. This related object will be treated as "parent".
        //!     Server version can be checked by using GetWSClient()->GetServerInfo()
        virtual MobileDgn::Utils::AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName (),
            MobileDgn::Utils::HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSUpdateObjectResult> SendUpdateObjectRequest
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8String eTag = nullptr,
            MobileDgn::Utils::HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSDeleteObjectResult> SendDeleteObjectRequest
            (
            ObjectIdCR objectId,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSUpdateFileResult> SendUpdateFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            MobileDgn::Utils::HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSRepositoryClient : public IWSRepositoryClient
    {
    private:
        std::shared_ptr<struct ClientConnection> m_connection;
        IWSClientPtr m_serverClient;

    private:
        WSRepositoryClient (std::shared_ptr<struct ClientConnection> connection);

    public:
        struct Timeout
            {
            struct Connection
                {
                static const uint32_t Default;
                };
            struct Transfer
                {
                static const uint32_t GetObject;
                static const uint32_t GetObjects;
                static const uint32_t FileDownload;
                static const uint32_t Upload;
                };
            };

        //! @param[in] serverUrl - address to supported server/site
        //! @param[in] repositoryId - repository identifier
        //! @param[in] defaultHeaders - headers used for each request. User-Agent is recomended for being able to identify client in server.
        //!                             Mas-Uuid and Mas-App-Guid are posible for licensing purposes.
        //! @param[in] customHandler - custom http handler for testing purposes.
        //! @param[in] defaultSchemaPath - schema that should be used for server with WebApi v1.x. Will override server schema if added. 
        //! Note: schema is mandatory for server with WebApi v1.1. Use WSClient::GetServerInfo() to figure out version if needed.
        WSCLIENT_EXPORT static IWSRepositoryClientPtr Create
            (
            Utf8StringCR serverUrl,
            Utf8StringCR repositoryId,
            MobileDgn::Utils::HttpRequestHeadersCR defaultHeaders,
            BeFileNameCP defaultSchemaPath = nullptr,
            MobileDgn::Utils::IHttpHandlerPtr customHandler = nullptr
            );

        WSCLIENT_EXPORT IWSClientPtr GetWSClient () const override;
        WSCLIENT_EXPORT Utf8StringCR GetRepositoryId () const override;

        WSCLIENT_EXPORT void SetCredentials (MobileDgn::Utils::Credentials credentials);

        //! Check if user can access repository
        WSCLIENT_EXPORT std::shared_ptr<MobileDgn::Utils::PackagedAsyncTask<MobileDgn::Utils::AsyncResult<void, WSError>>> VerifyAccess
            (
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        WSCLIENT_EXPORT MobileDgn::Utils::AsyncTaskPtr<WSObjectsResult> SendGetObjectRequest
            (
            ObjectIdCR objectId,
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        WSCLIENT_EXPORT MobileDgn::Utils::AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequest
            (
            ObjectIdCR parentObjectId,
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        WSCLIENT_EXPORT MobileDgn::Utils::AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequest
            (
            ObjectIdCR parentObjectId,
            const bset<Utf8String>& propertiesToSelect,
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        WSCLIENT_EXPORT MobileDgn::Utils::AsyncTaskPtr<WSFileResult> SendGetFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::HttpRequest::ProgressCallbackCR downloadProgressCallback = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        WSCLIENT_EXPORT MobileDgn::Utils::AsyncTaskPtr<WSObjectsResult> SendGetSchemasRequest
            (
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        WSCLIENT_EXPORT MobileDgn::Utils::AsyncTaskPtr<WSObjectsResult> SendQueryRequest
            (
            WSQueryCR query,
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        WSCLIENT_EXPORT MobileDgn::Utils::AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName (),
            MobileDgn::Utils::HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        WSCLIENT_EXPORT MobileDgn::Utils::AsyncTaskPtr<WSUpdateObjectResult> SendUpdateObjectRequest
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8String eTag = nullptr,
            MobileDgn::Utils::HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        WSCLIENT_EXPORT MobileDgn::Utils::AsyncTaskPtr<WSDeleteObjectResult> SendDeleteObjectRequest
            (
            ObjectIdCR objectId,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        WSCLIENT_EXPORT MobileDgn::Utils::AsyncTaskPtr<WSUpdateFileResult> SendUpdateFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            MobileDgn::Utils::HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
