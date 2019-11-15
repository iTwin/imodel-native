/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "DataSourceDefs.h"
#include "DataSourceCached.h"


class DataSourceCloud : public DataSourceCached
{
protected:

    typedef DataSourceCached    Super;

public:

            DataSourceCloud        (DataSourceAccount *sourceAccount, const SessionName &session);
};