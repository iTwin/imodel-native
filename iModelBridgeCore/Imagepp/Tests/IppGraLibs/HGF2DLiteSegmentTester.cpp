//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DLiteSegmentTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "HGF2DLiteSegmentTester.h"

HGF2DLiteSegmentTester::HGF2DLiteSegmentTester() 
    {

     // VERTICAL SEGMENT
    VerticalSegment1 = HGF2DLiteSegment(HGF2DPosition(0.1, 0.1), HGF2DPosition(0.1, 10.1));
    VerticalSegment2 = HGF2DLiteSegment(HGF2DPosition(0.1, 10.1), HGF2DPosition(0.1, 0.1));
    VerticalSegment3 = HGF2DLiteSegment(HGF2DPosition(0.1, 0.1), HGF2DPosition(0.1, 0.1 + MYEPSILON));
    VerticalSegment4 = HGF2DLiteSegment(HGF2DPosition(0.1 + MYEPSILON, 0.1), HGF2DPosition(0.1+MYEPSILON, 10.1));
    VerticalSegment5 = HGF2DLiteSegment(HGF2DPosition(-10.0, -10.0), HGF2DPosition(-10.0, 0.0));
    CloseVerticalSegment1 = HGF2DLiteSegment(HGF2DPosition(0.1+MYEPSILON, 0.1), HGF2DPosition(0.1, 10.1));

    VerticalPoint0d0 = HGF2DPosition(0.1, 0.1);
    VerticalPoint0d1 = HGF2DPosition(0.1, 1.1);
    VerticalPoint0d5 = HGF2DPosition(0.1, 5.1);
    VerticalPoint1d0 = HGF2DPosition(0.1, 10.1);

    Vertical3Point0d0 = HGF2DPosition(0.1, 0.1);
    Vertical3Point0d1 = HGF2DPosition(0.1, 0.1 + (0.1 * MYEPSILON));
    Vertical3Point0d5 = HGF2DPosition(0.1, 0.1 + (0.5 * MYEPSILON));
    Vertical3Point1d0 = HGF2DPosition(0.1, 0.1 + MYEPSILON);

    VerticalMidPoint1 = HGF2DPosition(0.1, 5.1);
    VerticalMidPoint3 = HGF2DPosition(0.1, 0.1 + (MYEPSILON/2));

    VerticalClosePoint1A = HGF2DPosition(0.0, 0.0);
    VerticalClosePoint1B = HGF2DPosition(0.0, 5.0);
    VerticalClosePoint1C = HGF2DPosition(0.2, 7.0);
    VerticalClosePoint1D = HGF2DPosition(1000000.0, 10.0);
    VerticalCloseMidPoint1 = HGF2DPosition(0.0, 5.1);

    VerticalClosePoint3A = HGF2DPosition(0.0, 0.0);
    VerticalClosePoint3B = HGF2DPosition(0.0, 0.1 + (2*MYEPSILON/3));
    VerticalClosePoint3C = HGF2DPosition(0.2, 0.1 + (MYEPSILON/3));
    VerticalClosePoint3D = HGF2DPosition(1000000.0, 0.1 + (MYEPSILON/2));
    VerticalCloseMidPoint3 = HGF2DPosition(0.0, 0.1 + (MYEPSILON/2));

    // HORIZONTAL SEGMENT
    HorizontalSegment1 = HGF2DLiteSegment(HGF2DPosition(0.1, 0.1), HGF2DPosition(10.1, 0.1));
    HorizontalSegment2 = HGF2DLiteSegment(HGF2DPosition(10.1, 0.1), HGF2DPosition(0.1, 0.1));
    HorizontalSegment3 = HGF2DLiteSegment(HGF2DPosition(0.1, 0.1), HGF2DPosition(0.1 + MYEPSILON, 0.1));
    HorizontalSegment5 = HGF2DLiteSegment(HGF2DPosition(0.1, 0.1), HGF2DPosition(10.1 + MYEPSILON, 0.1));

    HorizontalPoint0d0 = HGF2DPosition(0.1, 0.1);
    HorizontalPoint0d1 = HGF2DPosition(1.1, 0.1);
    HorizontalPoint0d5 = HGF2DPosition(5.1, 0.1);
    HorizontalPoint1d0 = HGF2DPosition(10.1, 0.1);

    Horizontal3Point0d0 = HGF2DPosition(0.1, 0.1);
    Horizontal3Point0d1 = HGF2DPosition(0.1 + (0.1 * MYEPSILON), 0.1);
    Horizontal3Point0d5 = HGF2DPosition(0.1 + (0.5 * MYEPSILON), 0.1);
    Horizontal3Point1d0 = HGF2DPosition(0.1 + MYEPSILON, 0.1);

    HorizontalMidPoint1 = HGF2DPosition(5.1, 0.1);
    HorizontalMidPoint3 = HGF2DPosition(0.1 + (MYEPSILON/2), 0.1);

    HorizontalClosePoint1A = HGF2DPosition(0.0, 0.0);
    HorizontalClosePoint1B = HGF2DPosition(5.0, 0.0);
    HorizontalClosePoint1C = HGF2DPosition(7.0, 0.2);
    HorizontalClosePoint1D = HGF2DPosition(10.0, 1000000.0);
    HorizontalCloseMidPoint1 = HGF2DPosition(5.1, 0.0);

    HorizontalClosePoint3A = HGF2DPosition(0.0, 0.0);
    HorizontalClosePoint3B = HGF2DPosition(0.1 + (2*MYEPSILON/3), 0.0);
    HorizontalClosePoint3C = HGF2DPosition(0.1 + (MYEPSILON/3), 0.2);
    HorizontalClosePoint3D = HGF2DPosition(0.1 + (MYEPSILON/2), 1000000.0);
    HorizontalCloseMidPoint3 = HGF2DPosition(0.1 + (MYEPSILON/2), 0.0);
  
    // MISC SEGMENTS
    MiscSegment1 = HGF2DLiteSegment(HGF2DPosition(0.1, 0.1), HGF2DPosition(10.1, 10.1));
    MiscSegment2 = HGF2DLiteSegment(HGF2DPosition(10.1, 10.1), HGF2DPosition(0.1, 0.1));
    MiscSegment6 = HGF2DLiteSegment(HGF2DPosition(0.1, 0.1), HGF2DPosition(-9.9, 10.1));
    MiscSegment7 = HGF2DLiteSegment(HGF2DPosition(0.2, 0.0), HGF2DPosition(-9.8, 10.0));
    DisjointSegment1 = HGF2DLiteSegment(HGF2DPosition(-0.1, -0.1), HGF2DPosition(-10.1, -10.24));
    ContiguousExtentSegment1 = HGF2DLiteSegment(HGF2DPosition(10.1, 0.1), HGF2DPosition(20.1, 10.1));
    FlirtingExtentSegment1 = HGF2DLiteSegment(HGF2DPosition(10.1, 0.1), HGF2DPosition(20.1, -10.1));
    FlirtingExtentLinkedSegment1 = HGF2DLiteSegment(HGF2DPosition(10.1, 10.1), HGF2DPosition(20.1, 0.1));
    ParallelSegment1 = HGF2DLiteSegment(HGF2DPosition(0.2, 0.1), HGF2DPosition(10.2, 10.1));
    LinkedParallelSegment1 = HGF2DLiteSegment(HGF2DPosition(10.1, 10.1), HGF2DPosition(20.1, 20.1));
    NearParallelSegment1 = HGF2DLiteSegment(HGF2DPosition(0.2, 0.1), HGF2DPosition(10.2, 10.1+MYEPSILON / 100.0));
    CloseNearParallelSegment1 = HGF2DLiteSegment(HGF2DPosition(0.1+MYEPSILON, 0.1), HGF2DPosition(10.1+MYEPSILON, 10.1+MYEPSILON / 100.0));
    ConnectedSegment1 = HGF2DLiteSegment(HGF2DPosition(0.2, 0.0), HGF2DPosition(0.0, 0.2));
    ConnectingSegment1 = HGF2DLiteSegment(HGF2DPosition(10.2, 0.0), HGF2DPosition(2.0, 2.0));
    ConnectedSegment1A = HGF2DLiteSegment(HGF2DPosition(20.2, 0.0), HGF2DPosition(0.0, 20.2));
    ConnectingSegment1A = HGF2DLiteSegment(HGF2DPosition(2.0, 2.0), HGF2DPosition(10.2, 0.0));
    LinkedSegment1 = HGF2DLiteSegment(HGF2DPosition(0.1, 0.1), HGF2DPosition(10.0, 3.2));
    LinkedSegment1A = HGF2DLiteSegment(HGF2DPosition(10.1, 10.1), HGF2DPosition(10.0, 3.2));

    Misc3Point0d0 = HGF2DPosition(0.1, 0.1);
    MiscMidPoint1 = HGF2DPosition(5.1, 5.1);
    MiscClosePoint3A = HGF2DPosition(0.0, 0.0);
    MiscClosePoint3B = HGF2DPosition(0.0, 0.1 + (2*MYEPSILON/3));
    MiscClosePoint3C = HGF2DPosition(0.2, 0.1 + (MYEPSILON/3));
    MiscClosePoint3D = HGF2DPosition(1000000.0, 0.1 + (MYEPSILON/2));
    MiscCloseMidPoint3 = HGF2DPosition(0.0, 0.1 + (MYEPSILON/2));

    VeryFarPoint = HGF2DPosition(21E123, 1E123);
    VeryFarAlignedPoint = HGF2DPosition(21E123, 21E123);
    VeryFarAlignedNegativePoint = HGF2DPosition(-21E123, -21E123);
    VeryFarNegativePoint = HGF2DPosition(-1E123, -21E123);
    MidPoint = HGF2DPosition(5.1, 5.1);

    MiscMidPoint6 = HGF2DPosition(-4.9, 5.1);

    // LARGE SEGMENTS
    LargeSegment1 = HGF2DLiteSegment(HGF2DPosition(-1E123, -21E123), HGF2DPosition(9E123, 19E123));
    LargeSegment2 = HGF2DLiteSegment(HGF2DPosition(9E123, 19E123), HGF2DPosition(-1E123, -21E123));
    ParallelLargeSegment1 = HGF2DLiteSegment(HGF2DPosition(-1.000000001E123, -21E123), HGF2DPosition(9.000000001E123, 19E123));

    LargePoint0d0 = HGF2DPosition(-1E123, -21E123);
    LargePoint0d1 = HGF2DPosition(0.0, -17E123);
    LargePoint0d5 = HGF2DPosition(4E123, -1E123);
    LargePoint1d0 = HGF2DPosition(9E123, 19E123);

    LargeMidPoint1 = HGF2DPosition(4E123, -1E123);

    LargeClosePoint1A = HGF2DPosition(0.0, 0.0);
    LargeClosePoint1B = HGF2DPosition(1E124, 1E124);
    LargeClosePoint1C = HGF2DPosition(-1E124, -1E124);
    LargeClosePoint1D = HGF2DPosition(10.0, 1E120);
    LargeCloseMidPoint1 = HGF2DPosition(4.0001E123, -1.0002E123);

    // POSITIVE SEGMENTS
    PositiveSegment1 = HGF2DLiteSegment(HGF2DPosition(1E123, 21E123), HGF2DPosition(11E123, 41E123));
    PositiveSegment2 = HGF2DLiteSegment(HGF2DPosition(11E123, 41E123), HGF2DPosition(1E123, 21E123));
    ParallelPositiveSegment1 = HGF2DLiteSegment(HGF2DPosition(1.000001E123, 21E123), HGF2DPosition(11.000001E123, 41E123));

    PositivePoint0d0 = HGF2DPosition(1E123, 21E123);
    PositivePoint0d1 = HGF2DPosition(2E123, 23E123);
    PositivePoint0d5 = HGF2DPosition(6E123, 31E123);
    PositivePoint1d0 = HGF2DPosition(11E123, 41E123);

    PositiveMidPoint1 = HGF2DPosition(6E123, 31E123);

    PositiveClosePoint1A = HGF2DPosition(0.0, 0.0);
    PositiveClosePoint1B = HGF2DPosition(1E124, 1E124);
    PositiveClosePoint1C = HGF2DPosition(-1E124, -1E124);
    PositiveClosePoint1D = HGF2DPosition(10.0, 1E120);
    PositiveCloseMidPoint1 = HGF2DPosition(6.0001E123, 31.0002E123);

    // NEGATIVE SEGMENTS
    NegativeSegment1 = HGF2DLiteSegment(HGF2DPosition(-1E123, -21E123), HGF2DPosition(-11E123, -41E123));
    NegativeSegment2 = HGF2DLiteSegment(HGF2DPosition(-11E123, -41E123), HGF2DPosition(-1E123, -21E123));
    ParallelNegativeSegment1 =HGF2DLiteSegment(HGF2DPosition(-1.000001E123, -21E123), HGF2DPosition(-11.000001E123, -41E123));

    NegativePoint0d0 = HGF2DPosition(-1E123, -21E123);
    NegativePoint0d1 = HGF2DPosition(-2E123, -23E123);
    NegativePoint0d5 = HGF2DPosition(-6E123, -31E123);
    NegativePoint1d0 = HGF2DPosition(-11E123, -41E123);

    NegativeMidPoint1 = HGF2DPosition(-6E123, -31E123);

    NegativeClosePoint1A = HGF2DPosition(0.0, 0.0);
    NegativeClosePoint1B = HGF2DPosition(1E124, 1E124);
    NegativeClosePoint1C = HGF2DPosition(-1E124, -1E124);
    NegativeClosePoint1D = HGF2DPosition(10.0, 1E120);
    NegativeCloseMidPoint1 = HGF2DPosition(-6.0001E123, -31.0002E123);

    }

