#include "stdafx.h"
#include "DataSourceCURL.h"
#include "DataSourceAccountCURL.h"
#include <assert.h>

DataSourceCURL::DataSourceCURL(DataSourceAccount * sourceAccount, const SessionName &session) : DataSourceCloud(sourceAccount, session)
{
    setBuffer(nullptr);
}

DataSourceCURL::~DataSourceCURL(void)
{

}

DataSourceStatus DataSourceCURL::open(const DataSourceURL & sourceURL, DataSourceMode sourceMode)
{
    DataSourceURL    url = sourceURL;

    url.normalize();

    return Super::open(url, sourceMode);
}


DataSourceStatus DataSourceCURL::read(Buffer *dest, DataSize destSize, DataSize &readSize, DataSize size)
{
                                                            // Pass to superclass for read (Note: Real reads are down through download calls)
    DataSourceStatus status = Super::read(dest, destSize, readSize, size);

    //assert(readSize > 0);

    return status;
}

DataSourceStatus DataSourceCURL::close(void)
{
    return Super::close();
}

DataSourceAccountCURL * DataSourceCURL::getAccountCURL(void)
{
    return dynamic_cast<DataSourceAccountCURL *>(getAccount());
}

