/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "DataSourceDefs.h"
#include <string>
#include <map>
#include <curl/curl.h>
#include <json/json.h>


#include "DataSource.h"
#include "DataSourceAccountCURL.h"
#include "DataSourceBuffer.h"
#include "DataSourceMode.h"
#include "Manager.h"

namespace WSGServer
    {
    namespace Request
        {
        typedef   std::wstring     protocol;
        typedef   std::wstring     server;
        typedef   std::wstring     port;
        }
    typedef   std::string      token;
    typedef   std::wstring     version;
    typedef   std::wstring     apiID;
    typedef   std::wstring     repository;
    typedef   std::wstring     schema;
    typedef   std::wstring     class_name;
    typedef   std::wstring     instanceIDPrefix;
    typedef   std::wstring     organizationID;
    typedef   std::wstring     parameters;
    }

class DataSourceAccountWSG : public DataSourceAccountCURL
{

protected:

    typedef DataSourceAccountCURL           Super;
    typedef std::string                     WSGEtag;
    typedef std::string                     WSGToken;
    typedef std::string                     AzureDirectPrefix;
    typedef std::string                     AzureDirectSuffix;

    WSGServer::Request::protocol            wsgProtocol         = L"https:";
    WSGServer::Request::port                wsgPort             = L"443";
    WSGServer::version                      wsgVersion          = L"v2.4";
    WSGServer::apiID                        wsgAPIID            = L"Repositories";
    WSGServer::repository                   wsgRepository       = L"S3MXECPlugin--Server";
    WSGServer::schema                       wsgSchema           = L"S3MX";
    WSGServer::class_name                   wsgClassName        = L"Document";
    WSGServer::organizationID               wsgOrganizationID; // Obtained by making a first call to RDS


public:
                                            DataSourceAccountWSG                (void) = delete;
                                            DataSourceAccountWSG                (const AccountName &account, const AccountIdentifier &identifier, const AccountKey &key);
        virtual                            ~DataSourceAccountWSG                (void);

        DataSourceStatus                    setAccount                          (const AccountName &account, const AccountIdentifier & identifier, const AccountKey & key) override;

        virtual void                        setPrefixPath                       (const DataSourceURL &prefix) override;

        DataSource                   *      createDataSource                    (const SessionName &session) override;
        DataSourceStatus                    destroyDataSource                   (DataSource *dataSource) override;

        DataSourceStatus                    downloadBlobSync                    (DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize &readSize) override;
        DataSourceStatus                    downloadBlobSync                    (DataSourceURL &blobPath, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size, const DataSource::SessionName &session) override;
        DataSourceStatus                    uploadBlobSync                      (DataSourceURL & url, const std::wstring &filename, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size) override;
        DataSourceStatus                    uploadBlobSync                      (DataSource & dataSource, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size) override;
    
#if 0
        DataSourceStatus                    uploadBlobSync                      (const DataSourceURL &blobPath, const WSGEtag &etag, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size);
#endif

        virtual void                        setWSGTokenGetterCallback           (const std::function<std::string (void)>& tokenUpdater) override;

        CLOUD_EXPORT      void              setOrganizationID                   (const WSGServer::organizationID& orgID);
        CLOUD_EXPORT      void              setUseDirectAzureCalls(const bool& isDirect);

private :
       std::function<std::string (void)>    m_getWSGToken;
       WSGToken                             m_wsgToken;
       bool                                 m_isValid = true;
       bool                                 m_useDirectAzureCalls = true;
       AzureDirectPrefix                    m_AzureDirectPrefix;
       AzureDirectSuffix                    m_AzureDirectSuffix;

       WSGToken                             getWSGToken                         (const DataSourceURL &url);
       WSGEtag                              getWSGHandshake                     (const DataSourceURL &url, const DataSourceURL &filename, DataSourceBuffer::BufferSize size);
       bool                                 needsUpdateToken                    (const WSGToken& token);
       void                                 updateToken                         (const WSGToken& newToken, DataSourceURL url);
       WSGServer::organizationID            getOrganizationID                   (const DataSourceURL& url);

       bool                                 isValid                             (void);
    };
