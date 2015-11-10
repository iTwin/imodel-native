//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DComplexShapeTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "EnvironnementTest.h"
#include "HGF2DComplexShapeTester.h"

// THE PRESENT TEST CANNOT MAKE TESTS WITH NON-LINEAR TRANSFORMATION MODELS

HGF2DComplexShapeTester::HGF2DComplexShapeTester() 
    {
    
    // Linears
    MyLinear1 = HGF2DPolySegment();
    MyLinear1.AppendPoint(HGF2DPosition(10.0, 10.0));
    MyLinear1.AppendPoint(HGF2DPosition(10.0, 20.0));
    MyLinear1.AppendPoint(HGF2DPosition(20.0, 20.0));
    MyLinear1.AppendPoint(HGF2DPosition(20.0, 10.0));
    MyLinear1.AppendPoint(HGF2DPosition(10.0, 10.0));
    
    MyLinear2 = HGF2DPolySegment();
    MyLinear2.AppendPoint(HGF2DPosition(12.0, 12.0));
    MyLinear2.AppendPoint(HGF2DPosition(12.0, 18.0));
    MyLinear2.AppendPoint(HGF2DPosition(18.0, 18.0));
    MyLinear2.AppendPoint(HGF2DPosition(18.0, 12.0));
    MyLinear2.AppendPoint(HGF2DPosition(12.0, 12.0));
    
    MyLinear3 = HGF2DPolySegment();
    MyLinear3.AppendPoint(HGF2DPosition(18.0, 12.0));
    MyLinear3.AppendPoint(HGF2DPosition(18.0, 18.0));
    MyLinear3.AppendPoint(HGF2DPosition(20.0, 18.0));
    MyLinear3.AppendPoint(HGF2DPosition(20.0, 12.0));
    MyLinear3.AppendPoint(HGF2DPosition(18.0, 12.0));
    
    
    MyLinear4 = HGF2DPolySegment();
    MyLinear4.AppendPoint(HGF2DPosition(12.0, 10.0));
    MyLinear4.AppendPoint(HGF2DPosition(12.0, 20.0));
    MyLinear4.AppendPoint(HGF2DPosition(21.0, 20.0));
    MyLinear4.AppendPoint(HGF2DPosition(21.0, 10.0));
    MyLinear4.AppendPoint(HGF2DPosition(12.0, 10.0));
    
    MyLinear5 = HGF2DPolySegment();
    MyLinear5.AppendPoint(HGF2DPosition(17.0, 12.0));
    MyLinear5.AppendPoint(HGF2DPosition(17.0, 18.0));
    MyLinear5.AppendPoint(HGF2DPosition(20.0, 18.0));
    MyLinear5.AppendPoint(HGF2DPosition(20.0, 12.0));
    MyLinear5.AppendPoint(HGF2DPosition(17.0, 12.0));

    Poly1Point0d0 = HGF2DPosition(10.0, 10.0);
    Poly1Point0d1 = HGF2DPosition(15.0, 10.0);
    Poly1Point0d5 = HGF2DPosition(20.0, 20.0);
    Poly1Point1d0 = HGF2DPosition(10.0, 10.0 + (1.1 * MYEPSILON));

    }

