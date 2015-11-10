////:>--------------------------------------------------------------------------------------+
////:>
////:>     $Source: Tests/IppGraLibs/HGF2DPolygonOfSegmentsTester.cpp $
////:>
////:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
////:>
////:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "EnvironnementTest.h"
#include "HGF2DPolygonOfSegmentsTester.h"

// THE PRESENT TEST CANNOT MAKE TESTS WITH NON-LINEAR TRANSFORMATION MODELS

HGF2DPolygonOfSegmentsTester::HGF2DPolygonOfSegmentsTester() 
    {

    // Polygons
    Poly1A = HGF2DPolygonOfSegments(Rect1A);

    NorthContiguousPolyA = HGF2DPolygonOfSegments(NorthContiguousRectA);
    EastContiguousPolyA = HGF2DPolygonOfSegments(EastContiguousRectA);
    WestContiguousPolyA = HGF2DPolygonOfSegments(WestContiguousRectA);
    SouthContiguousPolyA = HGF2DPolygonOfSegments(SouthContiguousRectA);

    NETipPolyA = HGF2DPolygonOfSegments(NETipRectA);
    NWTipPolyA = HGF2DPolygonOfSegments(NWTipRectA);
    SETipPolyA = HGF2DPolygonOfSegments(SETipRectA);
    SWTipPolyA = HGF2DPolygonOfSegments(SWTipRectA);

    VerticalFitPolyA = HGF2DPolygonOfSegments(VerticalFitRectA);
    HorizontalFitPolyA = HGF2DPolygonOfSegments(HorizontalFitRectA);

    DisjointPolyA = HGF2DPolygonOfSegments(DisjointRectA);
    NegativePolyA = HGF2DPolygonOfSegments(NegativeRectA);

    MiscPoly1A = HGF2DPolygonOfSegments(MiscRect1A);

    EnglobPoly1A = HGF2DPolygonOfSegments(EnglobRect1A);
    EnglobPoly2A = HGF2DPolygonOfSegments(EnglobRect2A);
    EnglobPoly3A = HGF2DPolygonOfSegments(EnglobRect3A);

    IncludedPoly1A = HGF2DPolygonOfSegments(IncludedRect1A);
    IncludedPoly2A = HGF2DPolygonOfSegments(IncludedRect2A);
    IncludedPoly3A = HGF2DPolygonOfSegments(IncludedRect3A);
    IncludedPoly4A = HGF2DPolygonOfSegments(IncludedRect4A);
    IncludedPoly5A = HGF2DPolygonOfSegments(IncludedRect5A);
    IncludedPoly6A = HGF2DPolygonOfSegments(IncludedRect6A);
    IncludedPoly7A = HGF2DPolygonOfSegments(IncludedRect7A);
    IncludedPoly8A = HGF2DPolygonOfSegments(IncludedRect8A);
    IncludedPoly9A = HGF2DPolygonOfSegments(IncludedRect9A);

    PolyClosePoint1AA = HGF2DPosition(21.1, 10.1);
    PolyClosePoint1BA = HGF2DPosition(9.0, 9.0);
    PolyClosePoint1CA = HGF2DPosition(19.9, 10.0);
    PolyClosePoint1DA = HGF2DPosition(0.1, 15.0);
    PolyCloseMidPoint1A = HGF2DPosition(15.0, 20.1);

    Poly1Point0d0A = HGF2DPosition(10.0, 10.0);
    Poly1Point0d1A = HGF2DPosition(15.0, 10.0);
    Poly1Point0d5A = HGF2DPosition(20.0, 20.0);
    Poly1Point1d0A = HGF2DPosition(10.0, 10.0+(1.1 * MYEPSILON));

    PolyMidPoint1A = HGF2DPosition(15.0, 20.0);

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
TEST_F(HGF2DPolygonOfSegmentsTester,  ConstructorTest)
    {

    // Default Constructor
    HGF2DPolygonOfSegments    APoly1A;

    // Constructor with a coordinate system
    HGF2DPolygonOfSegments    APoly2;

    // Constructor with HGF2DPolySegment
    HGF2DPolySegment LinearTest;
    LinearTest.AppendPoint(HGF2DPosition(10.0, 10.0));
    LinearTest.AppendPoint(HGF2DPosition(20.0, 10.0));
    LinearTest.AppendPoint(HGF2DPosition(20.0, 20.0));
    LinearTest.AppendPoint(HGF2DPosition(10.0, 20.0));
    LinearTest.AppendPoint(HGF2DPosition(10.0, 10.0));
    
    HGF2DPolygonOfSegments PolyTest(LinearTest);

    //Constructor from Rectangle
    HGF2DPolygonOfSegments PolyTest2(Rect1A);   
    ASSERT_TRUE(PolyTest2.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(PolyTest2.IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(PolyTest2.IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(PolyTest2.IsPointOn(HGF2DPosition(20.0, 20.0)));

    //Constructor from PolySegment
    HGF2DPolySegment    PolySegment1;
    PolySegment1.AppendPoint(HGF2DPosition(1.0, 1.0));
    PolySegment1.AppendPoint(HGF2DPosition(2.0, 2.0));
    PolySegment1.AppendPoint(HGF2DPosition(3.0, 2.0));
    PolySegment1.AppendPoint(HGF2DPosition(1.0, 1.0));

    HGF2DPolygonOfSegments PolyTest3(PolySegment1);
    ASSERT_TRUE(PolyTest3.IsPointOn(HGF2DPosition(1.0, 1.0)));
    ASSERT_TRUE(PolyTest3.IsPointOn(HGF2DPosition(2.0, 2.0)));
    ASSERT_TRUE(PolyTest3.IsPointOn(HGF2DPosition(3.0, 2.0)));

    // Copy Constructor test
    HGF2DPolySegment  AComp1;
    AComp1.AppendPoint(HGF2DPosition(10.0, 10.1));
    AComp1.AppendPoint(HGF2DPosition(10.0, 20.1));
    AComp1.AppendPoint(HGF2DPosition(20.0, 20.1));
    AComp1.AppendPoint(HGF2DPosition(20.0, 10.1));
    AComp1.AppendPoint(HGF2DPosition(10.0, 10.1));

    HGF2DPolygonOfSegments    APoly3(AComp1);
    HGF2DPolygonOfSegments    APoly4(APoly3);

    HFCPtr<HGF2DLinear> theLinear = APoly4.GetLinear();  
    HFCPtr<HGF2DPolySegment> thePolySegment = static_cast<HGF2DPolySegment*>(&*theLinear);
    HGF2DPolySegment  AComp2(*thePolySegment);
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.1, AComp2.GetPoint(0).GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.1, AComp2.GetPoint(1).GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.1, AComp2.GetPoint(2).GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.1, AComp2.GetPoint(3).GetY());

    //Constructor from array
    HGF2DPolygonOfSegments APoly5(MyPolyCount, DblArray);

    ASSERT_TRUE(APoly5.IsPointOn(HGF2DPosition(0.0, 0.0)));
    ASSERT_TRUE(APoly5.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(APoly5.IsPointOn(HGF2DPosition(15.0, 5.0)));
    ASSERT_TRUE(APoly5.IsPointOn(HGF2DPosition(0.0, 5.0)));
    ASSERT_TRUE(APoly5.IsPointOn(HGF2DPosition(5.0, 0.0)));
    ASSERT_TRUE(APoly5.IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(APoly5.IsPointOn(HGF2DPosition(-20.0, 20.0)));
    
    }

//==================================================================================
// operator= test
// operator=(const HGF2DPolygonOfSegments& pi_rObj);
// SetLinear(const HGF2DLinear& pi_rLinear);
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  OperatorTest)
    {

    HGF2DPolygonOfSegments    APoly1A;

    HGF2DPolySegment  AComp1;
    AComp1.AppendPoint(HGF2DPosition(10.0, 10.1));
    AComp1.AppendPoint(HGF2DPosition(10.0, 20.1));
    AComp1.AppendPoint(HGF2DPosition(20.0, 20.1));
    AComp1.AppendPoint(HGF2DPosition(20.0, 10.1));
    AComp1.AppendPoint(HGF2DPosition(10.0, 10.1));

// &&AR The SetLinear method has been deactivated from HGF2DPolygonOfSegment
//    APoly1A.SetLinear(AComp1);

    HGF2DPolygonOfSegments    APoly2;

    APoly2 = APoly1A;

    HFCPtr<HGF2DLinear> theLinear = APoly2.GetLinear();  
    HFCPtr<HGF2DPolySegment> thePolySegment = static_cast<HGF2DPolySegment*>(&*theLinear);
    HGF2DPolySegment  AComp2(*thePolySegment);
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.1, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.1, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.1, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.1, AComp2.GetPoint(3).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(4).GetX());
    ASSERT_DOUBLE_EQ(10.1, AComp2.GetPoint(4).GetY());

    }

//==================================================================================
// GenerateCorrespondingRectangle() const;
// RepresentsARectangle() const;
//==================================================================================
TEST_F (HGF2DPolygonOfSegmentsTester, RectangleTest)
    {
    
    ASSERT_TRUE(IncludedPoly1A.RepresentsARectangle());

    HGF2DRectangle* rectangle = IncludedPoly1A.GenerateCorrespondingRectangle();
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), rectangle->GetShapeType());
    ASSERT_TRUE(rectangle->IsPointOn(HGF2DPosition(10.0, 10.0)));

    HGF2DPolySegment  Triangle;
    Triangle.AppendPoint(HGF2DPosition(0.0, 0.0));
    Triangle.AppendPoint(HGF2DPosition(5.0, 5.0));
    Triangle.AppendPoint(HGF2DPosition(0.0, 5.0));
    Triangle.AppendPoint(HGF2DPosition(0.0, 0.0));

    HGF2DPolygonOfSegments APolyTest(Triangle);

    ASSERT_FALSE(APolyTest.RepresentsARectangle());

    }

//==================================================================================
// IsConvex() const;
//==================================================================================
TEST_F (HGF2DPolygonOfSegmentsTester, IsConvexTest)
    {

    ASSERT_TRUE(NETipPolyA.IsConvex());
    ASSERT_TRUE(NWTipPolyA.IsConvex());
    ASSERT_TRUE(SETipPolyA.IsConvex());
    ASSERT_TRUE(SWTipPolyA.IsConvex());

    HGF2DPolySegment  ConcaveComplexLinear1;
    ConcaveComplexLinear1.AppendPoint(HGF2DPosition(0.0, 0.0));
    ConcaveComplexLinear1.AppendPoint(HGF2DPosition(0.0, 10.0));
    ConcaveComplexLinear1.AppendPoint(HGF2DPosition(5.0, 5.0));
    ConcaveComplexLinear1.AppendPoint(HGF2DPosition(10.0, 10.0));
    ConcaveComplexLinear1.AppendPoint(HGF2DPosition(0.0, 10.0));
    ConcaveComplexLinear1.AppendPoint(HGF2DPosition(0.0, 0.0));

    HGF2DPolygonOfSegments ConcavePoly1A(ConcaveComplexLinear1);

    ASSERT_FALSE(ConcavePoly1A.IsConvex());

    HGF2DPolySegment  ConcaveComplexLinear2;
    ConcaveComplexLinear2.AppendPoint(HGF2DPosition(0.0, 0.0));
    ConcaveComplexLinear2.AppendPoint(HGF2DPosition(0.0, -10.0));
    ConcaveComplexLinear2.AppendPoint(HGF2DPosition(5.0, -5.0));
    ConcaveComplexLinear2.AppendPoint(HGF2DPosition(10.0, -10.0));
    ConcaveComplexLinear2.AppendPoint(HGF2DPosition(10.0, 0.0));
    ConcaveComplexLinear2.AppendPoint(HGF2DPosition(5.0, -5.0));
    ConcaveComplexLinear2.AppendPoint(HGF2DPosition(0.0, 0.0));

    HGF2DPolygonOfSegments ConcavePoly2(ConcaveComplexLinear2);

    ASSERT_FALSE(ConcavePoly2.IsConvex());

    }

//==================================================================================
// Rotate(const HGFAngle& pi_rAngle, const HGF2DPosition& pi_rRotationOrigin);
//==================================================================================
TEST_F (HGF2DPolygonOfSegmentsTester, RotateTest)
    {

    HGF2DPolySegment    RotateTest1;

    RotateTest1.AppendPoint(HGF2DPosition(1.0, 1.0));
    RotateTest1.AppendPoint(HGF2DPosition(2.0, 2.0));
    RotateTest1.AppendPoint(HGF2DPosition(3.0, 2.0));
    RotateTest1.AppendPoint(HGF2DPosition(3.0, 3.0));
    RotateTest1.AppendPoint(HGF2DPosition(1.0, 1.0));

    HGF2DPolygonOfSegments PolyTest1(RotateTest1);

    PolyTest1.Rotate(PI, HGF2DPosition(0.0, 0.0)); 

    ASSERT_TRUE(PolyTest1.IsPointOn(HGF2DPosition(-1.0, -1.0)));
    ASSERT_TRUE(PolyTest1.IsPointOn(HGF2DPosition(-2.0, -2.0)));
    ASSERT_TRUE(PolyTest1.IsPointOn(HGF2DPosition(-3.0, -2.0)));
    ASSERT_TRUE(PolyTest1.IsPointOn(HGF2DPosition(-3.0, -3.0)));

    PolyTest1.Rotate(-PI, HGF2DPosition(0.0, 0.0)); 

    ASSERT_TRUE(PolyTest1.IsPointOn(HGF2DPosition(1.0, 1.0)));
    ASSERT_TRUE(PolyTest1.IsPointOn(HGF2DPosition(2.0, 2.0)));
    ASSERT_TRUE(PolyTest1.IsPointOn(HGF2DPosition(3.0, 2.0)));
    ASSERT_TRUE(PolyTest1.IsPointOn(HGF2DPosition(3.0, 3.0)));
    
    PolyTest1.Rotate(PI, HGF2DPosition(1.0, 1.0)); 

    ASSERT_TRUE(PolyTest1.IsPointOn(HGF2DPosition(1.00, 1.00)));
    ASSERT_TRUE(PolyTest1.IsPointOn(HGF2DPosition(0.00, 0.00)));
    ASSERT_TRUE(PolyTest1.IsPointOn(HGF2DPosition(-1.0, 0.00)));
    ASSERT_TRUE(PolyTest1.IsPointOn(HGF2DPosition(-1.0, -1.0)));

    }

//==================================================================================
// Move(const HGF2DDisplacement& pi_rDisplacement);
//==================================================================================
TEST_F (HGF2DPolygonOfSegmentsTester, MoveTest)
    {

    HGF2DDisplacement Translation1(10.0, 10.0);
    HGF2DDisplacement Translation2(0.0, 10.0);
    HGF2DDisplacement Translation3(10.0, 0.0);
    HGF2DDisplacement Translation4(0.0, 0.0);
    HGF2DDisplacement Translation5(-10.0, 10.0);
    HGF2DDisplacement Translation6(10.0, -10.0);
    
    HGF2DPolygonOfSegments Poly1A(IncludedRect1A);
    Poly1A.Move(Translation1);

    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(25.0, 25.0)));
 
    HGF2DPolygonOfSegments Poly2(IncludedRect2A);
    Poly2.Move(Translation2);
    
    ASSERT_TRUE(Poly2.IsPointOn(HGF2DPosition(15.0, 20.0)));
    ASSERT_TRUE(Poly2.IsPointOn(HGF2DPosition(20.0, 25.0)));

    HGF2DPolygonOfSegments Poly3(IncludedRect3A);
    Poly3.Move(Translation3);

    ASSERT_TRUE(Poly3.IsPointOn(HGF2DPosition(25.0, 15.0)));
    ASSERT_TRUE(Poly3.IsPointOn(HGF2DPosition(30.0, 20.0)));
   
    HGF2DPolygonOfSegments Poly4(IncludedRect4A);
    Poly4.Move(Translation4);
 
    ASSERT_TRUE(Poly4.IsPointOn(HGF2DPosition(10.0, 15.0)));
    ASSERT_TRUE(Poly4.IsPointOn(HGF2DPosition(15.0, 20.0)));
    
    HGF2DPolygonOfSegments Poly5(IncludedRect5A);
    Poly5.Move(Translation5);

    ASSERT_TRUE(Poly5.IsPointOn(HGF2DPosition(2.0, 22.0)));
    ASSERT_TRUE(Poly5.IsPointOn(HGF2DPosition(8.0, 28.0)));
    
    HGF2DPolygonOfSegments Poly6(IncludedRect6A);
    Poly6.Move(Translation6);
    
    }

//==================================================================================
// Scale(double pi_ScaleFactor, const HGF2DPosition& pi_rScaleOrigin)
//================================================================================== 
TEST_F (HGF2DPolygonOfSegmentsTester, ScaleTest)
    {

    HGF2DPosition Origin(0.0, 0.0);

    HGF2DPolygonOfSegments Poly1A(IncludedRect1A);
    Poly1A.Scale(2.0, Origin);

    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(30.0, 30.0)));
    
    HGF2DPolygonOfSegments Poly2(IncludedRect2A);
    Poly2.Scale(-2.0, Origin); 

    ASSERT_TRUE(Poly2.IsPointOn(HGF2DPosition(-30.0, -20.0)));
    ASSERT_TRUE(Poly2.IsPointOn(HGF2DPosition(-40.0, -30.0)));

    HGF2DPolygonOfSegments Poly3(IncludedRect3A);
    Poly3.Scale(0.5, Origin); 

    ASSERT_TRUE(Poly3.IsPointOn(HGF2DPosition(7.50, 7.50)));
    ASSERT_TRUE(Poly3.IsPointOn(HGF2DPosition(10.0, 10.0)));
   
    HGF2DPolygonOfSegments Poly4(IncludedRect4A);
    Poly4.Scale(2.0, HGF2DPosition(5.0, 5.0));

    ASSERT_TRUE(Poly4.IsPointOn(HGF2DPosition(15.0, 25.0)));
    ASSERT_TRUE(Poly4.IsPointOn(HGF2DPosition(25.0, 35.0)));
   
    HGF2DPolygonOfSegments Poly5(IncludedRect5A);
    Poly5.Scale(2.0, HGF2DPosition(-5.0, 5.0));

    ASSERT_TRUE(Poly5.IsPointOn(HGF2DPosition(29.0, 19.0)));
    ASSERT_TRUE(Poly5.IsPointOn(HGF2DPosition(41.0, 31.0)));
    
    HGF2DPolygonOfSegments Poly6(IncludedRect6A);
    Poly6.Scale(2.0, HGF2DPosition(5.0, -5.0));

    ASSERT_TRUE(Poly6.IsPointOn(HGF2DPosition(15.0, 25.0)));
    ASSERT_TRUE(Poly6.IsPointOn(HGF2DPosition(35.0, 35.0)));
    
    HGF2DPolygonOfSegments Poly7(IncludedRect7A);
    Poly7.Scale(0.5, HGF2DPosition(5.0, 5.0));

    ASSERT_TRUE(Poly7.IsPointOn(HGF2DPosition(7.5, 7.5)));
    ASSERT_TRUE(Poly7.IsPointOn(HGF2DPosition(10.0, 12.5)));
   
    }

//==================================================================================
// AllocateParallelCopy(const HGFDistance& pi_rOffset,HGF2DVector::ArbitraryDirection pi_DirectionToRight, const HGF2DLine* pi_pFirstPointAlignment,
//                     const HGF2DLine* pi_pLastPointAlignment) const
//==================================================================================
TEST_F (HGF2DPolygonOfSegmentsTester, AllocateParallelCopyTest)
    {

    HFCPtr<HGF2DPolygonOfSegments> APoly1A = (HGF2DPolygonOfSegments*) IncludedPoly1A.AllocateParallelCopy(1.0, HGF2DVector::ALPHA);

    ASSERT_TRUE(APoly1A->IsPointOn(HGF2DPosition(11.0, 11.0)));
    ASSERT_TRUE(APoly1A->IsPointOn(HGF2DPosition(11.0, 14.0)));
    ASSERT_TRUE(APoly1A->IsPointOn(HGF2DPosition(14.0, 11.0)));
    ASSERT_TRUE(APoly1A->IsPointOn(HGF2DPosition(14.0, 14.0)));

    HFCPtr<HGF2DPolygonOfSegments> APoly2 = (HGF2DPolygonOfSegments*) IncludedPoly1A.AllocateParallelCopy(1.0, HGF2DVector::BETA);

    ASSERT_TRUE(APoly2->IsPointOn(HGF2DPosition(9.0, 9.0)));
    ASSERT_TRUE(APoly2->IsPointOn(HGF2DPosition(9.0, 16.0)));
    ASSERT_TRUE(APoly2->IsPointOn(HGF2DPosition(16.0, 9.0)));
    ASSERT_TRUE(APoly2->IsPointOn(HGF2DPosition(16.0, 16.0)));

    #ifdef WIP_IPPTEST_BUG_14
    HFCPtr<HGF2DPolygonOfSegments> APoly3 = (HGF2DPolygonOfSegments*) IncludedPoly1A.AllocateParallelCopy(5.0, HGF2DVector::ALPHA);
    #endif    

    HGF2DPolySegment ParallelTest;
    ParallelTest.AppendPoint(HGF2DPosition(0.0, 0.0));
    ParallelTest.AppendPoint(HGF2DPosition(0.0, 10.0));
    ParallelTest.AppendPoint(HGF2DPosition(4.0, 10.0));
    ParallelTest.AppendPoint(HGF2DPosition(4.0, 2.0));
    ParallelTest.AppendPoint(HGF2DPosition(8.0, 2.0));
    ParallelTest.AppendPoint(HGF2DPosition(5.0, 10.0));
    ParallelTest.AppendPoint(HGF2DPosition(10.0, 10.0));
    ParallelTest.AppendPoint(HGF2DPosition(10.0, 0.0));
    ParallelTest.AppendPoint(HGF2DPosition(0.0, 0.0));

    HGF2DPolygonOfSegments APoly4(ParallelTest);

    #ifdef WIP_IPPTEST_BUG_15
    HFCPtr<HGF2DPolygonOfSegments> APoly5 = (HGF2DPolygonOfSegments*) APoly4.AllocateParallelCopy(5.0, HGF2DVector::ALPHA);
    HFCPtr<HGF2DPolygonOfSegments> APoly6 = (HGF2DPolygonOfSegments*) APoly4.AllocateParallelCopy(5.0, HGF2DVector::BETA);
    #endif

    }

//==================================================================================
// GetPoint() const;
// GetLinear(HGF2DSimpleShape::RotationDirection pi_DirectionDesired) const;
// AllocateLinear(HGF2DSimpleShape::RotationDirection pi_DirectionDesired) const;
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  GetPointTest)
    {
    HFCPtr<HGF2DLinear> theLinear = Poly1A.GetLinear();  
    HFCPtr<HGF2DPolySegment> thePolySegment = static_cast<HGF2DPolySegment*>(&*theLinear);

    
    HGF2DPolySegment  MyLinearOfPoly1A(*thePolySegment);

    // verify that there are 4 linears
    ASSERT_EQ(4, MyLinearOfPoly1A.GetSize());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPoly1A.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPoly1A.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPoly1A.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPoly1A.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPoly1A.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPoly1A.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPoly1A.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPoly1A.GetPoint(3).GetY());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPoly1A.GetPoint(4).GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPoly1A.GetPoint(4).GetY());

    HFCPtr<HGF2DLinear> theLinear2 = Poly1A.GetLinear(HGF2DSimpleShape::CW);  
    HFCPtr<HGF2DPolySegment> thePolySegment2 = static_cast<HGF2DPolySegment*>(&*theLinear2);

    HGF2DPolySegment  MyLinearOfPolyCW(*thePolySegment2);
    ASSERT_EQ(4, MyLinearOfPolyCW.GetSize());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCW.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCW.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCW.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPolyCW.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPolyCW.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPolyCW.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPolyCW.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCW.GetPoint(3).GetY());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCW.GetPoint(4).GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCW.GetPoint(4).GetY());

    HFCPtr<HGF2DLinear> theLinear3 = Poly1A.GetLinear(HGF2DSimpleShape::CCW);  
    HFCPtr<HGF2DPolySegment> thePolySegment3 = static_cast<HGF2DPolySegment*>(&*theLinear3);
    
    HGF2DPolySegment  MyLinearOfPolyCCW(*thePolySegment3);
    ASSERT_EQ(4, MyLinearOfPolyCCW.GetSize());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCCW.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCCW.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPolyCCW.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCCW.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPolyCCW.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPolyCCW.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCCW.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPolyCCW.GetPoint(3).GetY());
    
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCCW.GetPoint(4).GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPolyCCW.GetPoint(4).GetY());


    }

//==================================================================================
// Perimeter calculation test
// CalculatePerimeter() const;
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  CalculatePerimeterTest)
    {
    
    // Test with linear 1
    ASSERT_DOUBLE_EQ(40.0, Poly1A.CalculatePerimeter());
    ASSERT_DOUBLE_EQ(40.0, NegativePolyA.CalculatePerimeter());

    }

//==================================================================================
// Area Calculation test
// CalculateArea() const;
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  CalculateAreaTest)
    {

    ASSERT_DOUBLE_EQ(100.0, Poly1A.CalculateArea());
    ASSERT_DOUBLE_EQ(100.0, NegativePolyA.CalculateArea());
  
    }

//==================================================================================
// Drop( HGF2DPositionCollection* po_pPoints, const HGFDistance& pi_rTolerance ) const
//==================================================================================
TEST_F (HGF2DPolygonOfSegmentsTester, DropTest)
    {

    HGF2DPositionCollection Locations;

    HGF2DPolygonOfSegments APoly(Rect1A);

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
// CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const;
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  CalculateClosestPointTest)
    {
    
    // Test with linear 1
    ASSERT_DOUBLE_EQ(20.0, Poly1A.CalculateClosestPoint(PolyClosePoint1AA).GetX());
    ASSERT_DOUBLE_EQ(10.1, Poly1A.CalculateClosestPoint(PolyClosePoint1AA).GetY());
    ASSERT_DOUBLE_EQ(10.0, Poly1A.CalculateClosestPoint(PolyClosePoint1BA).GetX());
    ASSERT_DOUBLE_EQ(10.0, Poly1A.CalculateClosestPoint(PolyClosePoint1BA).GetY());
    ASSERT_DOUBLE_EQ(19.9, Poly1A.CalculateClosestPoint(PolyClosePoint1CA).GetX());
    ASSERT_DOUBLE_EQ(10.0, Poly1A.CalculateClosestPoint(PolyClosePoint1CA).GetY());
    ASSERT_DOUBLE_EQ(10.0, Poly1A.CalculateClosestPoint(PolyClosePoint1DA).GetX());
    ASSERT_DOUBLE_EQ(15.0, Poly1A.CalculateClosestPoint(PolyClosePoint1DA).GetY());
    ASSERT_DOUBLE_EQ(15.0, Poly1A.CalculateClosestPoint(PolyCloseMidPoint1A).GetX());
    ASSERT_DOUBLE_EQ(20.0, Poly1A.CalculateClosestPoint(PolyCloseMidPoint1A).GetY());

    // Tests with special points
    ASSERT_DOUBLE_EQ(20.0, Poly1A.CalculateClosestPoint(VeryFarPointA).GetX());
    ASSERT_DOUBLE_EQ(20.0, Poly1A.CalculateClosestPoint(VeryFarPointA).GetY());
    ASSERT_DOUBLE_EQ(15.0, Poly1A.CalculateClosestPoint(PolyMidPoint1A).GetX());
    ASSERT_DOUBLE_EQ(20.0, Poly1A.CalculateClosestPoint(PolyMidPoint1A).GetY());
   
    }

