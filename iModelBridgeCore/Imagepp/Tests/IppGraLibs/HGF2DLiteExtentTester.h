//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DLiteExtentTester.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DLiteExtentTester
//-----------------------------------------------------------------------------

#pragma once

#define MYEPSILON   (HNumeric<double>::GLOBAL_EPSILON())
 
// Preparation of required environement
class HGF2DLiteExtentTester : public testing::Test 
    {   

protected :

    HGF2DLiteExtentTester();
    ~HGF2DLiteExtentTester() {};

    //Extent
    HGF2DLiteExtent Extent1;
    HGF2DLiteExtent Extent2;
    HGF2DLiteExtent Extent3;

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