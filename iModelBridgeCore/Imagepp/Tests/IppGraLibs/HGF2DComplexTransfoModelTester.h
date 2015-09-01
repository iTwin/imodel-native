//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DComplexTransfoModelTester.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DComplexTransfoModelTester
//-----------------------------------------------------------------------------

#pragma once

#define MYEPSILON   (HNumeric<double>::GLOBAL_EPSILON())
 
// Preparation of required environement
class HGF2DComplexTransfoModelTester : public testing::Test 
    {   

protected :

    HGF2DComplexTransfoModelTester();
    ~HGF2DComplexTransfoModelTester() {};

    //General
    HGF2DDisplacement   Displacement;
    double              Rotation;
    double              ScalingX;
    double              ScalingY;
    double              Scaling;
    double              Anortho;
    HGF2DIdentity       MyIdentity; 

    // COORDINATE SYSTEMS
    HFCPtr<HGF2DCoordSys>   pWorld;
    HFCPtr<HGF2DCoordSys>   pSys1;

    //Transformation 
    HGF2DAffine         Affine;
    HGF2DHelmert        Helmert;
    HGF2DProjective     Projective;
    HGF2DSimilitude     Similitude;
    HGF2DStretch        Stretch;
    HGF2DTranslation    Translation;

    };