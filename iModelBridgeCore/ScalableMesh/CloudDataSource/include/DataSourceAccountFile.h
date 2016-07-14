#pragma once
#include "DataSourceDefs.h"
#include "DataSource.h"
#include "DataSourceAccount.h"

class DataSourceAccountFile : public DataSourceAccount
{

public:
                                DataSourceAccountFile        (const ServiceName &service, const AccountName &account);
                                DataSourceAccountFile        (const ServiceName &service, const AccountName &account, const AccountIdentifier identifier, const AccountKey key);

    DataSource                *    createDataSource            (void);
    DataSourceStatus            destroyDataSource            (DataSource *dataSource);

};
