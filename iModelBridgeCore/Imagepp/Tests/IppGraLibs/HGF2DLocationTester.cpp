//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DLocationTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "HGF2DLocationTester.h"

HGF2DLocationTester::HGF2DLocationTester() 
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

    //With CoordSys
    Location1 = HGF2DLocation(pWorld);

    //With Distance
    Location2 = HGF2DLocation(5.0, 10.0, pWorld);

    }

//==================================================================================
// ConstructionTest
//==================================================================================
TEST_F (HGF2DLocationTester, ConstructionTest)
    {

    //Default Constructor
    HGF2DLocation Location;

    //Constructor with CoordSys
    HGF2DLocation Location1(pWorld);
    ASSERT_EQ(pWorld, Location1.GetCoordSys());

    //With Double
    HGF2DLocation Location3(5.0, 10.0, pWorld);
    ASSERT_EQ(pWorld, Location3.GetCoordSys());
    ASSERT_DOUBLE_EQ(5.00, Location3.GetX());
    ASSERT_DOUBLE_EQ(10.0, Location3.GetY());

    //With Position
    HGF2DLocation Location4(HGF2DPosition(5.0, 10.0), pWorld);
    ASSERT_EQ(pWorld, Location4.GetCoordSys());
    ASSERT_DOUBLE_EQ(5.00, Location4.GetX());
    ASSERT_DOUBLE_EQ(10.0, Location4.GetY());

    //Copy Constructor
    HGF2DLocation Location5(Location4);
    ASSERT_EQ(pWorld, Location5.GetCoordSys());
    ASSERT_DOUBLE_EQ(5.00, Location5.GetX());
    ASSERT_DOUBLE_EQ(10.0, Location5.GetY());

    //Copy Constructor with another CoordSys
    HGF2DLocation Location6(Location4, pSys1);
    ASSERT_EQ(pSys1, Location6.GetCoordSys());

    }

//==================================================================================
// operator=(const HGF2DLocation& pi_rObj);
// operator==(const HGF2DLocation& pi_rObj) const;
// operator!=(const HGF2DLocation& pi_rObj) const;
// operator<(const HGF2DLocation& pi_rObj) const;
//==================================================================================
TEST_F (HGF2DLocationTester, OperatorTest)
    {

    //Operator=
    HGF2DLocation Location3 = Location2;
    ASSERT_EQ(pWorld, Location3.GetCoordSys());
    ASSERT_DOUBLE_EQ(5.00, Location3.GetX());
    ASSERT_DOUBLE_EQ(10.0, Location3.GetY());

    //Operator==
    ASSERT_TRUE(Location1 == Location1);
    ASSERT_FALSE(Location1 == Location2);
    ASSERT_TRUE(Location2 == Location3);

    //Operator!=
    ASSERT_FALSE(Location1 != Location1);
    ASSERT_TRUE(Location1 != Location2);
    ASSERT_FALSE(Location2 != Location3);

    HGF2DLocation Location4(20.0, 20.0, pWorld);

    #ifdef WIP_IPPTEST_BUG_20
    ASSERT_TRUE(Location2 < Location4);
    ASSERT_FALSE(Location2 < Location2);
    #endif

    }
