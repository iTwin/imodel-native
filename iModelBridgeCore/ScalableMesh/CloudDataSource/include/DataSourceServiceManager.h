#pragma once
#include "DataSourceDefs.h"
#include "Manager.h"
#include "DataSourceService.h"
#include "DataSourceStatus.h"
#include "DataSourceAccount.h"


class DataSourceServiceManager : public Manager<DataSourceService>
{
public:

    typedef  ItemName                           ServiceName;

protected:

public:

                                                DataSourceServiceManager      (DataSourceManager &manager);
                                               ~DataSourceServiceManager      (void);

            DataSourceStatus                    initialize                    (DataSourceManager &manager);

			void								shutdown					  (void);

            DataSourceStatus                    addService                    (DataSourceService *service);
            DataSourceStatus                    destroyService                (const ServiceName &serviceName);
            DataSourceStatus                    destroyServices               (void);

            CLOUD_EXPORT DataSourceService  *   getService                    (const ServiceName &serviceName);

            DataSourceAccount               *   getAccount                    (const DataSourceAccount::AccountName &accountName);
};