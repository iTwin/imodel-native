#include "stdafx.h"
#include "DataSourceServiceManager.h"
#include "DataSourceStatus.h"

#include "DataSourceServiceFile.h"
#include "DataSourceServiceAzure.h"


DataSourceServiceManager::DataSourceServiceManager(DataSourceManager &manager)
{
    initialize(manager);
}

DataSourceServiceManager::~DataSourceServiceManager(void)
{
    destroyServices();
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
    DataSourceAccount   * account;

    ApplyFunction getAccount = [this, accountName, &account]( ItemMap::iterator it) -> bool
    {
        DataSourceService *service = it->second;        
        if (service)
        {
            account = service->getAccount(accountName);
            if (account)
            {
                                                            // Found, so stop traversing
                return false;
            }
        }
                                                            // Not found, so continue traversing
        return true;
    };

                                                            // Apply to all services
    apply(getAccount);
                                                            // Return account if found
    return account;
}

DataSourceStatus DataSourceServiceManager::destroyService(const ServiceName & serviceName)
{
                                                            // Find and destroy service
    if (destroy(serviceName, true))
    {
        return DataSourceStatus();
    }

    return DataSourceStatus(DataSourceStatus::Status_Error_Unknown_Service);
}


DataSourceStatus DataSourceServiceManager::destroyServices(void)
{
    if (destroyAll(true))
    {
        return DataSourceStatus();
    }

    return DataSourceStatus(DataSourceStatus::Status_Error);
}

