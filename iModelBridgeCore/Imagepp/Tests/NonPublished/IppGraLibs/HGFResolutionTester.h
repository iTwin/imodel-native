//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/NonPublished/IppGraLibs/HGFResolutionTester.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGFResolutionTester
//-----------------------------------------------------------------------------

#pragma once
 
// Preparation of required environement
class HGFResolutionTester : public testing::Test 
    {   
protected :

    uint16_t SubImage;
    double   Resolution;
    uint32_t    Width;
    uint32_t    Height;

    };