//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGFModelSimplificationTester.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGFModelSimplificationTester
//-----------------------------------------------------------------------------

#pragma once

#define MYEPSILON   (HNumeric<double>::GLOBAL_EPSILON())
 
// Preparation of required environement
class HGFModelSimplificationTester : public testing::Test 
    {   

protected :

    HGFModelSimplificationTester();

    HFCPtr<HGF2DTransfoModel> pResult;

    //Similitude
    HGF2DSimilitude   Similitude1;
    HGF2DSimilitude   Similitude6;

    //Stretch
    HGF2DStretch      Stretch2;

    //Projective
    HGF2DProjective   Projective10;
    HFCMatrix<3, 3> TheMatrix;

    };