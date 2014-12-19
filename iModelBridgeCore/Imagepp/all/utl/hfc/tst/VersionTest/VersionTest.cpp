//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/tst/VersionTest/VersionTest.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCVersion.h>

const uint32_t v0[] = {0};
const uint32_t v1[] = {1};
const uint32_t v12[] = {1, 2};
const uint32_t v101[] = {1, 0, 1};
const uint32_t v1002[] = {1, 0, 0, 2};
const uint32_t v14001[] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};


void main(void)
    {
    printf("0 == 0 : %lu\r\n", HFCVersion() == HFCVersion());
    printf("0 != 0 : %lu\r\n", HFCVersion() != HFCVersion());
    printf("0 <  0 : %lu\r\n", HFCVersion() <  HFCVersion());
    printf("0 <= 0 : %lu\r\n", HFCVersion() <= HFCVersion());
    printf("0 >  0 : %lu\r\n", HFCVersion() >  HFCVersion());
    printf("0 >= 0 : %lu\r\n", HFCVersion() >= HFCVersion());
    printf("\r\n");

    printf("0 == 1 : %lu\r\n", HFCVersion("", "", 1, v0) == HFCVersion("", "", 1, v1));
    printf("0 != 1 : %lu\r\n", HFCVersion("", "", 1, v0) != HFCVersion("", "", 1, v1));
    printf("0 <  1 : %lu\r\n", HFCVersion("", "", 1, v0) <  HFCVersion("", "", 1, v1));
    printf("0 <= 1 : %lu\r\n", HFCVersion("", "", 1, v0) <= HFCVersion("", "", 1, v1));
    printf("0 >  1 : %lu\r\n", HFCVersion("", "", 1, v0) >  HFCVersion("", "", 1, v1));
    printf("0 >= 1 : %lu\r\n", HFCVersion("", "", 1, v0) >= HFCVersion("", "", 1, v1));
    printf("\r\n");

    printf("1 == 0 : %lu\r\n", HFCVersion("", "", 1, v1) == HFCVersion("", "", 1, v0));
    printf("1 != 0 : %lu\r\n", HFCVersion("", "", 1, v1) != HFCVersion("", "", 1, v0));
    printf("1 <  0 : %lu\r\n", HFCVersion("", "", 1, v1) <  HFCVersion("", "", 1, v0));
    printf("1 <= 0 : %lu\r\n", HFCVersion("", "", 1, v1) <= HFCVersion("", "", 1, v0));
    printf("1 >  0 : %lu\r\n", HFCVersion("", "", 1, v1) >  HFCVersion("", "", 1, v0));
    printf("1 >= 0 : %lu\r\n", HFCVersion("", "", 1, v1) >= HFCVersion("", "", 1, v0));
    printf("\r\n");

    printf("1.2 == 1 : %lu\r\n", HFCVersion("", "", 1, v12) == HFCVersion("", "", 1, v1));
    printf("1.2 != 1 : %lu\r\n", HFCVersion("", "", 2, v12) != HFCVersion("", "", 1, v1));
    printf("1.2 <  1 : %lu\r\n", HFCVersion("", "", 2, v12) <  HFCVersion("", "", 1, v1));
    printf("1.2 <= 1 : %lu\r\n", HFCVersion("", "", 2, v12) <= HFCVersion("", "", 1, v1));
    printf("1.2 >  1 : %lu\r\n", HFCVersion("", "", 2, v12) >  HFCVersion("", "", 1, v1));
    printf("1.2 >= 1 : %lu\r\n", HFCVersion("", "", 2, v12) >= HFCVersion("", "", 1, v1));
    printf("\r\n");

    printf("1.0.1 == 1.0.0.2 : %lu\r\n", HFCVersion("", "", 3, v101) == HFCVersion("", "", 4, v1002));
    printf("1.0.1 != 1.0.0.2 : %lu\r\n", HFCVersion("", "", 3, v101) != HFCVersion("", "", 4, v1002));
    printf("1.0.1 <  1.0.0.2 : %lu\r\n", HFCVersion("", "", 3, v101) <  HFCVersion("", "", 4, v1002));
    printf("1.0.1 <= 1.0.0.2 : %lu\r\n", HFCVersion("", "", 3, v101) <= HFCVersion("", "", 4, v1002));
    printf("1.0.1 >  1.0.0.2 : %lu\r\n", HFCVersion("", "", 3, v101) >  HFCVersion("", "", 4, v1002));
    printf("1.0.1 >= 1.0.0.2 : %lu\r\n", HFCVersion("", "", 3, v101) >= HFCVersion("", "", 4, v1002));
    printf("\r\n");

    printf("1.0 == 1. 40x0 . 1: %lu\r\n", HFCVersion("", "", 1, v1) == HFCVersion("", "", 42, v14001));
    printf("1.0 != 1. 40x0 . 1: %lu\r\n", HFCVersion("", "", 1, v1) != HFCVersion("", "", 42, v14001));
    printf("1.0 <  1. 40x0 . 1: %lu\r\n", HFCVersion("", "", 1, v1) <  HFCVersion("", "", 42, v14001));
    printf("1.0 <= 1. 40x0 . 1: %lu\r\n", HFCVersion("", "", 1, v1) <= HFCVersion("", "", 42, v14001));
    printf("1.0 >  1. 40x0 . 1: %lu\r\n", HFCVersion("", "", 1, v1) >  HFCVersion("", "", 42, v14001));
    printf("1.0 >= 1. 40x0 . 1: %lu\r\n", HFCVersion("", "", 1, v1) >= HFCVersion("", "", 42, v14001));
    printf("\r\n");
    }