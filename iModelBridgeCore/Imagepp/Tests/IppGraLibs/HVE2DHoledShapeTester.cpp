//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HVE2DHoledShapeTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "EnvironnementTest.h"
#include "HVE2DHoledShapeTester.h"

HVE2DHoledShapeTester::HVE2DHoledShapeTester() 
    {

    //Linears
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

    Segment1D = HVE2DSegment(HGF2DLocation(12.0, 12.0, pWorld), HGF2DLocation(12.0, 18.0, pWorld));
    Segment2D = HVE2DSegment(HGF2DLocation(12.0, 18.0, pWorld), HGF2DLocation(18.0, 18.0, pWorld));
    Segment3D = HVE2DSegment(HGF2DLocation(18.0, 18.0, pWorld), HGF2DLocation(18.0, 12.0, pWorld));
    Segment4D = HVE2DSegment(HGF2DLocation(18.0, 12.0, pWorld), HGF2DLocation(12.0, 12.0, pWorld));

    MyLinear4 = HVE2DComplexLinear(pWorld);
    MyLinear4.AppendLinear(Segment1D);
    MyLinear4.AppendLinear(Segment2D);
    MyLinear4.AppendLinear(Segment3D);
    MyLinear4.AppendLinear(Segment4D);

    Segment1E = HVE2DSegment(HGF2DLocation(12.0, 10.0, pWorld), HGF2DLocation(12.0, 20.0, pWorld));
    Segment2E = HVE2DSegment(HGF2DLocation(12.0, 20.0, pWorld), HGF2DLocation(21.0, 20.0, pWorld));
    Segment3E = HVE2DSegment(HGF2DLocation(21.0, 20.0, pWorld), HGF2DLocation(21.0, 10.0, pWorld));
    Segment4E = HVE2DSegment(HGF2DLocation(21.0, 10.0, pWorld), HGF2DLocation(12.0, 10.0, pWorld));

    MyLinear5 = HVE2DComplexLinear(pWorld);
    MyLinear5.AppendLinear(Segment1E);
    MyLinear5.AppendLinear(Segment2E);
    MyLinear5.AppendLinear(Segment3E);
    MyLinear5.AppendLinear(Segment4E);

    Segment1F = HVE2DSegment(HGF2DLocation(18.0, 12.0, pWorld), HGF2DLocation(18.0, 18.0, pWorld));
    Segment2F = HVE2DSegment(HGF2DLocation(18.0, 18.0, pWorld), HGF2DLocation(20.0, 18.0, pWorld));
    Segment3F = HVE2DSegment(HGF2DLocation(20.0, 18.0, pWorld), HGF2DLocation(20.0, 12.0, pWorld));
    Segment4F = HVE2DSegment(HGF2DLocation(20.0, 12.0, pWorld), HGF2DLocation(18.0, 12.0, pWorld));

    MyLinear6 = HVE2DComplexLinear(pWorld);
    MyLinear6.AppendLinear(Segment1F);
    MyLinear6.AppendLinear(Segment2F);
    MyLinear6.AppendLinear(Segment3F);
    MyLinear6.AppendLinear(Segment4F);

    //Point
    Poly1Point0d0 = HGF2DLocation(10.0, 10.0, pWorld);
    Poly1Point0d1 = HGF2DLocation(15.0, 10.0, pWorld);
    Poly1Point0d5 = HGF2DLocation(20.0, 20.0, pWorld);
    Poly1Point1d0 = HGF2DLocation(10.0, 10.0 + (1.1 * MYEPSILON), pWorld);

    }

// THE PRESENT TEST CANNOT MAKE TESTS WITH NON-LINEAR TRANSFORMATION MODELS

