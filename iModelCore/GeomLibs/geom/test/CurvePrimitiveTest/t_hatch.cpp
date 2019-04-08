//
//
#include "testHarness.h"
#include <GeomSerialization/GeomSerializationApi.h>

#include <Regions/regionsAPI.h>
#include <Regions/rimsbsAPI.h>
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

double s_batchSpace = 5.0;
void ExerciseHatch (CurveVectorR region, double angleDegrees = 30.0, double spacing = 2.0, double x0 = 0.0, double y0 = 0.0, bool convertToBspline = false)
    {
    if (convertToBspline)
        {
        auto region1 = region.CloneAsBsplines ();
        ExerciseHatch (*region1, angleDegrees, spacing, 0, 0, false);
        }
    else
        {
        DRange3d range;
        region.GetRange (range);
        SaveAndRestoreCheckTransform shifter (s_batchSpace + 2.0 * range.XLength (), 0, 0);
        DPoint3d startPoint = DPoint3d::From (x0, y0, 0);
        double angleRadians = Angle::DegreesToRadians (angleDegrees);
        bvector<DSegment3d> hatch;
        bvector<HatchSegmentPosition> positions;
        CurveVector::CreateXYHatch (hatch, &positions, region, startPoint, angleRadians, spacing);
        Check::SaveTransformed (region);
        Check::SaveTransformed (hatch);
        Check::Shift (2.0 * range.YLength (), 0);
        bvector<DSegment3d> dashes;
        //double a = 0.25 * range.XLength ();
        bvector<double> dashLengths {5,-2,2,-1};
        DashData dashData;
        dashData.SetDashLengths (dashLengths);
        dashData.AppendDashes (hatch,  positions, dashes);
        Check::SaveTransformed (dashes);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(XYHatch,Linear)
    {
    // an outer loop
    auto outerLoop = CurveVector::CreateLinear (bvector<DPoint3d> {
            DPoint3d::From (0,0),
            DPoint3d::From (20,0),
            DPoint3d::From (18,10),
            DPoint3d::From (1,10),
            DPoint3d::From (0,0)},
        CurveVector::BOUNDARY_TYPE_Outer
        );
    // a hole loop, input counterclockwise 
    auto holeA = CurveVector::CreateLinear (bvector<DPoint3d> {
            DPoint3d::From (2,2),
            DPoint3d::From (6,2),
            DPoint3d::From (6,5),
            DPoint3d::From (2,5),
            DPoint3d::From (2,2)},
        CurveVector::BOUNDARY_TYPE_Inner
        );
    // a hole loop, defined clockwise 
    auto holeB = CurveVector::CreateLinear (bvector<DPoint3d> {
            DPoint3d::From (12,2),
            DPoint3d::From (12,5),
            DPoint3d::From (16,5),
            DPoint3d::From (16,2),
            DPoint3d::From (12,2)},
        CurveVector::BOUNDARY_TYPE_Inner
        );

    // a hole loop, but it jumps outside ...
    auto holeC = CurveVector::CreateLinear (bvector<DPoint3d> {
            DPoint3d::From (8,8),
            DPoint3d::From (12,9),
            DPoint3d::From (12,12),
            DPoint3d::From (8,12),
            DPoint3d::From (8,8)},
        CurveVector::BOUNDARY_TYPE_Inner
        );

    auto parityRegion = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
    parityRegion->Add (outerLoop);
    parityRegion->Add (holeA);
    parityRegion->Add (holeB);
    parityRegion->Add (holeC);

    ExerciseHatch (*outerLoop);
    ExerciseHatch (*holeA);
    ExerciseHatch (*parityRegion);
    Check::ClearGeometry ("XYHatch.Linear");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(XYHatch,WithArcs)
    {
    for (auto asBspline : bvector<bool> { false, true})
        {
        auto lastTransform = Check::GetTransform ();
        for (auto spacing : {1.0, 2.0, 5.0})
            {
            SaveAndRestoreCheckTransform shifter (0, 500, 0);

            for (auto degrees : {0.0, 30.0, 120.0, 0.0, 90.0})
                {
                auto regionB = SampleGeometryCreator::CreateBoundaryWithAllCurveTypes (10.0);
                ExerciseHatch (*regionB, degrees, spacing, 0,0,asBspline);
                Check::Shift (100,0,0);  // regionA messes up spacing

                auto regionA = SampleGeometryCreator::CircleInRectangle (0,0, 10,
                            -11,-15, 18, 8);
                ExerciseHatch (*regionA, degrees, spacing, 0,0, asBspline);
                Check::Shift (100,0,0);  // regionA messes up spacing

                auto regionC = SampleGeometryCreator::CreateAnnulusWithManyArcSectors (5, 3);
                ExerciseHatch (*regionC, degrees, spacing, 0,0, asBspline);
                lastTransform = Check::GetTransform ();
                Check::Shift (40,0,0);  // regionA messes up spacing
                }
            }
        // scruffy way to get the x out of the last transform but back to 0 y ...
        lastTransform.form3d[1][3] = 0.0;
        Check::SetTransform (lastTransform);
        Check::Shift (50,0,0);
        Check::SaveTransformed (*SampleGeometryCreator::CreateSwoosh (0,0, 0, 50));
        Check::Shift (20,0,0);
        }
    Check::ClearGeometry ("XYHatch.WithArcs");
    }

