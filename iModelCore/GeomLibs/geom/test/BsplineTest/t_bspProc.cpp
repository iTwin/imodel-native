//
//
#include "testHarness.h"

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL
//static int s_noisy = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MSBspline,RecursiveOffset)
    {
    auto bcurve = MSBsplineCurve::CreateFromPolesAndOrder
        (
        bvector<DPoint3d>
            {
            DPoint3d::From (0,0),
            DPoint3d::From (3,0),
            DPoint3d::From (5,0),
            DPoint3d::From (7,2),
            DPoint3d::From (9,6),
            DPoint3d::From (9,10),
            DPoint3d::From (11,14),
            DPoint3d::From (13,15)
            },
        nullptr,
        nullptr,
        3,
        false,
        true
        );
    auto cpCurve = ICurvePrimitive::CreateBsplineCurve (bcurve);

    double tolerance = 1.0e-4;
    for (int select = 0; select < 3; select++)
        {
        Check::SaveTransformed (*cpCurve);
        for (double offsetDistance = 1.0; fabs (offsetDistance) < 8.0;
                    offsetDistance = 
                        offsetDistance > 0 ? -offsetDistance : -offsetDistance + 1.0
                        )
            {
            MSBsplineCurve offsetCurve;

            if (select == 0 && Check::Int (SUCCESS, bsprcurv_offset
                    (
                    &offsetCurve,
                    bcurve.get(),
                    offsetDistance,
                    0,
                    2,
                    tolerance,
                    nullptr
                    )))
                {
                auto cpOffset = ICurvePrimitive::CreateBsplineCurveSwapFromSource (offsetCurve);
                Check::SaveTransformed (*cpOffset);
                }
            if (select == 1 && Check::Int (SUCCESS, bsprcurv_offset
                    (
                    &offsetCurve,
                    bcurve.get(),
                    offsetDistance,
                    0,
                    1,
                    tolerance,
                    nullptr
                    )))
                {
                auto cpOffset = ICurvePrimitive::CreateBsplineCurveSwapFromSource (offsetCurve);
                Check::SaveTransformed (*cpOffset);
                }
            else if (select == 2)
                {
                CurveOffsetOptions options (offsetDistance);
                auto bcurve1 = bcurve->CreateCopyOffsetXY (offsetDistance, offsetDistance, options);
                if (bcurve1.IsValid ())
                    {
                    auto cpOffset = ICurvePrimitive::CreateBsplineCurve (bcurve1);
                    Check::SaveTransformed (*cpOffset);
                    }
                }
            }
        Check::Shift (20,0,0);
        }
    Check::ClearGeometry ("MSBsplineCurve.RecursiveOffset");
    }