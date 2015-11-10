//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DUniverseTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "EnvironnementTest.h"
#include "HGF2DUniverseTester.h"

#define DOUBLE_MAX (1.7976931348623157e+308)


// THE PRESENT TEST CANNOT MAKE TESTS WITH NON-LINEAR TRANSFORMATION MODELS

HGF2DUniverseTester::HGF2DUniverseTester() 
    {
    
    // Universe
    Universe1 = HGF2DUniverse();
    Universe2 = HGF2DUniverse();

    // Rectangle
    Rectangle1 = HGF2DRectangle(0.0, 0.0, 10.0, 10.0);
    Rectangle2 = HGF2DRectangle(0.0, 0.0, 10.0, 10.0);

    // Location
    Origin1 = HGF2DPosition(0.0, 0.0);
    Origin2 = HGF2DPosition(0.0, 0.0);

    }

//==================================================================================
// Universe Construction tests
//==================================================================================
TEST_F(HGF2DUniverseTester, ConstructorsTest)
    {

    // Default Universe
    HGF2DUniverse DefaultUniverse;

    // With CoordSys
    HGF2DUniverse Universe;

    // Copy Constructor
    HGF2DUniverse Universe2;

    }

//==================================================================================
// operator=(const HGF2DPolygon& pi_rObj);
//==================================================================================
TEST_F (HGF2DUniverseTester, OperatorTest)
    {
    
    HGF2DUniverse NewUniverse = Universe2;

    }

//==================================================================================
// AllocateCopyInCoordSys( const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys ) const
// Clone() const
//==================================================================================
TEST_F (HGF2DUniverseTester, AllocateCopyTest)
    {
    
    HFCPtr<HGF2DUniverse> NewUniverse1 = (HGF2DUniverse*) (&*(Universe1.AllocTransformDirect(*(pSys1->GetTransfoModelTo(pWorld)))));

    HFCPtr<HGF2DUniverse> NewUniverse2 = (HGF2DUniverse*) &*(Universe1.Clone());

    }

//==================================================================================
// GetLinear( HGF2DSimpleShape::RotationDirection pi_DirectionDesired ) const
//==================================================================================
TEST_F (HGF2DUniverseTester, LinearTest)
    {

   
    HFCPtr<HGF2DPolySegment> Linear1 = static_cast<HGF2DPolySegment*>(&*(Universe1.GetLinear(HGF2DSimpleShape::CW)));

    ASSERT_EQ(4, Linear1->GetSize());   
    ASSERT_DOUBLE_EQ(-DOUBLE_MAX, Linear1->GetStartPoint().GetX()); 
    ASSERT_DOUBLE_EQ(-DOUBLE_MAX, Linear1->GetStartPoint().GetY());      

    }

//==================================================================================
// AreAdjacent( const HGF2DVector& pi_rVector ) const
//==================================================================================
TEST_F (HGF2DUniverseTester, AreAdjacentTest)
    {

    ASSERT_FALSE(Universe1.AreAdjacent(Rectangle1));
    ASSERT_FALSE(Universe2.AreAdjacent(Rectangle2));
    
    }

//==================================================================================
// AreContiguous( const HGF2DVector& pi_rVector ) const
// AreContiguousAt( const HGF2DVector& pi_rVector, const HGF2DPosition& pi_rPoint ) const
//==================================================================================
TEST_F (HGF2DUniverseTester, AreContiguousTest)
    {
    
    ASSERT_FALSE(Universe1.AreContiguous(Rectangle1));
    ASSERT_FALSE(Universe2.AreContiguous(Rectangle2));

    ASSERT_FALSE(Universe1.AreContiguousAt(Rectangle1, Origin1));
    ASSERT_FALSE(Universe2.AreContiguousAt(Rectangle2, Origin2));
    
    }

//==================================================================================
// CalculateArea() const
//==================================================================================
TEST_F (HGF2DUniverseTester, CalculateAreaTest)
    {
    
    ASSERT_DOUBLE_EQ(numeric_limits<double>::infinity(), Universe1.CalculateArea());
    ASSERT_DOUBLE_EQ(numeric_limits<double>::infinity(), Universe2.CalculateArea());
    
    }


//==================================================================================
// CalculateClosestPoint( const HGF2DPosition& pi_rPoint ) const
//==================================================================================
TEST_F (HGF2DUniverseTester, CalculateClosestPointTest)
    {
   
    ASSERT_DOUBLE_EQ(DOUBLE_MAX, Universe1.CalculateClosestPoint(Origin1).GetX());
    ASSERT_DOUBLE_EQ(DOUBLE_MAX, Universe1.CalculateClosestPoint(Origin1).GetY());

    ASSERT_DOUBLE_EQ(DOUBLE_MAX, Universe2.CalculateClosestPoint(Origin2).GetX());
    ASSERT_DOUBLE_EQ(DOUBLE_MAX, Universe2.CalculateClosestPoint(Origin2).GetY());
   
    }
    	
//==================================================================================
// CalculatePerimeter() const
//==================================================================================
TEST_F (HGF2DUniverseTester, CalculatePerimeterTest)
    {
    
    ASSERT_DOUBLE_EQ(numeric_limits<double>::infinity(), Universe1.CalculatePerimeter());
    ASSERT_DOUBLE_EQ(numeric_limits<double>::infinity(), Universe2.CalculatePerimeter());
    
    }