// THE PRESENT TEST CANNOT MAKE TESTS WITH NON-LINEAR TRANSFORMATION MODELS

//==================================================================================
// Segment Construction tests
// HGF2DLiteSegment();
// HGF2DLiteSegment(const HGF2DPosition&, const HGF2DPosition&);
// HGF2DLiteSegment(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
// HGF2DLiteSegment(const HGF2DLiteSegment&    pi_rObject);
//==================================================================================
#pragma optimize( "", off )                             // Disable optimization to solve a potential compiler bug?
TEST_F (HGF2DLiteSegmentTester, ConstructionTest)
    {

    // Default Constructor
    HGF2DLiteSegment    Segment1;

    // Preparation of the two points
    HGF2DPosition   FirstSegmentPoint(10.0, 10.2);
    HGF2DPosition   SecondSegmentPoint(-10000.0, 100.3);

    HGF2DLiteSegment    Segment2(FirstSegmentPoint, SecondSegmentPoint);
    ASSERT_NEAR(10.00000, Segment2.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.20000, Segment2.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(-10000.0, Segment2.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(100.3000, Segment2.GetEndPoint().GetY(), MYEPSILON);

    HGF2DLiteSegment    Segment3(FirstSegmentPoint, SecondSegmentPoint);      
    ASSERT_NEAR(10.00000, Segment3.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.20000, Segment3.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(-10000.0, Segment3.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(100.3000, Segment3.GetEndPoint().GetY(), MYEPSILON);

    //Copy Constructor
    HGF2DLiteSegment    Segment4(Segment3);
    ASSERT_NEAR(10.00000, Segment4.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.20000, Segment4.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(-10000.0, Segment4.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(100.3000, Segment4.GetEndPoint().GetY(), MYEPSILON);

    }
#pragma optimize( "", on )
//==================================================================================
// operator= test
// operator=(const HGF2DLiteSegment& pi_rObj);
//==================================================================================
TEST_F (HGF2DLiteSegmentTester, OperatorTest)
    {

    // Test with different coord sys
    HGF2DPosition   FirstSegmentPoint(10.0, 10.2);
    HGF2DPosition   SecondSegmentPoint(-10000.0, 100.3);
    HGF2DLiteSegment    Segment1(FirstSegmentPoint, SecondSegmentPoint);
    HGF2DLiteSegment    Segment2(SecondSegmentPoint, FirstSegmentPoint);

    Segment2 = Segment1;
    ASSERT_DOUBLE_EQ(10.00000, Segment2.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.20000, Segment2.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-10000.0, Segment2.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(100.3000, Segment2.GetEndPoint().GetY());

    }

//==================================================================================
// operator== test
// operator==(const HGF2DLiteSegment& pi_rObj);
//==================================================================================
TEST_F (HGF2DLiteSegmentTester, OperatorEqualityTest)
    {

    ASSERT_TRUE(VerticalSegment1 == VerticalSegment1);
    ASSERT_FALSE(VerticalSegment1 == HorizontalSegment1);
    ASSERT_TRUE(VerticalSegment1 == HGF2DLiteSegment(VerticalSegment1));

    }

//==================================================================================
// Segment coordinate setting test
// void                     SetStartPoint(const HGF2DPosition& pi_rNewStartPoint);
// void                     SetEndPoint(const HGF2DPosition& pi_rNewEndPoint);
// const HGF2DPosition&     GetStartPoint() const;
// const HGF2DPosition&     GetEndPoint() const;
//==================================================================================
TEST_F (HGF2DLiteSegmentTester, CoordinateTest)
    {

    // Test set to same coordinates
    HGF2DPosition   FirstSegmentPoint(10.0, 10.2);
    HGF2DPosition   SecondSegmentPoint(-10000.0, 100.3);
    HGF2DLiteSegment    Segment1(FirstSegmentPoint, SecondSegmentPoint);

    Segment1.SetStartPoint(SecondSegmentPoint);
    ASSERT_DOUBLE_EQ(-10000.0, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(100.3000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-10000.0, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(100.3000, Segment1.GetEndPoint().GetY());

    Segment1.SetEndPoint(FirstSegmentPoint);
    ASSERT_DOUBLE_EQ(-10000.0, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(100.3000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.00000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.20000, Segment1.GetEndPoint().GetY());

    Segment1.SetStartPoint(HGF2DPosition(10.0, 10.0));
    ASSERT_DOUBLE_EQ(10.0, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, Segment1.GetStartPoint().GetY());

    Segment1.SetEndPoint(HGF2DPosition(10.0, 10.0));
    ASSERT_DOUBLE_EQ(10.0, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, Segment1.GetEndPoint().GetY());

    // Test with vertical segment
    HGF2DLiteSegment    Segment2(VerticalSegment1);

    Segment2.SetStartPoint(HGF2DPosition(23.5, 23.5));
    ASSERT_DOUBLE_EQ(23.5, Segment2.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(23.5, Segment2.GetStartPoint().GetY());

    Segment2.SetEndPoint(HGF2DPosition(23.5, 23.5));
    ASSERT_DOUBLE_EQ(23.5, Segment2.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(23.5, Segment2.GetEndPoint().GetY());

    Segment2.SetStartPoint(HGF2DPosition(23.6, 23.8));
    ASSERT_DOUBLE_EQ(23.6, Segment2.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(23.8, Segment2.GetStartPoint().GetY());

    Segment2.SetEndPoint(HGF2DPosition(-3.6, -23.8));
    ASSERT_DOUBLE_EQ(-3.60, Segment2.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-23.8, Segment2.GetEndPoint().GetY()); 

    // Test with inverted vertical segment
    HGF2DLiteSegment    Segment3(VerticalSegment2);

    Segment3.SetStartPoint(HGF2DPosition(23.5, 23.5));
    ASSERT_DOUBLE_EQ(23.5, Segment3.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(23.5, Segment3.GetStartPoint().GetY());

    Segment3.SetEndPoint(HGF2DPosition(23.5, 23.5));
    ASSERT_DOUBLE_EQ(23.5, Segment3.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(23.5, Segment3.GetEndPoint().GetY());

    Segment3.SetStartPoint(HGF2DPosition(23.6, 23.8));
    ASSERT_DOUBLE_EQ(23.6, Segment3.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(23.8, Segment3.GetStartPoint().GetY());

    Segment3.SetEndPoint(HGF2DPosition(-3.6, -23.8));
    ASSERT_DOUBLE_EQ(-3.60, Segment3.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-23.8, Segment3.GetEndPoint().GetY()); 

    }

//==================================================================================
// double   GetTolerance() const;
// void     SetTolerance(double pi_Tolerance);
//==================================================================================
TEST_F (HGF2DLiteSegmentTester, ToleranceTest)
    {

    VerticalSegment1.SetTolerance(1.0);
    ASSERT_DOUBLE_EQ(1.0, VerticalSegment1.GetTolerance());

    VerticalSegment1.SetTolerance(MYEPSILON);
    ASSERT_DOUBLE_EQ(MYEPSILON, VerticalSegment1.GetTolerance());

    }

//==================================================================================
// Segment intersection test
// IntersectSegment(const HGF2DLiteSegment& pi_rSegment,HGF2DPosition* po_pPoint)const;
// IntersectSegmentExtremityIncluded(const HGF2DLiteSegment& pi_rSegment, HGF2DPosition* po_pPoint,
//                                   bool* po_pCrossesAtExtremity = NULL) const;
//==================================================================================
TEST_F (HGF2DLiteSegmentTester, IntersectSegmentTest)
    {

    HGF2DPosition   DumPoint;

    // Test with extent disjoint segments
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, MiscSegment1.IntersectSegment(DisjointSegment1, &DumPoint));
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, MiscSegment1.IntersectSegmentExtremityIncluded(DisjointSegment1, &DumPoint));

    // Test with disjoint but touching by a side segments
    // HERE THERE IS A DIFFERENCE BETWEEN HVE2DSegment and HGF2DLiteSegment. If outer extent do not overlap
    // Then NO_CROSS is returned instead of PARALLEL
    ASSERT_NE(HGF2DLiteSegment::PARALLEL, MiscSegment1.IntersectSegment(ContiguousExtentSegment1, &DumPoint));
    ASSERT_NE(HGF2DLiteSegment::PARALLEL, MiscSegment1.IntersectSegmentExtremityIncluded(ContiguousExtentSegment1, &DumPoint));

    // Test with disjoint but touching by a tip segments but not linked
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, MiscSegment1.IntersectSegment(FlirtingExtentSegment1, &DumPoint));
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, MiscSegment1.IntersectSegmentExtremityIncluded(FlirtingExtentSegment1, &DumPoint));

    // Test with disjoint but touching by a tip segments linked
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, MiscSegment1.IntersectSegment(FlirtingExtentLinkedSegment1, &DumPoint));
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, MiscSegment1.IntersectSegmentExtremityIncluded(FlirtingExtentLinkedSegment1, &DumPoint));

    // Test with vertical segment
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, VerticalSegment1.IntersectSegment(MiscSegment1, &DumPoint));
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, VerticalSegment1.IntersectSegmentExtremityIncluded(MiscSegment1, &DumPoint));

    // Test with inverted vertical segment
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, VerticalSegment2.IntersectSegment(MiscSegment1, &DumPoint));
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, VerticalSegment2.IntersectSegmentExtremityIncluded(MiscSegment1, &DumPoint));

    // Test with close vertical segments
    // HERE THERE IS A DIFFERENCE BETWEEN HVE2DSegment and HGF2DLiteSegment. If outer extent do not overlap
    // Then NO_CROSS is returned instead of PARALLEL
    ASSERT_NE(HGF2DLiteSegment::PARALLEL, VerticalSegment1.IntersectSegment(VerticalSegment4, &DumPoint));
    ASSERT_NE(HGF2DLiteSegment::PARALLEL, VerticalSegment1.IntersectSegmentExtremityIncluded(VerticalSegment4, &DumPoint));

    // Test with horizontal segment
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, HorizontalSegment1.IntersectSegment(MiscSegment1, &DumPoint));
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, HorizontalSegment1.IntersectSegmentExtremityIncluded(MiscSegment1, &DumPoint));

    // Test with inverted horizontal segment
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, HorizontalSegment2.IntersectSegment(MiscSegment1, &DumPoint));
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, HorizontalSegment2.IntersectSegmentExtremityIncluded(MiscSegment1, &DumPoint));

    // Test with parallel segments
    ASSERT_EQ(HGF2DLiteSegment::PARALLEL, MiscSegment1.IntersectSegment(ParallelSegment1, &DumPoint));
    ASSERT_EQ(HGF2DLiteSegment::PARALLEL, MiscSegment1.IntersectSegmentExtremityIncluded(ParallelSegment1, &DumPoint));

    // Test with near parallel segments
    ASSERT_NE(HGF2DLiteSegment::PARALLEL, MiscSegment1.IntersectSegment(NearParallelSegment1, &DumPoint));
    ASSERT_NE(HGF2DLiteSegment::PARALLEL, MiscSegment1.IntersectSegmentExtremityIncluded(NearParallelSegment1, &DumPoint));

    // Tests with close near parallel segments
    ASSERT_NE(HGF2DLiteSegment::PARALLEL, MiscSegment1.IntersectSegment(CloseNearParallelSegment1, &DumPoint));
    ASSERT_NE(HGF2DLiteSegment::PARALLEL, MiscSegment1.IntersectSegmentExtremityIncluded(CloseNearParallelSegment1, &DumPoint));

    // Tests with connected segments
    // At start point...
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, MiscSegment1.IntersectSegment(ConnectedSegment1, &DumPoint));
    ASSERT_EQ(HGF2DLiteSegment::CROSS_FOUND, MiscSegment1.IntersectSegmentExtremityIncluded(ConnectedSegment1, &DumPoint));

    // At end point ...
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, MiscSegment1.IntersectSegment(ConnectedSegment1A, &DumPoint));
    ASSERT_EQ(HGF2DLiteSegment::CROSS_FOUND, MiscSegment1.IntersectSegmentExtremityIncluded(ConnectedSegment1A, &DumPoint));

    // Tests with linked segments
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, MiscSegment1.IntersectSegment(LinkedSegment1, &DumPoint));
    ASSERT_EQ(HGF2DLiteSegment::CROSS_FOUND, MiscSegment1.IntersectSegmentExtremityIncluded(LinkedSegment1, &DumPoint));

    // Tests with vertical EPSILON sized segment
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, VerticalSegment3.IntersectSegment(MiscSegment1, &DumPoint));
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, VerticalSegment3.IntersectSegmentExtremityIncluded(MiscSegment1, &DumPoint));

    // Tests with horizontal EPSILON sized segment
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, HorizontalSegment3.IntersectSegment(MiscSegment1, &DumPoint));
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, HorizontalSegment3.IntersectSegmentExtremityIncluded(MiscSegment1, &DumPoint));

    // Test with very large segment
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, LargeSegment1.IntersectSegment(MiscSegment1, &DumPoint));
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, LargeSegment1.IntersectSegmentExtremityIncluded(MiscSegment1, &DumPoint));

    // Test with segments way into positive regions
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, PositiveSegment1.IntersectSegment(MiscSegment1, &DumPoint));
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, PositiveSegment1.IntersectSegmentExtremityIncluded(MiscSegment1, &DumPoint));

    // Test with segments way into negative regions
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, NegativeSegment1.IntersectSegment(MiscSegment1, &DumPoint));
    ASSERT_EQ(HGF2DLiteSegment::NO_CROSS, NegativeSegment1.IntersectSegmentExtremityIncluded(MiscSegment1, &DumPoint));

    }

