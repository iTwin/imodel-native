//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HVE2DSegmentTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "EnvironnementTest.h"
#include "HVE2DSegmentTester.h"

//==================================================================================
// Segment Construction tests
// HVE2DSegment();
// HVE2DSegment(const HGF2DLocation&, const HGF2DLocation&);
// HVE2DSegment(const HGF2DLocation& pi_rStartPoint,
//             const HGF2DDisplacement& pi_rDisplacement);
// HVE2DSegment(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
// HVE2DSegment(const HVE2DSegment&    pi_rObject);
//==================================================================================
TEST_F (HVE2DSegmentTester, ConstructionTest)
    {

    // Default Constructor
    HVE2DSegment    Segment1;

    // Contructor by two points
    HGF2DLocation   FirstSegmentPoint(10.0, 10.2, pWorld);
    HGF2DLocation   SecondSegmentPoint(-10000.0, 100.3, pWorld);

    HVE2DSegment    Segment2(FirstSegmentPoint, SecondSegmentPoint);
    ASSERT_EQ(pWorld, Segment2.GetCoordSys());
    ASSERT_DOUBLE_EQ(10.00000, Segment2.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.20000, Segment2.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-10000.0, Segment2.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(100.3000, Segment2.GetEndPoint().GetY());

    HVE2DSegment    Segment3(FirstSegmentPoint, SecondSegmentPoint);
    ASSERT_EQ(pWorld, Segment3.GetCoordSys());
    ASSERT_DOUBLE_EQ(10.00000, Segment3.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.20000, Segment3.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-10000.0, Segment3.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(100.3000, Segment3.GetEndPoint().GetY());

    // Constructor by point and displacement
    HGF2DDisplacement   Displacement1(10.0, 10.0);
    HGF2DDisplacement   Displacement2(0.0, 10.0);
    HGF2DDisplacement   Displacement3(-10.0, -10.0);
    HGF2DDisplacement   Displacement4(0.0, 0.0);

    HVE2DSegment    Segment4(FirstSegmentPoint, Displacement1);
    ASSERT_EQ(pWorld, Segment4.GetCoordSys());
    ASSERT_DOUBLE_EQ(10.0, Segment4.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.2, Segment4.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, Segment4.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.2, Segment4.GetEndPoint().GetY());

    HVE2DSegment    Segment5(FirstSegmentPoint, Displacement2);
    ASSERT_EQ(pWorld, Segment5.GetCoordSys());
    ASSERT_DOUBLE_EQ(10.0, Segment5.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.2, Segment5.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, Segment5.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.2, Segment5.GetEndPoint().GetY());

    HVE2DSegment    Segment6(FirstSegmentPoint, Displacement3);
    ASSERT_EQ(pWorld, Segment6.GetCoordSys());
    ASSERT_DOUBLE_EQ(10.0000000000000000, Segment6.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.2000000000000000, Segment6.GetStartPoint().GetY());
    ASSERT_NEAR(0.0, Segment6.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.19999999999999929, Segment6.GetEndPoint().GetY());

    HVE2DSegment    Segment7(FirstSegmentPoint, Displacement4);
    ASSERT_EQ(pWorld, Segment7.GetCoordSys());
    ASSERT_DOUBLE_EQ(10.0, Segment7.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.2, Segment7.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, Segment7.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.2, Segment7.GetEndPoint().GetY());

    // Constructor with only coordinate system
    HVE2DSegment    Segment8(pSys1);
    ASSERT_EQ(Segment8.GetCoordSys(), pSys1);

    //Copy Constructor
    HVE2DSegment    Segment9(Segment4);
    ASSERT_EQ(pWorld, Segment9.GetCoordSys());
    ASSERT_DOUBLE_EQ(10.0, Segment9.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.2, Segment9.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, Segment9.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.2, Segment9.GetEndPoint().GetY());

    }

//==================================================================================
// operator= test
// operator=(const HVE2DSegment& pi_rObj);
//==================================================================================
TEST_F (HVE2DSegmentTester, OperatorTest)
    {
     
    // Test with different coord sys
    HGF2DLocation   FirstSegmentPoint(10.0, 10.2, pWorld);
    HGF2DLocation   SecondSegmentPoint(-10000.0, 100.3, pWorld);
    HVE2DSegment    Segment1(FirstSegmentPoint, SecondSegmentPoint);

    HVE2DSegment    Segment2(SecondSegmentPoint, FirstSegmentPoint);

    Segment2 = Segment1;
    ASSERT_EQ(pWorld, Segment2.GetCoordSys());
    ASSERT_DOUBLE_EQ(10.00000, Segment2.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.20000, Segment2.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-10000.0, Segment2.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(100.3000, Segment2.GetEndPoint().GetY());

    // Test with a NULL segment
    HVE2DSegment    Segment3(pSys1);
    HVE2DSegment    Segment4;

    Segment4 = Segment3;
    ASSERT_EQ(pSys1, Segment4.GetCoordSys());
    ASSERT_TRUE(Segment4.IsNull());

    }

//==================================================================================
// Segment coordinate setting test
// void               SetStartPoint(const HGF2DLocation& pi_rNewStartPoint);
// void               SetEndPoint(const HGF2DLocation& pi_rNewEndPoint);
// void               SetRawStartPoint(double pi_X, double pi_Y);
// void               SetRawEndPoint(double pi_X, double pi_Y);
//==================================================================================
TEST_F (HVE2DSegmentTester, CoordinateTest)
    {

    // Test set to same coordinates
    HGF2DLocation   FirstSegmentPoint(10.0, 10.2, pWorld);
    HGF2DLocation   SecondSegmentPoint(-10000.0, 100.3, pWorld);
    HVE2DSegment    Segment1(FirstSegmentPoint, SecondSegmentPoint);

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
    HVE2DSegment    Segment2(VerticalSegment1);

    Segment2.SetStartPoint(HGF2DLocation(23.5, 23.5, pWorld));
    ASSERT_DOUBLE_EQ(23.5, Segment2.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(23.5, Segment2.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment2.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment2.GetEndPoint().GetY());

    Segment2.SetEndPoint(HGF2DLocation(23.5, 23.5, pWorld));
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
    HVE2DSegment    Segment3(VerticalSegment2);

    Segment3.SetStartPoint(HGF2DLocation(23.5, 23.5, pWorld));
    ASSERT_DOUBLE_EQ(23.5, Segment3.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(23.5, Segment3.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment3.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment3.GetEndPoint().GetY());

    Segment3.SetEndPoint(HGF2DLocation(23.5, 23.5, pWorld));
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
TEST_F (HVE2DSegmentTester, CalculateLineTest)
    {

    // LINES
    HGF2DLine       Line1(HGF2DLocation(1.1, 0.1, pWorld), HGF2DLocation(-10.1, 10.1, pWorld));
    HGF2DLine       VerticalLine1C(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(0.1, 10.1 + MYEPSILON, pWorld));
    HGF2DLine       Line1P(HGF2DLocation(0.2, 0.1, pWorld), HGF2DLocation(10.2, 10.1, pWorld));
    HGF2DLine       Line1NP(HGF2DLocation(0.2, 0.1, pWorld), HGF2DLocation(10.2, 10.1+MYEPSILON, pWorld));
    HGF2DLine       Line1CNP(HGF2DLocation(0.1+MYEPSILON, 0.1, pWorld), HGF2DLocation(10.1-MYEPSILON, 10.1, pWorld));
    HGF2DLine       Line1CONNECTED(HGF2DLocation(0.21, 0.1, pWorld), HGF2DLocation(10.1, 10.1, pWorld));
    HGF2DLine       HorizontalLine1(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(10.1, 0.1, pWorld));
    HGF2DLine       InvalidLine(HGF2DLocation(0.1, 0.1, pNoLinear), HGF2DLocation(10.1, 0.1, pNoLinear));

    // Test with vertical segment
    HGF2DLine   DumLine1 = VerticalSegment1.CalculateLine();
    ASSERT_EQ(pWorld, DumLine1.GetCoordSys());
    ASSERT_TRUE(DumLine1.IsVertical());
    ASSERT_DOUBLE_EQ(0.1, DumLine1.GetIntercept());

    // Test with inverted vertical segment
    HGF2DLine   DumLine2 = VerticalSegment2.CalculateLine();
    ASSERT_EQ(pWorld, DumLine2.GetCoordSys());
    ASSERT_TRUE(DumLine2.IsVertical());
    ASSERT_DOUBLE_EQ(0.1, DumLine2.GetIntercept());

    // Test with horizontal segment
    HGF2DLine   DumLine3 = HorizontalSegment1.CalculateLine();
    ASSERT_EQ(pWorld, DumLine3.GetCoordSys());
    ASSERT_FALSE(DumLine3.IsVertical());
    ASSERT_DOUBLE_EQ(0.1, DumLine3.GetIntercept());
    ASSERT_NEAR(0.0, DumLine3.GetSlope(), MYEPSILON);

    // Test with inverted horizontal segment
    HGF2DLine   DumLine4 = HorizontalSegment2.CalculateLine();
    ASSERT_EQ(pWorld, DumLine4.GetCoordSys());
    ASSERT_FALSE(DumLine4.IsVertical());
    ASSERT_DOUBLE_EQ(0.1, DumLine4.GetIntercept());
    ASSERT_NEAR(0.0, DumLine4.GetSlope(), MYEPSILON);

    // Tests with vertical EPSILON sized segment
    HGF2DLine   DumLine5 = VerticalSegment3.CalculateLine();
    ASSERT_EQ(pWorld, DumLine5.GetCoordSys());
    ASSERT_TRUE(DumLine5.IsVertical());
    ASSERT_DOUBLE_EQ(0.1, DumLine5.GetIntercept());

    // Tests with horizontal EPSILON sized segment
    HGF2DLine   DumLine6 = HorizontalSegment3.CalculateLine();
    ASSERT_EQ(pWorld, DumLine6.GetCoordSys());
    ASSERT_FALSE(DumLine6.IsVertical());
    ASSERT_DOUBLE_EQ(0.1, DumLine6.GetIntercept());
    ASSERT_NEAR(0.0, DumLine6.GetSlope(), MYEPSILON);

    // Tests with miscalenious EPSILON size segment
    HGF2DLine   DumLine7 = MiscSegment3.CalculateLine();
    ASSERT_EQ(pWorld, DumLine7.GetCoordSys());
    ASSERT_FALSE(DumLine7.IsVertical());
    ASSERT_DOUBLE_EQ(-0.35107085049510683, DumLine7.GetIntercept());
    ASSERT_DOUBLE_EQ(4.510708504951068700, DumLine7.GetSlope());

    // Test with very large segment
    HGF2DLine   DumLine8 = LargeSegment1.CalculateLine();
    ASSERT_EQ(pWorld, DumLine8.GetCoordSys());
    ASSERT_FALSE(DumLine8.IsVertical());
    ASSERT_DOUBLE_EQ(-1.7E124, DumLine8.GetIntercept());
    ASSERT_DOUBLE_EQ(4.000000, DumLine8.GetSlope());

    // Test with segments way into positive regions
    HGF2DLine   DumLine9 = PositiveSegment1.CalculateLine();
    ASSERT_EQ(pWorld, DumLine9.GetCoordSys());
    ASSERT_FALSE(DumLine9.IsVertical());
    ASSERT_DOUBLE_EQ(1.9E124, DumLine9.GetIntercept());
    ASSERT_DOUBLE_EQ(2.00000, DumLine9.GetSlope());

    // Test with segments way into negative regions
    HGF2DLine   DumLine10 = NegativeSegment1.CalculateLine();
    ASSERT_EQ(pWorld, DumLine10.GetCoordSys());
    ASSERT_FALSE(DumLine10.IsVertical());
    ASSERT_DOUBLE_EQ(-1.9E124, DumLine10.GetIntercept());
    ASSERT_DOUBLE_EQ(2.000000, DumLine10.GetSlope());

    // Test with a NULL segments
    HGF2DLine   DumLine11 = NullSegment1.CalculateLine();
    ASSERT_EQ(pWorld, DumLine11.GetCoordSys());
    ASSERT_TRUE(DumLine11.IsVertical());

    } 

//==================================================================================
// Line intersection test
// IntersectLine(const HGF2DLine& pi_rLine,HGF2DLocation* po_pPoint)const;
//==================================================================================
TEST_F (HVE2DSegmentTester, IntersectLineTest)
    {

    // LINES
    HGF2DLine       Line1(HGF2DLocation(1.1, 0.1, pWorld), HGF2DLocation(-10.1, 10.1, pWorld));
    HGF2DLine       VerticalLine1C(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(0.1, 10.1 + MYEPSILON, pWorld));
    HGF2DLine       Line1P(HGF2DLocation(0.2, 0.1, pWorld), HGF2DLocation(10.2, 10.1, pWorld));
    HGF2DLine       Line1NP(HGF2DLocation(0.2, 0.1, pWorld), HGF2DLocation(10.2, 10.1+MYEPSILON, pWorld));
    HGF2DLine       Line1CNP(HGF2DLocation(0.1+MYEPSILON, 0.1, pWorld), HGF2DLocation(10.1-MYEPSILON, 10.1, pWorld));
    HGF2DLine       Line1CONNECTED(HGF2DLocation(0.21, 0.1, pWorld), HGF2DLocation(10.1, 10.1, pWorld));
    HGF2DLine       HorizontalLine1(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(10.1, 0.1, pWorld));
    HGF2DLine       InvalidLine1(HGF2DLocation(0.1, 0.1, pNoLinear), HGF2DLocation(10.1, 0.1, pNoLinear));

    HGF2DLocation   DumPoint;

    // Test with vertical segment
    ASSERT_EQ(HVE2DSegment::CROSS_FOUND, VerticalSegment1.IntersectLine(Line1, &DumPoint));
    ASSERT_DOUBLE_EQ(0.1, DumPoint.GetPosition().GetX());

    // Test with inverted vertical segment
    ASSERT_EQ(HVE2DSegment::CROSS_FOUND, VerticalSegment2.IntersectLine(Line1, &DumPoint));
    ASSERT_DOUBLE_EQ(0.1, DumPoint.GetPosition().GetX());

    // Test with close vertical segments
    ASSERT_EQ(HVE2DSegment::PARALLEL, VerticalSegment1.IntersectLine(VerticalLine1C, &DumPoint));

    // Test with horizontal segment
    ASSERT_EQ(HVE2DSegment::CROSS_FOUND, HorizontalSegment1.IntersectLine(Line1, &DumPoint));
    ASSERT_DOUBLE_EQ(0.1, DumPoint.GetPosition().GetY()); 

    // Test with horizontal line
    ASSERT_EQ(HVE2DSegment::PARALLEL, HorizontalSegment1.IntersectLine(HorizontalLine1, &DumPoint));

    // Test with inverted horizontal segment
    ASSERT_EQ(HVE2DSegment::CROSS_FOUND, HorizontalSegment2.IntersectLine(Line1, &DumPoint));
    ASSERT_DOUBLE_EQ(0.1, DumPoint.GetPosition().GetY()); 

    // Test with parallel segment
    ASSERT_EQ(HVE2DSegment::PARALLEL, MiscSegment1.IntersectLine(Line1P, &DumPoint));

    // Test with near parallel segments
    ASSERT_NE(HVE2DSegment::PARALLEL, MiscSegment1.IntersectLine(Line1NP, &DumPoint));

    // Tests with close near parallel segment
    ASSERT_NE(HVE2DSegment::PARALLEL, MiscSegment1.IntersectLine(Line1CNP, &DumPoint));

    // Tests with connected segment
    ASSERT_EQ(HVE2DSegment::NO_CROSS, MiscSegment1.IntersectLine(Line1CONNECTED, &DumPoint));

    // Tests with vertical EPSILON sized segment
    ASSERT_EQ(HVE2DSegment::NO_CROSS, VerticalSegment3.IntersectLine(Line1, &DumPoint));

    // Tests with horizontal EPSILON sized segment
    ASSERT_EQ(HVE2DSegment::NO_CROSS, HorizontalSegment3.IntersectLine(Line1, &DumPoint));

    // Tests with miscalenious EPSILON size segment
    ASSERT_EQ(HVE2DSegment::NO_CROSS, MiscSegment3.IntersectLine(Line1, &DumPoint));

    // Test with very large segment
    ASSERT_EQ(HVE2DSegment::CROSS_FOUND, LargeSegment1.IntersectLine(Line1, &DumPoint));

    // Test with segments way into positive regions
    ASSERT_EQ(HVE2DSegment::NO_CROSS, PositiveSegment1.IntersectLine(Line1, &DumPoint));

    // Test with segments way into negative regions
    ASSERT_EQ(HVE2DSegment::NO_CROSS, NegativeSegment1.IntersectLine(Line1, &DumPoint));

    // Test with a NULL segment
    ASSERT_EQ(HVE2DSegment::NO_CROSS, NullSegment1.IntersectLine(Line1, &DumPoint));

    }

//==================================================================================
// Segment intersection test
// IntersectSegment(const HVE2DSegment& pi_rSegment,HGF2DLocation* po_pPoint)const;
// IntersectSegmentSCS( const HVE2DSegment& pi_rSegment, HGF2DLocation* po_pPoint ) const
//==================================================================================
TEST_F (HVE2DSegmentTester, IntersectSegmentTest)
    {

    HGF2DLocation   DumPoint;

    // Test with extent disjoint segments
    ASSERT_EQ(HVE2DSegment::NO_CROSS, MiscSegment1.IntersectSegment(DisjointSegment1, &DumPoint));

    // Test with disjoint but touching by a side segments
    ASSERT_EQ(HVE2DSegment::PARALLEL, MiscSegment1.IntersectSegment(ContiguousExtentSegment1, &DumPoint));

    // Test with disjoint but touching by a tip segments but not linked
    ASSERT_EQ(HVE2DSegment::NO_CROSS, MiscSegment1.IntersectSegment(FlirtingExtentSegment1, &DumPoint));

    // Test with disjoint but touching by a tip segments linked
    ASSERT_EQ(HVE2DSegment::NO_CROSS, MiscSegment1.IntersectSegment(FlirtingExtentLinkedSegment1, &DumPoint));

    // Test with vertical segment
    ASSERT_EQ(HVE2DSegment::NO_CROSS, VerticalSegment1.IntersectSegment(MiscSegment1, &DumPoint));

    // Test with inverted vertical segment
    ASSERT_EQ(HVE2DSegment::NO_CROSS, VerticalSegment2.IntersectSegment(MiscSegment1, &DumPoint));

    // Test with close vertical segments
    ASSERT_EQ(HVE2DSegment::PARALLEL, VerticalSegment1.IntersectSegment(VerticalSegment4, &DumPoint));

    // Test with horizontal segment
    ASSERT_EQ(HVE2DSegment::NO_CROSS, HorizontalSegment1.IntersectSegment(MiscSegment1, &DumPoint));

    // Test with inverted horizontal segment
    ASSERT_EQ(HVE2DSegment::NO_CROSS, HorizontalSegment2.IntersectSegment(MiscSegment1, &DumPoint));

    // Test with parallel segments
    ASSERT_EQ(HVE2DSegment::PARALLEL, MiscSegment1.IntersectSegment(ParallelSegment1, &DumPoint));

    // Test with near parallel segments
    ASSERT_NE(HVE2DSegment::PARALLEL, MiscSegment1.IntersectSegment(NearParallelSegment1, &DumPoint));

    // Tests with close near parallel segments
    ASSERT_NE(HVE2DSegment::PARALLEL, MiscSegment1.IntersectSegment(CloseNearParallelSegment1, &DumPoint));

    // Tests with connected segments
    // At start point...
    ASSERT_EQ(HVE2DSegment::NO_CROSS, MiscSegment1.IntersectSegment(ConnectedSegment1, &DumPoint));
    ASSERT_EQ(HVE2DSegment::NO_CROSS, MiscSegment1.IntersectSegment(ConnectingSegment1, &DumPoint));

    // At end point ...
    ASSERT_EQ(HVE2DSegment::NO_CROSS, MiscSegment1.IntersectSegment(ConnectedSegment1A, &DumPoint));
    ASSERT_EQ(HVE2DSegment::NO_CROSS, MiscSegment1.IntersectSegment(ConnectingSegment1A, &DumPoint));

    // Tests with linked segments
    ASSERT_EQ(HVE2DSegment::NO_CROSS, MiscSegment1.IntersectSegment(LinkedSegment1, &DumPoint));
    ASSERT_EQ(HVE2DSegment::NO_CROSS, MiscSegment1.IntersectSegment(LinkedSegment1A, &DumPoint));

    // Tests with vertical EPSILON sized segment
    ASSERT_EQ(HVE2DSegment::NO_CROSS, VerticalSegment3.IntersectSegment(MiscSegment1, &DumPoint));

    // Tests with horizontal EPSILON sized segment
    ASSERT_EQ(HVE2DSegment::NO_CROSS, HorizontalSegment3.IntersectSegment(MiscSegment1, &DumPoint));

    // Tests with miscalenious EPSILON size segment
    ASSERT_EQ(HVE2DSegment::NO_CROSS, MiscSegment3.IntersectSegment(MiscSegment1, &DumPoint));

    // Test with very large segment
    ASSERT_EQ(HVE2DSegment::NO_CROSS, LargeSegment1.IntersectSegment(MiscSegment1, &DumPoint));

    // Test with segments way into positive regions
    ASSERT_EQ(HVE2DSegment::NO_CROSS, PositiveSegment1.IntersectSegment(MiscSegment1, &DumPoint));

    // Test with segments way into negative regions
    ASSERT_EQ(HVE2DSegment::NO_CROSS, NegativeSegment1.IntersectSegment(MiscSegment1, &DumPoint));

    // Test with a NULL segment
    ASSERT_EQ(HVE2DSegment::NO_CROSS, NullSegment1.IntersectSegment(MiscSegment1, &DumPoint));

    }

//==================================================================================
// IntersectSegmentExtremityIncludedSCS(const HVE2DSegment& pi_rSegment,HGF2DLocation* po_pPoint, bool* po_pIntersectsAtExtremity) const
//==================================================================================
TEST_F (HVE2DSegmentTester, IntersectSegmentExtremityIncludedSCSTest)
    {

    HGF2DLocation   DumPoint;

    // Test with extent disjoint segments
    ASSERT_EQ(HVE2DSegment::NO_CROSS, MiscSegment1.IntersectSegmentSCS(DisjointSegment1, &DumPoint));

    // Test with disjoint but touching by a side segments
    ASSERT_EQ(HVE2DSegment::NO_CROSS, MiscSegment1.IntersectSegmentSCS(ContiguousExtentSegment1, &DumPoint));

    // Tests with connected segments
    // At start point...
    ASSERT_EQ(HVE2DSegment::CROSS_FOUND, MiscSegment1.IntersectSegmentExtremityIncludedSCS(ConnectedSegment1, &DumPoint));
    ASSERT_EQ(HVE2DSegment::CROSS_FOUND, MiscSegment1.IntersectSegmentExtremityIncludedSCS(ConnectingSegment1, &DumPoint));

    // At end point ...
    ASSERT_EQ(HVE2DSegment::CROSS_FOUND, MiscSegment1.IntersectSegmentExtremityIncludedSCS(ConnectedSegment1A, &DumPoint));
    ASSERT_EQ(HVE2DSegment::CROSS_FOUND, MiscSegment1.IntersectSegmentExtremityIncludedSCS(ConnectingSegment1A, &DumPoint));

    // Tests with linked segments
    ASSERT_EQ(HVE2DSegment::CROSS_FOUND, MiscSegment1.IntersectSegmentExtremityIncludedSCS(LinkedSegment1, &DumPoint));
    ASSERT_EQ(HVE2DSegment::CROSS_FOUND, MiscSegment1.IntersectSegmentExtremityIncludedSCS(LinkedSegment1A, &DumPoint));

    // Tests with vertical EPSILON sized segment
    ASSERT_EQ(HVE2DSegment::NO_CROSS, VerticalSegment3.IntersectSegmentExtremityIncludedSCS(MiscSegment1, &DumPoint));

    // Tests with horizontal EPSILON sized segment
    ASSERT_EQ(HVE2DSegment::NO_CROSS, HorizontalSegment3.IntersectSegmentExtremityIncludedSCS(MiscSegment1, &DumPoint));

    // Tests with miscalenious EPSILON size segment
    ASSERT_EQ(HVE2DSegment::NO_CROSS, MiscSegment3.IntersectSegmentExtremityIncludedSCS(MiscSegment1, &DumPoint));

    // Test with very large segment
    ASSERT_EQ(HVE2DSegment::NO_CROSS, LargeSegment1.IntersectSegmentExtremityIncludedSCS(MiscSegment1, &DumPoint));

    // Test with segments way into positive regions
    ASSERT_EQ(HVE2DSegment::NO_CROSS, PositiveSegment1.IntersectSegmentExtremityIncludedSCS(MiscSegment1, &DumPoint));

    // Test with segments way into negative regions
    ASSERT_EQ(HVE2DSegment::NO_CROSS, NegativeSegment1.IntersectSegmentExtremityIncludedSCS(MiscSegment1, &DumPoint));

    // Test with a NULL segment
    ASSERT_EQ(HVE2DSegment::NO_CROSS, NullSegment1.IntersectSegmentExtremityIncludedSCS(MiscSegment1, &DumPoint));

    }

//==================================================================================
// Parallelitude test
// IsParallelTo(const HVE2DSegment& pi_rSegment) const;
// IsParallelTo(const HGF2DLine& pi_rLine) const;
//==================================================================================
TEST_F (HVE2DSegmentTester, ParallelitudeTest)
    {

    // LINES
    HGF2DLine       Line1(HGF2DLocation(1.1, 0.1, pWorld), HGF2DLocation(-10.1, 10.1, pWorld));
    HGF2DLine       VerticalLine1C(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(0.1, 10.1 + MYEPSILON, pWorld));
    HGF2DLine       Line1P(HGF2DLocation(0.2, 0.1, pWorld), HGF2DLocation(10.2, 10.1, pWorld));
    HGF2DLine       Line1NP(HGF2DLocation(0.2, 0.1, pWorld), HGF2DLocation(10.2, 10.1+MYEPSILON, pWorld));
    HGF2DLine       Line1CNP(HGF2DLocation(0.1+MYEPSILON, 0.1, pWorld), HGF2DLocation(10.1-MYEPSILON, 10.1, pWorld));
    HGF2DLine       Line1CONNECTED(HGF2DLocation(0.21, 0.1, pWorld), HGF2DLocation(10.1, 10.1, pWorld));
    HGF2DLine       HorizontalLine1(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(10.1, 0.1, pWorld));

    // Test with segments

    // Test with extent disjoint segments
    ASSERT_FALSE(MiscSegment1.IsParallelTo(DisjointSegment1));

    // Test with vertical segment
    ASSERT_TRUE(VerticalSegment1.IsParallelTo(VerticalSegment5));
    ASSERT_FALSE(VerticalSegment1.IsParallelTo(MiscSegment1));

    // Test with inverted vertical segment
    ASSERT_TRUE(VerticalSegment2.IsParallelTo(VerticalSegment5));
    ASSERT_FALSE(VerticalSegment2.IsParallelTo(MiscSegment1));

    // Test with close vertical segments
    ASSERT_TRUE(VerticalSegment1.IsParallelTo(CloseVerticalSegment1));

    // Test with horizontal segment
    ASSERT_TRUE(HorizontalSegment1.IsParallelTo(HorizontalSegment5));
    ASSERT_FALSE(HorizontalSegment1.IsParallelTo(MiscSegment1));

    // Test with inverted horizontal segment
    ASSERT_TRUE(HorizontalSegment2.IsParallelTo(HorizontalSegment5));
    ASSERT_FALSE(HorizontalSegment2.IsParallelTo(MiscSegment1));

    // Test with near parallel segments
    ASSERT_TRUE(MiscSegment1.IsParallelTo(NearParallelSegment1));

    // Tests with close near parallel segments
    ASSERT_TRUE(MiscSegment1.IsParallelTo(CloseNearParallelSegment1));

    // Tests with connected segments
    ASSERT_FALSE(MiscSegment1.IsParallelTo(ConnectedSegment1));
    ASSERT_FALSE(MiscSegment1.IsParallelTo(ConnectingSegment1));

    // Tests with linked segments
    ASSERT_FALSE(MiscSegment1.IsParallelTo(LinkedSegment1));
    ASSERT_TRUE(MiscSegment1.IsParallelTo(LinkedParallelSegment1));

    // Tests with vertical EPSILON sized segment
    ASSERT_TRUE(VerticalSegment2.IsParallelTo(VerticalSegment5));
    ASSERT_FALSE(VerticalSegment2.IsParallelTo(MiscSegment1));

    // Tests with horizontal EPSILON sized segment
    ASSERT_TRUE(HorizontalSegment2.IsParallelTo(HorizontalSegment5));
    ASSERT_FALSE(HorizontalSegment2.IsParallelTo(MiscSegment1));

    // Tests with miscalenious EPSILON size segment
    ASSERT_TRUE(MiscSegment3.IsParallelTo(MiscSegment3A));
    ASSERT_FALSE(MiscSegment3.IsParallelTo(MiscSegment1));

    // Test with very large segment
    ASSERT_TRUE(LargeSegment1.IsParallelTo(ParallelLargeSegment1));
    ASSERT_FALSE(LargeSegment1.IsParallelTo(MiscSegment1));

    // Test with segments way into positive regions
    ASSERT_TRUE(PositiveSegment1.IsParallelTo(ParallelPositiveSegment1));
    ASSERT_FALSE(PositiveSegment1.IsParallelTo(MiscSegment1));

    // Test with segments way into negative regions
    ASSERT_TRUE(NegativeSegment1.IsParallelTo(ParallelNegativeSegment1));
    ASSERT_FALSE(NegativeSegment1.IsParallelTo(MiscSegment1));

    // Test with a NULL segment
    ASSERT_TRUE(NullSegment1.IsParallelTo(NullSegment2));
    ASSERT_FALSE(NullSegment1.IsParallelTo(MiscSegment1));

    // Test with a line

    // Test with vertical segment
    ASSERT_FALSE(VerticalSegment1.IsParallelTo(Line1));

    // Test with inverted vertical segment
    ASSERT_FALSE(VerticalSegment2.IsParallelTo(Line1));

    // Test with close vertical segments
    ASSERT_FALSE(VerticalSegment1.IsParallelTo(Line1P));

    // Test with horizontal segment
    ASSERT_FALSE(HorizontalSegment1.IsParallelTo(Line1));

    // Test with inverted horizontal segment
    ASSERT_FALSE(HorizontalSegment2.IsParallelTo(Line1));

    // Test with parallel segment
    ASSERT_TRUE(MiscSegment1.IsParallelTo(Line1P));

    // Test with near parallel segment
    ASSERT_TRUE(MiscSegment1.IsParallelTo(Line1NP));

    // Tests with close near parallel segments
    ASSERT_TRUE(MiscSegment1.IsParallelTo(Line1CNP));

    // Tests with connected segment
    ASSERT_FALSE(MiscSegment1.IsParallelTo(Line1CONNECTED));

    // Tests with vertical EPSILON sized segment
    ASSERT_FALSE(VerticalSegment3.IsParallelTo(Line1));

    // Tests with horizontal EPSILON sized segment
    ASSERT_FALSE(HorizontalSegment3.IsParallelTo(Line1));

    // Tests with miscalenious EPSILON size segment
    ASSERT_FALSE(MiscSegment3.IsParallelTo(Line1));

    // Test with very large segment
    ASSERT_FALSE(LargeSegment1.IsParallelTo(Line1));

    // Test with segments way into positive regions
    ASSERT_FALSE(PositiveSegment1.IsParallelTo(Line1));

    // Test with segments way into negative regions
    ASSERT_FALSE(NegativeSegment1.IsParallelTo(Line1));

    // Test with a NULL segment
    ASSERT_FALSE(NullSegment1.IsParallelTo(Line1));

    }

//==================================================================================
// Type extraction test
// GetType() const;
//==================================================================================
TEST_F (HVE2DSegmentTester, GetTypeTest)
    {

    // Basic test
    ASSERT_EQ(HVE2DSegment::CLASS_ID, MiscSegment1.GetBasicLinearType());
    ASSERT_EQ(HVE2DSegment::CLASS_ID, VerticalSegment1.GetBasicLinearType());

    }

//==================================================================================
// Length calculation test
// CalculateLength() const;
//==================================================================================
TEST_F (HVE2DSegmentTester, CalculateLengthTest)
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
TEST_F (HVE2DSegmentTester, CalculateRelativePointTest)
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
// CalculateRelativePosition(const HGF2DLocation& pi_rPointOnLinear) const;
//==================================================================================
TEST_F (HVE2DSegmentTester, CalculateRelativePositionTest)
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
// CalculateRayArea(const HGF2DLocation& pi_rPoint) const;
//==================================================================================
TEST_F (HVE2DSegmentTester, CalculateRayArea) 
    {

    // Test with vertical segment
    ASSERT_DOUBLE_EQ(0.505, VerticalSegment1.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));

    // Test with inverted vertical segment
    ASSERT_DOUBLE_EQ(0.005, VerticalSegment2.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));

    // Test with horizontal segment
    ASSERT_DOUBLE_EQ(0.005, HorizontalSegment1.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));

    // Test with inverted horizontal segment
    ASSERT_DOUBLE_EQ(0.505, HorizontalSegment2.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));

    // Tests with vertical EPSILON sized segment
    ASSERT_DOUBLE_EQ(0.0050000050000000009, VerticalSegment3.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));

    // Tests with horizontal EPSILON sized segment
    ASSERT_DOUBLE_EQ(0.005, HorizontalSegment3.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));

    // Tests with miscalenious EPSILON size segment
    ASSERT_DOUBLE_EQ(0.0050000048814800363, MiscSegment3.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));

    // Test with very large segment
    ASSERT_DOUBLE_EQ(-9.4999999999999990e+246, LargeSegment1.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));

    // Test with segments way into positive regions
    ASSERT_DOUBLE_EQ(2.0500000000000001e+247, PositiveSegment1.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));
     
    // Test with segments way into negative regions
    ASSERT_DOUBLE_EQ(2.0500000000000001e+247, NegativeSegment1.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));
    
    }

