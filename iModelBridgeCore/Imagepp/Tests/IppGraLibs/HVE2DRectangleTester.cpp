//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HVE2DRectangleTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "EnvironnementTest.h"
#include "HVE2DRectangleTester.h"

//==================================================================================
// Rectangle Construction tests
//==================================================================================
TEST_F (HVE2DRectangleTest, ConstructionTest)
    {

    // Default Constructor
    HVE2DRectangle    ARect1;

    // Constructor with a coordinate system
    HVE2DRectangle    ARect2(pWorld);
    ASSERT_EQ(pWorld, ARect2.GetCoordSys());

    //Constructor with Double
    HVE2DRectangle    ARect3(10.0, 10.1, 20.0, 20.1, pWorld);
    ASSERT_TRUE(ARect3.IsPointOn(HGF2DLocation(10.0, 10.1, pWorld)));
    ASSERT_TRUE(ARect3.IsPointOn(HGF2DLocation(20.0, 10.1, pWorld)));
    ASSERT_TRUE(ARect3.IsPointOn(HGF2DLocation(10.0, 20.1, pWorld)));
    ASSERT_TRUE(ARect3.IsPointOn(HGF2DLocation(20.0, 20.1, pWorld)));

    //Constructor with Location
    HVE2DRectangle    ARect4(HGF2DLocation(10.0, 10.1, pWorld), HGF2DLocation(20.0, 20.1, pWorld));
    ASSERT_TRUE(ARect4.IsPointOn(HGF2DLocation(10.0, 10.1, pWorld)));
    ASSERT_TRUE(ARect4.IsPointOn(HGF2DLocation(20.0, 10.1, pWorld)));
    ASSERT_TRUE(ARect4.IsPointOn(HGF2DLocation(10.0, 20.1, pWorld)));
    ASSERT_TRUE(ARect4.IsPointOn(HGF2DLocation(20.0, 20.1, pWorld)));

    //Constructor with Extent
    HVE2DRectangle    ARect5(HGF2DExtent(10.0, 10.1, 20.0, 20.1, pWorld));
    ASSERT_TRUE(ARect5.IsPointOn(HGF2DLocation(10.0, 10.1, pWorld)));
    ASSERT_TRUE(ARect5.IsPointOn(HGF2DLocation(20.0, 10.1, pWorld)));
    ASSERT_TRUE(ARect5.IsPointOn(HGF2DLocation(10.0, 20.1, pWorld)));
    ASSERT_TRUE(ARect5.IsPointOn(HGF2DLocation(20.0, 20.1, pWorld)));

    // Copy Constructor test
    HVE2DRectangle    ARect6(ARect5);
    ASSERT_TRUE(ARect6.IsPointOn(HGF2DLocation(10.0, 10.1, pWorld)));
    ASSERT_TRUE(ARect6.IsPointOn(HGF2DLocation(20.0, 10.1, pWorld)));
    ASSERT_TRUE(ARect6.IsPointOn(HGF2DLocation(10.0, 20.1, pWorld)));
    ASSERT_TRUE(ARect6.IsPointOn(HGF2DLocation(20.0, 20.1, pWorld)));

    }

//==================================================================================
// operator= test
// operator=(const HVE2DRectangle& pi_rObj);
//==================================================================================
TEST_F (HVE2DRectangleTest, OperatorTest)
    {
    
    HVE2DRectangle    ARect1(pSys1);
    HVE2DRectangle    ARect2(10.0, 10.1, 20.0, 20.1, pWorld);

    ARect1 = ARect2;

    ASSERT_EQ(pWorld, ARect1.GetCoordSys());
    ASSERT_TRUE(ARect1.IsPointOn(HGF2DLocation(10.0, 10.1, pWorld)));
    ASSERT_TRUE(ARect1.IsPointOn(HGF2DLocation(20.0, 10.1, pWorld)));
    ASSERT_TRUE(ARect1.IsPointOn(HGF2DLocation(10.0, 20.1, pWorld)));
    ASSERT_TRUE(ARect1.IsPointOn(HGF2DLocation(20.0, 20.1, pWorld)));

    }

//==================================================================================
//  SetRectangle(const HGF2DLocation& pi_rFirstPoint, const HGF2DLocation& pi_rSecondPoint);
//  SetRectangle(double pi_XMin ,double pi_YMin, double pi_XMax, double pi_YMax);
//  GetRectangle(HGF2DLocation* pi_pMinPoint, HGF2DLocation* pi_pMaxPoint) const;
//  GetRectangle(double* po_pXMin, double* po_pYMin, double* po_pXMax, double* po_pYMax) const;
//==================================================================================
TEST_F (HVE2DRectangleTest, GetSetRectangleTest)
    {

    HVE2DRectangle    ARect1(pWorld);
    ARect1.SetRectangle(10.0, 10.1, 20.0, 20.1);
    
    ASSERT_EQ(pWorld, ARect1.GetCoordSys());
    ASSERT_TRUE(ARect1.IsPointOn(HGF2DLocation(10.0, 10.1, pWorld)));
    ASSERT_TRUE(ARect1.IsPointOn(HGF2DLocation(20.0, 10.1, pWorld)));
    ASSERT_TRUE(ARect1.IsPointOn(HGF2DLocation(10.0, 20.1, pWorld)));
    ASSERT_TRUE(ARect1.IsPointOn(HGF2DLocation(20.0, 20.1, pWorld)));   
    
    HVE2DRectangle    ARect2(pWorld);
    ARect2.SetRectangle(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(20.0, 20.1, pWorld));
    
    ASSERT_EQ(pWorld, ARect2.GetCoordSys());
    ASSERT_TRUE(ARect2.IsPointOn(HGF2DLocation(10.0, 10.1, pWorld)));
    ASSERT_TRUE(ARect2.IsPointOn(HGF2DLocation(20.0, 10.1, pWorld)));
    ASSERT_TRUE(ARect2.IsPointOn(HGF2DLocation(10.0, 20.1, pWorld)));
    ASSERT_TRUE(ARect2.IsPointOn(HGF2DLocation(20.0, 20.1, pWorld)));  

    double Xmin;
    double Xmax;
    double Ymin;
    double Ymax;

    ARect2.GetRectangle(&Xmin, &Ymin, &Xmax, &Ymax);
    ASSERT_DOUBLE_EQ(10.0, Xmin);
    ASSERT_DOUBLE_EQ(10.0, Ymin);
    ASSERT_DOUBLE_EQ(20.0, Xmax);
    ASSERT_DOUBLE_EQ(20.1, Ymax);
    
    HGF2DLocation FirstLocation;
    HGF2DLocation SecondLocation;

    ARect2.GetRectangle(&FirstLocation, &SecondLocation);
    ASSERT_DOUBLE_EQ(10.0, FirstLocation.GetX());
    ASSERT_DOUBLE_EQ(10.0, FirstLocation.GetY());
    ASSERT_DOUBLE_EQ(20.0, SecondLocation.GetX());
    ASSERT_DOUBLE_EQ(20.1, SecondLocation.GetY());

    }

//==================================================================================
//  Overlaps(const HVE2DShape& pi_rShape) const;
//==================================================================================
TEST_F (HVE2DRectangleTest, OverlapsTest)
    {

    ASSERT_TRUE(Rect1.Overlaps(Rect1));
    ASSERT_FALSE(NETipRect.Overlaps(NWTipRect));

    }

//==================================================================================
// GetLinear( HVE2DSimpleShape::RotationDirection pi_DirectionDesired ) const
// AllocateLinear( HVE2DSimpleShape::RotationDirection pi_DirectionDesired ) const
//==================================================================================
TEST_F (HVE2DRectangleTest, GetLinearTest)
    {

    HVE2DComplexLinear  MyLinearOfRect1(Rect1.GetLinear());

    // verify that there are 4 linears
    ASSERT_EQ(4, MyLinearOfRect1.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRect1.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRect1.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRect1.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRect1.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRect1.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRect1.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRect1.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRect1.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRect1.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRect1.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRect1.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRect1.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRect1.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRect1.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRect1.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRect1.GetLinear(3).GetEndPoint().GetY());

    HVE2DComplexLinear  MyLinearOfRectCW(Rect1.GetLinear(HVE2DSimpleShape::CW));
    ASSERT_EQ(4, MyLinearOfRectCW.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRectCW.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRectCW.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRectCW.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRectCW.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRectCW.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRectCW.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRectCW.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRectCW.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRectCW.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRectCW.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRectCW.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRectCW.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRectCW.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRectCW.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRectCW.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRectCW.GetLinear(3).GetEndPoint().GetY());

    HVE2DComplexLinear  MyLinearOfRectCCW(Rect1.GetLinear(HVE2DSimpleShape::CCW));
    ASSERT_EQ(4, MyLinearOfRectCCW.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRectCCW.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRectCCW.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRectCCW.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRectCCW.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRectCCW.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRectCCW.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRectCCW.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRectCCW.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRectCCW.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRectCCW.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRectCCW.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRectCCW.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRectCCW.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfRectCCW.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRectCCW.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfRectCCW.GetLinear(3).GetEndPoint().GetY());

    HFCPtr<HVE2DComplexLinear>  MyLinearAllocate(Rect1.AllocateLinear(HVE2DSimpleShape::CCW));
    ASSERT_EQ(4, MyLinearAllocate->GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, MyLinearAllocate->GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearAllocate->GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, MyLinearAllocate->GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearAllocate->GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearAllocate->GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearAllocate->GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, MyLinearAllocate->GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearAllocate->GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearAllocate->GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearAllocate->GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, MyLinearAllocate->GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearAllocate->GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, MyLinearAllocate->GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearAllocate->GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, MyLinearAllocate->GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearAllocate->GetLinear(3).GetEndPoint().GetY());

    }

