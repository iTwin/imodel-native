//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGFModelCompositionTester.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGFModelCompositionTester
//-----------------------------------------------------------------------------

#pragma once

#define MYEPSILON   (HNumeric<double>::GLOBAL_EPSILON())
 
// Preparation of required environement
class HGFModelCompositionTester : public testing::Test 
    {   

protected :

    HGFModelCompositionTester();

    HGF2DAffine     Model1;
    HGF2DStretch    Model2;
    HGF2DProjective Model4;
    HGF2DSimilitude Model3;

    HFCPtr<HGF2DTransfoModel> pResultModel1;
    HFCPtr<HGF2DTransfoModel> pResultModel2;
    HFCPtr<HGF2DTransfoModel> pResultModel3;

    HFCMatrix<3,3> Matrix1;
    HFCMatrix<3,3> Matrix2;
    HFCMatrix<3,3> Matrix3;

    };