/*--------------------------------------------------------------------------------------+
|
|     $Source: test/scenario/ValueTests.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include "TestFixture.h"
#include <regex>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

struct ValueTests : ECTestFixture
    {
    void compareDateTimes(DateTimeCR dateTime1, DateTimeCR dateTime2)
        {
        EXPECT_TRUE (dateTime1.Compare (dateTime2, false));
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
//SetIGeometry uses SetBinary which throws exception for this type
TEST_F(ValueTests, DISABLED_IGeometrySetGet)
    {
    const byte binary[] = {0x00, 0x01, 0x02, 0x03};
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
    const byte binary[] = {0x00, 0x01, 0x02, 0x03};
    DateTime dateTime = DateTime(DateTime::DATETIMEKIND_Utc, 2013, 2, 14, 9, 58, 17, 4560000);
    DPoint2d point2d = {123.456, 456.789};
    DPoint3d point3d = {1.2, -3.4, 5.6};
    WChar *unichar = L"TestingTesting\x017D\x06C6\x0F3D\x132B\x25E7\x277C\x28BE";
    
    ECValue value;
    
    EXPECT_EQ (value.SetString(unichar), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), unichar);
    
    EXPECT_EQ (value.SetBinary(binary, sizeof(binary)), ECOBJECTS_STATUS_Success);
    //EXPECT_STREQ (value.ToString().c_str(), L"AEgADw==");  //this is probably BASE64 encoded
    EXPECT_TRUE (std::regex_match (value.ToString().c_str(), std::wregex(L"^AEgAD.==$")));
    
    EXPECT_EQ (value.SetBoolean(false), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), L"False");
    
    EXPECT_EQ (value.SetDateTime(dateTime), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), L"2013-02-14T09:58:17.456Z");  //Fix if conversion is region specific
    
    EXPECT_EQ (value.SetDouble(3.14159265359), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), L"3.1415926535900001");
    
    EXPECT_EQ (value.SetInteger(-255), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), L"-255");
    
    EXPECT_EQ (value.SetLong(1234567890L), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), L"1234567890");
    
    EXPECT_EQ (value.SetPoint2D(point2d), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), L"123.456,456.78899999999999");
    
    EXPECT_EQ (value.SetPoint3D(point3d), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.ToString().c_str(), L"1.2,-3.3999999999999999,5.5999999999999996");
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ValueTests, DISABLED_IGeometryToString)
    {
    byte binary[] = {0x00, 0x01, 0x02, 0x03};
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
    DateTime dateTime = DateTime(DateTime::DATETIMEKIND_Utc, 2013, 2, 14, 9, 58, 17, 4560000);
    ECValue value = ECValue(dateTime);
    
    EXPECT_TRUE (value.IsDateTime());
    compareDateTimes(value.GetDateTime(), dateTime);
    EXPECT_EQ (value.GetDateTimeTicks(), 634964326974560000LL);
    EXPECT_EQ (value.GetDateTimeUnixMillis(), 1360835897456L);
    }
    
END_BENTLEY_ECOBJECT_NAMESPACE
