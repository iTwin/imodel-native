/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/ValueTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
using namespace ECN;

#include <regex>

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct ValueTests : ECTestFixture
    {
    void compareDateTimes(DateTimeCR dateTime1, DateTimeCR dateTime2)
        {
        EXPECT_TRUE (dateTime1.Equals (dateTime2, false));
        EXPECT_EQ (dateTime1.GetYear(), dateTime2.GetYear());
        EXPECT_EQ (dateTime1.GetMonth(), dateTime2.GetMonth());
        EXPECT_EQ (dateTime1.GetDay(), dateTime2.GetDay());
        EXPECT_EQ (dateTime1.GetHour(), dateTime2.GetHour());
        EXPECT_EQ (dateTime1.GetMinute(), dateTime2.GetMinute());
        EXPECT_EQ (dateTime1.GetSecond(), dateTime2.GetSecond());
        EXPECT_EQ (dateTime1.GetMillisecond(), dateTime2.GetMillisecond());
        EXPECT_EQ (dateTime1.GetHectoNanosecond(), dateTime2.GetHectoNanosecond());
        }
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ValueTests, ECValueToString)
    {
    const Byte binary[] = {0x00, 0x01, 0x02, 0x03};
    DateTime dateTime = DateTime(DateTime::Kind::Utc, 2013, 2, 14, 9, 58, 17, 4560000);
    DPoint2d point2d = {123.456, 456.789};
    DPoint3d point3d = {1.2, -3.4, 5.6};
    WChar *unichar = L"TestingTesting\x017D\x06C6\x0F3D\x132B\x25E7\x277C\x28BE";
    Utf8String unicharUtf8;
    BeStringUtilities::WCharToUtf8(unicharUtf8, unichar);
    ECValue value;
    
    EXPECT_EQ (value.SetUtf8CP(unicharUtf8.c_str()), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), unicharUtf8.c_str());
    
    EXPECT_EQ (value.SetBinary(binary, sizeof(binary)), ECOBJECTS_STATUS_Success);
    //EXPECT_STREQ (value.ToString().c_str(), L"AEgADw==");  //this is probably BASE64 encoded
    EXPECT_TRUE (std::regex_match (value.ToString().c_str(), std::regex("^AEgAD.==$")));
    
    EXPECT_EQ (value.SetBoolean(false), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), "False");
    
    EXPECT_EQ (value.SetDateTime(dateTime), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), "2013-02-14T09:58:17.456Z");  //Fix if conversion is region specific
    
    EXPECT_EQ (value.SetDouble(3.14159265359), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), "3.1415926535900001");
    
    EXPECT_EQ (value.SetInteger(-255), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), "-255");
    
    EXPECT_EQ (value.SetLong(1234567890L), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), "1234567890");
    
    EXPECT_EQ (value.SetPoint2D(point2d), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), "123.456,456.78899999999999");
    
    EXPECT_EQ (value.SetPoint3D(point3d), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), "1.2,-3.3999999999999999,5.5999999999999996");
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ValueTests, ValueReadOnly)
    {
    ECSchemaPtr schema;
    ECClassP base;
    
    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    schema->CreateClass(base, "BaseClass");

    PrimitiveECPropertyP readOnlyProp;
    base->CreatePrimitiveProperty(readOnlyProp, "readOnlyProp");
    readOnlyProp->SetIsReadOnly(true);

    EXPECT_TRUE(base->GetPropertyP ("readOnlyProp")->GetIsReadOnly());
    
    IECInstancePtr instance = base->GetDefaultStandaloneEnabler()->CreateInstance();
    //since the instance has no value initially, it can be set.This was done so that instances could be deserialized even if they had readonly property.This is the same as in the managed API.
    EXPECT_EQ (instance->SetValue ("readOnlyProp", ECValue ("Some value")), ECOBJECTS_STATUS_Success);
    }

END_BENTLEY_ECN_TEST_NAMESPACE
