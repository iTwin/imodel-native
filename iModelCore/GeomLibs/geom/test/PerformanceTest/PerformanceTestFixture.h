/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <Bentley/BeFileName.h>
#include <Bentley/BeTimeUtilities.h>
#include <Geom/GeomApi.h>
#include "GeomLibsTests.h"

BEGIN_GEOMLIBS_TESTS_NAMESPACE

#define GEOMPERFORMANCELOG (*NativeLogging::LoggingManager::GetLogger (L"GeomLibsPerf"))
typedef bpair<Utf8String, double> T_TimerResultPair;

struct PerformanceTestFixture : ::testing::Test
    {
    protected:
        PerformanceTestFixture() {};

        static void LogResultsToFile(bmap<Utf8String, double> results);

    };

END_GEOMLIBS_TESTS_NAMESPACE
