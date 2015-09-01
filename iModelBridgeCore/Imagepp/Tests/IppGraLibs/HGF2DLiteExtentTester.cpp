//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DLiteExtentTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "HGF2DLiteExtentTester.h"

HGF2DLiteExtentTester::HGF2DLiteExtentTester() 
    {

    //General
    Translation = HGF2DDisplacement(10.0, 30.48);
    Rotation = PI/4;
    ScalingX = 1.00001;
    ScalingY = 2.00001;
    Anortho = -0.000001;
    Affine1 = HGF2DAffine(Translation, Rotation, ScalingX, ScalingY, Anortho);

    pWorld = new HGF2DCoordSys();
    pSys1 = new HGF2DCoordSys(Affine1, pWorld);

    //Extent
    Extent1 = HGF2DLiteExtent(HGF2DPosition(0.0, 0.0), HGF2DPosition(10.0, 10.0));
    Extent2 = HGF2DLiteExtent(HGF2DPosition(0.0, 0.0), HGF2DPosition(20.0, 20.0));
    Extent3 = HGF2DLiteExtent(HGF2DPosition(0.0, 0.0), HGF2DPosition(10.0, 20.0));

    }

// THE PRESENT TEST CANNOT MAKE TESTS WITH NON-LINEAR TRANSFORMATION MODELS

//==================================================================================
// Construction tests
//==================================================================================
TEST_F (HGF2DLiteExtentTester, ConstructorsTest)
    {

    //Default constructor
    HGF2DLiteExtent defaultExtent;
    ASSERT_FALSE(defaultExtent.IsDefined());

    //With Position
    HGF2DLiteExtent Extent2(HGF2DPosition(0.0, 0.0), HGF2DPosition(10.0, 10.0));
    ASSERT_TRUE(Extent2.IsDefined());

    ASSERT_NEAR(0.0, Extent2.GetXMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Extent2.GetXMax());
    ASSERT_NEAR(0.0, Extent2.GetYMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Extent2.GetYMax());

    //With double
    HGF2DLiteExtent Extent3(0.0, 0.0, 10.0, 10.0);
    ASSERT_TRUE(Extent3.IsDefined());

    ASSERT_NEAR(0.0, Extent3.GetXMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Extent3.GetXMax());
    ASSERT_NEAR(0.0, Extent3.GetYMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Extent3.GetYMax());

    //Copy Constructor
    HGF2DLiteExtent Extent5(Extent3);
    ASSERT_TRUE(Extent5.IsDefined());

    ASSERT_NEAR(0.0, Extent5.GetXMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Extent5.GetXMax());
    ASSERT_NEAR(0.0, Extent5.GetYMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Extent5.GetYMax());

    }

//==================================================================================
// operator=(const HGF2DLiteExtent& pi_rObj);
//==================================================================================
TEST_F (HGF2DLiteExtentTester, OperatorTest)
    {

    HGF2DLiteExtent Extent2 = Extent1;
    ASSERT_NEAR(0.0, Extent2.GetXMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Extent2.GetXMax());
    ASSERT_NEAR(0.0, Extent2.GetYMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Extent2.GetYMax());

    }

//==================================================================================
// operator==(const HGF2DLiteExtent& pi_rObj) const;
// operator!=(const HGF2DLiteExtent& pi_rObj) const;
// IsEqualTo(const HGF2DLiteExtent& pi_rObj) const;
// IsEqualTo(const HGF2DLiteExtent& pi_rObj, double pi_Epsilon) const;
//==================================================================================
TEST_F (HGF2DLiteExtentTester, CompareTest)
    {

    HGF2DLiteExtent Extent3(HGF2DPosition(0.0, 0.0 + MYEPSILON), HGF2DPosition(10.0, 10.0));

    ASSERT_TRUE(Extent1 == Extent1);
    ASSERT_FALSE(Extent2 == Extent1);
    ASSERT_FALSE(Extent3 == Extent1);

    ASSERT_FALSE(Extent1 != Extent1);
    ASSERT_TRUE(Extent2 != Extent1);
    ASSERT_TRUE(Extent3 != Extent1);

    ASSERT_TRUE(Extent1.IsEqualTo(Extent1));
    ASSERT_FALSE(Extent2.IsEqualTo(Extent1));
    ASSERT_TRUE(Extent3.IsEqualTo(Extent1));

    ASSERT_TRUE(Extent1.IsEqualTo(Extent1, MYEPSILON));
    ASSERT_FALSE(Extent2.IsEqualTo(Extent1, MYEPSILON));
    ASSERT_TRUE(Extent3.IsEqualTo(Extent1, MYEPSILON));
    
    ASSERT_TRUE(Extent1.IsEqualTo(Extent1, 10.0));
    ASSERT_TRUE(Extent2.IsEqualTo(Extent1, 10.0));
    ASSERT_TRUE(Extent3.IsEqualTo(Extent1, 10.0));

    }

