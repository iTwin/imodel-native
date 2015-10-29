////:>--------------------------------------------------------------------------------------+
////:>
////:>     $Source: Tests/IppGraLibs/HVE2DPolygonOfSegmentsTester.cpp $
////:>
////:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
////:>
////:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "EnvironnementTest.h"
#include "HVE2DPolygonOfSegmentsTester.h"

// THE PRESENT TEST CANNOT MAKE TESTS WITH NON-LINEAR TRANSFORMATION MODELS

HVE2DPolygonOfSegmentsTester::HVE2DPolygonOfSegmentsTester() 
    {

    // Polygons
    Poly1 = HVE2DPolygonOfSegments(Rect1);

    NorthContiguousPoly = HVE2DPolygonOfSegments(NorthContiguousRect);
    EastContiguousPoly = HVE2DPolygonOfSegments(EastContiguousRect);
    WestContiguousPoly = HVE2DPolygonOfSegments(WestContiguousRect);
    SouthContiguousPoly = HVE2DPolygonOfSegments(SouthContiguousRect);

    NETipPoly = HVE2DPolygonOfSegments(NETipRect);
    NWTipPoly = HVE2DPolygonOfSegments(NWTipRect);
    SETipPoly = HVE2DPolygonOfSegments(SETipRect);
    SWTipPoly = HVE2DPolygonOfSegments(SWTipRect);

    VerticalFitPoly = HVE2DPolygonOfSegments(VerticalFitRect);
    HorizontalFitPoly = HVE2DPolygonOfSegments(HorizontalFitRect);

    DisjointPoly = HVE2DPolygonOfSegments(DisjointRect);
    NegativePoly = HVE2DPolygonOfSegments(NegativeRect);

    MiscPoly1 = HVE2DPolygonOfSegments(MiscRect1);

    EnglobPoly1 = HVE2DPolygonOfSegments(EnglobRect1);
    EnglobPoly2 = HVE2DPolygonOfSegments(EnglobRect2);
    EnglobPoly3 = HVE2DPolygonOfSegments(EnglobRect3);

    IncludedPoly1 = HVE2DPolygonOfSegments(IncludedRect1);
    IncludedPoly2 = HVE2DPolygonOfSegments(IncludedRect2);
    IncludedPoly3 = HVE2DPolygonOfSegments(IncludedRect3);
    IncludedPoly4 = HVE2DPolygonOfSegments(IncludedRect4);
    IncludedPoly5 = HVE2DPolygonOfSegments(IncludedRect5);
    IncludedPoly6 = HVE2DPolygonOfSegments(IncludedRect6);
    IncludedPoly7 = HVE2DPolygonOfSegments(IncludedRect7);
    IncludedPoly8 = HVE2DPolygonOfSegments(IncludedRect8);
    IncludedPoly9 = HVE2DPolygonOfSegments(IncludedRect9);

    PolyClosePoint1A = HGF2DLocation(21.1, 10.1, pWorld);
    PolyClosePoint1B = HGF2DLocation(9.0, 9.0, pWorld);
    PolyClosePoint1C = HGF2DLocation(19.9, 10.0, pWorld);
    PolyClosePoint1D = HGF2DLocation(0.1, 15.0, pWorld);
    PolyCloseMidPoint1 = HGF2DLocation(15.0, 20.1, pWorld);

    Poly1Point0d0 = HGF2DLocation(10.0, 10.0, pWorld);
    Poly1Point0d1 = HGF2DLocation(15.0, 10.0, pWorld);
    Poly1Point0d5 = HGF2DLocation(20.0, 20.0, pWorld);
    Poly1Point1d0 = HGF2DLocation(10.0, 10.0+(1.1 * MYEPSILON), pWorld);

    PolyMidPoint1 = HGF2DLocation(15.0, 20.0, pWorld);

    //Array
    DblArray[0] = 0.0;
    DblArray[1] = 0.0;

    DblArray[2] = 10.0;
    DblArray[3] = 10.0;

    DblArray[4] = 15.0;
    DblArray[5] = 5.0;

    DblArray[6] = 0.0;
    DblArray[7] = 5.0;

    DblArray[8] = 5.0;
    DblArray[9] = 0.0;

    DblArray[10] = 20.0;
    DblArray[11] = 20.0;

    DblArray[12] = -20.0;
    DblArray[13] = 20.0;

    DblArray[14] = 0.0;
    DblArray[15] = 0.0;

    MyPolyCount = 16;

    }

//==================================================================================
//Construction tests
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ConstructorTest)
    {

    // Default Constructor
    HVE2DPolygonOfSegments    APoly1;

    // Constructor with a coordinate system
    HVE2DPolygonOfSegments    APoly2(pWorld);
    ASSERT_EQ(pWorld, APoly2.GetCoordSys());

    // Constructor with HVE2DComplexLinear
    HVE2DComplexLinear LinearTest(pWorld);
    LinearTest.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld)));
    LinearTest.AppendLinear(HVE2DSegment(HGF2DLocation(20.0, 10.0, pWorld), HGF2DLocation(20.0, 20.0, pWorld)));
    LinearTest.AppendLinear(HVE2DSegment(HGF2DLocation(20.0, 20.0, pWorld), HGF2DLocation(10.0, 20.0, pWorld)));
    LinearTest.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 20.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld)));
    
    HVE2DPolygonOfSegments PolyTest(LinearTest);
    ASSERT_EQ(pWorld, PolyTest.GetCoordSys());

    //Constructor from Rectangle
    HVE2DPolygonOfSegments PolyTest2(Rect1);   
    ASSERT_EQ(pWorld, PolyTest2.GetCoordSys());
    ASSERT_TRUE(PolyTest2.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(PolyTest2.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(PolyTest2.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(PolyTest2.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    //Constructor from PolySegment
    HVE2DPolySegment    PolySegment1(pWorld);
    PolySegment1.AppendPoint(HGF2DLocation(1.0, 1.0, pWorld));
    PolySegment1.AppendPoint(HGF2DLocation(2.0, 2.0, pWorld));
    PolySegment1.AppendPoint(HGF2DLocation(3.0, 2.0, pWorld));
    PolySegment1.AppendPoint(HGF2DLocation(1.0, 1.0, pWorld));

    HVE2DPolygonOfSegments PolyTest3(PolySegment1);
    ASSERT_EQ(pWorld, PolyTest3.GetCoordSys());
    ASSERT_TRUE(PolyTest3.IsPointOn(HGF2DLocation(1.0, 1.0, pWorld)));
    ASSERT_TRUE(PolyTest3.IsPointOn(HGF2DLocation(2.0, 2.0, pWorld)));
    ASSERT_TRUE(PolyTest3.IsPointOn(HGF2DLocation(3.0, 2.0, pWorld)));

    // Copy Constructor test
    HVE2DSegment    Segment1(HGF2DLocation(10.0, 10.1, pWorld), HGF2DLocation(10.0, 20.1, pWorld));
    HVE2DSegment    Segment2(HGF2DLocation(10.0, 20.1, pWorld), HGF2DLocation(20.0, 20.1, pWorld));
    HVE2DSegment    Segment3(HGF2DLocation(20.0, 20.1, pWorld), HGF2DLocation(20.0, 10.1, pWorld));
    HVE2DSegment    Segment4(HGF2DLocation(20.0, 10.1, pWorld), HGF2DLocation(10.0, 10.1, pWorld));

    HVE2DComplexLinear  AComp1(pWorld);
    AComp1.AppendLinear(Segment1);
    AComp1.AppendLinear(Segment2);
    AComp1.AppendLinear(Segment3);
    AComp1.AppendLinear(Segment4);

    HVE2DPolygonOfSegments    APoly3(AComp1);
    HVE2DPolygonOfSegments    APoly4(APoly3);
    ASSERT_EQ(pWorld, APoly4.GetCoordSys());

    HVE2DComplexLinear  AComp2 = APoly4.GetLinear();  
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.1, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.1, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.1, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.1, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, AComp2.GetLinear(3).GetEndPoint().GetY());

    //Constructor from array
    HVE2DPolygonOfSegments APoly5(MyPolyCount, DblArray, pWorld);

    ASSERT_EQ(pWorld, APoly5.GetCoordSys());

    ASSERT_TRUE(APoly5.IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(APoly5.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(APoly5.IsPointOn(HGF2DLocation(15.0, 5.0, pWorld)));
    ASSERT_TRUE(APoly5.IsPointOn(HGF2DLocation(0.0, 5.0, pWorld)));
    ASSERT_TRUE(APoly5.IsPointOn(HGF2DLocation(5.0, 0.0, pWorld)));
    ASSERT_TRUE(APoly5.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(APoly5.IsPointOn(HGF2DLocation(-20.0, 20.0, pWorld)));
    
    }

//==================================================================================
// operator= test
// operator=(const HVE2DPolygonOfSegments& pi_rObj);
// SetLinear(const HVE2DLinear& pi_rLinear);
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  OperatorTest)
    {

    HVE2DPolygonOfSegments    APoly1(pWorld);
    HVE2DSegment    Segment1(HGF2DLocation(10.0, 10.1, pWorld), HGF2DLocation(10.0, 20.1, pWorld));
    HVE2DSegment    Segment2(HGF2DLocation(10.0, 20.1, pWorld), HGF2DLocation(20.0, 20.1, pWorld));
    HVE2DSegment    Segment3(HGF2DLocation(20.0, 20.1, pWorld), HGF2DLocation(20.0, 10.1, pWorld));
    HVE2DSegment    Segment4(HGF2DLocation(20.0, 10.1, pWorld), HGF2DLocation(10.0, 10.1, pWorld));

    HVE2DComplexLinear  AComp1(pWorld);
    AComp1.AppendLinear(Segment1);
    AComp1.AppendLinear(Segment2);
    AComp1.AppendLinear(Segment3);
    AComp1.AppendLinear(Segment4);
    APoly1.SetLinear(AComp1);

    HVE2DPolygonOfSegments    APoly2(pSys1);

    APoly2 = APoly1;

    ASSERT_EQ(pWorld, APoly2.GetCoordSys());

    HVE2DComplexLinear  AComp2 = APoly2.GetLinear();
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.1, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.1, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.1, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.1, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, AComp2.GetLinear(3).GetEndPoint().GetY());

    }

//==================================================================================
// GenerateCorrespondingRectangle() const;
// RepresentsARectangle() const;
//==================================================================================
TEST_F (HVE2DPolygonOfSegmentsTester, RectangleTest)
    {
    
    ASSERT_TRUE(IncludedPoly1.RepresentsARectangle());

    HVE2DRectangle* rectangle = IncludedPoly1.GenerateCorrespondingRectangle();
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), rectangle->GetShapeType());
    ASSERT_TRUE(rectangle->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));

    HVE2DComplexLinear  Triangle(pWorld);
    Triangle.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(5.0, 5.0, pWorld)));
    Triangle.AppendLinear(HVE2DSegment(HGF2DLocation(5.0, 5.0, pWorld), HGF2DLocation(0.0, 5.0, pWorld)));
    Triangle.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 5.0, pWorld), HGF2DLocation(0.0, 0.0, pWorld)));

    HVE2DPolygonOfSegments APolyTest(Triangle);

    ASSERT_FALSE(APolyTest.RepresentsARectangle());

    }

//==================================================================================
// IsConvex() const;
//==================================================================================
TEST_F (HVE2DPolygonOfSegmentsTester, IsConvexTest)
    {

    ASSERT_TRUE(NETipPoly.IsConvex());
    ASSERT_TRUE(NWTipPoly.IsConvex());
    ASSERT_TRUE(SETipPoly.IsConvex());
    ASSERT_TRUE(SWTipPoly.IsConvex());

    HVE2DComplexLinear  ConcaveComplexLinear1(pWorld);
    ConcaveComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(0.0, 10.0, pWorld)));
    ConcaveComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 10.0, pWorld), HGF2DLocation(5.0, 5.0, pWorld)));
    ConcaveComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(5.0, 5.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld)));
    ConcaveComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(0.0, 10.0, pWorld)));
    ConcaveComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 10.0, pWorld), HGF2DLocation(0.0, 0.0, pWorld)));

    HVE2DPolygonOfSegments ConcavePoly1(ConcaveComplexLinear1);

    ASSERT_FALSE(ConcavePoly1.IsConvex());

    HVE2DComplexLinear  ConcaveComplexLinear2(pWorld);
    ConcaveComplexLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(0.0, -10.0, pWorld)));
    ConcaveComplexLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, -10.0, pWorld), HGF2DLocation(5.0, -5.0, pWorld)));
    ConcaveComplexLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(5.0, -5.0, pWorld), HGF2DLocation(10.0, -10.0, pWorld)));
    ConcaveComplexLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, -10.0, pWorld), HGF2DLocation(10.0, 0.0, pWorld)));
    ConcaveComplexLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 0.0, pWorld), HGF2DLocation(5.0, -5.0, pWorld)));
    ConcaveComplexLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(5.0, -5.0, pWorld), HGF2DLocation(0.0, 0.0, pWorld)));

    HVE2DPolygonOfSegments ConcavePoly2(ConcaveComplexLinear2);

    ASSERT_FALSE(ConcavePoly2.IsConvex());

    }

//==================================================================================
// Rotate(const HGFAngle& pi_rAngle, const HGF2DLocation& pi_rRotationOrigin);
//==================================================================================
TEST_F (HVE2DPolygonOfSegmentsTester, RotateTest)
    {

    HVE2DPolySegment    RotateTest1(pWorld);

    RotateTest1.AppendPoint(HGF2DLocation(1.0, 1.0, pWorld));
    RotateTest1.AppendPoint(HGF2DLocation(2.0, 2.0, pWorld));
    RotateTest1.AppendPoint(HGF2DLocation(3.0, 2.0, pWorld));
    RotateTest1.AppendPoint(HGF2DLocation(3.0, 3.0, pWorld));
    RotateTest1.AppendPoint(HGF2DLocation(1.0, 1.0, pWorld));

    HVE2DPolygonOfSegments PolyTest1(RotateTest1);

    PolyTest1.Rotate(PI, HGF2DLocation(0.0, 0.0, pWorld)); 

    ASSERT_TRUE(PolyTest1.IsPointOn(HGF2DLocation(-1.0, -1.0, pWorld)));
    ASSERT_TRUE(PolyTest1.IsPointOn(HGF2DLocation(-2.0, -2.0, pWorld)));
    ASSERT_TRUE(PolyTest1.IsPointOn(HGF2DLocation(-3.0, -2.0, pWorld)));
    ASSERT_TRUE(PolyTest1.IsPointOn(HGF2DLocation(-3.0, -3.0, pWorld)));

    PolyTest1.Rotate(-PI, HGF2DLocation(0.0, 0.0, pWorld)); 

    ASSERT_TRUE(PolyTest1.IsPointOn(HGF2DLocation(1.0, 1.0, pWorld)));
    ASSERT_TRUE(PolyTest1.IsPointOn(HGF2DLocation(2.0, 2.0, pWorld)));
    ASSERT_TRUE(PolyTest1.IsPointOn(HGF2DLocation(3.0, 2.0, pWorld)));
    ASSERT_TRUE(PolyTest1.IsPointOn(HGF2DLocation(3.0, 3.0, pWorld)));
    
    PolyTest1.Rotate(PI, HGF2DLocation(1.0, 1.0, pWorld)); 

    ASSERT_TRUE(PolyTest1.IsPointOn(HGF2DLocation(1.00, 1.00, pWorld)));
    ASSERT_TRUE(PolyTest1.IsPointOn(HGF2DLocation(0.00, 0.00, pWorld)));
    ASSERT_TRUE(PolyTest1.IsPointOn(HGF2DLocation(-1.0, 0.00, pWorld)));
    ASSERT_TRUE(PolyTest1.IsPointOn(HGF2DLocation(-1.0, -1.0, pWorld)));

    }

//==================================================================================
// Move(const HGF2DDisplacement& pi_rDisplacement);
//==================================================================================
TEST_F (HVE2DPolygonOfSegmentsTester, MoveTest)
    {

    HGF2DDisplacement Translation1(10.0, 10.0);
    HGF2DDisplacement Translation2(0.0, 10.0);
    HGF2DDisplacement Translation3(10.0, 0.0);
    HGF2DDisplacement Translation4(0.0, 0.0);
    HGF2DDisplacement Translation5(-10.0, 10.0);
    HGF2DDisplacement Translation6(10.0, -10.0);
    
    HVE2DPolygonOfSegments Poly1(IncludedRect1);
    Poly1.Move(Translation1);

    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(25.0, 25.0, pWorld)));
 
    HVE2DPolygonOfSegments Poly2(IncludedRect2);
    Poly2.Move(Translation2);
    
    ASSERT_TRUE(Poly2.IsPointOn(HGF2DLocation(15.0, 20.0, pWorld)));
    ASSERT_TRUE(Poly2.IsPointOn(HGF2DLocation(20.0, 25.0, pWorld)));

    HVE2DPolygonOfSegments Poly3(IncludedRect3);
    Poly3.Move(Translation3);

    ASSERT_TRUE(Poly3.IsPointOn(HGF2DLocation(25.0, 15.0, pWorld)));
    ASSERT_TRUE(Poly3.IsPointOn(HGF2DLocation(30.0, 20.0, pWorld)));
   
    HVE2DPolygonOfSegments Poly4(IncludedRect4);
    Poly4.Move(Translation4);
 
    ASSERT_TRUE(Poly4.IsPointOn(HGF2DLocation(10.0, 15.0, pWorld)));
    ASSERT_TRUE(Poly4.IsPointOn(HGF2DLocation(15.0, 20.0, pWorld)));
    
    HVE2DPolygonOfSegments Poly5(IncludedRect5);
    Poly5.Move(Translation5);

    ASSERT_TRUE(Poly5.IsPointOn(HGF2DLocation(2.0, 22.0, pWorld)));
    ASSERT_TRUE(Poly5.IsPointOn(HGF2DLocation(8.0, 28.0, pWorld)));
    
    HVE2DPolygonOfSegments Poly6(IncludedRect6);
    Poly6.Move(Translation6);
    
    }

//==================================================================================
// Scale(double pi_ScaleFactor, const HGF2DLocation& pi_rScaleOrigin)
//================================================================================== 
TEST_F (HVE2DPolygonOfSegmentsTester, ScaleTest)
    {

    HGF2DLocation Origin(0.0, 0.0, pWorld);

    HVE2DPolygonOfSegments Poly1(IncludedRect1);
    Poly1.Scale(2.0, Origin);

    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(30.0, 30.0, pWorld)));
    
    HVE2DPolygonOfSegments Poly2(IncludedRect2);
    Poly2.Scale(-2.0, Origin); 

    ASSERT_TRUE(Poly2.IsPointOn(HGF2DLocation(-30.0, -20.0, pWorld)));
    ASSERT_TRUE(Poly2.IsPointOn(HGF2DLocation(-40.0, -30.0, pWorld)));

    HVE2DPolygonOfSegments Poly3(IncludedRect3);
    Poly3.Scale(0.5, Origin); 

    ASSERT_TRUE(Poly3.IsPointOn(HGF2DLocation(7.50, 7.50, pWorld)));
    ASSERT_TRUE(Poly3.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
   
    HVE2DPolygonOfSegments Poly4(IncludedRect4);
    Poly4.Scale(2.0, HGF2DLocation(5.0, 5.0, pWorld));

    ASSERT_TRUE(Poly4.IsPointOn(HGF2DLocation(15.0, 25.0, pWorld)));
    ASSERT_TRUE(Poly4.IsPointOn(HGF2DLocation(25.0, 35.0, pWorld)));
   
    HVE2DPolygonOfSegments Poly5(IncludedRect5);
    Poly5.Scale(2.0, HGF2DLocation(-5.0, 5.0, pWorld));

    ASSERT_TRUE(Poly5.IsPointOn(HGF2DLocation(29.0, 19.0, pWorld)));
    ASSERT_TRUE(Poly5.IsPointOn(HGF2DLocation(41.0, 31.0, pWorld)));
    
    HVE2DPolygonOfSegments Poly6(IncludedRect6);
    Poly6.Scale(2.0, HGF2DLocation(5.0, -5.0, pWorld));

    ASSERT_TRUE(Poly6.IsPointOn(HGF2DLocation(15.0, 25.0, pWorld)));
    ASSERT_TRUE(Poly6.IsPointOn(HGF2DLocation(35.0, 35.0, pWorld)));
    
    HVE2DPolygonOfSegments Poly7(IncludedRect7);
    Poly7.Scale(0.5, HGF2DLocation(5.0, 5.0, pWorld));

    ASSERT_TRUE(Poly7.IsPointOn(HGF2DLocation(7.5, 7.5, pWorld)));
    ASSERT_TRUE(Poly7.IsPointOn(HGF2DLocation(10.0, 12.5, pWorld)));
   
    }

//==================================================================================
// AllocateParallelCopy(const HGFDistance& pi_rOffset,HVE2DVector::ArbitraryDirection pi_DirectionToRight, const HGF2DLine* pi_pFirstPointAlignment,
//                     const HGF2DLine* pi_pLastPointAlignment) const
//==================================================================================
TEST_F (HVE2DPolygonOfSegmentsTester, AllocateParallelCopyTest)
    {

    HFCPtr<HVE2DPolygonOfSegments> APoly1 = (HVE2DPolygonOfSegments*) IncludedPoly1.AllocateParallelCopy(1.0, HVE2DVector::ALPHA);

    ASSERT_TRUE(APoly1->IsPointOn(HGF2DLocation(11.0, 11.0, pWorld)));
    ASSERT_TRUE(APoly1->IsPointOn(HGF2DLocation(11.0, 14.0, pWorld)));
    ASSERT_TRUE(APoly1->IsPointOn(HGF2DLocation(14.0, 11.0, pWorld)));
    ASSERT_TRUE(APoly1->IsPointOn(HGF2DLocation(14.0, 14.0, pWorld)));

    HFCPtr<HVE2DPolygonOfSegments> APoly2 = (HVE2DPolygonOfSegments*) IncludedPoly1.AllocateParallelCopy(1.0, HVE2DVector::BETA);

    ASSERT_TRUE(APoly2->IsPointOn(HGF2DLocation(9.0, 9.0, pWorld)));
    ASSERT_TRUE(APoly2->IsPointOn(HGF2DLocation(9.0, 16.0, pWorld)));
    ASSERT_TRUE(APoly2->IsPointOn(HGF2DLocation(16.0, 9.0, pWorld)));
    ASSERT_TRUE(APoly2->IsPointOn(HGF2DLocation(16.0, 16.0, pWorld)));

    #ifdef WIP_IPPTEST_BUG_14
    HFCPtr<HVE2DPolygonOfSegments> APoly3 = (HVE2DPolygonOfSegments*) IncludedPoly1.AllocateParallelCopy(5.0, HVE2DVector::ALPHA);
    #endif    

    HVE2DComplexLinear ParallelTest(pWorld);
    ParallelTest.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(0.0, 10.0, pWorld)));
    ParallelTest.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 10.0, pWorld), HGF2DLocation(4.0, 10.0, pWorld)));
    ParallelTest.AppendLinear(HVE2DSegment(HGF2DLocation(4.0, 10.0, pWorld), HGF2DLocation(4.0, 2.0, pWorld)));
    ParallelTest.AppendLinear(HVE2DSegment(HGF2DLocation(4.0, 2.0, pWorld), HGF2DLocation(8.0, 2.0, pWorld)));
    ParallelTest.AppendLinear(HVE2DSegment(HGF2DLocation(8.0, 2.0, pWorld), HGF2DLocation(5.0, 10.0, pWorld)));
    ParallelTest.AppendLinear(HVE2DSegment(HGF2DLocation(5.0, 10.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld)));
    ParallelTest.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(10.0, 0.0, pWorld)));
    ParallelTest.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 0.0, pWorld), HGF2DLocation(0.0, 0.0, pWorld)));

    HVE2DPolygonOfSegments APoly4(ParallelTest);

    #ifdef WIP_IPPTEST_BUG_15
    HFCPtr<HVE2DPolygonOfSegments> APoly5 = (HVE2DPolygonOfSegments*) APoly4.AllocateParallelCopy(5.0, HVE2DVector::ALPHA);
    HFCPtr<HVE2DPolygonOfSegments> APoly6 = (HVE2DPolygonOfSegments*) APoly4.AllocateParallelCopy(5.0, HVE2DVector::BETA);
    #endif

    }

//==================================================================================
// GetLinear() const;
// GetLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const;
// AllocateLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const;
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  GetLinearTest)
    {
        
    HVE2DComplexLinear  MyLinearOfPoly1(Poly1.GetLinear());

    // verify that there are 4 linears
    ASSERT_EQ(4, MyLinearOfPoly1.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPoly1.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPoly1.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPoly1.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPoly1.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPoly1.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPoly1.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPoly1.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPoly1.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPoly1.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPoly1.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPoly1.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPoly1.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPoly1.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPoly1.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPoly1.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPoly1.GetLinear(3).GetEndPoint().GetY());

    HVE2DComplexLinear  MyLinearOfPolyCW(Poly1.GetLinear(HVE2DSimpleShape::CW));
    ASSERT_EQ(4, MyLinearOfPolyCW.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCW.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCW.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCW.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPolyCW.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCW.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPolyCW.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPolyCW.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPolyCW.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPolyCW.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPolyCW.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPolyCW.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCW.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPolyCW.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCW.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCW.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCW.GetLinear(3).GetEndPoint().GetY());

    HVE2DComplexLinear  MyLinearOfPolyCCW(Poly1.GetLinear(HVE2DSimpleShape::CCW));
    ASSERT_EQ(4, MyLinearOfPolyCCW.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCCW.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCCW.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPolyCCW.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCCW.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPolyCCW.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCCW.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPolyCCW.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPolyCCW.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPolyCCW.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPolyCCW.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCCW.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPolyCCW.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCCW.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPolyCCW.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCCW.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCCW.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DComplexLinear>  MyPtrLinearOfPolyCCW(Poly1.AllocateLinear(HVE2DSimpleShape::CCW));
    ASSERT_EQ(4, MyPtrLinearOfPolyCCW->GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, MyPtrLinearOfPolyCCW->GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyPtrLinearOfPolyCCW->GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, MyPtrLinearOfPolyCCW->GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyPtrLinearOfPolyCCW->GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, MyPtrLinearOfPolyCCW->GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyPtrLinearOfPolyCCW->GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, MyPtrLinearOfPolyCCW->GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyPtrLinearOfPolyCCW->GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, MyPtrLinearOfPolyCCW->GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyPtrLinearOfPolyCCW->GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, MyPtrLinearOfPolyCCW->GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyPtrLinearOfPolyCCW->GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, MyPtrLinearOfPolyCCW->GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyPtrLinearOfPolyCCW->GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, MyPtrLinearOfPolyCCW->GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyPtrLinearOfPolyCCW->GetLinear(3).GetEndPoint().GetY());

    }

//==================================================================================
// Perimeter calculation test
// CalculatePerimeter() const;
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  CalculatePerimeterTest)
    {
    
    // Test with linear 1
    ASSERT_DOUBLE_EQ(40.0, Poly1.CalculatePerimeter());
    ASSERT_DOUBLE_EQ(40.0, NegativePoly.CalculatePerimeter());

    }

//==================================================================================
// Area Calculation test
// CalculateArea() const;
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  CalculateAreaTest)
    {

    ASSERT_DOUBLE_EQ(100.0, Poly1.CalculateArea());
    ASSERT_DOUBLE_EQ(100.0, NegativePoly.CalculateArea());
  
    }

//==================================================================================
// Drop( HGF2DLocationCollection* po_pPoints, const HGFDistance& pi_rTolerance ) const
//==================================================================================
TEST_F (HVE2DPolygonOfSegmentsTester, DropTest)
    {

    HGF2DLocationCollection Locations;

    HVE2DPolygonOfSegments APoly(Rect1);

    APoly.Drop(&Locations, MYEPSILON);
    ASSERT_EQ(10.0, Locations[0].GetX());
    ASSERT_EQ(10.0, Locations[0].GetY());
    ASSERT_EQ(10.0, Locations[1].GetX());
    ASSERT_EQ(20.0, Locations[1].GetY());
    ASSERT_EQ(20.0, Locations[2].GetX());
    ASSERT_EQ(20.0, Locations[2].GetY());
    ASSERT_EQ(20.0, Locations[3].GetX());
    ASSERT_EQ(10.0, Locations[3].GetY());
    ASSERT_EQ(10.0, Locations[4].GetX());
    ASSERT_EQ(10.0, Locations[4].GetY());

    }

//==================================================================================
// Closest point calculation test
// CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  CalculateClosestPointTest)
    {
    
    // Test with linear 1
    ASSERT_DOUBLE_EQ(20.0, Poly1.CalculateClosestPoint(PolyClosePoint1A).GetX());
    ASSERT_DOUBLE_EQ(10.1, Poly1.CalculateClosestPoint(PolyClosePoint1A).GetY());
    ASSERT_DOUBLE_EQ(10.0, Poly1.CalculateClosestPoint(PolyClosePoint1B).GetX());
    ASSERT_DOUBLE_EQ(10.0, Poly1.CalculateClosestPoint(PolyClosePoint1B).GetY());
    ASSERT_DOUBLE_EQ(19.9, Poly1.CalculateClosestPoint(PolyClosePoint1C).GetX());
    ASSERT_DOUBLE_EQ(10.0, Poly1.CalculateClosestPoint(PolyClosePoint1C).GetY());
    ASSERT_DOUBLE_EQ(10.0, Poly1.CalculateClosestPoint(PolyClosePoint1D).GetX());
    ASSERT_DOUBLE_EQ(15.0, Poly1.CalculateClosestPoint(PolyClosePoint1D).GetY());
    ASSERT_DOUBLE_EQ(15.0, Poly1.CalculateClosestPoint(PolyCloseMidPoint1).GetX());
    ASSERT_DOUBLE_EQ(20.0, Poly1.CalculateClosestPoint(PolyCloseMidPoint1).GetY());

    // Tests with special points
    ASSERT_DOUBLE_EQ(20.0, Poly1.CalculateClosestPoint(VeryFarPoint).GetX());
    ASSERT_DOUBLE_EQ(20.0, Poly1.CalculateClosestPoint(VeryFarPoint).GetY());
    ASSERT_DOUBLE_EQ(15.0, Poly1.CalculateClosestPoint(PolyMidPoint1).GetX());
    ASSERT_DOUBLE_EQ(20.0, Poly1.CalculateClosestPoint(PolyMidPoint1).GetY());
   
    }

//==================================================================================
// Intersection test (with other complex linears only)
// Intersect(const HVE2DVector& pi_rVector, HGF2DLocationCollection* po_pCrossPoints) const;
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  IntersectTest)
    {

    HGF2DLocationCollection   DumPoints;

    // Test with extent disjoint linears
    ASSERT_EQ(0, Poly1.Intersect(DisjointLinear1, &DumPoints));
    DumPoints.clear();

    // Test with disjoint but touching by a side
    ASSERT_EQ(0, Poly1.Intersect(ContiguousExtentLinear1, &DumPoints));
    DumPoints.clear();

    // Test with disjoint but touching by a tip but not linked
    ASSERT_EQ(0, Poly1.Intersect(FlirtingExtentLinear1, &DumPoints));
    DumPoints.clear();

    // Tests with connected linears
    // At start point...
    ASSERT_EQ(0, Poly1.Intersect(ConnectingLinear1, &DumPoints));
    DumPoints.clear();

    // At end point ...
    ASSERT_EQ(0, Poly1.Intersect(ConnectingLinear1A, &DumPoints));
    DumPoints.clear();

    // Tests with linked segments
    ASSERT_EQ(0, Poly1.Intersect(LinkedLinear1, &DumPoints));
    DumPoints.clear();

    // Special cases
    ASSERT_EQ(1, Poly1.Intersect(ComplexLinearCase1, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(13.5, DumPoints[0].GetY());
    DumPoints.clear();

    ASSERT_EQ(1, Poly1.Intersect(ComplexLinearCase2, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(15.0, DumPoints[0].GetY());
    DumPoints.clear();

    ASSERT_EQ(1, Poly1.Intersect(ComplexLinearCase3, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());
    DumPoints.clear();

    ASSERT_EQ(0, Poly1.Intersect(ComplexLinearCase4, &DumPoints));
    DumPoints.clear();

    ASSERT_EQ(1, Poly1.Intersect(ComplexLinearCase5, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());
    DumPoints.clear();

    ASSERT_EQ(0, Poly1.Intersect(ComplexLinearCase5A, &DumPoints));
    DumPoints.clear();

    ASSERT_EQ(0, Poly1.Intersect(ComplexLinearCase6, &DumPoints));
    DumPoints.clear();

    ASSERT_EQ(0, Poly1.Intersect(ComplexLinearCase7, &DumPoints));
    DumPoints.clear();

    }

//==================================================================================
// Contiguousness Test
//   ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
//                              HGF2DLocationCollection* po_pContiguousnessPoints) const;
//   ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
//                                const HGF2DLocation& pi_rPoint,
//                                HGF2DLocation* pi_pFirstContiguousnessPoint,
//                                HVE2DLocation* pi_pSecondContiguousnessPoint) const;
//    HDLL virtual bool      AreContiguous(const HVE2DVector& pi_rVector) const;
//    HDLL virtual bool      AreContiguousAt(const HVE2DVector& pi_rVector,
//                                            const HGF2DLocation& pi_rPoint) const;
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ContiguousnessTest)
    {

    HGF2DLocationCollection     DumPoints;
    HGF2DLocation   FirstDumPoint;
    HGF2DLocation   SecondDumPoint;

    // Test with contiguous linears
    ASSERT_TRUE(Poly1.AreContiguous(ComplexLinearCase6));
    ASSERT_TRUE(Poly1.AreContiguousAt(ComplexLinearCase6, PolyMidPoint1));
    ASSERT_EQ(2, Poly1.ObtainContiguousnessPoints(ComplexLinearCase6, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetY());

    Poly1.ObtainContiguousnessPointsAt(ComplexLinearCase6, LinearMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(10.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetY());

    // Test with non contiguous linears
    ASSERT_FALSE(Poly1.AreContiguous(ComplexLinearCase1));

    DumPoints.clear();

    // Test with contiguous Polygon
    ASSERT_TRUE(Poly1.AreContiguous(NorthContiguousPoly));
    ASSERT_TRUE(Poly1.AreContiguousAt(NorthContiguousPoly, PolyMidPoint1));
    ASSERT_EQ(2, Poly1.ObtainContiguousnessPoints(NorthContiguousPoly, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[1].GetY());

    Poly1.ObtainContiguousnessPointsAt(NorthContiguousPoly, PolyMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(10.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(20.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, SecondDumPoint.GetY());

    DumPoints.clear();

    ASSERT_TRUE(Poly1.AreContiguous(VerticalFitPoly));
    ASSERT_TRUE(Poly1.AreContiguousAt(VerticalFitPoly, HGF2DLocation(17.0, 10.0, pWorld)));
    ASSERT_EQ(4, Poly1.ObtainContiguousnessPoints(VerticalFitPoly, &DumPoints));
    ASSERT_DOUBLE_EQ(15.0, DumPoints[3].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[3].GetY());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[2].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[2].GetY());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[1].GetY());
    ASSERT_DOUBLE_EQ(15.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetY());

    Poly1.ObtainContiguousnessPointsAt(VerticalFitPoly, HGF2DLocation(17.0, 10.0, pWorld), &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(20.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(15.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetY());

    DumPoints.clear();

    ASSERT_TRUE(Poly1.AreContiguous(IncludedPoly1));
    ASSERT_TRUE(Poly1.AreContiguousAt(IncludedPoly1, HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_EQ(2, Poly1.ObtainContiguousnessPoints(IncludedPoly1, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(15.0, DumPoints[1].GetY());
    ASSERT_DOUBLE_EQ(15.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());

    Poly1.ObtainContiguousnessPointsAt(IncludedPoly1, HGF2DLocation(10.0, 10.0, pWorld), &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(10.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(15.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(15.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetY());

    }

//==================================================================================
// Cloning tests
// Clone() const;
// AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  CloningTest)
    {

    //General Clone Test
    HFCPtr<HVE2DPolygonOfSegments> pClone = (HVE2DPolygonOfSegments*)Poly1.Clone();

    ASSERT_FALSE(pClone->IsEmpty());
    ASSERT_EQ(pWorld, pClone->GetCoordSys());
    ASSERT_EQ(HVE2DPolygonOfSegments::CLASS_ID, pClone->GetShapeType());

    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));  
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld))); 
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld))); 
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld))); 

    // Test with the same coordinate system
    HFCPtr<HVE2DPolygonOfSegments> pClone3 = (HVE2DPolygonOfSegments*)Poly1.AllocateCopyInCoordSys(pWorld);

    ASSERT_FALSE(pClone3->IsEmpty());
    ASSERT_EQ(pWorld, pClone3->GetCoordSys());
    ASSERT_EQ(HVE2DPolygonOfSegments::CLASS_ID, pClone3->GetShapeType());

    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));  
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld))); 
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld))); 
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));  

    // Test with a translation between systems
    Translation = HGF2DDisplacement (10.0, 10.0);
    HGF2DTranslation myTranslation(Translation);
    HFCPtr<HGF2DCoordSys> pWorldTranslation = new HGF2DCoordSys(myTranslation, pWorld);
   
    HFCPtr<HVE2DPolygonOfSegments> pClone5 = (HVE2DPolygonOfSegments*)Poly1.AllocateCopyInCoordSys(pWorldTranslation);

    ASSERT_FALSE(pClone5->IsEmpty());
    ASSERT_EQ(pWorldTranslation, pClone5->GetCoordSys());
    ASSERT_EQ(HVE2DPolygonOfSegments::CLASS_ID, pClone5->GetShapeType());

    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(0.0, 0.0, pWorldTranslation)));  
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(0.0, 10.0, pWorldTranslation))); 
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(10.0, 10.0, pWorldTranslation))); 
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(10.0, 0.0, pWorldTranslation))); 

    // Test with a stretch between systems
    HGF2DStretch myStretch;
    myStretch.SetTranslation(Translation);
    myStretch.SetXScaling(0.5);
    myStretch.SetYScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldStretch = new HGF2DCoordSys(myStretch, pWorld);
    
    HFCPtr<HVE2DPolygonOfSegments> pClone6 = (HVE2DPolygonOfSegments*)Poly1.AllocateCopyInCoordSys(pWorldStretch);

    ASSERT_FALSE(pClone6->IsEmpty());
    ASSERT_EQ(pWorldStretch, pClone6->GetCoordSys());
    ASSERT_EQ(HVE2DPolygonOfSegments::CLASS_ID, pClone6->GetShapeType());

    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(0.0, 0.0, pWorldStretch)));  
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(0.0, 20.0, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(20.0, 20.0, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(20.0, 0.0, pWorldStretch))); 

    // Test with a similitude between systems
    HGF2DSimilitude mySimilitude;
    mySimilitude.SetRotation(PI);
    mySimilitude.SetScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldSimilitude = new HGF2DCoordSys(mySimilitude, pWorld);
   
    HFCPtr<HVE2DPolygonOfSegments> pClone7 = (HVE2DPolygonOfSegments*)Poly1.AllocateCopyInCoordSys(pWorldSimilitude);

    ASSERT_FALSE(pClone7->IsEmpty());
    ASSERT_EQ(pWorldSimilitude, pClone7->GetCoordSys());
    ASSERT_EQ(HVE2DPolygonOfSegments::CLASS_ID, pClone7->GetShapeType());

    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-20.0, -20.0, pWorldSimilitude)));  
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-20.0, -40.0, pWorldSimilitude))); 
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-40.0, -40.0, pWorldSimilitude))); 
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-40.0, -20.0, pWorldSimilitude))); 

    // Test with a affine between systems
    HGF2DAffine myAffine;
    myAffine.SetTranslation(Translation);
    myAffine.SetRotation(PI);
    myAffine.SetXScaling(0.5);
    myAffine.SetYScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldAffine = new HGF2DCoordSys(myAffine, pWorld);
    
    HFCPtr<HVE2DPolygonOfSegments> pClone8 = (HVE2DPolygonOfSegments*)Poly1.AllocateCopyInCoordSys(pWorldAffine);

    ASSERT_FALSE(pClone8->IsEmpty());
    ASSERT_EQ(pWorldAffine, pClone8->GetCoordSys());
    ASSERT_EQ(HVE2DPolygonOfSegments::CLASS_ID, pClone8->GetShapeType());

    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(0.0, 0.0, pWorldAffine)));  
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(0.0, -20.0, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-20.0, -20.0, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-20.0, 0.0, pWorldAffine))); 
  
    }

