/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/bvector_performanceTest.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"
#include <Bentley/bvector.h>

    static int s_niters = 10000;

    struct Struct {double a, b;};

/*---------------------------------------------------------------------------------**//**
* @bsitest                                    Sam.Wilson                        02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PerformanceBentley, bvector)
    {
    StopWatch timer;
    // push_back and grow
    if (true)
        {
        bvector<int> v;
        timer.Start();
        for (int i=0; i<s_niters; ++i)
            v.push_back (i);
        ASSERT_EQ (v.size(), s_niters);
        timer.Stop();
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_niters, "push_back integers");

        bvector<Struct> vs;
        timer.Start();
        for (int i=0; i<s_niters; ++i)
            vs.push_back (Struct());
        ASSERT_EQ (v.size(), s_niters);
        timer.Stop();
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_niters, "push_back structs");
        }

    // push_back within allocated space
    if (true)
        {
        bvector<int> v;
        v.reserve (s_niters);
        timer.Start();
        for (int i=0; i<s_niters; ++i)
            v.push_back (i);
        ASSERT_EQ (v.size(), s_niters);
        timer.Stop();
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_niters, "push_back within allocated space. Integers");

        bvector<Struct> vs;
        v.reserve (s_niters);
        timer.Start();
        for (int i=0; i<s_niters; ++i)
            vs.push_back (Struct());
        ASSERT_EQ (v.size(), s_niters);
        timer.Stop();
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_niters, "push_back within allocated space. Structs");
        }

    //  indexing
    if (true)
        {
        bvector<double> v (s_niters);
        timer.Start();
        for (int i=0; i<s_niters; ++i)
            v[i] = (double)i;
        timer.Stop();
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_niters, "indexing. Integers");

        for (int i=0; i<s_niters; ++i)
            ASSERT_EQ (v[i], (double)i);

        bvector<Struct> vs (s_niters);
        timer.Start();
        for (int i=0; i<s_niters; ++i)
            vs[i].a = vs[i].b = (double)i;
        timer.Stop();
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_niters, "indexing. Structs");

        for (int i=0; i<s_niters; ++i)
            ASSERT_EQ (vs[i].a, (double)i);
        }

    //  indexing (down)
    if (true)
        {
        bvector<double> v (s_niters);
        timer.Start();
        for (int i=s_niters-1; i>=0; --i)
            v[i] = (double)i;
        timer.Stop();
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_niters, "indexing (down). Integers");

        for (int i=s_niters-1; i>=0; --i)
            ASSERT_EQ (v[i], (double)i);

        bvector<Struct> vs (s_niters);
        timer.Start();
        for (int i=s_niters-1; i>=0; --i)
            vs[i].a = vs[i].b = (double)i;
        timer.Stop();
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_niters, "indexing (down). Structs");

        for (int i=s_niters-1; i>=0; --i)
            ASSERT_EQ (vs[i].a, (double)i);


        // find
        timer.Start();
        for (int i=0; i<s_niters; ++i)
            {
            bvector<double>::const_iterator it = std::find (v.begin(), v.end(), (double)(s_niters/2));
            ASSERT_TRUE( it != v.end() );
            ASSERT_EQ( *it, (double)(s_niters/2) );
            }
        timer.Stop();
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_niters, "find");

        }
    }