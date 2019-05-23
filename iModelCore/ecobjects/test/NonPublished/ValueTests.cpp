/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include <regex>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct ValueTests : ECTestFixture
    {
    void checkBinary (Utf8CP text, ECValue &value)
        {
        const Byte binary[] = { 0x00, 0x01, 0x02, 0x03 };

        EXPECT_EQ (value.SetUtf8CP (text), SUCCESS);
        EXPECT_TRUE (value.ConvertToPrimitiveType (PRIMITIVETYPE_Binary));
        EXPECT_TRUE (value.IsBinary ());
        size_t sizeOut;
        const Byte *binaryOut = value.GetBinary (sizeOut);
        EXPECT_EQ (sizeOut, sizeof(binary));
        for (size_t i = 0; i<sizeOut; i++)
            EXPECT_EQ (binaryOut[i], binary[i]);
        }
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ValueTests, ECValueToString)
    {
    const Byte binary[] = {0x00, 0x01, 0x02, 0x03};
    DateTime dateTime = DateTime(DateTime::Kind::Utc, 2013, 2, 14, 9, 58, 17, 456);
    DPoint2d point2d = {123.456, 456.789};
    DPoint3d point3d = {1.2, -3.4, 5.6};
    WChar const*unichar = L"TestingTesting\x017D\x06C6\x0F3D\x132B\x25E7\x277C\x28BE";
    Utf8String unicharUtf8;
    BeStringUtilities::WCharToUtf8(unicharUtf8, unichar);
    ECValue value;
    
    EXPECT_EQ (value.SetUtf8CP(unicharUtf8.c_str()), SUCCESS);
    EXPECT_STREQ (value.ToString().c_str(), unicharUtf8.c_str());
    
    EXPECT_EQ (value.SetBinary(binary, sizeof(binary)), SUCCESS);
    //EXPECT_STREQ (value.ToString().c_str(), L"AEgADw==");  //this is probably BASE64 encoded
    EXPECT_TRUE (std::regex_match (value.ToString().c_str(), std::regex("^AAECA.==$")));
    
    EXPECT_EQ (value.SetBoolean(false), SUCCESS);
    EXPECT_STREQ (value.ToString().c_str(), "False");
    
    EXPECT_EQ (value.SetDateTime(dateTime), SUCCESS);
    EXPECT_STREQ (value.ToString().c_str(), "2013-02-14T09:58:17.456Z");  //Fix if conversion is region specific
    
    EXPECT_EQ (value.SetDouble(3.14159265359), SUCCESS);
    EXPECT_STREQ (value.ToString().c_str(), "3.1415926535900001");
    
    EXPECT_EQ (value.SetInteger(-255), SUCCESS);
    EXPECT_STREQ (value.ToString().c_str(), "-255");
    
    EXPECT_EQ (value.SetLong(1234567890L), SUCCESS);
    EXPECT_STREQ (value.ToString().c_str(), "1234567890");
    
    EXPECT_EQ (value.SetPoint2d(point2d), SUCCESS);
    EXPECT_STREQ (value.ToString().c_str(), "123.456,456.78899999999999");
    
    EXPECT_EQ (value.SetPoint3d(point3d), SUCCESS);
    EXPECT_STREQ (value.ToString().c_str(), "1.2,-3.3999999999999999,5.5999999999999996");
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ValueTests, ValueReadOnly)
    {
    ECSchemaPtr schema;
    ECEntityClassP base;
    
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    schema->CreateEntityClass(base, "BaseClass");

    PrimitiveECPropertyP readOnlyProp;
    base->CreatePrimitiveProperty(readOnlyProp, "readOnlyProp");
    readOnlyProp->SetIsReadOnly(true);

    EXPECT_TRUE(base->GetPropertyP ("readOnlyProp")->GetIsReadOnly());
    
    IECInstancePtr instance = base->GetDefaultStandaloneEnabler()->CreateInstance();
    //since the instance has no value initially, it can be set.This was done so that instances could be deserialized even if they had readonly property.This is the same as in the managed API.
    EXPECT_EQ (instance->SetValue ("readOnlyProp", ECValue ("Some value")), ECObjectsStatus::Success);
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
    Utf8String unicharutf8;
    BeStringUtilities::WCharToUtf8(unicharutf8, unichar);

    EXPECT_EQ (value.SetUtf8CP(unicharutf8.c_str()), SUCCESS);
    EXPECT_TRUE (value.ConvertToPrimitiveType(PRIMITIVETYPE_String));
    EXPECT_TRUE (value.IsString());
    EXPECT_STREQ (value.GetUtf8CP(), unicharutf8.c_str());
    
    checkBinary("AAECAw==", value);
    
    Utf8CP boolString[] = {"True", "False", "true", "FALSE", "1", "0"};
    bool boolResults[] = {true, false, true, false, true, false};
    for (int i=0; i<sizeof(boolResults); i++)
        {
        EXPECT_EQ (value.SetUtf8CP(boolString[i]), SUCCESS);
        EXPECT_TRUE (value.ConvertToPrimitiveType(PRIMITIVETYPE_Boolean));
        EXPECT_TRUE (value.IsBoolean());
        EXPECT_TRUE (value.GetBoolean() == boolResults[i]);
        }
    
    EXPECT_EQ (value.SetUtf8CP("634964326974560000"), SUCCESS);  //Human readable string is not handled
    EXPECT_TRUE (value.ConvertToPrimitiveType(PRIMITIVETYPE_DateTime));
    EXPECT_TRUE (value.IsDateTime());
    EXPECT_EQ (value.GetDateTime().GetYear(), 2013);
    EXPECT_EQ (value.GetDateTime().GetMonth(), 2);
    EXPECT_EQ (value.GetDateTime().GetDay(), 14);
    EXPECT_EQ (value.GetDateTime().GetHour(), 9);
    EXPECT_EQ (value.GetDateTime().GetMinute(), 58);
    EXPECT_EQ (value.GetDateTime().GetSecond(), 17);
    EXPECT_EQ (value.GetDateTime().GetMillisecond(), 456);
    
    EXPECT_EQ (value.SetUtf8CP("-3.14159265359"), SUCCESS);
    EXPECT_TRUE (value.ConvertToPrimitiveType(PRIMITIVETYPE_Double));
    EXPECT_TRUE (value.IsDouble());
    EXPECT_EQ (value.GetDouble(), -3.14159265359);
    
    EXPECT_EQ (value.SetUtf8CP("256"), SUCCESS);
    EXPECT_TRUE (value.ConvertToPrimitiveType(PRIMITIVETYPE_Integer));
    EXPECT_TRUE (value.IsInteger());
    EXPECT_EQ (value.GetInteger(), 256);
    
    EXPECT_EQ (value.SetUtf8CP("9876543210"), SUCCESS);
    EXPECT_TRUE (value.ConvertToPrimitiveType(PRIMITIVETYPE_Long));
    EXPECT_TRUE (value.IsLong());
    EXPECT_EQ (value.GetLong(), 9876543210L);
    
    EXPECT_EQ (value.SetUtf8CP("123.456,456.789"), SUCCESS);
    EXPECT_TRUE (value.ConvertToPrimitiveType(PRIMITIVETYPE_Point2d));
    EXPECT_TRUE (value.IsPoint2d());
    DPoint2d point2dOut = value.GetPoint2d();
    EXPECT_EQ (0, memcmp(&point2d, &point2dOut, sizeof(point2d)));
    
    EXPECT_EQ (value.SetUtf8CP("1.2,-3.4,5.6"), SUCCESS);
    EXPECT_TRUE (value.ConvertToPrimitiveType(PRIMITIVETYPE_Point3d));
    EXPECT_TRUE (value.IsPoint3d());
    DPoint3d point3dOut = value.GetPoint3d();
    EXPECT_EQ (0, memcmp(&point3d, &point3dOut, sizeof(point3d)));
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ValueTests, StringToIGeometry)
    {
    ECValue value;
    const Byte binary[] = {0x00, 0x01, 0x02, 0x03};
    
    EXPECT_EQ (value.SetUtf8CP("AAECAw=="), SUCCESS);
    EXPECT_TRUE (value.ConvertToPrimitiveType(PRIMITIVETYPE_IGeometry));
    EXPECT_TRUE (value.IsBinary());
    size_t sizeOut;
    const Byte *binaryOut = value.GetBinary(sizeOut);
    EXPECT_EQ (sizeOut, sizeof(binary));
    for (size_t i=0; i<sizeOut; i++)
        EXPECT_EQ (binaryOut[i], binary[i]);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   09/14
//+---------------+---------------+---------------+---------------+---------------+------
void AssertStringOwnership(WCharCP strW, Utf8CP strUtf8, bool makeCopy, bool expectedOwnsWCharCP, bool expectedOwnsUtf8CP)
    {
    ASSERT_TRUE(!WString::IsNullOrEmpty(strW) || !Utf8String::IsNullOrEmpty(strUtf8));
    enum class OriginalECValueStringEncoding
        {
        WChar,
        Utf8
        };

    auto createECValue = [] (ECValueR value, WCharCP strW, Utf8CP strUtf8, bool makeCopy)
        {
        if (strW != nullptr)
            {
            value.SetWCharCP(strW, makeCopy);
            return OriginalECValueStringEncoding::WChar;
            }
        else
            {
            value.SetUtf8CP(strUtf8, makeCopy);
            return OriginalECValueStringEncoding::Utf8;
            }
        };

    ECValue v1;
    createECValue(v1, strW, strUtf8, makeCopy);
    ASSERT_EQ(expectedOwnsWCharCP, v1.OwnsWCharCP());
    ASSERT_EQ(expectedOwnsUtf8CP, v1.OwnsUtf8CP());

    //now call GetXX methods which changes ownership
    //(use a new ECValue for each as it changes its state)
    ECValue v2;
    auto encodingWhenCreated = createECValue(v2, strW, strUtf8, makeCopy);
    v2.GetWCharCP();
    if (encodingWhenCreated == OriginalECValueStringEncoding::WChar)
        //if value was created with WChar, ownership depends on makecopy flag used at creation time
        ASSERT_EQ(makeCopy, v2.OwnsWCharCP());
    else
        ASSERT_TRUE(v2.OwnsWCharCP());

    //other encodings should not be affected yet
    ASSERT_EQ(expectedOwnsUtf8CP, v2.OwnsUtf8CP());

    ECValue v3;
    encodingWhenCreated = createECValue(v3, strW, strUtf8, makeCopy);
    v3.GetUtf8CP();
    if (encodingWhenCreated == OriginalECValueStringEncoding::Utf8)
        //if value was created with Utf8, ownership depends on makecopy flag used at creation time
        ASSERT_EQ(makeCopy, v3.OwnsUtf8CP());
    else
        ASSERT_TRUE(v3.OwnsUtf8CP());

    //other encodings should not be affected yet
    ASSERT_EQ(expectedOwnsWCharCP, v3.OwnsWCharCP());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   09/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECValueTests, StringOwnership)
    {
    //This ATP does only test the WChar and Utf8CP ownerships. It doesn't include
    //the Utf16CP flavor because the behavior for it is platform-dependent.
    //The main use case is to test UTF-8 ownership anyways.
    WCharCP strW = L"WChar";
    AssertStringOwnership(strW, nullptr, false, //create ECValue which doesn't own the string
                          false, //expected value for OwnsWCharCP
                          false);//expected value for OwnsUtf8CP
    AssertStringOwnership(strW, nullptr, true, //create ECValue which owns the string
                          true, //expected value for OwnsWCharCP
                          false);//expected value for OwnsUtf8CP

    Utf8CP strUtf8 = "Utf8CP";
    AssertStringOwnership(nullptr, strUtf8, false, //create ECValue which doesn't own the string
                          false, //expected value for OwnsWCharCP
                          false); // expected value for OwnsUtf8CP
    AssertStringOwnership(nullptr, strUtf8, true, //create ECValue which owns the string
                          false, //expected value for OwnsWCharCP
                          true); //expected value for OwnsUtf8CP
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Gintaras.Volkvicius 11/18
//---------------------------------------------------------------------------------------
TEST_F(ValueTests, DateTimeTest)
    {
    // Arrange

    DateTime dateTime1(2018, 11, 1);
    DateTime dateTime2(DateTime::Kind::Unspecified, 2018, 11, 1, 0, 0);

    DateTime dateTime3(DateTime::Kind::Unspecified, 2018, 11, 1, 14, 14);
    DateTime dateTime4(DateTime::Kind::Local, 2018, 11, 1, 14, 14);
    DateTime dateTime5(DateTime::Kind::Utc, 2018, 11, 1, 14, 14);

    DateTime dateTime6 = DateTime::CreateTimeOfDay(14, 23, 30);
    DateTime dateTime7(DateTime::Kind::Utc, 2000, 1, 1, 14, 23, 30);
    DateTime dateTime8(DateTime::Kind::Utc, 2000, 1, 2, 14, 23, 30);

    DateTime dateTime9(DateTime::Kind::Unspecified, 2018, 11, 1, 24, 0);
    DateTime dateTime10(DateTime::Kind::Unspecified, 2018, 11, 2, 0, 0);

    DateTime dateTime11 = DateTime::CreateTimeOfDay(24, 0, 0);
    DateTime dateTime12 = DateTime::CreateTimeOfDay(0, 0, 0);

    // Act & Assert

    ECValue value1(dateTime1);
    ECValue value2(dateTime2);
    EXPECT_EQ(value1, value2) << value1.ToString() << " and " << value2.ToString();

    ECValue value3(dateTime3);
    ECValue value4(dateTime4);
    ECValue value5(dateTime5);
    EXPECT_NE(value3, value4) << value3.ToString() << " and " << value4.ToString();
    EXPECT_EQ(value3, value5) << value3.ToString() << " and " << value5.ToString();
    EXPECT_NE(value4, value5) << value4.ToString() << " and " << value5.ToString();

    ECValue value6(dateTime6);
    ECValue value7(dateTime7);
    ECValue value8(dateTime8);
    EXPECT_EQ(value6, value7) << value6.ToString() << " and " << value7.ToString();
    EXPECT_NE(value6, value8) << value6.ToString() << " and " << value8.ToString();

    ECValue value9(dateTime9);
    ECValue value10(dateTime10);
    EXPECT_EQ(value9, value10) << value9.ToString() << " and " << value10.ToString();

    ECValue value11(dateTime11);
    ECValue value12(dateTime12);
    EXPECT_NE(value11, value12) << value11.ToString() << " and " << value12.ToString();

    // Testing commutativity

    EXPECT_EQ(value1 == value2, value2 == value1);
    EXPECT_EQ(value3 == value4, value4 == value3);
    EXPECT_EQ(value3 == value5, value5 == value3);
    EXPECT_EQ(value4 == value5, value5 == value4);
    EXPECT_EQ(value6 == value7, value7 == value6);
    EXPECT_EQ(value6 == value8, value8 == value6);
    EXPECT_EQ(value9 == value10, value10 == value9);
    EXPECT_EQ(value11 == value12, value12 == value11);
    }

END_BENTLEY_ECN_TEST_NAMESPACE