//==================================================================================
// CalculatePerimeter() const;
//==================================================================================
TEST_F (HVE2DRectangleTest, CalculatePerimeterTest)
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
TEST_F (HVE2DRectangleTest, CalculateAreaTest)
    {

    ASSERT_DOUBLE_EQ(100.0, Rect1.CalculateArea());
    ASSERT_DOUBLE_EQ(100.0, NegativeRect.CalculateArea());
    ASSERT_DOUBLE_EQ(25.00, IncludedRect1.CalculateArea());
    ASSERT_DOUBLE_EQ(25.00, IncludedRect2.CalculateArea());
    ASSERT_DOUBLE_EQ(25.00, IncludedRect3.CalculateArea());
    ASSERT_DOUBLE_EQ(25.00, IncludedRect4.CalculateArea());
    ASSERT_DOUBLE_EQ(36.00, IncludedRect5.CalculateArea());
    ASSERT_DOUBLE_EQ(50.00, IncludedRect6.CalculateArea());
    ASSERT_DOUBLE_EQ(50.00, IncludedRect7.CalculateArea());
    ASSERT_DOUBLE_EQ(50.00, IncludedRect8.CalculateArea());
    ASSERT_DOUBLE_EQ(50.00, IncludedRect9.CalculateArea());

    }

//==================================================================================
// CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
//==================================================================================
TEST_F (HVE2DRectangleTest, CalculateClosestPointTest)
    {

    // Test with linear 1
    ASSERT_DOUBLE_EQ(20.0, Rect1.CalculateClosestPoint(RectClosePoint1A).GetX());
    ASSERT_DOUBLE_EQ(10.1, Rect1.CalculateClosestPoint(RectClosePoint1A).GetY());
    ASSERT_DOUBLE_EQ(10.0, Rect1.CalculateClosestPoint(RectClosePoint1B).GetX());
    ASSERT_DOUBLE_EQ(10.0, Rect1.CalculateClosestPoint(RectClosePoint1B).GetY());
    ASSERT_DOUBLE_EQ(19.9, Rect1.CalculateClosestPoint(RectClosePoint1C).GetX());
    ASSERT_DOUBLE_EQ(10.0, Rect1.CalculateClosestPoint(RectClosePoint1C).GetY());
    ASSERT_DOUBLE_EQ(10.0, Rect1.CalculateClosestPoint(RectClosePoint1D).GetX());
    ASSERT_DOUBLE_EQ(15.0, Rect1.CalculateClosestPoint(RectClosePoint1D).GetY());
    ASSERT_DOUBLE_EQ(15.0, Rect1.CalculateClosestPoint(RectCloseMidPoint1).GetX());
    ASSERT_DOUBLE_EQ(20.0, Rect1.CalculateClosestPoint(RectCloseMidPoint1).GetY());

    // Tests with special points
    ASSERT_DOUBLE_EQ(20.0, Rect1.CalculateClosestPoint(VeryFarPoint).GetX());
    ASSERT_DOUBLE_EQ(20.0, Rect1.CalculateClosestPoint(VeryFarPoint).GetY());
    ASSERT_DOUBLE_EQ(15.0, Rect1.CalculateClosestPoint(RectMidPoint1).GetX());
    ASSERT_DOUBLE_EQ(20.0, Rect1.CalculateClosestPoint(RectMidPoint1).GetY());

    }

//==================================================================================
// Intersection test (with other complex linears only)
// Intersect(const HVE2DVector& pi_rVector, HGF2DLocationCollection* po_pCrossPoints) const;
//==================================================================================
TEST_F (HVE2DRectangleTest, IntersectTest)
    {

    HGF2DLocationCollection   DumPoints;

    // Test with extent disjoint linears
    ASSERT_EQ(0, Rect1.Intersect(DisjointLinear1, &DumPoints));

    // Test with disjoint but touching by a side
    ASSERT_EQ(0, Rect1.Intersect(ContiguousExtentLinear1, &DumPoints));

    // Test with disjoint but touching by a tip but not linked
    ASSERT_EQ(0, Rect1.Intersect(FlirtingExtentLinear1, &DumPoints));

    // Tests with connected linears
    // At start point...
    ASSERT_EQ(0, Rect1.Intersect(ConnectingLinear1, &DumPoints));

    // At end point ...
    ASSERT_EQ(0, Rect1.Intersect(ConnectingLinear1A, &DumPoints));

    // Tests with linked segments
    ASSERT_EQ(0, Rect1.Intersect(LinkedLinear1, &DumPoints));

    // Special cases
    ASSERT_EQ(1, Rect1.Intersect(ComplexLinearCase1, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(13.5, DumPoints[0].GetY());
    DumPoints.clear();

    ASSERT_EQ(1, Rect1.Intersect(ComplexLinearCase2, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(15.0, DumPoints[0].GetY());
    DumPoints.clear();

    ASSERT_EQ(1, Rect1.Intersect(ComplexLinearCase3, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());
    DumPoints.clear();

    ASSERT_EQ(0, Rect1.Intersect(ComplexLinearCase4, &DumPoints));
    DumPoints.clear();

    ASSERT_EQ(1, Rect1.Intersect(ComplexLinearCase5, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());
    DumPoints.clear();

    ASSERT_EQ(0, Rect1.Intersect(ComplexLinearCase5A, &DumPoints));
    ASSERT_EQ(0, Rect1.Intersect(ComplexLinearCase6, &DumPoints));
    ASSERT_EQ(0, Rect1.Intersect(ComplexLinearCase7, &DumPoints));

    }

//==================================================================================
// Contiguousness Test
//   ObtainContiguousnessPoints(const HGF2DVector& pi_rVector,
//                              HGF2DLocationCollection* po_pContiguousnessPoints) const;
//   ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
//                                const HGF2DLocation& pi_rPoint,
//                                HGF2DLocation* pi_pFirstContiguousnessPoint,
//                                HGF2DLocation* pi_pSecondContiguousnessPoint) const;
//    HDLL virtual bool      AreContiguous(const HVE2DVector& pi_rVector) const;
//    HDLL virtual bool      AreContiguousAt(const HGF2DVector& pi_rVector,
//                                            const HGF2DLocation& pi_rPoint) const;
//==================================================================================
TEST_F (HVE2DRectangleTest, ContiguousnessTest)
    {
    HGF2DLocationCollection     DumPoints;
    HGF2DLocation   FirstDumPoint;
    HGF2DLocation   SecondDumPoint;

    // Test with contiguous linears
    ASSERT_TRUE(Rect1.AreContiguous(ComplexLinearCase6));

    ASSERT_TRUE(Rect1.AreContiguousAt(ComplexLinearCase6, RectMidPoint1));

    ASSERT_EQ(2, Rect1.ObtainContiguousnessPoints(ComplexLinearCase6, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetY());

    Rect1.ObtainContiguousnessPointsAt(ComplexLinearCase6, LinearMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(10.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetY());

    // Test with non contiguous linears
    ASSERT_FALSE(Rect1.AreContiguous(ComplexLinearCase1));
    DumPoints.clear();

    // Test with contiguous rectangle
    ASSERT_TRUE(Rect1.AreContiguous(NorthContiguousRect));
    ASSERT_TRUE(Rect1.AreContiguousAt(NorthContiguousRect, RectMidPoint1));

    ASSERT_EQ(2, Rect1.ObtainContiguousnessPoints(NorthContiguousRect, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[1].GetY());

    Rect1.ObtainContiguousnessPointsAt(NorthContiguousRect, RectMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(10.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(20.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, SecondDumPoint.GetY());

    DumPoints.clear();

    ASSERT_TRUE(Rect1.AreContiguous(VerticalFitRect));
    ASSERT_TRUE(Rect1.AreContiguousAt(VerticalFitRect, HGF2DLocation(17.0, 10.0, pWorld)));
    ASSERT_EQ(4, Rect1.ObtainContiguousnessPoints(VerticalFitRect, &DumPoints));
    ASSERT_DOUBLE_EQ(15.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[1].GetY());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[2].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[2].GetY());
    ASSERT_DOUBLE_EQ(15.0, DumPoints[3].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[3].GetY());

    Rect1.ObtainContiguousnessPointsAt(VerticalFitRect, HGF2DLocation(17.0, 10.0, pWorld), &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(20.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(15.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetY());

    DumPoints.clear();

    ASSERT_TRUE(Rect1.AreContiguous(IncludedRect1));

    ASSERT_TRUE(Rect1.AreContiguousAt(IncludedRect1, HGF2DLocation(10.0, 10.0, pWorld)));

    ASSERT_EQ(2, Rect1.ObtainContiguousnessPoints(IncludedRect1, &DumPoints));
    ASSERT_DOUBLE_EQ(15.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(15.0, DumPoints[1].GetY());

    Rect1.ObtainContiguousnessPointsAt(IncludedRect1, HGF2DLocation(10.0, 10.0, pWorld), &FirstDumPoint, &SecondDumPoint);
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
TEST_F (HVE2DRectangleTest, CloningTest)
    {

    // General Clone Test
    HFCPtr<HVE2DRectangle> pClone = (HVE2DRectangle*)Rect1.Clone();
    ASSERT_FALSE(pClone->IsEmpty());
    ASSERT_EQ(pWorld, pClone->GetCoordSys());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pClone->GetShapeType());

    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));  
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld))); 
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld))); 
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld))); 

     // Test with the same coordinate system
    HFCPtr<HVE2DRectangle> pClone3 = (HVE2DRectangle*)Rect1.AllocateCopyInCoordSys(pWorld);
    ASSERT_FALSE(pClone3->IsEmpty());
    ASSERT_EQ(pWorld, pClone3->GetCoordSys());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pClone3->GetShapeType());

    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));  
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld))); 
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld))); 
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld))); 

    // Test with a translation between systems
    Translation = HGF2DDisplacement (10.0, 10.0);
    HGF2DTranslation myTranslation(Translation);
    HFCPtr<HGF2DCoordSys> pWorldTranslation = new HGF2DCoordSys(myTranslation, pWorld);

    HFCPtr<HVE2DRectangle> pClone5 = (HVE2DRectangle*)Rect1.AllocateCopyInCoordSys(pWorldTranslation);
    ASSERT_FALSE(pClone5->IsEmpty());
    ASSERT_EQ(pWorldTranslation, pClone5->GetCoordSys());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pClone5->GetShapeType());

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

    HFCPtr<HVE2DRectangle> pClone6 = (HVE2DRectangle*)Rect1.AllocateCopyInCoordSys(pWorldStretch);
    ASSERT_FALSE(pClone6->IsEmpty());
    ASSERT_EQ(pWorldStretch, pClone6->GetCoordSys());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pClone6->GetShapeType());

    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(0.0, 0.0, pWorldStretch)));  
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(0.0, 20.0, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(20.0, 20.0, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(20.0, 0.0, pWorldStretch))); 

    // Test with a stretch between systems
    HGF2DStretch myStretch2;
    myStretch2.SetXScaling(0.5);
    myStretch2.SetYScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldStretch2 = new HGF2DCoordSys(myStretch2, pWorld);

    HFCPtr<HVE2DRectangle> pClone10 = (HVE2DRectangle*)Rect1.AllocateCopyInCoordSys(pWorldStretch2);
    ASSERT_FALSE(pClone10->IsEmpty());
    ASSERT_EQ(pWorldStretch2, pClone10->GetCoordSys());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pClone10->GetShapeType());

    ASSERT_TRUE(pClone10->IsPointOn(HGF2DLocation(20.0, 20.0, pWorldStretch2)));  
    ASSERT_TRUE(pClone10->IsPointOn(HGF2DLocation(20.0, 40.0, pWorldStretch2))); 
    ASSERT_TRUE(pClone10->IsPointOn(HGF2DLocation(40.0, 40.0, pWorldStretch2))); 
    ASSERT_TRUE(pClone10->IsPointOn(HGF2DLocation(40.0, 40.0, pWorldStretch2))); 

    // Test with a similitude between systems
    HGF2DSimilitude mySimilitude;
    mySimilitude.SetRotation(PI);
    mySimilitude.SetScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldSimilitude = new HGF2DCoordSys(mySimilitude, pWorld);

    HFCPtr<HVE2DRectangle> pClone7 = (HVE2DRectangle*)Rect1.AllocateCopyInCoordSys(pWorldSimilitude);
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

    HFCPtr<HVE2DRectangle> pClone8 = (HVE2DRectangle*)Rect1.AllocateCopyInCoordSys(pWorldAffine);
    ASSERT_FALSE(pClone8->IsEmpty());
    ASSERT_EQ(pWorldAffine, pClone8->GetCoordSys());
    ASSERT_EQ(HVE2DPolygonOfSegments::CLASS_ID, pClone8->GetShapeType());

    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(0.0, 0.0, pWorldAffine)));  
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(0.0, -20.0, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-20.0, -20.0, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-20.0, 0.0, pWorldAffine))); 

    // Test with a similitude between systems
    Rect1.MakeEmpty();

    HFCPtr<HVE2DRectangle> pClone9 = (HVE2DRectangle*)Rect1.AllocateCopyInCoordSys(pWorldSimilitude);

    ASSERT_TRUE(pClone9->IsEmpty());
    ASSERT_EQ(pWorldSimilitude, pClone9->GetCoordSys());
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pClone9->GetShapeType());
        
    }
    

