/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"
#include <Bentley/bmap.h>

    static int s_niters = 10000;

    struct Struct {double a, b;};

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PerformanceBentley, bmap)
    {
    StopWatch timer;
    //  Load in order
    if (true)
        {
        bmap<int,Struct> m;
        timer.Start();
        for (int i=0; i<s_niters; ++i)
            m[i].a = m[i].b = (double)i;
        timer.Stop();
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_niters, "Insert");

        // access out of order
        timer.Start();
        for (int i=s_niters-1; i>=0; --i)
            ASSERT_EQ (m[i].a, (double)i);
        timer.Stop();
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_niters, "Access out of order");

        // access in order
        timer.Start();
        for (int i=0; i<s_niters; ++i)
            ASSERT_EQ (m[i].a, (double)i);
        timer.Stop();
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_niters, "Access out in order");


        // remove isolated elements
        size_t wasSize = m.size();
        size_t nerased = 0;
        timer.Start();
        for (int i=0; i<s_niters; i += 10)
            {
            ASSERT_EQ( m.erase (i), 1 );
            ++nerased;
            }
        ASSERT_TRUE (m.size() == wasSize-nerased);
        timer.Stop();
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_niters, "Remove Isolated elements");

        // insert elements out of order
        timer.Start();
        for (int i=0; i<s_niters; i += 10)
            {
            ASSERT_TRUE( m.insert (make_bpair(i,Struct())).second == true );
            }
        timer.Stop();
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_niters, "Insert out of order");

        }

    //  Load out of order    TBD...
    }
