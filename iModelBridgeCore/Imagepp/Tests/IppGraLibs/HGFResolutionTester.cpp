//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGFResolutionTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "HGFResolutionTester.h"

//==================================================================================
// TestGeneral
//==================================================================================
TEST_F (HGFResolutionTester, TestGeneral)
    {

    HGFResolutionDescriptor ResolutionDescriptor(1025, 1025, 256, 256);

    SubImage = 0;
    ResolutionDescriptor.GetDescription(SubImage, &Resolution, &Width, &Height);
    ASSERT_DOUBLE_EQ(1.0000, Resolution);
    ASSERT_DOUBLE_EQ(1025.0, Width);
    ASSERT_DOUBLE_EQ(1025.0, Height);

    SubImage = 1;
    ResolutionDescriptor.GetDescription(SubImage, &Resolution, &Width, &Height);
    ASSERT_DOUBLE_EQ(0.500, Resolution);
    ASSERT_DOUBLE_EQ(513.0, Width);
    ASSERT_DOUBLE_EQ(513.0, Height);

    SubImage = 2;
    ResolutionDescriptor.GetDescription(SubImage, &Resolution, &Width, &Height);
    ASSERT_DOUBLE_EQ(0.250, Resolution);
    ASSERT_DOUBLE_EQ(257.0, Width);
    ASSERT_DOUBLE_EQ(257.0, Height);

    SubImage = 3;
    ResolutionDescriptor.GetDescription(SubImage, &Resolution, &Width, &Height);
    ASSERT_DOUBLE_EQ(0.125, Resolution);
    ASSERT_DOUBLE_EQ(129.0, Width);
    ASSERT_DOUBLE_EQ(129.0, Height);

    }