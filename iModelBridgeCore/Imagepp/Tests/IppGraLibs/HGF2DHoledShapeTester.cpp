//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DHoledShapeTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "EnvironnementTest.h"
#include "HGF2DHoledShapeTester.h"
// #include "HGF2DHoledShape.h"

HGF2DHoledShapeTester::HGF2DHoledShapeTester() 
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
    
    MyLinear4 = HGF2DPolySegment();
    MyLinear4.AppendPoint(HGF2DPosition(18.0, 12.0));
    MyLinear4.AppendPoint(HGF2DPosition(18.0, 18.0));
    MyLinear4.AppendPoint(HGF2DPosition(20.0, 18.0));
    MyLinear4.AppendPoint(HGF2DPosition(20.0, 12.0));
    MyLinear4.AppendPoint(HGF2DPosition(18.0, 12.0));
    
    
    MyLinear5 = HGF2DPolySegment();
    MyLinear5.AppendPoint(HGF2DPosition(12.0, 10.0));
    MyLinear5.AppendPoint(HGF2DPosition(12.0, 20.0));
    MyLinear5.AppendPoint(HGF2DPosition(21.0, 20.0));
    MyLinear5.AppendPoint(HGF2DPosition(21.0, 10.0));
    MyLinear5.AppendPoint(HGF2DPosition(12.0, 10.0));
    
    MyLinear6 = HGF2DPolySegment();
    MyLinear6.AppendPoint(HGF2DPosition(17.0, 12.0));
    MyLinear6.AppendPoint(HGF2DPosition(17.0, 18.0));
    MyLinear6.AppendPoint(HGF2DPosition(20.0, 18.0));
    MyLinear6.AppendPoint(HGF2DPosition(20.0, 12.0));
    MyLinear6.AppendPoint(HGF2DPosition(17.0, 12.0));

    Poly1Point0d0 = HGF2DPosition(10.0, 10.0);
    Poly1Point0d1 = HGF2DPosition(15.0, 10.0);
    Poly1Point0d5 = HGF2DPosition(20.0, 20.0);
    Poly1Point1d0 = HGF2DPosition(10.0, 10.0 + (1.1 * MYEPSILON));

    }

// THE PRESENT TEST CANNOT MAKE TESTS WITH NON-LINEAR TRANSFORMATION MODELS

