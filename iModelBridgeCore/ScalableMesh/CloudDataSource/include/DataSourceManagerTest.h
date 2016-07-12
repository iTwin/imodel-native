#pragma once
#include "DataSourceDefs.h"
#include "DataSourceStatus.h"
#include "DataSourceURL.h"
#include "DataSourceManager.h"


class DataSourceManagerTest
{

protected:

    DataSourceManager    dataSourceManager;

protected:

    DataSourceStatus    testBasicWriteRead            (DataSource *dataSource, const DataSourceURL &url, DataSourceBuffer::BufferSize size);
    DataSourceStatus    testBasicRead                (DataSource *dataSource, const DataSourceURL &url, DataSourceBuffer::BufferSize size);
    DataSourceStatus    testUpload                    (DataSource *dataSource, const DataSourceURL &directory);

public:

    DataSourceStatus    testDataSources                (void);

    DataSourceStatus    testDataSourceFile            (void);
    DataSourceStatus    testDataSourceAzure            (void);

};

