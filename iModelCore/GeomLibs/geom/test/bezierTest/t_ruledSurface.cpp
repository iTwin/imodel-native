#include "testHarness.h"

// ASSUME surface has functional z
void CheckRuledXYPierce
(
DPoint3dCP coffA,
DPoint3dCP coffB,
int order,
double u,
double v
)
    {
    DPoint3d xyz;
    DVec3d dXdu, dXdv;
    bsiBezier_evaluateRuled (u, v, coffA, coffB, order, xyz, dXdu, dXdv);
    bvector<SolidLocationDetail>pickData;
    DPoint2d xy = DPoint2d::From (xyz.x, xyz.y);
    bsiBezier_addRuledPatchXYIntersections (pickData, xy, coffA, coffB, order);
    Check::Size (1, pickData.size (), "pick count");
    if (pickData.size () == 1)
        {
        Check::Near (xyz, pickData[0].GetXYZ (), "pick recovery");
        }

    }

void CheckBilinear (DPoint3dCP coffA, DPoint3dCP coffB)
    {
    for (double u = 0; u <= 1.0; u+= 0.25)
        {
        for (double v = 0; v < 1.0; v += 0.25)
            {
            DPoint3d xyzBez;
            DVec3d dXBezdu, dXBezdv;
            DPoint3d xyz0v = DPoint3d::FromInterpolate (coffA[0], v, coffB[0]);
            DPoint3d xyz1v = DPoint3d::FromInterpolate (coffA[1], v, coffB[1]);
            DPoint3d xyzu0 = DPoint3d::FromInterpolate (coffA[0], u, coffA[1]);
            DPoint3d xyzu1 = DPoint3d::FromInterpolate (coffB[0], u, coffB[1]);
            DPoint3d xyz   = DPoint3d::FromInterpolate (xyz0v, u, xyz1v);
            bsiBezier_evaluateRuled (u, v, coffA, coffB, 2, xyzBez, dXBezdu, dXBezdv);
            DVec3d   dXdv  = DVec3d::FromStartEnd (xyzu0, xyzu1);
            DVec3d   dXdu  = DVec3d::FromStartEnd (xyz0v, xyz1v);
            Check::Near (xyz, xyzBez, "bilinear ruled");
            Check::Near (dXdv, dXBezdv, "bilinear ruled");
            Check::Near (dXdu, dXBezdu, "bilinear ruled");
            }
        }
    }

void CheckRuledPierce (DPoint3dCP coffA, DPoint3dCP coffB, int order)
    {
    for (double u = 0; u <= 1.0; u+= 0.25)
        {
        for (double v = 0; v < 1.0; v += 0.25)
            {
            if (u != 0.0 && v != 0.0)
                CheckRuledXYPierce (coffA, coffB, order, u, v);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RuledBezier,Bilinear0)
    {
    DPoint3d coffA[] = 
        {
        {1,2},
        {3,2}, 
        };

    DPoint3d coffB[] = 
        {
        {1,4},
        {3,4}, 
        };

    CheckBilinear (coffA, coffB);
    CheckRuledPierce (coffA, coffB, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RuledBezier,Bilinear1)
    {
    DPoint3d coffA[] = 
        {
        {1,2},
        {3,-1}, 
        };

    DPoint3d coffB[] = 
        {
        {1,5},
        {4,4}, 
        };

    CheckBilinear (coffA, coffB);
    CheckRuledPierce (coffA, coffB, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RuledBezier,Quad0)
    {
    DPoint3d coffA[] = 
        {
        {1,2},
        {3,-1}, 
        {5, -1}
        };

    DPoint3d coffB[] = 
        {
        {1,5},
        {4,4},
        {5, 4}
        };

    CheckRuledPierce (coffA, coffB, 3);
    }
