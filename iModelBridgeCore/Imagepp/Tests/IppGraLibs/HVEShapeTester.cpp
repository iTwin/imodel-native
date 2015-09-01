//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HVEShapeTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "EnvironnementTest.h"
#include "HVEShapeTester.h"

HVEShapeTester::HVEShapeTester() 
    {

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

    MyShapeCount = 16;

    //Extent
    Extent = HGF2DExtent(0.0, 0.0, 10.0, 10.0, pWorld);

    //Shape
    Shape1 = HVEShape(pWorld);    
    Shape3 = HVEShape(Extent);
    Shape4 = HVEShape(0.0, 0.0, 10.0, 10.0, pWorld);
    Shape6 = HVEShape(&MyShapeCount, DblArray, pWorld);
    Shape7 = HVEShape(0.0, 0.0, 5.0, 5.0, pWorld);
    Shape8 = HVEShape(-5.0, -5.0, 0.0, 0.0, pWorld);

    }

// THE PRESENT TEST CANNOT MAKE TESTS WITH NON-LINEAR TRANSFORMATION MODELS

//==================================================================================
// Shape Construction tests
//==================================================================================
TEST_F (HVEShapeTester, ConstructorsTest)
    {

    // Default Constructor
    HVEShape Shape;

    // Constructor with a coordinate system
    HVEShape Shape1(pWorld);
    ASSERT_EQ(pWorld, Shape1.GetCoordSys());

    HVEShape Shape2(pSys1);
    ASSERT_EQ(pSys1, Shape2.GetCoordSys());

    //Constructor from extent
    HGF2DExtent Extent(0.0, 0.0, 10.0, 10.0, pWorld);
    HVEShape Shape3(Extent);
    
    ASSERT_EQ(pWorld, Shape3.GetCoordSys());

    ASSERT_TRUE(Shape3.IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(Shape3.IsPointOn(HGF2DLocation(0.0, 10.0, pWorld)));
    ASSERT_TRUE(Shape3.IsPointOn(HGF2DLocation(10.0, 0.0, pWorld)));
    ASSERT_TRUE(Shape3.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));

    //Constructor representing a rectangle
    HVEShape Shape4(0.0, 0.0, 10.0, 10.0, pWorld);

    ASSERT_EQ(pWorld, Shape4.GetCoordSys());

    ASSERT_TRUE(Shape4.IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(Shape4.IsPointOn(HGF2DLocation(0.0, 10.0, pWorld)));
    ASSERT_TRUE(Shape4.IsPointOn(HGF2DLocation(10.0, 0.0, pWorld)));
    ASSERT_TRUE(Shape4.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));

    //Copy constructor
    HVEShape Shape5(Shape4);

    ASSERT_EQ(pWorld, Shape5.GetCoordSys());

    ASSERT_TRUE(Shape5.IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(Shape5.IsPointOn(HGF2DLocation(0.0, 10.0, pWorld)));
    ASSERT_TRUE(Shape5.IsPointOn(HGF2DLocation(10.0, 0.0, pWorld)));
    ASSERT_TRUE(Shape5.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));

    //Constructor from array
    HVEShape Shape6(&MyShapeCount, DblArray, pWorld);

    ASSERT_EQ(pWorld, Shape6.GetCoordSys());

    ASSERT_TRUE(Shape6.IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(Shape6.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(Shape6.IsPointOn(HGF2DLocation(15.0, 5.0, pWorld)));
    ASSERT_TRUE(Shape6.IsPointOn(HGF2DLocation(0.0, 5.0, pWorld)));
    ASSERT_TRUE(Shape6.IsPointOn(HGF2DLocation(5.0, 0.0, pWorld)));
    ASSERT_TRUE(Shape6.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(Shape6.IsPointOn(HGF2DLocation(-20.0, 20.0, pWorld)));
        
    }

//==================================================================================
// operator=(const HVE2DPolygon& pi_rObj);
//==================================================================================
TEST_F (HVEShapeTester, OperatorTest)
    {
    
    HVEShape NewShape = Shape4;

    ASSERT_EQ(pWorld, NewShape.GetCoordSys());

    ASSERT_TRUE(NewShape.IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(NewShape.IsPointOn(HGF2DLocation(0.0, 10.0, pWorld)));
    ASSERT_TRUE(NewShape.IsPointOn(HGF2DLocation(10.0, 0.0, pWorld)));
    ASSERT_TRUE(NewShape.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));

    }

//==================================================================================
// ChangeCoordSys( const HFCPtr<HGF2DCoordSys>& pi_pCoordSys )
//==================================================================================
TEST_F (HVEShapeTester, ChangeCoordSysTest)
    {

    Shape1.ChangeCoordSys(pSys1);
    ASSERT_EQ(pSys1, Shape1.GetCoordSys());

    Shape1.ChangeCoordSys(pWorld);
    ASSERT_EQ(pWorld, Shape1.GetCoordSys());

    Shape3.ChangeCoordSys(pSys1);
    ASSERT_EQ(pSys1, Shape3.GetCoordSys());

    Shape3.ChangeCoordSys(pWorld);
    ASSERT_EQ(pWorld, Shape3.GetCoordSys());

    Shape4.ChangeCoordSys(pSys1);
    ASSERT_EQ(pSys1, Shape4.GetCoordSys());

    Shape4.ChangeCoordSys(pWorld);
    ASSERT_EQ(pWorld, Shape4.GetCoordSys());

    Shape6.ChangeCoordSys(pSys1);
    ASSERT_EQ(pSys1, Shape6.GetCoordSys());

    Shape6.ChangeCoordSys(pWorld);
    ASSERT_EQ(pWorld, Shape6.GetCoordSys());

    }

//==================================================================================
// GetExtent() const
//==================================================================================
TEST_F (HVEShapeTester, GetExtentTest)
    {

    ASSERT_NEAR(0.0, Shape3.GetExtent().GetXMin(), MYEPSILON);
    ASSERT_NEAR(0.0, Shape3.GetExtent().GetYMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Shape3.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(10.0, Shape3.GetExtent().GetYMax());

    ASSERT_NEAR(0.0, Shape4.GetExtent().GetXMin(), MYEPSILON);
    ASSERT_NEAR(0.0, Shape4.GetExtent().GetYMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Shape4.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(10.0, Shape4.GetExtent().GetYMax());

    ASSERT_NEAR(0.0, Shape7.GetExtent().GetXMin(), MYEPSILON);
    ASSERT_NEAR(0.0, Shape7.GetExtent().GetYMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(5.0, Shape7.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(5.0, Shape7.GetExtent().GetYMax());

    }

//==================================================================================
// GetShapePtr() const
//==================================================================================
TEST_F (HVEShapeTester, GetShapePtrTest)
    {

    ASSERT_TRUE(Shape.GetShapePtr()->IsEmpty());
    ASSERT_TRUE(Shape1.GetShapePtr()->IsEmpty());
    ASSERT_FALSE(Shape3.GetShapePtr()->IsEmpty());
    ASSERT_FALSE(Shape4.GetShapePtr()->IsEmpty());
    ASSERT_FALSE(Shape6.GetShapePtr()->IsEmpty());
    ASSERT_FALSE(Shape7.GetShapePtr()->IsEmpty());

    }

//==================================================================================
// IsEmpty() test
// MakEmpty() test
//==================================================================================
TEST_F (HVEShapeTester, EmptyTest)
    {

    ASSERT_FALSE(Shape3.IsEmpty());
    Shape3.MakeEmpty();
    ASSERT_TRUE(Shape3.IsEmpty());

    }

//==================================================================================
// IsPointIn()
// IsPointOn()
//==================================================================================
TEST_F (HVEShapeTester, IsPointTest)
    {

    ASSERT_TRUE(Shape4.IsPointIn(HGF2DLocation(1.0, 1.0, pWorld)));
    ASSERT_TRUE(Shape4.IsPointIn(HGF2DLocation(2.0, 2.0, pWorld)));
    ASSERT_TRUE(Shape4.IsPointIn(HGF2DLocation(3.0, 3.0, pWorld)));
    ASSERT_TRUE(Shape4.IsPointIn(HGF2DLocation(4.0, 4.0, pWorld)));

    ASSERT_TRUE(Shape4.IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(Shape4.IsPointOn(HGF2DLocation(0.0, 10.0, pWorld)));
    ASSERT_TRUE(Shape4.IsPointOn(HGF2DLocation(10.0, 0.0, pWorld)));
    ASSERT_TRUE(Shape4.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));

    }

//==================================================================================
// IsRectangle() const
//==================================================================================
TEST_F (HVEShapeTester, IsRectangleTest)
    {
    
    ASSERT_TRUE(Shape3.IsRectangle());
    ASSERT_TRUE(Shape4.IsRectangle());
    ASSERT_FALSE(Shape6.IsRectangle());
    ASSERT_TRUE(Shape7.IsRectangle());

    }

//==================================================================================
// Matches( const HVEShape& pi_rObj ) const
//==================================================================================
TEST_F (HVEShapeTester, MatchTest)
    { 	

    // With himself
    ASSERT_TRUE(Shape3.Matches(Shape3));

    //With another who match
    ASSERT_TRUE(Shape3.Matches(Shape4));

    //With another who doesnt match
    ASSERT_FALSE(Shape3.Matches(Shape7));

    }

//==================================================================================
// Differentiate( const HVEShape& pi_rObj )
//==================================================================================
TEST_F (HVEShapeTester, DifferentiateTest)
    {

    //Clone of Shape
    HVEShape CloneShape3(Shape3);
    HVEShape Clone2Shape3(Shape3);
    HVEShape CloneShape4(Shape4);
    HVEShape Clone2Shape4(Shape4);

    // With himself
    CloneShape3.Differentiate(Shape3);
    ASSERT_TRUE(CloneShape3.IsEmpty());

    // With equivalent
    Clone2Shape3.Differentiate(Shape4);
    ASSERT_TRUE(Clone2Shape3.IsEmpty());

    // With another who intersect
    CloneShape4.Differentiate(Shape7);
    ASSERT_FALSE(CloneShape4.IsEmpty());

    ASSERT_TRUE(CloneShape4.IsPointOn(HGF2DLocation(0.0, 5.0, pWorld)));
    ASSERT_TRUE(CloneShape4.IsPointOn(HGF2DLocation(0.0, 10.0, pWorld)));
    ASSERT_TRUE(CloneShape4.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(CloneShape4.IsPointOn(HGF2DLocation(0.0, 10.0, pWorld)));
    ASSERT_TRUE(CloneShape4.IsPointOn(HGF2DLocation(5.0, 0.0, pWorld)));
    ASSERT_TRUE(CloneShape4.IsPointOn(HGF2DLocation(5.0, 5.0, pWorld)));

    // With another who doesnt intersect
    Clone2Shape4.Differentiate(Shape8);
    ASSERT_FALSE(Clone2Shape4.IsEmpty());

    ASSERT_TRUE(Clone2Shape4.IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(Clone2Shape4.IsPointOn(HGF2DLocation(0.0, 10.0, pWorld)));
    ASSERT_TRUE(Clone2Shape4.IsPointOn(HGF2DLocation(10.0, 0.0, pWorld)));
    ASSERT_TRUE(Clone2Shape4.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));

    }

//==================================================================================
// Intersect( const HVEShape& pi_rObj )
//==================================================================================
TEST_F (HVEShapeTester, IntersectTest)
    {

    //Clone of Shape
    HVEShape CloneShape3(Shape3);
    HVEShape Clone2Shape3(Shape3);
    HVEShape CloneShape4(Shape4);
    HVEShape Clone2Shape4(Shape4);

    // With himself
    CloneShape3.Intersect(Shape3);
    ASSERT_FALSE(CloneShape3.IsEmpty());

    ASSERT_TRUE(CloneShape3.IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(CloneShape3.IsPointOn(HGF2DLocation(0.0, 10.0, pWorld)));
    ASSERT_TRUE(CloneShape3.IsPointOn(HGF2DLocation(10.0, 0.0, pWorld)));
    ASSERT_TRUE(CloneShape3.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));

    // With equivalent
    Clone2Shape3.Intersect(Shape4);
    ASSERT_FALSE(Clone2Shape3.IsEmpty());

    ASSERT_TRUE(Clone2Shape3.IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(Clone2Shape3.IsPointOn(HGF2DLocation(0.0, 10.0, pWorld)));
    ASSERT_TRUE(Clone2Shape3.IsPointOn(HGF2DLocation(10.0, 0.0, pWorld)));
    ASSERT_TRUE(Clone2Shape3.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));

    // With another who intersect
    CloneShape4.Intersect(Shape7);
    ASSERT_FALSE(Shape4.IsEmpty());

    ASSERT_TRUE(CloneShape4.IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(CloneShape4.IsPointOn(HGF2DLocation(0.0, 5.0, pWorld)));
    ASSERT_TRUE(CloneShape4.IsPointOn(HGF2DLocation(5.0, 5.0, pWorld)));
    ASSERT_TRUE(CloneShape4.IsPointOn(HGF2DLocation(5.0, 0.0, pWorld)));

    // With another who doesnt intersect
    Clone2Shape4.Intersect(Shape8);
    ASSERT_TRUE(Clone2Shape4.IsEmpty());

    }

//==================================================================================
// Unify( const HVEShape& pi_rObj )
//==================================================================================
TEST_F (HVEShapeTester, UnifyTest)
    {

    //Clone of Shape
    HVEShape CloneShape3(Shape3);
    HVEShape Clone2Shape3(Shape3);
    HVEShape CloneShape4(Shape4);
    HVEShape Clone2Shape4(Shape4);

    // With himself
    CloneShape3.Unify(Shape3);
    ASSERT_FALSE(CloneShape3.IsEmpty());

    ASSERT_TRUE(CloneShape3.IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(CloneShape3.IsPointOn(HGF2DLocation(0.0, 10.0, pWorld)));
    ASSERT_TRUE(CloneShape3.IsPointOn(HGF2DLocation(10.0, 0.0, pWorld)));
    ASSERT_TRUE(CloneShape3.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));

    // With equivalent
    Clone2Shape3.Unify(Shape4);
    ASSERT_FALSE(Clone2Shape3.IsEmpty());

    ASSERT_TRUE(Clone2Shape3.IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(Clone2Shape3.IsPointOn(HGF2DLocation(0.0, 10.0, pWorld)));
    ASSERT_TRUE(Clone2Shape3.IsPointOn(HGF2DLocation(10.0, 0.0, pWorld)));
    ASSERT_TRUE(Clone2Shape3.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));

    // With another who intersect
    CloneShape4.Unify(Shape7);
    ASSERT_FALSE(Shape4.IsEmpty());

    ASSERT_TRUE(Clone2Shape3.IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(Clone2Shape3.IsPointOn(HGF2DLocation(0.0, 10.0, pWorld)));
    ASSERT_TRUE(Clone2Shape3.IsPointOn(HGF2DLocation(10.0, 0.0, pWorld)));
    ASSERT_TRUE(Clone2Shape3.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));

    // With another who doesnt intersect
    Clone2Shape4.Unify(Shape8);
    ASSERT_FALSE(Clone2Shape4.IsEmpty());

    ASSERT_TRUE(Clone2Shape4.IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(Clone2Shape4.IsPointOn(HGF2DLocation(0.0, 10.0, pWorld)));
    ASSERT_TRUE(Clone2Shape4.IsPointOn(HGF2DLocation(10.0, 0.0, pWorld)));
    ASSERT_TRUE(Clone2Shape4.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(Clone2Shape4.IsPointOn(HGF2DLocation(-5.0, 0.0, pWorld)));
    ASSERT_TRUE(Clone2Shape4.IsPointOn(HGF2DLocation(0.0, -5.0, pWorld)));
    ASSERT_TRUE(Clone2Shape4.IsPointOn(HGF2DLocation(-5.0, -5.0, pWorld)));

    }

//==================================================================================
// Rotate( const HGFAngle& pi_rAngle, const HGF2DLocation& pi_rOrigin )
//==================================================================================
TEST_F (HVEShapeTester, RotateTest)
    {

    Shape3.Rotate(PI, HGF2DLocation(0.0, 0.0, pWorld));

    ASSERT_TRUE(Shape3.IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(Shape3.IsPointOn(HGF2DLocation(0.0, -10.0, pWorld)));
    ASSERT_TRUE(Shape3.IsPointOn(HGF2DLocation(-10.0, 0.0, pWorld)));
    ASSERT_TRUE(Shape3.IsPointOn(HGF2DLocation(-10.0, -10.0, pWorld)));

    }

//==================================================================================
// Scale( HDOUBLE pi_ScaleFactor, const HGF2DLocation& pi_rOrigin )
//==================================================================================
TEST_F (HVEShapeTester, ScaleTest)
    {

    Shape3.Scale(2.0, HGF2DLocation(0.0, 0.0, pWorld));

    ASSERT_TRUE(Shape3.IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(Shape3.IsPointOn(HGF2DLocation(0.0, 20.0, pWorld)));
    ASSERT_TRUE(Shape3.IsPointOn(HGF2DLocation(20.0, 0.0, pWorld)));
    ASSERT_TRUE(Shape3.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    Shape4.Scale(2.0, HGF2DLocation(1.0, 1.0, pWorld));

    ASSERT_TRUE(Shape4.IsPointOn(HGF2DLocation(-1.0, -1.0, pWorld)));
    ASSERT_TRUE(Shape4.IsPointOn(HGF2DLocation(-1.0, 19.0, pWorld)));
    ASSERT_TRUE(Shape4.IsPointOn(HGF2DLocation(19.0, -1.0, pWorld)));
    ASSERT_TRUE(Shape4.IsPointOn(HGF2DLocation(19.0, 19.0, pWorld)));

    HVEShape Shape9(10.0, 10.0, 20.0, 20.0, pWorld);
    
    Shape9.Scale(2.0, HGF2DLocation(0.0, 0.0, pWorld));

    ASSERT_TRUE(Shape9.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(Shape9.IsPointOn(HGF2DLocation(20.0, 40.0, pWorld)));
    ASSERT_TRUE(Shape9.IsPointOn(HGF2DLocation(40.0, 20.0, pWorld)));
    ASSERT_TRUE(Shape9.IsPointOn(HGF2DLocation(40.0, 40.0, pWorld)));

    Shape9.Scale(2.0, HGF2DLocation(5.0, 10.0, pWorld));

    ASSERT_TRUE(Shape9.IsPointOn(HGF2DLocation(35.0, 30.0, pWorld)));
    ASSERT_TRUE(Shape9.IsPointOn(HGF2DLocation(35.0, 70.0, pWorld)));
    ASSERT_TRUE(Shape9.IsPointOn(HGF2DLocation(75.0, 30.0, pWorld)));
    ASSERT_TRUE(Shape9.IsPointOn(HGF2DLocation(35.0, 70.0, pWorld)));

    }