/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/Ecutils_Tests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include "DgnPlatform/ECUtils.h"

USING_NAMESPACE_BENTLEY_DPTEST
USING_NAMESPACE_BENTLEY_DGN
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   12/16
+---------------+---------------+---------------+---------------+---------------+------*/

struct EcutilsTests : public DgnDbTestFixture
{
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(EcutilsTests, LoadECValueFromJson)
{
    ECN::ECValue check;
    Json::Value obj(Json::objectValue);
    obj["Value"] = "name";
    obj["Type"] = "s";
    ASSERT_EQ(DgnDbStatus::Success, ECUtils::LoadECValueFromJson(check, obj));
    ASSERT_EQ(check.ToString(), "name");
    // set json type to bool 
    obj["Value"] = "true";
    obj["Type"] = "boolean";
    ASSERT_EQ(DgnDbStatus::Success, ECUtils::LoadECValueFromJson(check, obj));
    ASSERT_EQ(check.GetBoolean(), true);
    // set json type to int 
    obj["Value"] = "1";
    obj["Type"] = "int";
    ASSERT_EQ(DgnDbStatus::Success, ECUtils::LoadECValueFromJson(check, obj));
    ASSERT_EQ(check.GetInteger(), 1);
    ASSERT_EQ(check.GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_Integer);
    // set json type to double 
    obj["Value"] = "10.0";
    obj["Type"] = "double";
    ASSERT_EQ(DgnDbStatus::Success, ECUtils::LoadECValueFromJson(check, obj));
    ASSERT_EQ(check.GetDouble(), 10.0);
    obj.clear();
    ASSERT_EQ(DgnDbStatus::BadArg, ECUtils::LoadECValueFromJson(check, obj));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(EcutilsTests, ConvertECValueToJson)
    {
    ECN::ECValue EcValue(true);
    Json::Value obj(Json::objectValue);

    ASSERT_EQ(BentleyStatus::BSISUCCESS, ECUtils::ConvertECValueToJson(obj,EcValue));
    ASSERT_EQ(obj.asBool(), true);

    EcValue.Clear();
    EcValue.SetDouble(101.001);
    ASSERT_EQ(BentleyStatus::BSISUCCESS, ECUtils::ConvertECValueToJson(obj, EcValue));
    ASSERT_EQ(obj.asDouble(),101.001);
  
    EcValue.Clear();
    EcValue.SetLong((uint64_t)1000000000001);
    ASSERT_EQ(BentleyStatus::BSISUCCESS, ECUtils::ConvertECValueToJson(obj, EcValue));
    ASSERT_EQ(obj.asUInt64(), (uint64_t)1000000000001);

    EcValue.Clear();
    EcValue.SetInteger(100);
    ASSERT_EQ(BentleyStatus::BSISUCCESS, ECUtils::ConvertECValueToJson(obj, EcValue));
    ASSERT_EQ(obj.asInt(), 100);
    EcValue.Clear();

    EcValue.SetUtf8CP("StringValue");
    ASSERT_EQ(BentleyStatus::BSISUCCESS, ECUtils::ConvertECValueToJson(obj, EcValue));
    ASSERT_EQ(obj.asString(), "StringValue");

    EcValue.Clear();
    Json::Value obj2(Json::arrayValue);
    EcValue.SetPoint2d(DPoint2d::From( 8, 8));
    ASSERT_EQ(BentleyStatus::BSISUCCESS, ECUtils::ConvertECValueToJson(obj2, EcValue));
    ASSERT_EQ(obj2[0], 8.0);
    ASSERT_EQ(obj2[1], 8.0);

    EcValue.Clear();
    EcValue.SetPoint3d(DPoint3d::From( 0, 8, 8));
    ASSERT_EQ(BentleyStatus::BSISUCCESS, ECUtils::ConvertECValueToJson(obj2, EcValue));
    ASSERT_EQ(obj2[0], 0.0);
    ASSERT_EQ(obj2[1], 8.0);
    ASSERT_EQ(obj2[2], 8.0);

    EcValue.Clear();
    EcValue.SetDateTime(DateTime(2016,12,16));
    ASSERT_EQ(BentleyStatus::BSISUCCESS, ECUtils::ConvertECValueToJson(obj, EcValue));
    ASSERT_EQ(obj.asString(), "2016-12-16");
       
    EcValue.Clear();
    ASSERT_EQ(BentleyStatus::ERROR, ECUtils::ConvertECValueToJson(obj, EcValue));

    EcValue.SetBoolean(TRUE);
    EcValue.SetIsNull(TRUE);
    ASSERT_EQ(BentleyStatus::BSISUCCESS, ECUtils::ConvertECValueToJson(obj, EcValue));
    ASSERT_EQ(obj.isNull(), true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(EcutilsTests, ConvertJsonToECValue)
    {
    ECN::ECValue EcValue;
    Json::Value obj;
    obj= true;
    ASSERT_EQ(BentleyStatus::BSISUCCESS, ECUtils::ConvertJsonToECValue(EcValue,obj, ECN::PrimitiveType::PRIMITIVETYPE_Boolean));
    ASSERT_EQ(EcValue.GetBoolean(), true);

    obj = 10;
    ASSERT_EQ(BentleyStatus::BSISUCCESS, ECUtils::ConvertJsonToECValue(EcValue, obj, ECN::PrimitiveType::PRIMITIVETYPE_Integer));
    ASSERT_EQ(EcValue.GetInteger(), 10);

    obj = 10.01;
    ASSERT_EQ(BentleyStatus::BSISUCCESS, ECUtils::ConvertJsonToECValue(EcValue, obj, ECN::PrimitiveType::PRIMITIVETYPE_Double));
    ASSERT_EQ(EcValue.GetDouble(), 10.01);

    obj = "StringValue";
    ASSERT_EQ(BentleyStatus::BSISUCCESS, ECUtils::ConvertJsonToECValue(EcValue, obj, ECN::PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(EcValue.ToString(), "StringValue");

    // Giving wrong PrimitiveType
    Json::Value obj2(Json::nullValue);
    ASSERT_EQ(BentleyStatus::BSIERROR, ECUtils::ConvertJsonToECValue(EcValue, obj2, ECN::PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(EcValue.IsNull(),true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(EcutilsTests, StoreECValueAsJson)
    {
    ECN::ECValue EcValue(true);
    Json::Value obj;

    ASSERT_EQ(DgnDbStatus::Success, ECUtils::StoreECValueAsJson(obj, EcValue));
    ASSERT_EQ(obj["Type"], "boolean") << obj.toStyledString();
    ASSERT_EQ(obj["Value"], "True");

    EcValue.Clear();
    EcValue.SetDouble(101.001);
    ASSERT_EQ(DgnDbStatus::Success, ECUtils::StoreECValueAsJson(obj, EcValue));
    ASSERT_EQ(obj["Type"], "double");
    ASSERT_EQ(obj["Value"], "101.001");

    EcValue.Clear();
    EcValue.SetLong((uint64_t)1000000000001);
    ASSERT_EQ(DgnDbStatus::Success, ECUtils::ECUtils::StoreECValueAsJson(obj, EcValue));
    ASSERT_EQ(obj["Type"], "long");
    ASSERT_EQ(obj["Value"], "1000000000001");

    EcValue.Clear();
    EcValue.SetInteger(100);
    ASSERT_EQ(DgnDbStatus::Success, ECUtils::StoreECValueAsJson(obj, EcValue));
    ASSERT_EQ(obj["Type"], "integer");
    ASSERT_EQ(obj["Value"], "100");

    EcValue.Clear();
    EcValue.SetUtf8CP("StringValue");
    ASSERT_EQ(DgnDbStatus::Success, ECUtils::StoreECValueAsJson(obj, EcValue));
    ASSERT_EQ(obj["Type"], "string");
    ASSERT_EQ(obj["Value"], "StringValue");

    EcValue.Clear();
    EcValue.SetPoint2d(DPoint2d::From(8, 8));
    ASSERT_EQ(DgnDbStatus::Success, ECUtils::StoreECValueAsJson(obj, EcValue));
    ASSERT_EQ(obj["Type"], "point2d");
    ASSERT_EQ(obj["Value"], "8,8");

    EcValue.Clear();
    EcValue.SetPoint3d(DPoint3d::From(8, 8, 9));
    ASSERT_EQ(DgnDbStatus::Success, ECUtils::StoreECValueAsJson(obj, EcValue));
    ASSERT_EQ(obj["Type"], "point3d");
    ASSERT_EQ(obj["Value"], "8,8,9");

    EcValue.Clear();
    EcValue.SetDateTime(DateTime(2016, 12, 16));
    ASSERT_EQ(DgnDbStatus::Success, ECUtils::StoreECValueAsJson(obj, EcValue));
    ASSERT_EQ(obj["Type"], "datetime");
    ASSERT_EQ(obj["Value"], "2016-12-16"); 

    ECN::ECValue EcValue2;
    ASSERT_EQ(DgnDbStatus::BadArg, ECUtils::StoreECValueAsJson(obj, EcValue2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(EcutilsTests, ToJsonPropertiesFromECProperties)
    {
    SetupSeedProject();
    Json::Value obj(Json::objectValue);
    ECN::ECClassCP ecClass(m_db->Schemas().GetClass(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_CLASS_NAME));
    ASSERT_TRUE(nullptr != ecClass);
    ECN::IECInstancePtr ecInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    ECN::IECInstanceCR ecInstance1 = *ecInstance;
    ASSERT_EQ(BentleyStatus::BSIERROR, ECUtils::ToJsonPropertiesFromECProperties(obj, ecInstance1, "v,T"));
    ASSERT_EQ(ECN::ECObjectsStatus::Success,ecInstance->SetValue("i",ECN::ECValue(10)));
    ASSERT_EQ(ECN::ECObjectsStatus::Success,ecInstance->SetValue("l", ECN::ECValue((uint64_t)1000000000001)));
    ASSERT_EQ(ECN::ECObjectsStatus::Success, ecInstance->SetValue("p2d", ECN::ECValue(DPoint2d::From( 8, 8))));
    ASSERT_EQ(BentleyStatus::BSISUCCESS, ECUtils::ToJsonPropertiesFromECProperties(obj, ecInstance1, "i,l,p2d"));
    ASSERT_EQ(obj["i"], 10);
    ASSERT_EQ(obj["l"], Json::Value(1000000000001));
    ASSERT_EQ(obj["p2d"][0], 8.0);
    ASSERT_EQ(obj["p2d"][1], 8.0);
    }
