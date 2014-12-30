/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/bvector_performanceTest.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/bvector.h>

    static int s_niters = 10000;

    struct Struct {double a, b;};

TEST(PerformanceBentley, bvector)
    {
    // push_back and grow
    if (true)
        {
        bvector<int> v;
        for (int i=0; i<s_niters; ++i)
            v.push_back (i);
        ASSERT_EQ (v.size(), s_niters);

        bvector<Struct> vs;
        for (int i=0; i<s_niters; ++i)
            vs.push_back (Struct());
        ASSERT_EQ (v.size(), s_niters);
        }

    // push_back within allocated space
    if (true)
        {
        bvector<int> v;
        v.reserve (s_niters);
        for (int i=0; i<s_niters; ++i)
            v.push_back (i);
        ASSERT_EQ (v.size(), s_niters);

        bvector<Struct> vs;
        v.reserve (s_niters);
        for (int i=0; i<s_niters; ++i)
            vs.push_back (Struct());
        ASSERT_EQ (v.size(), s_niters);
        }

    //  indexing
    if (true)
        {
        bvector<double> v (s_niters);
        for (int i=0; i<s_niters; ++i)
            v[i] = (double)i;
        for (int i=0; i<s_niters; ++i)
            ASSERT_EQ (v[i], (double)i);

        bvector<Struct> vs (s_niters);
        for (int i=0; i<s_niters; ++i)
            vs[i].a = vs[i].b = (double)i;
        for (int i=0; i<s_niters; ++i)
            ASSERT_EQ (vs[i].a, (double)i);
        }

    //  indexing (down)
    if (true)
        {
        bvector<double> v (s_niters);
        for (int i=s_niters-1; i>=0; --i)
            v[i] = (double)i;
        for (int i=s_niters-1; i>=0; --i)
            ASSERT_EQ (v[i], (double)i);

        bvector<Struct> vs (s_niters);
        for (int i=s_niters-1; i>=0; --i)
            vs[i].a = vs[i].b = (double)i;
        for (int i=s_niters-1; i>=0; --i)
            ASSERT_EQ (vs[i].a, (double)i);


        // find
        for (int i=0; i<s_niters; ++i)
            {
            bvector<double>::const_iterator it = std::find (v.begin(), v.end(), (double)(s_niters/2));
            ASSERT_TRUE( it != v.end() );
            ASSERT_EQ( *it, (double)(s_niters/2) );
            }
        }
    }