////==================================================================================
// Constructor Test
////==================================================================================
TEST_F (HGF2DHoledShapeTester,  ConstructorTest) 
    { 

    //Default Constructor
    HGF2DHoledShape Shape1;

    //Constructor with CoordSys
    HGF2DHoledShape Shape2;

    ///With a simple shape
    HGF2DHoledShape Shape3(Rect1A);
    ASSERT_TRUE(Shape3.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(Shape3.IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(Shape3.IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(Shape3.IsPointOn(HGF2DPosition(20.0, 20.0)));

    //Copy Constructor
    HGF2DHoledShape Shape4(Shape3);
    ASSERT_TRUE(Shape4.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(Shape4.IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(Shape4.IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(Shape4.IsPointOn(HGF2DPosition(20.0, 20.0)));

    }

//==================================================================================
// operator=(const HGF2DPolygonOfSegments& pi_rObj);
//==================================================================================
TEST_F (HGF2DHoledShapeTester, OperatorTest)
    {
    
    HGF2DHoledShape Shape1(Rect1A);

    HGF2DHoledShape Shape2 = Shape1;

    ASSERT_TRUE(Shape2.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(Shape2.IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(Shape2.IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(Shape2.IsPointOn(HGF2DPosition(20.0, 20.0)));

    }

//==================================================================================
// AddHole(const HGF2DSimpleShape& pi_rSimpleShape);
// Drop(HGF2DPositionCollection* po_pPoint, const HGFDistance& pi_rTolerance) const;
//==================================================================================
TEST_F (HGF2DHoledShapeTester, AddHoleTest)
    {

    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);
    HGF2DHoledShape HoledShape2(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    ASSERT_TRUE(HoledShape1.HasHoles());
    ASSERT_DOUBLE_EQ(64.0, HoledShape1.CalculateArea());

    HGF2DPositionCollection Locations;
    HoledShape1.Drop(&Locations, HGLOBAL_EPSILON);

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

////==================================================================================
// SetBaseShape(const HGF2DSimpleShape& pi_rComplex);
// GetBaseShape() const;
////==================================================================================
TEST_F (HGF2DHoledShapeTester,  BaseShapeTest) 
    { 

    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    HoledShape1.SetBaseShape(HGF2DRectangle(14.0, 14.0, 16.0, 16.0)); 

    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(14.0, 14.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(14.0, 16.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(16.0, 14.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(16.0, 16.0)));

    HGF2DHoledShape BaseShape = HoledShape1.GetBaseShape();

    ASSERT_TRUE(BaseShape.IsPointOn(HGF2DPosition(14.0, 14.0)));
    ASSERT_TRUE(BaseShape.IsPointOn(HGF2DPosition(14.0, 16.0)));
    ASSERT_TRUE(BaseShape.IsPointOn(HGF2DPosition(16.0, 14.0)));
    ASSERT_TRUE(BaseShape.IsPointOn(HGF2DPosition(16.0, 16.0)));

    }

////==================================================================================
// IsSimple() const;
// IsComplex() const;
////==================================================================================
TEST_F (HGF2DHoledShapeTester,  SimpleTest) 
    { 
    
    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);
    HGF2DHoledShape NoHoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    ASSERT_FALSE(HoledShape1.IsSimple());
    ASSERT_FALSE(NoHoledShape1.IsSimple());

    ASSERT_FALSE(HoledShape1.IsComplex());
    ASSERT_FALSE(NoHoledShape1.IsComplex());

    }

////==================================================================================
// GetShapeList() const;
// HasHoles() const;
// GetHoleList() const;
////==================================================================================
TEST_F (HGF2DHoledShapeTester,  ListTest) 
    { 
    
    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);
    HGF2DHoledShape NoHoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    ASSERT_TRUE(HoledShape1.HasHoles());
    ASSERT_FALSE(NoHoledShape1.HasHoles());

    ASSERT_EQ(1, HoledShape1.GetHoleList().size());

    }

////==================================================================================
// CalculateArea() const;
// CalculatePerimeter() const;
////==================================================================================
TEST_F (HGF2DHoledShapeTester,  AreaPerimenterTest) 
    { 
    
    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    ASSERT_DOUBLE_EQ(HoledShape1.CalculateArea(), Poly1A.CalculateArea() - Poly1B.CalculateArea());
    ASSERT_DOUBLE_EQ(HoledShape1.CalculatePerimeter(), Poly1A.CalculatePerimeter() + Poly1B.CalculatePerimeter() );

    }

////==================================================================================
// MakeEmpty();
// IsEmpty();
////==================================================================================
TEST_F (HGF2DHoledShapeTester,  EmptyTest) 
    { 

    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    ASSERT_FALSE(HoledShape1.IsEmpty());
    
    HoledShape1.MakeEmpty();

    ASSERT_TRUE(HoledShape1.IsEmpty());

    }

////==================================================================================
//IsPointIn(const HGF2DPosition& pi_rPoint, double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const;
////==================================================================================
TEST_F (HGF2DHoledShapeTester,  IsPointInTest) 
    { 

    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    //On the Border
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DPosition(10.0, 10.0)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DPosition(20.0, 10.0)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DPosition(10.0, 20.0)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DPosition(20.0, 20.0)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DPosition(12.0, 12.0)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DPosition(12.0, 18.0)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DPosition(18.0, 12.0)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DPosition(18.0, 18.0)));

    //In the HoledShape but not in the hole
    ASSERT_TRUE(HoledShape1.IsPointIn(HGF2DPosition(11.0, 11.0)));
    ASSERT_TRUE(HoledShape1.IsPointIn(HGF2DPosition(19.0, 11.0)));
    ASSERT_TRUE(HoledShape1.IsPointIn(HGF2DPosition(11.0, 19.0)));
    ASSERT_TRUE(HoledShape1.IsPointIn(HGF2DPosition(19.0, 19.0)));
    ASSERT_TRUE(HoledShape1.IsPointIn(HGF2DPosition(13.0, 19.0)));
    ASSERT_TRUE(HoledShape1.IsPointIn(HGF2DPosition(19.0, 13.0)));
    ASSERT_TRUE(HoledShape1.IsPointIn(HGF2DPosition(19.0, 19.0)));

    //In the hole of the HoledShape
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DPosition(13.0, 13.0)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DPosition(14.0, 14.0)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DPosition(15.0, 15.0)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DPosition(16.0, 16.0)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DPosition(17.0, 17.0)));

    //Outside the HoledShape
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DPosition(0.0, 0.0)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DPosition(-14.0, 14.0)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DPosition(15.0, -15.0)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DPosition(16.0, -16.0)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DPosition(-17.0, 17.0)));

    }

////==================================================================================
// CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const;
////==================================================================================
TEST_F (HGF2DHoledShapeTester,  CalculateClosestPointTest) 
    { 
    
    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    ASSERT_DOUBLE_EQ(10.0, HoledShape1.CalculateClosestPoint(HGF2DPosition(0.0, 0.0)).GetX());
    ASSERT_DOUBLE_EQ(10.0, HoledShape1.CalculateClosestPoint(HGF2DPosition(0.0, 0.0)).GetY());
    ASSERT_DOUBLE_EQ(10.0, HoledShape1.CalculateClosestPoint(HGF2DPosition(10.0, 10.0)).GetX());
    ASSERT_DOUBLE_EQ(10.0, HoledShape1.CalculateClosestPoint(HGF2DPosition(10.0, 10.0)).GetY());
    ASSERT_DOUBLE_EQ(10.0, HoledShape1.CalculateClosestPoint(HGF2DPosition(11.0, 11.0)).GetX());
    ASSERT_DOUBLE_EQ(11.0, HoledShape1.CalculateClosestPoint(HGF2DPosition(11.0, 11.0)).GetY());
    ASSERT_DOUBLE_EQ(12.0, HoledShape1.CalculateClosestPoint(HGF2DPosition(11.5, 11.5)).GetX());
    ASSERT_DOUBLE_EQ(12.0, HoledShape1.CalculateClosestPoint(HGF2DPosition(11.5, 11.5)).GetY());
    ASSERT_DOUBLE_EQ(12.0, HoledShape1.CalculateClosestPoint(HGF2DPosition(13.0, 13.0)).GetX());
    ASSERT_DOUBLE_EQ(13.0, HoledShape1.CalculateClosestPoint(HGF2DPosition(13.0, 13.0)).GetY());

    }

////==================================================================================
// Intersect(const HGF2DVector& pi_rVector, HGF2DPositionCollection* po_pCrossPoints) const;
////==================================================================================
TEST_F (HGF2DHoledShapeTester,  IntersectTest) 
    { 
    
    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    HGF2DPositionCollection   DumPoints;

    //Segment doesn't touch the HoledShape
    HGF2DSegment Segment1(HGF2DPosition(-10.0, 0.0), HGF2DPosition(0.0, 0.0));
    ASSERT_EQ(0, HoledShape1.Intersect(Segment1, &DumPoints));

    //Segment flirt the HoledShape
    HGF2DSegment Segment2(HGF2DPosition(-10.0, 0.0), HGF2DPosition(10.0 - MYEPSILON, 10.0 - MYEPSILON));
    ASSERT_EQ(0, HoledShape1.Intersect(Segment2, &DumPoints));

    //Segment touch one point of the HoledShape
    HGF2DSegment Segment3(HGF2DPosition(-10.0, 0.0), HGF2DPosition(10.0, 10.0));
    ASSERT_EQ(0, HoledShape1.Intersect(Segment3, &DumPoints));

    //Segment pass throught
    HGF2DSegment Segment4(HGF2DPosition(9.0, 9.0), HGF2DPosition(13.0, 13.0));
    ASSERT_EQ(2, HoledShape1.Intersect(Segment4, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(12.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(12.0, DumPoints[1].GetY());

    DumPoints.clear();

    //Segment pass completly throught
    HGF2DSegment Segment5(HGF2DPosition(9.0, 9.0), HGF2DPosition(21.0, 21.0));
    ASSERT_EQ(4, HoledShape1.Intersect(Segment5, &DumPoints));
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetY());
    ASSERT_DOUBLE_EQ(18.0, DumPoints[2].GetX());
    ASSERT_DOUBLE_EQ(18.0, DumPoints[2].GetY());
    ASSERT_DOUBLE_EQ(12.0, DumPoints[3].GetX());
    ASSERT_DOUBLE_EQ(12.0, DumPoints[3].GetY());

    DumPoints.clear();

    //Segment completly in the hole
    HGF2DSegment Segment6(HGF2DPosition(13.0, 13.0), HGF2DPosition(15.0, 15.0));
    ASSERT_EQ(0, HoledShape1.Intersect(Segment6, &DumPoints));

    //Segment partially in the hole
    HGF2DSegment Segment7(HGF2DPosition(13.0, 13.0), HGF2DPosition(21.0, 21.0));
    ASSERT_EQ(2, HoledShape1.Intersect(Segment7, &DumPoints));

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
TEST_F(HGF2DHoledShapeTester,  ContiguousnessTest)
    {

    HGF2DPositionCollection     DumPoints;
    HGF2DPosition   FirstDumPoint;
    HGF2DPosition   SecondDumPoint;

    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    // Test with contiguous linears
    ASSERT_TRUE(HoledShape1.AreContiguous(ComplexLinearCase6A));
    ASSERT_TRUE(HoledShape1.AreContiguousAt(ComplexLinearCase6A, HGF2DPosition(15.0, 20.0)));
    ASSERT_EQ(2, HoledShape1.ObtainContiguousnessPoints(ComplexLinearCase6A, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetY());

    HoledShape1.ObtainContiguousnessPointsAt(ComplexLinearCase6A, LinearMidPoint1A, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(10.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetY());

    DumPoints.clear();

    // Test with non contiguous linears
    ASSERT_FALSE(HoledShape1.AreContiguous(ComplexLinearCase1A));

    // Test contiguous linear with the hole
    HGF2DPolySegment ComplexLinearHole;
    ComplexLinearHole.AppendPoint(HGF2DPosition(12.0, 12.0));
    ComplexLinearHole.AppendPoint(HGF2DPosition(12.0, 18.0));

    ASSERT_TRUE(HoledShape1.AreContiguous(ComplexLinearHole));
    ASSERT_TRUE(HoledShape1.AreContiguousAt(ComplexLinearHole, HGF2DPosition(12.0, 12.0)));
    ASSERT_EQ(2, HoledShape1.ObtainContiguousnessPoints(ComplexLinearHole, &DumPoints));
    ASSERT_DOUBLE_EQ(12.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(12.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(12.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(18.0, DumPoints[1].GetY());

    HoledShape1.ObtainContiguousnessPointsAt(ComplexLinearHole, HGF2DPosition(12.0, 16.0), &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(12.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(12.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(12.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(18.0, SecondDumPoint.GetY());

    DumPoints.clear();

    HGF2DPolySegment ComplexLinearHole2;
    ComplexLinearHole2.AppendPoint(HGF2DPosition(12.0, 12.0));
    ComplexLinearHole2.AppendPoint(HGF2DPosition(12.0, 18.0));
    ComplexLinearHole2.AppendPoint(HGF2DPosition(18.0, 18.0));

    ASSERT_TRUE(HoledShape1.AreContiguous(ComplexLinearHole2));
    ASSERT_TRUE(HoledShape1.AreContiguousAt(ComplexLinearHole2, HGF2DPosition(12.0, 12.0)));
    ASSERT_EQ(2, HoledShape1.ObtainContiguousnessPoints(ComplexLinearHole2, &DumPoints));
    ASSERT_DOUBLE_EQ(12.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(12.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(18.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(18.0, DumPoints[1].GetY());

    HoledShape1.ObtainContiguousnessPointsAt(ComplexLinearHole2, HGF2DPosition(12.0, 16.0), &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(12.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(12.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(18.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(18.0, SecondDumPoint.GetY());

    DumPoints.clear();

    HGF2DPolySegment ComplexLinearHole3;
    ComplexLinearHole3.AppendPoint(HGF2DPosition(12.0, 12.0));
    ComplexLinearHole3.AppendPoint(HGF2DPosition(12.0, 18.0));
    ComplexLinearHole3.AppendPoint(HGF2DPosition(18.0, 18.0));
    ComplexLinearHole3.AppendPoint(HGF2DPosition(18.0, 12.0));
    ComplexLinearHole3.AppendPoint(HGF2DPosition(12.0, 12.0));

    ASSERT_TRUE(HoledShape1.AreContiguous(ComplexLinearHole3));
    ASSERT_TRUE(HoledShape1.AreContiguousAt(ComplexLinearHole3, HGF2DPosition(12.0, 12.0)));
    ASSERT_EQ(0, HoledShape1.ObtainContiguousnessPoints(ComplexLinearHole3, &DumPoints));

    DumPoints.clear();

    }

//==================================================================================
// Interaction info with other segment
// Crosses(const HGF2DVector& pi_rVector) const;
// AreAdjacent(const HGF2DVector& pi_rVector) const;
// IsPointOn(const HGF2DPosition& pi_rTestPoint) const;
// IsPointOn(const HGF2DPosition& pi_rTestPoint) const;
//==================================================================================
TEST_F(HGF2DHoledShapeTester,  InteractionTest)
    {

    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);
     
    // Tests with a vertical segment
    ASSERT_TRUE(HoledShape1.Crosses(ComplexLinearCase1A));
    ASSERT_FALSE(HoledShape1.AreAdjacent(ComplexLinearCase1A));

    ASSERT_TRUE(HoledShape1.Crosses(ComplexLinearCase2A));
    ASSERT_TRUE(HoledShape1.AreAdjacent(ComplexLinearCase2A));

    ASSERT_TRUE(HoledShape1.Crosses(ComplexLinearCase3A));
    ASSERT_FALSE(HoledShape1.AreAdjacent(ComplexLinearCase3A));

    ASSERT_FALSE(HoledShape1.Crosses(ComplexLinearCase4A));
    ASSERT_TRUE(HoledShape1.AreAdjacent(ComplexLinearCase4A));

    ASSERT_TRUE(HoledShape1.Crosses(ComplexLinearCase5B));
    ASSERT_TRUE(HoledShape1.AreAdjacent(ComplexLinearCase5B));

    ASSERT_FALSE(HoledShape1.Crosses(ComplexLinearCase6A));
    ASSERT_TRUE(HoledShape1.AreAdjacent(ComplexLinearCase6A));

    ASSERT_FALSE(HoledShape1.Crosses(ComplexLinearCase7A));
    ASSERT_FALSE(HoledShape1.AreAdjacent(ComplexLinearCase7A));

    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_FALSE(HoledShape1.IsPointOn(HGF2DPosition(10.0 - 1.1 * MYEPSILON, 10.0-1.1 * MYEPSILON)));
    ASSERT_FALSE(HoledShape1.IsPointOn(HGF2DPosition(15.0, 20.0 - 1.1 * MYEPSILON)));
    ASSERT_FALSE(HoledShape1.IsPointOn(HGF2DPosition(15.0, 20.0 + 1.1 * MYEPSILON)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(15.0, 20.0 + 0.9 * MYEPSILON)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(15.0, 20.0 - 0.9 * MYEPSILON)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(15.0, 20.0)));

    
    }

//==================================================================================
// Bearing calculation tests
// CalculateBearing(const HGF2DPosition& pi_rPositionPoint,
//                  HGF2DVector::ArbitraryDiPolyion pi_DiPolyion = HGF2DVector::BETA) const;
// CalculateAngularAcceleration(const HGF2DPosition& pi_rPositionPoint,
//                              HGF2DVector::ArbitraryDiPolyion pi_DiPolyion = HGF2DVector::BETA) const;
//==================================================================================
TEST_F(HGF2DHoledShapeTester,  BearingTest)
    {

    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    // Obtain bearing ALPHA of a vertical segment
    ASSERT_NEAR(0.0, HoledShape1.CalculateBearing(Poly1Point0d0, HGF2DVector::ALPHA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(PI/2, HoledShape1.CalculateBearing(Poly1Point0d0, HGF2DVector::BETA).GetAngle());
    ASSERT_NEAR(0.0, HoledShape1.CalculateBearing(Poly1Point0d1, HGF2DVector::ALPHA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(PI, HoledShape1.CalculateBearing(Poly1Point0d1, HGF2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(PI, HoledShape1.CalculateBearing(Poly1Point0d5, HGF2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(3*PI/2, HoledShape1.CalculateBearing(Poly1Point0d5, HGF2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(3*PI/2, HoledShape1.CalculateBearing(Poly1Point1d0, HGF2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(PI/2, HoledShape1.CalculateBearing(Poly1Point1d0, HGF2DVector::BETA).GetAngle());

    // ANGULAR ACCELERATION
    // Obtain angular acceleration ALPHA of a vertical segment
    ASSERT_NEAR(0.0, HoledShape1.CalculateAngularAcceleration(Poly1Point0d0, HGF2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, HoledShape1.CalculateAngularAcceleration(Poly1Point0d0, HGF2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, HoledShape1.CalculateAngularAcceleration(Poly1Point0d1, HGF2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, HoledShape1.CalculateAngularAcceleration(Poly1Point0d1, HGF2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, HoledShape1.CalculateAngularAcceleration(Poly1Point0d5, HGF2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, HoledShape1.CalculateAngularAcceleration(Poly1Point0d5, HGF2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, HoledShape1.CalculateAngularAcceleration(Poly1Point1d0, HGF2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, HoledShape1.CalculateAngularAcceleration(Poly1Point1d0, HGF2DVector::BETA), MYEPSILON);
   
    }

//==================================================================================
// Extent calculation test
// GetExtent() const;
//==================================================================================
TEST_F (HGF2DHoledShapeTester, GetExtentTest)
    {

    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);
    HGF2DHoledShape NoHoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    ASSERT_DOUBLE_EQ(10.0, HoledShape1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(10.0, HoledShape1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(20.0, HoledShape1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(20.0, HoledShape1.GetExtent().GetYMax());

    ASSERT_DOUBLE_EQ(10.0, NoHoledShape1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(10.0, NoHoledShape1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(20.0, NoHoledShape1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(20.0, NoHoledShape1.GetExtent().GetYMax());

    }

////==================================================================================
// Scale(double pi_ScaleFactor, const HGF2DPosition& pi_rScaleOrigin);
////==================================================================================
TEST_F (HGF2DHoledShapeTester,  ScaleTest) 
    { 
    
    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    HoledShape1.Scale(1.0, HGF2DPosition(0.0, 0.0));

    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(12.0, 12.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(12.0, 18.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(18.0, 12.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(18.0, 18.0)));

    HoledShape1.Scale(2.0, HGF2DPosition(0.0, 0.0));

    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(40.0, 20.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(20.0, 40.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(40.0, 40.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(24.0, 24.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(24.0, 36.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(36.0, 24.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(36.0, 36.0)));

    HoledShape1.Scale(2.0, HGF2DPosition(10.0, 20.0));

    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(30.0, 20.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(70.0, 20.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(30.0, 60.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(70.0, 60.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(38.0, 28.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(38.0, 52.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(62.0, 28.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(62.0, 52.0)));
    
    }

////==================================================================================
// Move(const HGF2DDisplacement& pi_rDisplacement);
////==================================================================================
TEST_F (HGF2DHoledShapeTester,  MoveTest) 
    { 

    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    HoledShape1.Move(HGF2DDisplacement(10.0, 10.0));

    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(30.0, 20.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(20.0, 30.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(30.0, 30.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(22.0, 22.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(22.0, 28.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(28.0, 22.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(28.0, 28.0)));

    HoledShape1.Move(HGF2DDisplacement(-10.0, -10.0));

    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(12.0, 12.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(12.0, 18.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(18.0, 12.0)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DPosition(18.0, 18.0)));

    }

////==================================================================================
// DifferentiateShape(const HGF2DShape& pi_rShape) const;
// DifferentiateFromShape(const HGF2DShape& pi_rShape) const;
////==================================================================================
TEST_F (HGF2DHoledShapeTester,  DifferentiateTest) 
    { 
    
    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);
    HGF2DHoledShape HoledShape2(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);
    HoledShape2.AddHole(HGF2DRectangle(13.0, 13.0, 14.0, 14.0));

    //Rectangle is in the hole
    HFCPtr<HGF2DShape>     pResultShape1 = HoledShape1.DifferentiateShape(HGF2DRectangle(13.0, 13.0, 16.0, 16.0));

    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(12.0, 12.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(12.0, 18.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(18.0, 12.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(18.0, 18.0)));

    //Rectangle is equal to the hole
    HFCPtr<HGF2DShape>     pResultShape2 = HoledShape1.DifferentiateShape(HGF2DRectangle(12.0, 12.0, 18.0, 18.0));

    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(12.0, 12.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(12.0, 18.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(18.0, 12.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(18.0, 18.0)));

    //Rectangle is bigger than hole but less than base
    HFCPtr<HGF2DShape>     pResultShape3 = HoledShape1.DifferentiateShape(HGF2DRectangle(11.0, 11.0, 19.0, 19.0));

    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(11.0, 11.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(11.0, 19.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(19.0, 11.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(19.0, 19.0)));

    //Rectangle is the same as base
    HFCPtr<HGF2DShape>     pResultShape4 = HoledShape1.DifferentiateShape(HGF2DRectangle(10.0, 10.0, 20.0, 20.0));

    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape4->GetShapeType());

    HoledShape2.Move(HGF2DDisplacement(2.0, 0.0));
    HFCPtr<HGF2DShape>     pResultShape5 = HoledShape1.DifferentiateShape(HoledShape2);

    ASSERT_EQ(HGF2DPolygonOfSegments::CLASS_ID, pResultShape5->GetShapeType());

    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(12.0, 10.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(12.0, 20.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(12.0, 12.0)));

    HoledShape2.Move(HGF2DDisplacement(3.5, 0.0));
    HFCPtr<HGF2DShape>     pResultShape6 = HoledShape1.DifferentiateShape(HoledShape2);

    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(15.5, 10.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(15.5, 12.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(12.0, 12.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(12.0, 18.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(15.5, 18.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(15.5, 20.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(10.0, 20.0)));

    HFCPtr<HGF2DShape>     pResultShape7 = HoledShape1.DifferentiateShape(HGF2DRectangle(18.5, 18.5, 19.5, 19.5));

    ASSERT_EQ(2, pResultShape7->GetHoleList().size());

    HoledShape2.Move(HGF2DDisplacement(2.0, 0.0));
    HFCPtr<HGF2DShape>     pResultShape8 = HoledShape2.DifferentiateFromShape(HoledShape1);

    ASSERT_EQ(HGF2DPolygonOfSegments::CLASS_ID, pResultShape8->GetShapeType());

    ASSERT_TRUE(pResultShape8->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape8->IsPointOn(HGF2DPosition(12.0, 10.0)));
    ASSERT_TRUE(pResultShape8->IsPointOn(HGF2DPosition(12.0, 20.0)));
    ASSERT_TRUE(pResultShape8->IsPointOn(HGF2DPosition(12.0, 10.0)));

    HoledShape2.Move(HGF2DDisplacement(3.5, 0.0));
    HFCPtr<HGF2DShape>     pResultShape9 = HoledShape2.DifferentiateFromShape(HoledShape1);

    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DPosition(15.5, 10.0)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DPosition(15.5, 12.0)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DPosition(12.0, 12.0)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DPosition(12.0, 18.0)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DPosition(15.5, 18.0)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DPosition(15.5, 20.0)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DPosition(10.0, 20.0)));

    }

////==================================================================================
// IntersectShape(const HGF2DShape& pi_rShape) const;
////==================================================================================
TEST_F (HGF2DHoledShapeTester,  IntersectShapeTest) 
    { 
    
    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);
    HGF2DHoledShape HoledShape2(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);
    HoledShape2.AddHole(HGF2DRectangle(13.0, 13.0, 14.0, 14.0));

    //Rectangle is in the hole
    HFCPtr<HGF2DShape>     pResultShape1 = HoledShape1.IntersectShape(HGF2DRectangle(13.0, 13.0, 16.0, 16.0));

    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape1->GetShapeType());

    //Rectangle is equal to the hole
    HFCPtr<HGF2DShape>     pResultShape2 = HoledShape1.IntersectShape(HGF2DRectangle(12.0, 12.0, 18.0, 18.0));

    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, pResultShape2->GetShapeType());

    //Rectangle is bigger than hole but less than base
    HFCPtr<HGF2DShape>     pResultShape3 = HoledShape1.IntersectShape(HGF2DRectangle(11.0, 11.0, 19.0, 19.0));

    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(12.0, 12.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(18.0, 12.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(12.0, 18.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(18.0, 18.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(11.0, 11.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(11.0, 19.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(19.0, 11.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(19.0, 19.0)));

    //Rectangle is the same as base
    HFCPtr<HGF2DShape>     pResultShape4 = HoledShape1.IntersectShape(HGF2DRectangle(10.0, 10.0, 20.0, 20.0));

    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(12.0, 12.0)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(12.0, 18.0)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(18.0, 12.0)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(18.0, 18.0)));

    HoledShape2.Move(HGF2DDisplacement(2.0, 0.0));
    HFCPtr<HGF2DShape>     pResultShape5 = HoledShape1.IntersectShape(HoledShape2);

    ASSERT_EQ(HGF2DPolygonOfSegments::CLASS_ID, pResultShape5->GetShapeType());

    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(12.0, 10.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(12.0, 12.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(18.0, 12.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(18.0, 18.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(12.0, 18.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(12.0, 20.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DPosition(20.0, 10.0)));

    HoledShape2.Move(HGF2DDisplacement(3.5, 0.0));
    HFCPtr<HGF2DShape>     pResultShape6 = HoledShape1.IntersectShape(HoledShape2);

    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(15.5, 10.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(15.5, 12.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(18.0, 12.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(18.0, 18.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(15.5, 18.0)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DPosition(15.5, 20.0)));

    }

////==================================================================================
// UnifyShape(const HGF2DShape& pi_rShape) const;
////==================================================================================
TEST_F (HGF2DHoledShapeTester,  UnifyTest) 
    { 
    
    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);
    HGF2DHoledShape HoledShape2(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);
    HoledShape2.AddHole(HGF2DRectangle(13.0, 13.0, 14.0, 14.0));

    //Rectangle is in the hole
    HFCPtr<HGF2DShape>     pResultShape1 = HoledShape1.UnifyShape(HGF2DRectangle(13.0, 13.0, 16.0, 16.0));

    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(12.0, 12.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(12.0, 18.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(18.0, 12.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(18.0, 18.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(13.0, 13.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(13.0, 16.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(16.0, 13.0)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DPosition(16.0, 16.0)));

    //Rectangle is equal to the hole
    HFCPtr<HGF2DShape>     pResultShape2 = HoledShape1.UnifyShape(HGF2DRectangle(12.0, 12.0, 18.0, 18.0));

    ASSERT_FALSE(pResultShape2->HasHoles());

    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DPosition(20.0, 20.0)));

    //Rectangle is bigger than hole but less than base
    HFCPtr<HGF2DShape>     pResultShape3 = HoledShape1.UnifyShape(HGF2DRectangle(11.0, 11.0, 19.0, 19.0));

    ASSERT_FALSE(pResultShape3->HasHoles());

    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DPosition(20.0, 20.0)));

    //Rectangle is bigger than base
    HFCPtr<HGF2DShape>     pResultShape4 = HoledShape1.UnifyShape(HGF2DRectangle(0.0, 0.0, 40.0, 40.0));

    ASSERT_FALSE(pResultShape4->HasHoles());

    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(0.0, 0.0)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(40.0, 0.0)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(0.0, 40.0)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DPosition(40.0, 40.0)));

    }

////==================================================================================
// Cloning tests
// Clone() const;
// AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
////==================================================================================
TEST_F (HGF2DHoledShapeTester,  CloningTest) 
    { 
    
    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    //General Clone Test
    HFCPtr<HGF2DHoledShape> pClone = (HGF2DHoledShape*)HoledShape1.Clone();
    ASSERT_FALSE(pClone->IsEmpty());
    
    ASSERT_TRUE(pClone->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pClone->IsPointOn(HGF2DPosition(20.0, 10.0)));
    ASSERT_TRUE(pClone->IsPointOn(HGF2DPosition(10.0, 20.0)));
    ASSERT_TRUE(pClone->IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(pClone->IsPointOn(HGF2DPosition(12.0, 12.0)));
    ASSERT_TRUE(pClone->IsPointOn(HGF2DPosition(12.0, 18.0)));
    ASSERT_TRUE(pClone->IsPointOn(HGF2DPosition(18.0, 12.0)));
    ASSERT_TRUE(pClone->IsPointOn(HGF2DPosition(18.0, 18.0)));
         


    // Test with a translation between systems
    Translation = HGF2DDisplacement (10.0, 10.0);
    HGF2DTranslation myTranslation(Translation);
  
    HFCPtr<HGF2DHoledShape> pClone5 = (HGF2DHoledShape*) (&*(HoledShape1.AllocTransformDirect(myTranslation)));
    ASSERT_FALSE(pClone5->IsEmpty());

    ASSERT_TRUE(pClone5->IsPointOn(HGF2DPosition(0.0, 0.0)));
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DPosition(10.0, 0.0)));
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DPosition(0.0, 10.0)));
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DPosition(10.0, 10.0)));
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DPosition(2.0, 2.0)));
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DPosition(2.0, 8.0)));
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DPosition(8.0, 2.0)));
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DPosition(8.0, 8.0)));

    // Test with a stretch between systems
    HGF2DStretch myStretch;
    myStretch.SetTranslation(Translation);
    myStretch.SetXScaling(0.5);
    myStretch.SetYScaling(0.5);
   
    HFCPtr<HGF2DHoledShape> pClone6 = (HGF2DHoledShape*)(&*(HoledShape1.AllocTransformDirect(myStretch)));
    ASSERT_FALSE(pClone6->IsEmpty());
     
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DPosition(0.0, 0.0)));
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DPosition(20.0, 0.0)));
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DPosition(0.0, 20.0)));
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DPosition(20.0, 20.0)));
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DPosition(4.0, 4.0)));
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DPosition(4.0, 16.0)));
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DPosition(16.0, 4.0)));
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DPosition(16.0, 16.0)));

    // Test with a similitude between systems
    HGF2DSimilitude mySimilitude;
    mySimilitude.SetRotation(PI);
    mySimilitude.SetScaling(0.5);
  
    HFCPtr<HGF2DHoledShape> pClone7 = (HGF2DHoledShape*)(&*(HoledShape1.AllocTransformDirect(mySimilitude)));
    ASSERT_FALSE(pClone7->IsEmpty());
    
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DPosition(-20.0, -20.0)));
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DPosition(-40.0, -20.0)));
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DPosition(-20.0, -40.0)));
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DPosition(-40.0, -40.0)));
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DPosition(-24.0, -24.0)));
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DPosition(-24.0, -36.0)));
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DPosition(-36.0, -24.0)));
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DPosition(-36.0, -36.0)));

    // Test with a affine between systems
    HGF2DAffine myAffine;
    myAffine.SetTranslation(Translation);
    myAffine.SetRotation(PI);
    myAffine.SetXScaling(0.5);
    myAffine.SetYScaling(0.5);
   
    HFCPtr<HGF2DHoledShape> pClone8 = (HGF2DHoledShape*)(&*(HoledShape1.AllocTransformDirect(myAffine)));
    ASSERT_FALSE(pClone8->IsEmpty());

    ASSERT_TRUE(pClone8->IsPointOn(HGF2DPosition(0.0, 0.0)));
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DPosition(-20.0, 0.0)));
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DPosition(0.0, -20.0)));
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DPosition(-20.0, -20.0)));
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DPosition(-4.0, -4.0)));
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DPosition(-4.0, -16.0)));
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DPosition(-16.0, -4.0)));
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DPosition(-16.0, -16.0)));

    }

//==================================================================================
// ********************************************************************************
//==================================================================================

//==================================================================================
// Diff test with overlapping hole
//==================================================================================
TEST_F (HGF2DHoledShapeTester, ModifyShapeTest)
    {
   
    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    HGF2DPolygonOfSegments    Poly1C(MyLinear6);

    HFCPtr<HGF2DShape> pResultShape = HoledShape1.DifferentiateShape(Poly1C);
    ASSERT_FALSE(pResultShape->HasHoles());
    ASSERT_DOUBLE_EQ(52.0, pResultShape->CalculateArea());

    HFCPtr<HGF2DShape> pResultShape2 = HoledShape1.IntersectShape(Poly1C);
    ASSERT_FALSE(pResultShape2->HasHoles());
    ASSERT_DOUBLE_EQ(12.0, pResultShape2->CalculateArea());

    HFCPtr<HGF2DShape> pResultShape3 = HoledShape1.UnifyShape(Poly1C);
    ASSERT_TRUE(pResultShape3->HasHoles());
    ASSERT_DOUBLE_EQ(64.0, pResultShape3->CalculateArea());

    HFCPtr<HGF2DShape> pResultShape4 = Poly1C.DifferentiateFromShape(HoledShape1);
    ASSERT_FALSE(pResultShape4->HasHoles());
    ASSERT_DOUBLE_EQ(52.0, pResultShape4->CalculateArea());

    }

//==================================================================================
// Diff test with overlapping hole
//==================================================================================
TEST_F (HGF2DHoledShapeTester, ModifyShapeTest2)
    {

    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear4);
    HoledShape1.AddHole(Poly1B);

    HGF2DPolygonOfSegments    Poly1C(MyLinear5);
    HGF2DHoledShape     HoledShape2(Poly1C);

    HGF2DPolygonOfSegments    Poly1D(MyLinear6);
    HoledShape2.AddHole(Poly1D);

    HFCPtr<HGF2DShape> pResultShape = HoledShape1.DifferentiateShape(HoledShape2);
    ASSERT_FALSE(pResultShape->HasHoles());
    ASSERT_DOUBLE_EQ(32.0, pResultShape->CalculateArea());

    HFCPtr<HGF2DShape> pResultShape2 = HoledShape1.IntersectShape(HoledShape2);
    ASSERT_TRUE(pResultShape2->IsComplex());
    ASSERT_DOUBLE_EQ(32.0, pResultShape2->CalculateArea());

    HFCPtr<HGF2DShape> pResultShape3 = HoledShape1.UnifyShape(HoledShape2);
    ASSERT_FALSE(pResultShape3->HasHoles());
    ASSERT_DOUBLE_EQ(110.0, pResultShape3->CalculateArea());

    HFCPtr<HGF2DShape> pResultShape4 = HoledShape2.DifferentiateFromShape(HoledShape1);
    ASSERT_FALSE(pResultShape4->HasHoles());
    ASSERT_DOUBLE_EQ(32.0, pResultShape4->CalculateArea());

    }

//==================================================================================
// Diff test with overlapping hole
//==================================================================================
TEST_F (HGF2DHoledShapeTester, ModifyShapeTest3)
    {

    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear4);
    HoledShape1.AddHole(Poly1B);

    HGF2DHoledShape     HoledShape2(HoledShape1);

    HFCPtr<HGF2DShape> pResultShape = HoledShape1.DifferentiateShape(HoledShape2);
    ASSERT_TRUE(pResultShape->IsEmpty());

    HFCPtr<HGF2DShape> pResultShape2 = HoledShape1.IntersectShape(HoledShape2);
    ASSERT_TRUE(pResultShape2->HasHoles());
    ASSERT_DOUBLE_EQ(64.0, pResultShape2->CalculateArea());

    HFCPtr<HGF2DShape> pResultShape3 = HoledShape1.UnifyShape(HoledShape2);
    ASSERT_TRUE(pResultShape3->HasHoles());
    ASSERT_DOUBLE_EQ(64.0, pResultShape3->CalculateArea());

    HFCPtr<HGF2DShape> pResultShape4 = HoledShape2.DifferentiateFromShape(HoledShape1);
    ASSERT_TRUE(pResultShape4->IsEmpty());

    }

//==================================================================================
// Diff test with overlapping hole
//==================================================================================
TEST_F (HGF2DHoledShapeTester, ModifyShapeTest4)
    {

    HGF2DPolygonOfSegments    Poly1A(MyLinear1);
    HGF2DHoledShape HoledShape1(Poly1A);

    HGF2DPolygonOfSegments    Poly1B(MyLinear4);
    HoledShape1.AddHole(Poly1B);

    HGF2DPolygonOfSegments    Poly1C(MyLinear5);
    HGF2DHoledShape     HoledShape2(Poly1C);

    HGF2DSegment    Segment1G(HGF2DPosition(17.0, 12.0), HGF2DPosition(17.0, 18.0));
    HGF2DSegment    Segment2G(HGF2DPosition(17.0, 18.0), HGF2DPosition(20.0, 18.0));
    HGF2DSegment    Segment3G(HGF2DPosition(20.0, 18.0), HGF2DPosition(20.0, 12.0));
    HGF2DSegment    Segment4G(HGF2DPosition(20.0, 12.0), HGF2DPosition(17.0, 12.0));

    HGF2DPolySegment  MyLinear7;
    MyLinear7.AppendPoint(HGF2DPosition(17.0, 12.0));
    MyLinear7.AppendPoint(HGF2DPosition(17.0, 18.0));
    MyLinear7.AppendPoint(HGF2DPosition(20.0, 18.0));
    MyLinear7.AppendPoint(HGF2DPosition(20.0, 12.0));
    MyLinear7.AppendPoint(HGF2DPosition(17.0, 12.0));
    

    HGF2DPolygonOfSegments    Poly1D(MyLinear7);

    HoledShape2.AddHole(Poly1D);

    HFCPtr<HGF2DShape> pResultShape = HoledShape1.DifferentiateShape(HoledShape2);
    ASSERT_FALSE(pResultShape->HasHoles());
    ASSERT_DOUBLE_EQ(32.0, pResultShape->CalculateArea());

    HFCPtr<HGF2DShape> pResultShape2 = HoledShape1.IntersectShape(HoledShape2);
    ASSERT_TRUE(pResultShape2->IsComplex());
    ASSERT_DOUBLE_EQ(32.0, pResultShape2->CalculateArea());

    HFCPtr<HGF2DShape> pResultShape3 = HoledShape1.UnifyShape(HoledShape2);
    ASSERT_TRUE(pResultShape3->HasHoles());
    ASSERT_DOUBLE_EQ(104.0, pResultShape3->CalculateArea());

    HFCPtr<HGF2DShape> pResultShape4 = HoledShape2.DifferentiateFromShape(HoledShape1);

    ASSERT_FALSE(pResultShape4->HasHoles());
    ASSERT_DOUBLE_EQ(32.0, pResultShape4->CalculateArea());

    }


//==================================================================================
// Case which failed 16 feb 1998
//==================================================================================
TEST_F (HGF2DHoledShapeTester, UnifyShapeTest)
    {

    HFCPtr<HGF2DCoordSys>   pTheCurrentWorld = new HGF2DCoordSys();
        
    HGF2DPolySegment TheLinear1;
    TheLinear1.AppendPoint(HGF2DPosition(332946.275207202940000 , 5053264.982890859200000));
    TheLinear1.AppendPoint(HGF2DPosition(332946.275207202940000 , 5067904.982890859200000));
    TheLinear1.AppendPoint(HGF2DPosition(343846.275207202940000 , 5067904.982890859200000));
    TheLinear1.AppendPoint(HGF2DPosition(343846.275207202940000 , 5053264.982890859200000));
    TheLinear1.AppendPoint(HGF2DPosition(332946.275207202940000 , 5053264.982890859200000));
    

    HGF2DPolygonOfSegments BaseShape(TheLinear1);

    HGF2DHoledShape  MyHoled(BaseShape);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(341177.246740867090000 , 5061604.832691806400000 ));
    TheLinear2.AppendPoint(HGF2DPosition(342460.051131906220000 , 5061085.528745555300000 ));
    TheLinear2.AppendPoint(HGF2DPosition(342118.889531906230000 , 5060183.418745555000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(342055.379185642580000 , 5060226.807001913000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(341780.836003819250000 , 5059534.034487032300000 ));
    TheLinear2.AppendPoint(HGF2DPosition(341721.252014425350000 , 5059566.685309503200000 ));
    TheLinear2.AppendPoint(HGF2DPosition(341422.392846682460000 , 5058872.039676371000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(341353.551715074170000 , 5058923.200298768500000 ));
    TheLinear2.AppendPoint(HGF2DPosition(341102.787610408970000 , 5058218.806746337600000 ));
    TheLinear2.AppendPoint(HGF2DPosition(341018.473817025950000 , 5058245.244122228600000 ));
    TheLinear2.AppendPoint(HGF2DPosition(340710.631351472980000 , 5057445.429118268200000 ));
    TheLinear2.AppendPoint(HGF2DPosition(339340.487223472970000 , 5057965.696110268100000 ));
    TheLinear2.AppendPoint(HGF2DPosition(339544.469184219080000 , 5058495.667933515300000 ));
    TheLinear2.AppendPoint(HGF2DPosition(338548.192346269730000 , 5059111.277800547000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(338869.578704333860000 , 5059640.620037358300000 ));
    TheLinear2.AppendPoint(HGF2DPosition(338732.219809614940000 , 5059739.195244156800000 ));
    TheLinear2.AppendPoint(HGF2DPosition(338892.919336697200000 , 5060015.955540797700000 ));
    TheLinear2.AppendPoint(HGF2DPosition(338858.029650495740000 , 5060035.606053715600000 ));
    TheLinear2.AppendPoint(HGF2DPosition(339109.368566016610000 , 5060463.416973751000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(339094.638933242650000 , 5060472.955741963300000 ));
    TheLinear2.AppendPoint(HGF2DPosition(339336.151112755120000 , 5060851.971206161200000 ));
    TheLinear2.AppendPoint(HGF2DPosition(339264.579283097530000 , 5060905.711991752500000 ));
    TheLinear2.AppendPoint(HGF2DPosition(339697.443283097530000 , 5061597.091991752400000 ));
    TheLinear2.AppendPoint(HGF2DPosition(339844.199008590430000 , 5061504.173285123000000 ));
    TheLinear2.AppendPoint(HGF2DPosition(339848.373542428250000 , 5061516.001130996300000 ));
    TheLinear2.AppendPoint(HGF2DPosition(339670.860111388200000 , 5061651.556841972300000 ));
    TheLinear2.AppendPoint(HGF2DPosition(339718.129087228740000 , 5061721.838345524900000 ));
    TheLinear2.AppendPoint(HGF2DPosition(339709.818191450730000 , 5061727.697993952800000 ));
    TheLinear2.AppendPoint(HGF2DPosition(340128.870191450750000 , 5062351.817993952900000 ));
    TheLinear2.AppendPoint(HGF2DPosition(341240.398191450750000 , 5061700.949993953100000 ));
    TheLinear2.AppendPoint(HGF2DPosition(341177.246740867090000 , 5061604.832691806400000 ));

    HGF2DPolygonOfSegments TheHole(TheLinear2);

    MyHoled.AddHole(TheHole);

    HGF2DPolySegment  TheLinear3;

    TheLinear3.AppendPoint(HGF2DPosition(340591.273152340440000 , 5061106.222726583500000 ));
    TheLinear3.AppendPoint(HGF2DPosition(340555.199749660210000 , 5061060.921244148200000 ));
    TheLinear3.AppendPoint(HGF2DPosition(340553.930404597490000 , 5061054.805308845800000 ));
    TheLinear3.AppendPoint(HGF2DPosition(340747.516262911380000 , 5060932.236001815600000 ));
    TheLinear3.AppendPoint(HGF2DPosition(340756.159139819270000 , 5060954.832439032400000 ));
    TheLinear3.AppendPoint(HGF2DPosition(340974.188876229980000 , 5060869.697018148400000 ));
    TheLinear3.AppendPoint(HGF2DPosition(340836.253131906210000 , 5060967.434345554600000 ));
    TheLinear3.AppendPoint(HGF2DPosition(340951.431374093460000 , 5061261.138863132300000 ));
    TheLinear3.AppendPoint(HGF2DPosition(340767.850191450740000 , 5060981.725993952700000 ));
    TheLinear3.AppendPoint(HGF2DPosition(340591.273152340440000 , 5061106.222726583500000 ));

    HGF2DPolygonOfSegments TheSimple(TheLinear3);

    HFCPtr<HGF2DShape> NewShape = MyHoled.UnifyShape(TheSimple);

    }

//==================================================================================
// Case which failed 16 feb 2000
//==================================================================================
TEST_F (HGF2DHoledShapeTester, DifferentiateShapeTest)
    {
        
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(0.0, 0.0 ));
    TheLinear1.AppendPoint(HGF2DPosition(0.0, 100.0 ));
    TheLinear1.AppendPoint(HGF2DPosition(100.0, 100.0 ));
    TheLinear1.AppendPoint(HGF2DPosition(100.0, 0.0 ));
    TheLinear1.AppendPoint(HGF2DPosition(0.0, 0.0 ));

    HGF2DPolygonOfSegments BaseShape(TheLinear1);

    HGF2DHoledShape  MyHoled(BaseShape);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(30.0, 30.0 ));
    TheLinear2.AppendPoint(HGF2DPosition(30.0, 70.0 ));
    TheLinear2.AppendPoint(HGF2DPosition(70.0, 70.0 ));
    TheLinear2.AppendPoint(HGF2DPosition(70.0, 30.0 ));
    TheLinear2.AppendPoint(HGF2DPosition(30.0, 30.0 ));

    HGF2DPolygonOfSegments TheHole(TheLinear2);

    MyHoled.AddHole(TheHole);

    HGF2DPolySegment  TheLinear3;

    TheLinear3.AppendPoint(HGF2DPosition(10.0, 10.0 ));
    TheLinear3.AppendPoint(HGF2DPosition(10.0, 70.0 ));
    TheLinear3.AppendPoint(HGF2DPosition(30.0, 70.0 ));
    TheLinear3.AppendPoint(HGF2DPosition(30.0, 10.0 ));
    TheLinear3.AppendPoint(HGF2DPosition(10.0, 10.0 ));

    HGF2DPolygonOfSegments TheSimple(TheLinear3);

    HFCPtr<HGF2DShape> NewShape = MyHoled.DifferentiateShape(TheSimple);

    ASSERT_DOUBLE_EQ(7200.0, NewShape->CalculateArea());


    }

//==================================================================================
// Intersection when hole overlaps rectangle to intersect with
// Case which failed 16 feb 2000
//==================================================================================
TEST_F (HGF2DHoledShapeTester, ModifyShapeTest5)
    {
        
      
    HGF2DPolySegment  TheLinear1;

    TheLinear1.AppendPoint(HGF2DPosition(-100.0, -100.0 ));
    TheLinear1.AppendPoint(HGF2DPosition(-100.0, 200.0 ));
    TheLinear1.AppendPoint(HGF2DPosition(200.0, 200.0 ));
    TheLinear1.AppendPoint(HGF2DPosition(200.0, -100.0 ));
    TheLinear1.AppendPoint(HGF2DPosition(-100.0, -100.0 ));

    HGF2DPolygonOfSegments BaseShape(TheLinear1);

    HGF2DHoledShape  MyHoled(BaseShape);

    HGF2DPolySegment  TheLinear2;

    TheLinear2.AppendPoint(HGF2DPosition(0.0, 0.0 ));
    TheLinear2.AppendPoint(HGF2DPosition(0.0, 100.0));
    TheLinear2.AppendPoint(HGF2DPosition(100.0, 100.0));
    TheLinear2.AppendPoint(HGF2DPosition(100.0, 0.0));
    TheLinear2.AppendPoint(HGF2DPosition(0.0, 0.0 ));
    
    HGF2DPolygonOfSegments TheHole(TheLinear2);

    MyHoled.AddHole(TheHole);

    HGF2DRectangle TheRectangle(0.0, 0.0, 100.0, 100.0);

    HFCPtr<HGF2DShape> NewShape = MyHoled.DifferentiateShape(TheRectangle);
    ASSERT_DOUBLE_EQ(80000.0, NewShape->CalculateArea());

    NewShape = TheRectangle.DifferentiateShape(MyHoled);
    ASSERT_DOUBLE_EQ(10000.0, NewShape->CalculateArea());

    NewShape = MyHoled.IntersectShape(TheRectangle);
    ASSERT_NEAR(0.0, NewShape->CalculateArea(), MYEPSILON);

    NewShape = TheRectangle.IntersectShape(MyHoled);
    ASSERT_NEAR(0.0, NewShape->CalculateArea(), MYEPSILON);

    NewShape = MyHoled.UnifyShape(TheRectangle);
    ASSERT_DOUBLE_EQ(90000.0, NewShape->CalculateArea());

    NewShape = TheRectangle.UnifyShape(MyHoled);
    ASSERT_DOUBLE_EQ(90000.0, NewShape->CalculateArea());

    }