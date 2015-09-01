//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HVE2DUniverseTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "EnvironnementTest.h"
#include "HVE2DUniverseTester.h"

#define DOUBLE_MAX (1.7976931348623157e+308)


// THE PRESENT TEST CANNOT MAKE TESTS WITH NON-LINEAR TRANSFORMATION MODELS

HVE2DUniverseTester::HVE2DUniverseTester() 
    {
    
    // Universe
    Universe1 = HVE2DUniverse(pWorld);
    Universe2 = HVE2DUniverse(pSys1);

    // Rectangle
    Rectangle1 = HVE2DRectangle(0.0, 0.0, 10.0, 10.0, pWorld);
    Rectangle2 = HVE2DRectangle(0.0, 0.0, 10.0, 10.0, pSys1);

    // Location
    Origin1 = HGF2DLocation(0.0, 0.0, pWorld);
    Origin2 = HGF2DLocation(0.0, 0.0, pSys1);

    }

//==================================================================================
// Universe Construction tests
//==================================================================================
TEST_F(HVE2DUniverseTester, ConstructorsTest)
    {

    // Default Universe
    HVE2DUniverse DefaultUniverse;

    // With CoordSys
    HVE2DUniverse Universe(pWorld);
    ASSERT_EQ(pWorld, Universe.GetCoordSys());

    // Copy Constructor
    HVE2DUniverse Universe2(Universe);
    ASSERT_EQ(pWorld, Universe2.GetCoordSys());

    }

//==================================================================================
// operator=(const HVE2DPolygon& pi_rObj);
//==================================================================================
TEST_F (HVE2DUniverseTester, OperatorTest)
    {
    
    HVE2DUniverse NewUniverse = Universe2;

    ASSERT_EQ(pSys1, NewUniverse.GetCoordSys());

    }

//==================================================================================
// AllocateCopyInCoordSys( const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys ) const
// Clone() const
//==================================================================================
TEST_F (HVE2DUniverseTester, AllocateCopyTest)
    {
    
    HFCPtr<HVE2DUniverse> NewUniverse1 = (HVE2DUniverse*) Universe1.AllocateCopyInCoordSys(pSys1);
    ASSERT_EQ(pSys1, NewUniverse1->GetCoordSys());

    HFCPtr<HVE2DUniverse> NewUniverse2 = (HVE2DUniverse*) Universe1.Clone();
    ASSERT_EQ(pWorld, NewUniverse2->GetCoordSys());

    }

//==================================================================================
// AllocateLinear( HVE2DSimpleShape::RotationDirection pi_DirectionDesired ) const
// GetLinear( HVE2DSimpleShape::RotationDirection pi_DirectionDesired ) const
//==================================================================================
TEST_F (HVE2DUniverseTester, LinearTest)
    {
  
    HFCPtr<HVE2DComplexLinear> NewComplexLinear = Universe1.AllocateLinear(HVE2DSimpleShape::CW);

    ASSERT_FALSE(NewComplexLinear->IsEmpty());
    ASSERT_EQ(4, NewComplexLinear->GetNumberOfLinears());
    ASSERT_DOUBLE_EQ(-DOUBLE_MAX, NewComplexLinear->GetStartPoint().GetX()); 
    ASSERT_DOUBLE_EQ(-DOUBLE_MAX, NewComplexLinear->GetStartPoint().GetY()); 
   
    HVE2DComplexLinear Linear1 = Universe1.GetLinear(HVE2DSimpleShape::CW);

    ASSERT_FALSE(Linear1.IsEmpty());
    ASSERT_EQ(4, Linear1.GetNumberOfLinears());   
    ASSERT_DOUBLE_EQ(-DOUBLE_MAX, Linear1.GetStartPoint().GetX()); 
    ASSERT_DOUBLE_EQ(-DOUBLE_MAX, Linear1.GetStartPoint().GetY());      

    }

//==================================================================================
// AreAdjacent( const HVE2DVector& pi_rVector ) const
//==================================================================================
TEST_F (HVE2DUniverseTester, AreAdjacentTest)
    {

    ASSERT_FALSE(Universe1.AreAdjacent(Rectangle1));
    ASSERT_FALSE(Universe2.AreAdjacent(Rectangle2));
    
    }

//==================================================================================
// AreContiguous( const HVE2DVector& pi_rVector ) const
// AreContiguousAt( const HVE2DVector& pi_rVector, const HGF2DLocation& pi_rPoint ) const
//==================================================================================
TEST_F (HVE2DUniverseTester, AreContiguousTest)
    {
    
    ASSERT_FALSE(Universe1.AreContiguous(Rectangle1));
    ASSERT_FALSE(Universe2.AreContiguous(Rectangle2));

    ASSERT_FALSE(Universe1.AreContiguousAt(Rectangle1, Origin1));
    ASSERT_FALSE(Universe2.AreContiguousAt(Rectangle2, Origin2));
    
    }

