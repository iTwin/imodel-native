/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "stdafx.h"
#include "DataSourceAzure.h"
#include "DataSourceAccountAzure.h"
#include <assert.h>

DataSourceAzure::DataSourceAzure(DataSourceAccount * sourceAccount, const SessionName &session) : DataSourceCloud(sourceAccount, session)
{
    setBuffer(nullptr);
}

DataSourceAzure::~DataSourceAzure(void)
{

}

DataSourceStatus DataSourceAzure::open(const DataSourceURL & sourceURL, DataSourceMode sourceMode)
{
    DataSourceURL    url = sourceURL;

    url.normalize();

    return Super::open(url, sourceMode);
}


DataSourceStatus DataSourceAzure::read(Buffer *dest, DataSize destSize, DataSize &readSize, DataSize size)
{
                                                            // Pass to superclass for read (Note: Real reads are down through download calls)
    DataSourceStatus status = Super::read(dest, destSize, readSize, size);

    assert(readSize > 0);

    return status;
}

DataSourceStatus DataSourceAzure::read(std::vector<Buffer>& dest)
    {
    // Pass to superclass for read (Note: Real reads are down through download calls)
    return Super::read(dest);
    }

DataSourceStatus DataSourceAzure::close(void)
{
    return Super::close();
}

DataSourceAccountAzure * DataSourceAzure::getAccountAzure(void)
{
    return dynamic_cast<DataSourceAccountAzure *>(getAccount());
}

