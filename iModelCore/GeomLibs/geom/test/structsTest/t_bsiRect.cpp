/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <Bentley/BeNumerical.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  7/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BSIRect,Hello)
    {
    int x0 = 1;
    int x1 = 8;
    int x2 = 10;
    int x3 = 12;

    int y0 = -1;
    int y1 = 10;
    int y2 = 12;
    int y3 = 16;

    auto point0 = Point2d::From (x0, y0);
    auto point1 = Point2d::From (x1, y1);
    auto point2 = Point2d::From (x2, y2);
    auto r01 = BSIRect::From (x0, y0, x1, y1);
    auto q01 = BSIRect::From (&point0, &point1);
    Check::True (r01.IsEqual (&q01), "BSIRect::From");
    Check::Int (x0, r01.Left (), "Left");
    Check::Int (x1, r01.Right (), "Right");
    Check::Int (y1, r01.Bottom (), "Bottom");   // screen coordinates go down
    Check::Int (y0, r01.Top (), "Top");         // screen coordinates go down
    BSIRect r12, q12;
    r12.Init (x1, y1, x2, y2);
    q12.Init (&point1, &point2);
    Check::False (r01.IsEqual (&r12), "unequal BSIRect");

    auto r01A = r01;
    int insetX = 2;
    int insetY = 1;
    r01A.Inset (insetX, insetY);
    Check::Int (r01.Area (), r01A.Area ()
                + 2 * r01A.Height () * insetX
                + 2 * r01A.Width () * insetY
                + 4 * insetX * insetY, " Area of inset");
    r01A.Expand (insetY);
    auto r012 = r01;
    r012.Union(&r12);
    Check::True (r012.PointInside (point1), "PointInside");
    auto rLarge = BSIRect::From (x0, y0, x3, y3);

    auto r = BSIRect::From (x0, y1, x3, y2);
    Check::False (r.IsContained (&r01));
    Check::True (r12.IsContained (&rLarge));

    BSIRect overlap;
    BSIRect limit = r;
    Check::True (r.Overlap (nullptr, &r012));
    Check::True (r.Overlap (&overlap, &r012));
    limit.Limit (&r012);
    Check::True (limit.IsEqual (&overlap));

    auto rZ = r012;
    rZ.Inset (20,40);
    int dx = 2;
    int dy = 3;
    auto w = limit.Width ();
    auto h = limit.Height ();
    limit.Offset (dx, dy);
    auto w1 = limit.Width ();
    auto h1 = limit.Height ();
    // note offset moves both corners by dx,dy -- it is not an "expand" outwards
    Check::Int (w, w1);
    Check::Int (h, h1);

    rZ = r012;
    r012.Limit (&limit);
    }