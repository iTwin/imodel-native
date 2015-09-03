/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Performance/PerformanceTestFixture.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <ECObjects/ECObjectsAPI.h>
#include <Bentley/BeTimeUtilities.h>
#include <Logging/bentleylogging.h>
#include "../TestFixture/TestFixture.h"

#define PERFORMANCELOG (*NativeLogging::LoggingManager::GetLogger (L"Performance"))
typedef bpair<Utf8String, double> T_TimerResultPair;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

    //=======================================================================================    
    //! @bsiclass                                                 Krischan.Eberle      09/2012
    //=======================================================================================    
struct PerformanceTestFixture : public ECTestFixture
    {
    protected:
        PerformanceTestFixture() {};
        virtual ~PerformanceTestFixture () {};

        static void LogResultsToFile(bmap<Utf8String, double> results);

    };

END_BENTLEY_ECN_TEST_NAMESPACE