//==================================================================================
// Intersection test (with other complex linears only)
// Intersect(const HGF2DVector& pi_rVector, HGF2DPositionCollection* po_pCrossPoints) const;
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  IntersectTest)
    {

    HGF2DPositionCollection   DumPoints;

    // Test with extent disjoint linears
    ASSERT_EQ(0, Poly1A.Intersect(DisjointLinear1A, &DumPoints));
    DumPoints.clear();

    // Test with disjoint but touching by a side
    ASSERT_EQ(0, Poly1A.Intersect(ContiguousExtentLinear1A, &DumPoints));
    DumPoints.clear();

    // Test with disjoint but touching by a tip but not linked
    ASSERT_EQ(0, Poly1A.Intersect(FlirtingExtentLinear1A, &DumPoints));
    DumPoints.clear();

    // Tests with connected linears
    // At start point...
    ASSERT_EQ(0, Poly1A.Intersect(ConnectingLinear1B, &DumPoints));
    DumPoints.clear();

    // At end point ...
    ASSERT_EQ(0, Poly1A.Intersect(ConnectingLinear1AA, &DumPoints));
    DumPoints.clear();

    // Tests with linked segments
    ASSERT_EQ(0, Poly1A.Intersect(LinkedLinear1AA, &DumPoints));
    DumPoints.clear();

    // Special cases
    ASSERT_EQ(1, Poly1A.Intersect(ComplexLinearCase1A, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(13.5, DumPoints[0].GetY());
    DumPoints.clear();

    ASSERT_EQ(1, Poly1A.Intersect(ComplexLinearCase2A, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(15.0, DumPoints[0].GetY());
    DumPoints.clear();

    ASSERT_EQ(1, Poly1A.Intersect(ComplexLinearCase3A, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());
    DumPoints.clear();

    ASSERT_EQ(0, Poly1A.Intersect(ComplexLinearCase4A, &DumPoints));
    DumPoints.clear();

    ASSERT_EQ(1, Poly1A.Intersect(ComplexLinearCase5B, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());
    DumPoints.clear();

    ASSERT_EQ(0, Poly1A.Intersect(ComplexLinearCase5B, &DumPoints));
    DumPoints.clear();

    ASSERT_EQ(0, Poly1A.Intersect(ComplexLinearCase6A, &DumPoints));
    DumPoints.clear();

    ASSERT_EQ(0, Poly1A.Intersect(ComplexLinearCase7A, &DumPoints));
    DumPoints.clear();

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
TEST_F(HGF2DPolygonOfSegmentsTester,  ContiguousnessTest)
    {

    HGF2DPositionCollection     DumPoints;
    HGF2DPosition   FirstDumPoint;
    HGF2DPosition   SecondDumPoint;

    // Test with contiguous linears
    ASSERT_TRUE(Poly1A.AreContiguous(ComplexLinearCase6A));
    ASSERT_TRUE(Poly1A.AreContiguousAt(ComplexLinearCase6A, PolyMidPoint1A));
    ASSERT_EQ(2, Poly1A.ObtainContiguousnessPoints(ComplexLinearCase6A, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetY());

    Poly1A.ObtainContiguousnessPointsAt(ComplexLinearCase6A, LinearMidPoint1A, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(10.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetY());

    // Test with non contiguous linears
    ASSERT_FALSE(Poly1A.AreContiguous(ComplexLinearCase1A));

    DumPoints.clear();

    // Test with contiguous Polygon
    ASSERT_TRUE(Poly1A.AreContiguous(NorthContiguousPolyA));
    ASSERT_TRUE(Poly1A.AreContiguousAt(NorthContiguousPolyA, PolyMidPoint1A));
    ASSERT_EQ(2, Poly1A.ObtainContiguousnessPoints(NorthContiguousPolyA, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[1].GetY());

    Poly1A.ObtainContiguousnessPointsAt(NorthContiguousPolyA, PolyMidPoint1A, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(10.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(20.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, SecondDumPoint.GetY());

    DumPoints.clear();

    ASSERT_TRUE(Poly1A.AreContiguous(VerticalFitPolyA));
    ASSERT_TRUE(Poly1A.AreContiguousAt(VerticalFitPolyA, HGF2DPosition(17.0, 10.0)));
    ASSERT_EQ(4, Poly1A.ObtainContiguousnessPoints(VerticalFitPolyA, &DumPoints));
    ASSERT_DOUBLE_EQ(15.0, DumPoints[3].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[3].GetY());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[2].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[2].GetY());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[1].GetY());
    ASSERT_DOUBLE_EQ(15.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetY());

    Poly1A.ObtainContiguousnessPointsAt(VerticalFitPolyA, HGF2DPosition(17.0, 10.0), &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(20.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(15.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetY());

    DumPoints.clear();

    ASSERT_TRUE(Poly1A.AreContiguous(IncludedPoly1A));
    ASSERT_TRUE(Poly1A.AreContiguousAt(IncludedPoly1A, HGF2DPosition(10.0, 10.0)));
    ASSERT_EQ(2, Poly1A.ObtainContiguousnessPoints(IncludedPoly1A, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(15.0, DumPoints[1].GetY());
    ASSERT_DOUBLE_EQ(15.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());

    Poly1A.ObtainContiguousnessPointsAt(IncludedPoly1A, HGF2DPosition(10.0, 10.0), &FirstDumPoint, &SecondDumPoint);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  CloningTest)
    {

    //General Clone Test
    HFCPtr<HGF2DPolygonOfSegments> pClone = (HGF2DPolygonOfSegments*)Poly1A.Clone();

    ASSERT_FALSE(pClone->IsEmpty());
    ASSERT_EQ(HGF2DPolygonOfSegments::CLASS_ID, pClone->GetShapeType());

    ASSERT_TRUE(pClone->IsPointOn(HGF2DPosition(10.0, 10.0)));  
    ASSERT_TRUE(pClone->IsPointOn(HGF2DPosition(10.0, 20.0))); 
    ASSERT_TRUE(pClone->IsPointOn(HGF2DPosition(20.0, 20.0))); 
    ASSERT_TRUE(pClone->IsPointOn(HGF2DPosition(20.0, 10.0))); 

    // Test with the same coordinate system
    HFCPtr<HGF2DPolygonOfSegments> pClone3 = (HGF2DPolygonOfSegments*)(&*(Poly1A.AllocTransformDirect(HGF2DIdentity())));

    ASSERT_FALSE(pClone3->IsEmpty());
    ASSERT_EQ(HGF2DPolygonOfSegments::CLASS_ID, pClone3->GetShapeType());

    ASSERT_TRUE(pClone3->IsPointOn(HGF2DPosition(10.0, 10.0)));  
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DPosition(10.0, 20.0))); 
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DPosition(20.0, 20.0))); 
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DPosition(20.0, 10.0)));  

    // Test with a translation between systems
    Translation = HGF2DDisplacement (10.0, 10.0);
    HGF2DTranslation myTranslation(Translation);
   
    HFCPtr<HGF2DPolygonOfSegments> pClone5 = (HGF2DPolygonOfSegments*)(&*(Poly1A.AllocTransformDirect(myTranslation)));

    ASSERT_FALSE(pClone5->IsEmpty());
    ASSERT_EQ(HGF2DPolygonOfSegments::CLASS_ID, pClone5->GetShapeType());

    ASSERT_TRUE(pClone5->IsPointOn(HGF2DPosition(0.0, 0.0)));  
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DPosition(0.0, 10.0))); 
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DPosition(10.0, 10.0))); 
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DPosition(10.0, 0.0))); 

    // Test with a stretch between systems
    HGF2DStretch myStretch;
    myStretch.SetTranslation(Translation);
    myStretch.SetXScaling(0.5);
    myStretch.SetYScaling(0.5);
    
    HFCPtr<HGF2DPolygonOfSegments> pClone6 = (HGF2DPolygonOfSegments*)(&*(Poly1A.AllocTransformDirect(myStretch)));

    ASSERT_FALSE(pClone6->IsEmpty());
    ASSERT_EQ(HGF2DPolygonOfSegments::CLASS_ID, pClone6->GetShapeType());

    ASSERT_TRUE(pClone6->IsPointOn(HGF2DPosition(0.0, 0.0)));  
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DPosition(0.0, 20.0))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DPosition(20.0, 20.0))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DPosition(20.0, 0.0))); 

    // Test with a similitude between systems
    HGF2DSimilitude mySimilitude;
    mySimilitude.SetRotation(PI);
    mySimilitude.SetScaling(0.5);
   
    HFCPtr<HGF2DPolygonOfSegments> pClone7 = (HGF2DPolygonOfSegments*)(&*(Poly1A.AllocTransformDirect(mySimilitude)));

    ASSERT_FALSE(pClone7->IsEmpty());
    ASSERT_EQ(HGF2DPolygonOfSegments::CLASS_ID, pClone7->GetShapeType());

    ASSERT_TRUE(pClone7->IsPointOn(HGF2DPosition(-20.0, -20.0)));  
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DPosition(-20.0, -40.0))); 
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DPosition(-40.0, -40.0))); 
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DPosition(-40.0, -20.0))); 

    // Test with a affine between systems
    HGF2DAffine myAffine;
    myAffine.SetTranslation(Translation);
    myAffine.SetRotation(PI);
    myAffine.SetXScaling(0.5);
    myAffine.SetYScaling(0.5);
    
    HFCPtr<HGF2DPolygonOfSegments> pClone8 = (HGF2DPolygonOfSegments*)(&*(Poly1A.AllocTransformDirect(myAffine)));

    ASSERT_FALSE(pClone8->IsEmpty());
    ASSERT_EQ(HGF2DPolygonOfSegments::CLASS_ID, pClone8->GetShapeType());

    ASSERT_TRUE(pClone8->IsPointOn(HGF2DPosition(0.0, 0.0)));  
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DPosition(0.0, -20.0))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DPosition(-20.0, -20.0))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DPosition(-20.0, 0.0))); 
  
    }

//==================================================================================
// Interaction info with other segment
// Crosses(const HGF2DVector& pi_rVector) const;
// AreAdjacent(const HGF2DVector& pi_rVector) const;
// IsPointOn(const HGF2DPosition& pi_rTestPoint) const;
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  InteractionTest)
    {
     
    // Tests with a vertical segment
    ASSERT_TRUE(Poly1A.Crosses(ComplexLinearCase1A));
    ASSERT_FALSE(Poly1A.AreAdjacent(ComplexLinearCase1A));

    ASSERT_TRUE(Poly1A.Crosses(ComplexLinearCase2A));
    ASSERT_TRUE(Poly1A.AreAdjacent(ComplexLinearCase2A));

    ASSERT_TRUE(Poly1A.Crosses(ComplexLinearCase3A));
    ASSERT_FALSE(Poly1A.AreAdjacent(ComplexLinearCase3A));

    ASSERT_FALSE(Poly1A.Crosses(ComplexLinearCase4A));
    ASSERT_TRUE(Poly1A.AreAdjacent(ComplexLinearCase4A));

    ASSERT_TRUE(Poly1A.Crosses(ComplexLinearCase5B));
    ASSERT_TRUE(Poly1A.AreAdjacent(ComplexLinearCase5B));

    ASSERT_FALSE(Poly1A.Crosses(ComplexLinearCase6A));
    ASSERT_TRUE(Poly1A.AreAdjacent(ComplexLinearCase6A));

    ASSERT_FALSE(Poly1A.Crosses(ComplexLinearCase7A));
    ASSERT_FALSE(Poly1A.AreAdjacent(ComplexLinearCase7A));

    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(10.0 - 1.1*MYEPSILON, 10.0-1.1*MYEPSILON)));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(PolyMidPoint1A.GetX(), PolyMidPoint1A.GetY() - 1.1*MYEPSILON)));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(PolyMidPoint1A.GetX(), PolyMidPoint1A.GetY() + 1.1*MYEPSILON)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(PolyMidPoint1A.GetX(), PolyMidPoint1A.GetY() - 0.9*MYEPSILON)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(PolyMidPoint1A.GetX(), PolyMidPoint1A.GetY() + 0.9*MYEPSILON)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(PolyMidPoint1A)));

    ASSERT_TRUE(Poly1A.IsPointOn(Poly1A.GetLinear()->GetStartPoint(), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1A.IsPointOn(Poly1A.GetLinear()->GetEndPoint(), HGF2DVector::EXCLUDE_EXTREMITIES));
    
    }

