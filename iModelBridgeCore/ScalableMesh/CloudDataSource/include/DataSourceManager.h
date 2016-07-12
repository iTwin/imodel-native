#pragma once
#include "DataSourceDefs.h"
#include "DataSource.h"
#include "Manager.h"
#include "DataSourceServiceManager.h"

class DataSourceManager : public Manager<DataSource>, public DataSourceServiceManager
{
public:

    typedef DataSource::Name    DataSourceName;

protected:

public:
CLOUD_EXPORT                                DataSourceManager        (void);

    DataSource *                         createDataSource        (const DataSourceName &name, const DataSourceAccount::AccountName &account, const DataSourceStoreConfig *config = nullptr);
    DataSource *                         createDataSource        (const DataSourceName &name, DataSourceAccount &account, const DataSourceStoreConfig *config = nullptr);

    DataSource *                         getOrCreateDataSource   (const DataSourceName &name, DataSourceAccount &account, bool *created = nullptr);

    DataSourceStatus                     destroyDataSource       (DataSource *dataSource);

    DataSourceAccount *                  initializeAzureTest     (void);

};