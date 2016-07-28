
#include "stdafx.h"

#include "DataSourceManagerTest.h"
#include "DataSourceAzure.h"
#include "DataSourceStatus.h"
#include "include\DataSourceManagerTest.h"

typedef    unsigned long        TestValue;


CLOUD_EXPORT DataSourceManagerTest::DataSourceManagerTest(void)
{
}

CLOUD_EXPORT DataSourceManagerTest::~DataSourceManagerTest(void)
{
}

CLOUD_EXPORT DataSourceStatus DataSourceManagerTest::testDataSources(void)
{
    DataSourceStatus    status;

    //if ((status = testDataSourceFile()).isFailed())
    //    return status;

    if ((status = testDataSourceAzure()).isFailed())
        return status;

    return DataSourceStatus();
}


DataSourceStatus DataSourceManagerTest::testBasicWriteRead(DataSource *dataSource, const DataSourceURL &url, DataSourceBuffer::BufferSize dataSize)
{
    if (dataSource == nullptr || dataSize == 0)
        return DataSourceStatus(DataSourceStatus::Status_Error_Bad_Parameters);

                                                            // Set up source/dest test buffers
    DataSourceStatus                    status;
    unsigned long long                    numTestValues = dataSize / sizeof(TestValue);
    DataSourceBuffer::BufferSize        bufferSize = dataSize;
    TestValue                        *    bufferSource = nullptr;
    TestValue                        *    bufferDest = nullptr;;

    DataSourceBuffer::BufferSize readSize;

    try
    {
        if ((bufferSource = new TestValue[numTestValues]) == nullptr)
            throw DataSourceStatus(DataSourceStatus::Status_Error_Memory_Allocation);

        if ((bufferDest = new TestValue[numTestValues]) == nullptr)
            throw DataSourceStatus(DataSourceStatus::Status_Error_Memory_Allocation);

                                                            // Create source buffer contents
        for (TestValue t = 0; t < numTestValues; t++)
        {
            bufferSource[t] = t;
        }
                                                            // Write some data and close
        dataSource->open(url, DataSourceMode_Write);
        dataSource->write(reinterpret_cast<DataSourceBuffer::BufferData *>(bufferSource), bufferSize);
        dataSource->close();
                                                            // Read back data and close
        dataSource->open(url, DataSourceMode_Read);
        dataSource->read(reinterpret_cast<DataSourceBuffer::BufferData *>(bufferDest), bufferSize, readSize, bufferSize);
        dataSource->close();

                                                            // Verify dest buffer contents
        for (unsigned int t = 0; t < numTestValues; t++)
        {
            if (bufferDest[t] != bufferSource[t])
                return DataSourceStatus(DataSourceStatus::Status_Error_Test_Failed);
        }
    }
    catch (DataSourceStatus s)
    {
        status = s;
    }

    if (bufferSource != nullptr)
        delete bufferSource;

    if (bufferDest != nullptr)
        delete bufferDest;

    return status;
}

DataSourceStatus DataSourceManagerTest::testBasicRead(DataSource * dataSource, const DataSourceURL & url, DataSourceBuffer::BufferSize dataSize)
{
    if (dataSource == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error_Test_Failed);

                                                            // Set up source/dest test buffers
    DataSourceStatus                status;
    unsigned long long                numTestValues = dataSize / sizeof(TestValue);
    DataSourceBuffer::BufferSize    bufferSize = dataSize;
    TestValue                     *    bufferDest = nullptr;
    DataSourceBuffer::BufferSize    readSize;

    try
    {
        if ((bufferDest = new TestValue[numTestValues]) == nullptr)
            throw DataSourceStatus(DataSourceStatus::Status_Error_Memory_Allocation);

                                                            // Read back data and close
        if ((status = dataSource->open(url, DataSourceMode_Read)).isFailed())
            throw status;

        if ((status = dataSource->read(reinterpret_cast<DataSourceBuffer::BufferData *>(bufferDest), bufferSize, readSize)).isFailed())
            throw status;

        if ((status = dataSource->close()).isFailed())
            throw status;
                                                            // Verify dest buffer contents
        for (unsigned int t = 0; t < numTestValues; t++)
        {
            if (bufferDest[t] != t)
                return DataSourceStatus(DataSourceStatus::Status_Error_Test_Failed);
        }
    }
    catch (DataSourceStatus s)
    {
        status = s;
    }

    if (bufferDest != nullptr)
        delete bufferDest;

    return status;
}

DataSourceStatus DataSourceManagerTest::testUpload(DataSource * dataSource, const DataSourceURL & directory)
{
    (void) dataSource;
    (void) directory;
    
    return DataSourceStatus();
}

