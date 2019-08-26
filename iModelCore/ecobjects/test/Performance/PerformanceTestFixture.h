/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

//=======================================================================================    
//! @bsiclass                                                 Krischan.Eberle      09/2012
//=======================================================================================    
struct PerformanceTestFixture : public ECTestFixture
{
protected:
    static void LogResultsToFile(bmap<Utf8String, double> results);

    static void TimeSchema(WCharCP schemaName, ECSchemaReadContextPtr schemaContext, Utf8String testcaseName, Utf8String testName);
    static void TimeInstance(WCharCP schemaName, WCharCP instanceXmlFile, ECSchemaReadContextPtr schemaContext, Utf8String testcaseName, Utf8String testName);
};

END_BENTLEY_ECN_TEST_NAMESPACE
