/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "DataSourceDefs.h"
#include "Manager.h"
#include "DataSourceService.h"
#include "DataSourceStatus.h"
#include "DataSourceAccount.h"


class DataSourceServiceManager : public Manager<DataSourceService, true>
{
public:

    typedef  ItemName                           ServiceName;

protected:

public:

                                       explicit DataSourceServiceManager      (DataSourceManager &manager);
                                               ~DataSourceServiceManager      (void);

            DataSourceStatus                    initialize                    (DataSourceManager &manager);

			void								shutdown					  (void);

            DataSourceStatus                    addService                    (DataSourceService *service);
            DataSourceStatus                    destroyService                (const ServiceName &serviceName);
            DataSourceStatus                    destroyServices               (void);

            CLOUD_EXPORT DataSourceService  *   getService                    (const ServiceName &serviceName);

            DataSourceAccount               *   getAccount                    (const DataSourceAccount::AccountName &accountName);
};