//==================================================================================
// Bearing calculation tests
// CalculateBearing(const HGF2DPosition& pi_rPositionPoint,
//                  HGF2DVector::ArbitraryDiPolyion pi_DiPolyion = HGF2DVector::BETA) const;
// CalculateAngularAcceleration(const HGF2DPosition& pi_rPositionPoint,
//                              HGF2DVector::ArbitraryDiPolyion pi_DiPolyion = HGF2DVector::BETA) const;
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  BearingTest)
    {

    // Obtain bearing ALPHA of a vertical segment
    ASSERT_NEAR(0.0, Poly1A.CalculateBearing(Poly1Point0d0A, HGF2DVector::ALPHA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.5707963267948966, Poly1A.CalculateBearing(Poly1Point0d0A, HGF2DVector::BETA).GetAngle());
    ASSERT_NEAR(0.0, Poly1A.CalculateBearing(Poly1Point0d1A, HGF2DVector::ALPHA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(3.1415926535897931, Poly1A.CalculateBearing(Poly1Point0d1A, HGF2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(3.1415926535897931, Poly1A.CalculateBearing(Poly1Point0d5A, HGF2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(4.7123889803846897, Poly1A.CalculateBearing(Poly1Point0d5A, HGF2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(4.7123889803846897, Poly1A.CalculateBearing(Poly1Point1d0A, HGF2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(1.5707963267948966, Poly1A.CalculateBearing(Poly1Point1d0A, HGF2DVector::BETA).GetAngle());

    // ANGULAR ACCELERATION
    // Obtain angular acceleration ALPHA of a vertical segment
    ASSERT_NEAR(0.0, Poly1A.CalculateAngularAcceleration(Poly1Point0d0A, HGF2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, Poly1A.CalculateAngularAcceleration(Poly1Point0d0A, HGF2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, Poly1A.CalculateAngularAcceleration(Poly1Point0d1A, HGF2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, Poly1A.CalculateAngularAcceleration(Poly1Point0d1A, HGF2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, Poly1A.CalculateAngularAcceleration(Poly1Point0d5A, HGF2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, Poly1A.CalculateAngularAcceleration(Poly1Point0d5A, HGF2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, Poly1A.CalculateAngularAcceleration(Poly1Point1d0A, HGF2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, Poly1A.CalculateAngularAcceleration(Poly1Point1d0A, HGF2DVector::BETA), MYEPSILON);
   
    }
        
//==================================================================================
// Extent calculation test
// GetExtent() const;
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  GetExtentTest)
    {

    // Obtain extent of linear 1
    ASSERT_DOUBLE_EQ(10.0, Poly1A.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(10.0, Poly1A.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(20.0, Poly1A.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(20.0, Poly1A.GetExtent().GetYMax());

    }

//==================================================================================
// IsEmpty() test
// MakEmpty() test
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  EmptyTest)
    {

    HGF2DPolygonOfSegments  MyOtherPoly(Poly1A);
    ASSERT_FALSE(MyOtherPoly.IsEmpty());

    MyOtherPoly.MakeEmpty();
    ASSERT_TRUE(MyOtherPoly.IsEmpty());
  
    }

//==================================================================================
// GetShapeType() 
//==================================================================================
  TEST_F(HGF2DPolygonOfSegmentsTester,  ClassIDTest)
    {     
    
    HGF2DPolygonOfSegments  MyOtherPoly(Poly1A);        
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), Poly1A.GetShapeType());
    
    }

//==================================================================================
// IsPointIn()
// IsPointOn()
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  IsPointTest)
    {

    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(10.0, 10.0)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(15.0, 10.0)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(20.0, 10.0)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(20.0, 15.0)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(20.0, 20.0)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(15.0, 20.0)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(10.0, 20.0)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(10.0, 15.0)));

    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(100.0, 100.0)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(10.0-1.1*MYEPSILON, 10.0)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(10.0, 10.0-1.1*MYEPSILON)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(10.0-1.1*MYEPSILON, 15.0)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(10.0-1.1*MYEPSILON, 20.0)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(10.0, 20.0+1.1*MYEPSILON)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(15.0, 20.0+1.1*MYEPSILON)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(20.0, 20.0+1.1*MYEPSILON)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(20.0+1.1*MYEPSILON, 20.0)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(20.0+1.1*MYEPSILON, 15.0)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(20.0+1.1*MYEPSILON, 10.0)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(20.0, 10.0-1.1*MYEPSILON)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(15.0, 10.0-1.1*MYEPSILON)));

    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(10.0-0.9*MYEPSILON, 10.0)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(10.0, 10.0-0.9*MYEPSILON)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(10.0-0.9*MYEPSILON, 15.0)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(10.0-0.9*MYEPSILON, 20.0)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(10.0, 20.0+0.9*MYEPSILON)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(15.0, 20.0+0.9*MYEPSILON)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(20.0, 20.0+0.9*MYEPSILON)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(20.0+0.9*MYEPSILON, 20.0)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(20.0+0.9*MYEPSILON, 15.0)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(20.0+0.9*MYEPSILON, 10.0)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(20.0, 10.0-0.9*MYEPSILON)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(15.0, 10.0-0.9*MYEPSILON)));

    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(10.0+0.9*MYEPSILON, 15.0)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(15.0, 20.0-0.9*MYEPSILON)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(20.0-0.9*MYEPSILON, 15.0)));
    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(15.0, 10.0+0.9*MYEPSILON)));

    ASSERT_TRUE(Poly1A.IsPointIn(HGF2DPosition(10.0+1.1*MYEPSILON, 15.0)));
    ASSERT_TRUE(Poly1A.IsPointIn(HGF2DPosition(15.0, 20.0-1.1*MYEPSILON)));
    ASSERT_TRUE(Poly1A.IsPointIn(HGF2DPosition(20.0-1.1*MYEPSILON, 15.0)));
    ASSERT_TRUE(Poly1A.IsPointIn(HGF2DPosition(15.0, 10.0+1.1*MYEPSILON)));

    ASSERT_TRUE(Poly1A.IsPointIn(HGF2DPosition(12.0, 15.0)));
    ASSERT_TRUE(Poly1A.IsPointIn(HGF2DPosition(15.0, 18.0)));
    ASSERT_TRUE(Poly1A.IsPointIn(HGF2DPosition(18.0, 15.0)));
    ASSERT_TRUE(Poly1A.IsPointIn(HGF2DPosition(15.0, 12.0)));
    ASSERT_TRUE(Poly1A.IsPointIn(HGF2DPosition(15.0, 15.0)));

    ASSERT_FALSE(Poly1A.IsPointIn(HGF2DPosition(-15.0, -15.0)));

    // IsPointOn
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(15.0, 10.0)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(20.0, 15.0)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(15.0, 20.0)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(10.0, 15.0)));

    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(100.0, 100.0)));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(10.0-1.1*MYEPSILON, 10.0)));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(10.0, 10.0-1.1*MYEPSILON)));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(10.0-1.1*MYEPSILON, 15.0)));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(10.0-1.1*MYEPSILON, 20.0)));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(10.0, 20.0+1.1*MYEPSILON)));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(15.0, 20.0+1.1*MYEPSILON)));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(20.0, 20.0+1.1*MYEPSILON)));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(20.0+1.1*MYEPSILON, 20.0)));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(20.0+1.1*MYEPSILON, 15.0)));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(20.0+1.1*MYEPSILON, 10.0)));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(20.0, 10.0-1.1*MYEPSILON)));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(15.0, 10.0-1.1*MYEPSILON)));

    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(10.0-0.9*MYEPSILON, 10.0)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(10.0, 10.0-0.9*MYEPSILON)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(10.0-0.9*MYEPSILON, 15.0)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(10.0-0.9*MYEPSILON, 20.0)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(10.0, 20.0+0.9*MYEPSILON)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(15.0, 20.0+0.9*MYEPSILON)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(20.0, 20.0+0.9*MYEPSILON)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(20.0+0.9*MYEPSILON, 20.0)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(20.0+0.9*MYEPSILON, 15.0)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(20.0+0.9*MYEPSILON, 10.0)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(20.0, 10.0-0.9*MYEPSILON)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(15.0, 10.0-0.9*MYEPSILON)));

    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(10.0+0.9*MYEPSILON, 15.0)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(15.0, 20.0-0.9*MYEPSILON)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(20.0-0.9*MYEPSILON, 15.0)));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(15.0, 10.0+0.9*MYEPSILON)));

    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(10.0+1.1*MYEPSILON, 15.0)));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(15.0, 20.0-1.1*MYEPSILON)));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(20.0-1.1*MYEPSILON, 15.0)));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(15.0, 10.0+1.1*MYEPSILON)));

    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(12.0, 15.0)));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(15.0, 18.0)));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(18.0, 15.0)));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(15.0, 12.0)));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(15.0, 15.0)));

    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(-15.0, -15.0)));

    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(10.0, 10.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(15.0, 10.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(20.0, 10.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(20.0, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(20.0, 20.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(15.0, 20.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(10.0, 20.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(10.0, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(100.0, 100.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(10.0-1.1*MYEPSILON, 10.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(10.0, 10.0-1.1*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(10.0-1.1*MYEPSILON, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(10.0-1.1*MYEPSILON, 20.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(10.0, 20.0+1.1*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(15.0, 20.0+1.1*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(20.0, 20.0+1.1*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(20.0+1.1*MYEPSILON, 20.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(20.0+1.1*MYEPSILON, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(20.0+1.1*MYEPSILON, 10.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(20.0, 10.0-1.1*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(15.0, 10.0-1.1*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(10.0-0.9*MYEPSILON, 10.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(10.0, 10.0-0.9*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(10.0-0.9*MYEPSILON, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(10.0-0.9*MYEPSILON, 20.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(10.0, 20.0+0.9*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(15.0, 20.0+0.9*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(20.0, 20.0+0.9*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(20.0+0.9*MYEPSILON, 20.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(20.0+0.9*MYEPSILON, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(20.0+0.9*MYEPSILON, 10.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(20.0, 10.0-0.9*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(15.0, 10.0-0.9*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(10.0+0.9*MYEPSILON, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(15.0, 20.0-0.9*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(20.0-0.9*MYEPSILON, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1A.IsPointOn(HGF2DPosition(15.0, 10.0+0.9*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(10.0+1.1*MYEPSILON, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(15.0, 20.0-1.1*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(20.0-1.1*MYEPSILON, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(15.0, 10.0+1.1*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(12.0, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(15.0, 18.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(18.0, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(15.0, 12.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(15.0, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_FALSE(Poly1A.IsPointOn(HGF2DPosition(-15.0, -15.0), HGF2DVector::EXCLUDE_EXTREMITIES));

    }

//==================================================================================
// UnifyShape(const HGF2DShape& pi_rShape) const;
// UnifyShape(const HGF2DShape& pi_rShape) const;
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  UnifyShapeTest)
    {
        
    HGF2DPolySegment  AComp2;

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape1 = (HGF2DPolygonOfSegments *)Poly1A.UnifyShape(NorthContiguousPolyA);

    ASSERT_DOUBLE_EQ(200.0, pResultShape1->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape1->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape1->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape2 = (HGF2DPolygonOfSegments *)Poly1A.UnifyShape(EastContiguousPolyA);

    ASSERT_DOUBLE_EQ(200.0, pResultShape2->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape2->GetShapeType());

    AComp2 =  (*static_cast<HGF2DPolySegment*>(&*(pResultShape2->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(30.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(30.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape3 = (HGF2DPolygonOfSegments *)Poly1A.UnifyShape(WestContiguousPolyA);

    ASSERT_DOUBLE_EQ(200.0, pResultShape2->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape3->GetShapeType());

    AComp2 =  (*static_cast<HGF2DPolySegment*>(&*(pResultShape3->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_NEAR(0.0, AComp2.GetPoint(0).GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_NEAR(0.0, AComp2.GetPoint(1).GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape4 = (HGF2DPolygonOfSegments *) Poly1A.UnifyShape(SouthContiguousPolyA);

    ASSERT_DOUBLE_EQ(200.0, pResultShape4->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape4->GetShapeType());

    AComp2 =  (*static_cast<HGF2DPolySegment*>(&*(pResultShape4->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_NEAR(0.0, AComp2.GetPoint(0).GetY(), MYEPSILON);

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_NEAR(0.0, AComp2.GetPoint(3).GetY(), MYEPSILON);

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape5 = (HGF2DPolygonOfSegments *) Poly1A.UnifyShape(VerticalFitPolyA);

    ASSERT_DOUBLE_EQ(150.0, pResultShape5->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape5->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape5->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(25.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(25.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape6 = (HGF2DPolygonOfSegments *) Poly1A.UnifyShape(HorizontalFitPolyA);

    ASSERT_DOUBLE_EQ(150.0, pResultShape6->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape6->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape6->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DShape>     pResultShape7 = Poly1A.UnifyShape(DisjointPolyA);

    ASSERT_NE(pResultShape7->GetShapeType(), static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID));
    ASSERT_TRUE(pResultShape7->IsComplex());
    ASSERT_FALSE(pResultShape7->HasHoles());
    ASSERT_DOUBLE_EQ(200.0, pResultShape7->CalculateArea());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape8 = (HGF2DPolygonOfSegments *) Poly1A.UnifyShape(MiscPoly1A);

    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape8->GetShapeType());
    ASSERT_DOUBLE_EQ(175.0, pResultShape8->CalculateArea());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape8->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(8, AComp2.GetSize()); 

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(4).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(4).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(5).GetX());
    ASSERT_DOUBLE_EQ(5.0, AComp2.GetPoint(5).GetY());

    ASSERT_DOUBLE_EQ(5.0, AComp2.GetPoint(6).GetX());
    ASSERT_DOUBLE_EQ(5.0, AComp2.GetPoint(6).GetY());

    ASSERT_DOUBLE_EQ(5.0, AComp2.GetPoint(7).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(7).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape9 = (HGF2DPolygonOfSegments *) Poly1A.UnifyShape(EnglobPoly1A); 

    ASSERT_DOUBLE_EQ(400.0, pResultShape9->CalculateArea());
    #ifdef WIP_IPPTEST_BUG_1
    //Amelioration : Being able to realize that the simple shape is a polygon in this case
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape9->GetShapeType());
    #endif

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape9->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(30.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(30.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape10 = (HGF2DPolygonOfSegments *) Poly1A.UnifyShape(EnglobPoly2A);

    ASSERT_DOUBLE_EQ(900.0, pResultShape10->CalculateArea());              
    #ifdef WIP_IPPTEST_BUG_1
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape10->GetShapeType());
    #endif

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape10->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_NEAR(0.0, AComp2.GetPoint(0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp2.GetPoint(0).GetY(), MYEPSILON);

    ASSERT_NEAR(0.0, AComp2.GetPoint(1).GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(30.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(30.0, AComp2.GetPoint(3).GetX());
    ASSERT_NEAR(0.0, AComp2.GetPoint(3).GetY(), MYEPSILON);

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape11 = (HGF2DPolygonOfSegments *) Poly1A.UnifyShape(EnglobPoly3A);

    ASSERT_DOUBLE_EQ(200.0, pResultShape11->CalculateArea());
    #ifdef WIP_IPPTEST_BUG_1
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape11->GetShapeType());
    #endif

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape11->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape12 = (HGF2DPolygonOfSegments *) Poly1A.UnifyShape(IncludedPoly1A);

    ASSERT_DOUBLE_EQ(100.0, pResultShape12->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape12->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape12->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape13 = (HGF2DPolygonOfSegments *) Poly1A.UnifyShape(IncludedPoly2A);

    ASSERT_DOUBLE_EQ(100.0, pResultShape13->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape13->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape13->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape14 = (HGF2DPolygonOfSegments *) Poly1A.UnifyShape(IncludedPoly3A);

    ASSERT_DOUBLE_EQ(100.0, pResultShape14->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape14->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape14->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape15 = (HGF2DPolygonOfSegments *) Poly1A.UnifyShape(IncludedPoly4A);

    ASSERT_DOUBLE_EQ(100.0, pResultShape15->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape15->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape15->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape16 = (HGF2DPolygonOfSegments *)Poly1A.UnifyShape(IncludedPoly5A);

    ASSERT_DOUBLE_EQ(100.0, pResultShape16->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape16->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape16->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY()); 

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape17 = (HGF2DPolygonOfSegments *) Poly1A.UnifyShape(IncludedPoly6A);

    ASSERT_DOUBLE_EQ(100.0, pResultShape17->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape17->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape17->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape18 = (HGF2DPolygonOfSegments *) Poly1A.UnifyShape(IncludedPoly7A);

    ASSERT_DOUBLE_EQ(100.0, pResultShape18->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape18->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape18->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape19 = (HGF2DPolygonOfSegments *) Poly1A.UnifyShape(IncludedPoly8A);

    ASSERT_DOUBLE_EQ(100.0, pResultShape19->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape19->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape19->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape20 = (HGF2DPolygonOfSegments *) Poly1A.UnifyShape(IncludedPoly9A);

    ASSERT_DOUBLE_EQ(100.0, pResultShape20->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape20->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape20->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape21 = (HGF2DPolygonOfSegments *) Poly1A.UnifyShape(IncludedPoly9A);

    ASSERT_DOUBLE_EQ(100.0, pResultShape21->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape21->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape21->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());
    
    }

//==================================================================================
// IntersectShape(const HGF2DShape& pi_rShape) const;
// IntersectShape(const HGF2DShape& pi_rShape) const;
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  IntersectShapeTest)
    {

    HGF2DPolySegment  AComp2;

    HFCPtr<HGF2DShape>     pResultShape1 = Poly1A.IntersectShape(NorthContiguousPolyA);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape1->GetShapeType());
    ASSERT_TRUE(pResultShape1->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape2 = Poly1A.IntersectShape(EastContiguousPolyA);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape2->GetShapeType());
    ASSERT_TRUE(pResultShape2->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape3 = Poly1A.IntersectShape(WestContiguousPolyA);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape3->GetShapeType());
    ASSERT_TRUE(pResultShape3->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape4 = Poly1A.IntersectShape(SouthContiguousPolyA);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape4->GetShapeType());
    ASSERT_TRUE(pResultShape4->IsEmpty());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape5 = (HGF2DPolygonOfSegments*)Poly1A.IntersectShape(VerticalFitPolyA);

    ASSERT_DOUBLE_EQ(50.0, pResultShape5->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape5->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape5->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape6 = (HGF2DPolygonOfSegments*)Poly1A.IntersectShape(HorizontalFitPolyA);

    ASSERT_DOUBLE_EQ(50.0, pResultShape6->CalculateArea());

    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape6->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape6->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DShape>     pResultShape7 = Poly1A.IntersectShape(DisjointPolyA);
    ASSERT_EQ(pResultShape7->GetShapeType(), HGF2DVoidShape::CLASS_ID);
    ASSERT_TRUE(pResultShape7->IsEmpty());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape8 = (HGF2DPolygonOfSegments*)Poly1A.IntersectShape(MiscPoly1A);

    ASSERT_DOUBLE_EQ(25.0, pResultShape8->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape8->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape8->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape9 = (HGF2DPolygonOfSegments*)Poly1A.IntersectShape(EnglobPoly1A);

    ASSERT_DOUBLE_EQ(100.0, pResultShape9->CalculateArea());
    #ifdef WIP_IPPTEST_BUG_1
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape9->GetShapeType());
    #endif

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape9->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape10 = (HGF2DPolygonOfSegments*)Poly1A.IntersectShape(EnglobPoly2A);

    ASSERT_DOUBLE_EQ(100.0, pResultShape10->CalculateArea());
    #ifdef WIP_IPPTEST_BUG_1
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape10->GetShapeType());
    #endif

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape10->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape11 = (HGF2DPolygonOfSegments*)Poly1A.IntersectShape(EnglobPoly3A);

    ASSERT_DOUBLE_EQ(100.0, pResultShape11->CalculateArea());
    #ifdef WIP_IPPTEST_BUG_1
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape11->GetShapeType());
    #endif

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape11->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape12 = (HGF2DPolygonOfSegments*)Poly1A.IntersectShape(IncludedPoly1A);

    ASSERT_DOUBLE_EQ(25.0, pResultShape12->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape12->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape12->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape13 = (HGF2DPolygonOfSegments*) Poly1A.IntersectShape(IncludedPoly2A);

    ASSERT_DOUBLE_EQ(25.0, pResultShape13->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape13->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape13->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape14 = (HGF2DPolygonOfSegments*)Poly1A.IntersectShape(IncludedPoly3A);

    ASSERT_DOUBLE_EQ(25.0, pResultShape14->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape14->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape14->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape15 = (HGF2DPolygonOfSegments*) Poly1A.IntersectShape(IncludedPoly4A);

    ASSERT_DOUBLE_EQ(25.0, pResultShape15->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape15->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape15->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape16 = (HGF2DPolygonOfSegments*)Poly1A.IntersectShape(IncludedPoly5A);

    ASSERT_DOUBLE_EQ(36.0, pResultShape16->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape16->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape16->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(12.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(12.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(12.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(18.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(18.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(18.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(18.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(12.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape17 = (HGF2DPolygonOfSegments*) Poly1A.IntersectShape(IncludedPoly6A);

    ASSERT_DOUBLE_EQ(50.0, pResultShape17->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape17->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape17->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape18 = (HGF2DPolygonOfSegments*)Poly1A.IntersectShape(IncludedPoly7A);

    ASSERT_DOUBLE_EQ(50.0, pResultShape18->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape18->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape18->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape19 = (HGF2DPolygonOfSegments*)Poly1A.IntersectShape(IncludedPoly8A);

    ASSERT_DOUBLE_EQ(50.0, pResultShape19->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape19->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape19->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape20 = (HGF2DPolygonOfSegments*) Poly1A.IntersectShape(IncludedPoly9A);

    ASSERT_DOUBLE_EQ(50.0, pResultShape20->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape20->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape20->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape21 = (HGF2DPolygonOfSegments*)Poly1A.IntersectShape(IncludedPoly9A);

    ASSERT_DOUBLE_EQ(50.0, pResultShape21->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape21->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape21->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(3).GetY());

    }

//==================================================================================
// DifferentiateShapeTest
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  DifferentiateShapeTest)
    {

    HGF2DPolySegment  AComp2;

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape1 = (HGF2DPolygonOfSegments*) Poly1A.DifferentiateShape(NorthContiguousPolyA);
    
    ASSERT_DOUBLE_EQ(100.0, pResultShape1->CalculateArea());    
    #ifdef WIP_IPPTEST_BUG_1
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape1->GetShapeType());
    #endif

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape1->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape2 = (HGF2DPolygonOfSegments*) Poly1A.DifferentiateShape(EastContiguousPolyA);

    ASSERT_DOUBLE_EQ(100.0, pResultShape2->CalculateArea());
    #ifdef WIP_IPPTEST_BUG_1
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape2->GetShapeType());
    #endif

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape2->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape3 = (HGF2DPolygonOfSegments*) Poly1A.DifferentiateShape(WestContiguousPolyA);

    ASSERT_DOUBLE_EQ(100.0, pResultShape3->CalculateArea());
    #ifdef WIP_IPPTEST_BUG_1
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape3->GetShapeType());
    #endif
    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape3->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape4 = (HGF2DPolygonOfSegments*) Poly1A.DifferentiateShape(SouthContiguousPolyA);

    ASSERT_DOUBLE_EQ(100.0, pResultShape4->CalculateArea());
    #ifdef WIP_IPPTEST_BUG_1
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape4->GetShapeType());
    #endif

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape4->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape5 = (HGF2DPolygonOfSegments*) Poly1A.DifferentiateShape(VerticalFitPolyA);

    ASSERT_DOUBLE_EQ(50.0, pResultShape5->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape5->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape5->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape6 = (HGF2DPolygonOfSegments*) Poly1A.DifferentiateShape(HorizontalFitPolyA);

    ASSERT_DOUBLE_EQ(50.0, pResultShape6->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape6->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape6->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape7 = (HGF2DPolygonOfSegments*) Poly1A.DifferentiateShape(DisjointPolyA);

    ASSERT_DOUBLE_EQ(100.0, pResultShape7->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape7->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape7->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape8 = (HGF2DPolygonOfSegments*) Poly1A.DifferentiateShape(MiscPoly1A);

    ASSERT_DOUBLE_EQ(75.0, pResultShape8->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape8->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape8->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(6, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(4).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(4).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(5).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(5).GetY());

    HFCPtr<HGF2DShape>     pResultShape9 = Poly1A.DifferentiateShape(EnglobPoly1A);

    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape9->GetShapeType());
    ASSERT_TRUE(pResultShape9->IsEmpty());

    ASSERT_NEAR(0.0, pResultShape9->CalculateArea(), MYEPSILON);  // must be 0

    HFCPtr<HGF2DShape>     pResultShape10 = Poly1A.DifferentiateShape(EnglobPoly2A);

    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape10->GetShapeType());
    ASSERT_TRUE(pResultShape10->IsEmpty());

    ASSERT_NEAR(0.0, pResultShape10->CalculateArea(), MYEPSILON); // must be 0

    HFCPtr<HGF2DShape>     pResultShape11 = Poly1A.DifferentiateShape(EnglobPoly3A);

    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape11->GetShapeType());
    ASSERT_TRUE(pResultShape11->IsEmpty());

    ASSERT_NEAR(0.0, pResultShape11->CalculateArea(), MYEPSILON); // must be 0

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape12 = (HGF2DPolygonOfSegments*) Poly1A.DifferentiateShape(IncludedPoly1A);

    ASSERT_DOUBLE_EQ(75.0, pResultShape12->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape12->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape12->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(6, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(4).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(4).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(5).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(5).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape13 = (HGF2DPolygonOfSegments*) Poly1A.DifferentiateShape(IncludedPoly2A);

    ASSERT_DOUBLE_EQ(75.0, pResultShape13->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape13->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape13->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(6, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(3).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(4).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(4).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(5).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(5).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape14 = (HGF2DPolygonOfSegments*) Poly1A.DifferentiateShape(IncludedPoly3A);

    ASSERT_DOUBLE_EQ(75.0, pResultShape14->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape14->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape14->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(6, AComp2.GetSize());
    
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(3).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(4).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(4).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(5).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(5).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape15 = (HGF2DPolygonOfSegments*) Poly1A.DifferentiateShape(IncludedPoly4A);

    ASSERT_DOUBLE_EQ(75.0, pResultShape15->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape15->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape15->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(6, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(4).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(4).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(5).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(5).GetY());

    HFCPtr<HGF2DShape>     pResultShape16 = Poly1A.DifferentiateShape(IncludedPoly5A);

    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape16->GetShapeType());
    ASSERT_TRUE(pResultShape16->HasHoles());
    ASSERT_DOUBLE_EQ(64.0, pResultShape16->CalculateArea());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape17 = (HGF2DPolygonOfSegments*) Poly1A.DifferentiateShape(IncludedPoly6A);

    ASSERT_DOUBLE_EQ(50.0, pResultShape17->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape17->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape17->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape18 = (HGF2DPolygonOfSegments*) Poly1A.DifferentiateShape(IncludedPoly7A);

    ASSERT_DOUBLE_EQ(50.0, pResultShape18->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape18->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape18->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape19 = (HGF2DPolygonOfSegments*) Poly1A.DifferentiateShape(IncludedPoly8A);

    ASSERT_DOUBLE_EQ(50.0, pResultShape19->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape19->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape19->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape20 = (HGF2DPolygonOfSegments*)Poly1A.DifferentiateShape(IncludedPoly9A);

    ASSERT_DOUBLE_EQ(50.0, pResultShape20->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape20->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape20->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    }



//==================================================================================
// DifferentiateFromShapeTest
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  DifferentiateFromShapeTest)
    {
        
    HGF2DPolySegment  AComp2;

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape1 = (HGF2DPolygonOfSegments*)Poly1A.DifferentiateFromShape(NorthContiguousPolyA);

    ASSERT_DOUBLE_EQ(100.0, pResultShape1->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape1->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape1->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape2 = (HGF2DPolygonOfSegments*)Poly1A.DifferentiateFromShape(EastContiguousPolyA);

    ASSERT_DOUBLE_EQ(100.0, pResultShape2->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape2->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape2->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(30.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(30.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape3 = (HGF2DPolygonOfSegments*)Poly1A.DifferentiateFromShape(WestContiguousPolyA);

    ASSERT_DOUBLE_EQ(100.0, pResultShape3->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape3->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape3->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_NEAR(0.0, AComp2.GetPoint(0).GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_NEAR(0.0, AComp2.GetPoint(1).GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape4 = (HGF2DPolygonOfSegments*)Poly1A.DifferentiateFromShape(SouthContiguousPolyA);

    ASSERT_DOUBLE_EQ(100.0, pResultShape4->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape4->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape4->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_NEAR(0.0, AComp2.GetPoint(0).GetY(), MYEPSILON);

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_NEAR(0.0, AComp2.GetPoint(3).GetY(), MYEPSILON);

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape5 =(HGF2DPolygonOfSegments*) Poly1A.DifferentiateFromShape(VerticalFitPolyA);

    ASSERT_DOUBLE_EQ(50.0, pResultShape5->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape5->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape5->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(25.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(25.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape6 = (HGF2DPolygonOfSegments*)Poly1A.DifferentiateFromShape(HorizontalFitPolyA);

    ASSERT_DOUBLE_EQ(50.0, pResultShape6->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape6->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape6->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape7 = (HGF2DPolygonOfSegments*)Poly1A.DifferentiateFromShape(DisjointPolyA);
    
    ASSERT_DOUBLE_EQ(100.0, pResultShape7->CalculateArea()); 
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape7->GetShapeType());
    
    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape7->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(-10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(-10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(-10.0, AComp2.GetPoint(1).GetX());
    ASSERT_NEAR(0.0, AComp2.GetPoint(1).GetY(), MYEPSILON);

    ASSERT_NEAR(0.0, AComp2.GetPoint(2).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp2.GetPoint(2).GetY(), MYEPSILON);

    ASSERT_NEAR(0.0, AComp2.GetPoint(3).GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(-10.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape8 = (HGF2DPolygonOfSegments*)Poly1A.DifferentiateFromShape(MiscPoly1A);
    
    ASSERT_DOUBLE_EQ(75.0, pResultShape8->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape8->GetShapeType());
    
    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape8->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(6, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(5.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(5.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(5.0, AComp2.GetPoint(3).GetY());

    ASSERT_DOUBLE_EQ(5.0, AComp2.GetPoint(4).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(4).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(5).GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetPoint(5).GetY());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape9 = (HGF2DPolygonOfSegments*)Poly1A.DifferentiateFromShape(EnglobPoly1A);

    ASSERT_DOUBLE_EQ(300.0, pResultShape9->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape9->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape9->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(6, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetPoint(3).GetY());

    ASSERT_DOUBLE_EQ(30.0, AComp2.GetPoint(4).GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetPoint(4).GetY());

    ASSERT_DOUBLE_EQ(30.0, AComp2.GetPoint(5).GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(5).GetY());

    HFCPtr<HGF2DShape>     pResultShape10 = (HGF2DPolygonOfSegments*)Poly1A.DifferentiateFromShape(EnglobPoly2A);

    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape10->GetShapeType());
    ASSERT_TRUE(pResultShape10->HasHoles());
    ASSERT_DOUBLE_EQ(800.0, pResultShape10->CalculateArea());

    HFCPtr<HGF2DPolygonOfSegments>     pResultShape11 = (HGF2DPolygonOfSegments*) Poly1A.DifferentiateFromShape(EnglobPoly3A);

    ASSERT_DOUBLE_EQ(100.0, pResultShape11->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape11->GetShapeType());

    AComp2 = (*static_cast<HGF2DPolySegment*>(&*(pResultShape11->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp2.GetSize());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetPoint(3).GetY());

    HFCPtr<HGF2DShape>     pResultShape12 = Poly1A.DifferentiateFromShape(IncludedPoly1A);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape12->GetShapeType());
    ASSERT_TRUE(pResultShape12->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape13 = Poly1A.DifferentiateFromShape(IncludedPoly2A);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape13->GetShapeType());
    ASSERT_TRUE(pResultShape13->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape14 = Poly1A.DifferentiateFromShape(IncludedPoly3A);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape14->GetShapeType());
    ASSERT_TRUE(pResultShape14->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape15 = Poly1A.DifferentiateFromShape(IncludedPoly4A);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape15->GetShapeType());
    ASSERT_TRUE(pResultShape15->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape16 = Poly1A.DifferentiateFromShape(IncludedPoly5A);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape16->GetShapeType());
    ASSERT_TRUE(pResultShape16->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape17 = Poly1A.DifferentiateFromShape(IncludedPoly6A);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape17->GetShapeType());
    ASSERT_TRUE(pResultShape17->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape18 = Poly1A.DifferentiateFromShape(IncludedPoly7A);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape18->GetShapeType());
    ASSERT_TRUE(pResultShape18->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape19 = Poly1A.DifferentiateFromShape(IncludedPoly8A);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape19->GetShapeType());
    ASSERT_TRUE(pResultShape19->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape20 = Poly1A.DifferentiateFromShape(IncludedPoly9A);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape20->GetShapeType());
    ASSERT_TRUE(pResultShape20->IsEmpty());
    
    }
    
//==================================================================================
// SPECIAL TESTS
// The following are all case which failed with previous library
//==================================================================================

//==================================================================================
// IntersectShapeTestWhoFailed
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  IntersectShapeTestWhoFailed)
    {

    HGF2DSegment    Segment1A(HGF2DPosition(-1.7E308, -1.7E308), HGF2DPosition(-1.7E308, 1.7E308));
    HGF2DSegment    Segment2A(HGF2DPosition(-1.7E308, 1.7E308), HGF2DPosition(1.7E308, 1.7E308));
    HGF2DSegment    Segment3A(HGF2DPosition(1.7E308, 1.7E308), HGF2DPosition(1.7E308, -1.7E308));
    HGF2DSegment    Segment4A(HGF2DPosition(1.7E308, -1.7E308), HGF2DPosition(-1.7E308, -1.7E308));
    HGF2DPolySegment  MyLinear1;
    MyLinear1.AppendPoint(HGF2DPosition(-1.7E308, -1.7E308));
    MyLinear1.AppendPoint(HGF2DPosition(-1.7E308, 1.7E308));
    MyLinear1.AppendPoint(HGF2DPosition(1.7E308, 1.7E308));
    MyLinear1.AppendPoint(HGF2DPosition(1.7E308, -1.7E308));
    MyLinear1.AppendPoint(HGF2DPosition(-1.7E308, -1.7E308));

    HGF2DPolygonOfSegments    Poly1A(MyLinear1);

    HGF2DSegment    Segment1B(HGF2DPosition(0.0, 0.0), HGF2DPosition(395.59575494628, -76.896025049963));
    HGF2DSegment    Segment2B(HGF2DPosition(395.59575494628, -76.896025049963), HGF2DPosition(471.91935301076, 315.75484834585));
    HGF2DSegment    Segment3B(HGF2DPosition(471.91935301076, 315.75484834585), HGF2DPosition(76.323598064480, 392.65087339581));
    HGF2DSegment    Segment4B(HGF2DPosition(76.323598064480, 392.65087339581), HGF2DPosition(0.0, 0.0));
    HGF2DPolySegment  MyLinear2;
    MyLinear2.AppendPoint(HGF2DPosition(0.0, 0.0));
    MyLinear2.AppendPoint(HGF2DPosition(395.59575494628, -76.896025049963));
    MyLinear2.AppendPoint(HGF2DPosition(471.91935301076, 315.75484834585));
    MyLinear2.AppendPoint(HGF2DPosition(76.323598064480, 392.65087339581));
    MyLinear2.AppendPoint(HGF2DPosition(0.0, 0.0));

    HGF2DPolygonOfSegments    Poly1AB(MyLinear2);

    HFCPtr<HGF2DPolygonOfSegments>     pResult1A = (HGF2DPolygonOfSegments*) Poly1A.IntersectShape(Poly1AB);

    ASSERT_DOUBLE_EQ(161200.000000001717, pResult1A->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResult1A->GetShapeType());
    ASSERT_TRUE(pResult1A->IsSimple());

    HGF2DPolySegment  AComp1A;
    AComp1A = (*static_cast<HGF2DPolySegment*>(&*(pResult1A->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp1A.GetSize());

    ASSERT_NEAR(0.0, AComp1A.GetPoint(0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp1A.GetPoint(0).GetY(), MYEPSILON);

    ASSERT_DOUBLE_EQ(76.323598064480, AComp1A.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(392.65087339581, AComp1A.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(471.919353010760, AComp1A.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(315.754848345850, AComp1A.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(395.595754946280, AComp1A.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(-76.896025049963, AComp1A.GetPoint(3).GetY());

    }

//==================================================================================
// Test which failed on 21 may 1997
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  IntersectShapeTestWithPointerWhoFailed)
    {
   
    HFCPtr<HGF2DShape>      pShape1 = new HGF2DRectangle (0.0, 0.0, 415.0, 409.0);

    HGF2DPolySegment  TheLinear;

    TheLinear.AppendPoint(HGF2DPosition(0.0 , 256.02));
    TheLinear.AppendPoint(HGF2DPosition(83.0 , 256.02));
    TheLinear.AppendPoint(HGF2DPosition(83.0 , 0.02));
    TheLinear.AppendPoint(HGF2DPosition(0.0 , 0.02)); 
    TheLinear.AppendPoint(HGF2DPosition(0.0 , 256.02));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear);

    HFCPtr<HGF2DPolygonOfSegments> pResult = (HGF2DPolygonOfSegments*) pShape1->IntersectShape(*pShape2);

    ASSERT_DOUBLE_EQ(21248.0, pResult->CalculateArea());
    #ifdef WIP_IPPTEST_BUG_1
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResult->GetShapeType());
    #endif       
    ASSERT_TRUE(pResult->IsSimple());

    HGF2DPolySegment  AComp;
    AComp = (*static_cast<HGF2DPolySegment*>(&*(pResult->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp.GetSize());

    ASSERT_NEAR(0.0, AComp.GetPoint(0).GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(256.0, AComp.GetPoint(0).GetY());
    
    ASSERT_DOUBLE_EQ(83.00, AComp.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(256.0, AComp.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(83.0, AComp.GetPoint(2).GetX());
    ASSERT_NEAR(0.0, AComp.GetPoint(2).GetY(), MYEPSILON);

    ASSERT_NEAR(0.0, AComp.GetPoint(3).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp.GetPoint(3).GetY(), MYEPSILON);

    }

//==================================================================================
// Another test which failed on 22 may 1997
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  IntersectShapeTestWithPointerWhoFailed2)
    {
        

    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(0.0, 0.0));
    TheLinear1.AppendPoint(HGF2DPosition(256.0, 0.0));
    TheLinear1.AppendPoint(HGF2DPosition(256.0, 256.0));
    TheLinear1.AppendPoint(HGF2DPosition(0.0, 256.0));
    TheLinear1.AppendPoint(HGF2DPosition(0.0, 0.0));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(256.0, 105.02));
    TheLinear2.AppendPoint(HGF2DPosition(256.0, 0.02));
    TheLinear2.AppendPoint(HGF2DPosition(0.0, 0.02));
    TheLinear2.AppendPoint(HGF2DPosition(0.0, 105.02));
    TheLinear2.AppendPoint(HGF2DPosition(256.0, 105.02));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DRectangle> pResult = (HGF2DRectangle*) pShape1->IntersectShape(*pShape2);

    ASSERT_DOUBLE_EQ(26880.0, pResult->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResult->GetShapeType());
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
TEST_F(HGF2DPolygonOfSegmentsTester,  IntersectShapeTestWithPointerWhoFailed3)
    {
           
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(256.0 , 331.90680836798));
    TheLinear1.AppendPoint(HGF2DPosition(172.85086905196 , 370.67988489798));
    TheLinear1.AppendPoint(HGF2DPosition(71.345071697720 , 153.0));
    TheLinear1.AppendPoint(HGF2DPosition(256.0 , 153.0));
    TheLinear1.AppendPoint(HGF2DPosition(256.0 , 331.90680836798));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  Linear2;

    Linear2.AppendPoint(HGF2DPosition(256.0 , 331.906808367982));
    Linear2.AppendPoint(HGF2DPosition(172.85086905196 , 370.679884897982));
    Linear2.AppendPoint(HGF2DPosition(53.476108564268 , 114.679884897982));
    Linear2.AppendPoint(HGF2DPosition(256.0 , 114.679884897982));
    Linear2.AppendPoint(HGF2DPosition(256.0 , 331.906808367982));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(Linear2);

    HFCPtr<HGF2DPolygonOfSegments> pResult = (HGF2DPolygonOfSegments*) pShape1->IntersectShape(*pShape2);

    ASSERT_DOUBLE_EQ(27535.8045875850621, pResult->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResult->GetShapeType());
    ASSERT_TRUE(pResult->IsSimple());

    HGF2DPolySegment  AComp;
    AComp = (*static_cast<HGF2DPolySegment*>(&*(pResult->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp.GetSize());  

    ASSERT_DOUBLE_EQ(256.00000000000, AComp.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(331.90680836798, AComp.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(256.00000000000, AComp.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(153.00000000000, AComp.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(71.345071697720, AComp.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(153.00000000000, AComp.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(172.85086905196, AComp.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(370.67988489798, AComp.GetPoint(3).GetY());

    }

//==================================================================================
// Test which failed on may 30 1997
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  IntersectShapeTestWithPointerWhoFailed4)
    {
           
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(0.0 , 0.0));
    TheLinear1.AppendPoint(HGF2DPosition(0.0 , 256.0));
    TheLinear1.AppendPoint(HGF2DPosition(256.0 , 256.0));
    TheLinear1.AppendPoint(HGF2DPosition(256.0 , 0.0));
    TheLinear1.AppendPoint(HGF2DPosition(0.0 , 0.0));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(256.0 , 38.7730765300082));
    TheLinear2.AppendPoint(HGF2DPosition(172.85086905196, 0.02));
    TheLinear2.AppendPoint(HGF2DPosition(53.476108564268 , 256.02));
    TheLinear2.AppendPoint(HGF2DPosition(256.0 , 256.02));
    TheLinear2.AppendPoint(HGF2DPosition(256.0 , 38.7730765300082));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DRectangle> pResult = (HGF2DRectangle*) pShape1->IntersectShape(*pShape2);
    
    ASSERT_DOUBLE_EQ(34954.1730562968150, pResult->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResult->GetShapeType());
    ASSERT_TRUE(pResult->IsSimple());

    HGF2DPolySegment  AComp;
    AComp = (*static_cast<HGF2DPolySegment*>(&*(pResult->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(4, AComp.GetSize());

    ASSERT_DOUBLE_EQ(256.00000000000, AComp.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(38.773076530008, AComp.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(172.85086905196, AComp.GetPoint(1).GetX());
    ASSERT_NEAR(0.0, AComp.GetPoint(1).GetY(), MYEPSILON);

    ASSERT_DOUBLE_EQ(53.476108564268, AComp.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(256.00000000000, AComp.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(256.00000000000, AComp.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(256.00000000000, AComp.GetPoint(3).GetY());

    }

//==================================================================================
// Test which failed on june 4 1997
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  IntersectShapeTestWithPointerWhoFailed5)
    {
          
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(0.0 , 0.0));
    TheLinear1.AppendPoint(HGF2DPosition(172.85086905196 , 370.67988489798));
    TheLinear1.AppendPoint(HGF2DPosition(548.96860067216 , 195.29330627558));
    TheLinear1.AppendPoint(HGF2DPosition(376.11773162020 , -175.38657862240));
    TheLinear1.AppendPoint(HGF2DPosition(0.0 , 0.0));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(77.0, 165.127032879222));
    TheLinear2.AppendPoint(HGF2DPosition(77.0, 114.679884897982));
    TheLinear2.AppendPoint(HGF2DPosition(53.476108564268 , 114.679884897982));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DPolygonOfSegments> pResult = (HGF2DPolygonOfSegments*) pShape1->IntersectShape(*pShape2);

    ASSERT_DOUBLE_EQ(593.356616176497936, pResult->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResult->GetShapeType());
    ASSERT_TRUE(pResult->IsSimple());

    HGF2DPolySegment  AComp;
    AComp = (*static_cast<HGF2DPolySegment*>(&*(pResult->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(3, AComp.GetSize());

    ASSERT_DOUBLE_EQ(77.000000000000, AComp.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(165.12703287922, AComp.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(77.000000000000, AComp.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(114.67988489798, AComp.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(53.476108564268, AComp.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(114.67988489798, AComp.GetPoint(2).GetY());

    }

//==================================================================================
// Test which failed on july 9 1997
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  IntersectShapeTestWithPointerWhoFailed6)
    {
           
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(256.0 , -56.753833636596));
    TheLinear1.AppendPoint(HGF2DPosition(256.0 , 49.442061113575));
    TheLinear1.AppendPoint(HGF2DPosition(189.15280219211 , 80.613421377599));
    TheLinear1.AppendPoint(HGF2DPosition(37.5906557384530807, 80.6134213775989394));
    TheLinear1.AppendPoint(HGF2DPosition(0.0, 0.0));
    TheLinear1.AppendPoint(HGF2DPosition(232.01479348138, -108.19027500563));
    TheLinear1.AppendPoint(HGF2DPosition(256.0 , -56.753833636596));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(256.0, -119.37476048769));
    TheLinear2.AppendPoint(HGF2DPosition(0.0, 0.0));
    TheLinear2.AppendPoint(HGF2DPosition(37.5906557384530799, 80.6134213775989536));
    TheLinear2.AppendPoint(HGF2DPosition(256.0, 80.613421377599));
    TheLinear2.AppendPoint(HGF2DPosition(256.0, -119.37476048769));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DPolygonOfSegments> pResult = (HGF2DPolygonOfSegments*) pShape2->IntersectShape(*pShape1);

    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResult->GetShapeType());
    ASSERT_TRUE(pResult->IsSimple());
    ASSERT_DOUBLE_EQ(32609.0025554273561, pResult->CalculateArea());

    HGF2DPolySegment  AComp;
    AComp = (*static_cast<HGF2DPolySegment*>(&*(pResult->GetLinear(HGF2DSimpleShape::CW))));
    ASSERT_EQ(6, AComp.GetSize());

    ASSERT_DOUBLE_EQ(256.000000000000000, AComp.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(-56.753833636595999, AComp.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(232.014793481380, AComp.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(-108.19027500563, AComp.GetPoint(1).GetY());

    ASSERT_NEAR(0.0, AComp.GetPoint(2).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp.GetPoint(2).GetY(), MYEPSILON);

    ASSERT_DOUBLE_EQ(37.590655738453080, AComp.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(80.613421377598939, AComp.GetPoint(3).GetY());

    ASSERT_DOUBLE_EQ(189.15280219210999, AComp.GetPoint(4).GetX());
    ASSERT_DOUBLE_EQ(80.613421377598939, AComp.GetPoint(4).GetY());

    ASSERT_DOUBLE_EQ(256.000000000000000, AComp.GetPoint(5).GetX());
    ASSERT_DOUBLE_EQ(49.4420611135750010, AComp.GetPoint(5).GetY());
 
    } 



//==================================================================================
// Test which failed on aug 27, 1997
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed)
    {

    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(2048.000000000000000 , 1097.903989243390200));
    TheLinear1.AppendPoint(HGF2DPosition(2168.002336591538600 , 986.0000000000001100));
    TheLinear1.AppendPoint(HGF2DPosition(2470.000000000000000 , 986.0000000000001100));
    TheLinear1.AppendPoint(HGF2DPosition(2470.000000000000000 , 704.3826228932581400));
    TheLinear1.AppendPoint(HGF2DPosition(2569.421928625488400 , 611.6701745570791200));
    TheLinear1.AppendPoint(HGF2DPosition(2048.000000000000000 , 611.6701745570791200));
    TheLinear1.AppendPoint(HGF2DPosition(2048.000000000000000 , 1097.903989243390200));

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(1279.605405332091600 , 0.000000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(1280.000000000000000 , 0.000000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(1280.000000000000000 , 1024.000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(1248.098025825285000 , 1024.000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(1657.011823802983100 , 1462.506362048594600));
    TheLinear2.AppendPoint(HGF2DPosition(2168.002336591538600 , 986.0000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(1396.000000000000000 , 986.0000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(1396.000000000000000 , 136.0000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(2233.100842751780000 , 136.0000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(1721.775008346975300 , -412.329825442920880));
    TheLinear2.AppendPoint(HGF2DPosition(1279.605405332091600 , 0.000000000000000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);
    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(6714.37009156061685, pResult->CalculateArea());

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(778332.088479116559, pResult->CalculateArea());

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(162576.011547499569, pResult->CalculateArea());

    pResult = pShape1->DifferentiateFromShape(*pShape2);
    ASSERT_DOUBLE_EQ(609041.706840056227, pResult->CalculateArea());

    }

//==================================================================================
// Test which failed on sep 15, 1997
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed2)
    {
   
    HGF2DPolySegment  TheLinear1; 

    TheLinear1.AppendPoint(HGF2DPosition(0.000000, 0.000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.000000, 0.000001));
    TheLinear1.AppendPoint(HGF2DPosition(0.000001, 0.000001));
    TheLinear1.AppendPoint(HGF2DPosition(0.000001, 0.000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.000000, 0.000000));

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(0.00000000000000000 , -229.449314192432380));
    TheLinear2.AppendPoint(HGF2DPosition(0.00000000000000000 , 453.2173524742341900));
    TheLinear2.AppendPoint(HGF2DPosition(682.666666666666630 , 453.2173524742341900));
    TheLinear2.AppendPoint(HGF2DPosition(682.666666666666630 , -229.449314192432380));
    TheLinear2.AppendPoint(HGF2DPosition(0.00000000000000000 , -229.449314192432380));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);
    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(466033.777777777635, pResult->CalculateArea());

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);

    pResult = pShape1->DifferentiateFromShape(*pShape2);
    ASSERT_DOUBLE_EQ(466033.777777777635, pResult->CalculateArea());

    }

//==================================================================================
// Test which failed on sep 25, 1997
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed3)
    {
  
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(0.000, 0.000));
    TheLinear1.AppendPoint(HGF2DPosition(0.000, 256.0));
    TheLinear1.AppendPoint(HGF2DPosition(256.0, 256.0));
    TheLinear1.AppendPoint(HGF2DPosition(256.0, 0.000));
    TheLinear1.AppendPoint(HGF2DPosition(0.000, 0.000));

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(0.00000000000000000 , 192.000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(26.0000000000000430 , 253.252161511417650));
    TheLinear2.AppendPoint(HGF2DPosition(26.0000000000000430 , 409.000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(441.000000000000060 , 409.000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(441.000000000000060 , 0.00000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(26.0000000000000430 , 0.00000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(26.0000000000000430 , 180.963654778550250));
    TheLinear2.AppendPoint(HGF2DPosition(0.00000000000000000 , 192.000000000000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);
    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(59819.7505875272618, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(176391.000000000029, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(5716.24941247273272, pResult->CalculateArea());
    
    HGF2DRectangle  Rect1(0.0, 0.0, 256.0, 256.0);

    pResult = Rect1.DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(5716.24941247273363, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShape(*pShape2);
    ASSERT_DOUBLE_EQ(110855.000000000029, pResult->CalculateArea());    
    
    }

//==================================================================================
// Test which failed on sep 26, 1997
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed4)
    {
       
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(126.000002333325810 , 374.000007290433640));
    TheLinear1.AppendPoint(HGF2DPosition(382.000007298135420 , 374.000007365927220));
    TheLinear1.AppendPoint(HGF2DPosition(382.000007373628990 , 118.000002401117640));
    TheLinear1.AppendPoint(HGF2DPosition(126.000002408819400 , 118.000002325624050));
    TheLinear1.AppendPoint(HGF2DPosition(126.000002333325810 , 374.000007290433640));

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(126.000002405280640 , 130.000002633843170));
    TheLinear2.AppendPoint(HGF2DPosition(382.000007298135420 , 130.000002709336770));
    TheLinear2.AppendPoint(HGF2DPosition(382.000007301674200 , 118.000002401117640));
    TheLinear2.AppendPoint(HGF2DPosition(126.000002408819400 , 118.000002325624050));
    TheLinear2.AppendPoint(HGF2DPosition(126.000002405280640 , 130.000002633843170));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);
    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(3072.00015698717970, pResult->CalculateArea());    

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.002562214824, pResult->CalculateArea());   

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(62464.0024228270776, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
       
    }

//==================================================================================
// Test which failed on sep 26, 1997
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed5)
    {
       
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(126.000002408819410 , 118.0000023256240000));
    TheLinear1.AppendPoint(HGF2DPosition(382.000007298135470 , 118.0000024011175800));
    TheLinear1.AppendPoint(HGF2DPosition(382.000007370090320 , -126.000002330966550));
    TheLinear1.AppendPoint(HGF2DPosition(126.000002480774240 , -126.000002406460140));
    TheLinear1.AppendPoint(HGF2DPosition(126.000002408819410 , 118.0000023256240000));

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(126.000002257832220 , -126.000002406460110));
    TheLinear2.AppendPoint(HGF2DPosition(126.000002257832220 , 130.0000026338431100));
    TheLinear2.AppendPoint(HGF2DPosition(382.000007298135420 , 130.0000026338431100));
    TheLinear2.AppendPoint(HGF2DPosition(382.000007298135420 , -126.000002406460110));
    TheLinear2.AppendPoint(HGF2DPosition(126.000002257832220 , -126.000002406460110));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);
    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(62464.0024044066740, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.0025806352787, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShape(*pShape2);
    ASSERT_DOUBLE_EQ(3072.00017534392236, pResult->CalculateArea());
      
    }

//==================================================================================
// Test which failed on sep 29, 1997
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed6)
    {
  
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(600.76736535497 , -864.4478207542));
    TheLinear1.AppendPoint(HGF2DPosition(598.11357904592, -16.448934551255));
    TheLinear1.AppendPoint(HGF2DPosition(1210.1105822119, -14.533701979160));
    TheLinear1.AppendPoint(HGF2DPosition(1212.7643685210, -862.52954950332));
    TheLinear1.AppendPoint(HGF2DPosition(600.76736535497 , -864.4478207542));

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(0.50213688201825, 0.00000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(0.00000000000000, 160.453759742730));
    TheLinear2.AppendPoint(HGF2DPosition(115.799175663380, 160.816150983050));
    TheLinear2.AppendPoint(HGF2DPosition(116.301312545400, 0.36239124032448));
    TheLinear2.AppendPoint(HGF2DPosition(0.50213688201825, 0.00000000000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);
    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(537557.528943443554, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(518976.933863138081, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShape(*pShape2);
    ASSERT_DOUBLE_EQ(18580.5950803055275, pResult->CalculateArea());
    
    }

//==================================================================================
// Test which failed on April 18, 2002
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  AllocateCopyInCoordSysTestWhoFailed)
    {
   

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

    HGF2DPolySegment  AddPolySegment1;
    AddPolySegment1.AppendPoint(HGF2DPosition(402409.93750000, 4693423.0000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(402516.78125000, 4693437.0000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(402484.00000000, 4693438.0000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(402409.93750000, 4693423.0000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(AddPolySegment1);

    HFCPtr<HGF2DShape> pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(MyModel));

    }

//==================================================================================
// Test which failed on sep 29, 1997
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed7) 
    {
    
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(126.000002257832220 , 630.000012255243180));
    TheLinear1.AppendPoint(HGF2DPosition(382.000007222641780 , 630.000012330736810));
    TheLinear1.AppendPoint(HGF2DPosition(382.000007298135420 , 374.000007365927160));
    TheLinear1.AppendPoint(HGF2DPosition(126.000002333325810 , 374.000007290433590));
    TheLinear1.AppendPoint(HGF2DPosition(126.000002257832220 , 630.000012255243180));

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(126.000002329787050 , 386.000007674146330));
    TheLinear2.AppendPoint(HGF2DPosition(382.000007298135420 , 386.000007749639910));
    TheLinear2.AppendPoint(HGF2DPosition(382.000007301674200 , 374.000007365927160));
    TheLinear2.AppendPoint(HGF2DPosition(126.000002333325810 , 374.000007290433590));
    TheLinear2.AppendPoint(HGF2DPosition(126.000002329787050 , 386.000007674146330));

    HGF2DLiteSegment LiteSegment1(HGF2DPosition(382.00000729814, 386.00000774964),
                                  HGF2DPosition(382.00000729814, 374.00000736593));
    HGF2DLiteSegment LiteSegment2(HGF2DPosition(382.00000729814, 386.00000774964),
                                  HGF2DPosition(382.00000730167, 374.00000736593));


    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);
    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    // Resumate of problem
    ASSERT_FALSE(LiteSegment1.Crosses(LiteSegment2));

    HFCPtr<HGF2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(3072.00017721946596, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.002581541194, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(62464.0024219211700, pResult->CalculateArea());  

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);   
    
    }

//==================================================================================
// Test which failed on sep 30, 1997
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed8)
    {
   
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(244.000000000000000 , -0.0000001474484110));
    TheLinear1.AppendPoint(HGF2DPosition(-12.000000000000014 , -0.0000000719548210));
    TheLinear1.AppendPoint(HGF2DPosition(-11.999999926865598 , 248.000000075493490));
    TheLinear1.AppendPoint(HGF2DPosition(244.000000073134430 , 247.999999999999890));
    TheLinear1.AppendPoint(HGF2DPosition(244.000000000000000 , -0.0000001474484110));

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(0.00000000000000000 , -0.0000000000000570));
    TheLinear2.AppendPoint(HGF2DPosition(0.00000000000000000 , 247.999999999999940));
    TheLinear2.AppendPoint(HGF2DPosition(244.000000000000000 , 247.999999999999940));
    TheLinear2.AppendPoint(HGF2DPosition(244.000000000000000 , -0.0000000000000570));
    TheLinear2.AppendPoint(HGF2DPosition(0.00000000000000000 , -0.0000000000000570));

    HGF2DLiteSegment LiteSegment1(HGF2DPosition(-11.999999926865598, 248.000000075493490),
                                  HGF2DPosition(244.000000073134430, 247.999999999999890));
    HGF2DLiteSegment LiteSegment2(HGF2DPosition(0.00000000000000000, 247.999999999999940),
                                  HGF2DPosition(244.000000000000000, 247.999999999999940));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);
    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    // Resumate of problem
    ASSERT_FALSE(LiteSegment1.Crosses(LiteSegment2));

    HFCPtr<HGF2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(60512.00000000000, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(63488.000057073128, pResult->CalculateArea());   

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(2976.00000176938328, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);    

    pResult = pShape1->DifferentiateFromShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
       
    }

//==================================================================================
// Test which failed on sep 30, 1997
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed9)
    {
    
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(256.000002509535310 , -374.000007328770210));
    TheLinear1.AppendPoint(HGF2DPosition(382.000007298135420 , -374.000007365927220));
    TheLinear1.AppendPoint(HGF2DPosition(382.000007308751720 , -337.999994959696720));
    TheLinear1.AppendPoint(HGF2DPosition(256.000002520151610 , -337.999994922539710));
    TheLinear1.AppendPoint(HGF2DPosition(256.000002509535310 , -374.000007328770210));

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(256.000002520151610 , -593.999997479848390));
    TheLinear2.AppendPoint(HGF2DPosition(256.000002520151610 , -337.999994959696780));
    TheLinear2.AppendPoint(HGF2DPosition(512.000005040303220 , -337.999994959696780));
    TheLinear2.AppendPoint(HGF2DPosition(512.000005040303220 , -593.999997479848390));
    TheLinear2.AppendPoint(HGF2DPosition(256.000002520151610 , -593.999997479848390));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);
    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    // Resumate of problem
    HFCPtr<HGF2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(4536.00173557470679, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.0012903176248, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShape(*pShape2);
    ASSERT_DOUBLE_EQ(60999.9995584427088, pResult->CalculateArea());    
   
    }

//==================================================================================
// Test which failed on sep 30, 1997
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed10)
    {
  
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(198.000002966624810 , 748.000013416875620));
    TheLinear1.AppendPoint(HGF2DPosition(349.583085012412430 , 748.000013461576940));
    TheLinear1.AppendPoint(HGF2DPosition(349.583085029172540 , 691.166168779997070));
    TheLinear1.AppendPoint(HGF2DPosition(198.000002983384950 , 691.166168735295740));
    TheLinear1.AppendPoint(HGF2DPosition(198.000002966624810 , 748.000013416875620));

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(0.00000000000000000 , 691.1661687799971800));
    TheLinear2.AppendPoint(HGF2DPosition(0.00000000000000000 , 1040.749253792409500));
    TheLinear2.AppendPoint(HGF2DPosition(349.583085012412430 , 1040.749253792409500));
    TheLinear2.AppendPoint(HGF2DPosition(349.583085012412430 , 691.1661687799971800));
    TheLinear2.AppendPoint(HGF2DPosition(0.00000000000000000 , 691.1661687799971800));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);
    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(8615.0493413454733, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(122208.33332679553, pResult->CalculateArea());   

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);   

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShape(*pShape2);
    ASSERT_DOUBLE_EQ(113593.28399373978, pResult->CalculateArea());
    
    }

//==================================================================================
// Test which failed on sep 30, 1997
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed11)
    {
   
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(243.9999999999999700 , -0.0000001509871700));
    TheLinear1.AppendPoint(HGF2DPosition(-267.999995110684150 , 0.00000000000000800));
    TheLinear1.AppendPoint(HGF2DPosition(-267.999994996853960 , 385.999995224514290));
    TheLinear1.AppendPoint(HGF2DPosition(244.0000001138301400 , 385.999995073527090));
    TheLinear1.AppendPoint(HGF2DPosition(243.9999999999999700 , -0.0000001509871700));

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(0.00000000000000000 , 0.00000000000001400));
    TheLinear2.AppendPoint(HGF2DPosition(0.00000000000000000 , 256.000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(244.000000000000000 , 256.000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(244.000000000000000 , 0.00000000000001400));
    TheLinear2.AppendPoint(HGF2DPosition(0.00000000000000000 , 0.00000000000001400));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);
    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(62464.000000000000, pResult->CalculateArea());    

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(197631.99578891927, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(135167.99561445243, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);   

    pResult = pShape1->DifferentiateFromShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
        
    }

//==================================================================================
// Test which failed on sep 30, 1997
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed12)
    {

    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(78.7500007520188060 , 15.7500001457591220));
    TheLinear1.AppendPoint(HGF2DPosition(78.7500007566634250 , 0.00000001857850100));
    TheLinear1.AppendPoint(HGF2DPosition(15.7500001504037610 , 0.00000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(15.7500001504037610 , 15.7500001457591220));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000000000000000 , 15.7500001457591220));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000000000000000 , 121.999999099797760));
    TheLinear1.AppendPoint(HGF2DPosition(134.249998678397080 , 121.999999099797760));
    TheLinear1.AppendPoint(HGF2DPosition(134.249998678397080 , 15.7500001457591220));
    TheLinear1.AppendPoint(HGF2DPosition(78.7500007520188060 , 15.7500001457591220));

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(0.00000000000000000 , 0.00000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(0.00000000000000000 , 663.000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(841.000000000000000 , 663.000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(841.000000000000000 , 0.00000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(0.00000000000000000 , 0.00000000000000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);
    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(15256.3122372689940, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(557583.000000000000, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShape(*pShape2);
    ASSERT_DOUBLE_EQ(542326.68775506504, pResult->CalculateArea());   
   
    }

//==================================================================================
// Test which failed on sep 30, 1997
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed13)
    {
    
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(1.4309074174435E-6, 439.00001100173));
    TheLinear1.AppendPoint(HGF2DPosition(256.00000639572000, 439.00001107722));
    TheLinear1.AppendPoint(HGF2DPosition(256.00000646767000, 194.99999970023));
    TheLinear1.AppendPoint(HGF2DPosition(1.5028622470936E-6, 194.99999962474));
    TheLinear1.AppendPoint(HGF2DPosition(1.4309074174435E-6, 439.00001100173));

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(0.000000000000000, 194.99999970023));
    TheLinear2.AppendPoint(HGF2DPosition(0.000000000000000, 707.00000306043));
    TheLinear2.AppendPoint(HGF2DPosition(512.0000033602000, 707.00000306043));
    TheLinear2.AppendPoint(HGF2DPosition(512.0000033602000, 194.99999970023));
    TheLinear2.AppendPoint(HGF2DPosition(0.000000000000000, 194.99999970023));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);
    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(62464.004123923172, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(262144.00344084483, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);   

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);    

    pResult = pShape1->DifferentiateFromShape(*pShape2);
    ASSERT_DOUBLE_EQ(199679.99932658442, pResult->CalculateArea());   
    
    }

//==================================================================================
// Test which failed on sep 30, 1997
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed14)
    {
    
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(382.000007298135420 , 374.000007365927220));
    TheLinear1.AppendPoint(HGF2DPosition(630.000012107794650 , 374.000007439061620));
    TheLinear1.AppendPoint(HGF2DPosition(630.000012179749550 , 130.000002633843140));
    TheLinear1.AppendPoint(HGF2DPosition(382.000007370090260 , 130.000002560708710));
    TheLinear1.AppendPoint(HGF2DPosition(382.000007298135420 , 374.000007365927220));

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(382.000007298135470 , 130.000002633843110));
    TheLinear2.AppendPoint(HGF2DPosition(382.000007298135470 , 386.000007674146330));
    TheLinear2.AppendPoint(HGF2DPosition(638.000012338438640 , 386.000007674146330));
    TheLinear2.AppendPoint(HGF2DPosition(638.000012338438640 , 130.000002633843110));
    TheLinear2.AppendPoint(HGF2DPosition(382.000007298135470 , 130.000002633843110));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);
    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(60512.002365251079, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.002580635250, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);   

    pResult = pShape1->DifferentiateFromShape(*pShape2);
    ASSERT_DOUBLE_EQ(5024.0002156743722, pResult->CalculateArea());   

    }

//==================================================================================
// Test which failed on oct 1, 1997
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed15)
    {
   
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(6.65414858785482000 , 204.415769530740250));
    TheLinear1.AppendPoint(HGF2DPosition(256.000000000000000 , 204.415769457208940));
    TheLinear1.AppendPoint(HGF2DPosition(256.000000015211980 , 256.000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(6.65414860306680100 , 256.000000073531340));
    TheLinear1.AppendPoint(HGF2DPosition(6.65414858785482000 , 204.415769530740250));

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(0.00000000000000000 , 0.00000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(0.00000000000000000 , 256.000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(256.000000000000000 , 256.000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(256.000000000000000 , 0.00000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(0.00000000000000000 , 0.00000000000000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);
    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(12862.3138841326217, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.0000000000000, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);    

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShape(*pShape2);
    ASSERT_DOUBLE_EQ(52673.686125671731, pResult->CalculateArea());
        
    }

//==================================================================================
// Test which failed on oct 1, 1997
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed16)
    {
   
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(243.999999924491790 , -192.000000147448450));
    TheLinear1.AppendPoint(HGF2DPosition(-12.000000151016373 , -192.000000071954870));
    TheLinear1.AppendPoint(HGF2DPosition(-12.000000075522783 , 64.00000000355332500));
    TheLinear1.AppendPoint(HGF2DPosition(243.999999999985360 , 63.99999992805972900));
    TheLinear1.AppendPoint(HGF2DPosition(243.999999924491790 , -192.000000147448450));

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(-0.000000000000057 , 0.0000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(-0.000000000000057 , 64.000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(63.999999999999943 , 64.000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(63.999999999999943 , 0.0000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(-0.000000000000057 , 0.0000000000000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);
    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(4096.00000000000000, pResult->CalculateArea());    

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.0000773128995, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(61440.000040960411, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);    

    }



//==================================================================================
// Test which failed on oct 6, 1997
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  CalculateSpatialPositionOfNonCrossingLinearTestWhoFailed)
    {

    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(200554.736158946010000 , -6398.2045972025953000));
    TheLinear1.AppendPoint(HGF2DPosition(196150.683067803180000 , -53764.925311697683000));
    TheLinear1.AppendPoint(HGF2DPosition(172671.965121527440000 , -51581.925965582428000));
    TheLinear1.AppendPoint(HGF2DPosition(172671.965121527440000 , -6398.2045972025953000));
    TheLinear1.AppendPoint(HGF2DPosition(200554.736158946010000 , -6398.2045972025953000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DSegment    MySegment(HGF2DPosition(200554.736158945980000 , -6398.2045972025953000),
                              HGF2DPosition(198455.729354262730000 , -28973.561214771802000));

    pShape1->CalculateSpatialPositionOfNonCrossingLinear(MySegment);

    }

//==================================================================================
// Test which failed on dec 16, 1997
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWhoFailed)
    {
   
    HGF2DRectangle  Shape1(-3579929.5748470003, 85666005.023929002, 797561.58597685, 90043496.184753);

    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(-3220838.5030607, 85666005.023929));
    TheLinear1.AppendPoint(HGF2DPosition(-3579929.5748470, 85666005.023929));
    TheLinear1.AppendPoint(HGF2DPosition(-3579929.5748470, 90043496.184753));
    TheLinear1.AppendPoint(HGF2DPosition(-3220838.5030607, 90043496.184753));
    TheLinear1.AppendPoint(HGF2DPosition(-3220838.5030607, 85666005.023929));

    HGF2DPolygonOfSegments  Shape2(TheLinear1);

    HFCPtr<HGF2DShape> pResult = Shape1.IntersectShape(Shape2);
    ASSERT_DOUBLE_EQ(1571917992675.34570, pResult->CalculateArea());   

    pResult = Shape1.DifferentiateShape(Shape2);
    ASSERT_DOUBLE_EQ(17590510870416.2519, pResult->CalculateArea());    

    pResult = Shape1.UnifyShape(Shape2);
    ASSERT_DOUBLE_EQ(19162428863091.5976, pResult->CalculateArea());    
    
    }

//==================================================================================
// Test which failed on dec 19, 1997
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWhoFailed2)
    {
         
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(-3579929.5748469676, 76911022.702281535));
    TheLinear1.AppendPoint(HGF2DPosition(-3579929.5748469676, 81288513.863105357));
    TheLinear1.AppendPoint(HGF2DPosition(797561.585976853970, 81288513.863105357));
    TheLinear1.AppendPoint(HGF2DPosition(797561.585976853970, 76911022.702281535));
    TheLinear1.AppendPoint(HGF2DPosition(-3579929.5748469676, 76911022.702281535));

    HGF2DPolygonOfSegments  Shape1(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(-3220838.5030606692, 76911022.702281520));
    TheLinear2.AppendPoint(HGF2DPosition(-3579929.5748469797, 76911022.702281520));
    TheLinear2.AppendPoint(HGF2DPosition(-3579929.5748469797, 81288513.863105103));
    TheLinear2.AppendPoint(HGF2DPosition(-3220838.5030606692, 81288513.863105103));
    TheLinear2.AppendPoint(HGF2DPosition(-3220838.5030606692, 76911022.702281520));

    HGF2DPolygonOfSegments  Shape2(TheLinear2);

    HFCPtr<HGF2DShape> pResult = Shape1.IntersectShape(Shape2);
    ASSERT_DOUBLE_EQ(1571917992675.24121, pResult->CalculateArea());    

    pResult = Shape1.DifferentiateShape(Shape2);
    ASSERT_DOUBLE_EQ(17590510870415.4765, pResult->CalculateArea());    

    pResult = Shape1.UnifyShape(Shape2);
    ASSERT_DOUBLE_EQ(19162428863090.195, pResult->CalculateArea());    
    
    }

//==================================================================================
// Test which failed on Jan 16 1998
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  IntersectShapeTestWhoFailed2)
    {
  
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(343186.275207202940000, 5067813.655207243700000));
    TheLinear1.AppendPoint(HGF2DPosition(343186.275207202940000, 5062784.982890859200000));
    TheLinear1.AppendPoint(HGF2DPosition(343666.732587660310000, 5062784.982890859200000));
    TheLinear1.AppendPoint(HGF2DPosition(343656.275207202940000, 5067814.982890859200000));
    TheLinear1.AppendPoint(HGF2DPosition(343656.275206638850000, 5067814.982890858300000));
    TheLinear1.AppendPoint(HGF2DPosition(343186.275207202940000, 5067813.655207243700000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(343656.27520720294, 5067814.9828908592));
    TheLinear2.AppendPoint(HGF2DPosition(343186.27520720294, 5067813.6552072437));
    TheLinear2.AppendPoint(HGF2DPosition(343186.27520720294, 5062784.9828908592));
    TheLinear2.AppendPoint(HGF2DPosition(343666.73258766031, 5062784.9828908592));
    TheLinear2.AppendPoint(HGF2DPosition(343656.27520720294, 5067814.9828908592));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_DOUBLE_EQ(2390088.30620082330, pResult->CalculateArea());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(2390088.30620074272, pResult->CalculateArea());
    
    }

//==================================================================================
// Test which failed on Jan 19 1998
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed17)
    {
   
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(2029951690.000000200000000 , 2069995506.99999980000000));
    TheLinear1.AppendPoint(HGF2DPosition(2029909908.805370600000000 , 2070045299.88880250000000));
    TheLinear1.AppendPoint(HGF2DPosition(2029978852.805251400000000 , 2070103150.77367420000000));
    TheLinear1.AppendPoint(HGF2DPosition(2030020633.999881000000000 , 2070053357.88487150000000));
    TheLinear1.AppendPoint(HGF2DPosition(2029951690.000000200000000 , 2069995506.99999980000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(2029951690.000000200000000 , 2069995506.99999980000000));
    TheLinear2.AppendPoint(HGF2DPosition(2029945974.905895500000000 , 2070002317.98393110000000));
    TheLinear2.AppendPoint(HGF2DPosition(2029945974.905895500000000 , 2070031573.10052470000000));
    TheLinear2.AppendPoint(HGF2DPosition(2029982041.006420400000000 , 2070031573.10052470000000));
    TheLinear2.AppendPoint(HGF2DPosition(2029982041.006420400000000 , 2070020974.51829270000000));
    TheLinear2.AppendPoint(HGF2DPosition(2029951690.000000200000000 , 2069995506.99999980000000));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed18)
    {
   
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(0.000 , -1.490116119384800E-8));
    TheLinear1.AppendPoint(HGF2DPosition(0.000 , 255.99999998509884010));
    TheLinear1.AppendPoint(HGF2DPosition(256.0 , 255.99999998509884010));
    TheLinear1.AppendPoint(HGF2DPosition(256.0 , -1.490116119384800E-8));
    TheLinear1.AppendPoint(HGF2DPosition(0.000 , -1.490116119384800E-8));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition( -1.4901161193848E-8, 237.11464065313000000));
    TheLinear2.AppendPoint(HGF2DPosition( -1.4901161193848E-8, 255.99999997020000000));
    TheLinear2.AppendPoint(HGF2DPosition( 256.000000000000000, 255.99999998509884010));
    TheLinear2.AppendPoint(HGF2DPosition( 256.000000000000000, -1.490116119384800E-8));
    TheLinear2.AppendPoint(HGF2DPosition( 198.962807521220000, -1.490116119384800E-8));
    TheLinear2.AppendPoint(HGF2DPosition( -1.4901161193848E-8, 237.11464065313000000));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed19)
    {
    
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(0.00000000000000000, 0.000000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(-1.4901161193848E-8, 256.0000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(256.000000000000000, 256.0000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(256.000000000000000,  -1.4901161193848E-8));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000000000000000, 0.000000000000000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(0.0, 0.0 ));
    TheLinear2.AppendPoint(HGF2DPosition(0.0, 256.0));
    TheLinear2.AppendPoint(HGF2DPosition(256.0, 256.0));
    TheLinear2.AppendPoint(HGF2DPosition(256.0, 0.0));
    TheLinear2.AppendPoint(HGF2DPosition(0.0, 0.0 ));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed20)
    {
   
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(-1.490116119384800E-8, -1.490116119384800E-8));
    TheLinear1.AppendPoint(HGF2DPosition(-1.490116119384800E-8, 255.99999998509884010));
    TheLinear1.AppendPoint(HGF2DPosition(255.99999998509884010, 256.00000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(255.99999998509884010, 0.0000000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(-1.490116119384800E-8, -1.490116119384800E-8));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(-1.490127488068500E-8, -2.2737367544323E-13 ));
    TheLinear2.AppendPoint(HGF2DPosition(-1.490127488068500E-8, 256.00000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(255.99999998509884010, 256.00000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(255.99999998509884010, 0.0000000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(-1.490127488068500E-8, -2.2737367544323E-13 ));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed21)
    {
   
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(0.0000000000000, -9.313225746154800E-9));
    TheLinear1.AppendPoint(HGF2DPosition(0.0000000000000, 255.99999998509884010));
    TheLinear1.AppendPoint(HGF2DPosition(63.277044698596, 255.99999999255000000));
    TheLinear1.AppendPoint(HGF2DPosition(90.183728918433, -3.725290298461900E-9));
    TheLinear1.AppendPoint(HGF2DPosition(0.0000000000000, -9.313225746154800E-9));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(0.0, 0.0));
    TheLinear2.AppendPoint(HGF2DPosition(0.0, 256.0 ));
    TheLinear2.AppendPoint(HGF2DPosition(256.0, 256.0));
    TheLinear2.AppendPoint(HGF2DPosition(256.0, 0.0));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed22)
    {
  
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(2029947377.3103, 2070045465.0066));
    TheLinear1.AppendPoint(HGF2DPosition(2029947377.3103, 2070057954.5083));
    TheLinear1.AppendPoint(HGF2DPosition(2029959866.8120, 2070057954.5083));
    TheLinear1.AppendPoint(HGF2DPosition(2029959866.8120, 2070045465.0066));
    TheLinear1.AppendPoint(HGF2DPosition(2029947377.3103, 2070045465.0066));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(2029947377.3103, 2070045465.0066 ));
    TheLinear2.AppendPoint(HGF2DPosition(2029947377.3103, 2070057954.5083 ));
    TheLinear2.AppendPoint(HGF2DPosition(2029959866.8120, 2070057954.5083 ));
    TheLinear2.AppendPoint(HGF2DPosition(2029959866.8120, 2070045465.0066 ));
    TheLinear2.AppendPoint(HGF2DPosition(2029947377.3103, 2070045465.0066 ));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
// Test which failed on Feb 09 1998
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed23)
    {
    
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(15282.044160147476000 , -10973.717432186515000 ));
    TheLinear1.AppendPoint(HGF2DPosition(28082.024664637727000 , -10996.057635270005000 ));
    TheLinear1.AppendPoint(HGF2DPosition(28104.364867721215000 , 1803.92286922024500000 ));
    TheLinear1.AppendPoint(HGF2DPosition(15304.384363230965000 , 1826.26307230373460000 ));
    TheLinear1.AppendPoint(HGF2DPosition(15282.044160147476000 , -10973.717432186515000 ));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(15282.044160147572000 , -11041.43617278322900 ));
    TheLinear2.AppendPoint(HGF2DPosition(15282.044160147572000 , 14603.205242364347000 ));
    TheLinear2.AppendPoint(HGF2DPosition(40926.685575295152000 , 14603.205242364347000 ));
    TheLinear2.AppendPoint(HGF2DPosition(40926.685575295152000 , -11041.43617278322900 ));
    TheLinear2.AppendPoint(HGF2DPosition(15282.044160147572000 , -11041.43617278322900 ));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed25)
    {
   
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(-4.8885340220295E-12 , 0.0000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(110.8848526511200000 , 63.947435208261 ));
    TheLinear1.AppendPoint(HGF2DPosition(-2.8421709430404E-14 , 256.00000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(-256.000000000000000 , 256.00000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(-256.000000000000000 , 0.0000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(-4.8885340220295E-12 , 0.0000000000000 ));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(0.000, 0.000));
    TheLinear2.AppendPoint(HGF2DPosition(0.000, 256.0));
    TheLinear2.AppendPoint(HGF2DPosition(128.0, 256.0));
    TheLinear2.AppendPoint(HGF2DPosition(128.0, 0.000));
    TheLinear2.AppendPoint(HGF2DPosition(0.000, 0.000));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed26)
    {
    
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(-10362.597254913127000 , 36528.9980348294920000));
    TheLinear1.AppendPoint(HGF2DPosition(-10362.597254332386000 , 36528.9980355215960000));
    TheLinear1.AppendPoint(HGF2DPosition(-10362.597254999999000 , 36528.9980349999970000));
    TheLinear1.AppendPoint(HGF2DPosition(-65772.130034309317000 , 107449.965859604910000));
    TheLinear1.AppendPoint(HGF2DPosition(-14551.431049872443000 , 147467.961755772760000));
    TheLinear1.AppendPoint(HGF2DPosition(36652.0408752940860000 , 81930.5063259228050000));
    TheLinear1.AppendPoint(HGF2DPosition(100362.597255166530000 , 28471.0019654725110000));
    TheLinear1.AppendPoint(HGF2DPosition(58581.4026255414790000 , -21321.886837261045000));
    TheLinear1.AppendPoint(HGF2DPosition(-10362.597254455088000 , 36528.9980339305330000));
    TheLinear1.AppendPoint(HGF2DPosition(30496.5477215592730000 , -43661.589141953082000));
    TheLinear1.AppendPoint(HGF2DPosition(-27418.876350684626000 , -73170.971625023667000));
    TheLinear1.AppendPoint(HGF2DPosition(-68278.021327243885000 , 7019.61555192941520000));
    TheLinear1.AppendPoint(HGF2DPosition(-10362.597255000004000 , 36528.9980349999970000));
    TheLinear1.AppendPoint(HGF2DPosition(-10362.597254913127000 , 36528.9980348294920000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(-87507.654318190165000 , 82882.424776904780000));
    TheLinear2.AppendPoint(HGF2DPosition(-10362.597254999990000 , 36528.998035000004000));
    TheLinear2.AppendPoint(HGF2DPosition(-68278.021327243885000 , 7019.6155519294152000));
    TheLinear2.AppendPoint(HGF2DPosition(-59816.342063631593000 , -9587.365065051119000));
    TheLinear2.AppendPoint(HGF2DPosition(-120985.12918734361000 , 27166.550231267436000));
    TheLinear2.AppendPoint(HGF2DPosition(-87507.654318190165000 , 82882.424776904780000));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed27) 
    {

    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(-10362.597255000001000 , 36528.998034999997000 ));
    TheLinear1.AppendPoint(HGF2DPosition(-97689.212619839687000 , 58301.968638970102000 ));
    TheLinear1.AppendPoint(HGF2DPosition(-81964.289405861273000 , 121371.19084690989000 ));
    TheLinear1.AppendPoint(HGF2DPosition(5362.32595897841020000 , 99598.220242939773000 ));
    TheLinear1.AppendPoint(HGF2DPosition(-10362.597255000001000 , 36528.998034999997000 ));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(-10362.597254836835000 , 36528.998034920412000 ));
    TheLinear2.AppendPoint(HGF2DPosition(-10362.597254332386000 , 36528.998035521596000 ));
    TheLinear2.AppendPoint(HGF2DPosition(-10362.597254999999000 , 36528.998034999997000 ));
    TheLinear2.AppendPoint(HGF2DPosition(-65772.130034309317000 , 107449.96585960491000 ));
    TheLinear2.AppendPoint(HGF2DPosition(-14551.431049872443000 , 147467.96175577276000 ));
    TheLinear2.AppendPoint(HGF2DPosition(36652.0408752940860000 , 81930.506325922805000 ));
    TheLinear2.AppendPoint(HGF2DPosition(100362.597255166530000 , 28471.001965472511000 ));
    TheLinear2.AppendPoint(HGF2DPosition(65684.5981222158590000 , -12856.62808520035500 ));
    TheLinear2.AppendPoint(HGF2DPosition(42034.7423706349950000 , -61346.01818546281700 ));
    TheLinear2.AppendPoint(HGF2DPosition(18406.5218307773900000 , -49821.76501835301300 ));
    TheLinear2.AppendPoint(HGF2DPosition(-27418.876350684626000 , -73170.97162502366700 ));
    TheLinear2.AppendPoint(HGF2DPosition(-59816.342063631593000 , -9587.365065051119000 ));
    TheLinear2.AppendPoint(HGF2DPosition(-120985.12918734361000 , 27166.550231267436000 ));
    TheLinear2.AppendPoint(HGF2DPosition(-87507.654318190165000 , 82882.424776904780000 ));
    TheLinear2.AppendPoint(HGF2DPosition(-10362.597254999990000 , 36528.998035000004000 ));
    TheLinear2.AppendPoint(HGF2DPosition(-10362.597254836835000 , 36528.998034920412000 ));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed28)
    {

    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(251.3389155211043500 , 1878.728020202368500 ));
    TheLinear1.AppendPoint(HGF2DPosition(0.000000000000000000 , 2306.538940237835000 ));
    TheLinear1.AppendPoint(HGF2DPosition(556.2084000000031700 , 2619.805740238167300 ));
    TheLinear1.AppendPoint(HGF2DPosition(597.7641999999759700 , 2533.497540238313400 ));
    TheLinear1.AppendPoint(HGF2DPosition(1157.169200000003900 , 2942.662340237759100 ));
    TheLinear1.AppendPoint(HGF2DPosition(1360.495711533061700 , 2597.007270631380400 ));
    TheLinear1.AppendPoint(HGF2DPosition(1363.329634746885900 , 2598.842491989955300 ));
    TheLinear1.AppendPoint(HGF2DPosition(1736.640594746917500 , 2032.088579989969700 ));
    TheLinear1.AppendPoint(HGF2DPosition(1709.411828939744700 , 1793.080524571239900 ));
    TheLinear1.AppendPoint(HGF2DPosition(1945.621632601774800 , 1445.451002201065400 ));
    TheLinear1.AppendPoint(HGF2DPosition(1695.900754101807300 , 1287.339685107581300 ));
    TheLinear1.AppendPoint(HGF2DPosition(1697.170099164475700 , 1281.223749805241800 ));
    TheLinear1.AppendPoint(HGF2DPosition(1737.691559314960600 , 1230.336334732361100 ));
    TheLinear1.AppendPoint(HGF2DPosition(1973.305540954985200 , 1395.165000000037300 ));
    TheLinear1.AppendPoint(HGF2DPosition(2247.280540955020100 , 986.3100000005215400 ));
    TheLinear1.AppendPoint(HGF2DPosition(2179.840540955017800 , 927.3000000007450600 ));
    TheLinear1.AppendPoint(HGF2DPosition(2382.160540955024800 , 644.8950000004842900 ));
    TheLinear1.AppendPoint(HGF2DPosition(1269.400540955015500 , 0.000000000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(797.3205409549991600 , 708.1200000001117600 ));
    TheLinear1.AppendPoint(HGF2DPosition(917.4365554355317700 , 794.4533854080364100 ));
    TheLinear1.AppendPoint(HGF2DPosition(839.4136326017906000 , 745.0530022010207200 ));
    TheLinear1.AppendPoint(HGF2DPosition(406.5496326017892000 , 1436.433002200909000 ));
    TheLinear1.AppendPoint(HGF2DPosition(478.1214622594416100 , 1490.173787791281900 ));
    TheLinear1.AppendPoint(HGF2DPosition(236.6092827469110500 , 1869.189251990057500 ));
    TheLinear1.AppendPoint(HGF2DPosition(251.3389155211043500 , 1878.728020202368500 ));
    TheLinear1.AppendPoint(HGF2DPosition(941.8916180721134900 , 789.1439447365701200 ));
    TheLinear1.AppendPoint(HGF2DPosition(990.3438919325126300 , 826.1438629571348400 ));
    TheLinear1.AppendPoint(HGF2DPosition(986.1693580946885000 , 837.9717088313773300 ));
    TheLinear1.AppendPoint(HGF2DPosition(932.7557957404642400 , 804.1527957096695900 ));
    TheLinear1.AppendPoint(HGF2DPosition(251.3389155211043500 , 1878.728020202368500 ));


    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(0.0000, 0.0000));
    TheLinear2.AppendPoint(HGF2DPosition(0.0000, 2942.0));
    TheLinear2.AppendPoint(HGF2DPosition(2382.0, 2942.0));
    TheLinear2.AppendPoint(HGF2DPosition(2382.0, 0.0000));
    TheLinear2.AppendPoint(HGF2DPosition(0.0000, 0.0000));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed29)
    {
    
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(-10362.597254836834000 , 36528.998034920412000 ));
    TheLinear1.AppendPoint(HGF2DPosition(-10362.597254332393000 , 36528.998035521588000 ));
    TheLinear1.AppendPoint(HGF2DPosition(-10362.597255000001000 , 36528.998034999997000 ));
    TheLinear1.AppendPoint(HGF2DPosition(-65772.130034309332000 , 107449.96585960491000 ));
    TheLinear1.AppendPoint(HGF2DPosition(-14551.431049872444000 , 147467.96175577276000 ));
    TheLinear1.AppendPoint(HGF2DPosition(36652.0408752940640000 , 81930.506325922819000 ));
    TheLinear1.AppendPoint(HGF2DPosition(100362.597255166530000 , 28471.001965472511000 ));
    TheLinear1.AppendPoint(HGF2DPosition(65684.5981222158590000 , -12856.62808520035500 ));
    TheLinear1.AppendPoint(HGF2DPosition(42034.7423706349950000 , -61346.01818546281700 ));
    TheLinear1.AppendPoint(HGF2DPosition(18406.5218307773830000 , -49821.76501835301300 ));
    TheLinear1.AppendPoint(HGF2DPosition(-27418.876350684630000 , -73170.97162502366700 ));
    TheLinear1.AppendPoint(HGF2DPosition(-59816.342063631586000 , -9587.365065051140800 ));
    TheLinear1.AppendPoint(HGF2DPosition(-120985.12918734361000 , 27166.550231267429000 ));
    TheLinear1.AppendPoint(HGF2DPosition(-87507.654318190165000 , 82882.424776904780000 ));
    TheLinear1.AppendPoint(HGF2DPosition(-10362.597255000001000 , 36528.998034999997000 ));
    TheLinear1.AppendPoint(HGF2DPosition(-10362.597254836834000 , 36528.998034920412000 ));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(-94435.220360731255000 , 71353.018749675452000 ));
    TheLinear2.AppendPoint(HGF2DPosition(-81964.289405861273000 , 121371.19084690987000 ));
    TheLinear2.AppendPoint(HGF2DPosition(-56181.662813618503000 , 114942.86005061449000 ));
    TheLinear2.AppendPoint(HGF2DPosition(-65772.130034309332000 , 107449.96585960491000 ));
    TheLinear2.AppendPoint(HGF2DPosition(-10362.597255000001000 , 36528.998034999997000 ));
    TheLinear2.AppendPoint(HGF2DPosition(-87507.654318190165000 , 82882.424776904780000 ));
    TheLinear2.AppendPoint(HGF2DPosition(-94435.220360731255000 , 71353.018749675452000 ));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed30)
    {
           
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(1397168087.498626900000000 , -1597261746.348763900000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(1396690582.511111700000000 , -1630473344.953076400000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(1427712484.592282800000000 , -1630473344.953076400000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(1427712484.592282800000000 , -1597700911.289259000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(1397168087.498626900000000 , -1597261746.348763900000000 ));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(1397168087.498626700000000 , -1597261746.348764200000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(1394500885.987970100000000 , -1782771904.954012900000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(1557455317.014201400000000 , -1785114850.954406700000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(1560122518.524858000000000 , -1599604692.349158000000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(1397168087.498626700000000 , -1597261746.348764200000000 ));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed31)
    {
        
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(444.110497237569010 , 222.055248618784530 ));
    TheLinear1.AppendPoint(HGF2DPosition(444.110497237569010 , 444.110497237569060 ));
    TheLinear1.AppendPoint(HGF2DPosition(666.165745856353510 , 444.110497237569060 ));
    TheLinear1.AppendPoint(HGF2DPosition(666.165745856353510 , 222.055248618784530 ));
    TheLinear1.AppendPoint(HGF2DPosition(444.110497237569010 , 222.055248618784530 ));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(639.999999999999770 , 222.055248618784530 ));
    TheLinear2.AppendPoint(HGF2DPosition(639.999999999999770 , 256.000000000000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(640.000000000000000 , 256.000000000000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(640.000000000000000 , 222.055248618784530 ));
    TheLinear2.AppendPoint(HGF2DPosition(639.999999999999770 , 222.055248618784530 ));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed32)
    {    
    
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(34844.116927999996000 , 44491.157767857141000 ));
    TheLinear1.AppendPoint(HGF2DPosition(41887.133631999997000 , 44491.157767857141000 ));
    TheLinear1.AppendPoint(HGF2DPosition(41887.133631999990000 , -2278.875032142859700 ));
    TheLinear1.AppendPoint(HGF2DPosition(34844.116927999996000 , -2278.875032142859700 ));
    TheLinear1.AppendPoint(HGF2DPosition(34844.116927999996000 , 44491.157767857141000 ));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(24059.49759999998400 , 26663.521735845723000 ));
    TheLinear2.AppendPoint(HGF2DPosition(6231.861567988563400 , 44491.157767857141000 ));
    TheLinear2.AppendPoint(HGF2DPosition(41887.13363201139900 , 44491.157767857134000 ));
    TheLinear2.AppendPoint(HGF2DPosition(24059.49759999998400 , 26663.521735845723000 ));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed33)
    {
         
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(0.000, 0.00 ));
    TheLinear1.AppendPoint(HGF2DPosition(10.01, 0.10 ));
    TheLinear1.AppendPoint(HGF2DPosition(10.02, 10.2 ));
    TheLinear1.AppendPoint(HGF2DPosition(-0.10, 10.1 ));
    TheLinear1.AppendPoint(HGF2DPosition(0.000, 0.00 ));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HFCPtr<HGF2DShape> pShape2 = new HGF2DRectangle(5.0, -1.0, 5.00000000001, 11.0);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed34)
    {
          
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(0.000, 0.000 ));
    TheLinear1.AppendPoint(HGF2DPosition(0.000, 256.0 ));
    TheLinear1.AppendPoint(HGF2DPosition(256.0, 256.0 ));
    TheLinear1.AppendPoint(HGF2DPosition(256.0, 0.000 ));
    TheLinear1.AppendPoint(HGF2DPosition(0.000, 0.000 ));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(255.99999998156, 166.49374991798 ));
    TheLinear2.AppendPoint(HGF2DPosition(343.31683259336, 363.58989382296 ));
    TheLinear2.AppendPoint(HGF2DPosition(511.00000000000, 304.00000000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(424.68316736055, 107.58989382327 ));
    TheLinear2.AppendPoint(HGF2DPosition(255.99999998156, 166.49374991798 ));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed35)
    {
        
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(0.000000000000000 , 256.000000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(0.000739457713053 , 256.000000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(0.000000000000000 , 255.692716162332320 ));
    TheLinear1.AppendPoint(HGF2DPosition(0.000000000000000 , 256.000000000000000 ));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(-255.385432279390440 , 256.307283860515550 ));
    TheLinear2.AppendPoint(HGF2DPosition(-254.770864558780860 , 511.692716140115580 ));
    TheLinear2.AppendPoint(HGF2DPosition(0.614567720555641000 , 511.078148419505400 ));
    TheLinear2.AppendPoint(HGF2DPosition(-0.00000000005394400 , 255.692716139905340 ));
    TheLinear2.AppendPoint(HGF2DPosition(-255.385432279390440 , 256.307283860515550 ));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed36)
    {
    
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(0.0000000000000000 , 51.046683371067047 ));
    TheLinear1.AppendPoint(HGF2DPosition(14.514608025550842 , 42.666670471429825 ));
    TheLinear1.AppendPoint(HGF2DPosition(0.0000000000000000 , 34.286657571792603 ));
    TheLinear1.AppendPoint(HGF2DPosition(0.0000000000000000 , 51.046683371067047 ));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(51.465018710229799 , 64.000000000000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(0.0000000000000000 , 34.286657594899594 ));
    TheLinear2.AppendPoint(HGF2DPosition(0.0000000000000000 , 64.000000000000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(51.465018710229799 , 64.000000000000000 ));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed37)
    {
  
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(59.405016660690308 , 11.333375036716461 ));
    TheLinear1.AppendPoint(HGF2DPosition(64.000000000000000 , 8.6804601848125460 ));
    TheLinear1.AppendPoint(HGF2DPosition(64.000000000000000 , 0.0000000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(39.775035202503204 , 0.0000000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(59.405016660690308 , 11.333375036716461 ));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(39.775035227748781 , 0.0000000000000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(64.000000000000000 , 13.986289931850713 ));
    TheLinear2.AppendPoint(HGF2DPosition(64.000000000000000 , 0.0000000000000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(39.775035227748781 , 0.0000000000000000 ));

    HFCPtr<HGF2DShape>pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed38)
    {
           
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(0.0000000000000000 , 41.672118563598019 ));
    TheLinear1.AppendPoint(HGF2DPosition(64.000000000000000 , 4.7217013355800860 ));
    TheLinear1.AppendPoint(HGF2DPosition(64.000000000000000 , 0.0000000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(0.0000000000000000 , 0.0000000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(0.0000000000000000 , 41.672118563598019 ));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(12.133750259876251 , 34.666694611310959 ));
    TheLinear2.AppendPoint(HGF2DPosition(0.0000000000000000 , 27.661270588636398 ));
    TheLinear2.AppendPoint(HGF2DPosition(0.0000000000000000 , 41.672118544578552 ));
    TheLinear2.AppendPoint(HGF2DPosition(12.133750259876251 , 34.666694611310959 ));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed39)
    {
          
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(256.000000000000000, 51.200000002614 ));
    TheLinear1.AppendPoint(HGF2DPosition(256.000000000000000, 64.000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(320.000000000000000, 64.000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(320.000000000000000, 0.0000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(256.000000013070000, 0.0000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(256.000000013070000, 51.200000002614 ));
    TheLinear1.AppendPoint(HGF2DPosition(256.000000000000000, 51.200000002614 ));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(256.00000001307, 0.0000000000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(256.00000001307, 51.200000002614 ));
    TheLinear2.AppendPoint(HGF2DPosition(307.00000001307, 51.200000002614 ));
    TheLinear2.AppendPoint(HGF2DPosition(307.00000001307, 0.0000000000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(256.00000001307, 0.0000000000000 ));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed40)
    {
          
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(0.000000000000000 , 41.672118563598019 ));
    TheLinear1.AppendPoint(HGF2DPosition(64.00000000000000 , 4.7217013355800860 ));
    TheLinear1.AppendPoint(HGF2DPosition(64.00000000000000 , 0.0000000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(0.000000000000000 , 0.0000000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(0.000000000000000 , 41.672118563598019 ));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(12.133750259876251 , 34.666694611310959 ));
    TheLinear2.AppendPoint(HGF2DPosition(0.0000000000000000 , 27.661270588636398 ));
    TheLinear2.AppendPoint(HGF2DPosition(0.0000000000000000 , 41.672118544578552 ));
    TheLinear2.AppendPoint(HGF2DPosition(12.133750259876251 , 34.666694611310959 ));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
 TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed41)
    {
    
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(-838960.575561190260000 , 1443576.103989492600000 ));
    TheLinear1.AppendPoint(HGF2DPosition(-413480.105918325430000 , -666897.741530794650000 ));
    TheLinear1.AppendPoint(HGF2DPosition(1706857.907802271400000 , -239428.613782825880000 ));
    TheLinear1.AppendPoint(HGF2DPosition(1281377.438159406400000 , 1871045.231737461400000 ));
    TheLinear1.AppendPoint(HGF2DPosition(-838960.575561190260000 , 1443576.103989492600000 ));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(-838960.575561276990000 , 1443576.103989754800000 ));
    TheLinear2.AppendPoint(HGF2DPosition(-413480.105917771700000 , -666897.741530503150000 ));
    TheLinear2.AppendPoint(HGF2DPosition(1706857.907802418800000 , -239428.613782369070000 ));
    TheLinear2.AppendPoint(HGF2DPosition(1281377.438160031600000 , 1871045.231737896800000 ));
    TheLinear2.AppendPoint(HGF2DPosition(-838960.575561276990000 , 1443576.103989754800000 ));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed42)
    {
          
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(47997.0000001080000, -6.0799102301491E-6 ));
    TheLinear1.AppendPoint(HGF2DPosition(47997.0000001080000, 6.0606155668938e-10 ));
    TheLinear1.AppendPoint(HGF2DPosition(31998.9999999740000, 5.82035241515110E-8 ));
    TheLinear1.AppendPoint(HGF2DPosition(31998.9999999740000, -4.2752602357745E-6 ));
    TheLinear1.AppendPoint(HGF2DPosition(15998.9999999750000, -4.2831004951204E-6 ));
    TheLinear1.AppendPoint(HGF2DPosition(15998.9999999750000, -1.2255894791577E-9 ));
    TheLinear1.AppendPoint(HGF2DPosition(6.8693350428327E-12, 1.25284195532590E-9 ));
    TheLinear1.AppendPoint(HGF2DPosition(1.75541220750240E-8, 11998.9999562820000 ));
    TheLinear1.AppendPoint(HGF2DPosition(1.1692050834664E-11, 12000.0000000010000 ));
    TheLinear1.AppendPoint(HGF2DPosition(8.44476795030410E-8, 23997.9998530850000 ));
    TheLinear1.AppendPoint(HGF2DPosition(6.69925762238890E-8, 23998.9999086140000 ));
    TheLinear1.AppendPoint(HGF2DPosition(1.30288784238330E-7, 35996.9998298540000 ));
    TheLinear1.AppendPoint(HGF2DPosition(8.44457848633660E-8, 35997.9998767400000 ));
    TheLinear1.AppendPoint(HGF2DPosition(1.47869624092590E-7, 47995.9997635870000 ));
    TheLinear1.AppendPoint(HGF2DPosition(1.23085794970060E-7, 47996.9997648790000 ));
    TheLinear1.AppendPoint(HGF2DPosition(1.67107853747390E-7, 60504.0481535610000 ));
    TheLinear1.AppendPoint(HGF2DPosition(60505.3091066160000, 60504.0481535610000 ));
    TheLinear1.AppendPoint(HGF2DPosition(60505.3091064030000, -6.2438056493748e-6 ));
    TheLinear1.AppendPoint(HGF2DPosition(47997.0000001080000, -6.0799102301491E-6 ));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(0.0000000000000, 0.00000 ));
    TheLinear2.AppendPoint(HGF2DPosition(0.0000000000000, 12000.0 ));
    TheLinear2.AppendPoint(HGF2DPosition(15999.000000024, 12000.0 ));
    TheLinear2.AppendPoint(HGF2DPosition(15999.000000024, 0.00000 ));
    TheLinear2.AppendPoint(HGF2DPosition(0.0000000000000, 0.00000 ));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed43)
    {
            
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(-32000.000000000029000 , 0.0000000149011610000));
    TheLinear1.AppendPoint(HGF2DPosition(-32000.000000000029000 , 12000.000000015267000));
    TheLinear1.AppendPoint(HGF2DPosition(-16000.000000000029000 , 12000.000000008227000));
    TheLinear1.AppendPoint(HGF2DPosition(9876.00000000000000000 , 12000.000000011541000));
    TheLinear1.AppendPoint(HGF2DPosition(9876.00000000000000000 , 0.0000000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(-32000.000000000029000 , 0.0000000149011610000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(-0.00000000002910400 , 12000.000000003360000));
    TheLinear2.AppendPoint(HGF2DPosition(-0.00000000002910400 , 24000.000000006719000));
    TheLinear2.AppendPoint(HGF2DPosition(8667.999999999970900 , 24000.000000002212000));
    TheLinear2.AppendPoint(HGF2DPosition(8667.999999999970900 , 11999.999999998852000));
    TheLinear2.AppendPoint(HGF2DPosition(-0.00000000002910400 , 12000.000000003360000));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  AllocateCopyInCoordSysTestWhoFailed2)
    {


    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(24998.4375000170640 , 0.0000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(24998.4375000170640 , 0.0000594766647000));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000000000000000 , 0.0000595476536000));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000000000000000 , 18750.000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0000000000000 , 18750.000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0000000000000 , 18749.999993365014));
    TheLinear1.AppendPoint(HGF2DPosition(24998.4374999393650 , 18749.999993365020));
    TheLinear1.AppendPoint(HGF2DPosition(24998.4375000608570 , 0.0000067755764000));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0000000000000 , 0.0000067755688000));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0000000000000 , 0.0000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(24998.4375000170640 , 0.0000000000000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    #ifdef WIP_IPPTEST_BUG_2
    // This tests what happens when a polygon of segments becomes autocontiguous
    // as a result of scaling ... the result shoud be complex
    HFCPtr<HGF2DShape> pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 10.0, 10.0)));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
    
    pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 100.0, 100.0)));
    //ASSERT_TRUE(pResult->IsComplex()); 
    //ASSERT_EQ(2, pResult->GetShapeList().size());    
    #endif

    }

//==================================================================================
// Additional AllocateCopyInCoordSys tests
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  AllocateCopyInCoordSysTestWhoFailed3)
    {
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(25000.0 , 0.00000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000 , 0.00000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000 , 18000.0000));
    TheLinear1.AppendPoint(HGF2DPosition(12000.0 , 18000.0000));
    TheLinear1.AppendPoint(HGF2DPosition(12000.0 , 0.00000005));
    TheLinear1.AppendPoint(HGF2DPosition(18000.0 , 0.00000005));
    TheLinear1.AppendPoint(HGF2DPosition(18000.0 , 18000.0000));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0 , 18000.0000));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0 , 0.00000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HFCPtr<HGF2DShape> pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 10.0, 10.0)));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
    
    pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 100.0, 100.0)));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
       
    }

//==================================================================================
// Additional AllocateCopyInCoordSys tests
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  AllocateCopyInCoordSysTestWhoFailed4)
    {
    
    HGF2DPolySegment  TheLinear1;
    TheLinear1.AppendPoint(HGF2DPosition(25000.0 , 0.00000000));
    TheLinear1.AppendPoint(HGF2DPosition(12000.0 , 0.00000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000 , 0.00000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000 , 18000.0000));
    TheLinear1.AppendPoint(HGF2DPosition(12000.0 , 18000.0000));
    TheLinear1.AppendPoint(HGF2DPosition(12000.0 , 0.00000005));
    TheLinear1.AppendPoint(HGF2DPosition(18000.0 , 0.00000005));
    TheLinear1.AppendPoint(HGF2DPosition(18000.0 , 18000.0000));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0 , 18000.0000));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0 , 0.00000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HFCPtr<HGF2DShape> pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 10.0, 10.0)));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
    
    pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 100.0, 100.0)));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
    
    }

//==================================================================================
// Additional AllocateCopyInCoordSys tests
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  AllocateCopyInCoordSysTestWhoFailed5)
    {
    
    
    HGF2DPolySegment  TheLinear1;
    TheLinear1.AppendPoint(HGF2DPosition(25000.0 , -10000.000));
    TheLinear1.AppendPoint(HGF2DPosition(15000.0 , -10000.000));
    TheLinear1.AppendPoint(HGF2DPosition(15000.0 , 0.00000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000 , 0.00000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000 , 18000.0000));
    TheLinear1.AppendPoint(HGF2DPosition(12000.0 , 18000.0000));
    TheLinear1.AppendPoint(HGF2DPosition(12000.0 , 0.00000005));
    TheLinear1.AppendPoint(HGF2DPosition(18000.0 , 0.00000005));
    TheLinear1.AppendPoint(HGF2DPosition(18000.0 , 18000.0000));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0 , 18000.0000));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0 , -10000.000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HFCPtr<HGF2DShape> pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 10.0, 10.0)));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());    

    pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 100.0, 100.0)));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
    
    }

//==================================================================================
// Additional AllocateCopyInCoordSys tests
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  AllocateCopyInCoordSysTestWhoFailed6)
    {
   
    
    HGF2DPolySegment  TheLinear1;
    TheLinear1.AppendPoint(HGF2DPosition(25000.0 , -10000.00000));
    TheLinear1.AppendPoint(HGF2DPosition(20000.0 , -10000.00000));
    TheLinear1.AppendPoint(HGF2DPosition(20000.0 , 0.0000000000));
    TheLinear1.AppendPoint(HGF2DPosition(15000.0 , 0.0000000000));
    TheLinear1.AppendPoint(HGF2DPosition(15000.0 , -10000.00000));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000 , -10000.00000));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000 , 18000.000000));
    TheLinear1.AppendPoint(HGF2DPosition(12000.0 , 18000.000000));
    TheLinear1.AppendPoint(HGF2DPosition(12000.0 , 0.0000000500));
    TheLinear1.AppendPoint(HGF2DPosition(18000.0 , 0.0000000500));
    TheLinear1.AppendPoint(HGF2DPosition(18000.0 , 18000.000000));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0 , 18000.000000));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0 , -10000.00000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HFCPtr<HGF2DShape> pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 10.0, 10.0)));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());    

    pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 100.0, 100.0)));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());    

    }

//==================================================================================
// Additional AllocateCopyInCoordSys test
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  AllocateCopyInCoordSysTestWhoFailed7)
    {
   
    
    HGF2DPolySegment  TheLinear1;
    TheLinear1.AppendPoint(HGF2DPosition(25000.0000 , -10000.0));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000000 , -10000.0));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000000 , 18000.00));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0000 , 18000.00));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0000 , 12000.00));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000005 , 12000.00));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000005 , 11000.00));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0000 , 11000.00));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0000 , 0.000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000005 , 0.000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000005 , -5000.00));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0000 , -5000.00));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0000 , -10000.0));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HFCPtr<HGF2DShape> pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 10.0, 10.0)));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(3, pResult->GetShapeList().size());
    
    pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 100.0, 100.0)));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(3, pResult->GetShapeList().size());
    
    }

//==================================================================================
// Test which failed on August 22, 2000
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester, ModifyShapeWithPointerWhoFailed44)
    {
          
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(-16000.000000000007, -21754.775510203093));
    TheLinear1.AppendPoint(HGF2DPosition(-16000.000000000007, 12000.0000000061850));
    TheLinear1.AppendPoint(HGF2DPosition(34245.2244897959170, 12000.0000000122160));
    TheLinear1.AppendPoint(HGF2DPosition(34245.2244897959170, -21754.775510197065));
    TheLinear1.AppendPoint(HGF2DPosition(-16000.000000000007, -21754.775510203093));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(0.00000, 0.00000));
    TheLinear2.AppendPoint(HGF2DPosition(0.00000, 12000.0));
    TheLinear2.AppendPoint(HGF2DPosition(16000.0, 12000.0));
    TheLinear2.AppendPoint(HGF2DPosition(16000.0, 0.00000));
    TheLinear2.AppendPoint(HGF2DPosition(0.00000, 0.00000));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);
    
    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  AppendPointTestWhoFailed)
    {
   
    HGF2DPolySegment PolySegment;

    HGF2DPosition Pt1(496871.025, 3519368.775);
    PolySegment.AppendPoint(Pt1);

    HGF2DPosition Pt2(496871.025, 3520443.975);
    PolySegment.AppendPoint(Pt2);

    HGF2DPosition Pt3(498791.025, 3520443.975);
    PolySegment.AppendPoint(Pt3);

    HGF2DPosition Pt4(498791.025, 3519368.775);
    PolySegment.AppendPoint(Pt4);

    HGF2DPosition Pt5(496871.025, 3519368.775);
    PolySegment.AppendPoint(Pt5);

    HGF2DPolygonOfSegments OriginalPolygon(PolySegment);

    HGF2DVector::ArbitraryDirection Dir = HGF2DVector::ALPHA;

    double Offset(5.0);

    HFCPtr<HGF2DPolygonOfSegments> NewPolygon = OriginalPolygon.AllocateParallelCopy(Offset, Dir);

    }

//==================================================================================
// Test which failed on Sept 6, 2000 (BUG #4737)
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester, ModifyShapeWithPointerWhoFailed45 )
    {
   
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(1131.370849898475900 , 0.000000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.000000000000000000 , 1131.370849898476100));
    TheLinear1.AppendPoint(HGF2DPosition(331.3708498984757400 , 1462.741699796951700));
    TheLinear1.AppendPoint(HGF2DPosition(331.3708498984757400 , 800.0000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(1931.370849898475600 , 800.0000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(1931.370849898475600 , 1268.629150101523500));
    TheLinear1.AppendPoint(HGF2DPosition(2028.427124746189700 , 1365.685424949237600));
    TheLinear1.AppendPoint(HGF2DPosition(2262.000000000000000 , 1132.112549695427300));
    TheLinear1.AppendPoint(HGF2DPosition(2262.000000000000000 , 1130.629150101524100));
    TheLinear1.AppendPoint(HGF2DPosition(1131.370849898475900 , 0.000000000000000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(363.370849898475970 , 768.0000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(223.999999999999890 , 907.3708498984760800));
    TheLinear2.AppendPoint(HGF2DPosition(223.999999999999890 , 1024.000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(479.999999999999890 , 1024.000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(479.999999999999890 , 768.0000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(363.370849898475970 , 768.0000000000000000));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);
    
    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed46)
    {
        
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(1697.056274847713700 , 0.000000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.000000000000000000 , 1697.056274847714300));
    TheLinear1.AppendPoint(HGF2DPosition(497.0562748477141200 , 2194.112549695428200));
    TheLinear1.AppendPoint(HGF2DPosition(497.0562748477141200 , 1199.999999999999800));
    TheLinear1.AppendPoint(HGF2DPosition(2097.056274847714100 , 1199.999999999999800));
    TheLinear1.AppendPoint(HGF2DPosition(2097.056274847714100 , 1297.056274847713700));
    TheLinear1.AppendPoint(HGF2DPosition(2194.112549695428200 , 1199.999999999999500));
    TheLinear1.AppendPoint(HGF2DPosition(3042.640687119285800 , 2048.528137423857100));
    TheLinear1.AppendPoint(HGF2DPosition(3394.000000000000000 , 1697.168824543141900));
    TheLinear1.AppendPoint(HGF2DPosition(3394.000000000000000 , 1696.943725152285400));
    TheLinear1.AppendPoint(HGF2DPosition(1697.056274847713700 , 0.000000000000000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(624.000000000000450 , 1073.056274847713700));
    TheLinear2.AppendPoint(HGF2DPosition(417.056274847714350 , 1280.000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(624.000000000000450 , 1280.000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(624.000000000000450 , 1073.056274847713700));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);
    
    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester, ModifyShapeWithPointerWhoFailed47 )
    {
           
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(76.00, 0.000));
    TheLinear1.AppendPoint(HGF2DPosition(76.00, 5.000));
    TheLinear1.AppendPoint(HGF2DPosition(70.00, 5.000));
    TheLinear1.AppendPoint(HGF2DPosition(70.00, 0.000));
    TheLinear1.AppendPoint(HGF2DPosition(0.000, 0.000));
    TheLinear1.AppendPoint(HGF2DPosition(0.000, 256.0));
    TheLinear1.AppendPoint(HGF2DPosition(256.0, 256.0));
    TheLinear1.AppendPoint(HGF2DPosition(256.0, 0.000));
    TheLinear1.AppendPoint(HGF2DPosition(76.00, 0.000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HFCPtr<HGF2DShape> pShape2 = new HGF2DRectangle(0.0, 0.0, 256.0, 256.0);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
    HGF2DPolygonOfSegments MySameRect(HGF2DRectangle(0.0, 0.0, 256.0, 256.0));

    ASSERT_EQ(HGF2DShape::S_IN, MySameRect.CalculateSpatialPositionOf(*pShape1));
    ASSERT_EQ(HGF2DShape::S_OUT, pShape1->CalculateSpatialPositionOf(MySameRect));
    ASSERT_EQ(HGF2DShape::S_IN, pShape2->CalculateSpatialPositionOf(*pShape1));
    ASSERT_EQ(HGF2DShape::S_OUT, pShape1->CalculateSpatialPositionOf(*pShape2));
   
    }

//==================================================================================
// Test which failed on Sept 11, 2000
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester, ModifyShapeWithPointerWhoFailed48)
    {
            
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(0.000, 0.000));
    TheLinear1.AppendPoint(HGF2DPosition(0.000, 256.0));
    TheLinear1.AppendPoint(HGF2DPosition(256.0, 256.0));
    TheLinear1.AppendPoint(HGF2DPosition(256.0, 0.000));
    TheLinear1.AppendPoint(HGF2DPosition(0.000, 0.000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(1.9746125868655, 0.29621359212831000));
    TheLinear2.AppendPoint(HGF2DPosition(2.3695301674602, 0.29621359212763000));
    TheLinear2.AppendPoint(HGF2DPosition(2.3695301674600, 6.8212102632970E-13));
    TheLinear2.AppendPoint(HGF2DPosition(1.9746125868653, 1.5916157281026E-12));
    TheLinear2.AppendPoint(HGF2DPosition(1.9745879029738, 7.9580786405131E-13));
    TheLinear2.AppendPoint(HGF2DPosition(1.5796703223790,-1.1368683772162E-13));
    TheLinear2.AppendPoint(HGF2DPosition(1.1847527417843,7.47832018532790E-10));
    TheLinear2.AppendPoint(HGF2DPosition(1.1847527417843, 0.29618890691438000));
    TheLinear2.AppendPoint(HGF2DPosition(1.5796703223791, 0.29618890691438000));
    TheLinear2.AppendPoint(HGF2DPosition(1.5796703223792, 0.29621359212672000));
    TheLinear2.AppendPoint(HGF2DPosition(1.9745879029739, 0.29621359212763000));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester, ModifyShapeWithPointerWhoFailed49 )
    {
           
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(224.0, 800.00));
    TheLinear1.AppendPoint(HGF2DPosition(480.0, 1056.0));
    TheLinear1.AppendPoint(HGF2DPosition(736.0, 800.00));
    TheLinear1.AppendPoint(HGF2DPosition(480.0, 544.00));
    TheLinear1.AppendPoint(HGF2DPosition(224.0, 800.00));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);
    HFCPtr<HGF2DShape> pShape2 = new HGF2DRectangle(0.0, 512.0, 256.0, 768.0);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
    ASSERT_EQ(HGF2DShape::S_OUT, pShape2->CalculateSpatialPositionOf(*pShape1));
    ASSERT_EQ(HGF2DShape::S_OUT, pShape1->CalculateSpatialPositionOf(*pShape2));
    
    }

//==================================================================================
// Test which failed on Nov 9, 2000
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester, ModifyShapeWithPointerWhoFailed50 )
    {
        
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(1030.3762349323, 255.99999998882));
    TheLinear1.AppendPoint(HGF2DPosition(778.69140000000, 255.99999999255));
    TheLinear1.AppendPoint(HGF2DPosition(778.31236900000, 277.71485400000));
    TheLinear1.AppendPoint(HGF2DPosition(1029.9205400000, 282.10669100000));
    TheLinear1.AppendPoint(HGF2DPosition(1030.3762349323, 255.99999998882));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);
    HFCPtr<HGF2DShape> pShape2 = new HGF2DRectangle(0.0, 0.0, 4251.0, 256.0);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester, ModifyShapeWithPointerWhoFailed51)
    {
          
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(0.0008822139352560 , 0.01213040202856100));
    TheLinear1.AppendPoint(HGF2DPosition(0.0007792119868100 , 253.640242036432030));
    TheLinear1.AppendPoint(HGF2DPosition(233.40057656611316 , 240.064462127164010));
    TheLinear1.AppendPoint(HGF2DPosition(219.43766127480194 , 0.01214403659105300));
    TheLinear1.AppendPoint(HGF2DPosition(0.0008822139352560 , 0.01213040202856100));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    pShape1->SetAutoToleranceActive(false);
    pShape1->SetTolerance(0.001);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(0.000812306855621 , 0.0000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(0.000892448239028 , 50.512210074812174));
    TheLinear2.AppendPoint(HGF2DPosition(0.000582597115946 , 256.00000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(256.0000000000000 , 256.00000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(256.0000000000000 , 0.0000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(0.000812306855621 , 0.0000000000000000));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester, AutoCrossesTestWhoFailed)
    {

    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(17.9195179616799580 , 0.012126367539167));
    TheLinear1.AppendPoint(HGF2DPosition(-0.0000012393575160 , 0.012124864384532));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000170688144900 , 0.012191701680422));
    TheLinear1.AppendPoint(HGF2DPosition(189.655513013247400 , 0.012132421135902));
    TheLinear1.AppendPoint(HGF2DPosition(189.655512792989610 , 0.012128636240959));
    TheLinear1.AppendPoint(HGF2DPosition(168.188499589683490 , 0.012121440842748));
    TheLinear1.AppendPoint(HGF2DPosition(157.454992987914010 , 0.012117844074965));
    TheLinear1.AppendPoint(HGF2DPosition(152.088239687145690 , 0.012116044759750));
    TheLinear1.AppendPoint(HGF2DPosition(150.746551361866300 , 0.012115595862269));
    TheLinear1.AppendPoint(HGF2DPosition(150.075707199284810 , 0.012115368619561));
    TheLinear1.AppendPoint(HGF2DPosition(149.740285117877650 , 0.012115256860852));
    TheLinear1.AppendPoint(HGF2DPosition(149.572574077406900 , 0.012115200981498));
    TheLinear1.AppendPoint(HGF2DPosition(149.530646317056380 , 0.012115186080337));
    TheLinear1.AppendPoint(HGF2DPosition(149.509682436939330 , 0.012115180492401));
    TheLinear1.AppendPoint(HGF2DPosition(149.499200497055430 , 0.012115176767111));
    TheLinear1.AppendPoint(HGF2DPosition(149.496580011909830 , 0.012115174904466));
    TheLinear1.AppendPoint(HGF2DPosition(149.495924890739840 , 0.012115174904466));
    TheLinear1.AppendPoint(HGF2DPosition(149.495761110447350 , 0.012115174904466));
    TheLinear1.AppendPoint(HGF2DPosition(149.495720165199600 , 0.012115174904466));
    TheLinear1.AppendPoint(HGF2DPosition(149.495699692750350 , 0.012115173041821));
    TheLinear1.AppendPoint(HGF2DPosition(149.495681825559590 , 0.012169376015663));
    TheLinear1.AppendPoint(HGF2DPosition(149.495599935296920 , 0.012169374153018));
    TheLinear1.AppendPoint(HGF2DPosition(149.495272374944760 , 0.012169376015663));
    TheLinear1.AppendPoint(HGF2DPosition(149.493962133536120 , 0.012169374153018));
    TheLinear1.AppendPoint(HGF2DPosition(149.488721167901530 , 0.012169374153018));
    TheLinear1.AppendPoint(HGF2DPosition(149.404865716584030 , 0.012169348075986));
    TheLinear1.AppendPoint(HGF2DPosition(146.721491272561250 , 0.012168470770121));
    TheLinear1.AppendPoint(HGF2DPosition(103.787500168895350 , 0.012154435738921));
    TheLinear1.AppendPoint(HGF2DPosition(17.9195179616799580 , 0.012126367539167));

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
TEST_F(HGF2DPolygonOfSegmentsTester, ModifyShapeWithPointerWhoFailed52)
    {
            
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(0.000000000000000 , 0.000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.000000000000000 , 256.0000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(256.0000000000000 , 256.0000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(256.0000000000000 , 0.000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.000000000000000 , 0.000000000000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(-0.00000000011641500 , -255.999924341682340));
    TheLinear2.AppendPoint(HGF2DPosition(0.000044487445848000 , 874.6514387526549400));
    TheLinear2.AppendPoint(HGF2DPosition(1507.537023507495200 , 874.6513630896806700));
    TheLinear2.AppendPoint(HGF2DPosition(1507.536979380151000 , -256.000000000465660));
    TheLinear2.AppendPoint(HGF2DPosition(-0.00000000011641500 , -255.999924341682340));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    pShape2->SetAutoToleranceActive(false);
    pShape2->SetTolerance(0.001);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester, IsAutoContiguousTestWhoFailed)
    {
  
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(-146612.616979246520000 , -3733745.133152585500000));
    TheLinear1.AppendPoint(HGF2DPosition(-146610.838938417060000 , -3733745.029725480400000));
    TheLinear1.AppendPoint(HGF2DPosition(-146612.480445448800000 , -3733716.810040059500000));
    TheLinear1.AppendPoint(HGF2DPosition(-146614.121941842870000 , -3733688.590353258400000));
    TheLinear1.AppendPoint(HGF2DPosition(-146614.760026228150000 , -3733677.620692211700000));
    TheLinear1.AppendPoint(HGF2DPosition(-146614.760025639170000 , -3733677.620692211700000));
    TheLinear1.AppendPoint(HGF2DPosition(-146612.878627912110000 , -3733709.964706689100000));
    TheLinear1.AppendPoint(HGF2DPosition(-146610.865070915780000 , -3733744.580448843500000));
    TheLinear1.AppendPoint(HGF2DPosition(-146610.838938297970000 , -3733745.029725568800000));
    TheLinear1.AppendPoint(HGF2DPosition(-146612.616979246520000 , -3733745.133152585500000));

    HGF2DPolygonOfSegments MyPolygon(TheLinear1);

    HGF2DPolySegment  AddPolySegment1;

    AddPolySegment1.AppendPoint(HGF2DPosition(74.796603888942343 , 51.514879816683035));
    AddPolySegment1.AppendPoint(HGF2DPosition(74.796616508015234 , 103.020908160924170));
    AddPolySegment1.AppendPoint(HGF2DPosition(74.796616688978688 , 123.042521701129810));
    AddPolySegment1.AppendPoint(HGF2DPosition(74.796617760372797 , 123.042521638825620));
    AddPolySegment1.AppendPoint(HGF2DPosition(74.796607313434947 , 64.008850551921000));
    AddPolySegment1.AppendPoint(HGF2DPosition(74.796613226371505 , 0.828861127342226));
    AddPolySegment1.AppendPoint(HGF2DPosition(74.796603888942343 , 51.514879816683035));

    AddPolySegment1.SetAutoToleranceActive(false);
    AddPolySegment1.SetTolerance(0.00001);

    ASSERT_TRUE(AddPolySegment1.IsAutoContiguous());

    AddPolySegment1.RemoveAutoContiguousNeedles(true);
    ASSERT_FALSE(AddPolySegment1.IsAutoContiguous());

    }

//==================================================================================
// Test which failed on Nov 16, 2000
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester, AllocateComplexShapeFromAutoContiguousPolySegmentTestWhoFailed)
    {
    
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(-146612.616979246520000 , -3733745.133152585500000));
    TheLinear1.AppendPoint(HGF2DPosition(-146610.838938417060000 , -3733745.029725480400000));
    TheLinear1.AppendPoint(HGF2DPosition(-146612.480445448800000 , -3733716.810040059500000));
    TheLinear1.AppendPoint(HGF2DPosition(-146614.121941842870000 , -3733688.590353258400000));
    TheLinear1.AppendPoint(HGF2DPosition(-146614.760026228150000 , -3733677.620692211700000));
    TheLinear1.AppendPoint(HGF2DPosition(-146614.760025639170000 , -3733677.620692211700000));
    TheLinear1.AppendPoint(HGF2DPosition(-146612.878627912110000 , -3733709.964706689100000));
    TheLinear1.AppendPoint(HGF2DPosition(-146610.865070915780000 , -3733744.580448843500000));
    TheLinear1.AppendPoint(HGF2DPosition(-146610.838938297970000 , -3733745.029725568800000));
    TheLinear1.AppendPoint(HGF2DPosition(-146612.616979246520000 , -3733745.133152585500000));

    HGF2DPolygonOfSegments MyPolygon(TheLinear1);

    HGF2DPolySegment  AddPolySegment1;

    AddPolySegment1.AppendPoint(HGF2DPosition(37.398305383578645 , 0.00442506910800300));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398301944801993 , 25.7574399086932540));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398308254143032 , 51.5104540812430190));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398305256362420 , 77.2634688865724400));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398309950957461 , 103.016483169801460));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398308470115047 , 128.769497876260170));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398312018577869 , 154.522512236347180));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398312263270519 , 154.996322700473460));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398324268186833 , 154.996322002121960));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398317068121948 , 128.004424629423450));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398313771654699 , 64.0044246748469730));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398312104805100 , 2.41303710120554900));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398305487265958 , 0.00442486824301700));
    AddPolySegment1.AppendPoint(HGF2DPosition(36.551915531009243 , 0.00442465802248200));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398305383578645 , 0.00442506910800300));

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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed53)
    {
   
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(0.000000000000000 , 51.46683371067047 ));
    TheLinear1.AppendPoint(HGF2DPosition(14.14608025550842 , 42.66670471429825 ));
    TheLinear1.AppendPoint(HGF2DPosition(0.000000000000000 , 34.86657571792603 ));
    TheLinear1.AppendPoint(HGF2DPosition(0.000000000000000 , 51.46683371067047 ));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(51.65018710229799 , 64.00000000000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(0.000000000000000 , 34.86657594899594 ));
    TheLinear2.AppendPoint(HGF2DPosition(0.000000000000000 , 64.00000000000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(51.65018710229799 , 64.00000000000000 ));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed54)
    {
          
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(59.05016660690308 , 11.33375036716461 ));
    TheLinear1.AppendPoint(HGF2DPosition(64.00000000000000 , 8.804601848125460 ));
    TheLinear1.AppendPoint(HGF2DPosition(64.00000000000000 , 0.000000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(39.75035202503204 , 0.000000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(59.05016660690308 , 11.33375036716461 ));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(39.775035227748781 , 0.0000000000000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(64.000000000000000 , 13.986289931850713 ));
    TheLinear2.AppendPoint(HGF2DPosition(64.000000000000000 , 0.0000000000000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(39.775035227748781 , 0.0000000000000000 ));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed55)
    {
          
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(0.000000000000000 , 41.72118563598019 ));
    TheLinear1.AppendPoint(HGF2DPosition(64.00000000000000 , 4.217013355800860 ));
    TheLinear1.AppendPoint(HGF2DPosition(64.00000000000000 , 0.000000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(0.000000000000000 , 0.000000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(0.000000000000000 , 41.72118563598019 ));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(12.33750259876251 , 34.66694611310959 ));
    TheLinear2.AppendPoint(HGF2DPosition(0.000000000000000 , 27.61270588636398 ));
    TheLinear2.AppendPoint(HGF2DPosition(0.000000000000000 , 41.72118544578552 ));
    TheLinear2.AppendPoint(HGF2DPosition(12.33750259876251 , 34.66694611310959 ));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed56)
    {
            
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(256.000000000000000, 51.0000000261400 ));
    TheLinear1.AppendPoint(HGF2DPosition(256.000000000000000, 64.0000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(320.000000000000000, 64.0000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(320.000000000000000, 0.00000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(256.000000130700000, 0.00000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(256.000000130700000, 51.0000000261400 ));
    TheLinear1.AppendPoint(HGF2DPosition(256.000000000000000, 51.0000000261400 ));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(256.0000001307, 0.000000000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(256.0000001307, 51.00000002614 ));
    TheLinear2.AppendPoint(HGF2DPosition(307.0000001307, 51.00000002614 ));
    TheLinear2.AppendPoint(HGF2DPosition(307.0000001307, 0.000000000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(256.0000001307, 0.000000000000 ));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed57)
    {
           
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(0.000000000000000 , 41.72118563598019 ));
    TheLinear1.AppendPoint(HGF2DPosition(64.00000000000000 , 4.217013355800860 ));
    TheLinear1.AppendPoint(HGF2DPosition(64.00000000000000 , 0.000000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(0.000000000000000 , 0.000000000000000 ));
    TheLinear1.AppendPoint(HGF2DPosition(0.000000000000000 , 41.72118563598019 ));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(12.33750259876251 , 34.66694611310959 ));
    TheLinear2.AppendPoint(HGF2DPosition(0.000000000000000 , 27.61270588636398 ));
    TheLinear2.AppendPoint(HGF2DPosition(0.000000000000000 , 41.72118544578552 ));
    TheLinear2.AppendPoint(HGF2DPosition(12.33750259876251 , 34.66694611310959 ));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed58)
    {
          
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(-838960.75561190260000 , 1443576.03989492600000 ));
    TheLinear1.AppendPoint(HGF2DPosition(-413480.05918325430000 , -666897.41530794650000 ));
    TheLinear1.AppendPoint(HGF2DPosition(1706857.07802271400000 , -239428.13782825880000 ));
    TheLinear1.AppendPoint(HGF2DPosition(1281377.38159406400000 , 1871045.31737461400000 ));
    TheLinear1.AppendPoint(HGF2DPosition(-838960.75561190260000 , 1443576.03989492600000 ));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(-838960.75561276990000 , 1443576.03989754800000 ));
    TheLinear2.AppendPoint(HGF2DPosition(-413480.05917771700000 , -666897.41530503150000 ));
    TheLinear2.AppendPoint(HGF2DPosition(1706857.07802418800000 , -239428.13782369070000 ));
    TheLinear2.AppendPoint(HGF2DPosition(1281377.38160031600000 , 1871045.31737896800000 ));
    TheLinear2.AppendPoint(HGF2DPosition(-838960.75561276990000 , 1443576.03989754800000 ));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed59)
    {
          
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(-32000.000000000290 , 0.000000149011610000));
    TheLinear1.AppendPoint(HGF2DPosition(-32000.000000000290 , 12000.00000015267000));
    TheLinear1.AppendPoint(HGF2DPosition(-16000.000000000290 , 12000.00000008227000));
    TheLinear1.AppendPoint(HGF2DPosition(9876.00000000000000 , 12000.00000011541000));
    TheLinear1.AppendPoint(HGF2DPosition(9876.00000000000000 , 0.000000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(-32000.000000000290 , 0.000000149011610000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(-0.00000000029104 , 12000.00000003360000));
    TheLinear2.AppendPoint(HGF2DPosition(-0.00000000029104 , 24000.00000006719000));
    TheLinear2.AppendPoint(HGF2DPosition(8667.999999999709 , 24000.00000002212000));
    TheLinear2.AppendPoint(HGF2DPosition(8667.999999999709 , 11999.99999998852000));
    TheLinear2.AppendPoint(HGF2DPosition(-0.00000000029104 , 12000.00000003360000));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);
    
    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester, AllocateCopyInCoordSysTestWhoFailed8)
    {


    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(24998.375000170640 , 0.000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(24998.375000170640 , 0.100594766647000));
    TheLinear1.AppendPoint(HGF2DPosition(0.0000000000000000 , 0.100595476536000));
    TheLinear1.AppendPoint(HGF2DPosition(0.0000000000000000 , 18750.00000000000));
    TheLinear1.AppendPoint(HGF2DPosition(25000.000000000000 , 18750.00000000000));
    TheLinear1.AppendPoint(HGF2DPosition(25000.000000000000 , 18749.99993365014));
    TheLinear1.AppendPoint(HGF2DPosition(24998.374999393650 , 18749.99993365020));
    TheLinear1.AppendPoint(HGF2DPosition(24998.375000608570 , 0.100067755764000));
    TheLinear1.AppendPoint(HGF2DPosition(25000.000000000000 , 0.100067755688000));
    TheLinear1.AppendPoint(HGF2DPosition(25000.000000000000 , 0.000000000000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);
    pShape1->SetAutoToleranceActive(false);
    pShape1->SetTolerance(1E-8);

    HFCPtr<HGF2DShape> pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 100.0, 100.0)));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
    
    pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 1000.0, 1000.0)));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
       
    }