////==================================================================================
// Constructor Test
////==================================================================================
TEST_F (HVE2DHoledShapeTester,  ConstructorTest) 
    { 

    //Default Constructor
    HVE2DHoledShape Shape1;

    //Constructor with CoordSys
    HVE2DHoledShape Shape2(pWorld);
    ASSERT_EQ(pWorld, Shape2.GetCoordSys());

    ///With a simple shape
    HVE2DHoledShape Shape3(Rect1);
    ASSERT_EQ(pWorld, Shape3.GetCoordSys());
    ASSERT_TRUE(Shape3.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(Shape3.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(Shape3.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(Shape3.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    //Copy Constructor
    HVE2DHoledShape Shape4(Shape3);
    ASSERT_EQ(pWorld, Shape4.GetCoordSys());
    ASSERT_TRUE(Shape4.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(Shape4.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(Shape4.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(Shape4.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    }

//==================================================================================
// operator=(const HVE2DPolygon& pi_rObj);
//==================================================================================
TEST_F (HVE2DHoledShapeTester, OperatorTest)
    {
    
    HVE2DHoledShape Shape1(Rect1);

    HVE2DHoledShape Shape2 = Shape1;

    ASSERT_EQ(pWorld, Shape2.GetCoordSys());
    ASSERT_TRUE(Shape2.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(Shape2.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(Shape2.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(Shape2.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    }

//==================================================================================
// AddHole(const HVE2DSimpleShape& pi_rSimpleShape);
// Drop(HGF2DLocationCollection* po_pPoint, const HGFDistance& pi_rTolerance) const;
//==================================================================================
TEST_F (HVE2DHoledShapeTester, AddHoleTest)
    {

    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);
    HVE2DHoledShape HoledShape2(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    ASSERT_TRUE(HoledShape1.HasHoles());
    ASSERT_DOUBLE_EQ(64.0, HoledShape1.CalculateArea());

    HGF2DLocationCollection Locations;
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
// SetBaseShape(const HVE2DSimpleShape& pi_rComplex);
// GetBaseShape() const;
////==================================================================================
TEST_F (HVE2DHoledShapeTester,  BaseShapeTest) 
    { 

    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    HoledShape1.SetBaseShape(HVE2DRectangle(14.0, 14.0, 16.0, 16.0, pWorld)); 

    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(14.0, 14.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(14.0, 16.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(16.0, 14.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(16.0, 16.0, pWorld)));

    HVE2DHoledShape BaseShape = HoledShape1.GetBaseShape();

    ASSERT_TRUE(BaseShape.IsPointOn(HGF2DLocation(14.0, 14.0, pWorld)));
    ASSERT_TRUE(BaseShape.IsPointOn(HGF2DLocation(14.0, 16.0, pWorld)));
    ASSERT_TRUE(BaseShape.IsPointOn(HGF2DLocation(16.0, 14.0, pWorld)));
    ASSERT_TRUE(BaseShape.IsPointOn(HGF2DLocation(16.0, 16.0, pWorld)));

    }

////==================================================================================
// IsSimple() const;
// IsComplex() const;
////==================================================================================
TEST_F (HVE2DHoledShapeTester,  SimpleTest) 
    { 
    
    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);
    HVE2DHoledShape NoHoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
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
TEST_F (HVE2DHoledShapeTester,  ListTest) 
    { 
    
    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);
    HVE2DHoledShape NoHoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    ASSERT_TRUE(HoledShape1.HasHoles());
    ASSERT_FALSE(NoHoledShape1.HasHoles());

    ASSERT_EQ(1, HoledShape1.GetHoleList().size());

    }

////==================================================================================
// CalculateArea() const;
// CalculatePerimeter() const;
////==================================================================================
TEST_F (HVE2DHoledShapeTester,  AreaPerimenterTest) 
    { 
    
    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    ASSERT_DOUBLE_EQ(HoledShape1.CalculateArea(), Poly1A.CalculateArea() - Poly1B.CalculateArea());
    ASSERT_DOUBLE_EQ(HoledShape1.CalculatePerimeter(), Poly1A.CalculatePerimeter() + Poly1B.CalculatePerimeter() );

    }

////==================================================================================
// MakeEmpty();
// IsEmpty();
////==================================================================================
TEST_F (HVE2DHoledShapeTester,  EmptyTest) 
    { 

    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    ASSERT_FALSE(HoledShape1.IsEmpty());
    
    HoledShape1.MakeEmpty();

    ASSERT_TRUE(HoledShape1.IsEmpty());

    }

////==================================================================================
//IsPointIn(const HGF2DLocation& pi_rPoint, double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
////==================================================================================
TEST_F (HVE2DHoledShapeTester,  IsPointInTest) 
    { 

    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    //On the Border
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DLocation(12.0, 12.0, pWorld)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DLocation(12.0, 18.0, pWorld)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DLocation(18.0, 12.0, pWorld)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DLocation(18.0, 18.0, pWorld)));

    //In the HoledShape but not in the hole
    ASSERT_TRUE(HoledShape1.IsPointIn(HGF2DLocation(11.0, 11.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointIn(HGF2DLocation(19.0, 11.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointIn(HGF2DLocation(11.0, 19.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointIn(HGF2DLocation(19.0, 19.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointIn(HGF2DLocation(13.0, 19.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointIn(HGF2DLocation(19.0, 13.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointIn(HGF2DLocation(19.0, 19.0, pWorld)));

    //In the hole of the HoledShape
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DLocation(13.0, 13.0, pWorld)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DLocation(14.0, 14.0, pWorld)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DLocation(15.0, 15.0, pWorld)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DLocation(16.0, 16.0, pWorld)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DLocation(17.0, 17.0, pWorld)));

    //Outside the HoledShape
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DLocation(-14.0, 14.0, pWorld)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DLocation(15.0, -15.0, pWorld)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DLocation(16.0, -16.0, pWorld)));
    ASSERT_FALSE(HoledShape1.IsPointIn(HGF2DLocation(-17.0, 17.0, pWorld)));

    }

////==================================================================================
// CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
////==================================================================================
TEST_F (HVE2DHoledShapeTester,  CalculateClosestPointTest) 
    { 
    
    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    ASSERT_DOUBLE_EQ(10.0, HoledShape1.CalculateClosestPoint(HGF2DLocation(0.0, 0.0, pWorld)).GetX());
    ASSERT_DOUBLE_EQ(10.0, HoledShape1.CalculateClosestPoint(HGF2DLocation(0.0, 0.0, pWorld)).GetY());
    ASSERT_DOUBLE_EQ(10.0, HoledShape1.CalculateClosestPoint(HGF2DLocation(10.0, 10.0, pWorld)).GetX());
    ASSERT_DOUBLE_EQ(10.0, HoledShape1.CalculateClosestPoint(HGF2DLocation(10.0, 10.0, pWorld)).GetY());
    ASSERT_DOUBLE_EQ(10.0, HoledShape1.CalculateClosestPoint(HGF2DLocation(11.0, 11.0, pWorld)).GetX());
    ASSERT_DOUBLE_EQ(11.0, HoledShape1.CalculateClosestPoint(HGF2DLocation(11.0, 11.0, pWorld)).GetY());
    ASSERT_DOUBLE_EQ(12.0, HoledShape1.CalculateClosestPoint(HGF2DLocation(11.5, 11.5, pWorld)).GetX());
    ASSERT_DOUBLE_EQ(12.0, HoledShape1.CalculateClosestPoint(HGF2DLocation(11.5, 11.5, pWorld)).GetY());
    ASSERT_DOUBLE_EQ(12.0, HoledShape1.CalculateClosestPoint(HGF2DLocation(13.0, 13.0, pWorld)).GetX());
    ASSERT_DOUBLE_EQ(13.0, HoledShape1.CalculateClosestPoint(HGF2DLocation(13.0, 13.0, pWorld)).GetY());

    }

////==================================================================================
// Intersect(const HVE2DVector& pi_rVector, HGF2DLocationCollection* po_pCrossPoints) const;
////==================================================================================
TEST_F (HVE2DHoledShapeTester,  IntersectTest) 
    { 
    
    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    HGF2DLocationCollection   DumPoints;

    //Segment doesn't touch the HoledShape
    HVE2DSegment Segment1(HGF2DLocation(-10.0, 0.0, pWorld), HGF2DLocation(0.0, 0.0, pWorld));
    ASSERT_EQ(0, HoledShape1.Intersect(Segment1, &DumPoints));

    //Segment flirt the HoledShape
    HVE2DSegment Segment2(HGF2DLocation(-10.0, 0.0, pWorld), HGF2DLocation(10.0 - MYEPSILON, 10.0 - MYEPSILON, pWorld));
    ASSERT_EQ(0, HoledShape1.Intersect(Segment2, &DumPoints));

    //Segment touch one point of the HoledShape
    HVE2DSegment Segment3(HGF2DLocation(-10.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    ASSERT_EQ(0, HoledShape1.Intersect(Segment3, &DumPoints));

    //Segment pass throught
    HVE2DSegment Segment4(HGF2DLocation(9.0, 9.0, pWorld), HGF2DLocation(13.0, 13.0, pWorld));
    ASSERT_EQ(2, HoledShape1.Intersect(Segment4, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(12.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(12.0, DumPoints[1].GetY());

    DumPoints.clear();

    //Segment pass completly throught
    HVE2DSegment Segment5(HGF2DLocation(9.0, 9.0, pWorld), HGF2DLocation(21.0, 21.0, pWorld));
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
    HVE2DSegment Segment6(HGF2DLocation(13.0, 13.0, pWorld), HGF2DLocation(15.0, 15.0, pWorld));
    ASSERT_EQ(0, HoledShape1.Intersect(Segment6, &DumPoints));

    //Segment partially in the hole
    HVE2DSegment Segment7(HGF2DLocation(13.0, 13.0, pWorld), HGF2DLocation(21.0, 21.0, pWorld));
    ASSERT_EQ(2, HoledShape1.Intersect(Segment7, &DumPoints));

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
TEST_F(HVE2DHoledShapeTester,  ContiguousnessTest)
    {

    HGF2DLocationCollection     DumPoints;
    HGF2DLocation   FirstDumPoint;
    HGF2DLocation   SecondDumPoint;

    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    // Test with contiguous linears
    ASSERT_TRUE(HoledShape1.AreContiguous(ComplexLinearCase6));
    ASSERT_TRUE(HoledShape1.AreContiguousAt(ComplexLinearCase6, HGF2DLocation(15.0, 20.0, pWorld)));
    ASSERT_EQ(2, HoledShape1.ObtainContiguousnessPoints(ComplexLinearCase6, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetY());

    HoledShape1.ObtainContiguousnessPointsAt(ComplexLinearCase6, LinearMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(10.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(20.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetY());

    DumPoints.clear();

    // Test with non contiguous linears
    ASSERT_FALSE(HoledShape1.AreContiguous(ComplexLinearCase1));

    // Test contiguous linear with the hole
    HVE2DComplexLinear ComplexLinearHole(pWorld);
    ComplexLinearHole.AppendLinear(HVE2DSegment(HGF2DLocation(12.0, 12.0, pWorld), HGF2DLocation(12.0, 18.0, pWorld)));

    ASSERT_TRUE(HoledShape1.AreContiguous(ComplexLinearHole));
    ASSERT_TRUE(HoledShape1.AreContiguousAt(ComplexLinearHole, HGF2DLocation(12.0, 12.0, pWorld)));
    ASSERT_EQ(2, HoledShape1.ObtainContiguousnessPoints(ComplexLinearHole, &DumPoints));
    ASSERT_DOUBLE_EQ(12.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(12.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(12.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(18.0, DumPoints[1].GetY());

    HoledShape1.ObtainContiguousnessPointsAt(ComplexLinearHole, HGF2DLocation(12.0, 16.0, pWorld), &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(12.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(12.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(12.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(18.0, SecondDumPoint.GetY());

    DumPoints.clear();

    HVE2DComplexLinear ComplexLinearHole2(pWorld);
    ComplexLinearHole2.AppendLinear(HVE2DSegment(HGF2DLocation(12.0, 12.0, pWorld), HGF2DLocation(12.0, 18.0, pWorld)));
    ComplexLinearHole2.AppendLinear(HVE2DSegment(HGF2DLocation(12.0, 18.0, pWorld), HGF2DLocation(18.0, 18.0, pWorld)));

    ASSERT_TRUE(HoledShape1.AreContiguous(ComplexLinearHole2));
    ASSERT_TRUE(HoledShape1.AreContiguousAt(ComplexLinearHole2, HGF2DLocation(12.0, 12.0, pWorld)));
    ASSERT_EQ(2, HoledShape1.ObtainContiguousnessPoints(ComplexLinearHole2, &DumPoints));
    ASSERT_DOUBLE_EQ(12.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(12.0, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(18.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(18.0, DumPoints[1].GetY());

    HoledShape1.ObtainContiguousnessPointsAt(ComplexLinearHole2, HGF2DLocation(12.0, 16.0, pWorld), &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(12.0, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(12.0, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(18.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(18.0, SecondDumPoint.GetY());

    DumPoints.clear();

    HVE2DComplexLinear ComplexLinearHole3(pWorld);
    ComplexLinearHole3.AppendLinear(HVE2DSegment(HGF2DLocation(12.0, 12.0, pWorld), HGF2DLocation(12.0, 18.0, pWorld)));
    ComplexLinearHole3.AppendLinear(HVE2DSegment(HGF2DLocation(12.0, 18.0, pWorld), HGF2DLocation(18.0, 18.0, pWorld)));
    ComplexLinearHole3.AppendLinear(HVE2DSegment(HGF2DLocation(18.0, 18.0, pWorld), HGF2DLocation(18.0, 12.0, pWorld)));
    ComplexLinearHole3.AppendLinear(HVE2DSegment(HGF2DLocation(18.0, 12.0, pWorld), HGF2DLocation(12.0, 12.0, pWorld)));

    ASSERT_TRUE(HoledShape1.AreContiguous(ComplexLinearHole3));
    ASSERT_TRUE(HoledShape1.AreContiguousAt(ComplexLinearHole3, HGF2DLocation(12.0, 12.0, pWorld)));
    ASSERT_EQ(0, HoledShape1.ObtainContiguousnessPoints(ComplexLinearHole3, &DumPoints));

    DumPoints.clear();

    }

//==================================================================================
// Interaction info with other segment
// Crosses(const HVE2DVector& pi_rVector) const;
// AreAdjacent(const HVE2DVector& pi_rVector) const;
// IsPointOn(const HGF2DLocation& pi_rTestPoint) const;
// IsPointOnSCS(const HGF2DLocation& pi_rTestPoint) const;
//==================================================================================
TEST_F(HVE2DHoledShapeTester,  InteractionTest)
    {

    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);
     
    // Tests with a vertical segment
    ASSERT_TRUE(HoledShape1.Crosses(ComplexLinearCase1));
    ASSERT_FALSE(HoledShape1.AreAdjacent(ComplexLinearCase1));

    ASSERT_TRUE(HoledShape1.Crosses(ComplexLinearCase2));
    ASSERT_TRUE(HoledShape1.AreAdjacent(ComplexLinearCase2));

    ASSERT_TRUE(HoledShape1.Crosses(ComplexLinearCase3));
    ASSERT_FALSE(HoledShape1.AreAdjacent(ComplexLinearCase3));

    ASSERT_FALSE(HoledShape1.Crosses(ComplexLinearCase4));
    ASSERT_TRUE(HoledShape1.AreAdjacent(ComplexLinearCase4));

    ASSERT_TRUE(HoledShape1.Crosses(ComplexLinearCase5));
    ASSERT_TRUE(HoledShape1.AreAdjacent(ComplexLinearCase5));

    ASSERT_FALSE(HoledShape1.Crosses(ComplexLinearCase6));
    ASSERT_TRUE(HoledShape1.AreAdjacent(ComplexLinearCase6));

    ASSERT_FALSE(HoledShape1.Crosses(ComplexLinearCase7));
    ASSERT_FALSE(HoledShape1.AreAdjacent(ComplexLinearCase7));

    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_FALSE(HoledShape1.IsPointOn(HGF2DLocation(10.0 - 1.1 * MYEPSILON, 10.0-1.1 * MYEPSILON, pWorld)));
    ASSERT_FALSE(HoledShape1.IsPointOn(HGF2DLocation(15.0, 20.0 - 1.1 * MYEPSILON, pWorld)));
    ASSERT_FALSE(HoledShape1.IsPointOn(HGF2DLocation(15.0, 20.0 + 1.1 * MYEPSILON, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(15.0, 20.0 + 0.9 * MYEPSILON, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(15.0, 20.0 - 0.9 * MYEPSILON, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(15.0, 20.0, pWorld)));

    ASSERT_TRUE(HoledShape1.IsPointOnSCS(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_FALSE(HoledShape1.IsPointOnSCS(HGF2DLocation(10.0 - 1.1 * MYEPSILON, 10.0-1.1 * MYEPSILON, pWorld)));
    ASSERT_FALSE(HoledShape1.IsPointOnSCS(HGF2DLocation(15.0, 20.0 - 1.1 * MYEPSILON, pWorld)));
    ASSERT_FALSE(HoledShape1.IsPointOnSCS(HGF2DLocation(15.0, 20.0 + 1.1 * MYEPSILON, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOnSCS(HGF2DLocation(15.0, 20.0 + 0.9 * MYEPSILON, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOnSCS(HGF2DLocation(15.0, 20.0 - 0.9 * MYEPSILON, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOnSCS(HGF2DLocation(15.0, 20.0, pWorld)));
    
    }

//==================================================================================
// Bearing calculation tests
// CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
//                  HVE2DVector::ArbitraryDiPolyion pi_DiPolyion = HVE2DVector::BETA) const;
// CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
//                              HVE2DVector::ArbitraryDiPolyion pi_DiPolyion = HVE2DVector::BETA) const;
//==================================================================================
TEST_F(HVE2DHoledShapeTester,  BearingTest)
    {

    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    // Obtain bearing ALPHA of a vertical segment
    ASSERT_NEAR(0.0, HoledShape1.CalculateBearing(Poly1Point0d0, HVE2DVector::ALPHA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(PI/2, HoledShape1.CalculateBearing(Poly1Point0d0, HVE2DVector::BETA).GetAngle());
    ASSERT_NEAR(0.0, HoledShape1.CalculateBearing(Poly1Point0d1, HVE2DVector::ALPHA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(PI, HoledShape1.CalculateBearing(Poly1Point0d1, HVE2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(PI, HoledShape1.CalculateBearing(Poly1Point0d5, HVE2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(3*PI/2, HoledShape1.CalculateBearing(Poly1Point0d5, HVE2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(3*PI/2, HoledShape1.CalculateBearing(Poly1Point1d0, HVE2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(PI/2, HoledShape1.CalculateBearing(Poly1Point1d0, HVE2DVector::BETA).GetAngle());

    // ANGULAR ACCELERATION
    // Obtain angular acceleration ALPHA of a vertical segment
    ASSERT_NEAR(0.0, HoledShape1.CalculateAngularAcceleration(Poly1Point0d0, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, HoledShape1.CalculateAngularAcceleration(Poly1Point0d0, HVE2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, HoledShape1.CalculateAngularAcceleration(Poly1Point0d1, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, HoledShape1.CalculateAngularAcceleration(Poly1Point0d1, HVE2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, HoledShape1.CalculateAngularAcceleration(Poly1Point0d5, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, HoledShape1.CalculateAngularAcceleration(Poly1Point0d5, HVE2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, HoledShape1.CalculateAngularAcceleration(Poly1Point1d0, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, HoledShape1.CalculateAngularAcceleration(Poly1Point1d0, HVE2DVector::BETA), MYEPSILON);
   
    }

//==================================================================================
// Extent calculation test
// GetExtent() const;
//==================================================================================
TEST_F (HVE2DHoledShapeTester, GetExtentTest)
    {

    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);
    HVE2DHoledShape NoHoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
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
// Scale(double pi_ScaleFactor, const HGF2DLocation& pi_rScaleOrigin);
////==================================================================================
TEST_F (HVE2DHoledShapeTester,  ScaleTest) 
    { 
    
    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    HoledShape1.Scale(1.0, HGF2DLocation(0.0, 0.0, pWorld));

    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(12.0, 12.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(12.0, 18.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(18.0, 12.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(18.0, 18.0, pWorld)));

    HoledShape1.Scale(2.0, HGF2DLocation(0.0, 0.0, pWorld));

    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(40.0, 20.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(20.0, 40.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(40.0, 40.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(24.0, 24.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(24.0, 36.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(36.0, 24.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(36.0, 36.0, pWorld)));

    HoledShape1.Scale(2.0, HGF2DLocation(10.0, 20.0, pWorld));

    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(30.0, 20.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(70.0, 20.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(30.0, 60.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(70.0, 60.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(38.0, 28.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(38.0, 52.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(62.0, 28.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(62.0, 52.0, pWorld)));
    
    }

////==================================================================================
// Move(const HGF2DDisplacement& pi_rDisplacement);
////==================================================================================
TEST_F (HVE2DHoledShapeTester,  MoveTest) 
    { 

    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    HoledShape1.Move(HGF2DDisplacement(10.0, 10.0));

    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(30.0, 20.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(20.0, 30.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(30.0, 30.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(22.0, 22.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(22.0, 28.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(28.0, 22.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(28.0, 28.0, pWorld)));

    HoledShape1.Move(HGF2DDisplacement(-10.0, -10.0));

    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(12.0, 12.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(12.0, 18.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(18.0, 12.0, pWorld)));
    ASSERT_TRUE(HoledShape1.IsPointOn(HGF2DLocation(18.0, 18.0, pWorld)));

    }

////==================================================================================
// DifferentiateShapeSCS(const HVE2DShape& pi_rShape) const;
// DifferentiateFromShapeSCS(const HVE2DShape& pi_rShape) const;
////==================================================================================
TEST_F (HVE2DHoledShapeTester,  DifferentiateTest) 
    { 
    
    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);
    HVE2DHoledShape HoledShape2(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);
    HoledShape2.AddHole(HVE2DRectangle(13.0, 13.0, 14.0, 14.0, pWorld));

    //Rectangle is in the hole
    HFCPtr<HVE2DShape>     pResultShape1 = HoledShape1.DifferentiateShapeSCS(HVE2DRectangle(13.0, 13.0, 16.0, 16.0, pWorld));

    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(12.0, 12.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(12.0, 18.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(18.0, 12.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(18.0, 18.0, pWorld)));

    //Rectangle is equal to the hole
    HFCPtr<HVE2DShape>     pResultShape2 = HoledShape1.DifferentiateShapeSCS(HVE2DRectangle(12.0, 12.0, 18.0, 18.0, pWorld));

    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(12.0, 12.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(12.0, 18.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(18.0, 12.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(18.0, 18.0, pWorld)));

    //Rectangle is bigger than hole but less than base
    HFCPtr<HVE2DShape>     pResultShape3 = HoledShape1.DifferentiateShapeSCS(HVE2DRectangle(11.0, 11.0, 19.0, 19.0, pWorld));

    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(11.0, 11.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(11.0, 19.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(19.0, 11.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(19.0, 19.0, pWorld)));

    //Rectangle is the same as base
    HFCPtr<HVE2DShape>     pResultShape4 = HoledShape1.DifferentiateShapeSCS(HVE2DRectangle(10.0, 10.0, 20.0, 20.0, pWorld));

    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape4->GetShapeType());

    HoledShape2.Move(HGF2DDisplacement(2.0, 0.0));
    HFCPtr<HVE2DShape>     pResultShape5 = HoledShape1.DifferentiateShapeSCS(HoledShape2);

    ASSERT_EQ(HVE2DPolygon::CLASS_ID, pResultShape5->GetShapeType());

    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(12.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(12.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(12.0, 12.0, pWorld)));

    HoledShape2.Move(HGF2DDisplacement(3.5, 0.0));
    HFCPtr<HVE2DShape>     pResultShape6 = HoledShape1.DifferentiateShapeSCS(HoledShape2);

    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(15.5, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(15.5, 12.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(12.0, 12.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(12.0, 18.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(15.5, 18.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(15.5, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));

    HFCPtr<HVE2DShape>     pResultShape7 = HoledShape1.DifferentiateShapeSCS(HVE2DRectangle(18.5, 18.5, 19.5, 19.5, pWorld));

    ASSERT_EQ(2, pResultShape7->GetHoleList().size());

    HoledShape2.Move(HGF2DDisplacement(2.0, 0.0));
    HFCPtr<HVE2DShape>     pResultShape8 = HoledShape2.DifferentiateFromShapeSCS(HoledShape1);

    ASSERT_EQ(HVE2DPolygon::CLASS_ID, pResultShape8->GetShapeType());

    ASSERT_TRUE(pResultShape8->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape8->IsPointOn(HGF2DLocation(12.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape8->IsPointOn(HGF2DLocation(12.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape8->IsPointOn(HGF2DLocation(12.0, 10.0, pWorld)));

    HoledShape2.Move(HGF2DDisplacement(3.5, 0.0));
    HFCPtr<HVE2DShape>     pResultShape9 = HoledShape2.DifferentiateFromShapeSCS(HoledShape1);

    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DLocation(15.5, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DLocation(15.5, 12.0, pWorld)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DLocation(12.0, 12.0, pWorld)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DLocation(12.0, 18.0, pWorld)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DLocation(15.5, 18.0, pWorld)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DLocation(15.5, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape9->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));

    }

////==================================================================================
// IntersectShapeSCS(const HVE2DShape& pi_rShape) const;
////==================================================================================
TEST_F (HVE2DHoledShapeTester,  IntersectShapeSCSTest) 
    { 
    
    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);
    HVE2DHoledShape HoledShape2(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);
    HoledShape2.AddHole(HVE2DRectangle(13.0, 13.0, 14.0, 14.0, pWorld));

    //Rectangle is in the hole
    HFCPtr<HVE2DShape>     pResultShape1 = HoledShape1.IntersectShapeSCS(HVE2DRectangle(13.0, 13.0, 16.0, 16.0, pWorld));

    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape1->GetShapeType());

    //Rectangle is equal to the hole
    HFCPtr<HVE2DShape>     pResultShape2 = HoledShape1.IntersectShapeSCS(HVE2DRectangle(12.0, 12.0, 18.0, 18.0, pWorld));

    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, pResultShape2->GetShapeType());

    //Rectangle is bigger than hole but less than base
    HFCPtr<HVE2DShape>     pResultShape3 = HoledShape1.IntersectShapeSCS(HVE2DRectangle(11.0, 11.0, 19.0, 19.0, pWorld));

    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(12.0, 12.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(18.0, 12.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(12.0, 18.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(18.0, 18.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(11.0, 11.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(11.0, 19.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(19.0, 11.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(19.0, 19.0, pWorld)));

    //Rectangle is the same as base
    HFCPtr<HVE2DShape>     pResultShape4 = HoledShape1.IntersectShapeSCS(HVE2DRectangle(10.0, 10.0, 20.0, 20.0, pWorld));

    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(12.0, 12.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(12.0, 18.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(18.0, 12.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(18.0, 18.0, pWorld)));

    HoledShape2.Move(HGF2DDisplacement(2.0, 0.0));
    HFCPtr<HVE2DShape>     pResultShape5 = HoledShape1.IntersectShapeSCS(HoledShape2);

    ASSERT_EQ(HVE2DPolygon::CLASS_ID, pResultShape5->GetShapeType());

    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(12.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(12.0, 12.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(18.0, 12.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(18.0, 18.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(12.0, 18.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(12.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape5->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));

    HoledShape2.Move(HGF2DDisplacement(3.5, 0.0));
    HFCPtr<HVE2DShape>     pResultShape6 = HoledShape1.IntersectShapeSCS(HoledShape2);

    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(15.5, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(15.5, 12.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(18.0, 12.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(18.0, 18.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(15.5, 18.0, pWorld)));
    ASSERT_TRUE(pResultShape6->IsPointOn(HGF2DLocation(15.5, 20.0, pWorld)));

    }

////==================================================================================
// UnifyShapeSCS(const HVE2DShape& pi_rShape) const;
////==================================================================================
TEST_F (HVE2DHoledShapeTester,  UnifyTest) 
    { 
    
    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);
    HVE2DHoledShape HoledShape2(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);
    HoledShape2.AddHole(HVE2DRectangle(13.0, 13.0, 14.0, 14.0, pWorld));

    //Rectangle is in the hole
    HFCPtr<HVE2DShape>     pResultShape1 = HoledShape1.UnifyShapeSCS(HVE2DRectangle(13.0, 13.0, 16.0, 16.0, pWorld));

    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(12.0, 12.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(12.0, 18.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(18.0, 12.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(18.0, 18.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(13.0, 13.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(13.0, 16.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(16.0, 13.0, pWorld)));
    ASSERT_TRUE(pResultShape1->IsPointOn(HGF2DLocation(16.0, 16.0, pWorld)));

    //Rectangle is equal to the hole
    HFCPtr<HVE2DShape>     pResultShape2 = HoledShape1.UnifyShapeSCS(HVE2DRectangle(12.0, 12.0, 18.0, 18.0, pWorld));

    ASSERT_FALSE(pResultShape2->HasHoles());

    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape2->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    //Rectangle is bigger than hole but less than base
    HFCPtr<HVE2DShape>     pResultShape3 = HoledShape1.UnifyShapeSCS(HVE2DRectangle(11.0, 11.0, 19.0, 19.0, pWorld));

    ASSERT_FALSE(pResultShape3->HasHoles());

    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pResultShape3->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    //Rectangle is bigger than base
    HFCPtr<HVE2DShape>     pResultShape4 = HoledShape1.UnifyShapeSCS(HVE2DRectangle(0.0, 0.0, 40.0, 40.0, pWorld));

    ASSERT_FALSE(pResultShape4->HasHoles());

    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(40.0, 0.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(0.0, 40.0, pWorld)));
    ASSERT_TRUE(pResultShape4->IsPointOn(HGF2DLocation(40.0, 40.0, pWorld)));

    }

////==================================================================================
// Cloning tests
// Clone() const;
// AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
////==================================================================================
TEST_F (HVE2DHoledShapeTester,  CloningTest) 
    { 
    
    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    //General Clone Test
    HFCPtr<HVE2DHoledShape> pClone = (HVE2DHoledShape*)HoledShape1.Clone();
    ASSERT_FALSE(pClone->IsEmpty());
    ASSERT_EQ(pWorld, pClone->GetCoordSys());
    
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(12.0, 12.0, pWorld)));
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(12.0, 18.0, pWorld)));
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(18.0, 12.0, pWorld)));
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(18.0, 18.0, pWorld)));
         
    // Test with the same coordinate system
    HFCPtr<HVE2DHoledShape> pClone2 = (HVE2DHoledShape*) HoledShape1.AllocateCopyInCoordSys(pWorld);
    ASSERT_FALSE(pClone2->IsEmpty());
    ASSERT_EQ(pWorld, pClone2->GetCoordSys());
    
    ASSERT_TRUE(pClone2->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(pClone2->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(pClone2->IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(pClone2->IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(pClone2->IsPointOn(HGF2DLocation(12.0, 12.0, pWorld)));
    ASSERT_TRUE(pClone2->IsPointOn(HGF2DLocation(12.0, 18.0, pWorld)));
    ASSERT_TRUE(pClone2->IsPointOn(HGF2DLocation(18.0, 12.0, pWorld)));
    ASSERT_TRUE(pClone2->IsPointOn(HGF2DLocation(18.0, 18.0, pWorld)));

    // Test with a translation between systems
    Translation = HGF2DDisplacement (10.0, 10.0);
    HGF2DTranslation myTranslation(Translation);
    HFCPtr<HGF2DCoordSys> pWorldTranslation = new HGF2DCoordSys(myTranslation, pWorld);
  
    HFCPtr<HVE2DHoledShape> pClone5 = (HVE2DHoledShape*)HoledShape1.AllocateCopyInCoordSys(pWorldTranslation);
    ASSERT_FALSE(pClone5->IsEmpty());
    ASSERT_EQ(pWorldTranslation, pClone5->GetCoordSys());

    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(0.0, 0.0, pWorldTranslation)));
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(10.0, 0.0, pWorldTranslation)));
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(0.0, 10.0, pWorldTranslation)));
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(10.0, 10.0, pWorldTranslation)));
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(2.0, 2.0, pWorldTranslation)));
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(2.0, 8.0, pWorldTranslation)));
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(8.0, 2.0, pWorldTranslation)));
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(8.0, 8.0, pWorldTranslation)));

    // Test with a stretch between systems
    HGF2DStretch myStretch;
    myStretch.SetTranslation(Translation);
    myStretch.SetXScaling(0.5);
    myStretch.SetYScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldStretch = new HGF2DCoordSys(myStretch, pWorld);
   
    HFCPtr<HVE2DHoledShape> pClone6 = (HVE2DHoledShape*)HoledShape1.AllocateCopyInCoordSys(pWorldStretch);
    ASSERT_FALSE(pClone6->IsEmpty());
    ASSERT_EQ(pWorldStretch, pClone6->GetCoordSys());
     
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(0.0, 0.0, pWorldStretch)));
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(20.0, 0.0, pWorldStretch)));
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(0.0, 20.0, pWorldStretch)));
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(20.0, 20.0, pWorldStretch)));
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(4.0, 4.0, pWorldStretch)));
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(4.0, 16.0, pWorldStretch)));
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(16.0, 4.0, pWorldStretch)));
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(16.0, 16.0, pWorldStretch)));

    // Test with a similitude between systems
    HGF2DSimilitude mySimilitude;
    mySimilitude.SetRotation(PI);
    mySimilitude.SetScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldSimilitude = new HGF2DCoordSys(mySimilitude, pWorld);
  
    HFCPtr<HVE2DHoledShape> pClone7 = (HVE2DHoledShape*)HoledShape1.AllocateCopyInCoordSys(pWorldSimilitude);
    ASSERT_FALSE(pClone7->IsEmpty());
    ASSERT_EQ(pWorldSimilitude, pClone7->GetCoordSys());
    
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-20.0, -20.0, pWorldSimilitude)));
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-40.0, -20.0, pWorldSimilitude)));
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-20.0, -40.0, pWorldSimilitude)));
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-40.0, -40.0, pWorldSimilitude)));
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-24.0, -24.0, pWorldSimilitude)));
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-24.0, -36.0, pWorldSimilitude)));
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-36.0, -24.0, pWorldSimilitude)));
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-36.0, -36.0, pWorldSimilitude)));

    // Test with a affine between systems
    HGF2DAffine myAffine;
    myAffine.SetTranslation(Translation);
    myAffine.SetRotation(PI);
    myAffine.SetXScaling(0.5);
    myAffine.SetYScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldAffine = new HGF2DCoordSys(myAffine, pWorld);
   
    HFCPtr<HVE2DHoledShape> pClone8 = (HVE2DHoledShape*)HoledShape1.AllocateCopyInCoordSys(pWorldAffine);
    ASSERT_FALSE(pClone8->IsEmpty());
    ASSERT_EQ(pWorldAffine, pClone8->GetCoordSys());

    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(0.0, 0.0, pWorldAffine)));
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-20.0, 0.0, pWorldAffine)));
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(0.0, -20.0, pWorldAffine)));
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-20.0, -20.0, pWorldAffine)));
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-4.0, -4.0, pWorldAffine)));
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-4.0, -16.0, pWorldAffine)));
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-16.0, -4.0, pWorldAffine)));
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-16.0, -16.0, pWorldAffine)));

    }

//==================================================================================
// ********************************************************************************
//==================================================================================

//==================================================================================
// Diff test with overlapping hole
//==================================================================================
TEST_F (HVE2DHoledShapeTester, ModifyShapeTest)
    {
   
    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear2);
    HoledShape1.AddHole(Poly1B);

    HVE2DPolygon    Poly1C(MyLinear6);

    HFCPtr<HVE2DShape> pResultShape = HoledShape1.DifferentiateShape(Poly1C);
    ASSERT_FALSE(pResultShape->HasHoles());
    ASSERT_DOUBLE_EQ(52.0, pResultShape->CalculateArea());

    HFCPtr<HVE2DShape> pResultShape2 = HoledShape1.IntersectShape(Poly1C);
    ASSERT_FALSE(pResultShape2->HasHoles());
    ASSERT_DOUBLE_EQ(12.0, pResultShape2->CalculateArea());

    HFCPtr<HVE2DShape> pResultShape3 = HoledShape1.UnifyShape(Poly1C);
    ASSERT_TRUE(pResultShape3->HasHoles());
    ASSERT_DOUBLE_EQ(64.0, pResultShape3->CalculateArea());

    HFCPtr<HVE2DShape> pResultShape4 = Poly1C.DifferentiateFromShapeSCS(HoledShape1);
    ASSERT_FALSE(pResultShape4->HasHoles());
    ASSERT_DOUBLE_EQ(52.0, pResultShape4->CalculateArea());

    }

//==================================================================================
// Diff test with overlapping hole
//==================================================================================
TEST_F (HVE2DHoledShapeTester, ModifyShapeTest2)
    {

    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear4);
    HoledShape1.AddHole(Poly1B);

    HVE2DPolygon    Poly1C(MyLinear5);
    HVE2DHoledShape     HoledShape2(Poly1C);

    HVE2DPolygon    Poly1D(MyLinear6);
    HoledShape2.AddHole(Poly1D);

    HFCPtr<HVE2DShape> pResultShape = HoledShape1.DifferentiateShape(HoledShape2);
    ASSERT_FALSE(pResultShape->HasHoles());
    ASSERT_DOUBLE_EQ(32.0, pResultShape->CalculateArea());

    HFCPtr<HVE2DShape> pResultShape2 = HoledShape1.IntersectShape(HoledShape2);
    ASSERT_TRUE(pResultShape2->IsComplex());
    ASSERT_DOUBLE_EQ(32.0, pResultShape2->CalculateArea());

    HFCPtr<HVE2DShape> pResultShape3 = HoledShape1.UnifyShape(HoledShape2);
    ASSERT_FALSE(pResultShape3->HasHoles());
    ASSERT_DOUBLE_EQ(110.0, pResultShape3->CalculateArea());

    HFCPtr<HVE2DShape> pResultShape4 = HoledShape2.DifferentiateFromShapeSCS(HoledShape1);
    ASSERT_FALSE(pResultShape4->HasHoles());
    ASSERT_DOUBLE_EQ(32.0, pResultShape4->CalculateArea());

    }

//==================================================================================
// Diff test with overlapping hole
//==================================================================================
TEST_F (HVE2DHoledShapeTester, ModifyShapeTest3)
    {

    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear4);
    HoledShape1.AddHole(Poly1B);

    HVE2DHoledShape     HoledShape2(HoledShape1);

    HFCPtr<HVE2DShape> pResultShape = HoledShape1.DifferentiateShape(HoledShape2);
    ASSERT_TRUE(pResultShape->IsEmpty());

    HFCPtr<HVE2DShape> pResultShape2 = HoledShape1.IntersectShape(HoledShape2);
    ASSERT_TRUE(pResultShape2->HasHoles());
    ASSERT_DOUBLE_EQ(64.0, pResultShape2->CalculateArea());

    HFCPtr<HVE2DShape> pResultShape3 = HoledShape1.UnifyShape(HoledShape2);
    ASSERT_TRUE(pResultShape3->HasHoles());
    ASSERT_DOUBLE_EQ(64.0, pResultShape3->CalculateArea());

    HFCPtr<HVE2DShape> pResultShape4 = HoledShape2.DifferentiateFromShapeSCS(HoledShape1);
    ASSERT_TRUE(pResultShape4->IsEmpty());

    }

//==================================================================================
// Diff test with overlapping hole
//==================================================================================
TEST_F (HVE2DHoledShapeTester, ModifyShapeTest4)
    {

    HVE2DPolygon    Poly1A(MyLinear1);
    HVE2DHoledShape HoledShape1(Poly1A);

    HVE2DPolygon    Poly1B(MyLinear4);
    HoledShape1.AddHole(Poly1B);

    HVE2DPolygon    Poly1C(MyLinear5);
    HVE2DHoledShape     HoledShape2(Poly1C);

    HVE2DSegment    Segment1G(HGF2DLocation(17.0, 12.0, pWorld), HGF2DLocation(17.0, 18.0, pWorld));
    HVE2DSegment    Segment2G(HGF2DLocation(17.0, 18.0, pWorld), HGF2DLocation(20.0, 18.0, pWorld));
    HVE2DSegment    Segment3G(HGF2DLocation(20.0, 18.0, pWorld), HGF2DLocation(20.0, 12.0, pWorld));
    HVE2DSegment    Segment4G(HGF2DLocation(20.0, 12.0, pWorld), HGF2DLocation(17.0, 12.0, pWorld));

    HVE2DComplexLinear  MyLinear7(pWorld);
    MyLinear7.AppendLinear(Segment1G);
    MyLinear7.AppendLinear(Segment2G);
    MyLinear7.AppendLinear(Segment3G);
    MyLinear7.AppendLinear(Segment4G);

    HVE2DPolygon    Poly1D(MyLinear7);

    HoledShape2.AddHole(Poly1D);

    HFCPtr<HVE2DShape> pResultShape = HoledShape1.DifferentiateShape(HoledShape2);
    ASSERT_FALSE(pResultShape->HasHoles());
    ASSERT_DOUBLE_EQ(32.0, pResultShape->CalculateArea());

    HFCPtr<HVE2DShape> pResultShape2 = HoledShape1.IntersectShape(HoledShape2);
    ASSERT_TRUE(pResultShape2->IsComplex());
    ASSERT_DOUBLE_EQ(32.0, pResultShape2->CalculateArea());

    HFCPtr<HVE2DShape> pResultShape3 = HoledShape1.UnifyShape(HoledShape2);
    ASSERT_TRUE(pResultShape3->HasHoles());
    ASSERT_DOUBLE_EQ(104.0, pResultShape3->CalculateArea());

    HFCPtr<HVE2DShape> pResultShape4 = HoledShape2.DifferentiateFromShapeSCS(HoledShape1);

    ASSERT_FALSE(pResultShape4->HasHoles());
    ASSERT_DOUBLE_EQ(32.0, pResultShape4->CalculateArea());

    }


//==================================================================================
// Case which failed 16 feb 1998
//==================================================================================
TEST_F (HVE2DHoledShapeTester, UnifyShapeSCSTest)
    {

    HFCPtr<HGF2DCoordSys>   pTheCurrentWorld = new HGF2DCoordSys();
        
    HVE2DComplexLinear  TheLinear1(pTheCurrentWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(332946.275207202940000 , 5053264.982890859200000 , pTheCurrentWorld),
                                         HGF2DLocation(332946.275207202940000 , 5067904.982890859200000 , pTheCurrentWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(332946.275207202940000 , 5067904.982890859200000 , pTheCurrentWorld),
                                         HGF2DLocation(343846.275207202940000 , 5067904.982890859200000 , pTheCurrentWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(343846.275207202940000 , 5067904.982890859200000 , pTheCurrentWorld),
                                         HGF2DLocation(343846.275207202940000 , 5053264.982890859200000 , pTheCurrentWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(343846.275207202940000 , 5053264.982890859200000 , pTheCurrentWorld),
                                         HGF2DLocation(332946.275207202940000 , 5053264.982890859200000 , pTheCurrentWorld)));

    HVE2DPolygonOfSegments BaseShape(TheLinear1);

    HVE2DHoledShape  MyHoled(BaseShape);

    HVE2DComplexLinear  TheLinear2(pTheCurrentWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(341177.246740867090000 , 5061604.832691806400000 , pTheCurrentWorld),
                                         HGF2DLocation(342460.051131906220000 , 5061085.528745555300000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(342460.051131906220000 , 5061085.528745555300000 , pTheCurrentWorld),
                                         HGF2DLocation(342118.889531906230000 , 5060183.418745555000000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(342118.889531906230000 , 5060183.418745555000000 , pTheCurrentWorld),
                                         HGF2DLocation(342055.379185642580000 , 5060226.807001913000000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(342055.379185642580000 , 5060226.807001913000000 , pTheCurrentWorld),
                                         HGF2DLocation(341780.836003819250000 , 5059534.034487032300000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(341780.836003819250000 , 5059534.034487032300000 , pTheCurrentWorld),
                                         HGF2DLocation(341721.252014425350000 , 5059566.685309503200000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(341721.252014425350000 , 5059566.685309503200000 , pTheCurrentWorld),
                                         HGF2DLocation(341422.392846682460000 , 5058872.039676371000000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(341422.392846682460000 , 5058872.039676371000000 , pTheCurrentWorld),
                                         HGF2DLocation(341353.551715074170000 , 5058923.200298768500000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(341353.551715074170000 , 5058923.200298768500000 , pTheCurrentWorld),
                                         HGF2DLocation(341102.787610408970000 , 5058218.806746337600000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(341102.787610408970000 , 5058218.806746337600000 , pTheCurrentWorld),
                                         HGF2DLocation(341018.473817025950000 , 5058245.244122228600000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(341018.473817025950000 , 5058245.244122228600000 , pTheCurrentWorld),
                                         HGF2DLocation(340710.631351472980000 , 5057445.429118268200000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(340710.631351472980000 , 5057445.429118268200000 , pTheCurrentWorld),
                                         HGF2DLocation(339340.487223472970000 , 5057965.696110268100000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(339340.487223472970000 , 5057965.696110268100000 , pTheCurrentWorld),
                                         HGF2DLocation(339544.469184219080000 , 5058495.667933515300000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(339544.469184219080000 , 5058495.667933515300000 , pTheCurrentWorld),
                                         HGF2DLocation(338548.192346269730000 , 5059111.277800547000000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(338548.192346269730000 , 5059111.277800547000000 , pTheCurrentWorld),
                                         HGF2DLocation(338869.578704333860000 , 5059640.620037358300000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(338869.578704333860000 , 5059640.620037358300000 , pTheCurrentWorld),
                                         HGF2DLocation(338732.219809614940000 , 5059739.195244156800000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(338732.219809614940000 , 5059739.195244156800000 , pTheCurrentWorld),
                                         HGF2DLocation(338892.919336697200000 , 5060015.955540797700000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(338892.919336697200000 , 5060015.955540797700000 , pTheCurrentWorld),
                                         HGF2DLocation(338858.029650495740000 , 5060035.606053715600000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(338858.029650495740000 , 5060035.606053715600000 , pTheCurrentWorld),
                                         HGF2DLocation(339109.368566016610000 , 5060463.416973751000000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(339109.368566016610000 , 5060463.416973751000000 , pTheCurrentWorld),
                                         HGF2DLocation(339094.638933242650000 , 5060472.955741963300000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(339094.638933242650000 , 5060472.955741963300000 , pTheCurrentWorld),
                                         HGF2DLocation(339336.151112755120000 , 5060851.971206161200000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(339336.151112755120000 , 5060851.971206161200000 , pTheCurrentWorld),
                                         HGF2DLocation(339264.579283097530000 , 5060905.711991752500000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(339264.579283097530000 , 5060905.711991752500000 , pTheCurrentWorld),
                                         HGF2DLocation(339697.443283097530000 , 5061597.091991752400000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(339697.443283097530000 , 5061597.091991752400000 , pTheCurrentWorld),
                                         HGF2DLocation(339844.199008590430000 , 5061504.173285123000000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(339844.199008590430000 , 5061504.173285123000000 , pTheCurrentWorld),
                                         HGF2DLocation(339848.373542428250000 , 5061516.001130996300000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(339848.373542428250000 , 5061516.001130996300000 , pTheCurrentWorld),
                                         HGF2DLocation(339670.860111388200000 , 5061651.556841972300000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(339670.860111388200000 , 5061651.556841972300000 , pTheCurrentWorld),
                                         HGF2DLocation(339718.129087228740000 , 5061721.838345524900000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(339718.129087228740000 , 5061721.838345524900000 , pTheCurrentWorld),
                                         HGF2DLocation(339709.818191450730000 , 5061727.697993952800000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(339709.818191450730000 , 5061727.697993952800000 , pTheCurrentWorld),
                                         HGF2DLocation(340128.870191450750000 , 5062351.817993952900000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(340128.870191450750000 , 5062351.817993952900000 , pTheCurrentWorld),
                                         HGF2DLocation(341240.398191450750000 , 5061700.949993953100000 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(341240.398191450750000 , 5061700.949993953100000 , pTheCurrentWorld),
                                         HGF2DLocation(341177.246740867090000 , 5061604.832691806400000 , pTheCurrentWorld)));

    HVE2DPolygonOfSegments TheHole(TheLinear2);

    MyHoled.AddHole(TheHole);

    HVE2DComplexLinear  TheLinear3(pTheCurrentWorld);

    TheLinear3.AppendLinear(HVE2DSegment(HGF2DLocation(340591.273152340440000 , 5061106.222726583500000 , pTheCurrentWorld),
                                         HGF2DLocation(340555.199749660210000 , 5061060.921244148200000 , pTheCurrentWorld)));
    TheLinear3.AppendLinear(HVE2DSegment(HGF2DLocation(340555.199749660210000 , 5061060.921244148200000 , pTheCurrentWorld),
                                         HGF2DLocation(340553.930404597490000 , 5061054.805308845800000 , pTheCurrentWorld)));
    TheLinear3.AppendLinear(HVE2DSegment(HGF2DLocation(340553.930404597490000 , 5061054.805308845800000 , pTheCurrentWorld),
                                         HGF2DLocation(340747.516262911380000 , 5060932.236001815600000 , pTheCurrentWorld)));
    TheLinear3.AppendLinear(HVE2DSegment(HGF2DLocation(340747.516262911380000 , 5060932.236001815600000 , pTheCurrentWorld),
                                         HGF2DLocation(340756.159139819270000 , 5060954.832439032400000 , pTheCurrentWorld)));
    TheLinear3.AppendLinear(HVE2DSegment(HGF2DLocation(340756.159139819270000 , 5060954.832439032400000 , pTheCurrentWorld),
                                         HGF2DLocation(340974.188876229980000 , 5060869.697018148400000 , pTheCurrentWorld)));
    TheLinear3.AppendLinear(HVE2DSegment(HGF2DLocation(340974.188876229980000 , 5060869.697018148400000 , pTheCurrentWorld),
                                         HGF2DLocation(340836.253131906210000 , 5060967.434345554600000 , pTheCurrentWorld)));
    TheLinear3.AppendLinear(HVE2DSegment(HGF2DLocation(340836.253131906210000 , 5060967.434345554600000 , pTheCurrentWorld),
                                         HGF2DLocation(340951.431374093460000 , 5061261.138863132300000 , pTheCurrentWorld)));
    TheLinear3.AppendLinear(HVE2DSegment(HGF2DLocation(340951.431374093460000 , 5061261.138863132300000 , pTheCurrentWorld),
                                         HGF2DLocation(340767.850191450740000 , 5060981.725993952700000 , pTheCurrentWorld)));
    TheLinear3.AppendLinear(HVE2DSegment(HGF2DLocation(340767.850191450740000 , 5060981.725993952700000 , pTheCurrentWorld),
                                         HGF2DLocation(340591.273152340440000 , 5061106.222726583500000 , pTheCurrentWorld)));

    HVE2DPolygonOfSegments TheSimple(TheLinear3);

    HFCPtr<HVE2DShape> NewShape = MyHoled.UnifyShapeSCS(TheSimple);

    }

//==================================================================================
// Case which failed 16 feb 2000
//==================================================================================
TEST_F (HVE2DHoledShapeTester, DifferentiateShapeTest)
    {
    HFCPtr<HGF2DCoordSys>   pTheCurrentWorld = new HGF2DCoordSys();
        
    HVE2DComplexLinear  TheLinear1(pTheCurrentWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 0.0 , pTheCurrentWorld), HGF2DLocation(0.0, 100.0 , pTheCurrentWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 100.0 , pTheCurrentWorld), HGF2DLocation(100.0, 100.0 , pTheCurrentWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(100.0, 100.0 , pTheCurrentWorld), HGF2DLocation(100.0, 0.0 , pTheCurrentWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(100.0, 0.0 , pTheCurrentWorld), HGF2DLocation(0.0, 0.0 , pTheCurrentWorld)));

    HVE2DPolygonOfSegments BaseShape(TheLinear1);

    HVE2DHoledShape  MyHoled(BaseShape);

    HVE2DComplexLinear  TheLinear2(pTheCurrentWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(30.0, 30.0 , pTheCurrentWorld), HGF2DLocation(30.0, 70.0 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(30.0, 70.0 , pTheCurrentWorld), HGF2DLocation(70.0, 70.0 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(70.0, 70.0 , pTheCurrentWorld), HGF2DLocation(70.0, 30.0 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(70.0, 30.0 , pTheCurrentWorld), HGF2DLocation(30.0, 30.0 , pTheCurrentWorld)));

    HVE2DPolygonOfSegments TheHole(TheLinear2);

    MyHoled.AddHole(TheHole);

    HVE2DComplexLinear  TheLinear3(pTheCurrentWorld);

    TheLinear3.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 10.0 , pTheCurrentWorld), HGF2DLocation(10.0, 70.0 , pTheCurrentWorld)));
    TheLinear3.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 70.0 , pTheCurrentWorld), HGF2DLocation(30.0, 70.0 , pTheCurrentWorld)));
    TheLinear3.AppendLinear(HVE2DSegment(HGF2DLocation(30.0, 70.0 , pTheCurrentWorld), HGF2DLocation(30.0, 10.0 , pTheCurrentWorld)));
    TheLinear3.AppendLinear(HVE2DSegment(HGF2DLocation(30.0, 10.0 , pTheCurrentWorld), HGF2DLocation(10.0, 10.0 , pTheCurrentWorld)));

    HVE2DPolygonOfSegments TheSimple(TheLinear3);

    HFCPtr<HVE2DShape> NewShape = MyHoled.DifferentiateShape(TheSimple);

    ASSERT_DOUBLE_EQ(7200.0, NewShape->CalculateArea());


    }

//==================================================================================
// Intersection when hole overlaps rectangle to intersect with
// Case which failed 16 feb 2000
//==================================================================================
TEST_F (HVE2DHoledShapeTester, ModifyShapeTest5)
    {
        
    HFCPtr<HGF2DCoordSys>   pTheCurrentWorld = new HGF2DCoordSys();
        
    HVE2DComplexLinear  TheLinear1(pTheCurrentWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-100.0, -100.0 , pTheCurrentWorld), HGF2DLocation(-100.0, 200.0 , pTheCurrentWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-100.0, 200.0 , pTheCurrentWorld), HGF2DLocation(200.0, 200.0 , pTheCurrentWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(200.0, 200.0 , pTheCurrentWorld), HGF2DLocation(200.0, -100.0 , pTheCurrentWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(200.0, -100.0 , pTheCurrentWorld), HGF2DLocation(-100.0, -100.0 , pTheCurrentWorld)));

    HVE2DPolygonOfSegments BaseShape(TheLinear1);

    HVE2DHoledShape  MyHoled(BaseShape);

    HVE2DComplexLinear  TheLinear2(pTheCurrentWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 0.0 , pTheCurrentWorld), HGF2DLocation(0.0, 100.0 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 100.0 , pTheCurrentWorld), HGF2DLocation(100.0, 100.0 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(100.0, 100.0 , pTheCurrentWorld), HGF2DLocation(100.0, 0.0 , pTheCurrentWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(100.0, 0.0 , pTheCurrentWorld), HGF2DLocation(0.0, 0.0 , pTheCurrentWorld)));

    HVE2DPolygonOfSegments TheHole(TheLinear2);

    MyHoled.AddHole(TheHole);

    HVE2DRectangle TheRectangle(0.0, 0.0, 100.0, 100.0, pTheCurrentWorld);

    HFCPtr<HVE2DShape> NewShape = MyHoled.DifferentiateShape(TheRectangle);
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