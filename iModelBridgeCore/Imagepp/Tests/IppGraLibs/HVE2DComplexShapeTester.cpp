//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HVE2DComplexShapeTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "EnvironnementTest.h"
#include "HVE2DComplexShapeTester.h"

// THE PRESENT TEST CANNOT MAKE TESTS WITH NON-LINEAR TRANSFORMATION MODELS

HVE2DComplexShapeTester::HVE2DComplexShapeTester() 
    {
    
    // Linears
    Segment1A = HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(10.0, 20.0, pWorld));
    Segment2A = HVE2DSegment(HGF2DLocation(10.0, 20.0, pWorld), HGF2DLocation(20.0, 20.0, pWorld));
    Segment3A = HVE2DSegment(HGF2DLocation(20.0, 20.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld));
    Segment4A = HVE2DSegment(HGF2DLocation(20.0, 10.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    MyLinear1 = HVE2DComplexLinear(pWorld);
    MyLinear1.AppendLinear(Segment1A);
    MyLinear1.AppendLinear(Segment2A);
    MyLinear1.AppendLinear(Segment3A);
    MyLinear1.AppendLinear(Segment4A);

    Segment1B = HVE2DSegment(HGF2DLocation(12.0, 12.0, pWorld), HGF2DLocation(12.0, 18.0, pWorld));
    Segment2B = HVE2DSegment(HGF2DLocation(12.0, 18.0, pWorld), HGF2DLocation(18.0, 18.0, pWorld));
    Segment3B = HVE2DSegment(HGF2DLocation(18.0, 18.0, pWorld), HGF2DLocation(18.0, 12.0, pWorld));
    Segment4B = HVE2DSegment(HGF2DLocation(18.0, 12.0, pWorld), HGF2DLocation(12.0, 12.0, pWorld));

    MyLinear2 = HVE2DComplexLinear(pWorld);
    MyLinear2.AppendLinear(Segment1B);
    MyLinear2.AppendLinear(Segment2B);
    MyLinear2.AppendLinear(Segment3B);
    MyLinear2.AppendLinear(Segment4B);

    Segment1C = HVE2DSegment(HGF2DLocation(18.0, 12.0, pWorld), HGF2DLocation(18.0, 18.0, pWorld));
    Segment2C = HVE2DSegment(HGF2DLocation(18.0, 18.0, pWorld), HGF2DLocation(20.0, 18.0, pWorld));
    Segment3C = HVE2DSegment(HGF2DLocation(20.0, 18.0, pWorld), HGF2DLocation(20.0, 12.0, pWorld));
    Segment4C = HVE2DSegment(HGF2DLocation(20.0, 12.0, pWorld), HGF2DLocation(18.0, 12.0, pWorld));

    MyLinear3 = HVE2DComplexLinear(pWorld);
    MyLinear3.AppendLinear(Segment1C);
    MyLinear3.AppendLinear(Segment2C);
    MyLinear3.AppendLinear(Segment3C);
    MyLinear3.AppendLinear(Segment4C);

    Segment1D = HVE2DSegment(HGF2DLocation(12.0, 10.0, pWorld), HGF2DLocation(12.0, 20.0, pWorld));
    Segment2D = HVE2DSegment(HGF2DLocation(12.0, 20.0, pWorld), HGF2DLocation(21.0, 20.0, pWorld));
    Segment3D = HVE2DSegment(HGF2DLocation(21.0, 20.0, pWorld), HGF2DLocation(21.0, 10.0, pWorld));
    Segment4D = HVE2DSegment(HGF2DLocation(21.0, 10.0, pWorld), HGF2DLocation(12.0, 10.0, pWorld));

    MyLinear4 = HVE2DComplexLinear(pWorld);
    MyLinear4.AppendLinear(Segment1D);
    MyLinear4.AppendLinear(Segment2D);
    MyLinear4.AppendLinear(Segment3D);
    MyLinear4.AppendLinear(Segment4D);

    Segment1E = HVE2DSegment(HGF2DLocation(17.0, 12.0, pWorld), HGF2DLocation(17.0, 18.0, pWorld));
    Segment2E = HVE2DSegment(HGF2DLocation(17.0, 18.0, pWorld), HGF2DLocation(20.0, 18.0, pWorld));
    Segment3E = HVE2DSegment(HGF2DLocation(20.0, 18.0, pWorld), HGF2DLocation(20.0, 12.0, pWorld));
    Segment4E = HVE2DSegment(HGF2DLocation(20.0, 12.0, pWorld), HGF2DLocation(17.0, 12.0, pWorld));

    MyLinear5 = HVE2DComplexLinear(pWorld);
    MyLinear5.AppendLinear(Segment1E);
    MyLinear5.AppendLinear(Segment2E);
    MyLinear5.AppendLinear(Segment3E);
    MyLinear5.AppendLinear(Segment4E);

    Poly1Point0d0 = HGF2DLocation(10.0, 10.0, pWorld);
    Poly1Point0d1 = HGF2DLocation(15.0, 10.0, pWorld);
    Poly1Point0d5 = HGF2DLocation(20.0, 20.0, pWorld);
    Poly1Point1d0 = HGF2DLocation(10.0, 10.0 + (1.1 * MYEPSILON), pWorld);

    }

//==================================================================================
// Constructor test
//==================================================================================
TEST_F (HVE2DComplexShapeTester, ConstructorTest)
    {

    // Default Constructor
    HVE2DComplexShape ComplexShape1;

    ASSERT_FALSE(ComplexShape1.IsSimple());
    ASSERT_TRUE(ComplexShape1.IsComplex());
    ASSERT_TRUE(ComplexShape1.IsEmpty());

    //Constructor with CoordSys
    HVE2DComplexShape ComplexShape2(pWorld);

    ASSERT_EQ(pWorld, ComplexShape2.GetCoordSys());
    ASSERT_FALSE(ComplexShape2.IsSimple());
    ASSERT_TRUE(ComplexShape2.IsComplex());
    ASSERT_TRUE(ComplexShape2.IsEmpty());

    //Constructor with a SimpleShape
    HVE2DComplexShape ComplexShape3(Rect1);

    ASSERT_EQ(pWorld, ComplexShape3.GetCoordSys());
    ASSERT_FALSE(ComplexShape3.IsSimple());
    ASSERT_TRUE(ComplexShape3.IsComplex());
    ASSERT_FALSE(ComplexShape3.IsEmpty());

    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    //Copy Constructor
    HVE2DComplexShape ComplexShape4(ComplexShape3);

    ASSERT_EQ(pWorld, ComplexShape4.GetCoordSys());
    ASSERT_FALSE(ComplexShape4.IsSimple());
    ASSERT_TRUE(ComplexShape4.IsComplex());
    ASSERT_FALSE(ComplexShape4.IsEmpty());

    ASSERT_TRUE(ComplexShape4.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(ComplexShape4.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(ComplexShape4.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(ComplexShape4.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    }

//==================================================================================
// operator= test
// operator=(const HVE2DRectangle& pi_rObj);
//==================================================================================
TEST_F (HVE2DComplexShapeTester, OperatorTest)
    {
    
    HVE2DComplexShape ComplexShape1(Rect1);
    HVE2DComplexShape ComplexShape2(pWorld);

    ComplexShape2 = ComplexShape1;

    ASSERT_EQ(pWorld, ComplexShape2.GetCoordSys());
    ASSERT_FALSE(ComplexShape2.IsSimple());
    ASSERT_TRUE(ComplexShape2.IsComplex());
    ASSERT_FALSE(ComplexShape2.IsEmpty());

    ASSERT_TRUE(ComplexShape2.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(ComplexShape2.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(ComplexShape2.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(ComplexShape2.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    }

//==================================================================================
// IsSimple() const;
// IsComplex() const;
// GetShapeType() const;
// IsEmpty () const;
// HasHoles() const;
//==================================================================================
TEST_F (HVE2DComplexShapeTester, IsTest)
    {

    // Empty HVE2DComplexShape 
    HVE2DComplexShape ComplexShape1(pWorld);

    ASSERT_FALSE(ComplexShape1.IsSimple());
    ASSERT_TRUE(ComplexShape1.IsComplex());
    ASSERT_EQ(HVE2DComplexShape::CLASS_ID, ComplexShape1.GetShapeType());
    ASSERT_TRUE(ComplexShape1.IsEmpty());
    ASSERT_FALSE(ComplexShape1.HasHoles());

    // Rectangle HVE2DComplexShape  
    ComplexShape1.AddShape(Rect1);

    ASSERT_FALSE(ComplexShape1.IsSimple());
    ASSERT_TRUE(ComplexShape1.IsComplex());
    ASSERT_FALSE(ComplexShape1.IsEmpty());
    ASSERT_FALSE(ComplexShape1.HasHoles());
    ASSERT_EQ(HVE2DComplexShape::CLASS_ID, ComplexShape1.GetShapeType());

    // HoledShape HVE2DComplexShape
    HVE2DHoledShape HoledShape1(Rect1);
    HoledShape1.AddHole(HVE2DRectangle(15.0, 15.0, 17.0, 17.0, pWorld));
    HVE2DComplexShape ComplexShape2(HoledShape1);
    
    ASSERT_FALSE(ComplexShape2.IsSimple());
    ASSERT_TRUE(ComplexShape2.IsComplex());
    ASSERT_FALSE(ComplexShape2.IsEmpty());
    ASSERT_FALSE(ComplexShape2.HasHoles());
    ASSERT_EQ(HVE2DComplexShape::CLASS_ID, ComplexShape2.GetShapeType());

    }

//==================================================================================
//GetShapeList() const;
//==================================================================================
TEST_F (HVE2DComplexShapeTester, GetShapeListTest)
    {

    // Empty HVE2DComplexShape
    HVE2DComplexShape ComplexShape(pWorld);
    ASSERT_EQ(0, ComplexShape.GetShapeList().size());

    // HoledShape HVE2DComplexShape
    HVE2DHoledShape HoledShape1(Rect1);
    HoledShape1.AddHole(HVE2DRectangle(15.0, 15.0, 17.0, 17.0, pWorld));
    ComplexShape.AddShape(HoledShape1);

    ASSERT_EQ(1, ComplexShape.GetShapeList().size());

    // HoledShape + Rectangle HVE2DComplexShape
    ComplexShape.AddShape(HVE2DRectangle(15.5, 15.5, 16.5, 16.5, pWorld));
    ASSERT_EQ(2, ComplexShape.GetShapeList().size());

    // 2 HoledShape + Rectangle HVE2DComplexShape
    HVE2DHoledShape HoledShape2(HVE2DRectangle(0.0, 0.0, 100.0, 100.0, pWorld));
    HoledShape2.AddHole(HVE2DRectangle(5.0, 5.0, 25.0, 25.0, pWorld));
    
    ComplexShape.AddShape(HoledShape2);
    ASSERT_EQ(3, ComplexShape.GetShapeList().size());

    }

//==================================================================================
// CalculatePerimeter() const;
//==================================================================================
TEST_F (HVE2DComplexShapeTester, CalculatePerimeterTest)
    {

    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    // With Polygon
    ASSERT_DOUBLE_EQ(HoledShape1.CalculatePerimeter(), Poly1A.CalculatePerimeter() + Poly1B.CalculatePerimeter() );

    //With Rectangle
    ASSERT_DOUBLE_EQ(40.0, HVE2DComplexShape(Rect1).CalculatePerimeter());
    ASSERT_DOUBLE_EQ(20.0, HVE2DComplexShape(IncludedRect1).CalculatePerimeter());
    ASSERT_DOUBLE_EQ(20.0, HVE2DComplexShape(IncludedRect2).CalculatePerimeter());
    ASSERT_DOUBLE_EQ(20.0, HVE2DComplexShape(IncludedRect3).CalculatePerimeter());

    }

//==================================================================================
// CalculateArea() const;
//==================================================================================
TEST_F (HVE2DComplexShapeTester, CalculateAreaTest)
    {

    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    //With Polygon
    ASSERT_DOUBLE_EQ(HoledShape1.CalculateArea(), Poly1A.CalculateArea() - Poly1B.CalculateArea());

    //With Rectangle
    ASSERT_DOUBLE_EQ(100.0, HVE2DComplexShape(Rect1).CalculateArea());
    ASSERT_DOUBLE_EQ(100.0, HVE2DComplexShape(NegativeRect).CalculateArea());
    ASSERT_DOUBLE_EQ(25.00, HVE2DComplexShape(IncludedRect1).CalculateArea());
    ASSERT_DOUBLE_EQ(25.00, HVE2DComplexShape(IncludedRect2).CalculateArea());

    }

//==================================================================================
// IsEmpty() test
// MakEmpty() test
//==================================================================================
TEST_F (HVE2DComplexShapeTester, EmptyTest)
    {

    HVE2DComplexShape  ComplexShape(Rect1);

    ComplexShape.MakeEmpty();

    ASSERT_TRUE(ComplexShape.IsEmpty());
    ASSERT_EQ(HVE2DComplexShape::CLASS_ID, ComplexShape.GetShapeType());

    }

//==================================================================================
// Drop( HGF2DLocationCollection* po_pPoints, const HGFDistance& pi_rTolerance ) const
//==================================================================================
TEST_F (HVE2DComplexShapeTester, DropTest)
    {

    // With Rectangle
    HGF2DLocationCollection Locations;
    HVE2DComplexShape  ComplexShape1(Rect1);

    ComplexShape1.Drop(&Locations, MYEPSILON);
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

    Locations.clear();

    // With HoledShape
    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    HVE2DComplexShape  ComplexShape2(HoledShape1);

    ComplexShape2.Drop(&Locations, HGLOBAL_EPSILON);
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
    ASSERT_DOUBLE_EQ(12.0, Locations[5].GetX());
    ASSERT_DOUBLE_EQ(12.0, Locations[5].GetY());
    ASSERT_DOUBLE_EQ(12.0, Locations[6].GetX());
    ASSERT_DOUBLE_EQ(18.0, Locations[6].GetY());
    ASSERT_DOUBLE_EQ(18.0, Locations[7].GetX());
    ASSERT_DOUBLE_EQ(18.0, Locations[7].GetY());
    ASSERT_DOUBLE_EQ(18.0, Locations[8].GetX());
    ASSERT_DOUBLE_EQ(12.0, Locations[8].GetY());
    ASSERT_DOUBLE_EQ(12.0, Locations[9].GetX());
    ASSERT_DOUBLE_EQ(12.0, Locations[9].GetY());

    }

//==================================================================================
// CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
//==================================================================================
TEST_F (HVE2DComplexShapeTester, CalculateClosestPointTest)
    {

    // With Rectangle
    HVE2DComplexShape  ComplexShape1(Rect1);

    ASSERT_DOUBLE_EQ(20.0, ComplexShape1.CalculateClosestPoint(RectClosePoint1A).GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexShape1.CalculateClosestPoint(RectClosePoint1A).GetY());
    ASSERT_DOUBLE_EQ(10.0, ComplexShape1.CalculateClosestPoint(RectClosePoint1B).GetX());
    ASSERT_DOUBLE_EQ(10.0, ComplexShape1.CalculateClosestPoint(RectClosePoint1B).GetY());
    ASSERT_DOUBLE_EQ(19.9, ComplexShape1.CalculateClosestPoint(RectClosePoint1C).GetX());
    ASSERT_DOUBLE_EQ(10.0, ComplexShape1.CalculateClosestPoint(RectClosePoint1C).GetY());
    ASSERT_DOUBLE_EQ(10.0, ComplexShape1.CalculateClosestPoint(RectClosePoint1D).GetX());
    ASSERT_DOUBLE_EQ(15.0, ComplexShape1.CalculateClosestPoint(RectClosePoint1D).GetY());
    ASSERT_DOUBLE_EQ(15.0, ComplexShape1.CalculateClosestPoint(RectCloseMidPoint1).GetX());
    ASSERT_DOUBLE_EQ(20.0, ComplexShape1.CalculateClosestPoint(RectCloseMidPoint1).GetY());
    ASSERT_DOUBLE_EQ(20.0, ComplexShape1.CalculateClosestPoint(VeryFarPoint).GetX());
    ASSERT_DOUBLE_EQ(20.0, ComplexShape1.CalculateClosestPoint(VeryFarPoint).GetY());
    ASSERT_DOUBLE_EQ(15.0, ComplexShape1.CalculateClosestPoint(RectMidPoint1).GetX());
    ASSERT_DOUBLE_EQ(20.0, ComplexShape1.CalculateClosestPoint(RectMidPoint1).GetY());

    // With HoledShape
    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    HVE2DComplexShape  ComplexShape2(HoledShape1);

    ASSERT_DOUBLE_EQ(10.0, ComplexShape2.CalculateClosestPoint(HGF2DLocation(0.0, 0.0, pWorld)).GetX());
    ASSERT_DOUBLE_EQ(10.0, ComplexShape2.CalculateClosestPoint(HGF2DLocation(0.0, 0.0, pWorld)).GetY());
    ASSERT_DOUBLE_EQ(10.0, ComplexShape2.CalculateClosestPoint(HGF2DLocation(10.0, 10.0, pWorld)).GetX());
    ASSERT_DOUBLE_EQ(10.0, ComplexShape2.CalculateClosestPoint(HGF2DLocation(10.0, 10.0, pWorld)).GetY());
    ASSERT_DOUBLE_EQ(10.0, ComplexShape2.CalculateClosestPoint(HGF2DLocation(11.0, 11.0, pWorld)).GetX());
    ASSERT_DOUBLE_EQ(11.0, ComplexShape2.CalculateClosestPoint(HGF2DLocation(11.0, 11.0, pWorld)).GetY());
    ASSERT_DOUBLE_EQ(12.0, ComplexShape2.CalculateClosestPoint(HGF2DLocation(11.5, 11.5, pWorld)).GetX());
    ASSERT_DOUBLE_EQ(12.0, ComplexShape2.CalculateClosestPoint(HGF2DLocation(11.5, 11.5, pWorld)).GetY());
    ASSERT_DOUBLE_EQ(12.0, ComplexShape2.CalculateClosestPoint(HGF2DLocation(13.0, 13.0, pWorld)).GetX());
    ASSERT_DOUBLE_EQ(13.0, ComplexShape2.CalculateClosestPoint(HGF2DLocation(13.0, 13.0, pWorld)).GetY());

    }

//==================================================================================
// Intersect(const HVE2DVector& pi_rVector, HGF2DLocationCollection* po_pCrossPoints) const;
//==================================================================================
TEST_F (HVE2DComplexShapeTester, IntersectTest)
    {

    //With Rectangle
    HGF2DLocationCollection   DumPoints;
    HVE2DComplexShape  ComplexShape1(Rect1);

    ASSERT_EQ(0, ComplexShape1.Intersect(DisjointLinear1, &DumPoints));
    ASSERT_EQ(0, ComplexShape1.Intersect(ContiguousExtentLinear1, &DumPoints));
    ASSERT_EQ(0, ComplexShape1.Intersect(FlirtingExtentLinear1, &DumPoints));
    ASSERT_EQ(0, ComplexShape1.Intersect(ConnectingLinear1, &DumPoints));
    ASSERT_EQ(0, ComplexShape1.Intersect(ConnectingLinear1A, &DumPoints));
    ASSERT_EQ(0, ComplexShape1.Intersect(LinkedLinear1, &DumPoints));

    ASSERT_EQ(1, ComplexShape1.Intersect(ComplexLinearCase1, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(13.5, DumPoints[0].GetY());

    DumPoints.clear();
    ASSERT_EQ(1, ComplexShape1.Intersect(ComplexLinearCase2, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(15.0, DumPoints[0].GetY());

    DumPoints.clear();
    ASSERT_EQ(1, ComplexShape1.Intersect(ComplexLinearCase3, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());

    DumPoints.clear();
    ASSERT_EQ(0, ComplexShape1.Intersect(ComplexLinearCase4, &DumPoints));

    DumPoints.clear();
    ASSERT_EQ(1, ComplexShape1.Intersect(ComplexLinearCase5, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());

    DumPoints.clear();
    ASSERT_EQ(0, ComplexShape1.Intersect(ComplexLinearCase5A, &DumPoints));
    ASSERT_EQ(0, ComplexShape1.Intersect(ComplexLinearCase6, &DumPoints));
    ASSERT_EQ(0, ComplexShape1.Intersect(ComplexLinearCase7, &DumPoints));

    // With HoledShape
    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);
    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);
    HVE2DComplexShape  ComplexShape2(HoledShape1);

    HVE2DSegment Segment1(HGF2DLocation(-10.0, 0.0, pWorld), HGF2DLocation(0.0, 0.0, pWorld));
    ASSERT_EQ(0, ComplexShape2.Intersect(Segment1, &DumPoints));

    HVE2DSegment Segment2(HGF2DLocation(-10.0, 0.0, pWorld), HGF2DLocation(10.0 - MYEPSILON, 10.0 - MYEPSILON, pWorld));
    ASSERT_EQ(0, ComplexShape2.Intersect(Segment2, &DumPoints));

    HVE2DSegment Segment3(HGF2DLocation(-10.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    ASSERT_EQ(0, ComplexShape2.Intersect(Segment3, &DumPoints));

    HVE2DSegment Segment4(HGF2DLocation(9.0, 9.0, pWorld), HGF2DLocation(13.0, 13.0, pWorld));
    ASSERT_EQ(2, ComplexShape2.Intersect(Segment4, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(12.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(12.0, DumPoints[1].GetY());

    DumPoints.clear();

    HVE2DSegment Segment5(HGF2DLocation(9.0, 9.0, pWorld), HGF2DLocation(21.0, 21.0, pWorld));
    ASSERT_EQ(4, ComplexShape2.Intersect(Segment5, &DumPoints));
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetY());
    ASSERT_DOUBLE_EQ(18.0, DumPoints[2].GetX());
    ASSERT_DOUBLE_EQ(18.0, DumPoints[2].GetY());
    ASSERT_DOUBLE_EQ(12.0, DumPoints[3].GetX());
    ASSERT_DOUBLE_EQ(12.0, DumPoints[3].GetY());

    DumPoints.clear();

    HVE2DSegment Segment6(HGF2DLocation(13.0, 13.0, pWorld), HGF2DLocation(15.0, 15.0, pWorld));
    ASSERT_EQ(0, ComplexShape2.Intersect(Segment6, &DumPoints));

    HVE2DSegment Segment7(HGF2DLocation(13.0, 13.0, pWorld), HGF2DLocation(21.0, 21.0, pWorld));
    ASSERT_EQ(2, ComplexShape2.Intersect(Segment7, &DumPoints));

    }

//==================================================================================
// IsPointIn(const HGF2DLocation& pi_rPoint) const;
//==================================================================================
TEST_F (HVE2DComplexShapeTester, IsPointInTest)
    {
    
    // With Rectangle
    ASSERT_FALSE(HVE2DComplexShape(Rect1).IsPointIn(HGF2DLocation(10.0+0.9*MYEPSILON, 15.0, pWorld)));
    ASSERT_FALSE(HVE2DComplexShape(Rect1).IsPointIn(HGF2DLocation(15.0, 20.0-0.9*MYEPSILON, pWorld)));
    ASSERT_FALSE(HVE2DComplexShape(Rect1).IsPointIn(HGF2DLocation(20.0-0.9*MYEPSILON, 15.0, pWorld)));
    ASSERT_FALSE(HVE2DComplexShape(Rect1).IsPointIn(HGF2DLocation(15.0, 10.0+0.9*MYEPSILON, pWorld)));
    ASSERT_TRUE(HVE2DComplexShape(Rect1).IsPointIn(HGF2DLocation(10.0+1.1*MYEPSILON, 15.0, pWorld)));
    ASSERT_TRUE(HVE2DComplexShape(Rect1).IsPointIn(HGF2DLocation(15.0, 20.0-1.1*MYEPSILON, pWorld)));
    ASSERT_TRUE(HVE2DComplexShape(Rect1).IsPointIn(HGF2DLocation(20.0-1.1*MYEPSILON, 15.0, pWorld)));
    ASSERT_TRUE(HVE2DComplexShape(Rect1).IsPointIn(HGF2DLocation(15.0, 10.0+1.1*MYEPSILON, pWorld)));

    // With HoledShape
    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    ASSERT_TRUE(HVE2DComplexShape(HoledShape1).IsPointIn(HGF2DLocation(13.0, 19.0, pWorld)));
    ASSERT_TRUE(HVE2DComplexShape(HoledShape1).IsPointIn(HGF2DLocation(19.0, 13.0, pWorld)));
    ASSERT_TRUE(HVE2DComplexShape(HoledShape1).IsPointIn(HGF2DLocation(19.0, 19.0, pWorld)));
    ASSERT_FALSE(HVE2DComplexShape(HoledShape1).IsPointIn(HGF2DLocation(13.0, 13.0, pWorld)));
    ASSERT_FALSE(HVE2DComplexShape(HoledShape1).IsPointIn(HGF2DLocation(14.0, 14.0, pWorld)));
    ASSERT_FALSE(HVE2DComplexShape(HoledShape1).IsPointIn(HGF2DLocation(15.0, 15.0, pWorld)));

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
TEST_F (HVE2DComplexShapeTester, ContiguousnessTest)
    {
    HGF2DLocationCollection     DumPoints;
    HGF2DLocation   FirstDumPoint;
    HGF2DLocation   SecondDumPoint;

    // Rectangle HVE2DComplexShape
    HVE2DComplexShape CSRect1(Rect1);

    ASSERT_TRUE(CSRect1.AreContiguous(ComplexLinearCase6));

    ASSERT_TRUE(CSRect1.AreContiguousAt(ComplexLinearCase6, RectMidPoint1));

    ASSERT_EQ(2, CSRect1.ObtainContiguousnessPoints(ComplexLinearCase6, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetY());

    CSRect1.ObtainContiguousnessPointsAt(ComplexLinearCase6, LinearMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(10.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetY());

    // Test with non contiguous linears
    ASSERT_FALSE(CSRect1.AreContiguous(ComplexLinearCase1));

    // Test with contiguous rectangle
    DumPoints.clear();

    ASSERT_TRUE(CSRect1.AreContiguous(NorthContiguousRect));

    ASSERT_TRUE(CSRect1.AreContiguousAt(NorthContiguousRect, RectMidPoint1));

    ASSERT_EQ(2, CSRect1.ObtainContiguousnessPoints(NorthContiguousRect, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[1].GetY());

    CSRect1.ObtainContiguousnessPointsAt(NorthContiguousRect, RectMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(10.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(20.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, SecondDumPoint.GetY());

    DumPoints.clear();

    // HoledShape HVE2DComplexShape
    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    HVE2DComplexShape CSHoledShape1(HoledShape1);

    // Test with contiguous linears
    ASSERT_TRUE(CSHoledShape1.AreContiguous(ComplexLinearCase6));
    ASSERT_TRUE(CSHoledShape1.AreContiguousAt(ComplexLinearCase6, HGF2DLocation(15.0, 20.0, pWorld)));
    ASSERT_EQ(2, CSHoledShape1.ObtainContiguousnessPoints(ComplexLinearCase6, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetY());

    CSHoledShape1.ObtainContiguousnessPointsAt(ComplexLinearCase6, LinearMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(10.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetY());

    DumPoints.clear();

    // Test with non contiguous linears
    ASSERT_FALSE(CSHoledShape1.AreContiguous(ComplexLinearCase1));

    // Test contiguous linear with the hole
    HVE2DComplexLinear ComplexLinearHole(pWorld);
    ComplexLinearHole.AppendLinear(HVE2DSegment(HGF2DLocation(12.0, 12.0, pWorld), HGF2DLocation(12.0, 18.0, pWorld)));

    ASSERT_TRUE(CSHoledShape1.AreContiguous(ComplexLinearHole));
    ASSERT_TRUE(CSHoledShape1.AreContiguousAt(ComplexLinearHole, HGF2DLocation(12.0, 12.0, pWorld)));
    ASSERT_EQ(2, CSHoledShape1.ObtainContiguousnessPoints(ComplexLinearHole, &DumPoints));
    ASSERT_DOUBLE_EQ(12.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(12.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(12.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(18.0, DumPoints[1].GetY());

    CSHoledShape1.ObtainContiguousnessPointsAt(ComplexLinearHole, HGF2DLocation(12.0, 16.0, pWorld), &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(12.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(12.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(12.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(18.0, SecondDumPoint.GetY());

    DumPoints.clear();

    }

//==================================================================================
// Cloning tests
// Clone() const;
// AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
//==================================================================================
TEST_F (HVE2DComplexShapeTester, CloningTest)
    {

    HVE2DComplexShape CSRect1(Rect1);

    // General Clone Test
    HFCPtr<HVE2DComplexShape> pClone = (HVE2DComplexShape*) CSRect1.Clone();  
    ASSERT_FALSE(pClone->IsEmpty());
    ASSERT_EQ(pWorld, pClone->GetCoordSys());
    ASSERT_EQ(HVE2DComplexShape::CLASS_ID, pClone->GetShapeType());

    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));  
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld))); 
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld))); 
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld))); 

     // Test with the same coordinate system
    HFCPtr<HVE2DComplexShape> pClone3 = (HVE2DComplexShape*) CSRect1.AllocateCopyInCoordSys(pWorld);
    ASSERT_FALSE(pClone3->IsEmpty());
    ASSERT_EQ(pWorld, pClone3->GetCoordSys());
    ASSERT_EQ(HVE2DComplexShape::CLASS_ID, pClone3->GetShapeType());

    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));  
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld))); 
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld))); 
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld))); 

    // Test with a translation between systems
    Translation = HGF2DDisplacement (10.0, 10.0);
    HGF2DTranslation myTranslation(Translation);
    HFCPtr<HGF2DCoordSys> pWorldTranslation = new HGF2DCoordSys(myTranslation, pWorld);
    
    HFCPtr<HVE2DComplexShape> pClone5 = (HVE2DComplexShape*) CSRect1.AllocateCopyInCoordSys(pWorldTranslation);
    ASSERT_FALSE(pClone5->IsEmpty());
    ASSERT_EQ(pWorldTranslation, pClone5->GetCoordSys());
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pClone5->GetShapeType());

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
    
    HFCPtr<HVE2DComplexShape> pClone6 = (HVE2DComplexShape*) CSRect1.AllocateCopyInCoordSys(pWorldStretch);
    ASSERT_FALSE(pClone6->IsEmpty());
    ASSERT_EQ(pWorldStretch, pClone6->GetCoordSys());
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pClone6->GetShapeType());

    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(0.0, 0.0, pWorldStretch)));  
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(0.0, 20.0, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(20.0, 20.0, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(20.0, 0.0, pWorldStretch))); 

    // Test with a stretch between systems
    HGF2DStretch myStretch2;
    myStretch2.SetXScaling(0.5);
    myStretch2.SetYScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldStretch2 = new HGF2DCoordSys(myStretch2, pWorld);
  
    HFCPtr<HVE2DComplexShape> pClone10 = (HVE2DComplexShape*) CSRect1.AllocateCopyInCoordSys(pWorldStretch2);
    ASSERT_FALSE(pClone10->IsEmpty());
    ASSERT_EQ(pWorldStretch2, pClone10->GetCoordSys());
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pClone10->GetShapeType());

    ASSERT_TRUE(pClone10->IsPointOn(HGF2DLocation(20.0, 20.0, pWorldStretch2)));  
    ASSERT_TRUE(pClone10->IsPointOn(HGF2DLocation(20.0, 40.0, pWorldStretch2))); 
    ASSERT_TRUE(pClone10->IsPointOn(HGF2DLocation(40.0, 40.0, pWorldStretch2))); 
    ASSERT_TRUE(pClone10->IsPointOn(HGF2DLocation(40.0, 40.0, pWorldStretch2))); 

    // Test with a similitude between systems
    HGF2DSimilitude mySimilitude;
    mySimilitude.SetRotation(PI);
    mySimilitude.SetScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldSimilitude = new HGF2DCoordSys(mySimilitude, pWorld);
  
    HFCPtr<HVE2DComplexShape> pClone7 = (HVE2DComplexShape*) CSRect1.AllocateCopyInCoordSys(pWorldSimilitude);
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
   
    HFCPtr<HVE2DComplexShape> pClone8 = (HVE2DComplexShape*) CSRect1.AllocateCopyInCoordSys(pWorldAffine);
    ASSERT_FALSE(pClone8->IsEmpty());
    ASSERT_EQ(pWorldAffine, pClone8->GetCoordSys());
    ASSERT_EQ(HVE2DPolygonOfSegments::CLASS_ID, pClone8->GetShapeType());

    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(0.0, 0.0, pWorldAffine)));  
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(0.0, -20.0, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-20.0, -20.0, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-20.0, 0.0, pWorldAffine))); 

    #ifdef WIP_IPPTEST_BUG_16
    //// Test with a similitude between systems
    //CSRect1.MakeEmpty();

    //HFCPtr<HVE2DComplexShape> pClone9 = (HVE2DComplexShape*) CSRect1.AllocateCopyInCoordSys(pWorldSimilitude);

    //ASSERT_TRUE(pClone9->IsEmpty());
    //ASSERT_EQ(pWorldSimilitude, pClone9->GetCoordSys());
    //ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pClone9->GetShapeType());
    #endif
        
    }

//==================================================================================
// Interaction info with other segment
// Crosses(const HVE2DVector& pi_rVector) const;
// AreAdjacent(const HVE2DVector& pi_rVector) const;
// IsPointOn(const HGF2DLocation& pi_rTestPoint) const;
// IsPointOnSCS(const HGF2DLocation& pi_rTestPoint) const;
//==================================================================================
TEST_F(HVE2DComplexShapeTester,  InteractionTest)
    {

    // Rectangle HVE2DComplexShape
    HVE2DComplexShape CSRect1(Rect1);

    ASSERT_TRUE(CSRect1.Crosses(ComplexLinearCase1));
    ASSERT_FALSE(CSRect1.AreAdjacent(ComplexLinearCase1));

    ASSERT_TRUE(CSRect1.Crosses(ComplexLinearCase2));
    ASSERT_TRUE(CSRect1.AreAdjacent(ComplexLinearCase2));

    ASSERT_TRUE(CSRect1.Crosses(ComplexLinearCase3));
    ASSERT_FALSE(CSRect1.AreAdjacent(ComplexLinearCase3));

    ASSERT_FALSE(CSRect1.Crosses(ComplexLinearCase4));
    ASSERT_TRUE(CSRect1.AreAdjacent(ComplexLinearCase4));

    ASSERT_TRUE(CSRect1.Crosses(ComplexLinearCase5));
    ASSERT_TRUE(CSRect1.AreAdjacent(ComplexLinearCase5));

    ASSERT_FALSE(CSRect1.Crosses(ComplexLinearCase6));
    ASSERT_TRUE(CSRect1.AreAdjacent(ComplexLinearCase6));

    ASSERT_FALSE(CSRect1.Crosses(ComplexLinearCase7));
    ASSERT_FALSE(CSRect1.AreAdjacent(ComplexLinearCase7));

    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_FALSE(CSRect1.IsPointOn(HGF2DLocation(10.0 - 1.1*MYEPSILON, 10.0-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(CSRect1.IsPointOn(RectMidPoint1-HGF2DDisplacement(0.0, 1.1*MYEPSILON)));
    ASSERT_FALSE(CSRect1.IsPointOn(RectMidPoint1-HGF2DDisplacement(0.0, -1.1*MYEPSILON)));
    ASSERT_TRUE(CSRect1.IsPointOn(RectMidPoint1-HGF2DDisplacement(0.0, 0.9*MYEPSILON)));
    ASSERT_TRUE(CSRect1.IsPointOn(RectMidPoint1-HGF2DDisplacement(0.0, -0.9*MYEPSILON)));
    ASSERT_TRUE(CSRect1.IsPointOn(RectMidPoint1));
    ASSERT_TRUE(CSRect1.IsPointOn(Rect1.GetLinear().GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(CSRect1.IsPointOn(Rect1.GetLinear().GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_TRUE(CSRect1.IsPointOnSCS(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_FALSE(CSRect1.IsPointOnSCS(HGF2DLocation(10.0 - 1.1*MYEPSILON, 10.0-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(CSRect1.IsPointOnSCS(RectMidPoint1-HGF2DDisplacement(0.0, 1.1*MYEPSILON)));
    ASSERT_FALSE(CSRect1.IsPointOnSCS(RectMidPoint1-HGF2DDisplacement(0.0, -1.1*MYEPSILON)));
    ASSERT_TRUE(CSRect1.IsPointOnSCS(RectMidPoint1-HGF2DDisplacement(0.0, 0.9*MYEPSILON)));
    ASSERT_TRUE(CSRect1.IsPointOnSCS(RectMidPoint1-HGF2DDisplacement(0.0, -0.9*MYEPSILON)));
    ASSERT_TRUE(CSRect1.IsPointOnSCS(RectMidPoint1));
    ASSERT_TRUE(CSRect1.IsPointOnSCS(Rect1.GetLinear().GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(CSRect1.IsPointOnSCS(Rect1.GetLinear().GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));

    // HoledShape HVE2DComplexShape
    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    HVE2DComplexShape CSHoledShape(HoledShape1);
     
    // Tests with a vertical segment
    ASSERT_TRUE(CSHoledShape.Crosses(ComplexLinearCase1));
    ASSERT_FALSE(CSHoledShape.AreAdjacent(ComplexLinearCase1));

    ASSERT_TRUE(CSHoledShape.Crosses(ComplexLinearCase2));
    ASSERT_TRUE(CSHoledShape.AreAdjacent(ComplexLinearCase2));

    ASSERT_TRUE(CSHoledShape.Crosses(ComplexLinearCase3));
    ASSERT_FALSE(CSHoledShape.AreAdjacent(ComplexLinearCase3));

    ASSERT_FALSE(CSHoledShape.Crosses(ComplexLinearCase4));
    ASSERT_TRUE(CSHoledShape.AreAdjacent(ComplexLinearCase4));

    ASSERT_TRUE(CSHoledShape.Crosses(ComplexLinearCase5));
    ASSERT_TRUE(CSHoledShape.AreAdjacent(ComplexLinearCase5));

    ASSERT_FALSE(CSHoledShape.Crosses(ComplexLinearCase6));
    ASSERT_TRUE(CSHoledShape.AreAdjacent(ComplexLinearCase6));

    ASSERT_FALSE(CSHoledShape.Crosses(ComplexLinearCase7));
    ASSERT_FALSE(CSHoledShape.AreAdjacent(ComplexLinearCase7));

    ASSERT_TRUE(CSHoledShape.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_FALSE(CSHoledShape.IsPointOn(HGF2DLocation(10.0 - 1.1 * MYEPSILON, 10.0-1.1 * MYEPSILON, pWorld)));
    ASSERT_FALSE(CSHoledShape.IsPointOn(HGF2DLocation(15.0, 20.0 - 1.1 * MYEPSILON, pWorld)));
    ASSERT_FALSE(CSHoledShape.IsPointOn(HGF2DLocation(15.0, 20.0 + 1.1 * MYEPSILON, pWorld)));
    ASSERT_TRUE(CSHoledShape.IsPointOn(HGF2DLocation(15.0, 20.0 + 0.9 * MYEPSILON, pWorld)));
    ASSERT_TRUE(CSHoledShape.IsPointOn(HGF2DLocation(15.0, 20.0 - 0.9 * MYEPSILON, pWorld)));
    ASSERT_TRUE(CSHoledShape.IsPointOn(HGF2DLocation(15.0, 20.0, pWorld)));

    ASSERT_TRUE(CSHoledShape.IsPointOnSCS(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_FALSE(CSHoledShape.IsPointOnSCS(HGF2DLocation(10.0 - 1.1 * MYEPSILON, 10.0-1.1 * MYEPSILON, pWorld)));
    ASSERT_FALSE(CSHoledShape.IsPointOnSCS(HGF2DLocation(15.0, 20.0 - 1.1 * MYEPSILON, pWorld)));
    ASSERT_FALSE(CSHoledShape.IsPointOnSCS(HGF2DLocation(15.0, 20.0 + 1.1 * MYEPSILON, pWorld)));
    ASSERT_TRUE(CSHoledShape.IsPointOnSCS(HGF2DLocation(15.0, 20.0 + 0.9 * MYEPSILON, pWorld)));
    ASSERT_TRUE(CSHoledShape.IsPointOnSCS(HGF2DLocation(15.0, 20.0 - 0.9 * MYEPSILON, pWorld)));
    ASSERT_TRUE(CSHoledShape.IsPointOnSCS(HGF2DLocation(15.0, 20.0, pWorld)));
    
    }

//==================================================================================
// Bearing calculation tests
// CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
//                  HVE2DVector::ArbitraryDiPolyion pi_DiPolyion = HVE2DVector::BETA) const;
//==================================================================================
TEST_F(HVE2DComplexShapeTester,  BearingTest)
    {

    // Rectangle HVE2DComplexShape
    HVE2DComplexShape CSRect1(Rect1);

    // Obtain bearing ALPHA of a vertical segment
    ASSERT_NEAR(0.0, CSRect1.CalculateBearing(Rect1Point0d0, HVE2DVector::ALPHA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(PI/2, CSRect1.CalculateBearing(Rect1Point0d0, HVE2DVector::BETA).GetAngle());
    ASSERT_NEAR(0.0, CSRect1.CalculateBearing(Rect1Point0d1, HVE2DVector::ALPHA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(PI, CSRect1.CalculateBearing(Rect1Point0d1, HVE2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(PI, CSRect1.CalculateBearing(Rect1Point0d5, HVE2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(3*PI/2, CSRect1.CalculateBearing(Rect1Point0d5, HVE2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(3*PI/2, CSRect1.CalculateBearing(Rect1Point1d0, HVE2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(PI/2, CSRect1.CalculateBearing(Rect1Point1d0, HVE2DVector::BETA).GetAngle());

    // HoledShape HVE2DComplexShape
    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);
    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    HVE2DComplexShape CSHoledShape1(HoledShape1);

    // Obtain bearing ALPHA of a vertical segment
    ASSERT_NEAR(0.0, CSHoledShape1.CalculateBearing(Poly1Point0d0, HVE2DVector::ALPHA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(PI/2, CSHoledShape1.CalculateBearing(Poly1Point0d0, HVE2DVector::BETA).GetAngle());
    ASSERT_NEAR(0.0, CSHoledShape1.CalculateBearing(Poly1Point0d1, HVE2DVector::ALPHA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(PI, CSHoledShape1.CalculateBearing(Poly1Point0d1, HVE2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(PI, CSHoledShape1.CalculateBearing(Poly1Point0d5, HVE2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(3*PI/2, CSHoledShape1.CalculateBearing(Poly1Point0d5, HVE2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(3*PI/2, CSHoledShape1.CalculateBearing(Poly1Point1d0, HVE2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(PI/2, CSHoledShape1.CalculateBearing(Poly1Point1d0, HVE2DVector::BETA).GetAngle());
  
    }
   
//================================================================================== 
// CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
//                              HVE2DVector::ArbitraryDiPolyion pi_DiPolyion = HVE2DVector::BETA) const;
//==================================================================================
TEST_F (HVE2DComplexShapeTester, AngularAccelerationTest)
    {

    // Rectangle HVE2DComplexShape
    HVE2DComplexShape CSRect1(Rect1);

    // Obtain angular acceleration ALPHA of a vertical segment
    ASSERT_NEAR(0.0, CSRect1.CalculateAngularAcceleration(Rect1Point0d0, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, CSRect1.CalculateAngularAcceleration(Rect1Point0d0, HVE2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, CSRect1.CalculateAngularAcceleration(Rect1Point0d1, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, CSRect1.CalculateAngularAcceleration(Rect1Point0d1, HVE2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, CSRect1.CalculateAngularAcceleration(Rect1Point0d5, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, CSRect1.CalculateAngularAcceleration(Rect1Point0d5, HVE2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, CSRect1.CalculateAngularAcceleration(Rect1Point1d0, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, CSRect1.CalculateAngularAcceleration(Rect1Point1d0, HVE2DVector::BETA), MYEPSILON);

    // HoledShape HVE2DComplexShape
    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);
    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    HVE2DComplexShape CSHoledShape1(HoledShape1);

    // Obtain angular acceleration ALPHA of a vertical segment
    ASSERT_NEAR(0.0, CSHoledShape1.CalculateAngularAcceleration(Poly1Point0d0, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, CSHoledShape1.CalculateAngularAcceleration(Poly1Point0d0, HVE2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, CSHoledShape1.CalculateAngularAcceleration(Poly1Point0d1, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, CSHoledShape1.CalculateAngularAcceleration(Poly1Point0d1, HVE2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, CSHoledShape1.CalculateAngularAcceleration(Poly1Point0d5, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, CSHoledShape1.CalculateAngularAcceleration(Poly1Point0d5, HVE2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, CSHoledShape1.CalculateAngularAcceleration(Poly1Point1d0, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, CSHoledShape1.CalculateAngularAcceleration(Poly1Point1d0, HVE2DVector::BETA), MYEPSILON);

    }

//==================================================================================
// Extent calculation test
// GetExtent() const;
//==================================================================================
TEST_F (HVE2DComplexShapeTester, GetExtentTest)
    {

    // Rectangle HVE2DComplexShape
    ASSERT_DOUBLE_EQ(10.0, HVE2DComplexShape(Rect1).GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(10.0, HVE2DComplexShape(Rect1).GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(20.0, HVE2DComplexShape(Rect1).GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(20.0, HVE2DComplexShape(Rect1).GetExtent().GetYMax());

    ASSERT_DOUBLE_EQ(10.0, HVE2DComplexShape(IncludedRect1).GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(10.0, HVE2DComplexShape(IncludedRect1).GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(15.0, HVE2DComplexShape(IncludedRect1).GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(15.0, HVE2DComplexShape(IncludedRect1).GetExtent().GetYMax());

    ASSERT_DOUBLE_EQ(15.0, HVE2DComplexShape(IncludedRect2).GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(10.0, HVE2DComplexShape(IncludedRect2).GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(20.0, HVE2DComplexShape(IncludedRect2).GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(15.0, HVE2DComplexShape(IncludedRect2).GetExtent().GetYMax());

    ASSERT_DOUBLE_EQ(15.0, HVE2DComplexShape(IncludedRect3).GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(15.0, HVE2DComplexShape(IncludedRect3).GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(20.0, HVE2DComplexShape(IncludedRect3).GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(20.0, HVE2DComplexShape(IncludedRect3).GetExtent().GetYMax());

    // HoledShape HVE2DComplexShape
    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);
    HVE2DHoledShape NoHoledShape1(Poly1A);
    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    HVE2DComplexShape CSHoledShape1(HoledShape1);
    HVE2DComplexShape CSNoHoledShape1(NoHoledShape1);

    ASSERT_DOUBLE_EQ(10.0, CSHoledShape1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(10.0, CSHoledShape1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(20.0, CSHoledShape1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(20.0, CSHoledShape1.GetExtent().GetYMax());

    ASSERT_DOUBLE_EQ(10.0, CSNoHoledShape1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(10.0, CSNoHoledShape1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(20.0, CSNoHoledShape1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(20.0, CSNoHoledShape1.GetExtent().GetYMax());

    }

////==================================================================================
// Move(const HGF2DDisplacement& pi_rDisplacement);
////==================================================================================
TEST_F (HVE2DComplexShapeTester,  MoveTest) 
    { 

    // Rectangle HVE2DComplexShape
    HVE2DComplexShape CSRect1(Rect1);

    CSRect1.Move(HGF2DDisplacement(10.0, 10.0));

    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DLocation(30.0, 20.0, pWorld)));
    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DLocation(20.0, 30.0, pWorld)));
    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DLocation(30.0, 30.0, pWorld)));

    CSRect1.Move(HGF2DDisplacement(-10.0, -10.0));

    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    // HoledShape HVE2DComplexShape
    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);
    HVE2DComplexShape CSHoledShape1(HoledShape1);

    CSHoledShape1.Move(HGF2DDisplacement(10.0, 10.0));

    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(30.0, 20.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(20.0, 30.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(30.0, 30.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(22.0, 22.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(22.0, 28.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(28.0, 22.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(28.0, 28.0, pWorld)));

    CSHoledShape1.Move(HGF2DDisplacement(-10.0, -10.0));

    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(12.0, 12.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(12.0, 18.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(18.0, 12.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(18.0, 18.0, pWorld)));

    }

////==================================================================================
// Scale(double pi_ScaleFactor, const HGF2DLocation& pi_rScaleOrigin);
////==================================================================================
TEST_F (HVE2DComplexShapeTester,  ScaleTest) 
    { 

    // Rectangle HVE2DComplexShape
    HVE2DComplexShape CSRect1(Rect1);
    HGF2DLocation Origin(0.0, 0.0, pWorld);

    CSRect1.Scale(2.0, Origin);

    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld))); 
    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DLocation(40.0, 20.0, pWorld)));
    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DLocation(20.0, 40.0, pWorld)));
    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DLocation(40.0, 40.0, pWorld)));

    CSRect1.Scale(2.0, HGF2DLocation(10.0, 10.0, pWorld));

    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DLocation(30.0, 30.0, pWorld))); 
    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DLocation(70.0, 30.0, pWorld)));
    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DLocation(30.0, 70.0, pWorld)));
    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DLocation(70.0, 70.0, pWorld)));
   
    // HoledShape HVE2DComplexShape
    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);
    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);
    HVE2DComplexShape CSHoledShape1(HoledShape1);

    CSHoledShape1.Scale(1.0, HGF2DLocation(0.0, 0.0, pWorld));

    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(12.0, 12.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(12.0, 18.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(18.0, 12.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(18.0, 18.0, pWorld)));

    CSHoledShape1.Scale(2.0, HGF2DLocation(0.0, 0.0, pWorld));

    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(40.0, 20.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(20.0, 40.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(40.0, 40.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(24.0, 24.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(24.0, 36.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(36.0, 24.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(36.0, 36.0, pWorld)));

    CSHoledShape1.Scale(2.0, HGF2DLocation(10.0, 20.0, pWorld));

    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(30.0, 20.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(70.0, 20.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(30.0, 60.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(70.0, 60.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(38.0, 28.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(38.0, 52.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(62.0, 28.0, pWorld)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DLocation(62.0, 52.0, pWorld)));
    
    }

//==================================================================================
// AddShape() test
//==================================================================================
TEST_F (HVE2DComplexShapeTester, AddShapeTest)
    {

    // Empty HVE2DComplexShape add Rectangle
    HVE2DComplexShape ComplexShape1(pWorld);
    ComplexShape1.AddShape(Rect1);

    ASSERT_TRUE(ComplexShape1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(ComplexShape1.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(ComplexShape1.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(ComplexShape1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    // Rectangle HVE2DComplexShape add another Rectangle
    ComplexShape1.AddShape(HVE2DRectangle(30.0, 30.0, 50.0, 50.0, pWorld));

    ASSERT_TRUE(ComplexShape1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(ComplexShape1.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(ComplexShape1.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(ComplexShape1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(ComplexShape1.IsPointOn(HGF2DLocation(30.0, 30.0, pWorld)));
    ASSERT_TRUE(ComplexShape1.IsPointOn(HGF2DLocation(50.0, 30.0, pWorld)));
    ASSERT_TRUE(ComplexShape1.IsPointOn(HGF2DLocation(30.0, 50.0, pWorld)));
    ASSERT_TRUE(ComplexShape1.IsPointOn(HGF2DLocation(50.0, 50.0, pWorld)));

    // Empty HVE2DComplexShape add a Polygon
    HVE2DPolygon Poly1A(Rect1);
    HVE2DComplexShape ComplexShape2(pWorld);

    ComplexShape2.AddShape(Poly1A);

    ASSERT_TRUE(ComplexShape2.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(ComplexShape2.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(ComplexShape2.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(ComplexShape2.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    // Empty HVE2DComplexShape add a HoledShape
    HVE2DHoledShape HoledShape1(Poly1A);
    HoledShape1.AddHole(HVE2DRectangle(15.0, 15.0, 17.0, 17.0, pWorld));
    HVE2DComplexShape ComplexShape3(pWorld);

    ComplexShape3.AddShape(HoledShape1);

    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    // Add a Rectangle inside the HoledShape HVE2DComplexShape
    ComplexShape3.AddShape(HVE2DRectangle(15.5, 15.5, 16.5, 16.5, pWorld));

    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(15.5, 15.5, pWorld)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(16.5, 15.5, pWorld)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(15.5, 16.5, pWorld)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(16.5, 16.5, pWorld)));

    HVE2DHoledShape HoledShape2(HVE2DRectangle(0.0, 0.0, 100.0, 100.0, pWorld));
    HoledShape2.AddHole(HVE2DRectangle(5.0, 5.0, 25.0, 25.0, pWorld));
    
    // Add another HoledShape to ComplexShape3
    ComplexShape3.AddShape(HoledShape2);

    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(15.5, 15.5, pWorld)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(16.5, 15.5, pWorld)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(15.5, 16.5, pWorld)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(16.5, 16.5, pWorld)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(100.0, 0.0, pWorld)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(0.0, 100.0, pWorld)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DLocation(100.0, 100.0, pWorld)));
      
    }

//==================================================================================
// DifferentiateShapeSCS(const HVE2DShape& pi_rShape) const;
//==================================================================================
TEST_F (HVE2DComplexShapeTester, DifferentiateShapeTest)
    {

    HVE2DComplexShape CSRect1(Rect1);

    // NorthContiguousRect only touch CSRect1
    HFCPtr<HVE2DComplexShape>     pResultShape1 = (HVE2DComplexShape*) CSRect1.DifferentiateShapeSCS(NorthContiguousRect);
   
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape1->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape1->GetCoordSys());

    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    // VerticalFitRect is partially in CSRect1
    HFCPtr<HVE2DComplexShape>     pResultShape2 = (HVE2DComplexShape*) CSRect1.DifferentiateShapeSCS(VerticalFitRect);
  
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape2->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape2->GetCoordSys());

    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(15.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(15.0, 20.0, pWorld)));

    // DisjointRect doesn't touch CSRect1
    HFCPtr<HVE2DComplexShape>     pResultShape3 = (HVE2DComplexShape*) CSRect1.DifferentiateShapeSCS(DisjointRect);
    
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape3->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape3->GetCoordSys());

    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    // EnglobRect1 contains CSRect1
    HFCPtr<HVE2DComplexShape>     pResultShape4 = (HVE2DComplexShape*) CSRect1.DifferentiateShapeSCS(EnglobRect1);
    
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape4->GetShapeType());
    ASSERT_TRUE(pResultShape4->IsEmpty());

    // IncludedRect1 is inside CSRect1
    HFCPtr<HVE2DComplexShape>     pResultShape5 = (HVE2DComplexShape*) CSRect1.DifferentiateShapeSCS(IncludedRect1);

    ASSERT_EQ(HVE2DPolygonOfSegments::CLASS_ID, pResultShape5->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape5->GetCoordSys());

    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(10.0, 15.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(15.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(15.0, 15.0, pWorld)));

    // IncludedRect2 is completly inside CSRect1
    HVE2DRectangle IncludedRect2(11.0, 11.0, 19.0, 19.0, pWorld);
    HFCPtr<HVE2DComplexShape>  pResultShape6 = (HVE2DComplexShape*) CSRect1.DifferentiateShapeSCS(IncludedRect2);

    ASSERT_EQ(HVE2DHoledShape::CLASS_ID, pResultShape6->GetShapeType());

    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(11.0, 11.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(19.0, 11.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(11.0, 19.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(19.0, 19.0, pWorld)));

    // IncludedRect3 is completly inside CSRect1 and IncludedRect2
    HVE2DRectangle IncludedRect3(13.0, 13.0, 17.0, 17.0, pWorld);
    HFCPtr<HVE2DComplexShape>  pResultShape7 = (HVE2DComplexShape*) pResultShape6->DifferentiateShapeSCS(IncludedRect3);

    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DLocation(11.0, 11.0, pWorld)));
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DLocation(19.0, 11.0, pWorld)));
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DLocation(11.0, 19.0, pWorld)));
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DLocation(19.0, 19.0, pWorld)));

    }

//==================================================================================
// DifferentiateFromShapeSCS(const HVE2DShape& pi_rShape) const;
//==================================================================================
TEST_F (HVE2DComplexShapeTester, DifferentiateFromShapeTest)
    {

    HVE2DComplexShape CSRect1(Rect1);

    // NorthContiguousRect only touch CSRect1
    HFCPtr<HVE2DComplexShape>     pResultShape1 = (HVE2DComplexShape*) NorthContiguousRect.DifferentiateFromShapeSCS(CSRect1);
   
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape1->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape1->GetCoordSys());

    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    // VerticalFitRect is partially in CSRect1
    HFCPtr<HVE2DComplexShape>     pResultShape2 = (HVE2DComplexShape*) VerticalFitRect.DifferentiateFromShapeSCS(CSRect1);
  
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape2->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape2->GetCoordSys());

    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(15.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(15.0, 20.0, pWorld)));

    // DisjointRect doesn't touch CSRect1
    HFCPtr<HVE2DComplexShape>     pResultShape3 = (HVE2DComplexShape*) DisjointRect.DifferentiateFromShapeSCS(CSRect1);
    
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape3->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape3->GetCoordSys());

    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    // EnglobRect1 contains CSRect1
    HFCPtr<HVE2DComplexShape>     pResultShape4 = (HVE2DComplexShape*) EnglobRect1.DifferentiateFromShapeSCS(CSRect1);
    
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape4->GetShapeType());
    ASSERT_TRUE(pResultShape4->IsEmpty());

    // IncludedRect1 is inside CSRect1
    HFCPtr<HVE2DComplexShape>     pResultShape5 = (HVE2DComplexShape*) IncludedRect1.DifferentiateFromShapeSCS(CSRect1);

    ASSERT_EQ(HVE2DPolygonOfSegments::CLASS_ID, pResultShape5->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape5->GetCoordSys());

    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(10.0, 15.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(15.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(15.0, 15.0, pWorld)));

    // IncludedRect2 is completly inside CSRect1
    HVE2DRectangle IncludedRect2(11.0, 11.0, 19.0, 19.0, pWorld);
    HFCPtr<HVE2DComplexShape>  pResultShape6 = (HVE2DComplexShape*) IncludedRect2.DifferentiateFromShapeSCS(CSRect1);

    ASSERT_EQ(HVE2DHoledShape::CLASS_ID, pResultShape6->GetShapeType());

    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(11.0, 11.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(19.0, 11.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(11.0, 19.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(19.0, 19.0, pWorld)));  

    }

//==================================================================================
// IntersectShapeSCS(const HVE2DShape& pi_rShape) const;
//==================================================================================
TEST_F (HVE2DComplexShapeTester, IntersectShapeSCSTest)
    {

    HVE2DComplexShape CSRect1(Rect1);

    // NorthContiguousRect only touch CSRect1
    HFCPtr<HVE2DComplexShape>     pResultShape1 = (HVE2DComplexShape*) NorthContiguousRect.IntersectShapeSCS(CSRect1);

    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape1->GetShapeType());
    ASSERT_TRUE(pResultShape1->IsEmpty());
   
    // VerticalFitRect is partially in CSRect1
    HFCPtr<HVE2DComplexShape>     pResultShape2 = (HVE2DComplexShape*) VerticalFitRect.IntersectShapeSCS(CSRect1);
  
    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape2->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape2->GetCoordSys());

    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(15.0, 15.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(15.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(20.0, 15.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    // DisjointRect doesn't touch CSRect1
    HFCPtr<HVE2DComplexShape>     pResultShape3 = (HVE2DComplexShape*) DisjointRect.IntersectShapeSCS(CSRect1);

    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape3->GetShapeType());
    ASSERT_TRUE(pResultShape3->IsEmpty());

    // EnglobRect1 contains CSRect1
    HFCPtr<HVE2DComplexShape>     pResultShape4 = (HVE2DComplexShape*) EnglobRect1.IntersectShapeSCS(CSRect1);

    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape4->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape4->GetCoordSys());

    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    // IncludedRect1 is inside CSRect1
    HFCPtr<HVE2DComplexShape>     pResultShape5 = (HVE2DComplexShape*) IncludedRect1.IntersectShapeSCS(CSRect1);

    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape5->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape5->GetCoordSys());

    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(10.0, 15.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(15.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(15.0, 15.0, pWorld)));

    // IncludedRect2 is completly inside CSRect1
    HVE2DRectangle IncludedRect2(11.0, 11.0, 19.0, 19.0, pWorld);
    HFCPtr<HVE2DComplexShape>  pResultShape6 = (HVE2DComplexShape*) IncludedRect2.IntersectShapeSCS(CSRect1);

    ASSERT_EQ(HVE2DRectangle::CLASS_ID, pResultShape6->GetShapeType());
    ASSERT_EQ(pWorld, pResultShape6->GetCoordSys());

    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(11.0, 11.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(19.0, 11.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(11.0, 19.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(19.0, 19.0, pWorld)));

    }

//==================================================================================
// UnifyShapeSCS(const HVE2DShape& pi_rShape) const;
//==================================================================================
TEST_F (HVE2DComplexShapeTester, UnifyShapeSCSTest)
    {

    HVE2DComplexShape CSRect1(Rect1);

    // NorthContiguousRect only touch CSRect1
    HFCPtr<HVE2DComplexShape>     pResultShape1 = (HVE2DComplexShape*) NorthContiguousRect.UnifyShapeSCS(CSRect1); 

    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(10.0, 30.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(20.0, 30.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
   
    // VerticalFitRect is partially in CSRect1
    HFCPtr<HVE2DComplexShape>     pResultShape2 = (HVE2DComplexShape*) VerticalFitRect.UnifyShapeSCS(CSRect1);

    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(25.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(25.0, 20.0, pWorld)));
  
    // DisjointRect doesn't touch CSRect1
    HFCPtr<HVE2DComplexShape>     pResultShape3 = (HVE2DComplexShape*) DisjointRect.UnifyShapeSCS(CSRect1);

    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(-10.0, -10.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(-10.0, 0.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(0.0, -10.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));

    // EnglobRect1 contains CSRect1
    HFCPtr<HVE2DComplexShape>     pResultShape4 = (HVE2DComplexShape*) EnglobRect1.UnifyShapeSCS(CSRect1);

    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(10.0, 30.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(30.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(30.0, 30.0, pWorld)));

    // IncludedRect1 is inside CSRect1
    HFCPtr<HVE2DComplexShape>     pResultShape5 = (HVE2DComplexShape*) IncludedRect1.UnifyShapeSCS(CSRect1);

    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    // IncludedRect2 is completly inside CSRect1
    HVE2DRectangle IncludedRect2(11.0, 11.0, 19.0, 19.0, pWorld);
    HFCPtr<HVE2DComplexShape>  pResultShape6 = (HVE2DComplexShape*) IncludedRect2.UnifyShapeSCS(CSRect1);

    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    }