/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublishedScenario/ValueTests.cpp $
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
    
void checkBinary(WCharCP text, ECValue &value)
    {
    const byte binary[] = {0x00, 0x01, 0x02, 0x03};
    
    EXPECT_EQ (value.SetString(text), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (value.ConvertToPrimitiveType(PRIMITIVETYPE_Binary));
    EXPECT_TRUE (value.IsBinary());
    size_t sizeOut;
    const byte *binaryOut = value.GetBinary(sizeOut);
    EXPECT_EQ (sizeOut, sizeof(binary));
    for (size_t i=0; i<sizeOut; i++)
        EXPECT_EQ (binaryOut[i], binary[i]);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ValueTests, StringToECValue)
    {
    ECValue value;
    WCharCP unichar = L"TestingTesting\x017D\x06C6\x0F3D\x132B\x25E7\x277C\x28BE";
    DPoint2d point2d = {123.456, 456.789};
    DPoint3d point3d = {1.2, -3.4, 5.6};
    
    EXPECT_EQ (value.SetString(unichar), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (value.ConvertToPrimitiveType(PRIMITIVETYPE_String));
    EXPECT_TRUE (value.IsString());
    EXPECT_STREQ (value.GetString(), unichar);
    
    //All these strings decode into same byte array
    //Byte array is onverted into one of these strings
    checkBinary(L"AEgADg==", value);
    checkBinary(L"AEgADw==", value);
    checkBinary(L"AEgADA==", value);
    
    WCharCP boolString[] = {L"True", L"False", L"true", L"FALSE", L"1", L"0"};
    bool boolResults[] = {true, false, true, false, true, false};
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
//IGeometry is not handled in ConvertToPrimitiveType
TEST_F(ValueTests, DISABLED_StringToIGeometry)
    {
    ECValue value;
    const byte binary[] = {0x00, 0x01, 0x02, 0x03};
    
    EXPECT_EQ (value.SetString(L"AEgADw=="), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (value.ConvertToPrimitiveType(PRIMITIVETYPE_IGeometry));
    EXPECT_TRUE (value.IsBinary());
    size_t sizeOut;
    const byte *binaryOut = value.GetBinary(sizeOut);
    EXPECT_EQ (sizeOut, sizeof(binary));
    for (size_t i=0; i<sizeOut; i++)
        EXPECT_EQ (binaryOut[i], binary[i]);
    }
    
END_BENTLEY_ECOBJECT_NAMESPACE