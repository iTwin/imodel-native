//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DRectangleTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "EnvironnementTest.h"
#include "HGF2DRectangleTester.h"

//==================================================================================
// Rectangle Construction tests
//==================================================================================
TEST_F (HGF2DRectangleTest, ConstructionTest)
    {

    // Default Constructor
    HGF2DRectangle    ARect1;

    // Constructor with a coordinate system
    HGF2DRectangle    ARect2;

    //Constructor with Double
    HGF2DRectangle    ARect3(10.0, 10.1, 20.0, 20.1);
    ASSERT_TRUE(ARect3.IsPointOn(HGF2DPosition(10.0, 10.1)));
    ASSERT_TRUE(ARect3.IsPointOn(HGF2DPosition(20.0, 10.1)));
    ASSERT_TRUE(ARect3.IsPointOn(HGF2DPosition(10.0, 20.1)));
    ASSERT_TRUE(ARect3.IsPointOn(HGF2DPosition(20.0, 20.1)));

    //Constructor with Location
    HGF2DRectangle    ARect4(HGF2DPosition(10.0, 10.1), HGF2DPosition(20.0, 20.1));
    ASSERT_TRUE(ARect4.IsPointOn(HGF2DPosition(10.0, 10.1)));
    ASSERT_TRUE(ARect4.IsPointOn(HGF2DPosition(20.0, 10.1)));
    ASSERT_TRUE(ARect4.IsPointOn(HGF2DPosition(10.0, 20.1)));
    ASSERT_TRUE(ARect4.IsPointOn(HGF2DPosition(20.0, 20.1)));

    //Constructor with Extent
    HGF2DRectangle    ARect5(HGF2DLiteExtent(10.0, 10.1, 20.0, 20.1));
    ASSERT_TRUE(ARect5.IsPointOn(HGF2DPosition(10.0, 10.1)));
    ASSERT_TRUE(ARect5.IsPointOn(HGF2DPosition(20.0, 10.1)));
    ASSERT_TRUE(ARect5.IsPointOn(HGF2DPosition(10.0, 20.1)));
    ASSERT_TRUE(ARect5.IsPointOn(HGF2DPosition(20.0, 20.1)));

    // Copy Constructor test
    HGF2DRectangle    ARect6(ARect5);
    ASSERT_TRUE(ARect6.IsPointOn(HGF2DPosition(10.0, 10.1)));
    ASSERT_TRUE(ARect6.IsPointOn(HGF2DPosition(20.0, 10.1)));
    ASSERT_TRUE(ARect6.IsPointOn(HGF2DPosition(10.0, 20.1)));
    ASSERT_TRUE(ARect6.IsPointOn(HGF2DPosition(20.0, 20.1)));

    }

//==================================================================================
// operator= test
// operator=(const HGF2DRectangle& pi_rObj);
//==================================================================================
TEST_F (HGF2DRectangleTest, OperatorTest)
    {
    
    HGF2DRectangle    ARect1;
    HGF2DRectangle    ARect2(10.0, 10.1, 20.0, 20.1);

    ARect1 = ARect2;

    ASSERT_TRUE(ARect1.IsPointOn(HGF2DPosition(10.0, 10.1)));
    ASSERT_TRUE(ARect1.IsPointOn(HGF2DPosition(20.0, 10.1)));
    ASSERT_TRUE(ARect1.IsPointOn(HGF2DPosition(10.0, 20.1)));
    ASSERT_TRUE(ARect1.IsPointOn(HGF2DPosition(20.0, 20.1)));

    }

//==================================================================================
//  SetRectangle(const HGF2DPosition& pi_rFirstPoint, const HGF2DPosition& pi_rSecondPoint);
//  SetRectangle(double pi_XMin ,double pi_YMin, double pi_XMax, double pi_YMax);
//  GetRectangle(HGF2DPosition* pi_pMinPoint, HGF2DPosition* pi_pMaxPoint) const;
//  GetRectangle(double* po_pXMin, double* po_pYMin, double* po_pXMax, double* po_pYMax) const;
//==================================================================================
TEST_F (HGF2DRectangleTest, GetSetRectangleTest)
    {

    HGF2DRectangle    ARect1;
    ARect1.SetRectangle(10.0, 10.1, 20.0, 20.1);
    
    ASSERT_TRUE(ARect1.IsPointOn(HGF2DPosition(10.0, 10.1)));
    ASSERT_TRUE(ARect1.IsPointOn(HGF2DPosition(20.0, 10.1)));
    ASSERT_TRUE(ARect1.IsPointOn(HGF2DPosition(10.0, 20.1)));
    ASSERT_TRUE(ARect1.IsPointOn(HGF2DPosition(20.0, 20.1)));   
    
    HGF2DRectangle    ARect2;
    ARect2.SetRectangle(HGF2DPosition(10.0, 10.0), HGF2DPosition(20.0, 20.1));
    
    ASSERT_TRUE(ARect2.IsPointOn(HGF2DPosition(10.0, 10.1)));
    ASSERT_TRUE(ARect2.IsPointOn(HGF2DPosition(20.0, 10.1)));
    ASSERT_TRUE(ARect2.IsPointOn(HGF2DPosition(10.0, 20.1)));
    ASSERT_TRUE(ARect2.IsPointOn(HGF2DPosition(20.0, 20.1)));  

    double Xmin;
    double Xmax;
    double Ymin;
    double Ymax;

    ARect2.GetRectangle(&Xmin, &Ymin, &Xmax, &Ymax);
    ASSERT_DOUBLE_EQ(10.0, Xmin);
    ASSERT_DOUBLE_EQ(10.0, Ymin);
    ASSERT_DOUBLE_EQ(20.0, Xmax);
    ASSERT_DOUBLE_EQ(20.1, Ymax);
    
    HGF2DPosition FirstLocation;
    HGF2DPosition SecondLocation;

    ARect2.GetRectangle(&FirstLocation, &SecondLocation);
    ASSERT_DOUBLE_EQ(10.0, FirstLocation.GetX());
    ASSERT_DOUBLE_EQ(10.0, FirstLocation.GetY());
    ASSERT_DOUBLE_EQ(20.0, SecondLocation.GetX());
    ASSERT_DOUBLE_EQ(20.1, SecondLocation.GetY());

    }

//==================================================================================
//  Overlaps(const HGF2DShape& pi_rShape) const;
//==================================================================================
TEST_F (HGF2DRectangleTest, OverlapsTest)
    {

    ASSERT_TRUE(Rect1A.Overlaps(Rect1A));
    ASSERT_FALSE(NETipRectA.Overlaps(NWTipRectA));

    }

//==================================================================================
// GetLinear( HGF2DSimpleShape::RotationDirection pi_DirectionDesired ) const
// AllocateLinear( HGF2DSimpleShape::RotationDirection pi_DirectionDesired ) const
//==================================================================================
TEST_F (HGF2DRectangleTest, GetLinearTest)
    {

    HFCPtr<HGF2DLinear> theLinear = Rect1A.GetLinear();
    HFCPtr<HGF2DPolySegment> thePolySegment = static_cast<HGF2DPolySegment*>(&*theLinear);
    HGF2DPolySegment  MyLinearOfRect1(*thePolySegment);

    // verify that there are 4 linears
    ASSERT_EQ(4, MyLinearOfRect1.GetSize());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRect1.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRect1.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRect1.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRect1.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRect1.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRect1.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRect1.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRect1.GetPoint(3).GetY());

    HFCPtr<HGF2DLinear> theLinear2 = Rect1A.GetLinear(HGF2DSimpleShape::CW);
    HFCPtr<HGF2DPolySegment> thePolySegment2 = static_cast<HGF2DPolySegment*>(&*theLinear2);
    
    HGF2DPolySegment  MyLinearOfRectCW(*thePolySegment2);
    ASSERT_EQ(4, MyLinearOfRectCW.GetSize());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRectCW.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRectCW.GetPoint(0).GetY());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRectCW.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRectCW.GetPoint(1).GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRectCW.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRectCW.GetPoint(2).GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRectCW.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRectCW.GetPoint(3).GetY());

    HFCPtr<HGF2DLinear> theLinear3 = Rect1A.GetLinear(HGF2DSimpleShape::CCW);
    HFCPtr<HGF2DPolySegment> thePolySegment3 = static_cast<HGF2DPolySegment*>(&*theLinear3);

    HGF2DPolySegment  MyLinearOfRectCCW(*thePolySegment3);
    ASSERT_EQ(4, MyLinearOfRectCCW.GetSize());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRectCCW.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRectCCW.GetPoint(0).GetY());
                                                        
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRectCCW.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRectCCW.GetPoint(1).GetY());
                                                        
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRectCCW.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRectCCW.GetPoint(2).GetY());
                                                        
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRectCCW.GetPoint(3).GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRectCCW.GetPoint(3).GetY());

    }

