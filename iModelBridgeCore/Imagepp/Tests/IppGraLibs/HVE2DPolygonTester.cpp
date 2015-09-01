//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HVE2DPolygonTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "EnvironnementTest.h"
#include "HVE2DPolygonTester.h"

HVE2DPolygonTester::HVE2DPolygonTester() 
    {

    // Polygons
    PolyClosePoint1A = HGF2DLocation(21.1, 10.1, pWorld);
    PolyClosePoint1B = HGF2DLocation(9.00, 9.00, pWorld);
    PolyClosePoint1C = HGF2DLocation(19.9, 10.0, pWorld);
    PolyClosePoint1D = HGF2DLocation(0.10, 15.0, pWorld);

    PolyCloseMidPoint1 = HGF2DLocation(15.0, 20.1, pWorld);

    Poly1Point0d0 = HGF2DLocation(10.0, 10.0, pWorld);
    Poly1Point0d1 = HGF2DLocation(15.0, 10.0, pWorld);
    Poly1Point0d5 = HGF2DLocation(20.0, 20.0, pWorld);
    Poly1Point1d0 = HGF2DLocation(10.0, 10.0+(1.1 * MYEPSILON), pWorld);

    PolyMidPoint1 = HGF2DLocation(15.0, 20.0, pWorld);

    }

// THE PRESENT TEST CANNOT MAKE TESTS WITH NON-LINEAR TRANSFORMATION MODELS