//==================================================================================
// Additional AllocateCopyInCoordSys tests
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester, AllocateCopyInCoordSysTestWhoFailed9)
    {
           

    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(25000.0 , 0.0000000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000 , 0.0000000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000 , 18000.000000));
    TheLinear1.AppendPoint(HGF2DPosition(12000.0 , 18000.000000));
    TheLinear1.AppendPoint(HGF2DPosition(12000.0 , 50*MYEPSILON));
    TheLinear1.AppendPoint(HGF2DPosition(18000.0 , 50*MYEPSILON));
    TheLinear1.AppendPoint(HGF2DPosition(18000.0 , 18000.000000));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0 , 18000.000000));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0 , 0.0000000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HFCPtr<HGF2DShape> pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 100.0, 100.0)));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
    
    pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(HGF2DStretch(HGF2DDisplacement(0.0, 0.0),  1000.0, 1000.0)));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
        
    }

//==================================================================================
// Additional AllocateCopyInCoordSys tests
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester, AllocateCopyInCoordSysTestWhoFailed10)
    {
          

    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(25000.0 , 0.0000000000));
    TheLinear1.AppendPoint(HGF2DPosition(12000.0 , 0.0000000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000 , 0.0000000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000 , 18000.000000));
    TheLinear1.AppendPoint(HGF2DPosition(12000.0 , 18000.000000));
    TheLinear1.AppendPoint(HGF2DPosition(12000.0 , 50*MYEPSILON));
    TheLinear1.AppendPoint(HGF2DPosition(18000.0 , 50*MYEPSILON));
    TheLinear1.AppendPoint(HGF2DPosition(18000.0 , 18000.000000));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0 , 18000.000000));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0 , 0.0000000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HFCPtr<HGF2DShape> pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 100.0, 100.0)));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
    
    pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 1000.0, 1000.0)));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());  
    
    }