//==================================================================================
// CalculatePerimeter() const;
//==================================================================================
TEST_F (HGF2DRectangleTest, CalculatePerimeterTest)
    {

    ASSERT_DOUBLE_EQ(40.0, Rect1.CalculatePerimeter());
    ASSERT_DOUBLE_EQ(20.0, IncludedRect1.CalculatePerimeter());
    ASSERT_DOUBLE_EQ(20.0, IncludedRect2.CalculatePerimeter());
    ASSERT_DOUBLE_EQ(20.0, IncludedRect3.CalculatePerimeter());
    ASSERT_DOUBLE_EQ(20.0, IncludedRect4.CalculatePerimeter());
    ASSERT_DOUBLE_EQ(24.0, IncludedRect5.CalculatePerimeter());
    ASSERT_DOUBLE_EQ(30.0, IncludedRect6.CalculatePerimeter());
    ASSERT_DOUBLE_EQ(30.0, IncludedRect7.CalculatePerimeter());
    ASSERT_DOUBLE_EQ(30.0, IncludedRect8.CalculatePerimeter());
    ASSERT_DOUBLE_EQ(30.0, IncludedRect9.CalculatePerimeter());

    }

//==================================================================================
// CalculateArea() const;
//==================================================================================
TEST_F (HGF2DRectangleTest, CalculateAreaTest)
    {

    ASSERT_DOUBLE_EQ(100.0, Rect1A.CalculateArea());
    ASSERT_DOUBLE_EQ(100.0, NegativeRectA.CalculateArea());
    ASSERT_DOUBLE_EQ(25.00, IncludedRect1A.CalculateArea());
    ASSERT_DOUBLE_EQ(25.00, IncludedRect2A.CalculateArea());
    ASSERT_DOUBLE_EQ(25.00, IncludedRect3A.CalculateArea());
    ASSERT_DOUBLE_EQ(25.00, IncludedRect4A.CalculateArea());
    ASSERT_DOUBLE_EQ(36.00, IncludedRect5A.CalculateArea());
    ASSERT_DOUBLE_EQ(50.00, IncludedRect6A.CalculateArea());
    ASSERT_DOUBLE_EQ(50.00, IncludedRect7A.CalculateArea());
    ASSERT_DOUBLE_EQ(50.00, IncludedRect8A.CalculateArea());
    ASSERT_DOUBLE_EQ(50.00, IncludedRect9A.CalculateArea());

    }

//==================================================================================
// CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const;
//==================================================================================
TEST_F (HGF2DRectangleTest, CalculateClosestPointTest)
    {

    // Test with linear 1
    ASSERT_DOUBLE_EQ(20.0, Rect1A.CalculateClosestPoint(RectClosePoint1AA).GetX());
    ASSERT_DOUBLE_EQ(10.1, Rect1A.CalculateClosestPoint(RectClosePoint1AA).GetY());
    ASSERT_DOUBLE_EQ(10.0, Rect1A.CalculateClosestPoint(RectClosePoint1BA).GetX());
    ASSERT_DOUBLE_EQ(10.0, Rect1A.CalculateClosestPoint(RectClosePoint1BA).GetY());
    ASSERT_DOUBLE_EQ(19.9, Rect1A.CalculateClosestPoint(RectClosePoint1CA).GetX());
    ASSERT_DOUBLE_EQ(10.0, Rect1A.CalculateClosestPoint(RectClosePoint1CA).GetY());
    ASSERT_DOUBLE_EQ(10.0, Rect1A.CalculateClosestPoint(RectClosePoint1DA).GetX());
    ASSERT_DOUBLE_EQ(15.0, Rect1A.CalculateClosestPoint(RectClosePoint1DA).GetY());
    ASSERT_DOUBLE_EQ(15.0, Rect1A.CalculateClosestPoint(RectCloseMidPoint1A).GetX());
    ASSERT_DOUBLE_EQ(20.0, Rect1A.CalculateClosestPoint(RectCloseMidPoint1A).GetY());

    // Tests with special points
    ASSERT_DOUBLE_EQ(20.0, Rect1A.CalculateClosestPoint(VeryFarPointA).GetX());
    ASSERT_DOUBLE_EQ(20.0, Rect1A.CalculateClosestPoint(VeryFarPointA).GetY());
    ASSERT_DOUBLE_EQ(15.0, Rect1A.CalculateClosestPoint(RectMidPoint1A).GetX());
    ASSERT_DOUBLE_EQ(20.0, Rect1A.CalculateClosestPoint(RectMidPoint1A).GetY());

    }