//==================================================================================
// Constructor test
//==================================================================================
TEST_F (HGF2DComplexShapeTester, ConstructorTest)
    {

    // Default Constructor
    HGF2DComplexShape ComplexShape1;

    ASSERT_FALSE(ComplexShape1.IsSimple());
    ASSERT_TRUE(ComplexShape1.IsComplex());
    ASSERT_TRUE(ComplexShape1.IsEmpty());

    //Constructor with CoordSys
    HGF2DComplexShape ComplexShape2;

    ASSERT_FALSE(ComplexShape2.IsSimple());
    ASSERT_TRUE(ComplexShape2.IsComplex());
    ASSERT_TRUE(ComplexShape2.IsEmpty());

    //Constructor with a SimpleShape
    HGF2DComplexShape ComplexShape3(Rect1A);

    ASSERT_FALSE(ComplexShape3.IsSimple());
    ASSERT_TRUE(ComplexShape3.IsComplex());
    ASSERT_FALSE(ComplexShape3.IsEmpty());

    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(20.0, 20.0)));

    //Copy Constructor
    HGF2DComplexShape ComplexShape4(ComplexShape3);

    ASSERT_FALSE(ComplexShape4.IsSimple());
    ASSERT_TRUE(ComplexShape4.IsComplex());
    ASSERT_FALSE(ComplexShape4.IsEmpty());

    ASSERT_TRUE(ComplexShape4.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(ComplexShape4.IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(ComplexShape4.IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(ComplexShape4.IsPointOn(HGF2DPosition(20.0, 20.0)));

    }

//==================================================================================
// operator= test
// operator=(const HGF2DRectangle& pi_rObj);
//==================================================================================
TEST_F (HGF2DComplexShapeTester, OperatorTest)
    {
    
    HGF2DComplexShape ComplexShape1(Rect1A);
    HGF2DComplexShape ComplexShape2;

    ComplexShape2 = ComplexShape1;

    ASSERT_FALSE(ComplexShape2.IsSimple());
    ASSERT_TRUE(ComplexShape2.IsComplex());
    ASSERT_FALSE(ComplexShape2.IsEmpty());

    ASSERT_TRUE(ComplexShape2.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(ComplexShape2.IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(ComplexShape2.IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(ComplexShape2.IsPointOn(HGF2DPosition(20.0, 20.0)));

    }

//==================================================================================
// IsSimple() const;
// IsComplex() const;
// GetShapeType() const;
// IsEmpty () const;
// HasHoles() const;
//==================================================================================
TEST_F (HGF2DComplexShapeTester, IsTest)
    {

    // Empty HGF2DComplexShape 
    HGF2DComplexShape ComplexShape1;

    ASSERT_FALSE(ComplexShape1.IsSimple());
    ASSERT_TRUE(ComplexShape1.IsComplex());
    ASSERT_EQ(HGF2DComplexShape::CLASS_ID, ComplexShape1.GetShapeType());
    ASSERT_TRUE(ComplexShape1.IsEmpty());
    ASSERT_FALSE(ComplexShape1.HasHoles());

    // Rectangle HGF2DComplexShape  
    ComplexShape1.AddShape(Rect1A);

    ASSERT_FALSE(ComplexShape1.IsSimple());
    ASSERT_TRUE(ComplexShape1.IsComplex());
    ASSERT_FALSE(ComplexShape1.IsEmpty());
    ASSERT_FALSE(ComplexShape1.HasHoles());
    ASSERT_EQ(HGF2DComplexShape::CLASS_ID, ComplexShape1.GetShapeType());

    // HoledShape HGF2DComplexShape
    HGF2DHoledShape HoledShape1(Rect1A);
    HoledShape1.AddHole(HGF2DRectangle(15.0, 15.0, 17.0, 17.0));
    HGF2DComplexShape ComplexShape2(HoledShape1);
    
    ASSERT_FALSE(ComplexShape2.IsSimple());
    ASSERT_TRUE(ComplexShape2.IsComplex());
    ASSERT_FALSE(ComplexShape2.IsEmpty());
    ASSERT_FALSE(ComplexShape2.HasHoles());
    ASSERT_EQ(HGF2DComplexShape::CLASS_ID, ComplexShape2.GetShapeType());

    }

//==================================================================================
//GetShapeList() const;
//==================================================================================
TEST_F (HGF2DComplexShapeTester, GetShapeListTest)
    {

    // Empty HGF2DComplexShape
    HGF2DComplexShape ComplexShape;
    ASSERT_EQ(0, ComplexShape.GetShapeList().size());

    // HoledShape HGF2DComplexShape
    HGF2DHoledShape HoledShape1(Rect1A);
    HoledShape1.AddHole(HGF2DRectangle(15.0, 15.0, 17.0, 17.0));
    ComplexShape.AddShape(HoledShape1);

    ASSERT_EQ(1, ComplexShape.GetShapeList().size());

    // HoledShape + Rectangle HGF2DComplexShape
    ComplexShape.AddShape(HGF2DRectangle(15.5, 15.5, 16.5, 16.5));
    ASSERT_EQ(2, ComplexShape.GetShapeList().size());

    // 2 HoledShape + Rectangle HGF2DComplexShape
    HGF2DHoledShape HoledShape2(HGF2DRectangle(0.0, 0.0, 100.0, 100.0));
    HoledShape2.AddHole(HGF2DRectangle(5.0, 5.0, 25.0, 25.0));
    
    ComplexShape.AddShape(HoledShape2);
    ASSERT_EQ(3, ComplexShape.GetShapeList().size());

    }

//==================================================================================
// CalculatePerimeter() const;
//==================================================================================
TEST_F (HGF2DComplexShapeTester, CalculatePerimeterTest)
    {

    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    // With Polygon
    ASSERT_DOUBLE_EQ(HoledShape1.CalculatePerimeter(), Poly1A.CalculatePerimeter() + Poly1B.CalculatePerimeter() );

    //With Rectangle
    ASSERT_DOUBLE_EQ(40.0, HGF2DComplexShape(Rect1A).CalculatePerimeter());
    ASSERT_DOUBLE_EQ(20.0, HGF2DComplexShape(IncludedRect1A).CalculatePerimeter());
    ASSERT_DOUBLE_EQ(20.0, HGF2DComplexShape(IncludedRect2A).CalculatePerimeter());
    ASSERT_DOUBLE_EQ(20.0, HGF2DComplexShape(IncludedRect3A).CalculatePerimeter());

    }

//==================================================================================
// CalculateArea() const;
//==================================================================================
TEST_F (HGF2DComplexShapeTester, CalculateAreaTest)
    {

    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    //With Polygon
    ASSERT_DOUBLE_EQ(HoledShape1.CalculateArea(), Poly1A.CalculateArea() - Poly1B.CalculateArea());

    //With Rectangle
    ASSERT_DOUBLE_EQ(100.0, HGF2DComplexShape(Rect1A).CalculateArea());
    ASSERT_DOUBLE_EQ(100.0, HGF2DComplexShape(NegativeRectA).CalculateArea());
    ASSERT_DOUBLE_EQ(25.00, HGF2DComplexShape(IncludedRect1A).CalculateArea());
    ASSERT_DOUBLE_EQ(25.00, HGF2DComplexShape(IncludedRect2A).CalculateArea());

    }

//==================================================================================
// IsEmpty() test
// MakEmpty() test
//==================================================================================
TEST_F (HGF2DComplexShapeTester, EmptyTest)
    {

    HGF2DComplexShape  ComplexShape(Rect1A);

    ComplexShape.MakeEmpty();

    ASSERT_TRUE(ComplexShape.IsEmpty());
    ASSERT_EQ(HGF2DComplexShape::CLASS_ID, ComplexShape.GetShapeType());

    }

//==================================================================================
// Drop( HGF2DPositionCollection* po_pPoints, const HGFDistance& pi_rTolerance ) const
//==================================================================================
TEST_F (HGF2DComplexShapeTester, DropTest)
    {

    // With Rectangle
    HGF2DPositionCollection Locations;
    HGF2DComplexShape  ComplexShape1(Rect1A);

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
    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    HGF2DComplexShape  ComplexShape2(HoledShape1);

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
// CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const;
//==================================================================================
TEST_F (HGF2DComplexShapeTester, CalculateClosestPointTest)
    {

    // With Rectangle
    HGF2DComplexShape  ComplexShape1(Rect1A);

    ASSERT_DOUBLE_EQ(20.0, ComplexShape1.CalculateClosestPoint(RectClosePoint1AA).GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexShape1.CalculateClosestPoint(RectClosePoint1AA).GetY());
    ASSERT_DOUBLE_EQ(10.0, ComplexShape1.CalculateClosestPoint(RectClosePoint1BA).GetX());
    ASSERT_DOUBLE_EQ(10.0, ComplexShape1.CalculateClosestPoint(RectClosePoint1BA).GetY());
    ASSERT_DOUBLE_EQ(19.9, ComplexShape1.CalculateClosestPoint(RectClosePoint1CA).GetX());
    ASSERT_DOUBLE_EQ(10.0, ComplexShape1.CalculateClosestPoint(RectClosePoint1CA).GetY());
    ASSERT_DOUBLE_EQ(10.0, ComplexShape1.CalculateClosestPoint(RectClosePoint1DA).GetX());
    ASSERT_DOUBLE_EQ(15.0, ComplexShape1.CalculateClosestPoint(RectClosePoint1DA).GetY());
    ASSERT_DOUBLE_EQ(15.0, ComplexShape1.CalculateClosestPoint(RectCloseMidPoint1A).GetX());
    ASSERT_DOUBLE_EQ(20.0, ComplexShape1.CalculateClosestPoint(RectCloseMidPoint1A).GetY());
    ASSERT_DOUBLE_EQ(20.0, ComplexShape1.CalculateClosestPoint(VeryFarPointA).GetX());
    ASSERT_DOUBLE_EQ(20.0, ComplexShape1.CalculateClosestPoint(VeryFarPointA).GetY());
    ASSERT_DOUBLE_EQ(15.0, ComplexShape1.CalculateClosestPoint(RectMidPoint1A).GetX());
    ASSERT_DOUBLE_EQ(20.0, ComplexShape1.CalculateClosestPoint(RectMidPoint1A).GetY());

    // With HoledShape
    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    HGF2DComplexShape  ComplexShape2(HoledShape1);

    ASSERT_DOUBLE_EQ(10.0, ComplexShape2.CalculateClosestPoint(HGF2DPosition(0.0, 0.0)).GetX());
    ASSERT_DOUBLE_EQ(10.0, ComplexShape2.CalculateClosestPoint(HGF2DPosition(0.0, 0.0)).GetY());
    ASSERT_DOUBLE_EQ(10.0, ComplexShape2.CalculateClosestPoint(HGF2DPosition(10.0, 10.0)).GetX());
    ASSERT_DOUBLE_EQ(10.0, ComplexShape2.CalculateClosestPoint(HGF2DPosition(10.0, 10.0)).GetY());
    ASSERT_DOUBLE_EQ(10.0, ComplexShape2.CalculateClosestPoint(HGF2DPosition(11.0, 11.0)).GetX());
    ASSERT_DOUBLE_EQ(11.0, ComplexShape2.CalculateClosestPoint(HGF2DPosition(11.0, 11.0)).GetY());
    ASSERT_DOUBLE_EQ(12.0, ComplexShape2.CalculateClosestPoint(HGF2DPosition(11.5, 11.5)).GetX());
    ASSERT_DOUBLE_EQ(12.0, ComplexShape2.CalculateClosestPoint(HGF2DPosition(11.5, 11.5)).GetY());
    ASSERT_DOUBLE_EQ(12.0, ComplexShape2.CalculateClosestPoint(HGF2DPosition(13.0, 13.0)).GetX());
    ASSERT_DOUBLE_EQ(13.0, ComplexShape2.CalculateClosestPoint(HGF2DPosition(13.0, 13.0)).GetY());

    }

//==================================================================================
// Intersect(const HGF2DVector& pi_rVector, HGF2DPositionCollection* po_pCrossPoints) const;
//==================================================================================
TEST_F (HGF2DComplexShapeTester, IntersectTest)
    {

    //With Rectangle
    HGF2DPositionCollection   DumPoints;
    HGF2DComplexShape  ComplexShape1(Rect1A);

    ASSERT_EQ(0, ComplexShape1.Intersect(DisjointLinear1A, &DumPoints));
    ASSERT_EQ(0, ComplexShape1.Intersect(ContiguousExtentLinear1A, &DumPoints));
    ASSERT_EQ(0, ComplexShape1.Intersect(FlirtingExtentLinear1A, &DumPoints));
    ASSERT_EQ(0, ComplexShape1.Intersect(ConnectingLinear1B, &DumPoints));
    ASSERT_EQ(0, ComplexShape1.Intersect(ConnectingLinear1B, &DumPoints));
    ASSERT_EQ(0, ComplexShape1.Intersect(LinkedLinear1B, &DumPoints));

    ASSERT_EQ(1, ComplexShape1.Intersect(ComplexLinearCase1A, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(13.5, DumPoints[0].GetY());

    DumPoints.clear();
    ASSERT_EQ(1, ComplexShape1.Intersect(ComplexLinearCase2A, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(15.0, DumPoints[0].GetY());

    DumPoints.clear();
    ASSERT_EQ(1, ComplexShape1.Intersect(ComplexLinearCase3A, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());

    DumPoints.clear();
    ASSERT_EQ(0, ComplexShape1.Intersect(ComplexLinearCase4A, &DumPoints));

    DumPoints.clear();
    ASSERT_EQ(1, ComplexShape1.Intersect(ComplexLinearCase5B, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());

    DumPoints.clear();
    ASSERT_EQ(0, ComplexShape1.Intersect(ComplexLinearCase5AA, &DumPoints));
    ASSERT_EQ(0, ComplexShape1.Intersect(ComplexLinearCase6A, &DumPoints));
    ASSERT_EQ(0, ComplexShape1.Intersect(ComplexLinearCase7A, &DumPoints));

    // With HoledShape
    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);
    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);
    HGF2DComplexShape  ComplexShape2(HoledShape1);

    HGF2DSegment Segment1(HGF2DPosition(-10.0, 0.0), HGF2DPosition(0.0, 0.0));
    ASSERT_EQ(0, ComplexShape2.Intersect(Segment1, &DumPoints));

    HGF2DSegment Segment2(HGF2DPosition(-10.0, 0.0), HGF2DPosition(10.0 - MYEPSILON, 10.0 - MYEPSILON));
    ASSERT_EQ(0, ComplexShape2.Intersect(Segment2, &DumPoints));

    HGF2DSegment Segment3(HGF2DPosition(-10.0, 0.0), HGF2DPosition(10.0, 10.0));
    ASSERT_EQ(0, ComplexShape2.Intersect(Segment3, &DumPoints));

    HGF2DSegment Segment4(HGF2DPosition(9.0, 9.0), HGF2DPosition(13.0, 13.0));
    ASSERT_EQ(2, ComplexShape2.Intersect(Segment4, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(12.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(12.0, DumPoints[1].GetY());

    DumPoints.clear();

    HGF2DSegment Segment5(HGF2DPosition(9.0, 9.0), HGF2DPosition(21.0, 21.0));
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

    HGF2DSegment Segment6(HGF2DPosition(13.0, 13.0), HGF2DPosition(15.0, 15.0));
    ASSERT_EQ(0, ComplexShape2.Intersect(Segment6, &DumPoints));

    HGF2DSegment Segment7(HGF2DPosition(13.0, 13.0), HGF2DPosition(21.0, 21.0));
    ASSERT_EQ(2, ComplexShape2.Intersect(Segment7, &DumPoints));

    }

//==================================================================================
// IsPointIn(const HGF2DPosition& pi_rPoint) const;
//==================================================================================
TEST_F (HGF2DComplexShapeTester, IsPointInTest)
    {
    
    // With Rectangle
    ASSERT_FALSE(HGF2DComplexShape(Rect1A).IsPointIn(HGF2DPosition(10.0+0.9*MYEPSILON, 15.0)));
    ASSERT_FALSE(HGF2DComplexShape(Rect1A).IsPointIn(HGF2DPosition(15.0, 20.0-0.9*MYEPSILON)));
    ASSERT_FALSE(HGF2DComplexShape(Rect1A).IsPointIn(HGF2DPosition(20.0-0.9*MYEPSILON, 15.0)));
    ASSERT_FALSE(HGF2DComplexShape(Rect1A).IsPointIn(HGF2DPosition(15.0, 10.0+0.9*MYEPSILON)));
    ASSERT_TRUE(HGF2DComplexShape(Rect1A).IsPointIn(HGF2DPosition(10.0+1.1*MYEPSILON, 15.0)));
    ASSERT_TRUE(HGF2DComplexShape(Rect1A).IsPointIn(HGF2DPosition(15.0, 20.0-1.1*MYEPSILON)));
    ASSERT_TRUE(HGF2DComplexShape(Rect1A).IsPointIn(HGF2DPosition(20.0-1.1*MYEPSILON, 15.0)));
    ASSERT_TRUE(HGF2DComplexShape(Rect1A).IsPointIn(HGF2DPosition(15.0, 10.0+1.1*MYEPSILON)));

    // With HoledShape
    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    ASSERT_TRUE(HGF2DComplexShape(HoledShape1).IsPointIn(HGF2DPosition(13.0, 19.0)));
    ASSERT_TRUE(HGF2DComplexShape(HoledShape1).IsPointIn(HGF2DPosition(19.0, 13.0)));
    ASSERT_TRUE(HGF2DComplexShape(HoledShape1).IsPointIn(HGF2DPosition(19.0, 19.0)));
    ASSERT_FALSE(HGF2DComplexShape(HoledShape1).IsPointIn(HGF2DPosition(13.0, 13.0)));
    ASSERT_FALSE(HGF2DComplexShape(HoledShape1).IsPointIn(HGF2DPosition(14.0, 14.0)));
    ASSERT_FALSE(HGF2DComplexShape(HoledShape1).IsPointIn(HGF2DPosition(15.0, 15.0)));

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
TEST_F (HGF2DComplexShapeTester, ContiguousnessTest)
    {
    HGF2DPositionCollection     DumPoints;
    HGF2DPosition   FirstDumPoint;
    HGF2DPosition   SecondDumPoint;

    // Rectangle HGF2DComplexShape
    HGF2DComplexShape CSRect1(Rect1A);

    ASSERT_TRUE(CSRect1.AreContiguous(ComplexLinearCase6A));

    ASSERT_TRUE(CSRect1.AreContiguousAt(ComplexLinearCase6A, RectMidPoint1A));

    ASSERT_EQ(2, CSRect1.ObtainContiguousnessPoints(ComplexLinearCase6A, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetY());

    CSRect1.ObtainContiguousnessPointsAt(ComplexLinearCase6A, LinearMidPoint1A, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(10.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetY());

    // Test with non contiguous linears
    ASSERT_FALSE(CSRect1.AreContiguous(ComplexLinearCase1A));

    // Test with contiguous rectangle
    DumPoints.clear();

    ASSERT_TRUE(CSRect1.AreContiguous(NorthContiguousRectA));

    ASSERT_TRUE(CSRect1.AreContiguousAt(NorthContiguousRectA, RectMidPoint1A));

    ASSERT_EQ(2, CSRect1.ObtainContiguousnessPoints(NorthContiguousRectA, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[1].GetY());

    CSRect1.ObtainContiguousnessPointsAt(NorthContiguousRectA, RectMidPoint1A, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(10.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(20.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, SecondDumPoint.GetY());

    DumPoints.clear();

    // HoledShape HGF2DComplexShape
    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    HGF2DComplexShape CSHoledShape1(HoledShape1);

    // Test with contiguous linears
    ASSERT_TRUE(CSHoledShape1.AreContiguous(ComplexLinearCase6A));
    ASSERT_TRUE(CSHoledShape1.AreContiguousAt(ComplexLinearCase6A, HGF2DPosition(15.0, 20.0)));
    ASSERT_EQ(2, CSHoledShape1.ObtainContiguousnessPoints(ComplexLinearCase6A, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetY());

    CSHoledShape1.ObtainContiguousnessPointsAt(ComplexLinearCase6A, LinearMidPoint1A, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(10.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetY());

    DumPoints.clear();

    // Test with non contiguous linears
    ASSERT_FALSE(CSHoledShape1.AreContiguous(ComplexLinearCase1A));

    // Test contiguous linear with the hole
    HGF2DPolySegment ComplexLinearHole;
    ComplexLinearHole.AppendPoint(HGF2DPosition(12.0, 12.0));
    ComplexLinearHole.AppendPoint(HGF2DPosition(12.0, 18.0));

    ASSERT_TRUE(CSHoledShape1.AreContiguous(ComplexLinearHole));
    ASSERT_TRUE(CSHoledShape1.AreContiguousAt(ComplexLinearHole, HGF2DPosition(12.0, 12.0)));
    ASSERT_EQ(2, CSHoledShape1.ObtainContiguousnessPoints(ComplexLinearHole, &DumPoints));
    ASSERT_DOUBLE_EQ(12.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(12.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(12.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(18.0, DumPoints[1].GetY());

    CSHoledShape1.ObtainContiguousnessPointsAt(ComplexLinearHole, HGF2DPosition(12.0, 16.0), &FirstDumPoint, &SecondDumPoint);
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
TEST_F (HGF2DComplexShapeTester, CloningTest)
    {

    HGF2DComplexShape CSRect1(Rect1A);

    // General Clone Test
    HFCPtr<HGF2DComplexShape> pClone = (HGF2DComplexShape*) CSRect1.Clone();  
    ASSERT_FALSE(pClone->IsEmpty());
    ASSERT_EQ(HGF2DComplexShape::CLASS_ID, pClone->GetShapeType());

    ASSERT_TRUE(pClone->IsPointOn(HGF2DPosition(10.0, 10.0)));  
    ASSERT_TRUE(pClone->IsPointOn(HGF2DPosition(10.0, 20.0))); 
    ASSERT_TRUE(pClone->IsPointOn(HGF2DPosition(20.0, 20.0))); 
    ASSERT_TRUE(pClone->IsPointOn(HGF2DPosition(20.0, 10.0))); 



    // Test with a translation between systems
    Translation = HGF2DDisplacement (10.0, 10.0);
    HGF2DTranslation myTranslation(Translation);
    
    HFCPtr<HGF2DComplexShape> pClone5 = (HGF2DComplexShape*) (&*(CSRect1.AllocTransformDirect(myTranslation)));
    ASSERT_FALSE(pClone5->IsEmpty());
    ASSERT_EQ(HGF2DRectangle::CLASS_ID, pClone5->GetShapeType());

    ASSERT_TRUE(pClone5->IsPointOn(HGF2DPosition(0.0, 0.0)));  
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DPosition(0.0, 10.0))); 
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DPosition(10.0, 10.0))); 
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DPosition(10.0, 0.0))); 

    // Test with a stretch between systems
    HGF2DStretch myStretch;
    myStretch.SetTranslation(Translation);
    myStretch.SetXScaling(0.5);
    myStretch.SetYScaling(0.5);
    
    HFCPtr<HGF2DComplexShape> pClone6 = (HGF2DComplexShape*) (&*(CSRect1.AllocTransformDirect(myStretch)));
    ASSERT_FALSE(pClone6->IsEmpty());
    ASSERT_EQ(HGF2DRectangle::CLASS_ID, pClone6->GetShapeType());

    ASSERT_TRUE(pClone6->IsPointOn(HGF2DPosition(0.0, 0.0)));  
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DPosition(0.0, 20.0))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DPosition(20.0, 20.0))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DPosition(20.0, 0.0))); 

    // Test with a stretch between systems
    HGF2DStretch myStretch2;
    myStretch2.SetXScaling(0.5);
    myStretch2.SetYScaling(0.5);
  
    HFCPtr<HGF2DComplexShape> pClone10 = (HGF2DComplexShape*) (&*(CSRect1.AllocTransformDirect(myStretch2)));
    ASSERT_FALSE(pClone10->IsEmpty());
    ASSERT_EQ(HGF2DRectangle::CLASS_ID, pClone10->GetShapeType());

    ASSERT_TRUE(pClone10->IsPointOn(HGF2DPosition(20.0, 20.0)));  
    ASSERT_TRUE(pClone10->IsPointOn(HGF2DPosition(20.0, 40.0))); 
    ASSERT_TRUE(pClone10->IsPointOn(HGF2DPosition(40.0, 40.0))); 
    ASSERT_TRUE(pClone10->IsPointOn(HGF2DPosition(40.0, 40.0))); 

    // Test with a similitude between systems
    HGF2DSimilitude mySimilitude;
    mySimilitude.SetRotation(PI);
    mySimilitude.SetScaling(0.5);
  
    HFCPtr<HGF2DComplexShape> pClone7 = (HGF2DComplexShape*) (&*(CSRect1.AllocTransformDirect(mySimilitude)));
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
   
    HFCPtr<HGF2DComplexShape> pClone8 = (HGF2DComplexShape*) (&*(CSRect1.AllocTransformDirect(myAffine)));
    ASSERT_FALSE(pClone8->IsEmpty());
    ASSERT_EQ(HGF2DPolygonOfSegments::CLASS_ID, pClone8->GetShapeType());

    ASSERT_TRUE(pClone8->IsPointOn(HGF2DPosition(0.0, 0.0)));  
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DPosition(0.0, -20.0))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DPosition(-20.0, -20.0))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DPosition(-20.0, 0.0))); 

    #ifdef WIP_IPPTEST_BUG_16
    //// Test with a similitude between systems
    //CSRect1.MakeEmpty();

    //HFCPtr<HGF2DComplexShape> pClone9 = (HGF2DComplexShape*) CSRect1.AllocTransformDirect(pWorldSimilitude);

    //ASSERT_TRUE(pClone9->IsEmpty());
    //ASSERT_EQ(pWorldSimilitude, pClone9->GetCoordSys());
    //ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pClone9->GetShapeType());
    #endif
        
    }

//==================================================================================
// Interaction info with other segment
// Crosses(const HGF2DVector& pi_rVector) const;
// AreAdjacent(const HGF2DVector& pi_rVector) const;
// IsPointOn(const HGF2DPosition& pi_rTestPoint) const;
// IsPointOn(const HGF2DPosition& pi_rTestPoint) const;
//==================================================================================
TEST_F(HGF2DComplexShapeTester,  InteractionTest)
    {

    // Rectangle HGF2DComplexShape
    HGF2DComplexShape CSRect1(Rect1A);

    ASSERT_TRUE(CSRect1.Crosses(ComplexLinearCase1A));
    ASSERT_FALSE(CSRect1.AreAdjacent(ComplexLinearCase1A));

    ASSERT_TRUE(CSRect1.Crosses(ComplexLinearCase2A));
    ASSERT_TRUE(CSRect1.AreAdjacent(ComplexLinearCase2A));

    ASSERT_TRUE(CSRect1.Crosses(ComplexLinearCase3A));
    ASSERT_FALSE(CSRect1.AreAdjacent(ComplexLinearCase3A));

    ASSERT_FALSE(CSRect1.Crosses(ComplexLinearCase4A));
    ASSERT_TRUE(CSRect1.AreAdjacent(ComplexLinearCase4A));

    ASSERT_TRUE(CSRect1.Crosses(ComplexLinearCase5B));
    ASSERT_TRUE(CSRect1.AreAdjacent(ComplexLinearCase5B));

    ASSERT_FALSE(CSRect1.Crosses(ComplexLinearCase6A));
    ASSERT_TRUE(CSRect1.AreAdjacent(ComplexLinearCase6A));

    ASSERT_FALSE(CSRect1.Crosses(ComplexLinearCase7A));
    ASSERT_FALSE(CSRect1.AreAdjacent(ComplexLinearCase7A));

    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_FALSE(CSRect1.IsPointOn(HGF2DPosition(10.0 - 1.1*MYEPSILON, 10.0-1.1*MYEPSILON)));
    ASSERT_FALSE(CSRect1.IsPointOn(HGF2DPosition(RectMidPoint1A.GetX(), RectMidPoint1A.GetY() -1.1*MYEPSILON)));
    ASSERT_FALSE(CSRect1.IsPointOn(HGF2DPosition(RectMidPoint1A.GetX(), RectMidPoint1A.GetY() +1.1*MYEPSILON)));
    ASSERT_TRUE(CSRect1.IsPointOn (HGF2DPosition(RectMidPoint1A.GetX(), RectMidPoint1A.GetY() -0.9*MYEPSILON)));
    ASSERT_TRUE(CSRect1.IsPointOn (HGF2DPosition(RectMidPoint1A.GetX(), RectMidPoint1A.GetY() +0.9*MYEPSILON)));
    ASSERT_TRUE(CSRect1.IsPointOn(RectMidPoint1A));
    ASSERT_TRUE(CSRect1.IsPointOn(Rect1A.GetLinear()->GetStartPoint(), HGF2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(CSRect1.IsPointOn(Rect1A.GetLinear()->GetEndPoint(), HGF2DVector::EXCLUDE_EXTREMITIES));


    // HoledShape HGF2DComplexShape
    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    HGF2DComplexShape CSHoledShape(HoledShape1);
     
    // Tests with a vertical segment
    ASSERT_TRUE(CSHoledShape.Crosses(ComplexLinearCase1A));
    ASSERT_FALSE(CSHoledShape.AreAdjacent(ComplexLinearCase1A));

    ASSERT_TRUE(CSHoledShape.Crosses(ComplexLinearCase2A));
    ASSERT_TRUE(CSHoledShape.AreAdjacent(ComplexLinearCase2A));

    ASSERT_TRUE(CSHoledShape.Crosses(ComplexLinearCase3A));
    ASSERT_FALSE(CSHoledShape.AreAdjacent(ComplexLinearCase3A));

    ASSERT_FALSE(CSHoledShape.Crosses(ComplexLinearCase4A));
    ASSERT_TRUE(CSHoledShape.AreAdjacent(ComplexLinearCase4A));

    ASSERT_TRUE(CSHoledShape.Crosses(ComplexLinearCase5B));
    ASSERT_TRUE(CSHoledShape.AreAdjacent(ComplexLinearCase5B));

    ASSERT_FALSE(CSHoledShape.Crosses(ComplexLinearCase6A));
    ASSERT_TRUE(CSHoledShape.AreAdjacent(ComplexLinearCase6A));

    ASSERT_FALSE(CSHoledShape.Crosses(ComplexLinearCase7A));
    ASSERT_FALSE(CSHoledShape.AreAdjacent(ComplexLinearCase7A));

    ASSERT_TRUE(CSHoledShape.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_FALSE(CSHoledShape.IsPointOn(HGF2DPosition(10.0 - 1.1 * MYEPSILON, 10.0-1.1 * MYEPSILON)));
    ASSERT_FALSE(CSHoledShape.IsPointOn(HGF2DPosition(15.0, 20.0 - 1.1 * MYEPSILON)));
    ASSERT_FALSE(CSHoledShape.IsPointOn(HGF2DPosition(15.0, 20.0 + 1.1 * MYEPSILON)));
    ASSERT_TRUE(CSHoledShape.IsPointOn(HGF2DPosition(15.0, 20.0 + 0.9 * MYEPSILON)));
    ASSERT_TRUE(CSHoledShape.IsPointOn(HGF2DPosition(15.0, 20.0 - 0.9 * MYEPSILON)));
    ASSERT_TRUE(CSHoledShape.IsPointOn(HGF2DPosition(15.0, 20.0)));
  
    }

//==================================================================================
// Bearing calculation tests
// CalculateBearing(const HGF2DPosition& pi_rPositionPoint,
//                  HGF2DVector::ArbitraryDiPolyion pi_DiPolyion = HGF2DVector::BETA) const;
//==================================================================================
TEST_F(HGF2DComplexShapeTester,  BearingTest)
    {

    // Rectangle HGF2DComplexShape
    HGF2DComplexShape CSRect1(Rect1A);

    // Obtain bearing ALPHA of a vertical segment
    ASSERT_NEAR(0.0, CSRect1.CalculateBearing(Rect1Point0d0A, HGF2DVector::ALPHA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(PI/2, CSRect1.CalculateBearing(Rect1Point0d0A, HGF2DVector::BETA).GetAngle());
    ASSERT_NEAR(0.0, CSRect1.CalculateBearing(Rect1Point0d1A, HGF2DVector::ALPHA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(PI, CSRect1.CalculateBearing(Rect1Point0d1A, HGF2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(PI, CSRect1.CalculateBearing(Rect1Point0d5A, HGF2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(3*PI/2, CSRect1.CalculateBearing(Rect1Point0d5A, HGF2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(3*PI/2, CSRect1.CalculateBearing(Rect1Point1d0A, HGF2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(PI/2, CSRect1.CalculateBearing(Rect1Point1d0A, HGF2DVector::BETA).GetAngle());

    // HoledShape HGF2DComplexShape
    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);
    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    HGF2DComplexShape CSHoledShape1(HoledShape1);

    // Obtain bearing ALPHA of a vertical segment
    ASSERT_NEAR(0.0, CSHoledShape1.CalculateBearing(Poly1Point0d0, HGF2DVector::ALPHA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(PI/2, CSHoledShape1.CalculateBearing(Poly1Point0d0, HGF2DVector::BETA).GetAngle());
    ASSERT_NEAR(0.0, CSHoledShape1.CalculateBearing(Poly1Point0d1, HGF2DVector::ALPHA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(PI, CSHoledShape1.CalculateBearing(Poly1Point0d1, HGF2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(PI, CSHoledShape1.CalculateBearing(Poly1Point0d5, HGF2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(3*PI/2, CSHoledShape1.CalculateBearing(Poly1Point0d5, HGF2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(3*PI/2, CSHoledShape1.CalculateBearing(Poly1Point1d0, HGF2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(PI/2, CSHoledShape1.CalculateBearing(Poly1Point1d0, HGF2DVector::BETA).GetAngle());
  
    }
   
//================================================================================== 
// CalculateAngularAcceleration(const HGF2DPosition& pi_rPositionPoint,
//                              HGF2DVector::ArbitraryDiPolyion pi_DiPolyion = HGF2DVector::BETA) const;
//==================================================================================
TEST_F (HGF2DComplexShapeTester, AngularAccelerationTest)
    {

    // Rectangle HGF2DComplexShape
    HGF2DComplexShape CSRect1(Rect1A);

    // Obtain angular acceleration ALPHA of a vertical segment
    ASSERT_NEAR(0.0, CSRect1.CalculateAngularAcceleration(Rect1Point0d0A, HGF2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, CSRect1.CalculateAngularAcceleration(Rect1Point0d0A, HGF2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, CSRect1.CalculateAngularAcceleration(Rect1Point0d1A, HGF2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, CSRect1.CalculateAngularAcceleration(Rect1Point0d1A, HGF2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, CSRect1.CalculateAngularAcceleration(Rect1Point0d5A, HGF2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, CSRect1.CalculateAngularAcceleration(Rect1Point0d5A, HGF2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, CSRect1.CalculateAngularAcceleration(Rect1Point1d0A, HGF2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, CSRect1.CalculateAngularAcceleration(Rect1Point1d0A, HGF2DVector::BETA), MYEPSILON);

    // HoledShape HGF2DComplexShape
    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);
    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    HGF2DComplexShape CSHoledShape1(HoledShape1);

    // Obtain angular acceleration ALPHA of a vertical segment
    ASSERT_NEAR(0.0, CSHoledShape1.CalculateAngularAcceleration(Poly1Point0d0, HGF2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, CSHoledShape1.CalculateAngularAcceleration(Poly1Point0d0, HGF2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, CSHoledShape1.CalculateAngularAcceleration(Poly1Point0d1, HGF2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, CSHoledShape1.CalculateAngularAcceleration(Poly1Point0d1, HGF2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, CSHoledShape1.CalculateAngularAcceleration(Poly1Point0d5, HGF2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, CSHoledShape1.CalculateAngularAcceleration(Poly1Point0d5, HGF2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, CSHoledShape1.CalculateAngularAcceleration(Poly1Point1d0, HGF2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, CSHoledShape1.CalculateAngularAcceleration(Poly1Point1d0, HGF2DVector::BETA), MYEPSILON);

    }

//==================================================================================
// Extent calculation test
// GetExtent() const;
//==================================================================================
TEST_F (HGF2DComplexShapeTester, GetExtentTest)
    {

    // Rectangle HGF2DComplexShape
    ASSERT_DOUBLE_EQ(10.0, HGF2DComplexShape(Rect1A).GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(10.0, HGF2DComplexShape(Rect1A).GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(20.0, HGF2DComplexShape(Rect1A).GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(20.0, HGF2DComplexShape(Rect1A).GetExtent().GetYMax());

    ASSERT_DOUBLE_EQ(10.0, HGF2DComplexShape(IncludedRect1A).GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(10.0, HGF2DComplexShape(IncludedRect1A).GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(15.0, HGF2DComplexShape(IncludedRect1A).GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(15.0, HGF2DComplexShape(IncludedRect1A).GetExtent().GetYMax());

    ASSERT_DOUBLE_EQ(15.0, HGF2DComplexShape(IncludedRect2A).GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(10.0, HGF2DComplexShape(IncludedRect2A).GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(20.0, HGF2DComplexShape(IncludedRect2A).GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(15.0, HGF2DComplexShape(IncludedRect2A).GetExtent().GetYMax());

    ASSERT_DOUBLE_EQ(15.0, HGF2DComplexShape(IncludedRect3A).GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(15.0, HGF2DComplexShape(IncludedRect3A).GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(20.0, HGF2DComplexShape(IncludedRect3A).GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(20.0, HGF2DComplexShape(IncludedRect3A).GetExtent().GetYMax());

    // HoledShape HGF2DComplexShape
    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);
    HGF2DHoledShape NoHoledShape1(Poly1A);
    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    HGF2DComplexShape CSHoledShape1(HoledShape1);
    HGF2DComplexShape CSNoHoledShape1(NoHoledShape1);

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
TEST_F (HGF2DComplexShapeTester,  MoveTest) 
    { 

    // Rectangle HGF2DComplexShape
    HGF2DComplexShape CSRect1(Rect1A);

    CSRect1.Move(HGF2DDisplacement(10.0, 10.0));

    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DPosition(30.0, 20.0)));
    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DPosition(20.0, 30.0)));
    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DPosition(30.0, 30.0)));

    CSRect1.Move(HGF2DDisplacement(-10.0, -10.0));

    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DPosition(20.0, 20.0)));

    // HoledShape HGF2DComplexShape
    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);
    HGF2DComplexShape CSHoledShape1(HoledShape1);

    CSHoledShape1.Move(HGF2DDisplacement(10.0, 10.0));

    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(30.0, 20.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(20.0, 30.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(30.0, 30.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(22.0, 22.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(22.0, 28.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(28.0, 22.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(28.0, 28.0)));

    CSHoledShape1.Move(HGF2DDisplacement(-10.0, -10.0));

    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(12.0, 12.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(12.0, 18.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(18.0, 12.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(18.0, 18.0)));

    }

////==================================================================================
// Scale(double pi_ScaleFactor, const HGF2DPosition& pi_rScaleOrigin);
////==================================================================================
TEST_F (HGF2DComplexShapeTester,  ScaleTest) 
    { 

    // Rectangle HGF2DComplexShape
    HGF2DComplexShape CSRect1(Rect1A);
    HGF2DPosition Origin(0.0, 0.0);

    CSRect1.Scale(2.0, Origin);

    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DPosition(20.0, 20.0))); 
    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DPosition(40.0, 20.0)));
    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DPosition(20.0, 40.0)));
    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DPosition(40.0, 40.0)));

    CSRect1.Scale(2.0, HGF2DPosition(10.0, 10.0));

    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DPosition(30.0, 30.0))); 
    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DPosition(70.0, 30.0)));
    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DPosition(30.0, 70.0)));
    ASSERT_TRUE(CSRect1.IsPointOn(HGF2DPosition(70.0, 70.0)));
   
    // HoledShape HGF2DComplexShape
    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);
    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);
    HGF2DComplexShape CSHoledShape1(HoledShape1);

    CSHoledShape1.Scale(1.0, HGF2DPosition(0.0, 0.0));

    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(12.0, 12.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(12.0, 18.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(18.0, 12.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(18.0, 18.0)));

    CSHoledShape1.Scale(2.0, HGF2DPosition(0.0, 0.0));

    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(40.0, 20.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(20.0, 40.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(40.0, 40.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(24.0, 24.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(24.0, 36.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(36.0, 24.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(36.0, 36.0)));

    CSHoledShape1.Scale(2.0, HGF2DPosition(10.0, 20.0));

    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(30.0, 20.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(70.0, 20.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(30.0, 60.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(70.0, 60.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(38.0, 28.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(38.0, 52.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(62.0, 28.0)));
    ASSERT_TRUE(CSHoledShape1.IsPointOn(HGF2DPosition(62.0, 52.0)));
    
    }

//==================================================================================
// AddShape() test
//==================================================================================
TEST_F (HGF2DComplexShapeTester, AddShapeTest)
    {

    // Empty HGF2DComplexShape add Rectangle
    HGF2DComplexShape ComplexShape1;
    ComplexShape1.AddShape(Rect1A);

    ASSERT_TRUE(ComplexShape1.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(ComplexShape1.IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(ComplexShape1.IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(ComplexShape1.IsPointOn(HGF2DPosition(20.0, 20.0)));

    // Rectangle HGF2DComplexShape add another Rectangle
    ComplexShape1.AddShape(HGF2DRectangle(30.0, 30.0, 50.0, 50.0));

    ASSERT_TRUE(ComplexShape1.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(ComplexShape1.IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(ComplexShape1.IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(ComplexShape1.IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(ComplexShape1.IsPointOn(HGF2DPosition(30.0, 30.0)));
    ASSERT_TRUE(ComplexShape1.IsPointOn(HGF2DPosition(50.0, 30.0)));
    ASSERT_TRUE(ComplexShape1.IsPointOn(HGF2DPosition(30.0, 50.0)));
    ASSERT_TRUE(ComplexShape1.IsPointOn(HGF2DPosition(50.0, 50.0)));

    // Empty HGF2DComplexShape add a Polygon
    HGF2DPolygonOfSegments Poly1A(Rect1A);
    HGF2DComplexShape ComplexShape2;

    ComplexShape2.AddShape(Poly1A);

    ASSERT_TRUE(ComplexShape2.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(ComplexShape2.IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(ComplexShape2.IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(ComplexShape2.IsPointOn(HGF2DPosition(20.0, 20.0)));

    // Empty HGF2DComplexShape add a HoledShape
    HGF2DHoledShape HoledShape1(Poly1A);
    HoledShape1.AddHole(HGF2DRectangle(15.0, 15.0, 17.0, 17.0));
    HGF2DComplexShape ComplexShape3;

    ComplexShape3.AddShape(HoledShape1);

    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(20.0, 20.0)));

    // Add a Rectangle inside the HoledShape HGF2DComplexShape
    ComplexShape3.AddShape(HGF2DRectangle(15.5, 15.5, 16.5, 16.5));

    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(15.5, 15.5)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(16.5, 15.5)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(15.5, 16.5)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(16.5, 16.5)));

    HGF2DHoledShape HoledShape2(HGF2DRectangle(0.0, 0.0, 100.0, 100.0));
    HoledShape2.AddHole(HGF2DRectangle(5.0, 5.0, 25.0, 25.0));
    
    // Add another HoledShape to ComplexShape3
    ComplexShape3.AddShape(HoledShape2);

    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(15.5, 15.5)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(16.5, 15.5)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(15.5, 16.5)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(16.5, 16.5)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(0.0, 0.0)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(100.0, 0.0)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(0.0, 100.0)));
    ASSERT_TRUE(ComplexShape3.IsPointOn(HGF2DPosition(100.0, 100.0)));
      
    }

//==================================================================================
// DifferentiateShapeSCS(const HGF2DShape& pi_rShape) const;
//==================================================================================
TEST_F (HGF2DComplexShapeTester, DifferentiateShapeTest)
    {

    HGF2DComplexShape CSRect1(Rect1A);

    // NorthContiguousRect only touch CSRect1
    HFCPtr<HGF2DComplexShape>     pResultShape1 = (HGF2DComplexShape*) CSRect1.DifferentiateShape(NorthContiguousRectA);
   
    ASSERT_EQ(HGF2DRectangle::CLASS_ID, pResultShape1->GetShapeType());

    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(20.0, 20.0)));

    // VerticalFitRect is partially in CSRect1
    HFCPtr<HGF2DComplexShape>     pResultShape2 = (HGF2DComplexShape*) CSRect1.DifferentiateShape(VerticalFitRectA);
  
    ASSERT_EQ(HGF2DRectangle::CLASS_ID, pResultShape2->GetShapeType());

    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(15.0, 10.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(15.0, 20.0)));

    // DisjointRect doesn't touch CSRect1
    HFCPtr<HGF2DComplexShape>     pResultShape3 = (HGF2DComplexShape*) CSRect1.DifferentiateShape(DisjointRectA);
    
    ASSERT_EQ(HGF2DRectangle::CLASS_ID, pResultShape3->GetShapeType());

    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(20.0, 20.0)));

    // EnglobRect1 contains CSRect1
    HFCPtr<HGF2DComplexShape>     pResultShape4 = (HGF2DComplexShape*) CSRect1.DifferentiateShape(EnglobRect1A);
    
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape4->GetShapeType());
    ASSERT_TRUE(pResultShape4->IsEmpty());

    // IncludedRect1 is inside CSRect1
    HFCPtr<HGF2DComplexShape>     pResultShape5 = (HGF2DComplexShape*) CSRect1.DifferentiateShape(IncludedRect1A);

    ASSERT_EQ(HGF2DPolygonOfSegments::CLASS_ID, pResultShape5->GetShapeType());

    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(10.0, 15.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(15.0, 10.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(15.0, 15.0)));

    // IncludedRect2 is completly inside CSRect1
    HGF2DRectangle IncludedRect2(11.0, 11.0, 19.0, 19.0);
    HFCPtr<HGF2DComplexShape>  pResultShape6 = (HGF2DComplexShape*) CSRect1.DifferentiateShape(IncludedRect2A);

    ASSERT_EQ(HGF2DHoledShape::CLASS_ID, pResultShape6->GetShapeType());

    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(11.0, 11.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(19.0, 11.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(11.0, 19.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(19.0, 19.0)));

    // IncludedRect3 is completly inside CSRect1 and IncludedRect2
    HGF2DRectangle IncludedRect3(13.0, 13.0, 17.0, 17.0);
    HFCPtr<HGF2DComplexShape>  pResultShape7 = (HGF2DComplexShape*) pResultShape6->DifferentiateShape(IncludedRect3A);

    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DPosition(11.0, 11.0)));
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DPosition(19.0, 11.0)));
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DPosition(11.0, 19.0)));
    ASSERT_TRUE(pResultShape7->IsPointOn(HGF2DPosition(19.0, 19.0)));

    }

//==================================================================================
// DifferentiateFromShape(const HGF2DShape& pi_rShape) const;
//==================================================================================
TEST_F (HGF2DComplexShapeTester, DifferentiateFromShapeTest)
    {

    HGF2DComplexShape CSRect1(Rect1A);

    // NorthContiguousRect only touch CSRect1
    HFCPtr<HGF2DComplexShape>     pResultShape1 = (HGF2DComplexShape*) NorthContiguousRectA.DifferentiateFromShape(CSRect1);
   
    ASSERT_EQ(HGF2DRectangle::CLASS_ID, pResultShape1->GetShapeType());

    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(20.0, 20.0)));

    // VerticalFitRect is partially in CSRect1
    HFCPtr<HGF2DComplexShape>     pResultShape2 = (HGF2DComplexShape*) VerticalFitRectA.DifferentiateFromShape(CSRect1);
  
    ASSERT_EQ(HGF2DRectangle::CLASS_ID, pResultShape2->GetShapeType());

    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(15.0, 10.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(15.0, 20.0)));

    // DisjointRect doesn't touch CSRect1
    HFCPtr<HGF2DComplexShape>     pResultShape3 = (HGF2DComplexShape*) DisjointRectA.DifferentiateFromShape(CSRect1);
    
    ASSERT_EQ(HGF2DRectangle::CLASS_ID, pResultShape3->GetShapeType());

    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(20.0, 20.0)));

    // EnglobRect1 contains CSRect1
    HFCPtr<HGF2DComplexShape>     pResultShape4 = (HGF2DComplexShape*) EnglobRect1A.DifferentiateFromShape(CSRect1);
    
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape4->GetShapeType());
    ASSERT_TRUE(pResultShape4->IsEmpty());

    // IncludedRect1 is inside CSRect1
    HFCPtr<HGF2DComplexShape>     pResultShape5 = (HGF2DComplexShape*) IncludedRect1A.DifferentiateFromShape(CSRect1);

    ASSERT_EQ(HGF2DPolygonOfSegments::CLASS_ID, pResultShape5->GetShapeType());

    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(10.0, 15.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(15.0, 10.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(15.0, 15.0)));

    // IncludedRect2 is completly inside CSRect1
    HGF2DRectangle IncludedRect2(11.0, 11.0, 19.0, 19.0);
    HFCPtr<HGF2DComplexShape>  pResultShape6 = (HGF2DComplexShape*) IncludedRect2A.DifferentiateFromShape(CSRect1);

    ASSERT_EQ(HGF2DHoledShape::CLASS_ID, pResultShape6->GetShapeType());

    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(11.0, 11.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(19.0, 11.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(11.0, 19.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(19.0, 19.0)));  

    }

//==================================================================================
// IntersectShape(const HGF2DShape& pi_rShape) const;
//==================================================================================
TEST_F (HGF2DComplexShapeTester, IntersectShapeTest)
    {

    HGF2DComplexShape CSRect1(Rect1A);

    // NorthContiguousRect only touch CSRect1
    HFCPtr<HGF2DComplexShape>     pResultShape1 = (HGF2DComplexShape*) NorthContiguousRectA.IntersectShape(CSRect1);

    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape1->GetShapeType());
    ASSERT_TRUE(pResultShape1->IsEmpty());
   
    // VerticalFitRect is partially in CSRect1
    HFCPtr<HGF2DComplexShape>     pResultShape2 = (HGF2DComplexShape*) VerticalFitRectA.IntersectShape(CSRect1);
  
    ASSERT_EQ(HGF2DRectangle::CLASS_ID, pResultShape2->GetShapeType());

    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(15.0, 15.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(15.0, 20.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(20.0, 15.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(20.0, 20.0)));

    // DisjointRect doesn't touch CSRect1
    HFCPtr<HGF2DComplexShape>     pResultShape3 = (HGF2DComplexShape*) DisjointRectA.IntersectShape(CSRect1);

    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape3->GetShapeType());
    ASSERT_TRUE(pResultShape3->IsEmpty());

    // EnglobRect1 contains CSRect1
    HFCPtr<HGF2DComplexShape>     pResultShape4 = (HGF2DComplexShape*) EnglobRect1A.IntersectShape(CSRect1);

    ASSERT_EQ(HGF2DRectangle::CLASS_ID, pResultShape4->GetShapeType());

    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(20.0, 20.0)));

    // IncludedRect1 is inside CSRect1
    HFCPtr<HGF2DComplexShape>     pResultShape5 = (HGF2DComplexShape*) IncludedRect1A.IntersectShape(CSRect1);

    ASSERT_EQ(HGF2DRectangle::CLASS_ID, pResultShape5->GetShapeType());

    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(10.0, 15.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(15.0, 10.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(15.0, 15.0)));

    // IncludedRect2 is completly inside CSRect1
    HGF2DRectangle IncludedRect2(11.0, 11.0, 19.0, 19.0);
    HFCPtr<HGF2DComplexShape>  pResultShape6 = (HGF2DComplexShape*) IncludedRect2A.IntersectShape(CSRect1);

    ASSERT_EQ(HGF2DRectangle::CLASS_ID, pResultShape6->GetShapeType());

    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(11.0, 11.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(19.0, 11.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(11.0, 19.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(19.0, 19.0)));

    }

//==================================================================================
// UnifyShape(const HGF2DShape& pi_rShape) const;
//==================================================================================
TEST_F (HGF2DComplexShapeTester, UnifyShapeTest)
    {

    HGF2DComplexShape CSRect1(Rect1A);

    // NorthContiguousRect only touch CSRect1
    HFCPtr<HGF2DComplexShape>     pResultShape1 = (HGF2DComplexShape*) NorthContiguousRectA.UnifyShape(CSRect1); 

    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(10.0, 30.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(20.0, 30.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(20.0, 10.0)));
   
    // VerticalFitRect is partially in CSRect1
    HFCPtr<HGF2DComplexShape>     pResultShape2 = (HGF2DComplexShape*) VerticalFitRectA.UnifyShape(CSRect1);

    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(25.0, 10.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(25.0, 20.0)));
  
    // DisjointRect doesn't touch CSRect1
    HFCPtr<HGF2DComplexShape>     pResultShape3 = (HGF2DComplexShape*) DisjointRectA.UnifyShape(CSRect1);

    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(20.0, 20.0)));

    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(-10.0, -10.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(-10.0, 0.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(0.0, -10.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(0.0, 0.0)));

    // EnglobRect1 contains CSRect1
    HFCPtr<HGF2DComplexShape>     pResultShape4 = (HGF2DComplexShape*) EnglobRect1A.UnifyShape(CSRect1);

    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(10.0, 30.0)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(30.0, 10.0)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(30.0, 30.0)));

    // IncludedRect1 is inside CSRect1
    HFCPtr<HGF2DComplexShape>     pResultShape5 = (HGF2DComplexShape*) IncludedRect1A.UnifyShape(CSRect1);

    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(20.0, 20.0)));

    // IncludedRect2 is completly inside CSRect1
    HGF2DRectangle IncludedRect2(11.0, 11.0, 19.0, 19.0);
    HFCPtr<HGF2DComplexShape>  pResultShape6 = (HGF2DComplexShape*) IncludedRect2A.UnifyShape(CSRect1);

    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(20.0, 20.0)));

    }