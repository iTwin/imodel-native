/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "DataSourceDefs.h"
#include "DataSourceService.h"
#include "DataSourceAccount.h"
#include "DataSourceBuffer.h"


#define DATA_SOURCE_SERVICE_AZURE_DEFAULT_SEGMENT_SIZE  (32 * 1024)

#ifndef NDEBUG
#define DATA_SOURCE_SERVICE_AZURE_DEFAULT_TIMEOUT       (6000 * 1000)
#else
#define DATA_SOURCE_SERVICE_AZURE_DEFAULT_TIMEOUT       (60 * 1000)
#endif

class DataSourceServiceAzure : public DataSourceService
{

public:

    typedef DataSourceService   Super;

protected:

        DataSourceBuffer::BufferSize    defaultSegmentSize;
        DataSourceBuffer::Timeout       defaultTimeout;

public:

                                        DataSourceServiceAzure          (DataSourceManager &manager, const ServiceName &service);

        DataSourceAccount *             createAccount                   (const AccountName & account, const DataSourceAccount::AccountIdentifier identifier, const DataSourceAccount::AccountKey &key);
        DataSourceStatus                destroyAccount                  (const AccountName & account);

        void                            setDefaultSegmentSize           (DataSourceBuffer::BufferSize size);
        DataSourceBuffer::BufferSize    getDefaultSegmentSize           (void);
    
        void                            setDefaultTimeout               (DataSourceBuffer::Timeout time);
        DataSourceBuffer::Timeout       getDefaultTimeout               (void);
};

class DataSourceServiceAzureCURL : public DataSourceServiceAzure
    {

    public:

        typedef DataSourceServiceAzure   Super;

    public:

                                        DataSourceServiceAzureCURL          (DataSourceManager &manager, const ServiceName &service);

        DataSourceAccount *             createAccount                   (const AccountName & account, const DataSourceAccount::AccountIdentifier identifier, const DataSourceAccount::AccountKey &key);
    };
