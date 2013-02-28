/*--------------------------------------------------------------------------------------+
|
|     $Source: test/scenario/ValueTests.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include "TestFixture.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

struct ValueTests : ECTestFixture
    {
    void compareDateTimes(SystemTime dateTime1, SystemTime dateTime2)
        {
        EXPECT_EQ (dateTime1.wYear, dateTime2.wYear);
        EXPECT_EQ (dateTime1.wMonth, dateTime2.wMonth);
        EXPECT_EQ (dateTime1.wDay, dateTime2.wDay);
        EXPECT_EQ (dateTime1.wHour, dateTime2.wHour);
        EXPECT_EQ (dateTime1.wMinute, dateTime2.wMinute);
        EXPECT_EQ (dateTime1.wSecond, dateTime2.wSecond);
        EXPECT_EQ (dateTime1.wMilliseconds, dateTime2.wMilliseconds);
        }
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
//SetIGeometry uses SetBinary which throws exception for this type
TEST_F(ValueTests, DISABLED_IGeometrySetGet)
    {
    byte binary[4] = {0x00, 0x01, 0x02, 0x03};
    ECValue value;
    EXPECT_EQ (value.SetIGeometry(binary, sizeof(binary)), ECOBJECTS_STATUS_Success);
    
    EXPECT_TRUE (value.IsIGeometry());
    EXPECT_FALSE (value.IsBinary());
    
    size_t sizeOut;
    const byte *binaryOut = value.GetIGeometry(sizeOut);
    EXPECT_TRUE (binaryOut != NULL);
    EXPECT_EQ (sizeOut, 4);
    
    for (size_t i=0; i<sizeOut; i++)
        EXPECT_EQ (binaryOut[i], binary[i]);
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ValueTests, ECValueToString)
    {
    byte binary[4] = {0x00, 0x01, 0x02, 0x03};
    SystemTime dateTime = SystemTime(2013, 2, 14, 9, 58, 17, 456);
    DPoint2d point2d = {123.456, 456.789};
    DPoint3d point3d = {1.2, -3.4, 5.6};
    WChar *unichar = L"TestingTesting\x017D\x06C6\x0F3D\x132B\x25E7\x277C\x28BE";
    
    ECValue value;
    
    EXPECT_EQ (value.SetString(unichar), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), unichar);
    
    EXPECT_EQ (value.SetBinary(binary, sizeof(binary)), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), L"AEgADw==");  //this is probably BASE64 encoded
    
    EXPECT_EQ (value.SetBoolean(false), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), L"False");
    
    EXPECT_EQ (value.SetDateTime(dateTime), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), L"#2013/2/14-9:58:17:456#");  //Fix if conversion is region specific
    
    EXPECT_EQ (value.SetDouble(3.14159265359), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), L"3.14159265359");
    
    EXPECT_EQ (value.SetInteger(-255), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), L"-255");
    
    EXPECT_EQ (value.SetLong(1234567890L), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), L"1234567890");
    
    EXPECT_EQ (value.SetPoint2D(point2d), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), L"123.456,456.789");
    
    EXPECT_EQ (value.SetPoint3D(point3d), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), L"1.2,-3.4,5.6");
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ValueTests, DISABLED_IGeometryToString)
    {
    byte binary[4] = {0x00, 0x01, 0x02, 0x03};
    ECValue value;
    
    EXPECT_EQ (value.SetIGeometry(binary, sizeof(binary)), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), L"AEgADw==");  //this is probably BASE64 encoded
    };
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
//Read only flag is not honored
TEST_F(ValueTests, DISABLED_ValueReadOnly)
    {
    ECSchemaPtr schema;
    ECClassP base;
    
    ECSchema::CreateSchema(schema, L"TestSchema", 5, 5);
    schema->CreateClass(base, L"BaseClass");

    PrimitiveECPropertyP readOnlyProp;
    base->CreatePrimitiveProperty(readOnlyProp, L"readOnlyProp");
    readOnlyProp->SetIsReadOnly(true);

    EXPECT_TRUE(base->GetPropertyP (L"readOnlyProp")->GetIsReadOnly());
    
    IECInstancePtr instance = base->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_EQ (instance->SetValue (L"readOnlyProp", ECValue(L"Some value")), ECOBJECTS_STATUS_UnableToSetReadOnlyInstance);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ValueTests, DateTimeConversions)
    {
    SystemTime dateTime = SystemTime(2013, 2, 14, 9, 58, 17, 456);
    ECValue value = ECValue(dateTime);
    
    EXPECT_TRUE (value.IsDateTime());
    compareDateTimes(value.GetDateTime(), dateTime);
    EXPECT_EQ (value.GetDateTimeTicks(), 634964326974560000LL);
    EXPECT_EQ (value.GetDateTimeUnixMillis(), 1360835897456L);
    
    dateTime.InitFromUnixMillis(1260835897957L);
    value.SetDateTime(dateTime);
    
    compareDateTimes(value.GetDateTime(), dateTime);
    EXPECT_EQ (value.GetDateTimeTicks(), 633964326979570000LL);
    EXPECT_EQ (value.GetDateTimeUnixMillis(), 1260835897957L);

    _FILETIME fileTime;
    fileTime.dwLowDateTime  = (633974326929510000LL - 504911232000000000LL) & 0xffffffff; // = 1994575472L
    fileTime.dwHighDateTime = (633974326929510000LL - 504911232000000000LL) >> 32;        // = 30049843L
    dateTime.InitFromFileTime(fileTime);
    value.SetDateTime(dateTime);
    
    compareDateTimes(value.GetDateTime(), dateTime);
    EXPECT_EQ (value.GetDateTimeTicks(), 633974326929510000LL);
    EXPECT_EQ (value.GetDateTimeUnixMillis(), 1261835892951);
    }
    
END_BENTLEY_ECOBJECT_NAMESPACE