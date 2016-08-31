#include "../BackDoor/PublicApi/BackDoor/RoadRailAlignment/BackDoor.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sandy.Bugai                      05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RoadRailAlignmentTests, AlignmentGeometry)
    {
    // check offset of a curve with a spiral. This case is from TFS#423852
    char *curveStr = R"({"DgnCurveVector":{"Member":[{"CircularArc":{"placement":{"origin":[468940.85468616715, 2259430.2965899725, 0.0], "vectorX" : [-0.99819014094556791, -0.060136864892240141, 0.0], "vectorZ" : [0.0, 0.0, 1.0]}, "radius" : 329.34404740424247, "startAngle" : 61.642612721213297, "sweepAngle" : 4.7840624890499379}}, { "BsplineCurve":{"ControlPoint":[[468827.53393027303, 2259121.0622001579, 0.0], [468828.31526324566, 2259120.7758756098, 0.0], [468829.09896944772, 2259120.4920414952, 0.0], [468829.88497324131, 2259120.2105475673, 0.0]], "Knot" : [0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0], "closed" : false, "order" : 4} }, { "CircularArc":{"placement":{"origin":[468845.28819008218, 2259163.2689245688, 0.0], "vectorX" : [-0.14106343603729257, -0.99000055909749507, 0.0], "vectorZ" : [-0.0, 0.0, -1.0]}, "radius" : 45.720000000217716, "startAngle" : 11.574195194489512, "sweepAngle" : -11.574195194489512} }], "boundaryType" : 1}})";
    char *baselineStr = R"({"DgnCurveVector":{"Member":[{"CircularArc":{"placement":{"origin":[468940.85468616715,2259430.2965899725,0.0],"vectorX":[-0.99819014094556791,-0.060136864892240148,0.0],"vectorZ":[0.0,0.0,1.0]},"radius":326.90564740424247,"startAngle":61.642612721213297,"sweepAngle":4.7840624890499379}},{"LineString":{"Point":[[468828.37293664494,2259123.3517116490,0.0],[468829.15017972433,2259123.0679936092,0.0],[468829.92824471957,2259122.7861592383,0.0],[468830.70711154520,2259122.5061701778,0.0]]}},{"CircularArc":{"placement":{"origin":[468845.28819008218,2259163.2689245688,0.0],"vectorX":[-0.14106343603729257,-0.99000055909749496,0.0],"vectorZ":[-0.0,0.0,-1.0]},"radius":43.281600000217715,"startAngle":11.574195194489512,"sweepAngle":-11.574195194489512}}],"boundaryType":1}})";

    CurveVectorPtr curve = GeometryHelper::DeserializeCurveVector(curveStr);
    CurveOffsetOptions offsetOptionsRS(-2.4384000000000006);
    CurveVectorPtr offset = GeometryHelper::CloneOffsetCurvesXYNoBSpline(curve, offsetOptionsRS);
    GeometryDebug::Announce(*offset, "offset1");

    CurveVectorPtr expected = GeometryHelper::DeserializeCurveVector(baselineStr);
    ASSERT_TRUE(expected->IsSameStructureAndGeometry(*offset));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RoadRailAlignmentTests, BasicAlignmentTest)
    {
    DgnModelId modelId;
    DgnViewId viewId;
    DgnDbPtr projectPtr = CreateProject(L"BasicAlignmentTest.bim", modelId, viewId);
    }