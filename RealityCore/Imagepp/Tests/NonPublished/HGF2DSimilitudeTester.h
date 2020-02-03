//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DSimilitudeTester
//-----------------------------------------------------------------------------

#pragma once

#define MYEPSILON   (HNumeric<double>::GLOBAL_EPSILON())
  
// Preparation of required environement
class HGF2DSimilitudeTester : public testing::Test
    {   

protected :
  
    HGF2DSimilitudeTester();
    ~HGF2DSimilitudeTester() {};

    //Definition of Similitude
    HGF2DSimilitude DefaultSimilitude;
    HGF2DSimilitude Similitude2;

    //General
    HGF2DDisplacement   Translation;
    double              Rotation;
    double              ScalingX;
    double              ScalingY;
    double              Anortho;
    HGF2DAffine         Affine1;  
    HGF2DIdentity       MyIdentity; 

    // COORDINATE SYSTEMS
    HFCPtr<HGF2DCoordSys>   pWorld;
    HFCPtr<HGF2DCoordSys>   pSys1;

    };
