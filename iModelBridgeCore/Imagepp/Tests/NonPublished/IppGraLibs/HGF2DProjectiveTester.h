//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/NonPublished/IppGraLibs/HGF2DProjectiveTester.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DProjectiveTester
//-----------------------------------------------------------------------------

#pragma once

#define MYEPSILON   (HNumeric<double>::GLOBAL_EPSILON())
 
// Preparation of required environement
class HGF2DProjectiveTester : public testing::Test
    {   

protected :

    HGF2DProjectiveTester();
    ~HGF2DProjectiveTester() {};

    //Projective
    HGF2DProjective DefaultProjective;
    HGF2DProjective Projective2;

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