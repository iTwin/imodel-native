#pragma once

#include "DataSourceDefs.h"
#include "DataSourceCloud.h"
#include "DataSourceStatus.h"
#include "DataSourceBuffer.h"

#include <was/storage_account.h>
#include <was/blob.h>
class DataSourceAccountAzure;



class DataSourceAzure : public DataSourceCloud
{

protected:

    typedef DataSourceCloud                             Super;

    typedef azure::storage::cloud_blob                  Blob;
    typedef azure::storage::cloud_block_blob            BlockBlob;
    typedef azure::storage::cloud_page_blob             PageBlob;

protected:

    DataSourceAccountAzure *    getAccountAzure         (void);

    DataSourceStatus            flush                   (void);


public:

                                DataSourceAzure         (DataSourceAccount *sourceAccount);
                               ~DataSourceAzure         (void);

    DataSourceStatus            open                    (const DataSourceURL &sourceURL, DataSourceMode mode);
    DataSourceStatus            read                    (Buffer *dest, DataSize destSize, DataSize &readSize, DataSize size);
    DataSourceStatus            close                   (void);
};

