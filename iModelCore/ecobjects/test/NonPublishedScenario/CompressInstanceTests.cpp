/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublishedScenario/CompressInstanceTests.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"


BEGIN_BENTLEY_ECOBJECT_NAMESPACE

struct CompressInstanceTests : ECTestFixture
{

static WString       GetKitchenSinkSchemaXml ()
    {
    wchar_t* buff = L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        L"<ECSchema schemaName=\"KitchenSink\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        L"    <ECClass typeName=\"Manufacturer\" isStruct=\"True\" isDomainClass=\"True\">"
        L"        <ECProperty propertyName=\"Name\" typeName=\"string\" />"
        L"        <ECProperty propertyName=\"AccountNo\" typeName=\"int\" />"
        L"    </ECClass>"
        L"    <ECClass typeName=\"Complicated\" isStruct=\"True\" isDomainClass=\"True\">"
        L"        <ECProperty propertyName=\"ExtName\" typeName=\"string\" />"
        L"        <ECStructProperty propertyName=\"ExtStruct\" typeName=\"Manufacturer\" />"
        L"        <ECArrayProperty propertyName=\"ExtStructs\" typeName=\"Manufacturer\" />"
        L"    </ECClass>"

        L"    <ECClass typeName=\"FixedSizeArrayTester\" isStruct=\"True\" isDomainClass=\"True\">"
        L"        <ECArrayProperty propertyName=\"FixedString5\"     typeName=\"string\"            minOccurs=\"5\"  maxOccurs=\"5\" />"
        L"        <ECArrayProperty propertyName=\"FixedInt5\"        typeName=\"int\"               minOccurs=\"5\"  maxOccurs=\"5\" />"
        L"        <ECArrayProperty propertyName=\"Manufacturer5\"    typeName=\"Manufacturer\"      minOccurs=\"5\"  maxOccurs=\"5\" />"
        L"    </ECClass>"

        L"    <ECClass typeName=\"KitchenSink\" isDomainClass=\"True\">"
        L"        <ECProperty propertyName=\"myString\" typeName=\"string\" />"
        L"        <ECArrayProperty propertyName=\"myStringArray\" typeName=\"string\" />"
        L"        <ECProperty propertyName=\"myInt\" typeName=\"int\" />"
        L"        <ECArrayProperty propertyName=\"myIntArray\" typeName=\"int\" />"
        L"        <ECProperty propertyName=\"my3dPoint\"     typeName=\"point3d\" />"
        L"        <ECArrayProperty propertyName=\"my3dPointArray\" typeName=\"point3d\"/>"
        L"        <ECProperty propertyName=\"myDate\" typeName=\"dateTime\"  />"
        L"        <ECArrayProperty propertyName=\"myDateArray\" typeName=\"dateTime\" />"
        L"        <ECProperty propertyName=\"myLong\" typeName=\"long\" />"
        L"        <ECArrayProperty propertyName=\"myLongArray\" typeName=\"long\" />"
        L"        <ECProperty propertyName=\"myDouble\" typeName=\"double\" />"
        L"        <ECArrayProperty propertyName=\"myDoubleArray\" typeName=\"double\" />"
        L"        <ECProperty propertyName=\"myBool\" typeName=\"boolean\"  />"
        L"        <ECArrayProperty propertyName=\"myBoolArray\" typeName=\"boolean\" />"
        L"        <ECProperty propertyName=\"my2dPoint\" typeName=\"point2d\" />"
        L"        <ECArrayProperty propertyName=\"my2dPointArray\" typeName=\"point2d\" />"
        L"        <ECStructProperty propertyName=\"myManufacturerStruct\" typeName=\"Manufacturer\" />"
        L"        <ECArrayProperty propertyName=\"myManufacturerStructArray\" typeName=\"Manufacturer\"/>"
        L"        <ECStructProperty propertyName=\"myComplicated\" typeName=\"Complicated\" />"
        L"    </ECClass>"
        L"    <ECClass typeName=\"PointArrayTest\" isStruct=\"True\" isDomainClass=\"True\">"
        L"        <ECProperty propertyName=\"myString\" typeName=\"string\" />"
        L"        <ECProperty propertyName=\"myInt\" typeName=\"int\" />"
        L"        <ECArrayProperty propertyName=\"my3dPointArray\" typeName=\"point3d\"/>"
        L"    </ECClass>"
        L"</ECSchema>";

    return WString (buff);
    }
    
static ECSchemaPtr   CreateKitchenSinkSchema ()
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext(false);
    EXPECT_EQ (SUCCESS, ECSchema::ReadFromXmlString (schema, GetKitchenSinkSchemaXml().c_str(), *schemaContext));

    return schema;
    }

static void ValidateArrayCount  (ECN::StandaloneECInstanceCR instance, WCharCP propertyName, UInt32 expectedCount)
    {
    ECValue varray;
    EXPECT_TRUE (SUCCESS == instance.GetValue (varray, propertyName));
    UInt32 count = varray.GetArrayInfo().GetCount();
    EXPECT_TRUE (count == expectedCount);

    ECValue ventry;

    for (UInt32 i=0; i<count; i++)
        {
        EXPECT_TRUE (SUCCESS == instance.GetValue (ventry, propertyName, i));
        }
    }
};