//==================================================================================
// Contiguousness Test
//   ObtainContiguousnessPoints(const HGF2DVector& pi_rVector,
//                              HGF2DPositionCollection* po_pContiguousnessPoints) const;
//   ObtainContiguousnessPointsAt(const HGF2DVector& pi_rVector,
//                                const HGF2DPosition& pi_rPoint,
//                                HGF2DPosition* pi_pFirstContiguousnessPoint,
//                                HGF2DPosition* pi_pSecondContiguousnessPoint) const;
//    HDLL virtual bool      AreContiguous(const HGF2DVector& pi_rVector) const;
//    HDLL virtual bool      AreContiguousAt(const HGF2DVector& pi_rVector,
//                                            const HGF2DPosition& pi_rPoint) const;
//==================================================================================
TEST_F (HGF2DLiteSegmentTester, ContiguousnessTest)
    {

    HGF2DPositionCollection     DumPoints;
    HGF2DPosition   FirstDumPoint;
    HGF2DPosition   SecondDumPoint;

    // Test with vertical segments
    ASSERT_TRUE(VerticalSegment1.AreContiguous(VerticalSegment2));
    ASSERT_EQ(2, VerticalSegment1.ObtainContiguousnessPoints(VerticalSegment2, &DumPoints));
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(0.10, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.1, DumPoints[1].GetY());

    DumPoints.clear();

    // Test with horizontal segments
    ASSERT_TRUE(HorizontalSegment1.AreContiguous(HorizontalSegment2));
    ASSERT_EQ(2, HorizontalSegment1.ObtainContiguousnessPoints(HorizontalSegment2, &DumPoints));
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.1, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(0.10, DumPoints[1].GetY());

    ASSERT_FALSE(HorizontalSegment1.AreContiguous(MiscSegment1));

    DumPoints.clear();

    // Test with positive slope segments
    ASSERT_TRUE(MiscSegment1.AreContiguous(MiscSegment2));
    ASSERT_EQ(2, MiscSegment1.ObtainContiguousnessPoints(MiscSegment2, &DumPoints));
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.1, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.1, DumPoints[1].GetY());

    ASSERT_FALSE( MiscSegment1.AreContiguous(LargeSegment1));

    DumPoints.clear();

    // Tests with negative slope segments 
    ASSERT_TRUE(MiscSegment6.AreContiguous(MiscSegment7));
    ASSERT_EQ(2, MiscSegment6.ObtainContiguousnessPoints(MiscSegment7, &DumPoints));
    ASSERT_DOUBLE_EQ(0.10000000000000000, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(-9.8000000000000007, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.0000000000000000, DumPoints[1].GetY());

    ASSERT_FALSE( MiscSegment6.AreContiguous(MiscSegment1));

    }

//==================================================================================
// LinksTo(const HGF2DLiteSegment& pi_rSegment) const;
//==================================================================================
TEST_F (HGF2DLiteSegmentTester, LinksToTest)
    {

    // Test with himself
    ASSERT_TRUE(MiscSegment1.LinksTo(MiscSegment1));

    // Test with extent disjoint segments
    ASSERT_FALSE(MiscSegment1.LinksTo(DisjointSegment1));

    // Test with disjoint but touching by a side segments
    ASSERT_FALSE(MiscSegment1.LinksTo(ContiguousExtentSegment1));

    // Test with disjoint but touching by a tip segments but not linked
    ASSERT_FALSE(MiscSegment1.LinksTo(FlirtingExtentSegment1));

    // Test with disjoint but touching by a tip segments linked
    ASSERT_TRUE(MiscSegment1.LinksTo(FlirtingExtentLinkedSegment1));

    // Test with vertical segment
    ASSERT_TRUE(VerticalSegment1.LinksTo(MiscSegment1));

    // Test with inverted vertical segment
    ASSERT_TRUE(VerticalSegment2.LinksTo(MiscSegment1));

    // Tests with vertical EPSILON sized segment
    ASSERT_TRUE(VerticalSegment3.LinksTo(MiscSegment1));

    // Tests with horizontal EPSILON sized segment
    ASSERT_TRUE(HorizontalSegment3.LinksTo(MiscSegment1));

    // Test with very large segment
    ASSERT_FALSE(LargeSegment1.LinksTo(MiscSegment1));

    // Test with segments way into positive regions
    ASSERT_FALSE(PositiveSegment1.LinksTo(MiscSegment1));

    // Test with segments way into negative regions
    ASSERT_FALSE(NegativeSegment1.LinksTo(MiscSegment1));

    }

//==================================================================================
// Interaction info with other segment
// Crosses(const HGF2DVector& pi_rVector) const;
// AreAdjacent(const HGF2DVector& pi_rVector) const;
// AreParallel(const HGF2DLiteSegment& pi_rSegment, double pi_SlopeTolerance = 0.0) const;
// IsPointOn(const HGF2DPosition& pi_rTestPoint) const;
//==================================================================================
TEST_F (HGF2DLiteSegmentTester, InteractionTest)
    {

    // Tests with a vertical segment
    ASSERT_FALSE(VerticalSegment1.Crosses(VerticalSegment2));
    ASSERT_TRUE(VerticalSegment1.AreAdjacent(VerticalSegment2));
    ASSERT_TRUE(VerticalSegment1.AreParallel(VerticalSegment2));

    ASSERT_FALSE(VerticalSegment1.Crosses(MiscSegment1));
    ASSERT_FALSE(VerticalSegment1.AreAdjacent(MiscSegment1));
    ASSERT_FALSE(VerticalSegment1.AreParallel(MiscSegment1));

    ASSERT_FALSE(VerticalSegment1.Crosses(VerticalSegment3));
    ASSERT_TRUE(VerticalSegment1.AreAdjacent(VerticalSegment3));
    ASSERT_TRUE(VerticalSegment1.AreParallel(VerticalSegment3));

    ASSERT_FALSE(VerticalSegment1.Crosses(LargeSegment1));
    ASSERT_FALSE(VerticalSegment1.AreAdjacent(LargeSegment1));
    ASSERT_FALSE(VerticalSegment1.AreParallel(LargeSegment1));
   
    ASSERT_FALSE(VerticalSegment1.IsPointOn(HGF2DPosition(10, 10)));
    ASSERT_FALSE(VerticalSegment1.IsPointOn(HGF2DPosition(0.1, 0.1-1.1*MYEPSILON)));
    ASSERT_TRUE(VerticalSegment1.IsPointOn(VerticalSegment1.GetStartPoint()));
    ASSERT_TRUE(VerticalSegment1.IsPointOn(VerticalSegment1.GetEndPoint()));
    ASSERT_TRUE(VerticalSegment1.IsPointOn(VerticalMidPoint1));

    // Tests with an inverted vertical segment
    ASSERT_FALSE(VerticalSegment2.Crosses(VerticalSegment1));
    ASSERT_TRUE(VerticalSegment2.AreAdjacent(VerticalSegment1));
    ASSERT_TRUE(VerticalSegment2.AreParallel(VerticalSegment1));
   
    ASSERT_FALSE(VerticalSegment2.Crosses(MiscSegment1));
    ASSERT_FALSE(VerticalSegment2.AreAdjacent(MiscSegment1));
    ASSERT_FALSE(VerticalSegment2.AreParallel(MiscSegment1));

    ASSERT_FALSE(VerticalSegment2.Crosses(VerticalSegment3));
    ASSERT_TRUE(VerticalSegment2.AreAdjacent(VerticalSegment3));
    ASSERT_TRUE(VerticalSegment2.AreParallel(VerticalSegment3));

    ASSERT_FALSE(VerticalSegment2.Crosses(LargeSegment1));
    ASSERT_FALSE(VerticalSegment2.AreAdjacent(LargeSegment1));
    ASSERT_FALSE(VerticalSegment2.AreParallel(LargeSegment1));

    ASSERT_FALSE(VerticalSegment2.IsPointOn(HGF2DPosition(10, 10)));
    ASSERT_FALSE(VerticalSegment2.IsPointOn(HGF2DPosition(0.1, 0.1-1.1*MYEPSILON)));
    ASSERT_TRUE(VerticalSegment2.IsPointOn(VerticalSegment2.GetStartPoint()));
    ASSERT_TRUE(VerticalSegment2.IsPointOn(VerticalSegment2.GetEndPoint()));
    ASSERT_TRUE(VerticalSegment2.IsPointOn(VerticalMidPoint1));

    // Tests with a horizontal segment
    ASSERT_FALSE(HorizontalSegment1.Crosses(HorizontalSegment2));
    ASSERT_TRUE(HorizontalSegment1.AreAdjacent(HorizontalSegment2));
    ASSERT_TRUE(HorizontalSegment1.AreParallel(HorizontalSegment2));

    ASSERT_FALSE(HorizontalSegment1.Crosses(MiscSegment1));
    ASSERT_FALSE(HorizontalSegment1.AreAdjacent(MiscSegment1));
    ASSERT_FALSE(HorizontalSegment1.AreParallel(MiscSegment1));

    ASSERT_FALSE(HorizontalSegment1.Crosses(HorizontalSegment3));
    ASSERT_TRUE(HorizontalSegment1.AreAdjacent(HorizontalSegment3));
    ASSERT_TRUE(HorizontalSegment1.AreParallel(HorizontalSegment3));

    ASSERT_FALSE(HorizontalSegment1.Crosses(LargeSegment1));
    ASSERT_FALSE(HorizontalSegment1.AreAdjacent(LargeSegment1));
    ASSERT_FALSE(HorizontalSegment1.AreParallel(LargeSegment1));

    ASSERT_FALSE(HorizontalSegment1.IsPointOn(HGF2DPosition(10, 10)));
    ASSERT_FALSE(HorizontalSegment1.IsPointOn(HGF2DPosition(0.1-1.1*MYEPSILON, 0.1)));
    ASSERT_TRUE(HorizontalSegment1.IsPointOn(HorizontalSegment1.GetStartPoint()));
    ASSERT_TRUE(HorizontalSegment1.IsPointOn(HorizontalSegment1.GetEndPoint()));
    ASSERT_TRUE(HorizontalSegment1.IsPointOn(HorizontalMidPoint1));

    // Tests with an inverted horizontal segment
    ASSERT_FALSE(HorizontalSegment2.Crosses(HorizontalSegment1));
    ASSERT_TRUE(HorizontalSegment2.AreAdjacent(HorizontalSegment1));
    ASSERT_TRUE(HorizontalSegment2.AreParallel(HorizontalSegment1));

    ASSERT_FALSE(HorizontalSegment2.Crosses(MiscSegment1));
    ASSERT_FALSE(HorizontalSegment2.AreAdjacent(MiscSegment1));
    ASSERT_FALSE(HorizontalSegment2.AreParallel(MiscSegment1));

    ASSERT_FALSE(HorizontalSegment2.Crosses(HorizontalSegment3));
    ASSERT_TRUE(HorizontalSegment2.AreAdjacent(HorizontalSegment3));
    ASSERT_TRUE(HorizontalSegment2.AreParallel(HorizontalSegment3));

    ASSERT_FALSE(HorizontalSegment2.Crosses(LargeSegment1));
    ASSERT_FALSE(HorizontalSegment2.AreAdjacent(LargeSegment1));
    ASSERT_FALSE(HorizontalSegment2.AreParallel(LargeSegment1));

    ASSERT_FALSE(HorizontalSegment2.IsPointOn(HGF2DPosition(10, 10)));
    ASSERT_FALSE(HorizontalSegment2.IsPointOn(HGF2DPosition(0.1-1.1*MYEPSILON, 0.1)));
    ASSERT_TRUE(HorizontalSegment2.IsPointOn(HorizontalSegment2.GetStartPoint()));
    ASSERT_TRUE(HorizontalSegment2.IsPointOn(HorizontalSegment2.GetEndPoint()));
    ASSERT_TRUE(HorizontalSegment2.IsPointOn(HorizontalMidPoint1));

    // Tests with a positive slope segment
    ASSERT_FALSE(MiscSegment1.Crosses(MiscSegment2));
    ASSERT_TRUE(MiscSegment1.AreAdjacent(MiscSegment2));
    ASSERT_TRUE(MiscSegment1.AreParallel(MiscSegment2));

    ASSERT_FALSE(MiscSegment1.Crosses(HorizontalSegment1));
    ASSERT_FALSE(MiscSegment1.AreAdjacent(HorizontalSegment1));
    ASSERT_FALSE(MiscSegment1.AreParallel(HorizontalSegment1));

    ASSERT_FALSE(MiscSegment1.Crosses(LargeSegment1));
    ASSERT_FALSE(MiscSegment1.AreAdjacent(LargeSegment1));
    ASSERT_FALSE(MiscSegment1.AreParallel(LargeSegment1));

    ASSERT_FALSE(MiscSegment1.IsPointOn(HGF2DPosition(20, 20)));
    ASSERT_FALSE(MiscSegment1.IsPointOn(HGF2DPosition(0.1, 0.1-1.1*MYEPSILON)));
    ASSERT_TRUE(MiscSegment1.IsPointOn(MiscSegment1.GetStartPoint()));
    ASSERT_TRUE(MiscSegment1.IsPointOn(MiscSegment1.GetEndPoint()));
    ASSERT_TRUE(MiscSegment1.IsPointOn(MiscMidPoint1));

    ASSERT_FALSE(VerticalSegment3.Crosses(LargeSegment1));
    ASSERT_FALSE(VerticalSegment3.AreAdjacent(LargeSegment1));
    ASSERT_FALSE(VerticalSegment3.AreParallel(LargeSegment1));

    ASSERT_FALSE(VerticalSegment3.IsPointOn(HGF2DPosition(10, 10)));
    ASSERT_FALSE(VerticalSegment3.IsPointOn(HGF2DPosition(0.1, 0.1-1.1*MYEPSILON)));
    ASSERT_TRUE(VerticalSegment3.IsPointOn(VerticalSegment3.GetStartPoint()));
    ASSERT_TRUE(VerticalSegment3.IsPointOn(VerticalSegment3.GetEndPoint()));
    ASSERT_TRUE(VerticalSegment3.IsPointOn(VerticalMidPoint3));

    // Tests with an horizontal EPSILON SIZED segment
    ASSERT_FALSE(HorizontalSegment3.Crosses(HorizontalSegment1));
    ASSERT_TRUE(HorizontalSegment3.AreAdjacent(HorizontalSegment1));
    ASSERT_FALSE(HorizontalSegment3.Crosses(HorizontalSegment1));

    ASSERT_FALSE(HorizontalSegment3.Crosses(MiscSegment1));
    ASSERT_FALSE(HorizontalSegment3.AreAdjacent(MiscSegment1));
    ASSERT_FALSE(HorizontalSegment3.AreParallel(MiscSegment1));

    ASSERT_FALSE(HorizontalSegment3.Crosses(LargeSegment1));
    ASSERT_FALSE(HorizontalSegment3.AreAdjacent(LargeSegment1));
    ASSERT_FALSE(HorizontalSegment3.AreParallel(LargeSegment1));

    ASSERT_FALSE(HorizontalSegment3.IsPointOn(HGF2DPosition(10, 10)));
    ASSERT_FALSE(HorizontalSegment3.IsPointOn(HGF2DPosition(0.1, 0.1-1.1*MYEPSILON)));
    ASSERT_TRUE(HorizontalSegment3.IsPointOn(HorizontalSegment3.GetStartPoint()));
    ASSERT_TRUE(HorizontalSegment3.IsPointOn(HorizontalSegment3.GetEndPoint()));
    ASSERT_TRUE(HorizontalSegment3.IsPointOn(HorizontalMidPoint3));

    // Test with parallel segments
    ASSERT_TRUE(MiscSegment1.AreParallel(ParallelSegment1));

    // Test with near parallel segments
    ASSERT_FALSE(MiscSegment1.AreParallel(NearParallelSegment1));
    ASSERT_TRUE(MiscSegment1.AreParallel(NearParallelSegment1, 2*MYEPSILON));

    // Tests with close near parallel segments
    ASSERT_FALSE(MiscSegment1.AreParallel(CloseNearParallelSegment1));
    ASSERT_TRUE(MiscSegment1.AreParallel(CloseNearParallelSegment1, 2*MYEPSILON));

    }

