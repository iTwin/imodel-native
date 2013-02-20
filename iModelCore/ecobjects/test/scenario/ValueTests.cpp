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

struct ValueTests : ECTestFixture {};
struct ValueAccessorTests : ECTestFixture {};
    
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
//ConvertToPrimitiveType is not in Published API
TEST_F(ValueTests, StringToECValue)
    {
    ECValue value;
    WCharCP unichar = L"TestingTesting\x017D\x06C6\x0F3D\x132B\x25E7\x277C\x28BE";
    byte binary[4] = {0x00, 0x01, 0x02, 0x03};
    DPoint2d point2d = {123.456, 456.789};
    DPoint3d point3d = {1.2, -3.4, 5.6};
    
    EXPECT_EQ (value.SetString(unichar), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (value.ConvertToPrimitiveType(PRIMITIVETYPE_String));
    EXPECT_TRUE (value.IsString());
    EXPECT_STREQ (value.GetString(), unichar);
    
    EXPECT_EQ (value.SetString(L"AEgADw=="), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (value.ConvertToPrimitiveType(PRIMITIVETYPE_Binary));
    EXPECT_TRUE (value.IsBinary());
    size_t sizeOut;
    const byte *binaryOut = value.GetBinary(sizeOut);
    EXPECT_EQ (sizeOut, 4);
    for (size_t i=0; i<sizeOut; i++)
        EXPECT_EQ (binaryOut[i], binary[i]);
    
    WCharCP boolString[6] = {L"True", L"False", L"true", L"FALSE", L"1", L"0"};
    bool boolResults[6] = {true, false, true, false, true, false};
    for (int i=0; i<sizeof(boolResults); i++)
        {
        EXPECT_EQ (value.SetString(boolString[i]), ECOBJECTS_STATUS_Success);
        EXPECT_TRUE (value.ConvertToPrimitiveType(PRIMITIVETYPE_Boolean));
        EXPECT_TRUE (value.IsBoolean());
        EXPECT_TRUE (value.GetBoolean() == boolResults[i]);
        }
    
    EXPECT_EQ (value.SetString(L"634964326974560000"), ECOBJECTS_STATUS_Success);  //Human readable string is not handled
    EXPECT_TRUE (value.ConvertToPrimitiveType(PRIMITIVETYPE_DateTime));
    EXPECT_TRUE (value.IsDateTime());
    EXPECT_EQ (value.GetDateTime().wYear, 2013);
    EXPECT_EQ (value.GetDateTime().wMonth, 2);
    EXPECT_EQ (value.GetDateTime().wDay, 14);
    EXPECT_EQ (value.GetDateTime().wHour, 9);
    EXPECT_EQ (value.GetDateTime().wMinute, 58);
    EXPECT_EQ (value.GetDateTime().wSecond, 17);
    EXPECT_EQ (value.GetDateTime().wMilliseconds, 456);
    
    EXPECT_EQ (value.SetString(L"-3.14159265359"), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (value.ConvertToPrimitiveType(PRIMITIVETYPE_Double));
    EXPECT_TRUE (value.IsDouble());
    EXPECT_EQ (value.GetDouble(), -3.14159265359);
    
    EXPECT_EQ (value.SetString(L"256"), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (value.ConvertToPrimitiveType(PRIMITIVETYPE_Integer));
    EXPECT_TRUE (value.IsInteger());
    EXPECT_EQ (value.GetInteger(), 256);
    
    EXPECT_EQ (value.SetString(L"9876543210"), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (value.ConvertToPrimitiveType(PRIMITIVETYPE_Long));
    EXPECT_TRUE (value.IsLong());
    EXPECT_EQ (value.GetLong(), 9876543210L);
    
    EXPECT_EQ (value.SetString(L"123.456,456.789"), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (value.ConvertToPrimitiveType(PRIMITIVETYPE_Point2D));
    EXPECT_TRUE (value.IsPoint2D());
    DPoint2d point2dOut = value.GetPoint2D();
    EXPECT_EQ (0, memcmp(&point2d, &point2dOut, sizeof(point2d)));
    
    EXPECT_EQ (value.SetString(L"1.2,-3.4,5.6"), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (value.ConvertToPrimitiveType(PRIMITIVETYPE_Point3D));
    EXPECT_TRUE (value.IsPoint3D());
    DPoint3d point3dOut = value.GetPoint3D();
    EXPECT_EQ (0, memcmp(&point3d, &point3dOut, sizeof(point3d)));
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
//ConvertToPrimitiveType is not in Published API
//IGeometry is not handled in ConvertToPrimitiveType
TEST_F(ValueTests, DISABLED_StringToIGeometry)
    {
    ECValue value;
    byte binary[4] = {0x00, 0x01, 0x02, 0x03};
    
    EXPECT_EQ (value.SetString(L"AEgADw=="), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (value.ConvertToPrimitiveType(PRIMITIVETYPE_IGeometry));
    EXPECT_TRUE (value.IsBinary());
    size_t sizeOut;
    const byte *binaryOut = value.GetBinary(sizeOut);
    EXPECT_EQ (sizeOut, 4);
    for (size_t i=0; i<sizeOut; i++)
        EXPECT_EQ (binaryOut[i], binary[i]);
    }
 
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
    
END_BENTLEY_ECOBJECT_NAMESPACE