//==================================================================================
// Interaction info with other segment
// Crosses(const HVE2DVector& pi_rVector) const;
// AreAdjacent(const HVE2DVector& pi_rVector) const;
// IsPointOn(const HGF2DLocation& pi_rTestPoint) const;
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  InteractionTest)
    {
     
    // Tests with a vertical segment
    ASSERT_TRUE(Poly1.Crosses(ComplexLinearCase1));
    ASSERT_FALSE(Poly1.AreAdjacent(ComplexLinearCase1));

    ASSERT_TRUE(Poly1.Crosses(ComplexLinearCase2));
    ASSERT_TRUE(Poly1.AreAdjacent(ComplexLinearCase2));

    ASSERT_TRUE(Poly1.Crosses(ComplexLinearCase3));
    ASSERT_FALSE(Poly1.AreAdjacent(ComplexLinearCase3));

    ASSERT_FALSE(Poly1.Crosses(ComplexLinearCase4));
    ASSERT_TRUE(Poly1.AreAdjacent(ComplexLinearCase4));

    ASSERT_TRUE(Poly1.Crosses(ComplexLinearCase5));
    ASSERT_TRUE(Poly1.AreAdjacent(ComplexLinearCase5));

    ASSERT_FALSE(Poly1.Crosses(ComplexLinearCase6));
    ASSERT_TRUE(Poly1.AreAdjacent(ComplexLinearCase6));

    ASSERT_FALSE(Poly1.Crosses(ComplexLinearCase7));
    ASSERT_FALSE(Poly1.AreAdjacent(ComplexLinearCase7));

    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(10.0 - 1.1*MYEPSILON, 10.0-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Poly1.IsPointOn(PolyMidPoint1-HGF2DDisplacement(0.0, 1.1*MYEPSILON)));
    ASSERT_FALSE(Poly1.IsPointOn(PolyMidPoint1-HGF2DDisplacement(0.0, -1.1*MYEPSILON)));
    ASSERT_TRUE(Poly1.IsPointOn(PolyMidPoint1-HGF2DDisplacement(0.0, 0.9*MYEPSILON)));
    ASSERT_TRUE(Poly1.IsPointOn(PolyMidPoint1-HGF2DDisplacement(0.0, -0.9*MYEPSILON)));
    ASSERT_TRUE(Poly1.IsPointOn(PolyMidPoint1));

    ASSERT_TRUE(Poly1.IsPointOn(Poly1.GetLinear().GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOn(Poly1.GetLinear().GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    
    }

//==================================================================================
// Bearing calculation tests
// CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
//                  HVE2DVector::ArbitraryDiPolyion pi_DiPolyion = HVE2DVector::BETA) const;
// CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
//                              HVE2DVector::ArbitraryDiPolyion pi_DiPolyion = HVE2DVector::BETA) const;
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  BearingTest)
    {

    // Obtain bearing ALPHA of a vertical segment
    ASSERT_NEAR(0.0, Poly1.CalculateBearing(Poly1Point0d0, HVE2DVector::ALPHA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.5707963267948966, Poly1.CalculateBearing(Poly1Point0d0, HVE2DVector::BETA).GetAngle());
    ASSERT_NEAR(0.0, Poly1.CalculateBearing(Poly1Point0d1, HVE2DVector::ALPHA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(3.1415926535897931, Poly1.CalculateBearing(Poly1Point0d1, HVE2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(3.1415926535897931, Poly1.CalculateBearing(Poly1Point0d5, HVE2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(4.7123889803846897, Poly1.CalculateBearing(Poly1Point0d5, HVE2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(4.7123889803846897, Poly1.CalculateBearing(Poly1Point1d0, HVE2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(1.5707963267948966, Poly1.CalculateBearing(Poly1Point1d0, HVE2DVector::BETA).GetAngle());

    // ANGULAR ACCELERATION
    // Obtain angular acceleration ALPHA of a vertical segment
    ASSERT_NEAR(0.0, Poly1.CalculateAngularAcceleration(Poly1Point0d0, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, Poly1.CalculateAngularAcceleration(Poly1Point0d0, HVE2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, Poly1.CalculateAngularAcceleration(Poly1Point0d1, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, Poly1.CalculateAngularAcceleration(Poly1Point0d1, HVE2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, Poly1.CalculateAngularAcceleration(Poly1Point0d5, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, Poly1.CalculateAngularAcceleration(Poly1Point0d5, HVE2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, Poly1.CalculateAngularAcceleration(Poly1Point1d0, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, Poly1.CalculateAngularAcceleration(Poly1Point1d0, HVE2DVector::BETA), MYEPSILON);
   
    }
        
//==================================================================================
// Extent calculation test
// GetExtent() const;
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  GetExtentTest)
    {

    // Obtain extent of linear 1
    ASSERT_DOUBLE_EQ(10.0, Poly1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(10.0, Poly1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(20.0, Poly1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(20.0, Poly1.GetExtent().GetYMax());

    }

//==================================================================================
// IsEmpty() test
// MakEmpty() test
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  EmptyTest)
    {

    HVE2DPolygonOfSegments  MyOtherPoly(Poly1);
    ASSERT_FALSE(MyOtherPoly.IsEmpty());

    MyOtherPoly.MakeEmpty();
    ASSERT_TRUE(MyOtherPoly.IsEmpty());
  
    }

//==================================================================================
// GetShapeType() 
//==================================================================================
  TEST_F(HVE2DPolygonOfSegmentsTester,  ClassIDTest)
    {     
    
    HVE2DPolygonOfSegments  MyOtherPoly(Poly1);        
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), Poly1.GetShapeType());
    
    }

//==================================================================================
// IsPointIn()
// IsPointOn()
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  IsPointTest)
    {

    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(15.0, 10.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(20.0, 15.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(15.0, 20.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(10.0, 15.0, pWorld)));

    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(100.0, 100.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(10.0-1.1*MYEPSILON, 10.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(10.0, 10.0-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(10.0-1.1*MYEPSILON, 15.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(10.0-1.1*MYEPSILON, 20.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(10.0, 20.0+1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(15.0, 20.0+1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(20.0, 20.0+1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(20.0+1.1*MYEPSILON, 20.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(20.0+1.1*MYEPSILON, 15.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(20.0+1.1*MYEPSILON, 10.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(20.0, 10.0-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(15.0, 10.0-1.1*MYEPSILON, pWorld)));

    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(10.0-0.9*MYEPSILON, 10.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(10.0, 10.0-0.9*MYEPSILON, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(10.0-0.9*MYEPSILON, 15.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(10.0-0.9*MYEPSILON, 20.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(10.0, 20.0+0.9*MYEPSILON, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(15.0, 20.0+0.9*MYEPSILON, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(20.0, 20.0+0.9*MYEPSILON, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(20.0+0.9*MYEPSILON, 20.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(20.0+0.9*MYEPSILON, 15.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(20.0+0.9*MYEPSILON, 10.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(20.0, 10.0-0.9*MYEPSILON, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(15.0, 10.0-0.9*MYEPSILON, pWorld)));

    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(10.0+0.9*MYEPSILON, 15.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(15.0, 20.0-0.9*MYEPSILON, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(20.0-0.9*MYEPSILON, 15.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(15.0, 10.0+0.9*MYEPSILON, pWorld)));

    ASSERT_TRUE(Poly1.IsPointIn(HGF2DLocation(10.0+1.1*MYEPSILON, 15.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointIn(HGF2DLocation(15.0, 20.0-1.1*MYEPSILON, pWorld)));
    ASSERT_TRUE(Poly1.IsPointIn(HGF2DLocation(20.0-1.1*MYEPSILON, 15.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointIn(HGF2DLocation(15.0, 10.0+1.1*MYEPSILON, pWorld)));

    ASSERT_TRUE(Poly1.IsPointIn(HGF2DLocation(12.0, 15.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointIn(HGF2DLocation(15.0, 18.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointIn(HGF2DLocation(18.0, 15.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointIn(HGF2DLocation(15.0, 12.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointIn(HGF2DLocation(15.0, 15.0, pWorld)));

    ASSERT_FALSE(Poly1.IsPointIn(HGF2DLocation(-15.0, -15.0, pWorld)));

    // IsPointOn
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(15.0, 10.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(20.0, 15.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(15.0, 20.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(10.0, 15.0, pWorld)));

    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(100.0, 100.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(10.0-1.1*MYEPSILON, 10.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(10.0, 10.0-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(10.0-1.1*MYEPSILON, 15.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(10.0-1.1*MYEPSILON, 20.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(10.0, 20.0+1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(15.0, 20.0+1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(20.0, 20.0+1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(20.0+1.1*MYEPSILON, 20.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(20.0+1.1*MYEPSILON, 15.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(20.0+1.1*MYEPSILON, 10.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(20.0, 10.0-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(15.0, 10.0-1.1*MYEPSILON, pWorld)));

    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(10.0-0.9*MYEPSILON, 10.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(10.0, 10.0-0.9*MYEPSILON, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(10.0-0.9*MYEPSILON, 15.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(10.0-0.9*MYEPSILON, 20.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(10.0, 20.0+0.9*MYEPSILON, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(15.0, 20.0+0.9*MYEPSILON, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(20.0, 20.0+0.9*MYEPSILON, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(20.0+0.9*MYEPSILON, 20.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(20.0+0.9*MYEPSILON, 15.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(20.0+0.9*MYEPSILON, 10.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(20.0, 10.0-0.9*MYEPSILON, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(15.0, 10.0-0.9*MYEPSILON, pWorld)));

    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(10.0+0.9*MYEPSILON, 15.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(15.0, 20.0-0.9*MYEPSILON, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(20.0-0.9*MYEPSILON, 15.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(15.0, 10.0+0.9*MYEPSILON, pWorld)));

    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(10.0+1.1*MYEPSILON, 15.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(15.0, 20.0-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(20.0-1.1*MYEPSILON, 15.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(15.0, 10.0+1.1*MYEPSILON, pWorld)));

    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(12.0, 15.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(15.0, 18.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(18.0, 15.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(15.0, 12.0, pWorld)));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(15.0, 15.0, pWorld)));

    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(-15.0, -15.0, pWorld)));

    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(15.0, 10.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(20.0, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(15.0, 20.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(10.0, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(100.0, 100.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(10.0-1.1*MYEPSILON, 10.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(10.0, 10.0-1.1*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(10.0-1.1*MYEPSILON, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(10.0-1.1*MYEPSILON, 20.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(10.0, 20.0+1.1*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(15.0, 20.0+1.1*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(20.0, 20.0+1.1*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(20.0+1.1*MYEPSILON, 20.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(20.0+1.1*MYEPSILON, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(20.0+1.1*MYEPSILON, 10.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(20.0, 10.0-1.1*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(15.0, 10.0-1.1*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(10.0-0.9*MYEPSILON, 10.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(10.0, 10.0-0.9*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(10.0-0.9*MYEPSILON, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(10.0-0.9*MYEPSILON, 20.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(10.0, 20.0+0.9*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(15.0, 20.0+0.9*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(20.0, 20.0+0.9*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(20.0+0.9*MYEPSILON, 20.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(20.0+0.9*MYEPSILON, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(20.0+0.9*MYEPSILON, 10.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(20.0, 10.0-0.9*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(15.0, 10.0-0.9*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(10.0+0.9*MYEPSILON, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(15.0, 20.0-0.9*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(20.0-0.9*MYEPSILON, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(15.0, 10.0+0.9*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(10.0+1.1*MYEPSILON, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(15.0, 20.0-1.1*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(20.0-1.1*MYEPSILON, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(15.0, 10.0+1.1*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(12.0, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(15.0, 18.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(18.0, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(15.0, 12.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(15.0, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_FALSE(Poly1.IsPointOn(HGF2DLocation(-15.0, -15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));

    }

//==================================================================================
// UnifyShapeSCS(const HVE2DShape& pi_rShape) const;
// UnifyShape(const HVE2DShape& pi_rShape) const;
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  UnifyShapeTest)
    {
        
    HVE2DComplexLinear  AComp2;

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape1 = (HVE2DPolygonOfSegments *)Poly1.UnifyShape(NorthContiguousPoly);

    ASSERT_DOUBLE_EQ(200.0, pResultShape1->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape1->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape1->GetCoordSys());

    AComp2 = pResultShape1->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape2 = (HVE2DPolygonOfSegments *)Poly1.UnifyShape(EastContiguousPoly);

    ASSERT_DOUBLE_EQ(200.0, pResultShape2->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape2->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape2->GetCoordSys());

    AComp2 = pResultShape2->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape3 = (HVE2DPolygonOfSegments *)Poly1.UnifyShape(WestContiguousPoly);

    ASSERT_DOUBLE_EQ(200.0, pResultShape2->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape3->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape3->GetCoordSys());

    AComp2 = pResultShape3->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_NEAR(0.0, AComp2.GetLinear(0).GetStartPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_NEAR(0.0, AComp2.GetLinear(0).GetEndPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_NEAR(0.0, AComp2.GetLinear(1).GetStartPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_NEAR(0.0, AComp2.GetLinear(3).GetEndPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape4 = (HVE2DPolygonOfSegments *) Poly1.UnifyShape(SouthContiguousPoly);

    ASSERT_DOUBLE_EQ(200.0, pResultShape4->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape4->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape4->GetCoordSys());

    AComp2 = pResultShape4->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_NEAR(0.0, AComp2.GetLinear(0).GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_NEAR(0.0, AComp2.GetLinear(2).GetEndPoint().GetY(), MYEPSILON);

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_NEAR(0.0, AComp2.GetLinear(3).GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_NEAR(0.0, AComp2.GetLinear(3).GetEndPoint().GetY(), MYEPSILON);

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape5 = (HVE2DPolygonOfSegments *) Poly1.UnifyShape(VerticalFitPoly);

    ASSERT_DOUBLE_EQ(150.0, pResultShape5->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape5->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape5->GetCoordSys());

    AComp2 = pResultShape5->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape6 = (HVE2DPolygonOfSegments *) Poly1.UnifyShape(HorizontalFitPoly);

    ASSERT_DOUBLE_EQ(150.0, pResultShape6->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape6->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape6->GetCoordSys());

    AComp2 = pResultShape6->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DShape>     pResultShape7 = Poly1.UnifyShape(DisjointPoly);

    ASSERT_NE(pResultShape7->GetShapeType(), static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID));
    ASSERT_TRUE(pResultShape7->IsComplex());
    ASSERT_FALSE(pResultShape7->HasHoles());
    ASSERT_DOUBLE_EQ(200.0, pResultShape7->CalculateArea());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape8 = (HVE2DPolygonOfSegments *) Poly1.UnifyShape(MiscPoly1);

    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape8->GetShapeType());
    ASSERT_DOUBLE_EQ(175.0, pResultShape8->CalculateArea());

    AComp2 = pResultShape8->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(8, AComp2.GetNumberOfLinears()); 

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(4).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(4).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(4).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(5.0, AComp2.GetLinear(4).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(5).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.0, AComp2.GetLinear(5).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.0, AComp2.GetLinear(5).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(5.0, AComp2.GetLinear(5).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(5.0, AComp2.GetLinear(6).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.0, AComp2.GetLinear(6).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.0, AComp2.GetLinear(6).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(6).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(5.0, AComp2.GetLinear(7).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(7).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(7).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(7).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape9 = (HVE2DPolygonOfSegments *) Poly1.UnifyShape(EnglobPoly1); 

    ASSERT_DOUBLE_EQ(400.0, pResultShape9->CalculateArea());
    #ifdef WIP_IPPTEST_BUG_1
    //Amelioration : Being able to realize that the simple shape is a polygon in this case
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape9->GetShapeType());
    #endif
    ASSERT_EQ(pWorld, pResultShape9->GetCoordSys());

    AComp2 = pResultShape9->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape10 = (HVE2DPolygonOfSegments *) Poly1.UnifyShape(EnglobPoly2);

    ASSERT_DOUBLE_EQ(900.0, pResultShape10->CalculateArea());              
    #ifdef WIP_IPPTEST_BUG_1
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape10->GetShapeType());
    #endif
    ASSERT_EQ(pWorld, pResultShape10->GetCoordSys());

    AComp2 = pResultShape10->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_NEAR(0.0, AComp2.GetLinear(0).GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp2.GetLinear(0).GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp2.GetLinear(0).GetEndPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_NEAR(0.0, AComp2.GetLinear(1).GetStartPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_NEAR(0.0, AComp2.GetLinear(2).GetEndPoint().GetY(), MYEPSILON);

    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_NEAR(0.0, AComp2.GetLinear(3).GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp2.GetLinear(3).GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp2.GetLinear(3).GetEndPoint().GetY(), MYEPSILON);

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape11 = (HVE2DPolygonOfSegments *) Poly1.UnifyShape(EnglobPoly3);

    ASSERT_DOUBLE_EQ(200.0, pResultShape11->CalculateArea());
    #ifdef WIP_IPPTEST_BUG_1
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape11->GetShapeType());
    #endif
    ASSERT_EQ(pWorld, pResultShape11->GetCoordSys());

    AComp2 = pResultShape11->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape12 = (HVE2DPolygonOfSegments *) Poly1.UnifyShape(IncludedPoly1);

    ASSERT_DOUBLE_EQ(100.0, pResultShape12->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape12->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape12->GetCoordSys());

    AComp2 = pResultShape12->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape13 = (HVE2DPolygonOfSegments *) Poly1.UnifyShape(IncludedPoly2);

    ASSERT_DOUBLE_EQ(100.0, pResultShape13->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape13->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape13->GetCoordSys());

    AComp2 = pResultShape13->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape14 = (HVE2DPolygonOfSegments *) Poly1.UnifyShape(IncludedPoly3);

    ASSERT_DOUBLE_EQ(100.0, pResultShape14->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape14->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape14->GetCoordSys());

    AComp2 = pResultShape14->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape15 = (HVE2DPolygonOfSegments *) Poly1.UnifyShape(IncludedPoly4);

    ASSERT_DOUBLE_EQ(100.0, pResultShape15->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape15->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape15->GetCoordSys());

    AComp2 = pResultShape15->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape16 = (HVE2DPolygonOfSegments *)Poly1.UnifyShape(IncludedPoly5);

    ASSERT_DOUBLE_EQ(100.0, pResultShape16->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape16->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape16->GetCoordSys());

    AComp2 = pResultShape16->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY()); 
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape17 = (HVE2DPolygonOfSegments *) Poly1.UnifyShape(IncludedPoly6);

    ASSERT_DOUBLE_EQ(100.0, pResultShape17->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape17->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape17->GetCoordSys());

    AComp2 = pResultShape17->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape18 = (HVE2DPolygonOfSegments *) Poly1.UnifyShape(IncludedPoly7);

    ASSERT_DOUBLE_EQ(100.0, pResultShape18->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape18->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape18->GetCoordSys());

    AComp2 = pResultShape18->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape19 = (HVE2DPolygonOfSegments *) Poly1.UnifyShape(IncludedPoly8);

    ASSERT_DOUBLE_EQ(100.0, pResultShape19->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape19->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape19->GetCoordSys());

    AComp2 = pResultShape19->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape20 = (HVE2DPolygonOfSegments *) Poly1.UnifyShape(IncludedPoly9);

    ASSERT_DOUBLE_EQ(100.0, pResultShape20->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape20->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape20->GetCoordSys());

    AComp2 = pResultShape20->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape21 = (HVE2DPolygonOfSegments *) Poly1.UnifyShapeSCS(IncludedPoly9);

    ASSERT_DOUBLE_EQ(100.0, pResultShape21->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape21->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape21->GetCoordSys());

    AComp2 = pResultShape21->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());
    
    }

//==================================================================================
// IntersectShapeSCS(const HVE2DShape& pi_rShape) const;
// IntersectShape(const HVE2DShape& pi_rShape) const;
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  IntersectShapeTest)
    {

    HVE2DComplexLinear  AComp2;

    HFCPtr<HVE2DShape>     pResultShape1 = Poly1.IntersectShape(NorthContiguousPoly);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape1->GetShapeType());
    ASSERT_TRUE(pResultShape1->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape2 = Poly1.IntersectShape(EastContiguousPoly);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape2->GetShapeType());
    ASSERT_TRUE(pResultShape2->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape3 = Poly1.IntersectShape(WestContiguousPoly);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape3->GetShapeType());
    ASSERT_TRUE(pResultShape3->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape4 = Poly1.IntersectShape(SouthContiguousPoly);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape4->GetShapeType());
    ASSERT_TRUE(pResultShape4->IsEmpty());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape5 = (HVE2DPolygonOfSegments*)Poly1.IntersectShape(VerticalFitPoly);

    ASSERT_DOUBLE_EQ(50.0, pResultShape5->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape5->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape5->GetCoordSys());

    AComp2 = pResultShape5->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape6 = (HVE2DPolygonOfSegments*)Poly1.IntersectShape(HorizontalFitPoly);

    ASSERT_DOUBLE_EQ(50.0, pResultShape6->CalculateArea());

    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape6->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape6->GetCoordSys());

    AComp2 = pResultShape6->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DShape>     pResultShape7 = Poly1.IntersectShape(DisjointPoly);
    ASSERT_EQ(pResultShape7->GetShapeType(), HVE2DVoidShape::CLASS_ID);
    ASSERT_TRUE(pResultShape7->IsEmpty());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape8 = (HVE2DPolygonOfSegments*)Poly1.IntersectShape(MiscPoly1);

    ASSERT_DOUBLE_EQ(25.0, pResultShape8->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape8->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape8->GetCoordSys());

    AComp2 = pResultShape8->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape9 = (HVE2DPolygonOfSegments*)Poly1.IntersectShape(EnglobPoly1);

    ASSERT_DOUBLE_EQ(100.0, pResultShape9->CalculateArea());
    #ifdef WIP_IPPTEST_BUG_1
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape9->GetShapeType());
    #endif
    ASSERT_EQ(pWorld, pResultShape9->GetCoordSys());

    AComp2 = pResultShape9->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape10 = (HVE2DPolygonOfSegments*)Poly1.IntersectShape(EnglobPoly2);

    ASSERT_DOUBLE_EQ(100.0, pResultShape10->CalculateArea());
    #ifdef WIP_IPPTEST_BUG_1
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape10->GetShapeType());
    #endif
    ASSERT_EQ(pWorld, pResultShape10->GetCoordSys());

    AComp2 = pResultShape10->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape11 = (HVE2DPolygonOfSegments*)Poly1.IntersectShape(EnglobPoly3);

    ASSERT_DOUBLE_EQ(100.0, pResultShape11->CalculateArea());
    #ifdef WIP_IPPTEST_BUG_1
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape11->GetShapeType());
    #endif
    ASSERT_EQ(pWorld, pResultShape11->GetCoordSys());

    AComp2 = pResultShape11->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape12 = (HVE2DPolygonOfSegments*)Poly1.IntersectShape(IncludedPoly1);

    ASSERT_DOUBLE_EQ(25.0, pResultShape12->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape12->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape12->GetCoordSys());

    AComp2 = pResultShape12->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape13 = (HVE2DPolygonOfSegments*) Poly1.IntersectShape(IncludedPoly2);

    ASSERT_DOUBLE_EQ(25.0, pResultShape13->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape13->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape13->GetCoordSys());

    AComp2 = pResultShape13->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape14 = (HVE2DPolygonOfSegments*)Poly1.IntersectShape(IncludedPoly3);

    ASSERT_DOUBLE_EQ(25.0, pResultShape14->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape14->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape14->GetCoordSys());

    AComp2 = pResultShape14->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape15 = (HVE2DPolygonOfSegments*) Poly1.IntersectShape(IncludedPoly4);

    ASSERT_DOUBLE_EQ(25.0, pResultShape15->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape15->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape15->GetCoordSys());

    AComp2 = pResultShape15->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape16 = (HVE2DPolygonOfSegments*)Poly1.IntersectShape(IncludedPoly5);

    ASSERT_DOUBLE_EQ(36.0, pResultShape16->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape16->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape16->GetCoordSys());

    AComp2 = pResultShape16->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(12.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(12.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(12.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(18.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(12.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(18.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(18.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(18.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(18.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(18.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(18.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(12.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(18.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(12.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(12.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(12.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape17 = (HVE2DPolygonOfSegments*) Poly1.IntersectShape(IncludedPoly6);

    ASSERT_DOUBLE_EQ(50.0, pResultShape17->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape17->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape17->GetCoordSys());

    AComp2 = pResultShape17->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape18 = (HVE2DPolygonOfSegments*)Poly1.IntersectShape(IncludedPoly7);

    ASSERT_DOUBLE_EQ(50.0, pResultShape18->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape18->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape18->GetCoordSys());

    AComp2 = pResultShape18->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape19 = (HVE2DPolygonOfSegments*)Poly1.IntersectShape(IncludedPoly8);

    ASSERT_DOUBLE_EQ(50.0, pResultShape19->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape19->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape19->GetCoordSys());

    AComp2 = pResultShape19->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape20 = (HVE2DPolygonOfSegments*) Poly1.IntersectShape(IncludedPoly9);

    ASSERT_DOUBLE_EQ(50.0, pResultShape20->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape20->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape20->GetCoordSys());

    AComp2 = pResultShape20->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape21 = (HVE2DPolygonOfSegments*)Poly1.IntersectShapeSCS(IncludedPoly9);

    ASSERT_DOUBLE_EQ(50.0, pResultShape21->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape21->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape21->GetCoordSys());

    AComp2 = pResultShape21->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    }

//==================================================================================
// DifferentiateShapeTest
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  DifferentiateShapeTest)
    {

    HVE2DComplexLinear  AComp2;

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape1 = (HVE2DPolygonOfSegments*) Poly1.DifferentiateShape(NorthContiguousPoly);
    
    ASSERT_DOUBLE_EQ(100.0, pResultShape1->CalculateArea());    
    #ifdef WIP_IPPTEST_BUG_1
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape1->GetShapeType());
    #endif
    ASSERT_EQ(pWorld, pResultShape1->GetCoordSys());

    AComp2 = pResultShape1->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape2 = (HVE2DPolygonOfSegments*) Poly1.DifferentiateShape(EastContiguousPoly);

    ASSERT_DOUBLE_EQ(100.0, pResultShape2->CalculateArea());
    #ifdef WIP_IPPTEST_BUG_1
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape2->GetShapeType());
    #endif
    ASSERT_EQ(pWorld, pResultShape2->GetCoordSys());

    AComp2 = pResultShape2->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape3 = (HVE2DPolygonOfSegments*) Poly1.DifferentiateShape(WestContiguousPoly);

    ASSERT_DOUBLE_EQ(100.0, pResultShape3->CalculateArea());
    #ifdef WIP_IPPTEST_BUG_1
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape3->GetShapeType());
    #endif
    ASSERT_EQ(pWorld, pResultShape3->GetCoordSys());
    AComp2 = pResultShape3->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape4 = (HVE2DPolygonOfSegments*) Poly1.DifferentiateShape(SouthContiguousPoly);

    ASSERT_DOUBLE_EQ(100.0, pResultShape4->CalculateArea());
    #ifdef WIP_IPPTEST_BUG_1
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape4->GetShapeType());
    #endif
    ASSERT_EQ(pWorld, pResultShape4->GetCoordSys());

    AComp2 = pResultShape4->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape5 = (HVE2DPolygonOfSegments*) Poly1.DifferentiateShape(VerticalFitPoly);

    ASSERT_DOUBLE_EQ(50.0, pResultShape5->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape5->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape5->GetCoordSys());

    AComp2 = pResultShape5->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape6 = (HVE2DPolygonOfSegments*) Poly1.DifferentiateShape(HorizontalFitPoly);

    ASSERT_DOUBLE_EQ(50.0, pResultShape6->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape6->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape6->GetCoordSys());

    AComp2 = pResultShape6->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape7 = (HVE2DPolygonOfSegments*) Poly1.DifferentiateShape(DisjointPoly);

    ASSERT_DOUBLE_EQ(100.0, pResultShape7->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape7->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape7->GetCoordSys());

    AComp2 = pResultShape7->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape8 = (HVE2DPolygonOfSegments*) Poly1.DifferentiateShape(MiscPoly1);

    ASSERT_DOUBLE_EQ(75.0, pResultShape8->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape8->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape8->GetCoordSys());

    AComp2 = pResultShape8->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(6, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(4).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(4).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(4).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(4).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(5).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(5).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(5).GetEndPoint().GetY());

    HFCPtr<HVE2DShape>     pResultShape9 = Poly1.DifferentiateShape(EnglobPoly1);

    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape9->GetShapeType());
    ASSERT_TRUE(pResultShape9->IsEmpty());

    ASSERT_NEAR(0.0, pResultShape9->CalculateArea(), MYEPSILON);  // must be 0

    HFCPtr<HVE2DShape>     pResultShape10 = Poly1.DifferentiateShape(EnglobPoly2);

    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape10->GetShapeType());
    ASSERT_TRUE(pResultShape10->IsEmpty());

    ASSERT_NEAR(0.0, pResultShape10->CalculateArea(), MYEPSILON); // must be 0

    HFCPtr<HVE2DShape>     pResultShape11 = Poly1.DifferentiateShape(EnglobPoly3);

    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape11->GetShapeType());
    ASSERT_TRUE(pResultShape11->IsEmpty());

    ASSERT_NEAR(0.0, pResultShape11->CalculateArea(), MYEPSILON); // must be 0

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape12 = (HVE2DPolygonOfSegments*) Poly1.DifferentiateShape(IncludedPoly1);

    ASSERT_DOUBLE_EQ(75.0, pResultShape12->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape12->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape12->GetCoordSys());

    AComp2 = pResultShape12->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(6, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(4).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(4).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(4).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(4).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(5).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(5).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(5).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape13 = (HVE2DPolygonOfSegments*) Poly1.DifferentiateShape(IncludedPoly2);

    ASSERT_DOUBLE_EQ(75.0, pResultShape13->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape13->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape13->GetCoordSys());

    AComp2 = pResultShape13->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(6, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(4).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(4).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(4).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(4).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(5).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetEndPoint().GetY());  

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape14 = (HVE2DPolygonOfSegments*) Poly1.DifferentiateShape(IncludedPoly3);

    ASSERT_DOUBLE_EQ(75.0, pResultShape14->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape14->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape14->GetCoordSys());

    AComp2 = pResultShape14->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(6, AComp2.GetNumberOfLinears());
    
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(4).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(4).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(4).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(4).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(5).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetEndPoint().GetY());  

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape15 = (HVE2DPolygonOfSegments*) Poly1.DifferentiateShape(IncludedPoly4);

    ASSERT_DOUBLE_EQ(75.0, pResultShape15->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape15->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape15->GetCoordSys());

    AComp2 = pResultShape15->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(6, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(4).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(4).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(4).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(4).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(5).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetEndPoint().GetY());  

    HFCPtr<HVE2DShape>     pResultShape16 = Poly1.DifferentiateShape(IncludedPoly5);

    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape16->GetShapeType());
    ASSERT_TRUE(pResultShape16->HasHoles());
    ASSERT_DOUBLE_EQ(64.0, pResultShape16->CalculateArea());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape17 = (HVE2DPolygonOfSegments*) Poly1.DifferentiateShape(IncludedPoly6);

    ASSERT_DOUBLE_EQ(50.0, pResultShape17->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape17->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape17->GetCoordSys());

    AComp2 = pResultShape17->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape18 = (HVE2DPolygonOfSegments*) Poly1.DifferentiateShape(IncludedPoly7);

    ASSERT_DOUBLE_EQ(50.0, pResultShape18->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape18->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape18->GetCoordSys());

    AComp2 = pResultShape18->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape19 = (HVE2DPolygonOfSegments*) Poly1.DifferentiateShape(IncludedPoly8);

    ASSERT_DOUBLE_EQ(50.0, pResultShape19->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape19->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape19->GetCoordSys());

    AComp2 = pResultShape19->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape20 = (HVE2DPolygonOfSegments*)Poly1.DifferentiateShape(IncludedPoly9);

    ASSERT_DOUBLE_EQ(50.0, pResultShape20->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape20->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape20->GetCoordSys());

    AComp2 = pResultShape20->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    }

//==================================================================================
// DifferentiateShapeSCS(const HVE2DShape& pi_rShape) const;
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  DifferentiateShapeSCSTest)
    {
        
    HVE2DComplexLinear  AComp2;

    HFCPtr<HVE2DShape>     pResultShape1 = NorthContiguousPoly.DifferentiateShapeSCS(Poly1);

    ASSERT_EQ(pWorld, pResultShape1->GetCoordSys());
    ASSERT_DOUBLE_EQ(100.0, pResultShape1->CalculateArea());

    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(10.0, 30.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(20.0, 30.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape2 = EastContiguousPoly.DifferentiateShapeSCS(Poly1);

    ASSERT_EQ(pWorld, pResultShape2->GetCoordSys());
    ASSERT_DOUBLE_EQ(100.0, pResultShape2->CalculateArea());

    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(30.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(30.0, 10.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape3 = WestContiguousPoly.DifferentiateShapeSCS(Poly1);

    ASSERT_EQ(pWorld, pResultShape3->GetCoordSys());
    ASSERT_DOUBLE_EQ(100.0, pResultShape3->CalculateArea());

    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(0.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(0.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape4 = SouthContiguousPoly.DifferentiateShapeSCS(Poly1);

    ASSERT_EQ(pWorld, pResultShape4->GetCoordSys());
    ASSERT_DOUBLE_EQ(100.0, pResultShape4->CalculateArea());

    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(10.0, 0.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(20.0, 0.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape5 = VerticalFitPoly.DifferentiateShapeSCS(Poly1);

    ASSERT_DOUBLE_EQ(50.0, pResultShape5->CalculateArea());
    ASSERT_EQ(pWorld, pResultShape5->GetCoordSys());

    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(25.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(25.0, 10.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape6 = HorizontalFitPoly.DifferentiateShapeSCS(Poly1);

    ASSERT_DOUBLE_EQ(50.0, pResultShape6->CalculateArea());
    ASSERT_EQ(pWorld, pResultShape6->GetCoordSys());

    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(10.0, 25.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(20.0, 25.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape7 = DisjointPoly.DifferentiateShapeSCS(Poly1);

    ASSERT_EQ(pWorld, pResultShape7->GetCoordSys());
    ASSERT_DOUBLE_EQ(100.0, pResultShape7->CalculateArea());

    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DLocation(-10.0, -10.0, pWorld)));
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DLocation(-10.0, 0.0, pWorld)));
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DLocation(0.0, -10.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape8 = MiscPoly1.DifferentiateShapeSCS(Poly1);

    ASSERT_EQ(pWorld, pResultShape8->GetCoordSys());
    ASSERT_DOUBLE_EQ(75.0, pResultShape8->CalculateArea());

    ASSERT_TRUE(pResultShape8->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape8->IsPointOn(HGF2DLocation(15.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape8->IsPointOn(HGF2DLocation(15.0, 5.0, pWorld)));
    ASSERT_TRUE(pResultShape8->IsPointOn(HGF2DLocation(5.0, 5.0, pWorld)));
    ASSERT_TRUE(pResultShape8->IsPointOn(HGF2DLocation(5.0, 15.0, pWorld)));
    ASSERT_TRUE(pResultShape8->IsPointOn(HGF2DLocation(10.0, 15.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape9 = EnglobPoly1.DifferentiateShapeSCS(Poly1);
   
    ASSERT_EQ(pWorld, pResultShape9->GetCoordSys());
    ASSERT_DOUBLE_EQ(300.0, pResultShape9->CalculateArea());

    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DLocation(10.0, 30.0, pWorld)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DLocation(30.0, 30.0, pWorld)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DLocation(30.0, 10.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape10 = EnglobPoly2.DifferentiateShapeSCS(Poly1);

    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape10->GetShapeType());
    ASSERT_TRUE(pResultShape10->HasHoles());
    ASSERT_DOUBLE_EQ(800.0, pResultShape10->CalculateArea());

    HFCPtr<HVE2DShape>     pResultShape11 = EnglobPoly3.DifferentiateShapeSCS(Poly1);

    ASSERT_DOUBLE_EQ(100.0, pResultShape11->CalculateArea());
    ASSERT_EQ(pWorld, pResultShape11->GetCoordSys());

    ASSERT_TRUE(pResultShape11->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape11->IsPointOn(HGF2DLocation(10.0, 30.0, pWorld)));
    ASSERT_TRUE(pResultShape11->IsPointOn(HGF2DLocation(20.0, 30.0, pWorld)));
    ASSERT_TRUE(pResultShape11->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape12 = IncludedPoly1.DifferentiateShapeSCS(Poly1);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape12->GetShapeType());
    ASSERT_TRUE(pResultShape12->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape13 = IncludedPoly2.DifferentiateShapeSCS(Poly1);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape13->GetShapeType());
    ASSERT_TRUE(pResultShape13->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape14 = IncludedPoly3.DifferentiateShapeSCS(Poly1);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape14->GetShapeType());
    ASSERT_TRUE(pResultShape14->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape15 = IncludedPoly4.DifferentiateShapeSCS(Poly1);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape15->GetShapeType());
    ASSERT_TRUE(pResultShape15->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape16 = IncludedPoly5.DifferentiateShapeSCS(Poly1);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape16->GetShapeType());
    ASSERT_TRUE(pResultShape16->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape17 = IncludedPoly6.DifferentiateShapeSCS(Poly1);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape17->GetShapeType());
    ASSERT_TRUE(pResultShape17->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape18 = IncludedPoly7.DifferentiateShapeSCS(Poly1);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape18->GetShapeType());
    ASSERT_TRUE(pResultShape18->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape19 = IncludedPoly8.DifferentiateShapeSCS(Poly1);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape19->GetShapeType());
    ASSERT_TRUE(pResultShape19->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape20 = IncludedPoly9.DifferentiateShapeSCS(Poly1);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape20->GetShapeType());
    ASSERT_TRUE(pResultShape20->IsEmpty());
    
    }

//==================================================================================
// DifferentiateFromShapeSCSTest
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  DifferentiateFromShapeSCSTest)
    {
        
    HVE2DComplexLinear  AComp2;

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape1 = (HVE2DPolygonOfSegments*)Poly1.DifferentiateFromShapeSCS(NorthContiguousPoly);

    ASSERT_DOUBLE_EQ(100.0, pResultShape1->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape1->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape1->GetCoordSys());

    AComp2 = pResultShape1->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape2 = (HVE2DPolygonOfSegments*)Poly1.DifferentiateFromShapeSCS(EastContiguousPoly);

    ASSERT_DOUBLE_EQ(100.0, pResultShape2->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape2->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape2->GetCoordSys());

    AComp2 = pResultShape2->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape3 = (HVE2DPolygonOfSegments*)Poly1.DifferentiateFromShapeSCS(WestContiguousPoly);

    ASSERT_DOUBLE_EQ(100.0, pResultShape3->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape3->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape3->GetCoordSys());

    AComp2 = pResultShape3->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_NEAR(0.0, AComp2.GetLinear(0).GetStartPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_NEAR(0.0, AComp2.GetLinear(0).GetEndPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_NEAR(0.0, AComp2.GetLinear(1).GetStartPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_NEAR(0.0, AComp2.GetLinear(3).GetEndPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());   

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape4 = (HVE2DPolygonOfSegments*)Poly1.DifferentiateFromShapeSCS(SouthContiguousPoly);

    ASSERT_DOUBLE_EQ(100.0, pResultShape4->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape4->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape4->GetCoordSys());

    AComp2 = pResultShape4->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_NEAR(0.0, AComp2.GetLinear(0).GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_NEAR(0.0, AComp2.GetLinear(2).GetEndPoint().GetY(), MYEPSILON);

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_NEAR(0.0, AComp2.GetLinear(3).GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_NEAR(0.0, AComp2.GetLinear(3).GetEndPoint().GetY(), MYEPSILON);   

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape5 =(HVE2DPolygonOfSegments*) Poly1.DifferentiateFromShapeSCS(VerticalFitPoly);

    ASSERT_DOUBLE_EQ(50.0, pResultShape5->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape5->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape5->GetCoordSys());

    AComp2 = pResultShape5->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape6 = (HVE2DPolygonOfSegments*)Poly1.DifferentiateFromShapeSCS(HorizontalFitPoly);

    ASSERT_DOUBLE_EQ(50.0, pResultShape6->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape6->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape6->GetCoordSys());

    AComp2 = pResultShape6->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape7 = (HVE2DPolygonOfSegments*)Poly1.DifferentiateFromShapeSCS(DisjointPoly);
    
    ASSERT_DOUBLE_EQ(100.0, pResultShape7->CalculateArea()); 
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape7->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape7->GetCoordSys());
    
    AComp2 = pResultShape7->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(-10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_NEAR(0.0, AComp2.GetLinear(0).GetEndPoint().GetY(), MYEPSILON);

    ASSERT_DOUBLE_EQ(-10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_NEAR(0.0, AComp2.GetLinear(1).GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp2.GetLinear(1).GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp2.GetLinear(1).GetEndPoint().GetY(), MYEPSILON);

    ASSERT_NEAR(0.0, AComp2.GetLinear(2).GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp2.GetLinear(2).GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp2.GetLinear(2).GetEndPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(-10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_NEAR(0.0, AComp2.GetLinear(3).GetStartPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(-10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-10.0, AComp2.GetLinear(3).GetEndPoint().GetY());  

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape8 = (HVE2DPolygonOfSegments*)Poly1.DifferentiateFromShapeSCS(MiscPoly1);
    
    ASSERT_DOUBLE_EQ(75.0, pResultShape8->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape8->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape8->GetCoordSys());
    
    AComp2 = pResultShape8->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(6, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(5.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(5.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(5.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetEndPoint().GetY());   

    ASSERT_DOUBLE_EQ(5.0, AComp2.GetLinear(4).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(4).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(4).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(4).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(5).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetEndPoint().GetY());   

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape9 = (HVE2DPolygonOfSegments*)Poly1.DifferentiateFromShapeSCS(EnglobPoly1);

    ASSERT_DOUBLE_EQ(300.0, pResultShape9->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape9->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape9->GetCoordSys());

    AComp2 = pResultShape9->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(6, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(3).GetEndPoint().GetY());   

    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(4).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(4).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(4).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(4).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(5).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(5).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetEndPoint().GetY());     

    HFCPtr<HVE2DShape>     pResultShape10 = (HVE2DPolygonOfSegments*)Poly1.DifferentiateFromShapeSCS(EnglobPoly2);

    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape10->GetShapeType());
    ASSERT_TRUE(pResultShape10->HasHoles());
    ASSERT_DOUBLE_EQ(800.0, pResultShape10->CalculateArea());

    HFCPtr<HVE2DPolygonOfSegments>     pResultShape11 = (HVE2DPolygonOfSegments*) Poly1.DifferentiateFromShapeSCS(EnglobPoly3);

    ASSERT_DOUBLE_EQ(100.0, pResultShape11->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape11->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape11->GetCoordSys());

    AComp2 = pResultShape11->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetEndPoint().GetY()); 

    HFCPtr<HVE2DShape>     pResultShape12 = Poly1.DifferentiateFromShapeSCS(IncludedPoly1);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape12->GetShapeType());
    ASSERT_TRUE(pResultShape12->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape13 = Poly1.DifferentiateFromShapeSCS(IncludedPoly2);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape13->GetShapeType());
    ASSERT_TRUE(pResultShape13->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape14 = Poly1.DifferentiateFromShapeSCS(IncludedPoly3);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape14->GetShapeType());
    ASSERT_TRUE(pResultShape14->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape15 = Poly1.DifferentiateFromShapeSCS(IncludedPoly4);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape15->GetShapeType());
    ASSERT_TRUE(pResultShape15->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape16 = Poly1.DifferentiateFromShapeSCS(IncludedPoly5);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape16->GetShapeType());
    ASSERT_TRUE(pResultShape16->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape17 = Poly1.DifferentiateFromShapeSCS(IncludedPoly6);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape17->GetShapeType());
    ASSERT_TRUE(pResultShape17->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape18 = Poly1.DifferentiateFromShapeSCS(IncludedPoly7);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape18->GetShapeType());
    ASSERT_TRUE(pResultShape18->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape19 = Poly1.DifferentiateFromShapeSCS(IncludedPoly8);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape19->GetShapeType());
    ASSERT_TRUE(pResultShape19->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape20 = Poly1.DifferentiateFromShapeSCS(IncludedPoly9);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape20->GetShapeType());
    ASSERT_TRUE(pResultShape20->IsEmpty());
    
    }
    
//==================================================================================
// SPECIAL TESTS
// The following are all case which failed with previous library
//==================================================================================

//==================================================================================
// IntersectShapeTestWhoFailed
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  IntersectShapeTestWhoFailed)
    {

    HVE2DSegment    Segment1A(HGF2DLocation(-1.7E308, -1.7E308, pWorld), HGF2DLocation(-1.7E308, 1.7E308, pWorld));
    HVE2DSegment    Segment2A(HGF2DLocation(-1.7E308, 1.7E308, pWorld), HGF2DLocation(1.7E308, 1.7E308, pWorld));
    HVE2DSegment    Segment3A(HGF2DLocation(1.7E308, 1.7E308, pWorld), HGF2DLocation(1.7E308, -1.7E308, pWorld));
    HVE2DSegment    Segment4A(HGF2DLocation(1.7E308, -1.7E308, pWorld), HGF2DLocation(-1.7E308, -1.7E308, pWorld));
    HVE2DComplexLinear  MyLinear1(pWorld);
    MyLinear1.AppendLinear(Segment1A);
    MyLinear1.AppendLinear(Segment2A);
    MyLinear1.AppendLinear(Segment3A);
    MyLinear1.AppendLinear(Segment4A);

    HVE2DPolygonOfSegments    Poly1A(MyLinear1);

    HVE2DSegment    Segment1B(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(395.59575494628, -76.896025049963, pWorld));
    HVE2DSegment    Segment2B(HGF2DLocation(395.59575494628, -76.896025049963, pWorld), HGF2DLocation(471.91935301076, 315.75484834585, pWorld));
    HVE2DSegment    Segment3B(HGF2DLocation(471.91935301076, 315.75484834585, pWorld), HGF2DLocation(76.323598064480, 392.65087339581, pWorld));
    HVE2DSegment    Segment4B(HGF2DLocation(76.323598064480, 392.65087339581, pWorld), HGF2DLocation(0.0, 0.0, pWorld));
    HVE2DComplexLinear  MyLinear2(pWorld);
    MyLinear2.AppendLinear(Segment1B);
    MyLinear2.AppendLinear(Segment2B);
    MyLinear2.AppendLinear(Segment3B);
    MyLinear2.AppendLinear(Segment4B);

    HVE2DPolygonOfSegments    Poly1B(MyLinear2);

    HFCPtr<HVE2DPolygonOfSegments>     pResult1A = (HVE2DPolygonOfSegments*) Poly1A.IntersectShape(Poly1B);

    ASSERT_DOUBLE_EQ(161200.000000001717, pResult1A->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResult1A->GetShapeType());
    ASSERT_TRUE(pResult1A->IsSimple());

    HVE2DComplexLinear  AComp1A;
    AComp1A = pResult1A->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp1A.GetNumberOfLinears());

    ASSERT_NEAR(0.0, AComp1A.GetLinear(0).GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp1A.GetLinear(0).GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(76.323598064480, AComp1A.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(392.65087339581, AComp1A.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(76.323598064480, AComp1A.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(392.65087339581, AComp1A.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(471.91935301076, AComp1A.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(315.75484834585, AComp1A.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(471.919353010760, AComp1A.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(315.754848345850, AComp1A.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(395.595754946280, AComp1A.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-76.896025049963, AComp1A.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(395.595754946280, AComp1A.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-76.896025049963, AComp1A.GetLinear(3).GetStartPoint().GetY());
    ASSERT_NEAR(0.0, AComp1A.GetLinear(3).GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp1A.GetLinear(3).GetEndPoint().GetY(), MYEPSILON);

    }

//==================================================================================
// Test which failed on 21 may 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  IntersectShapeTestWithPointerWhoFailed)
    {
   
    HFCPtr<HGF2DCoordSys>   pWorld2 = new HGF2DCoordSys(HGF2DIdentity(), pWorld);
    HFCPtr<HVE2DShape>      pShape1 = new HVE2DRectangle (0.0, 0.0, 415.0, 409.0, pWorld);

    HVE2DComplexLinear  TheLinear(pWorld2);

    TheLinear.AppendLinear(HVE2DSegment(HGF2DLocation(0.0 , 256.0, pWorld2), HGF2DLocation(83.0 , 256.0, pWorld2)));
    TheLinear.AppendLinear(HVE2DSegment(HGF2DLocation(83.0 , 256.0, pWorld2), HGF2DLocation(83.0 , 0.0, pWorld2)));
    TheLinear.AppendLinear(HVE2DSegment(HGF2DLocation(83.0 , 0.0, pWorld2), HGF2DLocation(0.0 , 0.0, pWorld2)));
    TheLinear.AppendLinear(HVE2DSegment(HGF2DLocation(0.0 , 0.0, pWorld2), HGF2DLocation(0.0 , 256.0, pWorld2)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear);

    HFCPtr<HVE2DPolygonOfSegments> pResult = (HVE2DPolygonOfSegments*) pShape1->IntersectShape(*pShape2);

    ASSERT_DOUBLE_EQ(21248.0, pResult->CalculateArea());
    #ifdef WIP_IPPTEST_BUG_1
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResult->GetShapeType());
    #endif       
    ASSERT_TRUE(pResult->IsSimple());

    HVE2DComplexLinear  AComp;
    AComp = pResult->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp.GetNumberOfLinears());

    ASSERT_NEAR(0.0, AComp.GetLinear(0).GetStartPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(256.0, AComp.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(83.00, AComp.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(256.0, AComp.GetLinear(0).GetEndPoint().GetY());
    
    ASSERT_DOUBLE_EQ(83.00, AComp.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(256.0, AComp.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(83.00, AComp.GetLinear(1).GetEndPoint().GetX());
    ASSERT_NEAR(0.0, AComp.GetLinear(1).GetEndPoint().GetY(), MYEPSILON);

    ASSERT_DOUBLE_EQ(83.0, AComp.GetLinear(2).GetStartPoint().GetX());
    ASSERT_NEAR(0.0, AComp.GetLinear(2).GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp.GetLinear(2).GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp.GetLinear(2).GetEndPoint().GetY(), MYEPSILON);

    ASSERT_NEAR(0.0, AComp.GetLinear(3).GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp.GetLinear(3).GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp.GetLinear(3).GetEndPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(256.0, AComp.GetLinear(3).GetEndPoint().GetY());

    }

//==================================================================================
// Another test which failed on 22 may 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  IntersectShapeTestWithPointerWhoFailed2)
    {
        
    HFCPtr<HGF2DCoordSys>   pWorld2 =    new HGF2DCoordSys(HGF2DIdentity(), pWorld);

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(256.0, 0.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0, 0.0, pWorld), HGF2DLocation(256.0, 256.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0, 256.0, pWorld), HGF2DLocation(0.0, 256.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 256.0, pWorld), HGF2DLocation(0.0, 0.0, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld2);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.0, 105.0, pWorld2), HGF2DLocation(256.0, 0.0, pWorld2)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.0, 0.0, pWorld2), HGF2DLocation(0.0, 0.0, pWorld2)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld2), HGF2DLocation(0.0,  105.0, pWorld2)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 105.0, pWorld2), HGF2DLocation(256.0, 105.0, pWorld2)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DRectangle> pResult = (HVE2DRectangle*) pShape1->IntersectShape(*pShape2);

    ASSERT_DOUBLE_EQ(26880.0, pResult->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResult->GetShapeType());
    ASSERT_TRUE(pResult->IsSimple());

    double RectXMin;
    double RectXMax;
    double RectYMin;
    double RectYMax;

    pResult->GetRectangle(&RectXMin, &RectYMin, &RectXMax, &RectYMax);

    ASSERT_NEAR(0.0, RectXMin, MYEPSILON);
    ASSERT_DOUBLE_EQ(256.0, RectXMax);
    ASSERT_NEAR(0.0, RectYMin, MYEPSILON);
    ASSERT_DOUBLE_EQ(105.0, RectYMax);
    
    }

//==================================================================================
// Test which failed on may 28 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  IntersectShapeTestWithPointerWhoFailed3)
    {
           
    HFCPtr<HGF2DCoordSys>   pWorld2 =    new HGF2DCoordSys(HGF2DIdentity(), pWorld);

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 331.90680836798, pWorld), HGF2DLocation(172.85086905196 , 370.67988489798, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(172.85086905196 , 370.67988489798, pWorld), HGF2DLocation(71.345071697720 , 153.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(71.345071697720 , 153.0, pWorld), HGF2DLocation(256.0 , 153.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 153.0, pWorld), HGF2DLocation(256.0 , 331.90680836798, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  Linear2(pWorld2);

    Linear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 331.90680836798, pWorld2), HGF2DLocation(172.85086905196 , 370.67988489798, pWorld2)));
    Linear2.AppendLinear(HVE2DSegment(HGF2DLocation(172.85086905196 , 370.67988489798, pWorld2), HGF2DLocation(53.476108564268 , 114.67988489798, pWorld2)));
    Linear2.AppendLinear(HVE2DSegment(HGF2DLocation(53.476108564268 , 114.67988489798, pWorld2), HGF2DLocation(256.0 , 114.67988489798, pWorld2)));
    Linear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 114.67988489798, pWorld2), HGF2DLocation(256.0 , 331.90680836798, pWorld2)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(Linear2);

    HFCPtr<HVE2DPolygonOfSegments> pResult = (HVE2DPolygonOfSegments*) pShape1->IntersectShape(*pShape2);

    ASSERT_DOUBLE_EQ(27535.8045875850621, pResult->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResult->GetShapeType());
    ASSERT_TRUE(pResult->IsSimple());

    HVE2DComplexLinear  AComp;
    AComp = pResult->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp.GetNumberOfLinears());  

    ASSERT_DOUBLE_EQ(256.00000000000, AComp.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(331.90680836798, AComp.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(256.00000000000, AComp.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(153.00000000000, AComp.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(256.00000000000, AComp.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(153.00000000000, AComp.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(71.345071697720, AComp.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(153.00000000000, AComp.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(71.345071697720, AComp.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(153.00000000000, AComp.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(172.85086905196, AComp.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(370.67988489798, AComp.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(172.85086905196, AComp.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(370.67988489798, AComp.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(256.00000000000, AComp.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(331.90680836798, AComp.GetLinear(3).GetEndPoint().GetY());

    }

//==================================================================================
// Test which failed on may 30 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  IntersectShapeTestWithPointerWhoFailed4)
    {
           
    HFCPtr<HGF2DCoordSys>   pWorld2 = new HGF2DCoordSys(HGF2DIdentity(), pWorld);

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0 , 0.0, pWorld), HGF2DLocation(0.0 , 256.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0 , 256.0, pWorld), HGF2DLocation(256.0 , 256.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 256.0, pWorld), HGF2DLocation(256.0 , 0.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 0.0, pWorld), HGF2DLocation(0.0 , 0.0, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld2);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 38.773076530008, pWorld2), HGF2DLocation(172.85086905196, 0.0, pWorld2)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(172.85086905196, 0.0, pWorld2), HGF2DLocation(53.476108564268 , 256.0, pWorld2)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(53.476108564268 , 256.0, pWorld2), HGF2DLocation(256.0 , 256.0, pWorld2)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 256.0, pWorld2), HGF2DLocation(256.0 , 38.773076530008, pWorld2)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DRectangle> pResult = (HVE2DRectangle*) pShape1->IntersectShape(*pShape2);
    
    ASSERT_DOUBLE_EQ(34954.1730562968150, pResult->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResult->GetShapeType());
    ASSERT_TRUE(pResult->IsSimple());

    HVE2DComplexLinear  AComp;
    AComp = pResult->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(256.00000000000, AComp.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(38.773076530008, AComp.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(172.85086905196, AComp.GetLinear(0).GetEndPoint().GetX());
    ASSERT_NEAR(0.0, AComp.GetLinear(0).GetEndPoint().GetY(), MYEPSILON);

    ASSERT_DOUBLE_EQ(172.85086905196, AComp.GetLinear(1).GetStartPoint().GetX());
    ASSERT_NEAR(0.0, AComp.GetLinear(1).GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(53.476108564268, AComp.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(256.00000000000, AComp.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(53.476108564268, AComp.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(256.00000000000, AComp.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(256.00000000000, AComp.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(256.00000000000, AComp.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(256.00000000000, AComp.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(256.00000000000, AComp.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(256.00000000000, AComp.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(38.773076530008, AComp.GetLinear(3).GetEndPoint().GetY());

    }

//==================================================================================
// Test which failed on june 4 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  IntersectShapeTestWithPointerWhoFailed5)
    {
          
    HFCPtr<HGF2DCoordSys>   pWorld2 = new HGF2DCoordSys(HGF2DIdentity(), pWorld);

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0 , 0.0, pWorld), HGF2DLocation(172.85086905196 , 370.67988489798, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(172.85086905196 , 370.67988489798, pWorld), HGF2DLocation(548.96860067216 , 195.29330627558, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(548.96860067216 , 195.29330627558, pWorld), HGF2DLocation(376.11773162020 , -175.38657862240, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(376.11773162020 , -175.38657862240, pWorld), HGF2DLocation(0.0 , 0.0, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld2);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(77.0, 165.12703287922, pWorld2), HGF2DLocation(77.0, 114.67988489798, pWorld2)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(77.0, 114.67988489798, pWorld2), HGF2DLocation(53.476108564268 , 114.67988489798, pWorld2)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(53.476108564268 , 114.67988489798, pWorld2), HGF2DLocation(77.0 , 165.12703287922, pWorld2)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DPolygonOfSegments> pResult = (HVE2DPolygonOfSegments*) pShape1->IntersectShape(*pShape2);

    ASSERT_DOUBLE_EQ(593.356616176497936, pResult->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResult->GetShapeType());
    ASSERT_TRUE(pResult->IsSimple());

    HVE2DComplexLinear  AComp;
    AComp = pResult->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(3, AComp.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(77.000000000000, AComp.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(165.12703287922, AComp.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(77.000000000000, AComp.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(114.67988489798, AComp.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(77.000000000000, AComp.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(114.67988489798, AComp.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(53.476108564268, AComp.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(114.67988489798, AComp.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(53.476108564268, AComp.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(114.67988489798, AComp.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(77.000000000000, AComp.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(165.12703287922, AComp.GetLinear(2).GetEndPoint().GetY());

    }

//==================================================================================
// Test which failed on july 9 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  IntersectShapeTestWithPointerWhoFailed6)
    {
           
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , -56.753833636596, pWorld), HGF2DLocation(256.0 , 49.442061113575, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 49.442061113575, pWorld), HGF2DLocation(189.15280219211 , 80.613421377599, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(189.15280219211 , 80.613421377599, pWorld), HGF2DLocation(37.5906557384530807, 80.6134213775989394, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(37.5906557384530807, 80.6134213775989394, pWorld), HGF2DLocation(0.0, 0.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(232.01479348138, -108.19027500563, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(232.01479348138, -108.19027500563, pWorld), HGF2DLocation(256.0,  -56.753833636596, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.0, -119.37476048769, pWorld), HGF2DLocation(0.0, 0.0, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(37.5906557384530799, 80.6134213775989536, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(37.5906557384530799, 80.6134213775989536, pWorld), HGF2DLocation(256.0, 80.613421377599, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.0, 80.613421377599, pWorld), HGF2DLocation(256.0, -119.37476048769, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DPolygonOfSegments> pResult = (HVE2DPolygonOfSegments*) pShape2->IntersectShape(*pShape1);

    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResult->GetShapeType());
    ASSERT_TRUE(pResult->IsSimple());
    ASSERT_DOUBLE_EQ(32609.0025554273561, pResult->CalculateArea());

    HVE2DComplexLinear  AComp;
    AComp = pResult->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(6, AComp.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(256.000000000000000, AComp.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-56.753833636595999, AComp.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(232.014793481380000, AComp.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-108.19027500563000, AComp.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(232.014793481380, AComp.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-108.19027500563, AComp.GetLinear(1).GetStartPoint().GetY());
    ASSERT_NEAR(0.0, AComp.GetLinear(1).GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp.GetLinear(1).GetEndPoint().GetY(), MYEPSILON);

    ASSERT_NEAR(0.0, AComp.GetLinear(2).GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp.GetLinear(2).GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(37.590655738453080, AComp.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(80.613421377598939, AComp.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(37.590655738453080, AComp.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(80.613421377598939, AComp.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(189.15280219210999, AComp.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(80.613421377598939, AComp.GetLinear(3).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(189.15280219210999, AComp.GetLinear(4).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(80.613421377598939, AComp.GetLinear(4).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(256.00000000000000, AComp.GetLinear(4).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(49.442061113575001, AComp.GetLinear(4).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(256.000000000000000, AComp.GetLinear(5).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(49.4420611135750010, AComp.GetLinear(5).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(256.000000000000000, AComp.GetLinear(5).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-56.753833636595999, AComp.GetLinear(5).GetEndPoint().GetY());
 
    } 

//==================================================================================
// Test which failed on aug 26, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  GenerateScanLineTestWhoFailed)
    {

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 175.0, pWorld), HGF2DLocation(256.0 , 30.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 30.0, pWorld), HGF2DLocation(230.0 , 30.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(230.0 , 30.0, pWorld), HGF2DLocation(230.0 , 30.00000000001, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(230.0 , 30.00000000001, pWorld), HGF2DLocation(230.0 , 30.00000000002, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(230.0 , 30.00000000002, pWorld), HGF2DLocation(230.0 , 175.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(230.0 , 175.0, pWorld), HGF2DLocation(256.0 , 175.0, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    // Create shape of same size
    HVEShape  MyShape(pShape1->GetExtent());

    // Generate scanlines for this dum shape (this insures creation of Y)
    HGFScanLines  TheScanLines(false, pWorld);

    MyShape.GenerateScanLines(TheScanLines);
    
    }

//==================================================================================
// Test which failed on aug 26, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  GenerateScanLineTestWhoFailed2)
    {
           
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 234.38339996446, pWorld), HGF2DLocation(243.5 , 237.5, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(243.5 , 237.5, pWorld), HGF2DLocation(243.625 , 238.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(243.625 , 238.0, pWorld), HGF2DLocation(256.0 , 238.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 238.0, pWorld), HGF2DLocation(256.0 , 234.38339996446, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    // Create shape of same size
    HVEShape  MyShape(pShape1->GetExtent());

    // Generate scanlines for this dum shape (this insures creation of Y)
    HGFScanLines TheScanLines(false, pWorld);

    // Generate scanlines for our shape
    MyShape.GenerateScanLines(TheScanLines);
    
    }

//==================================================================================
// Test which failed on aug 26, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  GenerateScanLineTestWhoFailed3)
    {
   
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000 , 163.061, pWorld), HGF2DLocation(250.225 , 154.500, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(250.225 , 154.500, pWorld), HGF2DLocation(256.000 , 150.605, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000 , 150.605, pWorld), HGF2DLocation(256.000 , 122.101, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000 , 122.101, pWorld), HGF2DLocation(53.2254 , 79.0000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(53.2254 , 79.0000, pWorld), HGF2DLocation(15.6029 , 256.000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(15.6029 , 256.000, pWorld), HGF2DLocation(256.000 , 256.000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000 , 256.000, pWorld), HGF2DLocation(256.000 , 163.061, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    // Create shape of same size
    HVEShape  MyShape(pShape1->GetExtent());

    // Generate scanlines for this dum shape (this insures creation of Y)
    HGFScanLines  TheScanLines(false, pWorld);

    // Generate scanlines for our shape
    MyShape.GenerateScanLines(TheScanLines);

    }

//==================================================================================
// Test which failed on aug 27, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed)
    {

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2048.000000000000000 , 1097.903989243390200, pWorld),
                                         HGF2DLocation(2168.002336591538600 , 986.0000000000001100, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2168.002336591538600 , 986.0000000000001100, pWorld),
                                         HGF2DLocation(2470.000000000000000 , 986.0000000000001100, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2470.000000000000000 , 986.0000000000001100, pWorld),
                                         HGF2DLocation(2470.000000000000000 , 704.3826228932581400, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2470.000000000000000 , 704.3826228932581400, pWorld),
                                         HGF2DLocation(2569.421928625488400 , 611.6701745570791200, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2569.421928625488400 , 611.6701745570791200, pWorld),
                                         HGF2DLocation(2048.000000000000000 , 611.6701745570791200, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2048.000000000000000 , 611.6701745570791200, pWorld),
                                         HGF2DLocation(2048.000000000000000 , 1097.903989243390200, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1279.605405332091600 , 0.000000000000000000, pWorld),
                                         HGF2DLocation(1280.000000000000000 , 0.000000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1280.000000000000000 , 0.000000000000000000, pWorld),
                                         HGF2DLocation(1280.000000000000000 , 1024.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1280.000000000000000 , 1024.000000000000000, pWorld),
                                         HGF2DLocation(1248.098025825285000 , 1024.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1248.098025825285000 , 1024.000000000000000, pWorld),
                                         HGF2DLocation(1657.011823802983100 , 1462.506362048594600, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1657.011823802983100 , 1462.506362048594600, pWorld),
                                         HGF2DLocation(2168.002336591538600 , 986.0000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2168.002336591538600 , 986.0000000000000000, pWorld),
                                         HGF2DLocation(1396.000000000000000 , 986.0000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1396.000000000000000 , 986.0000000000000000, pWorld),
                                         HGF2DLocation(1396.000000000000000 , 136.0000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1396.000000000000000 , 136.0000000000000000, pWorld),
                                         HGF2DLocation(2233.100842751780000 , 136.0000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2233.100842751780000 , 136.0000000000000000, pWorld),
                                         HGF2DLocation(1721.775008346975300 , -412.329825442920880, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1721.775008346975300 , -412.329825442920880, pWorld),
                                         HGF2DLocation(1279.605405332091600 , 0.000000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(6714.37009156061685, pResult->CalculateArea());

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(778332.088479116559, pResult->CalculateArea());

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(162576.011547499569, pResult->CalculateArea());

    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_DOUBLE_EQ(609041.706840056227, pResult->CalculateArea());

    }

//==================================================================================
// Test which failed on sep 15, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed2)
    {
   
    HVE2DComplexLinear  TheLinear1(pWorld); 

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000, 0.000000, pWorld), HGF2DLocation(0.000000, 0.000001, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000, 0.000001, pWorld), HGF2DLocation(0.000001, 0.000001, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000001, 0.000001, pWorld), HGF2DLocation(0.000001, 0.000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000001, 0.000000, pWorld), HGF2DLocation(0.000000, 0.000000, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , -229.449314192432380, pWorld),
                                         HGF2DLocation(0.00000000000000000 , 453.2173524742341900, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , 453.2173524742341900, pWorld),
                                         HGF2DLocation(682.666666666666630 , 453.2173524742341900, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(682.666666666666630 , 453.2173524742341900, pWorld),
                                         HGF2DLocation(682.666666666666630 , -229.449314192432380, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(682.666666666666630 , -229.449314192432380, pWorld),
                                         HGF2DLocation(0.00000000000000000 , -229.449314192432380, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(466033.777777777635, pResult->CalculateArea());

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);

    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_DOUBLE_EQ(466033.777777777635, pResult->CalculateArea());

    }

//==================================================================================
// Test which failed on sep 25, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed3)
    {
  
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000, 0.000, pWorld), HGF2DLocation(0.000, 256.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000, 256.0, pWorld), HGF2DLocation(256.0, 256.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0, 256.0, pWorld), HGF2DLocation(256.0, 0.000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0, 0.000, pWorld), HGF2DLocation(0.000, 0.000, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , 192.000000000000000, pWorld),
                                        HGF2DLocation(26.00000000000004300 , 253.252161511417650, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(26.0000000000000430 , 253.252161511417650, pWorld),
                                         HGF2DLocation(26.0000000000000430 , 409.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(26.0000000000000430 , 409.000000000000000, pWorld),
                                         HGF2DLocation(441.000000000000060 , 409.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(441.000000000000060 , 409.000000000000000, pWorld),
                                         HGF2DLocation(441.000000000000060 , 0.00000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(441.000000000000060 , 0.00000000000000000, pWorld),
                                         HGF2DLocation(26.0000000000000430 , 0.00000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(26.0000000000000430 , 0.00000000000000000, pWorld),
                                         HGF2DLocation(26.0000000000000430 , 180.963654778550250, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(26.0000000000000430 , 180.963654778550250, pWorld),
                                         HGF2DLocation(0.00000000000000000 , 192.000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(59819.7505875272618, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(176391.000000000029, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(5716.24941247273272, pResult->CalculateArea());
    
    HVE2DRectangle  Rect1(0.0, 0.0, 256.0, 256.0, pWorld);

    pResult = Rect1.DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(5716.24941247273363, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_DOUBLE_EQ(110855.000000000029, pResult->CalculateArea());    
    
    }

//==================================================================================
// Test which failed on sep 26, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed4)
    {
       
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(126.000002333325810 , 374.000007290433640, pWorld),
                                         HGF2DLocation(382.000007298135420 , 374.000007365927220, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(382.000007298135420 , 374.000007365927220, pWorld),
                                         HGF2DLocation(382.000007373628990 , 118.000002401117640, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(382.000007373628990 , 118.000002401117640, pWorld),
                                         HGF2DLocation(126.000002408819400 , 118.000002325624050, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(126.000002408819400 , 118.000002325624050, pWorld),
                                         HGF2DLocation(126.000002333325810 , 374.000007290433640, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(126.000002405280640 , 130.000002633843170, pWorld),
                                         HGF2DLocation(382.000007298135420 , 130.000002709336770, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(382.000007298135420 , 130.000002709336770, pWorld),
                                         HGF2DLocation(382.000007301674200 , 118.000002401117640, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(382.000007301674200 , 118.000002401117640, pWorld),
                                         HGF2DLocation(126.000002408819400 , 118.000002325624050, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(126.000002408819400 , 118.000002325624050, pWorld),
                                         HGF2DLocation(126.000002405280640 , 130.000002633843170, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(3072.00015698717970, pResult->CalculateArea());    

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.002562214824, pResult->CalculateArea());   

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(62464.0024228270776, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
       
    }

//==================================================================================
// Test which failed on sep 26, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed5)
    {
       
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(126.000002408819410 , 118.0000023256240000, pWorld),
                                         HGF2DLocation(382.000007298135470 , 118.0000024011175800, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(382.000007298135470 , 118.0000024011175800, pWorld),
                                         HGF2DLocation(382.000007370090320 , -126.000002330966550, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(382.000007370090320 , -126.000002330966550, pWorld),
                                         HGF2DLocation(126.000002480774240 , -126.000002406460140, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(126.000002480774240 , -126.000002406460140, pWorld),
                                         HGF2DLocation(126.000002408819410 , 118.0000023256240000, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(126.000002257832220 , -126.000002406460110, pWorld),
                                         HGF2DLocation(126.000002257832220 , 130.0000026338431100, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(126.000002257832220 , 130.0000026338431100, pWorld),
                                         HGF2DLocation(382.000007298135420 , 130.0000026338431100, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(382.000007298135420 , 130.0000026338431100, pWorld),
                                         HGF2DLocation(382.000007298135420 , -126.000002406460110, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(382.000007298135420 , -126.000002406460110, pWorld),
                                         HGF2DLocation(126.000002257832220 , -126.000002406460110, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(62464.0024044066740, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.0025806352787, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_DOUBLE_EQ(3072.00017534392236, pResult->CalculateArea());
      
    }

//==================================================================================
// Test which failed on sep 29, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed6)
    {
  
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(600.76736535497 , -864.4478207542, pWorld),
                                         HGF2DLocation(598.11357904592, -16.448934551255, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(598.11357904592, -16.448934551255, pWorld),
                                         HGF2DLocation(1210.1105822119, -14.533701979160, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1210.1105822119, -14.533701979160, pWorld),
                                         HGF2DLocation(1212.7643685210, -862.52954950332, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1212.7643685210, -862.52954950332, pWorld),
                                         HGF2DLocation(600.76736535497 , -864.4478207542, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.50213688201825, 0.00000000000000, pWorld),
                                         HGF2DLocation(0.00000000000000, 160.453759742730, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000, 160.453759742730, pWorld),
                                         HGF2DLocation(115.799175663380, 160.816150983050, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(115.799175663380, 160.816150983050, pWorld),
                                         HGF2DLocation(116.301312545400, 0.36239124032448, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(116.301312545400, 0.36239124032448, pWorld),
                                         HGF2DLocation(0.50213688201825, 0.00000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(537557.528943443554, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(518976.933863138081, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_DOUBLE_EQ(18580.5950803055275, pResult->CalculateArea());
    
    }

//==================================================================================
// Test which failed on April 18, 2002
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  AllocateCopyInCoordSysTestWhoFailed)
    {
   
    HFCPtr<HGF2DCoordSys>   pWorld = new HGF2DCoordSys();

    HFCMatrix<3, 3> MyMatrix;
    MyMatrix[0][0] = -65.4023661385180000;
    MyMatrix[0][1] = -12.5450503464860000;
    MyMatrix[0][2] = 404855.3743669100000;
    MyMatrix[1][0] = -762.296496103310000;
    MyMatrix[1][1] = -140.015895911930000;
    MyMatrix[1][2] = 4690515.004039000000;
    MyMatrix[2][0] = -0.00016253176480847;
    MyMatrix[2][1] = -2.98356438587470E-5;
    MyMatrix[2][2] = 1.000000000000000000;

    HGF2DProjective MyModel(MyMatrix);

    HFCPtr<HGF2DCoordSys> pNewModel = new HGF2DCoordSys(MyModel, pWorld);

    HVE2DPolySegment  AddPolySegment1(pWorld);
    AddPolySegment1.AppendPoint(HGF2DLocation(402409.93750000, 4693423.0000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(402516.78125000, 4693437.0000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(402484.00000000, 4693438.0000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(402409.93750000, 4693423.0000000, pWorld));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(AddPolySegment1);

    HFCPtr<HVE2DShape> pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pNewModel));

    }

//==================================================================================
// Test which failed on sep 29, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed7) 
    {
    
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(126.000002257832220 , 630.000012255243180, pWorld),
                                         HGF2DLocation(382.000007222641780 , 630.000012330736810, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(382.000007222641780 , 630.000012330736810, pWorld),
                                         HGF2DLocation(382.000007298135420 , 374.000007365927160, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(382.000007298135420 , 374.000007365927160, pWorld),
                                         HGF2DLocation(126.000002333325810 , 374.000007290433590, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(126.000002333325810 , 374.000007290433590, pWorld),
                                         HGF2DLocation(126.000002257832220 , 630.000012255243180, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(126.000002329787050 , 386.000007674146330, pWorld),
                                         HGF2DLocation(382.000007298135420 , 386.000007749639910, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(382.000007298135420 , 386.000007749639910, pWorld),
                                         HGF2DLocation(382.000007301674200 , 374.000007365927160, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(382.000007301674200 , 374.000007365927160, pWorld),
                                         HGF2DLocation(126.000002333325810 , 374.000007290433590, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(126.000002333325810 , 374.000007290433590, pWorld),
                                         HGF2DLocation(126.000002329787050 , 386.000007674146330, pWorld)));

    HGF2DLiteSegment LiteSegment1(HGF2DPosition(382.00000729814, 386.00000774964),
                                  HGF2DPosition(382.00000729814, 374.00000736593));
    HGF2DLiteSegment LiteSegment2(HGF2DPosition(382.00000729814, 386.00000774964),
                                  HGF2DPosition(382.00000730167, 374.00000736593));


    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    // Resumate of problem
    ASSERT_FALSE(LiteSegment1.Crosses(LiteSegment2));

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(3072.00017721946596, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.002581541194, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(62464.0024219211700, pResult->CalculateArea());  

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);   
    
    }

//==================================================================================
// Test which failed on sep 30, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed8)
    {
   
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(244.000000000000000 , -0.0000001474484110, pWorld),
                                         HGF2DLocation(-12.000000000000014 , -0.0000000719548210, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-12.000000000000014 , -0.0000000719548210, pWorld),
                                         HGF2DLocation(-11.999999926865598 , 248.000000075493490, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-11.999999926865598 , 248.000000075493490, pWorld),
                                         HGF2DLocation(244.000000073134430 , 247.999999999999890, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(244.000000073134430 , 247.999999999999890, pWorld),
                                         HGF2DLocation(244.000000000000000 , -0.0000001474484110, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , -0.0000000000000570, pWorld),
                                         HGF2DLocation(0.00000000000000000 , 247.999999999999940, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , 247.999999999999940, pWorld),
                                         HGF2DLocation(244.000000000000000 , 247.999999999999940, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(244.000000000000000 , 247.999999999999940, pWorld),
                                         HGF2DLocation(244.000000000000000 , -0.0000000000000570, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(244.000000000000000 , -0.0000000000000570, pWorld),
                                         HGF2DLocation(0.00000000000000000 , -0.0000000000000570, pWorld)));

    HGF2DLiteSegment LiteSegment1(HGF2DPosition(-11.999999926865598, 248.000000075493490),
                                  HGF2DPosition(244.000000073134430, 247.999999999999890));
    HGF2DLiteSegment LiteSegment2(HGF2DPosition(0.00000000000000000, 247.999999999999940),
                                  HGF2DPosition(244.000000000000000, 247.999999999999940));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    // Resumate of problem
    ASSERT_FALSE(LiteSegment1.Crosses(LiteSegment2));

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(60512.00000000000, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(63488.000057073128, pResult->CalculateArea());   

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(2976.00000176938328, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);    

    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
       
    }

//==================================================================================
// Test which failed on sep 30, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed9)
    {
    
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000002509535310 , -374.000007328770210, pWorld),
                                         HGF2DLocation(382.000007298135420 , -374.000007365927220, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(382.000007298135420 , -374.000007365927220, pWorld),
                                         HGF2DLocation(382.000007308751720 , -337.999994959696720, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(382.000007308751720 , -337.999994959696720, pWorld),
                                         HGF2DLocation(256.000002520151610 , -337.999994922539710, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000002520151610 , -337.999994922539710, pWorld),
                                         HGF2DLocation(256.000002509535310 , -374.000007328770210, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.000002520151610 , -593.999997479848390, pWorld),
                                         HGF2DLocation(256.000002520151610 , -337.999994959696780, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.000002520151610 , -337.999994959696780, pWorld),
                                         HGF2DLocation(512.000005040303220 , -337.999994959696780, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(512.000005040303220 , -337.999994959696780, pWorld),
                                         HGF2DLocation(512.000005040303220 , -593.999997479848390, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(512.000005040303220 , -593.999997479848390, pWorld),
                                         HGF2DLocation(256.000002520151610 , -593.999997479848390, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    // Resumate of problem
    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(4536.00173557470679, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.0012903176248, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_DOUBLE_EQ(60999.9995584427088, pResult->CalculateArea());    
   
    }

//==================================================================================
// Test which failed on sep 30, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed10)
    {
  
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(198.000002966624810 , 748.000013416875620, pWorld),
                                         HGF2DLocation(349.583085012412430 , 748.000013461576940, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(349.583085012412430 , 748.000013461576940, pWorld),
                                         HGF2DLocation(349.583085029172540 , 691.166168779997070, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(349.583085029172540 , 691.166168779997070, pWorld),
                                         HGF2DLocation(198.000002983384950 , 691.166168735295740, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(198.000002983384950 , 691.166168735295740, pWorld),
                                         HGF2DLocation(198.000002966624810 , 748.000013416875620, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , 691.1661687799971800, pWorld),
                                         HGF2DLocation(0.00000000000000000 , 1040.749253792409500, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , 1040.749253792409500, pWorld),
                                         HGF2DLocation(349.583085012412430 , 1040.749253792409500, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(349.583085012412430 , 1040.749253792409500, pWorld),
                                         HGF2DLocation(349.583085012412430 , 691.1661687799971800, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(349.583085012412430 , 691.1661687799971800, pWorld),
                                         HGF2DLocation(0.00000000000000000 , 691.1661687799971800, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(8615.0493413454733, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(122208.33332679553, pResult->CalculateArea());   

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);   

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_DOUBLE_EQ(113593.28399373978, pResult->CalculateArea());
    
    }

//==================================================================================
// Test which failed on sep 30, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed11)
    {
   
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(243.9999999999999700 , -0.0000001509871700, pWorld),
                                         HGF2DLocation(-267.999995110684150 , 0.00000000000000800, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-267.999995110684150 , 0.00000000000000800, pWorld),
                                         HGF2DLocation(-267.999994996853960 , 385.999995224514290, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-267.999994996853960 , 385.999995224514290, pWorld),
                                         HGF2DLocation(244.0000001138301400 , 385.999995073527090, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(244.0000001138301400 , 385.999995073527090, pWorld),
                                         HGF2DLocation(243.9999999999999700 , -0.0000001509871700, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , 0.00000000000001400, pWorld),
                                         HGF2DLocation(0.00000000000000000 , 256.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , 256.000000000000000, pWorld),
                                         HGF2DLocation(244.000000000000000 , 256.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(244.000000000000000 , 256.000000000000000, pWorld),
                                         HGF2DLocation(244.000000000000000 , 0.00000000000001400, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(244.000000000000000 , 0.00000000000001400, pWorld),
                                         HGF2DLocation(0.00000000000000000 , 0.00000000000001400, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(62464.000000000000, pResult->CalculateArea());    

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(197631.99578891927, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(135167.99561445243, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);   

    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
        
    }

//==================================================================================
// Test which failed on sep 30, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed12)
    {

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(78.7500007520188060 , 15.7500001457591220, pWorld),
                                         HGF2DLocation(78.7500007566634250 , 0.00000001857850100, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(78.7500007566634250 , 0.00000001857850100, pWorld),
                                         HGF2DLocation(15.7500001504037610 , 0.00000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(15.7500001504037610 , 0.00000000000000000, pWorld),
                                         HGF2DLocation(15.7500001504037610 , 15.7500001457591220, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(15.7500001504037610 , 15.7500001457591220, pWorld),
                                         HGF2DLocation(0.00000000000000000 , 15.7500001457591220, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , 15.7500001457591220, pWorld),
                                         HGF2DLocation(0.00000000000000000 , 121.999999099797760, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , 121.999999099797760, pWorld),
                                         HGF2DLocation(134.249998678397080 , 121.999999099797760, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(134.249998678397080 , 121.999999099797760, pWorld),
                                         HGF2DLocation(134.249998678397080 , 15.7500001457591220, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(134.249998678397080 , 15.7500001457591220, pWorld),
                                         HGF2DLocation(78.7500007520188060 , 15.7500001457591220, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , 0.00000000000000000, pWorld),
                                         HGF2DLocation(0.00000000000000000 , 663.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , 663.000000000000000, pWorld),
                                         HGF2DLocation(841.000000000000000 , 663.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(841.000000000000000 , 663.000000000000000, pWorld),
                                         HGF2DLocation(841.000000000000000 , 0.00000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(841.000000000000000 , 0.00000000000000000, pWorld),
                                         HGF2DLocation(0.00000000000000000 , 0.00000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(15256.3122372689940, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(557583.000000000000, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_DOUBLE_EQ(542326.68775506504, pResult->CalculateArea());   
   
    }

//==================================================================================
// Test which failed on sep 30, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed13)
    {
    
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1.4309074174435E-6, 439.00001100173, pWorld),
                                         HGF2DLocation(256.00000639572000, 439.00001107722, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.00000639572000, 439.00001107722, pWorld),
                                         HGF2DLocation(256.00000646767000, 194.99999970023, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.00000646767000, 194.99999970023, pWorld),
                                         HGF2DLocation(1.5028622470936E-6, 194.99999962474, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1.5028622470936E-6, 194.99999962474, pWorld),
                                         HGF2DLocation(1.4309074174435E-6, 439.00001100173, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000, 194.99999970023, pWorld),
                                         HGF2DLocation(0.000000000000000, 707.00000306043, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000, 707.00000306043, pWorld),
                                         HGF2DLocation(512.0000033602000, 707.00000306043, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(512.0000033602000, 707.00000306043, pWorld),
                                         HGF2DLocation(512.0000033602000, 194.99999970023, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(512.0000033602000, 194.99999970023, pWorld),
                                         HGF2DLocation(0.000000000000000, 194.99999970023, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(62464.004123923172, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(262144.00344084483, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);   

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);    

    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_DOUBLE_EQ(199679.99932658442, pResult->CalculateArea());   
    
    }

//==================================================================================
// Test which failed on sep 30, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed14)
    {
    
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(382.000007298135420 , 374.000007365927220, pWorld),
                                         HGF2DLocation(630.000012107794650 , 374.000007439061620, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(630.000012107794650 , 374.000007439061620, pWorld),
                                         HGF2DLocation(630.000012179749550 , 130.000002633843140, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(630.000012179749550 , 130.000002633843140, pWorld),
                                         HGF2DLocation(382.000007370090260 , 130.000002560708710, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(382.000007370090260 , 130.000002560708710, pWorld),
                                         HGF2DLocation(382.000007298135420 , 374.000007365927220, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(382.000007298135470 , 130.000002633843110, pWorld),
                                         HGF2DLocation(382.000007298135470 , 386.000007674146330, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(382.000007298135470 , 386.000007674146330, pWorld),
                                         HGF2DLocation(638.000012338438640 , 386.000007674146330, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(638.000012338438640 , 386.000007674146330, pWorld),
                                         HGF2DLocation(638.000012338438640 , 130.000002633843110, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(638.000012338438640 , 130.000002633843110, pWorld),
                                         HGF2DLocation(382.000007298135470 , 130.000002633843110, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(60512.002365251079, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.002580635250, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);   

    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_DOUBLE_EQ(5024.0002156743722, pResult->CalculateArea());   

    }

//==================================================================================
// Test which failed on oct 1, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed15)
    {
   
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(6.65414858785482000 , 204.415769530740250, pWorld),
                                         HGF2DLocation(256.000000000000000 , 204.415769457208940, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000000000000 , 204.415769457208940, pWorld),
                                         HGF2DLocation(256.000000015211980 , 256.000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000015211980 , 256.000000000000000, pWorld),
                                         HGF2DLocation(6.65414860306680100 , 256.000000073531340, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(6.65414860306680100 , 256.000000073531340, pWorld),
                                         HGF2DLocation(6.65414858785482000 , 204.415769530740250, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , 0.00000000000000000, pWorld),
                                         HGF2DLocation(0.00000000000000000 , 256.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , 256.000000000000000, pWorld),
                                         HGF2DLocation(256.000000000000000 , 256.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000000000000 , 256.000000000000000, pWorld),
                                         HGF2DLocation(256.000000000000000 , 0.00000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000000000000 , 0.00000000000000000, pWorld),
                                         HGF2DLocation(0.00000000000000000 , 0.00000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(12862.3138841326217, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.0000000000000, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);    

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_DOUBLE_EQ(52673.686125671731, pResult->CalculateArea());
        
    }

//==================================================================================
// Test which failed on oct 1, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed16)
    {
   
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(243.999999924491790 , -192.000000147448450, pWorld),
                                         HGF2DLocation(-12.000000151016373 , -192.000000071954870, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-12.000000151016373 , -192.000000071954870, pWorld),
                                         HGF2DLocation(-12.000000075522783 , 64.00000000355332500, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-12.000000075522783 , 64.00000000355332500, pWorld),
                                         HGF2DLocation(243.999999999985360 , 63.99999992805972900, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(243.999999999985360 , 63.99999992805972900, pWorld),
                                         HGF2DLocation(243.999999924491790 , -192.000000147448450, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-0.000000000000057 , 0.0000000000000000, pWorld),
                                         HGF2DLocation(-0.000000000000057 , 64.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-0.000000000000057 , 64.000000000000000, pWorld),
                                         HGF2DLocation(63.999999999999943 , 64.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(63.999999999999943 , 64.000000000000000, pWorld),
                                         HGF2DLocation(63.999999999999943 , 0.0000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(63.999999999999943 , 0.0000000000000000, pWorld),
                                         HGF2DLocation(-0.000000000000057 , 0.0000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(4096.00000000000000, pResult->CalculateArea());    

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.0000773128995, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(61440.000040960411, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);    

    }

//==================================================================================
// Test which failed on oct 3, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  GenerateScanLinesTestWhoFailed3)
    {
  
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000000000000 , 38.3819243488055970, pWorld),
                                         HGF2DLocation(194.576081173565060 , 0.00000000000005700, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(194.576081173565060 , 0.00000000000005700, pWorld),
                                         HGF2DLocation(51.8084874008201270 , 0.00000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(51.8084874008201270 , 0.00000000000000000, pWorld),
                                         HGF2DLocation(0.00000000000000000 , 82.9109112849207580, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , 82.9109112849207580, pWorld),
                                         HGF2DLocation(0.00000000000011400 , 180.285041520714510, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000011400 , 180.285041520714510, pWorld),
                                         HGF2DLocation(11.0000000000000000 , 187.158604391717180, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(11.0000000000000000 , 187.158604391717180, pWorld),
                                         HGF2DLocation(11.0000000000000000 , 71.5000000000000850, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(11.0000000000000000 , 71.5000000000000850, pWorld),
                                         HGF2DLocation(223.750000000000110 , 71.5000000000000850, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(223.750000000000110 , 71.5000000000000850, pWorld),
                                         HGF2DLocation(223.750000000000110 , 190.500000000000090, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(223.750000000000110 , 190.500000000000090, pWorld),
                                         HGF2DLocation(16.3473507671213840 , 190.500000000000060, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(16.3473507671213840 , 190.500000000000060, pWorld),
                                         HGF2DLocation(121.169262419310260 , 256.000000000000060, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(121.169262419310260 , 256.000000000000060, pWorld),
                                         HGF2DLocation(193.711604572729020 , 256.000000000000110, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(193.711604572729020 , 256.000000000000110, pWorld),
                                         HGF2DLocation(256.000000000000000 , 156.317730039175560, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000000000000 , 156.317730039175560, pWorld),
                                         HGF2DLocation(256.000000000000000 , 38.3819243488055970, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    // Create shape of same size
    HVEShape  MyShape(*pShape1);

    // Generate scanlines for this dum shape (this insures creation of Y)
    HGFScanLines  TheScanLines(false, pWorld);

    MyShape.GenerateScanLines(TheScanLines);
   
    }

//==================================================================================
// Test which failed on oct 6, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  CalculateSpatialPositionOfNonCrossingLinearTestWhoFailed)
    {

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(200554.736158946010000 , -6398.2045972025953000, pWorld),
                                         HGF2DLocation(196150.683067803180000 , -53764.925311697683000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(196150.683067803180000 , -53764.925311697683000, pWorld),
                                         HGF2DLocation(172671.965121527440000 , -51581.925965582428000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(172671.965121527440000 , -51581.925965582428000, pWorld),
                                         HGF2DLocation(172671.965121527440000 , -6398.2045972025953000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(172671.965121527440000 , -6398.2045972025953000, pWorld),
                                         HGF2DLocation(200554.736158946010000 , -6398.2045972025953000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DSegment    MySegment(HGF2DLocation(200554.736158945980000 , -6398.2045972025953000, pWorld),
                              HGF2DLocation(198455.729354262730000 , -28973.561214771802000 ,pWorld));

    pShape1->CalculateSpatialPositionOfNonCrossingLinear(MySegment);

    }

//==================================================================================
// Test which failed on dec 16, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWhoFailed)
    {
   
    HVE2DRectangle  Shape1(-3579929.5748470003, 85666005.023929002, 797561.58597685, 90043496.184753, pWorld);

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-3220838.5030607, 85666005.023929, pWorld),
                                         HGF2DLocation(-3579929.5748470, 85666005.023929, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-3579929.5748470, 85666005.023929, pWorld),
                                         HGF2DLocation(-3579929.5748470, 90043496.184753, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-3579929.5748470, 90043496.184753, pWorld),
                                         HGF2DLocation(-3220838.5030607, 90043496.184753, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-3220838.5030607, 90043496.184753, pWorld),
                                         HGF2DLocation(-3220838.5030607, 85666005.023929, pWorld)));

    HVE2DPolygonOfSegments  Shape2(TheLinear1);

    HFCPtr<HVE2DShape> pResult = Shape1.IntersectShape(Shape2);
    ASSERT_DOUBLE_EQ(1571917992675.34570, pResult->CalculateArea());   

    pResult = Shape1.DifferentiateShape(Shape2);
    ASSERT_DOUBLE_EQ(17590510870416.2519, pResult->CalculateArea());    

    pResult = Shape1.UnifyShape(Shape2);
    ASSERT_DOUBLE_EQ(19162428863091.5976, pResult->CalculateArea());    
    
    }

//==================================================================================
// Test which failed on dec 19, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWhoFailed2)
    {
         
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-3579929.5748469676, 76911022.702281535, pWorld),
                                         HGF2DLocation(-3579929.5748469676, 81288513.863105357, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-3579929.5748469676, 81288513.863105357, pWorld),
                                         HGF2DLocation(797561.585976853970, 81288513.863105357, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(797561.585976853970, 81288513.863105357, pWorld),
                                         HGF2DLocation(797561.585976853970, 76911022.702281535, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(797561.585976853970, 76911022.702281535, pWorld),
                                         HGF2DLocation(-3579929.5748469676, 76911022.702281535, pWorld)));

    HVE2DPolygonOfSegments  Shape1(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-3220838.5030606692, 76911022.702281520, pWorld),
                                         HGF2DLocation(-3579929.5748469797, 76911022.702281520, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-3579929.5748469797, 76911022.702281520, pWorld),
                                         HGF2DLocation(-3579929.5748469797, 81288513.863105103, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-3579929.5748469797, 81288513.863105103, pWorld),
                                         HGF2DLocation(-3220838.5030606692, 81288513.863105103, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-3220838.5030606692, 81288513.863105103, pWorld),
                                         HGF2DLocation(-3220838.5030606692, 76911022.702281520, pWorld)));

    HVE2DPolygonOfSegments  Shape2(TheLinear2);

    HFCPtr<HVE2DShape> pResult = Shape1.IntersectShape(Shape2);
    ASSERT_DOUBLE_EQ(1571917992675.24121, pResult->CalculateArea());    

    pResult = Shape1.DifferentiateShape(Shape2);
    ASSERT_DOUBLE_EQ(17590510870415.4765, pResult->CalculateArea());    

    pResult = Shape1.UnifyShape(Shape2);
    ASSERT_DOUBLE_EQ(19162428863090.195, pResult->CalculateArea());    
    
    }

//==================================================================================
// Test which failed on Jan 16 1998
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  IntersectShapeTestWhoFailed2)
    {
  
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(343186.275207202940000, 5067813.655207243700000, pWorld),
                                         HGF2DLocation(343186.275207202940000, 5062784.982890859200000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(343186.275207202940000, 5062784.982890859200000, pWorld),
                                         HGF2DLocation(343666.732587660310000, 5062784.982890859200000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(343666.732587660310000, 5062784.982890859200000, pWorld),
                                         HGF2DLocation(343656.275207202940000, 5067814.982890859200000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(343656.275207202940000, 5067814.982890859200000, pWorld),
                                         HGF2DLocation(343656.275206638850000, 5067814.982890858300000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(343656.275206638850000, 5067814.982890858300000, pWorld),
                                         HGF2DLocation(343186.275207202940000, 5067813.655207243700000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(343656.27520720294, 5067814.9828908592, pWorld),
                                         HGF2DLocation(343186.27520720294, 5067813.6552072437, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(343186.27520720294, 5067813.6552072437, pWorld),
                                         HGF2DLocation(343186.27520720294, 5062784.9828908592, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(343186.27520720294, 5062784.9828908592, pWorld),
                                         HGF2DLocation(343666.73258766031, 5062784.9828908592, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(343666.73258766031, 5062784.9828908592, pWorld),
                                         HGF2DLocation(343656.27520720294, 5067814.9828908592, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_DOUBLE_EQ(2390088.30620082330, pResult->CalculateArea());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(2390088.30620074272, pResult->CalculateArea());
    
    }

//==================================================================================
// Test which failed on Jan 19 1998
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed17)
    {
   
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2029951690.000000200000000 , 2069995506.99999980000000, pWorld),
                                         HGF2DLocation(2029909908.805370600000000 , 2070045299.88880250000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2029909908.805370600000000 , 2070045299.88880250000000, pWorld),
                                         HGF2DLocation(2029978852.805251400000000 , 2070103150.77367420000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2029978852.805251400000000 , 2070103150.77367420000000, pWorld),
                                         HGF2DLocation(2030020633.999881000000000 , 2070053357.88487150000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2030020633.999881000000000 , 2070053357.88487150000000, pWorld),
                                         HGF2DLocation(2029951690.000000200000000 , 2069995506.99999980000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2029951690.000000200000000 , 2069995506.99999980000000, pWorld),
                                         HGF2DLocation(2029945974.905895500000000 , 2070002317.98393110000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2029945974.905895500000000 , 2070002317.98393110000000, pWorld),
                                         HGF2DLocation(2029945974.905895500000000 , 2070031573.10052470000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2029945974.905895500000000 , 2070031573.10052470000000, pWorld),
                                         HGF2DLocation(2029982041.006420400000000 , 2070031573.10052470000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2029982041.006420400000000 , 2070031573.10052470000000, pWorld),
                                         HGF2DLocation(2029982041.006420400000000 , 2070020974.51829270000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2029982041.006420400000000 , 2070020974.51829270000000, pWorld),
                                         HGF2DLocation(2029951690.000000200000000 , 2069995506.99999980000000, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_DOUBLE_EQ(894818494.406250000, pResult->CalculateArea());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(894818494.406250000, pResult->CalculateArea());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_DOUBLE_EQ(5850000000.00000000, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(5850000000.02148437, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(4955181505.60937500, pResult->CalculateArea());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(894818494.406250000, pResult->CalculateArea());    
    
    }

//==================================================================================
// Test which failed on Jan 20 1998
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed18)
    {
   
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000 , -1.490116119384800E-8, pWorld),
                                         HGF2DLocation(0.000 , 255.99999998509884010, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000 , 255.99999998509884010, pWorld),
                                         HGF2DLocation(256.0 , 255.99999998509884010, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 255.99999998509884010, pWorld),
                                         HGF2DLocation(256.0 , -1.490116119384800E-8, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , -1.490116119384800E-8, pWorld),
                                         HGF2DLocation(0.000 , -1.490116119384800E-8 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation( -1.4901161193848E-8, 237.11464065313000000, pWorld), 
                                         HGF2DLocation( -1.4901161193848E-8, 255.99999997020000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation( -1.4901161193848E-8, 255.99999997020000000, pWorld),
                                         HGF2DLocation( 256.000000000000000, 255.99999998509884010, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation( 256.000000000000000, 255.99999998509884010, pWorld),
                                         HGF2DLocation( 256.000000000000000, -1.490116119384800E-8, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation( 256.000000000000000, -1.490116119384800E-8, pWorld),
                                         HGF2DLocation( 198.962807521220000, -1.490116119384800E-8, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation( 198.962807521220000, -1.490116119384800E-8, pWorld),
                                         HGF2DLocation( -1.4901161193848E-8, 237.11464065313000000, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_DOUBLE_EQ(41947.502694292649, pResult->CalculateArea());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(41947.502694292649, pResult->CalculateArea());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_DOUBLE_EQ(65536.000000000000, pResult->CalculateArea());    

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.000003814697, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(23588.497305848363, pResult->CalculateArea());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(41947.502694292649, pResult->CalculateArea());
    
    }

//==================================================================================
// Test which failed on Jan 20 1998
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed19)
    {
    
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000, 0.000000000000000000, pWorld),
                                         HGF2DLocation(-1.4901161193848E-8, 256.0000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-1.4901161193848E-8, 256.0000000000000000, pWorld),
                                         HGF2DLocation(256.000000000000000, 256.0000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000000000000, 256.0000000000000000, pWorld),
                                         HGF2DLocation(256.000000000000000,  -1.4901161193848E-8, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000000000000,  -1.4901161193848E-8, pWorld),
                                         HGF2DLocation(0.00000000000000000, 0.000000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation( 0.0, 0.0 , pWorld), HGF2DLocation( 0.0, 256.0 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation( 0.0, 256.0 , pWorld), HGF2DLocation( 256.0, 256.0, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation( 256.0, 256.0, pWorld), HGF2DLocation( 256.0, 0.0, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation( 256.0, 0.0, pWorld), HGF2DLocation( 0.0, 0.0, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_DOUBLE_EQ(65536.000007629395, pResult->CalculateArea());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.000000000000, pResult->CalculateArea());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_DOUBLE_EQ(65536.0000038146972, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.0000038146972, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_TRUE(pResult->IsEmpty());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.0000000000000, pResult->CalculateArea());
    
    }

//==================================================================================
// Test which failed on Jan 20 1998
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed20)
    {
   
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-1.490116119384800E-8, -1.490116119384800E-8, pWorld),
                                         HGF2DLocation(-1.490116119384800E-8, 255.99999998509884010, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-1.490116119384800E-8, 255.99999998509884010, pWorld),
                                         HGF2DLocation(255.99999998509884010, 256.00000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(255.99999998509884010, 256.00000000000000000, pWorld),
                                         HGF2DLocation(255.99999998509884010, 0.0000000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(255.99999998509884010, 0.0000000000000000000, pWorld),
                                         HGF2DLocation(-1.490116119384800E-8, -1.490116119384800E-8, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-1.490127488068500E-8, -2.2737367544323E-13 , pWorld),
                                         HGF2DLocation(-1.490127488068500E-8, 256.00000000000000000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-1.490127488068500E-8, 256.00000000000000000 , pWorld),
                                        HGF2DLocation(255.999999985098840100, 256.00000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(255.99999998509884010, 256.00000000000000000, pWorld),
                                         HGF2DLocation(255.99999998509884010, 0.0000000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(255.99999998509884010, 0.0000000000000000000, pWorld),
                                         HGF2DLocation(-1.490127488068500E-8, -2.27373675443230E-13, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_DOUBLE_EQ(65536.000003814697, pResult->CalculateArea());    

    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.000000000087, pResult->CalculateArea());    

    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_DOUBLE_EQ(65536.0000000000000, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.0000000000000, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_TRUE(pResult->IsEmpty());    

    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_TRUE(pResult->IsEmpty());
    
    }

//==================================================================================
// Test which failed on Jan 20 1998
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed21)
    {
   
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000, -9.313225746154800E-9, pWorld),
                                         HGF2DLocation(0.0000000000000, 255.99999998509884010, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000, 255.99999998509884010, pWorld),
                                         HGF2DLocation(63.277044698596, 255.99999999255000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(63.277044698596, 255.99999999255000000, pWorld),
                                         HGF2DLocation(90.183728918433, -3.725290298461900E-9, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(90.183728918433, -3.725290298461900E-9, pWorld),
                                         HGF2DLocation(0.0000000000000, -9.313225746154800E-9, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(0.0, 256.0, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 256.0 , pWorld), HGF2DLocation(256.0, 256.0, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.0, 256.0, pWorld), HGF2DLocation(256.0, 0.0, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.0, 0.0, pWorld), HGF2DLocation(0.0, 0.0, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_DOUBLE_EQ(19642.9790227101402, pResult->CalculateArea());  

    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(19642.9790227101402, pResult->CalculateArea());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_DOUBLE_EQ(65536.000000238491, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.000000000000, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_DOUBLE_EQ(45893.0209767615961, pResult->CalculateArea());   

    }

//==================================================================================
// Test which failed on Jan 21 1998
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed22)
    {
  
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2029947377.3103, 2070045465.0066, pWorld),
                                         HGF2DLocation(2029947377.3103, 2070057954.5083, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2029947377.3103, 2070057954.5083, pWorld),
                                         HGF2DLocation(2029959866.8120, 2070057954.5083, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2029959866.8120, 2070057954.5083, pWorld),
                                         HGF2DLocation(2029959866.8120, 2070045465.0066, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2029959866.8120, 2070045465.0066, pWorld),
                                         HGF2DLocation(2029947377.3103, 2070045465.0066, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2029947377.3103, 2070045465.0066 , pWorld),
                                         HGF2DLocation(2029947377.3103, 2070057954.5083 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2029947377.3103, 2070057954.5083 , pWorld),
                                         HGF2DLocation(2029959866.8120, 2070057954.5083 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2029959866.8120, 2070057954.5083 , pWorld),
                                         HGF2DLocation(2029959866.8120, 2070045465.0066 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2029959866.8120, 2070045465.0066 , pWorld),
                                         HGF2DLocation(2029947377.3103, 2070045465.0066 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());

    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(155987652.715393930, pResult->CalculateArea());   

    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_DOUBLE_EQ(155987652.71484375, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(155987652.71484375, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);  

    }

//==================================================================================
// Test which failed on aug 26, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  RasterizeTestWhoFailed)
    {
           
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000000000000 , 85.607206146931276, pWorld),
                                         HGF2DLocation(153.057488331542120 , 149.59849718410987, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(153.057488331542120 , 149.59849718410987, pWorld),
                                         HGF2DLocation(111.724160724152170 , 84.732326121302322, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(111.724160724152170 , 84.732326121302322, pWorld),
                                         HGF2DLocation(98.4837432157219150 , 94.674092901172116, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(98.4837432157219150 , 94.674092901172116, pWorld),
                                         HGF2DLocation(178.561339551983110 , 222.57580927154049, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(178.561339551983110 , 222.57580927154049, pWorld),
                                         HGF2DLocation(256.000000000000000 , 173.54535306419712, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000000000000 , 173.54535306419712, pWorld),
                                         HGF2DLocation(256.000000000000000 , 85.607206146931276, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    // Create shape of same size
    HVEShape  MyShape(pShape1->GetExtent());

    // Generate scanlines for this dum shape (this insures creation of Y)
    HGFScanLines  TheScanLines(false, pWorld);

    pShape1->Rasterize(TheScanLines);

    }

//==================================================================================
// Test which failed on Feb 09 1998
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed23)
    {
    
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(15282.044160147476000 , -10973.717432186515000 , pWorld),
                                         HGF2DLocation(28082.024664637727000 , -10996.057635270005000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(28082.024664637727000 , -10996.057635270005000 , pWorld),
                                         HGF2DLocation(28104.364867721215000 , 1803.92286922024500000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(28104.364867721215000 , 1803.92286922024500000 , pWorld),
                                         HGF2DLocation(15304.384363230965000 , 1826.26307230373460000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(15304.384363230965000 , 1826.26307230373460000 , pWorld),
                                         HGF2DLocation(15282.044160147476000 , -10973.717432186515000 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(15282.044160147572000 , -11041.43617278322900 , pWorld),
                                         HGF2DLocation(15282.044160147572000 , 14603.205242364347000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(15282.044160147572000 , 14603.205242364347000 , pWorld),
                                         HGF2DLocation(40926.685575295152000 , 14603.205242364347000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(40926.685575295152000 , 14603.205242364347000 , pWorld),
                                         HGF2DLocation(40926.685575295152000 , -11041.43617278322900 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(40926.685575295152000 , -11041.43617278322900 , pWorld),
                                         HGF2DLocation(15282.044160147572000 , -11041.43617278322900 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());    

    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(163840000.000004291, pResult->CalculateArea());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_DOUBLE_EQ(657647633.311502456, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(657647633.311502456, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_DOUBLE_EQ(493807633.311498165, pResult->CalculateArea());
    
    }

//==================================================================================
// Test which failed on August 18 1998
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed25)
    {
   
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-4.8885340220295E-12 , 0.0000000000000 , pWorld), 
                                         HGF2DLocation(110.8848526511200000 , 63.947435208261 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(110.8848526511200000 , 63.947435208261 , pWorld),
                                         HGF2DLocation(-2.8421709430404E-14 , 256.00000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-2.8421709430404E-14 , 256.00000000000 , pWorld),
                                         HGF2DLocation(-256.000000000000000 , 256.00000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-256.000000000000000 , 256.00000000000 , pWorld),
                                         HGF2DLocation(-256.000000000000000 , 0.0000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-256.000000000000000 , 0.0000000000000 , pWorld),
                                         HGF2DLocation(-4.8885340220295E-12 , 0.0000000000000 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000, 0.000, pWorld), HGF2DLocation(0.000, 256.0, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000, 256.0, pWorld), HGF2DLocation(128.0, 256.0, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(128.0, 256.0, pWorld), HGF2DLocation(128.0, 0.000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(128.0, 0.000, pWorld), HGF2DLocation(0.000, 0.000, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());    

    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(14193.2611393438291, pResult->CalculateArea());   

    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_DOUBLE_EQ(98304.0000000000000, pResult->CalculateArea());    

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(98304.0000000000000, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(65535.9999999999927, pResult->CalculateArea());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_DOUBLE_EQ(18574.7388606566419, pResult->CalculateArea());    

    }

//==================================================================================
// Test which failed on October 7 1998
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed26)
    {
    
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.597254913127000 , 36528.9980348294920000 , pWorld),
                                         HGF2DLocation(-10362.597254332386000 , 36528.9980355215960000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.597254332386000 , 36528.9980355215960000 , pWorld),
                                         HGF2DLocation(-10362.597254999999000 , 36528.9980349999970000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.597254999999000 , 36528.9980349999970000 , pWorld),
                                         HGF2DLocation(-65772.130034309317000 , 107449.965859604910000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-65772.130034309317000 , 107449.965859604910000 , pWorld),
                                         HGF2DLocation(-14551.431049872443000 , 147467.961755772760000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-14551.431049872443000 , 147467.961755772760000 , pWorld),
                                         HGF2DLocation(36652.0408752940860000 , 81930.5063259228050000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(36652.0408752940860000 , 81930.5063259228050000 , pWorld),
                                         HGF2DLocation(100362.597255166530000 , 28471.0019654725110000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(100362.597255166530000 , 28471.0019654725110000 , pWorld),
                                         HGF2DLocation(58581.4026255414790000 , -21321.886837261045000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(58581.4026255414790000 , -21321.886837261045000 , pWorld),
                                         HGF2DLocation(-10362.597254455088000 , 36528.9980339305330000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.597254455088000 , 36528.9980339305330000 , pWorld),
                                         HGF2DLocation(30496.5477215592730000 , -43661.589141953082000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(30496.5477215592730000 , -43661.589141953082000 , pWorld),
                                         HGF2DLocation(-27418.876350684626000 , -73170.971625023667000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-27418.876350684626000 , -73170.971625023667000, pWorld),
                                         HGF2DLocation(-68278.021327243885000 , 7019.61555192941520000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-68278.021327243885000 , 7019.61555192941520000, pWorld),
                                         HGF2DLocation(-10362.597255000004000 , 36528.9980349999970000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.597255000004000 , 36528.9980349999970000, pWorld),
                                         HGF2DLocation(-10362.597254913127000 , 36528.9980348294920000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-87507.654318190165000 , 82882.424776904780000, pWorld),
                                         HGF2DLocation(-10362.597254999990000 , 36528.998035000004000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.597254999990000 , 36528.998035000004000, pWorld),
                                         HGF2DLocation(-68278.021327243885000 , 7019.6155519294152000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-68278.021327243885000 , 7019.6155519294152000, pWorld),
                                         HGF2DLocation(-59816.342063631593000 , -9587.365065051119000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-59816.342063631593000 , -9587.365065051119000, pWorld),
                                         HGF2DLocation(-120985.12918734361000 , 27166.550231267436000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-120985.12918734361000 , 27166.550231267436000, pWorld),
                                         HGF2DLocation(-87507.654318190165000 , 82882.424776904780000, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_TRUE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_TRUE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_DOUBLE_EQ(21744435351.1862068, pResult->CalculateArea());    

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(21744435351.1862068, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(17105934606.0171737, pResult->CalculateArea());    

    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_DOUBLE_EQ(4638500745.16903495, pResult->CalculateArea());
    
    }

//==================================================================================
// Another Test which failed on October 7 1998
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed27) 
    {

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.597255000001000 , 36528.998034999997000 , pWorld),
                                         HGF2DLocation(-97689.212619839687000 , 58301.968638970102000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-97689.212619839687000 , 58301.968638970102000 , pWorld),
                                         HGF2DLocation(-81964.289405861273000 , 121371.19084690989000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-81964.289405861273000 , 121371.19084690989000 , pWorld),
                                         HGF2DLocation(5362.32595897841020000 , 99598.220242939773000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(5362.32595897841020000 , 99598.220242939773000 , pWorld),
                                         HGF2DLocation(-10362.597255000001000 , 36528.998034999997000 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.597254836835000 , 36528.998034920412000 , pWorld),
                                         HGF2DLocation(-10362.597254332386000 , 36528.998035521596000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.597254332386000 , 36528.998035521596000 , pWorld),
                                         HGF2DLocation(-10362.597254999999000 , 36528.998034999997000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.597254999999000 , 36528.998034999997000 , pWorld),
                                         HGF2DLocation(-65772.130034309317000 , 107449.96585960491000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-65772.130034309317000 , 107449.96585960491000 , pWorld),
                                         HGF2DLocation(-14551.431049872443000 , 147467.96175577276000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-14551.431049872443000 , 147467.96175577276000 , pWorld),
                                         HGF2DLocation(36652.0408752940860000 , 81930.506325922805000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(36652.0408752940860000 , 81930.506325922805000 , pWorld),
                                         HGF2DLocation(100362.597255166530000 , 28471.001965472511000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(100362.597255166530000 , 28471.001965472511000 , pWorld),
                                         HGF2DLocation(65684.5981222158590000 , -12856.62808520035500 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(65684.5981222158590000 , -12856.62808520035500 , pWorld),
                                         HGF2DLocation(42034.7423706349950000 , -61346.01818546281700 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(42034.7423706349950000 , -61346.01818546281700 , pWorld),
                                         HGF2DLocation(18406.5218307773900000 , -49821.76501835301300 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(18406.5218307773900000 , -49821.76501835301300 , pWorld),
                                         HGF2DLocation(-27418.876350684626000 , -73170.97162502366700 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-27418.876350684626000 , -73170.97162502366700 , pWorld),
                                         HGF2DLocation(-59816.342063631593000 , -9587.365065051119000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-59816.342063631593000 , -9587.365065051119000 , pWorld),
                                         HGF2DLocation(-120985.12918734361000 , 27166.550231267436000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-120985.12918734361000 , 27166.550231267436000 , pWorld),
                                         HGF2DLocation(-87507.654318190165000 , 82882.424776904780000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-87507.654318190165000 , 82882.424776904780000 , pWorld),
                                         HGF2DLocation(-10362.597254999990000 , 36528.998035000004000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.597254999990000 , 36528.998035000004000 , pWorld),
                                         HGF2DLocation(-10362.597254836835000 , 36528.998034920412000 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(3819636995.75487518, pResult->CalculateArea());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_DOUBLE_EQ(25949026645.6181907, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(25949026645.6181907, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(2030363004.24512672, pResult->CalculateArea());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_DOUBLE_EQ(20099026645.616497, pResult->CalculateArea());    

    }

//==================================================================================
// Test which failed on January 4 1999
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed28)
    {

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(251.3389155211043500 , 1878.728020202368500 , pWorld),
                                         HGF2DLocation(0.000000000000000000 , 2306.538940237835000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000000 , 2306.538940237835000 , pWorld),
                                         HGF2DLocation(556.2084000000031700 , 2619.805740238167300 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(556.2084000000031700 , 2619.805740238167300 , pWorld),
                                         HGF2DLocation(597.7641999999759700 , 2533.497540238313400 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(597.7641999999759700 , 2533.497540238313400 , pWorld),
                                         HGF2DLocation(1157.169200000003900 , 2942.662340237759100 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1157.169200000003900 , 2942.662340237759100 , pWorld),
                                         HGF2DLocation(1360.495711533061700 , 2597.007270631380400 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1360.495711533061700 , 2597.007270631380400 , pWorld),
                                         HGF2DLocation(1363.329634746885900 , 2598.842491989955300 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1363.329634746885900 , 2598.842491989955300 , pWorld),
                                         HGF2DLocation(1736.640594746917500 , 2032.088579989969700 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1736.640594746917500 , 2032.088579989969700 , pWorld),
                                         HGF2DLocation(1709.411828939744700 , 1793.080524571239900 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1709.411828939744700 , 1793.080524571239900 , pWorld),
                                         HGF2DLocation(1945.621632601774800 , 1445.451002201065400 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1945.621632601774800 , 1445.451002201065400 , pWorld),
                                         HGF2DLocation(1695.900754101807300 , 1287.339685107581300 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1695.900754101807300 , 1287.339685107581300 , pWorld),
                                         HGF2DLocation(1697.170099164475700 , 1281.223749805241800 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1697.170099164475700 , 1281.223749805241800 , pWorld),
                                         HGF2DLocation(1737.691559314960600 , 1230.336334732361100 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1737.691559314960600 , 1230.336334732361100 , pWorld),
                                         HGF2DLocation(1973.305540954985200 , 1395.165000000037300 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1973.305540954985200 , 1395.165000000037300 , pWorld),
                                         HGF2DLocation(2247.280540955020100 , 986.3100000005215400 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2247.280540955020100 , 986.3100000005215400 , pWorld),
                                         HGF2DLocation(2179.840540955017800 , 927.3000000007450600 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2179.840540955017800 , 927.3000000007450600 , pWorld),
                                         HGF2DLocation(2382.160540955024800 , 644.8950000004842900 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2382.160540955024800 , 644.8950000004842900 , pWorld),
                                         HGF2DLocation(1269.400540955015500 , 0.000000000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1269.400540955015500 , 0.000000000000000000 , pWorld),
                                         HGF2DLocation(797.3205409549991600 , 708.1200000001117600 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(797.3205409549991600 , 708.1200000001117600 , pWorld),
                                         HGF2DLocation(917.4365554355317700 , 794.4533854080364100 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(917.4365554355317700 , 794.4533854080364100 , pWorld),
                                         HGF2DLocation(839.4136326017906000 , 745.0530022010207200 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(839.4136326017906000 , 745.0530022010207200 , pWorld),
                                         HGF2DLocation(406.5496326017892000 , 1436.433002200909000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(406.5496326017892000 , 1436.433002200909000 , pWorld),
                                         HGF2DLocation(478.1214622594416100 , 1490.173787791281900 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(478.1214622594416100 , 1490.173787791281900 , pWorld),
                                         HGF2DLocation(236.6092827469110500 , 1869.189251990057500 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(236.6092827469110500 , 1869.189251990057500 , pWorld),
                                         HGF2DLocation(251.3389155211043500 , 1878.728020202368500 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(251.3389155211043500 , 1878.728020202368500 , pWorld),
                                         HGF2DLocation(941.8916180721134900 , 789.1439447365701200 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(941.8916180721134900 , 789.1439447365701200 , pWorld),
                                         HGF2DLocation(990.3438919325126300 , 826.1438629571348400 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(990.3438919325126300 , 826.1438629571348400 , pWorld),
                                         HGF2DLocation(986.1693580946885000 , 837.9717088313773300 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(986.1693580946885000 , 837.9717088313773300 , pWorld),
                                         HGF2DLocation(932.7557957404642400 , 804.1527957096695900 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(932.7557957404642400 , 804.1527957096695900 , pWorld),
                                         HGF2DLocation(251.3389155211043500 , 1878.728020202368500 , pWorld)));


    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000, 0.0000, pWorld), HGF2DLocation(0.0000, 2942.0, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000, 2942.0, pWorld), HGF2DLocation(2382.0, 2942.0, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2382.0, 2942.0, pWorld), HGF2DLocation(2382.0, 0.0000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2382.0, 0.0000, pWorld), HGF2DLocation(0.0000, 0.0000, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(3447172.45511193666, pResult->CalculateArea());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(3447172.45511193666, pResult->CalculateArea());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_DOUBLE_EQ(7007844.45437281206, pResult->CalculateArea());    

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(7007844.45437281206, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(0.454372813101429074, pResult->CalculateArea());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_DOUBLE_EQ(3560671.54488806240, pResult->CalculateArea());
       
    }

//==================================================================================
// Test which failed on January 8 1999
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed29)
    {
    
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.597254836834000 , 36528.998034920412000 , pWorld),
                                         HGF2DLocation(-10362.597254332393000 , 36528.998035521588000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.597254332393000 , 36528.998035521588000 , pWorld),
                                         HGF2DLocation(-10362.597255000001000 , 36528.998034999997000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.597255000001000 , 36528.998034999997000 , pWorld),
                                         HGF2DLocation(-65772.130034309332000 , 107449.96585960491000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-65772.130034309332000 , 107449.96585960491000 , pWorld),
                                         HGF2DLocation(-14551.431049872444000 , 147467.96175577276000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-14551.431049872444000 , 147467.96175577276000 , pWorld),
                                         HGF2DLocation(36652.0408752940640000 , 81930.506325922819000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(36652.0408752940640000 , 81930.506325922819000 , pWorld),
                                         HGF2DLocation(100362.597255166530000 , 28471.001965472511000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(100362.597255166530000 , 28471.001965472511000 , pWorld),
                                         HGF2DLocation(65684.5981222158590000 , -12856.62808520035500 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(65684.5981222158590000 , -12856.62808520035500 , pWorld),
                                         HGF2DLocation(42034.7423706349950000 , -61346.01818546281700 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(42034.7423706349950000 , -61346.01818546281700 , pWorld),
                                         HGF2DLocation(18406.5218307773830000 , -49821.76501835301300 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(18406.5218307773830000 , -49821.76501835301300 , pWorld),
                                         HGF2DLocation(-27418.876350684630000 , -73170.97162502366700 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-27418.876350684630000 , -73170.97162502366700 , pWorld),
                                         HGF2DLocation(-59816.342063631586000 , -9587.365065051140800 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-59816.342063631586000 , -9587.365065051140800 , pWorld),
                                         HGF2DLocation(-120985.12918734361000 , 27166.550231267429000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-120985.12918734361000 , 27166.550231267429000 , pWorld),
                                         HGF2DLocation(-87507.654318190165000 , 82882.424776904780000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-87507.654318190165000 , 82882.424776904780000 , pWorld),
                                         HGF2DLocation(-10362.597255000001000 , 36528.998034999997000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.597255000001000 , 36528.998034999997000 , pWorld),
                                         HGF2DLocation(-10362.597254836834000 , 36528.998034920412000 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-94435.220360731255000 , 71353.018749675452000 , pWorld),
                                         HGF2DLocation(-81964.289405861273000 , 121371.19084690987000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-81964.289405861273000 , 121371.19084690987000 , pWorld),
                                         HGF2DLocation(-56181.662813618503000 , 114942.86005061449000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-56181.662813618503000 , 114942.86005061449000 , pWorld),
                                         HGF2DLocation(-65772.130034309332000 , 107449.96585960491000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-65772.130034309332000 , 107449.96585960491000 , pWorld),
                                         HGF2DLocation(-10362.597255000001000 , 36528.998034999997000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.597255000001000 , 36528.998034999997000 , pWorld),
                                         HGF2DLocation(-87507.654318190165000 , 82882.424776904780000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-87507.654318190165000 , 82882.424776904780000 , pWorld),
                                         HGF2DLocation(-94435.220360731255000 , 71353.018749675452000 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_TRUE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_TRUE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_DOUBLE_EQ(25949026645.6181945, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(25949026645.6181945, pResult->CalculateArea());   

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(23918663641.3730659, pResult->CalculateArea());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_DOUBLE_EQ(2030363004.24512577, pResult->CalculateArea());
    
    }

//==================================================================================
// Test which failed on May 18 1999
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed30)
    {
           
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1397168087.498626900000000 , -1597261746.348763900000000 , pWorld),
                                         HGF2DLocation(1396690582.511111700000000 , -1630473344.953076400000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1396690582.511111700000000 , -1630473344.953076400000000 , pWorld),
                                         HGF2DLocation(1427712484.592282800000000 , -1630473344.953076400000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1427712484.592282800000000 , -1630473344.953076400000000 , pWorld),
                                         HGF2DLocation(1427712484.592282800000000 , -1597700911.289259000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1427712484.592282800000000 , -1597700911.289259000000000 , pWorld),
                                         HGF2DLocation(1397168087.498626900000000 , -1597261746.348763900000000 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1397168087.498626700000000 , -1597261746.348764200000000 , pWorld),
                                         HGF2DLocation(1394500885.987970100000000 , -1782771904.954012900000000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1394500885.987970100000000 , -1782771904.954012900000000 , pWorld),
                                         HGF2DLocation(1557455317.014201400000000 , -1785114850.954406700000000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1557455317.014201400000000 , -1785114850.954406700000000 , pWorld),
                                         HGF2DLocation(1560122518.524858000000000 , -1599604692.349158000000000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1560122518.524858000000000 , -1599604692.349158000000000 , pWorld),
                                         HGF2DLocation(1397168087.498626700000000 , -1597261746.348764200000000 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(+1.01565059370763200E+15, pResult->CalculateArea());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_DOUBLE_EQ(3.0235951454215872E+16, pResult->CalculateArea());  

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(3.02359514542158560E+16, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_DOUBLE_EQ(2.92203008605082040E+16, pResult->CalculateArea());
    
    }

//==================================================================================
// Test which failed on May 19 1999
// 3 april 2000. We do not support autocontiguous polygons
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed31)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(444.110497237569010 , 222.055248618784530 , pWorld),
                                         HGF2DLocation(444.110497237569010 , 444.110497237569060 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(444.110497237569010 , 444.110497237569060 , pWorld),
                                         HGF2DLocation(666.165745856353510 , 444.110497237569060 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(666.165745856353510 , 444.110497237569060 , pWorld),
                                         HGF2DLocation(666.165745856353510 , 222.055248618784530 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(666.165745856353510 , 222.055248618784530 , pWorld),
                                         HGF2DLocation(444.110497237569010 , 222.055248618784530 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(639.999999999999770 , 222.055248618784530 , pWorld),
                                         HGF2DLocation(639.999999999999770 , 256.000000000000000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(639.999999999999770 , 256.000000000000000 , pWorld),
                                         HGF2DLocation(640.000000000000000 , 256.000000000000000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(640.000000000000000 , 256.000000000000000 , pWorld),
                                         HGF2DLocation(640.000000000000000 , 222.055248618784530 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(640.000000000000000 , 222.055248618784530 , pWorld),
                                         HGF2DLocation(639.999999999999770 , 222.055248618784530 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_TRUE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_TRUE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(49308.5334391502110, pResult->CalculateArea());   

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(49308.5334391502110, pResult->CalculateArea());   

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(49308.5334391502110, pResult->CalculateArea());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_TRUE(pResult->IsEmpty());   
    
    }

//==================================================================================
// Test which failed on June 9 1999
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed32)
    {    
    
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(34844.116927999996000 , 44491.157767857141000 , pWorld),
                                         HGF2DLocation(41887.133631999997000 , 44491.157767857141000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(41887.133631999997000 , 44491.157767857141000 , pWorld),
                                         HGF2DLocation(41887.133631999990000 , -2278.875032142859700 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(41887.133631999990000 , -2278.875032142859700 , pWorld),
                                         HGF2DLocation(34844.116927999996000 , -2278.875032142859700 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(34844.116927999996000 , -2278.875032142859700 , pWorld),
                                         HGF2DLocation(34844.116927999996000 , 44491.157767857141000 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(24059.49759999998400 , 26663.521735845723000 , pWorld),
                                         HGF2DLocation(6231.861567988563400 , 44491.157767857141000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(6231.861567988563400 , 44491.157767857141000 , pWorld),
                                         HGF2DLocation(41887.13363201139900 , 44491.157767857134000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(41887.13363201139900 , 44491.157767857134000 , pWorld),
                                         HGF2DLocation(24059.49759999998400 , 26663.521735845723000 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(24802042.146491855, pResult->CalculateArea());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(24802042.146491855, pResult->CalculateArea());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(622424686.60040784, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(622424686.60040784, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(304600080.11057603, pResult->CalculateArea());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(293022564.3433800, pResult->CalculateArea());
       
    }

//==================================================================================
// Test which failed on June 14 1999
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed33)
    {
         
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000, 0.00 , pWorld), HGF2DLocation(10.01, 0.10 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(10.01, 0.10 , pWorld), HGF2DLocation(10.02, 10.2 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(10.02, 10.2 , pWorld), HGF2DLocation(-0.10, 10.1 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-0.10, 10.1 , pWorld), HGF2DLocation(0.000, 0.00 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HFCPtr<HVE2DShape> pShape2 = new HVE2DRectangle(5.0, -1.0, 5.00000000001, 11.0, pWorld);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_TRUE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_TRUE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(101.660999999999987, pResult->CalculateArea());   

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(101.660999999999987, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(101.660999999999987, pResult->CalculateArea());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_TRUE(pResult->IsEmpty());
        
    }

//==================================================================================
// Test which failed on Sept 21, 1999
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed34)
    {
          
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000, 0.000 , pWorld), HGF2DLocation(0.000, 256.0 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000, 256.0 , pWorld), HGF2DLocation(256.0, 256.0 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0, 256.0 , pWorld), HGF2DLocation(256.0, 0.000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0, 0.000 , pWorld), HGF2DLocation(0.000, 0.000 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(255.99999998156, 166.49374991798 , pWorld),
                                         HGF2DLocation(343.31683259336, 363.58989382296 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(343.31683259336, 363.58989382296 , pWorld),
                                         HGF2DLocation(511.00000000000, 304.00000000000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(511.00000000000, 304.00000000000 , pWorld),
                                         HGF2DLocation(424.68316736055, 107.58989382327 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(424.68316736055, 107.58989382327 , pWorld),
                                         HGF2DLocation(255.99999998156, 166.49374991798 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);    

    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON); 
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(103770.189790698088, pResult->CalculateArea());    

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(103770.189790698088, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(65536.0000000000000, pResult->CalculateArea());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(38234.1897906980884, pResult->CalculateArea());   
    
    }

//==================================================================================
// Test which failed on Oct 14, 1999
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed35)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 256.000000000000000 , pWorld),
                                         HGF2DLocation(0.000739457713053 , 256.000000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000739457713053 , 256.000000000000000 , pWorld),
                                         HGF2DLocation(0.000000000000000 , 255.692716162332320 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 255.692716162332320 , pWorld),
                                         HGF2DLocation(0.000000000000000 , 256.000000000000000 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-255.385432279390440 , 256.307283860515550 , pWorld),
                                         HGF2DLocation(-254.770864558780860 , 511.692716140115580 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-254.770864558780860 , 511.692716140115580 , pWorld),
                                         HGF2DLocation(0.614567720555641000 , 511.078148419505400 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.614567720555641000 , 511.078148419505400 , pWorld),
                                         HGF2DLocation(-0.00000000005394400 , 255.692716139905340 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-0.00000000005394400 , 255.692716139905340 , pWorld),
                                         HGF2DLocation(-255.385432279390440 , 256.307283860515550 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(1.13611701929947000E-4, pResult->CalculateArea());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(1.13611701929947000E-4, pResult->CalculateArea());    

    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(65222.09671119030800, pResult->CalculateArea());   

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(65222.0967140540888, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_TRUE(pResult->IsEmpty());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(65222.096597578609, pResult->CalculateArea());   
    
    }

//==================================================================================
// Test which failed on April 3, 2000
//==================================================================================  
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed36)
    {
    
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000000 , 51.046683371067047 , pWorld),
                                         HGF2DLocation(14.514608025550842 , 42.666670471429825 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(14.514608025550842 , 42.666670471429825 , pWorld),
                                         HGF2DLocation(0.0000000000000000 , 34.286657571792603 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000000 , 34.286657571792603 , pWorld),
                                         HGF2DLocation(0.0000000000000000 , 51.046683371067047 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(51.465018710229799 , 64.000000000000000 , pWorld),
                                         HGF2DLocation(0.0000000000000000 , 34.286657594899594 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000000 , 34.286657594899594 , pWorld),
                                         HGF2DLocation(0.0000000000000000 , 64.000000000000000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000000 , 64.000000000000000 , pWorld),
                                         HGF2DLocation(51.465018710229799 , 64.000000000000000 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(121.63260248729401, pResult->CalculateArea());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(121.63260248729401, pResult->CalculateArea());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(764.598862005579348, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(764.59886141097843, pResult->CalculateArea());  

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);    

    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(642.96625841958667, pResult->CalculateArea());   
    
    }

//==================================================================================
// Test which failed on April 3, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed37)
    {
  
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(59.405016660690308 , 11.333375036716461 , pWorld),
                                         HGF2DLocation(64.000000000000000 , 8.6804601848125460 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(64.000000000000000 , 8.6804601848125460 , pWorld),
                                         HGF2DLocation(64.000000000000000 , 0.0000000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(64.000000000000000 , 0.0000000000000000 , pWorld),
                                         HGF2DLocation(39.775035202503204 , 0.0000000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(39.775035202503204 , 0.0000000000000000 , pWorld),
                                         HGF2DLocation(59.405016660690308 , 11.333375036716461 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(39.775035227748781 , 0.0000000000000000 , pWorld),
                                         HGF2DLocation(64.000000000000000 , 13.986289931850713 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(64.000000000000000 , 13.986289931850713 , pWorld),
                                         HGF2DLocation(64.000000000000000 , 0.0000000000000000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(64.000000000000000 , 0.0000000000000000 , pWorld),
                                         HGF2DLocation(39.775035227748781 , 0.0000000000000000 , pWorld)));

    HFCPtr<HVE2DShape>pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(157.21859061402006, pResult->CalculateArea());    

    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(157.21859061402006, pResult->CalculateArea());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(169.40869044678772, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(169.40869044678772, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);   

    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(12.1900996444270788, pResult->CalculateArea());   

    }

//==================================================================================
// Test which failed on April 3, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed38)
    {
           
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000000 , 41.672118563598019 , pWorld),
                                         HGF2DLocation(64.000000000000000 , 4.7217013355800860 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(64.000000000000000 , 4.7217013355800860 , pWorld),
                                         HGF2DLocation(64.000000000000000 , 0.0000000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(64.000000000000000 , 0.0000000000000000 , pWorld),
                                         HGF2DLocation(0.0000000000000000 , 0.0000000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000000 , 0.0000000000000000 , pWorld),
                                         HGF2DLocation(0.0000000000000000 , 41.672118563598019 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(12.133750259876251 , 34.666694611310959 , pWorld),
                                         HGF2DLocation(0.0000000000000000 , 27.661270588636398 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000000 , 27.661270588636398 , pWorld),
                                         HGF2DLocation(0.0000000000000000 , 41.672118544578552 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000000 , 41.672118544578552 , pWorld),
                                         HGF2DLocation(12.133750259876251 , 34.666694611310959 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(85.002065013249876, pResult->CalculateArea());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(85.002065013249876, pResult->CalculateArea());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(1484.6022367736994, pResult->CalculateArea());    

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(1484.6022361650764, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(1399.6001724933742, pResult->CalculateArea());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
       
    }

//==================================================================================
// Test which failed on April 23, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed39)
    {
          
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000000000000, 51.200000002614 , pWorld),
                                         HGF2DLocation(256.000000000000000, 64.000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000000000000, 64.000000000000 , pWorld),
                                         HGF2DLocation(320.000000000000000, 64.000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(320.000000000000000, 64.000000000000 , pWorld),
                                         HGF2DLocation(320.000000000000000, 0.0000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(320.000000000000000, 0.0000000000000 , pWorld),
                                         HGF2DLocation(256.000000013070000, 0.0000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000013070000, 0.0000000000000 , pWorld),
                                         HGF2DLocation(256.000000013070000, 51.200000002614 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000013070000, 51.200000002614 , pWorld),
                                         HGF2DLocation(256.000000000000000, 51.200000002614 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.00000001307, 0.0000000000000 , pWorld),
                                         HGF2DLocation(256.00000001307, 51.200000002614 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.00000001307, 51.200000002614 , pWorld),
                                         HGF2DLocation(307.00000001307, 51.200000002614 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(307.00000001307, 51.200000002614 , pWorld),
                                         HGF2DLocation(307.00000001307, 0.0000000000000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(307.00000001307, 0.0000000000000 , pWorld),
                                         HGF2DLocation(256.00000001307, 0.0000000000000 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(2611.20000013331446, pResult->CalculateArea());   

    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(2611.20000013331446, pResult->CalculateArea());
   
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(4095.9999993308147, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(4096.0000000000000, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(1484.79999919750025, pResult->CalculateArea());   

    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);   
    
    }

//==================================================================================
// Test which failed on April 3, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed40)
    {
          
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 41.672118563598019 , pWorld),
                                         HGF2DLocation(64.00000000000000 , 4.7217013355800860 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(64.00000000000000 , 4.7217013355800860 , pWorld),
                                         HGF2DLocation(64.00000000000000 , 0.0000000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(64.00000000000000 , 0.0000000000000000 , pWorld),
                                         HGF2DLocation(0.000000000000000 , 0.0000000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 0.0000000000000000 , pWorld),
                                         HGF2DLocation(0.000000000000000 , 41.672118563598019 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(12.133750259876251 , 34.666694611310959 , pWorld),
                                         HGF2DLocation(0.0000000000000000 , 27.661270588636398 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000000 , 27.661270588636398 , pWorld),
                                         HGF2DLocation(0.0000000000000000 , 41.672118544578552 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000000 , 41.672118544578552 , pWorld),
                                         HGF2DLocation(12.133750259876251 , 34.666694611310959 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(85.002065013249876, pResult->CalculateArea());   

    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(85.002065013249876, pResult->CalculateArea());
   
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(1484.6022367736994, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(1484.6022361650764, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(1399.6001724933742, pResult->CalculateArea());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
       
    }

//==================================================================================
// Test which failed on July 20, 2000
//==================================================================================
 TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed41)
    {
    
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-838960.575561190260000 , 1443576.103989492600000 , pWorld),
                                         HGF2DLocation(-413480.105918325430000 , -666897.741530794650000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-413480.105918325430000 , -666897.741530794650000 , pWorld),
                                         HGF2DLocation(1706857.907802271400000 , -239428.613782825880000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1706857.907802271400000 , -239428.613782825880000 , pWorld),
                                         HGF2DLocation(1281377.438159406400000 , 1871045.231737461400000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1281377.438159406400000 , 1871045.231737461400000 , pWorld),
                                         HGF2DLocation(-838960.575561190260000 , 1443576.103989492600000 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-838960.575561276990000 , 1443576.103989754800000 , pWorld),
                                         HGF2DLocation(-413480.105917771700000 , -666897.741530503150000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-413480.105917771700000 , -666897.741530503150000 , pWorld),
                                         HGF2DLocation(1706857.907802418800000 , -239428.613782369070000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1706857.907802418800000 , -239428.613782369070000 , pWorld),
                                         HGF2DLocation(1281377.438160031600000 , 1871045.231737896800000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1281377.438160031600000 , 1871045.231737896800000 , pWorld),
                                         HGF2DLocation(-838960.575561276990000 , 1443576.103989754800000 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());  

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_DOUBLE_EQ(1.629791259765625, pResult->CalculateArea());   
    
    }

//==================================================================================
// Test which failed on August 10, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed42)
    {
          
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(47997.0000001080000, -6.0799102301491E-6 , pWorld),
                                         HGF2DLocation(47997.0000001080000, 6.0606155668938E-10, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(47997.0000001080000, 6.0606155668938e-10 , pWorld),
                                         HGF2DLocation(31998.9999999740000, 5.82035241515110E-8 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(31998.9999999740000, 5.82035241515110E-8 , pWorld),
                                         HGF2DLocation(31998.9999999740000, -4.2752602357745E-6 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(31998.9999999740000, -4.2752602357745E-6 , pWorld),
                                         HGF2DLocation(15998.9999999750000, -4.2831004951204E-6 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(15998.9999999750000, -4.2831004951204E-6 , pWorld),
                                         HGF2DLocation(15998.9999999750000, -1.2255894791577E-9 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(15998.9999999750000, -1.2255894791577E-9 , pWorld),
                                         HGF2DLocation(6.8693350428327E-12, 1.25284195532590E-9 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(6.8693350428327E-12, 1.25284195532590E-9 , pWorld),
                                         HGF2DLocation(1.75541220750240E-8, 11998.9999562820000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1.75541220750240E-8, 11998.9999562820000 , pWorld),
                                         HGF2DLocation(1.1692050834664E-11, 12000.0000000010000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1.1692050834664E-11, 12000.0000000010000 , pWorld),
                                         HGF2DLocation(8.44476795030410E-8, 23997.9998530850000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(8.44476795030410E-8, 23997.9998530850000 , pWorld),
                                         HGF2DLocation(6.69925762238890E-8, 23998.9999086140000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(6.69925762238890E-8, 23998.9999086140000 , pWorld),
                                         HGF2DLocation(1.30288784238330E-7, 35996.9998298540000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1.30288784238330E-7, 35996.9998298540000 , pWorld),
                                         HGF2DLocation(8.44457848633660E-8, 35997.9998767400000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(8.44457848633660E-8, 35997.9998767400000 , pWorld),
                                         HGF2DLocation(1.47869624092590E-7, 47995.9997635870000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1.47869624092590E-7, 47995.9997635870000 , pWorld),
                                         HGF2DLocation(1.23085794970060E-7, 47996.9997648790000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1.23085794970060E-7, 47996.9997648790000 , pWorld),
                                         HGF2DLocation(1.67107853747390E-7, 60504.0481535610000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1.67107853747390E-7, 60504.0481535610000 , pWorld),
                                         HGF2DLocation(60505.3091066160000, 60504.0481535610000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(60505.3091066160000, 60504.0481535610000 , pWorld),
                                         HGF2DLocation(60505.3091064030000, -6.2438056493748e-6 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(60505.3091064030000, -6.2438056493748e-6 , pWorld),
                                         HGF2DLocation(47997.0000001080000, -6.0799102301491e-6 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000, 0.00000 , pWorld),
                                         HGF2DLocation(0.0000000000000, 12000.0 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000, 12000.0 , pWorld),
                                         HGF2DLocation(15999.000000024, 12000.0 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(15999.000000024, 12000.0 , pWorld),
                                         HGF2DLocation(15999.000000024, 0.00000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(15999.000000024, 0.00000 , pWorld),
                                         HGF2DLocation(0.0000000000000, 0.00000 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());    

    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());   

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());    

    pResult = pShape2->DifferentiateShape(*pShape1);
        
    }

//==================================================================================
// Test which failed on August 14, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed43)
    {
            
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-32000.000000000029000 , 0.0000000149011610000, pWorld),
                                         HGF2DLocation(-32000.000000000029000 , 12000.000000015267000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-32000.000000000029000 , 12000.000000015267000, pWorld),
                                         HGF2DLocation(-16000.000000000029000 , 12000.000000008227000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-16000.000000000029000 , 12000.000000008227000, pWorld),
                                         HGF2DLocation(9876.00000000000000000 , 12000.000000011541000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(9876.00000000000000000 , 12000.000000011541000, pWorld),
                                         HGF2DLocation(9876.00000000000000000 , 0.0000000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(9876.00000000000000000 , 0.0000000000000000000, pWorld),
                                         HGF2DLocation(-32000.000000000029000 , 0.0000000149011610000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-0.00000000002910400 , 12000.000000003360000, pWorld),
                                         HGF2DLocation(-0.00000000002910400 , 24000.000000006719000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-0.00000000002910400 , 24000.000000006719000, pWorld),
                                         HGF2DLocation(8667.999999999970900 , 24000.000000002212000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(8667.999999999970900 , 24000.000000002212000, pWorld),
                                         HGF2DLocation(8667.999999999970900 , 11999.999999998852000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(8667.999999999970900 , 11999.999999998852000, pWorld),
                                         HGF2DLocation(-0.00000000002910400 , 12000.000000003360000, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_TRUE(pResult->IsEmpty());   

    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_TRUE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());   

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());   

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
   
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
        
    }

//==================================================================================
// THE FOLLOWING TESTS ARE ONLY VALID FOR AN EPSILON OF 1E-8
//==================================================================================

//==================================================================================
// Test which failed on August 16, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  AllocateCopyInCoordSysTestWhoFailed2)
    {

    HFCPtr<HGF2DCoordSys>   pTheOtherWorld     = new HGF2DCoordSys(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 10.00, 10.00), pWorld);
    HFCPtr<HGF2DCoordSys>   pTheOtherWorld2    = new HGF2DCoordSys(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 100.0, 100.0), pWorld);

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(24998.4375000170640 , 0.0000000000000000, pWorld),
                                         HGF2DLocation(24998.4375000170640 , 0.0000594766647000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(24998.4375000170640 , 0.0000594766647000, pWorld),
                                         HGF2DLocation(0.00000000000000000 , 0.0000595476536000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , 0.0000595476536000, pWorld),
                                         HGF2DLocation(0.00000000000000000 , 18750.000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , 18750.000000000000, pWorld),
                                         HGF2DLocation(25000.0000000000000 , 18750.000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0000000000000 , 18750.000000000000, pWorld),
                                         HGF2DLocation(25000.0000000000000 , 18749.999993365014, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0000000000000 , 18749.999993365014, pWorld),
                                         HGF2DLocation(24998.4374999393650 , 18749.999993365020, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(24998.4374999393650 , 18749.999993365020, pWorld),
                                         HGF2DLocation(24998.4375000608570 , 0.0000067755764000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(24998.4375000608570 , 0.0000067755764000, pWorld),
                                         HGF2DLocation(25000.0000000000000 , 0.0000067755688000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0000000000000 , 0.0000067755688000, pWorld),
                                         HGF2DLocation(25000.0000000000000 , 0.0000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0000000000000 , 0.0000000000000000, pWorld),
                                         HGF2DLocation(24998.4375000170640 , 0.0000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    #ifdef WIP_IPPTEST_BUG_2
    // This tests what happens when a polygon of segments becomes autocontiguous
    // as a result of scaling ... the result shoud be complex
    HFCPtr<HVE2DShape> pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pTheOtherWorld));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
    
    pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pTheOtherWorld2));
    //ASSERT_TRUE(pResult->IsComplex()); 
    //ASSERT_EQ(2, pResult->GetShapeList().size());    
    #endif

    }

//==================================================================================
// Additional AllocateCopyInCoordSys tests
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  AllocateCopyInCoordSysTestWhoFailed3)
    {
   
    HFCPtr<HGF2DCoordSys>   pTheOtherWorld     = new HGF2DCoordSys(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 10.0, 10.0), pWorld);
    HFCPtr<HGF2DCoordSys>   pTheOtherWorld2    = new HGF2DCoordSys(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 100.0, 100.0), pWorld);
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0 , 0.00000000, pWorld), HGF2DLocation(0.00000 , 0.00000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000 , 0.00000000, pWorld), HGF2DLocation(0.00000 , 18000.0000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000 , 18000.0000, pWorld), HGF2DLocation(12000.0 , 18000.0000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(12000.0 , 18000.0000, pWorld), HGF2DLocation(12000.0 , 0.00000005, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(12000.0 , 0.00000005, pWorld), HGF2DLocation(18000.0 , 0.00000005, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(18000.0 , 0.00000005, pWorld), HGF2DLocation(18000.0 , 18000.0000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(18000.0 , 18000.0000, pWorld), HGF2DLocation(25000.0 , 18000.0000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0 , 18000.0000, pWorld), HGF2DLocation(25000.0 , 0.00000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HFCPtr<HVE2DShape> pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pTheOtherWorld));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
    
    pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pTheOtherWorld2));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
       
    }

//==================================================================================
// Additional AllocateCopyInCoordSys tests
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  AllocateCopyInCoordSysTestWhoFailed4)
    {
   
    HFCPtr<HGF2DCoordSys>   pTheOtherWorld     = new HGF2DCoordSys(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 10.0, 10.0), pWorld);
    HFCPtr<HGF2DCoordSys>   pTheOtherWorld2    = new HGF2DCoordSys(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 100.0, 100.0), pWorld);
    
    HVE2DComplexLinear  TheLinear1(pWorld);
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0 , 0.00000000, pWorld), HGF2DLocation(12000.0 , 0.000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(12000.0 , 0.00000000, pWorld), HGF2DLocation(0.00000 , 0.000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000 , 0.00000000, pWorld), HGF2DLocation(0.00000 , 18000.00000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000 , 18000.0000, pWorld), HGF2DLocation(12000.0 , 18000.00000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(12000.0 , 18000.0000, pWorld), HGF2DLocation(12000.0 , 0.000000050, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(12000.0 , 0.00000005, pWorld), HGF2DLocation(18000.0 , 0.000000050, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(18000.0 , 0.00000005, pWorld), HGF2DLocation(18000.0 , 18000.00000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(18000.0 , 18000.0000, pWorld), HGF2DLocation(25000.0 , 18000.00000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0 , 18000.0000, pWorld), HGF2DLocation(25000.0 , 0.000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HFCPtr<HVE2DShape> pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pTheOtherWorld));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
    
    pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pTheOtherWorld2));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
    
    }

//==================================================================================
// Additional AllocateCopyInCoordSys tests
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  AllocateCopyInCoordSysTestWhoFailed5)
    {
    
    HFCPtr<HGF2DCoordSys>   pTheOtherWorld     = new HGF2DCoordSys(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 10.0, 10.0), pWorld);
    HFCPtr<HGF2DCoordSys>   pTheOtherWorld2    = new HGF2DCoordSys(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 100.0, 100.0), pWorld);
    
    HVE2DComplexLinear  TheLinear1(pWorld);
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0 , -10000.000, pWorld), HGF2DLocation(15000.0 , -10000.0000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(15000.0 , -10000.000, pWorld), HGF2DLocation(15000.0 , 0.000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(15000.0 , 0.00000000, pWorld), HGF2DLocation(0.00000 , 0.000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000 , 0.00000000, pWorld), HGF2DLocation(0.00000 , 18000.00000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000 , 18000.0000, pWorld), HGF2DLocation(12000.0 , 18000.00000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(12000.0 , 18000.0000, pWorld), HGF2DLocation(12000.0 , 0.000000050, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(12000.0 , 0.00000005, pWorld), HGF2DLocation(18000.0 , 0.000000050, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(18000.0 , 0.00000005, pWorld), HGF2DLocation(18000.0 , 18000.00000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(18000.0 , 18000.0000, pWorld), HGF2DLocation(25000.0 , 18000.00000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0 , 18000.0000, pWorld), HGF2DLocation(25000.0 , -10000.0000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HFCPtr<HVE2DShape> pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pTheOtherWorld));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());    

    pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pTheOtherWorld2));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
    
    }

//==================================================================================
// Additional AllocateCopyInCoordSys tests
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  AllocateCopyInCoordSysTestWhoFailed6)
    {
   
    HFCPtr<HGF2DCoordSys>   pTheOtherWorld     = new HGF2DCoordSys(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 10.0, 10.0), pWorld);
    HFCPtr<HGF2DCoordSys>   pTheOtherWorld2    = new HGF2DCoordSys(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 100.0, 100.0), pWorld);
    
    HVE2DComplexLinear  TheLinear1(pWorld);
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0 , -10000.00000, pWorld), HGF2DLocation(20000.0 , -10000.0000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(20000.0 , -10000.00000, pWorld), HGF2DLocation(20000.0 , 0.000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(20000.0 , 0.0000000000, pWorld), HGF2DLocation(15000.0 , 0.000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(15000.0 , 0.0000000000, pWorld), HGF2DLocation(15000.0 , -10000.0000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(15000.0 , -10000.00000, pWorld), HGF2DLocation(0.00000 , -10000.0000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000 , -10000.00000, pWorld), HGF2DLocation(0.00000 , 18000.00000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000 , 18000.000000, pWorld), HGF2DLocation(12000.0 , 18000.00000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(12000.0 , 18000.000000, pWorld), HGF2DLocation(12000.0 , 0.000000050, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(12000.0 , 0.0000000500, pWorld), HGF2DLocation(18000.0 , 0.000000050, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(18000.0 , 0.0000000500, pWorld), HGF2DLocation(18000.0 , 18000.00000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(18000.0 , 18000.000000, pWorld), HGF2DLocation(25000.0 , 18000.00000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0 , 18000.000000, pWorld), HGF2DLocation(25000.0 , -10000.0000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HFCPtr<HVE2DShape> pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pTheOtherWorld));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());    

    pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pTheOtherWorld2));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());    

    }

//==================================================================================
// Additional AllocateCopyInCoordSys test
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  AllocateCopyInCoordSysTestWhoFailed7)
    {
   
    HFCPtr<HGF2DCoordSys>   pTheOtherWorld     = new HGF2DCoordSys(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 10.0, 10.0), pWorld);
    HFCPtr<HGF2DCoordSys>   pTheOtherWorld2    = new HGF2DCoordSys(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 100.0, 100.0), pWorld);
    
    HVE2DComplexLinear  TheLinear1(pWorld);
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0000 , -10000.0, pWorld), HGF2DLocation(0.000000000 , -10000.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000 , -10000.0, pWorld), HGF2DLocation(0.000000000 , 18000.00, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000 , 18000.00, pWorld), HGF2DLocation(25000.00000 , 18000.00, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0000 , 18000.00, pWorld), HGF2DLocation(25000.00000 , 12000.00, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0000 , 12000.00, pWorld), HGF2DLocation(0.000000050 , 12000.00, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000005 , 12000.00, pWorld), HGF2DLocation(0.000000050 , 11000.00, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000005 , 11000.00, pWorld), HGF2DLocation(25000.00000 , 11000.00, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0000 , 11000.00, pWorld), HGF2DLocation(25000.00000 , 0.000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0000 , 0.000000, pWorld), HGF2DLocation(0.000000050 , 0.000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000005 , 0.000000, pWorld), HGF2DLocation(0.000000050 , -5000.00, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000005 , -5000.00, pWorld), HGF2DLocation(25000.00000 , -5000.00, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0000 , -5000.00, pWorld), HGF2DLocation(25000.00000 , -10000.0, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HFCPtr<HVE2DShape> pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pTheOtherWorld));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(3, pResult->GetShapeList().size());
    
    pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pTheOtherWorld2));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(3, pResult->GetShapeList().size());
    
    }

//==================================================================================
// Test which failed on August 22, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester, ModifyShapeWithPointerWhoFailed44)
    {
          
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-16000.000000000007, -21754.775510203093, pWorld),
                                         HGF2DLocation(-16000.000000000007, 12000.0000000061850, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-16000.000000000007, 12000.0000000061850, pWorld),
                                         HGF2DLocation(34245.2244897959170, 12000.0000000122160, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(34245.2244897959170, 12000.0000000122160, pWorld),
                                         HGF2DLocation(34245.2244897959170, -21754.775510197065, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(34245.2244897959170, -21754.775510197065, pWorld),
                                         HGF2DLocation(-16000.000000000007, -21754.775510203093, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000, 0.00000, pWorld), HGF2DLocation(0.00000, 12000.0, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000, 12000.0, pWorld), HGF2DLocation(16000.0, 12000.0, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(16000.0, 12000.0, pWorld), HGF2DLocation(16000.0, 0.00000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(16000.0, 0.00000, pWorld), HGF2DLocation(0.00000, 0.00000, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);
    
    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());

    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());

    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());    

    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);    
    
    }

//==================================================================================
// Parallel copy test (MartinBu 25 august 2000)
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  AppendPointTestWhoFailed)
    {
   
    HVE2DPolySegment PolySegment(pWorld);

    HGF2DLocation Pt1(496871.025, 3519368.775 ,pWorld);
    PolySegment.AppendPoint(Pt1);

    HGF2DLocation Pt2(496871.025, 3520443.975 ,pWorld);
    PolySegment.AppendPoint(Pt2);

    HGF2DLocation Pt3(498791.025, 3520443.975 ,pWorld);
    PolySegment.AppendPoint(Pt3);

    HGF2DLocation Pt4(498791.025, 3519368.775 ,pWorld);
    PolySegment.AppendPoint(Pt4);

    HGF2DLocation Pt5(496871.025, 3519368.775 ,pWorld);
    PolySegment.AppendPoint(Pt5);

    HVE2DPolygonOfSegments OriginalPolygon(PolySegment);

    HVE2DVector::ArbitraryDirection Dir = HVE2DVector::ALPHA;

    double Offset(5.0);

    HFCPtr<HVE2DPolygonOfSegments> NewPolygon = OriginalPolygon.AllocateParallelCopy(Offset, Dir);

    }

//==================================================================================
// Test which failed on Sept 6, 2000 (BUG #4737)
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester, ModifyShapeWithPointerWhoFailed45 )
    {
   
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1131.370849898475900 , 0.000000000000000000, pWorld),
                                         HGF2DLocation(0.000000000000000000 , 1131.370849898476100, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000000 , 1131.370849898476100, pWorld),
                                         HGF2DLocation(331.3708498984757400 , 1462.741699796951700, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(331.3708498984757400 , 1462.741699796951700, pWorld),
                                         HGF2DLocation(331.3708498984757400 , 800.0000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(331.3708498984757400 , 800.0000000000000000, pWorld),
                                         HGF2DLocation(1931.370849898475600 , 800.0000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1931.370849898475600 , 800.0000000000000000, pWorld),
                                         HGF2DLocation(1931.370849898475600 , 1268.629150101523500, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1931.370849898475600 , 1268.629150101523500, pWorld),
                                         HGF2DLocation(2028.427124746189700 , 1365.685424949237600, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2028.427124746189700 , 1365.685424949237600, pWorld),
                                         HGF2DLocation(2262.000000000000000 , 1132.112549695427300, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2262.000000000000000 , 1132.112549695427300, pWorld),
                                         HGF2DLocation(2262.000000000000000 , 1130.629150101524100, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2262.000000000000000 , 1130.629150101524100, pWorld),
                                         HGF2DLocation(1131.370849898475900 , 0.000000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(363.370849898475970 , 768.0000000000000000, pWorld),
                                         HGF2DLocation(223.999999999999890 , 907.3708498984760800, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(223.999999999999890 , 907.3708498984760800, pWorld),
                                         HGF2DLocation(223.999999999999890 , 1024.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(223.999999999999890 , 1024.000000000000000, pWorld),
                                         HGF2DLocation(479.999999999999890 , 1024.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(479.999999999999890 , 1024.000000000000000, pWorld),
                                         HGF2DLocation(479.999999999999890 , 768.0000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(479.999999999999890 , 768.0000000000000000, pWorld),
                                         HGF2DLocation(363.370849898475970 , 768.0000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);
    
    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->UnifyShape(*pShape2); 
        
    #ifdef WIP_IPPTEST_BUG_3 
    ASSERT_FALSE(pResult->IsEmpty()); 
    #endif
            
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());    

    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
     
    }

//==================================================================================
// Test which failed on Sept 6, 2000 (BUG #4738)
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed46)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1697.056274847713700 , 0.000000000000000000, pWorld),
                                         HGF2DLocation(0.000000000000000000 , 1697.056274847714300, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000000 , 1697.056274847714300, pWorld),
                                         HGF2DLocation(497.0562748477141200 , 2194.112549695428200, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(497.0562748477141200 , 2194.112549695428200, pWorld),
                                         HGF2DLocation(497.0562748477141200 , 1199.999999999999800, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(497.0562748477141200 , 1199.999999999999800, pWorld),
                                         HGF2DLocation(2097.056274847714100 , 1199.999999999999800, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2097.056274847714100 , 1199.999999999999800, pWorld),
                                         HGF2DLocation(2097.056274847714100 , 1297.056274847713700, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2097.056274847714100 , 1297.056274847713700, pWorld),
                                         HGF2DLocation(2194.112549695428200 , 1199.999999999999500, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2194.112549695428200 , 1199.999999999999500, pWorld),
                                         HGF2DLocation(3042.640687119285800 , 2048.528137423857100, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(3042.640687119285800 , 2048.528137423857100, pWorld),
                                         HGF2DLocation(3394.000000000000000 , 1697.168824543141900, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(3394.000000000000000 , 1697.168824543141900, pWorld),
                                         HGF2DLocation(3394.000000000000000 , 1696.943725152285400, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(3394.000000000000000 , 1696.943725152285400, pWorld),
                                         HGF2DLocation(1697.056274847713700 , 0.000000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(624.000000000000450 , 1073.056274847713700, pWorld),
                                         HGF2DLocation(417.056274847714350 , 1280.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(417.056274847714350 , 1280.000000000000000, pWorld),
                                         HGF2DLocation(624.000000000000450 , 1280.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(624.000000000000450 , 1280.000000000000000, pWorld),
                                         HGF2DLocation(624.000000000000450 , 1073.056274847713700, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);
    
    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->UnifyShape(*pShape2);
        
    #ifdef WIP_IPPTEST_BUG_3
    //ASSERT_FALSE(pResult->IsEmpty());
    #endif 
          
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());   

    pResult = pShape2->DifferentiateShape(*pShape1);
       
    }

//==================================================================================
// Test which failed on Sept 7, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester, ModifyShapeWithPointerWhoFailed47 )
    {
           
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(76.00, 0.000, pWorld), HGF2DLocation(76.00, 5.000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(76.00, 5.000, pWorld), HGF2DLocation(70.00, 5.000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(70.00, 5.000, pWorld), HGF2DLocation(70.00, 0.000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(70.00, 0.000, pWorld), HGF2DLocation(0.000, 0.000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000, 0.000, pWorld), HGF2DLocation(0.000, 256.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000, 256.0, pWorld), HGF2DLocation(256.0, 256.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0, 256.0, pWorld), HGF2DLocation(256.0, 0.000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0, 0.000, pWorld), HGF2DLocation(76.00, 0.000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HFCPtr<HVE2DShape> pShape2 = new HVE2DRectangle(0.0, 0.0, 256.0, 256.0, pWorld);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(65506.0, pResult->CalculateArea());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(65506.0, pResult->CalculateArea());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(65536.0, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(65536.0, pResult->CalculateArea());   

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_TRUE(pResult->IsEmpty());   

    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(30.0, pResult->CalculateArea());
    
    // Other additional tests
    HVE2DPolygonOfSegments MySameRect(HVE2DRectangle(0.0, 0.0, 256.0, 256.0, pWorld));

    ASSERT_EQ(HVE2DShape::S_IN, MySameRect.CalculateSpatialPositionOf(*pShape1));
    ASSERT_EQ(HVE2DShape::S_OUT, pShape1->CalculateSpatialPositionOf(MySameRect));
    ASSERT_EQ(HVE2DShape::S_IN, pShape2->CalculateSpatialPositionOf(*pShape1));
    ASSERT_EQ(HVE2DShape::S_OUT, pShape1->CalculateSpatialPositionOf(*pShape2));
   
    }

//==================================================================================
// Test which failed on Sept 11, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester, ModifyShapeWithPointerWhoFailed48)
    {
            
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000, 0.000, pWorld), HGF2DLocation(0.000, 256.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000, 256.0, pWorld), HGF2DLocation(256.0, 256.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0, 256.0, pWorld), HGF2DLocation(256.0, 0.000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0, 0.000, pWorld), HGF2DLocation(0.000, 0.000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1.9746125868655, 0.29621359212831000, pWorld),
                                         HGF2DLocation(2.3695301674602, 0.29621359212763000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2.3695301674602, 0.29621359212763000, pWorld),
                                         HGF2DLocation(2.3695301674600, 6.8212102632970E-13, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2.3695301674600, 6.8212102632970E-13, pWorld),
                                         HGF2DLocation(1.9746125868653, 1.5916157281026E-12, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1.9746125868653, 1.5916157281026E-12, pWorld),
                                         HGF2DLocation(1.9745879029738, 7.9580786405131E-13, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1.9745879029738, 7.9580786405131E-13, pWorld),
                                         HGF2DLocation(1.5796703223790, -1.1368683772162E-13, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1.5796703223790, -1.1368683772162E-13, pWorld),
                                         HGF2DLocation(1.1847527417843, 7.47832018532790E-10, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1.1847527417843, 7.47832018532790E-10, pWorld),
                                         HGF2DLocation(1.1847527417843, 0.29618890691438000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1.1847527417843, 0.29618890691438000, pWorld),
                                         HGF2DLocation(1.5796703223791, 0.29618890691438000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1.5796703223791, 0.29618890691438000, pWorld),
                                         HGF2DLocation(1.5796703223792, 0.29621359212672000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1.5796703223792, 0.29621359212672000, pWorld),
                                         HGF2DLocation(1.9745879029739, 0.29621359212763000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1.9745879029739, 0.29621359212763000, pWorld),
                                         HGF2DLocation(1.9746125868655, 0.29621359212831000, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_TRUE(pResult->IsEmpty());
    
    }

//==================================================================================
// Test which failed on Sept 19, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester, ModifyShapeWithPointerWhoFailed49 )
    {
           
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(224.0, 800.00, pWorld), HGF2DLocation(480.0, 1056.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(480.0, 1056.0, pWorld), HGF2DLocation(736.0, 800.00, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(736.0, 800.00, pWorld), HGF2DLocation(480.0, 544.00, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(480.0, 544.00, pWorld), HGF2DLocation(224.0, 800.00, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DRectangle(0.0, 512.0, 256.0, 768.0, pWorld);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_TRUE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_TRUE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(196608.000000000000, pResult->CalculateArea());  

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(196608.000000000000, pResult->CalculateArea());  

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
   
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    // Other additional tests
    ASSERT_EQ(HVE2DShape::S_OUT, pShape2->CalculateSpatialPositionOf(*pShape1));
    ASSERT_EQ(HVE2DShape::S_OUT, pShape1->CalculateSpatialPositionOf(*pShape2));
    
    }

//==================================================================================
// Test which failed on Nov 9, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester, ModifyShapeWithPointerWhoFailed50 )
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1030.3762349323, 255.99999998882, pWorld),
                                         HGF2DLocation(778.69140000000, 255.99999999255, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(778.69140000000, 255.99999999255, pWorld),
                                         HGF2DLocation(778.31236900000, 277.71485400000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(778.31236900000, 277.71485400000, pWorld),
                                         HGF2DLocation(1029.9205400000, 282.10669100000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1029.9205400000, 282.10669100000, pWorld),
                                         HGF2DLocation(1030.3762349323, 255.99999998882, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DRectangle(0.0, 0.0, 4251.0, 256.0, pWorld);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_TRUE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_TRUE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());   

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());    

    }

//==================================================================================
// Test which failed on Nov 15, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester, ModifyShapeWithPointerWhoFailed51)
    {
          
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0008822139352560 , 0.01213040202856100, pWorld),
                                         HGF2DLocation(0.0007792119868100 , 253.640242036432030, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0007792119868100 , 253.640242036432030, pWorld),
                                         HGF2DLocation(233.40057656611316 , 240.064462127164010, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(233.40057656611316 , 240.064462127164010, pWorld),
                                         HGF2DLocation(219.43766127480194 , 0.01214403659105300, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(219.43766127480194 , 0.01214403659105300, pWorld),
                                         HGF2DLocation(0.0008822139352560 , 0.01213040202856100, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    pShape1->SetAutoToleranceActive(false);
    pShape1->SetTolerance(0.001);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000812306855621 , 0.0000000000000000, pWorld),
                                         HGF2DLocation(0.000892448239028 , 50.512210074812174, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000892448239028 , 50.512210074812174, pWorld),
                                         HGF2DLocation(0.000582597115946 , 256.00000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000582597115946 , 256.00000000000000, pWorld),
                                         HGF2DLocation(256.0000000000000 , 256.00000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.0000000000000 , 256.00000000000000, pWorld),
                                         HGF2DLocation(256.0000000000000 , 0.0000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.0000000000000 , 0.0000000000000000, pWorld),
                                         HGF2DLocation(0.000812306855621 , 0.0000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
       
    }

//==================================================================================
// AutoCrossesTestWhoFailed
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester, AutoCrossesTestWhoFailed)
    {

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(17.9195179616799580 , 0.012126367539167, pWorld),
                                         HGF2DLocation(-0.0000012393575160 , 0.012124864384532, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-0.0000012393575160 , 0.012124864384532, pWorld),
                                         HGF2DLocation(0.00000170688144900 , 0.012191701680422, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000170688144900 , 0.012191701680422, pWorld),
                                         HGF2DLocation(189.655513013247400 , 0.012132421135902, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(189.655513013247400 , 0.012132421135902, pWorld),
                                         HGF2DLocation(189.655512792989610 , 0.012128636240959, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(189.655512792989610 , 0.012128636240959, pWorld),
                                         HGF2DLocation(168.188499589683490 , 0.012121440842748, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(168.188499589683490 , 0.012121440842748, pWorld),
                                         HGF2DLocation(157.454992987914010 , 0.012117844074965, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(157.454992987914010 , 0.012117844074965, pWorld),
                                         HGF2DLocation(152.088239687145690 , 0.012116044759750, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(152.088239687145690 , 0.012116044759750, pWorld),
                                         HGF2DLocation(150.746551361866300 , 0.012115595862269, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(150.746551361866300 , 0.012115595862269, pWorld),
                                         HGF2DLocation(150.075707199284810 , 0.012115368619561, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(150.075707199284810 , 0.012115368619561, pWorld),
                                         HGF2DLocation(149.740285117877650 , 0.012115256860852, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(149.740285117877650 , 0.012115256860852, pWorld),
                                         HGF2DLocation(149.572574077406900 , 0.012115200981498, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(149.572574077406900 , 0.012115200981498, pWorld),
                                         HGF2DLocation(149.530646317056380 , 0.012115186080337, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(149.530646317056380 , 0.012115186080337, pWorld),
                                         HGF2DLocation(149.509682436939330 , 0.012115180492401, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(149.509682436939330 , 0.012115180492401, pWorld),
                                         HGF2DLocation(149.499200497055430 , 0.012115176767111, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(149.499200497055430 , 0.012115176767111, pWorld),
                                         HGF2DLocation(149.496580011909830 , 0.012115174904466, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(149.496580011909830 , 0.012115174904466, pWorld),
                                         HGF2DLocation(149.495924890739840 , 0.012115174904466, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(149.495924890739840 , 0.012115174904466, pWorld),
                                         HGF2DLocation(149.495761110447350 , 0.012115174904466, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(149.495761110447350 , 0.012115174904466, pWorld),
                                         HGF2DLocation(149.495720165199600 , 0.012115174904466, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(149.495720165199600 , 0.012115174904466, pWorld),
                                         HGF2DLocation(149.495699692750350 , 0.012115173041821, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(149.495699692750350 , 0.012115173041821, pWorld),
                                         HGF2DLocation(149.495681825559590 , 0.012169376015663, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(149.495681825559590 , 0.012169376015663, pWorld),
                                         HGF2DLocation(149.495599935296920 , 0.012169374153018, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(149.495599935296920 , 0.012169374153018, pWorld),
                                         HGF2DLocation(149.495272374944760 , 0.012169376015663, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(149.495272374944760 , 0.012169376015663, pWorld),
                                         HGF2DLocation(149.493962133536120 , 0.012169374153018, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(149.493962133536120 , 0.012169374153018, pWorld),
                                         HGF2DLocation(149.488721167901530 , 0.012169374153018, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(149.488721167901530 , 0.012169374153018, pWorld),
                                         HGF2DLocation(149.404865716584030 , 0.012169348075986, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(149.404865716584030 , 0.012169348075986, pWorld),
                                         HGF2DLocation(146.721491272561250 , 0.012168470770121, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(146.721491272561250 , 0.012168470770121, pWorld),
                                         HGF2DLocation(103.787500168895350 , 0.012154435738921, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(103.787500168895350 , 0.012154435738921, pWorld),
                                         HGF2DLocation(17.9195179616799580 , 0.012126367539167, pWorld)));

    TheLinear1.SetAutoToleranceActive(false);
    TheLinear1.SetTolerance(0.001);

    ASSERT_FALSE(TheLinear1.AutoCrosses());

    TheLinear1.SetTolerance(0.0001);
    ASSERT_FALSE(TheLinear1.AutoCrosses());

    TheLinear1.SetTolerance(0.00001);
    ASSERT_TRUE(TheLinear1.AutoCrosses());

    TheLinear1.SetTolerance(0.000001);
    ASSERT_TRUE(TheLinear1.AutoCrosses());

    TheLinear1.SetTolerance(0.0000001);
    ASSERT_TRUE(TheLinear1.AutoCrosses());

    TheLinear1.SetTolerance(0.00000001);
    ASSERT_TRUE(TheLinear1.AutoCrosses());
    
    }

//==================================================================================
// Test which failed on Nov 16, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester, ModifyShapeWithPointerWhoFailed52)
    {
            
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 0.000000000000000, pWorld),
                                         HGF2DLocation(0.000000000000000 , 256.0000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 256.0000000000000, pWorld),
                                         HGF2DLocation(256.0000000000000 , 256.0000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0000000000000 , 256.0000000000000, pWorld),
                                         HGF2DLocation(256.0000000000000 , 0.000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0000000000000 , 0.000000000000000, pWorld),
                                         HGF2DLocation(0.000000000000000 , 0.000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-0.00000000011641500 , -255.999924341682340, pWorld),
                                         HGF2DLocation(0.000044487445848000 , 874.6514387526549400, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000044487445848000 , 874.6514387526549400, pWorld),
                                         HGF2DLocation(1507.537023507495200 , 874.6513630896806700, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1507.537023507495200 , 874.6513630896806700, pWorld),
                                         HGF2DLocation(1507.536979380151000 , -256.000000000465660, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1507.536979380151000 , -256.000000000465660, pWorld),
                                         HGF2DLocation(-0.00000000011641500 , -255.999924341682340, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    pShape2->SetAutoToleranceActive(false);
    pShape2->SetTolerance(0.001);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());   
    
    }

//==================================================================================
// THE FOLLOWING TESTS ARE ONLY VALID FOR AN EPSILON OF 1E-8
//==================================================================================

//==================================================================================
// Test which failed on Nov 16, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester, IsAutoContiguousTestWhoFailed)
    {
  
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146612.616979246520000 , -3733745.133152585500000, pWorld),
                                         HGF2DLocation(-146610.838938417060000 , -3733745.029725480400000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146610.838938417060000 , -3733745.029725480400000, pWorld),
                                         HGF2DLocation(-146612.480445448800000 , -3733716.810040059500000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146612.480445448800000 , -3733716.810040059500000, pWorld),
                                         HGF2DLocation(-146614.121941842870000 , -3733688.590353258400000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146614.121941842870000 , -3733688.590353258400000, pWorld),
                                         HGF2DLocation(-146614.760026228150000 , -3733677.620692211700000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146614.760026228150000 , -3733677.620692211700000, pWorld),
                                         HGF2DLocation(-146614.760025639170000 , -3733677.620692211700000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146614.760025639170000 , -3733677.620692211700000, pWorld),
                                         HGF2DLocation(-146612.878627912110000 , -3733709.964706689100000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146612.878627912110000 , -3733709.964706689100000, pWorld),
                                         HGF2DLocation(-146610.865070915780000 , -3733744.580448843500000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146610.865070915780000 , -3733744.580448843500000, pWorld),
                                         HGF2DLocation(-146610.838938297970000 , -3733745.029725568800000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146610.838938297970000 , -3733745.029725568800000, pWorld),
                                         HGF2DLocation(-146612.616979246520000 , -3733745.133152585500000, pWorld)));

    HVE2DPolygonOfSegments MyPolygon(TheLinear1);

    HVE2DPolySegment  AddPolySegment1(pWorld);

    AddPolySegment1.AppendPoint(HGF2DLocation(74.796603888942343 , 51.514879816683035, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(74.796616508015234 , 103.020908160924170, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(74.796616688978688 , 123.042521701129810, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(74.796617760372797 , 123.042521638825620, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(74.796607313434947 , 64.008850551921000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(74.796613226371505 , 0.828861127342226, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(74.796603888942343 , 51.514879816683035, pWorld));

    AddPolySegment1.SetAutoToleranceActive(false);
    AddPolySegment1.SetTolerance(0.00001);

    ASSERT_TRUE(AddPolySegment1.IsAutoContiguous());

    AddPolySegment1.RemoveAutoContiguousNeedles(true);
    ASSERT_FALSE(AddPolySegment1.IsAutoContiguous());

    }

//==================================================================================
// Test which failed on Nov 16, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester, AllocateComplexShapeFromAutoContiguousPolySegmentTestWhoFailed)
    {
    
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146612.616979246520000 , -3733745.133152585500000, pWorld),
                                         HGF2DLocation(-146610.838938417060000 , -3733745.029725480400000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146610.838938417060000 , -3733745.029725480400000, pWorld),
                                         HGF2DLocation(-146612.480445448800000 , -3733716.810040059500000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146612.480445448800000 , -3733716.810040059500000, pWorld),
                                         HGF2DLocation(-146614.121941842870000 , -3733688.590353258400000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146614.121941842870000 , -3733688.590353258400000, pWorld),
                                         HGF2DLocation(-146614.760026228150000 , -3733677.620692211700000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146614.760026228150000 , -3733677.620692211700000, pWorld),
                                         HGF2DLocation(-146614.760025639170000 , -3733677.620692211700000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146614.760025639170000 , -3733677.620692211700000, pWorld),
                                         HGF2DLocation(-146612.878627912110000 , -3733709.964706689100000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146612.878627912110000 , -3733709.964706689100000, pWorld),
                                         HGF2DLocation(-146610.865070915780000 , -3733744.580448843500000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146610.865070915780000 , -3733744.580448843500000, pWorld),
                                         HGF2DLocation(-146610.838938297970000 , -3733745.029725568800000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146610.838938297970000 , -3733745.029725568800000, pWorld),
                                         HGF2DLocation(-146612.616979246520000 , -3733745.133152585500000, pWorld)));

    HVE2DPolygonOfSegments MyPolygon(TheLinear1);

    HVE2DPolySegment  AddPolySegment1(pWorld);

    AddPolySegment1.AppendPoint(HGF2DLocation(37.398305383578645 , 0.00442506910800300, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398301944801993 , 25.7574399086932540, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398308254143032 , 51.5104540812430190, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398305256362420 , 77.2634688865724400, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398309950957461 , 103.016483169801460, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398308470115047 , 128.769497876260170, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398312018577869 , 154.522512236347180, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398312263270519 , 154.996322700473460, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398324268186833 , 154.996322002121960, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398317068121948 , 128.004424629423450, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398313771654699 , 64.0044246748469730, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398312104805100 , 2.41303710120554900, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398305487265958 , 0.00442486824301700, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(36.551915531009243 , 0.00442465802248200, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398305383578645 , 0.00442506910800300, pWorld));

    AddPolySegment1.SetAutoToleranceActive(false);
    AddPolySegment1.SetTolerance(0.00001);

    ASSERT_TRUE(AddPolySegment1.IsAutoContiguous());

    AddPolySegment1.RemoveAutoContiguousNeedles(true);
    ASSERT_TRUE(AddPolySegment1.IsAutoContiguous());

    MyPolygon.AllocateComplexShapeFromAutoContiguousPolySegment(AddPolySegment1);

    }

//==================================================================================
// Test which failed on April 3, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed53)
    {
   
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 51.46683371067047 , pWorld),
                                         HGF2DLocation(14.14608025550842 , 42.66670471429825 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(14.14608025550842 , 42.66670471429825 , pWorld),
                                         HGF2DLocation(0.000000000000000 , 34.86657571792603 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 34.86657571792603 , pWorld),
                                         HGF2DLocation(0.000000000000000 , 51.46683371067047 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(51.65018710229799 , 64.00000000000000 , pWorld),
                                         HGF2DLocation(0.000000000000000 , 34.86657594899594 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 34.86657594899594 , pWorld),
                                         HGF2DLocation(0.000000000000000 , 64.00000000000000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 64.00000000000000 , pWorld),
                                         HGF2DLocation(51.65018710229799 , 64.00000000000000 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_NEAR(116.16168649443722, pResult->CalculateArea(), 10E-12);
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_NEAR(116.16168649443722, pResult->CalculateArea(), 10E-12);
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_NEAR(753.62600600179076, pResult->CalculateArea(), 10E-12);
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_NEAR(753.62600600179076, pResult->CalculateArea(), 10E-12);
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_NEAR(1.2526044193167323, pResult->CalculateArea(), 10E-12); 
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_NEAR(636.21171508803673, pResult->CalculateArea(), 10E-12); 
    
    }

//==================================================================================
// Test which failed on April 3, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed54)
    {
          
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(59.05016660690308 , 11.33375036716461 , pWorld),
                                         HGF2DLocation(64.00000000000000 , 8.804601848125460 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(64.00000000000000 , 8.804601848125460 , pWorld),
                                         HGF2DLocation(64.00000000000000 , 0.000000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(64.00000000000000 , 0.000000000000000 , pWorld),
                                         HGF2DLocation(39.75035202503204 , 0.000000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(39.75035202503204 , 0.000000000000000 , pWorld),
                                         HGF2DLocation(59.05016660690308 , 11.33375036716461 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(39.775035227748781 , 0.0000000000000000 , pWorld),
                                         HGF2DLocation(64.000000000000000 , 13.986289931850713 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(64.000000000000000 , 13.986289931850713 , pWorld),
                                         HGF2DLocation(64.000000000000000 , 0.0000000000000000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(64.000000000000000 , 0.0000000000000000 , pWorld),
                                         HGF2DLocation(39.775035227748781 , 0.0000000000000000 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(157.07306170822056, pResult->CalculateArea());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(157.07306170822056, pResult->CalculateArea());   

    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(171.546013178907, pResult->CalculateArea());   

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(171.546013178907, pResult->CalculateArea());  

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(2.137322732119344, pResult->CalculateArea()); 
   
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(12.335628738567095, pResult->CalculateArea());
       
    }

//==================================================================================
// Test which failed on April 3, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed55)
    {
          
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 41.72118563598019 , pWorld),
                                         HGF2DLocation(64.00000000000000 , 4.217013355800860 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(64.00000000000000 , 4.217013355800860 , pWorld),
                                         HGF2DLocation(64.00000000000000 , 0.000000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(64.00000000000000 , 0.000000000000000 , pWorld),
                                         HGF2DLocation(0.000000000000000 , 0.000000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 0.000000000000000 , pWorld),
                                         HGF2DLocation(0.000000000000000 , 41.72118563598019 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(12.33750259876251 , 34.66694611310959 , pWorld),
                                         HGF2DLocation(0.000000000000000 , 27.61270588636398 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 27.61270588636398 , pWorld),
                                         HGF2DLocation(0.000000000000000 , 41.72118544578552 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 41.72118544578552 , pWorld),
                                         HGF2DLocation(12.33750259876251 , 34.66694611310959 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(85.961965950575348, pResult->CalculateArea());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(85.961965950575348, pResult->CalculateArea());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(1471.0921034008938, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(1471.0921034008938, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(1384.0604017864168, pResult->CalculateArea());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(1.0697356639001407, pResult->CalculateArea());
     
    }

//==================================================================================
// Test which failed on April 23, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed56)
    {
            
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000000000000, 51.0000000261400 , pWorld),
                                         HGF2DLocation(256.000000000000000, 64.0000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000000000000, 64.0000000000000 , pWorld),
                                         HGF2DLocation(320.000000000000000, 64.0000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(320.000000000000000, 64.0000000000000 , pWorld),
                                         HGF2DLocation(320.000000000000000, 0.00000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(320.000000000000000, 0.00000000000000 , pWorld),
                                         HGF2DLocation(256.000000130700000, 0.00000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000130700000, 0.00000000000000 , pWorld),
                                         HGF2DLocation(256.000000130700000, 51.0000000261400 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000130700000, 51.0000000261400 , pWorld),
                                         HGF2DLocation(256.000000000000000, 51.0000000261400 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.0000001307, 0.000000000000 , pWorld),
                                         HGF2DLocation(256.0000001307, 51.00000002614 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.0000001307, 51.00000002614 , pWorld),
                                         HGF2DLocation(307.0000001307, 51.00000002614 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(307.0000001307, 51.00000002614 , pWorld),
                                         HGF2DLocation(307.0000001307, 0.000000000000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(307.0000001307, 0.000000000000 , pWorld),
                                         HGF2DLocation(256.0000001307, 0.000000000000 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_TRUE(pResult->IsEmpty());
       
    }

//==================================================================================
// Test which failed on April 3, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed57)
    {
           
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 41.72118563598019 , pWorld),
                                         HGF2DLocation(64.00000000000000 , 4.217013355800860 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(64.00000000000000 , 4.217013355800860 , pWorld),
                                         HGF2DLocation(64.00000000000000 , 0.000000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(64.00000000000000 , 0.000000000000000 , pWorld),
                                         HGF2DLocation(0.000000000000000 , 0.000000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 0.000000000000000 , pWorld),
                                         HGF2DLocation(0.000000000000000 , 41.72118563598019 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(12.33750259876251 , 34.66694611310959 , pWorld),
                                         HGF2DLocation(0.000000000000000 , 27.61270588636398 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 27.61270588636398 , pWorld),
                                         HGF2DLocation(0.000000000000000 , 41.72118544578552 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 41.72118544578552 , pWorld),
                                         HGF2DLocation(12.33750259876251 , 34.66694611310959 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(85.961965950575348, pResult->CalculateArea());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(85.961965950575348, pResult->CalculateArea());   

    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(1471.0921034008938, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(1471.0921034008938, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(1384.0604017864168, pResult->CalculateArea());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(1.0697356639001407, pResult->CalculateArea());
       
    }

//==================================================================================
// Test which failed on July 20, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed58)
    {
          
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-838960.75561190260000 , 1443576.03989492600000 , pWorld),
                                         HGF2DLocation(-413480.05918325430000 , -666897.41530794650000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-413480.05918325430000 , -666897.41530794650000 , pWorld),
                                         HGF2DLocation(1706857.07802271400000 , -239428.13782825880000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1706857.07802271400000 , -239428.13782825880000 , pWorld),
                                         HGF2DLocation(1281377.38159406400000 , 1871045.31737461400000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1281377.38159406400000 , 1871045.31737461400000 , pWorld),
                                         HGF2DLocation(-838960.75561190260000 , 1443576.03989492600000 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-838960.75561276990000 , 1443576.03989754800000 , pWorld),
                                         HGF2DLocation(-413480.05917771700000 , -666897.41530503150000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-413480.05917771700000 , -666897.41530503150000 , pWorld),
                                         HGF2DLocation(1706857.07802418800000 , -239428.13782369070000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1706857.07802418800000 , -239428.13782369070000 , pWorld),
                                         HGF2DLocation(1281377.38160031600000 , 1871045.31737896800000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1281377.38160031600000 , 1871045.31737896800000 , pWorld),
                                         HGF2DLocation(-838960.75561276990000 , 1443576.03989754800000 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_NEAR(4656796011527.4844, pResult->CalculateArea(), 10E-3);
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_NEAR(4656796011527.4844, pResult->CalculateArea(), 10E-3);
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_NEAR(4656796011556.3574, pResult->CalculateArea(), 10E-3);
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_NEAR(4656796011556.3574, pResult->CalculateArea(), 10E-3);
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_NEAR(12.55950927734375, pResult->CalculateArea(), 10E-3);
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_NEAR(16.314666505007704, pResult->CalculateArea(), 10E-3);
       
    }

//==================================================================================
// Test which failed on August 14, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed59)
    {
          
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-32000.000000000290 , 0.000000149011610000, pWorld),
                                         HGF2DLocation(-32000.000000000290 , 12000.00000015267000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-32000.000000000290 , 12000.00000015267000, pWorld),
                                         HGF2DLocation(-16000.000000000290 , 12000.00000008227000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-16000.000000000290 , 12000.00000008227000, pWorld),
                                         HGF2DLocation(9876.00000000000000 , 12000.00000011541000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(9876.00000000000000 , 12000.00000011541000, pWorld),
                                         HGF2DLocation(9876.00000000000000 , 0.000000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(9876.00000000000000 , 0.000000000000000000, pWorld),
                                         HGF2DLocation(-32000.000000000290 , 0.000000149011610000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-0.00000000029104 , 12000.00000003360000, pWorld),
                                         HGF2DLocation(-0.00000000029104 , 24000.00000006719000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-0.00000000029104 , 24000.00000006719000, pWorld),
                                         HGF2DLocation(8667.999999999709 , 24000.00000002212000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(8667.999999999709 , 24000.00000002212000, pWorld),
                                         HGF2DLocation(8667.999999999709 , 11999.99999998852000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(8667.999999999709 , 11999.99999998852000, pWorld),
                                         HGF2DLocation(-0.00000000029104 , 12000.00000003360000, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);
    
    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(0.0005432283396658022, pResult->CalculateArea());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(0.0005432283396658022, pResult->CalculateArea());   

    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(606528000.00038934, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(606528000.00038934, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(502511999.99989299, pResult->CalculateArea());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(104016000.00029115, pResult->CalculateArea());
      
    }

//==================================================================================
// Test which failed on August 16, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester, AllocateCopyInCoordSysTestWhoFailed8)
    {

    HFCPtr<HGF2DCoordSys>   pTheOtherWorld     = new HGF2DCoordSys(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 100.0, 100.0), pWorld);
    HFCPtr<HGF2DCoordSys>   pTheOtherWorld2    = new HGF2DCoordSys(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 1000.0, 1000.0), pWorld);

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(24998.375000170640 , 0.000000000000000, pWorld),
                                         HGF2DLocation(24998.375000170640 , 0.100594766647000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(24998.375000170640 , 0.100594766647000, pWorld),
                                         HGF2DLocation(0.0000000000000000 , 0.100595476536000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000000 , 0.100595476536000, pWorld),
                                         HGF2DLocation(0.0000000000000000 , 18750.00000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000000 , 18750.00000000000, pWorld),
                                         HGF2DLocation(25000.000000000000 , 18750.00000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.000000000000 , 18750.00000000000, pWorld),
                                         HGF2DLocation(25000.000000000000 , 18749.99993365014, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.000000000000 , 18749.99993365014, pWorld),
                                         HGF2DLocation(24998.374999393650 , 18749.99993365020, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(24998.374999393650 , 18749.99993365020, pWorld),
                                         HGF2DLocation(24998.375000608570 , 0.100067755764000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(24998.375000608570 , 0.100067755764000, pWorld),
                                         HGF2DLocation(25000.000000000000 , 0.100067755688000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.000000000000 , 0.100067755688000, pWorld),
                                         HGF2DLocation(25000.000000000000 , 0.000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.000000000000 , 0.000000000000000, pWorld),
                                         HGF2DLocation(24998.375000170640 , 0.000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    pShape1->SetAutoToleranceActive(false);
    pShape1->SetTolerance(1E-8);

    HFCPtr<HVE2DShape> pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pTheOtherWorld));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
    
    pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pTheOtherWorld2));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
       
    }

//==================================================================================
// Additional AllocateCopyInCoordSys tests
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester, AllocateCopyInCoordSysTestWhoFailed9)
    {
           
    HFCPtr<HGF2DCoordSys>   pTheOtherWorld     = new HGF2DCoordSys(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 100.0, 100.0), pWorld);
    HFCPtr<HGF2DCoordSys>   pTheOtherWorld2    = new HGF2DCoordSys(HGF2DStretch(HGF2DDisplacement(0.0, 0.0),  1000.0, 1000.0), pWorld);

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0 , 0.0000000000, pWorld),
                                         HGF2DLocation(0.00000 , 0.0000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000 , 0.0000000000, pWorld),
                                         HGF2DLocation(0.00000 , 18000.000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000 , 18000.000000, pWorld),
                                         HGF2DLocation(12000.0 , 18000.000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(12000.0 , 18000.000000, pWorld),
                                         HGF2DLocation(12000.0 , 50*MYEPSILON, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(12000.0 , 50*MYEPSILON, pWorld),
                                         HGF2DLocation(18000.0 , 50*MYEPSILON, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(18000.0 , 50*MYEPSILON, pWorld),
                                         HGF2DLocation(18000.0 , 18000.000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(18000.0 , 18000.000000, pWorld),
                                         HGF2DLocation(25000.0 , 18000.000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0 , 18000.000000, pWorld),
                                         HGF2DLocation(25000.0 , 0.0000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HFCPtr<HVE2DShape> pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pTheOtherWorld));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
    
    pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pTheOtherWorld2));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
        
    }

//==================================================================================
// Additional AllocateCopyInCoordSys tests
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester, AllocateCopyInCoordSysTestWhoFailed10)
    {
          
    HFCPtr<HGF2DCoordSys>   pTheOtherWorld     = new HGF2DCoordSys(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 100.0, 100.0), pWorld);
    HFCPtr<HGF2DCoordSys>   pTheOtherWorld2    = new HGF2DCoordSys(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 1000.0, 1000.0), pWorld);

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0 , 0.0000000000, pWorld),
                                         HGF2DLocation(12000.0 , 0.0000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(12000.0 , 0.0000000000, pWorld),
                                         HGF2DLocation(0.00000 , 0.0000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000 , 0.0000000000, pWorld),
                                         HGF2DLocation(0.00000 , 18000.000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000 , 18000.000000, pWorld),
                                         HGF2DLocation(12000.0 , 18000.000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(12000.0 , 18000.000000, pWorld),
                                         HGF2DLocation(12000.0 , 50*MYEPSILON, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(12000.0 , 50*MYEPSILON, pWorld),
                                         HGF2DLocation(18000.0 , 50*MYEPSILON, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(18000.0 , 50*MYEPSILON, pWorld),
                                         HGF2DLocation(18000.0 , 18000.000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(18000.0 , 18000.000000, pWorld),
                                         HGF2DLocation(25000.0 , 18000.000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0 , 18000.000000, pWorld),
                                         HGF2DLocation(25000.0 , 0.0000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HFCPtr<HVE2DShape> pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pTheOtherWorld));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
    
    pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pTheOtherWorld2));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());  
    
    }

//==================================================================================
// Additional AllocateCopyInCoordSys tests
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester, AllocateCopyInCoordSysTestWhoFailed11)
    {
            
    HFCPtr<HGF2DCoordSys>   pTheOtherWorld     = new HGF2DCoordSys(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 100.0, 100.0), pWorld);
    HFCPtr<HGF2DCoordSys>   pTheOtherWorld2    = new HGF2DCoordSys(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 1000.0, 1000.0), pWorld);

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0 , -10000.00000, pWorld),
                                         HGF2DLocation(15000.0 , -10000.00000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(15000.0 , -10000.00000, pWorld),
                                         HGF2DLocation(15000.0 , 0.0000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(15000.0 , 0.0000000000, pWorld),
                                         HGF2DLocation(0.00000 , 0.0000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000 , 0.0000000000, pWorld),
                                         HGF2DLocation(0.00000 , 18000.000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000 , 18000.000000, pWorld),
                                         HGF2DLocation(12000.0 , 18000.000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(12000.0 , 18000.000000, pWorld),
                                         HGF2DLocation(12000.0 , 50*MYEPSILON, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(12000.0 , 50*MYEPSILON, pWorld),
                                         HGF2DLocation(18000.0 , 50*MYEPSILON, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(18000.0 , 50*MYEPSILON, pWorld),
                                         HGF2DLocation(18000.0 , 18000.000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(18000.0 , 18000.000000, pWorld),
                                         HGF2DLocation(25000.0 , 18000.000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0 , 18000.000000, pWorld),
                                         HGF2DLocation(25000.0 , -10000.00000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HFCPtr<HVE2DShape> pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pTheOtherWorld));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
    
    pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pTheOtherWorld2));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
        
    }

//==================================================================================
// Additional AllocateCopyInCoordSys tests
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester, AllocateCopyInCoordSysTestWhoFailed12)
    {
   
    HFCPtr<HGF2DCoordSys>   pTheOtherWorld     = new HGF2DCoordSys(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 100.0, 100.0), pWorld);
    HFCPtr<HGF2DCoordSys>   pTheOtherWorld2    = new HGF2DCoordSys(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 1000.0, 1000.0), pWorld);

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0 , -10000.00000, pWorld),
                                         HGF2DLocation(20000.0 , -10000.00000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(20000.0 , -10000.00000, pWorld),
                                         HGF2DLocation(20000.0 , 0.0000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(20000.0 , 0.0000000000, pWorld),
                                         HGF2DLocation(15000.0 , 0.0000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(15000.0 , 0.0000000000, pWorld),
                                         HGF2DLocation(15000.0 , -10000.00000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(15000.0 , -10000.00000, pWorld),
                                         HGF2DLocation(0.00000 , -10000.00000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000 , -10000.00000, pWorld),
                                         HGF2DLocation(0.00000 , 18000.000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000 , 18000.000000, pWorld),
                                         HGF2DLocation(12000.0 , 18000.000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(12000.0 , 18000.000000, pWorld),
                                         HGF2DLocation(12000.0 , 50*MYEPSILON, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(12000.0 , 50*MYEPSILON, pWorld),
                                         HGF2DLocation(18000.0 , 50*MYEPSILON, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(18000.0 , 50*MYEPSILON, pWorld),
                                         HGF2DLocation(18000.0 , 18000.000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(18000.0 , 18000.000000, pWorld),
                                         HGF2DLocation(25000.0 , 18000.000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.0 , 18000.000000, pWorld),
                                         HGF2DLocation(25000.0 , -10000.00000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HFCPtr<HVE2DShape> pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pTheOtherWorld));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
    
    pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pTheOtherWorld2));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
     
    }

//==================================================================================
// Additional AllocateCopyInCoordSys tests
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester, AllocateCopyInCoordSysTestWhoFailed13)
    {
         
    HFCPtr<HGF2DCoordSys>   pTheOtherWorld     = new HGF2DCoordSys(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 100.0, 100.0), pWorld);
    HFCPtr<HGF2DCoordSys>   pTheOtherWorld2    = new HGF2DCoordSys(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 1000.0, 1000.0), pWorld);
    
    HVE2DComplexLinear  TheLinear1(pWorld);
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.000000 , -10000.0, pWorld),
                                         HGF2DLocation(0.0000000000 , -10000.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000 , -10000.0, pWorld),
                                         HGF2DLocation(0.0000000000 , 18000.00, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000 , 18000.00, pWorld),
                                         HGF2DLocation(25000.000000 , 18000.00, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.000000 , 18000.00, pWorld),
                                         HGF2DLocation(25000.000000 , 12000.00, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.000000 , 12000.00, pWorld),
                                         HGF2DLocation(50*MYEPSILON , 12000.00, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(50*MYEPSILON , 12000.00, pWorld),
                                         HGF2DLocation(50*MYEPSILON , 11000.00, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(50*MYEPSILON , 11000.00, pWorld),
                                         HGF2DLocation(25000.000000 , 11000.00, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.000000 , 11000.00, pWorld),
                                         HGF2DLocation(25000.000000 , 0.000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.000000 , 0.000000, pWorld),
                                         HGF2DLocation(50*MYEPSILON , 0.000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(50*MYEPSILON , 0.000000, pWorld),
                                         HGF2DLocation(50*MYEPSILON , -5000.00, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(50*MYEPSILON , -5000.00, pWorld),
                                         HGF2DLocation(25000.000000 , -5000.00, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(25000.000000 , -5000.00, pWorld),
                                         HGF2DLocation(25000.000000 , -10000.0, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HFCPtr<HVE2DShape> pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pTheOtherWorld));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(3, pResult->GetShapeList().size());
    
    pResult = static_cast<HVE2DShape*>(pShape1->AllocateCopyInCoordSys(pTheOtherWorld2));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(3, pResult->GetShapeList().size());
       
    }

//==================================================================================
// Test which failed on August 22, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed60)
    {
          
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-16000.00000000007, -21754.75510203093, pWorld),
                                         HGF2DLocation(-16000.00000000007, 12000.000000061850, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-16000.00000000007, 12000.000000061850, pWorld),
                                         HGF2DLocation(34245.244897959170, 12000.000000122160, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(34245.244897959170, 12000.000000122160, pWorld),
                                         HGF2DLocation(34245.244897959170, -21754.75510197065, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(34245.244897959170, -21754.75510197065, pWorld),
                                         HGF2DLocation(-16000.00000000007, -21754.75510203093, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000, 0.00000, pWorld),
                                         HGF2DLocation(0.00000, 12000.0, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000, 12000.0, pWorld),
                                         HGF2DLocation(16000.0, 12000.0, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(16000.0, 12000.0, pWorld),
                                         HGF2DLocation(16000.0, 0.00000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(16000.0, 0.00000, pWorld),
                                         HGF2DLocation(0.00000, 0.00000, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(192000000.00000, pResult->CalculateArea());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(192000000.00000, pResult->CalculateArea());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(1696015936.5752916, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(1696015936.572773, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(1504015936.572278, pResult->CalculateArea());
    
    pResult = pShape2->DifferentiateShape(*pShape1); 
    ASSERT_TRUE(pResult->IsEmpty());
       
    }

//==================================================================================
// Parallel copy test (MartinBu 25 august 2000)
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester, AllocateParallelCopyTestWhoFailed)
    {

    HVE2DPolySegment PolySegment(pWorld);

    HGF2DLocation Pt1(496871.025, 3519368.775 ,pWorld);
    PolySegment.AppendPoint(Pt1);

    HGF2DLocation Pt2(496871.025, 3520443.975 ,pWorld);
    PolySegment.AppendPoint(Pt2);

    HGF2DLocation Pt3(498791.025, 3520443.975 ,pWorld);
    PolySegment.AppendPoint(Pt3);

    HGF2DLocation Pt4(498791.025, 3519368.775 ,pWorld);
    PolySegment.AppendPoint(Pt4);

    HGF2DLocation Pt5(496871.025, 3519368.775 ,pWorld);
    PolySegment.AppendPoint(Pt5);

    HVE2DPolygonOfSegments OriginalPolygon(PolySegment);

    HVE2DVector::ArbitraryDirection Dir = HVE2DVector::ALPHA;

    double Offset(5.0);

    HFCPtr<HVE2DPolygonOfSegments> NewPolygon = OriginalPolygon.AllocateParallelCopy(Offset, Dir);

    }

//==================================================================================
// Test which failed on Sept 6, 2000 (BUG #4737)
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed61)
    {
           
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1131.70849898475900 , 0.00000000000000000, pWorld),
                                         HGF2DLocation(0.00000000000000000 , 1131.70849898476100, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , 1131.70849898476100, pWorld),
                                         HGF2DLocation(331.708498984757400 , 1462.41699796951700, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(331.708498984757400 , 1462.41699796951700, pWorld),
                                         HGF2DLocation(331.708498984757400 , 800.000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(331.708498984757400 , 800.000000000000000, pWorld),
                                         HGF2DLocation(1931.70849898475600 , 800.000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1931.70849898475600 , 800.000000000000000, pWorld),
                                         HGF2DLocation(1931.70849898475600 , 1268.29150101523500, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1931.70849898475600 , 1268.29150101523500, pWorld),
                                         HGF2DLocation(2028.27124746189700 , 1365.85424949237600, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2028.27124746189700 , 1365.85424949237600, pWorld),
                                         HGF2DLocation(2262.00000000000000 , 1132.12549695427300, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2262.00000000000000 , 1132.12549695427300, pWorld),
                                         HGF2DLocation(2262.00000000000000 , 1130.29150101524100, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2262.00000000000000 , 1130.29150101524100, pWorld),
                                         HGF2DLocation(1131.70849898475900 , 0.00000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(363.70849898475970 , 768.00000000000000, pWorld),
                                         HGF2DLocation(223.99999999999890 , 907.70849898476080, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(223.99999999999890 , 907.70849898476080, pWorld),
                                         HGF2DLocation(223.99999999999890 , 1024.00000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(223.99999999999890 , 1024.00000000000000, pWorld),
                                         HGF2DLocation(479.99999999999890 , 1024.00000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(479.99999999999890 , 1024.00000000000000, pWorld),
                                         HGF2DLocation(479.99999999999890 , 768.00000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(479.99999999999890 , 768.00000000000000, pWorld),
                                         HGF2DLocation(363.70849898475970 , 768.00000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);
    
    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->UnifyShape(*pShape2);
        
    #ifdef WIP_IPPTEST_BUG_3
    ASSERT_FALSE(pResult->IsEmpty()); 
    #endif
   
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    }

//==================================================================================
// Test which failed on Sept 6, 2000 (BUG #4738)
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed62)
    {
          
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1697.56274847713700 , 0.00000000000000000, pWorld),
                                         HGF2DLocation(0.00000000000000000 , 1697.56274847714300, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , 1697.56274847714300, pWorld),
                                         HGF2DLocation(497.562748477141200 , 2194.12549695428200, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(497.562748477141200 , 2194.12549695428200, pWorld),
                                         HGF2DLocation(497.562748477141200 , 1199.99999999999800, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(497.562748477141200 , 1199.99999999999800, pWorld),
                                         HGF2DLocation(2097.56274847714100 , 1199.99999999999800, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2097.56274847714100 , 1199.99999999999800, pWorld),
                                         HGF2DLocation(2097.56274847714100 , 1297.56274847713700, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2097.56274847714100 , 1297.56274847713700, pWorld),
                                         HGF2DLocation(2194.12549695428200 , 1199.99999999999500, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2194.12549695428200 , 1199.99999999999500, pWorld),
                                         HGF2DLocation(3042.40687119285800 , 2048.28137423857100, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(3042.40687119285800 , 2048.28137423857100, pWorld),
                                         HGF2DLocation(3394.00000000000000 , 1697.68824543141900, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(3394.00000000000000 , 1697.68824543141900, pWorld),
                                         HGF2DLocation(3394.00000000000000 , 1696.43725152285400, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(3394.00000000000000 , 1696.43725152285400, pWorld),
                                         HGF2DLocation(1697.56274847713700 , 0.000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(624.00000000000450 , 1073.56274847713700, pWorld),
                                         HGF2DLocation(417.56274847714350 , 1280.00000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(417.56274847714350 , 1280.00000000000000, pWorld),
                                         HGF2DLocation(624.00000000000450 , 1280.00000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(624.00000000000450 , 1280.00000000000000, pWorld),
                                         HGF2DLocation(624.00000000000450 , 1073.56274847713700, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());    

    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->UnifyShape(*pShape2);

    #ifdef WIP_IPPTEST_BUG_3
    ASSERT_FALSE(pResult->IsEmpty());
    #endif   

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());    
    
    }

//==================================================================================
// Test which failed on Sept 7, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed63)
    {
           
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(76.00, 0.000, pWorld), HGF2DLocation(76.00, 5.000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(76.00, 5.000, pWorld), HGF2DLocation(70.00, 5.000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(70.00, 5.000, pWorld), HGF2DLocation(70.00, 0.000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(70.00, 0.000, pWorld), HGF2DLocation(0.000, 0.000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000, 0.000, pWorld), HGF2DLocation(0.000, 256.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000, 256.0, pWorld), HGF2DLocation(256.0, 256.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0, 256.0, pWorld), HGF2DLocation(256.0, 0.000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0, 0.000, pWorld), HGF2DLocation(76.00, 0.000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DRectangle(0.0, 0.0, 256.0, 256.0, pWorld);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(65506.0, pResult->CalculateArea());   

    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(65506.0, pResult->CalculateArea());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(65536.0, pResult->CalculateArea());    

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(65536.0, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_TRUE(pResult->IsEmpty());   

    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(30.0, pResult->CalculateArea());   

    // Other additional tests
    HVE2DPolygonOfSegments MySameRect(HVE2DRectangle(0.0, 0.0, 256.0, 256.0, pWorld));

    ASSERT_EQ(HVE2DShape::S_IN, MySameRect.CalculateSpatialPositionOf(*pShape1));
    ASSERT_EQ(HVE2DShape::S_OUT, pShape1->CalculateSpatialPositionOf(MySameRect));
    ASSERT_EQ(HVE2DShape::S_IN, pShape2->CalculateSpatialPositionOf(*pShape1));
    ASSERT_EQ(HVE2DShape::S_OUT, pShape1->CalculateSpatialPositionOf(*pShape2));
    
    }

//==================================================================================
// Test which failed on Sept 19, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed64)
    {
            
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(224.0, 800.00, pWorld), HGF2DLocation(480.0, 1056.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(480.0, 1056.0, pWorld), HGF2DLocation(736.0, 800.00, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(736.0, 800.00, pWorld), HGF2DLocation(480.0, 544.00, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(480.0, 544.00, pWorld), HGF2DLocation(224.0, 800.00, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DRectangle(0.0, 512.0, 256.0, 768.0, pWorld);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_TRUE(pResult->IsEmpty());   

    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_TRUE(pResult->IsEmpty());    

    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    // Other additional tests
    ASSERT_EQ(HVE2DShape::S_OUT, pShape2->CalculateSpatialPositionOf(*pShape1));
    ASSERT_EQ(HVE2DShape::S_OUT, pShape1->CalculateSpatialPositionOf(*pShape2));
    
    }

//==================================================================================
// Test which failed on Nov 9, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed65)
    {
           
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1030.762349323, 255.9999998882, pWorld),
                                         HGF2DLocation(778.9140000000, 255.9999999255, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(778.9140000000, 255.9999999255, pWorld),
                                         HGF2DLocation(778.1236900000, 277.1485400000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(778.1236900000, 277.1485400000, pWorld),
                                         HGF2DLocation(1029.205400000, 282.0669100000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1029.205400000, 282.0669100000, pWorld),
                                         HGF2DLocation(1030.762349323, 255.9999998882, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DRectangle(0.0, 0.0, 4251.0, 256.0, pWorld);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
       
    }

//==================================================================================
// Test which failed on Nov 15, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed66)
    {
           
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.008822139352560 , 0.1213040202856100, pWorld),
                                         HGF2DLocation(0.007792119868100 , 253.40242036432030, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.007792119868100 , 253.40242036432030, pWorld),
                                         HGF2DLocation(233.0057656611316 , 240.64462127164010, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(233.0057656611316 , 240.64462127164010, pWorld),
                                         HGF2DLocation(219.3766127480194 , 0.1214403659105300, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(219.3766127480194 , 0.1214403659105300, pWorld),
                                         HGF2DLocation(0.008822139352560 , 0.1213040202856100, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    pShape1->SetAutoToleranceActive(false);
    pShape1->SetTolerance(0.001);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00812306855621 , 0.0000000000000000, pWorld),
                                         HGF2DLocation(0.00892448239028 , 50.122100748121740, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00892448239028 , 50.122100748121740, pWorld),
                                         HGF2DLocation(0.00582597115946 , 256.00000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00582597115946 , 256.00000000000000, pWorld),
                                         HGF2DLocation(256.000000000000 , 256.00000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000000000 , 256.00000000000000, pWorld),
                                         HGF2DLocation(256.000000000000 , 0.0000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000000000 , 0.0000000000000000, pWorld),
                                         HGF2DLocation(0.00812306855621 , 0.0000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
        
    }

//==================================================================================
// Test which failed on Nov 16, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed67)
    {
            
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , 0.00000000000000000, pWorld),
                                         HGF2DLocation(0.00000000000000000 , 256.000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , 256.000000000000000, pWorld),
                                         HGF2DLocation(256.000000000000000 , 256.000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000000000000 , 256.000000000000000, pWorld),
                                         HGF2DLocation(256.000000000000000 , 0.00000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000000000000 , 0.00000000000000000, pWorld),
                                         HGF2DLocation(0.00000000000000000 , 0.00000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-0.000000001164150 , -255.99924341682340, pWorld),
                                         HGF2DLocation(0.0004448744584800 , 874.514387526549400, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0004448744584800 , 874.514387526549400, pWorld),
                                         HGF2DLocation(1507.3702350749520 , 874.513630896806700, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1507.3702350749520 , 874.513630896806700, pWorld),
                                         HGF2DLocation(1507.3697938015100 , -256.00000000465660, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1507.3697938015100 , -256.00000000465660, pWorld),
                                         HGF2DLocation(-0.000000001164150 , -255.99924341682340, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    pShape2->SetAutoToleranceActive(false);
    pShape2->SetTolerance(0.001);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
      
    }

//==================================================================================
// Test which failed on Nov 16, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  IsAutoContiguousTestWhoFailed2)
    {
    
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146612.616979246520000 , -3733745.133152585500000, pWorld),
                                         HGF2DLocation(-146610.838938417060000 , -3733745.029725480400000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146610.838938417060000 , -3733745.029725480400000, pWorld),
                                         HGF2DLocation(-146612.480445448800000 , -3733716.810040059500000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146612.480445448800000 , -3733716.810040059500000, pWorld),
                                         HGF2DLocation(-146614.121941842870000 , -3733688.590353258400000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146614.121941842870000 , -3733688.590353258400000, pWorld),
                                         HGF2DLocation(-146614.760026228150000 , -3733677.620692211700000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146614.760026228150000 , -3733677.620692211700000, pWorld),
                                         HGF2DLocation(-146614.760025639170000 , -3733677.620692211700000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146614.760025639170000 , -3733677.620692211700000, pWorld),
                                         HGF2DLocation(-146612.878627912110000 , -3733709.964706689100000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146612.878627912110000 , -3733709.964706689100000, pWorld),
                                         HGF2DLocation(-146610.865070915780000 , -3733744.580448843500000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146610.865070915780000 , -3733744.580448843500000, pWorld),
                                         HGF2DLocation(-146610.838938297970000 , -3733745.029725568800000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-146610.838938297970000 , -3733745.029725568800000, pWorld),
                                         HGF2DLocation(-146612.616979246520000 , -3733745.133152585500000, pWorld)));

    HVE2DPolygonOfSegments MyPolygon(TheLinear1);

    HVE2DPolySegment  AddPolySegment1(pWorld);

    AddPolySegment1.AppendPoint(HGF2DLocation(24.624353316877436 , 0.004425250060435, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398305383578645 , 0.004425069108003, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398301869392405 , 19.319186263555068, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398301467879172 , 22.538313116279927, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398303852609480 , 38.633947080613552, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398305174882111 , 45.072200640873156, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398309209308998 , 54.729580936149411, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398305913017346 , 64.386961540812607, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398305256362420 , 77.263468886572440, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398305146288891 , 86.920849370524138, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398305846564291 , 90.139976147999874, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398309950957461 , 103.01648316980146, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398312240216100 , 109.45473670769246, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398309812264650 , 115.89299048231651, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398308924056224 , 119.11211736887199, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398308470115047 , 128.76949787626017, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398308374657361 , 141.64600518417248, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398312152666570 , 154.99632270698271, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398324268477438 , 154.99632200212196, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398314934021926 , 120.00442475209485, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398319692574731 , 112.50442480907817, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398322057602797 , 108.75442479488775, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398321363245735 , 105.00442496174850, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398318585817435 , 90.004425340236082, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398313030960594 , 60.004424714223930, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398317191746997 , 52.799085740268119, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398316444880379 , 45.593746993757655, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398314951205229 , 31.183069183142031, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398311963796687 , 2.3617122817676420, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(37.398305487265958 , 0.0044248682430170, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(36.551915525778384 , 0.0044246584874480, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(24.624353301068627 , 0.0044249780558200, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(24.624353316877436 , 0.0044252500604350, pWorld));

    AddPolySegment1.SetAutoToleranceActive(false);
    AddPolySegment1.SetTolerance(0.000001);

    ASSERT_TRUE(AddPolySegment1.IsAutoContiguous());

    AddPolySegment1.RemoveAutoContiguousNeedles(true);
    ASSERT_FALSE(AddPolySegment1.IsAutoContiguous());
       
    }

//==================================================================================
// Test which failed on Dec 8, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed68)
    {
    
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000, 93.657950460676, pWorld),
                                         HGF2DLocation(10.267621787879, 256.00000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(10.267621787879, 256.00000000000, pWorld),
                                         HGF2DLocation(256.00000000000, 256.00000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.00000000000, 256.00000000000, pWorld),
                                         HGF2DLocation(256.00000000000, 0.0000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.00000000000, 0.0000000000000, pWorld),
                                         HGF2DLocation(0.0000000000000, 0.0000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000, 0.0000000000000, pWorld),
                                         HGF2DLocation(0.0000000000000, 93.657950460676, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DPolySegment  AddPolySegment1(pWorld);

    AddPolySegment1.AppendPoint(HGF2DLocation(254.29769587355, 58.655061377831000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(256.00000000129, 58.547396021562000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(256.00000000037, 1.1175872057802E-8, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(250.58794853646, 7.4505813718678E-9, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(254.29769587355, 58.655061377831000, pWorld));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(AddPolySegment1);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_TRUE(pResult->IsEmpty());
          
    }

//==================================================================================
// Test which failed on Mar 22, 2001
//==================================================================================
  TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed69)
    {
    
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-187954649.101742240000000 , -778941047.647742270000000, pWorld),
                                         HGF2DLocation(-183717124.331314270000000 , -778941047.647742270000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-183717124.331314270000000 , -778941047.647742270000000, pWorld),
                                         HGF2DLocation(-183717124.331314270000000 , -779490571.969578980000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-183717124.331314270000000 , -779490571.969578980000000, pWorld),
                                         HGF2DLocation(-184667877.706137210000000 , -779387165.366099950000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-184667877.706137210000000 , -779387165.366099950000000, pWorld),
                                         HGF2DLocation(-186085451.411206330000000 , -780017587.359324810000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-186085451.411206330000000 , -780017587.359324810000000, pWorld),
                                         HGF2DLocation(-187293014.196996960000000 , -780542939.020325180000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-187293014.196996960000000 , -780542939.020325180000000, pWorld),
                                         HGF2DLocation(-188400179.033274800000000 , -781169114.454348090000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-188400179.033274800000000 , -781169114.454348090000000, pWorld),
                                         HGF2DLocation(-188400179.033274800000000 , -780808326.825356360000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-188400179.033274800000000 , -780808326.825356360000000, pWorld),
                                         HGF2DLocation(-185717932.302468900000000 , -779229559.867793800000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-185717932.302468900000000 , -779229559.867793800000000, pWorld),
                                         HGF2DLocation(-188400179.033274800000000 , -779889163.350910190000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-188400179.033274800000000 , -779889163.350910190000000, pWorld),
                                         HGF2DLocation(-188400179.033274800000000 , -779061072.119485860000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-188400179.033274800000000 , -779061072.119485860000000, pWorld),
                                         HGF2DLocation(-187954649.101742240000000 , -778941047.647742270000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DPolySegment  AddPolySegment1(pWorld);

    AddPolySegment1.AppendPoint(HGF2DLocation(-186257310.228082480000000 , -779547037.017389420000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-186903365.889312740000000 , -779927304.544230340000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-187571727.079627280000000 , -780595665.734544870000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-187504720.042213920000000 , -780662672.771958230000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-187293014.196999280000000 , -780542939.020335320000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-186085451.411205110000000 , -780017587.359333630000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-185878704.271933910000000 , -779925642.973537920000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-186257310.228082480000000 , -779547037.017389420000000, pWorld));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(AddPolySegment1);

    // This tests DOES NOT PASS WHEN EPSILON MULTIPLICATOR is 1E-14: 1E-13 is required
    if (HNumeric<double>::EPSILON_MULTIPLICATOR() < 1E-13)
        {
        pShape2->SetAutoToleranceActive(false);
        pShape2->SetTolerance(1E-4);
        pShape1->SetAutoToleranceActive(false);
        pShape1->SetTolerance(1E-4);
        }

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
   
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_TRUE(pResult->IsEmpty());
            
    }

//==================================================================================
// Test which failed on Dec 8, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed70)
    {
       
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(16712.962887222551000 , 256.000000000000000, pWorld),
                                         HGF2DLocation(16703.446371366757000 , 0.18155755210318600, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(16703.446371366757000 , 0.18155755210318600, pWorld),
                                         HGF2DLocation(9826.6569572419812000 , 256.000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(9826.6569572419812000 , 256.000000000000000, pWorld),
                                         HGF2DLocation(16712.962887222551000 , 256.000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DPolySegment  AddPolySegment1(pWorld);

    AddPolySegment1.AppendPoint(HGF2DLocation(16640.840166126243000 , 334.740166727452280, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(16715.788325136309000 , 331.952074763648450, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(16706.271637899998000 , 76.1290253463482710, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(16631.323478889932000 , 78.9171173101520650, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(16640.840166126243000 , 334.740166727452280, pWorld));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(AddPolySegment1);

    // This tests DOES NOT PASS WHEN EPSILON is 1E-8: 1E-7 is required
    if (HNumeric<double>::GLOBAL_EPSILON() < 1E-7)
        {     
        pShape2->SetAutoToleranceActive(false);
        pShape2->SetTolerance(1E-7);
        pShape1->SetAutoToleranceActive(false);
        pShape1->SetTolerance(1E-7);
        }

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
       
    }   

//==================================================================================
// Test which failed on Dec 8, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed71)
    {

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000, 93.579504606760, pWorld),
                                         HGF2DLocation(10.676217878790, 256.00000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(10.676217878790, 256.00000000000, pWorld),
                                         HGF2DLocation(256.00000000000, 256.00000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.00000000000, 256.00000000000, pWorld),
                                         HGF2DLocation(256.00000000000, 0.0000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.00000000000, 0.0000000000000, pWorld),
                                         HGF2DLocation(0.0000000000000, 0.0000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000, 0.0000000000000, pWorld),
                                         HGF2DLocation(0.0000000000000, 93.579504606760, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DPolySegment  AddPolySegment1(pWorld);

    AddPolySegment1.AppendPoint(HGF2DLocation(254.9769587355, 58.550613778310000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(256.0000000129, 58.473960215620000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(256.0000000037, 1.1175872057802E-7, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(250.8794853646, 7.4505813718678E-8, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(254.9769587355, 58.550613778310000, pWorld));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(AddPolySegment1);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_TRUE(pResult->IsEmpty());
    
    }

//==================================================================================
// Test which failed on Mar 22, 2001
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed72)
    {
         
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-187954649.01742240000000 , -778941047.47742270000000, pWorld),
                                         HGF2DLocation(-183717124.31314270000000 , -778941047.47742270000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-183717124.31314270000000 , -778941047.47742270000000, pWorld),
                                         HGF2DLocation(-183717124.31314270000000 , -779490571.69578980000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-183717124.31314270000000 , -779490571.69578980000000, pWorld),
                                         HGF2DLocation(-184667877.06137210000000 , -779387165.66099950000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-184667877.06137210000000 , -779387165.66099950000000, pWorld),
                                         HGF2DLocation(-186085451.11206330000000 , -780017587.59324810000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-186085451.11206330000000 , -780017587.59324810000000, pWorld),
                                         HGF2DLocation(-187293014.96996960000000 , -780542939.20325180000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-187293014.96996960000000 , -780542939.20325180000000, pWorld),
                                         HGF2DLocation(-188400179.33274800000000 , -781169114.54348090000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-188400179.33274800000000 , -781169114.54348090000000, pWorld),
                                         HGF2DLocation(-188400179.33274800000000 , -780808326.25356360000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-188400179.33274800000000 , -780808326.25356360000000, pWorld),
                                         HGF2DLocation(-185717932.02468900000000 , -779229559.67793800000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-185717932.02468900000000 , -779229559.67793800000000, pWorld),
                                         HGF2DLocation(-188400179.33274800000000 , -779889163.50910190000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-188400179.33274800000000 , -779889163.50910190000000, pWorld),
                                         HGF2DLocation(-188400179.33274800000000 , -779061072.19485860000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-188400179.33274800000000 , -779061072.19485860000000, pWorld),
                                         HGF2DLocation(-187954649.01742240000000 , -778941047.47742270000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DPolySegment  AddPolySegment1(pWorld);

    AddPolySegment1.AppendPoint(HGF2DLocation(-186257310.228082480000000 , -779547037.017389420000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-186903365.889312740000000 , -779927304.544230340000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-187571727.079627280000000 , -780595665.734544870000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-187504720.042213920000000 , -780662672.771958230000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-187293014.196999280000000 , -780542939.020335320000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-186085451.411205110000000 , -780017587.359333630000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-185878704.271933910000000 , -779925642.973537920000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-186257310.228082480000000 , -779547037.017389420000000, pWorld));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(AddPolySegment1);
    
    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
       
    }

//==================================================================================
// Test which failed on Dec 8, 2000
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed73)
    {
           
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(16712.62887222551000 , 256.000000000000000, pWorld),
                                         HGF2DLocation(16703.46371366757000 , 0.81557552103186000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(16703.46371366757000 , 0.81557552103186000, pWorld),
                                         HGF2DLocation(9826.569572419812000 , 256.000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(9826.569572419812000 , 256.000000000000000, pWorld),
                                         HGF2DLocation(16712.62887222551000 , 256.000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DPolySegment  AddPolySegment1(pWorld);

    AddPolySegment1.AppendPoint(HGF2DLocation(16640.40166126243000 , 334.40166727452280, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(16715.88325136309000 , 331.52074763648450, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(16706.71637899998000 , 76.290253463482710, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(16631.23478889932000 , 78.171173101520650, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(16640.40166126243000 , 334.40166727452280, pWorld));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(AddPolySegment1);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
       
    }

//==================================================================================
// Test which failed on Nov 6, 2001  TR75930
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  WIP_IPPTEST_BUG_4)
    {
   
    #ifdef WIP_IPPTEST_BUG_4
        
    HVE2DPolySegment  AddPolySegment1(pWorld);
    AddPolySegment1.AppendPoint(HGF2DLocation(343461467.000000060000000 , 517042351.999999940000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(344815973.470631420000000 , 515687845.529368520000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(346694630.449381350000000 , 517566502.508118390000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(345340123.978749870000000 , 518921008.978749810000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(343461467.000000060000000 , 517042351.999999940000000, pWorld));
    
    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(AddPolySegment1);

    HVE2DPolySegment  AddPolySegment2(pWorld);
    AddPolySegment2.AppendPoint(HGF2DLocation(343461467.000000060000000 , 517042351.999999880000000, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(344815973.470683040000000 , 515687845.529316840000000, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(346630485.970608830000000 , 517502358.029242580000000, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(345275979.499925730000000 , 518856864.499925610000000, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(343461467.000000060000000 , 517042351.999999880000000, pWorld));
        
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(AddPolySegment2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    pResult = pShape1->IntersectShape(*pShape2);

    #endif

    }

//==================================================================================
// Test which failed on Nov 6, 2001  TR75930
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  WIP_IPPTEST_BUG_4_2)
    {
    
    #ifdef WIP_IPPTEST_BUG_4
        
    HVE2DPolySegment  AddPolySegment1(pWorld);
    AddPolySegment1.AppendPoint(HGF2DLocation(343461467.000000060000000 , 517042351.999999940000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(344815973.470631420000000 , 515687845.529368520000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(346694630.449381350000000 , 517566502.508118390000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(345340123.978749870000000 , 518921008.978749810000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(343461467.000000060000000 , 517042351.999999940000000, pWorld));
    
    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(AddPolySegment1);

    HVE2DPolySegment  AddPolySegment2(pWorld);
    AddPolySegment2.AppendPoint(HGF2DLocation(343461467.000000060000000 , 517042351.999999880000000, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(344815973.470683040000000 , 515687845.529316840000000, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(346630485.970608830000000 , 517502358.029242580000000, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(345275979.499925730000000 , 518856864.499925610000000, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(343461467.000000060000000 , 517042351.999999880000000, pWorld));
    
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(AddPolySegment2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    pResult = pShape1->IntersectShape(*pShape2);

    HVE2DPolySegment  AddPolySegment3(pWorld);
    AddPolySegment3.AppendPoint(HGF2DLocation(2981.304621487855900 , 2177.617096203845000, pWorld));
    AddPolySegment3.AppendPoint(HGF2DLocation(3061.379443119512900 , 2290.133409291971500, pWorld));
    AddPolySegment3.AppendPoint(HGF2DLocation(2940.157848385162700 , 2383.015966769307900, pWorld));
    AddPolySegment3.AppendPoint(HGF2DLocation(2861.816244216635800 , 2269.171625692397400, pWorld));
    AddPolySegment3.AppendPoint(HGF2DLocation(2981.304621487855900 , 2177.617096203845000, pWorld));
        
    HFCPtr<HVE2DShape> pShape3 = new HVE2DPolygonOfSegments(AddPolySegment3);

    HVE2DPolySegment  AddPolySegment4(pWorld);
    AddPolySegment4.AppendPoint(HGF2DLocation(2981.304621465271300 , 2177.617095963563800, pWorld));
    AddPolySegment4.AppendPoint(HGF2DLocation(3061.379443096928300 , 2290.133409049827600, pWorld));
    AddPolySegment4.AppendPoint(HGF2DLocation(2940.157848363625800 , 2383.015966524835700, pWorld));
    AddPolySegment4.AppendPoint(HGF2DLocation(2861.816244194866200 , 2269.171625450253500, pWorld));
    AddPolySegment4.AppendPoint(HGF2DLocation(2981.304621465271300 , 2177.617095963563800, pWorld));
        
    HFCPtr<HVE2DShape> pShape4 = new HVE2DPolygonOfSegments(AddPolySegment4);

    HFCPtr<HVE2DShape> pResult2 = pShape4->IntersectShape(*pShape3);
    pResult2 = pShape3->IntersectShape(*pShape4);
        
    #endif
    }

//==================================================================================
// Test which failed on Dec 16, 2003
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  IntersectShapeWithAppendPointWhoFailed)
    {
            
    HVE2DPolySegment  AddPolySegment1(pWorld);
    AddPolySegment1.AppendPoint(HGF2DLocation(-0.000000000000057 , 299.815021846634070, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-0.000000000000057 , -0.0000000000001140, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(2.7832031249999430 , -0.0000000000003410, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(4.1748046875001140 , 0.00000000000022700, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(9.7412109374999720 , -0.0000000000003410, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(13.916015625000028 , 0.00000000000011400, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(22.265624999999972 , -0.0000000000003410, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(25.048828125000000 , 0.00000000000011400, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(29.223632812499915 , -0.0000000000001140, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(32.006835937500000 , 0.00000000000011400, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(33.398437500000000 , 0.00000000000000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(35.485839843750057 , 0.00000000000022700, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(42.443847656250014 , -0.0000000000003410, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(43.139648437499986 , 0.00000000000000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(45.922851562500014 , 0.00000000000000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(47.314453125000000 , 0.00000000000011400, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(49.401855468750000 , -0.0000000000001140, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(50.097656250000014 , 0.00000000000022700, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(52.880859375000028 , 0.00000000000000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(55.664062500000028 , 0.00000000000022700, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(57.055664062499929 , 0.00000000000000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(58.447265624999929 , 0.00000000000022700, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(59.838867187499972 , 0.00000000000000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(62.622070312499943 , 0.00000000000022700, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(64.013671874999986 , 0.00000000000011400, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(66.796874999999943 , 0.00000000000022700, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(68.188476562500028 , -0.0000000000003410, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(69.580078125000028 , -0.0000000000001140, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(70.275878906250000 , -0.0000000000004550, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(72.363281249999872 , 0.00000000000022700, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(73.754882812499929 , -0.0000000000001140, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(74.450683593749929 , 0.00000000000011400, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(75.146484374999915 , 0.00000000000000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(76.538085937499943 , 0.00000000000034100, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(77.233886718749915 , 0.00000000000011400, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(77.581787109375000 , 0.00000000000022700, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(81.408691406250057 , -0.0000000000005680, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(82.104492187500099 , 0.00000000000000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(83.496093750000099 , -0.0000000000005680, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(84.191894531250057 , -0.0000000000004550, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(84.539794921875014 , -0.0000000000004550, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(84.887695312500000 , 0.00000000000022700, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(87.670898437500028 , 0.00000000000011400, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(88.018798828125014 , 0.00000000000022700, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(94.628906250000128 , -0.0000000000004550, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(95.324707031249929 , 0.00000000000011400, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(96.020507812499872 , -0.0000000000001140, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(97.412109374999943 , 0.00000000000011400, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(102.28271484374994 , -0.0000000000001140, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(102.97851562500000 , 0.00000000000034100, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(109.93652343749997 , -0.0000000000003410, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(111.32812499999997 , 0.00000000000000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(115.50292968749994 , -0.0000000000003410, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(120.02563476562499 , 0.00000000000056800, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(122.46093750000000 , 0.00000000000022700, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(123.15673828124997 , 0.00000000000034100, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(123.85253906250000 , -0.0000000000001140, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(125.24414062499999 , 0.00000000000022700, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(126.63574218750000 , 0.00000000000011400, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(127.33154296875000 , 0.00000000000102300, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(129.41894531249994 , 0.00000000000011400, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(130.11474609374994 , 0.00000000000068200, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(133.59374999999994 , -0.0000000000017050, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(134.28955078125006 , 0.00000000000034100, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(136.37695312499989 , -0.0000000000011370, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(137.76855468749969 , 0.00000000000204600, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(139.16015624999977 , 0.00000000000102300, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(140.55175781249963 , 0.00000000000147800, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(141.94335937500023 , -0.0000000000003410, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(150.29296875000014 , 0.00000000000056800, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(151.68457031249923 , -0.0000000000023870, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(153.07617187499974 , -0.0000000000021600, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(155.16357421874957 , -0.0000000000026150, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(155.85937499999977 , 0.00000000000136400, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(161.42578125000040 , -0.0000000000017050, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(162.81738281249957 , 0.00000000000022700, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(164.20898437499974 , -0.0000000000003410, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(165.94848632812540 , 0.00000000000068200, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(169.77539062500082 , -0.0000000000001140, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(172.90649414062534 , 0.00000000000022700, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(175.34179687500020 , 0.00000000000011400, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(176.03759765625045 , 0.00000000000022700, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(176.38549804687506 , -0.0000000000005680, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(176.73339843750026 , -0.0000000000004550, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(178.12500000000026 , -0.0000000000006820, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(179.86450195312520 , -0.0000000000004550, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(180.21240234374989 , -0.0000000000010230, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(180.90820312499980 , -0.0000000000004550, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(183.34350585937472 , -0.0000000000007960, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(183.69140624999997 , -0.0000000000001140, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(187.17041015625006 , -0.0000000000004550, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(187.86621093750006 , -0.0000000000001140, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(189.25781250000014 , -0.0000000000005680, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(192.04101562500006 , 0.00000000000022700, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(197.60742187500017 , -0.0000000000005680, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(201.08642578125006 , 0.00000000000000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(201.78222656250011 , -0.0000000000001140, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(203.17382812499994 , 0.00000000000000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(204.56542968749994 , -0.0000000000001140, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(205.95703125000009 , 0.00000000000011400, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(207.34863281249997 , 0.00000000000011400, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(211.52343750000014 , 0.00000000000034100, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(214.30664062500003 , -0.0000000000004550, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(215.69824218750003 , -0.0000000000001140, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(217.08984375000014 , -0.0000000000001140, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(219.87304687500000 , 0.00000000000011400, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(225.43945312499983 , 0.00000000000011400, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(228.22265624999986 , 0.00000000000022700, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(239.35546875000014 , 0.00000000000000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(244.92187500000023 , 0.00000000000011400, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(289.45312500000006 , -0.0000000000001140, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(311.71875000000000 , 0.00000000000011400, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(322.85156250000000 , -0.0000000000001140, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(333.98437499999989 , 0.00000000000011400, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(345.11718750000000 , 0.00000000000000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(356.24999999999989 , 0.00000000000011400, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(356.25000000000000 , 300.000000000000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(0.2262398393800990 , 300.000000000000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-0.000000000000057 , 299.815021846634070, pWorld));
    
    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(AddPolySegment1);

    HVE2DPolySegment  AddPolySegment2(pWorld);
    AddPolySegment2.AppendPoint(HGF2DLocation(0.000000000000000, 0.000000000000000, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(0.000000000000000, 300.0000000000000, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(357.0000000000000, 300.0000000000000, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(357.0000000000000, 0.000000000000000, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(0.000000000000000, 0.000000000000000, pWorld));
    
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(AddPolySegment2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    pResult = pShape1->IntersectShape(*pShape2);
    
    }

//==================================================================================
// Test which failed on June 21, 2010
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed74)
    {
           
    HVE2DPolySegment  AddPolySegment1(pWorld);
    AddPolySegment1.AppendPoint(HGF2DLocation( 4472.03735685348510000000, -4302.9712390899658000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation( -6261.5444221496582000000, 3287.45898628234860000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation( -6296.5840774774551000000, -4317.2590339183807000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation( 4472.03735685348510000000, -4302.9712390899658000000, pWorld));
    
    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(AddPolySegment1);
        
    HVE2DPolySegment  AddPolySegment2(pWorld);
    AddPolySegment2.AppendPoint(HGF2DLocation(4483.77481311559680000000, 1826.44534158706670000000, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation( 3571.5499991178513000000, 1811.31066036224370000000, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation( 3572.0260994434357000000, 3274.46059226989750000000, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation( -6261.544421970844300000, 3287.45898675918580000000, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation( 4472.0373568534851000000, -4302.9712390899658000000, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation( 4538.3313429355621000000, 3273.18329262733460000000, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(  4491.370812356472000000, 3273.24536705017090000000, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(  4483.774813115596800000, 1826.44534158706670000000, pWorld));
    
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(AddPolySegment2);
    pShape1->SetTolerance(0.00001);
    pShape2->SetTolerance(0.00001);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    pResult = pShape1->IntersectShape(*pShape2);   
    pResult = pShape1->UnifyShape(*pShape2);
    pResult = pShape2->UnifyShape(*pShape1);
      
    }

//==================================================================================
// Test which failed on June 21, 2010   - second one
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed75)
    {
           
    HVE2DPolySegment  AddPolySegment1(pWorld);
    AddPolySegment1.AppendPoint(HGF2DLocation(47785.000000, 6605.000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(47775.000000, 6765.000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(48035.000000, 6855.000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(48045.000000, 6755.000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(48055.000000, 6625.000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(47975.000000, 6515.000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(47895.000000, 6505.000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(47845.000000, 6515.000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(47785.000000, 6535.000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(47785.000000, 6565.000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(47785.000000, 6605.000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(47785.000000, 6605.000000, pWorld));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(AddPolySegment1);

    HVE2DPolySegment  AddPolySegment2(pWorld);
    AddPolySegment2.AppendPoint(HGF2DLocation(47895.000000, 6505.000000, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(48055.000000, 6575.000000, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(47965.000000, 6655.000000, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(47845.000000, 6515.000000, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(47895.000000, 6505.000000, pWorld));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(AddPolySegment2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    pResult = pShape1->IntersectShape(*pShape2);
    pResult = pShape1->UnifyShape(*pShape2);
    pResult = pShape2->UnifyShape(*pShape1);
      
    }

//==================================================================================
// Test which failed on June 21, 2010   - second one
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  WIP_IPPTEST_BUG_4_3)
    {
    
    #ifdef WIP_IPPTEST_BUG_4      
    HVE2DRectangle MyRectangle(0.0, 0.0, 776.0, 530.0, pWorld);

    HVE2DPolySegment  AddPolySegment1(pWorld);
    AddPolySegment1.AppendPoint(HGF2DLocation(173881.594315156690, 31262.7623784915300 , pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-672196.87396625290, -119580.92306234955  , pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-207684.53954652144, -37058.254313532634  , pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(403111.141213428460, 72700.3005556127460 , pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(173881.594315156690, 31262.7623784915300  , pWorld));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(AddPolySegment1);

    HFCPtr<HVE2DShape> pResult = MyRectangle.IntersectShape (*pShape1);
    #endif
    
    }

//==================================================================================
// Test which failed on April 21, 2011
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed76)
    {

    HVE2DPolySegment  AddPolySegment1(pWorld);
    AddPolySegment1.AppendPoint(HGF2DLocation(2322428.2272334308,-129984.05221858087, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(3403111.0235287589,-129984.05221858087, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(3403111.0235287589, 683219.40062587289 , pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(2322428.2272334308, 683219.40062587289 , pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(2322428.2272334308,-129984.05221858087, pWorld));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(AddPolySegment1);

    HVE2DPolySegment  AddPolySegment2(pWorld);
    AddPolySegment2.AppendPoint(HGF2DLocation(2819212.7968047597, 226067.48340849765, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(2776007.6856932733, 290851.21372304671, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(2908987.9398707948, 382674.45995559543, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(3038933.3691252591, 478744.44468378508, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(3165708.4068487957, 578960.94212805433, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(3289180.7939698473, 683219.40062630130, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(3340372.8070185226, 624542.39371858397, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(3214978.7255764296, 518661.28182966216, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(3086230.5918779657, 416885.03909736732, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(2954262.7234428381, 319319.84440054093, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(2819212.7968047597, 226067.48340849765, pWorld));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(AddPolySegment2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape (*pShape2);

    }

//==================================================================================
// Test which failed on April 21, 2011
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  UnifyShapeWhoFailed)
    {
           
    HVE2DRectangle rect1(255000.00, -190000.0, 260000.0, -185000.0, pWorld);
    HVE2DRectangle rect2(255000.00, -200000.0, 260000.0, -195000.0, pWorld);
    HVE2DRectangle rect3(255000.00, -195000.0, 260000.0, -190000.0, pWorld);
    HVE2DRectangle rect4(260000.00, -190000.0, 265000.0, -185000.0, pWorld);
    HVE2DRectangle rect5(265000.00, -200000.0, 270000.0, -195000.0, pWorld);
    HVE2DRectangle rect6(260000.00, -200000.0, 265000.0, -195000.0, pWorld);
    HVE2DRectangle rect7(265000.00, -195000.0, 270000.0, -190000.0, pWorld);
    HVE2DRectangle rect8(260000.00, -195000.0, 265000.0, -190000.0, pWorld);
    HVE2DRectangle rect9(270000.00, -190000.0, 275000.0, -185000.0, pWorld);
    HVE2DRectangle rect10(270000.0, -200000.0, 275000.0, -195000.0, pWorld);
    HVE2DRectangle rect11(270000.0, -195000.0, 275000.0, -190000.0, pWorld);

    HFCPtr<HVE2DShape>pResult = rect1.UnifyShape (rect2);
    pResult = pResult->UnifyShape (rect3);
    pResult = pResult->UnifyShape (rect4);
    pResult = pResult->UnifyShape (rect5);
    pResult = pResult->UnifyShape (rect6);
    pResult = pResult->UnifyShape (rect7);
    pResult = pResult->UnifyShape (rect8);
    pResult = pResult->UnifyShape (rect9);
    pResult = pResult->UnifyShape (rect10);
    pResult = pResult->UnifyShape (rect11);

    }

//==================================================================================
// Test which failed on April 21, 2011
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  IntersectShapeWhoFailed)
    {        
    
    HVE2DPolySegment  AddPolySegment1(pWorld);
    AddPolySegment1.AppendPoint(HGF2DLocation(-8885727.1063404903, 9056297.5039131828, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-8884261.2748356014, 8864533.2997093443, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-5136267.6785397073, 8893182.6877240464, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-5137733.5100445943, 9084946.8919278868, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-8885727.1063404903, 9056297.5039131828, pWorld));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(AddPolySegment1);

    HVE2DPolySegment  AddPolySegment2(pWorld);
    AddPolySegment2.AppendPoint(HGF2DLocation(-8885727.1063404903, 9056297.5039131828, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(-5137733.5100445943, 9084946.8919278868, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(-5113480.2895529531, 5912072.3074665070, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(-8861473.8858488481, 5883422.9194518048, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(-8885727.1063404903, 9056297.5039131828, pWorld));

    HFCPtr<HVE2DShape>  pShape2 = new HVE2DPolygonOfSegments(AddPolySegment2);

    HFCPtr<HVE2DShape>  pResult = pShape1->IntersectShape (*pShape2);
        
    }


//==================================================================================
// Test which failed on Oct 16, 2015
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  SpatialPoisitionWhoFailed)
    {        
    
    HVE2DPolySegment  AddPolySegment1(pWorld);
    AddPolySegment1.AppendPoint(HGF2DLocation(-96.50, 39.875, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-96.50, 39.930, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-96.43, 39.930, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-96.43, 39.875, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-96.50, 39.875, pWorld));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(AddPolySegment1);

    HVE2DPolySegment  AddPolySegment2(pWorld);
    AddPolySegment2.AppendPoint(HGF2DLocation(-96.500, 39.875, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(-96.500, 39.930, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(-96.562, 39.930, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(-96.562, 39.875, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(-96.500, 39.875, pWorld));

    HFCPtr<HVE2DShape>  pShape2 = new HVE2DPolygonOfSegments(AddPolySegment2);

    HGFSpatialPosition position1 = pShape1->CalculateSpatialPositionOf(*pShape2);
    HGFSpatialPosition position2 = pShape2->CalculateSpatialPositionOf(*pShape1);

    ASSERT_EQ(S_OUT, position1);
    ASSERT_EQ(S_OUT, position2);
        
    }