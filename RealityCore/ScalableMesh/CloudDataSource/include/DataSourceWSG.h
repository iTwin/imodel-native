/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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

                                DataSourceWSG           (DataSourceAccount *sourceAccount, const SessionName &session);
                               ~DataSourceWSG           (void);

    DataSourceStatus            open                    (const DataSourceURL &sourceURL, DataSourceMode mode);
    DataSourceStatus            read                    (Buffer *dest, DataSize destSize, DataSize &readSize, DataSize size);
    DataSourceStatus            read                    (std::vector<Buffer>& dest);
    DataSourceStatus            close                   (void);
};

