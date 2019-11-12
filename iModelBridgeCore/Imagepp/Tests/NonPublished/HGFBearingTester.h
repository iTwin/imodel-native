//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGFBearingTester
//-----------------------------------------------------------------------------

#pragma once

#define MYEPSILON   (HNumeric<double>::GLOBAL_EPSILON())
 
// Preparation of required environement
class HGFBearingTester : public testing::Test 
    {   

protected :

    HGFBearingTester();

    //Bearing
    HGFBearing DefaultBearing;
    HGFBearing Bearing1;
   
    //Angle
    double MyAngle;
    
    };
