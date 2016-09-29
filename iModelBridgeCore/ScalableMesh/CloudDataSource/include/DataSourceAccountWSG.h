#pragma once
#include "DataSourceDefs.h"
#include <string>
#include <map>
#include <curl/curl.h>

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

    WSGServer::Request::protocol            wsgProtocol         = L"https:";
    WSGServer::Request::port                wsgPort             = L"443";
    WSGServer::version                      wsgVersion          = L"v2.4";
    WSGServer::apiID                        wsgAPIID            = L"Repositories";
    WSGServer::repository                   wsgRepository       = L"S3MXECPlugin--Server";
    WSGServer::schema                       wsgSchema           = L"S3MX";
    WSGServer::class_name                   wsgClassName        = L"Document";
    //WSGServer::organizationID               wsgOrganizationID   = L"5e41126f-6875-400f-9f75-4492c99ee544"; // Dev Bentley org id
    WSGServer::organizationID               wsgOrganizationID = L"e82a584b-9fae-409f-9581-fd154f7b9ef9"; // Connect Bentley org id
    

public:
                                            DataSourceAccountWSG                (void) = delete;
                                            DataSourceAccountWSG                (const AccountName &account, const AccountIdentifier &identifier, const AccountKey &key);
        virtual                            ~DataSourceAccountWSG                (void);

        DataSourceStatus                    setAccount                          (const AccountName &account, const AccountIdentifier & identifier, const AccountKey & key);

        virtual void                        setPrefixPath                       (const DataSourceURL &prefix) override;

        DataSource                   *      createDataSource                    (void);
        DataSourceStatus                    destroyDataSource                   (DataSource *dataSource);

        DataSourceStatus                    downloadBlobSync                    (DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize &readSize);
        DataSourceStatus                    downloadBlobSync                    (DataSourceURL &blobPath, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size);
        DataSourceStatus                    uploadBlobSync                      (DataSourceURL & url, const std::wstring &filename, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size);
        DataSourceStatus                    uploadBlobSync                      (DataSource & dataSource, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size);
        DataSourceStatus                    uploadBlobSync                      (const DataSourceURL &blobPath, const WSGEtag &etag, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size);

        virtual void                        setWSGTokenGetterCallback           (const std::function<std::string (void)>& tokenUpdater);

private :
       std::function<std::string (void)>    m_getWSGToken;
       bool                                 m_isValid = true;

       WSGToken                             getWSGToken                         (void);
       WSGEtag                              getWSGHandshake                     (const DataSourceURL &url, const DataSourceURL &filename, DataSourceBuffer::BufferSize size);

       bool                                 isValid                             (void);
    };