//==================================================================================
// Polygon Construction tests
//==================================================================================
TEST_F (HVE2DPolygonTester, ConstructionTest)
    {

    // Default Constructor
    HVE2DPolygon    APoly1;

    // Constructor with a coordinate system
    HVE2DPolygon    APoly2(pWorld);
    ASSERT_EQ(pWorld, APoly2.GetCoordSys());

    //Constructor from ComplexLinear
    HVE2DComplexLinear LinearTest(pWorld);
    HVE2DPolygon PolyTest(LinearTest);
    ASSERT_EQ(pWorld, PolyTest.GetCoordSys());

    //Constructor from Rectangle
    HVE2DPolygon PolyTest2(Rect1);   
    ASSERT_EQ(pWorld, PolyTest2.GetCoordSys());
    ASSERT_TRUE(PolyTest2.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(PolyTest2.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(PolyTest2.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(PolyTest2.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

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

    HVE2DPolygon    APoly3(AComp1);
    HVE2DPolygon    APoly4(APoly3);

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

    ASSERT_EQ(pWorld, APoly4.GetCoordSys());

    }

//==================================================================================
// operator= test
// operator=(const HVE2DPolygon& pi_rObj);
//==================================================================================
TEST_F (HVE2DPolygonTester, OperatorTest)
    {
   
    HVE2DSegment    Segment1(HGF2DLocation(10.0, 10.1, pWorld), HGF2DLocation(10.0, 20.1, pWorld));
    HVE2DSegment    Segment2(HGF2DLocation(10.0, 20.1, pWorld), HGF2DLocation(20.0, 20.1, pWorld));
    HVE2DSegment    Segment3(HGF2DLocation(20.0, 20.1, pWorld), HGF2DLocation(20.0, 10.1, pWorld));
    HVE2DSegment    Segment4(HGF2DLocation(20.0, 10.1, pWorld), HGF2DLocation(10.0, 10.1, pWorld));

    HVE2DComplexLinear  AComp1(pWorld);
    AComp1.AppendLinear(Segment1);
    AComp1.AppendLinear(Segment2);
    AComp1.AppendLinear(Segment3);
    AComp1.AppendLinear(Segment4);

    HVE2DPolygon    APoly1(AComp1);
    HVE2DPolygon    APoly2(pWorld);

    APoly2 = APoly1;

    ASSERT_EQ(pWorld, APoly2.GetCoordSys());

    HVE2DComplexLinear  AComp2 = APoly2.GetLinear();
    ASSERT_EQ(pWorld, AComp2.GetCoordSys());

    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    #ifdef WIP_IPPBUG_22

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

    #endif

    }

//==================================================================================
// SetLinear( const HVE2DLinear& pi_rLinear )
//==================================================================================
TEST_F (HVE2DPolygonTester, SetLinearTest)
    {

    HVE2DPolygon    APoly1(pWorld);
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

    HVE2DComplexLinear  AComp2 = APoly1.GetLinear();
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
// GetLinear() tests
// GetLinear() const;
// GetLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const;
// AllocateLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const;
//==================================================================================
TEST_F (HVE2DPolygonTester, GetLinearTest)
    {
 
    // Polygons
    HVE2DPolygon        Poly1(Rect1);
    HVE2DComplexLinear  MyLinearOfPoly1(Poly1.GetLinear());

    // verify that there are 4 linears
    ASSERT_EQ(4, MyLinearOfPoly1.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPoly1.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPoly1.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPoly1.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPoly1.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPoly1.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPoly1.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPoly1.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPoly1.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPoly1.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPoly1.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPoly1.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPoly1.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, MyLinearOfPoly1.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, MyLinearOfPoly1.GetLinear(3).GetStartPoint().GetY());
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

    HFCPtr<HVE2DComplexLinear>  MyPtrLinearOfPolyCCW = Poly1.AllocateLinear(HVE2DSimpleShape::CCW);
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
TEST_F (HVE2DPolygonTester, CalculatePerimeterTest)
    {

    // Polygons
    HVE2DPolygon        Poly1(Rect1);

    // Test with linear 1
    ASSERT_DOUBLE_EQ(40.0, Poly1.CalculatePerimeter());

    }

//==================================================================================
// Area Calculation test
// CalculateArea() const;
//==================================================================================
TEST_F (HVE2DPolygonTester, CalculateAreaTest)
    {

    // Polygons
    HVE2DPolygon    Poly1(Rect1);
    HVE2DPolygon    NegativePoly(NegativeRect);

    ASSERT_DOUBLE_EQ(100.0, Poly1.CalculateArea());
    ASSERT_DOUBLE_EQ(100.0, NegativePoly.CalculateArea());
  
    }

//==================================================================================
// Drop( HGF2DLocationCollection* po_pPoints, const HGFDistance& pi_rTolerance ) const
//==================================================================================
TEST_F (HVE2DPolygonTester, DropTest)
    {

    HGF2DLocationCollection Locations;

    HVE2DPolygon APoly(Rect1);

    APoly.Drop(&Locations, MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Locations[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, Locations[0].GetY());
    ASSERT_DOUBLE_EQ(20.0, Locations[1].GetX());
    ASSERT_DOUBLE_EQ(10.0, Locations[1].GetY());
    ASSERT_DOUBLE_EQ(20.0, Locations[2].GetX());
    ASSERT_DOUBLE_EQ(20.0, Locations[2].GetY());
    ASSERT_DOUBLE_EQ(10.0, Locations[3].GetX());
    ASSERT_DOUBLE_EQ(20.0, Locations[3].GetY());
    ASSERT_DOUBLE_EQ(10.0, Locations[4].GetX());
    ASSERT_DOUBLE_EQ(10.0, Locations[4].GetY());

}

//==================================================================================
// Closest point calculation test
// CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
//==================================================================================
TEST_F (HVE2DPolygonTester, CalculateClosestPoint)
    {

    //Polygons
    HVE2DPolygon        Poly1(Rect1);

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
TEST_F (HVE2DPolygonTester, IntersectTest)
    {

    HVE2DPolygon        Poly1(Rect1);
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
    ASSERT_EQ(10.0, DumPoints[0].GetX());
    ASSERT_EQ(13.5, DumPoints[0].GetY());
    DumPoints.clear();

    ASSERT_EQ(1, Poly1.Intersect(ComplexLinearCase2, &DumPoints));
    ASSERT_EQ(10.0, DumPoints[0].GetX());
    ASSERT_EQ(15.0, DumPoints[0].GetY());
    DumPoints.clear();

    ASSERT_EQ(1, Poly1.Intersect(ComplexLinearCase3, &DumPoints));
    ASSERT_EQ(10.0, DumPoints[0].GetX());
    ASSERT_EQ(10.0, DumPoints[0].GetY());
    DumPoints.clear();

    ASSERT_EQ(0, Poly1.Intersect(ComplexLinearCase4, &DumPoints));
    DumPoints.clear();

    ASSERT_EQ(1, Poly1.Intersect(ComplexLinearCase5, &DumPoints));
    ASSERT_EQ(10.0, DumPoints[0].GetX());
    ASSERT_EQ(10.0, DumPoints[0].GetY());
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
//                           HGF2DLocationCollection* po_pContiguousnessPoints) const;
//   ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
//                                const HGF2DLocation& pi_rPoint,
//                             HGF2DLocation* pi_pFirstContiguousnessPoint,
//                             HGF2DLocation* pi_pSecondContiguousnessPoint) const;
//    HDLL virtual bool      AreContiguous(const HVE2DVector& pi_rVector) const;
//    HDLL virtual bool      AreContiguousAt(const HGF2DVector& pi_rVector,
//                                            const HVE2DLocation& pi_rPoint) const;
//==================================================================================
TEST_F (HVE2DPolygonTester, ContiguousnessTest)
    {

    HVE2DPolygon        Poly1(Rect1);
    HGF2DLocationCollection     DumPoints;
    HGF2DLocation   FirstDumPoint;
    HGF2DLocation   SecondDumPoint;

    // Test with contiguous linears
    ASSERT_TRUE(Poly1.AreContiguous(ComplexLinearCase6));
    ASSERT_TRUE(Poly1.AreContiguousAt(ComplexLinearCase6, PolyMidPoint1));
    ASSERT_EQ(2, Poly1.ObtainContiguousnessPoints(ComplexLinearCase6, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[1].GetY());

    Poly1.ObtainContiguousnessPointsAt(ComplexLinearCase6, LinearMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(10.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, SecondDumPoint.GetY());

    DumPoints.clear();

    // Test with non contiguous linears
    ASSERT_FALSE(Poly1.AreContiguous(ComplexLinearCase1));
    
    // Test with contiguous Polygon
    HVE2DPolygon      NorthContiguousPoly(NorthContiguousRect);
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

    HVE2DPolygon      VerticalFitPoly(VerticalFitRect);
    ASSERT_TRUE(Poly1.AreContiguous(VerticalFitPoly));
    ASSERT_TRUE(Poly1.AreContiguousAt(VerticalFitPoly, HGF2DLocation(17.0, 10.0, pWorld)));
    ASSERT_EQ(4, Poly1.ObtainContiguousnessPoints(VerticalFitPoly, &DumPoints));
    ASSERT_DOUBLE_EQ(15.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetY());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[2].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[2].GetY());
    ASSERT_DOUBLE_EQ(15.0, DumPoints[3].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[3].GetY());

    Poly1.ObtainContiguousnessPointsAt(VerticalFitPoly, HGF2DLocation(17.0, 10.0, pWorld), &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(15.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(20.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetY());

    DumPoints.clear();

    HVE2DPolygon      IncludedPoly1(IncludedRect1);
    ASSERT_TRUE(Poly1.AreContiguous(IncludedPoly1));
    ASSERT_TRUE(Poly1.AreContiguousAt(IncludedPoly1, HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_EQ(2, Poly1.ObtainContiguousnessPoints(IncludedPoly1, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(15.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(15.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetY());

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
TEST_F (HVE2DPolygonTester, CloneTest) 
    {

    //General Clone Test
    HVE2DPolygon        Poly1(Rect1);
    HFCPtr<HVE2DPolygon> pClone = (HVE2DPolygon*)Poly1.Clone();
    ASSERT_FALSE(pClone->IsEmpty());
    ASSERT_EQ(pWorld, pClone->GetCoordSys());
    ASSERT_EQ(HVE2DPolygon::CLASS_ID, pClone->GetShapeType());

    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));  
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld))); 
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld))); 
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld))); 

    // Test with the same coordinate system
    HFCPtr<HVE2DPolygon> pClone3 = (HVE2DPolygon*)Poly1.AllocateCopyInCoordSys(pWorld);
    ASSERT_FALSE(pClone3->IsEmpty());
    ASSERT_EQ(pWorld, pClone3->GetCoordSys());
    ASSERT_EQ(HVE2DPolygon::CLASS_ID, pClone3->GetShapeType());

    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));  
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld))); 
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld))); 
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld))); 

    // Test with a translation between systems
    Translation = HGF2DDisplacement (10.0, 10.0);
    HGF2DTranslation myTranslation(Translation);
    HFCPtr<HGF2DCoordSys> pWorldTranslation = new HGF2DCoordSys(myTranslation, pWorld);

    HFCPtr<HVE2DPolygon> pClone5 = (HVE2DPolygon*)Poly1.AllocateCopyInCoordSys(pWorldTranslation);
    ASSERT_FALSE(pClone5->IsEmpty());
    ASSERT_EQ(pWorldTranslation, pClone5->GetCoordSys());
    ASSERT_EQ(HVE2DPolygon::CLASS_ID, pClone5->GetShapeType());

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

    HFCPtr<HVE2DPolygon> pClone6 = (HVE2DPolygon*)Poly1.AllocateCopyInCoordSys(pWorldStretch);
    ASSERT_FALSE(pClone6->IsEmpty());
    ASSERT_EQ(pWorldStretch, pClone6->GetCoordSys());
    ASSERT_EQ(HVE2DPolygon::CLASS_ID, pClone6->GetShapeType());

    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(0.0, 0.0, pWorldStretch)));  
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(0.0, 20.0, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(20.0, 20.0, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(20.0, 0.0, pWorldStretch))); 

    // Test with a similitude between systems
    HGF2DSimilitude mySimilitude;
    mySimilitude.SetRotation(PI);
    mySimilitude.SetScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldSimilitude = new HGF2DCoordSys(mySimilitude, pWorld);

    HFCPtr<HVE2DPolygon> pClone7 = (HVE2DPolygon*)Poly1.AllocateCopyInCoordSys(pWorldSimilitude);
    ASSERT_FALSE(pClone7->IsEmpty());
    ASSERT_EQ(pWorldSimilitude, pClone7->GetCoordSys());
    ASSERT_EQ(HVE2DPolygon::CLASS_ID, pClone7->GetShapeType());

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

    HFCPtr<HVE2DPolygon> pClone8 = (HVE2DPolygon*)Poly1.AllocateCopyInCoordSys(pWorldAffine);
    ASSERT_FALSE(pClone8->IsEmpty());
    ASSERT_EQ(pWorldAffine, pClone8->GetCoordSys());
    ASSERT_EQ(HVE2DPolygon::CLASS_ID, pClone8->GetShapeType());

    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(0.000, 0.000, pWorldAffine)));  
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(0.000, -20.0, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-20.0, -20.0, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-20.0, 0.000, pWorldAffine))); 
        
    }

