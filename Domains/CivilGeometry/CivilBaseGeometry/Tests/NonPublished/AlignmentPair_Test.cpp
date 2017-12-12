/*--------------------------------------------------------------------------------------+
|
|  $Source: CivilBaseGeometry/Tests/NonPublished/AlignmentPair_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestHeader.h"

#define ENABLE_SELF_INTERSECT_TEST 0
#define METERS_TO_ENGLISH_FEET 3.28083989501
#define METERS_TO_ENGLISH_SURVEY_FEET 3.2808333333465

//---------------------------------------------------------------------------------------
// Deserialize helper
//---------------------------------------------------------------------------------------
CurveVectorPtr deserializeCurve(Utf8CP data)
    {
    bvector<IGeometryPtr> geometries;
    if (!BentleyGeometryJson::TryJsonStringToGeometry(data, geometries))
        return nullptr;

    if (1 != geometries.size() || !geometries.front().IsValid() || IGeometry::GeometryType::CurveVector != geometries.front()->GetGeometryType())
        return nullptr;

    return geometries.front()->GetAsCurveVector();
    }
//---------------------------------------------------------------------------------------
// Serialized test case
//---------------------------------------------------------------------------------------
CurveVectorPtr loadHorizontalWith3Arcs()
    {
    char* data = R"({"DgnCurveVector":{"Member":[{"LineSegment":{"endPoint":[468527.09628635721,2259195.8327451251,0.0],"startPoint":[468467.05241349683,2259133.0569764804,0.0]}},{"CircularArc":{"placement":{"origin":[468660.06504850514,2259068.6505596871,0.0],"vectorX":[-0.72265631602204794,0.69120752955493914,0.0],"vectorZ":[0.0,0.0,-1.0]},"radius":183.99999999983672,"startAngle":0.0,"sweepAngle":53.716150749835080}},{"LineSegment":{"endPoint":[468810.71412946295,2259234.5356240273,0.0],"startPoint":[468683.89693985571,2259251.1006650710,0.0]}},{"CircularArc":{"placement":{"origin":[468834.54602081364,2259416.9857294112,0.0],"vectorX":[-0.12952114864504249,-0.99157665969589504,0.0],"vectorZ":[0.0,0.0,1.0]},"radius":183.99999999994705,"startAngle":0.0,"sweepAngle":44.478811234937702}},{"LineSegment":{"endPoint":[469012.89954726340,2259321.0600332650,0.0],"startPoint":[468945.37457296526,2259270.1081204033,0.0]}},{"CircularArc":{"placement":{"origin":[469123.72809941485,2259174.1824242566,0.0],"vectorX":[-0.6023290877793640,0.79824787504563977,0.0],"vectorZ":[0.0,0.0,-1.0]},"radius":184.00000000007492,"startAngle":0.0,"sweepAngle":27.666984995263061}},{"LineSegment":{"endPoint":[469321.36029021174,2259393.2817337015,0.0],"startPoint":[469093.77147908445,2259355.7274624659,0.0]}}],"boundaryType":1}})";
    return deserializeCurve(data);
    }
//---------------------------------------------------------------------------------------
// Serialized test case
//---------------------------------------------------------------------------------------
CurveVectorPtr loadHorizontalWith3Spirals()
    {
    char* data = R"({"DgnCurveVector":{"Member":[{"LineSegment":{"endPoint":[468561.57954157837,2258790.8192921854,0.0],"startPoint":[468480.62028229458,2258647.4757073857,0.0]}},{"TransitionSpiral":{"ActiveEndFraction":1.0,"ActiveStartFraction":0.0,"EndBearing":57.664927528731255,"EndRadius":-667.0,"SpiralType":"Clothoid","StartBearing":60.542601612221894,"StartRadius":0.0,"placement":{"origin":[468561.57954157837,2258790.8192921854,0.0],"vectorX":[1.0,0.0,0.0],"vectorZ":[0.0,0.0,1.0]}}},{"CircularArc":{"placement":{"origin":[469159.06811088009,2258491.8333408381,0.0],"vectorX":[-0.84493458172406199,0.53486965945619358,0.0],"vectorZ":[0.0,0.0,-1.0]},"radius":666.99999999985437,"startAngle":0.0,"sweepAngle":24.139401076219713}},{"TransitionSpiral":{"ActiveEndFraction":1.0,"ActiveStartFraction":0.0,"EndBearing":30.647852369032059,"EndRadius":0.0,"SpiralType":"Clothoid","StartBearing":33.525526452522698,"StartRadius":-667.0,"placement":{"origin":[468790.67837881838,2259047.8711141152,0.0],"vectorX":[1.0,0.0,0.0],"vectorZ":[0.0,0.0,1.0]}}},{"LineSegment":{"endPoint":[468988.41144292668,2259166.3367093648,0.0],"startPoint":[468847.73336211150,2259082.9812612082,0.0]}},{"TransitionSpiral":{"ActiveEndFraction":1.0,"ActiveStartFraction":0.0,"EndBearing":33.525526452501545,"EndRadius":667.0,"SpiralType":"Clothoid","StartBearing":30.647852369010899,"StartRadius":0.0,"placement":{"origin":[468988.41144292668,2259166.3367093648,0.0],"vectorX":[1.0,0.0,0.0],"vectorZ":[0.0,0.0,1.0]}}},{"CircularArc":{"placement":{"origin":[468677.07669415820,2259757.4846297354,0.0],"vectorX":[0.55230844387049549,-0.83363983999647706,0.0],"vectorZ":[0.0,0.0,1.0]},"radius":666.99999999995248,"startAngle":0.0,"sweepAngle":8.9699916153508799}},{"TransitionSpiral":{"ActiveEndFraction":1.0,"ActiveStartFraction":0.0,"EndBearing":45.373192151325881,"EndRadius":0.0,"SpiralType":"Clothoid","StartBearing":42.495518067835242,"StartRadius":667.0,"placement":{"origin":[469127.65689328074,2259265.685398180,0.0],"vectorX":[1.0,0.0,0.0],"vectorZ":[0.0,0.0,1.0]}}},{"LineSegment":{"endPoint":[469275.47792368190,2259413.8482709471,0.0],"startPoint":[469175.50975162716,2259312.5692706262,0.0]}},{"TransitionSpiral":{"ActiveEndFraction":1.0,"ActiveStartFraction":0.0,"EndBearing":42.495518067802706,"EndRadius":-667.0,"SpiralType":"Clothoid","StartBearing":45.373192151293345,"StartRadius":0.0,"placement":{"origin":[469275.47792368190,2259413.8482709471,0.0],"vectorX":[1.0,0.0,0.0],"vectorZ":[0.0,0.0,1.0]}}},{"CircularArc":{"placement":{"origin":[469773.91098115058,2258968.9329118370,0.0],"vectorX":[-0.67553253241685018,0.73733018224298774,0.0],"vectorZ":[0.0,0.0,-1.0]},"radius":667.00000000023249,"startAngle":0.0,"sweepAngle":6.2975135781208698}},{"TransitionSpiral":{"ActiveEndFraction":1.0,"ActiveStartFraction":0.0,"EndBearing":33.320330406195502,"EndRadius":0.0,"SpiralType":"Clothoid","StartBearing":36.198004489686149,"StartRadius":-667.0,"placement":{"origin":[469379.99574711901,2259507.1891597323,0.0],"vectorX":[1.0,0.0,0.0],"vectorZ":[0.0,0.0,1.0]}}},{"LineSegment":{"endPoint":[469577.74622473511,2259638.5295264777,0.0],"startPoint":[469435.35160885868,2259544.9214021587,0.0]}}],"boundaryType":1}})";
    return deserializeCurve(data);
    }


//---------------------------------------------------------------------------------------
// toleranced asserts
//---------------------------------------------------------------------------------------
void expectEqualCurves(CurveVectorCR expected, CurveVectorCR actual)
    {
    ASSERT_EQ(expected.size(), actual.size());

    for (size_t i = 0; i < expected.size(); ++i)
        {
        ICurvePrimitivePtr ePrimitive = expected[i];
        ICurvePrimitivePtr aPrimitive = actual[i];

        EXPECT_EQ(ePrimitive->GetCurvePrimitiveType(), aPrimitive->GetCurvePrimitiveType());
        DPoint3d eStart, eEnd, aStart, aEnd;
        ePrimitive->GetStartEnd(eStart, eEnd);
        aPrimitive->GetStartEnd(aStart, aEnd);

        EXPECT_EQ_DPOINT3D(eStart, aStart);
        EXPECT_EQ_DPOINT3D(eEnd, aEnd);
        }
    }

//---------------------------------------------------------------------------------------
// Creates an line alignment with:
//  hz: (0,0,0) -> (100,0,0)
//  vt: (0,0,800) -> (100,0,815)
//---------------------------------------------------------------------------------------
AlignmentPairPtr createLinearPair()
    {
    CurveVectorPtr hz = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    hz->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::From(0, 0, 0), DPoint3d::From(100.0, 0, 0))));

    CurveVectorPtr vt = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    vt->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::From(0.0, 0.0, 800.0), DPoint3d::From(100.0, 0.0, 815.0))));

    return AlignmentPair::Create(*hz, vt.get());
    }
//---------------------------------------------------------------------------------------
// Creates an line alignment with:
//  hz: (0,0,0) -> (100,0,0)
//  vt: (0,0,1000) -> (115.0,0,1115)  slope = 100%
//---------------------------------------------------------------------------------------
AlignmentPairPtr createLinearPairWithLongerVt()
    {
    CurveVectorPtr hz = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    hz->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::From(0, 0, 0), DPoint3d::From(100.0, 0, 0))));

    CurveVectorPtr vt = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    vt->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::From(0,0,1000), DPoint3d::From(115, 0, 1115))));

    return AlignmentPair::Create(*hz, vt.get());
    }
//---------------------------------------------------------------------------------------
// Creates an line alignment with:
//  hz: (0,0,0) -> (100,0,0)
//  vt: (0,0,1000) -> (85.0,0,1085)  slope = 100%
//---------------------------------------------------------------------------------------
AlignmentPairPtr createLinearPairWithShorterVt()
    {
    CurveVectorPtr hz = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    hz->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::From(0, 0, 0), DPoint3d::From(100.0, 0, 0))));

    CurveVectorPtr vt = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    vt->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::From(0,0,1000), DPoint3d::From(85, 0, 1085))));

    return AlignmentPair::Create(*hz, vt.get());
    }


//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        10/2016
//---------------------------------------------------------------------------------------
void AlignmentPair_Create()
    {
    CurveVectorPtr hz = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);

    //! Empty hz
    AlignmentPairPtr pair = AlignmentPair::Create(*hz, nullptr);
    EXPECT_TRUE(pair.IsValid()) << "Pair with empty hz is considered valid";

    //! Hz with single line primitive
    hz->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(100, 0, 0))));
    AlignmentPairPtr pair2 = AlignmentPair::Create(*hz, nullptr);
    EXPECT_TRUE(pair2.IsValid()) << "Failed to create pair with single hz primitive (no vt)";

    //! Empty but valid vt
    CurveVectorPtr vt = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    AlignmentPairPtr pair3 = AlignmentPair::Create(*hz, vt.get());
    EXPECT_TRUE(pair3.IsValid());
    EXPECT_FALSE(pair3->IsValidVertical()) << "Pair created with valid empty vt should set vt as null..";

    //! vt with single line primitive
    vt->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::From(0.0, 0.0, 800.0), DPoint3d::From(100.0, 0.0, 750.0))));
    AlignmentPairPtr pair4 = AlignmentPair::Create(*hz, vt.get());
    EXPECT_TRUE(pair4.IsValid());
    EXPECT_TRUE(pair4->IsValidVertical());

    //! Hz with invalid boundary type
    hz->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Inner);
    AlignmentPairPtr pair5 = AlignmentPair::Create(*hz, nullptr);
    EXPECT_TRUE(pair5.IsValid());
    EXPECT_EQ(CurveVector::BOUNDARY_TYPE_Open, pair5->GetHorizontalCurveVector().GetBoundaryType());

    hz->SetBoundaryType(CurveVector::BOUNDARY_TYPE_None);
    AlignmentPairPtr pair6 = AlignmentPair::Create(*hz, nullptr);
    EXPECT_TRUE(pair6.IsValid());
    EXPECT_EQ(CurveVector::BOUNDARY_TYPE_Open, pair6->GetHorizontalCurveVector().GetBoundaryType());

    hz->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Open);

    //! Vt with invalid boundary type
    vt->SetBoundaryType(CurveVector::BOUNDARY_TYPE_None);
    AlignmentPairPtr pair7 = AlignmentPair::Create(*hz, vt.get());
    EXPECT_TRUE(pair7.IsValid());
    EXPECT_EQ(CurveVector::BOUNDARY_TYPE_Open, pair7->GetVerticalCurveVector()->GetBoundaryType());

    vt->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Outer);
    AlignmentPairPtr pair8 = AlignmentPair::Create(*hz, vt.get());
    EXPECT_TRUE(pair8.IsValid());
    EXPECT_EQ(CurveVector::BOUNDARY_TYPE_Open, pair8->GetVerticalCurveVector()->GetBoundaryType());

    // Make sure the helper methods return valid results!
    ASSERT_TRUE(createLinearPair().IsValid());
    ASSERT_TRUE(createLinearPairWithLongerVt().IsValid());
    ASSERT_TRUE(createLinearPairWithShorterVt().IsValid());

    // This is just to make sure coverage picks it up.
    pair->InitLazyCaches();
    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        10/2016
//---------------------------------------------------------------------------------------
void AlignmentPair_GetCurveVectors()
    {
    CurveVectorPtr hz = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    hz->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::From(0, 0, 0), DPoint3d::From(100.0, 0, 0))));

    CurveVectorPtr vt = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    vt->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::From(0.0, 0.0, 800.0), DPoint3d::From(100.0, 0.0, 815.0))));

    AlignmentPairPtr pair = AlignmentPair::Create(*hz, vt.get());
    ASSERT_TRUE(pair.IsValid());

    expectEqualCurves(*hz, pair->GetHorizontalCurveVector());
    expectEqualCurves(*vt, *pair->GetVerticalCurveVector());

    // Try to retrieve empty vertical
    pair = AlignmentPair::Create(*hz, nullptr);
    ASSERT_TRUE(pair.IsValid());
    EXPECT_TRUE(nullptr == pair->GetVerticalCurveVector());

    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        10/2016
//---------------------------------------------------------------------------------------
void AlignmentPair_UpdateCurveVectors()
    {
    AlignmentPairPtr pair = createLinearPair();

    CurveVectorPtr hz = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    hz->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::From(18235, 124.6), DPoint3d::From(18635, 124.6))));

    CurveVectorPtr vt = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    vt->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::From(0.0, 0.0, 15.0), DPoint3d::From(400.0, 0.0, 19.4))));

    //! Update hz
    pair->UpdateHorizontalCurveVector(*hz);
    expectEqualCurves(*hz, pair->GetHorizontalCurveVector());

    //! Update vt
    pair->UpdateVerticalCurveVector(vt.get());
    ASSERT_TRUE(pair->IsValidVertical());
    expectEqualCurves(*vt, *pair->GetVerticalCurveVector());

    pair->UpdateVerticalCurveVector(nullptr);
    EXPECT_FALSE(pair->IsValidVertical());
    
    CurveVectorPtr emptyVt = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    pair->UpdateVerticalCurveVector(emptyVt.get());
    EXPECT_FALSE(pair->IsValidVertical());

    //! Update both
    pair = createLinearPair();
    pair->UpdateCurveVectors(*hz, vt.get());
    expectEqualCurves(*hz, pair->GetHorizontalCurveVector());
    expectEqualCurves(*vt, *pair->GetVerticalCurveVector());

    //! Update both setting empty hz and null as vt
    pair = createLinearPair();
    hz->clear();
    pair->UpdateCurveVectors(*hz, nullptr);
    
    expectEqualCurves(*hz, pair->GetHorizontalCurveVector());
    EXPECT_FALSE(pair->IsValidVertical());
    }

//---------------------------------------------------------------------------------------
// @betest                               Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
void AlignmentPair_CloneCurveVectors()
    {
    CurveVectorPtr hzCurve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    AlignmentPairPtr pair = AlignmentPair::Create(*hzCurve, nullptr);
    ASSERT_TRUE(pair.IsValid());

    // Clone empty hz / no vertical
    CurveVectorPtr hz = pair->CloneHorizontalCurveVector();
    EXPECT_TRUE(hz.IsValid());
    CurveVectorPtr vt = pair->CloneVerticalCurveVector();
    EXPECT_FALSE(vt.IsValid());

    pair = createLinearPair();

    // Clone as Meters
    hz = pair->CloneHorizontalCurveVector();
    ASSERT_TRUE(hz.IsValid());
    expectEqualCurves(pair->GetHorizontalCurveVector(), *hz);

    vt = pair->CloneVerticalCurveVector();
    ASSERT_TRUE(vt.IsValid());
    expectEqualCurves(*pair->GetVerticalCurveVector(), *vt);

    // Clone as EnglishFeet
    DPoint3d start, end;

    hz = pair->CloneHorizontalCurveVector(StandardUnit::EnglishFeet);
    ASSERT_TRUE(hz.IsValid());
    EXPECT_TRUE(hz->GetStartEnd(start, end));
    EXPECT_EQ_DPOINT3D(DPoint3d::FromZero(), start);
    EXPECT_EQ_DPOINT3D(DPoint3d::FromScale(DPoint3d::From(100, 0, 0), METERS_TO_ENGLISH_FEET), end);

    vt = pair->CloneVerticalCurveVector(StandardUnit::EnglishFeet);
    ASSERT_TRUE(vt.IsValid());
    EXPECT_TRUE(vt->GetStartEnd(start, end));
    EXPECT_EQ_DPOINT3D(DPoint3d::FromScale(DPoint3d::From(0, 0, 800), METERS_TO_ENGLISH_FEET), start);
    EXPECT_EQ_DPOINT3D(DPoint3d::FromScale(DPoint3d::From(100, 0, 815.0), METERS_TO_ENGLISH_FEET), end);
    
    // Clone as EnglishSurveyFeet
    hz = pair->CloneHorizontalCurveVector(StandardUnit::EnglishSurveyFeet);
    ASSERT_TRUE(hz.IsValid());
    EXPECT_TRUE(hz->GetStartEnd(start, end));
    EXPECT_EQ_DPOINT3D(DPoint3d::FromZero(), start);
    EXPECT_EQ_DPOINT3D(DPoint3d::FromScale(DPoint3d::From(100, 0, 0), METERS_TO_ENGLISH_SURVEY_FEET), end);

    vt = pair->CloneVerticalCurveVector(StandardUnit::EnglishSurveyFeet);
    ASSERT_TRUE(vt.IsValid());
    EXPECT_TRUE(vt->GetStartEnd(start, end));
    EXPECT_EQ_DPOINT3D(DPoint3d::FromScale(DPoint3d::From(0, 0, 800), METERS_TO_ENGLISH_SURVEY_FEET), start);
    EXPECT_EQ_DPOINT3D(DPoint3d::FromScale(DPoint3d::From(100, 0, 815.0), METERS_TO_ENGLISH_SURVEY_FEET), end);

    // Clone unsupported unit
    hz = pair->CloneHorizontalCurveVector(StandardUnit::AngleDegrees);
    EXPECT_FALSE(hz.IsValid());
    vt = pair->CloneVerticalCurveVector(StandardUnit::AngleDegrees);
    EXPECT_FALSE(vt.IsValid());
    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        10/2016
//---------------------------------------------------------------------------------------
void AlignmentPair_Clone()
    {
    AlignmentPairPtr pair = createLinearPair();
    AlignmentPairPtr cPair = pair->Clone();
    ASSERT_TRUE(cPair.IsValid());
    ASSERT_TRUE(!cPair->GetHorizontalCurveVector().empty());
    ASSERT_TRUE(nullptr != cPair->GetVerticalCurveVector());

    expectEqualCurves(pair->GetHorizontalCurveVector(), cPair->GetHorizontalCurveVector());
    expectEqualCurves(*pair->GetVerticalCurveVector(), *cPair->GetVerticalCurveVector());

    //! Verify editing the original will not affect the clone copy
    pair->UpdateHorizontalCurveVector(*CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open));
    ASSERT_FALSE(cPair->GetHorizontalCurveVector().empty());

    pair->UpdateVerticalCurveVector(nullptr);
    ASSERT_TRUE(nullptr != cPair->GetVerticalCurveVector());
    EXPECT_FALSE(cPair->GetVerticalCurveVector()->empty());
    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        10/2016
//---------------------------------------------------------------------------------------
void AlignmentPair_GetStartEnd()
    {
    //! Start/End with hz/vt of same length
    AlignmentPairPtr pair = createLinearPair();
    DPoint3d start, end;

    EXPECT_TRUE(pair->GetStartEnd(start, end, false));
    EXPECT_EQ_DPOINT3D(DPoint3d::From(0.0, 0.0, 800.0), start);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100.0, 0.0, 815.0), end);
    EXPECT_TRUE(pair->GetStartEnd(start, end, false));
    EXPECT_EQ_DPOINT3D(DPoint3d::From(0.0, 0.0, 800.0), start);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100.0, 0.0, 815.0), end);

    //! Start/End with hz/vt having longer vt
    AlignmentPairPtr pair2 = createLinearPairWithLongerVt();

    DPoint3d start2, end2;
    EXPECT_TRUE(pair2->GetStartEnd(start2, end2, false));
    EXPECT_EQ_DPOINT3D(DPoint3d::From(0.0, 0.0, 1000.0), start2);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100.0, 0.0, 1100.0), end2);
    EXPECT_TRUE(pair2->GetStartEnd(start2, end2, true));
    EXPECT_EQ_DPOINT3D(DPoint3d::From(0.0, 0.0, 1000.0), start2);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100.0, 0.0, 1100.0), end2);

    //! Start/End with hz/vt having shorter vt (end is off-limits)
    // When passing extendVertical=true, we should get the last elevation (1085.0)
    // We should get 0.0 when passing false
    AlignmentPairPtr pair3 = createLinearPairWithShorterVt();

    DPoint3d start3, end3;
    EXPECT_TRUE(pair3->GetStartEnd(start3, end3, true));
    EXPECT_EQ_DPOINT3D(DPoint3d::From(0.0, 0.0, 1000.0), start3);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100.0, 0.0, 1085.0), end3);
    EXPECT_TRUE(pair3->GetStartEnd(start3, end3, false));
    EXPECT_EQ_DPOINT3D(DPoint3d::From(0.0, 0.0, 1000.0), start3);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100.0, 0.0, 0.0), end3);


    //! Start and end stations tests.
    // End stations shouldn't change when the vt is shorter or longer than the vertical
    double startStation, endStation;
    EXPECT_TRUE(pair->GetStartAndEndDistancesAlong(startStation, endStation));
    EXPECT_EQ_DOUBLE(0.0, startStation);
    EXPECT_EQ_DOUBLE(100.0, endStation);

    double startStation2, endStation2;
    EXPECT_TRUE(pair2->GetStartAndEndDistancesAlong(startStation2, endStation2));
    EXPECT_EQ_DOUBLE(0.0, startStation2);
    EXPECT_EQ_DOUBLE(100.0, endStation2);

    double startStation3, endStation3;
    EXPECT_TRUE(pair3->GetStartAndEndDistancesAlong(startStation3, endStation3));
    EXPECT_EQ_DOUBLE(0.0, startStation3);
    EXPECT_EQ_DOUBLE(100.0, endStation3);

    //! Retrieve start/end with empty hz
    //! Should not crash
    CurveVectorPtr empty = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    pair->UpdateHorizontalCurveVector(*empty);

    DPoint3d dummy;
    EXPECT_FALSE(pair->GetStartEnd(dummy, dummy)) << "GetStartEnd should fail with empty Hz";

    double startStation4, endStation4;
    EXPECT_FALSE(pair->GetStartAndEndDistancesAlong(startStation4, endStation4)) << "GetStartAndEndDistancesAlong should fail with empty Hz";
    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
void AlignmentPair_HasSpirals()
    {
    AlignmentPairPtr pair = createLinearPair();
    EXPECT_FALSE(pair->HasSpirals());

    CurveVectorPtr hzWith3Spirals = loadHorizontalWith3Spirals();
    ASSERT_TRUE(hzWith3Spirals.IsValid());
    pair = AlignmentPair::Create(*hzWith3Spirals, nullptr);
    ASSERT_TRUE(pair.IsValid());
    EXPECT_TRUE(pair->HasSpirals());
    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        10/2016
//---------------------------------------------------------------------------------------
void AlignmentPair_LengthXY()
    {
    //! Make sure different vt lengths do not interfere
    AlignmentPairPtr pair = createLinearPair();
    EXPECT_EQ_DOUBLE(100.0, pair->LengthXY());

    AlignmentPairPtr pair2 = createLinearPairWithLongerVt();
    EXPECT_EQ_DOUBLE(100.0, pair2->LengthXY());

    AlignmentPairPtr pair3 = createLinearPairWithShorterVt();
    EXPECT_EQ_DOUBLE(100.0, pair3->LengthXY());

    //! Retrieve length of empty hz
    //! Should not crash
    CurveVectorPtr empty = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    pair->UpdateHorizontalCurveVector(*empty);
    EXPECT_EQ_DOUBLE(0.0, pair->LengthXY());
    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        10/2016
//---------------------------------------------------------------------------------------
void AlignmentPair_GetVerticalElevationAt()
    {
    AlignmentPairPtr pair = createLinearPair();

    //! On the alignment
    EXPECT_EQ_DOUBLE(800.0, pair->GetVerticalElevationAt(0.0));
    EXPECT_EQ_DOUBLE(807.5, pair->GetVerticalElevationAt(50.0));
    EXPECT_EQ_DOUBLE(815.0, pair->GetVerticalElevationAt(100.0));

    //! Outside the alignment, with extendVertical = true
    EXPECT_EQ_DOUBLE(800.0, pair->GetVerticalElevationAt(-1.0, true));
    EXPECT_EQ_DOUBLE(815.0, pair->GetVerticalElevationAt(101.0, true));
    // If we don't extend the vertical, we should get 0.0
    EXPECT_EQ_DOUBLE(0.0, pair->GetVerticalElevationAt(-1.0, false));
    EXPECT_EQ_DOUBLE(0.0, pair->GetVerticalElevationAt(101.0, false));

    //! On an alignment that has no vt
    AlignmentPairPtr pairNoVt = createLinearPair();
    pairNoVt->UpdateVerticalCurveVector(nullptr);
    EXPECT_EQ_DOUBLE(0.0, pairNoVt->GetVerticalElevationAt(50.0));
    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        10/2016
// This test covers GetPointAt and GetPointAtWithZ
//---------------------------------------------------------------------------------------
void AlignmentPair_GetPointAt()
    {
    AlignmentPairPtr pair = createLinearPair();
    AlignmentPairPtr pairNoVt = createLinearPair();
    pairNoVt->UpdateVerticalCurveVector(nullptr);

    //! Get points somewhere on the pair
    ValidatedDPoint3d point = pair->GetPointAt(0.0);
    EXPECT_TRUE(point.IsValid());
    EXPECT_EQ_DPOINT3D(DPoint3d::FromZero(), point);

    ValidatedDPoint3d point2 = pair->GetPointAt(50.0);
    EXPECT_TRUE(point2.IsValid());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(50.0, 0, 0), point2);

    ValidatedDPoint3d point3 = pair->GetPointAt(100.0);
    EXPECT_TRUE(point3.IsValid());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100.0,0,0), point3);

    ValidatedDPoint3d point4 = pairNoVt->GetPointAt(50.0);
    EXPECT_TRUE(point4.IsValid());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(50.0, 0, 0), point4);

    //! Get points outside the alignment
    ValidatedDPoint3d point5 = pair->GetPointAt(-1.0);
    EXPECT_FALSE(point5.IsValid());
    EXPECT_EQ_DPOINT3D(DPoint3d::FromZero(), point5);

    ValidatedDPoint3d point6 = pair->GetPointAt(101.0);
    EXPECT_FALSE(point6.IsValid());
    EXPECT_EQ_DPOINT3D(DPoint3d::FromZero(), point6);


    //! Get point on the alignment (with z)
    ValidatedDPoint3d point7 = pair->GetPointAtWithZ(0.0);
    EXPECT_TRUE(point7.IsValid());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(0, 0, 800), point7);

    ValidatedDPoint3d point8 = pair->GetPointAtWithZ(50.0);
    EXPECT_TRUE(point8.IsValid());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(50, 0, 807.5), point8);

    ValidatedDPoint3d point9 = pair->GetPointAtWithZ(100.0);
    EXPECT_TRUE(point9.IsValid());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100, 0, 815), point9);

    ValidatedDPoint3d point10 = pairNoVt->GetPointAtWithZ(50.0);
    EXPECT_TRUE(point10.IsValid());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(50.0, 0, 0), point10);

    //! Get point off the hz alignment, but on vt alignment (with z) when vt alignment is longer
    AlignmentPairPtr pair2 = createLinearPairWithLongerVt();
    ValidatedDPoint3d point11 = pair2->GetPointAtWithZ(105.0);
    EXPECT_FALSE(point11.IsValid());
    EXPECT_EQ_DPOINT3D(DPoint3d::FromZero(), point11);

    //! Get point on the hz alignment but off the vt alignment (with z) when vt is shorter
    // When passing extendVertical=frue, it should return the last elevation (1085)
    // When passing false, it should return an elevation of 0.0
    AlignmentPairPtr pair3 = createLinearPairWithShorterVt();
    ValidatedDPoint3d point12 = pair3->GetPointAtWithZ(95.0, true);
    EXPECT_TRUE(point12.IsValid());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(95, 0, 1085), point12);
    point12 = pair3->GetPointAtWithZ(95.0, false);
    EXPECT_TRUE(point12.IsValid());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(95, 0, 0), point12);

    //! Get point outside the pair (with z)
    ValidatedDPoint3d point13 = pair->GetPointAtWithZ(-1.0);
    EXPECT_FALSE(point13.IsValid());
    EXPECT_EQ_DPOINT3D(DPoint3d::FromZero(), point13);

    ValidatedDPoint3d point14 = pair->GetPointAtWithZ(101.0);
    EXPECT_FALSE(point14.IsValid());
    EXPECT_EQ_DPOINT3D(DPoint3d::FromZero(), point14);


    //! GetPointAt when hz is empty.
    //! Should not crash
    CurveVectorPtr empty = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    pair->UpdateHorizontalCurveVector(*empty);

    ValidatedDPoint3d point15 = pair->GetPointAt(1.0);
    EXPECT_FALSE(point15.IsValid());
    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        10/2016
// This test covers GetPointAndTangentAt and GetPointAndTangentAtWithZ
//---------------------------------------------------------------------------------------
void AlignmentPair_GetPointAndTangentAt()
    {
    AlignmentPairPtr pair = createLinearPair();
    AlignmentPairPtr pairNoVt = createLinearPair();
    pairNoVt->UpdateVerticalCurveVector(nullptr);

    //! Get point on the alignment
    //! at start
    DPoint3d point; 
    DVec3d tangent;
    EXPECT_EQ(true, pair->GetPointAndTangentAt(point, tangent, 0.0));
    EXPECT_EQ_DPOINT3D(DPoint3d::FromZero(), point);
    EXPECT_EQ_DPOINT3D(DVec3d::From(1.0, 0, 0), tangent);

    //! somewhere on the curve
    DPoint3d point2;
    DVec3d tangent2;
    EXPECT_EQ(true, pair->GetPointAndTangentAt(point2, tangent2, 50.0));
    EXPECT_EQ_DPOINT3D(DPoint3d::From(50, 0, 0), point2);
    EXPECT_EQ_DPOINT3D(DVec3d::From(1.0, 0, 0), tangent2);

    //! at end
    DPoint3d point3;
    DVec3d tangent3;
    EXPECT_EQ(true, pair->GetPointAndTangentAt(point3, tangent3, 100.0));
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100, 0, 0), point3);
    EXPECT_EQ_DPOINT3D(DVec3d::From(1.0, 0, 0), tangent3);

    //! somewhere on the curve (no vt)
    DPoint3d point4;
    DVec3d tangent4;
    EXPECT_EQ(true, pairNoVt->GetPointAndTangentAt(point4, tangent4, 50.0));
    EXPECT_EQ_DPOINT3D(DPoint3d::From(50, 0, 0), point4);
    EXPECT_EQ_DPOINT3D(DVec3d::From(1.0, 0, 0), tangent4);

    //! Before the alignment (outside)
    DPoint3d point5;
    DVec3d tangent5;
    EXPECT_EQ(false, pair->GetPointAndTangentAt(point5, tangent5, -1.0));

    //! After the alignment (outside)
    DPoint3d point6;
    DVec3d tangent6;
    EXPECT_EQ(false, pair->GetPointAndTangentAt(point6, tangent6, 101.0));


    //! Get point on the alignment (with z)
    //! at start
    DPoint3d point7;
    DVec3d tangent7;
    EXPECT_EQ(true, pair->GetPointAndTangentAtWithZ(point7, tangent7, 0.0));
    EXPECT_EQ_DPOINT3D(DPoint3d::From(0, 0, 800), point7);
    EXPECT_EQ_DPOINT3D(DVec3d::From(1.0, 0, 0), tangent7);
    
    //! somewhere on the curve
    DPoint3d point8;
    DVec3d tangent8;
    EXPECT_EQ(true, pair->GetPointAndTangentAtWithZ(point8, tangent8, 50.0));
    EXPECT_EQ_DPOINT3D(DPoint3d::From(50, 0, 807.5), point8);
    EXPECT_EQ_DPOINT3D(DVec3d::From(1.0, 0, 0), tangent8);
    
    //! at end
    DPoint3d point9;
    DVec3d tangent9;
    EXPECT_EQ(true, pair->GetPointAndTangentAtWithZ(point9, tangent9, 100.0));
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100, 0, 815), point9);
    EXPECT_EQ_DPOINT3D(DVec3d::From(1.0, 0, 0), tangent9);

    //! somewhere on the curve (no vt)
    DPoint3d point10;
    DVec3d tangent10;
    EXPECT_EQ(true, pairNoVt->GetPointAndTangentAtWithZ(point10, tangent10, 50.0));
    EXPECT_EQ_DPOINT3D(DPoint3d::From(50, 0, 0), point10);
    EXPECT_EQ_DPOINT3D(DVec3d::From(1.0, 0, 0), tangent10);

    //! Before the alignment (outside)
    DPoint3d point11;
    DVec3d tangent11;
    EXPECT_EQ(false, pair->GetPointAndTangentAtWithZ(point11, tangent11, -1.0));

    //! After the alignment (outside)
    DPoint3d point12;
    DVec3d tangent12;
    EXPECT_EQ(false, pair->GetPointAndTangentAtWithZ(point12, tangent12, 101.0));
    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        10/2016
//---------------------------------------------------------------------------------------
void AlignmentPair_GetPointAtAndOffset()
    {
    AlignmentPairPtr pair = createLinearPair();

    //          ^^ y
    //          ||
    //          ||
    //          ||
    //======================>
    //          ||          x
    //          ||
    //          ||
    // Considering our geometry goes from x = 0 to x = 100 following the axis, applying a positive offset
    // means we're going in opposite y direction.
    //  The offset uses a unit vector rotated CLOCKWISE by 90 degrees

    ValidatedDPoint3d vPoint = pair->GetPointAtAndOffset(0.0, 5.0);
    EXPECT_TRUE(vPoint.IsValid());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(0.0, -5.0, 0.0), vPoint.Value());

    vPoint = pair->GetPointAtAndOffset(25.0, 0.0);
    EXPECT_TRUE(vPoint.IsValid());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(25, 0.0, 0.0), vPoint.Value());

    vPoint = pair->GetPointAtAndOffset(100.0, -8.0);
    EXPECT_TRUE(vPoint.IsValid());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100.0, 8.0, 0.0), vPoint.Value());

    vPoint = pair->GetPointAtAndOffset(100.01, 8.0);
    EXPECT_FALSE(vPoint.IsValid());
    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        10/2016
// This test covers HorizontalDistanceFromStart and HorizontalDistanceFromEnd
//---------------------------------------------------------------------------------------
void AlignmentPair_HorizontalDistanceAlongFrom()
    {
    AlignmentPairPtr pair = createLinearPair();

    //! Cannot use EXPECT_EQ_DOUBLE for offsets.
    // Square root is giving bigger errors. Using epsilon seems reasonable
    double offset;

    EXPECT_EQ_DOUBLE(0.0, pair->HorizontalDistanceAlongFromStart(DPoint3d::FromZero(), &offset));
    EXPECT_TRUE(mgds_fc_epsilon > fabs(0.0 - offset));

    EXPECT_EQ_DOUBLE(50.0, pair->HorizontalDistanceAlongFromStart(DPoint3d::From(50.0, 14.8, 2.4), &offset));
    EXPECT_TRUE(mgds_fc_epsilon > fabs(-14.8 - offset));

    EXPECT_EQ_DOUBLE(100.0, pair->HorizontalDistanceAlongFromStart(DPoint3d::From(100.0, -234.1, 4324.0), &offset));
    EXPECT_TRUE(mgds_fc_epsilon > fabs(234.1 - offset));

    EXPECT_EQ_DOUBLE(100.0, pair->HorizontalDistanceAlongFromEnd(DPoint3d::FromZero()));
    EXPECT_EQ_DOUBLE(50.0, pair->HorizontalDistanceAlongFromEnd(DPoint3d::From(50.0, 14.8, 2.4)));
    EXPECT_EQ_DOUBLE(0.0, pair->HorizontalDistanceAlongFromEnd(DPoint3d::From(100.0, -234.1, 4324.0)));

    // outside alignment
    EXPECT_EQ_DOUBLE(0.0, pair->HorizontalDistanceAlongFromStart(DPoint3d::From(-1.0, 0, 0)));
    EXPECT_EQ_DOUBLE(100.0, pair->HorizontalDistanceAlongFromStart(DPoint3d::From(101.0, 0, 0)));

    EXPECT_EQ_DOUBLE(100.0, pair->HorizontalDistanceAlongFromEnd(DPoint3d::From(-1.0, 0, 0)));
    EXPECT_EQ_DOUBLE(0.0, pair->HorizontalDistanceAlongFromEnd(DPoint3d::From(101.0, 0, 0)));
    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        03/2017
//---------------------------------------------------------------------------------------
void AlignmentPair_GetPrimitiveAtPoint_GetPrimitiveAtStation_GetPathLocationDetailAtStation()
    {
    // 5 different lines each having a length of 100.0
    CurveVectorPtr hzAlignment = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    hzAlignment->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(100.0, 0, 0))));
    hzAlignment->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::From(100.0, 0, 0), DPoint3d::From(200.0, 0, 0))));
    hzAlignment->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::From(200.0, 0, 0), DPoint3d::From(300.0, 0, 0))));
    hzAlignment->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::From(300.0, 0, 0), DPoint3d::From(400.0, 0, 0))));
    hzAlignment->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::From(400.0, 0, 0), DPoint3d::From(500.0, 0, 0))));

    AlignmentPairPtr pair = AlignmentPair::Create(*hzAlignment, nullptr);

    // Should return first primitive
    // GetPrimitiveAtPoint
    DPoint3d refPoint0 = DPoint3d::From(10.0, 0.0, 0.0);
    ICurvePrimitiveCPtr prim0 = pair->GetPrimitiveAtPoint(refPoint0);
    ASSERT_TRUE(prim0.IsValid());
    DPoint3d start0, end0;
    prim0->GetStartEnd(start0, end0);
    EXPECT_EQ_DPOINT3D(DPoint3d::FromZero(), start0);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100.0, 0, 0), end0);

    DPoint3d refPoint1 = DPoint3d::From(340.0, 0, 0);
    ICurvePrimitiveCPtr prim1 = pair->GetPrimitiveAtPoint(refPoint1);
    ASSERT_TRUE(prim1.IsValid());
    DPoint3d start1, end1;
    prim1->GetStartEnd(start1, end1);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(300.0, 0, 0), start1);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(400.0, 0, 0), end1);

    DPoint3d refPoint2 = DPoint3d::From(600.0, 15.0, 0);
    ICurvePrimitiveCPtr prim2 = pair->GetPrimitiveAtPoint(refPoint2);
    ASSERT_TRUE(prim2.IsValid());
    DPoint3d start2, end2;
    prim2->GetStartEnd(start2, end2);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(400.0, 0, 0), start2);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(500.0, 0, 0), end2);

    // GetPrimitiveAtStation
    DPoint3d start, end;
    ICurvePrimitiveCPtr primitive = pair->GetPrimitiveAtStation(0.0);
    EXPECT_TRUE(primitive.IsValid());
    EXPECT_TRUE(primitive->GetStartEnd(start, end));
    EXPECT_EQ_DPOINT3D(DPoint3d::FromZero(), start);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100, 0, 0), end);

    primitive = pair->GetPrimitiveAtStation(105.0);
    EXPECT_TRUE(primitive.IsValid());
    EXPECT_TRUE(primitive->GetStartEnd(start, end));
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100, 0, 0), start);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(200, 0, 0), end);

    primitive = pair->GetPrimitiveAtStation(405.0);
    EXPECT_TRUE(primitive.IsValid());
    EXPECT_TRUE(primitive->GetStartEnd(start, end));
    EXPECT_EQ_DPOINT3D(DPoint3d::From(400, 0, 0), start);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(500, 0, 0), end);

    primitive = pair->GetPrimitiveAtStation(-1.0);
    EXPECT_FALSE(primitive.IsValid()) << "Station is out of bounds";

    primitive = pair->GetPrimitiveAtStation(1234.56);
    EXPECT_FALSE(primitive.IsValid()) << "Station is out of bounds";

    // GetPathLocationDetailAtStation
    ValidatedPathLocationDetail detail = pair->GetPathLocationDetailAtStation(-1.0);
    EXPECT_FALSE(detail.IsValid());
    
    detail = pair->GetPathLocationDetailAtStation(0.0);
    EXPECT_TRUE(detail.IsValid());

    detail = pair->GetPathLocationDetailAtStation(400.05);
    EXPECT_TRUE(detail.IsValid());

    detail = pair->GetPathLocationDetailAtStation(1234.56);
    EXPECT_FALSE(detail.IsValid());
    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        10/2016
// This test covers ClosestPoint, ClosestPointXY and ClosestPointAndTangentXY
//---------------------------------------------------------------------------------------
void AlignmentPair_ClosestPoint()
    {
    // Test with empty hz and null vertical
    CurveVectorPtr emptyHz = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    AlignmentPairPtr emptyPair = AlignmentPair::Create(*emptyHz, nullptr);
    ASSERT_TRUE(emptyPair.IsValid());

    DPoint3d result;
    DVec3d resultTangent;
    EXPECT_FALSE(emptyPair->ClosestPoint(result, DPoint3d::FromZero()));
    EXPECT_FALSE(emptyPair->ClosestPointXY(result, DPoint3d::FromZero()));
    EXPECT_FALSE(emptyPair->ClosestPointAndTangentXY(result, resultTangent, DPoint3d::FromZero()));

    AlignmentPairPtr pair = createLinearPair();

    //! at start
    ICurvePrimitive::CurvePrimitiveType type;
    DPoint3d located;
    DPoint3d locatedZ;
    DPoint3d locatedPT;
    DVec3d locatedTG;
    EXPECT_TRUE(pair->ClosestPointXY(located, DPoint3d::From(0.0, 5.0, 12.0)));
    EXPECT_TRUE(pair->ClosestPoint(locatedZ, DPoint3d::From(0.0, 5.0, 12.0), true, &type));
    EXPECT_TRUE(pair->ClosestPointAndTangentXY(locatedPT, locatedTG, DPoint3d::From(0.0, 5.0, 12.0)));
    EXPECT_EQ_DPOINT3D(DPoint3d::FromZero(), located);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(0,0, 800), locatedZ);
    EXPECT_EQ(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, type);
    EXPECT_EQ_DPOINT3D(DPoint3d::FromZero(), locatedPT);
    EXPECT_EQ_DPOINT3D(DVec3d::From(1,0,0), locatedTG);

    //! somewhere on the alignment
    DPoint3d located2;
    DPoint3d locatedZ2;
    DPoint3d locatedPT2;
    DVec3d locatedTG2;
    EXPECT_TRUE(pair->ClosestPointXY(located2, DPoint3d::From(50.0, -4.0, 432.0)));
    EXPECT_TRUE(pair->ClosestPoint(locatedZ2, DPoint3d::From(50.0, -4.0, 432.0)));
    EXPECT_TRUE(pair->ClosestPointAndTangentXY(locatedPT2, locatedTG2, DPoint3d::From(50.0, -4.0, 432.0)));
    EXPECT_EQ_DPOINT3D(DPoint3d::From(50, 0, 0), located2);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(50, 0, 807.5), locatedZ2);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(50, 0, 0), locatedPT2);
    EXPECT_EQ_DPOINT3D(DVec3d::From(1,0,0), locatedTG2);

    //! at end
    DPoint3d located3;
    DPoint3d locatedZ3;
    DPoint3d locatedPT3;
    DVec3d locatedTG3;
    EXPECT_TRUE(pair->ClosestPointXY(located3, DPoint3d::From(100.0, -4.0, 0)));
    EXPECT_TRUE(pair->ClosestPoint(locatedZ3, DPoint3d::From(100.0, -4.0, 0)));
    EXPECT_TRUE(pair->ClosestPointAndTangentXY(locatedPT3, locatedTG3, DPoint3d::From(100.0, -4.0, 432.0)));
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100, 0, 0), located3);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100, 0, 815), locatedZ3);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100, 0, 0), locatedPT3);
    EXPECT_EQ_DPOINT3D(DVec3d::From(1, 0, 0), locatedTG3);

    //! Before the alignment (outside)
    DPoint3d located4;
    DPoint3d locatedZ4;
    DPoint3d locatedPT4;
    DVec3d locatedTG4;
    EXPECT_TRUE(pair->ClosestPointXY(located4, DPoint3d::From(-1.0, 0, 0)));
    EXPECT_TRUE(pair->ClosestPoint(locatedZ4, DPoint3d::From(-1.0, 0, 0)));
    EXPECT_TRUE(pair->ClosestPointAndTangentXY(locatedPT4, locatedTG4, DPoint3d::From(-1.0, 0, 0)));

    //! After the alignment (outside)
    DPoint3d located5;
    DPoint3d locatedZ5;
    DPoint3d locatedPT5;
    DVec3d locatedTG5;
    EXPECT_TRUE(pair->ClosestPointXY(located5, DPoint3d::From(101.0, 30, 60)));
    EXPECT_TRUE(pair->ClosestPoint(locatedZ5, DPoint3d::From(101.0, 30, 60)));
    EXPECT_TRUE(pair->ClosestPointAndTangentXY(locatedPT5, locatedTG5, DPoint3d::From(101.0, 30, 60)));

    // Inside the Hz alignment, but outside the Vt
    AlignmentPairPtr pair2 = createLinearPairWithShorterVt();
    DPoint3d locatedZ6;
    EXPECT_TRUE(pair2->ClosestPoint(locatedZ6, DPoint3d::From(95.0, 10, 0), false));
    EXPECT_EQ_DPOINT3D(DPoint3d::From(95.0, 0, 0), locatedZ6);
    EXPECT_TRUE(pair2->ClosestPoint(locatedZ6, DPoint3d::From(95.0, 10, 0), true));
    EXPECT_EQ_DPOINT3D(DPoint3d::From(95.0, 0, 1085.0), locatedZ6);


    // Create a pair with a Hz alignment that has more than 1 primitive
    CurveVectorPtr hzCurve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    bvector<DPoint3d> points{ DPoint3d::FromZero(), DPoint3d::From(1, 0, 0), DPoint3d::From(2, 0, 0) };
    for (size_t i = 1; i < points.size(); ++i)
        hzCurve->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(points[i - 1], points[i])));

    pair = AlignmentPair::Create(*hzCurve, nullptr);
    ASSERT_TRUE(pair.IsValid());

    EXPECT_TRUE(pair->ClosestPoint(result, DPoint3d::From(2, 0, 0)));
    EXPECT_EQ_DPOINT3D(DPoint3d::From(2, 0, 0), result);
    }

#if ENABLE_SELF_INTERSECT_TEST
/*---------------------------------------------------------------------------------**//**
* @betest                                    Scott.Devoe                     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentPair_SelfIntersects()
    {
    const DPoint3d pt1 = DPoint3d::From(0.0, 0.0, 0.0);
    const DPoint3d pt2 = DPoint3d::From(100.0, 100.0, 0.0);
    const DPoint3d pt3 = DPoint3d::From(0.0, 100.0, 0.0);
    const DPoint3d pt4 = DPoint3d::From(100.0, 0.0, 0.0);
    CurveVectorPtr hzCurve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    ICurvePrimitivePtr line = ICurvePrimitive::CreateLine(DSegment3d::From(pt1, pt2));
    hzCurve->Add(line);
    ICurvePrimitivePtr line2 = ICurvePrimitive::CreateLine(DSegment3d::From(pt2, pt3));
    hzCurve->Add(line2);
    ICurvePrimitivePtr line3 = ICurvePrimitive::CreateLine(DSegment3d::From(pt3, pt4));
    hzCurve->Add(line3);

    AlignmentPairPtr pair = AlignmentPair::Create(*hzCurve, nullptr);
    ASSERT_TRUE(pair.IsValid());
    size_t numInts;

    EXPECT_TRUE(pair->SelfIntersects(&numInts));
    EXPECT_EQ(9, numInts); // should be 1!! talk to earlin
    }
#endif

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        10/2016
// This test covers GetPartialHorizontalAlignment (2), GetPartialVerticalAlignment, GetPartialAlignment(2)
//---------------------------------------------------------------------------------------
void AlignmentPair_GetPartialAlignment()
    {
    AlignmentPairPtr pair = createLinearPair();

    DPoint3d aStart, aEnd;
    pair->GetStartEnd(aStart, aEnd);
    DPoint3d aStartXY = DPoint3d::From(aStart.x, aStart.y);
    DPoint3d aEndXY = DPoint3d::From(aEnd.x, aEnd.y);

    //! Clone hz over full range
    CurveVectorPtr partialHz = pair->GetPartialHorizontalAlignment(0.0, 100.0);
    DPoint3d start, end;
    partialHz->GetStartEnd(start, end);

    EXPECT_EQ_DPOINT3D(aStartXY, start);
    EXPECT_EQ_DPOINT3D(aEndXY, end);
    EXPECT_EQ_DOUBLE(100.0, partialHz->Length());

    // Full range using points
    partialHz = pair->GetPartialHorizontalAlignment(DPoint3d::FromZero(), DPoint3d::From(100, 0, 0));
    EXPECT_TRUE(partialHz.IsValid());

    // Clone hz longer range
    partialHz = pair->GetPartialHorizontalAlignment(0.0, 102.0);
    EXPECT_FALSE(partialHz.IsValid());

    //! Clone hz partial range
    CurveVectorPtr partialHz2 = pair->GetPartialHorizontalAlignment(10.0, 90.0);
    DPoint3d start2, end2;
    partialHz2->GetStartEnd(start2, end2);

    EXPECT_EQ_DPOINT3D(DPoint3d::From(10.0, 0, 0), start2);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(90.0, 0, 0), end2);
    EXPECT_EQ_DOUBLE(80.0, partialHz2->Length());

    //! Clone hz null range
    CurveVectorPtr partialHz3 = pair->GetPartialHorizontalAlignment(10.0, 10.0);
    EXPECT_FALSE(partialHz3.IsValid());


    //! Clone vt full range
    CurveVectorPtr partialVt = pair->GetPartialVerticalAlignment(0.0, 100.0);
    DPoint3d start3, end3;
    partialVt->GetStartEnd(start3, end3);

    EXPECT_EQ_DPOINT3D(DPoint3d::From(0.0, 0, 800), start3);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100.0, 0, 815), end3);
    
    //! Clone vt longer range
    partialVt = pair->GetPartialVerticalAlignment(0.0, 102.0);
    EXPECT_TRUE(partialVt.IsValid()); //&&AG this one needswork. I don't think cloning using stationing larger than the actual length of curve should work

    partialVt = pair->GetPartialVerticalAlignment(0.0, 100.005);
    EXPECT_TRUE(partialVt.IsValid()) << "We accept some fuzz for the vertical end station";

    //! Clone vt partial range
    CurveVectorPtr partialVt2 = pair->GetPartialVerticalAlignment(25.0, 75.0);
    DPoint3d start4, end4;
    partialVt2->GetStartEnd(start4, end4);

    EXPECT_EQ_DPOINT3D(DPoint3d::From(0.0, 0, 803.75), start4);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(50.0, 0, 811.25), end4);

    //! Clone vt null range
    CurveVectorPtr partialVt3 = pair->GetPartialVerticalAlignment(10.0, 10.0);
    EXPECT_FALSE(partialVt3.IsValid());
    
    // Clone vt reversed range
    partialVt = pair->GetPartialVerticalAlignment(102.0, 0.0);
    EXPECT_TRUE(partialVt.IsValid()); //&&AG this one needswork. I don't think cloning using stationing larger than the actual length of curve should work
    partialVt = pair->GetPartialVerticalAlignment(98.0, 0.0);
    EXPECT_TRUE(partialVt.IsValid());

    
    //! Clone alignment full range
    AlignmentPairPtr cPair = pair->GetPartialAlignment(0.0, 100.0);
    ASSERT_TRUE(cPair.IsValid());
    expectEqualCurves(pair->GetHorizontalCurveVector(), cPair->GetHorizontalCurveVector());
    expectEqualCurves(*pair->GetVerticalCurveVector(), *cPair->GetVerticalCurveVector());

    cPair = pair->GetPartialAlignment(DPoint3d::FromZero(), DPoint3d::From(100, 0, 0));
    ASSERT_TRUE(cPair.IsValid());
    expectEqualCurves(pair->GetHorizontalCurveVector(), cPair->GetHorizontalCurveVector());
    expectEqualCurves(*pair->GetVerticalCurveVector(), *cPair->GetVerticalCurveVector());

    //! Clone partial range
    AlignmentPairPtr cPair2 = pair->GetPartialAlignment(10.0, 90.0);
    ASSERT_TRUE(cPair2.IsValid());
    DPoint3d start5, end5;
    cPair2->GetStartEnd(start5, end5);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(10.0, 0, 801.5), start5);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(90, 0, 813.5), end5);

    //! Clone null range
    AlignmentPairPtr cPair3 = pair->GetPartialAlignment(10.0, 10.0);
    EXPECT_FALSE(cPair3.IsValid());

    //! Clone bigger than the original
    AlignmentPairPtr cPair4 = pair->GetPartialAlignment(-1.0, 101.0);
    EXPECT_FALSE(cPair4.IsValid());

    // Get partial alignment when startStation > endStation
    AlignmentPairPtr cPair5 = pair->GetPartialAlignment(100.0, 0.0);
    ASSERT_TRUE(cPair5.IsValid());
    cPair5->GetStartEnd(start, end);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100.0, 0, 815.0), start);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(0, 0, 800.0), end);

    // Try to retrieve partial vt when we have none
    pair = AlignmentPair::Create(pair->GetHorizontalCurveVector(), nullptr);
    ASSERT_TRUE(pair.IsValid());

    CurveVectorPtr vt = pair->GetPartialVerticalAlignment(0.0, 10.0);
    EXPECT_FALSE(vt.IsValid());


    // Partial curves coverage
    ICurvePrimitivePtr primitive = ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(10, 0, 0)));
    ICurvePrimitivePtr partialPrimitive = ICurvePrimitive::CreatePartialCurve(primitive.get(), 0.0, 1.0);

    CurveVectorPtr hz = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    hz->push_back(partialPrimitive);
    pair = AlignmentPair::Create(*hz, nullptr);
    ASSERT_TRUE(pair.IsValid());
    AlignmentPairPtr result = pair->GetPartialAlignment(2.5, 7.5);
    EXPECT_TRUE(result.IsValid());

    partialPrimitive = ICurvePrimitive::CreatePartialCurve(primitive.get(), 0.2, 0.8);
    hz->at(0) = partialPrimitive;
    pair = AlignmentPair::Create(*hz, nullptr);
    ASSERT_TRUE(pair.IsValid());
    result = pair->GetPartialAlignment(3.0, 4.0);
    EXPECT_TRUE(result.IsValid());

    // Nesting of 2 partials
    ICurvePrimitivePtr partialPartial = ICurvePrimitive::CreatePartialCurve(partialPrimitive.get(), 0.0, 1.0);
    hz->at(0) = partialPartial;
    pair = AlignmentPair::Create(*hz, nullptr);
    ASSERT_TRUE(pair.IsValid());
    result = pair->GetPartialAlignment(3.0, 4.0);
    EXPECT_TRUE(result.IsValid());

    
    hz = result->CloneHorizontalCurveVector();
    AlignmentPair::TransformCurveWithPartialCurves(*hz, Transform::FromIdentity());
    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        10/2016
//---------------------------------------------------------------------------------------
void AlignmentPair_GetStrokedAlignment()
    {
    AlignmentPairPtr pair = createLinearPair();
    bvector<DPoint3d> stroked = pair->GetStrokedAlignment();
    ASSERT_NE(0, stroked.size());

    EXPECT_EQ_DPOINT3D(DPoint3d::From(0, 0, 800), stroked.front());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100, 0, 815), stroked.back());

    double length = 0;
    for (size_t i = 1; i < stroked.size(); ++i)
        {
        length += stroked[i - 1].DistanceXY(stroked[i]);
        }

    EXPECT_EQ_DOUBLE(100.0, length);
    EXPECT_EQ_DOUBLE(100.0, pair->LengthXY());

    pair = AlignmentPair::Create(pair->GetHorizontalCurveVector(), nullptr);
    ASSERT_TRUE(pair.IsValid());
    stroked = pair->GetStrokedAlignment();
    ASSERT_NE(0, stroked.size());

    EXPECT_EQ_DPOINT3D(DPoint3d::FromZero(), stroked.front());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100, 0, 0), stroked.back());
    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        10/2017
// This test method creates a test curve by filleting pair of lines
// ^ y
// |           1        3 _
// |          / \     /      \
// |         /   \  /           \
// |        0     2               4
// ------> x
//---------------------------------------------------------------------------------------
void AlignmentPair_ComputeMinimumRadius()
    {
    const bvector<DPoint3d> points
        {    
        DPoint3d::FromZero(),
        DPoint3d::From(50.0, 50.0),
        DPoint3d::From(100.0, 0.0),
        DPoint3d::From(180.0, 50.0),
        DPoint3d::From(300.0, 0.0)
        };
    const bvector<double> radiuses
        {
        20.0,
        30.0,
        60.0
        };

    CurveVectorPtr hz = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);

    for (size_t i = 1; i < points.size() - 1; ++i)
        {
        DPoint3dCR previous = points[i - 1];
        DPoint3dCR next = points[i + 1];
        const double radius = radiuses[i - 1];

        ValidatedDEllipse3d ellipse = DEllipse3d::FromFilletInCorner(previous, points[i], next, radius);
        if (!ellipse.IsValid())
            FAIL();

        DPoint3d ellStart, ellEnd;
        ellipse.Value().EvaluateEndPoints(ellStart, ellEnd);

        if (hz->empty())
            hz->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(previous, ellStart)));
        else
            hz->back()->TrySetEnd(ellStart);

        hz->push_back(ICurvePrimitive::CreateArc(ellipse));

        if (!ellEnd.AlmostEqualXY(next))
            hz->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(ellEnd, next)));
        }

    AlignmentPairPtr pair = AlignmentPair::Create(*hz, nullptr);
    EXPECT_EQ_DOUBLE(30.0, pair->ComputeLeftMinimumRadius());
    EXPECT_EQ_DOUBLE(20.0, pair->ComputeRightMinimumRadius());

    // Straight alignment should be returning CS_SPI_INFINITY as the minimum radius
    AlignmentPairPtr pair2 = createLinearPair();
    EXPECT_EQ_DOUBLE(CS_SPI_INFINITY, pair2->ComputeLeftMinimumRadius());
    EXPECT_EQ_DOUBLE(CS_SPI_INFINITY, pair2->ComputeRightMinimumRadius());
    }


//=======================================================================================
// AlignmentPI tests
//=======================================================================================
//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
void AlignmentPI_Tests()
    {
    DPoint3d disconnect;
    disconnect.InitDisconnect();

    AlignmentPI pi;

    // Unitialized PI
    EXPECT_TRUE(nullptr == pi.GetNoCurve());
    EXPECT_TRUE(nullptr == pi.GetNoCurveP());
    EXPECT_TRUE(nullptr == pi.GetArc());
    EXPECT_TRUE(nullptr == pi.GetArcP());
    EXPECT_TRUE(nullptr == pi.GetSCS());
    EXPECT_TRUE(nullptr == pi.GetSCSP());
    EXPECT_TRUE(nullptr == pi.GetSS());
    EXPECT_TRUE(nullptr == pi.GetSSP());
    EXPECT_FALSE(pi.IsInitialized());
    EXPECT_EQ_DPOINT3D(disconnect, pi.GetPILocation());
    EXPECT_EQ(AlignmentPI::TYPE_Uninitialized, pi.GetType());
    EXPECT_FALSE(pi.SetPILocation(DPoint3d::FromZero()));
    EXPECT_EQ_DOUBLE(0.0, pi.GetPseudoTangentLength());

    // NoCurve
    pi.InitNoCurve(DPoint3d::From(12, 34, 56));
    ASSERT_TRUE(nullptr != pi.GetNoCurve());
    EXPECT_TRUE(nullptr != pi.GetNoCurveP());
    EXPECT_TRUE(pi.IsInitialized());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(12, 34, 0), pi.GetPILocation());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(12, 34, 0), pi.GetNoCurve()->piPoint);
    EXPECT_EQ(AlignmentPI::TYPE_NoCurve, pi.GetType());
    EXPECT_TRUE(pi.SetPILocation(DPoint3d::FromOne()));
    EXPECT_EQ_DPOINT3D(DPoint3d::From(1, 1, 0), pi.GetPILocation());

    EXPECT_EQ_DOUBLE(0.0, pi.GetPseudoTangentLength()); // 0.0

    // Arc
    pi.InitArc(DPoint3d::From(56, 78, 90), 12.3);
    ASSERT_TRUE(nullptr != pi.GetArc());
    EXPECT_TRUE(nullptr != pi.GetArcP());
    EXPECT_TRUE(pi.IsInitialized());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(56, 78, 0), pi.GetPILocation());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(56, 78, 0), pi.GetArc()->arc.piPoint);
    EXPECT_EQ_DOUBLE(12.3, pi.GetArc()->arc.radius);
    EXPECT_TRUE(pi.SetPILocation(DPoint3d::FromZero()));
    EXPECT_EQ_DPOINT3D(DPoint3d::FromZero(), pi.GetPILocation());

    pi.GetArcP()->arc.startPoint = (DPoint3d::From(-5, 0, 0));
    EXPECT_EQ_DOUBLE(5.0, pi.GetPseudoTangentLength()); // Should be DistanceXY from piPoint to arc startPOint

    // SS
    pi.InitSS(DPoint3d::From(2, 3, 4), 9.81);
    ASSERT_TRUE(nullptr != pi.GetSS());
    EXPECT_TRUE(nullptr != pi.GetSSP());
    EXPECT_TRUE(pi.IsInitialized());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(2, 3, 0), pi.GetPILocation());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(2, 3, 0), pi.GetSS()->overallPI);
    EXPECT_EQ_DOUBLE(9.81, pi.GetSS()->spiral1.length);
    EXPECT_EQ_DOUBLE(9.81, pi.GetSS()->spiral2.length);
    EXPECT_TRUE(pi.SetPILocation(DPoint3d::FromOne()));
    EXPECT_EQ_DPOINT3D(DPoint3d::From(1, 1, 0), pi.GetPILocation());

    pi.GetSSP()->spiral1.startPoint = DPoint3d::From(-2, -3, 0);
    EXPECT_EQ_DOUBLE(5.0, pi.GetPseudoTangentLength()); // Should be DistanceXY from overallPI to spiral1 startPoint


    // SCS
    pi.InitSCS(DPoint3d::From(3, 4, 5), 1.64, 2.34, 3.67);
    ASSERT_TRUE(nullptr != pi.GetSCS());
    EXPECT_TRUE(nullptr != pi.GetSCSP());
    EXPECT_TRUE(pi.IsInitialized());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(3, 4, 0), pi.GetPILocation());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(3, 4, 0), pi.GetSCS()->overallPI);
    EXPECT_EQ_DOUBLE(1.64, pi.GetSCS()->arc.radius);
    EXPECT_EQ_DOUBLE(2.34, pi.GetSCS()->spiral1.length);
    EXPECT_EQ_DOUBLE(3.67, pi.GetSCS()->spiral2.length);
    EXPECT_TRUE(pi.SetPILocation(DPoint3d::FromZero()));
    EXPECT_EQ_DPOINT3D(DPoint3d::FromZero(), pi.GetPILocation());

    pi.GetSCSP()->spiral1.startPoint = DPoint3d::From(-4, -3, 0);
    EXPECT_EQ_DOUBLE(5.0, pi.GetPseudoTangentLength()); // Should be DistanceXY from overallPI to spiral1 startPoint
    }


//=======================================================================================
// AlignmentPairEditor tests
//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//  (-10,-10)                      (20, -10)
//      |                              |
//       \             Line           /
//        -------=======|======-------
//            (0, 0)    PI   (10, 0)
//---------------------------------------------------------------------------------------
CurveVectorPtr createHzWithStartEndArcs()
    {
    CurveVectorPtr hz = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);

    DEllipse3d ellipse1, ellipse2;
    if (!ellipse1.InitFromArcCenterStartEnd(DPoint3d::From(0, -10), DPoint3d::From(-10, -10), DPoint3d::FromZero()))
        return nullptr;

    if (!ellipse2.InitFromArcCenterStartEnd(DPoint3d::From(10, -10, 0), DPoint3d::From(10, 0), DPoint3d::From(20, -10)))
        return nullptr;

    hz->push_back(ICurvePrimitive::CreateArc(ellipse1));
    hz->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(5, 0))));
    hz->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::From(5, 0), DPoint3d::From(10, 0))));
    hz->push_back(ICurvePrimitive::CreateArc(ellipse2));
    return hz;
    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_Create_Clone()
    {
    AlignmentPairPtr pair = createLinearPair();

    // Create
    AlignmentPairEditorPtr editor = AlignmentPairEditor::Create(pair->GetHorizontalCurveVector(), pair->GetVerticalCurveVector());
    ASSERT_TRUE(editor.IsValid());
    expectEqualCurves(pair->GetHorizontalCurveVector(), editor->GetHorizontalCurveVector());

    editor = AlignmentPairEditor::Create(*pair);
    ASSERT_TRUE(editor.IsValid());
    expectEqualCurves(pair->GetHorizontalCurveVector(), editor->GetHorizontalCurveVector());

    editor = AlignmentPairEditor::CreateVerticalOnly(*pair->GetVerticalCurveVector());
    ASSERT_TRUE(editor.IsValid());
    ASSERT_TRUE(nullptr != editor->GetVerticalCurveVector());
    expectEqualCurves(*pair->GetVerticalCurveVector(), *editor->GetVerticalCurveVector());

    CurveVectorPtr vt = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    editor = AlignmentPairEditor::CreateVerticalOnly(*vt);
    EXPECT_FALSE(editor.IsValid());

    vt->push_back(ICurvePrimitive::CreateLine(DSegment3d::FromZero()));
    editor = AlignmentPairEditor::CreateVerticalOnly(*vt);
    EXPECT_FALSE(editor.IsValid());


    // Clone
    editor = AlignmentPairEditor::Create(*pair);
    AlignmentPairPtr clone = editor->Clone();
    ASSERT_TRUE(clone.IsValid());
    ASSERT_TRUE(nullptr != dynamic_cast<AlignmentPairEditorP>(clone.get()));
    expectEqualCurves(pair->GetHorizontalCurveVector(), clone->GetHorizontalCurveVector());
    ASSERT_TRUE(nullptr != clone->GetVerticalCurveVector());
    expectEqualCurves(*pair->GetVerticalCurveVector(), *clone->GetVerticalCurveVector());


    // Validate test curves before we start using them!
    ASSERT_TRUE(createHzWithStartEndArcs().IsValid());
    }
//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_GetPIs()
    {
// Test with 'Line' curve
    AlignmentPairPtr pair1 = createLinearPair();
    AlignmentPairEditorPtr editor = AlignmentPairEditor::Create(*pair1);
    ASSERT_TRUE(editor.IsValid());

    bvector<AlignmentPI> pis = editor->GetPIs();
    ASSERT_EQ(2, pis.size());
    EXPECT_EQ(AlignmentPI::TYPE_NoCurve, pis[0].GetType());
    EXPECT_EQ(AlignmentPI::TYPE_NoCurve, pis[1].GetType());

    bvector<DPoint3d> piPoints = editor->GetPIPoints();
    EXPECT_EQ(pis.size(), piPoints.size());

    AlignmentPI pi;
    EXPECT_TRUE(editor->GetPI(pi, 0));
    EXPECT_TRUE(editor->GetPI(pi, 1));
    EXPECT_FALSE(editor->GetPI(pi, 2));


// Test with 'Arc'
    DEllipse3d ellipse = DEllipse3d::FromCenterNormalRadius(DPoint3d::FromZero(), DVec3d::UnitZ(), 12.0);
    CurveVectorPtr hz = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    hz->push_back(ICurvePrimitive::CreateArc(ellipse));

    editor = AlignmentPairEditor::Create(*hz, nullptr);
    ASSERT_TRUE(editor.IsValid());
    pis = editor->GetPIs();
    EXPECT_EQ(0, pis.size()) << "Arc is a full ellipse. Should fail";

    ellipse = DEllipse3d::FromArcCenterStartEnd(DPoint3d::FromZero(), DPoint3d::FromOne(), DPoint3d::From(-1, -1, 0));
    ellipse.sweep = 0.0; // No sweep means an arc length of 0.0;
    hz->at(0) = ICurvePrimitive::CreateArc(ellipse);

    editor = AlignmentPairEditor::Create(*hz, nullptr);
    ASSERT_TRUE(editor.IsValid());
    pis = editor->GetPIs();
    EXPECT_EQ(0, pis.size()) << "Arc has null length. Should fail";


// Test with 'Line-Arc-Line-Arc-Line-Arc-Line' curve
    CurveVectorPtr hzWith3Arcs = loadHorizontalWith3Arcs();
    ASSERT_TRUE(hzWith3Arcs.IsValid());

    editor = AlignmentPairEditor::Create(*hzWith3Arcs, nullptr);
    ASSERT_TRUE(editor.IsValid());

    pis = editor->GetPIs();
    ASSERT_EQ(5, pis.size());

    ASSERT_EQ(AlignmentPI::TYPE_NoCurve, pis[0].GetType());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(468467.05241349683, 2259133.0569764804), pis[0].GetNoCurve()->piPoint);

    ASSERT_EQ(AlignmentPI::TYPE_Arc, pis[1].GetType());
    EXPECT_EQ_DOUBLE(184.0, pis[1].GetArc()->arc.radius);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(468660.06504850514, 2259068.6505596871), pis[1].GetArc()->arc.centerPoint);
    EXPECT_EQ(AlignmentPI::ORIENTATION_CW, pis[1].GetArc()->arc.orientation);

    ASSERT_EQ(AlignmentPI::TYPE_Arc, pis[2].GetType());
    EXPECT_EQ_DOUBLE(184.0, pis[2].GetArc()->arc.radius);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(468834.54602081364, 2259416.9857294112), pis[2].GetArc()->arc.centerPoint);
    EXPECT_EQ(AlignmentPI::ORIENTATION_CCW, pis[2].GetArc()->arc.orientation);

    ASSERT_EQ(AlignmentPI::TYPE_Arc, pis[3].GetType());
    EXPECT_EQ_DOUBLE(184.0, pis[3].GetArc()->arc.radius);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(469123.72809941485, 2259174.1824242566), pis[3].GetArc()->arc.centerPoint);
    EXPECT_EQ(AlignmentPI::ORIENTATION_CW, pis[3].GetArc()->arc.orientation);

    ASSERT_EQ(AlignmentPI::TYPE_NoCurve, pis[4].GetType());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(469321.36029021174, 2259393.2817337015), pis[4].GetNoCurve()->piPoint);


// Test with 'Line-SCS-Line-SCS-Line-SCS-Line
    CurveVectorPtr hzWith3Spirals = loadHorizontalWith3Spirals();
    ASSERT_TRUE(hzWith3Spirals.IsValid());

    editor = AlignmentPairEditor::Create(*hzWith3Spirals, nullptr);
    ASSERT_TRUE(editor.IsValid());

    pis = editor->GetPIs();
    ASSERT_EQ(5, pis.size());

    ASSERT_EQ(AlignmentPI::TYPE_NoCurve, pis[0].GetType());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(468480.62028229458, 2258647.4757073857), pis[0].GetNoCurve()->piPoint);

    ASSERT_EQ(AlignmentPI::TYPE_SCS, pis[1].GetType());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(468561.57954157837, 2258790.8192921854), pis[1].GetSCS()->spiral1.startPoint);
    EXPECT_EQ_DOUBLE(667.0, pis[1].GetSCS()->arc.radius);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(469159.06811088009, 2258491.8333408381), pis[1].GetSCS()->arc.centerPoint);
    EXPECT_EQ(AlignmentPI::ORIENTATION_CW, pis[1].GetSCS()->arc.orientation);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(468790.67837881838, 2259047.8711141152), pis[1].GetSCS()->spiral2.startPoint);

    ASSERT_EQ(AlignmentPI::TYPE_SCS, pis[2].GetType());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(468988.41144292668, 2259166.3367093648), pis[2].GetSCS()->spiral1.startPoint);
    EXPECT_EQ_DOUBLE(667.0, pis[2].GetSCS()->arc.radius);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(468677.07669415820, 2259757.4846297354), pis[2].GetSCS()->arc.centerPoint);
    EXPECT_EQ(AlignmentPI::ORIENTATION_CCW, pis[2].GetSCS()->arc.orientation);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(469127.65689328074, 2259265.685398180), pis[2].GetSCS()->spiral2.startPoint)
    
    ASSERT_EQ(AlignmentPI::TYPE_SCS, pis[3].GetType());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(469275.47792368190, 2259413.8482709471), pis[3].GetSCS()->spiral1.startPoint);
    EXPECT_EQ_DOUBLE(667.0, pis[3].GetSCS()->arc.radius);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(469773.91098115058, 2258968.9329118370), pis[3].GetSCS()->arc.centerPoint);
    EXPECT_EQ(AlignmentPI::ORIENTATION_CW, pis[3].GetSCS()->arc.orientation);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(469379.99574711901, 2259507.1891597323), pis[3].GetSCS()->spiral2.startPoint)

    ASSERT_EQ(AlignmentPI::TYPE_NoCurve, pis[4].GetType());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(469577.74622473511, 2259638.5295264777), pis[4].GetNoCurve()->piPoint);

    CurveVectorPtr hzStartsAndEndsWithArcs = hzWith3Arcs->Clone();
    hzStartsAndEndsWithArcs->erase(&hzStartsAndEndsWithArcs->front());
    hzStartsAndEndsWithArcs->erase(&hzStartsAndEndsWithArcs->back());

    // we should only have Arc-Line-Arc-Line-Arc now
    ASSERT_EQ(5, hzStartsAndEndsWithArcs->size());
    EXPECT_EQ(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc, hzStartsAndEndsWithArcs->at(0)->GetCurvePrimitiveType());
    EXPECT_EQ(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc, hzStartsAndEndsWithArcs->at(2)->GetCurvePrimitiveType());
    EXPECT_EQ(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc, hzStartsAndEndsWithArcs->at(4)->GetCurvePrimitiveType());

    editor = AlignmentPairEditor::Create(*hzStartsAndEndsWithArcs, nullptr);
    ASSERT_TRUE(editor.IsValid());

    pis = editor->GetPIs();
    ASSERT_EQ(3, pis.size());
    for (auto const& pi : pis)
        {
        ASSERT_TRUE(nullptr != pi.GetArc());
        }
    }
//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_UpdateCurveVectors()
    {
    CurveVectorPtr hz = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);

    AlignmentPairEditorPtr editor = AlignmentPairEditor::Create(*hz, nullptr);
    ASSERT_TRUE(editor.IsValid());

    editor->UpdateHorizontalCurveVector(*hz);
    editor->UpdateVerticalCurveVector(nullptr);
    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_InsertPI()
    {
    CurveVectorPtr hz = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    AlignmentPairEditorPtr editor = AlignmentPairEditor::Create(*hz, nullptr);
    ASSERT_TRUE(editor.IsValid());

    // Test with empty curve
    AlignmentPI pi;
    CurveVectorPtr result = editor->InsertPI(pi);
    EXPECT_FALSE(result.IsValid());
    result = editor->InsertPI(pi, 0);
    EXPECT_FALSE(result.IsValid());
    result = editor->InsertPI(pi, 1);
    EXPECT_FALSE(result.IsValid());

    // Straight line from (0,0,0) to (100,0,0)
    hz = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    hz->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(100, 0, 0))));

    editor = AlignmentPairEditor::Create(*hz, nullptr);
    ASSERT_TRUE(editor.IsValid());

    AlignmentPI newPI;
    EXPECT_FALSE(newPI.IsInitialized());

    // Uninitialized
    result = editor->InsertPI(newPI);
    EXPECT_FALSE(result.IsValid()) << "InsertPI with an unitialized PI should fail";

    // Grade Break
    newPI.InitNoCurve(DPoint3d::FromZero());
    result = editor->InsertPI(newPI);
    EXPECT_FALSE(result.IsValid()) << "Inserting a PI overlapping StartPI should fail";

    newPI.InitNoCurve(DPoint3d::From(100, 0, 0));
    result = editor->InsertPI(newPI);
    EXPECT_FALSE(result.IsValid()) << "Inserting a PI overlapping EndPI should fail";

    newPI.InitNoCurve(DPoint3d::From(50, 25));
    result = editor->InsertPI(newPI);
    EXPECT_TRUE(result.IsValid());
    EXPECT_EQ(2, result->size());

    AlignmentPairEditorPtr resultEditor = AlignmentPairEditor::Create(*result, nullptr);
    ASSERT_TRUE(resultEditor.IsValid());
    EXPECT_EQ(3, resultEditor->GetPIs().size());

    newPI.InitNoCurve(DPoint3d::From(50, 25));
    result = resultEditor->InsertPI(newPI);
    EXPECT_FALSE(result.IsValid()) << "Inserting a PI overlapping an existing PI should fail";
    newPI.InitNoCurve(DPoint3d::From(75, 30));
    result = resultEditor->InsertPI(newPI);
    ASSERT_TRUE(result.IsValid());
    EXPECT_EQ(3, result->size());

    // Arc
    newPI.InitArc(DPoint3d::From(50, 50), 0.0);
    result = editor->InsertPI(newPI);
    EXPECT_FALSE(result.IsValid()) << "Inserting a Arc PI with a null radius should fail";

    newPI.InitArc(DPoint3d::From(50, 50), 12.0);
    result = editor->InsertPI(newPI);
    ASSERT_TRUE(result.IsValid());
    EXPECT_EQ(3, result->size()); // Line-Arc-Line
    
    resultEditor = AlignmentPairEditor::Create(*result, nullptr);
    ASSERT_TRUE(resultEditor.IsValid());
    EXPECT_EQ(3, resultEditor->GetPIs().size());

    // Spiral-Curve-Spiral
    newPI.InitSCS(DPoint3d::From(50, 50), 12.0, 0.0, 15.0);
    result = editor->InsertPI(newPI);
    EXPECT_FALSE(result.IsValid()) << "Inserting a SCS PI with null spiral1 length should fail";

    newPI.InitSCS(DPoint3d::From(50, 50), 12.0, 15.0, 0.0);
    result = editor->InsertPI(newPI);
    EXPECT_FALSE(result.IsValid()) << "Inserting a SCS PI with null spiral2 length should fail";

    newPI.InitSCS(DPoint3d::From(50, 50), 12.0, 15.0, 15.0);
    result = editor->InsertPI(newPI);
    ASSERT_TRUE(result.IsValid());
    EXPECT_EQ(5, result->size()); // Line-Spiral-Arc-Spiral-Line

    resultEditor = AlignmentPairEditor::Create(*result, nullptr);
    ASSERT_TRUE(resultEditor.IsValid());
    EXPECT_EQ(3, resultEditor->GetPIs().size());

    // Spiral-Spiral
    newPI.InitSS(DPoint3d::From(50, 50), 15.0);
    result = editor->InsertPI(newPI);
    ASSERT_TRUE(result.IsValid());
    EXPECT_EQ(4, result->size()); // Line-Spiral-Spiral-Line

    resultEditor = AlignmentPairEditor::Create(*result, nullptr);
    ASSERT_TRUE(resultEditor.IsValid());
    EXPECT_EQ(3, resultEditor->GetPIs().size());


    // Test with curve that starts and ends with an Arc PI
    hz = createHzWithStartEndArcs();
    editor = AlignmentPairEditor::Create(*hz, nullptr);
    ASSERT_TRUE(editor.IsValid());

    newPI.InitNoCurve(DPoint3d::From(-10.0, -11.0)); // Before the first arc
    result = editor->InsertPI(newPI);
    EXPECT_FALSE(result.IsValid());

    newPI.InitNoCurve(DPoint3d::From(-1.0, -1.0)); // Inside the first arc
    result = editor->InsertPI(newPI);
    EXPECT_FALSE(result.IsValid());

    newPI.InitNoCurve(DPoint3d::From(4.95, 0)); // After the arc, but before the NoCurve PI
    result = editor->InsertPI(newPI);
    EXPECT_TRUE(result.IsValid());

    newPI.InitNoCurve(DPoint3d::From(5.05, 0)); // After the NoCurve PI, but before the last arc
    result = editor->InsertPI(newPI);
    EXPECT_TRUE(result.IsValid());

    newPI.InitNoCurve(DPoint3d::From(11, -1)); // Inside the last arc
    result = editor->InsertPI(newPI);
    EXPECT_FALSE(result.IsValid());

    newPI.InitNoCurve(DPoint3d::From(20, -11)); // After the last arc
    result = editor->InsertPI(newPI);
    EXPECT_FALSE(result.IsValid());
    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_DeletePI()
    {
    // Straight line from (0,0,0) to (100,0,0)
    CurveVectorPtr hz = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    hz->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(100, 0, 0))));

    AlignmentPairEditorPtr editor = AlignmentPairEditor::Create(*hz, nullptr);
    ASSERT_TRUE(editor.IsValid());

    CurveVectorPtr result = editor->DeletePI(0);
    EXPECT_FALSE(result.IsValid()) << "Deleting First PI should fail when it's a TYPE_NoCurve";
    
    result = editor->DeletePI(1);
    EXPECT_FALSE(result.IsValid()) << "Deleting Last PI should fail when it's a TYPE_NoCurve";

    result = editor->DeletePI(2);
    EXPECT_FALSE(result.IsValid()) << "Deleting PI with index out of bounds";

    result = editor->DeletePI(DPoint3d::FromZero());
    EXPECT_FALSE(result.IsValid());

    result = editor->DeletePI(DPoint3d::From(100, 0, 0));
    EXPECT_FALSE(result.IsValid());

    result = editor->DeletePI(DPoint3d::From(50.0, 0, 0));
    EXPECT_FALSE(result.IsValid());
    
    // Add a pi and try to delete it
    AlignmentPI pi;
    pi.InitNoCurve(DPoint3d::From(50.0, 5, 0));
    hz = editor->InsertPI(pi);
    ASSERT_TRUE(hz.IsValid());
    editor->UpdateHorizontalCurveVector(*hz);

    result = editor->DeletePI(DPoint3d::From(50.0, 5, 0));
    ASSERT_TRUE(result.IsValid());
    EXPECT_EQ(1, result->size());

    result = editor->DeletePI(1);
    ASSERT_TRUE(result.IsValid());
    EXPECT_EQ(1, result->size());

    // change first PI to arc type and try to delete it.
    // it should convert it to a NoCurve PI
    pi.InitArc(DPoint3d::FromZero(), 5.0);
    pi.GetArcP()->arc.startPoint = DPoint3d::From(-5.0, -5.0);
    result = editor->MovePI(0, pi);
    ASSERT_TRUE(result.IsValid());
    editor->UpdateHorizontalCurveVector(*result);

    result = editor->DeletePI(0);
    ASSERT_TRUE(result.IsValid());
    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_MovePI()
    {
    // Straight line from (0,0,0) to (100,0,0)
    CurveVectorPtr hz = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    hz->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(200, 0, 0))));

    AlignmentPairEditorPtr editor = AlignmentPairEditor::Create(*hz, nullptr);
    ASSERT_TRUE(editor.IsValid());

    CurveVectorPtr result = editor->MovePI(2, DPoint3d::FromOne());
    EXPECT_FALSE(result.IsValid());

    AlignmentPI pi;
    result = editor->MovePI(0, DPoint3d::From(200, 0, 0), &pi);
    EXPECT_FALSE(result.IsValid()) << "Move PI over another PI should fail";

    result = editor->MovePI(0, DPoint3d::FromZero(), &pi);
    EXPECT_TRUE(result.IsValid()) << "Move PI over itself should not fail";

    result = editor->MovePI(0, DPoint3d::From(10.0, 0, 0));
    EXPECT_TRUE(result.IsValid());

    result = editor->MovePI(2, DPoint3d::From(2, 0, 0));
    EXPECT_FALSE(result.IsValid()) << "Move PI does not move any PI; it should fail";

    result = editor->MovePI(1, DPoint3d::From(105,0,0));
    EXPECT_TRUE(result.IsValid());

    pi = AlignmentPI();
    result = editor->MovePI(1, pi);
    EXPECT_FALSE(result.IsValid()) << "Uninitialized PI cannot be moved";
    pi.InitNoCurve(DPoint3d::FromZero());
    result = editor->MovePI(2, pi);
    EXPECT_FALSE(result.IsValid()) << "Out of bounds";


    // Add a pi and try to move it
    pi;
    pi.InitNoCurve(DPoint3d::From(50.0, 5, 0));
    hz = editor->InsertPI(pi);
    ASSERT_TRUE(hz.IsValid());
    editor->UpdateHorizontalCurveVector(*hz);

    result = editor->MovePI(1, DPoint3d::From(55.0, 25, 0));
    EXPECT_TRUE(result.IsValid());

    // PI Conversions
    pi.InitArc(DPoint3d::From(50.0, 5, 0), 12.0);
    result = editor->MovePI(1, pi);
    ASSERT_TRUE(result.IsValid());
    ASSERT_EQ(3, result->size()); // Line-Arc-Line

    pi.InitSS(DPoint3d::From(100.0, 150, 0), 20.0);
    result = editor->MovePI(1, pi);
    ASSERT_TRUE(result.IsValid());
    ASSERT_EQ(4, result->size()); // Line-Spiral-Spiral-Line

    // Test with curve that starts and ends with an Arc PI
    hz = createHzWithStartEndArcs();
    editor = AlignmentPairEditor::Create(*hz, nullptr);
    ASSERT_TRUE(editor.IsValid());

    result = editor->MovePI(0, DPoint3d::From(5.01, -10));
    EXPECT_FALSE(result.IsValid());

    result = editor->MovePI(1, DPoint3d::From(-0.01, 0.0));
    EXPECT_FALSE(result.IsValid());

    result = editor->MovePI(1, DPoint3d::From(10.01, 0.0));
    EXPECT_FALSE(result.IsValid());

    result = editor->MovePI(2, DPoint3d::From(4.99, -10));
    EXPECT_FALSE(result.IsValid());
    }
//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_MovePC_MovePT()
    {
    AlignmentPairPtr pair = createLinearPair();

    AlignmentPairEditorPtr editor = AlignmentPairEditor::Create(*pair);
    ASSERT_TRUE(editor.IsValid());

    AlignmentPI pi;
    pi.InitArc(DPoint3d::From(50, 50, 0), 20.0);

    CurveVectorPtr hz = editor->InsertPI(pi);
    ASSERT_TRUE(hz.IsValid());
    editor->UpdateHorizontalCurveVector(*hz);

    CurveVectorPtr result = editor->MovePC(1, DPoint3d::From(40, 40, 0), &pi);
    ASSERT_TRUE(result.IsValid());
    double radius;
    ASSERT_EQ(3, result->size());
    ASSERT_EQ(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc, result->at(1)->GetCurvePrimitiveType());
    ASSERT_TRUE(result->at(1)->GetArcCP()->IsCircularXY(radius));
    EXPECT_EQ_DOUBLE(14.142135623730951, radius);

    result = editor->MovePC(1, DPoint3d::From(100, 0, 0));
    EXPECT_FALSE(result.IsValid());

    result = editor->MovePC(1, DPoint3d::From(200, 0, 0));
    EXPECT_FALSE(result.IsValid());

    // Start or end PI. Not valid
    result = editor->MovePC(0, DPoint3d::FromZero());
    EXPECT_FALSE(result.IsValid());
    result = editor->MovePC(2, DPoint3d::From(100, 0, 0));
    EXPECT_FALSE(result.IsValid());
    
    // Out of bounds
    result = editor->MovePC(3, DPoint3d::From(100, 0, 0));
    EXPECT_FALSE(result.IsValid()) << "Out of bounds";

    result = editor->MovePT(1, DPoint3d::From(60, 60, 0));
    ASSERT_TRUE(result.IsValid());
    radius = -1.0;
    ASSERT_EQ(3, result->size());
    ASSERT_EQ(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc, result->at(1)->GetCurvePrimitiveType());
    ASSERT_TRUE(result->at(1)->GetArcCP()->IsCircularXY(radius));
    EXPECT_EQ_DOUBLE(14.142135623730951, radius);

    result = editor->MovePT(1, DPoint3d::From(0, 0, 0));
    EXPECT_FALSE(result.IsValid());

    // With SCS
    editor = AlignmentPairEditor::Create(*pair);
    ASSERT_TRUE(editor.IsValid());

    pi.InitSCS(DPoint3d::From(50, 50, 0), 20, 5, 5);
    hz = editor->InsertPI(pi);
    ASSERT_TRUE(hz.IsValid());
    editor->UpdateHorizontalCurveVector(*hz);

    result = editor->MovePC(1, DPoint3d::From(40, 40, 0));
    ASSERT_TRUE(result.IsValid());

    // With another type of PI that's not start or end PI. Should fail
    pi.InitNoCurve(DPoint3d::From(40, 40));
    hz = editor->MovePI(1, pi);
    ASSERT_TRUE(hz.IsValid());
    editor->UpdateHorizontalCurveVector(*hz);

    result = editor->MovePC(1, DPoint3d::From(40, 40, 0));
    EXPECT_FALSE(result.IsValid()) << "Wrong PI type for MovePC/MovePT";
    result = editor->MovePT(1, DPoint3d::From(40, 40, 0));
    EXPECT_FALSE(result.IsValid()) << "Wrong PI type for MovePC/MovePT";

    }
//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_MoveBS_MoveES()
    {
    // Test with 'Line-SCS-Line-SCS-Line-SCS-Line
    CurveVectorPtr hzWith3Spirals = loadHorizontalWith3Spirals();
    ASSERT_TRUE(hzWith3Spirals.IsValid());

    AlignmentPairEditorPtr editor = AlignmentPairEditor::Create(*hzWith3Spirals, nullptr);
    ASSERT_TRUE(editor.IsValid());

    bvector<AlignmentPI> pis = editor->GetPIs();
    ASSERT_EQ(5, pis.size());
    AlignmentPI::SCSInfoCP pSCS = pis[1].GetSCS();
    ASSERT_TRUE(nullptr != pSCS);

    EXPECT_EQ_DOUBLE(67.0, pSCS->spiral1.length); // &&AG PLACEHOLDER

    // Try to move the BS point to make the spiral 25m long
    DVec3d vec1 = DVec3d::FromStartEndNormalize(pSCS->spiral1.endPoint, pSCS->spiral1.startPoint);
    vec1.ScaleToLength(25);
    DPoint3d toPt1 = DPoint3d::FromSumOf(pSCS->spiral1.endPoint, vec1);

    AlignmentPI resultPI;
    CurveVectorPtr result = editor->MoveBS(1, toPt1, &resultPI);
    ASSERT_TRUE(result.IsValid());
    
    pSCS = resultPI.GetSCS();
    ASSERT_TRUE(nullptr != pSCS);
    EXPECT_EQ_DOUBLE(25.00, pSCS->spiral1.length);
    EXPECT_EQ_DOUBLE(25.00, pSCS->spiral2.length);

    DVec3d vec2 = DVec3d::FromStartEndNormalize(pSCS->spiral2.startPoint, pSCS->spiral2.endPoint);
    vec2.ScaleToLength(30);
    DPoint3d toPt2 = DPoint3d::FromSumOf(pSCS->spiral2.startPoint, vec2);
    resultPI = AlignmentPI();

    result = editor->MoveES(1, toPt2, &resultPI);
    ASSERT_TRUE(result.IsValid());

    pSCS = resultPI.GetSCS();
    ASSERT_TRUE(nullptr != pSCS);
    EXPECT_EQ_DOUBLE(50.934676264190799, pSCS->spiral1.length);
    EXPECT_EQ_DOUBLE(50.934676264190799, pSCS->spiral2.length);

    vec2.ScaleToLength(300);
    toPt2 = DPoint3d::FromSumOf(pSCS->spiral2.startPoint, vec2);
    result = editor->MoveES(1, toPt2);
    EXPECT_TRUE(result.IsValid());
    
    vec2.ScaleToLength(360);
    toPt2 = DPoint3d::FromSumOf(pSCS->spiral2.startPoint, vec2);
    result = editor->MoveES(1, toPt2);
    EXPECT_FALSE(result.IsValid());

    result = editor->MoveBS(0, DPoint3d::FromZero());
    EXPECT_FALSE(result.IsValid()) << "Cannot MoveBS or MoveES on first PI";
    result = editor->MoveES(4, DPoint3d::FromZero());
    EXPECT_FALSE(result.IsValid()) << "Cannot MoveBS or MoveES on last PI";

    result = editor->MoveBS(10, DPoint3d::FromZero());
    EXPECT_FALSE(result.IsValid()) << "Out of bounds";


    }
//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_UpdateRadius()
    {
    // Test with 'Line-Arc-Line-Arc-Line-Arc-Line' curve
    CurveVectorPtr hzWith3Arcs = loadHorizontalWith3Arcs();
    ASSERT_TRUE(hzWith3Arcs.IsValid());

    AlignmentPairEditorPtr editor = AlignmentPairEditor::Create(*hzWith3Arcs, nullptr);
    ASSERT_TRUE(editor.IsValid());

    bvector<AlignmentPI> pis = editor->GetPIs();
    ASSERT_EQ(5, pis.size());
    AlignmentPI pi = pis[1];
    
    AlignmentPI::ArcInfoCP pArc = pi.GetArc();
    ASSERT_TRUE(nullptr != pArc);

    CurveVectorPtr result = editor->UpdateRadius(0, 10);
    EXPECT_FALSE(result.IsValid()) << "Invalid type for UpdateRadius";
    
    result = editor->UpdateRadius(1, 0.0);
    EXPECT_FALSE(result.IsValid()) << "Radius must be greater than 0.0";

    result = editor->UpdateRadius(1, 10.0);
    EXPECT_TRUE(result.IsValid());

    result = editor->UpdateRadius(1, 6000);
    EXPECT_FALSE(result.IsValid()) << "Radius should not fit in this design";

    result = editor->UpdateRadius(5, 5.0);
    EXPECT_FALSE(result.IsValid()) << "Out of bounds";


    // Test with 'Line-SCS-Line-SCS-Line-SCS-Line
    CurveVectorPtr hzWith3Spirals = loadHorizontalWith3Spirals();
    ASSERT_TRUE(hzWith3Spirals.IsValid());

    editor = AlignmentPairEditor::Create(*hzWith3Spirals, nullptr);
    ASSERT_TRUE(editor.IsValid());

    pis = editor->GetPIs();
    ASSERT_EQ(5, pis.size());
    pi = pis[1];

    AlignmentPI::SCSInfoCP pSCS = pi.GetSCS();
    ASSERT_TRUE(nullptr != pSCS);

    result = editor->UpdateRadius(1, 664.0, &pi);
    EXPECT_TRUE(result.IsValid());

    result = editor->UpdateRadius(1, 1400.0);
    EXPECT_FALSE(result.IsValid()) << "SCS should not fit in this design";

    // Replace first SCS by a NoCurvePI and try to update its radius
    pi.InitNoCurve(pis[1].GetPILocation());
    result = editor->MovePI(1, pi);
    ASSERT_TRUE(result.IsValid());
    editor->UpdateHorizontalCurveVector(*result);

    result = editor->UpdateRadius(1, 5.0);
    EXPECT_FALSE(result.IsValid()) << "invalid type for UpdateRadius";
    }
//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_UpdateSpiralLengths()
    {
    // Test with 'Line-SCS-Line-SCS-Line-SCS-Line
    CurveVectorPtr hzWith3Spirals = loadHorizontalWith3Spirals();
    ASSERT_TRUE(hzWith3Spirals.IsValid());

    AlignmentPairEditorPtr editor = AlignmentPairEditor::Create(*hzWith3Spirals, nullptr);
    ASSERT_TRUE(editor.IsValid());

    CurveVectorPtr result = editor->UpdateSpiralLengths(0, 12.3456);
    EXPECT_FALSE(result.IsValid());

    AlignmentPI pi;
    result = editor->UpdateSpiralLengths(1, 12.3456, &pi);
    ASSERT_TRUE(result.IsValid());
    AlignmentPI::SCSInfoCP pSCS = pi.GetSCS();
    ASSERT_TRUE(nullptr != pSCS);
    EXPECT_EQ_DOUBLE(12.3456, pSCS->spiral1.length);
    EXPECT_EQ_DOUBLE(12.3456, pSCS->spiral2.length);

    result = editor->UpdateSpiralLengths(1, 2400);
    EXPECT_FALSE(result.IsValid()) << "Spiral length shouldn't fit this design";

    result = editor->UpdateSpiralLengths(1, 0.0);
    EXPECT_FALSE(result.IsValid());

    result = editor->UpdateSpiralLengths(1, -1.0);
    EXPECT_FALSE(result.IsValid());

    result = editor->UpdateSpiralLengths(5, 1.0);
    EXPECT_FALSE(result.IsValid()) << "Out of bounds";

    // Replace first SCS with an SS
    pi.InitSS(pSCS->overallPI, pSCS->spiral1.length);
    result = editor->MovePI(1, pi);
    ASSERT_TRUE(result.IsValid());
    editor->UpdateHorizontalCurveVector(*result);

    result = editor->UpdateSpiralLengths(1, 12.0);
    EXPECT_TRUE(result.IsValid());
    
    // Replace first SS with a NoCurve (pi not suitable for this function)
    ASSERT_TRUE(editor->GetPI(pi, 1));
    pi.InitNoCurve(pi.GetPILocation());
    result = editor->MovePI(1, pi);
    ASSERT_TRUE(result.IsValid());
    editor->UpdateHorizontalCurveVector(*result);

    result = editor->UpdateSpiralLengths(1, 12.0);
    EXPECT_FALSE(result.IsValid()) << "Invalid type for UpdateSpiralLengths";
    }
//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_RemoveSpirals_AddSpirals()
    {
    // Test with 'Line-SCS-Line-SCS-Line-SCS-Line
    CurveVectorPtr hzWith3Spirals = loadHorizontalWith3Spirals();
    ASSERT_TRUE(hzWith3Spirals.IsValid());

    AlignmentPairEditorPtr editor = AlignmentPairEditor::Create(*hzWith3Spirals, nullptr);
    ASSERT_TRUE(editor.IsValid());

    CurveVectorPtr result = editor->RemoveSpirals(0);
    EXPECT_FALSE(result.IsValid()) << "Not a SCS PI. Cannot remove spirals";

    result = editor->RemoveSpirals(1);
    EXPECT_TRUE(result.IsValid());
    result = editor->RemoveSpirals(2);
    EXPECT_TRUE(result.IsValid());
    result = editor->RemoveSpirals(0);
    EXPECT_FALSE(result.IsValid());
    result = editor->RemoveSpirals(10);
    EXPECT_FALSE(result.IsValid());

    AlignmentPI pi;
    result = editor->RemoveSpirals(3, &pi);
    EXPECT_TRUE(result.IsValid());
    EXPECT_EQ(AlignmentPI::TYPE_Arc, pi.GetType());

    // Test with 'Line-Arc-Line-Arc-Line-Arc-Line' curve
    CurveVectorPtr hzWith3Arcs = loadHorizontalWith3Arcs();
    ASSERT_TRUE(hzWith3Arcs.IsValid());

    editor = AlignmentPairEditor::Create(*hzWith3Arcs, nullptr);
    ASSERT_TRUE(editor.IsValid());

    result = editor->RemoveSpirals(1);
    EXPECT_FALSE(result.IsValid());

    pi = AlignmentPI();
    result = editor->AddSpirals(1, 12.0, &pi);
    ASSERT_TRUE(result.IsValid());
    ASSERT_EQ(AlignmentPI::TYPE_SCS, pi.GetType());
    EXPECT_EQ_DOUBLE(12.0, pi.GetSCS()->spiral1.length);
    EXPECT_EQ_DOUBLE(12.0, pi.GetSCS()->spiral2.length);

    result = editor->AddSpirals(0, 12.0);
    EXPECT_FALSE(result.IsValid());

    result = editor->AddSpirals(10, 5.0);
    EXPECT_FALSE(result.IsValid());
    }


#if 0 //&&AG NEEDSWORK EDITOR
//=======================================================================================
//=======================================================================================
// AlignmentPairEditor starts here
//---------------------------------------------------------------------------------------
// Creates a vertical alignment with:
//  vt: (0, 0, 100) -> (length, 0, slope*length)
//---------------------------------------------------------------------------------------
AlignmentPairEditorPtr createLinearVtAlignmentXZ(double length, double slope)
    {
    const double zEnd = slope * length;

    CurveVectorPtr vt = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    vt->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::From(0, 0, 0), DPoint3d::From(length, 0, zEnd))));

    return AlignmentPairEditor::CreateVerticalOnly(*vt, false);
    }




//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        10/2016
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_RoadPVITests()
    {
    RoadPVI pvi(DPoint3d::FromZero(), 0.0);
    EXPECT_EQ_DOUBLE(0.0, pvi.KValue());
    EXPECT_EQ_DOUBLE(0.0, pvi.LengthFromK(0.0));
    EXPECT_EQ_DOUBLE(0.0, pvi.LengthFromK(100.0));

    RoadPVI pvi2(DPoint3d::FromOne(), 10.0);
    EXPECT_EQ_DOUBLE(0.0, pvi2.KValue());
    EXPECT_EQ_DOUBLE(0.0, pvi2.LengthFromK(0.0));
    EXPECT_EQ_DOUBLE(0.0, pvi2.LengthFromK(100.0));

    RoadPVI pvi3(DPoint3d::From(10.0, 0.0, 1.0), DPoint3d::FromZero(), DPoint3d::From(20.0, 0.0, 1.0), 20.0);
    EXPECT_EQ_DOUBLE(2.0, pvi3.KValue());
    EXPECT_EQ_DOUBLE(0, pvi3.LengthFromK(0.0));
    EXPECT_EQ_DOUBLE(100.0, pvi3.LengthFromK(10.0));

    RoadPVI pvi4(DPoint3d::From(30.0, 0.0, 12.0), DPoint3d::From(00.0, 0.0, 0.0), DPoint3d::From(60.0, 0.0, 0.0), 60.00);
    EXPECT_EQ_DOUBLE(0.75, pvi4.KValue());
    EXPECT_EQ_DOUBLE(0.0, pvi4.LengthFromK(0.0));
    EXPECT_EQ_DOUBLE(4307.2, pvi4.LengthFromK(53.84));
    }


//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        11/2016
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_Create()
    {
    CurveVectorPtr hz = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);

    //! Empty hz
    AlignmentPairEditorPtr editor = AlignmentPairEditor::Create(*hz, nullptr);
    EXPECT_TRUE(editor.IsValid()) << "Editor with empty hz is considered valid";

    //! Hz with single line primitive
    hz->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(100, 0, 0))));
    AlignmentPairEditorPtr editor2 = AlignmentPairEditor::Create(*hz, nullptr);
    EXPECT_TRUE(editor2.IsValid()) << "Failed to create editor with single hz primitive (no vt)";

    //! Hz with single line primitive, empty vt
    CurveVectorPtr emptyVt = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    AlignmentPairEditorPtr editor3 = AlignmentPairEditor::Create(*hz, emptyVt.get());
    EXPECT_TRUE(editor3.IsValid()) << "Failed to create editor with single hz primitive (empty vt)";

    //! Vt-only
    CurveVectorPtr vt = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    AlignmentPairEditorPtr editor4 = AlignmentPairEditor::CreateVerticalOnly(*vt, false);
    EXPECT_TRUE(editor4.IsValid()) << "Empty vertical alignment is considered valid";

    // Create straight vt from [0, 0, 100] to [100, 0, 115]
    vt->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::From(0.0, 0.0, 100.0), DPoint3d::From(100.0, 0.0, 115.0))));

    //! Hz with single line primitive, valid vt
    AlignmentPairEditorPtr editor5 = AlignmentPairEditor::Create(*hz, vt.get());
    EXPECT_TRUE(editor5.IsValid()) << "Failed to create editor with single hz primitive (valid vt)";
    
    //! Vt-only
    AlignmentPairEditorPtr editor6 = AlignmentPairEditor::CreateVerticalOnly(*vt, false);
    EXPECT_TRUE(editor6.IsValid());

    //! Before using anywhere else, make sure the helper methods create something valid
    AlignmentPairEditorPtr editor7 = createLinearVtAlignmentXZ(100.0, 0.0);
    ASSERT_TRUE(editor7.IsValid());
    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        12/2016
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_CreateFromSingleCurveVector()
    {
    CurveVectorPtr curve3d = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    AlignmentPairEditorPtr editor = AlignmentPairEditor::CreateFromSingleCurveVector(*curve3d);
    EXPECT_FALSE(editor.IsValid());

    bvector<DPoint3d> points;
    points.push_back(DPoint3d::From(6400, 3200, 1408));
    points.push_back(DPoint3d::From(6500, 3300, 1504.6));
    points.push_back(DPoint3d::From(6501, 3301, 1504.7));
    curve3d->push_back(ICurvePrimitive::CreateLineString(points));

    AlignmentPairEditorPtr editor2 = AlignmentPairEditor::CreateFromSingleCurveVector(*curve3d);
    ASSERT_TRUE(editor2.IsValid());
    EXPECT_TRUE(editor2->HorizontalCurveVector().IsValid());
    EXPECT_TRUE(editor2->VerticalCurveVector().IsValid());

    CurveVectorPtr hz2 = editor2->HorizontalCurveVector();
    ASSERT_TRUE(hz2.IsValid());
    ASSERT_EQ(2, hz2->size());
    ASSERT_EQ(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, hz2->front()->GetCurvePrimitiveType());
    ASSERT_EQ(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, hz2->back()->GetCurvePrimitiveType());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(6400, 3200, 0), hz2->front()->GetLineCP()->point[0]);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(6500, 3300, 0), hz2->front()->GetLineCP()->point[1]);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(6500, 3300, 0), hz2->back()->GetLineCP()->point[0]);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(6501, 3301, 0), hz2->back()->GetLineCP()->point[1]);
    
    CurveVectorPtr vt2 = editor2->VerticalCurveVector();
    ASSERT_TRUE(vt2.IsValid());
    ASSERT_EQ(2, vt2->size());
    ASSERT_EQ(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, vt2->front()->GetCurvePrimitiveType());
    ASSERT_EQ(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, vt2->back()->GetCurvePrimitiveType());
    EXPECT_EQ_DPOINT3D(DPoint3d::From(0, 0, 1408), vt2->front()->GetLineCP()->point[0]);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(141.42135623730951, 0.0, 1504.6), vt2->front()->GetLineCP()->point[1]);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(141.42135623730951, 0.0, 1504.6), vt2->back()->GetLineCP()->point[0]);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(142.83556979968262, 0.0, 1504.7), vt2->back()->GetLineCP()->point[1]);
    }


//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        12/2016
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_IsVerticalValid()
    {
    AlignmentPairEditorPtr editor = createLinearVtAlignmentXZ(100.0, 0.0);
    EXPECT_TRUE(editor->IsVerticalValid());

    editor->UpdateVerticalCurveVector(nullptr);
    EXPECT_FALSE(editor->IsVerticalValid());

    CurveVectorPtr empty = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    editor->UpdateVerticalCurveVector(empty.get());
    EXPECT_FALSE(editor->IsVerticalValid());
    }


//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        11/2016
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_InsertPVI()
    {
    AlignmentPairEditorPtr editor = createLinearVtAlignmentXZ(100, 0.0);
    RoadStationRangeEdit editRange;

    //! Can't insert PVI at start
    DPoint3d pvi = DPoint3d::FromZero();
    CurveVectorPtr result = editor->InsertParabolicPVI(pvi, 0.0, editRange);
    EXPECT_FALSE(result.IsValid());
    CurveVectorPtr result2 = editor->InsertParabolicPVI(pvi, 12.4, editRange);
    EXPECT_FALSE(result2.IsValid());

    //! Can't insert PVI at end
    DPoint3d pvi2 = DPoint3d::From(100.0, 0, 4.32);
    CurveVectorPtr result3 = editor->InsertParabolicPVI(pvi2, 0.0,  editRange);
    EXPECT_FALSE(result3.IsValid());
    CurveVectorPtr result4 = editor->InsertParabolicPVI(pvi2, 12.4,  editRange);
    EXPECT_FALSE(result4.IsValid());

    //! Insert PVI on the alignment
    DPoint3d pvi3 = DPoint3d::From(15.0, 0, 104.0);
    CurveVectorPtr result5 = editor->InsertParabolicPVI(pvi3, 0.0,  editRange);
    ASSERT_TRUE(result5.IsValid());
    EXPECT_EQ_DOUBLE(1.0, editRange.Ratio());

    //! Insert PVI that fits 'tight'
    CurveVectorPtr result6 = editor->InsertParabolicPVI(pvi3, 29.99,  editRange);
    EXPECT_TRUE(result6.IsValid());

    //! Insert PVI that doesn't fit
    CurveVectorPtr result7 = editor->InsertParabolicPVI(pvi3, 30.00,  editRange);
    EXPECT_FALSE(result7.IsValid());


    editor->UpdateVerticalCurveVector(result6.get());

    //! Insert PVI over existing PVI
    CurveVectorPtr result8 = editor->InsertParabolicPVI(pvi3, 6.0,  editRange);
    EXPECT_FALSE(result8.IsValid());
    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        11/2016
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_DeletePVI()
    {
    AlignmentPairEditorPtr editor = createLinearVtAlignmentXZ(100, 0.0);
    RoadStationRangeEdit editRange;

    CurveVectorPtr vtWith3PIs = editor->InsertParabolicPVI(DPoint3d::From(15.0, 0, 104.0), 12.0,  editRange);
    ASSERT_TRUE(vtWith3PIs.IsValid());

    editor->UpdateVerticalCurveVector(vtWith3PIs.get());

    //! Can't delete PVI at start
    CurveVectorPtr result = editor->DeletePVI(DPoint3d::FromZero(), editRange);
    EXPECT_FALSE(result.IsValid());

    //! Can't delete PVI at end
    CurveVectorPtr result2 = editor->DeletePVI(DPoint3d::From(100, 0, 0), editRange);
    EXPECT_FALSE(result2.IsValid());

    //! Delete PVI at wrong X location
    CurveVectorPtr result3 = editor->DeletePVI(DPoint3d::From(14.0, 0, 104.0), editRange);
    EXPECT_FALSE(result3.IsValid());

    //! Delete the center PVI.
    //! notice this method doesn't care about z-value of the point we're passing. Only uses 'point.x'
    CurveVectorPtr result4 = editor->DeletePVI(DPoint3d::From(15.0, 0, 999.0), editRange);
    ASSERT_TRUE(result4.IsValid());
    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        11/2016
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_MovePVI()
    {
    AlignmentPairEditorPtr editor = createLinearVtAlignmentXZ(100, 0.0);
    RoadStationRangeEdit editRange;

    CurveVectorPtr vtWith3PIs = editor->InsertParabolicPVI(DPoint3d::From(15.0, 0, 104.0), 12.0,  editRange);
    ASSERT_TRUE(vtWith3PIs.IsValid());
    editor->UpdateVerticalCurveVector(vtWith3PIs.get());


    //! Can't modify 'x' of start PVI. It will only update the 'z'
    CurveVectorPtr result = editor->MovePVI(DPoint3d::FromZero(), DPoint3d::From(1.0, 0.0, 1.0), editRange);
    ASSERT_TRUE(result.IsValid());
    DPoint3d start, dummy;
    result->GetStartEnd(start, dummy);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(0.0, 0.0, 1.0), start);

    //! Modify 'z' of start PVI
    CurveVectorPtr result2 = editor->MovePVI(DPoint3d::FromZero(), DPoint3d::From(0, 0, 5.0), editRange);
    ASSERT_TRUE(result2.IsValid());
    DPoint3d start2;
    result2->GetStartEnd(start2, dummy);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(0, 0, 5.0), start2);

    //! Can't modify 'x' of end PVI. It will only update the 'z'
    CurveVectorPtr result3 = editor->MovePVI(DPoint3d::From(100, 0, 0), DPoint3d::From(99, 0, 2.4), editRange);
    ASSERT_TRUE(result3.IsValid());
    DPoint3d end3;
    result3->GetStartEnd(dummy, end3);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100, 0, 2.4), end3);

    //! Modify 'z' of end PVI
    CurveVectorPtr result4 = editor->MovePVI(DPoint3d::From(100, 0, 0), DPoint3d::From(100, 0, 4.5), editRange);
    ASSERT_TRUE(result4.IsValid());
    DPoint3d end4;
    result4->GetStartEnd(dummy, end4);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100, 0, 4.5), end4);

    //! Can modify XZ of center PVI
    CurveVectorPtr result5 = editor->MovePVI(DPoint3d::From(15.0, 0, 104.0), DPoint3d::From(12.0, 0, 98.0), editRange);
    ASSERT_TRUE(result5.IsValid());

    //! Can't modify XZ of center PVI if it results in overlapping
    CurveVectorPtr result6 = editor->MovePVI(DPoint3d::From(15.0, 0, 104.0), DPoint3d::From(5.0, 0, 94.0), editRange);
    EXPECT_FALSE(result6.IsValid());
    }


//---------------------------------------------------------------------------------------
// @betest                               Colin.Jin                         08/2017
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_MoveSinglePVIWithK()
    {
    AlignmentPairEditorPtr editor = createLinearVtAlignmentXZ(100, 0.0);
    RoadStationRangeEdit editRange;

    // Test for Parabolic curve
    CurveVectorPtr vtWith3PIs = editor->InsertParabolicPVI(DPoint3d::From(50.0, 0, 20.0), 12.0, editRange);
    ASSERT_TRUE(vtWith3PIs.IsValid());
    editor->UpdateVerticalCurveVector(vtWith3PIs.get());

    bvector<RoadPVI> pvis = editor->GetPVIs();
    double currentK = pvis[1].KValue();
    EXPECT_EQ_DOUBLE(0.15, currentK);

    //Modify center PVI, to form a crest curve, K should be locked
    CurveVectorPtr result = editor->MovePVIWithK(DPoint3d::From(50.0, 0, 20.0), DPoint3d::From(40.0, 0, 30.0), editRange);
    ASSERT_TRUE(result.IsValid());
    
    editor->UpdateVerticalCurveVector(result.get());
    bvector<RoadPVI> pvis2 = editor->GetPVIs();
    DPoint3d newPVI = pvis2[1].poles[1];
    EXPECT_EQ_DPOINT3D(DPoint3d::From(40.0, 0, 30.0), newPVI);
    double newK = pvis2[1].KValue();
    EXPECT_EQ_DOUBLE(0.15, newK);

    //Modify center PVI, to form a sag curve, K should be locked
    CurveVectorPtr result2 = editor->MovePVIWithK(DPoint3d::From(40.0, 0, 30.0), DPoint3d::From(50.0, 0, -20.0), editRange);
    ASSERT_TRUE(result.IsValid());

    editor->UpdateVerticalCurveVector(result2.get());
    bvector<RoadPVI> pvis3 = editor->GetPVIs();
    newPVI = pvis3[1].poles[1];
    EXPECT_EQ_DPOINT3D(DPoint3d::From(50.0, 0, -20.0), newPVI);
    newK = pvis3[1].KValue();
    EXPECT_EQ_DOUBLE(0.15, newK);

    //! Can't modify center PVI if it results in overlapping
    CurveVectorPtr result3 = editor->MovePVIWithK(DPoint3d::From(50.0, 0, -20.0), DPoint3d::From(5.0, 0, 0.0), editRange);
    EXPECT_FALSE(result3.IsValid());

    // Test for Circular Curve
    editor = createLinearVtAlignmentXZ(100, 0.0);
    vtWith3PIs = editor->InsertCircularPVI(DPoint3d::From(50.0, 0, 20.0), 12.0, editRange);
    ASSERT_TRUE(vtWith3PIs.IsValid());
    editor->UpdateVerticalCurveVector(vtWith3PIs.get());

    bvector<RoadPVI> pvis4 = editor->GetPVIs();
    currentK = pvis4[1].radius / 100.0;
    EXPECT_EQ_DOUBLE(0.12, currentK);

    //Modify center PVI, to form a crest curve, K should be locked
    CurveVectorPtr result4 = editor->MovePVIWithK(DPoint3d::From(50.0, 0, 20.0), DPoint3d::From(40.0, 0, 30.0), editRange);
    ASSERT_TRUE(result4.IsValid());

    editor->UpdateVerticalCurveVector(result4.get());
    bvector<RoadPVI> pvis5 = editor->GetPVIs();
    newPVI = pvis5[1].poles[1];
    EXPECT_EQ_DPOINT3D(DPoint3d::From(40.0, 0, 30.0), newPVI);
    newK = pvis5[1].radius / 100.0;
    EXPECT_EQ_DOUBLE(0.12, newK);

    //Modify center PVI, to form a sag curve, K should be locked
    CurveVectorPtr result5 = editor->MovePVIWithK(DPoint3d::From(40.0, 0, 30.0), DPoint3d::From(50.0, 0, -20.0), editRange);
    ASSERT_TRUE(result5.IsValid());

    editor->UpdateVerticalCurveVector(result5.get());
    bvector<RoadPVI> pvis6 = editor->GetPVIs();
    newPVI = pvis6[1].poles[1];
    EXPECT_EQ_DPOINT3D(DPoint3d::From(50.0, 0, -20.0), newPVI);
    newK = pvis6[1].radius / 100.0;
    EXPECT_EQ_DOUBLE(0.12, newK);

    //! Can't modify center PVI if it results in overlapping
    CurveVectorPtr result6 = editor->MovePVIWithK(DPoint3d::From(50.0, 0, -20.0), DPoint3d::From(5.0, 0, 0.0), editRange);
    EXPECT_FALSE(result6.IsValid());
    }


//---------------------------------------------------------------------------------------
// @betest                               Colin.Jin                         08/2017
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_MoveTwoPVIsWithK()
    {
    AlignmentPairEditorPtr editor = createLinearVtAlignmentXZ(100, 0.0);
    RoadStationRangeEdit editRange;

    // Test for parabolic curves
    // Create a curve with 2 parabolic PVIs
    CurveVectorPtr vtWith4PIs = editor->InsertParabolicPVI(DPoint3d::From(30.0, 0, 20.0), 12.0, editRange);
    ASSERT_TRUE(vtWith4PIs.IsValid());
    editor->UpdateVerticalCurveVector(vtWith4PIs.get());

    vtWith4PIs = editor->InsertParabolicPVI(DPoint3d::From(70.0, 0, 20.0), 12.0, editRange);
    ASSERT_TRUE(vtWith4PIs.IsValid());
    editor->UpdateVerticalCurveVector(vtWith4PIs.get());

    bvector<RoadPVI> pvis = editor->GetPVIs();
    double kValue1 = pvis[1].KValue();
    EXPECT_EQ_DOUBLE(0.18, kValue1);
    double kValue2 = pvis[2].KValue();
    EXPECT_EQ_DOUBLE(0.18, kValue2);

    // Move one PVI, to form a crest curve, K for both PVIs should be locked
    CurveVectorPtr result = editor->MovePVIWithK(DPoint3d::From(30.0, 0, 20.0), DPoint3d::From(40.0, 0, 50.0), editRange);
    ASSERT_TRUE(result.IsValid());
    editor->UpdateVerticalCurveVector(result.get());

    bvector<RoadPVI> pvis2 = editor->GetPVIs();
    DPoint3d newPVI = pvis2[1].poles[1];
    EXPECT_EQ_DPOINT3D(DPoint3d::From(40.0, 0, 50.0), newPVI);
    double newK1 = pvis2[1].KValue();
    EXPECT_EQ_DOUBLE(0.18, newK1);
    double newK2 = pvis[2].KValue();
    EXPECT_EQ_DOUBLE(0.18, newK2);

    // Move one PVI, to form a sag curve, K for both PVIs should be locked
    CurveVectorPtr result2 = editor->MovePVIWithK(DPoint3d::From(40.0, 0, 50.0), DPoint3d::From(40.0, 0, -10.0), editRange);
    ASSERT_TRUE(result2.IsValid());
    editor->UpdateVerticalCurveVector(result2.get());

    bvector<RoadPVI> pvis3 = editor->GetPVIs();
    DPoint3d newPVI2 = pvis3[1].poles[1];
    EXPECT_EQ_DPOINT3D(DPoint3d::From(40.0, 0, -10.0), newPVI2);
    newK1 = pvis3[1].KValue();
    EXPECT_EQ_DOUBLE(0.18, newK1);
    newK2 = pvis3[2].KValue();
    EXPECT_EQ_DOUBLE(0.18, newK2);

    //! Can't move PVI if it results in overlapping
    CurveVectorPtr result3 = editor->MovePVIWithK(DPoint3d::From(40.0, 0, -10.0), DPoint3d::From(70.0, 0, 20.0), editRange);
    EXPECT_FALSE(result3.IsValid());

    // Test for circular curves
    // Create a curve with 2 Circular PVIs
    editor = createLinearVtAlignmentXZ(100, 0.0);
    vtWith4PIs = editor->InsertCircularPVI(DPoint3d::From(30.0, 0, 20.0), 12.0, editRange);
    ASSERT_TRUE(vtWith4PIs.IsValid());
    editor->UpdateVerticalCurveVector(vtWith4PIs.get());

    vtWith4PIs = editor->InsertCircularPVI(DPoint3d::From(70.0, 0, 20.0), 12.0, editRange);
    ASSERT_TRUE(vtWith4PIs.IsValid());
    editor->UpdateVerticalCurveVector(vtWith4PIs.get());

    bvector<RoadPVI> pvis4 = editor->GetPVIs();
    DPoint3d newPVI3 = pvis4[1].poles[1];
    EXPECT_EQ_DPOINT3D(DPoint3d::From(30.0, 0, 20.0), newPVI3);
    newK1 = pvis4[1].radius / 100.0;
    EXPECT_EQ_DOUBLE(0.12, newK1);
    newK2 = pvis4[2].radius / 100.0;
    EXPECT_EQ_DOUBLE(0.12, newK2);

    // Move one PVI, to form a crest curve, K for both PVIs should be locked
    CurveVectorPtr result4 = editor->MovePVIWithK(DPoint3d::From(30.0, 0, 20.0), DPoint3d::From(40.0, 0, 50.0), editRange);
    ASSERT_TRUE(result4.IsValid());

    editor->UpdateVerticalCurveVector(result4.get());
    bvector<RoadPVI> pvis5 = editor->GetPVIs();
    newPVI = pvis5[1].poles[1];
    EXPECT_EQ_DPOINT3D(DPoint3d::From(40.0, 0, 50.0), newPVI);
    newK1 = pvis5[1].radius / 100.0;
    EXPECT_EQ_DOUBLE(0.12, newK1);
    newK2 = pvis5[2].radius / 100.0;
    EXPECT_EQ_DOUBLE(0.12, newK2);

    // Move one PVI, to form a sag curve, K for both PVIs should be locked
    CurveVectorPtr result5 = editor->MovePVIWithK(DPoint3d::From(40.0, 0, 50.0), DPoint3d::From(40.0, 0, -20.0), editRange);
    ASSERT_TRUE(result5.IsValid());

    editor->UpdateVerticalCurveVector(result5.get());
    bvector<RoadPVI> pvis6 = editor->GetPVIs();
    newPVI = pvis6[1].poles[1];
    EXPECT_EQ_DPOINT3D(DPoint3d::From(40.0, 0, -20.0), newPVI);
    newK1 = pvis6[1].radius / 100.0;
    EXPECT_EQ_DOUBLE(0.12, newK1);
    newK2 = pvis6[2].radius / 100.0;
    EXPECT_EQ_DOUBLE(0.12, newK2);

    //! Can't move PVI if it results in overlapping
    CurveVectorPtr result6 = editor->MovePVIWithK(DPoint3d::From(40.0, 0, -20.0), DPoint3d::From(60.0, 20.0, 0.0), editRange);
    EXPECT_FALSE(result6.IsValid());
    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        11/2016
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_MoveVerticalTangent()
    {
    // [0, 0, 0] --> [100, 0, 10]
    AlignmentPairEditorPtr editor = createLinearVtAlignmentXZ(100, 0.10); // 10% slope
    RoadStationRangeEdit editRange;

    CurveVectorPtr result = editor->MoveVerticalTangent(DPoint3d::From(50.0, 0, 0), DPoint3d::From(50.0, 0, 10), editRange);
    ASSERT_TRUE(result.IsValid());
    DPoint3d start, end;
    result->GetStartEnd(start, end);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(0, 0, 10), start);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100, 0, 20), end);

    CurveVectorPtr vtWith3PIs = editor->InsertParabolicPVI(DPoint3d::From(15.0, 0, 104.0), 12.0,  editRange);
    ASSERT_TRUE(vtWith3PIs.IsValid());
    AlignmentPairEditorPtr editor2 = AlignmentPairEditor::CreateVerticalOnly(*vtWith3PIs, false);
    ASSERT_TRUE(editor2.IsValid());


    const double zBefore = editor2->GetVerticalElevationAtStation(15.0);
    CurveVectorPtr result2 = editor2->MoveVerticalTangent(DPoint3d::From(7.0, 0, 10.0), DPoint3d::From(7.0, 0, 15.0), editRange);
    ASSERT_TRUE(result2.IsValid());
    editor2->UpdateVerticalCurveVector(result2.get());

    const double zAfter = editor2->GetVerticalElevationAtStation(15.0);
    EXPECT_EQ_DOUBLE(4.9117647058823479, zAfter - zBefore);
    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        11/2016
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_MovePVCorPVT()
    {
    AlignmentPairEditorPtr editor = createLinearVtAlignmentXZ(30, 0.0);
    RoadStationRangeEdit editRange;

    //! PVI at [15, 0, 104]
    //! PVC at x = 9.0
    //! PVT at x = 21.0
    CurveVectorPtr vtWith3PIs = editor->InsertParabolicPVI(DPoint3d::From(15.0, 0, 104.0), 12.0,  editRange);
    ASSERT_TRUE(vtWith3PIs.IsValid());
    editor->UpdateVerticalCurveVector(vtWith3PIs.get());

    //! move PVC over start PVI
    CurveVectorPtr result = editor->MovePVCorPVT(9.0, 0.0, editRange);
    EXPECT_FALSE(result.IsValid());

    //! move PVC over its PVI
    //! this is valid and leads to a PVI with a length of 0.0
    CurveVectorPtr result2 = editor->MovePVCorPVT(9.0, 15.0, editRange);
    EXPECT_TRUE(result2.IsValid());

    //! move PVC beyond its PVI
    CurveVectorPtr result3 = editor->MovePVCorPVT(9.0, 16.0, editRange);
    EXPECT_FALSE(result3.IsValid());

    //! move PVT over end PVI
    CurveVectorPtr result4 = editor->MovePVCorPVT(21.0, 30.0, editRange);
    EXPECT_FALSE(result4.IsValid());

    //! move PVC over its PVI
    // this is valid and leads to a PVI with a length of 0.0
    CurveVectorPtr result5 = editor->MovePVCorPVT(21.0, 15.0, editRange);
    EXPECT_TRUE(result5.IsValid());

    //! move PVC beyond its PVI
    CurveVectorPtr result6 = editor->MovePVCorPVT(21.0, 14.0, editRange);
    EXPECT_FALSE(result6.IsValid());

    //! move PVC a bit on the left (longer PVI)
    CurveVectorPtr result7 = editor->MovePVCorPVT(9.0, 7.0, editRange);
    EXPECT_TRUE(result7.IsValid());

    //! move PVC a bit on the right (shorter PVI)
    CurveVectorPtr result8 = editor->MovePVCorPVT(9.0, 11.0, editRange);
    EXPECT_TRUE(result8.IsValid());

    //! move PVT a bit on the left (shorter PVI)
    CurveVectorPtr result9 = editor->MovePVCorPVT(21.0, 19.0, editRange);
    EXPECT_TRUE(result9.IsValid());

    //! move PVT a bit on the right (longer PVI)
    CurveVectorPtr result10 = editor->MovePVCorPVT(21.0, 23.0, editRange);
    EXPECT_TRUE(result10.IsValid());
    }



//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
AlignmentPairEditorPtr RoadVGeometrySetup()
    {
    CurveVectorPtr vAlign = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    vAlign->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 100.0, 1000.0, 0.0, 500.0)));
    AlignmentPairEditorPtr roadPtr = AlignmentPairEditor::CreateVerticalOnly(*vAlign, false);

    return roadPtr;
    }
//---------------------------------------------------------------------------------------
// @betest                                      Scott.Devoe                     07/2015
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_VGeometry()
    {
    AlignmentPairEditorPtr roadV = RoadVGeometrySetup();
    ASSERT_TRUE(roadV.IsValid());

    DPoint3d pt = DPoint3d::From(250.0, 0.0, 300.0);

    RoadStationRangeEdit editRange;
    // test insertion
    CurveVectorPtr output = roadV->InsertParabolicPVI(pt, 200.0,  editRange);
    roadV->UpdateCurveVectors(*roadV->HorizontalCurveVector(), output.get());
    ASSERT_EQ(output->size(), 3);

    // test delete
    output = roadV->DeletePVI(pt, editRange);
    roadV->UpdateCurveVectors(*roadV->HorizontalCurveVector(), output.get());
    ASSERT_EQ(output->size(), 1);

    DPoint3d pt1 = DPoint3d::From(250.0, 0.0, 70.0);   // create a sag point
    DPoint3d pt2 = DPoint3d::From(700.0, 0.0, 1250.0); // create a crest point

    output = roadV->InsertParabolicPVI(pt1, 200.0,  editRange);
    roadV->UpdateCurveVectors(*roadV->HorizontalCurveVector(), output.get());
    output = roadV->InsertParabolicPVI(pt2, 300.0,  editRange);
    roadV->UpdateCurveVectors(*roadV->HorizontalCurveVector(), output.get());

    double gradeChange = roadV->MaximumGradeChangeInPercent();
    ASSERT_EQ(round(gradeChange), 512.0);

    double maxGrade = roadV->MaximumGradeInPercent();
    ASSERT_EQ(round(maxGrade), 262.0);

    // test moving a PVI
    DPoint3d newLocation = DPoint3d::From(250.0, 0.0, 50.0);
    output = roadV->MovePVI(pt1, newLocation, editRange);
    roadV->UpdateCurveVectors(*roadV->HorizontalCurveVector(), output.get());
    maxGrade = roadV->MaximumGradeInPercent();
    ASSERT_EQ(round(maxGrade), 267.0);

    // test changing a vertical curve length
    // we have a PVT at 350, let's move it to 375
    // that should make the length of the resultant curve
    // in the vector 250
    output = roadV->MovePVCorPVT(350.0, 375.0, editRange);
    roadV->UpdateCurveVectors(*roadV->HorizontalCurveVector(), output.get());
    ICurvePrimitivePtr childPrimitive = output->at(1);
    ASSERT_TRUE(childPrimitive.IsValid());
    MSBsplineCurveCP bspline = childPrimitive->GetBsplineCurveCP();
    ASSERT_TRUE(bspline != nullptr);
    bvector<DPoint3d> poles;
    bspline->GetPoles(poles);
    ASSERT_TRUE(poles.size() == 3);
    double length = fabs(poles[0].x - poles[2].x);
    ASSERT_EQ(round(length), 250);

    // test moving the tangent
    double test1a = roadV->GetVerticalElevationAtStation(490.0);
    double test2a = roadV->GetVerticalElevationAtStation(510.0);
    DPoint3d fromPt = DPoint3d::From(500.0, 90.0, 90.0);
    DPoint3d toPt = DPoint3d::From(500.0, 100.0, 100.0); // raise 10 units
    output = roadV->MoveVerticalTangent(fromPt, toPt, editRange);
    roadV->UpdateCurveVectors(*roadV->HorizontalCurveVector(), output.get());
    double test1b = roadV->GetVerticalElevationAtStation(490.0);
    double test2b = roadV->GetVerticalElevationAtStation(510.0);

    ASSERT_TRUE(DoubleOps::AlmostEqual(10.0, fabs(test1b - test1a)));
    ASSERT_TRUE(DoubleOps::AlmostEqual(10.0, fabs(test2b - test2a)));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Scott.Devoe                     07/2015
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_VGeometryCrestAndSag()
    {
    AlignmentPairEditorPtr roadV = RoadVGeometrySetup();
    ASSERT_TRUE(roadV.IsValid());

    DPoint3d pt1 = DPoint3d::From(250.0, 0.0, 70.0);   // create a sag point
    DPoint3d pt2 = DPoint3d::From(700.0, 0.0, 1250.0); // create a crest point

    RoadStationRangeEdit editRange;
    CurveVectorPtr output = roadV->InsertParabolicPVI(pt1, 200.0,  editRange);
    roadV->UpdateCurveVectors(*roadV->HorizontalCurveVector(), output.get());
    output = roadV->InsertParabolicPVI(pt2, 300.0,  editRange);
    roadV->UpdateCurveVectors(*roadV->HorizontalCurveVector(), output.get());

    ASSERT_EQ(editRange.postEditRange.startStation, 150.0);
    ASSERT_EQ(editRange.postEditRange.endStation, 1000.0);

    // test the crest and sag points..
    bvector<double> idPts = roadV->CrestAndSagPointsStation();
    ASSERT_EQ(idPts.size(), 2);

    ASSERT_EQ(round(idPts[0]), 159.0);
    ASSERT_EQ(round(idPts[1]), 704.0);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Scott.Devoe                     04/2016
//---------------------------------------------------------------------------------------
void AlignmentPairEditor_VGeometryThrough()
    {
    AlignmentPairEditorPtr roadV = RoadVGeometrySetup();
    ASSERT_TRUE(roadV.IsValid());

    RoadStationRangeEdit rangeEdit;
    //DPoint3d throughPt = DPoint3d::From (750.0, 0.0, 200.0);
    //CurveVectorPtr newVert = roadV->ForceThroughPoint (throughPt, rangeEdit);

    //ASSERT_TRUE (DoubleOps::AlmostEqual (rangeEdit.postEditRange.startStation, 0.0));
    //ASSERT_TRUE (DoubleOps::AlmostEqual (rangeEdit.postEditRange.endStation, 1000.0));

    CurveVectorPtr newVert = roadV->ForceGradeAtStation(500.0, 0.02, rangeEdit);
    DPoint3d start, end;
    newVert->GetStartEnd(start, end);
    ASSERT_TRUE(DoubleOps::AlmostEqual(end.z, 120.0));

    ASSERT_TRUE(DoubleOps::AlmostEqual(rangeEdit.postEditRange.startStation, 0.0));
    ASSERT_TRUE(DoubleOps::AlmostEqual(rangeEdit.postEditRange.endStation, 1000.0));
    }


#endif


//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        10/2016
//---------------------------------------------------------------------------------------
TEST_F(CivilBaseGeometryTests, AlignmentPairTests)
    {
    //! AlignmentPair tests
    AlignmentPair_Create();
    AlignmentPair_GetCurveVectors();
    AlignmentPair_UpdateCurveVectors();
    AlignmentPair_CloneCurveVectors();
    AlignmentPair_Clone();
    AlignmentPair_GetStartEnd();
    AlignmentPair_HasSpirals();
    AlignmentPair_LengthXY();
    AlignmentPair_GetVerticalElevationAt();
    AlignmentPair_GetPointAt();
    AlignmentPair_GetPointAndTangentAt();
    AlignmentPair_GetPointAtAndOffset();
    AlignmentPair_HorizontalDistanceAlongFrom();
    AlignmentPair_GetPrimitiveAtPoint_GetPrimitiveAtStation_GetPathLocationDetailAtStation();
    AlignmentPair_ClosestPoint();

#if ENABLE_SELF_INTERSECT_TEST
    AlignmentPair_SelfIntersects();
#endif
    AlignmentPair_GetPartialAlignment();
    AlignmentPair_GetStrokedAlignment();
    AlignmentPair_ComputeMinimumRadius();

    // ALignmentPI tests
    AlignmentPI_Tests();

    //! AlignmentPairEditor tests
    AlignmentPairEditor_Create_Clone();
    AlignmentPairEditor_GetPIs();
    AlignmentPairEditor_UpdateCurveVectors();
    AlignmentPairEditor_InsertPI();
    AlignmentPairEditor_DeletePI();
    AlignmentPairEditor_MovePI();
    AlignmentPairEditor_MovePC_MovePT();
    AlignmentPairEditor_MoveBS_MoveES();
    AlignmentPairEditor_UpdateRadius();
    AlignmentPairEditor_UpdateSpiralLengths();
    AlignmentPairEditor_RemoveSpirals_AddSpirals();

#if 0 //&&AG NEEDSWORK EDITOR
    AlignmentPairEditor_RoadPVITests();

    AlignmentPairEditor_Create();
    AlignmentPairEditor_CreateFromSingleCurveVector();

    //! Vertical editing
    AlignmentPairEditor_IsVerticalValid();
    AlignmentPairEditor_InsertPVI();
    AlignmentPairEditor_DeletePVI();
    AlignmentPairEditor_MovePVI();
    AlignmentPairEditor_MoveSinglePVIWithK();
    AlignmentPairEditor_MoveTwoPVIsWithK();
    AlignmentPairEditor_MoveVerticalTangent();
    AlignmentPairEditor_MovePVCorPVT();
    AlignmentPairEditor_VGeometry();
    AlignmentPairEditor_VGeometryCrestAndSag();
    AlignmentPairEditor_VGeometryThrough();
#endif
    }
