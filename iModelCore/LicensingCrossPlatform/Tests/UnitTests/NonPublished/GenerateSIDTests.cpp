/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/NonPublished/GenerateSIDTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "GenerateSIDTests.h"

#include "../../../Licensing/GenerateSID.h"

USING_NAMESPACE_BENTLEY_LICENSING

USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

TEST_F(GenerateSIDTests, GetUserSID_Success)
    {
    GenerateSID sidData;

    Utf8String userSid = sidData.GetUserSID("zzhao", "4l847324");
    EXPECT_STREQ("+/E4BmbAucpPVn0wXz3MC0A/Xuc=", userSid.c_str());

    userSid = sidData.GetUserSID("mm0705", "4l847324");
    EXPECT_STREQ("ChkkJYQbBInmryubxMYHVdkY2to=", userSid.c_str());

    userSid = sidData.GetUserSID("max688\\system", "max688");
    EXPECT_STREQ("+4mIysGbq7IcK4LTc2bQE45i4qA=", userSid.c_str());

    userSid = sidData.GetUserSID("bgeperpc183$", "4l847324");
    EXPECT_STREQ("QXy4e5tP4Cpnq4AZ1EVA515Qmuk=", userSid.c_str());

    userSid = sidData.GetUserSID("212342135", "4l847324");
    EXPECT_STREQ("fl0J2Jl0inwIr0ZFqnqUr5O4upE=", userSid.c_str());

    userSid = sidData.GetUserSID("ma-awiah.gool", "4l847324");
    EXPECT_STREQ("EtwaRdNWpHLa/ALZ8sobsFCN86M=", userSid.c_str());

    userSid = sidData.GetUserSID("koskinen kimmo", "4l847324");
    EXPECT_STREQ("M+PjUBFe/01g9Qa6yqXqLm1G2po=", userSid.c_str());

    userSid = sidData.GetUserSID("ist-mgeniser\\administrator", "ist-mgeniser");
    EXPECT_STREQ("g/BpaH9X/itc8n1fcswn1ElAABk=", userSid.c_str());

    userSid = sidData.GetUserSID("mcenteno - b71e795ae21e6409854c9af8a728a91c6f7a014b", "4l847324");
    EXPECT_STREQ("+GpDnp0N7BKK/ZZTMlfLisPBScs=", userSid.c_str());

    userSid = sidData.GetUserSID("a032449@bentley.pw--dotb6pwhq.executive.stateofwv.gov:pw-primary", "4l847324");
    EXPECT_STREQ("xiNyWNpLqRmhXBZAjk+TmPMXlGs=", userSid.c_str());

    userSid = sidData.GetUserSID("pppilot/admin", "pppilot");
    EXPECT_STREQ("o2lsnhueDApmvSbm31mh3aetYnc=", userSid.c_str());

    userSid = sidData.GetUserSID(u8"王臣的电脑\\administrator", u8"王臣的电脑");
    EXPECT_STREQ("DQfUXnx9hC/TSn3Szfa2vbpueGM=", userSid.c_str());

    userSid = sidData.GetUserSID(u8"前　文化財調査屋外４", "4l847324");
    EXPECT_STREQ("Ne4IlXAElFOQI18gK5H88JjtUXU=", userSid.c_str());

    userSid = sidData.GetUserSID(u8"한정섭_강원_변전운영팀\\administrator", u8"한정섭_강원_변전운영팀");
    EXPECT_STREQ("hm0WHY+6ZMNa7bp+sV9fvHkayyk=", userSid.c_str());

    userSid = sidData.GetUserSID(u8"东北院-2011-20131111mj-2018022607", "4l847324");
    EXPECT_STREQ("E0W5Edm6LQ618bNNPywvWk3W9FM=", userSid.c_str());

    userSid = sidData.GetUserSID(u8"内蒙院-hxiaobo-pc-2018030101", "4l847324");
    EXPECT_STREQ("9rMuYaBDgrST33FOXdc+e1hPJHc=", userSid.c_str());

    userSid = sidData.GetUserSID(u8"ఏగనోకగీోల ఖదతతగసోీతో", "4l847324");
    EXPECT_STREQ("JaCGmuCJSx0g74OlDvFHdi/XGLc=", userSid.c_str());

    userSid = sidData.GetUserSID(u8"ربٌيإرزَقنيفرَححہةتج", "41847324");
    EXPECT_STREQ("Okt+QPozXsq9JSwThiukMgT8qUw=", userSid.c_str());

    userSid = sidData.GetUserSID(u8"الكوثر cs", "4l847324");
    EXPECT_STREQ("Mlb0jjWDaxIaXJjSSjKTOkvXti4=", userSid.c_str());

    userSid = sidData.GetUserSID(u8"משתמש-pc\\system", u8"משתמש-pc");
    EXPECT_STREQ("lUp5pyZ8CLkiePx0Es/CqL0z3NU=", userSid.c_str());

    userSid = sidData.GetUserSID(u8"אליאס עווד", "41847324");
    EXPECT_STREQ("Er3Hb1PxScXIYWM3c0Ofu870UQU=", userSid.c_str());

    userSid = sidData.GetUserSID(u8"русин д.@pw.pw_sl", "4l847324");
    EXPECT_STREQ("C2oBqzXh1x7ix4JjRr5UnYkzoto=", userSid.c_str());

    userSid = sidData.GetUserSID(u8"михалыч147", "4l847324");
    EXPECT_STREQ("Nhv2u2EDX/S3vzbanLMeveTCBDk=", userSid.c_str());

    userSid = sidData.GetUserSID(u8"罗昭强2017\\administrator", u8"罗昭强2017");
    EXPECT_STREQ("2Wv0hoD3KPvt9IH6cgeY6gzsIJI=", userSid.c_str());
    }

TEST_F(GenerateSIDTests, GetMachineSID_Success)
    {
    GenerateSID sidData;

    Utf8String userSid = sidData.GetMachineSID("4l847324");
    EXPECT_STREQ("Kqv8zD6ot0T0sSm067K4MpD00fU=", userSid.c_str());

    userSid = sidData.GetMachineSID("max688");
    EXPECT_STREQ("IrN/VPiaTCHXvdp4W17KGivR7Vc=", userSid.c_str());

    userSid = sidData.GetMachineSID("ist-mgeniser");
    EXPECT_STREQ("N26opoH+ECDDOuLkZ0YFgLyWnI4=", userSid.c_str());

    userSid = sidData.GetMachineSID("pppilot");
    EXPECT_STREQ("1d8cRwzPsOAn/S72I8Iop0210hk=", userSid.c_str());

    userSid = sidData.GetMachineSID(u8"王臣的电脑");
    EXPECT_STREQ("hfTrolGJVWB7MabttDUPDB2pRxo=", userSid.c_str());

    userSid = sidData.GetMachineSID(u8"한정섭_강원_변전운영팀");
    EXPECT_STREQ("9Lz4H6wQrVpjUdoJyUn1eKfw/mM=", userSid.c_str());

    userSid = sidData.GetMachineSID(u8"משתמש-pc");
    EXPECT_STREQ("Ys5d6wGqN0MbbXqGLwoRH92vh64=", userSid.c_str());

    userSid = sidData.GetMachineSID(u8"罗昭强2017");
    EXPECT_STREQ("KTzxGTusaH991SWjgQVYiGEKqNQ=", userSid.c_str());
    }