//==================================================================================
// Additional AllocateCopyInCoordSys tests
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester, AllocateCopyInCoordSysTestWhoFailed11)
    {
            
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(25000.0 , -10000.00000));
    TheLinear1.AppendPoint(HGF2DPosition(15000.0 , -10000.00000));
    TheLinear1.AppendPoint(HGF2DPosition(15000.0 , 0.0000000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000 , 0.0000000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000 , 18000.000000));
    TheLinear1.AppendPoint(HGF2DPosition(12000.0 , 18000.000000));
    TheLinear1.AppendPoint(HGF2DPosition(12000.0 , 50*MYEPSILON));
    TheLinear1.AppendPoint(HGF2DPosition(18000.0 , 50*MYEPSILON));
    TheLinear1.AppendPoint(HGF2DPosition(18000.0 , 18000.000000));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0 , 18000.000000));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0 , -10000.00000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HFCPtr<HGF2DShape> pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 100.0, 100.0)));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
    
    pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 1000.0, 1000.0)));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
        
    }

//==================================================================================
// Additional AllocateCopyInCoordSys tests
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester, AllocateCopyInCoordSysTestWhoFailed12)
    {
   
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(25000.0 , -10000.00000));
    TheLinear1.AppendPoint(HGF2DPosition(20000.0 , -10000.00000));
    TheLinear1.AppendPoint(HGF2DPosition(20000.0 , 0.0000000000));
    TheLinear1.AppendPoint(HGF2DPosition(15000.0 , 0.0000000000));
    TheLinear1.AppendPoint(HGF2DPosition(15000.0 , -10000.00000));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000 , -10000.00000));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000 , 18000.000000));
    TheLinear1.AppendPoint(HGF2DPosition(12000.0 , 18000.000000));
    TheLinear1.AppendPoint(HGF2DPosition(12000.0 , 50*MYEPSILON));
    TheLinear1.AppendPoint(HGF2DPosition(18000.0 , 50*MYEPSILON));
    TheLinear1.AppendPoint(HGF2DPosition(18000.0 , 18000.000000));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0 , 18000.000000));
    TheLinear1.AppendPoint(HGF2DPosition(25000.0 , -10000.00000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HFCPtr<HGF2DShape> pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 100.0, 100.0)));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
    
    pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 1000.0, 1000.0)));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(2, pResult->GetShapeList().size());
     
    }

