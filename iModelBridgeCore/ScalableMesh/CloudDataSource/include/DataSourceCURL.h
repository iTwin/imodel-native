#pragma once

#include "DataSourceDefs.h"
#include "DataSourceCloud.h"
#include "DataSourceStatus.h"
#include "DataSourceBuffer.h"

class DataSourceAccountCURL;



class DataSourceCURL : public DataSourceCloud
{

protected:

    typedef DataSourceCloud                             Super;

protected:

    DataSourceAccountCURL *      getAccountCURL         (void);

    DataSourceStatus            flush                   (void);


public:

                                DataSourceCURL         (DataSourceAccount *sourceAccount);
                               ~DataSourceCURL         (void);

    DataSourceStatus            open                    (const DataSourceURL &sourceURL, DataSourceMode mode);
    DataSourceStatus            read                    (Buffer *dest, DataSize destSize, DataSize &readSize, DataSize size);
    DataSourceStatus            close                   (void);
};

