#pragma once

#include "DataSourceDefs.h"
#include "DataSourceCloud.h"
#include "DataSourceStatus.h"
#include "DataSourceBuffer.h"

class DataSourceAccountWSG;



class DataSourceWSG : public DataSourceCloud
{

protected:

    typedef DataSourceCloud                             Super;

protected:

    DataSourceAccountWSG *      getAccountWSG           (void);

    DataSourceStatus            flush                   (void);


public:

                                DataSourceWSG         (DataSourceAccount *sourceAccount);
                               ~DataSourceWSG         (void);

    DataSourceStatus            open                    (const DataSourceURL &sourceURL, DataSourceMode mode);
    DataSourceStatus            read                    (Buffer *dest, DataSize destSize, DataSize &readSize, DataSize size);
    DataSourceStatus            close                   (void);
};