//==================================================================================
// Additional AllocateCopyInCoordSys tests
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester, AllocateCopyInCoordSysTestWhoFailed13)
    {
         
   
    HGF2DPolySegment  TheLinear1;
    TheLinear1.AppendPoint(HGF2DPosition(25000.000000 , -10000.0));
    TheLinear1.AppendPoint(HGF2DPosition(0.0000000000 , -10000.0));
    TheLinear1.AppendPoint(HGF2DPosition(0.0000000000 , 18000.00));
    TheLinear1.AppendPoint(HGF2DPosition(25000.000000 , 18000.00));
    TheLinear1.AppendPoint(HGF2DPosition(25000.000000 , 12000.00));
    TheLinear1.AppendPoint(HGF2DPosition(50*MYEPSILON , 12000.00));
    TheLinear1.AppendPoint(HGF2DPosition(50*MYEPSILON , 11000.00));
    TheLinear1.AppendPoint(HGF2DPosition(25000.000000 , 11000.00));
    TheLinear1.AppendPoint(HGF2DPosition(25000.000000 , 0.000000));
    TheLinear1.AppendPoint(HGF2DPosition(50*MYEPSILON , 0.000000));
    TheLinear1.AppendPoint(HGF2DPosition(50*MYEPSILON , -5000.00));
    TheLinear1.AppendPoint(HGF2DPosition(25000.000000 , -5000.00));
    TheLinear1.AppendPoint(HGF2DPosition(25000.000000 , -10000.0));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HFCPtr<HGF2DShape> pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 100.0, 100.0)));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(3, pResult->GetShapeList().size());
    
    pResult = static_cast<HGF2DShape*>(pShape1->AllocTransformDirect(HGF2DStretch(HGF2DDisplacement(0.0, 0.0), 1000.0, 1000.0)));
    ASSERT_TRUE(pResult->IsComplex());
    ASSERT_EQ(3, pResult->GetShapeList().size());
       
    }

