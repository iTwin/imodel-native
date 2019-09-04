/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//
//
#include "testHarness.h"

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

void OutputCurves (bvector<MSBsplineCurvePtr> &curves, double shiftX = 0.0, double shiftY = 0.0, double shiftZ = 0.0)
    {
    Check::Shift (shiftX, shiftY, shiftZ);
    for (auto &curve : curves)
        {
        Check::SaveTransformed (curve);
        Check::True (curve->HasValidCountsAndAllocations ());
        }
    Check::Shift (-shiftX, -shiftY, -shiftZ);
    }
//static int s_noisy = 0;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MSBsplineCurve,MakeCompatible)
    {
    auto inputShift = Transform::From (DVec3d::From (0,0,1));
    bvector<double> knotsToAdd {0.10, 0.456, 0.69};
    for (bool closed : {false, true})
        {
        SaveAndRestoreCheckTransform shifter (0,100,0);
        for (uint32_t numAddedCurve : { 1, 4})
            {
            SaveAndRestoreCheckTransform shifter (100,0,0);
            for (int order : {3,4,5})
                {
                SaveAndRestoreCheckTransform shifter (0, 20,0);
                for (size_t numAdd = 0; numAdd <= knotsToAdd.size (); numAdd++)
                    {
                    SaveAndRestoreCheckTransform shifter (10,0,0);
                    auto curveA = MSBsplineCurve::CreateFromPolesAndOrder (
                                    bvector<DPoint3d> {
                                            DPoint3d::From (0,0,0),
                                            DPoint3d::From (1,0,0),
                                            DPoint3d::From (2,1,0),
                                            DPoint3d::From (3,1,0),
                                            DPoint3d::From (3,2,0),
                                            DPoint3d::From (0,1,0)},
                                            nullptr, nullptr,
                                            order, closed, true);
                    auto curveB = curveA->CreateCopyTransformed (inputShift);
                    for (size_t i = 0; i < numAdd; i++)
                        curveB->AddKnot (knotsToAdd[i], 1);
                    auto shiftK = inputShift;
                    bvector<MSBsplineCurvePtr> curves {curveA, curveB};
                    for (uint32_t k = 2; k <= numAddedCurve; k++)
                        {
                        shiftK = inputShift * shiftK;
                        curves.push_back (curveA->CreateCopyTransformed (shiftK));
                        }
                    bvector<MSBsplineCurvePtr> compatibleCurves;
                    MSBsplineCurve::CloneCompatibleCurves (compatibleCurves, curves, true, false);
                    OutputCurves (curves);
                    OutputCurves (compatibleCurves, 0, 4, 0);
                    }
                }
            }
        }
    Check::ClearGeometry ("MSBsplineCurve.MakeCompatible");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MSBsplineCurve,CloneArcLengthCompatible)
    {
    auto inputShift = Transform::From (DVec3d::From (0,0,1));
    bvector<double> knotsToAdd {0.10, 0.456, 0.69};
    for (bool closed : {false, true})
        {
        SaveAndRestoreCheckTransform shifter (0,100,0);
        for (uint32_t numAddedCurve : { 1, 4})
            {
            SaveAndRestoreCheckTransform shifter (100,0,0);
            for (int order : {3,4,5})
                {
                SaveAndRestoreCheckTransform shifter (0, 20,0);
                for (size_t numAdd = 0; numAdd <= knotsToAdd.size (); numAdd++)
                    {
                    SaveAndRestoreCheckTransform shifter (10,0,0);
                    auto curveA = MSBsplineCurve::CreateFromPolesAndOrder (
                                    bvector<DPoint3d> {
                                            DPoint3d::From (0,0,0),
                                            DPoint3d::From (1,0,0),
                                            DPoint3d::From (2,1,0),
                                            DPoint3d::From (3,1,0),
                                            DPoint3d::From (3,2,0),
                                            DPoint3d::From (0,1,0)},
                                            nullptr, nullptr,
                                            order, closed, true);
                    auto curveB = curveA->CreateCopyTransformed (inputShift);
                    for (size_t i = 0; i < numAdd; i++)
                        curveB->AddKnot (knotsToAdd[i], 1);
                    auto shiftK = inputShift;
                    bvector<MSBsplineCurvePtr> curves {curveA, curveB};
                    for (uint32_t k = 2; k <= numAddedCurve; k++)
                        {
                        shiftK = inputShift * shiftK;
                        curves.push_back (curveA->CreateCopyTransformed (shiftK));
                        }
                    bvector<MSBsplineCurvePtr> compatibleCurves;
                    MSBsplineCurve::CloneArcLengthCompatibleCurves (compatibleCurves, curves, true, false);
                    OutputCurves (curves);
                    OutputCurves (compatibleCurves, 0, 4, 0);
                    }
                }
            }
        }
    Check::ClearGeometry ("MSBsplineCurve.CloneArcLengthCompatible");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MSBsplineCurve,SurfaceKnots)
    {
    for (double weight : { 0.0, 0.9})
        {
        SaveAndRestoreCheckTransform shifter (20,0);
        auto surface = SurfaceWithSinusoidalControlPolygon (3, 4,
                    10, 7,
                    0.1, 0.76,
                    -0.2, 1.2, weight);
        Check::True (surface->HasValidCountsAndAllocations (), "Initial Surface");
        Check::SaveTransformed (surface);
        Check::Shift (0,15);
        surface->AddKnot (0.48, 1, 0);
        surface->AddKnot (0.94, 2, 1);
        Check::True (surface->HasValidCountsAndAllocations (), "After knot insertion");
        Check::SaveTransformed (surface);
        double tol = 1.0e-6;
        surface->RemoveKnotsBounded (1, tol);
        surface->RemoveKnotsBounded (2, tol);
        Check::True (surface->HasValidCountsAndAllocations (), "After knot removal");
        Check::Shift (0,15);
        Check::SaveTransformed (surface);
        }
    Check::ClearGeometry ("MSBsplineCurve.SurfaceKnots");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MSBsplineCurve,CreateLoft)
    {
    auto inputShift = Transform::From (DVec3d::From (0,0,1));
    bvector<double> knotsToAdd {0.10, 0.456, 0.69};
    for (bool chordLength : {false, true})
        {
        SaveAndRestoreCheckTransform shifter (0,100,0);
        for (uint32_t numAddedCurve : { 1, 4})
            {
            SaveAndRestoreCheckTransform shifter (100,0,0);
            // negative order triggers convert-to-rational on first curve ...
            for (int signedOrderRational : {3,4,5, -3, -4})
                {
                SaveAndRestoreCheckTransform shifter (0, 20,0);
                for (size_t numAdd = 0; numAdd <= knotsToAdd.size (); numAdd++)
                    {
                    SaveAndRestoreCheckTransform shifter (10,0,0);
                    int order = abs (signedOrderRational);
                    auto curveA = MSBsplineCurve::CreateFromPolesAndOrder (
                                    bvector<DPoint3d> {
                                            DPoint3d::From (0,0,0),
                                            DPoint3d::From (1,0,0),
                                            DPoint3d::From (2,1,0),
                                            DPoint3d::From (3,1,0),
                                            DPoint3d::From (3,2,0),
                                            DPoint3d::From (0,1,0)},
                                            nullptr, nullptr,
                                            order, false, true);
                    if (signedOrderRational < 0)
                        curveA->MakeRational ();
                    auto curveB = curveA->CreateCopyTransformed (inputShift);
                    for (size_t i = 0; i < numAdd; i++)
                        curveB->AddKnot (knotsToAdd[i], 1);
                    auto shiftK = inputShift;
                    bvector<MSBsplineCurvePtr> curves {curveA, curveB};
                    for (uint32_t k = 2; k <= numAddedCurve; k++)
                        {
                        shiftK = inputShift * shiftK;
                        curves.push_back (curveA->CreateCopyTransformed (shiftK));
                        }
                    MSBsplineSurface surface;
                    OutputCurves (curves);
                    double tolerance = 1.0e-3;
                    if (Check::True (SUCCESS == surface.InitLoftingSurface (curves, nullptr, nullptr,
                            true, false, false, false, chordLength, false, tolerance),
                            "Construct surface loft"))
                        {
                        auto surfacePtr = surface.CreateCapture ();
                        Check::SaveTransformed (surfacePtr);
                        }
                    }
                }
            }
        }
    Check::ClearGeometry ("MSBsplineCurve.CreateLoft");
    }
