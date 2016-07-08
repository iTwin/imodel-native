#pragma once
#include "DataSourceDefs.h"
#include "DataSourceService.h"
#include "DataSourceAccount.h"


class  DataSourceServiceFile : public DataSourceService
{

public:

CLOUD_EXPORT									DataSourceServiceFile		(DataSourceManager &manager, const ServiceName &service);

CLOUD_EXPORT	DataSourceAccount			*	createAccount				(const AccountName & account, const DataSourceAccount::AccountIdentifier identifier, const DataSourceAccount::AccountKey &key);
CLOUD_EXPORT	DataSourceStatus				destroyAccount				(const AccountName & account);
};
