/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "DataSourceDefs.h"

class DataSourceStoreConfig
{
protected:

    enum AccessType
    {
        Type_NULL,

        Type_Normal,
        Type_Random_Access
    };

    enum DataDivision
    {
        Size_NULL,

        Size_Discrete,
        Size_Large
    };

protected:

    AccessType              accessType;
    DataDivision            dataDivision;

public:

};