#include "stdafx.h"
#include "DataSourceWSG.h"
#include "DataSourceAccountWSG.h"
#include <assert.h>

DataSourceWSG::DataSourceWSG(DataSourceAccount * sourceAccount) : DataSourceCloud(sourceAccount)
{
    setBuffer(nullptr);
}

DataSourceWSG::~DataSourceWSG(void)
{

}

DataSourceStatus DataSourceWSG::open(const DataSourceURL & sourceURL, DataSourceMode sourceMode)
{
    DataSourceURL    url = sourceURL;

    url.normalize();

    return Super::open(url, sourceMode);
}


DataSourceStatus DataSourceWSG::read(Buffer *dest, DataSize destSize, DataSize &readSize, DataSize size)
{
                                                            // Pass to superclass for read (Note: Real reads are down through download calls)
    DataSourceStatus status = Super::read(dest, destSize, readSize, size);

    assert(readSize > 0);

    return status;
}

DataSourceStatus DataSourceWSG::close(void)
{
    return Super::close();
}

DataSourceAccountWSG * DataSourceWSG::getAccountWSG(void)
{
    return dynamic_cast<DataSourceAccountWSG *>(getAccount());
}

