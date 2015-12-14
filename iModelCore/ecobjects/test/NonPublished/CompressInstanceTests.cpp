/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/CompressInstanceTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct CompressInstanceTests : ECTestFixture
    {
    ECSchemaPtr m_schema;
    Utf8CP kitchenSinkSchemaXML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"KitchenSink\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "    <ECClass typeName=\"Manufacturer\" isStruct=\"True\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"Name\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"AccountNo\" typeName=\"int\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"Complicated\" isStruct=\"True\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"ExtName\" typeName=\"string\" />"
        "        <ECStructProperty propertyName=\"ExtStruct\" typeName=\"Manufacturer\" />"
        "        <ECArrayProperty propertyName=\"ExtStructs\" typeName=\"Manufacturer\" />"
        "    </ECClass>"

        "    <ECClass typeName=\"FixedSizeArrayTester\" isStruct=\"True\" isDomainClass=\"True\">"
        "        <ECArrayProperty propertyName=\"FixedString5\"     typeName=\"string\"            minOccurs=\"5\"  maxOccurs=\"5\" />"
        "        <ECArrayProperty propertyName=\"FixedInt5\"        typeName=\"int\"               minOccurs=\"5\"  maxOccurs=\"5\" />"
        "        <ECArrayProperty propertyName=\"Manufacturer5\"    typeName=\"Manufacturer\"      minOccurs=\"5\"  maxOccurs=\"5\" />"
        "    </ECClass>"

        "    <ECClass typeName=\"KitchenSink\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"myString\" typeName=\"string\" />"
        "        <ECArrayProperty propertyName=\"myStringArray\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"myInt\" typeName=\"int\" />"
        "        <ECArrayProperty propertyName=\"myIntArray\" typeName=\"int\" />"
        "        <ECProperty propertyName=\"my3dPoint\"     typeName=\"point3d\" />"
        "        <ECArrayProperty propertyName=\"my3dPointArray\" typeName=\"point3d\"/>"
        "        <ECProperty propertyName=\"myDate\" typeName=\"dateTime\"  />"
        "        <ECArrayProperty propertyName=\"myDateArray\" typeName=\"dateTime\" />"
        "        <ECProperty propertyName=\"myLong\" typeName=\"long\" />"
        "        <ECArrayProperty propertyName=\"myLongArray\" typeName=\"long\" />"
        "        <ECProperty propertyName=\"myDouble\" typeName=\"double\" />"
        "        <ECArrayProperty propertyName=\"myDoubleArray\" typeName=\"double\" />"
        "        <ECProperty propertyName=\"myBool\" typeName=\"boolean\"  />"
        "        <ECArrayProperty propertyName=\"myBoolArray\" typeName=\"boolean\" />"
        "        <ECProperty propertyName=\"my2dPoint\" typeName=\"point2d\" />"
        "        <ECArrayProperty propertyName=\"my2dPointArray\" typeName=\"point2d\" />"
        "        <ECStructProperty propertyName=\"myManufacturerStruct\" typeName=\"Manufacturer\" />"
        "        <ECArrayProperty propertyName=\"myManufacturerStructArray\" typeName=\"Manufacturer\"/>"
        "        <ECStructProperty propertyName=\"myComplicated\" typeName=\"Complicated\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"PointArrayTest\" isStruct=\"True\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"myString\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"myInt\" typeName=\"int\" />"
        "        <ECArrayProperty propertyName=\"my3dPointArray\" typeName=\"point3d\"/>"
        "    </ECClass>"
        "</ECSchema>";

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ReadKitchenSinkSchemaFromXml ()
        {
        ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext (false);
        EXPECT_EQ (SchemaReadStatus::Success, ECSchema::ReadFromXmlString (m_schema, kitchenSinkSchemaXML, *schemaContext));
        EXPECT_TRUE (m_schema.IsValid ());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyArrayInfo (IECInstanceR instance, ECValueR v, Utf8CP accessString, uint32_t count, bool isFixedCount)
        {
        v.Clear ();
        EXPECT_TRUE (ECObjectsStatus::Success == instance.GetValue (v, accessString));
        EXPECT_EQ (count, v.GetArrayInfo ().GetCount ());
        EXPECT_EQ (isFixedCount, v.GetArrayInfo ().IsFixedCount ());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyIsNullArrayElements (IECInstanceR instance, ECValueR v, Utf8CP accessString, uint32_t start, uint32_t count, bool isNull)
        {
        for (uint32_t i = start; i < start + count; i++)
            {
            v.Clear ();
            EXPECT_TRUE (ECObjectsStatus::Success == instance.GetValue (v, accessString, i));
            EXPECT_TRUE (isNull == v.IsNull ());
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyString (IECInstanceR instance, ECValueR v, Utf8CP accessString, bool useIndex, uint32_t index, Utf8CP value)
        {
        v.Clear ();
        if (useIndex)
            EXPECT_TRUE (ECObjectsStatus::Success == instance.GetValue (v, accessString, index));
        else
            EXPECT_TRUE (ECObjectsStatus::Success == instance.GetValue (v, accessString));
        EXPECT_STREQ (value, v.GetUtf8CP ());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyString (IECInstanceR instance, ECValueR v, Utf8CP accessString, Utf8CP value)
        {
        return VerifyString (instance, v, accessString, false, 0, value);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetAndVerifyString (IECInstanceR instance, ECValueR v, Utf8CP accessString, Utf8CP value)
        {
        v.SetUtf8CP (value);
        EXPECT_TRUE (ECObjectsStatus::Success == instance.SetValue (accessString, v));
        VerifyString (instance, v, accessString, value);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyInteger (IECInstanceR instance, ECValueR v, Utf8CP accessString, bool useIndex, uint32_t index, uint32_t value)
        {
        v.Clear ();
        if (useIndex)
            EXPECT_TRUE (ECObjectsStatus::Success == instance.GetValue (v, accessString, index));
        else
            EXPECT_TRUE (ECObjectsStatus::Success == instance.GetValue (v, accessString));
        EXPECT_EQ (value, v.GetInteger ());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyInteger (IECInstanceR instance, ECValueR v, Utf8CP accessString, uint32_t value)
        {
        return VerifyInteger (instance, v, accessString, false, 0, value);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetAndVerifyInteger (IECInstanceR instance, ECValueR v, Utf8CP accessString, uint32_t value)
        {
        v.SetInteger (value);
        EXPECT_TRUE (ECObjectsStatus::Success == instance.SetValue (accessString, v));
        VerifyInteger (instance, v, accessString, value);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void validateArrayCount (ECN::StandaloneECInstanceCR instance, Utf8CP propertyName, uint32_t expectedCount)
        {
        ECValue varray;
        EXPECT_TRUE (ECObjectsStatus::Success == instance.GetValue (varray, propertyName));
        uint32_t count = varray.GetArrayInfo ().GetCount ();
        EXPECT_TRUE (count == expectedCount);

        ECValue ventry;
        for (uint32_t i = 0; i < count; i++)
            {
            EXPECT_TRUE (ECObjectsStatus::Success == instance.GetValue (ventry, propertyName, i));
            }
        }
    };

TEST_F (CompressInstanceTests, CheckVariableSizedPropertyAfterCallingCompress)
    {
    ReadKitchenSinkSchemaFromXml ();
    ECClassP ecClass = m_schema->GetClassP ("KitchenSink");
    ASSERT_TRUE (NULL != ecClass);

    StandaloneECEnablerPtr enabler = ecClass->GetDefaultStandaloneEnabler ();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance ();

    int        inCount = 100;
    double     inLength = 432.178;
    bool       inTest = true;

    ASSERT_EQ (ECObjectsStatus::Success, instance->SetValue ("myInt", ECValue (inCount)));
    ASSERT_EQ (ECObjectsStatus::Success, instance->SetValue ("myString", ECValue ("Test")));
    ASSERT_EQ (ECObjectsStatus::Success, instance->SetValue ("myDouble", ECValue (inLength)));
    ASSERT_EQ (ECObjectsStatus::Success, instance->SetValue ("myBool", ECValue (inTest)));

    ECValue ecValue;

    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "myInt"));
    EXPECT_TRUE (ecValue.GetInteger () == inCount);

    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "myString"));
    EXPECT_STREQ (ecValue.GetUtf8CP (), "Test");

    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "myDouble"));
    EXPECT_TRUE (ecValue.GetDouble () == inLength);

    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "myBool"));
    EXPECT_TRUE (ecValue.GetBoolean () == inTest);

    instance->Compress ();

    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "myInt"));
    EXPECT_TRUE (ecValue.GetInteger () == inCount);

    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "myString"));
    EXPECT_STREQ (ecValue.GetUtf8CP (), "Test");

    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "myDouble"));
    EXPECT_TRUE (ecValue.GetDouble () == inLength);

    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "myBool"));
    EXPECT_TRUE (ecValue.GetBoolean () == inTest);

    // define struct array
    StandaloneECEnablerPtr manufacturerEnabler = instance->GetEnablerR ().GetEnablerForStructArrayMember (m_schema->GetSchemaKey (), "Manufacturer");
    EXPECT_TRUE (manufacturerEnabler.IsValid ());

    ECValue v;
    ASSERT_TRUE (ECObjectsStatus::Success == instance->AddArrayElements ("myManufacturerStructArray", 4));
    instance->Compress ();
    VerifyArrayInfo (*instance, v, "myManufacturerStructArray", 4, false);
    instance->Compress ();
    VerifyIsNullArrayElements (*instance, v, "myManufacturerStructArray", 0, 4, true);

    IECInstancePtr manufInst = manufacturerEnabler->CreateInstance ().get ();

    SetAndVerifyString (*manufInst, v, "Name", "Nissan");
    instance->Compress ();
    SetAndVerifyInteger (*manufInst, v, "AccountNo", 3475);
    instance->Compress ();
    v.SetStruct (manufInst.get ());
    ASSERT_TRUE (ECObjectsStatus::Success == instance->SetValue ("myManufacturerStructArray", v, 0));

    manufInst = manufacturerEnabler->CreateInstance ().get ();
    SetAndVerifyString (*manufInst, v, "Name", "Kia");
    SetAndVerifyInteger (*manufInst, v, "AccountNo", 1791);
    v.SetStruct (manufInst.get ());
    ASSERT_TRUE (ECObjectsStatus::Success == instance->SetValue ("myManufacturerStructArray", v, 1));

    manufInst = manufacturerEnabler->CreateInstance ().get ();
    SetAndVerifyString (*manufInst, v, "Name", "Honda");
    SetAndVerifyInteger (*manufInst, v, "AccountNo", 1592);
    v.SetStruct (manufInst.get ());
    ASSERT_TRUE (ECObjectsStatus::Success == instance->SetValue ("myManufacturerStructArray", v, 2));

    manufInst = manufacturerEnabler->CreateInstance ().get ();
    SetAndVerifyString (*manufInst, v, "Name", "Chevy");
    SetAndVerifyInteger (*manufInst, v, "AccountNo", 19341);
    v.SetStruct (manufInst.get ());
    ASSERT_TRUE (ECObjectsStatus::Success == instance->SetValue ("myManufacturerStructArray", v, 3));
    instance->Compress ();
    VerifyIsNullArrayElements (*instance, v, "myManufacturerStructArray", 0, 4, false);

    // remove struct array element
    instance->RemoveArrayElement ("myManufacturerStructArray", 2);
    instance->Compress ();
    validateArrayCount (*instance, "myManufacturerStructArray", 3);
    }

END_BENTLEY_ECN_TEST_NAMESPACE