//==================================================================================
// Intersection test (with other complex linears only)
// Intersect(const HGF2DVector& pi_rVector, HGF2DPositionCollection* po_pCrossPoints) const;
//==================================================================================
TEST_F (HGF2DRectangleTest, IntersectTest)
    {

    HGF2DPositionCollection   DumPoints;

    // Test with extent disjoint linears
    ASSERT_EQ(0, Rect1A.Intersect(DisjointLinear1A, &DumPoints));

    // Test with disjoint but touching by a side
    ASSERT_EQ(0, Rect1A.Intersect(ContiguousExtentLinear1A, &DumPoints));

    // Test with disjoint but touching by a tip but not linked
    ASSERT_EQ(0, Rect1A.Intersect(FlirtingExtentLinear1A, &DumPoints));

    // Tests with connected linears
    // At start point...
    ASSERT_EQ(0, Rect1A.Intersect(ConnectingLinear1B, &DumPoints));

    // At end point ...
    ASSERT_EQ(0, Rect1A.Intersect(ConnectingLinear1AA, &DumPoints));

    // Tests with linked segments
    ASSERT_EQ(0, Rect1A.Intersect(LinkedLinear1B, &DumPoints));

    // Special cases
    ASSERT_EQ(1, Rect1A.Intersect(ComplexLinearCase1A, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(13.5, DumPoints[0].GetY());
    DumPoints.clear();

    ASSERT_EQ(1, Rect1A.Intersect(ComplexLinearCase2A, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(15.0, DumPoints[0].GetY());
    DumPoints.clear();

    ASSERT_EQ(1, Rect1A.Intersect(ComplexLinearCase3A, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());
    DumPoints.clear();

    ASSERT_EQ(0, Rect1A.Intersect(ComplexLinearCase4A, &DumPoints));
    DumPoints.clear();

    ASSERT_EQ(1, Rect1A.Intersect(ComplexLinearCase5B, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());
    DumPoints.clear();

    ASSERT_EQ(0, Rect1A.Intersect(ComplexLinearCase5AA, &DumPoints));
    ASSERT_EQ(0, Rect1A.Intersect(ComplexLinearCase6A, &DumPoints));
    ASSERT_EQ(0, Rect1A.Intersect(ComplexLinearCase7A, &DumPoints));

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
TEST_F (HGF2DRectangleTest, ContiguousnessTest)
    {
    HGF2DPositionCollection     DumPoints;
    HGF2DPosition   FirstDumPoint;
    HGF2DPosition   SecondDumPoint;

    // Test with contiguous linears
    ASSERT_TRUE(Rect1A.AreContiguous(ComplexLinearCase6A));

    ASSERT_TRUE(Rect1A.AreContiguousAt(ComplexLinearCase6A, RectMidPoint1A));

    ASSERT_EQ(2, Rect1A.ObtainContiguousnessPoints(ComplexLinearCase6A, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetY());

    Rect1A.ObtainContiguousnessPointsAt(ComplexLinearCase6A, LinearMidPoint1A, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(10.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetY());

    // Test with non contiguous linears
    ASSERT_FALSE(Rect1A.AreContiguous(ComplexLinearCase1A));
    DumPoints.clear();

    // Test with contiguous rectangle
    ASSERT_TRUE(Rect1A.AreContiguous(NorthContiguousRectA));
    ASSERT_TRUE(Rect1A.AreContiguousAt(NorthContiguousRectA, RectMidPoint1A));

    ASSERT_EQ(2, Rect1A.ObtainContiguousnessPoints(NorthContiguousRectA, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[1].GetY());

    Rect1A.ObtainContiguousnessPointsAt(NorthContiguousRectA, RectMidPoint1A, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(10.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(20.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, SecondDumPoint.GetY());

    DumPoints.clear();

    ASSERT_TRUE(Rect1A.AreContiguous(VerticalFitRectA));
    ASSERT_TRUE(Rect1A.AreContiguousAt(VerticalFitRectA, HGF2DPosition(17.0, 10.0)));
    ASSERT_EQ(4, Rect1A.ObtainContiguousnessPoints(VerticalFitRectA, &DumPoints));
    ASSERT_DOUBLE_EQ(15.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[1].GetY());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[2].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[2].GetY());
    ASSERT_DOUBLE_EQ(15.0, DumPoints[3].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[3].GetY());

    Rect1A.ObtainContiguousnessPointsAt(VerticalFitRectA, HGF2DPosition(17.0, 10.0), &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(20.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(15.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetY());

    DumPoints.clear();

    ASSERT_TRUE(Rect1A.AreContiguous(IncludedRect1A));

    ASSERT_TRUE(Rect1A.AreContiguousAt(IncludedRect1A, HGF2DPosition(10.0, 10.0)));

    ASSERT_EQ(2, Rect1A.ObtainContiguousnessPoints(IncludedRect1A, &DumPoints));
    ASSERT_DOUBLE_EQ(15.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(15.0, DumPoints[1].GetY());

    Rect1A.ObtainContiguousnessPointsAt(IncludedRect1A, HGF2DPosition(10.0, 10.0), &FirstDumPoint, &SecondDumPoint);
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
TEST_F (HGF2DRectangleTest, CloningTest)
    {

    // General Clone Test
    HFCPtr<HGF2DRectangle> pClone = (HGF2DRectangle*)Rect1A.Clone();
    ASSERT_FALSE(pClone->IsEmpty());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pClone->GetShapeType());

    ASSERT_TRUE(pClone->IsPointOn(HGF2DPosition(10.0, 10.0)));  
    ASSERT_TRUE(pClone->IsPointOn(HGF2DPosition(10.0, 20.0))); 
    ASSERT_TRUE(pClone->IsPointOn(HGF2DPosition(20.0, 20.0))); 
    ASSERT_TRUE(pClone->IsPointOn(HGF2DPosition(20.0, 10.0))); 

     // Test with the same coordinate system
    HFCPtr<HGF2DRectangle> pClone3 = (HGF2DRectangle*)(&*(Rect1A.AllocTransformDirect(HGF2DIdentity())));
    ASSERT_FALSE(pClone3->IsEmpty());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pClone3->GetShapeType());

    ASSERT_TRUE(pClone3->IsPointOn(HGF2DPosition(10.0, 10.0)));  
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DPosition(10.0, 20.0))); 
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DPosition(20.0, 20.0))); 
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DPosition(20.0, 10.0))); 

    // Test with a translation between systems
    Translation = HGF2DDisplacement (10.0, 10.0);
    HGF2DTranslation myTranslation(Translation);

    HFCPtr<HGF2DRectangle> pClone5 = (HGF2DRectangle*)(&*(Rect1A.AllocTransformDirect(myTranslation)));
    ASSERT_FALSE(pClone5->IsEmpty());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pClone5->GetShapeType());

    ASSERT_TRUE(pClone5->IsPointOn(HGF2DPosition(0.0, 0.0)));  
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DPosition(0.0, 10.0))); 
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DPosition(10.0, 10.0))); 
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DPosition(10.0, 0.0))); 

    // Test with a stretch between systems
    HGF2DStretch myStretch;
    myStretch.SetTranslation(Translation);
    myStretch.SetXScaling(0.5);
    myStretch.SetYScaling(0.5);

    HFCPtr<HGF2DRectangle> pClone6 = (HGF2DRectangle*)(&*(Rect1A.AllocTransformDirect(myStretch)));
    ASSERT_FALSE(pClone6->IsEmpty());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pClone6->GetShapeType());

    ASSERT_TRUE(pClone6->IsPointOn(HGF2DPosition(0.0, 0.0)));  
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DPosition(0.0, 20.0))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DPosition(20.0, 20.0))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DPosition(20.0, 0.0))); 

    // Test with a stretch between systems
    HGF2DStretch myStretch2;
    myStretch2.SetXScaling(0.5);
    myStretch2.SetYScaling(0.5);

    HFCPtr<HGF2DRectangle> pClone10 = (HGF2DRectangle*)(&*(Rect1A.AllocTransformDirect(myStretch2)));
    ASSERT_FALSE(pClone10->IsEmpty());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pClone10->GetShapeType());

    ASSERT_TRUE(pClone10->IsPointOn(HGF2DPosition(20.0, 20.0)));  
    ASSERT_TRUE(pClone10->IsPointOn(HGF2DPosition(20.0, 40.0))); 
    ASSERT_TRUE(pClone10->IsPointOn(HGF2DPosition(40.0, 40.0))); 
    ASSERT_TRUE(pClone10->IsPointOn(HGF2DPosition(40.0, 40.0))); 

    // Test with a similitude between systems
    HGF2DSimilitude mySimilitude;
    mySimilitude.SetRotation(PI);
    mySimilitude.SetScaling(0.5);

    HFCPtr<HGF2DRectangle> pClone7 = (HGF2DRectangle*)(&*(Rect1A.AllocTransformDirect(mySimilitude)));
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

    HFCPtr<HGF2DRectangle> pClone8 = (HGF2DRectangle*)(&*(Rect1A.AllocTransformDirect(myAffine)));
    ASSERT_FALSE(pClone8->IsEmpty());
    ASSERT_EQ(HGF2DPolygonOfSegments::CLASS_ID, pClone8->GetShapeType());

    ASSERT_TRUE(pClone8->IsPointOn(HGF2DPosition(0.0, 0.0)));  
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DPosition(0.0, -20.0))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DPosition(-20.0, -20.0))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DPosition(-20.0, 0.0))); 

    // Test with a similitude between systems
    Rect1A.MakeEmpty();

    HFCPtr<HGF2DRectangle> pClone9 = (HGF2DRectangle*)(&*(Rect1A.AllocTransformDirect(mySimilitude)));

    ASSERT_TRUE(pClone9->IsEmpty());
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pClone9->GetShapeType());
        
    }
    

//==================================================================================
// Interaction info with other segment
// Crosses(const HGF2DVector& pi_rVector) const;
// AreAdjacent(const HGF2DVector& pi_rVector) const;
// IsPointOn(const HGF2DPosition& pi_rTestPoint) const;
// IsPointOnSCS(const HGF2DPosition& pi_rTestPoint) const;
//==================================================================================
TEST_F (HGF2DRectangleTest, InteractionTest)
    {

    // Tests with a vertical segment
    ASSERT_TRUE(Rect1A.Crosses(ComplexLinearCase1A));
    ASSERT_FALSE(Rect1A.AreAdjacent(ComplexLinearCase1A));

    ASSERT_TRUE(Rect1A.Crosses(ComplexLinearCase2A));
    ASSERT_TRUE(Rect1A.AreAdjacent(ComplexLinearCase2A));

    ASSERT_TRUE(Rect1A.Crosses(ComplexLinearCase3A));
    ASSERT_FALSE(Rect1A.AreAdjacent(ComplexLinearCase3A));

    ASSERT_FALSE(Rect1A.Crosses(ComplexLinearCase4A));
    ASSERT_TRUE(Rect1A.AreAdjacent(ComplexLinearCase4A));

    ASSERT_TRUE(Rect1A.Crosses(ComplexLinearCase5B));
    ASSERT_TRUE(Rect1A.AreAdjacent(ComplexLinearCase5B));

    ASSERT_FALSE(Rect1A.Crosses(ComplexLinearCase6A));
    ASSERT_TRUE(Rect1A.AreAdjacent(ComplexLinearCase6A));

    ASSERT_FALSE(Rect1A.Crosses(ComplexLinearCase7A));
    ASSERT_FALSE(Rect1A.AreAdjacent(ComplexLinearCase7A));

    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(10.0 - 1.1*MYEPSILON, 10.0-1.1*MYEPSILON)));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(RectMidPoint1A.GetX(), RectMidPoint1A.GetY()-1.1*MYEPSILON)));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(RectMidPoint1A.GetX(), RectMidPoint1A.GetY()+1.1*MYEPSILON)));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(RectMidPoint1A.GetX(), RectMidPoint1A.GetY()-0.9*MYEPSILON)));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(RectMidPoint1A.GetX(), RectMidPoint1A.GetY()+0.9*MYEPSILON)));
    ASSERT_TRUE(Rect1A.IsPointOn(RectMidPoint1A));
    ASSERT_TRUE(Rect1A.IsPointOn(Rect1A.GetLinear()->GetStartPoint(), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1A.IsPointOn(Rect1A.GetLinear()->GetEndPoint(), HGF2DVector::EXCLUDE_EXTREMITIES));

    }

