#pragma once
#include "DataSourceDefs.h"
#include <string>

#include "DataSource.h"
#include "DataSourceStatus.h"
#include "Manager.h"
#include "DataSourceAccount.h"

class DataSourceManager;

class DataSourceService : public Manager<DataSourceAccount, true>
{

public:

    typedef std::wstring                        ServiceName;
    typedef std::wstring                        AccountName;


protected:

    DataSourceManager                       *   dataSourceManager;

    ServiceName                                 serviceName;

protected:

            void                                setDataSourceManager        (DataSourceManager &manager);

            void                                createAccount               (DataSourceManager &manager, DataSourceAccount &account);


public:
                                                DataSourceService           (DataSourceManager &manager, const ServiceName &name);
    virtual                                    ~DataSourceService           (void);

    CLOUD_EXPORT void                           setServiceName              (const ServiceName &serviceName);
    CLOUD_EXPORT const ServiceName          &   getServiceName              (void);

    CLOUD_EXPORT DataSourceManager          &   getDataSourceManager        (void);

    CLOUD_EXPORT virtual DataSourceAccount  *   createAccount               (const AccountName &accountName, const DataSourceAccount::AccountIdentifier identifier, const DataSourceAccount::AccountKey &key) = 0;
    CLOUD_EXPORT virtual DataSourceStatus       destroyAccount              (const AccountName &accountName);

    CLOUD_EXPORT DataSourceAccount          *   getOrCreateAccount          (const AccountName &accountName, const DataSourceAccount::AccountIdentifier identifier, const DataSourceAccount::AccountKey &key);

    CLOUD_EXPORT DataSourceAccount          *   getAccount                  (const AccountName &accountName);
};