//==================================================================================
// CalculateArea() const
//==================================================================================
TEST_F (HVE2DUniverseTester, CalculateAreaTest)
    {
    
    ASSERT_DOUBLE_EQ(numeric_limits<double>::infinity(), Universe1.CalculateArea());
    ASSERT_DOUBLE_EQ(numeric_limits<double>::infinity(), Universe2.CalculateArea());
    
    }


//==================================================================================
// CalculateClosestPoint( const HGF2DLocation& pi_rPoint ) const
//==================================================================================
TEST_F (HVE2DUniverseTester, CalculateClosestPointTest)
    {
   
    ASSERT_DOUBLE_EQ(DOUBLE_MAX, Universe1.CalculateClosestPoint(Origin1).GetX());
    ASSERT_DOUBLE_EQ(DOUBLE_MAX, Universe1.CalculateClosestPoint(Origin1).GetY());

    ASSERT_DOUBLE_EQ(DOUBLE_MAX, Universe2.CalculateClosestPoint(Origin2).GetX());
    ASSERT_DOUBLE_EQ(DOUBLE_MAX, Universe2.CalculateClosestPoint(Origin2).GetY());
   
    }
    	
//==================================================================================
// CalculatePerimeter() const
//==================================================================================
TEST_F (HVE2DUniverseTester, CalculatePerimeterTest)
    {
    
    ASSERT_DOUBLE_EQ(numeric_limits<double>::infinity(), Universe1.CalculatePerimeter());
    ASSERT_DOUBLE_EQ(numeric_limits<double>::infinity(), Universe2.CalculatePerimeter());
    
    }

//==================================================================================
// CalculateSpatialPositionOf( const HGF2DLocation& pi_rPoint ) const
// CalculateSpatialPositionOf( const HVE2DVector& pi_rVector ) const
//==================================================================================
TEST_F (HVE2DUniverseTester, CalculateSpatialPositionOfTest)
    {
    
    ASSERT_EQ(HVE2DShape::S_IN, Universe1.CalculateSpatialPositionOf(Origin1));
    ASSERT_EQ(HVE2DShape::S_IN, Universe2.CalculateSpatialPositionOf(Origin2));

    ASSERT_EQ(HVE2DShape::S_ON, Universe1.CalculateSpatialPositionOf(Universe2));
    ASSERT_EQ(HVE2DShape::S_ON, Universe2.CalculateSpatialPositionOf(Universe1));
    
    ASSERT_EQ(HVE2DShape::S_IN, Universe1.CalculateSpatialPositionOf(Rectangle1));
    ASSERT_EQ(HVE2DShape::S_IN, Universe2.CalculateSpatialPositionOf(Rectangle2));
    
    }

//==================================================================================
// Crosses( const HVE2DVector& pi_rVector ) const
//==================================================================================
TEST_F (HVE2DUniverseTester, CrossesTest)
    {

    ASSERT_FALSE(Universe1.Crosses(Rectangle1));
    ASSERT_FALSE(Universe2.Crosses(Rectangle2));
    
    }

//==================================================================================
// DifferentiateFromShape( const HVE2DShape& pi_rShape ) const
// DifferentiateFromShapeSCS( const HVE2DShape& pi_rShape ) const
// DifferentiateShapeSCS( const HVE2DShape& pi_rShape ) const
//==================================================================================
TEST_F (HVE2DUniverseTester, DifferentiateTest)
    {

    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, Universe1.DifferentiateFromShape(Rectangle1)->GetClassID());
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, Universe2.DifferentiateFromShape(Rectangle2)->GetClassID());

    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, Universe1.DifferentiateFromShapeSCS(Rectangle1)->GetClassID());
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, Universe2.DifferentiateFromShapeSCS(Rectangle2)->GetClassID());

    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, Universe1.DifferentiateShapeSCS(Universe1)->GetClassID());
    ASSERT_EQ(HVE2DVoidShape::CLASS_ID, Universe2.DifferentiateShapeSCS(Universe2)->GetClassID());

    ASSERT_EQ(HVE2DHoledShape::CLASS_ID, Universe1.DifferentiateShapeSCS(Rectangle1)->GetClassID());
    ASSERT_EQ(HVE2DHoledShape::CLASS_ID, Universe2.DifferentiateShapeSCS(Rectangle2)->GetClassID());
    
    }