TEST_F(CompressInstanceTests, CheckVariableSizedPropertyAfterCallingCompress)
    {


    ECSchemaPtr schema = CreateKitchenSinkSchema ();
    EXPECT_TRUE( schema.IsValid());
	ECClassP ecClass = schema->GetClassP (L"KitchenSink");
    ASSERT_TRUE (NULL != ecClass);

    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    int        inCount = 100;
    double     inLength = 432.178;
    bool       inTest = true;


    ASSERT_EQ(ECOBJECTS_STATUS_Success,instance->SetValue (L"myInt",        ECValue (inCount)));
    ASSERT_EQ(ECOBJECTS_STATUS_Success,instance->SetValue (L"myString",         ECValue (L"Test")));
    ASSERT_EQ(ECOBJECTS_STATUS_Success,instance->SetValue (L"myDouble",       ECValue (inLength)));
    ASSERT_EQ(ECOBJECTS_STATUS_Success,instance->SetValue (L"myBool", ECValue (inTest)));

    ECValue ecValue;

    EXPECT_TRUE (SUCCESS == instance->GetValue (ecValue, L"myInt"));
    EXPECT_TRUE (ecValue.GetInteger() == inCount);

    EXPECT_TRUE (SUCCESS == instance->GetValue (ecValue, L"myString"));
    EXPECT_STREQ (ecValue.GetString(), L"Test");

    EXPECT_TRUE (SUCCESS == instance->GetValue (ecValue, L"myDouble"));
    EXPECT_TRUE (ecValue.GetDouble() == inLength);

    EXPECT_TRUE (SUCCESS == instance->GetValue (ecValue, L"myBool"));
    EXPECT_TRUE (ecValue.GetBoolean() == inTest);


        instance->Compress();


        EXPECT_TRUE (SUCCESS == instance->GetValue (ecValue, L"myInt"));
    EXPECT_TRUE (ecValue.GetInteger() == inCount);

    EXPECT_TRUE (SUCCESS == instance->GetValue (ecValue, L"myString"));
    EXPECT_STREQ (ecValue.GetString(), L"Test");

    EXPECT_TRUE (SUCCESS == instance->GetValue (ecValue, L"myDouble"));
    EXPECT_TRUE (ecValue.GetDouble() == inLength);

    EXPECT_TRUE (SUCCESS == instance->GetValue (ecValue, L"myBool"));
    EXPECT_TRUE (ecValue.GetBoolean() == inTest);


    // define struct array
    StandaloneECEnablerPtr manufacturerEnabler = instance->GetEnablerR().GetEnablerForStructArrayMember (schema->GetSchemaKey(), L"Manufacturer");
    EXPECT_TRUE (manufacturerEnabler.IsValid());

    ECValue v;
    ASSERT_TRUE (ECOBJECTS_STATUS_Success == instance->AddArrayElements (L"myManufacturerStructArray", 4));
        instance->Compress();
    VerifyArrayInfo (*instance, v, L"myManufacturerStructArray", 4, false);
        instance->Compress();
    VerifyIsNullArrayElements (*instance, v, L"myManufacturerStructArray", 0, 4, true);

    IECInstancePtr manufInst = manufacturerEnabler->CreateInstance().get();

    SetAndVerifyString (*manufInst, v, L"Name", L"Nissan");
        instance->Compress();
    SetAndVerifyInteger (*manufInst, v, L"AccountNo", 3475);
        instance->Compress();
    v.SetStruct (manufInst.get());
    ASSERT_TRUE (SUCCESS == instance->SetValue (L"myManufacturerStructArray", v, 0));

    manufInst = manufacturerEnabler->CreateInstance().get();
    SetAndVerifyString (*manufInst, v, L"Name", L"Kia");
    SetAndVerifyInteger (*manufInst, v, L"AccountNo", 1791);
    v.SetStruct (manufInst.get());
    ASSERT_TRUE (SUCCESS == instance->SetValue (L"myManufacturerStructArray", v, 1));

    manufInst = manufacturerEnabler->CreateInstance().get();
    SetAndVerifyString (*manufInst, v, L"Name", L"Honda");
    SetAndVerifyInteger (*manufInst, v, L"AccountNo", 1592);
    v.SetStruct (manufInst.get());
    ASSERT_TRUE (SUCCESS == instance->SetValue (L"myManufacturerStructArray", v, 2));

    manufInst = manufacturerEnabler->CreateInstance().get();
    SetAndVerifyString (*manufInst, v, L"Name", L"Chevy");
    SetAndVerifyInteger (*manufInst, v, L"AccountNo", 19341);
    v.SetStruct (manufInst.get());
    ASSERT_TRUE (SUCCESS == instance->SetValue (L"myManufacturerStructArray", v, 3));
        instance->Compress();
    VerifyIsNullArrayElements (*instance, v, L"myManufacturerStructArray", 0, 4, false);

    // remove struct array element
    instance->RemoveArrayElement(L"myManufacturerStructArray", 2);
        instance->Compress();
    ValidateArrayCount (*instance, L"myManufacturerStructArray", 3);

   }
END_BENTLEY_ECOBJECT_NAMESPACE
