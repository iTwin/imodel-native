/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "DataSourceDefs.h"
#include "DataSource.h"
#include "DataSourceAccount.h"

class DataSourceAccountFile : public DataSourceAccount
{

public:
                                DataSourceAccountFile       (const ServiceName &service, const AccountName &account);
                                DataSourceAccountFile       (const ServiceName &service, const AccountName &account, const AccountIdentifier identifier, const AccountKey key);

    DataSource             *    createDataSource            (const SessionName &session) override;
    DataSourceStatus            destroyDataSource           (DataSource *dataSource) override;

    unsigned int                getDefaultNumTransferTasks  (void) override;

};