//==================================================================================
// IsPointIn(const HGF2DPosition& pi_rPoint) const;
// IsPointInnerIn(const HGF2DPosition& pi_rPoint) const;
// IsPointOutterIn(const HGF2DPosition& pi_rPoint) const;
// IsPointInnerIn(const HGF2DPosition& pi_rPoint, double pi_Tolerance) const;
// IsPointOutterIn(const HGF2DPosition& pi_rPoint, double pi_Tolerance) const;
//==================================================================================
TEST_F (HGF2DLiteExtentTester, IsPointInTest)
    {

    ASSERT_FALSE(Extent1.IsPointIn(HGF2DPosition(-0.1, -0.1)));
    ASSERT_TRUE(Extent1.IsPointIn(HGF2DPosition(0.0, 0.0)));
    ASSERT_TRUE(Extent1.IsPointIn(HGF2DPosition(1.0, 1.0)));
    ASSERT_TRUE(Extent1.IsPointIn(HGF2DPosition(10.0, 10.0)));
    ASSERT_FALSE(Extent1.IsPointIn(HGF2DPosition(10.1, 10.1)));

    ASSERT_FALSE(Extent1.IsPointInnerIn(HGF2DPosition(-0.1, -0.1)));
    ASSERT_FALSE(Extent1.IsPointInnerIn(HGF2DPosition(0.0, 0.0)));
    ASSERT_TRUE(Extent1.IsPointInnerIn(HGF2DPosition(1.0, 1.0)));
    ASSERT_FALSE(Extent1.IsPointInnerIn(HGF2DPosition(10.0, 10.0)));
    ASSERT_FALSE(Extent1.IsPointInnerIn(HGF2DPosition(10.1, 10.1)));

    }

//==================================================================================
// SetXMin(double    pi_XMin);
// SetYMin(double    pi_YMin);
// SetXMax(double    pi_XMax);
// SetYMax(double    pi_YMax);
//==================================================================================
TEST_F (HGF2DLiteExtentTester, SetTest)
    {

    Extent1.SetXMin(-50.0);
    ASSERT_DOUBLE_EQ(-50.0, Extent1.GetXMin());
    ASSERT_DOUBLE_EQ(10.00, Extent1.GetXMax());
    ASSERT_NEAR(0.0, Extent1.GetYMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.00, Extent1.GetYMax());

    Extent1.SetYMin(-50.0);
    ASSERT_DOUBLE_EQ(-50.0, Extent1.GetXMin());
    ASSERT_DOUBLE_EQ(10.00, Extent1.GetXMax());
    ASSERT_DOUBLE_EQ(-50.0, Extent1.GetYMin());
    ASSERT_DOUBLE_EQ(10.00, Extent1.GetYMax());

    Extent1.SetXMax(50.0);
    ASSERT_DOUBLE_EQ(-50.0, Extent1.GetXMin());
    ASSERT_DOUBLE_EQ(50.00, Extent1.GetXMax());
    ASSERT_DOUBLE_EQ(-50.0, Extent1.GetYMin());
    ASSERT_DOUBLE_EQ(10.00, Extent1.GetYMax());

    Extent1.SetYMax(50.0);
    ASSERT_DOUBLE_EQ(-50.0, Extent1.GetXMin());
    ASSERT_DOUBLE_EQ(50.00, Extent1.GetXMax());
    ASSERT_DOUBLE_EQ(-50.0, Extent1.GetYMin());
    ASSERT_DOUBLE_EQ(50.00, Extent1.GetYMax());

    }

