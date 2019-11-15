/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "DataSourceDefs.h"
#include "DataSourceStatus.h"
#include "DataSourceURL.h"
#include "DataSourceManager.h"


class DataSourceManagerTest
{

protected:

    DataSourceStatus    testBasicWriteRead           (DataSource *dataSource, const DataSourceURL &url, DataSourceBuffer::BufferSize size);
    DataSourceStatus    testBasicRead                (DataSource *dataSource, const DataSourceURL &url, DataSourceBuffer::BufferSize size);
    DataSourceStatus    testUpload                   (DataSource *dataSource, const DataSourceURL &directory);

public:

    CLOUD_EXPORT                        DataSourceManagerTest       (void);
    CLOUD_EXPORT                       ~DataSourceManagerTest       (void);

    CLOUD_EXPORT    DataSourceStatus    testDataSources              (void);

    CLOUD_EXPORT    DataSourceStatus    testDataSourceFile           (void);
#ifdef USE_WASTORAGE
    CLOUD_EXPORT    DataSourceStatus    testDataSourceAzure          (void);
#endif

};

