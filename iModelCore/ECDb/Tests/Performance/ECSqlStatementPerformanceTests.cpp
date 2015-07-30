/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/ECSqlStatementPerformanceTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceECSqlStatementTestFixture.h"

BEGIN_ECDBUNITTESTS_NAMESPACE
//=======================================================================================    
//! @bsiclass                                                 Krischan.Eberle      10/2013
//=======================================================================================    
TEST_F(PerformanceECSqlStatementTestFixture, GetValueIntegerWithIsNull)
    {
    StopWatch timer("",false);
    Utf8String testDetails="PerformanceECSqlStatementTestFixture,GetValueIntegerWithIsNull";
    PerformanceECSqlStatementTestFixture::GetIntegerValueAsserter asserter (1, "IntegerMember");
    PerformanceECSqlStatementTestFixture::RunGetValueWithIsNullTest (asserter,timer,testDetails);
    }
    
//=======================================================================================    
//! @bsiclass                                                 Krischan.Eberle      10/2013
//=======================================================================================    
TEST_F(PerformanceECSqlStatementTestFixture, GetValueIntegerWithoutIsNull)
    {
    StopWatch timer("", false);
    Utf8String testDetails = "PerformanceECSqlStatementTestFixture,GetValueIntegerWithoutIsNull";
    PerformanceECSqlStatementTestFixture::GetIntegerValueAsserter asserter (1, "IntegerMember");
    PerformanceECSqlStatementTestFixture::RunGetValueWithoutIsNullTest (asserter,timer,testDetails);
    }

//=======================================================================================    
//! @bsiclass                                                 Krischan.Eberle      10/2013
//=======================================================================================    
TEST_F(PerformanceECSqlStatementTestFixture, GetValueStringWithIsNull)
    {
    StopWatch timer("", false);
    Utf8String testDetails = "PerformanceECSqlStatementTestFixture,GetValueStringWithIsNull";
    PerformanceECSqlStatementTestFixture::GetStringValueAsserter asserter (2, "StringMember");
    PerformanceECSqlStatementTestFixture::RunGetValueWithIsNullTest (asserter,timer,testDetails);
    }

//=======================================================================================    
//! @bsiclass                                                 Krischan.Eberle      10/2013
//=======================================================================================    
TEST_F(PerformanceECSqlStatementTestFixture, GetValueStringWithoutIsNull)
    {
    StopWatch timer("", false);
    Utf8String testDetails = "PerformanceECSqlStatementTestFixture,GetValueStringWithoutIsNull";
    PerformanceECSqlStatementTestFixture::GetStringValueAsserter asserter (2, "StringMember");
    PerformanceECSqlStatementTestFixture::RunGetValueWithoutIsNullTest (asserter,timer,testDetails);
    }

//=======================================================================================    
//! @bsiclass                                                 Krischan.Eberle      10/2013
//=======================================================================================    
TEST_F(PerformanceECSqlStatementTestFixture, GetValuePoint3DWithIsNull)
    {
    StopWatch timer("", false);
    Utf8String testDetails = "PerformanceECSqlStatementTestFixture,GetValuePoint3DWithIsNull";
    PerformanceECSqlStatementTestFixture::GetPoint3DValueAsserter asserter (4, "StartPoint");
    PerformanceECSqlStatementTestFixture::RunGetValueWithIsNullTest (asserter,timer,testDetails);
    }

//=======================================================================================    
//! @bsiclass                                                 Krischan.Eberle      10/2013
//=======================================================================================    
TEST_F(PerformanceECSqlStatementTestFixture, GetValuePoint3DWithoutIsNull)
    {
    StopWatch timer("", false);
    Utf8String testDetails = "PerformanceECSqlStatementTestFixture,GetValuePoint3DWithoutIsNull";
    PerformanceECSqlStatementTestFixture::GetPoint3DValueAsserter asserter (4, "StartPoint");
    PerformanceECSqlStatementTestFixture::RunGetValueWithoutIsNullTest (asserter,timer,testDetails);
    }

END_ECDBUNITTESTS_NAMESPACE