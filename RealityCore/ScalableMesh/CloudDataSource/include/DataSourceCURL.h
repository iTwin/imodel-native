/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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

                                DataSourceCURL          (DataSourceAccount *sourceAccount, const SessionName &session);
                               ~DataSourceCURL          (void);

    DataSourceStatus            open                    (const DataSourceURL &sourceURL, DataSourceMode mode) override;
    DataSourceStatus            read                    (Buffer *dest, DataSize destSize, DataSize &readSize, DataSize size) override;
    DataSourceStatus            read                    (std::vector<Buffer>& dest) override;
    DataSourceStatus            close                   (void) override;
};

