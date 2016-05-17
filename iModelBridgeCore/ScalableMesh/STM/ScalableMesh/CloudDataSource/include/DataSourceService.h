#pragma once

#include <string>

#include "DataSource.h"
#include "DataSourceStatus.h"
#include "Manager.h"
#include "DataSourceAccount.h"

class DataSourceManager;


class DataSourceService : public Manager<DataSourceAccount>
{

public:

	typedef std::wstring					ServiceName;
	typedef std::wstring					AccountName;


protected:

	DataSourceManager					*	dataSourceManager;

	ServiceName								serviceName;

protected:

			void							setDataSourceManager		(DataSourceManager &manager);

			void							createAccount				(DataSourceManager &manager, DataSourceAccount &account);


public:
											DataSourceService			(DataSourceManager &manager, const ServiceName &name);

			void							setServiceName				(const ServiceName &serviceName);
			const ServiceName			&	getServiceName				(void);

			DataSourceManager			&	getDataSourceManager		(void);

	virtual	DataSourceAccount			*	createAccount				(const AccountName &accountName, const DataSourceAccount::AccountIdentifier identifier, const DataSourceAccount::AccountKey &key) = 0;
	virtual	DataSourceStatus				destroyAccount				(const AccountName &accountName) = 0;

			DataSourceAccount			*	getAccount					(const AccountName &accountName);
};