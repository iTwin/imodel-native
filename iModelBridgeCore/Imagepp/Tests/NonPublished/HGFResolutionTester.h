//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
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
