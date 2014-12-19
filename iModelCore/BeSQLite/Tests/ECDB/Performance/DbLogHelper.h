/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ECDB/Performance/DbLogHelper.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "PerformanceTests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE
//=======================================================================================    
//! @bsiclass                                            Carole.MacDonald      07/2014
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

END_ECDBUNITTESTS_NAMESPACE