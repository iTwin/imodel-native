//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/NonPublished/IppGraLibs/HGF2DExtentTester.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "HGF2DExtentTester.h"

HGF2DExtentTester::HGF2DExtentTester() 
    {

    //General
    Translation = HGF2DDisplacement(10.0, 30.48);
    Rotation = PI/4;
    ScalingX = 1.00001;
    ScalingY = 2.00001;
    Anortho = -0.000001;
    Affine1 = HGF2DAffine(Translation, Rotation, ScalingX, ScalingY, Anortho);

    // COORDINATE SYSTEMS
    pWorld = new HGF2DCoordSys();
    pSys1 = new HGF2DCoordSys(Affine1, pWorld);

    //Extent
    Extent1 = HGF2DExtent(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    Extent2 = HGF2DExtent(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(20.0, 20.0, pWorld));
    Extent3 = HGF2DExtent(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 20.0, pWorld));

    }

// THE PRESENT TEST CANNOT MAKE TESTS WITH NON-LINEAR TRANSFORMATION MODELS

//==================================================================================
// Construction tests
//==================================================================================
TEST_F (HGF2DExtentTester, ConstructorsTest)
    {

    //Default constructor
    HGF2DExtent defaultExtent;
    ASSERT_FALSE(defaultExtent.IsDefined());

    //With CoordSys
    HGF2DExtent Extent1(pWorld);
    ASSERT_EQ(pWorld, Extent1.GetCoordSys());
    ASSERT_FALSE(Extent1.IsDefined());

    //With Location
    HGF2DExtent Extent2(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    ASSERT_EQ(pWorld, Extent2.GetCoordSys());
    ASSERT_TRUE(Extent2.IsDefined());

    ASSERT_NEAR(0.0, Extent2.GetXMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Extent2.GetXMax());
    ASSERT_NEAR(0.0, Extent2.GetYMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Extent2.GetYMax());

    //With double
    HGF2DExtent Extent3(0.0, 0.0, 10.0, 10.0, pWorld);
    ASSERT_EQ(pWorld, Extent3.GetCoordSys());

    ASSERT_TRUE(Extent3.IsDefined());
    ASSERT_NEAR(0.0, Extent3.GetXMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Extent3.GetXMax());
    ASSERT_NEAR(0.0, Extent3.GetYMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Extent3.GetYMax());

    //With String
    string extentSerialized(Extent3.Serialize());
    HGF2DExtent Extent4(extentSerialized, pWorld);
    ASSERT_EQ(pWorld, Extent4.GetCoordSys());
    ASSERT_TRUE(Extent4.IsDefined());

    ASSERT_NEAR(0.0, Extent4.GetXMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Extent4.GetXMax());
    ASSERT_NEAR(0.0, Extent4.GetYMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Extent4.GetYMax());

    //Copy Constructor
    HGF2DExtent Extent5(Extent4);
    ASSERT_EQ(pWorld, Extent5.GetCoordSys());
    ASSERT_TRUE(Extent5.IsDefined());

    ASSERT_NEAR(0.0, Extent5.GetXMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Extent5.GetXMax());
    ASSERT_NEAR(0.0, Extent5.GetYMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Extent5.GetYMax());

    }

//==================================================================================
// operator=(const HGF2DExtent& pi_rObj);
//==================================================================================
TEST_F (HGF2DExtentTester, OperatorTest)
    {

    HGF2DExtent Extent2 = Extent1;
    ASSERT_EQ(pWorld, Extent2.GetCoordSys());

    ASSERT_NEAR(0.0, Extent2.GetXMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Extent2.GetXMax());
    ASSERT_NEAR(0.0, Extent2.GetYMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Extent2.GetYMax());

    }

//==================================================================================
// operator==(const HGF2DExtent& pi_rObj) const;
// operator!=(const HGF2DExtent& pi_rObj) const;
// IsEqualTo(const HGF2DExtent& pi_rObj) const;
// IsEqualTo(const HGF2DExtent& pi_rObj, double pi_Epsilon) const;
//==================================================================================
TEST_F (HGF2DExtentTester, CompareTest)
    {

    HGF2DExtent Extent3(HGF2DLocation(0.0, 0.0 + MYEPSILON, pWorld), HGF2DLocation(10.0, 10.0, pWorld));

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
//IsPointIn(const HGF2DLocation& pi_rPoint, bool pi_ExcludeBoundary = false) const;
//==================================================================================
TEST_F (HGF2DExtentTester, IsPointInTest)
    {

    ASSERT_FALSE(Extent1.IsPointIn(HGF2DLocation(-0.1, -0.1, pWorld)));
    ASSERT_TRUE(Extent1.IsPointIn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(Extent1.IsPointIn(HGF2DLocation(1.0, 1.0, pWorld)));
    ASSERT_TRUE(Extent1.IsPointIn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_FALSE(Extent1.IsPointIn(HGF2DLocation(10.1, 10.1, pWorld)));

    ASSERT_FALSE(Extent1.IsPointIn(HGF2DLocation(-0.1, -0.1, pWorld), true));
    ASSERT_FALSE(Extent1.IsPointIn(HGF2DLocation(0.0, 0.0, pWorld), true));
    ASSERT_TRUE(Extent1.IsPointIn(HGF2DLocation(1.0, 1.0, pWorld), true));
    ASSERT_FALSE(Extent1.IsPointIn(HGF2DLocation(10.0, 10.0, pWorld), true));
    ASSERT_FALSE(Extent1.IsPointIn(HGF2DLocation(10.1, 10.1, pWorld), true));

    }

//==================================================================================
// SetXMin(double    pi_XMin);
// SetYMin(double    pi_YMin);
// SetXMax(double    pi_XMax);
// SetYMax(double    pi_YMax);
//==================================================================================
TEST_F (HGF2DExtentTester, SetTest)
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
// SetOrigin(const HGF2DLocation& pi_rNewOrigin);
// SetCorner(const HGF2DLocation& pi_rNewCorner);
//==================================================================================
TEST_F (HGF2DExtentTester, OriginCornerTest)
    {

    ASSERT_NEAR(0.0, Extent1.GetOrigin().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, Extent1.GetOrigin().GetY(), MYEPSILON);

    ASSERT_DOUBLE_EQ(10.0, Extent1.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(10.0, Extent1.GetCorner().GetY());

    Extent1.SetOrigin(HGF2DLocation(5.0, 5.0, pWorld));
    ASSERT_DOUBLE_EQ(5.00, Extent1.GetOrigin().GetX());
    ASSERT_DOUBLE_EQ(5.00, Extent1.GetOrigin().GetY());
    ASSERT_DOUBLE_EQ(10.0, Extent1.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(10.0, Extent1.GetCorner().GetY());

    Extent1.SetCorner(HGF2DLocation(15.0, 15.0, pWorld));
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
TEST_F (HGF2DExtentTester, DimensionTest)
    {

    ASSERT_DOUBLE_EQ(10.0, Extent1.GetWidth());
    ASSERT_DOUBLE_EQ(20.0, Extent2.GetWidth());
    ASSERT_DOUBLE_EQ(10.0, Extent3.GetWidth());

    ASSERT_DOUBLE_EQ(10.0, Extent1.GetHeight());
    ASSERT_DOUBLE_EQ(20.0, Extent2.GetHeight());
    ASSERT_DOUBLE_EQ(20.0, Extent3.GetHeight());

    ASSERT_DOUBLE_EQ(100.0, Extent1.CalculateArea());
    ASSERT_DOUBLE_EQ(400.0, Extent2.CalculateArea());
    ASSERT_DOUBLE_EQ(200.0, Extent3.CalculateArea());

    }

//==================================================================================
// GetCoordSys () const;
// SetCoordSys (const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
// ChangeCoordSys (const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
//==================================================================================
TEST_F (HGF2DExtentTester, CoordSysTest)
    {

    HGF2DTranslation Translation(HGF2DDisplacement(10.0, 10.0));
    HFCPtr<HGF2DCoordSys> TranslationCoord = new HGF2DCoordSys(Translation, pWorld);
        
    ASSERT_EQ(pWorld, Extent1.GetCoordSys());

    Extent1.ChangeCoordSys(TranslationCoord);
    ASSERT_EQ(TranslationCoord, Extent1.GetCoordSys());

    Extent1.SetCoordSys(pWorld);
    ASSERT_EQ(pWorld, Extent1.GetCoordSys());

    }

//==================================================================================
// Move(const HGF2DDisplacement& pi_rOffset);
//==================================================================================
TEST_F (HGF2DExtentTester, MoveTest)
    {

    HGF2DDisplacement Translation1(10.0, 10.0);
    HGF2DDisplacement Translation2(0.0, 10.0);
    HGF2DDisplacement Translation3(10.0, 0.0);
    HGF2DDisplacement Translation4(10.0, -10.0);
    HGF2DDisplacement Translation5(-10.0, 10.0);

    Extent1.Move(Translation1);
    ASSERT_DOUBLE_EQ(10.0, Extent1.GetOrigin().GetX());
    ASSERT_DOUBLE_EQ(10.0, Extent1.GetOrigin().GetY());
    ASSERT_DOUBLE_EQ(20.0, Extent1.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(20.0, Extent1.GetCorner().GetY());

    Extent1.Move(Translation2);
    ASSERT_DOUBLE_EQ(10.0, Extent1.GetOrigin().GetX());
    ASSERT_DOUBLE_EQ(20.0, Extent1.GetOrigin().GetY());
    ASSERT_DOUBLE_EQ(20.0, Extent1.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(30.0, Extent1.GetCorner().GetY());

    Extent1.Move(Translation3);
    ASSERT_DOUBLE_EQ(20.0, Extent1.GetOrigin().GetX());
    ASSERT_DOUBLE_EQ(20.0, Extent1.GetOrigin().GetY());
    ASSERT_DOUBLE_EQ(30.0, Extent1.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(30.0, Extent1.GetCorner().GetY());

    Extent1.Move(Translation4);
    ASSERT_DOUBLE_EQ(30.0, Extent1.GetOrigin().GetX());
    ASSERT_DOUBLE_EQ(10.0, Extent1.GetOrigin().GetY());
    ASSERT_DOUBLE_EQ(40.0, Extent1.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(20.0, Extent1.GetCorner().GetY());

    Extent1.Move(Translation5);
    ASSERT_DOUBLE_EQ(20.0, Extent1.GetOrigin().GetX());
    ASSERT_DOUBLE_EQ(20.0, Extent1.GetOrigin().GetY());
    ASSERT_DOUBLE_EQ(30.0, Extent1.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(30.0, Extent1.GetCorner().GetY());

    }

//==================================================================================
// operator+(const HGF2DDisplacement& pi_rOffset) const;
// operator-(const HGF2DDisplacement& pi_rOffset) const;
// operator+=(const HGF2DDisplacement& pi_rOffset);
// operator-=(const HGF2DDisplacement& pi_rOffset);
//==================================================================================
TEST_F (HGF2DExtentTester, OperatorTest2)
    {

    HGF2DDisplacement Translation1(10.0, 10.0);
    HGF2DDisplacement Translation2(0.0, 10.0);
    HGF2DDisplacement Translation3(10.0, 0.0);
    HGF2DDisplacement Translation4(10.0, -10.0);
    HGF2DDisplacement Translation5(-10.0, 10.0);

    ASSERT_TRUE((Extent1 + Translation1).IsEqualTo(HGF2DExtent(10.0, 10.0, 20.0, 20.0, pWorld)));
    ASSERT_TRUE((Extent1 + Translation2).IsEqualTo(HGF2DExtent(0.0, 10.0, 10.0, 20.0, pWorld)));
    ASSERT_TRUE((Extent1 + Translation3).IsEqualTo(HGF2DExtent(10.0, 0.0, 20.0, 10.0, pWorld)));
    ASSERT_TRUE((Extent1 + Translation4).IsEqualTo(HGF2DExtent(10.0, -10.0, 20.0, 0.0, pWorld)));
    ASSERT_TRUE((Extent1 + Translation5).IsEqualTo(HGF2DExtent(-10.0, 10.0, 0.0, 20.0, pWorld)));

    ASSERT_TRUE((Extent1 - Translation1).IsEqualTo(HGF2DExtent(-10.0, -10.0, 0.0, 0.0, pWorld)));
    ASSERT_TRUE((Extent1 - Translation2).IsEqualTo(HGF2DExtent(0.0, -10.0, 10.0, 0.0, pWorld)));
    ASSERT_TRUE((Extent1 - Translation3).IsEqualTo( HGF2DExtent(-10.0, 0.0, 0.0, 10.0, pWorld)));
    ASSERT_TRUE((Extent1 - Translation4).IsEqualTo(HGF2DExtent(-10.0, 10.0, 0.0, 20.0, pWorld)));
    ASSERT_TRUE((Extent1 - Translation5).IsEqualTo(HGF2DExtent(10.0, -10.0, 20.0, 0.0, pWorld)));

    Extent1 += Translation1;
    ASSERT_TRUE(Extent1.IsEqualTo(HGF2DExtent(10.0, 10.0, 20.0, 20.0, pWorld)));
    Extent1 += Translation2;
    ASSERT_TRUE(Extent1.IsEqualTo(HGF2DExtent(10.0, 20.0, 20.0, 30.0, pWorld)));
    Extent1 += Translation3;
    ASSERT_TRUE(Extent1.IsEqualTo(HGF2DExtent(20.0, 20.0, 30.0, 30.0, pWorld)));
    Extent1 += Translation4;
    ASSERT_TRUE(Extent1.IsEqualTo(HGF2DExtent(30.0, 10.0, 40.0, 20.0, pWorld)));
    Extent1 += Translation5;
    ASSERT_TRUE(Extent1.IsEqualTo(HGF2DExtent(20.0, 20.0, 30.0, 30.0, pWorld)));

    Extent1 -= Translation1;
    ASSERT_TRUE(Extent1.IsEqualTo(HGF2DExtent(10.0, 10.0, 20.0, 20.0, pWorld)));
    Extent1 -= Translation2;
    ASSERT_TRUE(Extent1.IsEqualTo(HGF2DExtent(10.0, 0.0, 20.0, 10.0, pWorld)));
    Extent1 -= Translation3;
    ASSERT_TRUE(Extent1.IsEqualTo(HGF2DExtent(0.0, 0.0, 10.0, 10.0, pWorld)));
    Extent1 -= Translation4;
    ASSERT_TRUE(Extent1.IsEqualTo(HGF2DExtent(-10.0, 10.0, 0.0, 20.0, pWorld)));
    Extent1 -= Translation5;
    ASSERT_TRUE(Extent1.IsEqualTo(HGF2DExtent(0.0, 0.0, 10.0, 10.0, pWorld)));

    }

//==================================================================================
// Add (const HGF2DLocation& pi_rLocation);
// Add (const HGF2DExtent& pi_rExtent);
//==================================================================================
TEST_F (HGF2DExtentTester, AddTest)
    {

    Extent1.Add(HGF2DLocation(30.0, 30.0, pWorld));
    ASSERT_NEAR(0.0, Extent1.GetOrigin().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, Extent1.GetOrigin().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, Extent1.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(30.0, Extent1.GetCorner().GetY());

    Extent1.Add(HGF2DLocation(20.0, 20.0, pWorld));
    ASSERT_NEAR(0.0, Extent1.GetOrigin().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, Extent1.GetOrigin().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, Extent1.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(30.0, Extent1.GetCorner().GetY());

    Extent1.Add(HGF2DLocation(-10.0, -50.0, pWorld));
    ASSERT_DOUBLE_EQ(-10.0, Extent1.GetOrigin().GetX());
    ASSERT_DOUBLE_EQ(-50.0, Extent1.GetOrigin().GetY());
    ASSERT_DOUBLE_EQ(30.00, Extent1.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(30.00, Extent1.GetCorner().GetY());

    HGF2DExtent DefaultExtent(pWorld);

    DefaultExtent.Add(HGF2DLocation(30.0, 30.0, pWorld));
    ASSERT_TRUE(DefaultExtent.IsDefined());
    ASSERT_DOUBLE_EQ(30.0, DefaultExtent.GetOrigin().GetX());
    ASSERT_DOUBLE_EQ(30.0, DefaultExtent.GetOrigin().GetY());
    ASSERT_DOUBLE_EQ(30.0, DefaultExtent.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(30.0, DefaultExtent.GetCorner().GetY());

    DefaultExtent.Add(HGF2DLocation(0.0, 0.0, pWorld));
    ASSERT_TRUE(DefaultExtent.IsDefined());
    ASSERT_NEAR(0.0, DefaultExtent.GetOrigin().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, DefaultExtent.GetOrigin().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, DefaultExtent.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(30.0, DefaultExtent.GetCorner().GetY());

    Extent2.Add(HGF2DExtent(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(30.0, 30.0, pWorld)));
    ASSERT_NEAR(0.0, Extent2.GetOrigin().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, Extent2.GetOrigin().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, Extent2.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(30.0, Extent2.GetCorner().GetY());

    Extent2.Add(HGF2DExtent(0.0, 20.0, 0.0, 20.0, pWorld));
    ASSERT_NEAR(0.0, Extent2.GetOrigin().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, Extent2.GetOrigin().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, Extent2.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(30.0, Extent2.GetCorner().GetY());

    Extent2.Add(HGF2DExtent(HGF2DLocation(-10.0, -50.0, pWorld), HGF2DLocation(30.0, 30.0, pWorld)));
    ASSERT_DOUBLE_EQ(-10.0, Extent2.GetOrigin().GetX());
    ASSERT_DOUBLE_EQ(-50.0, Extent2.GetOrigin().GetY());
    ASSERT_DOUBLE_EQ(30.00, Extent2.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(30.00, Extent2.GetCorner().GetY());

    }

//==================================================================================
// Unify( const HGF2DExtent& pi_rObj )
//==================================================================================
TEST_F (HGF2DExtentTester, UnifyTest)
    {

    //Extent
    HGF2DExtent ExtentHimself = HGF2DExtent(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    HGF2DExtent ExtentEquivalent = HGF2DExtent(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    HGF2DExtent ExtentIntersect = HGF2DExtent(HGF2DLocation(5.0, 5.0, pWorld), HGF2DLocation(15.0, 15.0, pWorld));
    HGF2DExtent ExtentNoIntersect = HGF2DExtent(HGF2DLocation(-15.0, -15.0, pWorld), HGF2DLocation(-5.0, -5.0, pWorld));

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
// Intersect (const HGF2DExtent& pi_rObj);
//==================================================================================
TEST_F (HGF2DExtentTester, IntersectTest)
    {

    //Extent
    HGF2DExtent ExtentHimself = HGF2DExtent(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    HGF2DExtent ExtentEquivalent = HGF2DExtent(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    HGF2DExtent ExtentIntersect = HGF2DExtent(HGF2DLocation(5.0, 5.0, pWorld), HGF2DLocation(15.0, 15.0, pWorld));
    HGF2DExtent ExtentNoIntersect = HGF2DExtent(HGF2DLocation(-15.0, -15.0, pWorld), HGF2DLocation(-5.0, -5.0, pWorld));

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
// Differentiate (const HGF2DExtent& pi_rObj);
//==================================================================================
TEST_F (HGF2DExtentTester, DifferentiateTest)
    {

    //Extent
    HGF2DExtent ExtentHimself = HGF2DExtent(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    HGF2DExtent ExtentEquivalent = HGF2DExtent(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    HGF2DExtent ExtentEquivalent2 = HGF2DExtent(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    HGF2DExtent ExtentIntersect = HGF2DExtent(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    HGF2DExtent ExtentIntersect2 = HGF2DExtent(HGF2DLocation(5.0, 0.0, pWorld), HGF2DLocation(15.0, 15.0, pWorld));
    HGF2DExtent ExtentNoIntersect = HGF2DExtent(HGF2DLocation(-15.0, -15.0, pWorld), HGF2DLocation(-5.0, -5.0, pWorld));

    // With himself
    ExtentHimself.Differentiate(ExtentHimself);
    ASSERT_FALSE(ExtentHimself.IsDefined());

    // With equivalent
    ExtentEquivalent.Differentiate(ExtentEquivalent2);
    ASSERT_FALSE(ExtentEquivalent.IsDefined());

    // With another who intersect
    ExtentIntersect.Differentiate(ExtentIntersect2);
    ASSERT_TRUE(ExtentIntersect.IsDefined());

    ASSERT_NEAR(0.0, ExtentIntersect.GetOrigin().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ExtentIntersect.GetOrigin().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(5.00, ExtentIntersect.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(10.0, ExtentIntersect.GetCorner().GetY());

    // With another who doesnt intersect
    ExtentIntersect.Differentiate(ExtentNoIntersect);
    ASSERT_TRUE(ExtentIntersect.IsDefined());

    ASSERT_NEAR(0.0, ExtentIntersect.GetOrigin().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ExtentIntersect.GetOrigin().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(5.00, ExtentIntersect.GetCorner().GetX());
    ASSERT_DOUBLE_EQ(10.0, ExtentIntersect.GetCorner().GetY());

    }

//==================================================================================
// DoTheyOverlap (const HGF2DExtent& pi_rObj) const;
// OutterOverlaps(const HGF2DExtent& pi_rObj) const;
// OutterOverlaps(const HGF2DExtent& pi_rObj, double pi_Epsilon) const;
// InnerOverlaps(const HGF2DExtent& pi_rObj) const;
// InnerOverlaps(const HGF2DExtent& pi_rObj, double pi_Epsilon) const;
//==================================================================================
TEST_F (HGF2DExtentTester, OverlapTest)
    {

    //Extent
    HGF2DExtent ExtentHimself = HGF2DExtent(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    HGF2DExtent ExtentEquivalent = HGF2DExtent(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    HGF2DExtent ExtentIntersect = HGF2DExtent(HGF2DLocation(5.0, 5.0, pWorld), HGF2DLocation(15.0, 15.0, pWorld));
    HGF2DExtent ExtentAlmostIntersect = HGF2DExtent(HGF2DLocation(10.0, 10.0 + MYEPSILON, pWorld), HGF2DLocation(20.0, 20.0, pWorld));
    HGF2DExtent ExtentNoIntersect = HGF2DExtent(HGF2DLocation(-15.0, -15.0, pWorld), HGF2DLocation(-5.0, -5.0, pWorld));

    // With himself
    ASSERT_TRUE(ExtentHimself.DoTheyOverlap(ExtentHimself));
    ASSERT_TRUE(ExtentHimself.OutterOverlaps(ExtentHimself));
    ASSERT_TRUE(ExtentHimself.OutterOverlaps(ExtentHimself, MYEPSILON));
    ASSERT_TRUE(ExtentHimself.InnerOverlaps(ExtentHimself));
    ASSERT_TRUE(ExtentHimself.InnerOverlaps(ExtentHimself, MYEPSILON));

    // With equivalent
    ASSERT_TRUE(ExtentHimself.DoTheyOverlap(ExtentEquivalent));
    ASSERT_TRUE(ExtentHimself.OutterOverlaps(ExtentEquivalent));
    ASSERT_TRUE(ExtentHimself.OutterOverlaps(ExtentEquivalent, MYEPSILON));
    ASSERT_TRUE(ExtentHimself.InnerOverlaps(ExtentEquivalent));
    ASSERT_TRUE(ExtentHimself.InnerOverlaps(ExtentEquivalent, MYEPSILON));

    // With another who intersect
    ASSERT_TRUE(ExtentHimself.DoTheyOverlap(ExtentIntersect));
    ASSERT_TRUE(ExtentHimself.OutterOverlaps(ExtentIntersect));
    ASSERT_TRUE(ExtentHimself.OutterOverlaps(ExtentIntersect, MYEPSILON));
    ASSERT_TRUE(ExtentHimself.InnerOverlaps(ExtentIntersect));
    ASSERT_TRUE(ExtentHimself.InnerOverlaps(ExtentIntersect, MYEPSILON));

    //With another who almost intersect
    ASSERT_FALSE(ExtentHimself.DoTheyOverlap(ExtentAlmostIntersect));
    ASSERT_TRUE(ExtentHimself.OutterOverlaps(ExtentAlmostIntersect));
    ASSERT_TRUE(ExtentHimself.OutterOverlaps(ExtentAlmostIntersect, MYEPSILON));
    ASSERT_FALSE(ExtentHimself.InnerOverlaps(ExtentAlmostIntersect));
    ASSERT_FALSE(ExtentHimself.InnerOverlaps(ExtentAlmostIntersect, MYEPSILON));

    // With another who doesnt intersect
    ASSERT_FALSE(ExtentHimself.DoTheyOverlap(ExtentNoIntersect));
    ASSERT_FALSE(ExtentHimself.OutterOverlaps(ExtentNoIntersect));
    ASSERT_FALSE(ExtentHimself.OutterOverlaps(ExtentNoIntersect, MYEPSILON));
    ASSERT_FALSE(ExtentHimself.InnerOverlaps(ExtentNoIntersect));
    ASSERT_FALSE(ExtentHimself.InnerOverlaps(ExtentNoIntersect, MYEPSILON));

    }

//==================================================================================
// Contains(const HGF2DExtent& pi_rObj) const;
// InnerContains(const HGF2DExtent& pi_rObj) const;
// InnerContains(const HGF2DExtent& pi_rObj, double pi_Epsilon) const;
// OuterContains(const HGF2DExtent& pi_rObj) const;
// OuterContains(const HGF2DExtent& pi_rObj, double pi_Epsilon) const;
//==================================================================================
TEST_F (HGF2DExtentTester, ContainsTest)
    {
 
    //Extent
    HGF2DExtent ExtentHimself = HGF2DExtent(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    HGF2DExtent ExtentEquivalent = HGF2DExtent(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    HGF2DExtent ExtentInside = HGF2DExtent(HGF2DLocation(5.0, 5.0, pWorld), HGF2DLocation(9.0, 9.0, pWorld));
    HGF2DExtent ExtentNoInside = HGF2DExtent(HGF2DLocation(-15.0, -15.0, pWorld), HGF2DLocation(-5.0, -5.0, pWorld));
   
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


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2016
//----------------------------------------------------------------------------------------
TEST_F (HGF2DExtentTester, ProjectiveChangeCS)
    {
    double matrix[3][3];
    matrix[0][0] = 5.5147648962581428;
    matrix[0][1] = 0.0042851133851941970;
    matrix[0][2] = -7523.2436222951546;
    matrix[1][0] = -0.0050683470067062007;
    matrix[1][1] = 5.4906630316330816;
    matrix[1][2] = -41556.362377641810;
    matrix[2][0] = -1.4525233209147976e-010;
    matrix[2][1] = -6.5392539244920719e-008;
    matrix[2][2] = 1.0000000000000000;

    HFCPtr<HGF2DProjective> pProjec = new HGF2DProjective(matrix);

    HFCPtr<HGF2DCoordSys> pProjCS = new HGF2DCoordSys(*pProjec, pWorld);

    HFCPtr<HGF2DTransfoModel> pTransfoToProj = pWorld->GetTransfoModelTo(pProjCS);

    HGF2DRectangle currentExtent(0, 0, 512, 256);
    HFCPtr<HGF2DShape> resultShape = currentExtent.AllocTransformDirect(*pTransfoToProj);
    HGF2DLiteExtent finalExtent = resultShape->GetExtent();

    // Compute coordinates of four corners
    HGF2DLocation BottomLeft(0, 0, pWorld);
    HGF2DLocation TopLeft(0, 256, pWorld);
    HGF2DLocation TopRight(512, 256, pWorld);
    HGF2DLocation BottomRight(512, 0, pWorld);

    // Change coordinate sytem for these four points
    BottomLeft.ChangeCoordSys (pProjCS);
    TopLeft.ChangeCoordSys (pProjCS);
    TopRight.ChangeCoordSys (pProjCS);
    BottomRight.ChangeCoordSys (pProjCS);

    HGF2DExtent extent2(pProjCS);
    extent2.Add(BottomLeft);
    extent2.Add(TopLeft);
    extent2.Add(TopRight);
    extent2.Add(BottomRight);  

    ASSERT_DOUBLE_EQ(extent2.GetXMin(), finalExtent.GetXMin());
    ASSERT_DOUBLE_EQ(extent2.GetXMax(), finalExtent.GetXMax());
    ASSERT_DOUBLE_EQ(extent2.GetYMin(), finalExtent.GetYMin());
    ASSERT_DOUBLE_EQ(extent2.GetYMax(), finalExtent.GetYMax());
    }
 

