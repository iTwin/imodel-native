/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/bmap_performanceTest.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"
#include <Bentley/bmap.h>

    static int s_niters = 10000;

    struct Struct {double a, b;};

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
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "Insert", s_niters);

        // access out of order
        timer.Start();
        for (int i=s_niters-1; i>=0; --i)
            ASSERT_EQ (m[i].a, (double)i);
        timer.Stop();
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "Access out of order", s_niters);

        // access in order
        timer.Start();
        for (int i=0; i<s_niters; ++i)
            ASSERT_EQ (m[i].a, (double)i);
        timer.Stop();
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "Access out in order", s_niters);


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
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "Remove Isolated elements", s_niters);

        // insert elements out of order
        timer.Start();
        for (int i=0; i<s_niters; i += 10)
            {
            ASSERT_TRUE( m.insert (make_bpair(i,Struct())).second == true );
            }
        timer.Stop();
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "Insert out of order", s_niters);

        }

    //  Load out of order    TBD...
    }
