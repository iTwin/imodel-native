#pragma once
#include "DataSourceDefs.h"
#include "DataSource.h"
#include "DataSourceAccount.h"
#include "DataSourceURL.h"


class DataSourceAccountCached : public DataSourceAccount
{
protected:

    DataSourceAccount        *      m_cacheAccount;
    DataSourceURL                   m_cacheRoot;

protected:

        DataSource           *      getCacheDataSource            (SessionName &session);

public:

                                    DataSourceAccountCached       (void);
                                   ~DataSourceAccountCached       (void);

        DataSourceStatus            setCaching                    (DataSourceAccount &cacheAccount, const DataSourceURL &cachingRootPath);

        void                        setCacheAccount               (DataSourceAccount *account);
        DataSourceAccount    *      getCacheAccount               (void);

        DataSourceStatus            getFormattedCacheURL          (const DataSourceURL & sourceURL, DataSourceURL & cacheURL);
        
        unsigned int                getDefaultNumTransferTasks(void);
};