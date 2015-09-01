//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DAffineTester.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DAffineTester
//-----------------------------------------------------------------------------

#pragma once

#define MYEPSILON   (HNumeric<double>::GLOBAL_EPSILON())
 
// Preparation of required environement
class HGF2DAffineTester : public testing::Test
    {   

protected :
  
    HGF2DAffineTester();
    ~HGF2DAffineTester() {};

    //Definition of Affine
    HGF2DAffine         DefaultAffine;
    HGF2DAffine         Affine2;

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