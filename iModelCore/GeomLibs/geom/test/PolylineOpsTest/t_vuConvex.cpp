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
// Source code (with complete implementation) for CurveVectorXYOffsetContext struct ...
// BEWARE -- .mke dependencies might not know about this ...
#include "CurveVectorXYOffsetContext.h"

void FractalA(bvector<DPoint3d> &points, int numRecursion, double perpendicularFactor);
void Fractal0(bvector<DPoint3d> &points, int numRecursion, double perpendicularFactor);

TEST(Vu, XYOffset)
    {
    bvector<bvector<DPoint3d>> interiorLoops;
    double a = 10.0;
    double b = 20.0;
    auto pointsA = CreateT(a, b, 4.0, 2.0, 10.0, 12.0);
    bvector<DPoint3d> fractalA;
    FractalA (fractalA, 1, 0.4);
    bvector<DPoint3d> fractalB;
    Fractal0(fractalB, 1, 0.4);
    bvector<bvector<DPoint3d>> allLoops;
    allLoops.push_back (pointsA);
    allLoops.push_back(fractalA);
    allLoops.push_back(fractalB);
    for (bvector<DPoint3d> &singleLoop : allLoops )
        {
        auto range = DRange3d::From(singleLoop);
        double ax = 0.1 * range.XLength();
        double bx = 0.1 * range.XLength();
        double ay = 0.03 * range.YLength();
        double by = 0.03 * range.YLength();
        SaveAndRestoreCheckTransform shifter (0, 2.0 * range.YLength (), 0);
        for (auto degrees : {0.0, 1.0, -1.0, 5.0, 8.0, 20.0, 40.0})
            {
            bvector<bvector<DPoint3d>> inputLoops;
            inputLoops.push_back (singleLoop);
            auto transform = Transform::FromMatrixAndFixedPoint(
                    RotMatrix::FromAxisAndRotationAngle(2, Angle::DegreesToRadians (degrees)),
                    DPoint3d::FromInterpolate (range.low, 0.5, range.high));
            transform.Multiply (inputLoops.back ());
            vu_createXYOffsetLoops(inputLoops, ax, bx, ay, by, interiorLoops);
            Check::SaveTransformed(inputLoops);
            Check::SaveTransformed(interiorLoops);
            Check::Shift (3.0 * range.XLength (), 0, 0);
            }
        }
    Check::ClearGeometry("Vu.XYOffset");
    }

