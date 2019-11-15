/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "DataSourceDefs.h"
#include "DataSourceService.h"
#include "DataSourceAccount.h"


class  DataSourceServiceFile : public DataSourceService
{

public:

    typedef DataSourceService       Super;

public:

                                    DataSourceServiceFile       (DataSourceManager &manager, const ServiceName &service);

    DataSourceAccount          *    createAccount               (const AccountName & account, const DataSourceAccount::AccountIdentifier identifier, const DataSourceAccount::AccountKey &key);
    DataSourceStatus                destroyAccount              (const AccountName & account);
};
