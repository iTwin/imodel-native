//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGFBearingTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "HGFBearingTester.h"

HGFBearingTester::HGFBearingTester()
    {

    //Angle
    MyAngle = PI/4;

    //Bearing
    Bearing1 = HGFBearing(MyAngle);

    }

//==================================================================================
// Construction test  
//==================================================================================
TEST_F(HGFBearingTester, ConstructionTest)
    {
    
    ASSERT_NEAR(0.0, DefaultBearing.GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(MyAngle, Bearing1.GetAngle());

    //Copy Constructor
    HGFBearing Bearing2(Bearing1);
    ASSERT_DOUBLE_EQ(MyAngle, Bearing2.GetAngle());

    // Assignement operator
    HGFBearing Bearing3 = Bearing1;
    ASSERT_DOUBLE_EQ(MyAngle, Bearing3.GetAngle());

    }

//==================================================================================
// Compare operation
//==================================================================================
TEST_F(HGFBearingTester, CompareTest)
    {
    
    ASSERT_TRUE(DefaultBearing != Bearing1); 
    ASSERT_TRUE(Bearing1 == Bearing1); 
    ASSERT_TRUE(HGFBearing(Bearing1) == Bearing1);

    }

//==================================================================================
// Approximative compare operations
//    bool           IsEqualTo(const HGFBearing& pi_rObj) const;
//    bool           IsEqualTo(const HGFBearing& pi_rObj, double pi_Epsilon) const;
//    bool           IsEqualToAutoEpsilon(const HGFBearing& pi_rObj) const;
//    bool           IsBearingWithinSweep(const HGFAngle& pi_rSweep,
//                                        const HGFBearing& pi_rBearing) const;
//    bool           IsBearingWithinSweep(const HGFAngle& pi_rSweep,
//                                        const HGFBearing& pi_rBearing) const;
//==================================================================================
TEST_F(HGFBearingTester, ApproximativeCompareTest)
    {
    
    ASSERT_TRUE(DefaultBearing.IsEqualTo(DefaultBearing));
    ASSERT_TRUE(DefaultBearing.IsEqualTo(DefaultBearing, MYEPSILON));
    ASSERT_TRUE(DefaultBearing.IsEqualToAutoEpsilon(DefaultBearing));

    ASSERT_TRUE(Bearing1.IsEqualTo(Bearing1));
    ASSERT_TRUE(Bearing1.IsEqualTo(Bearing1, MYEPSILON));
    ASSERT_TRUE(Bearing1.IsEqualToAutoEpsilon(Bearing1));

    ASSERT_FALSE(Bearing1.IsEqualTo(DefaultBearing));
    ASSERT_FALSE(Bearing1.IsEqualTo(DefaultBearing, MYEPSILON));
    ASSERT_FALSE(Bearing1.IsEqualToAutoEpsilon(DefaultBearing));

    HGFBearing Bearing2(PI/4 + MYEPSILON);

    ASSERT_TRUE(Bearing1.IsEqualTo(Bearing2));
    ASSERT_TRUE(Bearing1.IsEqualTo(Bearing2, MYEPSILON));
    ASSERT_FALSE(Bearing1.IsEqualToAutoEpsilon(Bearing2));

    ASSERT_TRUE(DefaultBearing.IsBearingWithinSweep(0, DefaultBearing));
    ASSERT_TRUE(Bearing1.IsBearingWithinSweep(-PI/4, DefaultBearing));
    ASSERT_FALSE(Bearing1.IsBearingWithinSweep(-PI/4 + MYEPSILON, DefaultBearing));

    }

//==================================================================================
// CalculateTrigoAngle() const
//==================================================================================
TEST_F(HGFBearingTester, CalculateTrigoAngleTest)
    {

    ASSERT_NEAR(0.0, DefaultBearing.CalculateTrigoAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(PI/4, Bearing1.CalculateTrigoAngle());

    HGFBearing Bearing2(8 * PI);

    ASSERT_NEAR(0.0, Bearing2.CalculateTrigoAngle(), MYEPSILON);

    HGFBearing Bearing3(PI * 6 + PI);

    ASSERT_DOUBLE_EQ(PI, Bearing3.CalculateTrigoAngle());

    }

//==================================================================================
// const HGFAngle& GetAngle() const;
// void            SetAngle(const HGFAngle& pi_rAngle);
//==================================================================================
TEST_F(HGFBearingTester, AngleTest)
    {

    ASSERT_NEAR(0.0, DefaultBearing.GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(PI / 4, Bearing1.GetAngle());

    DefaultBearing.SetAngle(PI);
    ASSERT_DOUBLE_EQ(PI, DefaultBearing.GetAngle());

    }

//==================================================================================
// Arithmetic operations
// HGFBearing      operator+(const HGFAngle& pi_rAngle) const;
// HGFBearing      operator-(const HGFAngle& pi_rAngle) const;
// HGFAngle        operator-(const HGFBearing& pi_rObj) const;
// HGFBearing&     operator+=(const HGFAngle& pi_rAngle);
// HGFBearing&     operator-=(const HGFAngle& pi_rAngle);
//==================================================================================
TEST_F(HGFBearingTester, ArithmeticTest)
    {

    HGFBearing Add = DefaultBearing + 0.0;
    ASSERT_NEAR(0.0, Add.GetAngle(), MYEPSILON);

    Add = DefaultBearing + PI;
    ASSERT_DOUBLE_EQ(PI, Add.GetAngle());

    Add += -PI;
    ASSERT_NEAR(0.0, Add.GetAngle(), MYEPSILON);

    HGFBearing Soustract = DefaultBearing - 0.0;
    ASSERT_NEAR(0.0, Soustract.GetAngle(), MYEPSILON);

    Soustract = DefaultBearing - PI;
    ASSERT_DOUBLE_EQ(-PI, Soustract.GetAngle());

    Soustract -= PI;
    ASSERT_DOUBLE_EQ(-2 * PI, Soustract.GetAngle());

    ASSERT_NEAR(0.0, Soustract - Add, MYEPSILON);

    }