TEST(CurveVector, XYOffset)
    {
    bvector<bvector<DPoint3d>> interiorLoops;
    double a = 10.0;
    double b = 20.0;
    auto pointsA = CreateT(a, b, 4.0, 2.0, 10.0, 12.0);
    bvector<DPoint3d> fractalA;
    FractalA(fractalA, 1, 0.4);
    bvector<DPoint3d> fractalB;
    Fractal0(fractalB, 1, 0.4);
    bvector<bvector<DPoint3d>> allLoops;
    allLoops.push_back(pointsA);
    allLoops.push_back(fractalA);
    allLoops.push_back(fractalB);
    allLoops.push_back(bvector<DPoint3d> {
        DPoint3d::From (0,0,0),
        DPoint3d::From (10,0,0),
        DPoint3d::From (10,10,0),
        DPoint3d::From (0,10,0),
        DPoint3d::From (0,8,0),
        DPoint3d::From (8,8,0),
        DPoint3d::From (8,2,0),
        DPoint3d::From (0,2,0),
        DPoint3d::From (0,0,0)
        });
    for (bvector<DPoint3d> &singleLoop : allLoops)
        {
        auto range = DRange3d::From(singleLoop);
        double ax = 0.1 * range.XLength();
        double bx = 0.1 * range.XLength();
        double ay = 0.03 * range.YLength();
        double by = 0.03 * range.YLength();
        DRange2d chop;
        static double s_expansionFactor = 1.001;
        chop.low = DPoint2d::From (-s_expansionFactor * bx, -s_expansionFactor * by);
        chop.high = DPoint2d::From(s_expansionFactor * ax, s_expansionFactor * ay);

        auto textBox = CurveVector::CreateRectangle(
            -bx, -by, ax, ay, 0.0);
        Check::SaveTransformed(textBox);

        auto transformBX = Transform::From (-bx, -by, 0);
        auto transformBY = Transform::From (-bx,  by, 0);
        auto transformAX = Transform::From (ax, -ay, 0);
        auto transformAY = Transform::From(ax, ay, 0);
        double dy = 1.5 * range.YLength ();
        for (auto degrees : { 0.0, 1.0, -1.0, 5.0, 20.0, 40.0, 60.0 })
            {
            SaveAndRestoreCheckTransform shifter(3.0 * range.XLength(), 0, 0);
            auto rotatedChop = textBox->Clone(
                    Transform::From (RotMatrix::FromAxisAndRotationAngle(2, Angle::DegreesToRadians (degrees))));
#ifdef ShowCurveOffsetSteps
            auto transform = Transform::FromMatrixAndFixedPoint(
                RotMatrix::FromAxisAndRotationAngle(2, Angle::DegreesToRadians(degrees)),
                DPoint3d::FromInterpolate(range.low, 0.5, range.high));
            auto shape = CurveVector::CreateLinear (singleLoop, CurveVector::BOUNDARY_TYPE_Outer);
            shape->TransformInPlace (transform);
            auto shapeBX = shape->Clone(transformBX);
            auto shapeBY = shape->Clone(transformBY);
            auto shapeAX = shape->Clone(transformAX);
            auto shapeAY = shape->Clone(transformAY);


            CurveVectorPtr minusBX = CurveVector::AreaIntersection (*shapeBX, *shape);
            CurveVectorPtr minusBXBY = CurveVector::AreaIntersection (*minusBX, *shapeBY);

            CurveVectorPtr minusBXBYAX = CurveVector::AreaIntersection(*minusBXBY, *shapeAX);
            CurveVectorPtr minusBXBYAXAY = CurveVector::AreaIntersection(*minusBXBYAX, *shapeAY);

            Check::SaveTransformed(shape);
            Check::Shift (0, dy, 0);
            Check::SaveTransformed (minusBX);
            Check::Shift(0, dy, 0);
            Check::SaveTransformed (shape);
            Check::SaveTransformed (minusBXBY);

            Check::Shift(0, dy, 0);
            Check::SaveTransformed(shape);
            Check::SaveTransformed(minusBXBYAXAY);

            auto chopper0 = CurveVector::Create (CurveVector::BOUNDARY_TYPE_UnionRegion);
            CurveVectorXYOffsetContext::AddRectanglesAtKeyPoints(*shape, chop, *chopper0);
            auto chopper = CurveVector::ReduceToCCWAreas (*chopper0);
            CurveVectorPtr minusChop = CurveVector::AreaDifference (*minusBXBYAXAY, *chopper);
            Check::Shift(0, 2 * dy, 0);
            Check::SaveTransformed(minusBXBYAXAY);
            Check::Shift(0, dy, 0);
            Check::SaveTransformed(shape);
            Check::SaveTransformed(minusChop);
            Check::Shift(0, dy, 0);
            Check::SaveTransformed(chopper);

            Check::Shift (0, 2.0 * dy, 0);
            auto result = CurveVectorXYOffsetContext::ComputeCenterRegionForRectangularBox (*shape, ax + bx, ay + by);
            Check::SaveTransformed (result);
#else
            auto shape = CurveVector::CreateLinear(singleLoop, CurveVector::BOUNDARY_TYPE_Outer);
            Check::SaveTransformed (shape);
            Check::Shift (0,0,0.01);
            auto result = CurveVectorXYOffsetContext::ComputeCenterRegionForRectangularBox(*shape, ax + bx, ay + by,
                        Angle::DegreesToRadians (degrees));
            if (result.IsValid ())
                {
                double spacing = result->FastLength () * 0.10;
                auto strokeOptions = IFacetOptions::CreateForCurves ();
                strokeOptions->SetMaxEdgeLength (spacing);
                bvector<DPoint3d> locations;
                result->AddStrokePoints(locations, *strokeOptions);
                // output some scattered boxes . ..
                for (auto &xyz : locations)
                    {
                    auto box = rotatedChop->Clone (Transform::From (xyz));
                    Check::SaveTransformed (box);
                    }

                Check::Shift(0, 2.0 * dy, 0);
                Check::SaveTransformed(shape);
                // Check::SaveTransformed(result);
                for (int effort : {0, 1, 10})
                    {
                    // and output points with various effort to get to the inside . . .
                    DPoint3d xyzA;
                    if (CurveVectorXYOffsetContext::ChooseBoxCenter (*shape, *result, effort, xyzA))
                        Check::SaveTransformed (*rotatedChop->Clone(Transform::From(xyzA)));
                    }
                }
#endif
            }
        }
    Check::ClearGeometry("CurvVector.XYOffset");
    }