//==================================================================================
// Interaction info with other segment
// Crosses(const HVE2DVector& pi_rVector) const;
// AreAdjacent(const HVE2DVector& pi_rVector) const;
// IsPointOn(const HGF2DLocation& pi_rTestPoint) const;
//==================================================================================
TEST_F (HVE2DPolygonTester, InteractionTest)
    {

    HVE2DPolygon        Poly1(Rect1);

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
TEST_F (HVE2DPolygonTester, CalculateBearingTest)
    {

    HVE2DPolygon        Poly1(Rect1);

    // Obtain bearing ALPHA of a vertical segment
    ASSERT_DOUBLE_EQ(1.5707963267948966, Poly1.CalculateBearing(Poly1Point0d0, HVE2DVector::ALPHA).GetAngle());
    ASSERT_NEAR(0.0, Poly1.CalculateBearing(Poly1Point0d0, HVE2DVector::BETA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(3.1415926535897931, Poly1.CalculateBearing(Poly1Point0d1, HVE2DVector::ALPHA).GetAngle());
    ASSERT_NEAR(0.0, Poly1.CalculateBearing(Poly1Point0d1, HVE2DVector::BETA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(4.7123889803846897, Poly1.CalculateBearing(Poly1Point0d5, HVE2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(3.1415926535897931, Poly1.CalculateBearing(Poly1Point0d5, HVE2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(1.5707963267948966, Poly1.CalculateBearing(Poly1Point1d0, HVE2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(4.7123889803846897, Poly1.CalculateBearing(Poly1Point1d0, HVE2DVector::BETA).GetAngle());

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
TEST_F (HVE2DPolygonTester, GetExtentTest)
    {

    HVE2DPolygon        Poly1(Rect1);

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
TEST_F (HVE2DPolygonTester, EmptyTest)
    {

    HVE2DPolygon        Poly1(Rect1);
    HVE2DPolygon  MyOtherPoly(Poly1);

    ASSERT_FALSE(MyOtherPoly.IsEmpty());
    
    MyOtherPoly.MakeEmpty();

    ASSERT_TRUE(MyOtherPoly.IsEmpty());
 
    }

//==================================================================================
// GetShapeType() 
//==================================================================================
TEST_F (HVE2DPolygonTester, GetShapeType)
    {
    
    HVE2DPolygon        Poly1(Rect1);

    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), Poly1.GetShapeType());

    }

//==================================================================================
// IsPointIn()
// IsPointOn()
// IsPointOnSCS()
//==================================================================================
TEST_F (HVE2DPolygonTester, IsPointTest)
    {

    HVE2DPolygon        Poly1(Rect1);

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

    ASSERT_TRUE(Poly1.IsPointOnSCS(HGF2DLocation(10.0+0.9*MYEPSILON, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOnSCS(HGF2DLocation(15.0, 20.0-0.9*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOnSCS(HGF2DLocation(20.0-0.9*MYEPSILON, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Poly1.IsPointOnSCS(HGF2DLocation(15.0, 10.0+0.9*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_FALSE(Poly1.IsPointOnSCS(HGF2DLocation(10.0+1.1*MYEPSILON, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1.IsPointOnSCS(HGF2DLocation(15.0, 20.0-1.1*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1.IsPointOnSCS(HGF2DLocation(20.0-1.1*MYEPSILON, 15.0, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Poly1.IsPointOnSCS(HGF2DLocation(15.0, 10.0+1.1*MYEPSILON, pWorld), HVE2DVector::EXCLUDE_EXTREMITIES));

    }

//==================================================================================
// Spatial oriented operations UNIFY
//==================================================================================
TEST_F (HVE2DPolygonTester, UnifyShapeTest)
    {

    HVE2DComplexLinear  AComp2;
    HVE2DPolygon        Poly1(Rect1);

    HVE2DPolygon      NorthContiguousPoly(NorthContiguousRect);
    HFCPtr<HVE2DPolygon>     pResultShape1 = (HVE2DPolygon*) Poly1.UnifyShape(NorthContiguousPoly);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape1->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape1->GetCoordSys());

    AComp2 = pResultShape1->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(6, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(3).GetStartPoint().GetY());
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

    HVE2DPolygon      EastContiguousPoly(EastContiguousRect);
    HFCPtr<HVE2DPolygon>     pResultShape2 = (HVE2DPolygon*)Poly1.UnifyShape(EastContiguousPoly);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape2->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape2->GetCoordSys());
        
    AComp2 = pResultShape2->GetLinear(HVE2DSimpleShape::CW);
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
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(4).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(4).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(4).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(4).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(5).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetEndPoint().GetY());

    HVE2DPolygon      WestContiguousPoly(WestContiguousRect);
    HFCPtr<HVE2DPolygon>     pResultShape3 = (HVE2DPolygon*) Poly1.UnifyShape(WestContiguousPoly);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape3->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape3->GetCoordSys());
        
    AComp2 = pResultShape3->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(6, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_NEAR(0.0, AComp2.GetLinear(3).GetEndPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    ASSERT_NEAR(0.0, AComp2.GetLinear(4).GetStartPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(4).GetStartPoint().GetY());
    ASSERT_NEAR(0.0, AComp2.GetLinear(4).GetEndPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(4).GetEndPoint().GetY());

    ASSERT_NEAR(0.0, AComp2.GetLinear(5).GetStartPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(5).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(5).GetEndPoint().GetY());

    HVE2DPolygon      SouthContiguousPoly(SouthContiguousRect);
    HFCPtr<HVE2DPolygon>     pResultShape4 = (HVE2DPolygon*) Poly1.UnifyShape(SouthContiguousPoly);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape4->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape4->GetCoordSys());

    AComp2 = pResultShape4->GetLinear(HVE2DSimpleShape::CW);
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
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_NEAR(0.0, AComp2.GetLinear(3).GetEndPoint().GetY(), MYEPSILON);

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(4).GetStartPoint().GetX());
    ASSERT_NEAR(0.0, AComp2.GetLinear(4).GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(4).GetEndPoint().GetX());
    ASSERT_NEAR(0.0, AComp2.GetLinear(4).GetEndPoint().GetY(), MYEPSILON);

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetStartPoint().GetX());
    ASSERT_NEAR(0.0, AComp2.GetLinear(5).GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetEndPoint().GetY());

    HVE2DPolygon      VerticalFitPoly(VerticalFitRect);
    HFCPtr<HVE2DPolygon>    pResultShape5 = (HVE2DPolygon*) Poly1.UnifyShape(VerticalFitPoly);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape5->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape5->GetCoordSys());

    AComp2 = pResultShape5->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(8, AComp2.GetNumberOfLinears());

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
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(4).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(4).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(4).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(4).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(5).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(5).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(6).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(6).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(6).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(6).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(7).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(7).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(7).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(7).GetEndPoint().GetY());

    HVE2DPolygon      HorizontalFitPoly(HorizontalFitRect);
    HFCPtr<HVE2DPolygon>    pResultShape6 = (HVE2DPolygon*) Poly1.UnifyShape(HorizontalFitPoly);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape6->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape6->GetCoordSys());

    AComp2 = pResultShape6->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(8, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(4).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(4).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(4).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(4).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(5).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(5).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(5).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(5).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(6).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(6).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(6).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(6).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(7).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(7).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(7).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(7).GetEndPoint().GetY());

    HVE2DPolygon      DisjointPoly(DisjointRect);
    HFCPtr<HVE2DPolygon>    pResultShape7 = (HVE2DPolygon*) Poly1.UnifyShape(DisjointPoly);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape7->GetShapeType());
    ASSERT_TRUE(pResultShape7->IsComplex());
    ASSERT_FALSE(pResultShape7->HasHoles());

    HVE2DPolygon      MiscPoly1(MiscRect1);
    HFCPtr<HVE2DPolygon>    pResultShape8 = (HVE2DPolygon*) Poly1.UnifyShape(MiscPoly1);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape8->GetShapeType());
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
    ASSERT_DOUBLE_EQ(5.00, AComp2.GetLinear(4).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(5).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.00, AComp2.GetLinear(5).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.00, AComp2.GetLinear(5).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(5.00, AComp2.GetLinear(5).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(5.00, AComp2.GetLinear(6).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.00, AComp2.GetLinear(6).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.00, AComp2.GetLinear(6).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(6).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(5.00, AComp2.GetLinear(7).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(7).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(7).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(7).GetEndPoint().GetY());

    HVE2DPolygon      EnglobPoly1(EnglobRect1);
    HFCPtr<HVE2DPolygon>    pResultShape9 = (HVE2DPolygon*) Poly1.UnifyShape(EnglobPoly1);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape9->GetShapeType());
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

    HVE2DPolygon      EnglobPoly2(EnglobRect2);
    HFCPtr<HVE2DPolygon>    pResultShape10 = (HVE2DPolygon*) Poly1.UnifyShape(EnglobPoly2);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape10->GetShapeType());
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

    HVE2DPolygon      EnglobPoly3(EnglobRect3);
    HFCPtr<HVE2DPolygon>    pResultShape11 = (HVE2DPolygon*) Poly1.UnifyShape(EnglobPoly3);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape11->GetShapeType());       
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

    HVE2DPolygon      IncludedPoly1(IncludedRect1);
    HFCPtr<HVE2DPolygon>    pResultShape12 = (HVE2DPolygon*) Poly1.UnifyShape(IncludedPoly1);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape12->GetShapeType());
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

    HVE2DPolygon      IncludedPoly2(IncludedRect2);
    HFCPtr<HVE2DPolygon>    pResultShape13 = (HVE2DPolygon*) Poly1.UnifyShape(IncludedPoly2);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape13->GetShapeType());
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

    HVE2DPolygon      IncludedPoly3(IncludedRect3);
    HFCPtr<HVE2DPolygon>    pResultShape14 = (HVE2DPolygon*) Poly1.UnifyShape(IncludedPoly3);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape14->GetShapeType());
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

    HVE2DPolygon      IncludedPoly4(IncludedRect4);
    HFCPtr<HVE2DPolygon>    pResultShape15 = (HVE2DPolygon*) Poly1.UnifyShape(IncludedPoly4);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape15->GetShapeType());
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

    HVE2DPolygon      IncludedPoly5(IncludedRect5);
    HFCPtr<HVE2DPolygon>    pResultShape16 = (HVE2DPolygon*)Poly1.UnifyShape(IncludedPoly5);
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

    HVE2DPolygon      IncludedPoly6(IncludedRect6);
    HFCPtr<HVE2DPolygon>    pResultShape17 = (HVE2DPolygon*)Poly1.UnifyShape(IncludedPoly6);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape17->GetShapeType());
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

    HVE2DPolygon      IncludedPoly7(IncludedRect7);
    HFCPtr<HVE2DPolygon>    pResultShape18 = (HVE2DPolygon*)Poly1.UnifyShape(IncludedPoly7);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape18->GetShapeType());
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

    HVE2DPolygon      IncludedPoly8(IncludedRect8);
    HFCPtr<HVE2DPolygon>    pResultShape19 = (HVE2DPolygon*)Poly1.UnifyShape(IncludedPoly8);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape19->GetShapeType());
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

    HVE2DPolygon      IncludedPoly9(IncludedRect9);
    HFCPtr<HVE2DPolygon>     pResultShape20 = (HVE2DPolygon*) Poly1.UnifyShape(IncludedPoly9);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape20->GetShapeType());
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

    }

//==================================================================================
// Spatial oriented operations INTERSECT
//==================================================================================
TEST_F (HVE2DPolygonTester, IntersectShapeTest)
    {

    HVE2DComplexLinear  AComp2;
    HVE2DPolygon        Poly1(Rect1);
    
    HVE2DPolygon      NorthContiguousPoly(NorthContiguousRect);
    HFCPtr<HVE2DShape>     pResultShape1 = Poly1.IntersectShape(NorthContiguousPoly);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape1->GetShapeType());
    ASSERT_TRUE(pResultShape1->IsEmpty());

    HVE2DPolygon      EastContiguousPoly(EastContiguousRect);
    HFCPtr<HVE2DShape>     pResultShape2 = Poly1.IntersectShape(EastContiguousPoly);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape2->GetShapeType());
    ASSERT_TRUE(pResultShape2->IsEmpty());

    HVE2DPolygon      WestContiguousPoly(WestContiguousRect);
    HFCPtr<HVE2DShape>     pResultShape3 = Poly1.IntersectShape(WestContiguousPoly);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape3->GetShapeType());
    ASSERT_TRUE(pResultShape3->IsEmpty());

    HVE2DPolygon      SouthContiguousPoly(SouthContiguousRect);
    HFCPtr<HVE2DShape>     pResultShape4 = Poly1.IntersectShape(SouthContiguousPoly);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape4->GetShapeType());
    ASSERT_TRUE(pResultShape4->IsEmpty());

    HVE2DPolygon      VerticalFitPoly(VerticalFitRect);
    HFCPtr<HVE2DPolygon>     pResultShape5 = (HVE2DPolygon*) Poly1.IntersectShape(VerticalFitPoly);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape5->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape5->GetCoordSys());
    AComp2 = pResultShape5->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HVE2DPolygon      HorizontalFitPoly(HorizontalFitRect);
    HFCPtr<HVE2DPolygon>     pResultShape6 = (HVE2DPolygon*) Poly1.IntersectShape(HorizontalFitPoly);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape6->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape6->GetCoordSys());
    AComp2 = pResultShape6->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HVE2DPolygon      DisjointPoly(DisjointRect);
    HFCPtr<HVE2DShape>     pResultShape7 = Poly1.IntersectShape(DisjointPoly);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape7->GetShapeType());
    ASSERT_TRUE(pResultShape7->IsEmpty());

    HVE2DPolygon      MiscPoly1(MiscRect1);
    HFCPtr<HVE2DPolygon>     pResultShape8 = (HVE2DPolygon*) Poly1.IntersectShape(MiscPoly1);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape8->GetShapeType());
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

    HVE2DPolygon      EnglobPoly1(EnglobRect1);
    HFCPtr<HVE2DPolygon>     pResultShape9 = (HVE2DPolygon*) Poly1.IntersectShape(EnglobPoly1);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape9->GetShapeType());
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

    HVE2DPolygon      EnglobPoly2(EnglobRect2);
    HFCPtr<HVE2DPolygon>     pResultShape10 = (HVE2DPolygon*) Poly1.IntersectShape(EnglobPoly2);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape10->GetShapeType());
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

    HVE2DPolygon      EnglobPoly3(EnglobRect3);
    HFCPtr<HVE2DPolygon>     pResultShape11 = (HVE2DPolygon*) Poly1.IntersectShape(EnglobPoly3);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape11->GetShapeType());
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

    HVE2DPolygon      IncludedPoly1(IncludedRect1);
    HFCPtr<HVE2DPolygon>     pResultShape12 = (HVE2DPolygon*) Poly1.IntersectShape(IncludedPoly1);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape12->GetShapeType());
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

    HVE2DPolygon      IncludedPoly2(IncludedRect2);
    HFCPtr<HVE2DPolygon>     pResultShape13 = (HVE2DPolygon*) Poly1.IntersectShape(IncludedPoly2);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape13->GetShapeType());
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

    HVE2DPolygon      IncludedPoly3(IncludedRect3);
    HFCPtr<HVE2DPolygon>     pResultShape14 = (HVE2DPolygon*) Poly1.IntersectShape(IncludedPoly3);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape14->GetShapeType());
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

    HVE2DPolygon      IncludedPoly4(IncludedRect4);
    HFCPtr<HVE2DPolygon>     pResultShape15 = (HVE2DPolygon*) Poly1.IntersectShape(IncludedPoly4);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape15->GetShapeType());
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

    HVE2DPolygon      IncludedPoly5(IncludedRect5);
    HFCPtr<HVE2DPolygon>     pResultShape16 = (HVE2DPolygon*)Poly1.IntersectShape(IncludedPoly5);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape16->GetShapeType());
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

    HVE2DPolygon      IncludedPoly6(IncludedRect6);
    HFCPtr<HVE2DPolygon>     pResultShape17 = (HVE2DPolygon*)Poly1.IntersectShape(IncludedPoly6);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape17->GetShapeType());
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

    HVE2DPolygon      IncludedPoly7(IncludedRect7);
    HFCPtr<HVE2DPolygon>     pResultShape18 = (HVE2DPolygon*) Poly1.IntersectShape(IncludedPoly7);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape18->GetShapeType());
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

    HVE2DPolygon      IncludedPoly8(IncludedRect8);
    HFCPtr<HVE2DPolygon>     pResultShape19 = (HVE2DPolygon*) Poly1.IntersectShape(IncludedPoly8);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape19->GetShapeType());
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

    HVE2DPolygon      IncludedPoly9(IncludedRect9);
    HFCPtr<HVE2DPolygon>     pResultShape20 = (HVE2DPolygon*) Poly1.IntersectShape(IncludedPoly9);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape20->GetShapeType());
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

    }

