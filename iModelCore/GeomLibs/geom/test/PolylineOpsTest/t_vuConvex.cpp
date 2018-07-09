#include "testHarness.h"
#include <Vu/vuprint.fdf>
// unused - static int s_noisy = 0;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Vu,ConvexParts)
    {
    double a = 10.0;
    double b = 2.0;
    double c = 4.0;
    double d = 5.0;
    double e = 6.0;

    double f = 4.5;

    bvector<bvector<DPoint3d>> allLoops {
        bvector<DPoint3d> {
            DPoint3d::From (0,0,0),
            DPoint3d::From (a,0,0),
            DPoint3d::From (a,a,0),
            DPoint3d::From (0,a,0),
            },
        bvector<DPoint3d> {
            DPoint3d::From (b,b,0),
            DPoint3d::From (c,b,0),
            DPoint3d::From (c,c,0),
            DPoint3d::From (b,c,0),
            },
        bvector<DPoint3d> {
            DPoint3d::From (d,b,0),
            DPoint3d::From (e,b,0),
            DPoint3d::From (e,c,0),
            DPoint3d::From (d,c,0),
            },
        bvector<DPoint3d> {
            DPoint3d::From (f,d,0),
            DPoint3d::From (e,e,0),
            DPoint3d::From (f,e,0),
            DPoint3d::From (f,d,0),
            },
            // Wow - exact duplicate of [2] -- parity cancels, convex expansion starts 
            // somewhere that fortuitously floods it away.
        bvector<DPoint3d> {
            DPoint3d::From (d,b,0),
            DPoint3d::From (e,b,0),
            DPoint3d::From (e,c,0),
            DPoint3d::From (d,c,0),
            },
        };
    bvector<bvector<DPoint3d>> activeLoops, convexLoops;
    bvector<bvector<bool>> isBoundary;

    for (auto &loop : allLoops)
        {
        SaveAndRestoreCheckTransform shifter (2*a, 0,0);
        activeLoops.push_back (loop);
        vu_splitToConvexParts (activeLoops, 1, convexLoops, &isBoundary);
        Check::SaveTransformed (activeLoops);
        Check::Shift (0, 2.0 * a, 0);
        Check::SaveTransformed (convexLoops);
        }
    Check::ClearGeometry ("Vu.ConvexParts");
    }