//==================================================================================
// Interaction info with other segment
// Crosses(const HVE2DVector& pi_rVector) const;
// AreAdjacent(const HVE2DVector& pi_rVector) const;
// IsPointOn(const HGF2DLocation& pi_rTestPoint) const;
// IsPointOnSCS(const HGF2DLocation& pi_rTestPoint) const;
//==================================================================================
TEST_F (HVE2DRectangleTest, InteractionTest)
    {

    // Tests with a vertical segment
    ASSERT_TRUE(Rect1.Crosses(ComplexLinearCase1));
    ASSERT_FALSE(Rect1.AreAdjacent(ComplexLinearCase1));

    ASSERT_TRUE(Rect1.Crosses(ComplexLinearCase2));
    ASSERT_TRUE(Rect1.AreAdjacent(ComplexLinearCase2));

    ASSERT_TRUE(Rect1.Crosses(ComplexLinearCase3));
    ASSERT_FALSE(Rect1.AreAdjacent(ComplexLinearCase3));

    ASSERT_FALSE(Rect1.Crosses(ComplexLinearCase4));
    ASSERT_TRUE(Rect1.AreAdjacent(ComplexLinearCase4));

    ASSERT_TRUE(Rect1.Crosses(ComplexLinearCase5));
    ASSERT_TRUE(Rect1.AreAdjacent(ComplexLinearCase5));

    ASSERT_FALSE(Rect1.Crosses(ComplexLinearCase6));
    ASSERT_TRUE(Rect1.AreAdjacent(ComplexLinearCase6));

    ASSERT_FALSE(Rect1.Crosses(ComplexLinearCase7));
    ASSERT_FALSE(Rect1.AreAdjacent(ComplexLinearCase7));

    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(10.0 - 1.1*MYEPSILON, 10.0-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Rect1.IsPointOn(RectMidPoint1-HGF2DDisplacement(0.0, 1.1*MYEPSILON)));
    ASSERT_FALSE(Rect1.IsPointOn(RectMidPoint1-HGF2DDisplacement(0.0, -1.1*MYEPSILON)));
    ASSERT_TRUE(Rect1.IsPointOn(RectMidPoint1-HGF2DDisplacement(0.0, 0.9*MYEPSILON)));
    ASSERT_TRUE(Rect1.IsPointOn(RectMidPoint1-HGF2DDisplacement(0.0, -0.9*MYEPSILON)));
    ASSERT_TRUE(Rect1.IsPointOn(RectMidPoint1));
    ASSERT_TRUE(Rect1.IsPointOn(Rect1.GetLinear().GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1.IsPointOn(Rect1.GetLinear().GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_TRUE(Rect1.IsPointOnSCS(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointOnSCS(HGF2DLocation(10.0 - 1.1*MYEPSILON, 10.0-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Rect1.IsPointOnSCS(RectMidPoint1-HGF2DDisplacement(0.0, 1.1*MYEPSILON)));
    ASSERT_FALSE(Rect1.IsPointOnSCS(RectMidPoint1-HGF2DDisplacement(0.0, -1.1*MYEPSILON)));
    ASSERT_TRUE(Rect1.IsPointOnSCS(RectMidPoint1-HGF2DDisplacement(0.0, 0.9*MYEPSILON)));
    ASSERT_TRUE(Rect1.IsPointOnSCS(RectMidPoint1-HGF2DDisplacement(0.0, -0.9*MYEPSILON)));
    ASSERT_TRUE(Rect1.IsPointOnSCS(RectMidPoint1));
    ASSERT_TRUE(Rect1.IsPointOnSCS(Rect1.GetLinear().GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1.IsPointOnSCS(Rect1.GetLinear().GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));

    }

//==================================================================================
// Bearing calculation tests
// CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
//                  HVE2DVector::ArbitraryDirection pi_Direction = HVE2DVector::BETA) const;
// CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
//                              HVE2DVector::ArbitraryDirection pi_Direction = HVE2DVector::BETA) const;
//==================================================================================
TEST_F (HVE2DRectangleTest, BearingTest)
    {

    // Obtain bearing ALPHA of a vertical segment
    ASSERT_NEAR(0.0, Rect1.CalculateBearing(Rect1Point0d0, HVE2DVector::ALPHA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.5707963267948966, Rect1.CalculateBearing(Rect1Point0d0, HVE2DVector::BETA).GetAngle());
    ASSERT_NEAR(0.0, Rect1.CalculateBearing(Rect1Point0d1, HVE2DVector::ALPHA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(3.1415926535897931, Rect1.CalculateBearing(Rect1Point0d1, HVE2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(3.1415926535897931, Rect1.CalculateBearing(Rect1Point0d5, HVE2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(4.7123889803846897, Rect1.CalculateBearing(Rect1Point0d5, HVE2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(4.7123889803846897, Rect1.CalculateBearing(Rect1Point1d0, HVE2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(1.5707963267948966, Rect1.CalculateBearing(Rect1Point1d0, HVE2DVector::BETA).GetAngle());

    // ANGULAR ACCELERATION
    // Obtain angular acceleration ALPHA of a vertical segment
    ASSERT_NEAR(0.0, Rect1.CalculateAngularAcceleration(Rect1Point0d0, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, Rect1.CalculateAngularAcceleration(Rect1Point0d0, HVE2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, Rect1.CalculateAngularAcceleration(Rect1Point0d1, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, Rect1.CalculateAngularAcceleration(Rect1Point0d1, HVE2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, Rect1.CalculateAngularAcceleration(Rect1Point0d5, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, Rect1.CalculateAngularAcceleration(Rect1Point0d5, HVE2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, Rect1.CalculateAngularAcceleration(Rect1Point1d0, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, Rect1.CalculateAngularAcceleration(Rect1Point1d0, HVE2DVector::BETA), MYEPSILON);

    }

//==================================================================================
// Extent calculation test
// GetExtent() const;
//==================================================================================
TEST_F (HVE2DRectangleTest, GetExtentTest)
    {

    ASSERT_DOUBLE_EQ(10.0, Rect1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(10.0, Rect1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(20.0, Rect1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(20.0, Rect1.GetExtent().GetYMax());

    ASSERT_DOUBLE_EQ(10.0, IncludedRect1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(10.0, IncludedRect1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(15.0, IncludedRect1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(15.0, IncludedRect1.GetExtent().GetYMax());

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
TEST_F (HVE2DRectangleTest, MoveTest)
    {

    HGF2DDisplacement Translation1(10.0, 10.0);
    HGF2DDisplacement Translation2(0.0, 10.0);
    HGF2DDisplacement Translation3(10.0, 0.0);
    HGF2DDisplacement Translation4(0.0, 0.0);
    HGF2DDisplacement Translation5(-10.0, 0.0);
    HGF2DDisplacement Translation6(0.0, -10.0);
    HGF2DDisplacement Translation7(-10.0, -10.0);
    HGF2DDisplacement Translation8(10.0, -10.0);

    HGF2DLocation MinPoint;
    HGF2DLocation MaxPoint;

    IncludedRect1.Move(Translation1);
    IncludedRect1.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(20.0, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(25.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(25.0, MaxPoint.GetY());

    IncludedRect2.Move(Translation2);
    IncludedRect2.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(15.0, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(20.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(25.0, MaxPoint.GetY());

    IncludedRect3.Move(Translation3);
    IncludedRect3.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(25.0, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(15.0, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(30.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, MaxPoint.GetY());
   
    IncludedRect4.Move(Translation4);
    IncludedRect4.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(10.0, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(15.0, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(15.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, MaxPoint.GetY());
   
    IncludedRect5.Move(Translation5);
    IncludedRect5.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(2.00, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(12.0, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(8.00, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(18.0, MaxPoint.GetY());
    
    IncludedRect6.Move(Translation6);
    IncludedRect6.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(10.0, MinPoint.GetX());
    ASSERT_NEAR(0.0, MinPoint.GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(20.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(5.00, MaxPoint.GetY());
    
    IncludedRect7.Move(Translation7);
    IncludedRect7.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_NEAR(0.0, MinPoint.GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, MinPoint.GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(5.00, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, MaxPoint.GetY());
    
    IncludedRect8.Move(Translation8);
    IncludedRect8.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(25.0, MinPoint.GetX());
    ASSERT_NEAR(0.0, MinPoint.GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, MaxPoint.GetY());

    }

//==================================================================================
// Scale(double pi_ScaleFactor, const HGF2DLocation& pi_rScaleOrigin)
// Scale(double pi_ScaleFactorX, double pi_ScaleFactorY, const HGF2DLocation& pi_rScaleOrigin)
//================================================================================== 
TEST_F (HVE2DRectangleTest, ScaleTest)
    {

    HGF2DLocation Origin(0.0, 0.0, pWorld);
    HGF2DLocation MinPoint;
    HGF2DLocation MaxPoint;

    IncludedRect1.Scale(2.0, Origin);
    IncludedRect1.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(20.0, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(30.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(30.0, MaxPoint.GetY());
 
    IncludedRect2.Scale(5.0, Origin);
    IncludedRect2.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(75.00, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(50.00, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(100.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(75.00, MaxPoint.GetY());

    IncludedRect3.Scale(-2.0, Origin);
    IncludedRect3.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(-40.0, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(-40.0, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(-30.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(-30.0, MaxPoint.GetY());

    IncludedRect4.Scale(2.0, HGF2DLocation(0.5, 0.5, pWorld));
    IncludedRect4.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(19.5, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(29.5, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(29.5, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(39.5, MaxPoint.GetY());

    IncludedRect5.Scale(2.0, HGF2DLocation(-0.5, 0.5, pWorld));
    IncludedRect5.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(24.5, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(23.5, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(36.5, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(35.5, MaxPoint.GetY());

    IncludedRect1.Scale(1.0, 2.0, Origin);
    IncludedRect1.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(20.0, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(40.0, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(30.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(60.0, MaxPoint.GetY());

    IncludedRect2.Scale(-1.0, 2.0, Origin);
    IncludedRect2.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(-100.0, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(100.0, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(-75.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(150.0, MaxPoint.GetY());

    IncludedRect3.Scale(1.0, -2.0, Origin);
    IncludedRect3.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(-40.0, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(60.00, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(-30.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(80.00, MaxPoint.GetY());

    IncludedRect4.Scale(0.5, 2.0, Origin);
    IncludedRect4.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(9.750, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(59.00, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(14.75, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(79.00, MaxPoint.GetY());

    IncludedRect5.Scale(0.5, 0.2, HGF2DLocation(0.5, 0.5, pWorld));
    IncludedRect5.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(12.5, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(5.10, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(18.5, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(7.50, MaxPoint.GetY());

    IncludedRect6.Scale(1.0, 2.0, HGF2DLocation(-0.5, 0.5, pWorld));
    IncludedRect6.GetRectangle(&MinPoint, &MaxPoint);

    ASSERT_DOUBLE_EQ(10.0, MinPoint.GetX());
    ASSERT_DOUBLE_EQ(19.5, MinPoint.GetY());
    ASSERT_DOUBLE_EQ(20.0, MaxPoint.GetX());
    ASSERT_DOUBLE_EQ(29.5, MaxPoint.GetY());

    }

//==================================================================================
// IsEmpty() test
// MakEmpty() test
//==================================================================================
TEST_F (HVE2DRectangleTest, EmptyTest)
    {

    HVE2DRectangle  MyOtherRect(Rect1);
    MyOtherRect.MakeEmpty();
    ASSERT_TRUE(MyOtherRect.IsEmpty());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), Rect1.GetShapeType());

    }

//==================================================================================
// Drop( HGF2DLocationCollection* po_pPoints, const HGFDistance& pi_rTolerance ) const
//==================================================================================
TEST_F (HVE2DRectangleTest, DropTest)
    {

    HGF2DLocationCollection Locations;

    Rect1.Drop(&Locations, MYEPSILON);
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
TEST_F (HVE2DRectangleTest, IsPointTest)
    {

    //IsPointIn
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(15.0, 10.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(20.0, 15.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(15.0, 20.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(10.0, 15.0, pWorld)));

    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(100.0, 100.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(10.0-1.1*MYEPSILON, 10.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(10.0, 10.0-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(10.0-1.1*MYEPSILON, 15.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(10.0-1.1*MYEPSILON, 20.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(10.0, 20.0+1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(15.0, 20.0+1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(20.0, 20.0+1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(20.0+1.1*MYEPSILON, 20.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(20.0+1.1*MYEPSILON, 15.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(20.0+1.1*MYEPSILON, 10.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(20.0, 10.0-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(15.0, 10.0-1.1*MYEPSILON, pWorld)));

    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(10.0-0.9*MYEPSILON, 10.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(10.0, 10.0-0.9*MYEPSILON, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(10.0-0.9*MYEPSILON, 15.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(10.0-0.9*MYEPSILON, 20.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(10.0, 20.0+0.9*MYEPSILON, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(15.0, 20.0+0.9*MYEPSILON, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(20.0, 20.0+0.9*MYEPSILON, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(20.0+0.9*MYEPSILON, 20.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(20.0+0.9*MYEPSILON, 15.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(20.0+0.9*MYEPSILON, 10.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(20.0, 10.0-0.9*MYEPSILON, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(15.0, 10.0-0.9*MYEPSILON, pWorld)));

    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(10.0+0.9*MYEPSILON, 15.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(15.0, 20.0-0.9*MYEPSILON, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(20.0-0.9*MYEPSILON, 15.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(15.0, 10.0+0.9*MYEPSILON, pWorld)));

    ASSERT_TRUE(Rect1.IsPointIn(HGF2DLocation(10.0+1.1*MYEPSILON, 15.0, pWorld)));
    ASSERT_TRUE(Rect1.IsPointIn(HGF2DLocation(15.0, 20.0-1.1*MYEPSILON, pWorld)));
    ASSERT_TRUE(Rect1.IsPointIn(HGF2DLocation(20.0-1.1*MYEPSILON, 15.0, pWorld)));
    ASSERT_TRUE(Rect1.IsPointIn(HGF2DLocation(15.0, 10.0+1.1*MYEPSILON, pWorld)));

    ASSERT_TRUE(Rect1.IsPointIn(HGF2DLocation(12.0, 15.0, pWorld)));
    ASSERT_TRUE(Rect1.IsPointIn(HGF2DLocation(15.0, 18.0, pWorld)));
    ASSERT_TRUE(Rect1.IsPointIn(HGF2DLocation(18.0, 15.0, pWorld)));
    ASSERT_TRUE(Rect1.IsPointIn(HGF2DLocation(15.0, 12.0, pWorld)));
    ASSERT_TRUE(Rect1.IsPointIn(HGF2DLocation(15.0, 15.0, pWorld)));

    ASSERT_FALSE(Rect1.IsPointIn(HGF2DLocation(-15.0, -15.0, pWorld)));

    // IsPointOn
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(15.0, 10.0, pWorld)));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(20.0, 15.0, pWorld)));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(15.0, 20.0, pWorld)));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(10.0, 15.0, pWorld)));

    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(100.0, 100.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(10.0-1.1*MYEPSILON, 10.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(10.0, 10.0-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(10.0-1.1*MYEPSILON, 15.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(10.0-1.1*MYEPSILON, 20.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(10.0, 20.0+1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(15.0, 20.0+1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(20.0, 20.0+1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(20.0+1.1*MYEPSILON, 20.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(20.0+1.1*MYEPSILON, 15.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(20.0+1.1*MYEPSILON, 10.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(20.0, 10.0-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(15.0, 10.0-1.1*MYEPSILON, pWorld)));

    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(10.0-0.9*MYEPSILON, 10.0, pWorld)));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(10.0, 10.0-0.9*MYEPSILON, pWorld)));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(10.0-0.9*MYEPSILON, 15.0, pWorld)));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(10.0-0.9*MYEPSILON, 20.0, pWorld)));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(10.0, 20.0+0.9*MYEPSILON, pWorld)));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(15.0, 20.0+0.9*MYEPSILON, pWorld)));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(20.0, 20.0+0.9*MYEPSILON, pWorld)));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(20.0+0.9*MYEPSILON, 20.0, pWorld)));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(20.0+0.9*MYEPSILON, 15.0, pWorld)));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(20.0+0.9*MYEPSILON, 10.0, pWorld)));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(20.0, 10.0-0.9*MYEPSILON, pWorld)));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(15.0, 10.0-0.9*MYEPSILON, pWorld)));

    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(10.0+0.9*MYEPSILON, 15.0, pWorld)));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(15.0, 20.0-0.9*MYEPSILON, pWorld)));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(20.0-0.9*MYEPSILON, 15.0, pWorld)));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(15.0, 10.0+0.9*MYEPSILON, pWorld)));

    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(10.0+1.1*MYEPSILON, 15.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(15.0, 20.0-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(20.0-1.1*MYEPSILON, 15.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(15.0, 10.0+1.1*MYEPSILON, pWorld)));

    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(12.0, 15.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(15.0, 18.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(18.0, 15.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(15.0, 12.0, pWorld)));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(15.0, 15.0, pWorld)));

    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(-15.0, -15.0, pWorld)));

    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(15.0, 10.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(20.0, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(15.0, 20.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(10.0, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(100.0, 100.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(10.0-1.1*MYEPSILON, 10.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(10.0, 10.0-1.1*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(10.0-1.1*MYEPSILON, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(10.0-1.1*MYEPSILON, 20.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(10.0, 20.0+1.1*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(15.0, 20.0+1.1*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(20.0, 20.0+1.1*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(20.0+1.1*MYEPSILON, 20.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(20.0+1.1*MYEPSILON, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(20.0+1.1*MYEPSILON, 10.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(20.0, 10.0-1.1*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(15.0, 10.0-1.1*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(10.0-0.9*MYEPSILON, 10.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(10.0, 10.0-0.9*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(10.0-0.9*MYEPSILON, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(10.0-0.9*MYEPSILON, 20.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(10.0, 20.0+0.9*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(15.0, 20.0+0.9*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(20.0, 20.0+0.9*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(20.0+0.9*MYEPSILON, 20.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(20.0+0.9*MYEPSILON, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(20.0+0.9*MYEPSILON, 10.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(20.0, 10.0-0.9*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(15.0, 10.0-0.9*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(10.0+0.9*MYEPSILON, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(15.0, 20.0-0.9*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(20.0-0.9*MYEPSILON, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Rect1.IsPointOn(HGF2DLocation(15.0, 10.0+0.9*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(10.0+1.1*MYEPSILON, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(15.0, 20.0-1.1*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(20.0-1.1*MYEPSILON, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(15.0, 10.0+1.1*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(12.0, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(15.0, 18.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(18.0, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(15.0, 12.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(15.0, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_FALSE(Rect1.IsPointOn(HGF2DLocation(-15.0, -15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));

    }

//==================================================================================
// UnifyShape( const HVE2DShape& pi_rShape ) const
//==================================================================================
TEST_F (HVE2DRectangleTest, UnifyShapeTest)
    {

    HFCPtr<HVE2DShape>     pResultShape1 = Rect1.UnifyShape(NorthContiguousRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape1->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape1->GetCoordSys());
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(20.0, 30.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape2 = Rect1.UnifyShape(EastContiguousRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape2->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape2->GetCoordSys());
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(30.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape3 = Rect1.UnifyShape(WestContiguousRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape3->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape3->GetCoordSys());
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(0.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape4 = Rect1.UnifyShape(SouthContiguousRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape4->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape4->GetCoordSys());
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(10.0, 0.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape5 = Rect1.UnifyShape(VerticalFitRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape5->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape5->GetCoordSys());
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(25.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape6 = Rect1.UnifyShape(HorizontalFitRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape6->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape6->GetCoordSys());
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(20.0, 25.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape7 = Rect1.UnifyShape(DisjointRect);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape7->GetShapeType());

    HFCPtr<HVE2DShape>     pResultShape8 = Rect1.UnifyShape(MiscRect1);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape8->GetShapeType());

    HFCPtr<HVE2DShape>     pResultShape9 = Rect1.UnifyShape(EnglobRect1);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape9->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape9->GetCoordSys());
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DLocation(30.0, 30.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape10 = Rect1.UnifyShape(EnglobRect2);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape10->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape10->GetCoordSys());
    ASSERT_TRUE(pResultShape10->IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(pResultShape10->IsPointOn(HGF2DLocation(30.0, 30.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape11 = Rect1.UnifyShape(EnglobRect3);
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape11->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape11->GetCoordSys());
    ASSERT_TRUE(pResultShape11->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape11->IsPointOn(HGF2DLocation(20.0, 30.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape12 = Rect1.UnifyShape(IncludedRect1);
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape12->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape12->GetCoordSys());
    ASSERT_TRUE(pResultShape12->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape12->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape13 = Rect1.UnifyShape(IncludedRect2);
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape13->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape13->GetCoordSys());
    ASSERT_TRUE(pResultShape13->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape13->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape14 = Rect1.UnifyShape(IncludedRect3);
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape14->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape14->GetCoordSys());
    ASSERT_TRUE(pResultShape14->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape14->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape15 = Rect1.UnifyShape(IncludedRect4);
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape15->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape15->GetCoordSys());
    ASSERT_TRUE(pResultShape15->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape15->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape16 = Rect1.UnifyShape(IncludedRect5);
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape16->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape16->GetCoordSys());
    ASSERT_TRUE(pResultShape16->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape16->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape17 = Rect1.UnifyShape(IncludedRect6);
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape17->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape17->GetCoordSys());
    ASSERT_TRUE(pResultShape17->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape17->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape18 = Rect1.UnifyShape(IncludedRect7);
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape18->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape18->GetCoordSys());
    ASSERT_TRUE(pResultShape18->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape18->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape19 = Rect1.UnifyShape(IncludedRect8);
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape19->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape19->GetCoordSys());
    ASSERT_TRUE(pResultShape19->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape19->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape20 = Rect1.UnifyShape(IncludedRect9);
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape20->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape20->GetCoordSys());
    ASSERT_TRUE(pResultShape20->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape20->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    }

//==================================================================================
// UnifyShapeSCS ( const HVE2DShape& pi_rShape ) const
//==================================================================================
TEST_F (HVE2DRectangleTest, UnifyShapeSCSTest)
    {

    HFCPtr<HVE2DShape>     pResultShape1 = Rect1.UnifyShapeSCS(NorthContiguousRect);
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape1->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape1->GetCoordSys());
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(20.0, 30.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape2 = Rect1.UnifyShapeSCS(EastContiguousRect);
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape2->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape2->GetCoordSys());
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(30.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape3 = Rect1.UnifyShapeSCS(WestContiguousRect);
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape3->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape3->GetCoordSys());
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(0.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape4 = Rect1.UnifyShapeSCS(SouthContiguousRect);
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape4->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape4->GetCoordSys());
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(10.0, 0.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape5 = Rect1.UnifyShapeSCS(VerticalFitRect);
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape5->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape5->GetCoordSys());
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(25.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape6 = Rect1.UnifyShapeSCS(HorizontalFitRect);
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape6->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape6->GetCoordSys());
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(20.0, 25.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape7 = Rect1.UnifyShapeSCS(DisjointRect);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape7->GetShapeType());

    HFCPtr<HVE2DShape>     pResultShape8 = Rect1.UnifyShapeSCS(MiscRect1);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape8->GetShapeType());

    HFCPtr<HVE2DShape>     pResultShape9 = Rect1.UnifyShapeSCS(EnglobRect1);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape9->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape9->GetCoordSys());
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DLocation(10.0, 30.0, pWorld)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DLocation(30.0, 30.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape10 = Rect1.UnifyShapeSCS(EnglobRect2);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape10->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape10->GetCoordSys());
    ASSERT_TRUE(pResultShape10->IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(pResultShape10->IsPointOn(HGF2DLocation(30.0, 30.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape11 = Rect1.UnifyShapeSCS(EnglobRect3);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape11->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape11->GetCoordSys());
    ASSERT_TRUE(pResultShape11->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape11->IsPointOn(HGF2DLocation(20.0, 30.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape12 = Rect1.UnifyShapeSCS(IncludedRect1);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape12->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape12->GetCoordSys());
    ASSERT_TRUE(pResultShape12->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape12->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape13 = Rect1.UnifyShapeSCS(IncludedRect2);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape13->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape13->GetCoordSys());
    ASSERT_TRUE(pResultShape13->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape13->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape14 = Rect1.UnifyShapeSCS(IncludedRect3);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape14->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape14->GetCoordSys());
    ASSERT_TRUE(pResultShape14->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape14->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape15 = Rect1.UnifyShapeSCS(IncludedRect4);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape15->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape15->GetCoordSys());
    ASSERT_TRUE(pResultShape15->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape15->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape16 = Rect1.UnifyShapeSCS(IncludedRect5);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape16->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape16->GetCoordSys());
    ASSERT_TRUE(pResultShape16->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape16->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape17 = Rect1.UnifyShapeSCS(IncludedRect6);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape17->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape17->GetCoordSys());
    ASSERT_TRUE(pResultShape17->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape17->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape18 = Rect1.UnifyShapeSCS(IncludedRect7);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape18->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape18->GetCoordSys());
    ASSERT_TRUE(pResultShape18->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape18->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape19 = Rect1.UnifyShapeSCS(IncludedRect8);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape19->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape19->GetCoordSys());
    ASSERT_TRUE(pResultShape19->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape19->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape20 = Rect1.UnifyShapeSCS(IncludedRect9);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape20->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape20->GetCoordSys());
    ASSERT_TRUE(pResultShape20->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape20->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    }

//==================================================================================
// IntersectShape( const HVE2DShape& pi_rShape ) const
//==================================================================================
TEST_F (HVE2DRectangleTest, IntersectShapeTest)
    {

    HFCPtr<HVE2DShape>     pResultShape1 = Rect1.IntersectShape(NorthContiguousRect);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape1->GetShapeType());
    ASSERT_TRUE(pResultShape1->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape2 = Rect1.IntersectShape(EastContiguousRect);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape2->GetShapeType());
    ASSERT_TRUE(pResultShape2->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape3 = Rect1.IntersectShape(WestContiguousRect);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape3->GetShapeType());
    ASSERT_TRUE(pResultShape3->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape4 = Rect1.IntersectShape(SouthContiguousRect);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape4->GetShapeType());
    ASSERT_TRUE(pResultShape4->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape5 = Rect1.IntersectShape(VerticalFitRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape5->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape5->GetCoordSys());
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(15.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape6 = Rect1.IntersectShape(HorizontalFitRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape6->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape6->GetCoordSys());
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(10.0, 15.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape7 = Rect1.IntersectShape(DisjointRect);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape7->GetShapeType());
    ASSERT_TRUE(pResultShape7->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape8 = Rect1.IntersectShape(MiscRect1);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape8->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape8->GetCoordSys());
    ASSERT_TRUE(pResultShape8->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape8->IsPointOn(HGF2DLocation(15.0, 15.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape9 = Rect1.IntersectShape(EnglobRect1);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape9->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape9->GetCoordSys());
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape10 = Rect1.IntersectShape(EnglobRect2);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape10->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape10->GetCoordSys());
    ASSERT_TRUE(pResultShape10->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape10->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape11 = Rect1.IntersectShape(EnglobRect3);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape11->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape11->GetCoordSys());
    ASSERT_TRUE(pResultShape11->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape11->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape12 = Rect1.IntersectShape(IncludedRect1);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape12->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape12->GetCoordSys());
    ASSERT_TRUE(pResultShape12->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape12->IsPointOn(HGF2DLocation(15.0, 15.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape13 = Rect1.IntersectShape(IncludedRect2);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape13->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape13->GetCoordSys());
    ASSERT_TRUE(pResultShape13->IsPointOn(HGF2DLocation(15.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape13->IsPointOn(HGF2DLocation(20.0, 15.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape14 = Rect1.IntersectShape(IncludedRect3);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape14->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape14->GetCoordSys());
    ASSERT_TRUE(pResultShape14->IsPointOn(HGF2DLocation(15.0, 15.0, pWorld)));
    ASSERT_TRUE(pResultShape14->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape15 = Rect1.IntersectShape(IncludedRect4);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape15->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape15->GetCoordSys());
    ASSERT_TRUE(pResultShape15->IsPointOn(HGF2DLocation(10.0, 15.0, pWorld)));
    ASSERT_TRUE(pResultShape15->IsPointOn(HGF2DLocation(15.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape16 = Rect1.IntersectShape(IncludedRect5);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape16->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape16->GetCoordSys());
    ASSERT_TRUE(pResultShape16->IsPointOn(HGF2DLocation(12.0, 12.0, pWorld)));
    ASSERT_TRUE(pResultShape16->IsPointOn(HGF2DLocation(18.0, 18.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape17 = Rect1.IntersectShape(IncludedRect6);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape17->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape17->GetCoordSys());
    ASSERT_TRUE(pResultShape17->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape17->IsPointOn(HGF2DLocation(20.0, 15.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape18 = Rect1.IntersectShape(IncludedRect7);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape18->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape18->GetCoordSys());
    ASSERT_TRUE(pResultShape18->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape18->IsPointOn(HGF2DLocation(15.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape19 = Rect1.IntersectShape(IncludedRect8);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape19->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape19->GetCoordSys());
    ASSERT_TRUE(pResultShape19->IsPointOn(HGF2DLocation(15.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape19->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape20 = Rect1.IntersectShape(IncludedRect9);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape20->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape20->GetCoordSys());
    ASSERT_TRUE(pResultShape20->IsPointOn(HGF2DLocation(10.0, 15.0, pWorld)));
    ASSERT_TRUE(pResultShape20->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    }

//==================================================================================
// IntersectShapeSCS( const HVE2DShape& pi_rShape ) const
//==================================================================================
TEST_F (HVE2DRectangleTest, IntersectShapeSCSTest)
    {

    HFCPtr<HVE2DShape>     pResultShape1 = Rect1.IntersectShapeSCS(NorthContiguousRect);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape1->GetShapeType());
    ASSERT_TRUE(pResultShape1->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape2 = Rect1.IntersectShapeSCS(EastContiguousRect);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape2->GetShapeType());
    ASSERT_TRUE(pResultShape2->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape3 = Rect1.IntersectShapeSCS(WestContiguousRect);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape3->GetShapeType());
    ASSERT_TRUE(pResultShape3->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape4 = Rect1.IntersectShapeSCS(SouthContiguousRect);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape4->GetShapeType());
    ASSERT_TRUE(pResultShape4->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape5 = Rect1.IntersectShapeSCS(VerticalFitRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape5->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape5->GetCoordSys());
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(15.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape6 = Rect1.IntersectShapeSCS(HorizontalFitRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape6->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape6->GetCoordSys());
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(10.0, 15.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape7 = Rect1.IntersectShapeSCS(DisjointRect);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape7->GetShapeType());
    ASSERT_TRUE(pResultShape7->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape8 = Rect1.IntersectShapeSCS(MiscRect1);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape8->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape8->GetCoordSys());
    ASSERT_TRUE(pResultShape8->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape8->IsPointOn(HGF2DLocation(15.0, 15.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape9 = Rect1.IntersectShapeSCS(EnglobRect1);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape9->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape9->GetCoordSys());
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape10 = Rect1.IntersectShapeSCS(EnglobRect2);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape10->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape10->GetCoordSys());
    ASSERT_TRUE(pResultShape10->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape10->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape11 = Rect1.IntersectShapeSCS(EnglobRect3);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape11->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape11->GetCoordSys());
    ASSERT_TRUE(pResultShape11->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape11->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape12 = Rect1.IntersectShapeSCS(IncludedRect1);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape12->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape12->GetCoordSys());
    ASSERT_TRUE(pResultShape12->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape12->IsPointOn(HGF2DLocation(15.0, 15.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape13 = Rect1.IntersectShapeSCS(IncludedRect2);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape13->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape13->GetCoordSys());
    ASSERT_TRUE(pResultShape13->IsPointOn(HGF2DLocation(15.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape13->IsPointOn(HGF2DLocation(20.0, 15.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape14 = Rect1.IntersectShapeSCS(IncludedRect3);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape14->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape14->GetCoordSys());
    ASSERT_TRUE(pResultShape14->IsPointOn(HGF2DLocation(15.0, 15.0, pWorld)));
    ASSERT_TRUE(pResultShape14->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape15 = Rect1.IntersectShapeSCS(IncludedRect4);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape15->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape15->GetCoordSys());
    ASSERT_TRUE(pResultShape15->IsPointOn(HGF2DLocation(10.0, 15.0, pWorld)));
    ASSERT_TRUE(pResultShape15->IsPointOn(HGF2DLocation(15.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape16 = Rect1.IntersectShapeSCS(IncludedRect5);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape16->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape16->GetCoordSys());
    ASSERT_TRUE(pResultShape16->IsPointOn(HGF2DLocation(12.0, 12.0, pWorld)));
    ASSERT_TRUE(pResultShape16->IsPointOn(HGF2DLocation(18.0, 18.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape17 = Rect1.IntersectShapeSCS(IncludedRect6);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape17->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape17->GetCoordSys());
    ASSERT_TRUE(pResultShape17->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape17->IsPointOn(HGF2DLocation(20.0, 15.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape18 = Rect1.IntersectShapeSCS(IncludedRect7);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape18->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape18->GetCoordSys());
    ASSERT_TRUE(pResultShape18->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape18->IsPointOn(HGF2DLocation(15.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape19 = Rect1.IntersectShapeSCS(IncludedRect8);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape19->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape19->GetCoordSys());
    ASSERT_TRUE(pResultShape19->IsPointOn(HGF2DLocation(15.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape19->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape20 = Rect1.IntersectShapeSCS(IncludedRect9);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape20->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape20->GetCoordSys());
    ASSERT_TRUE(pResultShape20->IsPointOn(HGF2DLocation(10.0, 15.0, pWorld)));
    ASSERT_TRUE(pResultShape20->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    }

//==================================================================================
// DifferentiateShape( const HVE2DShape& pi_rShape ) const
//==================================================================================
TEST_F (HVE2DRectangleTest, DifferentiateShape)
    {

    // Spatial oriented operations DIFF
    HFCPtr<HVE2DShape>     pResultShape1 = Rect1.DifferentiateShape(NorthContiguousRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape1->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape1->GetCoordSys());
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape2 = Rect1.DifferentiateShape(EastContiguousRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape2->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape2->GetCoordSys());
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape3 = Rect1.DifferentiateShape(WestContiguousRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape3->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape3->GetCoordSys());
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape4 = Rect1.DifferentiateShape(SouthContiguousRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape4->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape4->GetCoordSys());
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape5 = Rect1.DifferentiateShape(VerticalFitRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape5->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape5->GetCoordSys());
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(15.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape6 = Rect1.DifferentiateShape(HorizontalFitRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape6->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape6->GetCoordSys());
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(20.0, 15.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape7 = Rect1.DifferentiateShape(DisjointRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape7->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape7->GetCoordSys());
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape8 = Rect1.DifferentiateShape(MiscRect1);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape8->GetShapeType());

    HFCPtr<HVE2DShape>     pResultShape9 = Rect1.DifferentiateShape(EnglobRect1);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape9->GetShapeType());
    ASSERT_TRUE(pResultShape9->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape10 = Rect1.DifferentiateShape(EnglobRect2);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape10->GetShapeType());
    ASSERT_TRUE(pResultShape10->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape11 = Rect1.DifferentiateShape(EnglobRect3);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape11->GetShapeType());
    ASSERT_TRUE(pResultShape11->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape12 = Rect1.DifferentiateShape(IncludedRect1);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape12->GetShapeType());

    HFCPtr<HVE2DShape>     pResultShape13 = Rect1.DifferentiateShape(IncludedRect2);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape13->GetShapeType());

    HFCPtr<HVE2DShape>     pResultShape14 = Rect1.DifferentiateShape(IncludedRect3);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape14->GetShapeType());

    HFCPtr<HVE2DShape>     pResultShape15 = Rect1.DifferentiateShape(IncludedRect4);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape15->GetShapeType());

    HFCPtr<HVE2DShape>     pResultShape16 = Rect1.DifferentiateShape(IncludedRect5);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape16->GetShapeType());

    HFCPtr<HVE2DShape>     pResultShape17 = Rect1.DifferentiateShape(IncludedRect6);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape17->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape17->GetCoordSys());
    ASSERT_TRUE(pResultShape17->IsPointOn(HGF2DLocation(10.0, 15.0, pWorld)));
    ASSERT_TRUE(pResultShape17->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape18 = Rect1.DifferentiateShape(IncludedRect7);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape18->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape18->GetCoordSys());
    ASSERT_TRUE(pResultShape18->IsPointOn(HGF2DLocation(15.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape18->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape19 = Rect1.DifferentiateShape(IncludedRect8);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape19->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape19->GetCoordSys());
    ASSERT_TRUE(pResultShape19->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape19->IsPointOn(HGF2DLocation(15.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape20 = Rect1.DifferentiateShape(IncludedRect9);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape20->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape20->GetCoordSys());
    ASSERT_TRUE(pResultShape20->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape20->IsPointOn(HGF2DLocation(20.0, 15.0, pWorld)));

    }

//==================================================================================
// DifferentiateShapeSCS(const HVE2DShape& pi_rShape) const;
//==================================================================================
TEST_F (HVE2DRectangleTest, DifferentiateShapeSCS)
    {

    // Spatial oriented operations DIFF
    HFCPtr<HVE2DShape>     pResultShape1 = Rect1.DifferentiateShapeSCS(NorthContiguousRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape1->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape1->GetCoordSys());
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape2 = Rect1.DifferentiateShapeSCS(EastContiguousRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape2->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape2->GetCoordSys());
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape3 = Rect1.DifferentiateShapeSCS(WestContiguousRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape3->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape3->GetCoordSys());
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape4 = Rect1.DifferentiateShapeSCS(SouthContiguousRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape4->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape4->GetCoordSys());
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape5 = Rect1.DifferentiateShapeSCS(VerticalFitRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape5->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape5->GetCoordSys());
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(15.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape6 = Rect1.DifferentiateShapeSCS(HorizontalFitRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape6->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape6->GetCoordSys());
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(20.0, 15.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape7 = Rect1.DifferentiateShapeSCS(DisjointRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape7->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape7->GetCoordSys());
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape8 = Rect1.DifferentiateShapeSCS(MiscRect1);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape8->GetShapeType());

    HFCPtr<HVE2DShape>     pResultShape9 = Rect1.DifferentiateShapeSCS(EnglobRect1);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape9->GetShapeType());
    ASSERT_TRUE(pResultShape9->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape10 = Rect1.DifferentiateShapeSCS(EnglobRect2);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape10->GetShapeType());
    ASSERT_TRUE(pResultShape10->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape11 = Rect1.DifferentiateShapeSCS(EnglobRect3);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape11->GetShapeType());
    ASSERT_TRUE(pResultShape11->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape12 = Rect1.DifferentiateShapeSCS(IncludedRect1);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape12->GetShapeType());

    HFCPtr<HVE2DShape>     pResultShape13 = Rect1.DifferentiateShapeSCS(IncludedRect2);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape13->GetShapeType());

    HFCPtr<HVE2DShape>     pResultShape14 = Rect1.DifferentiateShapeSCS(IncludedRect3);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape14->GetShapeType());

    HFCPtr<HVE2DShape>     pResultShape15 = Rect1.DifferentiateShapeSCS(IncludedRect4);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape15->GetShapeType());

    HFCPtr<HVE2DShape>     pResultShape16 = Rect1.DifferentiateShapeSCS(IncludedRect5);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape16->GetShapeType());

    HFCPtr<HVE2DShape>     pResultShape17 = Rect1.DifferentiateShapeSCS(IncludedRect6);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape17->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape17->GetCoordSys());
    ASSERT_TRUE(pResultShape17->IsPointOn(HGF2DLocation(10.0, 15.0, pWorld)));
    ASSERT_TRUE(pResultShape17->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape18 = Rect1.DifferentiateShapeSCS(IncludedRect7);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape18->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape18->GetCoordSys());
    ASSERT_TRUE(pResultShape18->IsPointOn(HGF2DLocation(15.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape18->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape19 = Rect1.DifferentiateShapeSCS(IncludedRect8);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape19->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape19->GetCoordSys());
    ASSERT_TRUE(pResultShape19->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape19->IsPointOn(HGF2DLocation(15.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape20 = Rect1.DifferentiateShapeSCS(IncludedRect9);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape20->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape20->GetCoordSys());
    ASSERT_TRUE(pResultShape20->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape20->IsPointOn(HGF2DLocation(20.0, 15.0, pWorld)));

    }

//==================================================================================
// DifferentiateFromShapeSCS( const HVE2DShape& pi_rShape ) const
//==================================================================================
TEST_F (HVE2DRectangleTest, DifferentiateFromShapeSCS)
    {
    
    // Spatial oriented operations DIFF
    HFCPtr<HVE2DShape>     pResultShape1 = Rect1.DifferentiateFromShapeSCS(NorthContiguousRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape1->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape1->GetCoordSys());
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(20.0, 30.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape2 = Rect1.DifferentiateFromShapeSCS(EastContiguousRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape2->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape2->GetCoordSys());
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(30.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape3 = Rect1.DifferentiateFromShapeSCS(WestContiguousRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape3->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape3->GetCoordSys());
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(0.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape4 = Rect1.DifferentiateFromShapeSCS(SouthContiguousRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape4->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape4->GetCoordSys());
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(10.0, 0.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape5 = Rect1.DifferentiateFromShapeSCS(VerticalFitRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape5->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape5->GetCoordSys());
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(25.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape6 = Rect1.DifferentiateFromShapeSCS(HorizontalFitRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape6->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape6->GetCoordSys());
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(20.0, 25.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape7 = Rect1.DifferentiateFromShapeSCS(DisjointRect);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape7->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape7->GetCoordSys());
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DLocation(-10.0, -10.0, pWorld)));
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape8 = Rect1.DifferentiateFromShapeSCS(MiscRect1);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape8->GetShapeType());

    HFCPtr<HVE2DShape>     pResultShape9 = Rect1.DifferentiateFromShapeSCS(EnglobRect1);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape9->GetShapeType());

    HFCPtr<HVE2DShape>     pResultShape10 = Rect1.DifferentiateFromShapeSCS(EnglobRect2);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape10->GetShapeType());

    HFCPtr<HVE2DShape>     pResultShape11 = Rect1.DifferentiateFromShapeSCS(EnglobRect3);
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape11->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape11->GetCoordSys());
    ASSERT_TRUE(pResultShape11->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape11->IsPointOn(HGF2DLocation(20.0, 30.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape12 = Rect1.DifferentiateFromShapeSCS(IncludedRect1);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape12->GetShapeType());
    ASSERT_TRUE(pResultShape12->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape13 = Rect1.DifferentiateFromShapeSCS(IncludedRect2);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape13->GetShapeType());
    ASSERT_TRUE(pResultShape13->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape14 = Rect1.DifferentiateFromShapeSCS(IncludedRect3);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape14->GetShapeType());
    ASSERT_TRUE(pResultShape14->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape15 = Rect1.DifferentiateFromShapeSCS(IncludedRect4);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape15->GetShapeType());
    ASSERT_TRUE(pResultShape15->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape16 = Rect1.DifferentiateFromShapeSCS(IncludedRect5);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape16->GetShapeType());
    ASSERT_TRUE(pResultShape16->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape17 = Rect1.DifferentiateFromShapeSCS(IncludedRect6);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape17->GetShapeType());
    ASSERT_TRUE(pResultShape17->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape18 = Rect1.DifferentiateFromShapeSCS(IncludedRect7);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape18->GetShapeType());
    ASSERT_TRUE(pResultShape18->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape19 = Rect1.DifferentiateFromShapeSCS(IncludedRect8);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape19->GetShapeType());
    ASSERT_TRUE(pResultShape19->IsEmpty());

    HFCPtr<HVE2DShape>     pResultShape20 = Rect1.DifferentiateFromShapeSCS(IncludedRect9);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape20->GetShapeType());
    ASSERT_TRUE(pResultShape20->IsEmpty());

    }

//==================================================================================
// Test which failed on May 4, 2001
// CalculateSpatialPositionOf
//==================================================================================
TEST_F (HVE2DRectangleTest, CalculateSpatialPositionOf)
    {
    
    HVE2DRectangle MyRectangle(0.0, 0.0, 10.0, 10.0, pWorld);

    HVE2DSegment MyTestSegment1(HGF2DLocation(1.00, 10.0, pWorld), HGF2DLocation(9.00, 10.0, pWorld));
    HVE2DSegment MyTestSegment2(HGF2DLocation(1.00, 0.00, pWorld), HGF2DLocation(9.00, 0.00, pWorld));
    HVE2DSegment MyTestSegment3(HGF2DLocation(0.00, 1.00, pWorld), HGF2DLocation(0.00, 9.00, pWorld));
    HVE2DSegment MyTestSegment4(HGF2DLocation(10.0, 1.00, pWorld), HGF2DLocation(10.0, 9.00, pWorld));

    HVE2DSegment MyTestSegment1A(HGF2DLocation(1.0, 10.0 - MYEPSILON, pWorld), HGF2DLocation(9.0, 10.0, pWorld));
    HVE2DSegment MyTestSegment2A(HGF2DLocation(1.0, 0.0 - MYEPSILON, pWorld), HGF2DLocation(9.0, 0.0, pWorld));
    HVE2DSegment MyTestSegment3A(HGF2DLocation(0.0 - MYEPSILON, 1.0, pWorld), HGF2DLocation(0.0, 9.0, pWorld));
    HVE2DSegment MyTestSegment4A(HGF2DLocation(10.0 - MYEPSILON, 1.0, pWorld), HGF2DLocation(10.0, 9.0, pWorld));

    HVE2DSegment MyTestSegment1B(HGF2DLocation(1.0, 10.0 + MYEPSILON, pWorld), HGF2DLocation(9.0, 10.0, pWorld));
    HVE2DSegment MyTestSegment2B(HGF2DLocation(1.0, 0.0 + MYEPSILON, pWorld), HGF2DLocation(9.0, 0.0, pWorld));
    HVE2DSegment MyTestSegment3B(HGF2DLocation(0.0 + MYEPSILON, 1.0, pWorld), HGF2DLocation(0.0, 9.0, pWorld));
    HVE2DSegment MyTestSegment4B(HGF2DLocation(10.0 + MYEPSILON, 1.0, pWorld), HGF2DLocation(10.0, 9.0, pWorld));

    HVE2DSegment MyTestSegment1C(HGF2DLocation(1.0, 10.0, pWorld), HGF2DLocation(9.0, 10.0 - MYEPSILON, pWorld));
    HVE2DSegment MyTestSegment2C(HGF2DLocation(1.0, 0.0, pWorld), HGF2DLocation(9.0, 0.0 - MYEPSILON, pWorld));
    HVE2DSegment MyTestSegment3C(HGF2DLocation(0.0, 1.0, pWorld), HGF2DLocation(0.0 - MYEPSILON, 9.0, pWorld));
    HVE2DSegment MyTestSegment4C(HGF2DLocation(10.0, 1.0, pWorld), HGF2DLocation(10.0 - MYEPSILON, 9.0, pWorld));

    HVE2DSegment MyTestSegment1D(HGF2DLocation(1.0, 10.0, pWorld), HGF2DLocation(9.0, 10.0 + MYEPSILON, pWorld));
    HVE2DSegment MyTestSegment2D(HGF2DLocation(1.0, 0.0, pWorld), HGF2DLocation(9.0, 0.0 + MYEPSILON, pWorld));
    HVE2DSegment MyTestSegment3D(HGF2DLocation(0.0, 1.0, pWorld), HGF2DLocation(0.0 + MYEPSILON, 9.0, pWorld));
    HVE2DSegment MyTestSegment4D(HGF2DLocation(10.0, 1.0, pWorld), HGF2DLocation(10.0 + MYEPSILON, 9.0, pWorld));

    HVE2DSegment MyTestSegment1E(HGF2DLocation(1.0, 10.0 - MYEPSILON, pWorld), HGF2DLocation(9.0, 10.0 - MYEPSILON, pWorld));
    HVE2DSegment MyTestSegment2E(HGF2DLocation(1.0, 0.0 - MYEPSILON, pWorld), HGF2DLocation(9.0, 0.0 - MYEPSILON, pWorld));
    HVE2DSegment MyTestSegment3E(HGF2DLocation(0.0 - MYEPSILON, 1.0, pWorld), HGF2DLocation(0.0 - MYEPSILON, 9.0, pWorld));
    HVE2DSegment MyTestSegment4E(HGF2DLocation(10.0 - MYEPSILON, 1.0, pWorld), HGF2DLocation(10.0 - MYEPSILON, 9.0, pWorld));

    HVE2DSegment MyTestSegment1F(HGF2DLocation(1.0, 10.0 + MYEPSILON, pWorld), HGF2DLocation(9.0, 10.0 - MYEPSILON, pWorld));
    HVE2DSegment MyTestSegment2F(HGF2DLocation(1.0, 0.0 + MYEPSILON, pWorld), HGF2DLocation(9.0, 0.0 - MYEPSILON, pWorld));
    HVE2DSegment MyTestSegment3F(HGF2DLocation(0.0 + MYEPSILON, 1.0, pWorld), HGF2DLocation(0.0 - MYEPSILON, 9.0, pWorld));
    HVE2DSegment MyTestSegment4F(HGF2DLocation(10.0 + MYEPSILON, 1.0, pWorld), HGF2DLocation(10.0 - MYEPSILON, 9.0, pWorld));

    HVE2DSegment MyTestSegment1G(HGF2DLocation(1.0, 10.0 - MYEPSILON, pWorld), HGF2DLocation(9.0, 10.0 + MYEPSILON, pWorld));
    HVE2DSegment MyTestSegment2G(HGF2DLocation(1.0, 0.0 - MYEPSILON, pWorld), HGF2DLocation(9.0, 0.0 + MYEPSILON, pWorld));
    HVE2DSegment MyTestSegment3G(HGF2DLocation(0.0 - MYEPSILON, 1.0, pWorld), HGF2DLocation(0.0 + MYEPSILON, 9.0, pWorld));
    HVE2DSegment MyTestSegment4G(HGF2DLocation(10.0 - MYEPSILON, 1.0, pWorld), HGF2DLocation(10.0 + MYEPSILON, 9.0, pWorld));

    HVE2DSegment MyTestSegment1H(HGF2DLocation(1.0, 10.0 + MYEPSILON, pWorld), HGF2DLocation(9.0, 10.0 + MYEPSILON, pWorld));
    HVE2DSegment MyTestSegment2H(HGF2DLocation(1.0, 0.0 + MYEPSILON, pWorld), HGF2DLocation(9.0, 0.0 + MYEPSILON, pWorld));
    HVE2DSegment MyTestSegment3H(HGF2DLocation(0.0 + MYEPSILON, 1.0, pWorld), HGF2DLocation(0.0 + MYEPSILON, 9.0, pWorld));
    HVE2DSegment MyTestSegment4H(HGF2DLocation(10.0 + MYEPSILON, 1.0, pWorld), HGF2DLocation(10.0 + MYEPSILON, 9.0, pWorld));

    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment1));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment2));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment3));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment4));

    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment1A));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment2A));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment3A));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment4A));

    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment1B));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment2B));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment3B));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment4B));

    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment1C));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment2C));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment3C));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment4C));

    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment1D));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment2D));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment3D));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment4D));

    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment1E));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment2E));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment3E));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment4E));

    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment1F));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment2F));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment3F));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment4F));

    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment1G));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment2G));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment3G));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment4G));

    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment1H));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment2H));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment3H));
    ASSERT_EQ(HVE2DShape::S_ON, MyRectangle.CalculateSpatialPositionOf(MyTestSegment4H));
   
    }

//==================================================================================
// FailedTestWithIntersect
//==================================================================================
TEST_F (HVE2DRectangleTest, FailedTestWithIntersect)
    {
   
    HVE2DRectangle MyRectangle(0.0, 0.0, 776.0, 530.0, pWorld);

    HVE2DPolySegment  AddPolySegment1(pWorld);
    AddPolySegment1.AppendPoint(HGF2DLocation(173881.594315156690, 31262.7623784915300 , pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-672196.87396625290, -119580.92306234955 , pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(-207684.53954652144, -37058.254313532634 , pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(403111.141213428460, 72700.3005556127460 , pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(173881.594315156690, 31262.7623784915300 , pWorld));


    HGF2DLocationCollection     crossPoints;
    MyRectangle.Intersect (AddPolySegment1, &crossPoints);

    }