//==================================================================================
// HGF2DLocation        operator+(const HGF2DDisplacement& pi_rOffset) const;
// HGF2DLocation        operator+(const HGF2DDisplacement& pi_rOffset, const HGF2DLocation& pi_rLocation);
// HGF2DDisplacement    operator-(const HGF2DLocation& pi_rLocation) const;
// HGF2DLocation        operator-(const HGF2DDisplacement& pi_rOffset) const;
// HGF2DLocation        operator+=(const HGF2DDisplacement& pi_rOffset);
// HGF2DLocation        operator-=(const HGF2DDisplacement& pi_rOffset);
//==================================================================================
TEST_F (HGF2DLocationTester, OperatorTest2)
    {

    HGF2DLocation Location3(20.0, 20.0, pWorld);

    ASSERT_TRUE((Location2 + HGF2DDisplacement(5.0, 10.0)).IsEqualTo(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE((Location2 + HGF2DDisplacement(-5.0, -10.0)).IsEqualTo(HGF2DLocation(0.0, 0.0, pWorld)));

    HGF2DDisplacement Soustract1 = Location3 - Location2;
    ASSERT_DOUBLE_EQ(15.0, Soustract1.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Soustract1.GetDeltaY());

    HGF2DDisplacement Soustract2 = Location2 - Location3;
    ASSERT_DOUBLE_EQ(-15.0, Soustract2.GetDeltaX());
    ASSERT_DOUBLE_EQ(-10.0, Soustract2.GetDeltaY());

    ASSERT_TRUE((Location2 - HGF2DDisplacement(-5.0, -10.0)).IsEqualTo(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE((Location2 - HGF2DDisplacement(5.0, 10.0)).IsEqualTo(HGF2DLocation(0.0, 0.0, pWorld)));

    Location3 += HGF2DDisplacement(5.0, 10.0);
    ASSERT_DOUBLE_EQ(25.0, Location3.GetX());
    ASSERT_DOUBLE_EQ(30.0, Location3.GetY());

    Location3 -= HGF2DDisplacement(5.0, 10.0);
    ASSERT_DOUBLE_EQ(20.0, Location3.GetX());
    ASSERT_DOUBLE_EQ(20.0, Location3.GetY());

    }

//==================================================================================
// ChangeCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpSystem);
// GetCoordSys() const;
// SetCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpSystem);
// ExpressedIn(const HFCPtr<HGF2DCoordSys>& pi_rpSystem) const;
//==================================================================================
TEST_F (HGF2DLocationTester, CoordSysTest)
    {
       
    HGF2DStretch    MyStretch(HGF2DDisplacement(10.0, 0.0), 1.0, 1.0); 
    HFCPtr<HGF2DCoordSys> pWorldStretch = new HGF2DCoordSys(MyStretch, pWorld);

    ASSERT_EQ(pWorld, Location2.GetCoordSys());

    Location2.ChangeCoordSys(pWorldStretch);

    ASSERT_EQ(pWorldStretch, Location2.GetCoordSys());
    ASSERT_TRUE(Location2.IsEqualTo(HGF2DLocation(-5.0, 10.0, pWorldStretch)));
    ASSERT_TRUE(Location2.ExpressedIn(pWorld).IsEqualTo(HGF2DLocation(5.0, 10.0, pWorld)));

    Location2.SetCoordSys(pWorld);

    ASSERT_EQ(pWorld, Location2.GetCoordSys());
    ASSERT_TRUE(Location2.IsEqualTo(HGF2DLocation(-5.0, 10.0, pWorld)));
    ASSERT_TRUE(Location2.ExpressedIn(pWorld).IsEqualTo(HGF2DLocation(-5.0, 10.0, pWorld)));

    }

//==================================================================================
// IsEqualTo(const HGF2DLocation& pi_rObj) const;
// IsEqualTo(const HGF2DLocation& pi_rObj, double pi_Epsilon) const;
// IsEqualToAutoEpsilon(const HGF2DLocation& pi_rObj) const;
// IsEqualToSCS(const HGF2DLocation& pi_rObj) const;
// IsEqualToSCS(const HGF2DLocation& pi_rObj, double pi_Epsilon) const;
// IsEqualToAutoEpsilonSCS(const HGF2DLocation& pi_rObj) const;
//==================================================================================
TEST_F (HGF2DLocationTester, EqualTest)
    {

    HGF2DLocation Location3(20.0, 20.0, pWorld);
    HGF2DLocation Location4(5.0 + 2 * MYEPSILON, 10.0, pWorld);

    ASSERT_TRUE(Location2.IsEqualTo(Location2));
    ASSERT_TRUE(Location2.IsEqualToAutoEpsilon(Location2));
    ASSERT_TRUE(Location2.IsEqualToSCS(Location2));
    ASSERT_TRUE(Location2.IsEqualToAutoEpsilonSCS(Location2));

    ASSERT_FALSE(Location2.IsEqualTo(Location3));
    ASSERT_FALSE(Location2.IsEqualToAutoEpsilon(Location3));
    ASSERT_FALSE(Location2.IsEqualToSCS(Location3));
    ASSERT_FALSE(Location2.IsEqualToAutoEpsilonSCS(Location3));

    ASSERT_FALSE(Location2.IsEqualTo(Location4));
    ASSERT_TRUE(Location2.IsEqualToAutoEpsilon(Location4));
    ASSERT_FALSE(Location2.IsEqualToSCS(Location4));
    ASSERT_TRUE(Location2.IsEqualToAutoEpsilonSCS(Location4));

    }

//==================================================================================
// GetX() const;
// GetY() const;
// SetX(const HGFDistance&  pi_rX);
// SetY(const HGFDistance&  pi_rY);
// Set(const HGF2DLocation& pi_rLocation);
// GetPosition() const;
//==================================================================================
TEST_F (HGF2DLocationTester, GetSetTest)
    {

    ASSERT_DOUBLE_EQ(5.0, Location2.GetX());
    ASSERT_DOUBLE_EQ(10.0, Location2.GetY());

    Location2.SetX(20.0);
    Location2.SetY(25.0);

    ASSERT_DOUBLE_EQ(20.0, Location2.GetX());
    ASSERT_DOUBLE_EQ(25.0, Location2.GetY());

    Location1.Set(Location2);

    ASSERT_DOUBLE_EQ(20.0, Location1.GetX());
    ASSERT_DOUBLE_EQ(25.0, Location1.GetY());

    ASSERT_DOUBLE_EQ(20.0, Location1.GetPosition().GetX());
    ASSERT_DOUBLE_EQ(25.0, Location1.GetPosition().GetY());

    }