#pragma once
#include "DataSourceDefs.h"
#include "DataSourceService.h"
#include "DataSourceAccount.h"
#include "DataSourceBuffer.h"


#define DATA_SOURCE_SERVICE_AZURE_DEFAULT_SEGMENT_SIZE  (64 * 1024)


class DataSourceServiceAzure : public DataSourceService
{

public:

    typedef DataSourceService   Super;

protected:

        DataSourceBuffer::BufferSize    defaultSegmentSize;

public:

                                        DataSourceServiceAzure          (DataSourceManager &manager, const ServiceName &service);

        DataSourceAccount *             createAccount                   (const AccountName & account, const DataSourceAccount::AccountIdentifier identifier, const DataSourceAccount::AccountKey &key);
        DataSourceStatus                destroyAccount                  (const AccountName & account);

        void                            setDefaultSegmentSize           (DataSourceBuffer::BufferSize size);
        DataSourceBuffer::BufferSize    getDefaultSegmentSize           (void);
};

