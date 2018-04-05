#include "stdafx.h"
#include "DataSourceAccountFile.h"
#include "DataSourceFile.h"

unsigned int const DATA_SOURCE_ACCOUNT_FILE_DEFAULT_TRANSFER_TASKS = 1;


DataSourceAccountFile::DataSourceAccountFile(const ServiceName &service, const AccountName &account) : DataSourceAccount(service, account)
    {
    getTransferScheduler()->initializeTransferTasks(getDefaultNumTransferTasks());
    }

DataSourceAccountFile::DataSourceAccountFile(const ServiceName &service, const AccountName & account, const AccountIdentifier identifier, const AccountKey key) : DataSourceAccount(service, account, identifier, key)
    {
    getTransferScheduler()->initializeTransferTasks(getDefaultNumTransferTasks());
    }

unsigned int DataSourceAccountFile::getDefaultNumTransferTasks(void)
    {
    return DATA_SOURCE_ACCOUNT_FILE_DEFAULT_TRANSFER_TASKS;
    }


DataSource * DataSourceAccountFile::createDataSource(const SessionName &session)
    {
    return new DataSourceFile(this, session);
    }

DataSourceStatus DataSourceAccountFile::destroyDataSource(DataSource * dataSource)
    {
    if (dataSource == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error);

    delete dataSource;

    return DataSourceStatus();
    }