//==================================================================================
// Spatial oriented operations DIFF
//==================================================================================
TEST_F (HVE2DPolygonTester, DifferentiateShapeTest)
    {

    HVE2DComplexLinear  AComp2;
    HVE2DPolygon        Poly1(Rect1);

    HVE2DPolygon      NorthContiguousPoly(NorthContiguousRect);  
    HFCPtr<HVE2DPolygon>     pResultShape1 = (HVE2DPolygon*) Poly1.DifferentiateShape(NorthContiguousPoly);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape1->GetShapeType());
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

    HVE2DPolygon      EastContiguousPoly(EastContiguousRect);
    HFCPtr<HVE2DPolygon>     pResultShape2 = (HVE2DPolygon*) Poly1.DifferentiateShape(EastContiguousPoly);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape2->GetShapeType());
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

    HVE2DPolygon      WestContiguousPoly(WestContiguousRect);
    HFCPtr<HVE2DPolygon>     pResultShape3 = (HVE2DPolygon*)Poly1.DifferentiateShape(WestContiguousPoly);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape3->GetShapeType());
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

    HVE2DPolygon      SouthContiguousPoly(SouthContiguousRect);
    HFCPtr<HVE2DPolygon>     pResultShape4 = (HVE2DPolygon*) Poly1.DifferentiateShape(SouthContiguousPoly);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape4->GetShapeType());
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

    HVE2DPolygon      VerticalFitPoly(VerticalFitRect);
    HFCPtr<HVE2DPolygon>     pResultShape5 = (HVE2DPolygon*)Poly1.DifferentiateShape(VerticalFitPoly);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape5->GetShapeType());
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

    HVE2DPolygon      HorizontalFitPoly(HorizontalFitRect);
    HFCPtr<HVE2DPolygon>     pResultShape6 = (HVE2DPolygon*)Poly1.DifferentiateShape(HorizontalFitPoly);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape6->GetShapeType());
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

    HVE2DPolygon      DisjointPoly(DisjointRect);
    HFCPtr<HVE2DPolygon>     pResultShape7 = (HVE2DPolygon*) Poly1.DifferentiateShape(DisjointPoly);
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

    HVE2DPolygon      MiscPoly1(MiscRect1);
    HFCPtr<HVE2DPolygon>     pResultShape8 = (HVE2DPolygon*) Poly1.DifferentiateShape(MiscPoly1);
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

    HVE2DPolygon      EnglobPoly1(EnglobRect1);
    HFCPtr<HVE2DShape>     pResultShape9 = Poly1.DifferentiateShape(EnglobPoly1);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape9->GetShapeType());
    ASSERT_TRUE(pResultShape9->IsEmpty());

    HVE2DPolygon      EnglobPoly2(EnglobRect2);
    HFCPtr<HVE2DShape>     pResultShape10 = Poly1.DifferentiateShape(EnglobPoly2);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape10->GetShapeType());
    ASSERT_TRUE(pResultShape10->IsEmpty());

    HVE2DPolygon      EnglobPoly3(EnglobRect3);
    HFCPtr<HVE2DShape>     pResultShape11 = Poly1.DifferentiateShape(EnglobPoly3);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape11->GetShapeType());
    ASSERT_TRUE(pResultShape11->IsEmpty());

    HVE2DPolygon      IncludedPoly1(IncludedRect1);
    HFCPtr<HVE2DPolygon>     pResultShape12 = (HVE2DPolygon*) Poly1.DifferentiateShape(IncludedPoly1);
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

    HVE2DPolygon      IncludedPoly2(IncludedRect2);
    HFCPtr<HVE2DPolygon>     pResultShape13 = (HVE2DPolygon*) Poly1.DifferentiateShape(IncludedPoly2);
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

    HVE2DPolygon      IncludedPoly3(IncludedRect3);
    HFCPtr<HVE2DPolygon>     pResultShape14 = (HVE2DPolygon*)Poly1.DifferentiateShape(IncludedPoly3);
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

    HVE2DPolygon      IncludedPoly4(IncludedRect4);
    HFCPtr<HVE2DPolygon>     pResultShape15 = (HVE2DPolygon*) Poly1.DifferentiateShape(IncludedPoly4);
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

    HVE2DPolygon      IncludedPoly5(IncludedRect5);
    HFCPtr<HVE2DShape>     pResultShape16 = Poly1.DifferentiateShape(IncludedPoly5);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape16->GetShapeType());
    ASSERT_TRUE(pResultShape16->HasHoles());

    HVE2DPolygon      IncludedPoly6(IncludedRect6);
    HFCPtr<HVE2DPolygon>     pResultShape17 = (HVE2DPolygon*) Poly1.DifferentiateShape(IncludedPoly6);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape17->GetShapeType());
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

    HVE2DPolygon      IncludedPoly7(IncludedRect7);
    HFCPtr<HVE2DPolygon>     pResultShape18 = (HVE2DPolygon*) Poly1.DifferentiateShape(IncludedPoly7);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape18->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape18->GetCoordSys());

    AComp2 = pResultShape18->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HVE2DPolygon      IncludedPoly8(IncludedRect8);
    HFCPtr<HVE2DPolygon>     pResultShape19 = (HVE2DPolygon*) Poly1.DifferentiateShape(IncludedPoly8);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape19->GetShapeType());
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

    HVE2DPolygon      IncludedPoly9(IncludedRect9);
    HFCPtr<HVE2DPolygon>     pResultShape20 = (HVE2DPolygon*) Poly1.DifferentiateShape(IncludedPoly9);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape20->GetShapeType());
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
// Spatial oriented operations DIFF
//==================================================================================
TEST_F (HVE2DPolygonTester, DifferentiateFromShapeSCSTest)
    {

    HVE2DComplexLinear  AComp2;
    HVE2DPolygon        Poly1(Rect1);

    HVE2DPolygon      NorthContiguousPoly(NorthContiguousRect);
    HFCPtr<HVE2DPolygon>     pResultShape1 = (HVE2DPolygon*) Poly1.DifferentiateFromShapeSCS(NorthContiguousPoly);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape1->GetShapeType());
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

    HVE2DPolygon      EastContiguousPoly(EastContiguousRect);
    HFCPtr<HVE2DPolygon>     pResultShape2 = (HVE2DPolygon*) Poly1.DifferentiateFromShapeSCS(EastContiguousPoly);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape2->GetShapeType());
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

    HVE2DPolygon      WestContiguousPoly(WestContiguousRect);
    HFCPtr<HVE2DPolygon>     pResultShape3 = (HVE2DPolygon*) Poly1.DifferentiateFromShapeSCS(WestContiguousPoly);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape3->GetShapeType());
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

    HVE2DPolygon      SouthContiguousPoly(SouthContiguousRect);
    HFCPtr<HVE2DPolygon>     pResultShape4 = (HVE2DPolygon*) Poly1.DifferentiateFromShapeSCS(SouthContiguousPoly);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape4->GetShapeType());
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

    HVE2DPolygon      VerticalFitPoly(VerticalFitRect);
    HFCPtr<HVE2DPolygon>     pResultShape5 = (HVE2DPolygon*) Poly1.DifferentiateFromShapeSCS(VerticalFitPoly);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape5->GetShapeType());
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

    HVE2DPolygon      HorizontalFitPoly(HorizontalFitRect);
    HFCPtr<HVE2DPolygon>     pResultShape6 = (HVE2DPolygon*) Poly1.DifferentiateFromShapeSCS(HorizontalFitPoly);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape6->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape6->GetCoordSys());

    AComp2 = pResultShape6->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HVE2DPolygon      DisjointPoly(DisjointRect);
    HFCPtr<HVE2DPolygon>     pResultShape7 = (HVE2DPolygon*) Poly1.DifferentiateFromShapeSCS(DisjointPoly);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape7->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape7->GetCoordSys());

    AComp2 = pResultShape7->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears()) ;

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

    HVE2DPolygon      MiscPoly1(MiscRect1);
    HFCPtr<HVE2DPolygon>     pResultShape8 = (HVE2DPolygon*) Poly1.DifferentiateFromShapeSCS(MiscPoly1);
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
    ASSERT_DOUBLE_EQ(5.00, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.00, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.00, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(5.00, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(5.00, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.00, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.00, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(5.00, AComp2.GetLinear(4).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(4).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(4).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(4).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(15.0, AComp2.GetLinear(5).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(5).GetEndPoint().GetY());

    HVE2DPolygon      EnglobPoly1(EnglobRect1);
    HFCPtr<HVE2DPolygon>     pResultShape9 = (HVE2DPolygon*) Poly1.DifferentiateFromShapeSCS(EnglobPoly1);
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

    HVE2DPolygon      EnglobPoly2(EnglobRect2);
    HFCPtr<HVE2DShape>     pResultShape10 = Poly1.DifferentiateFromShapeSCS(EnglobPoly2);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape10->GetShapeType());
    ASSERT_TRUE(pResultShape10->HasHoles());

    HVE2DPolygon      EnglobPoly3(EnglobRect3);
    HFCPtr<HVE2DPolygon>     pResultShape11 = (HVE2DPolygon*) Poly1.DifferentiateFromShapeSCS(EnglobPoly3);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResultShape11->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape11->GetCoordSys());

    AComp2 = pResultShape11->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp2.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(10.0, AComp2.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(30.0, AComp2.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, AComp2.GetLinear(3).GetEndPoint().GetY());

    HVE2DPolygon      IncludedPoly1(IncludedRect1);
    HFCPtr<HVE2DShape>     pResultShape12 = Poly1.DifferentiateFromShapeSCS(IncludedPoly1);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape12->GetShapeType());
    ASSERT_TRUE(pResultShape12->IsEmpty());

    HVE2DPolygon      IncludedPoly2(IncludedRect2);
    HFCPtr<HVE2DShape>     pResultShape13 = Poly1.DifferentiateFromShapeSCS(IncludedPoly2);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape13->GetShapeType());
    ASSERT_TRUE(pResultShape13->IsEmpty());

    HVE2DPolygon      IncludedPoly3(IncludedRect3);
    HFCPtr<HVE2DShape>     pResultShape14 = Poly1.DifferentiateFromShapeSCS(IncludedPoly3);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape14->GetShapeType());
    ASSERT_TRUE(pResultShape14->IsEmpty());

    HVE2DPolygon      IncludedPoly4(IncludedRect4);
    HFCPtr<HVE2DShape>     pResultShape15 = Poly1.DifferentiateFromShapeSCS(IncludedPoly4);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape15->GetShapeType());
    ASSERT_TRUE(pResultShape15->IsEmpty());

    HVE2DPolygon      IncludedPoly5(IncludedRect5);
    HFCPtr<HVE2DShape>     pResultShape16 = Poly1.DifferentiateFromShapeSCS(IncludedPoly5);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape16->GetShapeType());
    ASSERT_TRUE(pResultShape16->IsEmpty());

    HVE2DPolygon      IncludedPoly6(IncludedRect6);
    HFCPtr<HVE2DShape>     pResultShape17 = Poly1.DifferentiateFromShapeSCS(IncludedPoly6);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape17->GetShapeType());
    ASSERT_TRUE(pResultShape17->IsEmpty());

    HVE2DPolygon      IncludedPoly7(IncludedRect7);
    HFCPtr<HVE2DShape>     pResultShape18 = Poly1.DifferentiateFromShapeSCS(IncludedPoly7);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape18->GetShapeType());
    ASSERT_TRUE(pResultShape18->IsEmpty());

    HVE2DPolygon      IncludedPoly8(IncludedRect8);
    HFCPtr<HVE2DShape>     pResultShape19 = Poly1.DifferentiateFromShapeSCS(IncludedPoly8);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape19->GetShapeType());
    ASSERT_TRUE(pResultShape19->IsEmpty());

    HVE2DPolygon      IncludedPoly9(IncludedRect9);
    HFCPtr<HVE2DShape>     pResultShape20 = Poly1.DifferentiateFromShapeSCS(IncludedPoly9);
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape20->GetShapeType());
    ASSERT_TRUE(pResultShape20->IsEmpty());

    }