//==================================================================================
// GetExtent() const
//==================================================================================
TEST_F (HVE2DUniverseTester, GetExtentTest)
    {

    ASSERT_DOUBLE_EQ(-DOUBLE_MAX, Universe1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(-DOUBLE_MAX, Universe1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(DOUBLE_MAX, Universe1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(DOUBLE_MAX, Universe1.GetExtent().GetYMax());

    }

//==================================================================================
// GetShapeType() const
//==================================================================================
TEST_F (HVE2DUniverseTester, GetShapeTypeTest)
    {

    ASSERT_EQ(HVE2DUniverse::CLASS_ID, Universe1.GetShapeType());
    ASSERT_EQ(HVE2DUniverse::CLASS_ID, Universe2.GetShapeType());

    }

//==================================================================================
// IntersectShape( const HVE2DShape& pi_rShape ) const
// IntersectShapeSCS( const HVE2DShape& pi_rShape ) const
//==================================================================================
TEST_F (HVE2DUniverseTester, IntersectShapeTest)
    {

    HFCPtr<HVE2DRectangle> NewRectangle1 = (HVE2DRectangle*) Universe1.IntersectShape(Rectangle1);

    ASSERT_TRUE(NewRectangle1->IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(NewRectangle1->IsPointOn(HGF2DLocation(0.0, 10.0, pWorld)));
    ASSERT_TRUE(NewRectangle1->IsPointOn(HGF2DLocation(10.0, 0.0, pWorld)));
    ASSERT_TRUE(NewRectangle1->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));

    HFCPtr<HVE2DRectangle> NewRectangle2 = (HVE2DRectangle*) Universe2.IntersectShape(Rectangle1);

    ASSERT_TRUE(NewRectangle2->IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(NewRectangle2->IsPointOn(HGF2DLocation(0.0, 10.0, pWorld)));
    ASSERT_TRUE(NewRectangle2->IsPointOn(HGF2DLocation(10.0, 0.0, pWorld)));
    ASSERT_TRUE(NewRectangle2->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
 
    HFCPtr<HVE2DRectangle> NewRectangle3 = (HVE2DRectangle*) Universe1.IntersectShape(Rectangle1);

    ASSERT_TRUE(NewRectangle3->IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(NewRectangle3->IsPointOn(HGF2DLocation(0.0, 10.0, pWorld)));
    ASSERT_TRUE(NewRectangle3->IsPointOn(HGF2DLocation(10.0, 0.0, pWorld)));
    ASSERT_TRUE(NewRectangle3->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));

    HFCPtr<HVE2DRectangle> NewRectangle4 = (HVE2DRectangle*) Universe2.IntersectShape(Rectangle2);

    ASSERT_TRUE(NewRectangle4->IsPointOn(HGF2DLocation(0.0, 0.0, pSys1)));
    ASSERT_TRUE(NewRectangle4->IsPointOn(HGF2DLocation(0.0, 10.0, pSys1)));
    ASSERT_TRUE(NewRectangle4->IsPointOn(HGF2DLocation(10.0, 0.0, pSys1)));
    ASSERT_TRUE(NewRectangle4->IsPointOn(HGF2DLocation(10.0, 10.0, pSys1)));

    }

//==================================================================================
// IsPointIn( const HGF2DLocation& pi_rPoint, HDOUBLE pi_Tolerance ) const
// IsPointOn( const HGF2DLocation& pi_rPoint, HVE2DVector::ExtremityProcessing pi_ExtremityProcessing, HDOUBLE pi_Tolerance ) const
// IsPointOnSCS( const HGF2DLocation& pi_rPoint, HVE2DVector::ExtremityProcessing pi_ExtremityProcessing, HDOUBLE pi_Tolerance ) const
//==================================================================================
TEST_F (HVE2DUniverseTester, IsPointTest)
    {

    ASSERT_TRUE(Universe1.IsPointIn(Origin1));
    ASSERT_TRUE(Universe2.IsPointIn(Origin2));

    ASSERT_FALSE(Universe1.IsPointOn(Origin1));
    ASSERT_FALSE(Universe2.IsPointOn(Origin2));

    ASSERT_FALSE(Universe1.IsPointOnSCS(Origin1));
    ASSERT_FALSE(Universe2.IsPointOnSCS(Origin2));

    }

//==================================================================================
// MakeEmpty()
// IsEmpty()
//==================================================================================
TEST_F (HVE2DUniverseTester, EmptyTest)
    {
    
    Universe1.MakeEmpty();
    Universe2.MakeEmpty();

    ASSERT_FALSE(Universe1.IsEmpty());
    ASSERT_FALSE(Universe2.IsEmpty());

    }

//==================================================================================
// UnifyShape( const HVE2DShape& pi_rShape ) const
// UnifyShapeSCS( const HVE2DShape& pi_rShape ) const
//==================================================================================
TEST_F (HVE2DUniverseTester, UnifyShapeTest)
    {

    ASSERT_EQ(HVE2DUniverse::CLASS_ID, Universe1.UnifyShape(Rectangle1)->GetClassID());
    ASSERT_EQ(HVE2DUniverse::CLASS_ID, Universe2.UnifyShape(Rectangle2)->GetClassID());

    ASSERT_EQ(HVE2DUniverse::CLASS_ID, Universe1.UnifyShapeSCS(Rectangle1)->GetClassID());
    ASSERT_EQ(HVE2DUniverse::CLASS_ID, Universe2.UnifyShapeSCS(Rectangle2)->GetClassID());

    }