//==================================================================================
// ArePointsOnDifferentSides(const HGF2DPosition& pi_rPoint1, const HGF2DPosition& pi_rPoint2) const;
//==================================================================================
TEST_F (HGF2DLiteSegmentTester, ArePointsOnDifferentSidesTest)
    {

    // Test with vertical segment
    ASSERT_TRUE(VerticalSegment1.ArePointsOnDifferentSides(HGF2DPosition(0.0, 0.0), HGF2DPosition(10.0, 0.0)));
    ASSERT_FALSE(VerticalSegment1.ArePointsOnDifferentSides(HGF2DPosition(0.0, 0.0), HGF2DPosition(-1.0, 0.0)));
    ASSERT_FALSE(VerticalSegment1.ArePointsOnDifferentSides(HGF2DPosition(0.1, 0.0), HGF2DPosition(1.0, 0.0)));

    // Test with inverted vertical segment
    ASSERT_TRUE(VerticalSegment2.ArePointsOnDifferentSides(HGF2DPosition(0.0, 0.0), HGF2DPosition(10.0, 0.0)));
    ASSERT_FALSE(VerticalSegment2.ArePointsOnDifferentSides(HGF2DPosition(0.0, 0.0), HGF2DPosition(-1.0, 0.0)));
    ASSERT_FALSE(VerticalSegment2.ArePointsOnDifferentSides(HGF2DPosition(0.1, 0.0), HGF2DPosition(1.0, 0.0)));

    // Tests with vertical EPSILON sized segment
    ASSERT_TRUE(VerticalSegment3.ArePointsOnDifferentSides(HGF2DPosition(0.0, 0.0), HGF2DPosition(10.0, 0.0)));
    ASSERT_FALSE(VerticalSegment3.ArePointsOnDifferentSides(HGF2DPosition(0.0, 0.0), HGF2DPosition(-1.0, 0.0)));
    ASSERT_FALSE(VerticalSegment3.ArePointsOnDifferentSides(HGF2DPosition(0.1, 0.0), HGF2DPosition(1.0, 0.0)));

    // Test with horizontal segment
    ASSERT_TRUE(HorizontalSegment1.ArePointsOnDifferentSides(HGF2DPosition(0.0, 0.0), HGF2DPosition(0.0, 10.0)));
    ASSERT_FALSE(HorizontalSegment1.ArePointsOnDifferentSides(HGF2DPosition(0.0, 0.0), HGF2DPosition(.0, -1.0)));
    ASSERT_FALSE(HorizontalSegment1.ArePointsOnDifferentSides(HGF2DPosition(0.0, 0.1), HGF2DPosition(0.0, 0.1)));

    // Test with horizontal vertical segment
    ASSERT_TRUE(HorizontalSegment2.ArePointsOnDifferentSides(HGF2DPosition(0.0, 0.0), HGF2DPosition(0.0, 10.0)));
    ASSERT_FALSE(HorizontalSegment2.ArePointsOnDifferentSides(HGF2DPosition(0.0, 0.0), HGF2DPosition(.0, -1.0)));
    ASSERT_FALSE(HorizontalSegment2.ArePointsOnDifferentSides(HGF2DPosition(0.0, 0.1), HGF2DPosition(0.0, 0.1)));

    #ifdef WIP_IPPTEST_BUG_19

    // Tests with horizontal EPSILON sized segment
    ASSERT_TRUE(HorizontalSegment3.ArePointsOnDifferentSides(HGF2DPosition(0.0, 0.0), HGF2DPosition(0.0, 10.0)));
    ASSERT_FALSE(HorizontalSegment3.ArePointsOnDifferentSides(HGF2DPosition(0.0, 0.0), HGF2DPosition(.0, -1.0)));
    ASSERT_FALSE(HorizontalSegment3.ArePointsOnDifferentSides(HGF2DPosition(0.0, 0.1), HGF2DPosition(0.0, 0.1)));

    // Test with very large segment
    //ASSERT_TRUE(LargeSegment1.ArePointsOnDifferentSides(HGF2DPosition(-10.0E120, -10.0E120), HGF2DPosition(10.0E120, 10.0E120)));

    #endif

    }

//==================================================================================
// IntersectsAtSplitPoint(const HGF2DLiteSegment& pi_rFirstSegment, const HGF2DLiteSegment& pi_rSecondSegment) const;
//==================================================================================
TEST_F (HGF2DLiteSegmentTester, IntersectsAtSplitPointTest)
    {
        
    HGF2DLiteSegment VerticalSegment1(HGF2DPosition(0.0, 0.0), HGF2DPosition(0.0, 100.0));
    HGF2DLiteSegment VerticalSegment2(HGF2DPosition(0.0, 100.0), HGF2DPosition(0.0, 0.0));

    HGF2DLiteSegment HorizontalSegment1(HGF2DPosition(0.0, 0.0), HGF2DPosition(100.0, 0.0));
    HGF2DLiteSegment HorizontalSegment2(HGF2DPosition(100.0, 0.0), HGF2DPosition(0.0, 0.0));

    HGF2DLiteSegment MiscSegment1(HGF2DPosition(0.0, 0.0), HGF2DPosition(100.0, 100.0));
    HGF2DLiteSegment MiscSegment2(HGF2DPosition(0.0, 0.0), HGF2DPosition(10.0, 100000.0));
    HGF2DLiteSegment MiscSegment3(HGF2DPosition(10.0, 1000000.0), HGF2DPosition(0.0, 0.0));
    HGF2DLiteSegment MiscSegment4(HGF2DPosition(-100.0, 100.0), HGF2DPosition(0.0, 0.0));

    // Intersects true with vertical segment
    ASSERT_TRUE(VerticalSegment1.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(10.0, 50.0), HGF2DPosition(0.0, 50.0)),
                                                        HGF2DLiteSegment(HGF2DPosition(0.0, 50.0), HGF2DPosition(-10.0, 50.0))));

    ASSERT_TRUE(VerticalSegment1.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(10.0, 50.0), HGF2DPosition(0.0, 50.0)),
                                                        HGF2DLiteSegment(HGF2DPosition(0.0, 50.0), HGF2DPosition(-10.0, 1000.0))));

    ASSERT_TRUE(VerticalSegment1.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(10.0, 1000.0), HGF2DPosition(0.0, 50.0)),
                                                        HGF2DLiteSegment(HGF2DPosition(0.0, 50.0), HGF2DPosition(-10.0, 1000.0))));

    ASSERT_TRUE(VerticalSegment2.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(10.0, 50.0), HGF2DPosition(0.0, 50.0)),
                                                        HGF2DLiteSegment(HGF2DPosition(0.0, 50.0), HGF2DPosition(-10.0, 50.0))));

    ASSERT_TRUE(VerticalSegment2.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(10.0, 50.0), HGF2DPosition(0.0, 50.0)),
                                                        HGF2DLiteSegment(HGF2DPosition(0.0, 50.0), HGF2DPosition(-10.0, 1000.0))));

    ASSERT_TRUE(VerticalSegment2.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(10.0, 1000.0), HGF2DPosition(0.0, 50.0)),
                                                        HGF2DLiteSegment(HGF2DPosition(0.0, 50.0), HGF2DPosition(-10.0, 1000.0))));


    // Intersect false with vertical segment
    ASSERT_FALSE(VerticalSegment1.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(10.0, 50.0), HGF2DPosition(0.0, 50.0)),
                                                         HGF2DLiteSegment(HGF2DPosition(0.0, 50.0), HGF2DPosition(10.0, 1000.0))));

    ASSERT_FALSE(VerticalSegment1.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(10.0, 50.0), HGF2DPosition(0.0, 50.0)),
                                                         HGF2DLiteSegment(HGF2DPosition(0.0, 50.0), HGF2DPosition(10.0, 50.0))));

    ASSERT_FALSE(VerticalSegment1.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(10.0, 1000.0), HGF2DPosition(0.0, 50.0)),
                                                         HGF2DLiteSegment(HGF2DPosition(0.0, 50.0), HGF2DPosition(1000.0, -1000.0))));

    ASSERT_FALSE(VerticalSegment2.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(10.0, 50.0), HGF2DPosition(0.0, 50.0)),
                                                         HGF2DLiteSegment(HGF2DPosition(0.0, 50.0), HGF2DPosition(10.0, 1000.0))));

    ASSERT_FALSE(VerticalSegment2.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(10.0, 50.0), HGF2DPosition(0.0, 50.0)),
                                                         HGF2DLiteSegment(HGF2DPosition(0.0, 50.0), HGF2DPosition(10.0, 50.0))));

    ASSERT_FALSE(VerticalSegment2.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(10.0, 1000.0), HGF2DPosition(0.0, 50.0)),
                                                         HGF2DLiteSegment(HGF2DPosition(0.0, 50.0), HGF2DPosition(1000.0, -1000.0))));


    // Intersects true with horizontal segment
    ASSERT_TRUE(HorizontalSegment1.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(50.0, 10.0), HGF2DPosition(50.0, 0.0)),
                                                          HGF2DLiteSegment(HGF2DPosition(50.0, 0.0), HGF2DPosition(50.0, -10.0))));

    ASSERT_TRUE(HorizontalSegment1.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(50.0, 10.0), HGF2DPosition(50.0, 0.0)),
                                                          HGF2DLiteSegment(HGF2DPosition(50.0, 0.0), HGF2DPosition(1000.0, -10.0))));

    ASSERT_TRUE(HorizontalSegment1.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(1000.0, 10.0), HGF2DPosition(50.0, 0.0)),
                                                          HGF2DLiteSegment(HGF2DPosition(50.0, 0.0), HGF2DPosition(1000.0, -10.0))));

    ASSERT_TRUE(HorizontalSegment2.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(50.0, 10.0), HGF2DPosition(50.0, 0.0)),
                                                          HGF2DLiteSegment(HGF2DPosition(50.0, 0.0), HGF2DPosition(50.0, -10.0))));

    ASSERT_TRUE(HorizontalSegment2.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(50.0, 10.0), HGF2DPosition(50.0, 0.0)),
                                                          HGF2DLiteSegment(HGF2DPosition(50.0, 0.0), HGF2DPosition(1000.0, -10.0))));

    ASSERT_TRUE(HorizontalSegment2.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(1000.0, 10.0), HGF2DPosition(50.0, 0.0)),
                                                          HGF2DLiteSegment(HGF2DPosition(50.0, 0.0), HGF2DPosition(1000.0, -10.0))));


    // Intersect false with horizontal segment
    ASSERT_FALSE(HorizontalSegment1.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(50.0, 10.0), HGF2DPosition(50.0, 0.0)),
                                                           HGF2DLiteSegment(HGF2DPosition(50.0, 0.0), HGF2DPosition(1000.0, 10.0))));

    ASSERT_FALSE(HorizontalSegment1.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(50.0, 10.0), HGF2DPosition(50.0, 0.0)),
                                                           HGF2DLiteSegment(HGF2DPosition(50.0, 0.0), HGF2DPosition(50.0, 10.0))));

    ASSERT_FALSE(HorizontalSegment1.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(1000.0, 10.0), HGF2DPosition(50.0, 0.0)),
                                                           HGF2DLiteSegment(HGF2DPosition(50.0, 0.0), HGF2DPosition(-1000.0, 1000.0))));

    ASSERT_FALSE(HorizontalSegment2.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(50.0, 10.0), HGF2DPosition(50.0, 0.0)),
                                                           HGF2DLiteSegment(HGF2DPosition(50.0, 0.0), HGF2DPosition(1000.0, 10.0))));

    ASSERT_FALSE(HorizontalSegment2.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(50.0, 10.0), HGF2DPosition(50.0, 0.0)),
                                                           HGF2DLiteSegment(HGF2DPosition(50.0, 0.0), HGF2DPosition(50.0, 10.0))));

    ASSERT_FALSE(HorizontalSegment2.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(1000.0, 10.0), HGF2DPosition(50.0, 0.0)),
                                                           HGF2DLiteSegment(HGF2DPosition(50.0, 0.0), HGF2DPosition(-1000.0, 1000.0))));


    // Intersects true with misc segment 1
    ASSERT_TRUE(MiscSegment1.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(0.0, 50.0), HGF2DPosition(50.0, 50.0)),
                                                    HGF2DLiteSegment(HGF2DPosition(50.0, 50.0), HGF2DPosition(100.0, 50.0))));

    ASSERT_TRUE(MiscSegment1.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(0.0, 10.0), HGF2DPosition(50.0, 50.0)),
                                                    HGF2DLiteSegment(HGF2DPosition(50.0, 50.0), HGF2DPosition(100.0, 80.0))));

    ASSERT_TRUE(MiscSegment1.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(-100.0, 10000.0), HGF2DPosition(50.0, 50.0)),
                                                    HGF2DLiteSegment(HGF2DPosition(50.0, 50.0), HGF2DPosition(100.0, -10000.0))));

    // Intersects false with misc segment 1
    ASSERT_FALSE(MiscSegment1.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(0.0, 50.0), HGF2DPosition(50.0, 50.0)),
                                                     HGF2DLiteSegment(HGF2DPosition(50.0, 50.0), HGF2DPosition(-100.0, 50.0))));

    ASSERT_FALSE(MiscSegment1.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(0.0, 10.0), HGF2DPosition(50.0, 50.0)),
                                                     HGF2DLiteSegment(HGF2DPosition(50.0, 50.0), HGF2DPosition(-100.0, 80.0))));

    ASSERT_FALSE(MiscSegment1.IntersectsAtSplitPoint(HGF2DLiteSegment(HGF2DPosition(-100.0, 10000.0), HGF2DPosition(50.0, 50.0)),
                                                     HGF2DLiteSegment(HGF2DPosition(50.0, 50.0), HGF2DPosition(-100.0, 0.0))));

    }

