//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGFResolutionTester.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGFResolutionTester
//-----------------------------------------------------------------------------

#pragma once
 
// Preparation of required environement
class HGFResolutionTester : public testing::Test 
    {   
protected :

    unsigned short SubImage;
    double   Resolution;
    uint32_t    Width;
    uint32_t    Height;

    };