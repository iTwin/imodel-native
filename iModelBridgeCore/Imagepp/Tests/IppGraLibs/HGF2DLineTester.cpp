//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DLineTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "HGF2DLineTester.h"

HGF2DLineTester::HGF2DLineTester() 
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

    }

// THE PRESENT TEST CANNOT MAKE TESTS WITH NON-LINEAR TRANSFORMATION MODELS

//==================================================================================
// Construction tests
//==================================================================================
TEST_F (HGF2DLineTester, ConstructorsTest)
    {

    //With Locations
    HGF2DLine Line1(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    ASSERT_EQ(pWorld, Line1.GetCoordSys());
    ASSERT_NEAR(0.0, Line1.GetIntercept(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, Line1.GetSlope());

    //With Location + Bearing
    HGF2DLine Line2(HGF2DLocation(0.0, 0.0, pWorld), HGFBearing(PI/4));
    ASSERT_EQ(pWorld, Line2.GetCoordSys());
    ASSERT_NEAR(0.0, Line2.GetIntercept(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, Line2.GetSlope());

    //With Slope
    HGF2DLine Line3(pWorld, 1.0, 0.0);
    ASSERT_EQ(pWorld, Line3.GetCoordSys());
    ASSERT_NEAR(0.0, Line3.GetIntercept(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, Line3.GetSlope());

    //Copy Constructor
    HGF2DLine Line4(Line3);
    ASSERT_EQ(pWorld, Line4.GetCoordSys());
    ASSERT_NEAR(0.0, Line4.GetIntercept(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, Line4.GetSlope());

    }

//==================================================================================
// operator=(const HGF2DLiteExtent& pi_rObj);
//==================================================================================
TEST_F (HGF2DLineTester, OperatorTest)
    {

    HGF2DLine Line1(pWorld, 1.0, 0.0);
    HGF2DLine Line2(pSys1, 10.0, 10.0);

    Line2 = Line1;
    ASSERT_EQ(pWorld, Line2.GetCoordSys());
    ASSERT_NEAR(0.0, Line2.GetIntercept(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, Line2.GetSlope());

    }

//==================================================================================
// operator==(const HGF2DLiteExtent& pi_rObj) const;
// operator!=(const HGF2DLiteExtent& pi_rObj) const;
// IsEqualTo(const HGF2DLiteExtent& pi_rObj) const;
// IsEqualTo(const HGF2DLiteExtent& pi_rObj, double pi_Epsilon) const;
// IsEqualToAutoEpsilon(const HGF2DLine& pi_rObject) const;
//==================================================================================
TEST_F (HGF2DLineTester, CompareTest)
    {

    HGF2DLine Line1(pWorld, 1.0, 0.0);
    HGF2DLine Line2(pWorld, 1.0, 0.0);
    HGF2DLine Line3(pWorld, 1.0 + MYEPSILON, 0.0);
    HGF2DLine Line4(pWorld, 1.0, 0.0 + MYEPSILON);
    HGF2DLine Line5(pSys1, 1.0, 0.0);
    HGF2DLine Line6(pSys1, 10.0, 10.0);
    HGF2DLine Line7(pWorld, -1.0, 0.0);

    ASSERT_TRUE(Line1 == Line1);
    ASSERT_TRUE(Line1 == Line2);
    ASSERT_FALSE(Line1 == Line3);
    ASSERT_FALSE(Line1 == Line4);
    ASSERT_FALSE(Line1 == Line5);
    ASSERT_FALSE(Line1 == Line6);
    ASSERT_FALSE(Line1 == Line7);

    ASSERT_FALSE(Line1 != Line1);
    ASSERT_FALSE(Line1 != Line2);
    ASSERT_TRUE(Line1 != Line3);
    ASSERT_TRUE(Line1 != Line4);
    ASSERT_TRUE(Line1 != Line5);
    ASSERT_TRUE(Line1 != Line6);
    ASSERT_TRUE(Line1 != Line7);

    ASSERT_TRUE(Line1.IsEqualTo(Line1));
    ASSERT_TRUE(Line1.IsEqualTo(Line2));
    ASSERT_TRUE(Line1.IsEqualTo(Line3));
    ASSERT_TRUE(Line1.IsEqualTo(Line4));
    ASSERT_FALSE(Line1.IsEqualTo(Line5));
    ASSERT_FALSE(Line1.IsEqualTo(Line6));
    ASSERT_FALSE(Line1.IsEqualTo(Line7));

    ASSERT_TRUE(Line1.IsEqualTo(Line1, MYEPSILON));
    ASSERT_TRUE(Line1.IsEqualTo(Line2, MYEPSILON));
    ASSERT_TRUE(Line1.IsEqualTo(Line3, MYEPSILON));
    ASSERT_TRUE(Line1.IsEqualTo(Line4, MYEPSILON));
    ASSERT_FALSE(Line1.IsEqualTo(Line5, MYEPSILON));
    ASSERT_FALSE(Line1.IsEqualTo(Line6, MYEPSILON));
    ASSERT_FALSE(Line1.IsEqualTo(Line7, MYEPSILON));

    ASSERT_TRUE(Line1.IsEqualToAutoEpsilon(Line1));
    ASSERT_TRUE(Line1.IsEqualToAutoEpsilon(Line2));
    ASSERT_TRUE(Line1.IsEqualToAutoEpsilon(Line3));
    ASSERT_FALSE(Line1.IsEqualToAutoEpsilon(Line4));
    ASSERT_FALSE(Line1.IsEqualToAutoEpsilon(Line5));
    ASSERT_FALSE(Line1.IsEqualToAutoEpsilon(Line6));
    ASSERT_FALSE(Line1.IsEqualToAutoEpsilon(Line7));
  
    } 

//==================================================================================
// CalculateBearing() const
//==================================================================================
TEST_F (HGF2DLineTester, BearingTest)
    {

    HGF2DLine Line1(pWorld, 1.0, 0.0);
    HGF2DLine Line2(pSys1, 1.0, 0.0);
    HGF2DLine Line3(pWorld, -1.0, 0.0);
    HGF2DLine Line4(pWorld, 1.0, 30.0);
    HGF2DLine Line6(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(0.0, 10.0, pWorld));

    ASSERT_DOUBLE_EQ(PI/4,Line1.CalculateBearing().GetAngle());
    ASSERT_DOUBLE_EQ(PI/4, Line2.CalculateBearing().GetAngle());
    ASSERT_DOUBLE_EQ(-PI/4, Line3.CalculateBearing().GetAngle());
    ASSERT_DOUBLE_EQ(PI/4, Line4.CalculateBearing().GetAngle());
    ASSERT_DOUBLE_EQ(PI/2, Line6.CalculateBearing().GetAngle());

    }

//==================================================================================
// CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
// CalculateShortestDistance(const HGF2DLocation& pi_rPoint) const;
//==================================================================================
TEST_F (HGF2DLineTester, ClosestTest)
    {

    HGF2DLine Line1(pWorld, 1.0, 0.0);
    HGF2DLine Line2(pSys1, 1.0, 0.0);
    HGF2DLine Line3(pWorld, -1.0, 0.0);
    HGF2DLine Line4(pWorld, 1.0, 30.0);
    HGF2DLine Line6(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(0.0, 10.0, pWorld));

    ASSERT_NEAR(0.0, Line1.CalculateClosestPoint(HGF2DLocation(0.0, 0.0, pWorld)).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, Line2.CalculateClosestPoint(HGF2DLocation(0.0, 0.0, pSys1)).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, Line3.CalculateClosestPoint(HGF2DLocation(1.0, 1.0, pWorld)).GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(15.0, Line4.CalculateClosestPoint(HGF2DLocation(30.0, 30.0, pWorld)).GetX());
    ASSERT_NEAR(0.0, Line6.CalculateClosestPoint(HGF2DLocation(10.0, 10.0, pWorld)).GetX(), MYEPSILON);

    ASSERT_NEAR(0.0, Line1.CalculateClosestPoint(HGF2DLocation(0.0, 0.0, pWorld)).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, Line2.CalculateClosestPoint(HGF2DLocation(0.0, 0.0, pSys1)).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, Line3.CalculateClosestPoint(HGF2DLocation(1.0, 1.0, pWorld)).GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(45.0, Line4.CalculateClosestPoint(HGF2DLocation(30.0, 30.0, pWorld)).GetY());
    ASSERT_DOUBLE_EQ(10.0, Line6.CalculateClosestPoint(HGF2DLocation(10.0, 10.0, pWorld)).GetY());

    }

//==================================================================================
// IntersectLine(const HGF2DLine& pi_rLine,HGF2DLocation* po_pPoint)const;
//==================================================================================
TEST_F (HGF2DLineTester, IntersectLineTest)
    {

    HGF2DLine Line1(pWorld, 1.0, 0.0);
    HGF2DLine Line2(pSys1, 1.0, 0.0);
    HGF2DLine Line3(pWorld, -1.0, 0.0);
    HGF2DLine Line4(pWorld, 1.0, 30.0);
    HGF2DLine Line6(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(0.0, 10.0, pWorld));

    HGF2DLocation DumpLocation;

    ASSERT_EQ(HGF2DLine::PARALLEL, Line1.IntersectLine(Line1, &DumpLocation));
  
    ASSERT_EQ(HGF2DLine::CROSS_FOUND, Line1.IntersectLine(Line2, &DumpLocation));
    ASSERT_DOUBLE_EQ(15.119964160125436, DumpLocation.GetX());
    ASSERT_DOUBLE_EQ(15.119964160125436, DumpLocation.GetY());

    ASSERT_EQ(HGF2DLine::CROSS_FOUND, Line1.IntersectLine(Line3, &DumpLocation));
    ASSERT_NEAR(0.0, DumpLocation.GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, DumpLocation.GetY(), MYEPSILON);

    ASSERT_EQ(HGF2DLine::PARALLEL, Line1.IntersectLine(Line4, &DumpLocation));
 
    ASSERT_EQ(HGF2DLine::CROSS_FOUND, Line1.IntersectLine(Line6, &DumpLocation));
    ASSERT_NEAR(0.0, DumpLocation.GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, DumpLocation.GetY(), MYEPSILON);

    }

//==================================================================================
// GetIntercept() const;
// GetSlope () const;
//==================================================================================
TEST_F (HGF2DLineTester, GetTest)
    {

    HGF2DLine Line1(pWorld, 1.0, 0.0);
    HGF2DLine Line2(pSys1, 1.0, 0.0);
    HGF2DLine Line3(pWorld, -1.0, 0.0);
    HGF2DLine Line4(pWorld, 1.0, 30.0);
    HGF2DLine Line6(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(0.0, 10.0, pWorld));
    HGF2DLine Line7(pWorld, 0.5, 45.0);

    ASSERT_NEAR(0.0, Line1.GetIntercept(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, Line1.GetSlope());

    ASSERT_NEAR(0.0, Line2.GetIntercept(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, Line2.GetSlope());

    ASSERT_NEAR(0.0, Line3.GetIntercept(), MYEPSILON);
    ASSERT_DOUBLE_EQ(-1.0, Line3.GetSlope());

    ASSERT_DOUBLE_EQ(30.0, Line4.GetIntercept());
    ASSERT_DOUBLE_EQ(1.00, Line4.GetSlope());

    ASSERT_DOUBLE_EQ(45.0, Line7.GetIntercept());
    ASSERT_DOUBLE_EQ(0.50, Line7.GetSlope());

    }

//==================================================================================
// IsParallelTo(const HGF2DLine& pi_rLine) const;
// IsVertical() const;
//==================================================================================
TEST_F (HGF2DLineTester, IsTest)
    {

    HGF2DLine Line1(pWorld, 1.0, 0.0);
    HGF2DLine Line2(pSys1, 1.0, 0.0);
    HGF2DLine Line3(pWorld, -1.0, 0.0);
    HGF2DLine Line4(pWorld, 1.0, 30.0);
    HGF2DLine Line6(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(0.0, 10.0, pWorld));
    HGF2DLine Line7(HGF2DLocation(30.0, 0.0, pWorld), HGF2DLocation(30.0, 10.0, pWorld));

    ASSERT_TRUE(Line1.IsParallelTo(Line1));
    ASSERT_FALSE(Line1.IsParallelTo(Line2));
    ASSERT_FALSE(Line1.IsParallelTo(Line3));
    ASSERT_TRUE(Line1.IsParallelTo(Line4));
    ASSERT_FALSE(Line1.IsParallelTo(Line6));
    ASSERT_TRUE(Line6.IsParallelTo(Line7));

    ASSERT_FALSE(Line1.IsVertical());
    ASSERT_FALSE(Line2.IsVertical());
    ASSERT_FALSE(Line3.IsVertical());
    ASSERT_FALSE(Line4.IsVertical());
    ASSERT_TRUE(Line6.IsVertical());
    ASSERT_TRUE(Line7.IsVertical());

    }

//==================================================================================
// ChangeCoordSys(const HFCPtr<HGF2DCoordSys>&    pi_rpCoordSys);
// GetCoordSys() const;
// SetCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
//==================================================================================
TEST_F (HGF2DLineTester, CoordSysTest)
    {

    HGF2DTranslation Translation(HGF2DDisplacement(10.0, 0.0));
    HFCPtr<HGF2DCoordSys> TranslationCoord = new HGF2DCoordSys(Translation, pWorld);

    HGF2DLine Line1(pWorld, 1.0, 0.0);

    Line1.ChangeCoordSys(TranslationCoord);
    ASSERT_EQ(TranslationCoord, Line1.GetCoordSys());
    ASSERT_DOUBLE_EQ(10.0, Line1.GetIntercept());
    ASSERT_DOUBLE_EQ(1.00, Line1.GetSlope());

    Line1.SetCoordSys(pWorld);
    ASSERT_EQ(pWorld, Line1.GetCoordSys());
    ASSERT_DOUBLE_EQ(10.0, Line1.GetIntercept());
    ASSERT_DOUBLE_EQ(1.00, Line1.GetSlope());

    }