//==================================================================================
// Move(const HGF2DDisplacement& pi_rDisplacement);
//==================================================================================
TEST_F (HVE2DPolygonTester, MoveTest)
    {

    HGF2DDisplacement Translation1(10.0, 10.0);
    HGF2DDisplacement Translation2(0.0, 10.0);
    HGF2DDisplacement Translation3(10.0, 0.0);
    HGF2DDisplacement Translation4(0.0, 0.0);
    HGF2DDisplacement Translation5(0.0, -10.0);
    HGF2DDisplacement Translation6(-10.0, 0.0);
    HGF2DDisplacement Translation7(-10.0, -10.0);
    HGF2DDisplacement Translation8(10.0, -10.0);
    
    HVE2DPolygon Poly1(IncludedRect1);
    Poly1.Move(Translation1);

    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(25.0, 25.0, pWorld)));
 
    HVE2DPolygon Poly2(IncludedRect2);
    Poly2.Move(Translation2);
    
    ASSERT_TRUE(Poly2.IsPointOn(HGF2DLocation(15.0, 20.0, pWorld)));
    ASSERT_TRUE(Poly2.IsPointOn(HGF2DLocation(20.0, 25.0, pWorld)));

    HVE2DPolygon Poly3(IncludedRect3);
    Poly3.Move(Translation3);

    ASSERT_TRUE(Poly3.IsPointOn(HGF2DLocation(25.0, 15.0, pWorld)));
    ASSERT_TRUE(Poly3.IsPointOn(HGF2DLocation(30.0, 20.0, pWorld)));
   
    HVE2DPolygon Poly4(IncludedRect4);
    Poly4.Move(Translation4);
 
    ASSERT_TRUE(Poly4.IsPointOn(HGF2DLocation(10.0, 15.0, pWorld)));
    ASSERT_TRUE(Poly4.IsPointOn(HGF2DLocation(15.0, 20.0, pWorld)));
    
    HVE2DPolygon Poly5(IncludedRect5);
    Poly5.Move(Translation5);

    ASSERT_TRUE(Poly5.IsPointOn(HGF2DLocation(12.0, 2.0, pWorld)));
    ASSERT_TRUE(Poly5.IsPointOn(HGF2DLocation(18.0, 8.0, pWorld)));
    
    HVE2DPolygon Poly6(IncludedRect6);
    Poly6.Move(Translation6);

    ASSERT_TRUE(Poly6.IsPointOn(HGF2DLocation(0.0, 10.0, pWorld)));
    ASSERT_TRUE(Poly6.IsPointOn(HGF2DLocation(10.0, 15.0, pWorld)));
    
    HVE2DPolygon Poly7(IncludedRect7);
    Poly7.Move(Translation7);

    ASSERT_TRUE(Poly7.IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(Poly7.IsPointOn(HGF2DLocation(5.0, 10.0, pWorld)));
    
    HVE2DPolygon Poly8(IncludedRect8);
    Poly8.Move(Translation8);

    ASSERT_TRUE(Poly8.IsPointOn(HGF2DLocation(25.0, 0.0, pWorld)));
    ASSERT_TRUE(Poly8.IsPointOn(HGF2DLocation(30.0, 10.0, pWorld)));
   
    }

