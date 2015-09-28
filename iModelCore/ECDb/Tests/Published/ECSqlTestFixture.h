/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlTestFixture.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECSqlCrudAsserter.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================    
// @bsiclass                                                 Krischan.Eberle     04/2013
//=======================================================================================    
struct ECSqlTestFixture : public ECDbTestFixture
    {
private:
    virtual ECDbR _SetUp (Utf8CP ecdbFileName, WCharCP schemaECXmlFileName, ECDb::OpenParams openParams, int perClassRowCount);

protected:
    ECDbR SetUp (Utf8CP ecdbFileName, WCharCP schemaECXmlFileName, ECDb::OpenParams openParams, int perClassRowCount);

    static void BindFromJson (BentleyStatus& succeeded, ECSqlStatement const& statement, JsonValueCR jsonValue, IECSqlBinder& structBinder);
    static void VerifyECSqlValue (ECSqlStatement const& statement, JsonValueCR expectedValue, IECSqlValue const& ecsqlValue);

    };


END_ECDBUNITTESTS_NAMESPACE