//==================================================================================
// Test which failed on August 22, 2000
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed60)
    {
          
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(-16000.00000000007, -21754.75510203093));
    TheLinear1.AppendPoint(HGF2DPosition(-16000.00000000007, 12000.000000061850));
    TheLinear1.AppendPoint(HGF2DPosition(34245.244897959170, 12000.000000122160));
    TheLinear1.AppendPoint(HGF2DPosition(34245.244897959170, -21754.75510197065));
    TheLinear1.AppendPoint(HGF2DPosition(-16000.00000000007, -21754.75510203093));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(0.00000, 0.00000));
    TheLinear2.AppendPoint(HGF2DPosition(0.00000, 12000.0));
    TheLinear2.AppendPoint(HGF2DPosition(16000.0, 12000.0));
    TheLinear2.AppendPoint(HGF2DPosition(16000.0, 0.00000));
    TheLinear2.AppendPoint(HGF2DPosition(0.00000, 0.00000));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester, AllocateParallelCopyTestWhoFailed)
    {

    HGF2DPolySegment PolySegment;

    HGF2DPosition Pt1(496871.025, 3519368.775);
    PolySegment.AppendPoint(Pt1);

    HGF2DPosition Pt2(496871.025, 3520443.975);
    PolySegment.AppendPoint(Pt2);

    HGF2DPosition Pt3(498791.025, 3520443.975);
    PolySegment.AppendPoint(Pt3);

    HGF2DPosition Pt4(498791.025, 3519368.775);
    PolySegment.AppendPoint(Pt4);

    HGF2DPosition Pt5(496871.025, 3519368.775);
    PolySegment.AppendPoint(Pt5);

    HGF2DPolygonOfSegments OriginalPolygon(PolySegment);

    HGF2DVector::ArbitraryDirection Dir = HGF2DVector::ALPHA;

    double Offset(5.0);

    HFCPtr<HGF2DPolygonOfSegments> NewPolygon = OriginalPolygon.AllocateParallelCopy(Offset, Dir);

    }

//==================================================================================
// Test which failed on Sept 6, 2000 (BUG #4737)
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed61)
    {
           
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(1131.70849898475900 , 0.00000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000000000000000 , 1131.70849898476100));
    TheLinear1.AppendPoint(HGF2DPosition(331.708498984757400 , 1462.41699796951700));
    TheLinear1.AppendPoint(HGF2DPosition(331.708498984757400 , 800.000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(1931.70849898475600 , 800.000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(1931.70849898475600 , 1268.29150101523500));
    TheLinear1.AppendPoint(HGF2DPosition(2028.27124746189700 , 1365.85424949237600));
    TheLinear1.AppendPoint(HGF2DPosition(2262.00000000000000 , 1132.12549695427300));
    TheLinear1.AppendPoint(HGF2DPosition(2262.00000000000000 , 1130.29150101524100));
    TheLinear1.AppendPoint(HGF2DPosition(1131.70849898475900 , 0.00000000000000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(363.70849898475970 , 768.00000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(223.99999999999890 , 907.70849898476080));
    TheLinear2.AppendPoint(HGF2DPosition(223.99999999999890 ,1024.00000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(479.99999999999890 ,1024.00000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(479.99999999999890 , 768.00000000000000));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);
    
    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed62)
    {
          
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(1697.56274847713700 , 0.00000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000000000000000 , 1697.56274847714300));
    TheLinear1.AppendPoint(HGF2DPosition(497.562748477141200 , 2194.12549695428200));
    TheLinear1.AppendPoint(HGF2DPosition(497.562748477141200 , 1199.99999999999800));
    TheLinear1.AppendPoint(HGF2DPosition(2097.56274847714100 , 1199.99999999999800));
    TheLinear1.AppendPoint(HGF2DPosition(2097.56274847714100 , 1297.56274847713700));
    TheLinear1.AppendPoint(HGF2DPosition(2194.12549695428200 , 1199.99999999999500));
    TheLinear1.AppendPoint(HGF2DPosition(3042.40687119285800 , 2048.28137423857100));
    TheLinear1.AppendPoint(HGF2DPosition(3394.00000000000000 , 1697.68824543141900));
    TheLinear1.AppendPoint(HGF2DPosition(3394.00000000000000 , 1696.43725152285400));
    TheLinear1.AppendPoint(HGF2DPosition(1697.56274847713700 , 0.00000000000000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(624.00000000000450 , 1073.56274847713700));
    TheLinear2.AppendPoint(HGF2DPosition(417.56274847714350 , 1280.00000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(624.00000000000450 , 1280.00000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(624.00000000000450 , 1073.56274847713700));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed63)
    {
           
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(76.00, 0.000));
    TheLinear1.AppendPoint(HGF2DPosition(76.00, 5.000));
    TheLinear1.AppendPoint(HGF2DPosition(70.00, 5.000));
    TheLinear1.AppendPoint(HGF2DPosition(70.00, 0.000));
    TheLinear1.AppendPoint(HGF2DPosition(0.000, 0.000));
    TheLinear1.AppendPoint(HGF2DPosition(0.000, 256.0));
    TheLinear1.AppendPoint(HGF2DPosition(256.0, 256.0));
    TheLinear1.AppendPoint(HGF2DPosition(256.0, 0.000));
    TheLinear1.AppendPoint(HGF2DPosition(76.00, 0.000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);
    HFCPtr<HGF2DShape> pShape2 = new HGF2DRectangle(0.0, 0.0, 256.0, 256.0);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
    HGF2DPolygonOfSegments MySameRect(HGF2DRectangle(0.0, 0.0, 256.0, 256.0));

    ASSERT_EQ(HGF2DShape::S_IN, MySameRect.CalculateSpatialPositionOf(*pShape1));
    ASSERT_EQ(HGF2DShape::S_OUT, pShape1->CalculateSpatialPositionOf(MySameRect));
    ASSERT_EQ(HGF2DShape::S_IN, pShape2->CalculateSpatialPositionOf(*pShape1));
    ASSERT_EQ(HGF2DShape::S_OUT, pShape1->CalculateSpatialPositionOf(*pShape2));
    
    }

//==================================================================================
// Test which failed on Sept 19, 2000
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed64)
    {
            
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(224.0, 800.00));
    TheLinear1.AppendPoint(HGF2DPosition(480.0, 1056.0));
    TheLinear1.AppendPoint(HGF2DPosition(736.0, 800.00));
    TheLinear1.AppendPoint(HGF2DPosition(480.0, 544.00));
    TheLinear1.AppendPoint(HGF2DPosition(224.0, 800.00));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);
    HFCPtr<HGF2DShape> pShape2 = new HGF2DRectangle(0.0, 512.0, 256.0, 768.0);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
    ASSERT_EQ(HGF2DShape::S_OUT, pShape2->CalculateSpatialPositionOf(*pShape1));
    ASSERT_EQ(HGF2DShape::S_OUT, pShape1->CalculateSpatialPositionOf(*pShape2));
    
    }

//==================================================================================
// Test which failed on Nov 9, 2000
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed65)
    {
           
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(1030.762349323, 255.9999998882));
    TheLinear1.AppendPoint(HGF2DPosition(778.9140000000, 255.9999999255));
    TheLinear1.AppendPoint(HGF2DPosition(778.1236900000, 277.1485400000));
    TheLinear1.AppendPoint(HGF2DPosition(1029.205400000, 282.0669100000));
    TheLinear1.AppendPoint(HGF2DPosition(1030.762349323, 255.9999998882));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);
    HFCPtr<HGF2DShape> pShape2 = new HGF2DRectangle(0.0, 0.0, 4251.0, 256.0);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed66)
    {
           
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(0.008822139352560 , 0.1213040202856100));
    TheLinear1.AppendPoint(HGF2DPosition(0.007792119868100 , 253.40242036432030));
    TheLinear1.AppendPoint(HGF2DPosition(233.0057656611316 , 240.64462127164010));
    TheLinear1.AppendPoint(HGF2DPosition(219.3766127480194 , 0.1214403659105300));
    TheLinear1.AppendPoint(HGF2DPosition(0.008822139352560 , 0.1213040202856100));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);
    pShape1->SetAutoToleranceActive(false);
    pShape1->SetTolerance(0.001);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(0.00812306855621 , 0.0000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(0.00892448239028 , 50.122100748121740));
    TheLinear2.AppendPoint(HGF2DPosition(0.00582597115946 , 256.00000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(256.000000000000 , 256.00000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(256.000000000000 , 0.0000000000000000));
    TheLinear2.AppendPoint(HGF2DPosition(0.00812306855621 , 0.0000000000000000));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed67)
    {
            
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(0.00000000000000000 , 0.00000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000000000000000 , 256.000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(256.000000000000000 , 256.000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(256.000000000000000 , 0.00000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.00000000000000000 , 0.00000000000000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(-0.000000001164150 , -255.99924341682340));
    TheLinear2.AppendPoint(HGF2DPosition(0.0004448744584800 , 874.514387526549400));
    TheLinear2.AppendPoint(HGF2DPosition(1507.3702350749520 , 874.513630896806700));
    TheLinear2.AppendPoint(HGF2DPosition(1507.3697938015100 , -256.00000000465660));
    TheLinear2.AppendPoint(HGF2DPosition(-0.000000001164150 , -255.99924341682340));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(TheLinear2);

    pShape2->SetAutoToleranceActive(false);
    pShape2->SetTolerance(0.001);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  IsAutoContiguousTestWhoFailed2)
    {
    
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(-146612.616979246520000 , -3733745.133152585500000));
    TheLinear1.AppendPoint(HGF2DPosition(-146610.838938417060000 , -3733745.029725480400000));
    TheLinear1.AppendPoint(HGF2DPosition(-146612.480445448800000 , -3733716.810040059500000));
    TheLinear1.AppendPoint(HGF2DPosition(-146614.121941842870000 , -3733688.590353258400000));
    TheLinear1.AppendPoint(HGF2DPosition(-146614.760026228150000 , -3733677.620692211700000));
    TheLinear1.AppendPoint(HGF2DPosition(-146614.760025639170000 , -3733677.620692211700000));
    TheLinear1.AppendPoint(HGF2DPosition(-146612.878627912110000 , -3733709.964706689100000));
    TheLinear1.AppendPoint(HGF2DPosition(-146610.865070915780000 , -3733744.580448843500000));
    TheLinear1.AppendPoint(HGF2DPosition(-146610.838938297970000 , -3733745.029725568800000));
    TheLinear1.AppendPoint(HGF2DPosition(-146612.616979246520000 , -3733745.133152585500000));

    HGF2DPolygonOfSegments MyPolygon(TheLinear1);

    HGF2DPolySegment  AddPolySegment1;

    AddPolySegment1.AppendPoint(HGF2DPosition(24.624353316877436 , 0.004425250060435));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398305383578645 , 0.004425069108003));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398301869392405 , 19.319186263555068));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398301467879172 , 22.538313116279927));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398303852609480 , 38.633947080613552));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398305174882111 , 45.072200640873156));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398309209308998 , 54.729580936149411));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398305913017346 , 64.386961540812607));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398305256362420 , 77.263468886572440));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398305146288891 , 86.920849370524138));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398305846564291 , 90.139976147999874));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398309950957461 , 103.01648316980146));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398312240216100 , 109.45473670769246));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398309812264650 , 115.89299048231651));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398308924056224 , 119.11211736887199));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398308470115047 , 128.76949787626017));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398308374657361 , 141.64600518417248));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398312152666570 , 154.99632270698271));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398324268477438 , 154.99632200212196));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398314934021926 , 120.00442475209485));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398319692574731 , 112.50442480907817));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398322057602797 , 108.75442479488775));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398321363245735 , 105.00442496174850));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398318585817435 , 90.004425340236082));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398313030960594 , 60.004424714223930));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398317191746997 , 52.799085740268119));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398316444880379 , 45.593746993757655));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398314951205229 , 31.183069183142031));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398311963796687 , 2.3617122817676420));
    AddPolySegment1.AppendPoint(HGF2DPosition(37.398305487265958 , 0.0044248682430170));
    AddPolySegment1.AppendPoint(HGF2DPosition(36.551915525778384 , 0.0044246584874480));
    AddPolySegment1.AppendPoint(HGF2DPosition(24.624353301068627 , 0.0044249780558200));
    AddPolySegment1.AppendPoint(HGF2DPosition(24.624353316877436 , 0.0044252500604350));

    AddPolySegment1.SetAutoToleranceActive(false);
    AddPolySegment1.SetTolerance(0.000001);

    ASSERT_TRUE(AddPolySegment1.IsAutoContiguous());

    AddPolySegment1.RemoveAutoContiguousNeedles(true);
    ASSERT_FALSE(AddPolySegment1.IsAutoContiguous());
       
    }