//==================================================================================
// Shortening tests
// Shorten(double pi_StartRelativePos, double pi_EndRelativePos);
// Shorten(const HGF2DLocation& pi_rNewStartPoint,
//         const HGF2DLocation& pi_rNewEndPoint);
// ShortenTo(const HGF2DLocation& pi_rNewEndPoint);
// ShortenTo(double pi_EndRelativePosition);
// ShortenFrom(const HGF2DLocation& pi_rNewStartPoint);
// ShortenFrom(double pi_StartRelativePosition);
//==================================================================================
TEST_F (HVE2DSegmentTester,ShorteningTest)
    {

    // Test with vertical segment
    HVE2DSegment    Segment1(VerticalSegment1);

    Segment1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(2.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(8.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1;
    Segment1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.1000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(5.1000009999999989, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1;
    Segment1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(8.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1;
    Segment1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(2.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1;
    Segment1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1;
    Segment1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(8.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1;
    Segment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.099999, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1;
    Segment1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1;
    Segment1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1;
    Segment1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(2.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1;
    Segment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.099999, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.100000, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1;
    Segment1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1;
    Segment1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1;
    Segment1.ShortenFrom(VerticalMidPoint1);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1;
    Segment1.ShortenFrom(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1;
    Segment1.ShortenFrom(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1;
    Segment1.ShortenTo(VerticalMidPoint1);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(5.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1;
    Segment1.ShortenTo(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1;
    Segment1.ShortenTo(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1;
    Segment1.Shorten(Segment1.GetStartPoint(), VerticalMidPoint1);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(5.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1;
    Segment1.Shorten(VerticalMidPoint1, Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment1;
    Segment1.Shorten(Segment1.GetStartPoint(), Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    // Test with inverted vertical segment
    Segment1 = VerticalSegment2;
    Segment1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(8.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(2.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2;
    Segment1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.1000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(5.0999990000000004, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2;
    Segment1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(2.10, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2;
    Segment1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(8.100000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.099999999999999645, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2;
    Segment1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.10000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.099999999999999645, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2;
    Segment1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(2.10, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2;
    Segment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.100000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000009999999989, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2;
    Segment1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.10000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.099999999999999645, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2;
    Segment1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2;
    Segment1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(8.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2;
    Segment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000009999999989, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2;
    Segment1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.099999999999999645, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2;
    Segment1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2;
    Segment1.ShortenFrom(VerticalMidPoint1);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2;
    Segment1.ShortenFrom(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2;
    Segment1.ShortenFrom(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2;
    Segment1.ShortenTo(VerticalMidPoint1);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(5.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2;
    Segment1.ShortenTo(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2;
    Segment1.ShortenTo(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2;
    Segment1.Shorten(Segment1.GetStartPoint(), VerticalMidPoint1);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(5.10, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2;
    Segment1.Shorten(VerticalMidPoint1, Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment2;
    Segment1.Shorten(Segment1.GetStartPoint(), Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    // Test with horizontal segment
    Segment1 = HorizontalSegment1;
    Segment1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(2.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(8.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1;
    Segment1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_DOUBLE_EQ(5.1000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.1000009999999989, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1;
    Segment1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(8.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1;
    Segment1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(2.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1;
    Segment1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1;
    Segment1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(8.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1;
    Segment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.099999, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1;
    Segment1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1;
    Segment1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1;
    Segment1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(2.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1;
    Segment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(10.099999, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.100000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1;
    Segment1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1;
    Segment1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1;
    Segment1.ShortenFrom(HorizontalMidPoint1);
    ASSERT_DOUBLE_EQ(5.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1;
    Segment1.ShortenFrom(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1;
    Segment1.ShortenFrom(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1;
    Segment1.ShortenTo(HorizontalMidPoint1);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1;
    Segment1.ShortenTo(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1;
    Segment1.ShortenTo(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1;
    Segment1.Shorten(Segment1.GetStartPoint(), HorizontalMidPoint1);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1;
    Segment1.Shorten(HorizontalMidPoint1, Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(5.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment1;
    Segment1.Shorten(Segment1.GetStartPoint(), Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    // Test with inverted horizontal segment
    Segment1 = HorizontalSegment2;
    Segment1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(8.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(2.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2;
    Segment1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_DOUBLE_EQ(5.100000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.099999, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2;
    Segment1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(2.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2;
    Segment1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(8.100000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.099999999999999645, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2;
    Segment1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(10.10000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.099999999999999645, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2;
    Segment1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(2.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2;
    Segment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(10.100000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000009999999989, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2;
    Segment1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(10.10000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.099999999999999645, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2;
    Segment1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2;
    Segment1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(8.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2;
    Segment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1000009999999989, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000000000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2;
    Segment1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(0.099999999999999645, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2;
    Segment1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2;
    Segment1.ShortenFrom(HorizontalMidPoint1);
    ASSERT_DOUBLE_EQ(5.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2;
    Segment1.ShortenFrom(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2;
    Segment1.ShortenFrom(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2;
    Segment1.ShortenTo(HorizontalMidPoint1);
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2;
    Segment1.ShortenTo(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2;
    Segment1.ShortenTo(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2;
    Segment1.Shorten(Segment1.GetStartPoint(), HorizontalMidPoint1);
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2;
    Segment1.Shorten(HorizontalMidPoint1, Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(5.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment2;
    Segment1.Shorten(Segment1.GetStartPoint(), Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(10.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, Segment1.GetEndPoint().GetY());

    // Tests with vertical EPSILON sized segment
    Segment1 = VerticalSegment3;
    Segment1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000002, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000008, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3;
    Segment1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000005000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000005000001001, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3;
    Segment1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000008, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3;
    Segment1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000002, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000010, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3;
    Segment1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3;
    Segment1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000008, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3;
    Segment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009999999, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3;
    Segment1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3;
    Segment1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3;
    Segment1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000002, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000010, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3;
    Segment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009999999, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000010000000, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3;
    Segment1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3;
    Segment1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3;
    Segment1.ShortenFrom(VerticalMidPoint3);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000005, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000010, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3;
    Segment1.ShortenFrom(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3;
    Segment1.ShortenFrom(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3;
    Segment1.ShortenTo(VerticalMidPoint3);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000005, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3;
    Segment1.ShortenTo(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());
    
    Segment1 = VerticalSegment3;
    Segment1.ShortenTo(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3;
    Segment1.Shorten(Segment1.GetStartPoint(), VerticalMidPoint3);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000005, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3;
    Segment1.Shorten(VerticalMidPoint3, Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000005, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000010, Segment1.GetEndPoint().GetY());

    Segment1 = VerticalSegment3;
    Segment1.Shorten(Segment1.GetStartPoint(), Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetY());

    // Tests with horizontal EPSILON sized segment
    Segment1 = HorizontalSegment3;
    Segment1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(0.10000002, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000008, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3;
    Segment1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000005000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000005000001, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3;
    Segment1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000008, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3;
    Segment1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(0.10000002, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000010, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3;
    Segment1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000010, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3;
    Segment1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000008, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3;
    Segment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000009999999, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3;
    Segment1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3;
    Segment1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3;
    Segment1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(0.10000002, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000010, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetY());
    
    Segment1 = HorizontalSegment3;
    Segment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000009999999, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000010000000, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3;
    Segment1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3;
    Segment1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3;
    Segment1.ShortenFrom(HorizontalMidPoint3);
    ASSERT_DOUBLE_EQ(0.10000005, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000010, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3;
    Segment1.ShortenFrom(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3;
    Segment1.ShortenFrom(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3;
    Segment1.ShortenTo(HorizontalMidPoint3);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000005, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3;
    Segment1.ShortenTo(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3;
    Segment1.ShortenTo(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3;
    Segment1.Shorten(Segment1.GetStartPoint(), HorizontalMidPoint3);
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000005, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3;
    Segment1.Shorten(HorizontalMidPoint3, Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10000005, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000010, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, Segment1.GetEndPoint().GetY());

    Segment1 = HorizontalSegment3;
    Segment1.Shorten(Segment1.GetStartPoint(), Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000001, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Segment1.GetEndPoint().GetY());

    // Tests with miscalenious EPSILON size segment
    Segment1 = MiscSegment3;
    Segment1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(0.10000000432879229, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000001952592015, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000001731516911, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000007810368057, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3;
    Segment1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000001082198071, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000004881480036, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000001082198286, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000004881481012, Segment1.GetEndPoint().GetY());
    
    Segment1 = MiscSegment3;
    Segment1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000001731516911, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000007810368057, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3;
    Segment1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(0.10000000432879229, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000001952592015, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3;
    Segment1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3;
    Segment1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000001731516911, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000007810368057, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3;
    Segment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164395923, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762959096, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3;
    Segment1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3;
    Segment1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3;
    Segment1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(0.10000000432879229, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000001952592015, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3;
    Segment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000002164395923, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762959096, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3;
    Segment1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3;
    Segment1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3;
    Segment1.ShortenFrom(MiscMidPoint3);
    ASSERT_DOUBLE_EQ(0.099999974681068096, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000004311556202, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3;
    Segment1.ShortenFrom(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3;
    Segment1.ShortenFrom(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3;
    Segment1.ShortenTo(MiscMidPoint3);
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.099999974681068096, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000004311556202, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3;
    Segment1.ShortenTo(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3;
    Segment1.ShortenTo(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3;
    Segment1.Shorten(Segment1.GetStartPoint(), MiscMidPoint3);
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.099999974681068096, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000043115562020, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3;
    Segment1.Shorten(MiscMidPoint3, Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.099999974681068096, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000043115562020, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    Segment1 = MiscSegment3;
    Segment1.Shorten(Segment1.GetStartPoint(), Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, Segment1.GetEndPoint().GetY());

    // Test with very large segment
    Segment1 = LargeSegment1;
    Segment1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(1.000E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-13.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(7.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(11.00E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1;
    Segment1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(7.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(11.00E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1;
    Segment1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(1.000E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-13.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.00E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1;
    Segment1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.00E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1;
    Segment1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(7.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(11.00E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1;
    Segment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(-1.000000E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.00000E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(8.9999990E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(1.8999996E124, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1;
    Segment1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.00E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1;
    Segment1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1;
    Segment1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(1.000E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-13.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.00E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1;
    Segment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(8.9999990E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(1.8999996E124, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.0000000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.000000E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1;
    Segment1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(9.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(19.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1;
    Segment1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.00E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1;
    Segment1.ShortenFrom(LargeMidPoint1);
    ASSERT_DOUBLE_EQ(4.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-1.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1;
    Segment1.ShortenFrom(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.00E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1;
    Segment1.ShortenFrom(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(9.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(19.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1;
    Segment1.ShortenTo(LargeMidPoint1);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(4.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1;
    Segment1.ShortenTo(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1;
    Segment1.ShortenTo(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.00E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1;
    Segment1.Shorten(Segment1.GetStartPoint(), LargeMidPoint1);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(4.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1;
    Segment1.Shorten(LargeMidPoint1, Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(4.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-1.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = LargeSegment1;
    Segment1.Shorten(Segment1.GetStartPoint(), Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.00E123, Segment1.GetEndPoint().GetY());

    // Test with segments way into positive regions
    Segment1 = PositiveSegment1;
    Segment1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(3.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(37.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1;
    Segment1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(37.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1;
    Segment1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(3.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1;
    Segment1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1;
    Segment1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(37.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1;
    Segment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0000000E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.000000E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(1.0999999E124, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(4.0999998E124, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1;
    Segment1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1;
    Segment1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1;
    Segment1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(3.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1;
    Segment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0999999E124, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(4.0999998E124, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.000000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.000000E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1;
    Segment1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(11E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(41E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1;
    Segment1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, Segment1.GetEndPoint().GetY());

    // Due to EPSILON problems some of the following methods do not work properly
    Segment1 = PositiveSegment1;
    Segment1.ShortenFrom(PositiveMidPoint1);
    ASSERT_DOUBLE_EQ(6.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(31.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1;
    Segment1.ShortenFrom(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1;
    Segment1.ShortenFrom(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(11E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(41E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1;
    Segment1.ShortenTo(PositiveMidPoint1);
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(6.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(31.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1;
    Segment1.ShortenTo(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1;
    Segment1.ShortenTo(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1;
    Segment1.Shorten(Segment1.GetStartPoint(), PositiveMidPoint1);
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(6.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(31.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1;
    Segment1.Shorten(PositiveMidPoint1, Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(6.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(31.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = PositiveSegment1;
    Segment1.Shorten(Segment1.GetStartPoint(), Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, Segment1.GetEndPoint().GetY());

    // Test with segments way into negative regions
    Segment1 = NegativeSegment1;
    Segment1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(-3.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-25.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-9.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-37.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1;
    Segment1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-9.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-37.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1;
    Segment1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(-3.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-25.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1;
    Segment1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1;
    Segment1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-9.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-37.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1;
    Segment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(-10.000000E122, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.000000E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-1.0999999E124, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-4.0999998E124, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1;
    Segment1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1;
    Segment1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1;
    Segment1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(-3.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-25.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1;
    Segment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(-1.0999999E124, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-4.0999998E124, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.000000E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.000000E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1;
    Segment1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(-11E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-41E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1;
    Segment1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1;
    Segment1.ShortenFrom(NegativeMidPoint1);
    ASSERT_DOUBLE_EQ(-6.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-31.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, Segment1.GetEndPoint().GetY());
    
    Segment1 = NegativeSegment1;
    Segment1.ShortenFrom(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1;
    Segment1.ShortenFrom(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(-11E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-41E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1;
    Segment1.ShortenTo(NegativeMidPoint1);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-6.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-31.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1;
    Segment1.ShortenTo(Segment1.GetStartPoint());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1;
    Segment1.ShortenTo(Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(-10E122, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1;
    Segment1.Shorten(Segment1.GetStartPoint(), NegativeMidPoint1);
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-6.00E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-31.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1;
    Segment1.Shorten(NegativeMidPoint1, Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(-6.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-31.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, Segment1.GetEndPoint().GetY());

    Segment1 = NegativeSegment1;
    Segment1.Shorten(Segment1.GetStartPoint(), Segment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(-1.00E123, Segment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, Segment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, Segment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, Segment1.GetEndPoint().GetY());

    }

//==================================================================================
// AutoCrosses() const
//==================================================================================
TEST_F (HVE2DSegmentTester, AutoCrossesTest)
    {
    
    ASSERT_FALSE(VerticalSegment1.AutoCrosses());
    ASSERT_FALSE(HorizontalSegment1.AutoCrosses());
    ASSERT_FALSE(MiscSegment1.AutoCrosses());
    ASSERT_FALSE(LargeSegment1.AutoCrosses());

    }

//==================================================================================
// Closest point calculation test
// CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
//==================================================================================
TEST_F (HVE2DSegmentTester, CalculateClosestPointTest)
    {

    HGF2DLocation   VeryFarPoint(21E123, 1E123, pWorld);

    // Test with vertical segment
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment1.CalculateClosestPoint(VerticalClosePoint1A).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment1.CalculateClosestPoint(VerticalClosePoint1A).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment1.CalculateClosestPoint(VerticalClosePoint1B).GetX());
    ASSERT_DOUBLE_EQ(5.000000000000000000, VerticalSegment1.CalculateClosestPoint(VerticalClosePoint1B).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment1.CalculateClosestPoint(VerticalClosePoint1C).GetX());
    ASSERT_DOUBLE_EQ(7.000000000000000000, VerticalSegment1.CalculateClosestPoint(VerticalClosePoint1C).GetY());
    ASSERT_DOUBLE_EQ(0.099999999976716936, VerticalSegment1.CalculateClosestPoint(VerticalClosePoint1D).GetX());
    ASSERT_DOUBLE_EQ(10.00000000000000000, VerticalSegment1.CalculateClosestPoint(VerticalClosePoint1D).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment1.CalculateClosestPoint(VerticalCloseMidPoint1).GetX());
    ASSERT_DOUBLE_EQ(5.100000000000000000, VerticalSegment1.CalculateClosestPoint(VerticalCloseMidPoint1).GetY());

    // Test with inverted vertical segment
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment2.CalculateClosestPoint(VerticalClosePoint1A).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment2.CalculateClosestPoint(VerticalClosePoint1A).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment2.CalculateClosestPoint(VerticalClosePoint1B).GetX());
    ASSERT_DOUBLE_EQ(5.000000000000000000, VerticalSegment2.CalculateClosestPoint(VerticalClosePoint1B).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment2.CalculateClosestPoint(VerticalClosePoint1C).GetX());
    ASSERT_DOUBLE_EQ(7.000000000000000000, VerticalSegment2.CalculateClosestPoint(VerticalClosePoint1C).GetY());
    ASSERT_DOUBLE_EQ(0.099999999976716936, VerticalSegment2.CalculateClosestPoint(VerticalClosePoint1D).GetX());
    ASSERT_DOUBLE_EQ(10.00000000000000000, VerticalSegment2.CalculateClosestPoint(VerticalClosePoint1D).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment2.CalculateClosestPoint(VerticalCloseMidPoint1).GetX());
    ASSERT_DOUBLE_EQ(5.100000000000000000, VerticalSegment2.CalculateClosestPoint(VerticalCloseMidPoint1).GetY());

    // Test with horizontal segment
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment1.CalculateClosestPoint(HorizontalClosePoint1A).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment1.CalculateClosestPoint(HorizontalClosePoint1A).GetY());
    ASSERT_DOUBLE_EQ(5.000000000000000000, HorizontalSegment1.CalculateClosestPoint(HorizontalClosePoint1B).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment1.CalculateClosestPoint(HorizontalClosePoint1B).GetY());
    ASSERT_DOUBLE_EQ(7.000000000000000000, HorizontalSegment1.CalculateClosestPoint(HorizontalClosePoint1C).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment1.CalculateClosestPoint(HorizontalClosePoint1C).GetY());
    ASSERT_DOUBLE_EQ(10.00000000000000000, HorizontalSegment1.CalculateClosestPoint(HorizontalClosePoint1D).GetX());
    ASSERT_DOUBLE_EQ(0.099999999976716936, HorizontalSegment1.CalculateClosestPoint(HorizontalClosePoint1D).GetY());
    ASSERT_DOUBLE_EQ(5.100000000000000000, HorizontalSegment1.CalculateClosestPoint(HorizontalCloseMidPoint1).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment1.CalculateClosestPoint(HorizontalCloseMidPoint1).GetY());

    // Test with inverted horizontal segment
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment2.CalculateClosestPoint(HorizontalClosePoint1A).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment2.CalculateClosestPoint(HorizontalClosePoint1A).GetY());
    ASSERT_DOUBLE_EQ(5.000000000000000000, HorizontalSegment2.CalculateClosestPoint(HorizontalClosePoint1B).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment2.CalculateClosestPoint(HorizontalClosePoint1B).GetY());
    ASSERT_DOUBLE_EQ(7.000000000000000000, HorizontalSegment2.CalculateClosestPoint(HorizontalClosePoint1C).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment2.CalculateClosestPoint(HorizontalClosePoint1C).GetY());
    ASSERT_DOUBLE_EQ(10.00000000000000000, HorizontalSegment2.CalculateClosestPoint(HorizontalClosePoint1D).GetX());
    ASSERT_DOUBLE_EQ(0.099999999976716936, HorizontalSegment2.CalculateClosestPoint(HorizontalClosePoint1D).GetY());
    ASSERT_DOUBLE_EQ(5.100000000000000000, HorizontalSegment2.CalculateClosestPoint(HorizontalCloseMidPoint1).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment2.CalculateClosestPoint(HorizontalCloseMidPoint1).GetY());

    // Tests with vertical EPSILON sized segment
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment3.CalculateClosestPoint(VerticalClosePoint3A).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment3.CalculateClosestPoint(VerticalClosePoint3A).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment3.CalculateClosestPoint(VerticalClosePoint3B).GetX());
    ASSERT_DOUBLE_EQ(0.100000066666666680, VerticalSegment3.CalculateClosestPoint(VerticalClosePoint3B).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment3.CalculateClosestPoint(VerticalClosePoint3C).GetX());
    ASSERT_DOUBLE_EQ(0.100000033333333340, VerticalSegment3.CalculateClosestPoint(VerticalClosePoint3C).GetY());
    ASSERT_DOUBLE_EQ(0.099999999976716936, VerticalSegment3.CalculateClosestPoint(VerticalClosePoint3D).GetX());
    ASSERT_DOUBLE_EQ(0.100000050000000010, VerticalSegment3.CalculateClosestPoint(VerticalClosePoint3D).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalSegment3.CalculateClosestPoint(VerticalCloseMidPoint3).GetX());
    ASSERT_DOUBLE_EQ(0.100000050000000010, VerticalSegment3.CalculateClosestPoint(VerticalCloseMidPoint3).GetY());

    // Tests with horizontal EPSILON sized segment
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment3.CalculateClosestPoint(HorizontalClosePoint3A).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment3.CalculateClosestPoint(HorizontalClosePoint3A).GetY());
    ASSERT_DOUBLE_EQ(0.100000066666666680, HorizontalSegment3.CalculateClosestPoint(HorizontalClosePoint3B).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment3.CalculateClosestPoint(HorizontalClosePoint3B).GetY());
    ASSERT_DOUBLE_EQ(0.100000033333333340, HorizontalSegment3.CalculateClosestPoint(HorizontalClosePoint3C).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment3.CalculateClosestPoint(HorizontalClosePoint3C).GetY());
    ASSERT_DOUBLE_EQ(0.100000050000000010, HorizontalSegment3.CalculateClosestPoint(HorizontalClosePoint3D).GetX());
    ASSERT_DOUBLE_EQ(0.099999999976716936, HorizontalSegment3.CalculateClosestPoint(HorizontalClosePoint3D).GetY());
    ASSERT_DOUBLE_EQ(0.100000050000000010, HorizontalSegment3.CalculateClosestPoint(HorizontalCloseMidPoint3).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalSegment3.CalculateClosestPoint(HorizontalCloseMidPoint3).GetY());

    // Tests with miscalenious EPSILON size segment
    ASSERT_DOUBLE_EQ(0.100000000000000000, MiscSegment3.CalculateClosestPoint(MiscClosePoint3A).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, MiscSegment3.CalculateClosestPoint(MiscClosePoint3A).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, MiscSegment3.CalculateClosestPoint(MiscClosePoint3B).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, MiscSegment3.CalculateClosestPoint(MiscClosePoint3B).GetY());
    ASSERT_DOUBLE_EQ(0.100000021643961390, MiscSegment3.CalculateClosestPoint(MiscClosePoint3C).GetX());
    ASSERT_DOUBLE_EQ(0.100000097629600710, MiscSegment3.CalculateClosestPoint(MiscClosePoint3C).GetY());
    ASSERT_DOUBLE_EQ(0.100000021643961390, MiscSegment3.CalculateClosestPoint(MiscClosePoint3D).GetX());
    ASSERT_DOUBLE_EQ(0.100000097629600710, MiscSegment3.CalculateClosestPoint(MiscClosePoint3D).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, MiscSegment3.CalculateClosestPoint(MiscCloseMidPoint3).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, MiscSegment3.CalculateClosestPoint(MiscCloseMidPoint3).GetY());

    // Test with very large segment
    ASSERT_DOUBLE_EQ(4.00000000000000000E123, LargeSegment1.CalculateClosestPoint(LargeClosePoint1A).GetX());
    ASSERT_DOUBLE_EQ(-1.0000000000000000E123, LargeSegment1.CalculateClosestPoint(LargeClosePoint1A).GetY());
    ASSERT_DOUBLE_EQ(6.94117647058823470E123, LargeSegment1.CalculateClosestPoint(LargeClosePoint1B).GetX());
    ASSERT_DOUBLE_EQ(1.07647058823529420E124, LargeSegment1.CalculateClosestPoint(LargeClosePoint1B).GetY());
    ASSERT_DOUBLE_EQ(1.05882352941176280E123, LargeSegment1.CalculateClosestPoint(LargeClosePoint1C).GetX());
    ASSERT_DOUBLE_EQ(-1.2764705882352939E124, LargeSegment1.CalculateClosestPoint(LargeClosePoint1C).GetY());
    ASSERT_DOUBLE_EQ(4.00023529411764720E123, LargeSegment1.CalculateClosestPoint(LargeClosePoint1D).GetX());
    ASSERT_DOUBLE_EQ(-9.9905882352941167E122, LargeSegment1.CalculateClosestPoint(LargeClosePoint1D).GetY());

    // Test with a NULL segment
    ASSERT_NEAR(0.0, NullSegment1.CalculateClosestPoint(NegativeClosePoint1A).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, NullSegment1.CalculateClosestPoint(NegativeClosePoint1A).GetY(), MYEPSILON);

    // Tests with special points
    ASSERT_DOUBLE_EQ(10.1, MiscSegment1.CalculateClosestPoint(VeryFarPoint).GetX());
    ASSERT_DOUBLE_EQ(10.1, MiscSegment1.CalculateClosestPoint(VeryFarPoint).GetY());
    ASSERT_DOUBLE_EQ(5.10, MiscSegment1.CalculateClosestPoint(MidPoint).GetX());
    ASSERT_DOUBLE_EQ(5.10, MiscSegment1.CalculateClosestPoint(MidPoint).GetY());
    ASSERT_DOUBLE_EQ(0.10, MiscSegment1.CalculateClosestPoint(VeryFarNegativePoint).GetX());
    ASSERT_DOUBLE_EQ(0.10, MiscSegment1.CalculateClosestPoint(VeryFarNegativePoint).GetY());
    ASSERT_DOUBLE_EQ(10.1, MiscSegment1.CalculateClosestPoint(VeryFarAlignedPoint).GetX());
    ASSERT_DOUBLE_EQ(10.1, MiscSegment1.CalculateClosestPoint(VeryFarAlignedPoint).GetY());
    ASSERT_DOUBLE_EQ(0.10, MiscSegment1.CalculateClosestPoint(VeryFarAlignedNegativePoint).GetX());
    ASSERT_DOUBLE_EQ(0.10, MiscSegment1.CalculateClosestPoint(VeryFarAlignedNegativePoint).GetY());
    ASSERT_DOUBLE_EQ(0.10, MiscSegment1.CalculateClosestPoint(MiscSegment1.GetStartPoint()).GetX());
    ASSERT_DOUBLE_EQ(0.10, MiscSegment1.CalculateClosestPoint(MiscSegment1.GetStartPoint()).GetY());
    ASSERT_DOUBLE_EQ(10.1, MiscSegment1.CalculateClosestPoint(MiscSegment1.GetEndPoint()).GetX());
    ASSERT_DOUBLE_EQ(10.1, MiscSegment1.CalculateClosestPoint(MiscSegment1.GetEndPoint()).GetY());

    }

//==================================================================================
// Intersection test (with other segments only)
// Intersect(const HVE2DVector& pi_rVector, HGF2DLocationCollection* po_pCrossPoints) const;
//==================================================================================
TEST_F (HVE2DSegmentTester, IntersectTest)
    {

    HGF2DLocationCollection   DumPoints;

    // Test with extent disjoint segments
    ASSERT_EQ(0, MiscSegment1.Intersect(DisjointSegment1, &DumPoints));

    // Test with disjoint but touching by a side segments
    ASSERT_EQ(0, MiscSegment1.Intersect(ContiguousExtentSegment1, &DumPoints));

    // Test with disjoint but touching by a tip segments but not linked
    ASSERT_EQ(0, MiscSegment1.Intersect(FlirtingExtentSegment1, &DumPoints));

    // Test with disjoint but touching by a tip segments linked
    ASSERT_EQ(0, MiscSegment1.Intersect(FlirtingExtentLinkedSegment1, &DumPoints));

    // Test with vertical segment
    ASSERT_EQ(0, VerticalSegment1.Intersect(MiscSegment1, &DumPoints));

    // Test with inverted vertical segment
    ASSERT_EQ(0, VerticalSegment2.Intersect(MiscSegment1, &DumPoints));

    // Test with close vertical segments
    ASSERT_EQ(0, VerticalSegment1.Intersect(VerticalSegment4, &DumPoints));

    // Test with horizontal segment
    ASSERT_EQ(0, HorizontalSegment1.Intersect(MiscSegment1, &DumPoints));

    // Test with inverted horizontal segment
    ASSERT_EQ(0, HorizontalSegment2.Intersect(MiscSegment1, &DumPoints));

    // Test with parallel segments
    ASSERT_EQ(0, MiscSegment1.Intersect(ParallelSegment1, &DumPoints));

    // Test with near parallel segments
    ASSERT_EQ(0, MiscSegment1.Intersect(NearParallelSegment1, &DumPoints));

    // Tests with close near parallel segments
    ASSERT_EQ(0, MiscSegment1.Intersect(CloseNearParallelSegment1, &DumPoints));

    // Tests with connected segments
    // At start point...
    ASSERT_EQ(0, MiscSegment1.Intersect(ConnectedSegment1, &DumPoints));
    ASSERT_EQ(0, MiscSegment1.Intersect(ConnectingSegment1, &DumPoints));

    // At end point ...
    ASSERT_EQ(0, MiscSegment1.Intersect(ConnectedSegment1A, &DumPoints));
    ASSERT_EQ(0, MiscSegment1.Intersect(ConnectingSegment1A, &DumPoints));

    // Tests with linked segments
    ASSERT_EQ(0, MiscSegment1.Intersect(LinkedSegment1, &DumPoints));
    ASSERT_EQ(0, MiscSegment1.Intersect(LinkedSegment1A, &DumPoints));

    // Tests with vertical EPSILON sized segment
    ASSERT_EQ(0, VerticalSegment3.Intersect(MiscSegment1, &DumPoints));

    // Tests with horizontal EPSILON sized segment
    ASSERT_EQ(0, HorizontalSegment3.Intersect(MiscSegment1, &DumPoints));

    // Tests with miscalenious EPSILON size segment
    ASSERT_EQ(0, MiscSegment3.Intersect(MiscSegment1, &DumPoints));

    // Test with very large segment
    ASSERT_EQ(0, LargeSegment1.Intersect(MiscSegment1, &DumPoints));

    // Test with segments way into positive regions
    ASSERT_EQ(0, PositiveSegment1.Intersect(MiscSegment1, &DumPoints));

    // Test with segments way into negative regions
    ASSERT_EQ(0, NegativeSegment1.Intersect(MiscSegment1, &DumPoints));

    // Test with a NULL segment
    ASSERT_EQ(0, NullSegment1.Intersect(MiscSegment1, &DumPoints));

    }

//==================================================================================
// Contiguousness Test
//   ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
//                              HGF2DLocationCollection* po_pContiguousnessPoints) const;
//   ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
//                                const HGF2DLocation& pi_rPoint,
//                                HGF2DLocation* pi_pFirstContiguousnessPoint,
//                                HGF2DLocation* pi_pSecondContiguousnessPoint) const;
//    HDLL virtual bool      AreContiguous(const HVE2DVector& pi_rVector) const;
//    HDLL virtual bool      AreContiguousAt(const HVE2DVector& pi_rVector,
//                                            const HGF2DLocation& pi_rPoint) const;
//==================================================================================
TEST_F (HVE2DSegmentTester, ContiguousnessTest)
    {

    HGF2DLocationCollection     DumPoints;
    HGF2DLocation   FirstDumPoint;
    HGF2DLocation   SecondDumPoint;

    // Test with vertical segments
    ASSERT_TRUE(VerticalSegment1.AreContiguous(VerticalSegment2));
    ASSERT_TRUE(VerticalSegment1.AreContiguousAt(VerticalSegment2, VerticalMidPoint1));
    ASSERT_EQ(2, VerticalSegment1.ObtainContiguousnessPoints(VerticalSegment2, &DumPoints));
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(0.10, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.1, DumPoints[1].GetY());

    VerticalSegment1.ObtainContiguousnessPointsAt(VerticalSegment2, VerticalMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(0.10, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetY());

    DumPoints.clear();

    // Test with horizontal segments
    ASSERT_TRUE(HorizontalSegment1.AreContiguous(HorizontalSegment2));
    ASSERT_TRUE(HorizontalSegment1.AreContiguousAt(HorizontalSegment2, HorizontalMidPoint1));
    ASSERT_EQ(2, HorizontalSegment1.ObtainContiguousnessPoints(HorizontalSegment2, &DumPoints));
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.1, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(0.10, DumPoints[1].GetY());

    HorizontalSegment1.ObtainContiguousnessPointsAt(HorizontalSegment2, HorizontalMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10, SecondDumPoint.GetY());

    ASSERT_FALSE(HorizontalSegment1.AreContiguous(MiscSegment1));

    DumPoints.clear();

    // Test with positive slope segments
    ASSERT_TRUE(MiscSegment1.AreContiguous(MiscSegment2));
    ASSERT_TRUE(MiscSegment1.AreContiguousAt(MiscSegment2, MiscMidPoint1));
    ASSERT_EQ(2, MiscSegment1.ObtainContiguousnessPoints(MiscSegment2, &DumPoints));
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.1, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.1, DumPoints[1].GetY());

    MiscSegment1.ObtainContiguousnessPointsAt(MiscSegment2, MiscMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetY());

    ASSERT_FALSE(MiscSegment1.AreContiguous(LargeSegment1));

    DumPoints.clear();

    // Tests with negative slope segments
    ASSERT_TRUE(MiscSegment6.AreContiguous(MiscSegment7));
    ASSERT_TRUE(MiscSegment6.AreContiguousAt(MiscSegment7, MiscMidPoint6));
    ASSERT_EQ(2, MiscSegment6.ObtainContiguousnessPoints(MiscSegment7, &DumPoints));
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(-9.8, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetY());

    MiscSegment6.ObtainContiguousnessPointsAt(MiscSegment7, MiscMidPoint6, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(-9.8, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetY());

    ASSERT_FALSE(MiscSegment6.AreContiguous(MiscSegment1));

    DumPoints.clear();

    // Tests with vertical EPSILON sized segment   
    ASSERT_FALSE(VerticalSegment3.AreContiguous(VerticalSegment2));
    ASSERT_FALSE(VerticalSegment3.AreContiguous(HorizontalSegment3));

    // Tests with horizontal EPSILON sized segment
    ASSERT_FALSE(HorizontalSegment3.AreContiguous(HorizontalSegment2));
    ASSERT_FALSE(HorizontalSegment3.AreContiguous(MiscSegment3));

    // Test with a very large segment
    // Precision problem
    ASSERT_TRUE(LargeSegment1.AreContiguous(LargeSegment2));
    ASSERT_TRUE(LargeSegment1.AreContiguousAt(LargeSegment2, LargeMidPoint1));
    ASSERT_EQ(2, LargeSegment1.ObtainContiguousnessPoints(LargeSegment2, &DumPoints));
    ASSERT_DOUBLE_EQ(-10.0E122, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(-2.10E124, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(9.000E123, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(1.900E124, DumPoints[1].GetY());

    LargeSegment1.ObtainContiguousnessPointsAt(LargeSegment2, LargeMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(-9.9999999999999998E122, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(-2.1000000000000001E124, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(8.99999999999999970E123, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(1.89999999999999990E124, SecondDumPoint.GetY());

    ASSERT_FALSE(LargeSegment1.AreContiguous(HorizontalSegment1));

    DumPoints.clear();

    // Test with a segment way into positive regions   
    ASSERT_TRUE(PositiveSegment1.AreContiguous(PositiveSegment2));
    ASSERT_TRUE(PositiveSegment1.AreContiguousAt(PositiveSegment2, PositiveMidPoint1));
    ASSERT_EQ(2, PositiveSegment1.ObtainContiguousnessPoints(PositiveSegment2, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0E122, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(2.10E124, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(1.10E124, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(4.10E124, DumPoints[1].GetY());

    PositiveSegment1.ObtainContiguousnessPointsAt(PositiveSegment2, PositiveMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(10.0E122, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(2.10E124, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(1.10E124, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(4.10E124, SecondDumPoint.GetY());

    ASSERT_FALSE(PositiveSegment1.AreContiguous(HorizontalSegment1));

    DumPoints.clear();

    // Test with segment way into negative segments   
    ASSERT_TRUE(NegativeSegment1.AreContiguous(NegativeSegment2));
    ASSERT_TRUE(NegativeSegment1.AreContiguousAt(NegativeSegment2, NegativeMidPoint1));
    ASSERT_EQ(2, NegativeSegment1.ObtainContiguousnessPoints(NegativeSegment2, &DumPoints));
    ASSERT_DOUBLE_EQ(-10.0E122, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(-2.10E124, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(-1.10E124, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(-4.10E124, DumPoints[1].GetY());

    NegativeSegment1.ObtainContiguousnessPointsAt(NegativeSegment2, NegativeMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(-10.0E122, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(-2.10E124, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(-1.10E124, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(-4.10E124, SecondDumPoint.GetY());

    ASSERT_FALSE(NegativeSegment1.AreContiguous(HorizontalSegment1));

    }

//==================================================================================
// AreContiguousAtAndGet(const HVE2DVector& pi_rVector, const HGF2DLocation& pi_rPoint,
//                       HGF2DLocation* po_pFirstContiguousnessPoint, HGF2DLocation* po_pSecondContiguousnessPoint) const;
//==================================================================================
TEST_F (HVE2DSegmentTester, AreContiguousAtAndGetTest)
    {

    HGF2DLocation FirstDumPoint;
    HGF2DLocation SecondDumPoint;

    // Test with vertical segments
    ASSERT_TRUE(VerticalSegment1.AreContiguousAtAndGet(VerticalSegment2, VerticalMidPoint1, &FirstDumPoint, &SecondDumPoint));
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(0.10, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetY());

    // Test with horizontal segments
    ASSERT_TRUE(HorizontalSegment1.AreContiguousAtAndGet(HorizontalSegment2, HorizontalMidPoint1, &FirstDumPoint, &SecondDumPoint));
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10, SecondDumPoint.GetY());

    // Test with positive slope segments
    MiscSegment1.AreContiguousAtAndGet(MiscSegment2, MiscMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetY());

    // Tests with negative slope segments
    MiscSegment6.AreContiguousAtAndGet(MiscSegment7, MiscMidPoint6, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(0.10000000000000000, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(-9.8000000000000007, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0000000000000000, SecondDumPoint.GetY());

    // Test with a very large segment
    LargeSegment1.AreContiguousAtAndGet(LargeSegment2, LargeMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(-10.0E122, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(-2.10E124, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(9.000E123, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(1.900E124, SecondDumPoint.GetY());

    // Test with a segment way into positive regions
    PositiveSegment1.AreContiguousAtAndGet(PositiveSegment2, PositiveMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(10.0E122, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(2.10E124, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(1.10E124, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(4.10E124, SecondDumPoint.GetY());

    // Test with segment way into negative segments
    NegativeSegment1.AreContiguousAtAndGet(NegativeSegment2, NegativeMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(-10.0E122, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(-2.10E124, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(-1.10E124, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(-4.10E124, SecondDumPoint.GetY());

    }

//==================================================================================
// Cloning tests
// Clone() const;
// AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
//==================================================================================
TEST_F (HVE2DSegmentTester, CloningTest)
    {

    // General Clone test
    HFCPtr<HVE2DSegment> pClone = (HVE2DSegment*) MiscSegment1.Clone();
    ASSERT_EQ(pWorld, pClone->GetCoordSys());

    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(0.10, 0.10, pWorld))); 
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(10.1, 10.1, pWorld))); 

    // Test with the same coordinate system
    HFCPtr<HVE2DSegment> pClone3 = (HVE2DSegment*) MiscSegment1.AllocateCopyInCoordSys(pWorld);
    ASSERT_EQ(pWorld, pClone3->GetCoordSys());

    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(0.10, 0.10, pWorld))); 
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(10.1, 10.1, pWorld)));

    // Test with a translation between systems
    Translation = HGF2DDisplacement (10.0, 10.0);
    HGF2DTranslation myTranslation(Translation);
    HFCPtr<HGF2DCoordSys> pWorldTranslation = new HGF2DCoordSys(myTranslation, pWorld);

    HFCPtr<HVE2DSegment> pClone5 = (HVE2DSegment*) MiscSegment1.AllocateCopyInCoordSys(pWorldTranslation);
    ASSERT_EQ(pWorldTranslation, pClone5->GetCoordSys());

    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(-9.9, -9.9, pWorldTranslation))); 
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(0.10, 0.10, pWorldTranslation)));

    // Test with a stretch between systems
    HGF2DStretch myStretch;
    myStretch.SetTranslation(Translation);
    myStretch.SetXScaling(0.5);
    myStretch.SetYScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldStretch = new HGF2DCoordSys(myStretch, pWorld);

    HFCPtr<HVE2DSegment> pClone6 = (HVE2DSegment*) MiscSegment1.AllocateCopyInCoordSys(pWorldStretch);
    ASSERT_EQ(pWorldStretch, pClone6->GetCoordSys());

    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(-19.8, -19.8, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(0.200, 0.200, pWorldStretch)));

    // Test with a similitude between systems
    HGF2DSimilitude mySimilitude;
    mySimilitude.SetRotation(PI);
    mySimilitude.SetScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldSimilitude = new HGF2DCoordSys(mySimilitude, pWorld);

    HFCPtr<HVE2DSegment> pClone7 = (HVE2DSegment*) MiscSegment1.AllocateCopyInCoordSys(pWorldSimilitude);
    ASSERT_EQ(pWorldSimilitude, pClone7->GetCoordSys());

    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-0.20, -0.20 , pWorldSimilitude))); 
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-20.2, -20.2, pWorldSimilitude)));

    // Test with a affine between systems
    HGF2DAffine myAffine;
    myAffine.SetTranslation(Translation);
    myAffine.SetRotation(PI);
    myAffine.SetXScaling(0.5);
    myAffine.SetYScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldAffine = new HGF2DCoordSys(myAffine, pWorld);

    HFCPtr<HVE2DSegment> pClone8 = (HVE2DSegment*) MiscSegment1.AllocateCopyInCoordSys(pWorldAffine);
    ASSERT_EQ(pWorldAffine, pClone8->GetCoordSys());

    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(19.8, 19.8, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-0.2, -0.2, pWorldAffine)));

    }

//==================================================================================
// Interaction info with other segment
// Crosses(const HVE2DVector& pi_rVector) const;
// AreAdjacent(const HVE2DVector& pi_rVector) const;
// IsPointOn(const HGF2DLocation& pi_rTestPoint) const;
//==================================================================================
TEST_F (HVE2DSegmentTester, InteractionTest)
    {

    // Tests with a vertical segment
    ASSERT_FALSE(VerticalSegment1.Crosses(VerticalSegment2));
    ASSERT_TRUE(VerticalSegment1.AreAdjacent(VerticalSegment2));

    ASSERT_FALSE(VerticalSegment1.Crosses(MiscSegment1));
    ASSERT_FALSE(VerticalSegment1.AreAdjacent(MiscSegment1));

    ASSERT_FALSE(VerticalSegment1.Crosses(VerticalSegment3));
    ASSERT_TRUE(VerticalSegment1.AreAdjacent(VerticalSegment3));

    ASSERT_FALSE(VerticalSegment1.Crosses(LargeSegment1));
    ASSERT_FALSE(VerticalSegment1.AreAdjacent(LargeSegment1));

    ASSERT_FALSE(VerticalSegment1.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_FALSE(VerticalSegment1.IsPointOn(HGF2DLocation(0.1, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(VerticalSegment1.IsPointOn(VerticalMidPoint1-HGF2DDisplacement(1.1*MYEPSILON, 0)));
    ASSERT_FALSE(VerticalSegment1.IsPointOn(VerticalMidPoint1-HGF2DDisplacement(-1.1*MYEPSILON, 0)));
    ASSERT_TRUE(VerticalSegment1.IsPointOn(VerticalMidPoint1-HGF2DDisplacement(0.9*MYEPSILON, 0)));
    ASSERT_TRUE(VerticalSegment1.IsPointOn(VerticalMidPoint1-HGF2DDisplacement(-0.9*MYEPSILON, 0)));
    ASSERT_TRUE(VerticalSegment1.IsPointOn(VerticalSegment1.GetStartPoint()));
    ASSERT_TRUE(VerticalSegment1.IsPointOn(VerticalSegment1.GetEndPoint()));
    ASSERT_FALSE(VerticalSegment1.IsPointOn(VerticalSegment1.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(VerticalSegment1.IsPointOn(VerticalSegment1.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(VerticalSegment1.IsPointOn(VerticalMidPoint1));
    ASSERT_TRUE(VerticalSegment1.IsPointOn(VerticalMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    // Tests with an inverted vertical segment
    ASSERT_FALSE(VerticalSegment2.Crosses(VerticalSegment1));
    ASSERT_TRUE(VerticalSegment2.AreAdjacent(VerticalSegment1));

    ASSERT_FALSE(VerticalSegment2.Crosses(MiscSegment1));
    ASSERT_FALSE(VerticalSegment2.AreAdjacent(MiscSegment1));

    ASSERT_FALSE(VerticalSegment2.Crosses(VerticalSegment3));
    ASSERT_TRUE(VerticalSegment2.AreAdjacent(VerticalSegment3));

    ASSERT_FALSE(VerticalSegment2.Crosses(LargeSegment1));
    ASSERT_FALSE(VerticalSegment2.AreAdjacent(LargeSegment1));

    ASSERT_FALSE(VerticalSegment2.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_FALSE(VerticalSegment2.IsPointOn(HGF2DLocation(0.1, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(VerticalSegment2.IsPointOn(VerticalMidPoint1-HGF2DDisplacement(1.1*MYEPSILON, 0)));
    ASSERT_FALSE(VerticalSegment2.IsPointOn(VerticalMidPoint1-HGF2DDisplacement(-1.1*MYEPSILON, 0)));
    ASSERT_TRUE(VerticalSegment2.IsPointOn(VerticalMidPoint1-HGF2DDisplacement(0.9*MYEPSILON, 0)));
    ASSERT_TRUE(VerticalSegment2.IsPointOn(VerticalMidPoint1-HGF2DDisplacement(-0.9*MYEPSILON, 0)));
    ASSERT_TRUE(VerticalSegment2.IsPointOn(VerticalSegment2.GetStartPoint()));
    ASSERT_TRUE(VerticalSegment2.IsPointOn(VerticalSegment2.GetEndPoint()));
    ASSERT_TRUE(VerticalSegment2.IsPointOn(VerticalMidPoint1));
    ASSERT_FALSE(VerticalSegment2.IsPointOn(VerticalSegment2.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(VerticalSegment2.IsPointOn(VerticalSegment2.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(VerticalSegment2.IsPointOn(VerticalMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    // Tests with a horizontal segment
    ASSERT_FALSE(HorizontalSegment1.Crosses(HorizontalSegment2));
    ASSERT_TRUE(HorizontalSegment1.AreAdjacent(HorizontalSegment2));

    ASSERT_FALSE(HorizontalSegment1.Crosses(MiscSegment1));
    ASSERT_FALSE(HorizontalSegment1.AreAdjacent(MiscSegment1));

    ASSERT_FALSE(HorizontalSegment1.Crosses(HorizontalSegment3));
    ASSERT_TRUE(HorizontalSegment1.AreAdjacent(HorizontalSegment3));

    ASSERT_FALSE(HorizontalSegment1.Crosses(LargeSegment1));
    ASSERT_FALSE(HorizontalSegment1.AreAdjacent(LargeSegment1));

    ASSERT_FALSE(HorizontalSegment1.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_FALSE(HorizontalSegment1.IsPointOn(HGF2DLocation(0.1-1.1*MYEPSILON, 0.1, pWorld)));
    ASSERT_FALSE(HorizontalSegment1.IsPointOn(HorizontalMidPoint1-HGF2DDisplacement(0, 1.1*MYEPSILON)));
    ASSERT_FALSE(HorizontalSegment1.IsPointOn(HorizontalMidPoint1-HGF2DDisplacement(0, -1.1*MYEPSILON)));
    ASSERT_TRUE(HorizontalSegment1.IsPointOn(HorizontalMidPoint1-HGF2DDisplacement(0, 0.9*MYEPSILON)));
    ASSERT_TRUE(HorizontalSegment1.IsPointOn(HorizontalMidPoint1-HGF2DDisplacement(0, -0.9*MYEPSILON)));
    ASSERT_TRUE(HorizontalSegment1.IsPointOn(HorizontalSegment1.GetStartPoint()));
    ASSERT_TRUE(HorizontalSegment1.IsPointOn(HorizontalSegment1.GetEndPoint()));
    ASSERT_TRUE(HorizontalSegment1.IsPointOn(HorizontalMidPoint1));
    ASSERT_FALSE(HorizontalSegment1.IsPointOn(HorizontalSegment1.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(HorizontalSegment1.IsPointOn(HorizontalSegment1.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(HorizontalSegment1.IsPointOn(HorizontalMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    // Tests with an inverted horizontal segment
    ASSERT_FALSE(HorizontalSegment2.Crosses(HorizontalSegment1));
    ASSERT_TRUE(HorizontalSegment2.AreAdjacent(HorizontalSegment1));

    ASSERT_FALSE(HorizontalSegment2.Crosses(MiscSegment1));
    ASSERT_FALSE(HorizontalSegment2.AreAdjacent(MiscSegment1));

    ASSERT_FALSE(HorizontalSegment2.Crosses(HorizontalSegment3));
    ASSERT_TRUE(HorizontalSegment2.AreAdjacent(HorizontalSegment3));

    ASSERT_FALSE(HorizontalSegment2.Crosses(LargeSegment1));
    ASSERT_FALSE(HorizontalSegment2.AreAdjacent(LargeSegment1));

    ASSERT_FALSE(HorizontalSegment2.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_FALSE(HorizontalSegment2.IsPointOn(HGF2DLocation(0.1-1.1*MYEPSILON, 0.1, pWorld)));
    ASSERT_FALSE(HorizontalSegment2.IsPointOn(HorizontalMidPoint1-HGF2DDisplacement(0, 1.1*MYEPSILON)));
    ASSERT_FALSE(HorizontalSegment2.IsPointOn(HorizontalMidPoint1-HGF2DDisplacement(0, -1.1*MYEPSILON)));
    ASSERT_TRUE(HorizontalSegment2.IsPointOn(HorizontalMidPoint1-HGF2DDisplacement(0, 0.9*MYEPSILON)));
    ASSERT_TRUE(HorizontalSegment2.IsPointOn(HorizontalMidPoint1-HGF2DDisplacement(0, -0.9*MYEPSILON)));
    ASSERT_TRUE(HorizontalSegment2.IsPointOn(HorizontalSegment2.GetStartPoint()));
    ASSERT_TRUE(HorizontalSegment2.IsPointOn(HorizontalSegment2.GetEndPoint()));
    ASSERT_TRUE(HorizontalSegment2.IsPointOn(HorizontalMidPoint1));
    ASSERT_FALSE(HorizontalSegment2.IsPointOn(HorizontalSegment2.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(HorizontalSegment2.IsPointOn(HorizontalSegment2.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(HorizontalSegment2.IsPointOn(HorizontalMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    // Tests with a positive slope segment
    ASSERT_FALSE(MiscSegment1.Crosses(MiscSegment2));
    ASSERT_TRUE(MiscSegment1.AreAdjacent(MiscSegment2));

    ASSERT_FALSE(MiscSegment1.Crosses(HorizontalSegment1));
    ASSERT_FALSE(MiscSegment1.AreAdjacent(HorizontalSegment1));

    ASSERT_FALSE(MiscSegment1.Crosses(MiscSegment3));
    ASSERT_FALSE(MiscSegment1.AreAdjacent(MiscSegment3));

    ASSERT_FALSE(MiscSegment1.Crosses(LargeSegment1));
    ASSERT_FALSE(MiscSegment1.AreAdjacent(LargeSegment1));

    ASSERT_FALSE(MiscSegment1.IsPointOn(HGF2DLocation(20, 20, pWorld)));
    ASSERT_FALSE(MiscSegment1.IsPointOn(HGF2DLocation(0.1, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(MiscSegment1.IsPointOn(MiscMidPoint1-HGF2DDisplacement(1.1*MYEPSILON, -1.1*MYEPSILON)));
    ASSERT_FALSE(MiscSegment1.IsPointOn(MiscMidPoint1-HGF2DDisplacement(-1.1*MYEPSILON, 1.1*MYEPSILON)));
    ASSERT_TRUE(MiscSegment1.IsPointOn(MiscMidPoint1-HGF2DDisplacement(0.7*MYEPSILON, -0.7*MYEPSILON)));
    ASSERT_TRUE(MiscSegment1.IsPointOn(MiscMidPoint1-HGF2DDisplacement(-0.7*MYEPSILON, 0.7*MYEPSILON)));
    ASSERT_TRUE(MiscSegment1.IsPointOn(MiscSegment1.GetStartPoint()));
    ASSERT_TRUE(MiscSegment1.IsPointOn(MiscSegment1.GetEndPoint()));
    ASSERT_TRUE(MiscSegment1.IsPointOn(MiscMidPoint1));
    ASSERT_FALSE(MiscSegment1.IsPointOn(MiscSegment1.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(MiscSegment1.IsPointOn(MiscSegment1.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(MiscSegment1.IsPointOn(MiscMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    // Tests with a negative slope segment
    ASSERT_FALSE(MiscSegment2.Crosses(MiscSegment1));
    ASSERT_TRUE(MiscSegment2.AreAdjacent(MiscSegment1));

    ASSERT_FALSE(MiscSegment2.IsPointOn(HGF2DLocation(20, 20, pWorld)));
    ASSERT_FALSE(MiscSegment2.IsPointOn(HGF2DLocation(0.1, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(MiscSegment2.IsPointOn(MiscMidPoint1-HGF2DDisplacement(1.1*MYEPSILON, -1.1*MYEPSILON)));
    ASSERT_FALSE(MiscSegment2.IsPointOn(MiscMidPoint1-HGF2DDisplacement(-1.1*MYEPSILON, 1.1*MYEPSILON)));
    ASSERT_TRUE(MiscSegment2.IsPointOn(MiscMidPoint1-HGF2DDisplacement(0.7*MYEPSILON, -0.7*MYEPSILON)));
    ASSERT_TRUE(MiscSegment2.IsPointOn(MiscMidPoint1-HGF2DDisplacement(-0.7*MYEPSILON, 0.7*MYEPSILON)));
    ASSERT_TRUE(MiscSegment2.IsPointOn(MiscSegment1.GetStartPoint()));
    ASSERT_TRUE(MiscSegment2.IsPointOn(MiscSegment1.GetEndPoint()));
    ASSERT_TRUE(MiscSegment2.IsPointOn(MiscMidPoint1));
    ASSERT_FALSE(MiscSegment2.IsPointOn(MiscSegment1.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(MiscSegment2.IsPointOn(MiscSegment1.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(MiscSegment2.IsPointOn(MiscMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    // Tests with a vertical EPSILON sized segment
    ASSERT_FALSE(VerticalSegment3.Crosses(VerticalSegment1));
    ASSERT_TRUE(VerticalSegment3.AreAdjacent(VerticalSegment1));

    ASSERT_FALSE(VerticalSegment3.Crosses(MiscSegment1));
    ASSERT_FALSE(VerticalSegment3.AreAdjacent(MiscSegment1));

    ASSERT_FALSE(VerticalSegment3.Crosses(MiscSegment3));
    ASSERT_FALSE(VerticalSegment3.AreAdjacent(MiscSegment3));

    ASSERT_FALSE(VerticalSegment3.Crosses(LargeSegment1));
    ASSERT_FALSE(VerticalSegment3.AreAdjacent(LargeSegment1));

    ASSERT_FALSE(VerticalSegment3.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_FALSE(VerticalSegment3.IsPointOn(HGF2DLocation(0.1, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(VerticalSegment3.IsPointOn(VerticalMidPoint3-HGF2DDisplacement(1.1*MYEPSILON, 1.1*MYEPSILON)));
    ASSERT_FALSE(VerticalSegment3.IsPointOn(VerticalMidPoint3-HGF2DDisplacement(-1.1*MYEPSILON, -1.1*MYEPSILON)));
    ASSERT_TRUE(VerticalSegment3.IsPointOn(VerticalMidPoint3-HGF2DDisplacement(0.9*MYEPSILON, 0.9*MYEPSILON)));
    ASSERT_TRUE(VerticalSegment3.IsPointOn(VerticalMidPoint3-HGF2DDisplacement(-0.9*MYEPSILON, -0.9*MYEPSILON)));
    ASSERT_TRUE(VerticalSegment3.IsPointOn(VerticalSegment3.GetStartPoint()));
    ASSERT_TRUE(VerticalSegment3.IsPointOn(VerticalSegment3.GetEndPoint()));
    ASSERT_TRUE(VerticalSegment3.IsPointOn(VerticalMidPoint3));

    // Tests with an horizontal EPSILON SIZED segment
    ASSERT_FALSE(HorizontalSegment3.Crosses(HorizontalSegment1));
    ASSERT_TRUE(HorizontalSegment3.AreAdjacent(HorizontalSegment1));

    ASSERT_FALSE(HorizontalSegment3.Crosses(MiscSegment1));
    ASSERT_FALSE(HorizontalSegment3.AreAdjacent(MiscSegment1));

    ASSERT_FALSE(HorizontalSegment3.Crosses(MiscSegment3));
    ASSERT_FALSE(HorizontalSegment3.AreAdjacent(MiscSegment3));

    ASSERT_FALSE(HorizontalSegment3.Crosses(LargeSegment1));
    ASSERT_FALSE(HorizontalSegment3.AreAdjacent(LargeSegment1));

    ASSERT_FALSE(HorizontalSegment3.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_FALSE(HorizontalSegment3.IsPointOn(HGF2DLocation(0.1, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(HorizontalSegment3.IsPointOn(HorizontalMidPoint3-HGF2DDisplacement(0, 1.1*MYEPSILON)));
    ASSERT_FALSE(HorizontalSegment3.IsPointOn(HorizontalMidPoint3-HGF2DDisplacement(0, -1.1*MYEPSILON)));
    ASSERT_TRUE(HorizontalSegment3.IsPointOn(HorizontalMidPoint3-HGF2DDisplacement(0, 0.9*MYEPSILON)));
    ASSERT_TRUE(HorizontalSegment3.IsPointOn(HorizontalMidPoint3-HGF2DDisplacement(0, -0.9*MYEPSILON)));
    ASSERT_TRUE(HorizontalSegment3.IsPointOn(HorizontalSegment3.GetStartPoint()));
    ASSERT_TRUE(HorizontalSegment3.IsPointOn(HorizontalSegment3.GetEndPoint()));
    ASSERT_TRUE(HorizontalSegment3.IsPointOn(HorizontalMidPoint3));

    // Tests with a miscalenious EPSILON sized segment
    ASSERT_FALSE(MiscSegment3.Crosses(MiscSegment1));
    ASSERT_FALSE(MiscSegment3.AreAdjacent(MiscSegment1));

    ASSERT_FALSE(MiscSegment3.Crosses(LargeSegment1));
    ASSERT_FALSE(MiscSegment3.AreAdjacent(LargeSegment1));

    ASSERT_FALSE(MiscSegment3.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_FALSE(MiscSegment3.IsPointOn(HGF2DLocation(0.1, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(MiscSegment3.IsPointOn(MiscMidPoint3-HGF2DDisplacement(1.1*MYEPSILON, -1.1*MYEPSILON)));
    ASSERT_FALSE(MiscSegment3.IsPointOn(MiscMidPoint3-HGF2DDisplacement(1.1*MYEPSILON, -1.1*MYEPSILON)));
    ASSERT_TRUE(MiscSegment3.IsPointOn(MiscMidPoint3-HGF2DDisplacement(-0.9*MYEPSILON, 0.9*MYEPSILON)));
    ASSERT_TRUE(MiscSegment3.IsPointOn(MiscMidPoint3-HGF2DDisplacement(-0.9*MYEPSILON, -0.9*MYEPSILON)));
    ASSERT_TRUE(MiscSegment3.IsPointOn(MiscSegment3.GetStartPoint()));
    ASSERT_TRUE(MiscSegment3.IsPointOn(MiscSegment3.GetEndPoint()));
    ASSERT_TRUE(MiscSegment3.IsPointOn(MiscMidPoint3));

    // Due to precision problems, the following
    // Tests with a very large segment
    ASSERT_FALSE(LargeSegment1.Crosses(LargeSegment2));
    ASSERT_FALSE(LargeSegment1.AreAdjacent(LargeSegment2));

    ASSERT_FALSE(LargeSegment1.Crosses(MiscSegment1));
    ASSERT_FALSE(LargeSegment1.AreAdjacent(MiscSegment1));

    ASSERT_FALSE(LargeSegment1.Crosses(PositiveSegment1));
    ASSERT_FALSE(LargeSegment1.AreAdjacent(PositiveSegment1));

    ASSERT_FALSE(LargeSegment1.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_TRUE(LargeSegment1.IsPointOn(LargeSegment1.GetStartPoint()));
    ASSERT_TRUE(LargeSegment1.IsPointOn(LargeSegment1.GetEndPoint()));
    ASSERT_TRUE(LargeSegment1.IsPointOn(LargeMidPoint1));

    // Tests with a way into positive region segment
    ASSERT_FALSE(PositiveSegment1.Crosses(PositiveSegment2));
    ASSERT_FALSE(PositiveSegment1.AreAdjacent(PositiveSegment2));

    ASSERT_FALSE(PositiveSegment1.Crosses(MiscSegment1));
    ASSERT_FALSE(PositiveSegment1.AreAdjacent(MiscSegment1));

    ASSERT_FALSE(PositiveSegment1.Crosses(MiscSegment3));
    ASSERT_FALSE(PositiveSegment1.AreAdjacent(MiscSegment3));

    ASSERT_FALSE(PositiveSegment1.Crosses(LargeSegment1));
    ASSERT_FALSE(PositiveSegment1.AreAdjacent(LargeSegment1));

    ASSERT_FALSE(PositiveSegment1.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_TRUE(PositiveSegment1.IsPointOn(PositiveSegment1.GetStartPoint()));
    ASSERT_TRUE(PositiveSegment1.IsPointOn(PositiveSegment1.GetEndPoint()));
    ASSERT_TRUE(PositiveSegment1.IsPointOn(PositiveMidPoint1));

    // Tests with a way into negative region segment
    ASSERT_FALSE(NegativeSegment1.Crosses(NegativeSegment2));
    ASSERT_FALSE(NegativeSegment1.AreAdjacent(NegativeSegment2));

    ASSERT_FALSE(NegativeSegment1.Crosses(MiscSegment1));
    ASSERT_FALSE(NegativeSegment1.AreAdjacent(MiscSegment1));

    ASSERT_FALSE(NegativeSegment1.Crosses(MiscSegment3));
    ASSERT_FALSE(NegativeSegment1.AreAdjacent(MiscSegment3));

    ASSERT_FALSE(NegativeSegment1.Crosses(LargeSegment1));
    ASSERT_FALSE(NegativeSegment1.AreAdjacent(LargeSegment1));

    ASSERT_FALSE(NegativeSegment1.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_TRUE(NegativeSegment1.IsPointOn(NegativeSegment1.GetStartPoint()));
    ASSERT_TRUE(NegativeSegment1.IsPointOn(NegativeSegment1.GetEndPoint()));
    ASSERT_TRUE(NegativeSegment1.IsPointOn(NegativeMidPoint1));

    // Tests with a NULL segment
    ASSERT_FALSE(NullSegment1.Crosses(MiscSegment1));
    ASSERT_FALSE(NullSegment1.AreAdjacent(MiscSegment1));

    ASSERT_FALSE(NullSegment1.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_TRUE(NullSegment1.IsPointOn(NullSegment1.GetStartPoint()));
    ASSERT_TRUE(NullSegment1.IsPointOn(NullSegment1.GetEndPoint()));

    }

//==================================================================================
// IsPointOnSCS(const HGF2DLocation& pi_rTestPoint) const
//==================================================================================
TEST_F (HVE2DSegmentTester, IsPointOnSCSTest)
    {

    ASSERT_FALSE(VerticalSegment1.IsPointOnSCS(HGF2DLocation(10, 10, pWorld)));
    ASSERT_FALSE(VerticalSegment1.IsPointOnSCS(HGF2DLocation(0.1, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(VerticalSegment1.IsPointOnSCS(VerticalMidPoint1-HGF2DDisplacement(1.1*MYEPSILON, 0)));
    ASSERT_FALSE(VerticalSegment1.IsPointOnSCS(VerticalMidPoint1-HGF2DDisplacement(-1.1*MYEPSILON, 0)));
    ASSERT_TRUE(VerticalSegment1.IsPointOnSCS(VerticalMidPoint1-HGF2DDisplacement(0.9*MYEPSILON, 0)));
    ASSERT_TRUE(VerticalSegment1.IsPointOnSCS(VerticalMidPoint1-HGF2DDisplacement(-0.9*MYEPSILON, 0)));
    ASSERT_TRUE(VerticalSegment1.IsPointOnSCS(VerticalSegment1.GetStartPoint()));
    ASSERT_TRUE(VerticalSegment1.IsPointOnSCS(VerticalSegment1.GetEndPoint()));
    ASSERT_FALSE(VerticalSegment1.IsPointOnSCS(VerticalSegment1.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(VerticalSegment1.IsPointOnSCS(VerticalSegment1.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(VerticalSegment1.IsPointOnSCS(VerticalMidPoint1));
    ASSERT_TRUE(VerticalSegment1.IsPointOnSCS(VerticalMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    }

//==================================================================================
// Bearing calculation tests
// CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
//                  HVE2DVector::ArbitraryDirection pi_Direction = HVE2DVector::BETA) const;
//==================================================================================
TEST_F (HVE2DSegmentTester, BearingTest)
    {

    // Obtain bearing ALPHA of a vertical segment
    ASSERT_DOUBLE_EQ(4.7123889803846897, VerticalSegment1.CalculateBearing(VerticalMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a vertical segment
    ASSERT_DOUBLE_EQ(1.5707963267948966, VerticalSegment1.CalculateBearing(VerticalMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a inverted vertical segment
    ASSERT_DOUBLE_EQ(1.5707963267948966, VerticalSegment2.CalculateBearing(VerticalMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a inverted vertical segment
    ASSERT_DOUBLE_EQ(4.7123889803846897, VerticalSegment2.CalculateBearing(VerticalMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of an horizontal segment
    ASSERT_DOUBLE_EQ(3.1415926535897931, HorizontalSegment1.CalculateBearing(HorizontalMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of an horizontal segment
    ASSERT_NEAR(0.0, HorizontalSegment1.CalculateBearing(HorizontalMidPoint1, HVE2DVector::BETA).GetAngle(), MYEPSILON);

    // Obtain bearing ALPHA of an inverted horizontal segment
    ASSERT_NEAR(0.0, HorizontalSegment2.CalculateBearing(HorizontalMidPoint1, HVE2DVector::ALPHA).GetAngle(), MYEPSILON);

    // Obtain bearing BETA of an inverted horizontal segment
    ASSERT_DOUBLE_EQ(3.1415926535897931, HorizontalSegment2.CalculateBearing(HorizontalMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a positive slope segment
    ASSERT_DOUBLE_EQ(-2.3561944901923448, MiscSegment1.CalculateBearing(MiscMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a positive slope segment
    ASSERT_DOUBLE_EQ(0.78539816339744828, MiscSegment1.CalculateBearing(MiscMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a inverted positive slope segment
    ASSERT_DOUBLE_EQ(0.78539816339744828, MiscSegment2.CalculateBearing(MiscMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a inverted positive slope segment
    ASSERT_DOUBLE_EQ(-2.3561944901923448, MiscSegment2.CalculateBearing(MiscMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a vertical EPSILON SIZED segment
    ASSERT_DOUBLE_EQ(4.7123889803846897, VerticalSegment3.CalculateBearing(VerticalMidPoint3, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a vertical EPSILON SIZED segment
    ASSERT_DOUBLE_EQ(1.5707963267948966, VerticalSegment3.CalculateBearing(VerticalMidPoint3, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a horizontal EPSILON SIZED segment
    ASSERT_DOUBLE_EQ(3.1415926535897931, HorizontalSegment3.CalculateBearing(HorizontalMidPoint3, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a horizontal EPSILON SIZED segment
    ASSERT_NEAR(0.0, HorizontalSegment3.CalculateBearing(HorizontalMidPoint3, HVE2DVector::BETA).GetAngle(), MYEPSILON);

    // Obtain bearing ALPHA of a miscaleniuous EPSILON SIZED segment
    ASSERT_DOUBLE_EQ(-1.7889624832338027, MiscSegment3.CalculateBearing(MiscMidPoint3, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a miscaleniuous EPSILON SIZED segment
    ASSERT_DOUBLE_EQ(1.3526301703559906, MiscSegment3.CalculateBearing(MiscMidPoint3, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a very large segment
    ASSERT_DOUBLE_EQ(-1.8157749899217608, LargeSegment1.CalculateBearing(LargeMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a very large segment
    ASSERT_DOUBLE_EQ(1.3258176636680326, LargeSegment1.CalculateBearing(LargeMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a inverted very large segment
    ASSERT_DOUBLE_EQ(1.3258176636680326, LargeSegment2.CalculateBearing(LargeMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a inverted very large segment
    ASSERT_DOUBLE_EQ(-1.8157749899217608, LargeSegment2.CalculateBearing(LargeMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a segment way into negative coordinates
    ASSERT_DOUBLE_EQ(1.1071487177940904, NegativeSegment1.CalculateBearing(NegativeMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a segment way into negative coordinates
    ASSERT_DOUBLE_EQ(-2.0344439357957027, NegativeSegment1.CalculateBearing(NegativeMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a inverted segment way into negative coordinates
    ASSERT_DOUBLE_EQ(-2.0344439357957027, NegativeSegment2.CalculateBearing(NegativeMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a inverted segment way into negative coordinates
    ASSERT_DOUBLE_EQ(1.1071487177940904, NegativeSegment2.CalculateBearing(NegativeMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a segment way into positive values
    ASSERT_DOUBLE_EQ(-2.0344439357957027, PositiveSegment1.CalculateBearing(PositiveMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a segment way into positive values
    ASSERT_DOUBLE_EQ(1.1071487177940904, PositiveSegment1.CalculateBearing(PositiveMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a inverted segment way into positive values
    ASSERT_DOUBLE_EQ(1.1071487177940904, PositiveSegment2.CalculateBearing(PositiveMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a inverted segment way into positive values
    ASSERT_DOUBLE_EQ(-2.0344439357957027, PositiveSegment2.CalculateBearing(PositiveMidPoint1, HVE2DVector::BETA).GetAngle());

    }
        
//==================================================================================
// CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
//                              HVE2DVector::ArbitraryDirection pi_Direction = HVE2DVector::BETA) const;
//==================================================================================
TEST_F (HVE2DSegmentTester, AngularAccelerationTest)
    {

    // Obtain angular acceleration ALPHA of a vertical segment
    ASSERT_NEAR(0.0, VerticalSegment1.CalculateAngularAcceleration(VerticalMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a vertical segment
    ASSERT_NEAR(0.0, VerticalSegment1.CalculateAngularAcceleration(VerticalMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a inverted vertical segment
    ASSERT_NEAR(0.0, VerticalSegment2.CalculateAngularAcceleration(VerticalMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a inverted vertical segment
    ASSERT_NEAR(0.0, VerticalSegment2.CalculateAngularAcceleration(VerticalMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of an horizontal segment
    ASSERT_NEAR(0.0, HorizontalSegment1.CalculateAngularAcceleration(HorizontalMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of an horizontal segment
    ASSERT_NEAR(0.0, HorizontalSegment1.CalculateAngularAcceleration(HorizontalMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of an inverted horizontal segment
    ASSERT_NEAR(0.0, HorizontalSegment2.CalculateAngularAcceleration(HorizontalMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of an inverted horizontal segment
    ASSERT_NEAR(0.0, HorizontalSegment2.CalculateAngularAcceleration(HorizontalMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a positive slope segment
    ASSERT_NEAR(0.0, MiscSegment1.CalculateAngularAcceleration(MiscMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a positive slope segment
    ASSERT_NEAR(0.0, MiscSegment1.CalculateAngularAcceleration(MiscMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a inverted positive slope segment
    ASSERT_NEAR(0.0, MiscSegment2.CalculateAngularAcceleration(MiscMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a inverted positive slope segment
    ASSERT_NEAR(0.0, MiscSegment2.CalculateAngularAcceleration(MiscMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a vertical EPSILON SIZED segment
    ASSERT_NEAR(0.0, VerticalSegment3.CalculateAngularAcceleration(VerticalMidPoint3, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a vertical EPSILON SIZED segment
    ASSERT_NEAR(0.0, VerticalSegment3.CalculateAngularAcceleration(VerticalMidPoint3, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a horizontal EPSILON SIZED segment
    ASSERT_NEAR(0.0, HorizontalSegment3.CalculateAngularAcceleration(HorizontalMidPoint3, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a horizontal EPSILON SIZED segment
    ASSERT_NEAR(0.0, HorizontalSegment3.CalculateAngularAcceleration(HorizontalMidPoint3, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a miscaleniuous EPSILON SIZED segment
    ASSERT_NEAR(0.0, MiscSegment3.CalculateAngularAcceleration(MiscMidPoint3, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a miscaleniuous EPSILON SIZED segment
    ASSERT_NEAR(0.0, MiscSegment3.CalculateAngularAcceleration(MiscMidPoint3, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a very large segment
    ASSERT_NEAR(0.0, LargeSegment1.CalculateAngularAcceleration(LargeMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a very large segment
    ASSERT_NEAR(0.0, LargeSegment1.CalculateAngularAcceleration(LargeMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a inverted very large segment
    ASSERT_NEAR(0.0, LargeSegment2.CalculateAngularAcceleration(LargeMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a inverted very large segment
    ASSERT_NEAR(0.0, LargeSegment2.CalculateAngularAcceleration(LargeMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a segment way into negative coordinates
    ASSERT_NEAR(0.0, NegativeSegment1.CalculateAngularAcceleration(NegativeMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a segment way into negative coordinates
    ASSERT_NEAR(0.0, NegativeSegment1.CalculateAngularAcceleration(NegativeMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a inverted segment way into negative coordinates
    ASSERT_NEAR(0.0, NegativeSegment2.CalculateAngularAcceleration(NegativeMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a inverted segment way into negative coordinates
    ASSERT_NEAR(0.0, NegativeSegment2.CalculateAngularAcceleration(NegativeMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a segment way into positive values
    ASSERT_NEAR(0.0, PositiveSegment1.CalculateAngularAcceleration(PositiveMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a segment way into positive values
    ASSERT_NEAR(0.0, PositiveSegment1.CalculateAngularAcceleration(PositiveMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a inverted segment way into positive values
    ASSERT_NEAR(0.0, PositiveSegment2.CalculateAngularAcceleration(PositiveMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a inverted segment way into positive values
    ASSERT_NEAR(0.0, PositiveSegment2.CalculateAngularAcceleration(PositiveMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a NULL segment
    ASSERT_NEAR(0.0, NullSegment1.CalculateAngularAcceleration(HGF2DLocation(0.0, 0.0, pWorld), HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a NULL segment
    ASSERT_NEAR(0.0, NullSegment1.CalculateAngularAcceleration(HGF2DLocation(0.0, 0.0, pWorld), HVE2DVector::ALPHA), MYEPSILON);

    }

//==================================================================================
// Extent calculation test
// GetExtent() const;
//==================================================================================
TEST_F (HVE2DSegmentTester, GetExtentTest)
    {

    // Obtain extent of a vertical segment
    ASSERT_DOUBLE_EQ(0.10, VerticalSegment1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.10, VerticalSegment1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(0.10, VerticalSegment1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(10.1, VerticalSegment1.GetExtent().GetYMax());

    // Obtain extent of a inverted vertical segment
    ASSERT_DOUBLE_EQ(0.10, VerticalSegment2.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.10, VerticalSegment2.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(0.10, VerticalSegment2.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(10.1, VerticalSegment2.GetExtent().GetYMax());

    // Obtain extent of an horizontal segment
    ASSERT_DOUBLE_EQ(0.10, HorizontalSegment1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.10, HorizontalSegment1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(10.1, HorizontalSegment1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(0.10, HorizontalSegment1.GetExtent().GetYMax());

    // Obtain extent of an inverted horizontal segment
    ASSERT_DOUBLE_EQ(0.10, HorizontalSegment2.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.10, HorizontalSegment2.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(10.1, HorizontalSegment2.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(0.10, HorizontalSegment2.GetExtent().GetYMax());

    // Obtain extent of a positive slope segment
    ASSERT_DOUBLE_EQ(0.10, MiscSegment1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.10, MiscSegment1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(10.1, MiscSegment1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(10.1, MiscSegment1.GetExtent().GetYMax());

    // Obtain extent of a vertical EPSILON SIZED segment
    ASSERT_DOUBLE_EQ(0.1000000, VerticalSegment3.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.1000000, VerticalSegment3.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(0.1000000, VerticalSegment3.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(0.1000001, VerticalSegment3.GetExtent().GetYMax());

    // Obtain extent of a horizontal EPSILON SIZED segment
    ASSERT_DOUBLE_EQ(0.1000000, HorizontalSegment3.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.1000000, HorizontalSegment3.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(0.1000001, HorizontalSegment3.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(0.1000000, HorizontalSegment3.GetExtent().GetYMax());

    // Obtain extent of a miscaleniuous EPSILON SIZED segment
    ASSERT_DOUBLE_EQ(0.10000000000000000, MiscSegment3.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.10000000000000000, MiscSegment3.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(0.10000000000000000, VerticalSegment1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(0.10000009762960071, MiscSegment3.GetExtent().GetYMax());

    // Obtain extent of a very large segment
    ASSERT_DOUBLE_EQ(-1.00E123, LargeSegment1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(-21.0E123, LargeSegment1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(9.000E123, LargeSegment1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(19.00E123, LargeSegment1.GetExtent().GetYMax());

    // Obtain extent of a segment way into negative coordinates
    ASSERT_DOUBLE_EQ(-11.0E123, NegativeSegment1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(-41.0E123, NegativeSegment1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(-1.00E123, NegativeSegment1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(-21.0E123, NegativeSegment1.GetExtent().GetYMax());

    // Obtain extent of a segment way into positive values
    ASSERT_DOUBLE_EQ(1.00E123, PositiveSegment1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(21.0E123, PositiveSegment1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(11.0E123, PositiveSegment1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(41.0E123, PositiveSegment1.GetExtent().GetYMax());

    // Obtain extent of a NULL segment 
    ASSERT_NEAR(0.0, NullSegment1.GetExtent().GetXMin(), MYEPSILON);
    ASSERT_NEAR(0.0, NullSegment1.GetExtent().GetYMin(), MYEPSILON);
    ASSERT_NEAR(0.0, NullSegment1.GetExtent().GetXMax(), MYEPSILON);
    ASSERT_NEAR(0.0, NullSegment1.GetExtent().GetYMax(), MYEPSILON);

    }

//==================================================================================
// Rotate(const HGFAngle& pi_rAngle, const HGF2DLocation& pi_rOrigin);
//==================================================================================
TEST_F (HVE2DSegmentTester, RotateTest)
    {

    // Obtain rotate of a vertical segment
    VerticalSegment1.Rotate(PI, HGF2DLocation(0.0, 0.0, pWorld));
    ASSERT_DOUBLE_EQ(-0.10000000000000000, VerticalSegment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.10000000000000000, VerticalSegment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-0.10000000000000124, VerticalSegment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-10.1000000000000000, VerticalSegment1.GetEndPoint().GetY());

    // Obtain rotate of a inverted vertical segment
    VerticalSegment2.Rotate(PI, HGF2DLocation(0.0, 0.0, pWorld));
    ASSERT_DOUBLE_EQ(-0.10000000000000124, VerticalSegment2.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-10.1000000000000000, VerticalSegment2.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-0.10000000000000000, VerticalSegment2.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.10000000000000000, VerticalSegment2.GetEndPoint().GetY());

    // Obtain rotate of an horizontal segment
    HorizontalSegment1.Rotate(PI, HGF2DLocation(0.0, 0.0, pWorld));
    ASSERT_DOUBLE_EQ(-0.10000000000000000, HorizontalSegment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.10000000000000000, HorizontalSegment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-10.1000000000000000, HorizontalSegment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.09999999999999877, HorizontalSegment1.GetEndPoint().GetY());

    // Obtain rotate of an inverted horizontal segment
    HorizontalSegment2.Rotate(PI, HGF2DLocation(0.0, 0.0, pWorld));
    ASSERT_DOUBLE_EQ(-10.1000000000000000, HorizontalSegment2.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.09999999999999877, HorizontalSegment2.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-0.10000000000000000, HorizontalSegment2.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.10000000000000000, HorizontalSegment2.GetEndPoint().GetY());

    // Obtain rotate of a positive slope segment
    MiscSegment1.Rotate(PI, HGF2DLocation(0.0, 0.0, pWorld));
    ASSERT_DOUBLE_EQ(-0.10, MiscSegment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.10, MiscSegment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-10.1, MiscSegment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-10.1, MiscSegment1.GetEndPoint().GetY());

    // Obtain rotate of a vertical EPSILON SIZED segment
    VerticalSegment3.Rotate(PI, HGF2DLocation(0.0, 0.0, pWorld));
    ASSERT_DOUBLE_EQ(-0.10000000000000000, VerticalSegment3.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.10000000000000000, VerticalSegment3.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-0.10000000000000000, VerticalSegment3.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.10000009999999999, VerticalSegment3.GetEndPoint().GetY());

    // Obtain rotate of a horizontal EPSILON SIZED segment
    HorizontalSegment3.Rotate(PI, HGF2DLocation(0.0, 0.0, pWorld));
    ASSERT_DOUBLE_EQ(-0.10000000000000000, HorizontalSegment3.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.10000000000000000, HorizontalSegment3.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-0.10000009999999999, HorizontalSegment3.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.10000000000000000, HorizontalSegment3.GetEndPoint().GetY());

    // Obtain rotate of a very large segment
    LargeSegment1.Rotate(PI, HGF2DLocation(0.0, 0.0, pWorld));
    ASSERT_DOUBLE_EQ(1.00000000000000260E123, LargeSegment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(2.10000000000000010E124, LargeSegment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-8.9999999999999997E123, LargeSegment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-1.8999999999999999E124, LargeSegment1.GetEndPoint().GetY());

    // Obtain rotate of a segment way into negative coordinates
    NegativeSegment1.Rotate(PI, HGF2DLocation(0.0, 0.0, pWorld));
    ASSERT_DOUBLE_EQ(1.0000000000000026E123, NegativeSegment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(2.1000000000000001E124, NegativeSegment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(1.1000000000000001E124, NegativeSegment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(4.1000000000000000E124, NegativeSegment1.GetEndPoint().GetY());

    // Obtain rotate of a segment way into positive values
    PositiveSegment1.Rotate(PI, HGF2DLocation(0.0, 0.0, pWorld));
    ASSERT_DOUBLE_EQ(-1.0000000000000026E123, PositiveSegment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-2.1000000000000001E124, PositiveSegment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-1.1000000000000001E124, PositiveSegment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-4.1000000000000000E124, PositiveSegment1.GetEndPoint().GetY());

    // Obtain rotate of a NULL segment
    NullSegment1.Rotate(PI, HGF2DLocation(0.0, 0.0, pWorld));
    ASSERT_TRUE(NullSegment1.IsNull());

    }

//==================================================================================
// Additional tests for contiguousness who used to fail
//==================================================================================
TEST_F (HVE2DSegmentTester, ContiguousTestWhoFailed)
    {

    HVE2DSegment    HorizontalSegmentSup1(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld));
    HVE2DSegment    HorizontalSegmentSup2(HGF2DLocation(20.0, 10.0, pWorld), HGF2DLocation(35.0, 10.0, pWorld));

    ASSERT_FALSE(HorizontalSegmentSup1.AreContiguous(HorizontalSegmentSup2));
    ASSERT_FALSE(HorizontalSegmentSup2.AreContiguous(HorizontalSegmentSup1));

    HVE2DSegment    VerticalSegmentSup1(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(10.0, 20.0, pWorld));
    HVE2DSegment    VerticalSegmentSup2(HGF2DLocation(10.0, 20.0, pWorld), HGF2DLocation(10.0, 35.0, pWorld));

    ASSERT_FALSE(VerticalSegmentSup1.AreContiguous(VerticalSegmentSup2));
    ASSERT_FALSE(VerticalSegmentSup2.AreContiguous(VerticalSegmentSup1));

    // Other test which failed
    // all of the following are contiguous
    HVE2DSegment    HorizontalSegmentSup3(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld));
    HVE2DSegment    HorizontalSegmentSup4(HGF2DLocation(5.0, 10.0, pWorld), HGF2DLocation(35.0, 10.0, pWorld));
    HVE2DSegment    HorizontalSegmentSup5(HGF2DLocation(5.0, 10.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld));
    HVE2DSegment    HorizontalSegmentSup6(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(25.0, 10.0, pWorld));
    HVE2DSegment    HorizontalSegmentSup7(HGF2DLocation(5.0, 10.0, pWorld), HGF2DLocation(15.0, 10.0, pWorld));
    HVE2DSegment    HorizontalSegmentSup8(HGF2DLocation(13.0, 10.0, pWorld), HGF2DLocation(25.0, 10.0, pWorld));
    HVE2DSegment    HorizontalSegmentSup9(HGF2DLocation(12.0, 10.0, pWorld), HGF2DLocation(18.0, 10.0, pWorld));

    HVE2DSegment    HorizontalSegmentSup10(HGF2DLocation(20.0, 10.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    HVE2DSegment    HorizontalSegmentSup11(HGF2DLocation(35.0, 10.0, pWorld), HGF2DLocation(5.0, 10.0, pWorld));
    HVE2DSegment    HorizontalSegmentSup12(HGF2DLocation(20.0, 10.0, pWorld), HGF2DLocation(5.0, 10.0, pWorld));
    HVE2DSegment    HorizontalSegmentSup13(HGF2DLocation(25.0, 10.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    HVE2DSegment    HorizontalSegmentSup14(HGF2DLocation(15.0, 10.0, pWorld), HGF2DLocation(5.0, 10.0, pWorld));
    HVE2DSegment    HorizontalSegmentSup15(HGF2DLocation(25.0, 10.0, pWorld), HGF2DLocation(13.0, 10.0, pWorld));
    HVE2DSegment    HorizontalSegmentSup16(HGF2DLocation(18.0, 10.0, pWorld), HGF2DLocation(12.0, 10.0, pWorld));

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
    HVE2DSegment    VerticalSegmentSup3(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(10.0, 20.0, pWorld));
    HVE2DSegment    VerticalSegmentSup4(HGF2DLocation(10.0, 5.00, pWorld), HGF2DLocation(10.0, 35.0, pWorld));
    HVE2DSegment    VerticalSegmentSup5(HGF2DLocation(10.0, 5.00, pWorld), HGF2DLocation(10.0, 20.0, pWorld));
    HVE2DSegment    VerticalSegmentSup6(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(10.0, 25.0, pWorld));
    HVE2DSegment    VerticalSegmentSup7(HGF2DLocation(10.0, 5.00, pWorld), HGF2DLocation(10.0, 15.0, pWorld));
    HVE2DSegment    VerticalSegmentSup8(HGF2DLocation(10.0, 13.0, pWorld), HGF2DLocation(10.0, 25.0, pWorld));
    HVE2DSegment    VerticalSegmentSup9(HGF2DLocation(10.0, 12.0, pWorld), HGF2DLocation(10.0, 18.0, pWorld));

    HVE2DSegment    VerticalSegmentSup10(HGF2DLocation(10.0, 20.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    HVE2DSegment    VerticalSegmentSup11(HGF2DLocation(10.0, 35.0, pWorld), HGF2DLocation(10.0, 5.00, pWorld));
    HVE2DSegment    VerticalSegmentSup12(HGF2DLocation(10.0, 20.0, pWorld), HGF2DLocation(10.0, 5.00, pWorld));
    HVE2DSegment    VerticalSegmentSup13(HGF2DLocation(10.0, 25.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    HVE2DSegment    VerticalSegmentSup14(HGF2DLocation(10.0, 15.0, pWorld), HGF2DLocation(10.0, 5.00, pWorld));
    HVE2DSegment    VerticalSegmentSup15(HGF2DLocation(10.0, 25.0, pWorld), HGF2DLocation(10.0, 13.0, pWorld));
    HVE2DSegment    VerticalSegmentSup16(HGF2DLocation(10.0, 18.0, pWorld), HGF2DLocation(10.0, 12.0, pWorld));

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
TEST_F (HVE2DSegmentTester, ObtainContiguousnessPointsWhoFailed)
    {  
       
    HVE2DSegment    HorizontalSegmentSup23(HGF2DLocation(20.0, 20.0, pWorld), HGF2DLocation(10.0, 20.0, pWorld));
    HVE2DSegment    HorizontalSegmentSup24(HGF2DLocation(20.0+0.9*MYEPSILON, 20.0, pWorld), HGF2DLocation(-1.0, 20.0, pWorld));

    HGF2DLocationCollection     Contig23Points;
    ASSERT_EQ(2, HorizontalSegmentSup23.ObtainContiguousnessPoints(HorizontalSegmentSup24, &Contig23Points));

    }

//==================================================================================
// Another yet test which did fail (July 23 1997)
//==================================================================================
TEST_F (HVE2DSegmentTester, ObtainContiguousnessPointsWhoFailed2)
    {  
              
    HVE2DSegment    HorizontalSegmentSup1(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(0.0, 1E-6, pWorld));
    HVE2DSegment    HorizontalSegmentSup2(HGF2DLocation(0.0, 9.999999E-7, pWorld), HGF2DLocation(0.0, 7.0, pWorld));

    ASSERT_FALSE(HorizontalSegmentSup1.AreContiguous(HorizontalSegmentSup2));

    }

//==================================================================================
// Test which failed on Nov 15, 2000
//==================================================================================
TEST_F (HVE2DSegmentTester, CalculateLengthTestWhoFailed)
    {   

    HVE2DSegment MySegment(HGF2DLocation(410416.98821556, -3712392.8951395, pWorld),
                            HGF2DLocation(410416.98823327, -3711942.8951395, pWorld));

    HGF2DLocation IntermediatePoint(410416.98857254, -3712167.8951158, pWorld);

    HGF2DLocation ClosestPoint = MySegment.CalculateClosestPoint(IntermediatePoint);

    ASSERT_TRUE((ClosestPoint - IntermediatePoint).CalculateLength() < 0.001);

    }

