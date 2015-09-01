//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DExtentTester.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DExtentTester
//-----------------------------------------------------------------------------

#pragma once

#define MYEPSILON   (HNumeric<double>::GLOBAL_EPSILON())
 
// Preparation of required environement
class HGF2DExtentTester : public testing::Test 
    {   

protected :

    HGF2DExtentTester();
    ~HGF2DExtentTester() {};

    //Extent
    HGF2DExtent Extent1;
    HGF2DExtent Extent2;
    HGF2DExtent Extent3;

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