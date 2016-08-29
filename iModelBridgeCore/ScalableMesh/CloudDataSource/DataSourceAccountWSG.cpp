#include "stdafx.h"
#include <assert.h>
#include "DataSourceAccountWSG.h"
#include "DataSourceWSG.h"
#include <cpprest/rawptrstream.h>
#include <cpprest/producerconsumerstream.h>
#include "include\DataSourceAccountWSG.h"


DataSourceAccountWSG::DataSourceAccountWSG(const ServiceName & name, const AccountIdentifier & identifier, const AccountKey & key)
{
    setAccount(name, identifier, key);
                                                            // Default size is set by Service on creation
    setDefaultSegmentSize(0);

                                                            // Multi-threaded segmented transfers used for Azure, so initialize it
    getTransferScheduler().initializeTransferTasks(getDefaultNumTransferTasks());
}


unsigned int DataSourceAccountWSG::getDefaultNumTransferTasks(void)
{
    return DATA_SOURCE_SERVICE_WSG_DEFAULT_TRANSFER_TASKS;
}


DataSource * DataSourceAccountWSG::createDataSource(void)
{
                                                            // NOTE: This method is for internal use only, don't call this directly.
    DataSourceWSG *   dataSourceWSG;
                                                            // Create a new DataSourceAzure
    dataSourceWSG = new DataSourceWSG(this);
    if (dataSourceWSG == nullptr)
        return nullptr;
                                                            // Set the timeout from the account's default (which comes from the Service's default)
    dataSourceWSG->setTimeout(this->getDefaultTimeout());
                                                            // Set the segment size from the account's default (which comes from the Service's default)
    dataSourceWSG->setSegmentSize(this->getDefaultSegmentSize());

    return dataSourceWSG;
}


DataSourceStatus DataSourceAccountWSG::destroyDataSource(DataSource *dataSource)
{
    if (dataSource)
    {
        delete dataSource;

        return DataSourceStatus();
    }

    return DataSourceStatus(DataSourceStatus::Status_Error);
}


void DataSourceAccountWSG::setDefaultSegmentSize(DataSourceBuffer::BufferSize size)
{
    defaultSegmentSize = size;
}

DataSourceBuffer::BufferSize DataSourceAccountWSG::getDefaultSegmentSize(void)
{
    return defaultSegmentSize;
}

void DataSourceAccountWSG::setDefaultTimeout(DataSourceBuffer::Timeout time)
    {
    defaultTimeout = time;
    }

DataSourceBuffer::Timeout DataSourceAccountWSG::getDefaultTimeout(void)
    {
    return defaultTimeout;
    }

DataSourceStatus DataSourceAccountWSG::setAccount(const AccountName & account, const AccountIdentifier & identifier, const AccountKey & key)
{
    if (account.length() == 0 )
        return DataSourceStatus(DataSourceStatus::Status_Error_Bad_Parameters);
                                                            // Set details in base class
    DataSourceAccount::setAccount(ServiceName(L"DataSourceServiceWSG"), account, identifier, key);

    return DataSourceStatus();
}

void DataSourceAccountWSG::setPrefixPath(const DataSourceURL &prefix)
    {
    DataSourceURL url(this->wsgProtocol + L"//");
    url.append(this->getAccountIdentifier());
    if (!this->wsgPort.empty())
        {
                                                            // using the += operator prevents appending an unwanted / separator
        url += (L":" + this->wsgPort);
        }
    url.append(this->wsgVersion);
    url.append(this->wsgAPIID);
    url.append(this->wsgRepository);
    url.append(this->wsgSchema);
    url.append(this->wsgClassName);
    url.append(prefix);

    DataSourceAccount::setPrefixPath(url);
    }


DataSourceStatus DataSourceAccountWSG::downloadBlobSync(DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize & readSize)
{
    DataSourceURL    url;

    dataSource.getURL(url);

    return downloadBlobSync(url, dest, readSize, destSize);
}

DataSourceStatus DataSourceAccountWSG::downloadBlobSync(const DataSourceURL &/*url*/, DataSourceBuffer::BufferData * /*dest*/, DataSourceBuffer::BufferSize &/*readSize*/, DataSourceBuffer::BufferSize /*size*/)
{
    DataSourceStatus    status;
    DataSourceURL        blobPath;

    try
    {
    // implement curl download
    throw;
    }
    catch (...)
    {
        return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Download);
    }

    //return DataSourceStatus();
}

DataSourceStatus DataSourceAccountWSG::uploadBlobSync(const DataSourceURL &/*url*/, DataSourceBuffer::BufferData * /*source*/, DataSourceBuffer::BufferSize /*size*/)
{
    try
    {
    // implement curl upload
    throw;
    }
    catch (...)
    {
        return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Upload);
    }


    //return DataSourceStatus();
}
