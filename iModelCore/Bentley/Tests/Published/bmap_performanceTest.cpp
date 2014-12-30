/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/bmap_performanceTest.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/bmap.h>

    static int s_niters = 10000;

    struct Struct {double a, b;};

TEST(PerformanceBentley, bmap)
    {
    //  Load in order
    if (true)
        {
        bmap<int,Struct> m;
        for (int i=0; i<s_niters; ++i)
            m[i].a = m[i].b = (double)i;

        // access out of order
        for (int i=s_niters-1; i>=0; --i)
            ASSERT_EQ (m[i].a, (double)i);

        // access in order
        for (int i=0; i<s_niters; ++i)
            ASSERT_EQ (m[i].a, (double)i);

        // remove isolated elements
        size_t wasSize = m.size();
        size_t nerased = 0;
        for (int i=0; i<s_niters; i += 10)
            {
            ASSERT_EQ( m.erase (i), 1 );
            ++nerased;
            }
        ASSERT_TRUE (m.size() == wasSize-nerased);

        // insert elements out of order
        for (int i=0; i<s_niters; i += 10)
            {
            ASSERT_TRUE( m.insert (make_bpair(i,Struct())).second == true );
            }
        }

    //  Load out of order    TBD...
    }