//==================================================================================
// Additional tests for contiguousness
//==================================================================================
TEST_F (HGF2DLiteSegmentTester, AreContiguousTest1)
    {

    HGF2DLiteSegment    HorizontalSegmentSup1(HGF2DPosition(10.0, 10.0), HGF2DPosition(20.0, 10.0));
    HGF2DLiteSegment    HorizontalSegmentSup2(HGF2DPosition(20.0, 10.0), HGF2DPosition(35.0, 10.0));

    ASSERT_FALSE(HorizontalSegmentSup1.AreContiguous(HorizontalSegmentSup2));
    ASSERT_FALSE(HorizontalSegmentSup2.AreContiguous(HorizontalSegmentSup1));

    HGF2DLiteSegment    VerticalSegmentSup1(HGF2DPosition(10.0, 10.0), HGF2DPosition(10.0, 20.0));
    HGF2DLiteSegment    VerticalSegmentSup2(HGF2DPosition(10.0, 20.0), HGF2DPosition(10.0, 35.0));

    ASSERT_FALSE(VerticalSegmentSup1.AreContiguous(VerticalSegmentSup2));
    ASSERT_FALSE(VerticalSegmentSup2.AreContiguous(VerticalSegmentSup1));

    }
//==================================================================================
// Other test which failed
// all of the following are contiguous
//==================================================================================
TEST_F (HGF2DLiteSegmentTester, AreContiguousTest2)
    {

    HGF2DLiteSegment    HorizontalSegmentSup3(HGF2DPosition(10.0, 10.0), HGF2DPosition(20.0, 10.0));
    HGF2DLiteSegment    HorizontalSegmentSup4(HGF2DPosition(5.0, 10.0), HGF2DPosition(35.0, 10.0));
    HGF2DLiteSegment    HorizontalSegmentSup5(HGF2DPosition(5.0, 10.0), HGF2DPosition(20.0, 10.0));
    HGF2DLiteSegment    HorizontalSegmentSup6(HGF2DPosition(10.0, 10.0), HGF2DPosition(25.0, 10.0));
    HGF2DLiteSegment    HorizontalSegmentSup7(HGF2DPosition(5.0, 10.0), HGF2DPosition(15.0, 10.0));
    HGF2DLiteSegment    HorizontalSegmentSup8(HGF2DPosition(13.0, 10.0), HGF2DPosition(25.0, 10.0));
    HGF2DLiteSegment    HorizontalSegmentSup9(HGF2DPosition(12.0, 10.0), HGF2DPosition(18.0, 10.0));

    HGF2DLiteSegment    HorizontalSegmentSup10(HGF2DPosition(20.0, 10.0), HGF2DPosition(10.0, 10.0));
    HGF2DLiteSegment    HorizontalSegmentSup11(HGF2DPosition(35.0, 10.0), HGF2DPosition(5.0, 10.0));
    HGF2DLiteSegment    HorizontalSegmentSup12(HGF2DPosition(20.0, 10.0), HGF2DPosition(5.0, 10.0));
    HGF2DLiteSegment    HorizontalSegmentSup13(HGF2DPosition(25.0, 10.0), HGF2DPosition(10.0, 10.0));
    HGF2DLiteSegment    HorizontalSegmentSup14(HGF2DPosition(15.0, 10.0), HGF2DPosition(5.0, 10.0));
    HGF2DLiteSegment    HorizontalSegmentSup15(HGF2DPosition(25.0, 10.0), HGF2DPosition(13.0, 10.0));
    HGF2DLiteSegment    HorizontalSegmentSup16(HGF2DPosition(18.0, 10.0), HGF2DPosition(12.0, 10.0));

    ASSERT_TRUE(HorizontalSegmentSup3.AreContiguous(HorizontalSegmentSup4));
    ASSERT_TRUE(HorizontalSegmentSup4.AreContiguous(HorizontalSegmentSup3));
    ASSERT_TRUE(HorizontalSegmentSup3.AreContiguous(HorizontalSegmentSup5));
    ASSERT_TRUE(HorizontalSegmentSup5.AreContiguous(HorizontalSegmentSup3));
    ASSERT_TRUE(HorizontalSegmentSup3.AreContiguous(HorizontalSegmentSup6));
    ASSERT_TRUE(HorizontalSegmentSup6.AreContiguous(HorizontalSegmentSup3));
    ASSERT_TRUE(HorizontalSegmentSup3.AreContiguous(HorizontalSegmentSup7));
    ASSERT_TRUE(HorizontalSegmentSup7.AreContiguous(HorizontalSegmentSup3));
    ASSERT_TRUE(HorizontalSegmentSup3.AreContiguous(HorizontalSegmentSup8));
    ASSERT_TRUE(HorizontalSegmentSup8.AreContiguous(HorizontalSegmentSup3));
    ASSERT_TRUE(HorizontalSegmentSup3.AreContiguous(HorizontalSegmentSup9));
    ASSERT_TRUE(HorizontalSegmentSup9.AreContiguous(HorizontalSegmentSup3));
    ASSERT_TRUE(HorizontalSegmentSup3.AreContiguous(HorizontalSegmentSup10));
    ASSERT_TRUE(HorizontalSegmentSup10.AreContiguous(HorizontalSegmentSup3));
    ASSERT_TRUE(HorizontalSegmentSup3.AreContiguous(HorizontalSegmentSup11));
    ASSERT_TRUE(HorizontalSegmentSup11.AreContiguous(HorizontalSegmentSup3));
    ASSERT_TRUE(HorizontalSegmentSup3.AreContiguous(HorizontalSegmentSup12));
    ASSERT_TRUE(HorizontalSegmentSup12.AreContiguous(HorizontalSegmentSup3));
    ASSERT_TRUE(HorizontalSegmentSup3.AreContiguous(HorizontalSegmentSup13));
    ASSERT_TRUE(HorizontalSegmentSup13.AreContiguous(HorizontalSegmentSup3));
    ASSERT_TRUE(HorizontalSegmentSup3.AreContiguous(HorizontalSegmentSup14));
    ASSERT_TRUE(HorizontalSegmentSup14.AreContiguous(HorizontalSegmentSup3));
    ASSERT_TRUE(HorizontalSegmentSup3.AreContiguous(HorizontalSegmentSup15));
    ASSERT_TRUE(HorizontalSegmentSup15.AreContiguous(HorizontalSegmentSup3));
    ASSERT_TRUE(HorizontalSegmentSup3.AreContiguous(HorizontalSegmentSup16));
    ASSERT_TRUE(HorizontalSegmentSup16.AreContiguous(HorizontalSegmentSup3));

    ASSERT_TRUE(HorizontalSegmentSup4.AreContiguous(HorizontalSegmentSup5));
    ASSERT_TRUE(HorizontalSegmentSup5.AreContiguous(HorizontalSegmentSup4));
    ASSERT_TRUE(HorizontalSegmentSup4.AreContiguous(HorizontalSegmentSup6));
    ASSERT_TRUE(HorizontalSegmentSup6.AreContiguous(HorizontalSegmentSup4));
    ASSERT_TRUE(HorizontalSegmentSup4.AreContiguous(HorizontalSegmentSup7));
    ASSERT_TRUE(HorizontalSegmentSup7.AreContiguous(HorizontalSegmentSup4));
    ASSERT_TRUE(HorizontalSegmentSup4.AreContiguous(HorizontalSegmentSup8));
    ASSERT_TRUE(HorizontalSegmentSup8.AreContiguous(HorizontalSegmentSup4));
    ASSERT_TRUE(HorizontalSegmentSup4.AreContiguous(HorizontalSegmentSup9));
    ASSERT_TRUE(HorizontalSegmentSup9.AreContiguous(HorizontalSegmentSup4));
    ASSERT_TRUE(HorizontalSegmentSup4.AreContiguous(HorizontalSegmentSup10));
    ASSERT_TRUE(HorizontalSegmentSup10.AreContiguous(HorizontalSegmentSup4));
    ASSERT_TRUE(HorizontalSegmentSup4.AreContiguous(HorizontalSegmentSup11));
    ASSERT_TRUE(HorizontalSegmentSup11.AreContiguous(HorizontalSegmentSup4));
    ASSERT_TRUE(HorizontalSegmentSup4.AreContiguous(HorizontalSegmentSup12));
    ASSERT_TRUE(HorizontalSegmentSup12.AreContiguous(HorizontalSegmentSup4));
    ASSERT_TRUE(HorizontalSegmentSup4.AreContiguous(HorizontalSegmentSup13));
    ASSERT_TRUE(HorizontalSegmentSup13.AreContiguous(HorizontalSegmentSup4));
    ASSERT_TRUE(HorizontalSegmentSup4.AreContiguous(HorizontalSegmentSup14));
    ASSERT_TRUE(HorizontalSegmentSup14.AreContiguous(HorizontalSegmentSup4));
    ASSERT_TRUE(HorizontalSegmentSup4.AreContiguous(HorizontalSegmentSup15));
    ASSERT_TRUE(HorizontalSegmentSup15.AreContiguous(HorizontalSegmentSup4));
    ASSERT_TRUE(HorizontalSegmentSup4.AreContiguous(HorizontalSegmentSup16));
    ASSERT_TRUE(HorizontalSegmentSup16.AreContiguous(HorizontalSegmentSup4));

    ASSERT_TRUE(HorizontalSegmentSup5.AreContiguous(HorizontalSegmentSup6));
    ASSERT_TRUE(HorizontalSegmentSup6.AreContiguous(HorizontalSegmentSup5));
    ASSERT_TRUE(HorizontalSegmentSup5.AreContiguous(HorizontalSegmentSup7));
    ASSERT_TRUE(HorizontalSegmentSup7.AreContiguous(HorizontalSegmentSup5));
    ASSERT_TRUE(HorizontalSegmentSup5.AreContiguous(HorizontalSegmentSup8));
    ASSERT_TRUE(HorizontalSegmentSup8.AreContiguous(HorizontalSegmentSup5));
    ASSERT_TRUE(HorizontalSegmentSup5.AreContiguous(HorizontalSegmentSup9));
    ASSERT_TRUE(HorizontalSegmentSup9.AreContiguous(HorizontalSegmentSup5));
    ASSERT_TRUE(HorizontalSegmentSup5.AreContiguous(HorizontalSegmentSup10));
    ASSERT_TRUE(HorizontalSegmentSup10.AreContiguous(HorizontalSegmentSup5));
    ASSERT_TRUE(HorizontalSegmentSup5.AreContiguous(HorizontalSegmentSup11));
    ASSERT_TRUE(HorizontalSegmentSup11.AreContiguous(HorizontalSegmentSup5));
    ASSERT_TRUE(HorizontalSegmentSup5.AreContiguous(HorizontalSegmentSup12));
    ASSERT_TRUE(HorizontalSegmentSup12.AreContiguous(HorizontalSegmentSup5));
    ASSERT_TRUE(HorizontalSegmentSup5.AreContiguous(HorizontalSegmentSup13));
    ASSERT_TRUE(HorizontalSegmentSup13.AreContiguous(HorizontalSegmentSup5));
    ASSERT_TRUE(HorizontalSegmentSup5.AreContiguous(HorizontalSegmentSup14));
    ASSERT_TRUE(HorizontalSegmentSup14.AreContiguous(HorizontalSegmentSup5));
    ASSERT_TRUE(HorizontalSegmentSup5.AreContiguous(HorizontalSegmentSup15));
    ASSERT_TRUE(HorizontalSegmentSup15.AreContiguous(HorizontalSegmentSup5));
    ASSERT_TRUE(HorizontalSegmentSup5.AreContiguous(HorizontalSegmentSup16));
    ASSERT_TRUE(HorizontalSegmentSup16.AreContiguous(HorizontalSegmentSup5));

    ASSERT_TRUE(HorizontalSegmentSup6.AreContiguous(HorizontalSegmentSup7));
    ASSERT_TRUE(HorizontalSegmentSup7.AreContiguous(HorizontalSegmentSup6));
    ASSERT_TRUE(HorizontalSegmentSup6.AreContiguous(HorizontalSegmentSup8));
    ASSERT_TRUE(HorizontalSegmentSup8.AreContiguous(HorizontalSegmentSup6));
    ASSERT_TRUE(HorizontalSegmentSup6.AreContiguous(HorizontalSegmentSup9));
    ASSERT_TRUE(HorizontalSegmentSup9.AreContiguous(HorizontalSegmentSup6));
    ASSERT_TRUE(HorizontalSegmentSup6.AreContiguous(HorizontalSegmentSup10));
    ASSERT_TRUE(HorizontalSegmentSup10.AreContiguous(HorizontalSegmentSup6));
    ASSERT_TRUE(HorizontalSegmentSup6.AreContiguous(HorizontalSegmentSup11));
    ASSERT_TRUE(HorizontalSegmentSup11.AreContiguous(HorizontalSegmentSup6));
    ASSERT_TRUE(HorizontalSegmentSup6.AreContiguous(HorizontalSegmentSup12));
    ASSERT_TRUE(HorizontalSegmentSup12.AreContiguous(HorizontalSegmentSup6));
    ASSERT_TRUE(HorizontalSegmentSup6.AreContiguous(HorizontalSegmentSup13));
    ASSERT_TRUE(HorizontalSegmentSup13.AreContiguous(HorizontalSegmentSup6));
    ASSERT_TRUE(HorizontalSegmentSup6.AreContiguous(HorizontalSegmentSup14));
    ASSERT_TRUE(HorizontalSegmentSup14.AreContiguous(HorizontalSegmentSup6));
    ASSERT_TRUE(HorizontalSegmentSup6.AreContiguous(HorizontalSegmentSup15));
    ASSERT_TRUE(HorizontalSegmentSup15.AreContiguous(HorizontalSegmentSup6));
    ASSERT_TRUE(HorizontalSegmentSup6.AreContiguous(HorizontalSegmentSup16));
    ASSERT_TRUE(HorizontalSegmentSup16.AreContiguous(HorizontalSegmentSup6));

    ASSERT_TRUE(HorizontalSegmentSup7.AreContiguous(HorizontalSegmentSup8));
    ASSERT_TRUE(HorizontalSegmentSup8.AreContiguous(HorizontalSegmentSup7));
    ASSERT_TRUE(HorizontalSegmentSup7.AreContiguous(HorizontalSegmentSup9));
    ASSERT_TRUE(HorizontalSegmentSup9.AreContiguous(HorizontalSegmentSup7));
    ASSERT_TRUE(HorizontalSegmentSup7.AreContiguous(HorizontalSegmentSup10));
    ASSERT_TRUE(HorizontalSegmentSup10.AreContiguous(HorizontalSegmentSup7));
    ASSERT_TRUE(HorizontalSegmentSup7.AreContiguous(HorizontalSegmentSup11));
    ASSERT_TRUE(HorizontalSegmentSup11.AreContiguous(HorizontalSegmentSup7));
    ASSERT_TRUE(HorizontalSegmentSup7.AreContiguous(HorizontalSegmentSup12));
    ASSERT_TRUE(HorizontalSegmentSup12.AreContiguous(HorizontalSegmentSup7));
    ASSERT_TRUE(HorizontalSegmentSup7.AreContiguous(HorizontalSegmentSup13));
    ASSERT_TRUE(HorizontalSegmentSup13.AreContiguous(HorizontalSegmentSup7));
    ASSERT_TRUE(HorizontalSegmentSup7.AreContiguous(HorizontalSegmentSup14));
    ASSERT_TRUE(HorizontalSegmentSup14.AreContiguous(HorizontalSegmentSup7));
    ASSERT_TRUE(HorizontalSegmentSup7.AreContiguous(HorizontalSegmentSup15));
    ASSERT_TRUE(HorizontalSegmentSup15.AreContiguous(HorizontalSegmentSup7));
    ASSERT_TRUE(HorizontalSegmentSup7.AreContiguous(HorizontalSegmentSup16));
    ASSERT_TRUE(HorizontalSegmentSup16.AreContiguous(HorizontalSegmentSup7));

    ASSERT_TRUE(HorizontalSegmentSup8.AreContiguous(HorizontalSegmentSup9));
    ASSERT_TRUE(HorizontalSegmentSup9.AreContiguous(HorizontalSegmentSup8));
    ASSERT_TRUE(HorizontalSegmentSup8.AreContiguous(HorizontalSegmentSup10));
    ASSERT_TRUE(HorizontalSegmentSup10.AreContiguous(HorizontalSegmentSup8));
    ASSERT_TRUE(HorizontalSegmentSup8.AreContiguous(HorizontalSegmentSup11));
    ASSERT_TRUE(HorizontalSegmentSup11.AreContiguous(HorizontalSegmentSup8));
    ASSERT_TRUE(HorizontalSegmentSup8.AreContiguous(HorizontalSegmentSup12));
    ASSERT_TRUE(HorizontalSegmentSup12.AreContiguous(HorizontalSegmentSup8));
    ASSERT_TRUE(HorizontalSegmentSup8.AreContiguous(HorizontalSegmentSup13));
    ASSERT_TRUE(HorizontalSegmentSup13.AreContiguous(HorizontalSegmentSup8));
    ASSERT_TRUE(HorizontalSegmentSup8.AreContiguous(HorizontalSegmentSup14));
    ASSERT_TRUE(HorizontalSegmentSup14.AreContiguous(HorizontalSegmentSup8));
    ASSERT_TRUE(HorizontalSegmentSup8.AreContiguous(HorizontalSegmentSup15));
    ASSERT_TRUE(HorizontalSegmentSup15.AreContiguous(HorizontalSegmentSup8));
    ASSERT_TRUE(HorizontalSegmentSup8.AreContiguous(HorizontalSegmentSup16));
    ASSERT_TRUE(HorizontalSegmentSup16.AreContiguous(HorizontalSegmentSup8));

    ASSERT_TRUE(HorizontalSegmentSup9.AreContiguous(HorizontalSegmentSup10));
    ASSERT_TRUE(HorizontalSegmentSup10.AreContiguous(HorizontalSegmentSup9));
    ASSERT_TRUE(HorizontalSegmentSup9.AreContiguous(HorizontalSegmentSup11));
    ASSERT_TRUE(HorizontalSegmentSup11.AreContiguous(HorizontalSegmentSup9));
    ASSERT_TRUE(HorizontalSegmentSup9.AreContiguous(HorizontalSegmentSup12));
    ASSERT_TRUE(HorizontalSegmentSup12.AreContiguous(HorizontalSegmentSup9));
    ASSERT_TRUE(HorizontalSegmentSup9.AreContiguous(HorizontalSegmentSup13));
    ASSERT_TRUE(HorizontalSegmentSup13.AreContiguous(HorizontalSegmentSup9));
    ASSERT_TRUE(HorizontalSegmentSup9.AreContiguous(HorizontalSegmentSup14));
    ASSERT_TRUE(HorizontalSegmentSup14.AreContiguous(HorizontalSegmentSup9));
    ASSERT_TRUE(HorizontalSegmentSup9.AreContiguous(HorizontalSegmentSup15));
    ASSERT_TRUE(HorizontalSegmentSup15.AreContiguous(HorizontalSegmentSup9));
    ASSERT_TRUE(HorizontalSegmentSup9.AreContiguous(HorizontalSegmentSup16));
    ASSERT_TRUE(HorizontalSegmentSup16.AreContiguous(HorizontalSegmentSup9));

    ASSERT_TRUE(HorizontalSegmentSup10.AreContiguous(HorizontalSegmentSup11));
    ASSERT_TRUE(HorizontalSegmentSup11.AreContiguous(HorizontalSegmentSup10));
    ASSERT_TRUE(HorizontalSegmentSup10.AreContiguous(HorizontalSegmentSup12));
    ASSERT_TRUE(HorizontalSegmentSup12.AreContiguous(HorizontalSegmentSup10));
    ASSERT_TRUE(HorizontalSegmentSup10.AreContiguous(HorizontalSegmentSup13));
    ASSERT_TRUE(HorizontalSegmentSup13.AreContiguous(HorizontalSegmentSup10));
    ASSERT_TRUE(HorizontalSegmentSup10.AreContiguous(HorizontalSegmentSup14));
    ASSERT_TRUE(HorizontalSegmentSup14.AreContiguous(HorizontalSegmentSup10));
    ASSERT_TRUE(HorizontalSegmentSup10.AreContiguous(HorizontalSegmentSup15));
    ASSERT_TRUE(HorizontalSegmentSup15.AreContiguous(HorizontalSegmentSup10));
    ASSERT_TRUE(HorizontalSegmentSup10.AreContiguous(HorizontalSegmentSup16));
    ASSERT_TRUE(HorizontalSegmentSup16.AreContiguous(HorizontalSegmentSup10));

    ASSERT_TRUE(HorizontalSegmentSup11.AreContiguous(HorizontalSegmentSup12));
    ASSERT_TRUE(HorizontalSegmentSup12.AreContiguous(HorizontalSegmentSup11));
    ASSERT_TRUE(HorizontalSegmentSup11.AreContiguous(HorizontalSegmentSup13));
    ASSERT_TRUE(HorizontalSegmentSup13.AreContiguous(HorizontalSegmentSup11));
    ASSERT_TRUE(HorizontalSegmentSup11.AreContiguous(HorizontalSegmentSup14));
    ASSERT_TRUE(HorizontalSegmentSup14.AreContiguous(HorizontalSegmentSup11));
    ASSERT_TRUE(HorizontalSegmentSup11.AreContiguous(HorizontalSegmentSup15));
    ASSERT_TRUE(HorizontalSegmentSup15.AreContiguous(HorizontalSegmentSup11));
    ASSERT_TRUE(HorizontalSegmentSup11.AreContiguous(HorizontalSegmentSup16));
    ASSERT_TRUE(HorizontalSegmentSup16.AreContiguous(HorizontalSegmentSup11));

    ASSERT_TRUE(HorizontalSegmentSup12.AreContiguous(HorizontalSegmentSup13));
    ASSERT_TRUE(HorizontalSegmentSup13.AreContiguous(HorizontalSegmentSup12));
    ASSERT_TRUE(HorizontalSegmentSup12.AreContiguous(HorizontalSegmentSup14));
    ASSERT_TRUE(HorizontalSegmentSup14.AreContiguous(HorizontalSegmentSup12));
    ASSERT_TRUE(HorizontalSegmentSup12.AreContiguous(HorizontalSegmentSup15));
    ASSERT_TRUE(HorizontalSegmentSup15.AreContiguous(HorizontalSegmentSup12));
    ASSERT_TRUE(HorizontalSegmentSup12.AreContiguous(HorizontalSegmentSup16));
    ASSERT_TRUE(HorizontalSegmentSup16.AreContiguous(HorizontalSegmentSup12));

    ASSERT_TRUE(HorizontalSegmentSup13.AreContiguous(HorizontalSegmentSup14));
    ASSERT_TRUE(HorizontalSegmentSup14.AreContiguous(HorizontalSegmentSup13));
    ASSERT_TRUE(HorizontalSegmentSup13.AreContiguous(HorizontalSegmentSup15));
    ASSERT_TRUE(HorizontalSegmentSup15.AreContiguous(HorizontalSegmentSup13));
    ASSERT_TRUE(HorizontalSegmentSup13.AreContiguous(HorizontalSegmentSup16));
    ASSERT_TRUE(HorizontalSegmentSup16.AreContiguous(HorizontalSegmentSup13));

    ASSERT_TRUE(HorizontalSegmentSup14.AreContiguous(HorizontalSegmentSup15));
    ASSERT_TRUE(HorizontalSegmentSup15.AreContiguous(HorizontalSegmentSup14));
    ASSERT_TRUE(HorizontalSegmentSup14.AreContiguous(HorizontalSegmentSup16));
    ASSERT_TRUE(HorizontalSegmentSup16.AreContiguous(HorizontalSegmentSup14));

    ASSERT_TRUE(HorizontalSegmentSup15.AreContiguous(HorizontalSegmentSup16));
    ASSERT_TRUE(HorizontalSegmentSup16.AreContiguous(HorizontalSegmentSup15));

    }

