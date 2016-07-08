#pragma once
#include "DataSourceDefs.h"
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

CLOUD_EXPORT											DataSourceServiceManager	(DataSourceManager &manager);

CLOUD_EXPORT			DataSourceStatus				initialize					(DataSourceManager &maanger);

CLOUD_EXPORT			DataSourceStatus				addService					(DataSourceService *service);
CLOUD_EXPORT			DataSourceStatus				destroyService				(const ServiceName &serviceName);

CLOUD_EXPORT			DataSourceService			*	getService					(const ServiceName &serviceName);

CLOUD_EXPORT			DataSourceAccount			*	getAccount					(const DataSourceAccount::AccountName &accountName);
};