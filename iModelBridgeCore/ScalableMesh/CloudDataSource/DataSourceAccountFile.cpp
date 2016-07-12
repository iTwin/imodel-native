#include "stdafx.h"
#include "DataSourceAccountFile.h"
#include "DataSourceFile.h"


DataSourceAccountFile::DataSourceAccountFile(const ServiceName &service, const AccountName &account) : DataSourceAccount(service, account)
{

}

DataSourceAccountFile::DataSourceAccountFile(const ServiceName &service, const AccountName & account, const AccountIdentifier identifier, const AccountKey key) : DataSourceAccount(service, account, identifier, key)
{
    
}


DataSource * DataSourceAccountFile::createDataSource(void)
{
    return new DataSourceFile(this);
}

DataSourceStatus DataSourceAccountFile::destroyDataSource(DataSource * dataSource)
{
    if (dataSource == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error);

    delete dataSource;

    return DataSourceStatus();
}