//==================================================================================
// Other test which failed
// all of the following are contiguous
//==================================================================================
TEST_F (HGF2DLiteSegmentTester, AreContiguousTest3)
    {

    HGF2DLiteSegment    VerticalSegmentSup3(HGF2DPosition(10.0, 10.0), HGF2DPosition(10.0, 20.0));
    HGF2DLiteSegment    VerticalSegmentSup4(HGF2DPosition(10.0, 5.0), HGF2DPosition(10.0, 35.0));
    HGF2DLiteSegment    VerticalSegmentSup5(HGF2DPosition(10.0, 5.0), HGF2DPosition(10.0, 20.0));
    HGF2DLiteSegment    VerticalSegmentSup6(HGF2DPosition(10.0, 10.0), HGF2DPosition(10.0, 25.0));
    HGF2DLiteSegment    VerticalSegmentSup7(HGF2DPosition(10.0, 5.0), HGF2DPosition(10.0, 15.0));
    HGF2DLiteSegment    VerticalSegmentSup8(HGF2DPosition(10.0, 13.0), HGF2DPosition(10.0, 25.0));
    HGF2DLiteSegment    VerticalSegmentSup9(HGF2DPosition(10.0, 12.0), HGF2DPosition(10.0, 18.0));

    HGF2DLiteSegment    VerticalSegmentSup10(HGF2DPosition(10.0, 20.0), HGF2DPosition(10.0, 10.0));
    HGF2DLiteSegment    VerticalSegmentSup11(HGF2DPosition(10.0, 35.0), HGF2DPosition(10.0, 5.0));
    HGF2DLiteSegment    VerticalSegmentSup12(HGF2DPosition(10.0, 20.0), HGF2DPosition(10.0, 5.0));
    HGF2DLiteSegment    VerticalSegmentSup13(HGF2DPosition(10.0, 25.0), HGF2DPosition(10.0, 10.0));
    HGF2DLiteSegment    VerticalSegmentSup14(HGF2DPosition(10.0, 15.0), HGF2DPosition(10.0, 5.0));
    HGF2DLiteSegment    VerticalSegmentSup15(HGF2DPosition(10.0, 25.0), HGF2DPosition(10.0, 13.0));
    HGF2DLiteSegment    VerticalSegmentSup16(HGF2DPosition(10.0, 18.0), HGF2DPosition(10.0, 12.0));

    ASSERT_TRUE(VerticalSegmentSup3.AreContiguous(VerticalSegmentSup4));
    ASSERT_TRUE(VerticalSegmentSup4.AreContiguous(VerticalSegmentSup3));
    ASSERT_TRUE(VerticalSegmentSup3.AreContiguous(VerticalSegmentSup5));
    ASSERT_TRUE(VerticalSegmentSup5.AreContiguous(VerticalSegmentSup3));
    ASSERT_TRUE(VerticalSegmentSup3.AreContiguous(VerticalSegmentSup6));
    ASSERT_TRUE(VerticalSegmentSup6.AreContiguous(VerticalSegmentSup3));
    ASSERT_TRUE(VerticalSegmentSup3.AreContiguous(VerticalSegmentSup7));
    ASSERT_TRUE(VerticalSegmentSup7.AreContiguous(VerticalSegmentSup3));
    ASSERT_TRUE(VerticalSegmentSup3.AreContiguous(VerticalSegmentSup8));
    ASSERT_TRUE(VerticalSegmentSup8.AreContiguous(VerticalSegmentSup3));
    ASSERT_TRUE(VerticalSegmentSup3.AreContiguous(VerticalSegmentSup9));
    ASSERT_TRUE(VerticalSegmentSup9.AreContiguous(VerticalSegmentSup3));
    ASSERT_TRUE(VerticalSegmentSup3.AreContiguous(VerticalSegmentSup10));
    ASSERT_TRUE(VerticalSegmentSup10.AreContiguous(VerticalSegmentSup3));
    ASSERT_TRUE(VerticalSegmentSup3.AreContiguous(VerticalSegmentSup11));
    ASSERT_TRUE(VerticalSegmentSup11.AreContiguous(VerticalSegmentSup3));
    ASSERT_TRUE(VerticalSegmentSup3.AreContiguous(VerticalSegmentSup12));
    ASSERT_TRUE(VerticalSegmentSup12.AreContiguous(VerticalSegmentSup3));
    ASSERT_TRUE(VerticalSegmentSup3.AreContiguous(VerticalSegmentSup13));
    ASSERT_TRUE(VerticalSegmentSup13.AreContiguous(VerticalSegmentSup3));
    ASSERT_TRUE(VerticalSegmentSup3.AreContiguous(VerticalSegmentSup14));
    ASSERT_TRUE(VerticalSegmentSup14.AreContiguous(VerticalSegmentSup3));
    ASSERT_TRUE(VerticalSegmentSup3.AreContiguous(VerticalSegmentSup15));
    ASSERT_TRUE(VerticalSegmentSup15.AreContiguous(VerticalSegmentSup3));
    ASSERT_TRUE(VerticalSegmentSup3.AreContiguous(VerticalSegmentSup16));
    ASSERT_TRUE(VerticalSegmentSup16.AreContiguous(VerticalSegmentSup3));

    ASSERT_TRUE(VerticalSegmentSup4.AreContiguous(VerticalSegmentSup5));
    ASSERT_TRUE(VerticalSegmentSup5.AreContiguous(VerticalSegmentSup4));
    ASSERT_TRUE(VerticalSegmentSup4.AreContiguous(VerticalSegmentSup6));
    ASSERT_TRUE(VerticalSegmentSup6.AreContiguous(VerticalSegmentSup4));
    ASSERT_TRUE(VerticalSegmentSup4.AreContiguous(VerticalSegmentSup7));
    ASSERT_TRUE(VerticalSegmentSup7.AreContiguous(VerticalSegmentSup4));
    ASSERT_TRUE(VerticalSegmentSup4.AreContiguous(VerticalSegmentSup8));
    ASSERT_TRUE(VerticalSegmentSup8.AreContiguous(VerticalSegmentSup4));
    ASSERT_TRUE(VerticalSegmentSup4.AreContiguous(VerticalSegmentSup9));
    ASSERT_TRUE(VerticalSegmentSup9.AreContiguous(VerticalSegmentSup4));
    ASSERT_TRUE(VerticalSegmentSup4.AreContiguous(VerticalSegmentSup10));
    ASSERT_TRUE(VerticalSegmentSup10.AreContiguous(VerticalSegmentSup4));
    ASSERT_TRUE(VerticalSegmentSup4.AreContiguous(VerticalSegmentSup11));
    ASSERT_TRUE(VerticalSegmentSup11.AreContiguous(VerticalSegmentSup4));
    ASSERT_TRUE(VerticalSegmentSup4.AreContiguous(VerticalSegmentSup12));
    ASSERT_TRUE(VerticalSegmentSup12.AreContiguous(VerticalSegmentSup4));
    ASSERT_TRUE(VerticalSegmentSup4.AreContiguous(VerticalSegmentSup13));
    ASSERT_TRUE(VerticalSegmentSup13.AreContiguous(VerticalSegmentSup4));
    ASSERT_TRUE(VerticalSegmentSup4.AreContiguous(VerticalSegmentSup14));
    ASSERT_TRUE(VerticalSegmentSup14.AreContiguous(VerticalSegmentSup4));
    ASSERT_TRUE(VerticalSegmentSup4.AreContiguous(VerticalSegmentSup15));
    ASSERT_TRUE(VerticalSegmentSup15.AreContiguous(VerticalSegmentSup4));
    ASSERT_TRUE(VerticalSegmentSup4.AreContiguous(VerticalSegmentSup16));
    ASSERT_TRUE(VerticalSegmentSup16.AreContiguous(VerticalSegmentSup4));

    ASSERT_TRUE(VerticalSegmentSup5.AreContiguous(VerticalSegmentSup6));
    ASSERT_TRUE(VerticalSegmentSup6.AreContiguous(VerticalSegmentSup5));
    ASSERT_TRUE(VerticalSegmentSup5.AreContiguous(VerticalSegmentSup7));
    ASSERT_TRUE(VerticalSegmentSup7.AreContiguous(VerticalSegmentSup5));
    ASSERT_TRUE(VerticalSegmentSup5.AreContiguous(VerticalSegmentSup8));
    ASSERT_TRUE(VerticalSegmentSup8.AreContiguous(VerticalSegmentSup5));
    ASSERT_TRUE(VerticalSegmentSup5.AreContiguous(VerticalSegmentSup9));
    ASSERT_TRUE(VerticalSegmentSup9.AreContiguous(VerticalSegmentSup5));
    ASSERT_TRUE(VerticalSegmentSup5.AreContiguous(VerticalSegmentSup10));
    ASSERT_TRUE(VerticalSegmentSup10.AreContiguous(VerticalSegmentSup5));
    ASSERT_TRUE(VerticalSegmentSup5.AreContiguous(VerticalSegmentSup11));
    ASSERT_TRUE(VerticalSegmentSup11.AreContiguous(VerticalSegmentSup5));
    ASSERT_TRUE(VerticalSegmentSup5.AreContiguous(VerticalSegmentSup12));
    ASSERT_TRUE(VerticalSegmentSup12.AreContiguous(VerticalSegmentSup5));
    ASSERT_TRUE(VerticalSegmentSup5.AreContiguous(VerticalSegmentSup13));
    ASSERT_TRUE(VerticalSegmentSup13.AreContiguous(VerticalSegmentSup5));
    ASSERT_TRUE(VerticalSegmentSup5.AreContiguous(VerticalSegmentSup14));
    ASSERT_TRUE(VerticalSegmentSup14.AreContiguous(VerticalSegmentSup5));
    ASSERT_TRUE(VerticalSegmentSup5.AreContiguous(VerticalSegmentSup15));
    ASSERT_TRUE(VerticalSegmentSup15.AreContiguous(VerticalSegmentSup5));
    ASSERT_TRUE(VerticalSegmentSup5.AreContiguous(VerticalSegmentSup16));
    ASSERT_TRUE(VerticalSegmentSup16.AreContiguous(VerticalSegmentSup5));

    ASSERT_TRUE(VerticalSegmentSup6.AreContiguous(VerticalSegmentSup7));
    ASSERT_TRUE(VerticalSegmentSup7.AreContiguous(VerticalSegmentSup6));
    ASSERT_TRUE(VerticalSegmentSup6.AreContiguous(VerticalSegmentSup8));
    ASSERT_TRUE(VerticalSegmentSup8.AreContiguous(VerticalSegmentSup6));
    ASSERT_TRUE(VerticalSegmentSup6.AreContiguous(VerticalSegmentSup9));
    ASSERT_TRUE(VerticalSegmentSup9.AreContiguous(VerticalSegmentSup6));
    ASSERT_TRUE(VerticalSegmentSup6.AreContiguous(VerticalSegmentSup10));
    ASSERT_TRUE(VerticalSegmentSup10.AreContiguous(VerticalSegmentSup6));
    ASSERT_TRUE(VerticalSegmentSup6.AreContiguous(VerticalSegmentSup11));
    ASSERT_TRUE(VerticalSegmentSup11.AreContiguous(VerticalSegmentSup6));
    ASSERT_TRUE(VerticalSegmentSup6.AreContiguous(VerticalSegmentSup12));
    ASSERT_TRUE(VerticalSegmentSup12.AreContiguous(VerticalSegmentSup6));
    ASSERT_TRUE(VerticalSegmentSup6.AreContiguous(VerticalSegmentSup13));
    ASSERT_TRUE(VerticalSegmentSup13.AreContiguous(VerticalSegmentSup6));
    ASSERT_TRUE(VerticalSegmentSup6.AreContiguous(VerticalSegmentSup14));
    ASSERT_TRUE(VerticalSegmentSup14.AreContiguous(VerticalSegmentSup6));
    ASSERT_TRUE(VerticalSegmentSup6.AreContiguous(VerticalSegmentSup15));
    ASSERT_TRUE(VerticalSegmentSup15.AreContiguous(VerticalSegmentSup6));
    ASSERT_TRUE(VerticalSegmentSup6.AreContiguous(VerticalSegmentSup16));
    ASSERT_TRUE(VerticalSegmentSup16.AreContiguous(VerticalSegmentSup6));

    ASSERT_TRUE(VerticalSegmentSup7.AreContiguous(VerticalSegmentSup8));
    ASSERT_TRUE(VerticalSegmentSup8.AreContiguous(VerticalSegmentSup7));
    ASSERT_TRUE(VerticalSegmentSup7.AreContiguous(VerticalSegmentSup9));
    ASSERT_TRUE(VerticalSegmentSup9.AreContiguous(VerticalSegmentSup7));
    ASSERT_TRUE(VerticalSegmentSup7.AreContiguous(VerticalSegmentSup10));
    ASSERT_TRUE(VerticalSegmentSup10.AreContiguous(VerticalSegmentSup7));
    ASSERT_TRUE(VerticalSegmentSup7.AreContiguous(VerticalSegmentSup11));
    ASSERT_TRUE(VerticalSegmentSup11.AreContiguous(VerticalSegmentSup7));
    ASSERT_TRUE(VerticalSegmentSup7.AreContiguous(VerticalSegmentSup12));
    ASSERT_TRUE(VerticalSegmentSup12.AreContiguous(VerticalSegmentSup7));
    ASSERT_TRUE(VerticalSegmentSup7.AreContiguous(VerticalSegmentSup13));
    ASSERT_TRUE(VerticalSegmentSup13.AreContiguous(VerticalSegmentSup7));
    ASSERT_TRUE(VerticalSegmentSup7.AreContiguous(VerticalSegmentSup14));
    ASSERT_TRUE(VerticalSegmentSup14.AreContiguous(VerticalSegmentSup7));
    ASSERT_TRUE(VerticalSegmentSup7.AreContiguous(VerticalSegmentSup15));
    ASSERT_TRUE(VerticalSegmentSup15.AreContiguous(VerticalSegmentSup7));
    ASSERT_TRUE(VerticalSegmentSup7.AreContiguous(VerticalSegmentSup16));
    ASSERT_TRUE(VerticalSegmentSup16.AreContiguous(VerticalSegmentSup7));

    ASSERT_TRUE(VerticalSegmentSup8.AreContiguous(VerticalSegmentSup9));
    ASSERT_TRUE(VerticalSegmentSup9.AreContiguous(VerticalSegmentSup8));
    ASSERT_TRUE(VerticalSegmentSup8.AreContiguous(VerticalSegmentSup10));
    ASSERT_TRUE(VerticalSegmentSup10.AreContiguous(VerticalSegmentSup8));
    ASSERT_TRUE(VerticalSegmentSup8.AreContiguous(VerticalSegmentSup11));
    ASSERT_TRUE(VerticalSegmentSup11.AreContiguous(VerticalSegmentSup8));
    ASSERT_TRUE(VerticalSegmentSup8.AreContiguous(VerticalSegmentSup12));
    ASSERT_TRUE(VerticalSegmentSup12.AreContiguous(VerticalSegmentSup8));
    ASSERT_TRUE(VerticalSegmentSup8.AreContiguous(VerticalSegmentSup13));
    ASSERT_TRUE(VerticalSegmentSup13.AreContiguous(VerticalSegmentSup8));
    ASSERT_TRUE(VerticalSegmentSup8.AreContiguous(VerticalSegmentSup14));
    ASSERT_TRUE(VerticalSegmentSup14.AreContiguous(VerticalSegmentSup8));
    ASSERT_TRUE(VerticalSegmentSup8.AreContiguous(VerticalSegmentSup15));
    ASSERT_TRUE(VerticalSegmentSup15.AreContiguous(VerticalSegmentSup8));
    ASSERT_TRUE(VerticalSegmentSup8.AreContiguous(VerticalSegmentSup16));
    ASSERT_TRUE(VerticalSegmentSup16.AreContiguous(VerticalSegmentSup8));

    ASSERT_TRUE(VerticalSegmentSup9.AreContiguous(VerticalSegmentSup10));
    ASSERT_TRUE(VerticalSegmentSup10.AreContiguous(VerticalSegmentSup9));
    ASSERT_TRUE(VerticalSegmentSup9.AreContiguous(VerticalSegmentSup11));
    ASSERT_TRUE(VerticalSegmentSup11.AreContiguous(VerticalSegmentSup9));
    ASSERT_TRUE(VerticalSegmentSup9.AreContiguous(VerticalSegmentSup12));
    ASSERT_TRUE(VerticalSegmentSup12.AreContiguous(VerticalSegmentSup9));
    ASSERT_TRUE(VerticalSegmentSup9.AreContiguous(VerticalSegmentSup13));
    ASSERT_TRUE(VerticalSegmentSup13.AreContiguous(VerticalSegmentSup9));
    ASSERT_TRUE(VerticalSegmentSup9.AreContiguous(VerticalSegmentSup14));
    ASSERT_TRUE(VerticalSegmentSup14.AreContiguous(VerticalSegmentSup9));
    ASSERT_TRUE(VerticalSegmentSup9.AreContiguous(VerticalSegmentSup15));
    ASSERT_TRUE(VerticalSegmentSup15.AreContiguous(VerticalSegmentSup9));
    ASSERT_TRUE(VerticalSegmentSup9.AreContiguous(VerticalSegmentSup16));
    ASSERT_TRUE(VerticalSegmentSup16.AreContiguous(VerticalSegmentSup9));

    ASSERT_TRUE(VerticalSegmentSup10.AreContiguous(VerticalSegmentSup11));
    ASSERT_TRUE(VerticalSegmentSup11.AreContiguous(VerticalSegmentSup10));
    ASSERT_TRUE(VerticalSegmentSup10.AreContiguous(VerticalSegmentSup12));
    ASSERT_TRUE(VerticalSegmentSup12.AreContiguous(VerticalSegmentSup10));
    ASSERT_TRUE(VerticalSegmentSup10.AreContiguous(VerticalSegmentSup13));
    ASSERT_TRUE(VerticalSegmentSup13.AreContiguous(VerticalSegmentSup10));
    ASSERT_TRUE(VerticalSegmentSup10.AreContiguous(VerticalSegmentSup14));
    ASSERT_TRUE(VerticalSegmentSup14.AreContiguous(VerticalSegmentSup10));
    ASSERT_TRUE(VerticalSegmentSup10.AreContiguous(VerticalSegmentSup15));
    ASSERT_TRUE(VerticalSegmentSup15.AreContiguous(VerticalSegmentSup10));
    ASSERT_TRUE(VerticalSegmentSup10.AreContiguous(VerticalSegmentSup16));
    ASSERT_TRUE(VerticalSegmentSup16.AreContiguous(VerticalSegmentSup10));

    ASSERT_TRUE(VerticalSegmentSup11.AreContiguous(VerticalSegmentSup12));
    ASSERT_TRUE(VerticalSegmentSup12.AreContiguous(VerticalSegmentSup11));
    ASSERT_TRUE(VerticalSegmentSup11.AreContiguous(VerticalSegmentSup13));
    ASSERT_TRUE(VerticalSegmentSup13.AreContiguous(VerticalSegmentSup11));
    ASSERT_TRUE(VerticalSegmentSup11.AreContiguous(VerticalSegmentSup14));
    ASSERT_TRUE(VerticalSegmentSup14.AreContiguous(VerticalSegmentSup11));
    ASSERT_TRUE(VerticalSegmentSup11.AreContiguous(VerticalSegmentSup15));
    ASSERT_TRUE(VerticalSegmentSup15.AreContiguous(VerticalSegmentSup11));
    ASSERT_TRUE(VerticalSegmentSup11.AreContiguous(VerticalSegmentSup16));
    ASSERT_TRUE(VerticalSegmentSup16.AreContiguous(VerticalSegmentSup11));

    ASSERT_TRUE(VerticalSegmentSup12.AreContiguous(VerticalSegmentSup13));
    ASSERT_TRUE(VerticalSegmentSup13.AreContiguous(VerticalSegmentSup12));
    ASSERT_TRUE(VerticalSegmentSup12.AreContiguous(VerticalSegmentSup14));
    ASSERT_TRUE(VerticalSegmentSup14.AreContiguous(VerticalSegmentSup12));
    ASSERT_TRUE(VerticalSegmentSup12.AreContiguous(VerticalSegmentSup15));
    ASSERT_TRUE(VerticalSegmentSup15.AreContiguous(VerticalSegmentSup12));
    ASSERT_TRUE(VerticalSegmentSup12.AreContiguous(VerticalSegmentSup16));
    ASSERT_TRUE(VerticalSegmentSup16.AreContiguous(VerticalSegmentSup12));

    ASSERT_TRUE(VerticalSegmentSup13.AreContiguous(VerticalSegmentSup14));
    ASSERT_TRUE(VerticalSegmentSup14.AreContiguous(VerticalSegmentSup13));
    ASSERT_TRUE(VerticalSegmentSup13.AreContiguous(VerticalSegmentSup15));
    ASSERT_TRUE(VerticalSegmentSup15.AreContiguous(VerticalSegmentSup13));
    ASSERT_TRUE(VerticalSegmentSup13.AreContiguous(VerticalSegmentSup16));
    ASSERT_TRUE(VerticalSegmentSup16.AreContiguous(VerticalSegmentSup13));

    ASSERT_TRUE(VerticalSegmentSup14.AreContiguous(VerticalSegmentSup15));
    ASSERT_TRUE(VerticalSegmentSup15.AreContiguous(VerticalSegmentSup14));
    ASSERT_TRUE(VerticalSegmentSup14.AreContiguous(VerticalSegmentSup16));
    ASSERT_TRUE(VerticalSegmentSup16.AreContiguous(VerticalSegmentSup14));

    ASSERT_TRUE(VerticalSegmentSup15.AreContiguous(VerticalSegmentSup16));
    ASSERT_TRUE(VerticalSegmentSup16.AreContiguous(VerticalSegmentSup15));

    }

