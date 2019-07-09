#pragma once
#include "DataSourceDefs.h"
#include "DataSourceService.h"
#include "DataSourceAccount.h"
#include "DataSourceBuffer.h"


#define DATA_SOURCE_SERVICE_CURL_DEFAULT_SEGMENT_SIZE  (32 * 1024)

#ifndef NDEBUG
#define DATA_SOURCE_SERVICE_CURL_DEFAULT_TIMEOUT       (60 * 1000)
#else
#define DATA_SOURCE_SERVICE_CURL_DEFAULT_TIMEOUT       (60 * 1000)
#endif

class DataSourceServiceCURL : public DataSourceService
{

public:

    typedef DataSourceService   Super;

protected:

        DataSourceBuffer::BufferSize    defaultSegmentSize;
        DataSourceBuffer::Timeout       defaultTimeout;

public:

                                        DataSourceServiceCURL           (DataSourceManager &manager, const ServiceName &service);

        DataSourceAccount *             createAccount                   (const AccountName & account, const DataSourceAccount::AccountIdentifier identifier, const DataSourceAccount::AccountKey &key);
        DataSourceStatus                destroyAccount                  (const AccountName & account);

        void                            setDefaultSegmentSize           (DataSourceBuffer::BufferSize size);
        DataSourceBuffer::BufferSize    getDefaultSegmentSize           (void);
    
        void                            setDefaultTimeout               (DataSourceBuffer::Timeout time);
        DataSourceBuffer::Timeout       getDefaultTimeout               (void);
};

