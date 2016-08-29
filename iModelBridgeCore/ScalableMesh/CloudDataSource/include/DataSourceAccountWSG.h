#pragma once
#include "DataSourceDefs.h"
#include <string>

#include "DataSource.h"
#include "DataSourceAccount.h"
#include "DataSourceAccountCached.h"
#include "DataSourceBuffer.h"
#include "DataSourceMode.h"

unsigned int const DATA_SOURCE_SERVICE_WSG_DEFAULT_TRANSFER_TASKS = 16;

namespace WSGServer
    {
    namespace Request
        {
        typedef   std::wstring     protocol;
        typedef   std::wstring     server;
        typedef   std::wstring     port;
        }
    typedef   std::wstring     token;
    typedef   std::wstring     version;
    typedef   std::wstring     apiID;
    typedef   std::wstring     repository;
    typedef   std::wstring     schema;
    typedef   std::wstring     class_name;
    typedef   std::wstring     instanceIDPrefix;
    typedef   std::wstring     parameters;
    }

class DataSourceAccountWSG : public DataSourceAccountCached
{


protected:

    WSGServer::Request::protocol            wsgProtocol   = L"https:";
    WSGServer::Request::port                wsgPort       = L"443";
    WSGServer::version                      wsgVersion    = L"v2.3";
    WSGServer::apiID                        wsgAPIID      = L"Repositories";
    WSGServer::repository                   wsgRepository = L"S3MXECPlugin--Server";
    WSGServer::schema                       wsgSchema     = L"S3MX";
    WSGServer::class_name                   wsgClassName  = L"Document";

    DataSourceBuffer::BufferSize            defaultSegmentSize;
    DataSourceBuffer::Timeout               defaultTimeout;

protected:

    unsigned int                            getDefaultNumTransferTasks          (void);


public:
                                            DataSourceAccountWSG              (const AccountName &account, const AccountIdentifier &identifier, const AccountKey &key);
        virtual                            ~DataSourceAccountWSG              (void) = default;

        void                                setDefaultSegmentSize               (DataSourceBuffer::BufferSize size);
        DataSourceBuffer::BufferSize        getDefaultSegmentSize               (void);

        void                                setDefaultTimeout                   (DataSourceBuffer::Timeout time);
        DataSourceBuffer::Timeout           getDefaultTimeout                   (void);

        DataSourceStatus                    setAccount                          (const AccountName &account, const AccountIdentifier & identifier, const AccountKey & key);

        virtual void                        setPrefixPath                       (const DataSourceURL &prefix) override;

        DataSource                   *      createDataSource                    (void);
        DataSourceStatus                    destroyDataSource                   (DataSource *dataSource);

        DataSourceStatus                    downloadBlobSync                    (DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize &readSize);
        DataSourceStatus                    downloadBlobSync                    (const DataSourceURL &blobPath, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size);
        DataSourceStatus                    uploadBlobSync                      (const DataSourceURL &blobPath, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size);
};
