/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/AlignmentPair_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../BackDoor/PublicApi/BackDoor/RoadRailAlignment/BackDoor.h"

#define ENABLE_SELF_INTERSECT_TEST 0
#define METERS_TO_ENGLISH_FEET 3.28083989501
#define METERS_TO_ENGLISH_SURVEY_FEET 3.2808333333465

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

    //! Retrieve as meters
    expectEqualCurves(*hz, pair->GetHorizontalCurveVector());
    expectEqualCurves(*vt, *pair->GetVerticalCurveVector());
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
    AlignmentPairPtr pair = createLinearPair();

    //! Clone as EnglishFeet
    CurveVectorPtr englishFeetHz = pair->CloneHorizontalCurveVector(Dgn::StandardUnit::EnglishFeet);
    EXPECT_TRUE(englishFeetHz.IsValid());

    DPoint3d efStart, efEnd;
    englishFeetHz->GetStartEnd(efStart, efEnd);

    DPoint3d expectedEfEnd = DPoint3d::FromScale(DPoint3d::From(100.0, 0, 0), METERS_TO_ENGLISH_FEET);
    EXPECT_EQ_DPOINT3D(DPoint3d::FromZero(), efStart);
    EXPECT_EQ_DPOINT3D(expectedEfEnd, efEnd);

    //! Retrieve as EnglishSurveyFeet
    CurveVectorPtr englishSurveyFeetHz = pair->CloneHorizontalCurveVector(Dgn::StandardUnit::EnglishSurveyFeet);
    EXPECT_TRUE(englishSurveyFeetHz.IsValid());

    DPoint3d esfStart, esfEnd;
    englishSurveyFeetHz->GetStartEnd(esfStart, esfEnd);

    DPoint3d expectedEsfEnd = DPoint3d::FromScale(DPoint3d::From(100.0, 0, 0), METERS_TO_ENGLISH_SURVEY_FEET);
    EXPECT_EQ_DPOINT3D(DPoint3d::FromZero(), esfStart);
    EXPECT_EQ_DPOINT3D(expectedEsfEnd, esfEnd);


    CurveVectorPtr englishFeetVt = pair->CloneVerticalCurveVector(Dgn::StandardUnit::EnglishFeet);
    EXPECT_TRUE(englishFeetVt.IsValid());

    DPoint3d efVtStart, efVtEnd;
    englishFeetVt->GetStartEnd(efVtStart, efVtEnd);

    DPoint3d expectedEfVtStart = DPoint3d::FromScale(DPoint3d::From(0,0,800), METERS_TO_ENGLISH_FEET);
    DPoint3d expectedEfVtEnd = DPoint3d::FromScale(DPoint3d::From(100, 0, 815), METERS_TO_ENGLISH_FEET);
    EXPECT_EQ_DPOINT3D(expectedEfVtStart, efVtStart);
    EXPECT_EQ_DPOINT3D(expectedEfVtEnd, efVtEnd);
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

    EXPECT_EQ_DPOINT3D(DPoint3d::From(0.0, -5.0, 0.0), pair->GetPointAtAndOffset(0.0, 5.0));
    EXPECT_EQ_DPOINT3D(DPoint3d::From(25, 0.0, 0.0), pair->GetPointAtAndOffset(25.0, 0.0));
    EXPECT_EQ_DPOINT3D(DPoint3d::From(100.0, 8.0, 0.0), pair->GetPointAtAndOffset(100.0, -8.0));
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
void AlignmentPair_GetPrimitiveAtPoint()
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
    }

//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        10/2016
// This test covers ClosestPoint, ClosestPointXY and ClosestPointAndTangentXY
//---------------------------------------------------------------------------------------
void AlignmentPair_ClosestPoint()
    {
    AlignmentPairPtr pair = createLinearPair();

    //! at start
    DPoint3d located;
    DPoint3d locatedZ;
    DPoint3d locatedPT;
    DVec3d locatedTG;
    EXPECT_TRUE(pair->ClosestPointXY(located, DPoint3d::From(0.0, 5.0, 12.0)));
    EXPECT_TRUE(pair->ClosestPoint(locatedZ, DPoint3d::From(0.0, 5.0, 12.0)));
    EXPECT_TRUE(pair->ClosestPointAndTangentXY(locatedPT, locatedTG, DPoint3d::From(0.0, 5.0, 12.0)));
    EXPECT_EQ_DPOINT3D(DPoint3d::FromZero(), located);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(0,0, 800), locatedZ);
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
    
    //! Clone vt partial range
    CurveVectorPtr partialVt2 = pair->GetPartialVerticalAlignment(25.0, 75.0);
    DPoint3d start4, end4;
    partialVt2->GetStartEnd(start4, end4);

    EXPECT_EQ_DPOINT3D(DPoint3d::From(0.0, 0, 803.75), start4);
    EXPECT_EQ_DPOINT3D(DPoint3d::From(50.0, 0, 811.25), end4);

    //! Clone vt null range
    CurveVectorPtr partialVt3 = pair->GetPartialVerticalAlignment(10.0, 10.0);
    EXPECT_FALSE(partialVt3.IsValid());
    
    
    //! Clone alignment full range
    AlignmentPairPtr cPair = pair->GetPartialAlignment(0.0, 100.0);
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



#if 0 //&&AG NEEDSWORK EDITOR

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
TEST_F(RoadRailAlignmentTests, AlignmentPairTests)
    {
    //! AlignmentPair tests
    AlignmentPair_Create();
    AlignmentPair_GetCurveVectors();
    AlignmentPair_UpdateCurveVectors();
    AlignmentPair_CloneCurveVectors();
    AlignmentPair_Clone();
    AlignmentPair_GetStartEnd();
    AlignmentPair_LengthXY();
    AlignmentPair_GetVerticalElevationAt();
    AlignmentPair_GetPointAt();
    AlignmentPair_GetPointAndTangentAt();
    AlignmentPair_GetPointAtAndOffset();
    AlignmentPair_HorizontalDistanceAlongFrom();
    AlignmentPair_GetPrimitiveAtPoint();
    AlignmentPair_ClosestPoint();

#if ENABLE_SELF_INTERSECT_TEST
    AlignmentPair_SelfIntersects();
#endif
    AlignmentPair_GetPartialAlignment();
    AlignmentPair_GetStrokedAlignment();
    AlignmentPair_ComputeMinimumRadius();


#if 0 //&&AG NEEDSWORK EDITOR
    //! AlignmentPairEditor tests
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
