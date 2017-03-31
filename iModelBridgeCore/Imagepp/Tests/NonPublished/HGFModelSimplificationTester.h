//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/NonPublished/HGFModelSimplificationTester.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
    HFCPtr<HGF2DSimilitude>   m_pSimilitude1;
    HFCPtr<HGF2DSimilitude>   m_pSimilitude6;

    //Stretch
    HFCPtr<HGF2DStretch>      m_pStretch2;

    //Projective
    HFCPtr<HGF2DProjective>   m_pProjective10;
    };
