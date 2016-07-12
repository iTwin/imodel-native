#include "stdafx.h"
#include "DataSourceServiceManager.h"
#include "DataSourceStatus.h"

#include "DataSourceServiceFile.h"
#include "DataSourceServiceAzure.h"


DataSourceServiceManager::DataSourceServiceManager(DataSourceManager &manager)
{
    initialize(manager);
}

DataSourceStatus DataSourceServiceManager::initialize(DataSourceManager &manager)
{
    DataSourceService *    service;
    DataSourceStatus    status;

    if ((service = new DataSourceServiceFile(manager, DataSourceService::ServiceName(L"DataSourceServiceFile"))) == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error_Memory_Allocation);

    if ((status = addService(service)).isFailed())
        return status;


    if((service = new DataSourceServiceAzure(manager, DataSourceService::ServiceName(L"DataSourceServiceAzure"))) == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error_Test_Failed);

    if ((status = addService(service)).isFailed())
        return status;

    return status;

}

DataSourceStatus DataSourceServiceManager::addService(DataSourceService * service)
{
    if (service == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error);

    if (get(service->getServiceName()) != NULL)
        return DataSourceStatus(DataSourceStatus::Status_Error_Service_Exists);

    if (create(service->getServiceName(), service) == NULL)
        return DataSourceStatus(DataSourceStatus::Status_Error);

    return DataSourceStatus();
}

DataSourceService * DataSourceServiceManager::getService(const ServiceName & serviceName)
{
    return get(serviceName);
}

DataSourceAccount * DataSourceServiceManager::getAccount(const DataSourceAccount::AccountName & accountName)
{
    DataSourceService    *    service;
    DataSourceAccount    *    account;

    for (auto s : Manager<DataSourceService>::items)
    {
        service = s.second;
        if (service)
        {
            account = service->getAccount(accountName);
            if(account)
                return account;
        }
    }

    return nullptr;
}

DataSourceStatus DataSourceServiceManager::destroyService(const ServiceName & serviceName)
{
    const DataSourceService *service;

    if ((service = getService(serviceName)) != nullptr)
    {
        delete service;

        items.erase(serviceName);

        return DataSourceStatus();
    }

    return DataSourceStatus(DataSourceStatus::Status_Error);
}