//==================================================================================
// Scale(double pi_ScaleFactor, const HGF2DLocation& pi_rScaleOrigin)
//================================================================================== 
TEST_F (HVE2DPolygonTester, ScaleTest)
    {

    HGF2DLocation Origin(0.0, 0.0, pWorld);

    HVE2DPolygon Poly1(IncludedRect1);
    Poly1.Scale(2.0, Origin);

    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(Poly1.IsPointOn(HGF2DLocation(30.0, 30.0, pWorld)));
    
    HVE2DPolygon Poly2(IncludedRect2);
    Poly2.Scale(0.5, Origin);

    ASSERT_TRUE(Poly2.IsPointOn(HGF2DLocation(7.50, 5.0, pWorld)));
    ASSERT_TRUE(Poly2.IsPointOn(HGF2DLocation(10.0, 7.5, pWorld)));

    HVE2DPolygon Poly3(IncludedRect3);
    Poly3.Scale(-2.0, Origin);

    ASSERT_TRUE(Poly3.IsPointOn(HGF2DLocation(-30.0, -30.0, pWorld)));
    ASSERT_TRUE(Poly3.IsPointOn(HGF2DLocation(-40.0, -40.0, pWorld)));
   
    HVE2DPolygon Poly4(IncludedRect4);
    Poly4.Scale(2.0, HGF2DLocation(0.0, 0.5, pWorld));

    ASSERT_TRUE(Poly4.IsPointOn(HGF2DLocation(20.0, 29.5, pWorld)));
    ASSERT_TRUE(Poly4.IsPointOn(HGF2DLocation(30.0, 39.5, pWorld)));
   
    HVE2DPolygon Poly5(IncludedRect5);
    Poly5.Scale(2.0, HGF2DLocation(-0.5, 0.5, pWorld));

    ASSERT_TRUE(Poly5.IsPointOn(HGF2DLocation(24.5, 23.5, pWorld)));
    ASSERT_TRUE(Poly5.IsPointOn(HGF2DLocation(36.5, 35.5, pWorld)));
    
    }

