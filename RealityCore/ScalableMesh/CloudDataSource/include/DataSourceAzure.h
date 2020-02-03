/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "DataSourceDefs.h"
#include "DataSourceCloud.h"
#include "DataSourceStatus.h"
#include "DataSourceBuffer.h"

#ifdef USE_WASTORAGE
#include <was/storage_account.h>
#include <was/blob.h>
#endif
class DataSourceAccountAzure;



class DataSourceAzure : public DataSourceCloud
{

protected:

    typedef DataSourceCloud                             Super;

#ifdef USE_WASTORAGE
    typedef azure::storage::cloud_blob                  Blob;
    typedef azure::storage::cloud_block_blob            BlockBlob;
    typedef azure::storage::cloud_page_blob             PageBlob;
#endif

protected:

    DataSourceAccountAzure *    getAccountAzure         (void);

    DataSourceStatus            flush                   (void);


public:

                                DataSourceAzure         (DataSourceAccount *sourceAccount, const SessionName &session);
                               ~DataSourceAzure         (void);

    DataSourceStatus            open                    (const DataSourceURL &sourceURL, DataSourceMode mode);
    DataSourceStatus            read                    (Buffer *dest, DataSize destSize, DataSize &readSize, DataSize size);
    DataSourceStatus            read                    (std::vector<Buffer>& dest);
    DataSourceStatus            close                   (void);
};