//==================================================================================
// Bearing calculation tests
// CalculateBearing(const HGF2DPosition& pi_rPositionPoint,
//                  HGF2DVector::ArbitraryDirection pi_Direction = HGF2DVector::BETA) const;
// CalculateAngularAcceleration(const HGF2DPosition& pi_rPositionPoint,
//                              HGF2DVector::ArbitraryDirection pi_Direction = HGF2DVector::BETA) const;
//==================================================================================
TEST_F (HGF2DRectangleTest, BearingTest)
    {

    // Obtain bearing ALPHA of a vertical segment
    ASSERT_NEAR(0.0, Rect1A.CalculateBearing(Rect1Point0d0A, HGF2DVector::ALPHA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.5707963267948966, Rect1A.CalculateBearing(Rect1Point0d0A, HGF2DVector::BETA).GetAngle());
    ASSERT_NEAR(0.0, Rect1A.CalculateBearing(Rect1Point0d1A, HGF2DVector::ALPHA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(3.1415926535897931, Rect1A.CalculateBearing(Rect1Point0d1A, HGF2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(3.1415926535897931, Rect1A.CalculateBearing(Rect1Point0d5A, HGF2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(4.7123889803846897, Rect1A.CalculateBearing(Rect1Point0d5A, HGF2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(4.7123889803846897, Rect1A.CalculateBearing(Rect1Point1d0A, HGF2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(1.5707963267948966, Rect1A.CalculateBearing(Rect1Point1d0A, HGF2DVector::BETA).GetAngle());

    // ANGULAR ACCELERATION
    // Obtain angular acceleration ALPHA of a vertical segment
    ASSERT_NEAR(0.0, Rect1A.CalculateAngularAcceleration(Rect1Point0d0A, HGF2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, Rect1A.CalculateAngularAcceleration(Rect1Point0d0A, HGF2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, Rect1A.CalculateAngularAcceleration(Rect1Point0d1A, HGF2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, Rect1A.CalculateAngularAcceleration(Rect1Point0d1A, HGF2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, Rect1A.CalculateAngularAcceleration(Rect1Point0d5A, HGF2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, Rect1A.CalculateAngularAcceleration(Rect1Point0d5A, HGF2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, Rect1A.CalculateAngularAcceleration(Rect1Point1d0A, HGF2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, Rect1A.CalculateAngularAcceleration(Rect1Point1d0A, HGF2DVector::BETA), MYEPSILON);

    }

//==================================================================================
// Extent calculation test
// GetExtent() const;
//==================================================================================
TEST_F (HGF2DRectangleTest, GetExtentTest)
    {

    ASSERT_DOUBLE_EQ(10.0, Rect1A.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(10.0, Rect1A.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(20.0, Rect1A.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(20.0, Rect1A.GetExtent().GetYMax());

    ASSERT_DOUBLE_EQ(10.0, IncludedRect1A.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(10.0, IncludedRect1A.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(15.0, IncludedRect1A.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(15.0, IncludedRect1A.GetExtent().GetYMax());

    ASSERT_DOUBLE_EQ(15.0, IncludedRect2.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(10.0, IncludedRect2.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(20.0, IncludedRect2.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(15.0, IncludedRect2.GetExtent().GetYMax());

    ASSERT_DOUBLE_EQ(15.0, IncludedRect3.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(15.0, IncludedRect3.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(20.0, IncludedRect3.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(20.0, IncludedRect3.GetExtent().GetYMax());

    ASSERT_DOUBLE_EQ(10.0, IncludedRect4.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(15.0, IncludedRect4.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(15.0, IncludedRect4.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(20.0, IncludedRect4.GetExtent().GetYMax());

    ASSERT_DOUBLE_EQ(12.0, IncludedRect5.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(12.0, IncludedRect5.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(18.0, IncludedRect5.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(18.0, IncludedRect5.GetExtent().GetYMax());

    ASSERT_DOUBLE_EQ(10.0, IncludedRect6.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(10.0, IncludedRect6.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(20.0, IncludedRect6.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(15.0, IncludedRect6.GetExtent().GetYMax());

    ASSERT_DOUBLE_EQ(10.0, IncludedRect7.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(10.0, IncludedRect7.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(15.0, IncludedRect7.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(20.0, IncludedRect7.GetExtent().GetYMax());

    ASSERT_DOUBLE_EQ(15.0, IncludedRect8.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(10.0, IncludedRect8.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(20.0, IncludedRect8.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(20.0, IncludedRect8.GetExtent().GetYMax());

    ASSERT_DOUBLE_EQ(10.0, IncludedRect9.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(15.0, IncludedRect9.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(20.0, IncludedRect9.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(20.0, IncludedRect9.GetExtent().GetYMax());

    }

//==================================================================================
// Move(const HGF2DDisplacement& pi_rDisplacement);
//==================================================================================
TEST_F (HGF2DRectangleTest, MoveTest)
    {

    HGF2DDisplacement Translation1(10.0, 10.0);
    HGF2DDisplacement Translation2(0.0, 10.0);
    HGF2DDisplacement Translation3(10.0, 0.0);
    HGF2DDisplacement Translation4(0.0, 0.0);
    HGF2DDisplacement Translation5(-10.0, 0.0);
    HGF2DDisplacement Translation6(0.0, -10.0);
    HGF2DDisplacement Translation7(-10.0, -10.0);
    HGF2DDisplacement Translation8(10.0, -10.0);

    HGF2DPosition MinPoint;
    HGF2DPosition MaxPoint;

    IncludedRect1A.Move(Translation1);
    IncludedRect1A.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(20.0, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(25.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(25.0, MaxPoint.GetY());

    IncludedRect2A.Move(Translation2);
    IncludedRect2A.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(15.0, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(20.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(25.0, MaxPoint.GetY());

    IncludedRect3A.Move(Translation3);
    IncludedRect3A.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(25.0, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(15.0, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(30.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, MaxPoint.GetY());
   
    IncludedRect4A.Move(Translation4);
    IncludedRect4A.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(10.0, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(15.0, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(15.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, MaxPoint.GetY());
   
    IncludedRect5A.Move(Translation5);
    IncludedRect5A.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(2.00, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(12.0, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(8.00, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(18.0, MaxPoint.GetY());
    
    IncludedRect6A.Move(Translation6);
    IncludedRect6A.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(10.0, MinPoint.GetX());
    ASSERT_NEAR(0.0, MinPoint.GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(20.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(5.00, MaxPoint.GetY());
    
    IncludedRect7A.Move(Translation7);
    IncludedRect7A.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_NEAR(0.0, MinPoint.GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, MinPoint.GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(5.00, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, MaxPoint.GetY());
    
    IncludedRect8A.Move(Translation8);
    IncludedRect8A.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(25.0, MinPoint.GetX());
    ASSERT_NEAR(0.0, MinPoint.GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, MaxPoint.GetY());

    }

//==================================================================================
// Scale(double pi_ScaleFactor, const HGF2DPosition& pi_rScaleOrigin)
// Scale(double pi_ScaleFactorX, double pi_ScaleFactorY, const HGF2DPosition& pi_rScaleOrigin)
//================================================================================== 
TEST_F (HGF2DRectangleTest, ScaleTest)
    {

    HGF2DPosition Origin(0.0, 0.0);
    HGF2DPosition MinPoint;
    HGF2DPosition MaxPoint;

    IncludedRect1A.Scale(2.0, Origin);
    IncludedRect1A.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(20.0, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(30.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(30.0, MaxPoint.GetY());
 
    IncludedRect2A.Scale(5.0, Origin);
    IncludedRect2A.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(75.00, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(50.00, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(100.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(75.00, MaxPoint.GetY());

    IncludedRect3A.Scale(-2.0, Origin);
    IncludedRect3A.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(-40.0, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(-40.0, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(-30.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(-30.0, MaxPoint.GetY());

    IncludedRect4A.Scale(2.0, HGF2DPosition(0.5, 0.5));
    IncludedRect4A.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(19.5, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(29.5, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(29.5, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(39.5, MaxPoint.GetY());

    IncludedRect5A.Scale(2.0, HGF2DPosition(-0.5, 0.5));
    IncludedRect5A.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(24.5, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(23.5, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(36.5, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(35.5, MaxPoint.GetY());

    IncludedRect1A.Scale(1.0, 2.0, Origin);
    IncludedRect1A.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(20.0, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(40.0, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(30.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(60.0, MaxPoint.GetY());

    IncludedRect2A.Scale(-1.0, 2.0, Origin);
    IncludedRect2A.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(-100.0, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(100.0, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(-75.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(150.0, MaxPoint.GetY());

    IncludedRect3A.Scale(1.0, -2.0, Origin);
    IncludedRect3A.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(-40.0, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(60.00, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(-30.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(80.00, MaxPoint.GetY());

    IncludedRect4A.Scale(0.5, 2.0, Origin);
    IncludedRect4A.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(9.750, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(59.00, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(14.75, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(79.00, MaxPoint.GetY());

    IncludedRect5A.Scale(0.5, 0.2, HGF2DPosition(0.5, 0.5));
    IncludedRect5A.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(12.5, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(5.10, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(18.5, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(7.50, MaxPoint.GetY());

    IncludedRect6A.Scale(1.0, 2.0, HGF2DPosition(-0.5, 0.5));
    IncludedRect6A.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(10.0, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(19.5, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(20.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(29.5, MaxPoint.GetY());

    }

//==================================================================================
// IsEmpty() test
// MakEmpty() test
//==================================================================================
TEST_F (HGF2DRectangleTest, EmptyTest)
    {

    HGF2DRectangle  MyOtherRect(Rect1A);
    MyOtherRect.MakeEmpty();
    ASSERT_TRUE(MyOtherRect.IsEmpty());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), Rect1A.GetShapeType());

    }

//==================================================================================
// Drop( HGF2DPositionCollection* po_pPoints, const HGFDistance& pi_rTolerance ) const
//==================================================================================
TEST_F (HGF2DRectangleTest, DropTest)
    {

    HGF2DPositionCollection Locations;

    Rect1A.Drop(&Locations, MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Locations[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, Locations[0].GetY());
    ASSERT_DOUBLE_EQ(10.0, Locations[1].GetX());
    ASSERT_DOUBLE_EQ(20.0, Locations[1].GetY());
    ASSERT_DOUBLE_EQ(20.0, Locations[2].GetX());
    ASSERT_DOUBLE_EQ(20.0, Locations[2].GetY());
    ASSERT_DOUBLE_EQ(20.0, Locations[3].GetX());
    ASSERT_DOUBLE_EQ(10.0, Locations[3].GetY());
    ASSERT_DOUBLE_EQ(10.0, Locations[4].GetX());
    ASSERT_DOUBLE_EQ(10.0, Locations[4].GetY());

    }

//==================================================================================
// IsPointIn()
// IsPointOn()
//==================================================================================
TEST_F (HGF2DRectangleTest, IsPointTest)
    {

    //IsPointIn
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(10.0, 10.0)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(15.0, 10.0)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(20.0, 10.0)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(20.0, 15.0)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(20.0, 20.0)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(15.0, 20.0)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(10.0, 20.0)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(10.0, 15.0)));

    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(100.0, 100.0)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(10.0-1.1*MYEPSILON, 10.0)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(10.0, 10.0-1.1*MYEPSILON)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(10.0-1.1*MYEPSILON, 15.0)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(10.0-1.1*MYEPSILON, 20.0)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(10.0, 20.0+1.1*MYEPSILON)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(15.0, 20.0+1.1*MYEPSILON)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(20.0, 20.0+1.1*MYEPSILON)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(20.0+1.1*MYEPSILON, 20.0)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(20.0+1.1*MYEPSILON, 15.0)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(20.0+1.1*MYEPSILON, 10.0)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(20.0, 10.0-1.1*MYEPSILON)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(15.0, 10.0-1.1*MYEPSILON)));

    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(10.0-0.9*MYEPSILON, 10.0)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(10.0, 10.0-0.9*MYEPSILON)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(10.0-0.9*MYEPSILON, 15.0)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(10.0-0.9*MYEPSILON, 20.0)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(10.0, 20.0+0.9*MYEPSILON)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(15.0, 20.0+0.9*MYEPSILON)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(20.0, 20.0+0.9*MYEPSILON)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(20.0+0.9*MYEPSILON, 20.0)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(20.0+0.9*MYEPSILON, 15.0)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(20.0+0.9*MYEPSILON, 10.0)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(20.0, 10.0-0.9*MYEPSILON)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(15.0, 10.0-0.9*MYEPSILON)));

    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(10.0+0.9*MYEPSILON, 15.0)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(15.0, 20.0-0.9*MYEPSILON)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(20.0-0.9*MYEPSILON, 15.0)));
    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(15.0, 10.0+0.9*MYEPSILON)));

    ASSERT_TRUE(Rect1A.IsPointIn(HGF2DPosition(10.0+1.1*MYEPSILON, 15.0)));
    ASSERT_TRUE(Rect1A.IsPointIn(HGF2DPosition(15.0, 20.0-1.1*MYEPSILON)));
    ASSERT_TRUE(Rect1A.IsPointIn(HGF2DPosition(20.0-1.1*MYEPSILON, 15.0)));
    ASSERT_TRUE(Rect1A.IsPointIn(HGF2DPosition(15.0, 10.0+1.1*MYEPSILON)));

    ASSERT_TRUE(Rect1A.IsPointIn(HGF2DPosition(12.0, 15.0)));
    ASSERT_TRUE(Rect1A.IsPointIn(HGF2DPosition(15.0, 18.0)));
    ASSERT_TRUE(Rect1A.IsPointIn(HGF2DPosition(18.0, 15.0)));
    ASSERT_TRUE(Rect1A.IsPointIn(HGF2DPosition(15.0, 12.0)));
    ASSERT_TRUE(Rect1A.IsPointIn(HGF2DPosition(15.0, 15.0)));

    ASSERT_FALSE(Rect1A.IsPointIn(HGF2DPosition(-15.0, -15.0)));

    // IsPointOn
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(15.0, 10.0)));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(20.0, 15.0)));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(15.0, 20.0)));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(10.0, 15.0)));

    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(100.0, 100.0)));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(10.0-1.1*MYEPSILON, 10.0)));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(10.0, 10.0-1.1*MYEPSILON)));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(10.0-1.1*MYEPSILON, 15.0)));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(10.0-1.1*MYEPSILON, 20.0)));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(10.0, 20.0+1.1*MYEPSILON)));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(15.0, 20.0+1.1*MYEPSILON)));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(20.0, 20.0+1.1*MYEPSILON)));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(20.0+1.1*MYEPSILON, 20.0)));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(20.0+1.1*MYEPSILON, 15.0)));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(20.0+1.1*MYEPSILON, 10.0)));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(20.0, 10.0-1.1*MYEPSILON)));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(15.0, 10.0-1.1*MYEPSILON)));

    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(10.0-0.9*MYEPSILON, 10.0)));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(10.0, 10.0-0.9*MYEPSILON)));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(10.0-0.9*MYEPSILON, 15.0)));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(10.0-0.9*MYEPSILON, 20.0)));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(10.0, 20.0+0.9*MYEPSILON)));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(15.0, 20.0+0.9*MYEPSILON)));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(20.0, 20.0+0.9*MYEPSILON)));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(20.0+0.9*MYEPSILON, 20.0)));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(20.0+0.9*MYEPSILON, 15.0)));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(20.0+0.9*MYEPSILON, 10.0)));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(20.0, 10.0-0.9*MYEPSILON)));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(15.0, 10.0-0.9*MYEPSILON)));

    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(10.0+0.9*MYEPSILON, 15.0)));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(15.0, 20.0-0.9*MYEPSILON)));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(20.0-0.9*MYEPSILON, 15.0)));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(15.0, 10.0+0.9*MYEPSILON)));

    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(10.0+1.1*MYEPSILON, 15.0)));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(15.0, 20.0-1.1*MYEPSILON)));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(20.0-1.1*MYEPSILON, 15.0)));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(15.0, 10.0+1.1*MYEPSILON)));

    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(12.0, 15.0)));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(15.0, 18.0)));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(18.0, 15.0)));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(15.0, 12.0)));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(15.0, 15.0)));

    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(-15.0, -15.0)));

    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(10.0, 10.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(15.0, 10.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(20.0, 10.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(20.0, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(20.0, 20.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(15.0, 20.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(10.0, 20.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(10.0, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(100.0, 100.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(10.0-1.1*MYEPSILON, 10.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(10.0, 10.0-1.1*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(10.0-1.1*MYEPSILON, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(10.0-1.1*MYEPSILON, 20.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(10.0, 20.0+1.1*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(15.0, 20.0+1.1*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(20.0, 20.0+1.1*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(20.0+1.1*MYEPSILON, 20.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(20.0+1.1*MYEPSILON, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(20.0+1.1*MYEPSILON, 10.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(20.0, 10.0-1.1*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(15.0, 10.0-1.1*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(10.0-0.9*MYEPSILON, 10.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(10.0, 10.0-0.9*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(10.0-0.9*MYEPSILON, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(10.0-0.9*MYEPSILON, 20.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(10.0, 20.0+0.9*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(15.0, 20.0+0.9*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(20.0, 20.0+0.9*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(20.0+0.9*MYEPSILON, 20.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(20.0+0.9*MYEPSILON, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(20.0+0.9*MYEPSILON, 10.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(20.0, 10.0-0.9*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(15.0, 10.0-0.9*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(10.0+0.9*MYEPSILON, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(15.0, 20.0-0.9*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(20.0-0.9*MYEPSILON, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1A.IsPointOn(HGF2DPosition(15.0, 10.0+0.9*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(10.0+1.1*MYEPSILON, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(15.0, 20.0-1.1*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(20.0-1.1*MYEPSILON, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(15.0, 10.0+1.1*MYEPSILON), HGF2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(12.0, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(15.0, 18.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(18.0, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(15.0, 12.0), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(15.0, 15.0), HGF2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_FALSE(Rect1A.IsPointOn(HGF2DPosition(-15.0, -15.0), HGF2DVector::EXCLUDE_EXTREMITIES));

    }

//==================================================================================
// UnifyShape( const HGF2DShape& pi_rShape ) const
//==================================================================================
TEST_F (HGF2DRectangleTest, UnifyShapeTest)
    {

    HFCPtr<HGF2DShape>     pResultShape1 = Rect1A.UnifyShape(NorthContiguousRectA);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape1->GetShapeType());
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(20.0, 30.0)));

    HFCPtr<HGF2DShape>     pResultShape2 = Rect1A.UnifyShape(EastContiguousRectA);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape2->GetShapeType());
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(30.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape3 = Rect1A.UnifyShape(WestContiguousRectA);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape3->GetShapeType());
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(0.0, 10.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(20.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape4 = Rect1A.UnifyShape(SouthContiguousRectA);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape4->GetShapeType());
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(10.0, 0.0)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(20.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape5 = Rect1A.UnifyShape(VerticalFitRectA);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape5->GetShapeType());
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(25.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape6 = Rect1A.UnifyShape(HorizontalFitRectA);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape6->GetShapeType());
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(20.0, 25.0)));

    HFCPtr<HGF2DShape>     pResultShape7 = Rect1A.UnifyShape(DisjointRectA);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape7->GetShapeType());

    HFCPtr<HGF2DShape>     pResultShape8 = Rect1A.UnifyShape(MiscRect1A);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape8->GetShapeType());

    HFCPtr<HGF2DShape>     pResultShape9 = Rect1A.UnifyShape(EnglobRect1A);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape9->GetShapeType());
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DPosition(30.0, 30.0)));

    HFCPtr<HGF2DShape>     pResultShape10 = Rect1A.UnifyShape(EnglobRect2A);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape10->GetShapeType());
    ASSERT_TRUE(pResultShape10->IsPointOn(HGF2DPosition(0.0, 0.0)));
    ASSERT_TRUE(pResultShape10->IsPointOn(HGF2DPosition(30.0, 30.0)));

    HFCPtr<HGF2DShape>     pResultShape11 = Rect1A.UnifyShape(EnglobRect3A);
    ASSERT_EQ(HGF2DRectangle::CLASS_ID, pResultShape11->GetShapeType());
    ASSERT_TRUE(pResultShape11->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape11->IsPointOn(HGF2DPosition(20.0, 30.0)));

    HFCPtr<HGF2DShape>     pResultShape12 = Rect1A.UnifyShape(IncludedRect1A);
    ASSERT_EQ(HGF2DRectangle::CLASS_ID, pResultShape12->GetShapeType());
    ASSERT_TRUE(pResultShape12->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape12->IsPointOn(HGF2DPosition(20.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape13 = Rect1A.UnifyShape(IncludedRect2A);
    ASSERT_EQ(HGF2DRectangle::CLASS_ID, pResultShape13->GetShapeType());
    ASSERT_TRUE(pResultShape13->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape13->IsPointOn(HGF2DPosition(20.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape14 = Rect1A.UnifyShape(IncludedRect3A);
    ASSERT_EQ(HGF2DRectangle::CLASS_ID, pResultShape14->GetShapeType());
    ASSERT_TRUE(pResultShape14->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape14->IsPointOn(HGF2DPosition(20.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape15 = Rect1A.UnifyShape(IncludedRect4A);
    ASSERT_EQ(HGF2DRectangle::CLASS_ID, pResultShape15->GetShapeType());
    ASSERT_TRUE(pResultShape15->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape15->IsPointOn(HGF2DPosition(20.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape16 = Rect1A.UnifyShape(IncludedRect5A);
    ASSERT_EQ(HGF2DRectangle::CLASS_ID, pResultShape16->GetShapeType());
    ASSERT_TRUE(pResultShape16->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape16->IsPointOn(HGF2DPosition(20.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape17 = Rect1A.UnifyShape(IncludedRect6A);
    ASSERT_EQ(HGF2DRectangle::CLASS_ID, pResultShape17->GetShapeType());
    ASSERT_TRUE(pResultShape17->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape17->IsPointOn(HGF2DPosition(20.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape18 = Rect1A.UnifyShape(IncludedRect7A);
    ASSERT_EQ(HGF2DRectangle::CLASS_ID, pResultShape18->GetShapeType());
    ASSERT_TRUE(pResultShape18->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape18->IsPointOn(HGF2DPosition(20.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape19 = Rect1A.UnifyShape(IncludedRect8A);
    ASSERT_EQ(HGF2DRectangle::CLASS_ID, pResultShape19->GetShapeType());
    ASSERT_TRUE(pResultShape19->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape19->IsPointOn(HGF2DPosition(20.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape20 = Rect1A.UnifyShape(IncludedRect9A);
    ASSERT_EQ(HGF2DRectangle::CLASS_ID, pResultShape20->GetShapeType());
    ASSERT_TRUE(pResultShape20->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape20->IsPointOn(HGF2DPosition(20.0, 20.0)));

    }



//==================================================================================
// IntersectShape( const HGF2DShape& pi_rShape ) const
//==================================================================================
TEST_F (HGF2DRectangleTest, IntersectShapeTest)
    {

    HFCPtr<HGF2DShape>     pResultShape1 = Rect1A.IntersectShape(NorthContiguousRectA);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape1->GetShapeType());
    ASSERT_TRUE(pResultShape1->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape2 = Rect1A.IntersectShape(EastContiguousRectA);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape2->GetShapeType());
    ASSERT_TRUE(pResultShape2->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape3 = Rect1A.IntersectShape(WestContiguousRectA);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape3->GetShapeType());
    ASSERT_TRUE(pResultShape3->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape4 = Rect1A.IntersectShape(SouthContiguousRectA);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape4->GetShapeType());
    ASSERT_TRUE(pResultShape4->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape5 = Rect1A.IntersectShape(VerticalFitRectA);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape5->GetShapeType());
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(15.0, 10.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(20.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape6 = Rect1A.IntersectShape(HorizontalFitRectA);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape6->GetShapeType());
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(10.0, 15.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(20.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape7 = Rect1A.IntersectShape(DisjointRectA);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape7->GetShapeType());
    ASSERT_TRUE(pResultShape7->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape8 = Rect1A.IntersectShape(MiscRect1A);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape8->GetShapeType());
    ASSERT_TRUE(pResultShape8->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape8->IsPointOn(HGF2DPosition(15.0, 15.0)));

    HFCPtr<HGF2DShape>     pResultShape9 = Rect1A.IntersectShape(EnglobRect1A);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape9->GetShapeType());
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DPosition(20.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape10 = Rect1A.IntersectShape(EnglobRect2A);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape10->GetShapeType());
    ASSERT_TRUE(pResultShape10->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape10->IsPointOn(HGF2DPosition(20.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape11 = Rect1A.IntersectShape(EnglobRect3A);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape11->GetShapeType());
    ASSERT_TRUE(pResultShape11->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape11->IsPointOn(HGF2DPosition(20.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape12 = Rect1A.IntersectShape(IncludedRect1A);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape12->GetShapeType());
    ASSERT_TRUE(pResultShape12->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape12->IsPointOn(HGF2DPosition(15.0, 15.0)));

    HFCPtr<HGF2DShape>     pResultShape13 = Rect1A.IntersectShape(IncludedRect2A);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape13->GetShapeType());
    ASSERT_TRUE(pResultShape13->IsPointOn(HGF2DPosition(15.0, 10.0)));
    ASSERT_TRUE(pResultShape13->IsPointOn(HGF2DPosition(20.0, 15.0)));

    HFCPtr<HGF2DShape>     pResultShape14 = Rect1A.IntersectShape(IncludedRect3A);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape14->GetShapeType());
    ASSERT_TRUE(pResultShape14->IsPointOn(HGF2DPosition(15.0, 15.0)));
    ASSERT_TRUE(pResultShape14->IsPointOn(HGF2DPosition(20.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape15 = Rect1A.IntersectShape(IncludedRect4A);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape15->GetShapeType());
    ASSERT_TRUE(pResultShape15->IsPointOn(HGF2DPosition(10.0, 15.0)));
    ASSERT_TRUE(pResultShape15->IsPointOn(HGF2DPosition(15.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape16 = Rect1A.IntersectShape(IncludedRect5A);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape16->GetShapeType());
    ASSERT_TRUE(pResultShape16->IsPointOn(HGF2DPosition(12.0, 12.0)));
    ASSERT_TRUE(pResultShape16->IsPointOn(HGF2DPosition(18.0, 18.0)));

    HFCPtr<HGF2DShape>     pResultShape17 = Rect1A.IntersectShape(IncludedRect6A);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape17->GetShapeType());
    ASSERT_TRUE(pResultShape17->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape17->IsPointOn(HGF2DPosition(20.0, 15.0)));

    HFCPtr<HGF2DShape>     pResultShape18 = Rect1A.IntersectShape(IncludedRect7A);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape18->GetShapeType());
    ASSERT_TRUE(pResultShape18->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape18->IsPointOn(HGF2DPosition(15.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape19 = Rect1A.IntersectShape(IncludedRect8A);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape19->GetShapeType());
    ASSERT_TRUE(pResultShape19->IsPointOn(HGF2DPosition(15.0, 10.0)));
    ASSERT_TRUE(pResultShape19->IsPointOn(HGF2DPosition(20.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape20 = Rect1A.IntersectShape(IncludedRect9A);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape20->GetShapeType());
    ASSERT_TRUE(pResultShape20->IsPointOn(HGF2DPosition(10.0, 15.0)));
    ASSERT_TRUE(pResultShape20->IsPointOn(HGF2DPosition(20.0, 20.0)));

    }



//==================================================================================
// DifferentiateShape( const HGF2DShape& pi_rShape ) const
//==================================================================================
TEST_F (HGF2DRectangleTest, DifferentiateShape)
    {

    // Spatial oriented operations DIFF
    HFCPtr<HGF2DShape>     pResultShape1 = Rect1A.DifferentiateShape(NorthContiguousRectA);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape1->GetShapeType());
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(20.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape2 = Rect1A.DifferentiateShape(EastContiguousRectA);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape2->GetShapeType());
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(20.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape3 = Rect1A.DifferentiateShape(WestContiguousRectA);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape3->GetShapeType());
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(20.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape4 = Rect1A.DifferentiateShape(SouthContiguousRectA);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape4->GetShapeType());
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(20.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape5 = Rect1A.DifferentiateShape(VerticalFitRectA);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape5->GetShapeType());
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(15.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape6 = Rect1A.DifferentiateShape(HorizontalFitRectA);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape6->GetShapeType());
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(20.0, 15.0)));

    HFCPtr<HGF2DShape>     pResultShape7 = Rect1A.DifferentiateShape(DisjointRectA);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape7->GetShapeType());
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DPosition(20.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape8 = Rect1A.DifferentiateShape(MiscRect1A);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape8->GetShapeType());

    HFCPtr<HGF2DShape>     pResultShape9 = Rect1A.DifferentiateShape(EnglobRect1A);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape9->GetShapeType());
    ASSERT_TRUE(pResultShape9->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape10 = Rect1A.DifferentiateShape(EnglobRect2A);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape10->GetShapeType());
    ASSERT_TRUE(pResultShape10->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape11 = Rect1A.DifferentiateShape(EnglobRect3A);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape11->GetShapeType());
    ASSERT_TRUE(pResultShape11->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape12 = Rect1A.DifferentiateShape(IncludedRect1A);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape12->GetShapeType());

    HFCPtr<HGF2DShape>     pResultShape13 = Rect1A.DifferentiateShape(IncludedRect2A);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape13->GetShapeType());

    HFCPtr<HGF2DShape>     pResultShape14 = Rect1A.DifferentiateShape(IncludedRect3A);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape14->GetShapeType());

    HFCPtr<HGF2DShape>     pResultShape15 = Rect1A.DifferentiateShape(IncludedRect4A);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape15->GetShapeType());

    HFCPtr<HGF2DShape>     pResultShape16 = Rect1A.DifferentiateShape(IncludedRect5A);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape16->GetShapeType());

    HFCPtr<HGF2DShape>     pResultShape17 = Rect1A.DifferentiateShape(IncludedRect6A);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape17->GetShapeType());
    ASSERT_TRUE(pResultShape17->IsPointOn(HGF2DPosition(10.0, 15.0)));
    ASSERT_TRUE(pResultShape17->IsPointOn(HGF2DPosition(20.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape18 = Rect1A.DifferentiateShape(IncludedRect7A);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape18->GetShapeType());
    ASSERT_TRUE(pResultShape18->IsPointOn(HGF2DPosition(15.0, 10.0)));
    ASSERT_TRUE(pResultShape18->IsPointOn(HGF2DPosition(20.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape19 = Rect1A.DifferentiateShape(IncludedRect8A);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape19->GetShapeType());
    ASSERT_TRUE(pResultShape19->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape19->IsPointOn(HGF2DPosition(15.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape20 = Rect1A.DifferentiateShape(IncludedRect9A);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape20->GetShapeType());
    ASSERT_TRUE(pResultShape20->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape20->IsPointOn(HGF2DPosition(20.0, 15.0)));

    }



//==================================================================================
// DifferentiateFromShape( const HGF2DShape& pi_rShape ) const
//==================================================================================
TEST_F (HGF2DRectangleTest, DifferentiateFromShape)
    {
    
    // Spatial oriented operations DIFF
    HFCPtr<HGF2DShape>     pResultShape1 = Rect1A.DifferentiateFromShape(NorthContiguousRectA);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape1->GetShapeType());
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(20.0, 30.0)));

    HFCPtr<HGF2DShape>     pResultShape2 = Rect1A.DifferentiateFromShape(EastContiguousRectA);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape2->GetShapeType());
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(30.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape3 = Rect1A.DifferentiateFromShape(WestContiguousRectA);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape3->GetShapeType());
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(0.0, 10.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(10.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape4 = Rect1A.DifferentiateFromShape(SouthContiguousRectA);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape4->GetShapeType());
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(10.0, 0.0)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(20.0, 10.0)));

    HFCPtr<HGF2DShape>     pResultShape5 = Rect1A.DifferentiateFromShape(VerticalFitRectA);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape5->GetShapeType());
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(25.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape6 = Rect1A.DifferentiateFromShape(HorizontalFitRectA);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape6->GetShapeType());
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(20.0, 25.0)));

    HFCPtr<HGF2DShape>     pResultShape7 = Rect1A.DifferentiateFromShape(DisjointRectA);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape7->GetShapeType());
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DPosition(-10.0, -10.0)));
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DPosition(0.0, 0.0)));

    HFCPtr<HGF2DShape>     pResultShape8 = Rect1A.DifferentiateFromShape(MiscRect1A);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape8->GetShapeType());

    HFCPtr<HGF2DShape>     pResultShape9 = Rect1A.DifferentiateFromShape(EnglobRect1A);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape9->GetShapeType());

    HFCPtr<HGF2DShape>     pResultShape10 = Rect1A.DifferentiateFromShape(EnglobRect2A);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape10->GetShapeType());

    HFCPtr<HGF2DShape>     pResultShape11 = Rect1A.DifferentiateFromShape(EnglobRect3A);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HGF2DRectangle::CLASS_ID), pResultShape11->GetShapeType());
    ASSERT_TRUE(pResultShape11->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape11->IsPointOn(HGF2DPosition(20.0, 30.0)));

    HFCPtr<HGF2DShape>     pResultShape12 = Rect1A.DifferentiateFromShape(IncludedRect1A);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape12->GetShapeType());
    ASSERT_TRUE(pResultShape12->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape13 = Rect1A.DifferentiateFromShape(IncludedRect2A);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape13->GetShapeType());
    ASSERT_TRUE(pResultShape13->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape14 = Rect1A.DifferentiateFromShape(IncludedRect3A);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape14->GetShapeType());
    ASSERT_TRUE(pResultShape14->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape15 = Rect1A.DifferentiateFromShape(IncludedRect4A);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape15->GetShapeType());
    ASSERT_TRUE(pResultShape15->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape16 = Rect1A.DifferentiateFromShape(IncludedRect5A);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape16->GetShapeType());
    ASSERT_TRUE(pResultShape16->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape17 = Rect1A.DifferentiateFromShape(IncludedRect6A);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape17->GetShapeType());
    ASSERT_TRUE(pResultShape17->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape18 = Rect1A.DifferentiateFromShape(IncludedRect7A);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape18->GetShapeType());
    ASSERT_TRUE(pResultShape18->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape19 = Rect1A.DifferentiateFromShape(IncludedRect8A);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape19->GetShapeType());
    ASSERT_TRUE(pResultShape19->IsEmpty());

    HFCPtr<HGF2DShape>     pResultShape20 = Rect1A.DifferentiateFromShape(IncludedRect9A);
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape20->GetShapeType());
    ASSERT_TRUE(pResultShape20->IsEmpty());

    }

//==================================================================================
// Test which failed on May 4, 2001
// CalculateSpatialPositionOf
//==================================================================================
TEST_F (HGF2DRectangleTest, CalculateSpatialPositionOf)
    {
    
    HGF2DRectangle MyRectangle(0.0, 0.0, 10.0, 10.0);

    HGF2DSegment MyTestSegment1(HGF2DPosition(1.00, 10.0), HGF2DPosition(9.00, 10.0));
    HGF2DSegment MyTestSegment2(HGF2DPosition(1.00, 0.00), HGF2DPosition(9.00, 0.00));
    HGF2DSegment MyTestSegment3(HGF2DPosition(0.00, 1.00), HGF2DPosition(0.00, 9.00));
    HGF2DSegment MyTestSegment4(HGF2DPosition(10.0, 1.00), HGF2DPosition(10.0, 9.00));

    HGF2DSegment MyTestSegment1A(HGF2DPosition(1.0, 10.0 - MYEPSILON), HGF2DPosition(9.0, 10.0));
    HGF2DSegment MyTestSegment2A(HGF2DPosition(1.0, 0.0 - MYEPSILON), HGF2DPosition(9.0, 0.0));
    HGF2DSegment MyTestSegment3A(HGF2DPosition(0.0 - MYEPSILON, 1.0), HGF2DPosition(0.0, 9.0));
    HGF2DSegment MyTestSegment4A(HGF2DPosition(10.0 - MYEPSILON, 1.0), HGF2DPosition(10.0, 9.0));

    HGF2DSegment MyTestSegment1B(HGF2DPosition(1.0, 10.0 + MYEPSILON), HGF2DPosition(9.0, 10.0));
    HGF2DSegment MyTestSegment2B(HGF2DPosition(1.0, 0.0 + MYEPSILON), HGF2DPosition(9.0, 0.0));
    HGF2DSegment MyTestSegment3B(HGF2DPosition(0.0 + MYEPSILON, 1.0), HGF2DPosition(0.0, 9.0));
    HGF2DSegment MyTestSegment4B(HGF2DPosition(10.0 + MYEPSILON, 1.0), HGF2DPosition(10.0, 9.0));

    HGF2DSegment MyTestSegment1C(HGF2DPosition(1.0, 10.0), HGF2DPosition(9.0, 10.0 - MYEPSILON));
    HGF2DSegment MyTestSegment2C(HGF2DPosition(1.0, 0.0), HGF2DPosition(9.0, 0.0 - MYEPSILON));
    HGF2DSegment MyTestSegment3C(HGF2DPosition(0.0, 1.0), HGF2DPosition(0.0 - MYEPSILON, 9.0));
    HGF2DSegment MyTestSegment4C(HGF2DPosition(10.0, 1.0), HGF2DPosition(10.0 - MYEPSILON, 9.0));

    HGF2DSegment MyTestSegment1D(HGF2DPosition(1.0, 10.0), HGF2DPosition(9.0, 10.0 + MYEPSILON));
    HGF2DSegment MyTestSegment2D(HGF2DPosition(1.0, 0.0), HGF2DPosition(9.0, 0.0 + MYEPSILON));
    HGF2DSegment MyTestSegment3D(HGF2DPosition(0.0, 1.0), HGF2DPosition(0.0 + MYEPSILON, 9.0));
    HGF2DSegment MyTestSegment4D(HGF2DPosition(10.0, 1.0), HGF2DPosition(10.0 + MYEPSILON, 9.0));

    HGF2DSegment MyTestSegment1E(HGF2DPosition(1.0, 10.0 - MYEPSILON), HGF2DPosition(9.0, 10.0 - MYEPSILON));
    HGF2DSegment MyTestSegment2E(HGF2DPosition(1.0, 0.0 - MYEPSILON), HGF2DPosition(9.0, 0.0 - MYEPSILON));
    HGF2DSegment MyTestSegment3E(HGF2DPosition(0.0 - MYEPSILON, 1.0), HGF2DPosition(0.0 - MYEPSILON, 9.0));
    HGF2DSegment MyTestSegment4E(HGF2DPosition(10.0 - MYEPSILON, 1.0), HGF2DPosition(10.0 - MYEPSILON, 9.0));

    HGF2DSegment MyTestSegment1F(HGF2DPosition(1.0, 10.0 + MYEPSILON), HGF2DPosition(9.0, 10.0 - MYEPSILON));
    HGF2DSegment MyTestSegment2F(HGF2DPosition(1.0, 0.0 + MYEPSILON), HGF2DPosition(9.0, 0.0 - MYEPSILON));
    HGF2DSegment MyTestSegment3F(HGF2DPosition(0.0 + MYEPSILON, 1.0), HGF2DPosition(0.0 - MYEPSILON, 9.0));
    HGF2DSegment MyTestSegment4F(HGF2DPosition(10.0 + MYEPSILON, 1.0), HGF2DPosition(10.0 - MYEPSILON, 9.0));

    HGF2DSegment MyTestSegment1G(HGF2DPosition(1.0, 10.0 - MYEPSILON), HGF2DPosition(9.0, 10.0 + MYEPSILON));
    HGF2DSegment MyTestSegment2G(HGF2DPosition(1.0, 0.0 - MYEPSILON), HGF2DPosition(9.0, 0.0 + MYEPSILON));
    HGF2DSegment MyTestSegment3G(HGF2DPosition(0.0 - MYEPSILON, 1.0), HGF2DPosition(0.0 + MYEPSILON, 9.0));
    HGF2DSegment MyTestSegment4G(HGF2DPosition(10.0 - MYEPSILON, 1.0), HGF2DPosition(10.0 + MYEPSILON, 9.0));

    HGF2DSegment MyTestSegment1H(HGF2DPosition(1.0, 10.0 + MYEPSILON), HGF2DPosition(9.0, 10.0 + MYEPSILON));
    HGF2DSegment MyTestSegment2H(HGF2DPosition(1.0, 0.0 + MYEPSILON), HGF2DPosition(9.0, 0.0 + MYEPSILON));
    HGF2DSegment MyTestSegment3H(HGF2DPosition(0.0 + MYEPSILON, 1.0), HGF2DPosition(0.0 + MYEPSILON, 9.0));
    HGF2DSegment MyTestSegment4H(HGF2DPosition(10.0 + MYEPSILON, 1.0), HGF2DPosition(10.0 + MYEPSILON, 9.0));

    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment1));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment2));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment3));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment4));

    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment1A));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment2A));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment3A));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment4A));

    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment1B));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment2B));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment3B));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment4B));

    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment1C));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment2C));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment3C));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment4C));

    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment1D));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment2D));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment3D));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment4D));

    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment1E));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment2E));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment3E));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment4E));

    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment1F));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment2F));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment3F));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment4F));

    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment1G));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment2G));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment3G));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment4G));

    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment1H));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment2H));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment3H));
    ASSERT_EQ(HGF2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment4H));
   
    }

//==================================================================================
// FailedTestWithIntersect
//==================================================================================
TEST_F (HGF2DRectangleTest, FailedTestWithIntersect)
    {
   
    HGF2DRectangle MyRectangle(0.0, 0.0, 776.0, 530.0);

    HGF2DPolySegment  AddPolySegment1;
    AddPolySegment1.AppendPoint(HGF2DPosition(173881.594315156690, 31262.7623784915300 ));
    AddPolySegment1.AppendPoint(HGF2DPosition(-672196.87396625290, -119580.92306234955 ));
    AddPolySegment1.AppendPoint(HGF2DPosition(-207684.53954652144, -37058.254313532634 ));
    AddPolySegment1.AppendPoint(HGF2DPosition(403111.141213428460, 72700.3005556127460 ));
    AddPolySegment1.AppendPoint(HGF2DPosition(173881.594315156690, 31262.7623784915300 ));


    HGF2DPositionCollection     crossPoints;
    MyRectangle.Intersect (AddPolySegment1, &crossPoints);

    }