//==================================================================================
// GetOrigin() const;
// GetCorner() const;
// SetOrigin(const HGF2DPosition& pi_rNewOrigin);
// SetCorner(const HGF2DPosition& pi_rNewCorner);
//==================================================================================
TEST_F (HGF2DLiteExtentTester, OriginCornerTest)
    {

    ASSERT_NEAR(0.0, Extent1.GetOrigin().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, Extent1.GetOrigin().GetY(), MYEPSILON);

    ASSERT_DOUBLE_EQ(10.0, Extent1.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(10.0, Extent1.GetCorner().GetY());

    Extent1.SetOrigin(HGF2DPosition(5.0, 5.0));
    ASSERT_DOUBLE_EQ(5.00, Extent1.GetOrigin().GetX());
    ASSERT_DOUBLE_EQ(5.00, Extent1.GetOrigin().GetY());
    ASSERT_DOUBLE_EQ(10.0, Extent1.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(10.0, Extent1.GetCorner().GetY());

    Extent1.SetCorner(HGF2DPosition(15.0, 15.0));
    ASSERT_DOUBLE_EQ(5.00, Extent1.GetOrigin().GetX());
    ASSERT_DOUBLE_EQ(5.00, Extent1.GetOrigin().GetY());
    ASSERT_DOUBLE_EQ(15.0, Extent1.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(15.0, Extent1.GetCorner().GetY());

    }  

//==================================================================================
// GetWidth() const;
// GetHeight() const;
// CalculateArea() const;
//==================================================================================
TEST_F (HGF2DLiteExtentTester, DimensionTest)
    {

    ASSERT_DOUBLE_EQ(10.0, Extent1.GetWidth());
    ASSERT_DOUBLE_EQ(20.0, Extent2.GetWidth());
    ASSERT_DOUBLE_EQ(10.0, Extent3.GetWidth());

    ASSERT_DOUBLE_EQ(10.0, Extent1.GetHeight());
    ASSERT_DOUBLE_EQ(20.0, Extent2.GetHeight());
    ASSERT_DOUBLE_EQ(20.0, Extent3.GetHeight());

    }

//==================================================================================
// Add (const HGF2DPosition& pi_rLocation);
// Add (const HGF2DLiteExtent& pi_rExtent);
//==================================================================================
TEST_F (HGF2DLiteExtentTester, AddTest)
    {

    Extent1.Add(HGF2DPosition(30.0, 30.0));
    ASSERT_NEAR(0.0, Extent1.GetOrigin().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, Extent1.GetOrigin().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, Extent1.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(30.0, Extent1.GetCorner().GetY());

    Extent1.Add(HGF2DPosition(20.0, 20.0));
    ASSERT_NEAR(0.0, Extent1.GetOrigin().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, Extent1.GetOrigin().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, Extent1.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(30.0, Extent1.GetCorner().GetY());

    Extent1.Add(HGF2DPosition(-10.0, -50.0));
    ASSERT_DOUBLE_EQ(-10.0, Extent1.GetOrigin().GetX());
    ASSERT_DOUBLE_EQ(-50.0, Extent1.GetOrigin().GetY());
    ASSERT_DOUBLE_EQ(30.00, Extent1.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(30.00, Extent1.GetCorner().GetY());

    HGF2DLiteExtent DefaultExtent;

    DefaultExtent.Set(0.0, 0.0, 30.0, 30.0);
    ASSERT_TRUE(DefaultExtent.IsDefined());
    ASSERT_NEAR(0.0, DefaultExtent.GetOrigin().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, DefaultExtent.GetOrigin().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, DefaultExtent.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(30.0, DefaultExtent.GetCorner().GetY());

    Extent2.Add(HGF2DLiteExtent(HGF2DPosition(0.0, 0.0), HGF2DPosition(30.0, 30.0)));
    ASSERT_NEAR(0.0, Extent2.GetOrigin().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, Extent2.GetOrigin().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, Extent2.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(30.0, Extent2.GetCorner().GetY());

    Extent2.Add(HGF2DLiteExtent(0.0, 20.0, 0.0, 20.0));
    ASSERT_NEAR(0.0, Extent2.GetOrigin().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, Extent2.GetOrigin().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, Extent2.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(30.0, Extent2.GetCorner().GetY());

    Extent2.Add(HGF2DLiteExtent(HGF2DPosition(-10.0, -50.0), HGF2DPosition(30.0, 30.0)));
    ASSERT_DOUBLE_EQ(-10.0, Extent2.GetOrigin().GetX());
    ASSERT_DOUBLE_EQ(-50.0, Extent2.GetOrigin().GetY());
    ASSERT_DOUBLE_EQ(30.00, Extent2.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(30.00, Extent2.GetCorner().GetY());

    }

//==================================================================================
// Unify( const HGF2DLiteExtent& pi_rObj )
//==================================================================================
TEST_F (HGF2DLiteExtentTester, UnifyTest)
    {

    //Extent
    HGF2DLiteExtent ExtentHimself = HGF2DLiteExtent(HGF2DPosition(0.0, 0.0), HGF2DPosition(10.0, 10.0));
    HGF2DLiteExtent ExtentEquivalent = HGF2DLiteExtent(HGF2DPosition(0.0, 0.0), HGF2DPosition(10.0, 10.0));
    HGF2DLiteExtent ExtentIntersect = HGF2DLiteExtent(HGF2DPosition(5.0, 5.0), HGF2DPosition(15.0, 15.0));
    HGF2DLiteExtent ExtentNoIntersect = HGF2DLiteExtent(HGF2DPosition(-15.0, -15.0), HGF2DPosition(-5.0, -5.0));

    // With himself
    ExtentHimself.Union(ExtentHimself);
    ASSERT_TRUE(ExtentHimself.IsDefined());

    ASSERT_NEAR(0.0, ExtentHimself.GetOrigin().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ExtentHimself.GetOrigin().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, ExtentHimself.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(10.0, ExtentHimself.GetCorner().GetY());

    // With equivalent
    ExtentHimself.Union(ExtentEquivalent);
    ASSERT_TRUE(ExtentHimself.IsDefined());

    ASSERT_NEAR(0.0, ExtentHimself.GetOrigin().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ExtentHimself.GetOrigin().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, ExtentHimself.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(10.0, ExtentHimself.GetCorner().GetY());

    // With another who intersect
    ExtentHimself.Union(ExtentIntersect);
    ASSERT_TRUE(ExtentHimself.IsDefined());

    ASSERT_NEAR(0.0, ExtentHimself.GetOrigin().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ExtentHimself.GetOrigin().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(15.0, ExtentHimself.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(15.0, ExtentHimself.GetCorner().GetY());

    // With another who doesnt intersect
    ExtentHimself.Union(ExtentNoIntersect);
    ASSERT_TRUE(ExtentHimself.IsDefined());

    ASSERT_DOUBLE_EQ(-15.0, ExtentHimself.GetOrigin().GetX());
    ASSERT_DOUBLE_EQ(-15.0, ExtentHimself.GetOrigin().GetY());
    ASSERT_DOUBLE_EQ(15.00, ExtentHimself.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(15.00, ExtentHimself.GetCorner().GetY());

    }

//==================================================================================
// Intersect (const HGF2DLiteExtent& pi_rObj);
//==================================================================================
TEST_F (HGF2DLiteExtentTester, IntersectTest)
    {

    //Extent
    HGF2DLiteExtent ExtentHimself = HGF2DLiteExtent(HGF2DPosition(0.0, 0.0), HGF2DPosition(10.0, 10.0));
    HGF2DLiteExtent ExtentEquivalent = HGF2DLiteExtent(HGF2DPosition(0.0, 0.0), HGF2DPosition(10.0, 10.0));
    HGF2DLiteExtent ExtentIntersect = HGF2DLiteExtent(HGF2DPosition(5.0, 5.0), HGF2DPosition(15.0, 15.0));
    HGF2DLiteExtent ExtentNoIntersect = HGF2DLiteExtent(HGF2DPosition(-15.0, -15.0), HGF2DPosition(-5.0, -5.0));

    // With himself
    ExtentHimself.Intersect(ExtentHimself);
    ASSERT_TRUE(ExtentHimself.IsDefined());

    ASSERT_NEAR(0.0, ExtentHimself.GetOrigin().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ExtentHimself.GetOrigin().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, ExtentHimself.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(10.0, ExtentHimself.GetCorner().GetY());

    // With equivalent
    ExtentHimself.Intersect(ExtentEquivalent);
    ASSERT_TRUE(ExtentHimself.IsDefined());

    ASSERT_NEAR(0.0, ExtentHimself.GetOrigin().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ExtentHimself.GetOrigin().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, ExtentHimself.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(10.0, ExtentHimself.GetCorner().GetY());

    // With another who intersect
    ExtentHimself.Intersect(ExtentIntersect);
    ASSERT_TRUE(ExtentHimself.IsDefined());

    ASSERT_DOUBLE_EQ(5.00, ExtentHimself.GetOrigin().GetX());
    ASSERT_DOUBLE_EQ(5.00, ExtentHimself.GetOrigin().GetY());
    ASSERT_DOUBLE_EQ(10.0, ExtentHimself.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(10.0, ExtentHimself.GetCorner().GetY());

    // With another who doesnt intersect
    ExtentHimself.Intersect(ExtentNoIntersect);
    ASSERT_FALSE(ExtentHimself.IsDefined());

    }

//==================================================================================
// Overlaps (const HGF2DLiteExtent& pi_rObj) const;
// OutterOverlaps(const HGF2DLiteExtent& pi_rObj) const;
// OutterOverlaps(const HGF2DLiteExtent& pi_rObj, double pi_Epsilon) const;
// InnerOverlaps(const HGF2DLiteExtent& pi_rObj) const;
// InnerOverlaps(const HGF2DLiteExtent& pi_rObj, double pi_Epsilon) const;
//==================================================================================
TEST_F (HGF2DLiteExtentTester, OverlapTest)
    {

    //Extent
    HGF2DLiteExtent ExtentHimself = HGF2DLiteExtent(HGF2DPosition(0.0, 0.0), HGF2DPosition(10.0, 10.0));
    HGF2DLiteExtent ExtentEquivalent = HGF2DLiteExtent(HGF2DPosition(0.0, 0.0), HGF2DPosition(10.0, 10.0));
    HGF2DLiteExtent ExtentIntersect = HGF2DLiteExtent(HGF2DPosition(5.0, 5.0), HGF2DPosition(15.0, 15.0));
    HGF2DLiteExtent ExtentAlmostIntersect = HGF2DLiteExtent(HGF2DPosition(10.0, 10.0 + MYEPSILON), HGF2DPosition(20.0, 20.0));
    HGF2DLiteExtent ExtentNoIntersect = HGF2DLiteExtent(HGF2DPosition(-15.0, -15.0), HGF2DPosition(-5.0, -5.0));

    // With himself
    ASSERT_TRUE(ExtentHimself.Overlaps(ExtentHimself));
    ASSERT_TRUE(ExtentHimself.OutterOverlaps(ExtentHimself));
    ASSERT_TRUE(ExtentHimself.OutterOverlaps(ExtentHimself, MYEPSILON));
    ASSERT_TRUE(ExtentHimself.InnerOverlaps(ExtentHimself));
    ASSERT_TRUE(ExtentHimself.InnerOverlaps(ExtentHimself, MYEPSILON));

    // With equivalent
    ASSERT_TRUE(ExtentHimself.Overlaps(ExtentEquivalent));
    ASSERT_TRUE(ExtentHimself.OutterOverlaps(ExtentEquivalent));
    ASSERT_TRUE(ExtentHimself.OutterOverlaps(ExtentEquivalent, MYEPSILON));
    ASSERT_TRUE(ExtentHimself.InnerOverlaps(ExtentEquivalent));
    ASSERT_TRUE(ExtentHimself.InnerOverlaps(ExtentEquivalent, MYEPSILON));

    // With another who intersect
    ASSERT_TRUE(ExtentHimself.Overlaps(ExtentIntersect));
    ASSERT_TRUE(ExtentHimself.OutterOverlaps(ExtentIntersect));
    ASSERT_TRUE(ExtentHimself.OutterOverlaps(ExtentIntersect, MYEPSILON));
    ASSERT_TRUE(ExtentHimself.InnerOverlaps(ExtentIntersect));
    ASSERT_TRUE(ExtentHimself.InnerOverlaps(ExtentIntersect, MYEPSILON));

    //With another who almost intersect
    ASSERT_FALSE(ExtentHimself.Overlaps(ExtentAlmostIntersect));
    ASSERT_TRUE(ExtentHimself.OutterOverlaps(ExtentAlmostIntersect));
    ASSERT_TRUE(ExtentHimself.OutterOverlaps(ExtentAlmostIntersect, MYEPSILON));
    ASSERT_FALSE(ExtentHimself.InnerOverlaps(ExtentAlmostIntersect));
    ASSERT_FALSE(ExtentHimself.InnerOverlaps(ExtentAlmostIntersect, MYEPSILON));

    // With another who doesnt intersect
    ASSERT_FALSE(ExtentHimself.Overlaps(ExtentNoIntersect));
    ASSERT_FALSE(ExtentHimself.OutterOverlaps(ExtentNoIntersect));
    ASSERT_FALSE(ExtentHimself.OutterOverlaps(ExtentNoIntersect, MYEPSILON));
    ASSERT_FALSE(ExtentHimself.InnerOverlaps(ExtentNoIntersect));
    ASSERT_FALSE(ExtentHimself.InnerOverlaps(ExtentNoIntersect, MYEPSILON));

    }

//==================================================================================
// Contains(const HGF2DLiteExtent& pi_rObj) const;
// InnerContains(const HGF2DLiteExtent& pi_rObj) const;
// InnerContains(const HGF2DLiteExtent& pi_rObj, double pi_Epsilon) const;
// OuterContains(const HGF2DLiteExtent& pi_rObj) const;
// OuterContains(const HGF2DLiteExtent& pi_rObj, double pi_Epsilon) const;
//==================================================================================
TEST_F (HGF2DLiteExtentTester, ContainsTest)
    {
 
    //Extent
    HGF2DLiteExtent ExtentHimself = HGF2DLiteExtent(HGF2DPosition(0.0, 0.0), HGF2DPosition(10.0, 10.0));
    HGF2DLiteExtent ExtentEquivalent = HGF2DLiteExtent(HGF2DPosition(0.0, 0.0), HGF2DPosition(10.0, 10.0));
    HGF2DLiteExtent ExtentInside = HGF2DLiteExtent(HGF2DPosition(5.0, 5.0), HGF2DPosition(9.0, 9.0));
    HGF2DLiteExtent ExtentNoInside = HGF2DLiteExtent(HGF2DPosition(-15.0, -15.0), HGF2DPosition(-5.0, -5.0));
   
    // With himself
    ASSERT_FALSE(ExtentHimself.Contains(ExtentHimself));
    ASSERT_FALSE(ExtentHimself.InnerContains(ExtentHimself));
    ASSERT_FALSE(ExtentHimself.InnerContains(ExtentHimself, MYEPSILON));
    ASSERT_TRUE(ExtentHimself.OuterContains(ExtentHimself));
    ASSERT_TRUE(ExtentHimself.OuterContains(ExtentHimself, MYEPSILON));

    // With equivalent
    ASSERT_FALSE(ExtentHimself.Contains(ExtentEquivalent));
    ASSERT_FALSE(ExtentHimself.InnerContains(ExtentEquivalent));
    ASSERT_FALSE(ExtentHimself.InnerContains(ExtentEquivalent, MYEPSILON));
    ASSERT_TRUE(ExtentHimself.OuterContains(ExtentEquivalent));
    ASSERT_TRUE(ExtentHimself.OuterContains(ExtentEquivalent, MYEPSILON));

    // With another who is inside
    ASSERT_TRUE(ExtentHimself.Contains(ExtentInside));
    ASSERT_TRUE(ExtentHimself.InnerContains(ExtentInside));
    ASSERT_TRUE(ExtentHimself.InnerContains(ExtentInside, MYEPSILON));
    ASSERT_TRUE(ExtentHimself.OuterContains(ExtentInside));
    ASSERT_TRUE(ExtentHimself.OuterContains(ExtentInside, MYEPSILON));

    // With another who isnt inside
    ASSERT_FALSE(ExtentHimself.Contains(ExtentNoInside));
    ASSERT_FALSE(ExtentHimself.InnerContains(ExtentNoInside));
    ASSERT_FALSE(ExtentHimself.InnerContains(ExtentNoInside, MYEPSILON));
    ASSERT_FALSE(ExtentHimself.OuterContains(ExtentNoInside));
    ASSERT_FALSE(ExtentHimself.OuterContains(ExtentNoInside, MYEPSILON));

    }