//==================================================================================
// Test which failed on Dec 8, 2000
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed68)
    {
    
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(0.0000000000000, 93.657950460676));
    TheLinear1.AppendPoint(HGF2DPosition(10.267621787879, 256.00000000000));
    TheLinear1.AppendPoint(HGF2DPosition(256.00000000000, 256.00000000000));
    TheLinear1.AppendPoint(HGF2DPosition(256.00000000000, 0.0000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.0000000000000, 0.0000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.0000000000000, 93.657950460676));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  AddPolySegment1;

    AddPolySegment1.AppendPoint(HGF2DPosition(254.29769587355, 58.655061377831000));
    AddPolySegment1.AppendPoint(HGF2DPosition(256.00000000129, 58.547396021562000));
    AddPolySegment1.AppendPoint(HGF2DPosition(256.00000000037, 1.1175872057802E-8));
    AddPolySegment1.AppendPoint(HGF2DPosition(250.58794853646, 7.4505813718678E-9));
    AddPolySegment1.AppendPoint(HGF2DPosition(254.29769587355, 58.655061377831000));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(AddPolySegment1);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
  TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed69)
    {
    
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(-187954649.101742240000000 , -778941047.647742270000000));
    TheLinear1.AppendPoint(HGF2DPosition(-183717124.331314270000000 , -778941047.647742270000000));
    TheLinear1.AppendPoint(HGF2DPosition(-183717124.331314270000000 , -779490571.969578980000000));
    TheLinear1.AppendPoint(HGF2DPosition(-184667877.706137210000000 , -779387165.366099950000000));
    TheLinear1.AppendPoint(HGF2DPosition(-186085451.411206330000000 , -780017587.359324810000000));
    TheLinear1.AppendPoint(HGF2DPosition(-187293014.196996960000000 , -780542939.020325180000000));
    TheLinear1.AppendPoint(HGF2DPosition(-188400179.033274800000000 , -781169114.454348090000000));
    TheLinear1.AppendPoint(HGF2DPosition(-188400179.033274800000000 , -780808326.825356360000000));
    TheLinear1.AppendPoint(HGF2DPosition(-185717932.302468900000000 , -779229559.867793800000000));
    TheLinear1.AppendPoint(HGF2DPosition(-188400179.033274800000000 , -779889163.350910190000000));
    TheLinear1.AppendPoint(HGF2DPosition(-188400179.033274800000000 , -779061072.119485860000000));
    TheLinear1.AppendPoint(HGF2DPosition(-187954649.101742240000000 , -778941047.647742270000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  AddPolySegment1;

    AddPolySegment1.AppendPoint(HGF2DPosition(-186257310.228082480000000 , -779547037.017389420000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(-186903365.889312740000000 , -779927304.544230340000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(-187571727.079627280000000 , -780595665.734544870000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(-187504720.042213920000000 , -780662672.771958230000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(-187293014.196999280000000 , -780542939.020335320000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(-186085451.411205110000000 , -780017587.359333630000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(-185878704.271933910000000 , -779925642.973537920000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(-186257310.228082480000000 , -779547037.017389420000000));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(AddPolySegment1);

    // This tests DOES NOT PASS WHEN EPSILON MULTIPLICATOR is 1E-14: 1E-13 is required
    if (HNumeric<double>::EPSILON_MULTIPLICATOR() < 1E-13)
        {
        pShape2->SetAutoToleranceActive(false);
        pShape2->SetTolerance(1E-4);
        pShape1->SetAutoToleranceActive(false);
        pShape1->SetTolerance(1E-4);
        }

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed70)
    {
       
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(16712.962887222551000 , 256.000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(16703.446371366757000 , 0.18155755210318600));
    TheLinear1.AppendPoint(HGF2DPosition(9826.6569572419812000 , 256.000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(16712.962887222551000 , 256.000000000000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  AddPolySegment1;

    AddPolySegment1.AppendPoint(HGF2DPosition(16640.840166126243000 , 334.740166727452280));
    AddPolySegment1.AppendPoint(HGF2DPosition(16715.788325136309000 , 331.952074763648450));
    AddPolySegment1.AppendPoint(HGF2DPosition(16706.271637899998000 , 76.1290253463482710));
    AddPolySegment1.AppendPoint(HGF2DPosition(16631.323478889932000 , 78.9171173101520650));
    AddPolySegment1.AppendPoint(HGF2DPosition(16640.840166126243000 , 334.740166727452280));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(AddPolySegment1);

    // This tests DOES NOT PASS WHEN EPSILON is 1E-8: 1E-7 is required
    if (HNumeric<double>::GLOBAL_EPSILON() < 1E-7)
        {     
        pShape2->SetAutoToleranceActive(false);
        pShape2->SetTolerance(1E-7);
        pShape1->SetAutoToleranceActive(false);
        pShape1->SetTolerance(1E-7);
        }

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed71)
    {

    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(0.0000000000000, 93.579504606760));
    TheLinear1.AppendPoint(HGF2DPosition(10.676217878790, 256.00000000000));
    TheLinear1.AppendPoint(HGF2DPosition(256.00000000000, 256.00000000000));
    TheLinear1.AppendPoint(HGF2DPosition(256.00000000000, 0.0000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.0000000000000, 0.0000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(0.0000000000000, 93.579504606760));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  AddPolySegment1;

    AddPolySegment1.AppendPoint(HGF2DPosition(254.9769587355, 58.550613778310000));
    AddPolySegment1.AppendPoint(HGF2DPosition(256.0000000129, 58.473960215620000));
    AddPolySegment1.AppendPoint(HGF2DPosition(256.0000000037, 1.1175872057802E-7));
    AddPolySegment1.AppendPoint(HGF2DPosition(250.8794853646, 7.4505813718678E-8));
    AddPolySegment1.AppendPoint(HGF2DPosition(254.9769587355, 58.550613778310000));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(AddPolySegment1);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed72)
    {
         
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(-187954649.01742240000000 , -778941047.47742270000000));
    TheLinear1.AppendPoint(HGF2DPosition(-183717124.31314270000000 , -778941047.47742270000000));
    TheLinear1.AppendPoint(HGF2DPosition(-183717124.31314270000000 , -779490571.69578980000000));
    TheLinear1.AppendPoint(HGF2DPosition(-184667877.06137210000000 , -779387165.66099950000000));
    TheLinear1.AppendPoint(HGF2DPosition(-186085451.11206330000000 , -780017587.59324810000000));
    TheLinear1.AppendPoint(HGF2DPosition(-187293014.96996960000000 , -780542939.20325180000000));
    TheLinear1.AppendPoint(HGF2DPosition(-188400179.33274800000000 , -781169114.54348090000000));
    TheLinear1.AppendPoint(HGF2DPosition(-188400179.33274800000000 , -780808326.25356360000000));
    TheLinear1.AppendPoint(HGF2DPosition(-185717932.02468900000000 , -779229559.67793800000000));
    TheLinear1.AppendPoint(HGF2DPosition(-188400179.33274800000000 , -779889163.50910190000000));
    TheLinear1.AppendPoint(HGF2DPosition(-188400179.33274800000000 , -779061072.19485860000000));
    TheLinear1.AppendPoint(HGF2DPosition(-187954649.01742240000000 , -778941047.47742270000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  AddPolySegment1;

    AddPolySegment1.AppendPoint(HGF2DPosition(-186257310.228082480000000 , -779547037.017389420000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(-186903365.889312740000000 , -779927304.544230340000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(-187571727.079627280000000 , -780595665.734544870000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(-187504720.042213920000000 , -780662672.771958230000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(-187293014.196999280000000 , -780542939.020335320000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(-186085451.411205110000000 , -780017587.359333630000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(-185878704.271933910000000 , -779925642.973537920000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(-186257310.228082480000000 , -779547037.017389420000000));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(AddPolySegment1);
    
    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed73)
    {
           
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(16712.62887222551000 , 256.000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(16703.46371366757000 , 0.81557552103186000));
    TheLinear1.AppendPoint(HGF2DPosition(9826.569572419812000 , 256.000000000000000));
    TheLinear1.AppendPoint(HGF2DPosition(16712.62887222551000 , 256.000000000000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(TheLinear1);

    HGF2DPolySegment  AddPolySegment1;

    AddPolySegment1.AppendPoint(HGF2DPosition(16640.40166126243000 , 334.40166727452280));
    AddPolySegment1.AppendPoint(HGF2DPosition(16715.88325136309000 , 331.52074763648450));
    AddPolySegment1.AppendPoint(HGF2DPosition(16706.71637899998000 , 76.290253463482710));
    AddPolySegment1.AppendPoint(HGF2DPosition(16631.23478889932000 , 78.171173101520650));
    AddPolySegment1.AppendPoint(HGF2DPosition(16640.40166126243000 , 334.40166727452280));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(AddPolySegment1);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  WIP_IPPTEST_BUG_4)
    {
   
    #ifdef WIP_IPPTEST_BUG_4
        
    HGF2DPolySegment  AddPolySegment1;
    AddPolySegment1.AppendPoint(HGF2DPosition(343461467.000000060000000 , 517042351.999999940000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(344815973.470631420000000 , 515687845.529368520000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(346694630.449381350000000 , 517566502.508118390000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(345340123.978749870000000 , 518921008.978749810000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(343461467.000000060000000 , 517042351.999999940000000));
    
    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(AddPolySegment1);

    HGF2DPolySegment  AddPolySegment2;
    AddPolySegment2.AppendPoint(HGF2DPosition(343461467.000000060000000 , 517042351.999999880000000));
    AddPolySegment2.AppendPoint(HGF2DPosition(344815973.470683040000000 , 515687845.529316840000000));
    AddPolySegment2.AppendPoint(HGF2DPosition(346630485.970608830000000 , 517502358.029242580000000));
    AddPolySegment2.AppendPoint(HGF2DPosition(345275979.499925730000000 , 518856864.499925610000000));
    AddPolySegment2.AppendPoint(HGF2DPosition(343461467.000000060000000 , 517042351.999999880000000));
        
    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(AddPolySegment2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
    pResult = pShape1->IntersectShape(*pShape2);

    #endif

    }

//==================================================================================
// Test which failed on Nov 6, 2001  TR75930
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  WIP_IPPTEST_BUG_4_2)
    {
    
    #ifdef WIP_IPPTEST_BUG_4
        
    HGF2DPolySegment  AddPolySegment1;
    AddPolySegment1.AppendPoint(HGF2DPosition(343461467.000000060000000 , 517042351.999999940000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(344815973.470631420000000 , 515687845.529368520000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(346694630.449381350000000 , 517566502.508118390000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(345340123.978749870000000 , 518921008.978749810000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(343461467.000000060000000 , 517042351.999999940000000));
    
    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(AddPolySegment1);

    HGF2DPolySegment  AddPolySegment2;
    AddPolySegment2.AppendPoint(HGF2DPosition(343461467.000000060000000 , 517042351.999999880000000));
    AddPolySegment2.AppendPoint(HGF2DPosition(344815973.470683040000000 , 515687845.529316840000000));
    AddPolySegment2.AppendPoint(HGF2DPosition(346630485.970608830000000 , 517502358.029242580000000));
    AddPolySegment2.AppendPoint(HGF2DPosition(345275979.499925730000000 , 518856864.499925610000000));
    AddPolySegment2.AppendPoint(HGF2DPosition(343461467.000000060000000 , 517042351.999999880000000));
    
    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(AddPolySegment2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
    pResult = pShape1->IntersectShape(*pShape2);

    HGF2DPolySegment  AddPolySegment3;
    AddPolySegment3.AppendPoint(HGF2DPosition(2981.304621487855900 , 2177.617096203845000));
    AddPolySegment3.AppendPoint(HGF2DPosition(3061.379443119512900 , 2290.133409291971500));
    AddPolySegment3.AppendPoint(HGF2DPosition(2940.157848385162700 , 2383.015966769307900));
    AddPolySegment3.AppendPoint(HGF2DPosition(2861.816244216635800 , 2269.171625692397400));
    AddPolySegment3.AppendPoint(HGF2DPosition(2981.304621487855900 , 2177.617096203845000));
        
    HFCPtr<HGF2DShape> pShape3 = new HGF2DPolygonOfSegments(AddPolySegment3);

    HGF2DPolySegment  AddPolySegment4;
    AddPolySegment4.AppendPoint(HGF2DPosition(2981.304621465271300 , 2177.617095963563800));
    AddPolySegment4.AppendPoint(HGF2DPosition(3061.379443096928300 , 2290.133409049827600));
    AddPolySegment4.AppendPoint(HGF2DPosition(2940.157848363625800 , 2383.015966524835700));
    AddPolySegment4.AppendPoint(HGF2DPosition(2861.816244194866200 , 2269.171625450253500));
    AddPolySegment4.AppendPoint(HGF2DPosition(2981.304621465271300 , 2177.617095963563800));
        
    HFCPtr<HGF2DShape> pShape4 = new HGF2DPolygonOfSegments(AddPolySegment4);

    HFCPtr<HGF2DShape> pResult2 = pShape4->IntersectShape(*pShape3);
    pResult2 = pShape3->IntersectShape(*pShape4);
        
    #endif
    }

//==================================================================================
// Test which failed on Dec 16, 2003
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  IntersectShapeWithAppendPointWhoFailed)
    {
            
    HGF2DPolySegment  AddPolySegment1;
    AddPolySegment1.AppendPoint(HGF2DPosition(-0.000000000000057 , 299.815021846634070));
    AddPolySegment1.AppendPoint(HGF2DPosition(-0.000000000000057 , -0.0000000000001140));
    AddPolySegment1.AppendPoint(HGF2DPosition(2.7832031249999430 , -0.0000000000003410));
    AddPolySegment1.AppendPoint(HGF2DPosition(4.1748046875001140 , 0.00000000000022700));
    AddPolySegment1.AppendPoint(HGF2DPosition(9.7412109374999720 , -0.0000000000003410));
    AddPolySegment1.AppendPoint(HGF2DPosition(13.916015625000028 , 0.00000000000011400));
    AddPolySegment1.AppendPoint(HGF2DPosition(22.265624999999972 , -0.0000000000003410));
    AddPolySegment1.AppendPoint(HGF2DPosition(25.048828125000000 , 0.00000000000011400));
    AddPolySegment1.AppendPoint(HGF2DPosition(29.223632812499915 , -0.0000000000001140));
    AddPolySegment1.AppendPoint(HGF2DPosition(32.006835937500000 , 0.00000000000011400));
    AddPolySegment1.AppendPoint(HGF2DPosition(33.398437500000000 , 0.00000000000000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(35.485839843750057 , 0.00000000000022700));
    AddPolySegment1.AppendPoint(HGF2DPosition(42.443847656250014 , -0.0000000000003410));
    AddPolySegment1.AppendPoint(HGF2DPosition(43.139648437499986 , 0.00000000000000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(45.922851562500014 , 0.00000000000000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(47.314453125000000 , 0.00000000000011400));
    AddPolySegment1.AppendPoint(HGF2DPosition(49.401855468750000 , -0.0000000000001140));
    AddPolySegment1.AppendPoint(HGF2DPosition(50.097656250000014 , 0.00000000000022700));
    AddPolySegment1.AppendPoint(HGF2DPosition(52.880859375000028 , 0.00000000000000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(55.664062500000028 , 0.00000000000022700));
    AddPolySegment1.AppendPoint(HGF2DPosition(57.055664062499929 , 0.00000000000000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(58.447265624999929 , 0.00000000000022700));
    AddPolySegment1.AppendPoint(HGF2DPosition(59.838867187499972 , 0.00000000000000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(62.622070312499943 , 0.00000000000022700));
    AddPolySegment1.AppendPoint(HGF2DPosition(64.013671874999986 , 0.00000000000011400));
    AddPolySegment1.AppendPoint(HGF2DPosition(66.796874999999943 , 0.00000000000022700));
    AddPolySegment1.AppendPoint(HGF2DPosition(68.188476562500028 , -0.0000000000003410));
    AddPolySegment1.AppendPoint(HGF2DPosition(69.580078125000028 , -0.0000000000001140));
    AddPolySegment1.AppendPoint(HGF2DPosition(70.275878906250000 , -0.0000000000004550));
    AddPolySegment1.AppendPoint(HGF2DPosition(72.363281249999872 , 0.00000000000022700));
    AddPolySegment1.AppendPoint(HGF2DPosition(73.754882812499929 , -0.0000000000001140));
    AddPolySegment1.AppendPoint(HGF2DPosition(74.450683593749929 , 0.00000000000011400));
    AddPolySegment1.AppendPoint(HGF2DPosition(75.146484374999915 , 0.00000000000000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(76.538085937499943 , 0.00000000000034100));
    AddPolySegment1.AppendPoint(HGF2DPosition(77.233886718749915 , 0.00000000000011400));
    AddPolySegment1.AppendPoint(HGF2DPosition(77.581787109375000 , 0.00000000000022700));
    AddPolySegment1.AppendPoint(HGF2DPosition(81.408691406250057 , -0.0000000000005680));
    AddPolySegment1.AppendPoint(HGF2DPosition(82.104492187500099 , 0.00000000000000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(83.496093750000099 , -0.0000000000005680));
    AddPolySegment1.AppendPoint(HGF2DPosition(84.191894531250057 , -0.0000000000004550));
    AddPolySegment1.AppendPoint(HGF2DPosition(84.539794921875014 , -0.0000000000004550));
    AddPolySegment1.AppendPoint(HGF2DPosition(84.887695312500000 , 0.00000000000022700));
    AddPolySegment1.AppendPoint(HGF2DPosition(87.670898437500028 , 0.00000000000011400));
    AddPolySegment1.AppendPoint(HGF2DPosition(88.018798828125014 , 0.00000000000022700));
    AddPolySegment1.AppendPoint(HGF2DPosition(94.628906250000128 , -0.0000000000004550));
    AddPolySegment1.AppendPoint(HGF2DPosition(95.324707031249929 , 0.00000000000011400));
    AddPolySegment1.AppendPoint(HGF2DPosition(96.020507812499872 , -0.0000000000001140));
    AddPolySegment1.AppendPoint(HGF2DPosition(97.412109374999943 , 0.00000000000011400));
    AddPolySegment1.AppendPoint(HGF2DPosition(102.28271484374994 , -0.0000000000001140));
    AddPolySegment1.AppendPoint(HGF2DPosition(102.97851562500000 , 0.00000000000034100));
    AddPolySegment1.AppendPoint(HGF2DPosition(109.93652343749997 , -0.0000000000003410));
    AddPolySegment1.AppendPoint(HGF2DPosition(111.32812499999997 , 0.00000000000000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(115.50292968749994 , -0.0000000000003410));
    AddPolySegment1.AppendPoint(HGF2DPosition(120.02563476562499 , 0.00000000000056800));
    AddPolySegment1.AppendPoint(HGF2DPosition(122.46093750000000 , 0.00000000000022700));
    AddPolySegment1.AppendPoint(HGF2DPosition(123.15673828124997 , 0.00000000000034100));
    AddPolySegment1.AppendPoint(HGF2DPosition(123.85253906250000 , -0.0000000000001140));
    AddPolySegment1.AppendPoint(HGF2DPosition(125.24414062499999 , 0.00000000000022700));
    AddPolySegment1.AppendPoint(HGF2DPosition(126.63574218750000 , 0.00000000000011400));
    AddPolySegment1.AppendPoint(HGF2DPosition(127.33154296875000 , 0.00000000000102300));
    AddPolySegment1.AppendPoint(HGF2DPosition(129.41894531249994 , 0.00000000000011400));
    AddPolySegment1.AppendPoint(HGF2DPosition(130.11474609374994 , 0.00000000000068200));
    AddPolySegment1.AppendPoint(HGF2DPosition(133.59374999999994 , -0.0000000000017050));
    AddPolySegment1.AppendPoint(HGF2DPosition(134.28955078125006 , 0.00000000000034100));
    AddPolySegment1.AppendPoint(HGF2DPosition(136.37695312499989 , -0.0000000000011370));
    AddPolySegment1.AppendPoint(HGF2DPosition(137.76855468749969 , 0.00000000000204600));
    AddPolySegment1.AppendPoint(HGF2DPosition(139.16015624999977 , 0.00000000000102300));
    AddPolySegment1.AppendPoint(HGF2DPosition(140.55175781249963 , 0.00000000000147800));
    AddPolySegment1.AppendPoint(HGF2DPosition(141.94335937500023 , -0.0000000000003410));
    AddPolySegment1.AppendPoint(HGF2DPosition(150.29296875000014 , 0.00000000000056800));
    AddPolySegment1.AppendPoint(HGF2DPosition(151.68457031249923 , -0.0000000000023870));
    AddPolySegment1.AppendPoint(HGF2DPosition(153.07617187499974 , -0.0000000000021600));
    AddPolySegment1.AppendPoint(HGF2DPosition(155.16357421874957 , -0.0000000000026150));
    AddPolySegment1.AppendPoint(HGF2DPosition(155.85937499999977 , 0.00000000000136400));
    AddPolySegment1.AppendPoint(HGF2DPosition(161.42578125000040 , -0.0000000000017050));
    AddPolySegment1.AppendPoint(HGF2DPosition(162.81738281249957 , 0.00000000000022700));
    AddPolySegment1.AppendPoint(HGF2DPosition(164.20898437499974 , -0.0000000000003410));
    AddPolySegment1.AppendPoint(HGF2DPosition(165.94848632812540 , 0.00000000000068200));
    AddPolySegment1.AppendPoint(HGF2DPosition(169.77539062500082 , -0.0000000000001140));
    AddPolySegment1.AppendPoint(HGF2DPosition(172.90649414062534 , 0.00000000000022700));
    AddPolySegment1.AppendPoint(HGF2DPosition(175.34179687500020 , 0.00000000000011400));
    AddPolySegment1.AppendPoint(HGF2DPosition(176.03759765625045 , 0.00000000000022700));
    AddPolySegment1.AppendPoint(HGF2DPosition(176.38549804687506 , -0.0000000000005680));
    AddPolySegment1.AppendPoint(HGF2DPosition(176.73339843750026 , -0.0000000000004550));
    AddPolySegment1.AppendPoint(HGF2DPosition(178.12500000000026 , -0.0000000000006820));
    AddPolySegment1.AppendPoint(HGF2DPosition(179.86450195312520 , -0.0000000000004550));
    AddPolySegment1.AppendPoint(HGF2DPosition(180.21240234374989 , -0.0000000000010230));
    AddPolySegment1.AppendPoint(HGF2DPosition(180.90820312499980 , -0.0000000000004550));
    AddPolySegment1.AppendPoint(HGF2DPosition(183.34350585937472 , -0.0000000000007960));
    AddPolySegment1.AppendPoint(HGF2DPosition(183.69140624999997 , -0.0000000000001140));
    AddPolySegment1.AppendPoint(HGF2DPosition(187.17041015625006 , -0.0000000000004550));
    AddPolySegment1.AppendPoint(HGF2DPosition(187.86621093750006 , -0.0000000000001140));
    AddPolySegment1.AppendPoint(HGF2DPosition(189.25781250000014 , -0.0000000000005680));
    AddPolySegment1.AppendPoint(HGF2DPosition(192.04101562500006 , 0.00000000000022700));
    AddPolySegment1.AppendPoint(HGF2DPosition(197.60742187500017 , -0.0000000000005680));
    AddPolySegment1.AppendPoint(HGF2DPosition(201.08642578125006 , 0.00000000000000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(201.78222656250011 , -0.0000000000001140));
    AddPolySegment1.AppendPoint(HGF2DPosition(203.17382812499994 , 0.00000000000000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(204.56542968749994 , -0.0000000000001140));
    AddPolySegment1.AppendPoint(HGF2DPosition(205.95703125000009 , 0.00000000000011400));
    AddPolySegment1.AppendPoint(HGF2DPosition(207.34863281249997 , 0.00000000000011400));
    AddPolySegment1.AppendPoint(HGF2DPosition(211.52343750000014 , 0.00000000000034100));
    AddPolySegment1.AppendPoint(HGF2DPosition(214.30664062500003 , -0.0000000000004550));
    AddPolySegment1.AppendPoint(HGF2DPosition(215.69824218750003 , -0.0000000000001140));
    AddPolySegment1.AppendPoint(HGF2DPosition(217.08984375000014 , -0.0000000000001140));
    AddPolySegment1.AppendPoint(HGF2DPosition(219.87304687500000 , 0.00000000000011400));
    AddPolySegment1.AppendPoint(HGF2DPosition(225.43945312499983 , 0.00000000000011400));
    AddPolySegment1.AppendPoint(HGF2DPosition(228.22265624999986 , 0.00000000000022700));
    AddPolySegment1.AppendPoint(HGF2DPosition(239.35546875000014 , 0.00000000000000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(244.92187500000023 , 0.00000000000011400));
    AddPolySegment1.AppendPoint(HGF2DPosition(289.45312500000006 , -0.0000000000001140));
    AddPolySegment1.AppendPoint(HGF2DPosition(311.71875000000000 , 0.00000000000011400));
    AddPolySegment1.AppendPoint(HGF2DPosition(322.85156250000000 , -0.0000000000001140));
    AddPolySegment1.AppendPoint(HGF2DPosition(333.98437499999989 , 0.00000000000011400));
    AddPolySegment1.AppendPoint(HGF2DPosition(345.11718750000000 , 0.00000000000000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(356.24999999999989 , 0.00000000000011400));
    AddPolySegment1.AppendPoint(HGF2DPosition(356.25000000000000 , 300.000000000000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(0.2262398393800990 , 300.000000000000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(-0.000000000000057 , 299.815021846634070));
    
    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(AddPolySegment1);

    HGF2DPolySegment  AddPolySegment2;
    AddPolySegment2.AppendPoint(HGF2DPosition(0.000000000000000, 0.000000000000000));
    AddPolySegment2.AppendPoint(HGF2DPosition(0.000000000000000, 300.0000000000000));
    AddPolySegment2.AppendPoint(HGF2DPosition(357.0000000000000, 300.0000000000000));
    AddPolySegment2.AppendPoint(HGF2DPosition(357.0000000000000, 0.000000000000000));
    AddPolySegment2.AppendPoint(HGF2DPosition(0.000000000000000, 0.000000000000000));
    
    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(AddPolySegment2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
    pResult = pShape1->IntersectShape(*pShape2);
    
    }

//==================================================================================
// Test which failed on June 21, 2010
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed74)
    {
           
    HGF2DPolySegment  AddPolySegment1;
    AddPolySegment1.AppendPoint(HGF2DPosition( 4472.03735685348510000000, -4302.9712390899658000000));
    AddPolySegment1.AppendPoint(HGF2DPosition( -6261.5444221496582000000, 3287.45898628234860000000));
    AddPolySegment1.AppendPoint(HGF2DPosition( -6296.5840774774551000000, -4317.2590339183807000000));
    AddPolySegment1.AppendPoint(HGF2DPosition( 4472.03735685348510000000, -4302.9712390899658000000));
    
    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(AddPolySegment1);
        
    HGF2DPolySegment  AddPolySegment2;
    AddPolySegment2.AppendPoint(HGF2DPosition(4483.77481311559680000000, 1826.44534158706670000000));
    AddPolySegment2.AppendPoint(HGF2DPosition( 3571.5499991178513000000, 1811.31066036224370000000));
    AddPolySegment2.AppendPoint(HGF2DPosition( 3572.0260994434357000000, 3274.46059226989750000000));
    AddPolySegment2.AppendPoint(HGF2DPosition( -6261.544421970844300000, 3287.45898675918580000000));
    AddPolySegment2.AppendPoint(HGF2DPosition( 4472.0373568534851000000, -4302.9712390899658000000));
    AddPolySegment2.AppendPoint(HGF2DPosition( 4538.3313429355621000000, 3273.18329262733460000000));
    AddPolySegment2.AppendPoint(HGF2DPosition(  4491.370812356472000000, 3273.24536705017090000000));
    AddPolySegment2.AppendPoint(HGF2DPosition(  4483.774813115596800000, 1826.44534158706670000000));
    
    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(AddPolySegment2);
    pShape1->SetTolerance(0.00001);
    pShape2->SetTolerance(0.00001);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
    pResult = pShape1->IntersectShape(*pShape2);   
    pResult = pShape1->UnifyShape(*pShape2);
    pResult = pShape2->UnifyShape(*pShape1);
      
    }

//==================================================================================
// Test which failed on June 21, 2010   - second one
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed75)
    {
           
    HGF2DPolySegment  AddPolySegment1;
    AddPolySegment1.AppendPoint(HGF2DPosition(47785.000000, 6605.000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(47775.000000, 6765.000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(48035.000000, 6855.000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(48045.000000, 6755.000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(48055.000000, 6625.000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(47975.000000, 6515.000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(47895.000000, 6505.000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(47845.000000, 6515.000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(47785.000000, 6535.000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(47785.000000, 6565.000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(47785.000000, 6605.000000));
    AddPolySegment1.AppendPoint(HGF2DPosition(47785.000000, 6605.000000));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(AddPolySegment1);

    HGF2DPolySegment  AddPolySegment2;
    AddPolySegment2.AppendPoint(HGF2DPosition(47895.000000, 6505.000000));
    AddPolySegment2.AppendPoint(HGF2DPosition(48055.000000, 6575.000000));
    AddPolySegment2.AppendPoint(HGF2DPosition(47965.000000, 6655.000000));
    AddPolySegment2.AppendPoint(HGF2DPosition(47845.000000, 6515.000000));
    AddPolySegment2.AppendPoint(HGF2DPosition(47895.000000, 6505.000000));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(AddPolySegment2);

    HFCPtr<HGF2DShape> pResult = pShape2->IntersectShape(*pShape1);
    pResult = pShape1->IntersectShape(*pShape2);
    pResult = pShape1->UnifyShape(*pShape2);
    pResult = pShape2->UnifyShape(*pShape1);
      
    }

//==================================================================================
// Test which failed on June 21, 2010   - second one
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  WIP_IPPTEST_BUG_4_3)
    {
    
    #ifdef WIP_IPPTEST_BUG_4      
    HGF2DRectangle MyRectangle(0.0, 0.0, 776.0, 530.0);

    HGF2DPolySegment  AddPolySegment1;
    AddPolySegment1.AppendPoint(HGF2DPosition(173881.594315156690, 31262.7623784915300 ));
    AddPolySegment1.AppendPoint(HGF2DPosition(-672196.87396625290, -119580.92306234955  ));
    AddPolySegment1.AppendPoint(HGF2DPosition(-207684.53954652144, -37058.254313532634  ));
    AddPolySegment1.AppendPoint(HGF2DPosition(403111.141213428460, 72700.3005556127460 ));
    AddPolySegment1.AppendPoint(HGF2DPosition(173881.594315156690, 31262.7623784915300  ));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(AddPolySegment1);

    HFCPtr<HGF2DShape> pResult = MyRectangle.IntersectShape (*pShape1);
    #endif
    
    }

//==================================================================================
// Test which failed on April 21, 2011
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  ModifyShapeWithPointerWhoFailed76)
    {

    HGF2DPolySegment  AddPolySegment1;
    AddPolySegment1.AppendPoint(HGF2DPosition(2322428.2272334308,-129984.05221858087));
    AddPolySegment1.AppendPoint(HGF2DPosition(3403111.0235287589,-129984.05221858087));
    AddPolySegment1.AppendPoint(HGF2DPosition(3403111.0235287589, 683219.40062587289 ));
    AddPolySegment1.AppendPoint(HGF2DPosition(2322428.2272334308, 683219.40062587289 ));
    AddPolySegment1.AppendPoint(HGF2DPosition(2322428.2272334308,-129984.05221858087));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(AddPolySegment1);

    HGF2DPolySegment  AddPolySegment2;
    AddPolySegment2.AppendPoint(HGF2DPosition(2819212.7968047597, 226067.48340849765));
    AddPolySegment2.AppendPoint(HGF2DPosition(2776007.6856932733, 290851.21372304671));
    AddPolySegment2.AppendPoint(HGF2DPosition(2908987.9398707948, 382674.45995559543));
    AddPolySegment2.AppendPoint(HGF2DPosition(3038933.3691252591, 478744.44468378508));
    AddPolySegment2.AppendPoint(HGF2DPosition(3165708.4068487957, 578960.94212805433));
    AddPolySegment2.AppendPoint(HGF2DPosition(3289180.7939698473, 683219.40062630130));
    AddPolySegment2.AppendPoint(HGF2DPosition(3340372.8070185226, 624542.39371858397));
    AddPolySegment2.AppendPoint(HGF2DPosition(3214978.7255764296, 518661.28182966216));
    AddPolySegment2.AppendPoint(HGF2DPosition(3086230.5918779657, 416885.03909736732));
    AddPolySegment2.AppendPoint(HGF2DPosition(2954262.7234428381, 319319.84440054093));
    AddPolySegment2.AppendPoint(HGF2DPosition(2819212.7968047597, 226067.48340849765));

    HFCPtr<HGF2DShape> pShape2 = new HGF2DPolygonOfSegments(AddPolySegment2);

    HFCPtr<HGF2DShape> pResult = pShape1->IntersectShape (*pShape2);

    }

//==================================================================================
// Test which failed on April 21, 2011
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  UnifyShapeWhoFailed)
    {
           
    HGF2DRectangle rect1(255000.00, -190000.0, 260000.0, -185000.0);
    HGF2DRectangle rect2(255000.00, -200000.0, 260000.0, -195000.0);
    HGF2DRectangle rect3(255000.00, -195000.0, 260000.0, -190000.0);
    HGF2DRectangle rect4(260000.00, -190000.0, 265000.0, -185000.0);
    HGF2DRectangle rect5(265000.00, -200000.0, 270000.0, -195000.0);
    HGF2DRectangle rect6(260000.00, -200000.0, 265000.0, -195000.0);
    HGF2DRectangle rect7(265000.00, -195000.0, 270000.0, -190000.0);
    HGF2DRectangle rect8(260000.00, -195000.0, 265000.0, -190000.0);
    HGF2DRectangle rect9(270000.00, -190000.0, 275000.0, -185000.0);
    HGF2DRectangle rect10(270000.0, -200000.0, 275000.0, -195000.0);
    HGF2DRectangle rect11(270000.0, -195000.0, 275000.0, -190000.0);

    HFCPtr<HGF2DShape>pResult = rect1.UnifyShape (rect2);
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
TEST_F(HGF2DPolygonOfSegmentsTester,  IntersectShapeWhoFailed)
    {        
    
    HGF2DPolySegment  AddPolySegment1;
    AddPolySegment1.AppendPoint(HGF2DPosition(-8885727.1063404903, 9056297.5039131828));
    AddPolySegment1.AppendPoint(HGF2DPosition(-8884261.2748356014, 8864533.2997093443));
    AddPolySegment1.AppendPoint(HGF2DPosition(-5136267.6785397073, 8893182.6877240464));
    AddPolySegment1.AppendPoint(HGF2DPosition(-5137733.5100445943, 9084946.8919278868));
    AddPolySegment1.AppendPoint(HGF2DPosition(-8885727.1063404903, 9056297.5039131828));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(AddPolySegment1);

    HGF2DPolySegment  AddPolySegment2;
    AddPolySegment2.AppendPoint(HGF2DPosition(-8885727.1063404903, 9056297.5039131828));
    AddPolySegment2.AppendPoint(HGF2DPosition(-5137733.5100445943, 9084946.8919278868));
    AddPolySegment2.AppendPoint(HGF2DPosition(-5113480.2895529531, 5912072.3074665070));
    AddPolySegment2.AppendPoint(HGF2DPosition(-8861473.8858488481, 5883422.9194518048));
    AddPolySegment2.AppendPoint(HGF2DPosition(-8885727.1063404903, 9056297.5039131828));

    HFCPtr<HGF2DShape>  pShape2 = new HGF2DPolygonOfSegments(AddPolySegment2);

    HFCPtr<HGF2DShape>  pResult = pShape1->IntersectShape (*pShape2);
        
    }


//==================================================================================
// Test which failed on Oct 16, 2015
//==================================================================================
TEST_F(HGF2DPolygonOfSegmentsTester,  SpatialPoisitionWhoFailed)
    {        
    
    HGF2DPolySegment  AddPolySegment1;
    AddPolySegment1.AppendPoint(HGF2DPosition(-96.50, 39.875));
    AddPolySegment1.AppendPoint(HGF2DPosition(-96.50, 39.930));
    AddPolySegment1.AppendPoint(HGF2DPosition(-96.43, 39.930));
    AddPolySegment1.AppendPoint(HGF2DPosition(-96.43, 39.875));
    AddPolySegment1.AppendPoint(HGF2DPosition(-96.50, 39.875));

    HFCPtr<HGF2DShape> pShape1 = new HGF2DPolygonOfSegments(AddPolySegment1);

    HGF2DPolySegment  AddPolySegment2;
    AddPolySegment2.AppendPoint(HGF2DPosition(-96.500, 39.875));
    AddPolySegment2.AppendPoint(HGF2DPosition(-96.500, 39.930));
    AddPolySegment2.AppendPoint(HGF2DPosition(-96.562, 39.930));
    AddPolySegment2.AppendPoint(HGF2DPosition(-96.562, 39.875));
    AddPolySegment2.AppendPoint(HGF2DPosition(-96.500, 39.875));

    HFCPtr<HGF2DShape>  pShape2 = new HGF2DPolygonOfSegments(AddPolySegment2);

    ASSERT_EQ(HGF2DShape::S_OUT, pShape1->CalculateSpatialPositionOf(*pShape2));
    ASSERT_EQ(HGF2DShape::S_OUT, pShape2->CalculateSpatialPositionOf(*pShape1));
        
    }