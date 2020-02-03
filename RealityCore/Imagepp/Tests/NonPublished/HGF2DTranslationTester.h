//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DTranslationTester
//-----------------------------------------------------------------------------

#pragma once

#define MYEPSILON   (HNumeric<double>::GLOBAL_EPSILON())
 
// Preparation of required environement
class HGF2DTranslationTester : public testing::Test 
    {   

protected :
  
    HGF2DTranslationTester();
    ~HGF2DTranslationTester() {};

    //Definition of Similitude
    HGF2DTranslation DefaultTranslation;
    HGF2DTranslation Translation2;

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