//==================================================================================
// Another yet test which did fail
//==================================================================================
TEST_F (HGF2DLiteSegmentTester, ObtainContiguousnessPointsTest)
    {
        
    HGF2DLiteSegment    HorizontalSegmentSup23(HGF2DPosition(20.0, 20.0), HGF2DPosition(10.0, 20.0));
    HGF2DLiteSegment    HorizontalSegmentSup24(HGF2DPosition(20.0+0.9*MYEPSILON, 20.0), HGF2DPosition(-1.0, 20.0));

    HGF2DPositionCollection     Contig23Points;
    ASSERT_EQ(2, HorizontalSegmentSup23.ObtainContiguousnessPoints(HorizontalSegmentSup24, &Contig23Points));
       
    }

//==================================================================================
// Another yet test which did fail (July 23 1997)
//==================================================================================
TEST_F (HGF2DLiteSegmentTester, AreContiguous5)
    {
        
    HGF2DLiteSegment    HorizontalSegmentSup1(HGF2DPosition(0.0, 0.0), HGF2DPosition(0.0, 1E-6));
    HGF2DLiteSegment    HorizontalSegmentSup2(HGF2DPosition(0.0, 9.999999E-7), HGF2DPosition(0.0, 7.0));

    ASSERT_FALSE(HorizontalSegmentSup1.AreContiguous(HorizontalSegmentSup2));
    
    }

//==================================================================================
// Another yet test which did fail Aug 23 1997
//==================================================================================
TEST_F (HGF2DLiteSegmentTester, AreContiguous6)
    {

    HGF2DLiteSegment    HorizontalSegmentSup1(HGF2DPosition(2248.0, 495.41334701058), HGF2DPosition(2248.0, 298.10137161514));
    HGF2DLiteSegment    HorizontalSegmentSup2(HGF2DPosition(2248.0, 0.0), HGF2DPosition(2248.0, 298.10137161515));

    ASSERT_FALSE(HorizontalSegmentSup1.AreContiguous(HorizontalSegmentSup2));

    }