//==================================================================================
// CalculateSpatialPositionOf( const HGF2DPosition& pi_rPoint ) const
// CalculateSpatialPositionOf( const HGF2DVector& pi_rVector ) const
//==================================================================================
TEST_F (HGF2DUniverseTester, CalculateSpatialPositionOfTest)
    {
    
    ASSERT_EQ(HGF2DShape::S_IN, Universe1.CalculateSpatialPositionOf(Origin1));
    ASSERT_EQ(HGF2DShape::S_IN, Universe2.CalculateSpatialPositionOf(Origin2));

    ASSERT_EQ(HGF2DShape::S_ON, Universe1.CalculateSpatialPositionOf(Universe2));
    ASSERT_EQ(HGF2DShape::S_ON, Universe2.CalculateSpatialPositionOf(Universe1));
    
    ASSERT_EQ(HGF2DShape::S_IN, Universe1.CalculateSpatialPositionOf(Rectangle1));
    ASSERT_EQ(HGF2DShape::S_IN, Universe2.CalculateSpatialPositionOf(Rectangle2));
    
    }

//==================================================================================
// Crosses( const HGF2DVector& pi_rVector ) const
//==================================================================================
TEST_F (HGF2DUniverseTester, CrossesTest)
    {

    ASSERT_FALSE(Universe1.Crosses(Rectangle1));
    ASSERT_FALSE(Universe2.Crosses(Rectangle2));
    
    }

//==================================================================================
// DifferentiateFromShape( const HGF2DShape& pi_rShape ) const
// DifferentiateShape( const HGF2DShape& pi_rShape ) const
//==================================================================================
TEST_F (HGF2DUniverseTester, DifferentiateTest)
    {

    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, Universe1.DifferentiateFromShape(Rectangle1)->GetClassID());
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, Universe2.DifferentiateFromShape(Rectangle2)->GetClassID());

    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, Universe1.DifferentiateShape(Universe1)->GetClassID());
    ASSERT_EQ(HGF2DVoidShape::CLASS_ID, Universe2.DifferentiateShape(Universe2)->GetClassID());

    
    }

//==================================================================================
// GetExtent() const
//==================================================================================
TEST_F (HGF2DUniverseTester, GetExtentTest)
    {

    ASSERT_DOUBLE_EQ(-DOUBLE_MAX, Universe1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(-DOUBLE_MAX, Universe1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(DOUBLE_MAX, Universe1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(DOUBLE_MAX, Universe1.GetExtent().GetYMax());

    }

//==================================================================================
// GetShapeType() const
//==================================================================================
TEST_F (HGF2DUniverseTester, GetShapeTypeTest)
    {

    ASSERT_EQ(HGF2DUniverse::CLASS_ID, Universe1.GetShapeType());
    ASSERT_EQ(HGF2DUniverse::CLASS_ID, Universe2.GetShapeType());

    }

//==================================================================================
// IntersectShape( const HGF2DShape& pi_rShape ) const
//==================================================================================
TEST_F (HGF2DUniverseTester, IntersectShapeTest)
    {

    HFCPtr<HGF2DRectangle> NewRectangle1 = (HGF2DRectangle*) Universe1.IntersectShape(Rectangle1);

    ASSERT_TRUE(NewRectangle1->IsPointOn(HGF2DPosition(0.0, 0.0)));
    ASSERT_TRUE(NewRectangle1->IsPointOn(HGF2DPosition(0.0, 10.0)));
    ASSERT_TRUE(NewRectangle1->IsPointOn(HGF2DPosition(10.0, 0.0)));
    ASSERT_TRUE(NewRectangle1->IsPointOn(HGF2DPosition(10.0, 10.0)));

    HFCPtr<HGF2DRectangle> NewRectangle2 = (HGF2DRectangle*) Universe2.IntersectShape(Rectangle1);

    ASSERT_TRUE(NewRectangle2->IsPointOn(HGF2DPosition(0.0, 0.0)));
    ASSERT_TRUE(NewRectangle2->IsPointOn(HGF2DPosition(0.0, 10.0)));
    ASSERT_TRUE(NewRectangle2->IsPointOn(HGF2DPosition(10.0, 0.0)));
    ASSERT_TRUE(NewRectangle2->IsPointOn(HGF2DPosition(10.0, 10.0)));


    }

//==================================================================================
// IsPointIn( const HGF2DPosition& pi_rPoint, HDOUBLE pi_Tolerance ) const
//==================================================================================
TEST_F (HGF2DUniverseTester, IsPointTest)
    {

    ASSERT_TRUE(Universe1.IsPointIn(Origin1));
    ASSERT_TRUE(Universe2.IsPointIn(Origin2));

    }

//==================================================================================
// MakeEmpty()
// IsEmpty()
//==================================================================================
TEST_F (HGF2DUniverseTester, EmptyTest)
    {
    
    Universe1.MakeEmpty();
    Universe2.MakeEmpty();

    ASSERT_FALSE(Universe1.IsEmpty());
    ASSERT_FALSE(Universe2.IsEmpty());

    }

//==================================================================================
// UnifyShape( const HGF2DShape& pi_rShape ) const
//==================================================================================
TEST_F (HGF2DUniverseTester, UnifyShapeTest)
    {

    ASSERT_EQ(HGF2DUniverse::CLASS_ID, Universe1.UnifyShape(Rectangle1)->GetClassID());
    ASSERT_EQ(HGF2DUniverse::CLASS_ID, Universe2.UnifyShape(Rectangle2)->GetClassID());


    }