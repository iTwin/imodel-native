/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlTestFixture.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbPublishedTests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================    
// @bsiclass                                                 Krischan.Eberle     10/2015
//=======================================================================================    
struct ECSqlStatementTestFixture : public ECDbTestFixture
    {
protected:
    static void BindFromJson(BentleyStatus& succeeded, ECSqlStatement const& statement, JsonValueCR jsonValue, IECSqlBinder& structBinder);
    static void VerifyECSqlValue(ECSqlStatement const& statement, JsonValueCR expectedValue, IECSqlValue const& ecsqlValue);
    };

END_ECDBUNITTESTS_NAMESPACE