/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Tests/UnitTests/Published/GenerateSIDTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    EXPECT_TRUE(userSid.Equals("+/E4BmbAucpPVn0wXz3MC0A/Xuc="));
    
    userSid = sidData.GetUserSID("mm0705", "4l847324");
    EXPECT_TRUE(userSid.Equals("ChkkJYQbBInmryubxMYHVdkY2to="));

    userSid = sidData.GetUserSID("max688\\system", "max688");
    EXPECT_TRUE(userSid.Equals("+4mIysGbq7IcK4LTc2bQE45i4qA="));

    userSid = sidData.GetUserSID("bgeperpc183$", "4l847324");
    EXPECT_TRUE(userSid.Equals("QXy4e5tP4Cpnq4AZ1EVA515Qmuk="));

    userSid = sidData.GetUserSID("212342135", "4l847324");
    EXPECT_TRUE(userSid.Equals("fl0J2Jl0inwIr0ZFqnqUr5O4upE="));

    userSid = sidData.GetUserSID("ma-awiah.gool", "4l847324");
    EXPECT_TRUE(userSid.Equals("EtwaRdNWpHLa/ALZ8sobsFCN86M="));

    userSid = sidData.GetUserSID("koskinen kimmo", "4l847324");
    EXPECT_TRUE(userSid.Equals("M+PjUBFe/01g9Qa6yqXqLm1G2po="));

    userSid = sidData.GetUserSID("ist-mgeniser\\administrator", "ist-mgeniser");
    EXPECT_TRUE(userSid.Equals("g/BpaH9X/itc8n1fcswn1ElAABk="));

    userSid = sidData.GetUserSID("mcenteno - b71e795ae21e6409854c9af8a728a91c6f7a014b", "4l847324");
    EXPECT_TRUE(userSid.Equals("+GpDnp0N7BKK/ZZTMlfLisPBScs="));

    userSid = sidData.GetUserSID("a032449@bentley.pw--dotb6pwhq.executive.stateofwv.gov:pw-primary", "4l847324");
    EXPECT_TRUE(userSid.Equals("xiNyWNpLqRmhXBZAjk+TmPMXlGs="));
    
    userSid = sidData.GetUserSID("pppilot/admin", "pppilot");
    EXPECT_TRUE(userSid.Equals("o2lsnhueDApmvSbm31mh3aetYnc="));

    userSid = sidData.GetUserSID(u8"王臣的电脑\\administrator", u8"王臣的电脑");
    EXPECT_TRUE(userSid.Equals("DQfUXnx9hC/TSn3Szfa2vbpueGM="));
    
    userSid = sidData.GetUserSID(u8"前　文化財調査屋外４", "4l847324");
    EXPECT_TRUE(userSid.Equals("Ne4IlXAElFOQI18gK5H88JjtUXU="));

    userSid = sidData.GetUserSID(u8"한정섭_강원_변전운영팀\\administrator", u8"한정섭_강원_변전운영팀");
    EXPECT_TRUE(userSid.Equals("hm0WHY+6ZMNa7bp+sV9fvHkayyk="));

    userSid = sidData.GetUserSID(u8"东北院-2011-20131111mj-2018022607", "4l847324");
    EXPECT_TRUE(userSid.Equals("E0W5Edm6LQ618bNNPywvWk3W9FM="));

    userSid = sidData.GetUserSID(u8"内蒙院-hxiaobo-pc-2018030101", "4l847324");
    EXPECT_TRUE(userSid.Equals("9rMuYaBDgrST33FOXdc+e1hPJHc="));

    userSid = sidData.GetUserSID(u8"ఏగనోకగీోల ఖదతతగసోీతో", "4l847324");
    EXPECT_TRUE(userSid.Equals("JaCGmuCJSx0g74OlDvFHdi/XGLc="));

    userSid = sidData.GetUserSID(u8"ربٌيإرزَقنيفرَححہةتج", "41847324");
    EXPECT_TRUE(userSid.Equals("Okt+QPozXsq9JSwThiukMgT8qUw="));

    userSid = sidData.GetUserSID(u8"الكوثر cs", "4l847324");
    EXPECT_TRUE(userSid.Equals("Mlb0jjWDaxIaXJjSSjKTOkvXti4="));

    userSid = sidData.GetUserSID(u8"משתמש-pc\\system", u8"משתמש-pc");
    EXPECT_TRUE(userSid.Equals("lUp5pyZ8CLkiePx0Es/CqL0z3NU="));

    userSid = sidData.GetUserSID(u8"אליאס עווד", "41847324");
    EXPECT_TRUE(userSid.Equals("Er3Hb1PxScXIYWM3c0Ofu870UQU="));

    userSid = sidData.GetUserSID(u8"русин д.@pw.pw_sl", "4l847324");
    EXPECT_TRUE(userSid.Equals("C2oBqzXh1x7ix4JjRr5UnYkzoto="));

    userSid = sidData.GetUserSID(u8"михалыч147", "4l847324");
    EXPECT_TRUE(userSid.Equals("Nhv2u2EDX/S3vzbanLMeveTCBDk="));

    userSid = sidData.GetUserSID(u8"罗昭强2017\\administrator", u8"罗昭强2017");
    EXPECT_TRUE(userSid.Equals("2Wv0hoD3KPvt9IH6cgeY6gzsIJI="));
    }