DataSourceStatus DataSourceManagerTest::testDataSourceFile(void)
{
    DataSource            *    dataSource;
    DataSourceURL            urlFile(L"C:\\Users\\Lee.Bull\\temp\\file1");
    DataSourceStatus        status;
    DataSourceService    *    service;

    if ((service = dataSourceManager.getService(DataSourceService::ServiceName(L"DataSourceServiceFile"))) == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error_Test_Failed);

    service->createAccount(DataSourceAccount::AccountName(L"FileAccount"), DataSourceAccount::AccountIdentifier(), DataSourceAccount::AccountKey());

                                                            // Create DataSource
    dataSource = dataSourceManager.createDataSource(std::wstring(L"MyFileDataSource"), DataSourceService::ServiceName(L"FileAccount"), nullptr);
    if (dataSource == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error_Test_Failed);
                                                            // Run basic write/read test
    if ((status = testBasicWriteRead(dataSource, urlFile, 1024 * 1024 * 8)).isFailed())
        return status;
                                                            // Destroy the data source
    dataSourceManager.destroyDataSource(dataSource);

    return DataSourceStatus();
}


DataSourceStatus DataSourceManagerTest::testDataSourceAzure(void)
{
    DataSourceAzure                            *    dataSourceAzure;
    DataSourceAccount::AccountIdentifier        accountIdentifier(L"pcdsustest");
    DataSourceAccount::AccountKey                accountKey(L"3EQ8Yb3SfocqbYpeIUxvwu/aEdiza+MFUDgQcIkrxkp435c7BxV8k2gd+F+iK/8V2iho80kFakRpZBRwFJh8wQ==");
    DataSourceStatus                            status;
    DataSourceService                        *    serviceAzure;
    DataSourceService                        *    serviceFile;
    DataSourceAccount                        *    accountAzure;
    DataSourceAccount                        *    accountCaching;
    DataSourceBuffer::BufferSize                testDataSize = 1024 * 1024 * 8;

                                                            // Get the Azure service
    if ((serviceAzure = dataSourceManager.getService(DataSourceService::ServiceName(L"DataSourceServiceAzure"))) == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error_Test_Failed);
                                                            // Create an account on Azure
    accountAzure = serviceAzure->createAccount(DataSourceAccount::AccountName(L"AzureAccount"), accountIdentifier, accountKey);
                                                            // Create an Azure specific DataSource
    dataSourceAzure = dynamic_cast<DataSourceAzure *>(dataSourceManager.createDataSource(DataSourceManager::DataSourceName(L"MyAzureDataSource"), DataSourceAccount::AccountName(L"AzureAccount"), nullptr));
    if (dataSourceAzure == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error);
                                                            // Blobs will be split up into segments of this size
    dataSourceAzure->setSegmentSize(1024 * 64);
                                                            // Time I/O operation timeouts for threading
    dataSourceAzure->setTimeout(DataSource::Timeout(100000));

                                                            // Run basic write/read test
    if ((status = testBasicWriteRead(dataSourceAzure, DataSourceURL(L"testcontainer/testblob"), testDataSize)).isFailed())
        return status;

                                                            // Create an account for a local file cache

                                                            // Get the file service
    if ((serviceFile = dataSourceManager.getService(DataSourceService::ServiceName(L"DataSourceServiceFile"))) == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error_Test_Failed);
                                                            // Create an account on the file service for caching
    if ((accountCaching = serviceFile->createAccount(DataSourceAccount::AccountName(L"FileAccount"), DataSourceAccount::AccountIdentifier(), DataSourceAccount::AccountKey())) == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error_Test_Failed);

    accountAzure->setPrefixPath(DataSourceURL(L"C:\\Temp\\CacheAzure"));
                                                            // Set up local file based caching
    accountAzure->setCaching(*accountCaching, DataSourceURL());
                                                            // Enable caching for this DataSource
    dataSourceAzure->setCachingEnabled(true);

                                                            // Run basic read test with caching enabled. This pass will populate the cache.
    if ((status = testBasicRead(dataSourceAzure, DataSourceURL(L"testcontainer/testblob"), testDataSize)).isFailed())
        return status;
                                                            // Run basic write/read test with caching enabled. This pass will use the cache.
    if ((status = testBasicRead(dataSourceAzure, DataSourceURL(L"testcontainer/testblob"), testDataSize)).isFailed())
        return status;

                                                            // Destroy the DataSource
    dataSourceManager.destroyDataSource(dataSourceAzure);

                                                            // Return OK
    return DataSourceStatus();
}
