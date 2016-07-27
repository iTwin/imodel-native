#pragma once
#include "DataSourceDefs.h"
#include "DataSourceService.h"
#include "DataSourceAccount.h"


class DataSourceServiceAzure : public DataSourceService
{

public:

    typedef DataSourceService   Super;

public:

                                DataSourceServiceAzure          (DataSourceManager &manager, const ServiceName &service);

        DataSourceAccount *     createAccount                   (const AccountName & account, const DataSourceAccount::AccountIdentifier identifier, const DataSourceAccount::AccountKey &key);
        DataSourceStatus        destroyAccount                  (const AccountName & account);
};

