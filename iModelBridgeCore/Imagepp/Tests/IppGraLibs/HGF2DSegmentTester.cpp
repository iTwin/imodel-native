//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DSegmentTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "EnvironnementTest.h"
#include "HGF2DSegmentTester.h"

//==================================================================================
// Segment Construction tests
// HGF2DSegment();
// HGF2DSegment(const HGF2DPosition&, const HGF2DPosition&);
// HGF2DSegment(const HGF2DPosition& pi_rStartPoint,
//             const HGF2DDisplacement& pi_rDisplacement);
// HGF2DSegment(const HGF2DSegment&    pi_rObject);
//==================================================================================
TEST_F (HGF2DSegmentTester, ConstructionTest)
    {

    // Default Constructor
    HGF2DSegment    Segment1;

    // Contructor by two points
    HGF2DPosition   FirstSegmentPoint(10.0, 10.2);
    HGF2DPosition   SecondSegmentPoint(-10000.0, 100.3);

    HGF2DSegment    Segment2(FirstSegmentPoint, SecondSegmentPoint);
    ASSERT_DOUBLE_EQ(10.00000, Segment2.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.20000, Segment2.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-10000.0, Segment2.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(100.3000, Segment2.GetEndPoint().GetY());

    HGF2DSegment    Segment3(FirstSegmentPoint, SecondSegmentPoint);
    ASSERT_DOUBLE_EQ(10.00000, Segment3.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.20000, Segment3.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-10000.0, Segment3.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(100.3000, Segment3.GetEndPoint().GetY());

    // Constructor by point and displacement
    HGF2DDisplacement   Displacement1(10.0, 10.0);
    HGF2DDisplacement   Displacement2(0.0, 10.0);
    HGF2DDisplacement   Displacement3(-10.0, -10.0);
    HGF2DDisplacement   Displacement4(0.0, 0.0);

    HGF2DSegment    Segment4(FirstSegmentPoint, Displacement1);
    ASSERT_DOUBLE_EQ(10.0, Segment4.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.2, Segment4.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, Segment4.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.2, Segment4.GetEndPoint().GetY());

    HGF2DSegment    Segment5(FirstSegmentPoint, Displacement2);
    ASSERT_DOUBLE_EQ(10.0, Segment5.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.2, Segment5.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, Segment5.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.2, Segment5.GetEndPoint().GetY());

    HGF2DSegment    Segment6(FirstSegmentPoint, Displacement3);
    ASSERT_DOUBLE_EQ(10.0000000000000000, Segment6.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.2000000000000000, Segment6.GetStartPoint().GetY());
    ASSERT_NEAR(0.0, Segment6.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.19999999999999929, Segment6.GetEndPoint().GetY());

    HGF2DSegment    Segment7(FirstSegmentPoint, Displacement4);
    ASSERT_DOUBLE_EQ(10.0, Segment7.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.2, Segment7.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, Segment7.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.2, Segment7.GetEndPoint().GetY());

    //Copy Constructor
    HGF2DSegment    Segment9(Segment4);
    ASSERT_DOUBLE_EQ(10.0, Segment9.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.2, Segment9.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, Segment9.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.2, Segment9.GetEndPoint().GetY());

    }

//==================================================================================
// operator= test
// operator=(const HGF2DSegment& pi_rObj);
//==================================================================================
TEST_F (HGF2DSegmentTester, OperatorTest)
    {
     
    // Test with different coord sys
    HGF2DPosition   FirstSegmentPoint(10.0, 10.2);
    HGF2DPosition   SecondSegmentPoint(-10000.0, 100.3);
    HGF2DSegment    Segment1(FirstSegmentPoint, SecondSegmentPoint);

    HGF2DSegment    Segment2(SecondSegmentPoint, FirstSegmentPoint);

    Segment2 = Segment1;
    ASSERT_DOUBLE_EQ(10.00000, Segment2.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.20000, Segment2.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-10000.0, Segment2.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(100.3000, Segment2.GetEndPoint().GetY());

    // Test with a NULL segment
    HGF2DSegment    Segment3;
    HGF2DSegment    Segment4;

    Segment4 = Segment3;
    ASSERT_TRUE(Segment4.IsNull());

    }

//==================================================================================
// Segment coordinate setting test
// void               SetStartPoint(const HGF2DPosition& pi_rNewStartPoint);
// void               SetEndPoint(const HGF2DPosition& pi_rNewEndPoint);
// void               SetRawStartPoint(double pi_X, double pi_Y);
// void               SetRawEndPoint(double pi_X, double pi_Y);
//==================================================================================
TEST_F (HGF2DSegmentTester, CoordinateTest)
    {

    // Test set to same coordinates
    HGF2DPosition   FirstSegmentPoint(10.0, 10.2);
    HGF2DPosition   SecondSegmentPoint(-10000.0, 100.3);
    HGF2DSegment    Segment1(FirstSegmentPoint, SecondSegmentPoint);

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

    Segment1.SetRawStartPoint(10.0, 10.0);
    ASSERT_DOUBLE_EQ(10.0, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, Segment1.GetStartPoint().GetY());

    Segment1.SetRawEndPoint(10.0, 10.0);
    ASSERT_DOUBLE_EQ(10.0, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, Segment1.GetEndPoint().GetY());

    // Test with vertical segment
    HGF2DSegment    Segment2(VerticalSegment1A);

    Segment2.SetStartPoint(HGF2DPosition(23.5, 23.5));
    ASSERT_DOUBLE_EQ(23.5, Segment2.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(23.5, Segment2.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment2.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment2.GetEndPoint().GetY());

    Segment2.SetEndPoint(HGF2DPosition(23.5, 23.5));
    ASSERT_DOUBLE_EQ(23.5, Segment2.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(23.5, Segment2.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(23.5, Segment2.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(23.5, Segment2.GetEndPoint().GetY());

    Segment2.SetRawStartPoint(23.6, 23.8);
    ASSERT_DOUBLE_EQ(23.6, Segment2.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(23.8, Segment2.GetStartPoint().GetY());

    Segment2.SetRawEndPoint(-3.6, -23.8);
    ASSERT_DOUBLE_EQ(-3.60, Segment2.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-23.8, Segment2.GetEndPoint().GetY());

    // Test with inverted vertical segment
    HGF2DSegment    Segment3(VerticalSegment2A);

    Segment3.SetStartPoint(HGF2DPosition(23.5, 23.5));
    ASSERT_DOUBLE_EQ(23.5, Segment3.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(23.5, Segment3.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment3.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment3.GetEndPoint().GetY());

    Segment3.SetEndPoint(HGF2DPosition(23.5, 23.5));
    ASSERT_DOUBLE_EQ(23.5, Segment3.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(23.5, Segment3.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(23.5, Segment3.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(23.5, Segment3.GetEndPoint().GetY()); 

    Segment3.SetRawStartPoint(23.6, 23.8);
    ASSERT_DOUBLE_EQ(23.6, Segment3.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(23.8, Segment3.GetStartPoint().GetY());

    Segment3.SetRawEndPoint(-3.6, -23.8);
    ASSERT_DOUBLE_EQ(-3.60, Segment3.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-23.8, Segment3.GetEndPoint().GetY());

    }

//==================================================================================
// Line calculation test
// CalculateLine() const;
//==================================================================================
TEST_F (HGF2DSegmentTester, CalculateLineTest)
    {

    // LINES
    HGF2DLiteLine       Line1(HGF2DPosition(1.1, 0.1), HGF2DPosition(-10.1, 10.1));
    HGF2DLiteLine       VerticalLine1C(HGF2DPosition(0.1, 0.1), HGF2DPosition(0.1, 10.1 + MYEPSILON));
    HGF2DLiteLine       Line1P(HGF2DPosition(0.2, 0.1), HGF2DPosition(10.2, 10.1));
    HGF2DLiteLine       Line1NP(HGF2DPosition(0.2, 0.1), HGF2DPosition(10.2, 10.1+MYEPSILON));
    HGF2DLiteLine       Line1CNP(HGF2DPosition(0.1+MYEPSILON, 0.1), HGF2DPosition(10.1-MYEPSILON, 10.1));
    HGF2DLiteLine       Line1CONNECTED(HGF2DPosition(0.21, 0.1), HGF2DPosition(10.1, 10.1));
    HGF2DLiteLine       HorizontalLine1(HGF2DPosition(0.1, 0.1), HGF2DPosition(10.1, 0.1));
    HGF2DLiteLine       InvalidLine(HGF2DPosition(0.1, 0.1), HGF2DPosition(10.1, 0.1));

    // Test with vertical segment
    HGF2DLiteLine   DumLine1 = VerticalSegment1A.CalculateLine();
    ASSERT_TRUE(DumLine1.IsVertical());
    ASSERT_DOUBLE_EQ(0.1, DumLine1.GetIntercept());

    // Test with inverted vertical segment
    HGF2DLiteLine   DumLine2 = VerticalSegment2A.CalculateLine();
    ASSERT_TRUE(DumLine2.IsVertical());
    ASSERT_DOUBLE_EQ(0.1, DumLine2.GetIntercept());

    // Test with horizontal segment
    HGF2DLiteLine   DumLine3 = HorizontalSegment1A.CalculateLine();
    ASSERT_FALSE(DumLine3.IsVertical());
    ASSERT_DOUBLE_EQ(0.1, DumLine3.GetIntercept());
    ASSERT_NEAR(0.0, DumLine3.GetSlope(), MYEPSILON);

    // Test with inverted horizontal segment
    HGF2DLiteLine   DumLine4 = HorizontalSegment2A.CalculateLine();
    ASSERT_FALSE(DumLine4.IsVertical());
    ASSERT_DOUBLE_EQ(0.1, DumLine4.GetIntercept());
    ASSERT_NEAR(0.0, DumLine4.GetSlope(), MYEPSILON);

    // Tests with vertical EPSILON sized segment
    HGF2DLiteLine   DumLine5 = VerticalSegment3A.CalculateLine();
    ASSERT_TRUE(DumLine5.IsVertical());
    ASSERT_DOUBLE_EQ(0.1, DumLine5.GetIntercept());

    // Tests with horizontal EPSILON sized segment
    HGF2DLiteLine   DumLine6 = HorizontalSegment3A.CalculateLine();
    ASSERT_FALSE(DumLine6.IsVertical());
    ASSERT_DOUBLE_EQ(0.1, DumLine6.GetIntercept());
    ASSERT_NEAR(0.0, DumLine6.GetSlope(), MYEPSILON);

    // Tests with miscalenious EPSILON size segment
    HGF2DLiteLine   DumLine7 = MiscSegment3B.CalculateLine();
    ASSERT_FALSE(DumLine7.IsVertical());
    ASSERT_DOUBLE_EQ(-0.35107085049510683, DumLine7.GetIntercept());
    ASSERT_DOUBLE_EQ(4.510708504951068700, DumLine7.GetSlope());

    // Test with very large segment
    HGF2DLiteLine   DumLine8 = LargeSegment1A.CalculateLine();
    ASSERT_FALSE(DumLine8.IsVertical());
    ASSERT_DOUBLE_EQ(-1.7E124, DumLine8.GetIntercept());
    ASSERT_DOUBLE_EQ(4.000000, DumLine8.GetSlope());

    // Test with segments way into positive regions
    HGF2DLiteLine   DumLine9 = PositiveSegment1A.CalculateLine();
    ASSERT_FALSE(DumLine9.IsVertical());
    ASSERT_DOUBLE_EQ(1.9E124, DumLine9.GetIntercept());
    ASSERT_DOUBLE_EQ(2.00000, DumLine9.GetSlope());

    // Test with segments way into negative regions
    HGF2DLiteLine   DumLine10 = NegativeSegment1A.CalculateLine();
    ASSERT_FALSE(DumLine10.IsVertical());
    ASSERT_DOUBLE_EQ(-1.9E124, DumLine10.GetIntercept());
    ASSERT_DOUBLE_EQ(2.000000, DumLine10.GetSlope());

    // Test with a NULL segments
    HGF2DLiteLine   DumLine11 = NullSegment1A.CalculateLine();
    ASSERT_TRUE(DumLine11.IsVertical());

    } 

//==================================================================================
// Line intersection test
// IntersectLine(const HGF2DLine& pi_rLine,HGF2DPosition* po_pPoint)const;
//==================================================================================
TEST_F (HGF2DSegmentTester, IntersectLineTest)
    {

    // LINES
    HGF2DLiteLine       Line1(HGF2DPosition(1.1, 0.1), HGF2DPosition(-10.1, 10.1));
    HGF2DLiteLine       VerticalLine1C(HGF2DPosition(0.1, 0.1), HGF2DPosition(0.1, 10.1 + MYEPSILON));
    HGF2DLiteLine       Line1P(HGF2DPosition(0.2, 0.1), HGF2DPosition(10.2, 10.1));
    HGF2DLiteLine       Line1NP(HGF2DPosition(0.2, 0.1), HGF2DPosition(10.2, 10.1+MYEPSILON));
    HGF2DLiteLine       Line1CNP(HGF2DPosition(0.1+MYEPSILON, 0.1), HGF2DPosition(10.1-MYEPSILON, 10.1));
    HGF2DLiteLine       Line1CONNECTED(HGF2DPosition(0.21, 0.1), HGF2DPosition(10.1, 10.1));
    HGF2DLiteLine       HorizontalLine1(HGF2DPosition(0.1, 0.1), HGF2DPosition(10.1, 0.1));
    HGF2DLiteLine       InvalidLine1(HGF2DPosition(0.1, 0.1), HGF2DPosition(10.1, 0.1));

    HGF2DPosition   DumPoint;

    // Test with vertical segment
    ASSERT_EQ(HGF2DSegment::CROSS_FOUND, VerticalSegment1A.IntersectLine(Line1, &DumPoint));
    ASSERT_DOUBLE_EQ(0.1, DumPoint.GetX());

    // Test with inverted vertical segment
    ASSERT_EQ(HGF2DSegment::CROSS_FOUND, VerticalSegment2A.IntersectLine(Line1, &DumPoint));
    ASSERT_DOUBLE_EQ(0.1, DumPoint.GetX());

    // Test with close vertical segments
    ASSERT_EQ(HGF2DSegment::PARALLEL, VerticalSegment1A.IntersectLine(VerticalLine1C, &DumPoint));

    // Test with horizontal segment
    ASSERT_EQ(HGF2DSegment::CROSS_FOUND, HorizontalSegment1A.IntersectLine(Line1, &DumPoint));
    ASSERT_DOUBLE_EQ(0.1, DumPoint.GetY()); 

    // Test with horizontal line
    ASSERT_EQ(HGF2DSegment::PARALLEL, HorizontalSegment1A.IntersectLine(HorizontalLine1, &DumPoint));

    // Test with inverted horizontal segment
    ASSERT_EQ(HGF2DSegment::CROSS_FOUND, HorizontalSegment2A.IntersectLine(Line1, &DumPoint));
    ASSERT_DOUBLE_EQ(0.1, DumPoint.GetY()); 

    // Test with parallel segment
    ASSERT_EQ(HGF2DSegment::PARALLEL, MiscSegment1A.IntersectLine(Line1P, &DumPoint));

    // Test with near parallel segments
    ASSERT_NE(HGF2DSegment::PARALLEL, MiscSegment1A.IntersectLine(Line1NP, &DumPoint));

    // Tests with close near parallel segment
    ASSERT_NE(HGF2DSegment::PARALLEL, MiscSegment1A.IntersectLine(Line1CNP, &DumPoint));

    // Tests with connected segment
    ASSERT_EQ(HGF2DSegment::NO_CROSS, MiscSegment1A.IntersectLine(Line1CONNECTED, &DumPoint));

    // Tests with vertical EPSILON sized segment
    ASSERT_EQ(HGF2DSegment::NO_CROSS, VerticalSegment3A.IntersectLine(Line1, &DumPoint));

    // Tests with horizontal EPSILON sized segment
    ASSERT_EQ(HGF2DSegment::NO_CROSS, HorizontalSegment3A.IntersectLine(Line1, &DumPoint));

    // Tests with miscalenious EPSILON size segment
    ASSERT_EQ(HGF2DSegment::NO_CROSS, MiscSegment3B.IntersectLine(Line1, &DumPoint));

    // Test with very large segment
    ASSERT_EQ(HGF2DSegment::CROSS_FOUND, LargeSegment1A.IntersectLine(Line1, &DumPoint));

    // Test with segments way into positive regions
    ASSERT_EQ(HGF2DSegment::NO_CROSS, PositiveSegment1A.IntersectLine(Line1, &DumPoint));

    // Test with segments way into negative regions
    ASSERT_EQ(HGF2DSegment::NO_CROSS, NegativeSegment1A.IntersectLine(Line1, &DumPoint));

    // Test with a NULL segment
    ASSERT_EQ(HGF2DSegment::NO_CROSS, NullSegment1A.IntersectLine(Line1, &DumPoint));

    }

//==================================================================================
// Segment intersection test
// IntersectSegment(const HGF2DSegment& pi_rSegment,HGF2DPosition* po_pPoint)const;
// IntersectSegment( const HGF2DSegment& pi_rSegment, HGF2DPosition* po_pPoint ) const
//==================================================================================
TEST_F (HGF2DSegmentTester, IntersectSegmentTest)
    {

    HGF2DPosition   DumPoint;

    // Test with extent disjoint segments
    ASSERT_EQ(HGF2DSegment::NO_CROSS, MiscSegment1A.IntersectSegment(DisjointSegment1A, &DumPoint));

    // Test with disjoint but touching by a side segments
    ASSERT_EQ(HGF2DSegment::PARALLEL, MiscSegment1A.IntersectSegment(ContiguousExtentSegment1A, &DumPoint));

    // Test with disjoint but touching by a tip segments but not linked
    ASSERT_EQ(HGF2DSegment::NO_CROSS, MiscSegment1A.IntersectSegment(FlirtingExtentSegment1A, &DumPoint));

    // Test with disjoint but touching by a tip segments linked
    ASSERT_EQ(HGF2DSegment::NO_CROSS, MiscSegment1A.IntersectSegment(FlirtingExtentLinkedSegment1A, &DumPoint));

    // Test with vertical segment
    ASSERT_EQ(HGF2DSegment::NO_CROSS, VerticalSegment1A.IntersectSegment(MiscSegment1A, &DumPoint));

    // Test with inverted vertical segment
    ASSERT_EQ(HGF2DSegment::NO_CROSS, VerticalSegment2A.IntersectSegment(MiscSegment1A, &DumPoint));

    // Test with close vertical segments
    ASSERT_EQ(HGF2DSegment::PARALLEL, VerticalSegment1A.IntersectSegment(VerticalSegment4A, &DumPoint));

    // Test with horizontal segment
    ASSERT_EQ(HGF2DSegment::NO_CROSS, HorizontalSegment1A.IntersectSegment(MiscSegment1A, &DumPoint));

    // Test with inverted horizontal segment
    ASSERT_EQ(HGF2DSegment::NO_CROSS, HorizontalSegment2A.IntersectSegment(MiscSegment1A, &DumPoint));

    // Test with parallel segments
    ASSERT_EQ(HGF2DSegment::PARALLEL, MiscSegment1A.IntersectSegment(ParallelSegment1A, &DumPoint));

    // Test with near parallel segments
    ASSERT_NE(HGF2DSegment::PARALLEL, MiscSegment1A.IntersectSegment(NearParallelSegment1A, &DumPoint));

    // Tests with close near parallel segments
    ASSERT_NE(HGF2DSegment::PARALLEL, MiscSegment1A.IntersectSegment(CloseNearParallelSegment1A, &DumPoint));

    // Tests with connected segments
    // At start point...
    ASSERT_EQ(HGF2DSegment::NO_CROSS, MiscSegment1A.IntersectSegment(ConnectedSegment1B, &DumPoint));
    ASSERT_EQ(HGF2DSegment::NO_CROSS, MiscSegment1A.IntersectSegment(ConnectingSegment1B, &DumPoint));

    // At end point ...
    ASSERT_EQ(HGF2DSegment::NO_CROSS, MiscSegment1A.IntersectSegment(ConnectedSegment1AA, &DumPoint));
    ASSERT_EQ(HGF2DSegment::NO_CROSS, MiscSegment1A.IntersectSegment(ConnectingSegment1AA, &DumPoint));

    // Tests with linked segments
    ASSERT_EQ(HGF2DSegment::NO_CROSS, MiscSegment1A.IntersectSegment(LinkedSegment1B, &DumPoint));
    ASSERT_EQ(HGF2DSegment::NO_CROSS, MiscSegment1A.IntersectSegment(LinkedSegment1B, &DumPoint));

    // Tests with vertical EPSILON sized segment
    ASSERT_EQ(HGF2DSegment::NO_CROSS, VerticalSegment3A.IntersectSegment(MiscSegment1A, &DumPoint));

    // Tests with horizontal EPSILON sized segment
    ASSERT_EQ(HGF2DSegment::NO_CROSS, HorizontalSegment3A.IntersectSegment(MiscSegment1A, &DumPoint));

    // Tests with miscalenious EPSILON size segment
    ASSERT_EQ(HGF2DSegment::NO_CROSS, MiscSegment3B.IntersectSegment(MiscSegment1A, &DumPoint));

    // Test with very large segment
    ASSERT_EQ(HGF2DSegment::NO_CROSS, LargeSegment1A.IntersectSegment(MiscSegment1A, &DumPoint));

    // Test with segments way into positive regions
    ASSERT_EQ(HGF2DSegment::NO_CROSS, PositiveSegment1A.IntersectSegment(MiscSegment1A, &DumPoint));

    // Test with segments way into negative regions
    ASSERT_EQ(HGF2DSegment::NO_CROSS, NegativeSegment1A.IntersectSegment(MiscSegment1A, &DumPoint));

    // Test with a NULL segment
    ASSERT_EQ(HGF2DSegment::NO_CROSS, NullSegment1A.IntersectSegment(MiscSegment1A, &DumPoint));

    }



//==================================================================================
// Parallelitude test
// IsParallelTo(const HGF2DSegment& pi_rSegment) const;
// IsParallelTo(const HGF2DLine& pi_rLine) const;
//==================================================================================
TEST_F (HGF2DSegmentTester, ParallelitudeTest)
    {

    // LINES
    HGF2DLiteLine       Line1(HGF2DPosition(1.1, 0.1), HGF2DPosition(-10.1, 10.1));
    HGF2DLiteLine       VerticalLine1C(HGF2DPosition(0.1, 0.1), HGF2DPosition(0.1, 10.1 + MYEPSILON));
    HGF2DLiteLine       Line1P(HGF2DPosition(0.2, 0.1), HGF2DPosition(10.2, 10.1));
    HGF2DLiteLine       Line1NP(HGF2DPosition(0.2, 0.1), HGF2DPosition(10.2, 10.1+MYEPSILON));
    HGF2DLiteLine       Line1CNP(HGF2DPosition(0.1+MYEPSILON, 0.1), HGF2DPosition(10.1-MYEPSILON, 10.1));
    HGF2DLiteLine       Line1CONNECTED(HGF2DPosition(0.21, 0.1), HGF2DPosition(10.1, 10.1));
    HGF2DLiteLine       HorizontalLine1(HGF2DPosition(0.1, 0.1), HGF2DPosition(10.1, 0.1));

    // Test with segments

    // Test with extent disjoint segments
    ASSERT_FALSE(MiscSegment1A.IsParallelTo(DisjointSegment1A));

    // Test with vertical segment
    ASSERT_TRUE(VerticalSegment1A.IsParallelTo(VerticalSegment5A));
    ASSERT_FALSE(VerticalSegment1A.IsParallelTo(MiscSegment1A));

    // Test with inverted vertical segment
    ASSERT_TRUE(VerticalSegment2A.IsParallelTo(VerticalSegment5A));
    ASSERT_FALSE(VerticalSegment2A.IsParallelTo(MiscSegment1A));

    // Test with close vertical segments
    ASSERT_TRUE(VerticalSegment1A.IsParallelTo(CloseVerticalSegment1A));

    // Test with horizontal segment
    ASSERT_TRUE(HorizontalSegment1A.IsParallelTo(HorizontalSegment5A));
    ASSERT_FALSE(HorizontalSegment1A.IsParallelTo(MiscSegment1A));

    // Test with inverted horizontal segment
    ASSERT_TRUE(HorizontalSegment2A.IsParallelTo(HorizontalSegment5A));
    ASSERT_FALSE(HorizontalSegment2A.IsParallelTo(MiscSegment1A));

    // Test with near parallel segments
    ASSERT_TRUE(MiscSegment1A.IsParallelTo(NearParallelSegment1A));

    // Tests with close near parallel segments
    ASSERT_TRUE(MiscSegment1A.IsParallelTo(CloseNearParallelSegment1A));

    // Tests with connected segments
    ASSERT_FALSE(MiscSegment1A.IsParallelTo(ConnectedSegment1B));
    ASSERT_FALSE(MiscSegment1A.IsParallelTo(ConnectingSegment1B));

    // Tests with linked segments
    ASSERT_FALSE(MiscSegment1A.IsParallelTo(LinkedSegment1B));
    ASSERT_TRUE(MiscSegment1A.IsParallelTo(LinkedParallelSegment1A));

    // Tests with vertical EPSILON sized segment
    ASSERT_TRUE(VerticalSegment2A.IsParallelTo(VerticalSegment5A));
    ASSERT_FALSE(VerticalSegment2A.IsParallelTo(MiscSegment1A));

    // Tests with horizontal EPSILON sized segment
    ASSERT_TRUE(HorizontalSegment2A.IsParallelTo(HorizontalSegment5A));
    ASSERT_FALSE(HorizontalSegment2A.IsParallelTo(MiscSegment1A));

    // Tests with miscalenious EPSILON size segment
    ASSERT_TRUE(MiscSegment3B.IsParallelTo(MiscSegment3B));
    ASSERT_FALSE(MiscSegment3B.IsParallelTo(MiscSegment1A));

    // Test with very large segment
    ASSERT_TRUE(LargeSegment1A.IsParallelTo(ParallelLargeSegment1A));
    ASSERT_FALSE(LargeSegment1A.IsParallelTo(MiscSegment1A));

    // Test with segments way into positive regions
    ASSERT_TRUE(PositiveSegment1A.IsParallelTo(ParallelPositiveSegment1A));
    ASSERT_FALSE(PositiveSegment1A.IsParallelTo(MiscSegment1A));

    // Test with segments way into negative regions
    ASSERT_TRUE(NegativeSegment1A.IsParallelTo(ParallelNegativeSegment1A));
    ASSERT_FALSE(NegativeSegment1A.IsParallelTo(MiscSegment1A));

    // Test with a NULL segment
    ASSERT_TRUE(NullSegment1A.IsParallelTo(NullSegment2A));
    ASSERT_FALSE(NullSegment1A.IsParallelTo(MiscSegment1A));

    // Test with a line

    // Test with vertical segment
    ASSERT_FALSE(VerticalSegment1A.IsParallelTo(Line1));

    // Test with inverted vertical segment
    ASSERT_FALSE(VerticalSegment2A.IsParallelTo(Line1));

    // Test with close vertical segments
    ASSERT_FALSE(VerticalSegment1A.IsParallelTo(Line1P));

    // Test with horizontal segment
    ASSERT_FALSE(HorizontalSegment1A.IsParallelTo(Line1));

    // Test with inverted horizontal segment
    ASSERT_FALSE(HorizontalSegment2A.IsParallelTo(Line1));

    // Test with parallel segment
    ASSERT_TRUE(MiscSegment1A.IsParallelTo(Line1P));

    // Test with near parallel segment
    ASSERT_TRUE(MiscSegment1A.IsParallelTo(Line1NP));

    // Tests with close near parallel segments
    ASSERT_TRUE(MiscSegment1A.IsParallelTo(Line1CNP));

    // Tests with connected segment
    ASSERT_FALSE(MiscSegment1A.IsParallelTo(Line1CONNECTED));

    // Tests with vertical EPSILON sized segment
    ASSERT_FALSE(VerticalSegment3A.IsParallelTo(Line1));

    // Tests with horizontal EPSILON sized segment
    ASSERT_FALSE(HorizontalSegment3A.IsParallelTo(Line1));

    // Tests with miscalenious EPSILON size segment
    ASSERT_FALSE(MiscSegment3B.IsParallelTo(Line1));

    // Test with very large segment
    ASSERT_FALSE(LargeSegment1A.IsParallelTo(Line1));

    // Test with segments way into positive regions
    ASSERT_FALSE(PositiveSegment1A.IsParallelTo(Line1));

    // Test with segments way into negative regions
    ASSERT_FALSE(NegativeSegment1A.IsParallelTo(Line1));

    // Test with a NULL segment
    ASSERT_FALSE(NullSegment1A.IsParallelTo(Line1));

    }

//==================================================================================
// Type extraction test
// GetType() const;
//==================================================================================
TEST_F (HGF2DSegmentTester, GetTypeTest)
    {

    // Basic test
    ASSERT_EQ(HGF2DSegment::CLASS_ID, MiscSegment1.GetBasicLinearType());
    ASSERT_EQ(HGF2DSegment::CLASS_ID, VerticalSegment1.GetBasicLinearType());

    }

//==================================================================================
// Length calculation test
// CalculateLength() const;
//==================================================================================
TEST_F (HGF2DSegmentTester, CalculateLengthTest)
    {

    // Test with vertical segment
    ASSERT_DOUBLE_EQ(10.0000000000000, VerticalSegment1.CalculateLength());

    // Test with inverted vertical segment
    ASSERT_DOUBLE_EQ(10.0000000000000, VerticalSegment2.CalculateLength());

    // Test with horizontal segment
    ASSERT_DOUBLE_EQ(10.00000000000000, HorizontalSegment1.CalculateLength());

    // Test with inverted horizontal segment
    ASSERT_DOUBLE_EQ(10.00000000000000, HorizontalSegment2.CalculateLength());

    // Tests with miscalenious segment
    ASSERT_DOUBLE_EQ(14.142135623730951, MiscSegment1.CalculateLength());

    // Test with very large segment
    ASSERT_DOUBLE_EQ(4.12310562561766099E124, LargeSegment1.CalculateLength());

    // Test with segments way into positive regions
    ASSERT_DOUBLE_EQ(2.2360679774997898E124, PositiveSegment1.CalculateLength());

    // Test with segments way into negative regions
    ASSERT_DOUBLE_EQ(2.23606797749978976E124, NegativeSegment1.CalculateLength());

    // Test with a NULL segment
    ASSERT_NEAR(0.0, NullSegment1.CalculateLength(), MYEPSILON);
    
    }

//==================================================================================
// Relative point calculation test
// CalculateRelativePoint(double pi_RelativePos) const;
//==================================================================================
TEST_F (HGF2DSegmentTester, CalculateRelativePointTest)
    {

    // Test with vertical segment
    ASSERT_DOUBLE_EQ(0.10, VerticalSegment1.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(0.10, VerticalSegment1.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(0.10, VerticalSegment1.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(1.10, VerticalSegment1.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(0.10, VerticalSegment1.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(5.10, VerticalSegment1.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(0.10, VerticalSegment1.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(7.10, VerticalSegment1.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(0.10, VerticalSegment1.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(10.1, VerticalSegment1.CalculateRelativePoint(1.0).GetY());

    // Test with inverted vertical segment
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment2.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(10.10000000000000000, VerticalSegment2.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment2.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(9.100000000000000000, VerticalSegment2.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment2.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(5.100000000000000000, VerticalSegment2.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment2.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(3.100000000000000000, VerticalSegment2.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment2.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(0.099999999999999645, VerticalSegment2.CalculateRelativePoint(1.0).GetY());

    // Test with horizontal segment
    ASSERT_DOUBLE_EQ(0.10, HorizontalSegment1.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalSegment1.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(1.10, HorizontalSegment1.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalSegment1.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(5.10, HorizontalSegment1.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalSegment1.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(7.10, HorizontalSegment1.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalSegment1.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(10.1, HorizontalSegment1.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalSegment1.CalculateRelativePoint(1.0).GetY());

    // Test with inverted horizontal segment
    ASSERT_DOUBLE_EQ(10.1000000000000000, HorizontalSegment2.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, HorizontalSegment2.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(9.10000000000000000, HorizontalSegment2.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, HorizontalSegment2.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(5.10000000000000000, HorizontalSegment2.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, HorizontalSegment2.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(3.10000000000000000, HorizontalSegment2.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, HorizontalSegment2.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(0.099999999999999645, HorizontalSegment2.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalSegment2.CalculateRelativePoint(1.0).GetY());

    // Tests with vertical EPSILON sized segment
    ASSERT_DOUBLE_EQ(0.10000000, VerticalSegment3.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(0.10000000, VerticalSegment3.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(0.10000000, VerticalSegment3.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(0.10000001, VerticalSegment3.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(0.10000000, VerticalSegment3.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(0.10000005, VerticalSegment3.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(0.10000000, VerticalSegment3.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(0.10000007, VerticalSegment3.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(0.10000000, VerticalSegment3.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(0.10000010, VerticalSegment3.CalculateRelativePoint(1.0).GetY());

    // Tests with horizontal EPSILON sized segment
    ASSERT_DOUBLE_EQ(0.10000000, HorizontalSegment3.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(0.10000000, HorizontalSegment3.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(0.10000001, HorizontalSegment3.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(0.10000000, HorizontalSegment3.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(0.10000005, HorizontalSegment3.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(0.10000000, HorizontalSegment3.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(0.10000007, HorizontalSegment3.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(0.10000000, HorizontalSegment3.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(0.10000010, HorizontalSegment3.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(0.10000000, HorizontalSegment3.CalculateRelativePoint(1.0).GetY());

    // Tests with miscalenious EPSILON size segment
    ASSERT_DOUBLE_EQ(0.10000000000000000, MiscSegment3.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, MiscSegment3.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(0.10000000216439614, MiscSegment3.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(0.10000000976296007, MiscSegment3.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(0.10000001082198071, MiscSegment3.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(0.10000004881480036, MiscSegment3.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(0.10000001515077298, MiscSegment3.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(0.10000006834072050, MiscSegment3.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, MiscSegment3.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, MiscSegment3.CalculateRelativePoint(1.0).GetY());

    // Test with very large segment
    ASSERT_DOUBLE_EQ(-1.00E123, LargeSegment1.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, LargeSegment1.CalculateRelativePoint(0.0).GetY());
    ASSERT_NEAR(0.0, LargeSegment1.CalculateRelativePoint(0.1).GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(-17.0E123, LargeSegment1.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(4.000E123, LargeSegment1.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(-1.00E123, LargeSegment1.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(6.000E123, LargeSegment1.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(7.000E123, LargeSegment1.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(9.000E123, LargeSegment1.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(19.00E123, LargeSegment1.CalculateRelativePoint(1.0).GetY());

    // Test with segments way into positive regions
    ASSERT_DOUBLE_EQ(1.00E123, PositiveSegment1.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(21.0E123, PositiveSegment1.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(2.00E123, PositiveSegment1.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(23.0E123, PositiveSegment1.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(6.00E123, PositiveSegment1.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(31.0E123, PositiveSegment1.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(8.00E123, PositiveSegment1.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(35.0E123, PositiveSegment1.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(11.0E123, PositiveSegment1.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(41.0E123, PositiveSegment1.CalculateRelativePoint(1.0).GetY());

    // Test with segments way into negative regions
    ASSERT_DOUBLE_EQ(-1.00E123, NegativeSegment1.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, NegativeSegment1.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(-2.00E123, NegativeSegment1.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(-23.0E123, NegativeSegment1.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(-6.00E123, NegativeSegment1.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(-31.0E123, NegativeSegment1.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(-8.00E123, NegativeSegment1.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(-35.0E123, NegativeSegment1.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, NegativeSegment1.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, NegativeSegment1.CalculateRelativePoint(1.0).GetY());

    // Test with a NULL segment
    ASSERT_NEAR(0.0, NullSegment1.CalculateRelativePoint(0.0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, NullSegment1.CalculateRelativePoint(0.0).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, NullSegment1.CalculateRelativePoint(0.1).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, NullSegment1.CalculateRelativePoint(0.1).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, NullSegment1.CalculateRelativePoint(0.5).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, NullSegment1.CalculateRelativePoint(0.5).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, NullSegment1.CalculateRelativePoint(0.7).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, NullSegment1.CalculateRelativePoint(0.7).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, NullSegment1.CalculateRelativePoint(1.0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, NullSegment1.CalculateRelativePoint(1.0).GetY(), MYEPSILON);

    }

//==================================================================================
// Relative position calculation test
// CalculateRelativePosition(const HGF2DPosition& pi_rPointOnLinear) const;
//==================================================================================
TEST_F (HGF2DSegmentTester, CalculateRelativePositionTest)
    {

    // Test with vertical segment
    ASSERT_NEAR(0.0, VerticalSegment1.CalculateRelativePosition(VerticalPoint0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1, VerticalSegment1.CalculateRelativePosition(VerticalPoint0d1));
    ASSERT_DOUBLE_EQ(0.5, VerticalSegment1.CalculateRelativePosition(VerticalPoint0d5));
    ASSERT_DOUBLE_EQ(1.0, VerticalSegment1.CalculateRelativePosition(VerticalPoint1d0));

    // Test with inverted vertical segment
    ASSERT_DOUBLE_EQ(1.0, VerticalSegment2.CalculateRelativePosition(VerticalPoint0d0));
    ASSERT_DOUBLE_EQ(0.9, VerticalSegment2.CalculateRelativePosition(VerticalPoint0d1));
    ASSERT_DOUBLE_EQ(0.5, VerticalSegment2.CalculateRelativePosition(VerticalPoint0d5));
    ASSERT_NEAR(0.0, VerticalSegment2.CalculateRelativePosition(VerticalPoint1d0), MYEPSILON);

    // Test with horizontal segment
    ASSERT_NEAR(0.0, HorizontalSegment1.CalculateRelativePosition(HorizontalPoint0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1, HorizontalSegment1.CalculateRelativePosition(HorizontalPoint0d1));
    ASSERT_DOUBLE_EQ(0.5, HorizontalSegment1.CalculateRelativePosition(HorizontalPoint0d5));
    ASSERT_DOUBLE_EQ(1.0, HorizontalSegment1.CalculateRelativePosition(HorizontalPoint1d0));

    // Test with inverted horizontal segment
    ASSERT_DOUBLE_EQ(1.0, HorizontalSegment2.CalculateRelativePosition(HorizontalPoint0d0));
    ASSERT_DOUBLE_EQ(0.9, HorizontalSegment2.CalculateRelativePosition(HorizontalPoint0d1));
    ASSERT_DOUBLE_EQ(0.5, HorizontalSegment2.CalculateRelativePosition(HorizontalPoint0d5));
    ASSERT_NEAR(0.0, HorizontalSegment2.CalculateRelativePosition(HorizontalPoint1d0), MYEPSILON);

    // Tests with vertical EPSILON sized segment
    ASSERT_NEAR(0.0, VerticalSegment3.CalculateRelativePosition(Vertical3Point0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.099999999944488854, VerticalSegment3.CalculateRelativePosition(Vertical3Point0d1));
    ASSERT_DOUBLE_EQ(0.500000000000000000, VerticalSegment3.CalculateRelativePosition(Vertical3Point0d5));
    ASSERT_DOUBLE_EQ(1.000000000000000000, VerticalSegment3.CalculateRelativePosition(Vertical3Point1d0));

    // Tests with horizontal EPSILON sized segment
    ASSERT_NEAR(0.0, HorizontalSegment3.CalculateRelativePosition(Horizontal3Point0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.099999999944488854, HorizontalSegment3.CalculateRelativePosition(Horizontal3Point0d1));
    ASSERT_DOUBLE_EQ(0.500000000000000000, HorizontalSegment3.CalculateRelativePosition(Horizontal3Point0d5));
    ASSERT_DOUBLE_EQ(1.000000000000000000, HorizontalSegment3.CalculateRelativePosition(Horizontal3Point1d0));

    // Tests with miscalenious EPSILON size segment
    ASSERT_NEAR(0.0, MiscSegment3.CalculateRelativePosition(Misc3Point0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.099999999943141071, MiscSegment3.CalculateRelativePosition(Misc3Point0d1));
    ASSERT_DOUBLE_EQ(0.500000000000000000, MiscSegment3.CalculateRelativePosition(Misc3Point0d5));
    ASSERT_DOUBLE_EQ(1.000000000000000000, MiscSegment3.CalculateRelativePosition(Misc3Point1d0));

    // Test with very large segment
    ASSERT_NEAR(0.0, LargeSegment1.CalculateRelativePosition(LargePoint0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1, LargeSegment1.CalculateRelativePosition(LargePoint0d1));
    ASSERT_DOUBLE_EQ(0.5, LargeSegment1.CalculateRelativePosition(LargePoint0d5));
    ASSERT_DOUBLE_EQ(1.0, LargeSegment1.CalculateRelativePosition(LargePoint1d0));

    // Test with segments way into positive regions
    ASSERT_NEAR(0.0, PositiveSegment1.CalculateRelativePosition(PositivePoint0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1, PositiveSegment1.CalculateRelativePosition(PositivePoint0d1));
    ASSERT_DOUBLE_EQ(0.5, PositiveSegment1.CalculateRelativePosition(PositivePoint0d5));
    ASSERT_DOUBLE_EQ(1.0, PositiveSegment1.CalculateRelativePosition(PositivePoint1d0));

    // Test with segments way into negative regions
    ASSERT_NEAR(0.0, NegativeSegment1.CalculateRelativePosition(NegativePoint0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1, NegativeSegment1.CalculateRelativePosition(NegativePoint0d1));
    ASSERT_DOUBLE_EQ(0.5, NegativeSegment1.CalculateRelativePosition(NegativePoint0d5));
    ASSERT_DOUBLE_EQ(1.0, NegativeSegment1.CalculateRelativePosition(NegativePoint1d0));

    }

//==================================================================================
// RayArea Calculation test
// CalculateRayArea(const HGF2DPosition& pi_rPoint) const;
//==================================================================================
TEST_F (HGF2DSegmentTester, CalculateRayArea) 
    {

    // Test with vertical segment
    ASSERT_DOUBLE_EQ(0.505, VerticalSegment1A.CalculateRayArea(HGF2DPosition(0.0, 0.0)));

    // Test with inverted vertical segment
    ASSERT_DOUBLE_EQ(0.005, VerticalSegment2A.CalculateRayArea(HGF2DPosition(0.0, 0.0)));

    // Test with horizontal segment
    ASSERT_DOUBLE_EQ(0.005, HorizontalSegment1A.CalculateRayArea(HGF2DPosition(0.0, 0.0)));

    // Test with inverted horizontal segment
    ASSERT_DOUBLE_EQ(0.505, HorizontalSegment2A.CalculateRayArea(HGF2DPosition(0.0, 0.0)));

    // Tests with vertical EPSILON sized segment
    ASSERT_DOUBLE_EQ(0.0050000050000000009, VerticalSegment3A.CalculateRayArea(HGF2DPosition(0.0, 0.0)));

    // Tests with horizontal EPSILON sized segment
    ASSERT_DOUBLE_EQ(0.005, HorizontalSegment3A.CalculateRayArea(HGF2DPosition(0.0, 0.0)));

    // Tests with miscalenious EPSILON size segment
    ASSERT_DOUBLE_EQ(0.0050000048814800363, MiscSegment3B.CalculateRayArea(HGF2DPosition(0.0, 0.0)));

    // Test with very large segment
    ASSERT_DOUBLE_EQ(-9.4999999999999990e+246, LargeSegment1A.CalculateRayArea(HGF2DPosition(0.0, 0.0)));

    // Test with segments way into positive regions
    ASSERT_DOUBLE_EQ(2.0500000000000001e+247, PositiveSegment1A.CalculateRayArea(HGF2DPosition(0.0, 0.0)));
     
    // Test with segments way into negative regions
    ASSERT_DOUBLE_EQ(2.0500000000000001e+247, NegativeSegment1A.CalculateRayArea(HGF2DPosition(0.0, 0.0)));
    
    }

//==================================================================================
// Shortening tests
// Shorten(double pi_StartRelativePos, double pi_EndRelativePos);
// Shorten(const HGF2DPosition& pi_rNewStartPoint,
//         const HGF2DPosition& pi_rNewEndPoint);
// ShortenTo(const HGF2DPosition& pi_rNewEndPoint);
// ShortenTo(double pi_EndRelativePosition);
// ShortenFrom(const HGF2DPosition& pi_rNewStartPoint);
// ShortenFrom(double pi_StartRelativePosition);
//==================================================================================
TEST_F (HGF2DSegmentTester,ShorteningTest)
    {

    // Test with vertical segment
    HGF2DSegment    Segment1(VerticalSegment1A);

    Segment1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(2.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(8.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1A;
    Segment1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.1000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(5.1000009999999989, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1A;
    Segment1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(8.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1A;
    Segment1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(2.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1A;
    Segment1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1A;
    Segment1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(8.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1A;
    Segment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.099999, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1A;
    Segment1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1A;
    Segment1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1A;
    Segment1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(2.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1A;
    Segment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.099999, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.100000, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1A;
    Segment1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1A;
    Segment1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1A;
    Segment1.ShortenFrom(VerticalMidPoint1A);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1A;
    Segment1.ShortenFrom(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1A;
    Segment1.ShortenFrom(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1A;
    Segment1.ShortenTo(VerticalMidPoint1A);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(5.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1A;
    Segment1.ShortenTo(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1A;
    Segment1.ShortenTo(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1A;
    Segment1.Shorten(Segment1.GetStartPoint(), VerticalMidPoint1A);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(5.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1A;
    Segment1.Shorten(VerticalMidPoint1A, Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1A;
    Segment1.Shorten(Segment1.GetStartPoint(), Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    // Test with inverted vertical segment
    Segment1 = VerticalSegment2A;
    Segment1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(8.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(2.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2A;
    Segment1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.1000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(5.0999990000000004, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2A;
    Segment1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(2.10, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2A;
    Segment1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(8.100000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.099999999999999645, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2A;
    Segment1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.10000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.099999999999999645, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2A;
    Segment1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(2.10, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2A;
    Segment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.100000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000009999999989, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2A;
    Segment1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.10000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.099999999999999645, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2A;
    Segment1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2A;
    Segment1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(8.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2A;
    Segment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000009999999989, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2A;
    Segment1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.099999999999999645, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2A;
    Segment1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2A;
    Segment1.ShortenFrom(VerticalMidPoint1A);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2A;
    Segment1.ShortenFrom(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2A;
    Segment1.ShortenFrom(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2A;
    Segment1.ShortenTo(VerticalMidPoint1A);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(5.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2A;
    Segment1.ShortenTo(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2A;
    Segment1.ShortenTo(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2A;
    Segment1.Shorten(Segment1.GetStartPoint(), VerticalMidPoint1A);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(5.10, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2A;
    Segment1.Shorten(VerticalMidPoint1A, Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2A;
    Segment1.Shorten(Segment1.GetStartPoint(), Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    // Test with horizontal segment
    Segment1 = HorizontalSegment1A;
    Segment1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(2.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(8.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1A;
    Segment1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_DOUBLE_EQ(5.1000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.1000009999999989, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1A;
    Segment1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(8.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1A;
    Segment1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(2.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1A;
    Segment1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1A;
    Segment1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(8.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1A;
    Segment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.099999, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1A;
    Segment1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1A;
    Segment1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1A;
    Segment1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(2.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1A;
    Segment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(10.099999, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.100000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1A;
    Segment1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1A;
    Segment1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1A;
    Segment1.ShortenFrom(HorizontalMidPoint1A);
    ASSERT_DOUBLE_EQ(5.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1A;
    Segment1.ShortenFrom(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1A;
    Segment1.ShortenFrom(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1A;
    Segment1.ShortenTo(HorizontalMidPoint1A);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1A;
    Segment1.ShortenTo(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1A;
    Segment1.ShortenTo(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1A;
    Segment1.Shorten(Segment1.GetStartPoint(), HorizontalMidPoint1A);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1A;
    Segment1.Shorten(HorizontalMidPoint1A, Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(5.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1A;
    Segment1.Shorten(Segment1.GetStartPoint(), Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    // Test with inverted horizontal segment
    Segment1 = HorizontalSegment2A;
    Segment1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(8.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(2.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2A;
    Segment1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_DOUBLE_EQ(5.100000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.099999, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2A;
    Segment1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(2.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2A;
    Segment1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(8.100000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.099999999999999645, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2A;
    Segment1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(10.10000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.099999999999999645, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2A;
    Segment1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(2.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2A;
    Segment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(10.100000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000009999999989, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2A;
    Segment1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(10.10000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.099999999999999645, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2A;
    Segment1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2A;
    Segment1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(8.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2A;
    Segment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1000009999999989, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2A;
    Segment1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(0.099999999999999645, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2A;
    Segment1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2A;
    Segment1.ShortenFrom(HorizontalMidPoint1A);
    ASSERT_DOUBLE_EQ(5.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2A;
    Segment1.ShortenFrom(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2A;
    Segment1.ShortenFrom(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2A;
    Segment1.ShortenTo(HorizontalMidPoint1A);
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2A;
    Segment1.ShortenTo(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2A;
    Segment1.ShortenTo(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2A;
    Segment1.Shorten(Segment1.GetStartPoint(), HorizontalMidPoint1A);
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2A;
    Segment1.Shorten(HorizontalMidPoint1A, Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(5.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2A;
    Segment1.Shorten(Segment1.GetStartPoint(), Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    // Tests with vertical EPSILON sized segment
    Segment1 = VerticalSegment3A;
    Segment1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000002, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000008, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3A;
    Segment1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000005000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000005000001001, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3A;
    Segment1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000008, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3A;
    Segment1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000002, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000010, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3A;
    Segment1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3A;
    Segment1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000008, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3A;
    Segment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009999999, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3A;
    Segment1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3A;
    Segment1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3A;
    Segment1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000002, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000010, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3A;
    Segment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009999999, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000010000000, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3A;
    Segment1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3A;
    Segment1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3A;
    Segment1.ShortenFrom(VerticalMidPoint3A);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000005, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000010, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3A;
    Segment1.ShortenFrom(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3A;
    Segment1.ShortenFrom(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3A;
    Segment1.ShortenTo(VerticalMidPoint3A);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000005, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3A;
    Segment1.ShortenTo(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());
    
    Segment1 = VerticalSegment3A;
    Segment1.ShortenTo(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3A;
    Segment1.Shorten(Segment1.GetStartPoint(), VerticalMidPoint3A);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000005, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3A;
    Segment1.Shorten(VerticalMidPoint3A, Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000005, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000010, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3A;
    Segment1.Shorten(Segment1.GetStartPoint(), Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetY());

    // Tests with horizontal EPSILON sized segment
    Segment1 = HorizontalSegment3A;
    Segment1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(0.10000002, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000008, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3A;
    Segment1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000005000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000005000001, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3A;
    Segment1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000008, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3A;
    Segment1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(0.10000002, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000010, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3A;
    Segment1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000010, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3A;
    Segment1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000008, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3A;
    Segment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000009999999, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3A;
    Segment1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3A;
    Segment1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3A;
    Segment1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(0.10000002, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000010, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetY());
    
    Segment1 = HorizontalSegment3A;
    Segment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000009999999, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000010000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3A;
    Segment1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3A;
    Segment1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3A;
    Segment1.ShortenFrom(HorizontalMidPoint3A);
    ASSERT_DOUBLE_EQ(0.10000005, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000010, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3A;
    Segment1.ShortenFrom(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3A;
    Segment1.ShortenFrom(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3A;
    Segment1.ShortenTo(HorizontalMidPoint3A);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000005, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3A;
    Segment1.ShortenTo(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3A;
    Segment1.ShortenTo(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3A;
    Segment1.Shorten(Segment1.GetStartPoint(), HorizontalMidPoint3A);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000005, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3A;
    Segment1.Shorten(HorizontalMidPoint3A, Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10000005, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000010, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3A;
    Segment1.Shorten(Segment1.GetStartPoint(), Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetY());

    // Tests with miscalenious EPSILON size segment
    Segment1 = MiscSegment3B;
    Segment1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(0.10000000432879229, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000001952592015, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000001731516911, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000007810368057, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3B;
    Segment1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000001082198071, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000004881480036, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000001082198286, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000004881481012, Segment1.GetEndPoint().GetY());
    
    Segment1 = MiscSegment3B;
    Segment1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000001731516911, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000007810368057, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3B;
    Segment1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(0.10000000432879229, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000001952592015, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3B;
    Segment1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3B;
    Segment1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000001731516911, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000007810368057, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3B;
    Segment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164395923, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762959096, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3B;
    Segment1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3B;
    Segment1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3B;
    Segment1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(0.10000000432879229, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000001952592015, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3B;
    Segment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000002164395923, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762959096, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3B;
    Segment1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3B;
    Segment1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3B;
    Segment1.ShortenFrom(MiscMidPoint3A);
    ASSERT_DOUBLE_EQ(0.099999974681068096, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000004311556202, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3B;
    Segment1.ShortenFrom(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3B;
    Segment1.ShortenFrom(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3B;
    Segment1.ShortenTo(MiscMidPoint3A);
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.099999974681068096, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000004311556202, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3B;
    Segment1.ShortenTo(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3B;
    Segment1.ShortenTo(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3B;
    Segment1.Shorten(Segment1.GetStartPoint(), MiscMidPoint3A);
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.099999974681068096, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000043115562020, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3B;
    Segment1.Shorten(MiscMidPoint3A, Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.099999974681068096, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000043115562020, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3B;
    Segment1.Shorten(Segment1.GetStartPoint(), Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    // Test with very large segment
    Segment1 = LargeSegment1A;
    Segment1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(1.000E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-13.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(7.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(11.00E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1A;
    Segment1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(7.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(11.00E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1A;
    Segment1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(1.000E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-13.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.00E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1A;
    Segment1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.00E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1A;
    Segment1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(7.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(11.00E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1A;
    Segment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(-1.000000E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.00000E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(8.9999990E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(1.8999996E124, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1A;
    Segment1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.00E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1A;
    Segment1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1A;
    Segment1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(1.000E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-13.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.00E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1A;
    Segment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(8.9999990E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(1.8999996E124, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.0000000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.000000E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1A;
    Segment1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(9.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(19.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1A;
    Segment1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.00E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1A;
    Segment1.ShortenFrom(LargeMidPoint1A);
    ASSERT_DOUBLE_EQ(4.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-1.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1A;
    Segment1.ShortenFrom(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.00E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1A;
    Segment1.ShortenFrom(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(9.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(19.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1A;
    Segment1.ShortenTo(LargeMidPoint1A);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(4.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1A;
    Segment1.ShortenTo(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1A;
    Segment1.ShortenTo(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.00E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1A;
    Segment1.Shorten(Segment1.GetStartPoint(), LargeMidPoint1A);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(4.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1A;
    Segment1.Shorten(LargeMidPoint1A, Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(4.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-1.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1A;
    Segment1.Shorten(Segment1.GetStartPoint(), Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.00E123, Segment1.GetEndPoint().GetY());

    // Test with segments way into positive regions
    Segment1 = PositiveSegment1A;
    Segment1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(3.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(37.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1A;
    Segment1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(37.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1A;
    Segment1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(3.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1A;
    Segment1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1A;
    Segment1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(37.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1A;
    Segment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0000000E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.000000E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(1.0999999E124, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(4.0999998E124, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1A;
    Segment1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1A;
    Segment1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1A;
    Segment1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(3.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1A;
    Segment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0999999E124, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(4.0999998E124, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.000000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.000000E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1A;
    Segment1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(11E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(41E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1A;
    Segment1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, Segment1.GetEndPoint().GetY());

    // Due to EPSILON problems some of the following methods do not work properly
    Segment1 = PositiveSegment1A;
    Segment1.ShortenFrom(PositiveMidPoint1A);
    ASSERT_DOUBLE_EQ(6.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(31.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1A;
    Segment1.ShortenFrom(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1A;
    Segment1.ShortenFrom(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(11E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(41E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1A;
    Segment1.ShortenTo(PositiveMidPoint1A);
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(6.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(31.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1A;
    Segment1.ShortenTo(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1A;
    Segment1.ShortenTo(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1A;
    Segment1.Shorten(Segment1.GetStartPoint(), PositiveMidPoint1A);
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(6.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(31.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1A;
    Segment1.Shorten(PositiveMidPoint1A, Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(6.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(31.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1A;
    Segment1.Shorten(Segment1.GetStartPoint(), Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, Segment1.GetEndPoint().GetY());

    // Test with segments way into negative regions
    Segment1 = NegativeSegment1A;
    Segment1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(-3.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-25.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-9.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-37.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1A;
    Segment1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-9.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-37.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1A;
    Segment1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(-3.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-25.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1A;
    Segment1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1A;
    Segment1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-9.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-37.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1A;
    Segment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(-10.000000E122, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.000000E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-1.0999999E124, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-4.0999998E124, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1A;
    Segment1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1A;
    Segment1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1A;
    Segment1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(-3.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-25.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1A;
    Segment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(-1.0999999E124, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-4.0999998E124, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.000000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.000000E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1A;
    Segment1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(-11E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-41E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1A;
    Segment1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1A;
    Segment1.ShortenFrom(NegativeMidPoint1A);
    ASSERT_DOUBLE_EQ(-6.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-31.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, Segment1.GetEndPoint().GetY());
    
    Segment1 = NegativeSegment1A;
    Segment1.ShortenFrom(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1A;
    Segment1.ShortenFrom(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(-11E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-41E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1A;
    Segment1.ShortenTo(NegativeMidPoint1A);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-6.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-31.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1A;
    Segment1.ShortenTo(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1A;
    Segment1.ShortenTo(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(-10E122, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1A;
    Segment1.Shorten(Segment1.GetStartPoint(), NegativeMidPoint1A);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-6.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-31.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1A;
    Segment1.Shorten(NegativeMidPoint1A, Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(-6.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-31.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1A;
    Segment1.Shorten(Segment1.GetStartPoint(), Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, Segment1.GetEndPoint().GetY());

    }

//==================================================================================
// AutoCrosses() const
//==================================================================================
TEST_F (HGF2DSegmentTester, AutoCrossesTest)
    {
    
    ASSERT_FALSE(VerticalSegment1A.AutoCrosses());
    ASSERT_FALSE(HorizontalSegment1A.AutoCrosses());
    ASSERT_FALSE(MiscSegment1A.AutoCrosses());
    ASSERT_FALSE(LargeSegment1A.AutoCrosses());

    }

//==================================================================================
// Closest point calculation test
// CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const;
//==================================================================================
TEST_F (HGF2DSegmentTester, CalculateClosestPointTest)
    {

    HGF2DPosition   VeryFarPoint(21E123, 1E123);

    // Test with vertical segment
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment1A.CalculateClosestPoint(VerticalClosePoint1AA).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment1A.CalculateClosestPoint(VerticalClosePoint1AA).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment1A.CalculateClosestPoint(VerticalClosePoint1BA).GetX());
    ASSERT_DOUBLE_EQ(5.000000000000000000, VerticalSegment1A.CalculateClosestPoint(VerticalClosePoint1BA).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment1A.CalculateClosestPoint(VerticalClosePoint1CA).GetX());
    ASSERT_DOUBLE_EQ(7.000000000000000000, VerticalSegment1A.CalculateClosestPoint(VerticalClosePoint1CA).GetY());
    ASSERT_DOUBLE_EQ(0.099999999976716936, VerticalSegment1A.CalculateClosestPoint(VerticalClosePoint1DA).GetX());
    ASSERT_DOUBLE_EQ(10.00000000000000000, VerticalSegment1A.CalculateClosestPoint(VerticalClosePoint1DA).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment1A.CalculateClosestPoint(VerticalCloseMidPoint1A).GetX());
    ASSERT_DOUBLE_EQ(5.100000000000000000, VerticalSegment1A.CalculateClosestPoint(VerticalCloseMidPoint1A).GetY());

    // Test with inverted vertical segment
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment2A.CalculateClosestPoint(VerticalClosePoint1AA).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment2A.CalculateClosestPoint(VerticalClosePoint1AA).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment2A.CalculateClosestPoint(VerticalClosePoint1BA).GetX());
    ASSERT_DOUBLE_EQ(5.000000000000000000, VerticalSegment2A.CalculateClosestPoint(VerticalClosePoint1BA).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment2A.CalculateClosestPoint(VerticalClosePoint1CA).GetX());
    ASSERT_DOUBLE_EQ(7.000000000000000000, VerticalSegment2A.CalculateClosestPoint(VerticalClosePoint1CA).GetY());
    ASSERT_DOUBLE_EQ(0.099999999976716936, VerticalSegment2A.CalculateClosestPoint(VerticalClosePoint1DA).GetX());
    ASSERT_DOUBLE_EQ(10.00000000000000000, VerticalSegment2A.CalculateClosestPoint(VerticalClosePoint1DA).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment2A.CalculateClosestPoint(VerticalCloseMidPoint1A).GetX());
    ASSERT_DOUBLE_EQ(5.100000000000000000, VerticalSegment2A.CalculateClosestPoint(VerticalCloseMidPoint1A).GetY());

    // Test with horizontal segment
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment1A.CalculateClosestPoint(HorizontalClosePoint1AA).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment1A.CalculateClosestPoint(HorizontalClosePoint1AA).GetY());
    ASSERT_DOUBLE_EQ(5.000000000000000000, HorizontalSegment1A.CalculateClosestPoint(HorizontalClosePoint1BA).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment1A.CalculateClosestPoint(HorizontalClosePoint1BA).GetY());
    ASSERT_DOUBLE_EQ(7.000000000000000000, HorizontalSegment1A.CalculateClosestPoint(HorizontalClosePoint1CA).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment1A.CalculateClosestPoint(HorizontalClosePoint1CA).GetY());
    ASSERT_DOUBLE_EQ(10.00000000000000000, HorizontalSegment1A.CalculateClosestPoint(HorizontalClosePoint1DA).GetX());
    ASSERT_DOUBLE_EQ(0.099999999976716936, HorizontalSegment1A.CalculateClosestPoint(HorizontalClosePoint1DA).GetY());
    ASSERT_DOUBLE_EQ(5.100000000000000000, HorizontalSegment1A.CalculateClosestPoint(HorizontalCloseMidPoint1A).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment1A.CalculateClosestPoint(HorizontalCloseMidPoint1A).GetY());

    // Test with inverted horizontal segment
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment2A.CalculateClosestPoint(HorizontalClosePoint1AA).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment2A.CalculateClosestPoint(HorizontalClosePoint1AA).GetY());
    ASSERT_DOUBLE_EQ(5.000000000000000000, HorizontalSegment2A.CalculateClosestPoint(HorizontalClosePoint1BA).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment2A.CalculateClosestPoint(HorizontalClosePoint1BA).GetY());
    ASSERT_DOUBLE_EQ(7.000000000000000000, HorizontalSegment2A.CalculateClosestPoint(HorizontalClosePoint1CA).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment2A.CalculateClosestPoint(HorizontalClosePoint1CA).GetY());
    ASSERT_DOUBLE_EQ(10.00000000000000000, HorizontalSegment2A.CalculateClosestPoint(HorizontalClosePoint1DA).GetX());
    ASSERT_DOUBLE_EQ(0.099999999976716936, HorizontalSegment2A.CalculateClosestPoint(HorizontalClosePoint1DA).GetY());
    ASSERT_DOUBLE_EQ(5.100000000000000000, HorizontalSegment2A.CalculateClosestPoint(HorizontalCloseMidPoint1A).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment2A.CalculateClosestPoint(HorizontalCloseMidPoint1A).GetY());

    // Tests with vertical EPSILON sized segment
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment3A.CalculateClosestPoint(VerticalClosePoint3AA).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment3A.CalculateClosestPoint(VerticalClosePoint3AA).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment3A.CalculateClosestPoint(VerticalClosePoint3BA).GetX());
    ASSERT_DOUBLE_EQ(0.100000066666666680, VerticalSegment3A.CalculateClosestPoint(VerticalClosePoint3BA).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment3A.CalculateClosestPoint(VerticalClosePoint3CA).GetX());
    ASSERT_DOUBLE_EQ(0.100000033333333340, VerticalSegment3A.CalculateClosestPoint(VerticalClosePoint3CA).GetY());
    ASSERT_DOUBLE_EQ(0.099999999976716936, VerticalSegment3A.CalculateClosestPoint(VerticalClosePoint3DA).GetX());
    ASSERT_DOUBLE_EQ(0.100000050000000010, VerticalSegment3A.CalculateClosestPoint(VerticalClosePoint3DA).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment3A.CalculateClosestPoint(VerticalCloseMidPoint3A).GetX());
    ASSERT_DOUBLE_EQ(0.100000050000000010, VerticalSegment3A.CalculateClosestPoint(VerticalCloseMidPoint3A).GetY());

    // Tests with horizontal EPSILON sized segment
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment3A.CalculateClosestPoint(HorizontalClosePoint3AA).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment3A.CalculateClosestPoint(HorizontalClosePoint3AA).GetY());
    ASSERT_DOUBLE_EQ(0.100000066666666680, HorizontalSegment3A.CalculateClosestPoint(HorizontalClosePoint3BA).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment3A.CalculateClosestPoint(HorizontalClosePoint3BA).GetY());
    ASSERT_DOUBLE_EQ(0.100000033333333340, HorizontalSegment3A.CalculateClosestPoint(HorizontalClosePoint3CA).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment3A.CalculateClosestPoint(HorizontalClosePoint3CA).GetY());
    ASSERT_DOUBLE_EQ(0.100000050000000010, HorizontalSegment3A.CalculateClosestPoint(HorizontalClosePoint3DA).GetX());
    ASSERT_DOUBLE_EQ(0.099999999976716936, HorizontalSegment3A.CalculateClosestPoint(HorizontalClosePoint3DA).GetY());
    ASSERT_DOUBLE_EQ(0.100000050000000010, HorizontalSegment3A.CalculateClosestPoint(HorizontalCloseMidPoint3A).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment3A.CalculateClosestPoint(HorizontalCloseMidPoint3A).GetY());

    // Tests with miscalenious EPSILON size segment
    ASSERT_DOUBLE_EQ(0.100000000000000000, MiscSegment3B.CalculateClosestPoint(MiscClosePoint3AA).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, MiscSegment3B.CalculateClosestPoint(MiscClosePoint3AA).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, MiscSegment3B.CalculateClosestPoint(MiscClosePoint3BA).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, MiscSegment3B.CalculateClosestPoint(MiscClosePoint3BA).GetY());
    ASSERT_DOUBLE_EQ(0.100000021643961390, MiscSegment3B.CalculateClosestPoint(MiscClosePoint3CA).GetX());
    ASSERT_DOUBLE_EQ(0.100000097629600710, MiscSegment3B.CalculateClosestPoint(MiscClosePoint3CA).GetY());
    ASSERT_DOUBLE_EQ(0.100000021643961390, MiscSegment3B.CalculateClosestPoint(MiscClosePoint3DA).GetX());
    ASSERT_DOUBLE_EQ(0.100000097629600710, MiscSegment3B.CalculateClosestPoint(MiscClosePoint3DA).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, MiscSegment3B.CalculateClosestPoint(MiscCloseMidPoint3A).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, MiscSegment3B.CalculateClosestPoint(MiscCloseMidPoint3A).GetY());

    // Test with very large segment
    ASSERT_DOUBLE_EQ(4.00000000000000000E123, LargeSegment1A.CalculateClosestPoint(LargeClosePoint1AA).GetX());
    ASSERT_DOUBLE_EQ(-1.0000000000000000E123, LargeSegment1A.CalculateClosestPoint(LargeClosePoint1AA).GetY());
    ASSERT_DOUBLE_EQ(6.94117647058823470E123, LargeSegment1A.CalculateClosestPoint(LargeClosePoint1BA).GetX());
    ASSERT_DOUBLE_EQ(1.07647058823529420E124, LargeSegment1A.CalculateClosestPoint(LargeClosePoint1BA).GetY());
    ASSERT_DOUBLE_EQ(1.05882352941176280E123, LargeSegment1A.CalculateClosestPoint(LargeClosePoint1CA).GetX());
    ASSERT_DOUBLE_EQ(-1.2764705882352939E124, LargeSegment1A.CalculateClosestPoint(LargeClosePoint1CA).GetY());
    ASSERT_DOUBLE_EQ(4.00023529411764720E123, LargeSegment1A.CalculateClosestPoint(LargeClosePoint1DA).GetX());
    ASSERT_DOUBLE_EQ(-9.9905882352941167E122, LargeSegment1A.CalculateClosestPoint(LargeClosePoint1DA).GetY());

    // Test with a NULL segment
    ASSERT_NEAR(0.0, NullSegment1A.CalculateClosestPoint(NegativeClosePoint1AA).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, NullSegment1A.CalculateClosestPoint(NegativeClosePoint1AA).GetY(), MYEPSILON);

    // Tests with special points
    ASSERT_DOUBLE_EQ(10.1, MiscSegment1A.CalculateClosestPoint(VeryFarPointA).GetX());
    ASSERT_DOUBLE_EQ(10.1, MiscSegment1A.CalculateClosestPoint(VeryFarPointA).GetY());
    ASSERT_DOUBLE_EQ(5.10, MiscSegment1A.CalculateClosestPoint(MidPointA).GetX());
    ASSERT_DOUBLE_EQ(5.10, MiscSegment1A.CalculateClosestPoint(MidPointA).GetY());
    ASSERT_DOUBLE_EQ(0.10, MiscSegment1A.CalculateClosestPoint(VeryFarNegativePointA).GetX());
    ASSERT_DOUBLE_EQ(0.10, MiscSegment1A.CalculateClosestPoint(VeryFarNegativePointA).GetY());
    ASSERT_DOUBLE_EQ(10.1, MiscSegment1A.CalculateClosestPoint(VeryFarAlignedPointA).GetX());
    ASSERT_DOUBLE_EQ(10.1, MiscSegment1A.CalculateClosestPoint(VeryFarAlignedPointA).GetY());
    ASSERT_DOUBLE_EQ(0.10, MiscSegment1A.CalculateClosestPoint(VeryFarAlignedNegativePointA).GetX());
    ASSERT_DOUBLE_EQ(0.10, MiscSegment1A.CalculateClosestPoint(VeryFarAlignedNegativePointA).GetY());
    ASSERT_DOUBLE_EQ(0.10, MiscSegment1A.CalculateClosestPoint(MiscSegment1A.GetStartPoint()).GetX());
    ASSERT_DOUBLE_EQ(0.10, MiscSegment1A.CalculateClosestPoint(MiscSegment1A.GetStartPoint()).GetY());
    ASSERT_DOUBLE_EQ(10.1, MiscSegment1A.CalculateClosestPoint(MiscSegment1A.GetEndPoint()).GetX());
    ASSERT_DOUBLE_EQ(10.1, MiscSegment1A.CalculateClosestPoint(MiscSegment1A.GetEndPoint()).GetY());

    }

//==================================================================================
// Intersection test (with other segments only)
// Intersect(const HGF2DVector& pi_rVector, HGF2DPositionCollection* po_pCrossPoints) const;
//==================================================================================
TEST_F (HGF2DSegmentTester, IntersectTest)
    {

    HGF2DPositionCollection   DumPoints;

    // Test with extent disjoint segments
    ASSERT_EQ(0, MiscSegment1A.Intersect(DisjointSegment1A, &DumPoints));

    // Test with disjoint but touching by a side segments
    ASSERT_EQ(0, MiscSegment1A.Intersect(ContiguousExtentSegment1A, &DumPoints));

    // Test with disjoint but touching by a tip segments but not linked
    ASSERT_EQ(0, MiscSegment1A.Intersect(FlirtingExtentSegment1A, &DumPoints));

    // Test with disjoint but touching by a tip segments linked
    ASSERT_EQ(0, MiscSegment1A.Intersect(FlirtingExtentLinkedSegment1A, &DumPoints));

    // Test with vertical segment
    ASSERT_EQ(0, VerticalSegment1A.Intersect(MiscSegment1A, &DumPoints));

    // Test with inverted vertical segment
    ASSERT_EQ(0, VerticalSegment2A.Intersect(MiscSegment1A, &DumPoints));

    // Test with close vertical segments
    ASSERT_EQ(0, VerticalSegment1A.Intersect(VerticalSegment4A, &DumPoints));

    // Test with horizontal segment
    ASSERT_EQ(0, HorizontalSegment1A.Intersect(MiscSegment1A, &DumPoints));

    // Test with inverted horizontal segment
    ASSERT_EQ(0, HorizontalSegment2A.Intersect(MiscSegment1A, &DumPoints));

    // Test with parallel segments
    ASSERT_EQ(0, MiscSegment1A.Intersect(ParallelSegment1A, &DumPoints));

    // Test with near parallel segments
    ASSERT_EQ(0, MiscSegment1A.Intersect(NearParallelSegment1A, &DumPoints));

    // Tests with close near parallel segments
    ASSERT_EQ(0, MiscSegment1A.Intersect(CloseNearParallelSegment1A, &DumPoints));

    // Tests with connected segments
    // At start point...
    ASSERT_EQ(0, MiscSegment1A.Intersect(ConnectedSegment1B, &DumPoints));
    ASSERT_EQ(0, MiscSegment1A.Intersect(ConnectingSegment1B, &DumPoints));

    // At end point ...
    ASSERT_EQ(0, MiscSegment1A.Intersect(ConnectedSegment1AA, &DumPoints));
    ASSERT_EQ(0, MiscSegment1A.Intersect(ConnectingSegment1AA, &DumPoints));

    // Tests with linked segments
    ASSERT_EQ(0, MiscSegment1A.Intersect(LinkedSegment1B, &DumPoints));
    ASSERT_EQ(0, MiscSegment1A.Intersect(LinkedSegment1AA, &DumPoints));

    // Tests with vertical EPSILON sized segment
    ASSERT_EQ(0, VerticalSegment3A.Intersect(MiscSegment1A, &DumPoints));

    // Tests with horizontal EPSILON sized segment
    ASSERT_EQ(0, HorizontalSegment3A.Intersect(MiscSegment1A, &DumPoints));

    // Tests with miscalenious EPSILON size segment
    ASSERT_EQ(0, MiscSegment3B.Intersect(MiscSegment1A, &DumPoints));

    // Test with very large segment
    ASSERT_EQ(0, LargeSegment1A.Intersect(MiscSegment1A, &DumPoints));

    // Test with segments way into positive regions
    ASSERT_EQ(0, PositiveSegment1A.Intersect(MiscSegment1A, &DumPoints));

    // Test with segments way into negative regions
    ASSERT_EQ(0, NegativeSegment1A.Intersect(MiscSegment1A, &DumPoints));

    // Test with a NULL segment
    ASSERT_EQ(0, NullSegment1A.Intersect(MiscSegment1A, &DumPoints));

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
TEST_F (HGF2DSegmentTester, ContiguousnessTest)
    {

    HGF2DPositionCollection     DumPoints;
    HGF2DPosition   FirstDumPoint;
    HGF2DPosition   SecondDumPoint;

    // Test with vertical segments
    ASSERT_TRUE(VerticalSegment1A.AreContiguous(VerticalSegment2A));
    ASSERT_TRUE(VerticalSegment1A.AreContiguousAt(VerticalSegment2A, VerticalMidPoint1A));
    ASSERT_EQ(2, VerticalSegment1A.ObtainContiguousnessPoints(VerticalSegment2A, &DumPoints));
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(0.10, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.1, DumPoints[1].GetY());

    VerticalSegment1A.ObtainContiguousnessPointsAt(VerticalSegment2A, VerticalMidPoint1A, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(0.10, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetY());

    DumPoints.clear();

    // Test with horizontal segments
    ASSERT_TRUE(HorizontalSegment1A.AreContiguous(HorizontalSegment2A));
    ASSERT_TRUE(HorizontalSegment1A.AreContiguousAt(HorizontalSegment2A, HorizontalMidPoint1A));
    ASSERT_EQ(2, HorizontalSegment1A.ObtainContiguousnessPoints(HorizontalSegment2A, &DumPoints));
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.1, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(0.10, DumPoints[1].GetY());

    HorizontalSegment1A.ObtainContiguousnessPointsAt(HorizontalSegment2A, HorizontalMidPoint1A, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10, SecondDumPoint.GetY());

    ASSERT_FALSE(HorizontalSegment1A.AreContiguous(MiscSegment1A));

    DumPoints.clear();

    // Test with positive slope segments
    ASSERT_TRUE(MiscSegment1A.AreContiguous(MiscSegment2A));
    ASSERT_TRUE(MiscSegment1A.AreContiguousAt(MiscSegment2A, MiscMidPoint1A));
    ASSERT_EQ(2, MiscSegment1A.ObtainContiguousnessPoints(MiscSegment2A, &DumPoints));
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.1, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.1, DumPoints[1].GetY());

    MiscSegment1A.ObtainContiguousnessPointsAt(MiscSegment2A, MiscMidPoint1A, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetY());

    ASSERT_FALSE(MiscSegment1A.AreContiguous(LargeSegment1A));

    DumPoints.clear();

    // Tests with negative slope segments
    ASSERT_TRUE(MiscSegment6A.AreContiguous(MiscSegment7A));
    ASSERT_TRUE(MiscSegment6A.AreContiguousAt(MiscSegment7A, MiscMidPoint6A));
    ASSERT_EQ(2, MiscSegment6A.ObtainContiguousnessPoints(MiscSegment7A, &DumPoints));
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(-9.8, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetY());

    MiscSegment6A.ObtainContiguousnessPointsAt(MiscSegment7A, MiscMidPoint6A, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(-9.8, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetY());

    ASSERT_FALSE(MiscSegment6A.AreContiguous(MiscSegment1A));

    DumPoints.clear();

    // Tests with vertical EPSILON sized segment   
    ASSERT_FALSE(VerticalSegment3A.AreContiguous(VerticalSegment2A));
    ASSERT_FALSE(VerticalSegment3A.AreContiguous(HorizontalSegment3A));

    // Tests with horizontal EPSILON sized segment
    ASSERT_FALSE(HorizontalSegment3.AreContiguous(HorizontalSegment2));
    ASSERT_FALSE(HorizontalSegment3.AreContiguous(MiscSegment3));

    // Test with a very large segment
    // Precision problem
    ASSERT_TRUE(LargeSegment1A.AreContiguous(LargeSegment2A));
    ASSERT_TRUE(LargeSegment1A.AreContiguousAt(LargeSegment2A, LargeMidPoint1A));
    ASSERT_EQ(2, LargeSegment1A.ObtainContiguousnessPoints(LargeSegment2A, &DumPoints));
    ASSERT_DOUBLE_EQ(-10.0E122, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(-2.10E124, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(9.000E123, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(1.900E124, DumPoints[1].GetY());

    LargeSegment1A.ObtainContiguousnessPointsAt(LargeSegment2A, LargeMidPoint1A, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(-9.9999999999999998E122, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(-2.1000000000000001E124, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(8.99999999999999970E123, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(1.89999999999999990E124, SecondDumPoint.GetY());

    ASSERT_FALSE(LargeSegment1A.AreContiguous(HorizontalSegment1A));

    DumPoints.clear();

    // Test with a segment way into positive regions   
    ASSERT_TRUE(PositiveSegment1A.AreContiguous(PositiveSegment2A));
    ASSERT_TRUE(PositiveSegment1A.AreContiguousAt(PositiveSegment2A, PositiveMidPoint1A));
    ASSERT_EQ(2, PositiveSegment1A.ObtainContiguousnessPoints(PositiveSegment2A, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0E122, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(2.10E124, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(1.10E124, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(4.10E124, DumPoints[1].GetY());

    PositiveSegment1A.ObtainContiguousnessPointsAt(PositiveSegment2A, PositiveMidPoint1A, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(10.0E122, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(2.10E124, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(1.10E124, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(4.10E124, SecondDumPoint.GetY());

    ASSERT_FALSE(PositiveSegment1A.AreContiguous(HorizontalSegment1A));

    DumPoints.clear();

    // Test with segment way into negative segments   
    ASSERT_TRUE(NegativeSegment1A.AreContiguous(NegativeSegment2A));
    ASSERT_TRUE(NegativeSegment1A.AreContiguousAt(NegativeSegment2A, NegativeMidPoint1A));
    ASSERT_EQ(2, NegativeSegment1A.ObtainContiguousnessPoints(NegativeSegment2A, &DumPoints));
    ASSERT_DOUBLE_EQ(-10.0E122, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(-2.10E124, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(-1.10E124, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(-4.10E124, DumPoints[1].GetY());

    NegativeSegment1A.ObtainContiguousnessPointsAt(NegativeSegment2A, NegativeMidPoint1A, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(-10.0E122, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(-2.10E124, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(-1.10E124, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(-4.10E124, SecondDumPoint.GetY());

    ASSERT_FALSE(NegativeSegment1A.AreContiguous(HorizontalSegment1A));

    }

//==================================================================================
// AreContiguousAtAndGet(const HGF2DVector& pi_rVector, const HGF2DPosition& pi_rPoint,
//                       HGF2DPosition* po_pFirstContiguousnessPoint, HGF2DPosition* po_pSecondContiguousnessPoint) const;
//==================================================================================
TEST_F (HGF2DSegmentTester, AreContiguousAtAndGetTest)
    {

    HGF2DPosition FirstDumPoint;
    HGF2DPosition SecondDumPoint;

    // Test with vertical segments
    ASSERT_TRUE(VerticalSegment1A.AreContiguousAtAndGet(VerticalSegment2A, VerticalMidPoint1A, &FirstDumPoint, &SecondDumPoint));
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(0.10, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetY());

    // Test with horizontal segments
    ASSERT_TRUE(HorizontalSegment1A.AreContiguousAtAndGet(HorizontalSegment2A, HorizontalMidPoint1A, &FirstDumPoint, &SecondDumPoint));
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10, SecondDumPoint.GetY());

    // Test with positive slope segments
    MiscSegment1A.AreContiguousAtAndGet(MiscSegment2A, MiscMidPoint1A, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetY());

    // Tests with negative slope segments
    MiscSegment6A.AreContiguousAtAndGet(MiscSegment7A, MiscMidPoint6A, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(0.10000000000000000, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(-9.8000000000000007, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0000000000000000, SecondDumPoint.GetY());

    // Test with a very large segment
    LargeSegment1A.AreContiguousAtAndGet(LargeSegment2A, LargeMidPoint1A, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(-10.0E122, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(-2.10E124, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(9.000E123, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(1.900E124, SecondDumPoint.GetY());

    // Test with a segment way into positive regions
    PositiveSegment1A.AreContiguousAtAndGet(PositiveSegment2A, PositiveMidPoint1A, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(10.0E122, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(2.10E124, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(1.10E124, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(4.10E124, SecondDumPoint.GetY());

    // Test with segment way into negative segments
    NegativeSegment1A.AreContiguousAtAndGet(NegativeSegment2A, NegativeMidPoint1A, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(-10.0E122, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(-2.10E124, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(-1.10E124, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(-4.10E124, SecondDumPoint.GetY());

    }


//==================================================================================
// Interaction info with other segment
// Crosses(const HGF2DVector& pi_rVector) const;
// AreAdjacent(const HGF2DVector& pi_rVector) const;
// IsPointOn(const HGF2DPosition& pi_rTestPoint) const;
//==================================================================================
TEST_F (HGF2DSegmentTester, InteractionTest)
    {

    // Tests with a vertical segment
    ASSERT_FALSE(VerticalSegment1A.Crosses(VerticalSegment2A));
    ASSERT_TRUE(VerticalSegment1A.AreAdjacent(VerticalSegment2A));

    ASSERT_FALSE(VerticalSegment1A.Crosses(MiscSegment1A));
    ASSERT_FALSE(VerticalSegment1A.AreAdjacent(MiscSegment1A));

    ASSERT_FALSE(VerticalSegment1A.Crosses(VerticalSegment3A));
    ASSERT_TRUE(VerticalSegment1A.AreAdjacent(VerticalSegment3A));

    ASSERT_FALSE(VerticalSegment1A.Crosses(LargeSegment1A));
    ASSERT_FALSE(VerticalSegment1A.AreAdjacent(LargeSegment1A));

    ASSERT_FALSE(VerticalSegment1A.IsPointOn(HGF2DPosition(10, 10)));
    ASSERT_FALSE(VerticalSegment1A.IsPointOn(HGF2DPosition(0.1, 0.1-1.1*MYEPSILON)));

    ASSERT_TRUE(VerticalSegment1A.IsPointOn(VerticalSegment1A.GetStartPoint()));
    ASSERT_TRUE(VerticalSegment1A.IsPointOn(VerticalSegment1A.GetEndPoint()));
    ASSERT_FALSE(VerticalSegment1A.IsPointOn(VerticalSegment1A.GetStartPoint(), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(VerticalSegment1A.IsPointOn(VerticalSegment1A.GetEndPoint(), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(VerticalSegment1A.IsPointOn(VerticalMidPoint1A));
    ASSERT_TRUE(VerticalSegment1A.IsPointOn(VerticalMidPoint1A, HGF2DVector::EXCLUDE_EXTREMITIES));

    // Tests with an inverted vertical segment
    ASSERT_FALSE(VerticalSegment2A.Crosses(VerticalSegment1A));
    ASSERT_TRUE(VerticalSegment2A.AreAdjacent(VerticalSegment1A));

    ASSERT_FALSE(VerticalSegment2A.Crosses(MiscSegment1A));
    ASSERT_FALSE(VerticalSegment2A.AreAdjacent(MiscSegment1A));

    ASSERT_FALSE(VerticalSegment2A.Crosses(VerticalSegment3A));
    ASSERT_TRUE(VerticalSegment2A.AreAdjacent(VerticalSegment3A));

    ASSERT_FALSE(VerticalSegment2A.Crosses(LargeSegment1A));
    ASSERT_FALSE(VerticalSegment2A.AreAdjacent(LargeSegment1A));

    ASSERT_FALSE(VerticalSegment2A.IsPointOn(HGF2DPosition(10, 10)));
    ASSERT_FALSE(VerticalSegment2A.IsPointOn(HGF2DPosition(0.1, 0.1-1.1*MYEPSILON)));

    ASSERT_TRUE(VerticalSegment2A.IsPointOn(VerticalSegment2A.GetStartPoint()));
    ASSERT_TRUE(VerticalSegment2A.IsPointOn(VerticalSegment2A.GetEndPoint()));
    ASSERT_TRUE(VerticalSegment2A.IsPointOn(VerticalMidPoint1A));
    ASSERT_FALSE(VerticalSegment2A.IsPointOn(VerticalSegment2A.GetStartPoint(), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(VerticalSegment2A.IsPointOn(VerticalSegment2A.GetEndPoint(), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(VerticalSegment2A.IsPointOn(VerticalMidPoint1A, HGF2DVector::EXCLUDE_EXTREMITIES));

    // Tests with a horizontal segment
    ASSERT_FALSE(HorizontalSegment1A.Crosses(HorizontalSegment2A));
    ASSERT_TRUE(HorizontalSegment1A.AreAdjacent(HorizontalSegment2A));

    ASSERT_FALSE(HorizontalSegment1A.Crosses(MiscSegment1A));
    ASSERT_FALSE(HorizontalSegment1A.AreAdjacent(MiscSegment1A));

    ASSERT_FALSE(HorizontalSegment1A.Crosses(HorizontalSegment3A));
    ASSERT_TRUE(HorizontalSegment1A.AreAdjacent(HorizontalSegment3A));

    ASSERT_FALSE(HorizontalSegment1A.Crosses(LargeSegment1A));
    ASSERT_FALSE(HorizontalSegment1A.AreAdjacent(LargeSegment1A));

    ASSERT_FALSE(HorizontalSegment1A.IsPointOn(HGF2DPosition(10, 10)));
    ASSERT_FALSE(HorizontalSegment1A.IsPointOn(HGF2DPosition(0.1-1.1*MYEPSILON, 0.1)));

    ASSERT_TRUE(HorizontalSegment1A.IsPointOn(HorizontalSegment1A.GetStartPoint()));
    ASSERT_TRUE(HorizontalSegment1A.IsPointOn(HorizontalSegment1A.GetEndPoint()));
    ASSERT_TRUE(HorizontalSegment1A.IsPointOn(HorizontalMidPoint1A));
    ASSERT_FALSE(HorizontalSegment1A.IsPointOn(HorizontalSegment1A.GetStartPoint(), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(HorizontalSegment1A.IsPointOn(HorizontalSegment1A.GetEndPoint(), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(HorizontalSegment1A.IsPointOn(HorizontalMidPoint1A, HGF2DVector::EXCLUDE_EXTREMITIES));

    // Tests with an inverted horizontal segment
    ASSERT_FALSE(HorizontalSegment2A.Crosses(HorizontalSegment1A));
    ASSERT_TRUE(HorizontalSegment2A.AreAdjacent(HorizontalSegment1A));

    ASSERT_FALSE(HorizontalSegment2A.Crosses(MiscSegment1A));
    ASSERT_FALSE(HorizontalSegment2A.AreAdjacent(MiscSegment1A));

    ASSERT_FALSE(HorizontalSegment2A.Crosses(HorizontalSegment3A));
    ASSERT_TRUE(HorizontalSegment2A.AreAdjacent(HorizontalSegment3A));

    ASSERT_FALSE(HorizontalSegment2A.Crosses(LargeSegment1A));
    ASSERT_FALSE(HorizontalSegment2A.AreAdjacent(LargeSegment1A));

    ASSERT_FALSE(HorizontalSegment2A.IsPointOn(HGF2DPosition(10, 10)));
    ASSERT_FALSE(HorizontalSegment2A.IsPointOn(HGF2DPosition(0.1-1.1*MYEPSILON, 0.1)));

    ASSERT_TRUE(HorizontalSegment2A.IsPointOn(HorizontalSegment2A.GetStartPoint()));
    ASSERT_TRUE(HorizontalSegment2A.IsPointOn(HorizontalSegment2A.GetEndPoint()));
    ASSERT_TRUE(HorizontalSegment2A.IsPointOn(HorizontalMidPoint1A));
    ASSERT_FALSE(HorizontalSegment2A.IsPointOn(HorizontalSegment2A.GetStartPoint(), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(HorizontalSegment2A.IsPointOn(HorizontalSegment2A.GetEndPoint(), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(HorizontalSegment2A.IsPointOn(HorizontalMidPoint1A, HGF2DVector::EXCLUDE_EXTREMITIES));

    // Tests with a positive slope segment
    ASSERT_FALSE(MiscSegment1A.Crosses(MiscSegment2A));
    ASSERT_TRUE(MiscSegment1A.AreAdjacent(MiscSegment2A));

    ASSERT_FALSE(MiscSegment1A.Crosses(HorizontalSegment1A));
    ASSERT_FALSE(MiscSegment1A.AreAdjacent(HorizontalSegment1A));

    ASSERT_FALSE(MiscSegment1A.Crosses(MiscSegment3B));
    ASSERT_FALSE(MiscSegment1A.AreAdjacent(MiscSegment3B));

    ASSERT_FALSE(MiscSegment1A.Crosses(LargeSegment1A));
    ASSERT_FALSE(MiscSegment1A.AreAdjacent(LargeSegment1A));

    ASSERT_FALSE(MiscSegment1A.IsPointOn(HGF2DPosition(20, 20)));
    ASSERT_FALSE(MiscSegment1A.IsPointOn(HGF2DPosition(0.1, 0.1-1.1*MYEPSILON)));

    ASSERT_TRUE(MiscSegment1A.IsPointOn(MiscSegment1A.GetStartPoint()));
    ASSERT_TRUE(MiscSegment1A.IsPointOn(MiscSegment1A.GetEndPoint()));
    ASSERT_TRUE(MiscSegment1A.IsPointOn(MiscMidPoint1A));
    ASSERT_FALSE(MiscSegment1A.IsPointOn(MiscSegment1A.GetStartPoint(), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(MiscSegment1A.IsPointOn(MiscSegment1A.GetEndPoint(), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(MiscSegment1A.IsPointOn(MiscMidPoint1A, HGF2DVector::EXCLUDE_EXTREMITIES));

    // Tests with a negative slope segment
    ASSERT_FALSE(MiscSegment2A.Crosses(MiscSegment1A));
    ASSERT_TRUE(MiscSegment2A.AreAdjacent(MiscSegment1A));

    ASSERT_FALSE(MiscSegment2A.IsPointOn(HGF2DPosition(20, 20)));
    ASSERT_FALSE(MiscSegment2A.IsPointOn(HGF2DPosition(0.1, 0.1-1.1*MYEPSILON)));

    ASSERT_TRUE(MiscSegment2A.IsPointOn(MiscSegment1A.GetStartPoint()));
    ASSERT_TRUE(MiscSegment2A.IsPointOn(MiscSegment1A.GetEndPoint()));
    ASSERT_TRUE(MiscSegment2A.IsPointOn(MiscMidPoint1A));
    ASSERT_FALSE(MiscSegment2A.IsPointOn(MiscSegment1A.GetStartPoint(), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(MiscSegment2A.IsPointOn(MiscSegment1A.GetEndPoint(), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(MiscSegment2A.IsPointOn(MiscMidPoint1A, HGF2DVector::EXCLUDE_EXTREMITIES));

    // Tests with a vertical EPSILON sized segment
    ASSERT_FALSE(VerticalSegment3A.Crosses(VerticalSegment1A));
    ASSERT_TRUE(VerticalSegment3A.AreAdjacent(VerticalSegment1A));

    ASSERT_FALSE(VerticalSegment3A.Crosses(MiscSegment1A));
    ASSERT_FALSE(VerticalSegment3A.AreAdjacent(MiscSegment1A));

    ASSERT_FALSE(VerticalSegment3A.Crosses(MiscSegment3B));
    ASSERT_FALSE(VerticalSegment3A.AreAdjacent(MiscSegment3B));

    ASSERT_FALSE(VerticalSegment3A.Crosses(LargeSegment1A));
    ASSERT_FALSE(VerticalSegment3A.AreAdjacent(LargeSegment1A));

    ASSERT_FALSE(VerticalSegment3A.IsPointOn(HGF2DPosition(10, 10)));
    ASSERT_FALSE(VerticalSegment3A.IsPointOn(HGF2DPosition(0.1, 0.1-1.1*MYEPSILON)));

    ASSERT_TRUE(VerticalSegment3A.IsPointOn(VerticalSegment3A.GetStartPoint()));
    ASSERT_TRUE(VerticalSegment3A.IsPointOn(VerticalSegment3A.GetEndPoint()));
    ASSERT_TRUE(VerticalSegment3A.IsPointOn(VerticalMidPoint3A));

    // Tests with an horizontal EPSILON SIZED segment
    ASSERT_FALSE(HorizontalSegment3A.Crosses(HorizontalSegment1A));
    ASSERT_TRUE(HorizontalSegment3A.AreAdjacent(HorizontalSegment1A));

    ASSERT_FALSE(HorizontalSegment3A.Crosses(MiscSegment1A));
    ASSERT_FALSE(HorizontalSegment3A.AreAdjacent(MiscSegment1A));

    ASSERT_FALSE(HorizontalSegment3A.Crosses(MiscSegment3B));
    ASSERT_FALSE(HorizontalSegment3A.AreAdjacent(MiscSegment3B));

    ASSERT_FALSE(HorizontalSegment3A.Crosses(LargeSegment1A));
    ASSERT_FALSE(HorizontalSegment3A.AreAdjacent(LargeSegment1A));

    ASSERT_FALSE(HorizontalSegment3A.IsPointOn(HGF2DPosition(10, 10)));
    ASSERT_FALSE(HorizontalSegment3A.IsPointOn(HGF2DPosition(0.1, 0.1-1.1*MYEPSILON)));
    ASSERT_FALSE(HorizontalSegment3A.IsPointOn(HGF2DPosition(HorizontalMidPoint3A.GetX(), HorizontalMidPoint3A.GetY() - 1.1*MYEPSILON)));
    ASSERT_FALSE(HorizontalSegment3A.IsPointOn(HGF2DPosition(HorizontalMidPoint3A.GetX(), HorizontalMidPoint3A.GetY() + 1.1*MYEPSILON)));
    ASSERT_TRUE(HorizontalSegment3A.IsPointOn(HGF2DPosition(HorizontalMidPoint3A.GetX(), HorizontalMidPoint3A.GetY() - 0.9*MYEPSILON)));
    ASSERT_TRUE(HorizontalSegment3A.IsPointOn(HGF2DPosition(HorizontalMidPoint3A.GetX(), HorizontalMidPoint3A.GetY() + 0.9*MYEPSILON)));
    ASSERT_TRUE(HorizontalSegment3A.IsPointOn(HorizontalSegment3A.GetStartPoint()));
    ASSERT_TRUE(HorizontalSegment3A.IsPointOn(HorizontalSegment3A.GetEndPoint()));
    ASSERT_TRUE(HorizontalSegment3A.IsPointOn(HorizontalMidPoint3A));

    // Tests with a miscalenious EPSILON sized segment
    ASSERT_FALSE(MiscSegment3A.Crosses(MiscSegment1));
    ASSERT_FALSE(MiscSegment3A.AreAdjacent(MiscSegment1));

    ASSERT_FALSE(MiscSegment3A.Crosses(LargeSegment1));
    ASSERT_FALSE(MiscSegment3A.AreAdjacent(LargeSegment1));

    ASSERT_FALSE(MiscSegment3B.IsPointOn(HGF2DPosition(10, 10)));
    ASSERT_FALSE(MiscSegment3B.IsPointOn(HGF2DPosition(0.1, 0.1-1.1*MYEPSILON)));
    ASSERT_FALSE(MiscSegment3B.IsPointOn(HGF2DPosition(MiscMidPoint3A.GetX() - 1.1*MYEPSILON, MiscMidPoint3A.GetY() + 1.1*MYEPSILON)));
    ASSERT_FALSE(MiscSegment3B.IsPointOn(HGF2DPosition(MiscMidPoint3A.GetX() - 1.1*MYEPSILON, MiscMidPoint3A.GetY() + 1.1*MYEPSILON)));
    ASSERT_TRUE(MiscSegment3B.IsPointOn(HGF2DPosition(MiscMidPoint3A.GetX()  + 0.9*MYEPSILON, MiscMidPoint3A.GetY() + 0.9*MYEPSILON)));
    ASSERT_TRUE(MiscSegment3B.IsPointOn(HGF2DPosition(MiscMidPoint3A.GetX()  + 0.9*MYEPSILON, MiscMidPoint3A.GetY() + 0.9*MYEPSILON)));
    ASSERT_TRUE(MiscSegment3B.IsPointOn(MiscSegment3B.GetStartPoint()));
    ASSERT_TRUE(MiscSegment3B.IsPointOn(MiscSegment3B.GetEndPoint()));
    ASSERT_TRUE(MiscSegment3B.IsPointOn(MiscMidPoint3A));

    // Due to precision problems, the following
    // Tests with a very large segment
    ASSERT_FALSE(LargeSegment1A.Crosses(LargeSegment2A));
    ASSERT_FALSE(LargeSegment1A.AreAdjacent(LargeSegment2A));

    ASSERT_FALSE(LargeSegment1A.Crosses(MiscSegment1A));
    ASSERT_FALSE(LargeSegment1A.AreAdjacent(MiscSegment1A));

    ASSERT_FALSE(LargeSegment1A.Crosses(PositiveSegment1A));
    ASSERT_FALSE(LargeSegment1A.AreAdjacent(PositiveSegment1A));

    ASSERT_FALSE(LargeSegment1A.IsPointOn(HGF2DPosition(10, 10)));
    ASSERT_TRUE(LargeSegment1A.IsPointOn(LargeSegment1A.GetStartPoint()));
    ASSERT_TRUE(LargeSegment1A.IsPointOn(LargeSegment1A.GetEndPoint()));
    ASSERT_TRUE(LargeSegment1A.IsPointOn(LargeMidPoint1A));

    // Tests with a way into positive region segment
    ASSERT_FALSE(PositiveSegment1A.Crosses(PositiveSegment2A));
    ASSERT_FALSE(PositiveSegment1A.AreAdjacent(PositiveSegment2A));

    ASSERT_FALSE(PositiveSegment1A.Crosses(MiscSegment1A));
    ASSERT_FALSE(PositiveSegment1A.AreAdjacent(MiscSegment1A));

    ASSERT_FALSE(PositiveSegment1A.Crosses(MiscSegment3B));
    ASSERT_FALSE(PositiveSegment1A.AreAdjacent(MiscSegment3B));

    ASSERT_FALSE(PositiveSegment1A.Crosses(LargeSegment1A));
    ASSERT_FALSE(PositiveSegment1A.AreAdjacent(LargeSegment1A));

    ASSERT_FALSE(PositiveSegment1A.IsPointOn(HGF2DPosition(10, 10)));
    ASSERT_TRUE(PositiveSegment1A.IsPointOn(PositiveSegment1A.GetStartPoint()));
    ASSERT_TRUE(PositiveSegment1A.IsPointOn(PositiveSegment1A.GetEndPoint()));
    ASSERT_TRUE(PositiveSegment1A.IsPointOn(PositiveMidPoint1A));

    // Tests with a way into negative region segment
    ASSERT_FALSE(NegativeSegment1A.Crosses(NegativeSegment2A));
    ASSERT_FALSE(NegativeSegment1A.AreAdjacent(NegativeSegment2A));

    ASSERT_FALSE(NegativeSegment1A.Crosses(MiscSegment1A));
    ASSERT_FALSE(NegativeSegment1A.AreAdjacent(MiscSegment1A));

    ASSERT_FALSE(NegativeSegment1A.Crosses(MiscSegment3B));
    ASSERT_FALSE(NegativeSegment1A.AreAdjacent(MiscSegment3B));

    ASSERT_FALSE(NegativeSegment1A.Crosses(LargeSegment1A));
    ASSERT_FALSE(NegativeSegment1A.AreAdjacent(LargeSegment1A));

    ASSERT_FALSE(NegativeSegment1A.IsPointOn(HGF2DPosition(10, 10)));
    ASSERT_TRUE(NegativeSegment1A.IsPointOn(NegativeSegment1A.GetStartPoint()));
    ASSERT_TRUE(NegativeSegment1A.IsPointOn(NegativeSegment1A.GetEndPoint()));
    ASSERT_TRUE(NegativeSegment1A.IsPointOn(NegativeMidPoint1A));

    // Tests with a NULL segment
    ASSERT_FALSE(NullSegment1A.Crosses(MiscSegment1A));
    ASSERT_FALSE(NullSegment1A.AreAdjacent(MiscSegment1A));

    ASSERT_FALSE(NullSegment1A.IsPointOn(HGF2DPosition(10, 10)));
    ASSERT_TRUE(NullSegment1A.IsPointOn(NullSegment1A.GetStartPoint()));
    ASSERT_TRUE(NullSegment1A.IsPointOn(NullSegment1A.GetEndPoint()));

    }

//==================================================================================
// IsPointOn(const HGF2DPosition& pi_rTestPoint) const
//==================================================================================
TEST_F (HGF2DSegmentTester, IsPointOnTest)
    {

    ASSERT_FALSE(VerticalSegment1A.IsPointOn(HGF2DPosition(10, 10)));
    ASSERT_FALSE(VerticalSegment1A.IsPointOn(HGF2DPosition(0.1, 0.1-1.1*MYEPSILON)));
    ASSERT_FALSE(VerticalSegment1A.IsPointOn(HGF2DPosition(VerticalMidPoint1A.GetX() - 1.1*MYEPSILON, VerticalMidPoint1A.GetY())));
    ASSERT_FALSE(VerticalSegment1A.IsPointOn(HGF2DPosition(VerticalMidPoint1A.GetX() + 1.1*MYEPSILON, VerticalMidPoint1A.GetY())));
    ASSERT_TRUE(VerticalSegment1A.IsPointOn(HGF2DPosition(VerticalMidPoint1A.GetX() -0.9*MYEPSILON,   VerticalMidPoint1A.GetY())));
    ASSERT_TRUE(VerticalSegment1A.IsPointOn(HGF2DPosition(VerticalMidPoint1A.GetX() + 0.9*MYEPSILON,  VerticalMidPoint1A.GetY())));
    ASSERT_TRUE(VerticalSegment1A.IsPointOn(VerticalSegment1A.GetStartPoint()));
    ASSERT_TRUE(VerticalSegment1A.IsPointOn(VerticalSegment1A.GetEndPoint()));
    ASSERT_FALSE(VerticalSegment1A.IsPointOn(VerticalSegment1A.GetStartPoint(), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(VerticalSegment1A.IsPointOn(VerticalSegment1A.GetEndPoint(), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(VerticalSegment1A.IsPointOn(VerticalMidPoint1A));
    ASSERT_TRUE(VerticalSegment1A.IsPointOn(VerticalMidPoint1A, HGF2DVector::EXCLUDE_EXTREMITIES));

    }

//==================================================================================
// Bearing calculation tests
// CalculateBearing(const HGF2DPosition& pi_rPositionPoint,
//                  HGF2DVector::ArbitraryDirection pi_Direction = HGF2DVector::BETA) const;
//==================================================================================
TEST_F (HGF2DSegmentTester, BearingTest)
    {

    // Obtain bearing ALPHA of a vertical segment
    ASSERT_DOUBLE_EQ(4.7123889803846897, VerticalSegment1A.CalculateBearing(VerticalMidPoint1A, HGF2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a vertical segment
    ASSERT_DOUBLE_EQ(1.5707963267948966, VerticalSegment1A.CalculateBearing(VerticalMidPoint1A, HGF2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a inverted vertical segment
    ASSERT_DOUBLE_EQ(1.5707963267948966, VerticalSegment2A.CalculateBearing(VerticalMidPoint1A, HGF2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a inverted vertical segment
    ASSERT_DOUBLE_EQ(4.7123889803846897, VerticalSegment2A.CalculateBearing(VerticalMidPoint1A, HGF2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of an horizontal segment
    ASSERT_DOUBLE_EQ(3.1415926535897931, HorizontalSegment1A.CalculateBearing(HorizontalMidPoint1A, HGF2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of an horizontal segment
    ASSERT_NEAR(0.0, HorizontalSegment1A.CalculateBearing(HorizontalMidPoint1A, HGF2DVector::BETA).GetAngle(), MYEPSILON);

    // Obtain bearing ALPHA of an inverted horizontal segment
    ASSERT_NEAR(0.0, HorizontalSegment2A.CalculateBearing(HorizontalMidPoint1A, HGF2DVector::ALPHA).GetAngle(), MYEPSILON);

    // Obtain bearing BETA of an inverted horizontal segment
    ASSERT_DOUBLE_EQ(3.1415926535897931, HorizontalSegment2A.CalculateBearing(HorizontalMidPoint1A, HGF2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a positive slope segment
    ASSERT_DOUBLE_EQ(-2.3561944901923448, MiscSegment1A.CalculateBearing(MiscMidPoint1A, HGF2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a positive slope segment
    ASSERT_DOUBLE_EQ(0.78539816339744828, MiscSegment1A.CalculateBearing(MiscMidPoint1A, HGF2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a inverted positive slope segment
    ASSERT_DOUBLE_EQ(0.78539816339744828, MiscSegment2A.CalculateBearing(MiscMidPoint1A, HGF2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a inverted positive slope segment
    ASSERT_DOUBLE_EQ(-2.3561944901923448, MiscSegment2A.CalculateBearing(MiscMidPoint1A, HGF2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a vertical EPSILON SIZED segment
    ASSERT_DOUBLE_EQ(4.7123889803846897, VerticalSegment3A.CalculateBearing(VerticalMidPoint3A, HGF2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a vertical EPSILON SIZED segment
    ASSERT_DOUBLE_EQ(1.5707963267948966, VerticalSegment3A.CalculateBearing(VerticalMidPoint3A, HGF2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a horizontal EPSILON SIZED segment
    ASSERT_DOUBLE_EQ(3.1415926535897931, HorizontalSegment3A.CalculateBearing(HorizontalMidPoint3A, HGF2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a horizontal EPSILON SIZED segment
    ASSERT_NEAR(0.0, HorizontalSegment3A.CalculateBearing(HorizontalMidPoint3A, HGF2DVector::BETA).GetAngle(), MYEPSILON);

    // Obtain bearing ALPHA of a miscaleniuous EPSILON SIZED segment
    ASSERT_DOUBLE_EQ(-1.7889624832338027, MiscSegment3B.CalculateBearing(MiscMidPoint3A, HGF2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a miscaleniuous EPSILON SIZED segment
    ASSERT_DOUBLE_EQ(1.3526301703559906, MiscSegment3B.CalculateBearing(MiscMidPoint3A, HGF2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a very large segment
    ASSERT_DOUBLE_EQ(-1.8157749899217608, LargeSegment1A.CalculateBearing(LargeMidPoint1A, HGF2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a very large segment
    ASSERT_DOUBLE_EQ(1.3258176636680326, LargeSegment1A.CalculateBearing(LargeMidPoint1A, HGF2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a inverted very large segment
    ASSERT_DOUBLE_EQ(1.3258176636680326, LargeSegment2A.CalculateBearing(LargeMidPoint1A, HGF2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a inverted very large segment
    ASSERT_DOUBLE_EQ(-1.8157749899217608, LargeSegment2A.CalculateBearing(LargeMidPoint1A, HGF2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a segment way into negative coordinates
    ASSERT_DOUBLE_EQ(1.1071487177940904, NegativeSegment1A.CalculateBearing(NegativeMidPoint1A, HGF2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a segment way into negative coordinates
    ASSERT_DOUBLE_EQ(-2.0344439357957027, NegativeSegment1A.CalculateBearing(NegativeMidPoint1A, HGF2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a inverted segment way into negative coordinates
    ASSERT_DOUBLE_EQ(-2.0344439357957027, NegativeSegment2A.CalculateBearing(NegativeMidPoint1A, HGF2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a inverted segment way into negative coordinates
    ASSERT_DOUBLE_EQ(1.1071487177940904, NegativeSegment2A.CalculateBearing(NegativeMidPoint1A, HGF2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a segment way into positive values
    ASSERT_DOUBLE_EQ(-2.0344439357957027, PositiveSegment1A.CalculateBearing(PositiveMidPoint1A, HGF2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a segment way into positive values
    ASSERT_DOUBLE_EQ(1.1071487177940904, PositiveSegment1A.CalculateBearing(PositiveMidPoint1A, HGF2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a inverted segment way into positive values
    ASSERT_DOUBLE_EQ(1.1071487177940904, PositiveSegment2A.CalculateBearing(PositiveMidPoint1A, HGF2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a inverted segment way into positive values
    ASSERT_DOUBLE_EQ(-2.0344439357957027, PositiveSegment2A.CalculateBearing(PositiveMidPoint1A, HGF2DVector::BETA).GetAngle());

    }
        
//==================================================================================
// CalculateAngularAcceleration(const HGF2DPosition& pi_rPositionPoint,
//                              HGF2DVector::ArbitraryDirection pi_Direction = HGF2DVector::BETA) const;
//==================================================================================
TEST_F (HGF2DSegmentTester, AngularAccelerationTest)
    {

    // Obtain angular acceleration ALPHA of a vertical segment
    ASSERT_NEAR(0.0, VerticalSegment1A.CalculateAngularAcceleration(VerticalMidPoint1A, HGF2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a vertical segment
    ASSERT_NEAR(0.0, VerticalSegment1A.CalculateAngularAcceleration(VerticalMidPoint1A, HGF2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a inverted vertical segment
    ASSERT_NEAR(0.0, VerticalSegment2A.CalculateAngularAcceleration(VerticalMidPoint1A, HGF2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a inverted vertical segment
    ASSERT_NEAR(0.0, VerticalSegment2A.CalculateAngularAcceleration(VerticalMidPoint1A, HGF2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of an horizontal segment
    ASSERT_NEAR(0.0, HorizontalSegment1A.CalculateAngularAcceleration(HorizontalMidPoint1A, HGF2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of an horizontal segment
    ASSERT_NEAR(0.0, HorizontalSegment1A.CalculateAngularAcceleration(HorizontalMidPoint1A, HGF2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of an inverted horizontal segment
    ASSERT_NEAR(0.0, HorizontalSegment2A.CalculateAngularAcceleration(HorizontalMidPoint1A, HGF2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of an inverted horizontal segment
    ASSERT_NEAR(0.0, HorizontalSegment2A.CalculateAngularAcceleration(HorizontalMidPoint1A, HGF2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a positive slope segment
    ASSERT_NEAR(0.0, MiscSegment1A.CalculateAngularAcceleration(MiscMidPoint1A, HGF2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a positive slope segment
    ASSERT_NEAR(0.0, MiscSegment1A.CalculateAngularAcceleration(MiscMidPoint1A, HGF2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a inverted positive slope segment
    ASSERT_NEAR(0.0, MiscSegment2A.CalculateAngularAcceleration(MiscMidPoint1A, HGF2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a inverted positive slope segment
    ASSERT_NEAR(0.0, MiscSegment2A.CalculateAngularAcceleration(MiscMidPoint1A, HGF2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a vertical EPSILON SIZED segment
    ASSERT_NEAR(0.0, VerticalSegment3A.CalculateAngularAcceleration(VerticalMidPoint3A, HGF2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a vertical EPSILON SIZED segment
    ASSERT_NEAR(0.0, VerticalSegment3A.CalculateAngularAcceleration(VerticalMidPoint3A, HGF2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a horizontal EPSILON SIZED segment
    ASSERT_NEAR(0.0, HorizontalSegment3A.CalculateAngularAcceleration(HorizontalMidPoint3A, HGF2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a horizontal EPSILON SIZED segment
    ASSERT_NEAR(0.0, HorizontalSegment3A.CalculateAngularAcceleration(HorizontalMidPoint3A, HGF2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a miscaleniuous EPSILON SIZED segment
    ASSERT_NEAR(0.0, MiscSegment3B.CalculateAngularAcceleration(MiscMidPoint3A, HGF2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a miscaleniuous EPSILON SIZED segment
    ASSERT_NEAR(0.0, MiscSegment3B.CalculateAngularAcceleration(MiscMidPoint3A, HGF2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a very large segment
    ASSERT_NEAR(0.0, LargeSegment1A.CalculateAngularAcceleration(LargeMidPoint1A, HGF2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a very large segment
    ASSERT_NEAR(0.0, LargeSegment1A.CalculateAngularAcceleration(LargeMidPoint1A, HGF2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a inverted very large segment
    ASSERT_NEAR(0.0, LargeSegment2A.CalculateAngularAcceleration(LargeMidPoint1A, HGF2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a inverted very large segment
    ASSERT_NEAR(0.0, LargeSegment2A.CalculateAngularAcceleration(LargeMidPoint1A, HGF2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a segment way into negative coordinates
    ASSERT_NEAR(0.0, NegativeSegment1A.CalculateAngularAcceleration(NegativeMidPoint1A, HGF2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a segment way into negative coordinates
    ASSERT_NEAR(0.0, NegativeSegment1A.CalculateAngularAcceleration(NegativeMidPoint1A, HGF2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a inverted segment way into negative coordinates
    ASSERT_NEAR(0.0, NegativeSegment2A.CalculateAngularAcceleration(NegativeMidPoint1A, HGF2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a inverted segment way into negative coordinates
    ASSERT_NEAR(0.0, NegativeSegment2A.CalculateAngularAcceleration(NegativeMidPoint1A, HGF2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a segment way into positive values
    ASSERT_NEAR(0.0, PositiveSegment1A.CalculateAngularAcceleration(PositiveMidPoint1A, HGF2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a segment way into positive values
    ASSERT_NEAR(0.0, PositiveSegment1A.CalculateAngularAcceleration(PositiveMidPoint1A, HGF2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a inverted segment way into positive values
    ASSERT_NEAR(0.0, PositiveSegment2A.CalculateAngularAcceleration(PositiveMidPoint1A, HGF2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a inverted segment way into positive values
    ASSERT_NEAR(0.0, PositiveSegment2A.CalculateAngularAcceleration(PositiveMidPoint1A, HGF2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a NULL segment
    ASSERT_NEAR(0.0, NullSegment1A.CalculateAngularAcceleration(HGF2DPosition(0.0, 0.0), HGF2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a NULL segment
    ASSERT_NEAR(0.0, NullSegment1A.CalculateAngularAcceleration(HGF2DPosition(0.0, 0.0), HGF2DVector::ALPHA), MYEPSILON);

    }

//==================================================================================
// Extent calculation test
// GetExtent() const;
//==================================================================================
TEST_F (HGF2DSegmentTester, GetExtentTest)
    {

    // Obtain extent of a vertical segment
    ASSERT_DOUBLE_EQ(0.10, VerticalSegment1A.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.10, VerticalSegment1A.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(0.10, VerticalSegment1A.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(10.1, VerticalSegment1A.GetExtent().GetYMax());

    // Obtain extent of a inverted vertical segment
    ASSERT_DOUBLE_EQ(0.10, VerticalSegment2A.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.10, VerticalSegment2A.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(0.10, VerticalSegment2A.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(10.1, VerticalSegment2A.GetExtent().GetYMax());

    // Obtain extent of an horizontal segment
    ASSERT_DOUBLE_EQ(0.10, HorizontalSegment1A.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.10, HorizontalSegment1A.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(10.1, HorizontalSegment1A.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(0.10, HorizontalSegment1A.GetExtent().GetYMax());

    // Obtain extent of an inverted horizontal segment
    ASSERT_DOUBLE_EQ(0.10, HorizontalSegment2A.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.10, HorizontalSegment2A.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(10.1, HorizontalSegment2A.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(0.10, HorizontalSegment2A.GetExtent().GetYMax());

    // Obtain extent of a positive slope segment
    ASSERT_DOUBLE_EQ(0.10, MiscSegment1A.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.10, MiscSegment1A.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(10.1, MiscSegment1A.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(10.1, MiscSegment1A.GetExtent().GetYMax());

    // Obtain extent of a vertical EPSILON SIZED segment
    ASSERT_DOUBLE_EQ(0.1000000, VerticalSegment3A.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.1000000, VerticalSegment3A.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(0.1000000, VerticalSegment3A.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(0.1000001, VerticalSegment3A.GetExtent().GetYMax());

    // Obtain extent of a horizontal EPSILON SIZED segment
    ASSERT_DOUBLE_EQ(0.1000000, HorizontalSegment3A.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.1000000, HorizontalSegment3A.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(0.1000001, HorizontalSegment3A.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(0.1000000, HorizontalSegment3A.GetExtent().GetYMax());

    // Obtain extent of a miscaleniuous EPSILON SIZED segment
    ASSERT_DOUBLE_EQ(0.10000000000000000, MiscSegment3A.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.10000000000000000, MiscSegment3A.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(0.10000000000000000, VerticalSegment1A.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(0.10000009762960071, MiscSegment3A.GetExtent().GetYMax());

    // Obtain extent of a very large segment
    ASSERT_DOUBLE_EQ(-1.00E123, LargeSegment1A.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(-21.0E123, LargeSegment1A.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(9.000E123, LargeSegment1A.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(19.00E123, LargeSegment1A.GetExtent().GetYMax());

    // Obtain extent of a segment way into negative coordinates
    ASSERT_DOUBLE_EQ(-11.0E123, NegativeSegment1A.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(-41.0E123, NegativeSegment1A.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(-1.00E123, NegativeSegment1A.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(-21.0E123, NegativeSegment1A.GetExtent().GetYMax());

    // Obtain extent of a segment way into positive values
    ASSERT_DOUBLE_EQ(1.00E123, PositiveSegment1A.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(21.0E123, PositiveSegment1A.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(11.0E123, PositiveSegment1A.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(41.0E123, PositiveSegment1A.GetExtent().GetYMax());

    // Obtain extent of a NULL segment 
    ASSERT_NEAR(0.0, NullSegment1A.GetExtent().GetXMin(), MYEPSILON);
    ASSERT_NEAR(0.0, NullSegment1A.GetExtent().GetYMin(), MYEPSILON);
    ASSERT_NEAR(0.0, NullSegment1A.GetExtent().GetXMax(), MYEPSILON);
    ASSERT_NEAR(0.0, NullSegment1A.GetExtent().GetYMax(), MYEPSILON);

    }

//==================================================================================
// Rotate(const HGFAngle& pi_rAngle, const HGF2DPosition& pi_rOrigin);
//==================================================================================
TEST_F (HGF2DSegmentTester, RotateTest)
    {

    // Obtain rotate of a vertical segment
    VerticalSegment1A.Rotate(PI, HGF2DPosition(0.0, 0.0));
    ASSERT_DOUBLE_EQ(-0.10000000000000000, VerticalSegment1A.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.10000000000000000, VerticalSegment1A.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-0.10000000000000124, VerticalSegment1A.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-10.1000000000000000, VerticalSegment1A.GetEndPoint().GetY());

    // Obtain rotate of a inverted vertical segment
    VerticalSegment2A.Rotate(PI, HGF2DPosition(0.0, 0.0));
    ASSERT_DOUBLE_EQ(-0.10000000000000124, VerticalSegment2A.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-10.1000000000000000, VerticalSegment2A.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-0.10000000000000000, VerticalSegment2A.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.10000000000000000, VerticalSegment2A.GetEndPoint().GetY());

    // Obtain rotate of an horizontal segment
    HorizontalSegment1A.Rotate(PI, HGF2DPosition(0.0, 0.0));
    ASSERT_DOUBLE_EQ(-0.10000000000000000, HorizontalSegment1A.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.10000000000000000, HorizontalSegment1A.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-10.1000000000000000, HorizontalSegment1A.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.09999999999999877, HorizontalSegment1A.GetEndPoint().GetY());

    // Obtain rotate of an inverted horizontal segment
    HorizontalSegment2A.Rotate(PI, HGF2DPosition(0.0, 0.0));
    ASSERT_DOUBLE_EQ(-10.1000000000000000, HorizontalSegment2A.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.09999999999999877, HorizontalSegment2A.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-0.10000000000000000, HorizontalSegment2A.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.10000000000000000, HorizontalSegment2A.GetEndPoint().GetY());

    // Obtain rotate of a positive slope segment
    MiscSegment1A.Rotate(PI, HGF2DPosition(0.0, 0.0));
    ASSERT_DOUBLE_EQ(-0.10, MiscSegment1A.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.10, MiscSegment1A.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-10.1, MiscSegment1A.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-10.1, MiscSegment1A.GetEndPoint().GetY());

    // Obtain rotate of a vertical EPSILON SIZED segment
    VerticalSegment3A.Rotate(PI, HGF2DPosition(0.0, 0.0));
    ASSERT_DOUBLE_EQ(-0.10000000000000000, VerticalSegment3A.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.10000000000000000, VerticalSegment3A.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-0.10000000000000000, VerticalSegment3A.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.10000009999999999, VerticalSegment3A.GetEndPoint().GetY());

    // Obtain rotate of a horizontal EPSILON SIZED segment
    HorizontalSegment3A.Rotate(PI, HGF2DPosition(0.0, 0.0));
    ASSERT_DOUBLE_EQ(-0.10000000000000000, HorizontalSegment3A.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.10000000000000000, HorizontalSegment3A.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-0.10000009999999999, HorizontalSegment3A.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.10000000000000000, HorizontalSegment3A.GetEndPoint().GetY());

    // Obtain rotate of a very large segment
    LargeSegment1A.Rotate(PI, HGF2DPosition(0.0, 0.0));
    ASSERT_DOUBLE_EQ(1.00000000000000260E123, LargeSegment1A.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(2.10000000000000010E124, LargeSegment1A.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-8.9999999999999997E123, LargeSegment1A.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-1.8999999999999999E124, LargeSegment1A.GetEndPoint().GetY());

    // Obtain rotate of a segment way into negative coordinates
    NegativeSegment1A.Rotate(PI, HGF2DPosition(0.0, 0.0));
    ASSERT_DOUBLE_EQ(1.0000000000000026E123, NegativeSegment1A.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(2.1000000000000001E124, NegativeSegment1A.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(1.1000000000000001E124, NegativeSegment1A.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(4.1000000000000000E124, NegativeSegment1A.GetEndPoint().GetY());

    // Obtain rotate of a segment way into positive values
    PositiveSegment1A.Rotate(PI, HGF2DPosition(0.0, 0.0));
    ASSERT_DOUBLE_EQ(-1.0000000000000026E123, PositiveSegment1A.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-2.1000000000000001E124, PositiveSegment1A.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-1.1000000000000001E124, PositiveSegment1A.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-4.1000000000000000E124, PositiveSegment1A.GetEndPoint().GetY());

    // Obtain rotate of a NULL segment
    NullSegment1A.Rotate(PI, HGF2DPosition(0.0, 0.0));
    ASSERT_TRUE(NullSegment1.IsNull());

    }

//==================================================================================
// Additional tests for contiguousness who used to fail
//==================================================================================
TEST_F (HGF2DSegmentTester, ContiguousTestWhoFailed)
    {

    HGF2DSegment    HorizontalSegmentSup1(HGF2DPosition(10.0, 10.0), HGF2DPosition(20.0, 10.0));
    HGF2DSegment    HorizontalSegmentSup2(HGF2DPosition(20.0, 10.0), HGF2DPosition(35.0, 10.0));

    ASSERT_FALSE(HorizontalSegmentSup1.AreContiguous(HorizontalSegmentSup2));
    ASSERT_FALSE(HorizontalSegmentSup2.AreContiguous(HorizontalSegmentSup1));

    HGF2DSegment    VerticalSegmentSup1(HGF2DPosition(10.0, 10.0), HGF2DPosition(10.0, 20.0));
    HGF2DSegment    VerticalSegmentSup2(HGF2DPosition(10.0, 20.0), HGF2DPosition(10.0, 35.0));

    ASSERT_FALSE(VerticalSegmentSup1.AreContiguous(VerticalSegmentSup2));
    ASSERT_FALSE(VerticalSegmentSup2.AreContiguous(VerticalSegmentSup1));

    // Other test which failed
    // all of the following are contiguous
    HGF2DSegment    HorizontalSegmentSup3(HGF2DPosition(10.0, 10.0), HGF2DPosition(20.0, 10.0));
    HGF2DSegment    HorizontalSegmentSup4(HGF2DPosition(5.0, 10.0), HGF2DPosition(35.0, 10.0));
    HGF2DSegment    HorizontalSegmentSup5(HGF2DPosition(5.0, 10.0), HGF2DPosition(20.0, 10.0));
    HGF2DSegment    HorizontalSegmentSup6(HGF2DPosition(10.0, 10.0), HGF2DPosition(25.0, 10.0));
    HGF2DSegment    HorizontalSegmentSup7(HGF2DPosition(5.0, 10.0), HGF2DPosition(15.0, 10.0));
    HGF2DSegment    HorizontalSegmentSup8(HGF2DPosition(13.0, 10.0), HGF2DPosition(25.0, 10.0));
    HGF2DSegment    HorizontalSegmentSup9(HGF2DPosition(12.0, 10.0), HGF2DPosition(18.0, 10.0));

    HGF2DSegment    HorizontalSegmentSup10(HGF2DPosition(20.0, 10.0), HGF2DPosition(10.0, 10.0));
    HGF2DSegment    HorizontalSegmentSup11(HGF2DPosition(35.0, 10.0), HGF2DPosition(5.0, 10.0));
    HGF2DSegment    HorizontalSegmentSup12(HGF2DPosition(20.0, 10.0), HGF2DPosition(5.0, 10.0));
    HGF2DSegment    HorizontalSegmentSup13(HGF2DPosition(25.0, 10.0), HGF2DPosition(10.0, 10.0));
    HGF2DSegment    HorizontalSegmentSup14(HGF2DPosition(15.0, 10.0), HGF2DPosition(5.0, 10.0));
    HGF2DSegment    HorizontalSegmentSup15(HGF2DPosition(25.0, 10.0), HGF2DPosition(13.0, 10.0));
    HGF2DSegment    HorizontalSegmentSup16(HGF2DPosition(18.0, 10.0), HGF2DPosition(12.0, 10.0));

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


    // Other test which failed
    // all of the following are contiguous
    HGF2DSegment    VerticalSegmentSup3(HGF2DPosition(10.0, 10.0), HGF2DPosition(10.0, 20.0));
    HGF2DSegment    VerticalSegmentSup4(HGF2DPosition(10.0, 5.00), HGF2DPosition(10.0, 35.0));
    HGF2DSegment    VerticalSegmentSup5(HGF2DPosition(10.0, 5.00), HGF2DPosition(10.0, 20.0));
    HGF2DSegment    VerticalSegmentSup6(HGF2DPosition(10.0, 10.0), HGF2DPosition(10.0, 25.0));
    HGF2DSegment    VerticalSegmentSup7(HGF2DPosition(10.0, 5.00), HGF2DPosition(10.0, 15.0));
    HGF2DSegment    VerticalSegmentSup8(HGF2DPosition(10.0, 13.0), HGF2DPosition(10.0, 25.0));
    HGF2DSegment    VerticalSegmentSup9(HGF2DPosition(10.0, 12.0), HGF2DPosition(10.0, 18.0));

    HGF2DSegment    VerticalSegmentSup10(HGF2DPosition(10.0, 20.0), HGF2DPosition(10.0, 10.0));
    HGF2DSegment    VerticalSegmentSup11(HGF2DPosition(10.0, 35.0), HGF2DPosition(10.0, 5.00));
    HGF2DSegment    VerticalSegmentSup12(HGF2DPosition(10.0, 20.0), HGF2DPosition(10.0, 5.00));
    HGF2DSegment    VerticalSegmentSup13(HGF2DPosition(10.0, 25.0), HGF2DPosition(10.0, 10.0));
    HGF2DSegment    VerticalSegmentSup14(HGF2DPosition(10.0, 15.0), HGF2DPosition(10.0, 5.00));
    HGF2DSegment    VerticalSegmentSup15(HGF2DPosition(10.0, 25.0), HGF2DPosition(10.0, 13.0));
    HGF2DSegment    VerticalSegmentSup16(HGF2DPosition(10.0, 18.0), HGF2DPosition(10.0, 12.0));

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
TEST_F (HGF2DSegmentTester, ObtainContiguousnessPointsWhoFailed)
    {  
       
    HGF2DSegment    HorizontalSegmentSup23(HGF2DPosition(20.0, 20.0), HGF2DPosition(10.0, 20.0));
    HGF2DSegment    HorizontalSegmentSup24(HGF2DPosition(20.0+0.9*MYEPSILON, 20.0), HGF2DPosition(-1.0, 20.0));

    HGF2DPositionCollection     Contig23Points;
    ASSERT_EQ(2, HorizontalSegmentSup23.ObtainContiguousnessPoints(HorizontalSegmentSup24, &Contig23Points));

    }

//==================================================================================
// Another yet test which did fail (July 23 1997)
//==================================================================================
TEST_F (HGF2DSegmentTester, ObtainContiguousnessPointsWhoFailed2)
    {  
              
    HGF2DSegment    HorizontalSegmentSup1(HGF2DPosition(0.0, 0.0), HGF2DPosition(0.0, 1E-6));
    HGF2DSegment    HorizontalSegmentSup2(HGF2DPosition(0.0, 9.999999E-7), HGF2DPosition(0.0, 7.0));

    ASSERT_FALSE(HorizontalSegmentSup1.AreContiguous(HorizontalSegmentSup2));

    }

//==================================================================================
// Test which failed on Nov 15, 2000
//==================================================================================
TEST_F (HGF2DSegmentTester, CalculateLengthTestWhoFailed)
    {   

    HGF2DSegment MySegment(HGF2DPosition(410416.98821556, -3712392.8951395),
                            HGF2DPosition(410416.98823327, -3711942.8951395));

    HGF2DPosition IntermediatePoint(410416.98857254, -3712167.8951158);

    HGF2DPosition ClosestPoint = MySegment.CalculateClosestPoint(IntermediatePoint);

    ASSERT_TRUE((ClosestPoint - IntermediatePoint).CalculateLength() < 0.001);

    }