//==================================================================================
// SPECIAL TESTS
// The following are all case which failed with previous library
//==================================================================================

//==================================================================================
// IntersectShapeTest
//==================================================================================
TEST_F (HVE2DPolygonTester, IntersectShapeTest2)
    {

    HVE2DSegment    Segment1A(HGF2DLocation(-1.7E308, -1.7E308, pWorld), HGF2DLocation(-1.7E308, 1.70E308, pWorld));
    HVE2DSegment    Segment2A(HGF2DLocation(-1.7E308, 1.70E308, pWorld), HGF2DLocation(1.70E308, 1.70E308, pWorld));
    HVE2DSegment    Segment3A(HGF2DLocation(1.70E308, 1.70E308, pWorld), HGF2DLocation(1.70E308, -1.7E308, pWorld));
    HVE2DSegment    Segment4A(HGF2DLocation(1.70E308, -1.7E308, pWorld), HGF2DLocation(-1.7E308, -1.7E308, pWorld));
    HVE2DComplexLinear  MyLinear1(pWorld);
    MyLinear1.AppendLinear(Segment1A);
    MyLinear1.AppendLinear(Segment2A);
    MyLinear1.AppendLinear(Segment3A);
    MyLinear1.AppendLinear(Segment4A);

    HVE2DPolygon    Poly1A(MyLinear1);

    HVE2DSegment    Segment1B(HGF2DLocation(0.0000000000000, 0.00000000000000, pWorld), 
                              HGF2DLocation(395.59575494628, -76.896025049963, pWorld));
    HVE2DSegment    Segment2B(HGF2DLocation(395.59575494628, -76.896025049963, pWorld), 
                              HGF2DLocation(471.91935301076, 315.754848345850, pWorld));
    HVE2DSegment    Segment3B(HGF2DLocation(471.91935301076, 315.754848345850, pWorld), 
                              HGF2DLocation(76.323598064480, 392.650873395810, pWorld));
    HVE2DSegment    Segment4B(HGF2DLocation(76.323598064480, 392.650873395810, pWorld), 
                              HGF2DLocation(0.0000000000000, 0.00000000000000, pWorld));
    HVE2DComplexLinear  MyLinear2(pWorld);
    MyLinear2.AppendLinear(Segment1B);
    MyLinear2.AppendLinear(Segment2B);
    MyLinear2.AppendLinear(Segment3B);
    MyLinear2.AppendLinear(Segment4B);

    HVE2DPolygon    Poly1B(MyLinear2);

    HFCPtr<HVE2DPolygon>     pResult1A = (HVE2DPolygon*) Poly1A.IntersectShape(Poly1B);
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
// The coordinate systems are different (but identity)
//==================================================================================
TEST_F (HVE2DPolygonTester, IntersectShapeTest3)
    {

    HFCPtr<HGF2DCoordSys>   pCoordSysIdent = new HGF2DCoordSys(HGF2DIdentity(), pWorld);
    
    HFCPtr<HVE2DShape> pShape1 = new HVE2DRectangle (0.0, 0.0, 415.0, 409.0, pWorld);

    HVE2DComplexLinear  TheLinear(pCoordSysIdent);

    TheLinear.AppendLinear(HVE2DSegment(HGF2DLocation(0.00 , 256.0, pCoordSysIdent), HGF2DLocation(83.0 , 256.0, pCoordSysIdent)));
    TheLinear.AppendLinear(HVE2DSegment(HGF2DLocation(83.0 , 256.0, pCoordSysIdent), HGF2DLocation(83.0 , 0.000, pCoordSysIdent)));
    TheLinear.AppendLinear(HVE2DSegment(HGF2DLocation(83.0 , 0.000, pCoordSysIdent), HGF2DLocation(0.00 , 0.000, pCoordSysIdent)));
    TheLinear.AppendLinear(HVE2DSegment(HGF2DLocation(0.00 , 0.000, pCoordSysIdent), HGF2DLocation(0.00 , 256.0, pCoordSysIdent)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygon(TheLinear);
    HFCPtr<HVE2DPolygon> pResult = (HVE2DPolygon*) pShape1->IntersectShape(*pShape2);

    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResult->GetShapeType());
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
TEST_F (HVE2DPolygonTester, IntersectShapeTest4)
    {
   
    HFCPtr<HGF2DCoordSys>   pCoordSysIdent = new HGF2DCoordSys(HGF2DIdentity(), pWorld);

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000 , 0.000, pWorld), HGF2DLocation(256.0 , 0.000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 0.000, pWorld), HGF2DLocation(256.0 , 256.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 256.0, pWorld), HGF2DLocation(0.000 , 256.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000 , 256.0, pWorld), HGF2DLocation(0.000 , 0.000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygon(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pCoordSysIdent);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 105.0, pCoordSysIdent), HGF2DLocation(256.0 , 0.000, pCoordSysIdent)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 0.000, pCoordSysIdent), HGF2DLocation(0.000 , 0.000, pCoordSysIdent)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000 , 0.000, pCoordSysIdent), HGF2DLocation(0.000 , 105.0, pCoordSysIdent)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000 , 105.0, pCoordSysIdent), HGF2DLocation(256.0 , 105.0, pCoordSysIdent)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygon(TheLinear2);

    HFCPtr<HVE2DPolygon> pResult = (HVE2DPolygon*) pShape1->IntersectShape(*pShape2);

    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResult->GetShapeType());
    ASSERT_TRUE(pResult->IsSimple());
       
    HVE2DComplexLinear  AComp;
    AComp = pResult->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(256.0, AComp.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(105.0, AComp.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(256.0, AComp.GetLinear(0).GetEndPoint().GetX());
    ASSERT_NEAR(0.0, AComp.GetLinear(0).GetEndPoint().GetY(), MYEPSILON);

    ASSERT_DOUBLE_EQ(256.0, AComp.GetLinear(1).GetStartPoint().GetX());
    ASSERT_NEAR(0.0, AComp.GetLinear(1).GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp.GetLinear(1).GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp.GetLinear(1).GetEndPoint().GetY(), MYEPSILON);

    ASSERT_NEAR(0.0, AComp.GetLinear(2).GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp.GetLinear(2).GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp.GetLinear(2).GetEndPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(105.0, AComp.GetLinear(2).GetEndPoint().GetY());

    ASSERT_NEAR(0.0, AComp.GetLinear(3).GetStartPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(105.0, AComp.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(256.0, AComp.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(105.0, AComp.GetLinear(3).GetEndPoint().GetY());

    }

//==================================================================================
// Test which failed on 23 may 1997
// The coordinate systems are different (but identity)
//==================================================================================
TEST_F (HVE2DPolygonTester, IntersectShapeTest5)
    {

    HFCPtr<HGF2DCoordSys>   pCoordSysIdent = new HGF2DCoordSys(HGF2DIdentity(), pWorld);

    HFCPtr<HVE2DShape> pShape1 = new HVE2DRectangle (0.0, 210.0, 415.0, 409.0, pWorld);

    HVE2DComplexLinear  TheLinear(pCoordSysIdent);

    TheLinear.AppendLinear(HVE2DSegment(HGF2DLocation(256.0, 315.0, pCoordSysIdent), HGF2DLocation(256.0 , 256.0, pCoordSysIdent)));
    TheLinear.AppendLinear(HVE2DSegment(HGF2DLocation(256.0, 256.0, pCoordSysIdent), HGF2DLocation(0.000 , 256.0, pCoordSysIdent)));
    TheLinear.AppendLinear(HVE2DSegment(HGF2DLocation(0.000, 256.0, pCoordSysIdent), HGF2DLocation(0.000 , 315.0, pCoordSysIdent)));
    TheLinear.AppendLinear(HVE2DSegment(HGF2DLocation(0.000, 315.0, pCoordSysIdent), HGF2DLocation(256.0 , 315.0, pCoordSysIdent)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygon(TheLinear);

    HFCPtr<HVE2DPolygon> pResult = (HVE2DPolygon*) pShape1->IntersectShape(*pShape2);

    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResult->GetShapeType());
    ASSERT_TRUE(pResult->IsSimple());
      
    HVE2DComplexLinear  AComp;
    AComp = pResult->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(256.0, AComp.GetLinear(0).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(315.0, AComp.GetLinear(0).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(256.0, AComp.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(256.0, AComp.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(256.0, AComp.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(256.0, AComp.GetLinear(1).GetStartPoint().GetY());
    ASSERT_NEAR(0.0, AComp.GetLinear(1).GetEndPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(256.0, AComp.GetLinear(1).GetEndPoint().GetY());

    ASSERT_NEAR(0.0, AComp.GetLinear(2).GetStartPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(256.0, AComp.GetLinear(2).GetStartPoint().GetY());
    ASSERT_NEAR(0.0, AComp.GetLinear(2).GetEndPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(315.0, AComp.GetLinear(2).GetEndPoint().GetY());

    ASSERT_NEAR(0.0, AComp.GetLinear(3).GetStartPoint().GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(315.0, AComp.GetLinear(3).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(256.0, AComp.GetLinear(3).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(315.0, AComp.GetLinear(3).GetEndPoint().GetY());

    }

//==================================================================================
// Test which failed on may 28 1997
//==================================================================================
TEST_F (HVE2DPolygonTester, IntersectShapeTest6)
    {

    HFCPtr<HGF2DCoordSys>   pCoordSysIdent = new HGF2DCoordSys(HGF2DIdentity(), pWorld);

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.00000000000, 331.90680836798, pWorld),
                                         HGF2DLocation(172.85086905196, 370.67988489798, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(172.85086905196, 370.67988489798, pWorld),
                                         HGF2DLocation(71.345071697720, 153.00000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(71.345071697720, 153.00000000000, pWorld),
                                         HGF2DLocation(256.00000000000, 153.00000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.00000000000, 153.00000000000, pWorld),
                                         HGF2DLocation(256.00000000000, 331.90680836798, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygon(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pCoordSysIdent);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.00000000000 , 331.90680836798, pCoordSysIdent),
                                         HGF2DLocation(172.85086905196 , 370.67988489798, pCoordSysIdent)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(172.85086905196 , 370.67988489798, pCoordSysIdent),
                                         HGF2DLocation(53.476108564268 , 114.67988489798, pCoordSysIdent)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(53.476108564268 , 114.67988489798, pCoordSysIdent),
                                         HGF2DLocation(256.00000000000 , 114.67988489798, pCoordSysIdent)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.00000000000 , 114.67988489798, pCoordSysIdent),
                                         HGF2DLocation(256.00000000000 , 331.90680836798, pCoordSysIdent)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygon(TheLinear2);

    HFCPtr<HVE2DPolygon> pResult = (HVE2DPolygon*) pShape1->IntersectShape(*pShape2);

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
// Test which failed on may 30 1997 with polygon of segments
// but should be ok with polygon
//==================================================================================
TEST_F (HVE2DPolygonTester, IntersectShapeTest7)
    {

    HFCPtr<HGF2DCoordSys>   pCoordSysIdent = new HGF2DCoordSys(HGF2DIdentity(), pWorld);

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000 , 0.000, pWorld), HGF2DLocation(0.000 , 256.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000 , 256.0, pWorld), HGF2DLocation(256.0 , 256.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 256.0, pWorld), HGF2DLocation(256.0 , 0.000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 0.000, pWorld), HGF2DLocation(0.000 , 0.000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygon(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pCoordSysIdent);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.00000000000 , 38.773076530008, pCoordSysIdent),
                                         HGF2DLocation(172.85086905196 , 0.0000000000000, pCoordSysIdent)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(172.85086905196 , 0.0000000000000, pCoordSysIdent),
                                         HGF2DLocation(53.476108564268 , 256.00000000000, pCoordSysIdent)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(53.476108564268 , 256.00000000000, pCoordSysIdent),
                                         HGF2DLocation(256.00000000000 , 256.00000000000, pCoordSysIdent)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.00000000000 , 256.00000000000, pCoordSysIdent),
                                         HGF2DLocation(256.00000000000 , 38.773076530008, pCoordSysIdent)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygon(TheLinear2);

    HFCPtr<HVE2DPolygon> pResult = (HVE2DPolygon*) pShape1->IntersectShape(*pShape2);

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
// Test which failed on august 05 1997 with polygon
//==================================================================================
TEST_F (HVE2DPolygonTester, CalculateSpatialPositionOfTest)
    {
                
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00 , 0.000, pWorld), HGF2DLocation(64.0 , 0.000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(64.0 , 0.000, pWorld), HGF2DLocation(64.0 , 256.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(64.0 , 256.0, pWorld), HGF2DLocation(0.00 , 256.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00 , 256.0, pWorld), HGF2DLocation(0.00 , 0.000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygon(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00 , 256.0, pWorld), HGF2DLocation(0.00 , 246.0, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00 , 246.0, pWorld), HGF2DLocation(28.5 , 246.0, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(28.5 , 246.0, pWorld), HGF2DLocation(28.5 , 33.50, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(28.5 , 33.50, pWorld), HGF2DLocation(0.00 , 33.50, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00 , 33.50, pWorld), HGF2DLocation(0.00 , 0.000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00 , 0.000, pWorld), HGF2DLocation(64.0 , 0.000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(64.0 , 0.000, pWorld), HGF2DLocation(64.0 , 256.0, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(64.0 , 256.0, pWorld), HGF2DLocation(0.00 , 256.0, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygon(TheLinear2);

    pShape1->CalculateSpatialPositionOf(*pShape2);

    }