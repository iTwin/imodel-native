/*--------------------------------------------------------------------------------------+
|
|     $Source: test/scenario/CompressInstanceTests.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include <comdef.h>
#include "StopWatch.h"
#include "TestFixture.h"

#include <ECObjects\ECInstance.h>
#include <ECObjects\StandaloneECInstance.h>
#include <ECObjects\ECValue.h>
#include <ECObjects\ECSchema.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

using namespace std;

struct CompressInstanceTests : ECTestFixture {
//IECInstance GetClassInstance(L"CustomAttribute", *schema, *schemaOwner)
//{
//    return NULL;
//}

};
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

void VerifyArrayInfo (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 count, bool isFixedCount)
    {
    v.Clear();
    EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString));
    EXPECT_EQ (count, v.GetArrayInfo().GetCount());
    EXPECT_EQ (isFixedCount, v.GetArrayInfo().IsFixedCount());
    }

void VerifyIsNullArrayElements (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 start, UInt32 count
, bool isNull)
    {
    for (UInt32 i = start ; i < start + count ; i++)
        {
        v.Clear();
        EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString, i));
        EXPECT_TRUE (isNull == v.IsNull());
        }
    }
void VerifyString (IECInstanceR instance, ECValueR v, WCharCP accessString, bool useIndex, UInt32 index, WCharCP value)
    {
    v.Clear();
    if (useIndex)
        EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString, index));
    else
        EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString));
    EXPECT_STREQ (value, v.GetString());
    }
void VerifyString (IECInstanceR instance, ECValueR v, WCharCP accessString, WCharCP value)
    {
    return VerifyString (instance, v, accessString, false, 0, value);
    }
void SetAndVerifyString (IECInstanceR instance, ECValueR v, WCharCP accessString, WCharCP value)
    {
    v.SetString(value);
    EXPECT_TRUE (SUCCESS == instance.SetValue (accessString, v));
    VerifyString (instance, v, accessString, value);
    }

void VerifyInteger (IECInstanceR instance, ECValueR v, WCharCP accessString, bool useIndex, UInt32 index, UInt32 value)
    {
    v.Clear();
    if (useIndex)
        EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString, index));
    else
        EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString));
    EXPECT_EQ (value, v.GetInteger());
    }

void VerifyInteger (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 value)
    {
    return VerifyInteger (instance, v, accessString, false, 0, value);
    }


void SetAndVerifyInteger (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 value)
    {
    v.SetInteger(value);
    EXPECT_TRUE (SUCCESS == instance.SetValue (accessString, v));
    VerifyInteger (instance, v, accessString, value);
    }
static void validateArrayCount  (ECN::StandaloneECInstanceCR instance, WCharCP propertyName, UInt32 expectedCount)
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

TEST_F(CompressInstanceTests, CheckVariableSizedPropertyAfterCallingCompress)
    {


    ECSchemaPtr schema = CreateKitchenSinkSchema ();
    EXPECT_TRUE( schema.IsValid());
	ECClassP ecClass = schema->GetClassP (L"KitchenSink");
    ASSERT_TRUE (NULL != ecClass);

    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

	//SystemTime inTime = SystemTime::GetLocalTime();
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
    ASSERT_TRUE (ECOBJECTS_STATUS_Success == instance->AddArrayElements (L"myManufacturerStructArray[]", 4));
        instance->Compress();
    VerifyArrayInfo (*instance, v, L"myManufacturerStructArray[]", 4, false);
        instance->Compress();
    VerifyIsNullArrayElements (*instance, v, L"myManufacturerStructArray[]", 0, 4, true);

    IECInstancePtr manufInst = manufacturerEnabler->CreateInstance().get();

    SetAndVerifyString (*manufInst, v, L"Name", L"Nissan");
        instance->Compress();
    SetAndVerifyInteger (*manufInst, v, L"AccountNo", 3475);
        instance->Compress();
    v.SetStruct (manufInst.get());
    ASSERT_TRUE (SUCCESS == instance->SetValue (L"myManufacturerStructArray[]", v, 0));

    manufInst = manufacturerEnabler->CreateInstance().get();
    SetAndVerifyString (*manufInst, v, L"Name", L"Kia");
    SetAndVerifyInteger (*manufInst, v, L"AccountNo", 1791);
    v.SetStruct (manufInst.get());
    ASSERT_TRUE (SUCCESS == instance->SetValue (L"myManufacturerStructArray[]", v, 1));

    manufInst = manufacturerEnabler->CreateInstance().get();
    SetAndVerifyString (*manufInst, v, L"Name", L"Honda");
    SetAndVerifyInteger (*manufInst, v, L"AccountNo", 1592);
    v.SetStruct (manufInst.get());
    ASSERT_TRUE (SUCCESS == instance->SetValue (L"myManufacturerStructArray[]", v, 2));

    manufInst = manufacturerEnabler->CreateInstance().get();
    SetAndVerifyString (*manufInst, v, L"Name", L"Chevy");
    SetAndVerifyInteger (*manufInst, v, L"AccountNo", 19341);
    v.SetStruct (manufInst.get());
    ASSERT_TRUE (SUCCESS == instance->SetValue (L"myManufacturerStructArray[]", v, 3));
        instance->Compress();
    VerifyIsNullArrayElements (*instance, v, L"myManufacturerStructArray[]", 0, 4, false);

    // remove struct array element
    instance->RemoveArrayElement(L"myManufacturerStructArray[]", 2);
        instance->Compress();
    validateArrayCount (*instance, L"myManufacturerStructArray[]", 3);

   }
END_BENTLEY_ECOBJECT_NAMESPACE