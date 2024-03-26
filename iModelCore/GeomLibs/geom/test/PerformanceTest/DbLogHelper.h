/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeFileName.h>
#include <BeSQLite/BeSQLite.h>
#include "GeomLibsTests.h"

USING_NAMESPACE_BENTLEY_SQLITE

BEGIN_GEOMLIBS_TESTS_NAMESPACE
//=======================================================================================
//! @bsiclass
//=======================================================================================
struct DbLogHelper
    {
    private:
        Db             m_db;

        void    Initialize(BeFileNameR dbName);
        void    CreateTables();
        int     GetTestId(Utf8String testName);

    public:
        DbLogHelper(BeFileNameR dbName);
        DbLogHelper();

        void LogResults(bmap<Utf8String, double> results);
    };

END_GEOMLIBS_TESTS_NAMESPACE