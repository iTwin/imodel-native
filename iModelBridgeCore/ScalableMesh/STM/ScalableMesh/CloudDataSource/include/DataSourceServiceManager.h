#pragma once

#include "Manager.h"
#include "DataSourceService.h"
#include "DataSourceStatus.h"
#include "DataSourceAccount.h"


class DataSourceServiceManager : public Manager<DataSourceService>
{
public:

	typedef	ItemName				ServiceName;

protected:

public:

											DataSourceServiceManager	(DataSourceManager &manager);

			DataSourceStatus				initialize					(DataSourceManager &maanger);

			DataSourceStatus				addService					(DataSourceService *service);
			DataSourceStatus				destroyService				(const ServiceName &serviceName);

			DataSourceService			*	getService					(const ServiceName &serviceName);

			DataSourceAccount			*	getAccount					(const DataSourceAccount::AccountName &accountName);
};