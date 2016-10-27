/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/MemoryLayoutTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

#define N_FINAL_STRING_PROPS_IN_FAKE_CLASS 48

#define EXPECT_SUCCESS(EXPR) EXPECT_EQ (ECObjectsStatus::Success, (EXPR))

#include <ECObjects/ECInstance.h>
#include <ECObjects/StandaloneECInstance.h>
#include <ECObjects/ECValue.h>
#include <Bentley/BeTimeUtilities.h>
BEGIN_BENTLEY_ECN_TEST_NAMESPACE

using namespace BentleyApi::ECN;
using namespace std;

struct NonPublishedMemoryLayoutTests : ECTestFixture
    {
    static std::vector<Utf8String> s_propertyNames;

    void SetValuesForProfiling(StandaloneECInstanceR instance)
        {
        for (NameVector::const_iterator it = s_propertyNames.begin(); it != s_propertyNames.end(); ++it)
            instance.SetValue(it->c_str(), ECValue(it->c_str()));
        }
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    CaseyMullen     10/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyString(IECInstanceR instance, ECValueR v, Utf8CP accessString, bool useIndex, uint32_t index, Utf8CP value)
        {
        v.Clear();
        if (useIndex)
            EXPECT_TRUE(ECObjectsStatus::Success == instance.GetValue(v, accessString, index));
        else
            EXPECT_TRUE(ECObjectsStatus::Success == instance.GetValue(v, accessString));
        EXPECT_STREQ(value, v.GetUtf8CP());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Adam.Klatzkin                   01/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyString(IECInstanceR instance, ECValueR v, Utf8CP accessString, Utf8CP value)
        {
        return VerifyString(instance, v, accessString, false, 0, value);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    CaseyMullen     10/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetAndVerifyString(IECInstanceR instance, ECValueR v, Utf8CP accessString, Utf8CP value)
        {
        v.SetUtf8CP(value);
        EXPECT_TRUE(ECObjectsStatus::Success == instance.SetValue(accessString, v));
        VerifyString(instance, v, accessString, value);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    CaseyMullen     10/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyInteger(IECInstanceR instance, ECValueR v, Utf8CP accessString, bool useIndex, uint32_t index, uint32_t value)
        {
        v.Clear();
        if (useIndex)
            EXPECT_TRUE(ECObjectsStatus::Success == instance.GetValue(v, accessString, index));
        else
            EXPECT_TRUE(ECObjectsStatus::Success == instance.GetValue(v, accessString));
        EXPECT_EQ(value, v.GetInteger());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    CaseyMullen     10/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyInteger(IECInstanceR instance, ECValueR v, Utf8CP accessString, uint32_t value)
        {
        return VerifyInteger(instance, v, accessString, false, 0, value);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    CaseyMullen     10/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetAndVerifyInteger(IECInstanceR instance, ECValueR v, Utf8CP accessString, uint32_t value)
        {
        v.SetInteger(value);
        EXPECT_TRUE(ECObjectsStatus::Success == instance.SetValue(accessString, v));
        VerifyInteger(instance, v, accessString, value);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    CaseyMullen     10/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyDouble(IECInstanceR instance, ECValueR v, Utf8CP accessString, double value)
        {
        v.Clear();
        EXPECT_TRUE(ECObjectsStatus::Success == instance.GetValue(v, accessString));
        EXPECT_EQ(value, v.GetDouble());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    CaseyMullen     10/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetAndVerifyDouble(IECInstanceR instance, ECValueR v, Utf8CP accessString, double value)
        {
        v.SetDouble(value);
        EXPECT_TRUE(ECObjectsStatus::Success == instance.SetValue(accessString, v));
        VerifyDouble(instance, v, accessString, value);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    CaseyMullen     10/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyLong(IECInstanceR instance, ECValueR v, Utf8CP accessString, uint64_t value)
        {
        v.Clear();
        EXPECT_TRUE(ECObjectsStatus::Success == instance.GetValue(v, accessString));
        EXPECT_EQ(value, v.GetLong());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    CaseyMullen     10/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetAndVerifyLong(IECInstanceR instance, ECValueR v, Utf8CP accessString, uint64_t value)
        {
        v.SetLong(value);
        EXPECT_TRUE(ECObjectsStatus::Success == instance.SetValue(accessString, v));
        VerifyLong(instance, v, accessString, value);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    AdamKlatzkin     01/10
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyArrayInfo(IECInstanceR instance, ECValueR v, Utf8CP accessString, uint32_t count, bool isFixedCount)
        {
        v.Clear();
        EXPECT_TRUE(ECObjectsStatus::Success == instance.GetValue(v, accessString));
        EXPECT_EQ(count, v.GetArrayInfo().GetCount());
        EXPECT_EQ(isFixedCount, v.GetArrayInfo().IsFixedCount());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    AdamKlatzkin     01/10
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyOutOfBoundsError(IECInstanceR instance, ECValueR v, Utf8CP accessString, uint32_t index)
        {
        v.Clear();
        EXPECT_TRUE(ECObjectsStatus::IndexOutOfRange == instance.GetValue(v, accessString, index));
        EXPECT_TRUE(ECObjectsStatus::IndexOutOfRange == instance.SetValue(accessString, v, index));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    AdamKlatzkin     01/10
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyStringArray(IECInstanceR instance, ECValueR v, Utf8CP accessString, Utf8CP value, uint32_t start, uint32_t count)
        {
        Utf8String incrementingString = value;

        for (uint32_t i = start; i < start + count; i++)
            {
            incrementingString.append("X");
            VerifyString(instance, v, accessString, true, i, incrementingString.c_str());
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    AdamKlatzkin     01/10
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetAndVerifyStringArray(IECInstanceR instance, ECValueR v, Utf8CP accessString, Utf8CP value, uint32_t count)
        {
        Utf8String incrementingString = value;
        for (uint32_t i = 0; i < count; i++)
            {
            incrementingString.append("X");
            v.SetUtf8CP(incrementingString.c_str());

            // since the test sets some of the array values more than once to the same value we must check SUCCESS || ECObjectsStatus::PropertyValueMatchesNoChange 
            ECObjectsStatus status = instance.SetValue(accessString, v, i);
            EXPECT_TRUE(ECObjectsStatus::Success == status || ECObjectsStatus::PropertyValueMatchesNoChange == status);
            }

        VerifyStringArray(instance, v, accessString, value, 0, count);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    AdamKlatzkin     01/10
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyIntegerArray(IECInstanceR instance, ECValueR v, Utf8CP accessString, uint32_t baseValue, uint32_t start, uint32_t count)
        {
        for (uint32_t i = start; i < start + count; i++)
            {
            VerifyInteger(instance, v, accessString, true, i, baseValue++);
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    AdamKlatzkin     01/10
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetAndVerifyIntegerArray(IECInstanceR instance, ECValueR v, Utf8CP accessString, uint32_t baseValue, uint32_t count)
        {
        for (uint32_t i = 0; i < count; i++)
            {
            v.SetInteger(baseValue + i);

            // since the test sets some of the array values more than once to the same value we must check SUCCESS || ECObjectsStatus::PropertyValueMatchesNoChange 
            ECObjectsStatus status = instance.SetValue(accessString, v, i);
            EXPECT_TRUE(ECObjectsStatus::Success == status || ECObjectsStatus::PropertyValueMatchesNoChange == status);
            }

        VerifyIntegerArray(instance, v, accessString, baseValue, 0, count);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Adam.Klatzkin                   01/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyIsNullArrayElements(IECInstanceR instance, ECValueR v, Utf8CP accessString, uint32_t start, uint32_t count, bool isNull)
        {
        for (uint32_t i = start; i < start + count; i++)
            {
            v.Clear();
            EXPECT_TRUE(ECObjectsStatus::Success == instance.GetValue(v, accessString, i));
            EXPECT_TRUE(isNull == v.IsNull());
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    12/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    Utf8String    GetTestSchemaXMLString(Utf8CP schemaName, uint32_t versionMajor, uint32_t versionMinor, Utf8CP className)
        {
        Utf8Char fmt[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<ECSchema schemaName=\"%s\" nameSpacePrefix=\"test\" version=\"%02d.%02d\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            "    <ECClass typeName=\"EmptyClass\" isDomainClass=\"True\">"
            "    </ECClass>"
            "    <ECClass typeName=\"Manufacturer\" isStruct=\"True\" isDomainClass=\"True\">"
            "        <ECProperty propertyName=\"Name\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"AccountNo\" typeName=\"int\" />"
            "    </ECClass>"
            "    <ECClass typeName=\"CadData\" isStruct=\"True\" isDomainClass=\"True\">"
            "        <ECProperty propertyName=\"Name\"         typeName=\"string\" />"
            "        <ECProperty propertyName=\"Count\"        typeName=\"int\" />"
            "        <ECProperty propertyName=\"StartPoint\"   typeName=\"point3d\" />"
            "        <ECProperty propertyName=\"EndPoint\"     typeName=\"point3d\" />"
            "        <ECProperty propertyName=\"Size\"         typeName=\"point2d\" />"
            "        <ECProperty propertyName=\"Length\"       typeName=\"double\"  />"
            "        <ECProperty propertyName=\"Install_Date\" typeName=\"dateTime\"  />"
            "        <ECProperty propertyName=\"Service_Date\" typeName=\"dateTime\"  />"
            "        <ECProperty propertyName=\"Field_Tested\" typeName=\"boolean\"  />"
            "    </ECClass>"
            "    <ECClass typeName=\"AllPrimitives\" isStruct=\"True\" isDomainClass=\"True\">"
            "        <ECProperty propertyName=\"AString\"          typeName=\"string\" />"
            "        <ECProperty propertyName=\"AnInt\"            typeName=\"int\" />"
            "        <ECProperty propertyName=\"APoint3d\"         typeName=\"point3d\" />"
            "        <ECProperty propertyName=\"APoint2d\"         typeName=\"point2d\" />"
            "        <ECProperty propertyName=\"ADouble\"          typeName=\"double\"  />"
            "        <ECProperty propertyName=\"ADateTime\"        typeName=\"dateTime\"  />"
            "        <ECProperty propertyName=\"ABoolean\"         typeName=\"boolean\"  />"
            "        <ECProperty propertyName=\"ALong\"            typeName=\"long\"  />"
            "        <ECProperty propertyName=\"ABinary\"          typeName=\"binary\"  />"
            "        <ECProperty propertyName=\"ReadOnlyInt\"      typeName=\"int\" readOnly=\"True\"  />"
            "        <ECArrayProperty propertyName=\"SomeStrings\" typeName=\"string\" />"
            "        <ECArrayProperty propertyName=\"SomeInts\"    typeName=\"int\" />"
            "        <ECArrayProperty propertyName=\"SomePoint3ds\"    typeName=\"point3d\" />"
            "        <ECArrayProperty propertyName=\"SomePoint2ds\"    typeName=\"point2d\" />"
            "        <ECArrayProperty propertyName=\"SomeDoubles\"     typeName=\"double\"  />"
            "        <ECArrayProperty propertyName=\"SomeDateTimes\"   typeName=\"dateTime\"  />"
            "        <ECArrayProperty propertyName=\"SomeBooleans\"    typeName=\"boolean\"  />"
            "        <ECArrayProperty propertyName=\"SomeLongs\"       typeName=\"long\"  />"
            "        <ECArrayProperty propertyName=\"SomeBinaries\"    typeName=\"binary\"  />"
            "    </ECClass>"
            "    <ECClass typeName=\"FixedSizeArrayTester\" isStruct=\"True\" isDomainClass=\"True\">"
            "        <ECArrayProperty propertyName=\"FixedString1\"  typeName=\"string\"     minOccurs=\"1\"  maxOccurs=\"1\" />"
            "        <ECArrayProperty propertyName=\"FixedInt1\"     typeName=\"int\"        minOccurs=\"1\"  maxOccurs=\"1\" />"
            "        <ECArrayProperty propertyName=\"FixedString10\" typeName=\"string\"     minOccurs=\"10\" maxOccurs=\"10\" />"
            "        <ECArrayProperty propertyName=\"FixedInt10\"    typeName=\"int\"        minOccurs=\"10\" maxOccurs=\"10\" />"
            "        <ECArrayProperty propertyName=\"Struct1\"       typeName=\"BaseClass0\" minOccurs=\"1\"  maxOccurs=\"1\" />"
            "        <ECArrayProperty propertyName=\"Struct10\"      typeName=\"BaseClass0\" minOccurs=\"10\" maxOccurs=\"10\" />"
            "    </ECClass>"
            "    <ECClass typeName=\"ClassLayoutPerformanceTest0\" isStruct=\"True\" isDomainClass=\"True\">"
            "        <ECProperty propertyName=\"AString\"  typeName=\"string\" />"
            "        <ECProperty propertyName=\"AnInt\"    typeName=\"int\" />"
            "        <ECProperty propertyName=\"ADouble\"  typeName=\"double\"  />"
            "    </ECClass>"
            "    <ECClass typeName=\"ClassLayoutPerformanceTest1\" isStruct=\"True\" isDomainClass=\"True\">"
            "        <ECProperty propertyName=\"AMonkeywrench\"    typeName=\"int\" />"
            "        <ECProperty propertyName=\"ADouble\"          typeName=\"double\"  />"
            "        <ECProperty propertyName=\"AString\"          typeName=\"string\" />"
            "        <ECProperty propertyName=\"AnInt\"            typeName=\"int\" />"
            "    </ECClass>"
            "    <ECClass typeName=\"%s\" isDomainClass=\"True\">"
            "        <ECArrayProperty propertyName=\"BeginningArray\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"A\" typeName=\"int\" />"
            "        <ECProperty propertyName=\"AA\" typeName=\"int\" />"
            "        <ECProperty propertyName=\"B\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"C\" typeName=\"long\" />"
            "        <ECProperty propertyName=\"D\" typeName=\"double\" />"
            "        <ECProperty propertyName=\"S\" typeName=\"string\" />"
            "        <ECStructProperty propertyName=\"Manufacturer\" typeName=\"Manufacturer\" />"
            "        <ECProperty propertyName=\"Property0\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property1\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property2\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property3\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property4\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property5\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property6\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property7\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property8\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property9\" typeName=\"string\" />"
            "        <ECArrayProperty propertyName=\"FixedArrayFixedElement\" typeName=\"int\" minOccurs=\"10\" maxOccurs=\"10\"/>"
            "        <ECProperty propertyName=\"Property10\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property11\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property12\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property13\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property14\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property15\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property16\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property17\" typeName=\"string\" />"
            "        <ECArrayProperty propertyName=\"VariableArrayFixedElement\" typeName=\"int\"/>"
            "        <ECArrayProperty propertyName=\"FixedArrayVariableElement\" typeName=\"string\" minOccurs=\"12\" maxOccurs=\"12\"/>"
            "        <ECProperty propertyName=\"Property18\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property19\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property20\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property21\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property22\" typeName=\"string\" />"
            "        <ECArrayProperty propertyName=\"ManufacturerArray\" typeName=\"Manufacturer\"/>"
            "        <ECProperty propertyName=\"Property23\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property24\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property25\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property26\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property27\" typeName=\"string\" />"
            "        <ECArrayProperty propertyName=\"VariableArrayVariableElement\" typeName=\"string\"/>"
            "        <ECProperty propertyName=\"Property28\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property29\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property30\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property31\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property32\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property33\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property34\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property35\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property36\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property37\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property38\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property39\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property40\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property41\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property42\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property43\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property44\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property45\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property46\" typeName=\"string\" />"
            "        <ECProperty propertyName=\"Property47\" typeName=\"string\" />"
            "        <ECArrayProperty propertyName=\"EndingArray\" typeName=\"string\" />"
            "    </ECClass>"
            "    <ECClass typeName=\"NestedStructArray\" isStruct=\"True\" isDomainClass=\"True\">"
            "        <ECProperty propertyName=\"NestPropString\" typeName=\"string\" />"
            "        <ECArrayProperty propertyName=\"ManufacturerArray\" typeName=\"Manufacturer\"  minOccurs=\"0\" maxOccurs=\"unbounded\" />"
            "    </ECClass>"
            "    <ECClass typeName=\"ClassWithStructArray\" isStruct=\"True\" isDomainClass=\"True\">"
            "        <ECArrayProperty propertyName=\"StructArray\" typeName=\"AllPrimitives\"  minOccurs=\"0\" maxOccurs=\"unbounded\" />"
            "        <ECStructProperty propertyName=\"StructMember\" typeName=\"AllPrimitives\" />"
            "        <ECArrayProperty propertyName=\"ComplicatedStructArray\" typeName=\"NestedStructArray\"  minOccurs=\"0\" maxOccurs=\"unbounded\" />"
            "    </ECClass>"
            "    <ECClass typeName=\"ClassWithPolymorphicStructArray\" isStruct=\"True\" isDomainClass=\"True\">"
            "        <ECArrayProperty propertyName=\"PolymorphicStructArray\" typeName=\"BaseClass0\"  minOccurs=\"0\" maxOccurs=\"unbounded\" />"
            "    </ECClass>"
            "    <ECClass typeName=\"BaseClass0\" isStruct=\"True\" isDomainClass=\"True\">"
            "        <ECProperty propertyName=\"BaseIntProperty\" typeName=\"int\" />"
            "    </ECClass>"
            "    <ECClass typeName=\"DerivedClass0\" isStruct=\"True\" isDomainClass=\"True\">"
            "        <BaseClass>BaseClass0</BaseClass>"
            "        <ECProperty propertyName=\"DerivedStringProperty\" typeName=\"string\" />"
            "    </ECClass>"
            "    <ECClass typeName=\"DerivedClass1\" isStruct=\"True\" isDomainClass=\"True\">"
            "        <BaseClass>BaseClass0</BaseClass>"
            "        <ECProperty propertyName=\"DerivedDoubleProperty\" typeName=\"double\" />"
            "    </ECClass>"
            "</ECSchema>";

        Utf8String buff;
        buff.Sprintf(fmt, schemaName, versionMajor, versionMinor, className);

        return buff;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    12/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    ECSchemaPtr CreateTestSchema()
        {
        Utf8String schemaXMLString = GetTestSchemaXMLString("TestSchema", 0, 0, "TestClass");

        ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();

        ECSchemaPtr schema;
        EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXMLString.c_str(), *schemaContext));

        return schema;
        }

    typedef std::vector<Utf8String> NameVector;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    CaseyMullen    01/10
    +---------------+---------------+---------------+---------------+---------------+------*/
    ECSchemaPtr CreateProfilingSchema(int nStrings)
        {
        s_propertyNames.clear();

        Utf8String schemaXml =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<ECSchema schemaName=\"ProfilingSchema\" nameSpacePrefix=\"p\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            "    <ECClass typeName=\"Pidget\" isDomainClass=\"True\">";

        for (int i = 0; i < nStrings; i++)
            {
            Utf8String propertyName;
            propertyName.Sprintf("StringProperty%02d", i);
            s_propertyNames.push_back(propertyName);
            Utf8CP propertyFormat =
                "        <ECProperty propertyName=\"%s\" typeName=\"string\" />";
            Utf8String propertyXml;
            propertyXml.Sprintf(propertyFormat, propertyName.c_str());
            schemaXml += propertyXml;
            }

        schemaXml += "    </ECClass>"
            "</ECSchema>";

        ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();

        ECSchemaPtr schema;
        EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *schemaContext));

        return schema;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Adam.Klatzkin                   02/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ExerciseVariableCountIntArray(IECInstanceR instance, ECValue& v, Utf8Char* arrayAccessor, int baseValue)
        {
        // test insertion in an empty array
        ASSERT_TRUE(ECObjectsStatus::Success == instance.InsertArrayElements(arrayAccessor, 0, 5));
        VerifyArrayInfo(instance, v, arrayAccessor, 5, false);
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 0, 5, true);
        SetAndVerifyIntegerArray(instance, v, arrayAccessor, baseValue, 5);
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 0, 5, false);
        VerifyOutOfBoundsError(instance, v, arrayAccessor, 5);
        // test insertion in the middle of an array
        ASSERT_TRUE(ECObjectsStatus::Success == instance.InsertArrayElements(arrayAccessor, 3, 3));
        VerifyArrayInfo(instance, v, arrayAccessor, 8, false);
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 0, 3, false);
        VerifyIntegerArray(instance, v, arrayAccessor, baseValue, 0, 3);
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 3, 3, true);
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 6, 2, false);
        VerifyIntegerArray(instance, v, arrayAccessor, baseValue + 3, 6, 2);
        SetAndVerifyIntegerArray(instance, v, arrayAccessor, baseValue, 8);
        // test insertion at the beginning of an array
        ASSERT_TRUE(ECObjectsStatus::Success == instance.InsertArrayElements(arrayAccessor, 0, 4));
        VerifyArrayInfo(instance, v, arrayAccessor, 12, false);
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 0, 4, true);
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 4, 8, false);
        VerifyIntegerArray(instance, v, arrayAccessor, baseValue, 4, 8);
        SetAndVerifyIntegerArray(instance, v, arrayAccessor, baseValue, 12);
        // test insertion at the end of an array
        ASSERT_TRUE(ECObjectsStatus::Success == instance.AddArrayElements(arrayAccessor, 2));
        VerifyArrayInfo(instance, v, arrayAccessor, 14, false);
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 12, 2, true);
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 0, 12, false);
        VerifyIntegerArray(instance, v, arrayAccessor, baseValue, 0, 12);
        SetAndVerifyIntegerArray(instance, v, arrayAccessor, baseValue, 14);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Adam.Klatzkin                   02/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ExerciseVariableCountStringArray(IECInstanceR instance, ECValue& v, Utf8Char* arrayAccessor, Utf8Char* stringSeed)
        {
        // test insertion in an empty array
        ASSERT_TRUE(ECObjectsStatus::Success == instance.InsertArrayElements(arrayAccessor, 0, 5));
        VerifyArrayInfo(instance, v, arrayAccessor, 5, false);
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 0, 5, true);
        SetAndVerifyStringArray(instance, v, arrayAccessor, stringSeed, 5);
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 0, 5, false);
        VerifyOutOfBoundsError(instance, v, arrayAccessor, 5);
        // test insertion in the middle of an array
        ASSERT_TRUE(ECObjectsStatus::Success == instance.InsertArrayElements(arrayAccessor, 3, 3));
        VerifyArrayInfo(instance, v, arrayAccessor, 8, false);
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 0, 3, false);
        VerifyStringArray(instance, v, arrayAccessor, stringSeed, 0, 3);
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 3, 3, true);
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 6, 2, false);
        Utf8String stringSeedXXX(stringSeed);
        stringSeedXXX.append("XXX");
        VerifyStringArray(instance, v, arrayAccessor, stringSeedXXX.c_str(), 6, 2);
        SetAndVerifyStringArray(instance, v, arrayAccessor, stringSeed, 8);
        // test insertion at the beginning of an array
        ASSERT_TRUE(ECObjectsStatus::Success == instance.InsertArrayElements(arrayAccessor, 0, 4));
        VerifyArrayInfo(instance, v, arrayAccessor, 12, false);
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 0, 4, true);
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 4, 8, false);
        VerifyStringArray(instance, v, arrayAccessor, stringSeed, 4, 8);
        SetAndVerifyStringArray(instance, v, arrayAccessor, stringSeed, 12);
        // test insertion at the end of an array
        ASSERT_TRUE(ECObjectsStatus::Success == instance.AddArrayElements(arrayAccessor, 2));
        VerifyArrayInfo(instance, v, arrayAccessor, 14, false);
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 12, 2, true);
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 0, 12, false);
        VerifyStringArray(instance, v, arrayAccessor, stringSeed, 0, 12);
        SetAndVerifyStringArray(instance, v, arrayAccessor, stringSeed, 14);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Adam.Klatzkin                   02/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyVariableCountManufacturerArray(IECInstanceR instance, ECValue& v, Utf8Char* arrayAccessor)
        {
        VerifyArrayInfo(instance, v, arrayAccessor, 4, false);
        EXPECT_TRUE(ECObjectsStatus::Success == instance.GetValue(v, arrayAccessor));
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 0, 4, false);
        EXPECT_TRUE(ECObjectsStatus::Success == instance.GetValue(v, arrayAccessor, 0));
        EXPECT_TRUE(v.IsStruct());
        IECInstancePtr manufInst = v.GetStruct();
        VerifyString(*manufInst, v, "Name", "Nissan");
        VerifyInteger(*manufInst, v, "AccountNo", 3475);
        EXPECT_TRUE(ECObjectsStatus::Success == instance.GetValue(v, arrayAccessor, 1));
        EXPECT_TRUE(v.IsStruct());
        manufInst = v.GetStruct();
        VerifyString(*manufInst, v, "Name", "Ford");
        VerifyInteger(*manufInst, v, "AccountNo", 381);
        EXPECT_TRUE(ECObjectsStatus::Success == instance.GetValue(v, arrayAccessor, 2));
        EXPECT_TRUE(v.IsStruct());
        manufInst = v.GetStruct();
        VerifyString(*manufInst, v, "Name", "Chrysler");
        VerifyInteger(*manufInst, v, "AccountNo", 81645);
        EXPECT_TRUE(ECObjectsStatus::Success == instance.GetValue(v, arrayAccessor, 3));
        EXPECT_TRUE(v.IsStruct());
        manufInst = v.GetStruct();
        VerifyString(*manufInst, v, "Name", "Toyota");
        VerifyInteger(*manufInst, v, "AccountNo", 6823);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Adam.Klatzkin                   02/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ExerciseVariableCountManufacturerArray(IECInstanceR instance, StandaloneECEnablerR manufacturerEnabler, ECValue& v, Utf8Char* arrayAccessor)
        {
        VerifyArrayInfo(instance, v, arrayAccessor, 0, false);

        // create an array of two values
        ASSERT_TRUE(ECObjectsStatus::Success == instance.AddArrayElements(arrayAccessor, 2));
        VerifyArrayInfo(instance, v, arrayAccessor, 2, false);
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 0, 2, true);
        IECInstancePtr manufInst = manufacturerEnabler.CreateInstance().get();
        SetAndVerifyString(*manufInst, v, "Name", "Nissan");
        SetAndVerifyInteger(*manufInst, v, "AccountNo", 3475);
        v.SetStruct(manufInst.get());
        ASSERT_TRUE(ECObjectsStatus::Success == instance.SetValue(arrayAccessor, v, 0));
        manufInst = manufacturerEnabler.CreateInstance().get();
        SetAndVerifyString(*manufInst, v, "Name", "Kia");
        SetAndVerifyInteger(*manufInst, v, "AccountNo", 1791);
        v.SetStruct(manufInst.get());
        ASSERT_TRUE(ECObjectsStatus::Success == instance.SetValue(arrayAccessor, v, 1));
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 0, 2, false);

        // insert two elements in the middle of the array   
        ASSERT_TRUE(ECObjectsStatus::Success == instance.InsertArrayElements(arrayAccessor, 1, 2));
        VerifyArrayInfo(instance, v, arrayAccessor, 4, false);
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 0, 1, false);
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 1, 2, true);
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 3, 1, false);
        manufInst = manufacturerEnabler.CreateInstance().get();
        SetAndVerifyString(*manufInst, v, "Name", "Ford");
        SetAndVerifyInteger(*manufInst, v, "AccountNo", 381);
        v.SetStruct(manufInst.get());
        ASSERT_TRUE(ECObjectsStatus::Success == instance.SetValue(arrayAccessor, v, 1));
        manufInst = manufacturerEnabler.CreateInstance().get();
        SetAndVerifyString(*manufInst, v, "Name", "Chrysler");
        SetAndVerifyInteger(*manufInst, v, "AccountNo", 81645);
        v.SetStruct(manufInst.get());
        ASSERT_TRUE(ECObjectsStatus::Success == instance.SetValue(arrayAccessor, v, 2));
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 0, 4, false);

        // ensure we can set a struct array value to NULL        
        v.SetToNull();
        ASSERT_TRUE(ECObjectsStatus::Success == instance.SetValue(arrayAccessor, v, 3));
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 0, 3, false);
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 3, 1, true);
        manufInst = manufacturerEnabler.CreateInstance().get();
        SetAndVerifyString(*manufInst, v, "Name", "Acura");
        SetAndVerifyInteger(*manufInst, v, "AccountNo", 6);
        v.SetStruct(manufInst.get());
        ASSERT_TRUE(ECObjectsStatus::Success == instance.SetValue(arrayAccessor, v, 3));
        VerifyIsNullArrayElements(instance, v, arrayAccessor, 3, 1, false);
        manufInst = manufacturerEnabler.CreateInstance().get();
        SetAndVerifyString(*manufInst, v, "Name", "Toyota");
        SetAndVerifyInteger(*manufInst, v, "AccountNo", 6823);
        v.SetStruct(manufInst.get());
        ASSERT_TRUE(ECObjectsStatus::Success == instance.SetValue(arrayAccessor, v, 3));

        // ensure we can't set the array elements to other primitive types
        {
        DISABLE_ASSERTS
            v.SetInteger(35);
        ASSERT_TRUE(ECObjectsStatus::DataTypeNotSupported == instance.SetValue(arrayAccessor, v, 1));
        v.SetUtf8CP("foobar");
        ASSERT_TRUE(ECObjectsStatus::DataTypeNotSupported == instance.SetValue(arrayAccessor, v, 1));
        }

        // ensure we can not set the array to an instance of a struct that is not of the type (or derived from the type) of the property    
        ECClassCP incompatibleClass = manufacturerEnabler.GetClass().GetSchema().GetClassCP("AllPrimitives");
        ASSERT_TRUE(NULL != incompatibleClass);

        StandaloneECInstancePtr incompatibleInstance = incompatibleClass->GetDefaultStandaloneEnabler()->CreateInstance();
        v.SetStruct(incompatibleInstance.get());
        ASSERT_TRUE(ECObjectsStatus::UnableToSetStructArrayMemberInstance == instance.SetValue(arrayAccessor, v, 0));

        VerifyVariableCountManufacturerArray(instance, v, arrayAccessor);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    CaseyMullen     10/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ExerciseInstance(IECInstanceR instance, Utf8Char* valueForFinalStrings)
        {
        ECValue v;
        instance.GetValue(v, "D");

        double doubleValue = 1.0 / 3.0;
        SetAndVerifyDouble(instance, v, "D", doubleValue);

        SetAndVerifyInteger(instance, v, "A", 97);
        SetAndVerifyInteger(instance, v, "AA", 12);

        SetAndVerifyString(instance, v, "B", "Happy");
        SetAndVerifyString(instance, v, "B", "Very Happy");
        SetAndVerifyString(instance, v, "B", "sad");
        SetAndVerifyString(instance, v, "S", "Lucky");
        SetAndVerifyString(instance, v, "B", "Very Very Happy");
        VerifyString(instance, v, "S", "Lucky");
        SetAndVerifyString(instance, v, "Manufacturer.Name", "Charmed");
        SetAndVerifyString(instance, v, "S", "Lucky Strike");

        Utf8Char largeString[3300];
        largeString[0] = L'\0';
        for (int i = 0; i < 20; i++)
            strcat(largeString, "S2345678901234567890123456789012");

        SetAndVerifyString(instance, v, "S", largeString);

        for (int i = 0; i < N_FINAL_STRING_PROPS_IN_FAKE_CLASS; i++)
            {
            Utf8String propertyName;
            propertyName.Sprintf("Property%i", i);
            SetAndVerifyString(instance, v, propertyName.c_str(), valueForFinalStrings);
            }

#ifdef THESE_TESTS_DUPLICATE_PUBLISHED_TESTS
        VerifyArrayInfo(instance, v, "BeginningArray", 0, false);
        VerifyArrayInfo(instance, v, "FixedArrayFixedElement", 10, true);
        VerifyArrayInfo(instance, v, "VariableArrayFixedElement", 0, false);
        VerifyArrayInfo(instance, v, "FixedArrayVariableElement", 12, true);
        VerifyArrayInfo(instance, v, "VariableArrayVariableElement", 0, false);
        VerifyArrayInfo(instance, v, "EndingArray", 0, false);

        VerifyIsNullArrayElements(instance, v, "FixedArrayFixedElement", 0, 10, true);
        SetAndVerifyIntegerArray(instance, v, "FixedArrayFixedElement", 172, 10);
        VerifyIsNullArrayElements(instance, v, "FixedArrayFixedElement", 0, 10, false);
        SetAndVerifyIntegerArray(instance, v, "FixedArrayFixedElement", 283, 10);

        VerifyIsNullArrayElements(instance, v, "FixedArrayVariableElement", 0, 12, true);
        SetAndVerifyStringArray(instance, v, "FixedArrayVariableElement", "BaseString", 12);
        VerifyIsNullArrayElements(instance, v, "FixedArrayVariableElement", 0, 12, false);
        SetAndVerifyStringArray(instance, v, "FixedArrayVariableElement", "LaaaaaaargeString", 10);
        VerifyStringArray(instance, v, "FixedArrayVariableElement", "BaseStringXXXXXXXXXX", 10, 2);
        SetAndVerifyStringArray(instance, v, "FixedArrayVariableElement", "XString", 12);
#endif

        ExerciseVariableCountStringArray(instance, v, "BeginningArray", "BAValue");
        ExerciseVariableCountIntArray(instance, v, "VariableArrayFixedElement", 57);
        ExerciseVariableCountStringArray(instance, v, "VariableArrayVariableElement", "Var+Var");
        ExerciseVariableCountStringArray(instance, v, "EndingArray", "EArray");

        ECClassCP manufacturerClass = instance.GetClass().GetSchema().GetClassCP("Manufacturer");
        ASSERT_TRUE(NULL != manufacturerClass);

        StandaloneECEnablerPtr manufEnabler = manufacturerClass->GetDefaultStandaloneEnabler();
        ExerciseVariableCountManufacturerArray(instance, *manufEnabler, v, "ManufacturerArray");

        // Make sure that everything still has the final value that we expected
        VerifyString(instance, v, "S", largeString);
        VerifyInteger(instance, v, "A", 97);
        VerifyDouble(instance, v, "D", doubleValue);
        VerifyInteger(instance, v, "AA", 12);
        VerifyString(instance, v, "B", "Very Very Happy");
        VerifyString(instance, v, "Manufacturer.Name", "Charmed");
        for (int i = 0; i < N_FINAL_STRING_PROPS_IN_FAKE_CLASS; i++)
            {
            Utf8String propertyName;
            propertyName.Sprintf("Property%i", i);
            VerifyString(instance, v, propertyName.c_str(), valueForFinalStrings);
            }

        VerifyArrayInfo(instance, v, "BeginningArray", 14, false);
        VerifyIsNullArrayElements(instance, v, "BeginningArray", 0, 14, false);
        VerifyStringArray(instance, v, "BeginningArray", "BAValue", 0, 14);
        VerifyIsNullArrayElements(instance, v, "VariableArrayFixedElement", 0, 14, false);
        VerifyIntegerArray(instance, v, "VariableArrayFixedElement", 57, 0, 14);
        VerifyArrayInfo(instance, v, "VariableArrayVariableElement", 14, false);
        VerifyIsNullArrayElements(instance, v, "VariableArrayVariableElement", 0, 14, false);
        VerifyStringArray(instance, v, "VariableArrayVariableElement", "Var+Var", 0, 14);
        VerifyArrayInfo(instance, v, "EndingArray", 14, false);
        VerifyArrayInfo(instance, v, "VariableArrayFixedElement", 14, false);
        VerifyIsNullArrayElements(instance, v, "EndingArray", 0, 14, false);
        VerifyStringArray(instance, v, "EndingArray", "EArray", 0, 14);
        VerifyVariableCountManufacturerArray(instance, v, "ManufacturerArray");

#ifdef THESE_TESTS_DUPLICATE_PUBLISHED_TESTS
        VerifyArrayInfo(instance, v, "FixedArrayFixedElement", 10, true);
        VerifyIntegerArray(instance, v, "FixedArrayFixedElement", 283, 0, 10);
        VerifyArrayInfo(instance, v, "FixedArrayVariableElement", 12, true);
        VerifyIsNullArrayElements(instance, v, "FixedArrayVariableElement", 0, 12, false);
        VerifyStringArray(instance, v, "FixedArrayVariableElement", "XString", 0, 12);
#endif

        instance.ToString("").c_str();
        }
    };

std::vector<Utf8String> NonPublishedMemoryLayoutTests::s_propertyNames;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static void  checkValue(Utf8CP accessString, ECValueCR value, ECN::StandaloneECInstancePtr& instance)
    {
    uint32_t propertyIndex = 0;
    bool isSet = false;

    ECValue ecValue;

    EXPECT_TRUE(ECObjectsStatus::Success == instance->GetEnabler().GetPropertyIndex(propertyIndex, accessString));
    EXPECT_TRUE(ECObjectsStatus::Success == instance->GetValue(ecValue, propertyIndex));
    EXPECT_TRUE(ecValue.Equals(value));

    EXPECT_TRUE(ECObjectsStatus::Success == instance->IsPerPropertyBitSet(isSet, (uint8_t) PROPERTYFLAGINDEX_IsLoaded, propertyIndex));
    EXPECT_TRUE(true == isSet);
    EXPECT_TRUE(ECObjectsStatus::Success == instance->IsPerPropertyBitSet(isSet, (uint8_t) PROPERTYFLAGINDEX_IsReadOnly, propertyIndex));
    EXPECT_TRUE((0 == propertyIndex % 2) == isSet);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static void  setValue(Utf8CP accessString, ECValueCR value, ECN::StandaloneECInstancePtr& instance)
    {
    uint32_t propertyIndex = 0;
    bool isSet = false;

    EXPECT_TRUE(ECObjectsStatus::Success == instance->GetEnabler().GetPropertyIndex(propertyIndex, accessString));
    EXPECT_TRUE(ECObjectsStatus::Success == instance->SetValue(propertyIndex, value));

    EXPECT_TRUE(ECObjectsStatus::Success == instance->IsPerPropertyBitSet(isSet, (uint8_t) PROPERTYFLAGINDEX_IsLoaded, propertyIndex));
    EXPECT_TRUE(true == isSet) << "IECInstance::IsPerPropertyBitSet for property " << accessString;
    EXPECT_TRUE(ECObjectsStatus::Success == instance->SetPerPropertyBit((uint8_t) PROPERTYFLAGINDEX_IsReadOnly, propertyIndex, 0 == propertyIndex % 2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NonPublishedMemoryLayoutTests, CheckPerPropertyFlags)
    {
    ECSchemaPtr schema = CreateTestSchema();
    ASSERT_TRUE(schema.IsValid());
    ECClassP ecClass = schema->GetClassP("CadData");
    ASSERT_TRUE(NULL != ecClass);

    StandaloneECEnablerPtr enabler = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    uint8_t numBitPerProperty = instance->GetNumBitsInPerPropertyFlags();
    EXPECT_TRUE(numBitPerProperty == 2);

    DPoint2d   inSize = {10.5, 22.3};
    DPoint3d   inPoint1 = {10.10, 11.11, 12.12};
    DPoint3d   inPoint2 = {100.100, 110.110, 120.120};
    DateTime   inTime = DateTime::GetCurrentTimeUtc();
    int        inCount = 100;
    double     inLength = 432.178;
    bool       inTest = true;
    int64_t    inTicks = 634027121070910000;

    ECValue ecValue;
    ecValue.SetDateTimeTicks(inTicks);

    setValue("Count", ECValue(inCount), instance);
    setValue("Name", ECValue("Test"), instance);
    setValue("Length", ECValue(inLength), instance);
    setValue("Field_Tested", ECValue(inTest), instance);
    setValue("Size", ECValue(inSize), instance);
    setValue("StartPoint", ECValue(inPoint1), instance);
    setValue("EndPoint", ECValue(inPoint2), instance);
    setValue("Service_Date", ECValue(inTime), instance);
    setValue("Install_Date", ecValue, instance);

    checkValue("Count", ECValue(inCount), instance);
    checkValue("Name", ECValue("Test"), instance);
    checkValue("Length", ECValue(inLength), instance);
    checkValue("Field_Tested", ECValue(inTest), instance);
    checkValue("Size", ECValue(inSize), instance);
    checkValue("StartPoint", ECValue(inPoint1), instance);
    checkValue("EndPoint", ECValue(inPoint2), instance);
    checkValue("Service_Date", ECValue(inTime), instance);
    checkValue("Install_Date", ecValue, instance);

    bool isSet, lastPropertyEncountered = false;
    uint32_t propertyIndex = 0;
    instance->ClearAllPerPropertyFlags();
    ECObjectsStatus status;

    while (!lastPropertyEncountered)
        {
        for (uint8_t i = 0; i < numBitPerProperty; i++)
            {
            status = instance->IsPerPropertyBitSet(isSet, i, propertyIndex);
            // break when property index exceeds actual property count
            if (ECObjectsStatus::Success == status)
                {
                EXPECT_TRUE(false == isSet);
                }
            else
                {
                lastPropertyEncountered = true;
                break;
                }
            }

        propertyIndex++;
        }
    };

TEST_F(NonPublishedMemoryLayoutTests, PropertyLayoutBracketsTest)
    {
    // ClassLayout maintains a vector of PropertyLayouts sorted by access string.
    // We discovered a defect in which the access string used for sorting did not include the brackets[] for array properties, causing lookup to fail.
    // This test confirms that defect is corrected.
    Utf8Char schemaXml[] =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"BracketTestSchema\" nameSpacePrefix=\"bts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "<ECClass typeName=\"BracketTestClass\" isDomainClass=\"True\">"
        "<ECProperty propertyName=\"B0\" typeName=\"string\" />"
        "<ECProperty propertyName=\"B1\" typeName=\"string\" />"
        "<ECProperty propertyName=\"B2\" typeName=\"string\" />"
        "<ECProperty propertyName=\"B3\" typeName=\"string\" />"
        "<ECProperty propertyName=\"B4\" typeName=\"string\" />"
        "<ECProperty propertyName=\"B5\" typeName=\"string\" />"
        "<ECArrayProperty propertyName=\"B\" typeName=\"string\" minOccurs=\"0\" maxOccurs=\"unbounded\" />"
        "</ECClass>"
        "</ECSchema>";

    // If brackets are omitted, then "B" precedes "B0"
    // Else, "B0" preceds "B"
    // The order of declaration of properties in the schema matters here.
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext));

    ECClassP ecClass = schema->GetClassP("BracketTestClass");
    ASSERT_TRUE(NULL != ecClass);

    ClassLayoutPtr layout = ClassLayout::BuildFromClass(*ecClass);
    ASSERT_TRUE(layout.IsValid());

    PropertyLayoutCP propLayout;
    EXPECT_EQ(ECObjectsStatus::Success, layout->GetPropertyLayout(propLayout, "B"));   // would have failed prior to bug fix
    EXPECT_EQ(ECObjectsStatus::Success, layout->GetPropertyLayout(propLayout, "B0"));
    }

TEST_F(NonPublishedMemoryLayoutTests, ExpectCorrectPrimitiveTypeForNullValues)
    {
    ECSchemaPtr      schema = CreateTestSchema();
    ASSERT_TRUE(schema.IsValid());

    ECClassP ecClass = schema->GetClassP("AllPrimitives");
    ASSERT_TRUE(NULL != ecClass);

    StandaloneECEnablerPtr enabler = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    ECValue v;
    EXPECT_TRUE(ECObjectsStatus::Success == instance->GetValue(v, "AString"));
    EXPECT_TRUE(v.IsNull());
    EXPECT_TRUE(v.IsPrimitive());
    EXPECT_TRUE(v.IsString());

    EXPECT_TRUE(ECObjectsStatus::Success == instance->GetValue(v, "ABoolean"));
    EXPECT_TRUE(v.IsNull());
    EXPECT_TRUE(v.IsPrimitive());
    EXPECT_TRUE(v.IsBoolean());

    ecClass = schema->GetClassP("NestedStructArray");
    ASSERT_TRUE(NULL != ecClass);
    enabler = ecClass->GetDefaultStandaloneEnabler();
    instance = enabler->CreateInstance();

    EXPECT_TRUE(ECObjectsStatus::Success == instance->GetValue(v, "ManufacturerArray"));
    EXPECT_TRUE(v.IsArray());
    EXPECT_TRUE(ECObjectsStatus::Success == instance->AddArrayElements("ManufacturerArray", 2));
    EXPECT_TRUE(ECObjectsStatus::Success == instance->GetValue(v, "ManufacturerArray", 0));
    EXPECT_TRUE(v.IsStruct());
    EXPECT_TRUE(v.IsNull());

    ecClass = schema->GetClassP("FixedSizeArrayTester");
    ASSERT_TRUE(NULL != ecClass);
    enabler = ecClass->GetDefaultStandaloneEnabler();
    instance = enabler->CreateInstance();

#ifndef FIXED_COUNT_ARRAYS_ARE_SUPPORTED
    EXPECT_TRUE(ECObjectsStatus::Success == instance->AddArrayElements("Struct1", 1));
#endif

    EXPECT_TRUE(ECObjectsStatus::Success == instance->GetValue(v, "Struct1"));
    EXPECT_TRUE(v.IsArray());
    EXPECT_TRUE(ECObjectsStatus::Success == instance->GetValue(v, "Struct1", 0));
    EXPECT_TRUE(v.IsStruct());
    EXPECT_TRUE(v.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* MemoryECInstanceBase stores supporting instances as StandaloneECInstances.
* For efficiency we want to avoid making a copy of the supporting instance when setting
* it to the parent's struct array.
* But we also want to avoid having more than one parent instance claim ownership of
* a single supporting instance; otherwise modifying one would modify the other.
* @bsimethod                                                    Paul.Connelly   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NonPublishedMemoryLayoutTests, SupportingInstanceOwnership)
    {
    ECSchemaPtr schema = CreateTestSchema();
    ECClassP parentClass = schema->GetClassP("NestedStructArray");    // contains an array of Manufacturer structs
    ECClassP structClass = schema->GetClassP("Manufacturer");

    StandaloneECInstancePtr originalStruct = structClass->GetDefaultStandaloneEnabler()->CreateInstance();

    StandaloneECInstancePtr parentA = parentClass->GetDefaultStandaloneEnabler()->CreateInstance();
    StandaloneECInstancePtr parentB = parentClass->GetDefaultStandaloneEnabler()->CreateInstance();

    parentA->AddArrayElements("ManufacturerArray", 1);
    parentB->AddArrayElements("ManufacturerArray", 1);

    ECValue structVal;
    structVal.SetStruct(originalStruct.get());

    parentA->SetValue("ManufacturerArray", structVal, 0);
    parentB->SetValue("ManufacturerArray", structVal, 0);

    parentA->GetValue(structVal, "ManufacturerArray", 0);
    EXPECT_EQ(structVal.GetStruct().get(), originalStruct.get());

    parentB->GetValue(structVal, "ManufacturerArray", 0);
    EXPECT_NE(structVal.GetStruct().get(), originalStruct.get());
    }

/*---------------------------------------------------------------------------------**//**
* When we duplicate instances, we want to make sure that supporting instances are copied
* recursively.
* @bsimethod                                                    Paul.Connelly   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NonPublishedMemoryLayoutTests, CopyRecursiveSupportingInstances)
    {
    ECSchemaPtr schema = CreateTestSchema();

    // populate our source instance with nested struct arrays
    IECInstancePtr outer = schema->GetClassP("ClassWithStructArray")->GetDefaultStandaloneEnabler()->CreateInstance(),
        middle = schema->GetClassP("NestedStructArray")->GetDefaultStandaloneEnabler()->CreateInstance(),
        inner = schema->GetClassP("Manufacturer")->GetDefaultStandaloneEnabler()->CreateInstance();

    inner->SetValue("Name", ECValue("Hooray!"));

    ECValue structVal;
    middle->AddArrayElements("ManufacturerArray", 1);
    structVal.SetStruct(inner.get());
    middle->SetValue("ManufacturerArray", structVal, 0);

    outer->AddArrayElements("ComplicatedStructArray", 1);
    structVal.SetStruct(middle.get());
    outer->SetValue("ComplicatedStructArray", structVal, 0);

    // make a copy
    StandaloneECInstancePtr outerCopy = outer->GetEnabler().GetClass().GetDefaultStandaloneEnabler()->CreateInstance();
    outerCopy->CopyValues(*outer);

    // confirm the nested struct array instances have been copied as well
    outerCopy->GetValue(structVal, "ComplicatedStructArray", 0);
    IECInstancePtr middleCopy = structVal.GetStruct();
    EXPECT_NE(middleCopy.get(), middle.get());

    middleCopy->GetValue(structVal, "ManufacturerArray", 0);
    IECInstancePtr innerCopy = structVal.GetStruct();
    EXPECT_NE(innerCopy.get(), inner.get());

    // modify the original deepest supporting instance
    ECValue v("Oh no!");
    inner->SetValue("Name", v);

    // confirm our copy of the deepest supporting instance remains intact
    innerCopy->GetValue(v, "Name");
    EXPECT_EQ(0, strcmp(v.GetUtf8CP(), "Hooray!"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECDBufferTests : NonPublishedMemoryLayoutTests
    {
    template<typename T> void TestIsEmpty(IECInstanceR instance, Utf8CP accessor, T const& value)
        {
        ECDBuffer* buf = instance.GetECDBufferP();
        EXPECT_TRUE(buf->IsEmpty());

        ECValue v(value);
        EXPECT_EQ(ECObjectsStatus::Success, instance.SetValue(accessor, v));

        EXPECT_FALSE(buf->IsEmpty());

        v.Clear();
        EXPECT_EQ(ECObjectsStatus::Success, instance.SetValue(accessor, v));

        bool isNull = false;
        EXPECT_EQ(ECObjectsStatus::Success, instance.IsPropertyNull(isNull, accessor));
        EXPECT_TRUE(isNull);
        EXPECT_TRUE(buf->IsEmpty()) << accessor;

        buf->ClearValues();
        EXPECT_TRUE(buf->IsEmpty());
        }

    template<typename T> void TestIsEmptyArray(IECInstanceR instance, Utf8CP accessor, T const& value)
        {
        ECDBuffer& buf = *instance.GetECDBufferP();
        EXPECT_TRUE(buf.IsEmpty());

        EXPECT_EQ(ECObjectsStatus::Success, instance.AddArrayElements(accessor, 1));
        bool isNull = false;
        EXPECT_EQ(ECObjectsStatus::Success, instance.IsPropertyNull(isNull, accessor, 0));
        EXPECT_TRUE(isNull);
        EXPECT_FALSE(buf.IsEmpty());   // a non-empty array containing null elements => a non-empty IECInstance

        ECValue v(value);
        EXPECT_EQ(ECObjectsStatus::Success, instance.SetValue(accessor, v, 0));
        EXPECT_EQ(ECObjectsStatus::Success, instance.IsPropertyNull(isNull, accessor, 0));
        EXPECT_FALSE(isNull);
        EXPECT_FALSE(buf.IsEmpty());

        // Clearing out the array will not reset the null flag for the array property. But an empty array => empty IECInstance
        EXPECT_EQ(ECObjectsStatus::Success, instance.ClearArray(accessor));
        EXPECT_EQ(ECObjectsStatus::Success, instance.GetValue(v, accessor));
        EXPECT_EQ(0, v.GetArrayInfo().GetCount());
        EXPECT_TRUE(buf.IsEmpty()) << accessor;

        buf.ClearValues();
        EXPECT_TRUE(buf.IsEmpty());
        }

    struct ExpectedValue
        {
        ECValue         m_value;
        bool            m_expectExists;

        template<typename T> ExpectedValue(T const& val) : m_value(val), m_expectExists(true) {}
        ExpectedValue() : m_expectExists(false) {}
        };

    void TestValue(IECInstanceCR instance, Utf8CP accessor, ExpectedValue const& val, uint32_t arrayIndex = -1)
        {
        ECValue v;
        ECObjectsStatus status = -1 != arrayIndex ? instance.GetValue(v, accessor, arrayIndex) : instance.GetValue(v, accessor);
        EXPECT_EQ((ECObjectsStatus::Success == status), val.m_expectExists) << " for property " << accessor;

        if (ECObjectsStatus::Success == status)
            {
            if (val.m_expectExists)
                EXPECT_TRUE(val.m_value.Equals(v)) << "Expected: " << val.m_value.ToString().c_str() << " Actual: " << v.ToString().c_str() << " for property " << accessor;
            else
                printf("Expected: non-existent Actual: %s for property %s\n", v.ToString().c_str(), accessor);
            }
        }
    };

/*---------------------------------------------------------------------------------**//**
* Test the ECDBuffer::IsEmpty() method. Should return true if all values are null and
* all arrays are empty.
* @bsimethod                                                    Paul.Connelly   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDBufferTests, IsEmpty)
    {
    ECSchemaPtr schema = CreateTestSchema();

    IECInstancePtr instance = schema->GetClassP("Manufacturer")->GetDefaultStandaloneEnabler()->CreateInstance();
    TestIsEmpty(*instance, "AccountNo", 12345);   // fixed-sized property
    TestIsEmpty(*instance, "Name", "Ed");        // variable-sized property

    instance = schema->GetClassP("AllPrimitives")->GetDefaultStandaloneEnabler()->CreateInstance();
    TestIsEmptyArray(*instance, "SomeInts", 54321);
    TestIsEmptyArray(*instance, "SomeStrings", "abcdefg");

    instance = schema->GetClassP("ClassWithStructArray")->GetDefaultStandaloneEnabler()->CreateInstance();
    TestIsEmpty(*instance, "StructMember.AnInt", 12345);
    TestIsEmpty(*instance, "StructMember.AString", "bbbbb");
    TestIsEmptyArray(*instance, "StructMember.SomeInts", 54321);
    TestIsEmptyArray(*instance, "StructMember.SomeStrings", "lalalalala");
    }

/*---------------------------------------------------------------------------------**//**
* Simplification of above test to isolate some memory corruption when clearing the array.
* @bsimethod                                                    Paul.Connelly   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDBufferTests, ClearArray)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ArrayTest", "ts", 1, 0, 0);
    ECEntityClassP ecClass;
    schema->CreateEntityClass(ecClass, "TestClass");
    PrimitiveECPropertyP primProp;
    ecClass->CreatePrimitiveProperty(primProp, "Int", PRIMITIVETYPE_Integer);
    ecClass->CreatePrimitiveProperty(primProp, "String");

    PrimitiveArrayECPropertyP arrayProp;
    ecClass->CreatePrimitiveArrayProperty(arrayProp, "Ints", PRIMITIVETYPE_Integer);
    ecClass->CreatePrimitiveArrayProperty(arrayProp, "Strings", PRIMITIVETYPE_String);
    ecClass->CreatePrimitiveArrayProperty(arrayProp, "MoreInts", PRIMITIVETYPE_Integer);
    IECInstancePtr instance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();

    ECValue v;
    EXPECT_EQ(ECObjectsStatus::Success, instance->GetValue(v, "Ints"));
    EXPECT_EQ(0, v.GetArrayInfo().GetCount());
    EXPECT_EQ(ECObjectsStatus::Success, instance->GetValue(v, "Strings"));
    EXPECT_EQ(0, v.GetArrayInfo().GetCount());

    EXPECT_EQ(ECObjectsStatus::Success, instance->AddArrayElements("Ints", 1));
    EXPECT_EQ(ECObjectsStatus::Success, instance->GetValue(v, "Ints"));
    EXPECT_EQ(1, v.GetArrayInfo().GetCount());
    EXPECT_EQ(ECObjectsStatus::Success, instance->GetValue(v, "Strings"));
    EXPECT_EQ(0, v.GetArrayInfo().GetCount());

    // The problem was here:
    // After ClearArray() we fix up secondary offsets of other variable-sized properties by subtracting the number of bytes removed from the buffer
    // The Strings array had a secondary offset of zero; subtraction produced a negative offset, interpreted as positive offset into memory outside the buffer
    EXPECT_EQ(ECObjectsStatus::Success, instance->ClearArray("Ints"));
    EXPECT_EQ(ECObjectsStatus::Success, instance->GetValue(v, "Ints"));
    EXPECT_EQ(0, v.GetArrayInfo().GetCount());
    EXPECT_EQ(ECObjectsStatus::Success, instance->GetValue(v, "Strings"));
    EXPECT_EQ(0, v.GetArrayInfo().GetCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDBufferTests, MoreClearArrayTests)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "Test", "ts", 1, 0, 0);
    ECEntityClassP ecClass;
    schema->CreateEntityClass(ecClass, "Test");
    PrimitiveArrayECPropertyP arrayProp;
    ecClass->CreatePrimitiveArrayProperty(arrayProp, "Strings", PRIMITIVETYPE_String);
    PrimitiveECPropertyP primProp;
    ecClass->CreatePrimitiveProperty(primProp, "Stringy", PRIMITIVETYPE_String);

    IECInstancePtr inst = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    inst->AddArrayElements("Strings", 1);
    inst->SetValue("Strings", ECValue("String", false), 0);
    inst->ClearArray("Strings");
    }

/*---------------------------------------------------------------------------------**//**
* Test the ECValue flag that returns strings as pointers into instance data rather than
* making a copy.
* @bsimethod                                                    Paul.Connelly   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDBufferTests, PointersIntoInstanceMemory)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "InstancePointers", "ts", 1, 0, 0);
    ECEntityClassP ecClass;
    schema->CreateEntityClass(ecClass, "InstancePointers");
    PrimitiveECPropertyP ecprop;
    ecClass->CreatePrimitiveProperty(ecprop, "String", PRIMITIVETYPE_String);

    IECInstancePtr instance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    instance->SetValue("String", ECValue("string", false));

    ECValue v;
    instance->GetValue(v, "String");
    EXPECT_EQ(0, strcmp(v.GetUtf8CP(), "string"));

    // To test whether or not we got back a pointer into instance memory, we'll modify the memory. Real code would never do this of course.
    Utf8Char newStr[] = "STRING";
    Utf8Char* pStr = const_cast<Utf8Char*> (v.GetUtf8CP());
    memcpy(pStr, newStr, _countof(newStr) * sizeof(Utf8Char));

    instance->GetValue(v, "String");
    EXPECT_EQ(0, strcmp(v.GetUtf8CP(), "string"));   // did not modify instance data

    v.SetAllowsPointersIntoInstanceMemory(true);
    instance->GetValue(v, "String");

    pStr = const_cast<Utf8P> (v.GetUtf8CP());
    memcpy(pStr, newStr, _countof(newStr) * sizeof(Utf8Char));

    // The flag should not be reset when the ECValue was assigned a value
    EXPECT_TRUE(v.AllowsPointersIntoInstanceMemory());

    instance->GetValue(v, "String");
    //EXPECT_EQ (v.GetString(), pStr);                // got back pointer to same address in instance data
    //EXPECT_EQ (0, wcscmp (v.GetString(), newStr));  // modified instance memory directly through returned pointer
    }

/*---------------------------------------------------------------------------------**//**
* Test using ECDBuffer::CopyDataBuffer() to populate an ECDBuffer from another ECDBuffer
* created for a different ClassLayout.
* @bsimethod                                                    Paul.Connelly   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDBufferTests, ConvertDataBuffer)
    {
    // Create initial version of class
    ECSchemaPtr schemaA;
    ECSchema::CreateSchema(schemaA, "SchemaA", "ts", 1, 0, 0);
    ECEntityClassP classA;
    schemaA->CreateEntityClass(classA, "ClassA");

    PrimitiveECPropertyP prim;
    classA->CreatePrimitiveProperty(prim, "String", PRIMITIVETYPE_String);
    classA->CreatePrimitiveProperty(prim, "Int", PRIMITIVETYPE_Integer);
    classA->CreatePrimitiveProperty(prim, "Bool", PRIMITIVETYPE_Boolean);
    classA->CreatePrimitiveProperty(prim, "RemoveThisProperty", PRIMITIVETYPE_String);

    // initialize instance of initial class layout
    StandaloneECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    instanceA->SetValue("String", ECValue("ABC", false));
    instanceA->SetValue("Int", ECValue(123));
    instanceA->SetValue("Bool", ECValue(true));
    instanceA->SetValue("RemoveThisProperty", ECValue("stuff", false));

    // test we can copy the data buffer using the same class layout
    StandaloneECInstancePtr instanceA2 = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_EQ(ECObjectsStatus::Success, instanceA2->CopyDataBuffer(*instanceA, true));

    TestValue(*instanceA2, "String", "ABC");
    TestValue(*instanceA2, "Int", 123);
    TestValue(*instanceA2, "Bool", true);
    TestValue(*instanceA2, "RemoveThisProperty", "stuff");

    // Create a new version of the class with different layout
    ECSchemaPtr schemaA2;
    ECSchema::CreateSchema(schemaA2, "SchemaA", "ts", 2, 0 ,0);
    ECEntityClassP classA2;
    schemaA2->CreateEntityClass(classA2, "ClassA");

    classA2->CreatePrimitiveProperty(prim, "String", PRIMITIVETYPE_Integer);
    classA2->CreatePrimitiveProperty(prim, "Int", PRIMITIVETYPE_String);
    classA2->CreatePrimitiveProperty(prim, "AddedThisProperty", PRIMITIVETYPE_Double);
    classA2->CreatePrimitiveProperty(prim, "Bool", PRIMITIVETYPE_Boolean);

    // Create instance of new class layout and initialize from old layout
    instanceA2 = classA2->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_EQ(ECObjectsStatus::Success, instanceA2->CopyDataBuffer(*instanceA, true));

    TestValue(*instanceA2, "Int", "123"); // int->string converted
    TestValue(*instanceA2, "String", ECValue()); // string->int conversion failed
    TestValue(*instanceA2, "AddedThisProperty", ECValue());   // not present in old class, uninitialized
    TestValue(*instanceA2, "RemovedThisProperty", ExpectedValue());   // not present in new class
    TestValue(*instanceA2, "Bool", true); // no change, value preserved
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDBufferTests, ConvertDataBuffer_Arrays)
    {
    ECSchemaPtr schemaA;
    ECSchema::CreateSchema(schemaA, "SchemaA", "ts", 1, 0, 0);
    ECEntityClassP classA;
    schemaA->CreateEntityClass(classA, "ClassA");

    PrimitiveArrayECPropertyP prop;
    classA->CreatePrimitiveArrayProperty(prop, "IntArray", PRIMITIVETYPE_Integer);
    classA->CreatePrimitiveArrayProperty(prop, "StringArray", PRIMITIVETYPE_String);
    classA->CreatePrimitiveArrayProperty(prop, "BoolArray", PRIMITIVETYPE_Boolean);
    classA->CreatePrimitiveArrayProperty(prop, "Removed", PRIMITIVETYPE_String);

    StandaloneECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    instanceA->AddArrayElements("IntArray", 2);
    instanceA->SetValue("IntArray", ECValue(0), 0);
    instanceA->SetValue("IntArray", ECValue(1), 1);

    instanceA->AddArrayElements("StringArray", 2);
    instanceA->SetValue("StringArray", ECValue("abc"), 0);
    instanceA->SetValue("StringArray", ECValue("123"), 1);

    instanceA->AddArrayElements("BoolArray", 2);
    instanceA->SetValue("BoolArray", ECValue(false), 0);
    instanceA->SetValue("BoolArray", ECValue(true), 1);

    instanceA->AddArrayElements("Removed", 1);
    instanceA->SetValue("Removed", ECValue("stuff"), 0);

    ECSchemaPtr schemaA2;
    ECSchema::CreateSchema(schemaA2, "SchemaA", "ts", 2, 0, 0);
    ECEntityClassP classA2;
    schemaA2->CreateEntityClass(classA2, "ClassA");
    classA2->CreatePrimitiveArrayProperty(prop, "IntArray", PRIMITIVETYPE_String);
    classA2->CreatePrimitiveArrayProperty(prop, "StringArray", PRIMITIVETYPE_Integer);
    classA2->CreatePrimitiveArrayProperty(prop, "BoolArray", PRIMITIVETYPE_Boolean);
    classA2->CreatePrimitiveArrayProperty(prop, "Added", PRIMITIVETYPE_String);

    StandaloneECInstancePtr instanceA2 = classA2->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_EQ(ECObjectsStatus::Success, instanceA2->CopyDataBuffer(*instanceA, true));

    TestValue(*instanceA2, "IntArray", "0", 0);
    TestValue(*instanceA2, "IntArray", "1", 1);

    ECValue null;
    TestValue(*instanceA2, "StringArray", null, 0);
    TestValue(*instanceA2, "StringArray", ECValue(123), 1);

    TestValue(*instanceA2, "BoolArray", false, 0);
    TestValue(*instanceA2, "BoolArray", true, 1);

    TestValue(*instanceA2, "Removed", ExpectedValue(), 0);
    TestValue(*instanceA2, "Removed", ExpectedValue(), 1);

    ECValue emptyArray;
    emptyArray.SetPrimitiveArrayInfo(PRIMITIVETYPE_String, 0, false);
    TestValue(*instanceA2, "Added", emptyArray);
    }

/*---------------------------------------------------------------------------------**//**
* Copying a data buffer containing struct arrays should copy the struct identifiers
* intact, but should not copy the struct array instances themselves.
* @bsimethod                                                    Paul.Connelly   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDBufferTests, ConvertDataBuffer_StructArrays)
    {
    ECSchemaPtr schema1;
    ECSchema::CreateSchema(schema1, "Schema", "ts", 1, 0, 0);
    ECStructClassP struct1;
    schema1->CreateStructClass(struct1, "Struct");
    PrimitiveECPropertyP primProp;
    struct1->CreatePrimitiveProperty(primProp, "String", PRIMITIVETYPE_String);

    ECEntityClassP class1;
    schema1->CreateEntityClass(class1, "Class");
    StructArrayECPropertyP structArrayProp;
    class1->CreateStructArrayProperty(structArrayProp, "StructArray", struct1);

    StandaloneECInstancePtr instance1 = class1->GetDefaultStandaloneEnabler()->CreateInstance();
    instance1->AddArrayElements("StructArray", 2);
    IECInstancePtr structInstance = struct1->GetDefaultStandaloneEnabler()->CreateInstance();
    ECValue structVal;
    structVal.SetStruct(structInstance.get());
    instance1->SetValue("StructArray", structVal, 0);
    structInstance = struct1->GetDefaultStandaloneEnabler()->CreateInstance();
    structVal.SetStruct(structInstance.get());
    instance1->SetValue("StructArray", structVal, 1);

    ECSchemaPtr schema2;
    ECSchema::CreateSchema(schema2, "Schema", "ts", 2, 0, 0);
    ECStructClassP struct2;
    schema2->CreateStructClass(struct2, "Struct");
    struct2->CreatePrimitiveProperty(primProp, "String", PRIMITIVETYPE_Integer);

    ECEntityClassP class2;
    schema2->CreateEntityClass(class2, "Class");
    class2->CreatePrimitiveProperty(primProp, "Stuff", PRIMITIVETYPE_String);
    class2->CreateStructArrayProperty(structArrayProp, "StructArray", struct2);

    StandaloneECInstancePtr instance2 = class2->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_EQ(ECObjectsStatus::Success, instance2->CopyDataBuffer(*instance1, true));

    ECValue arrayVal;
    arrayVal.SetStructArrayInfo(2, false);
    TestValue(*instance2, "StructArray", arrayVal);
#ifdef STRUCT_ENTRIES_COPIED
    // this used to expect the struct array entries to be null, which happened to work due to a bug in the copying code
    // After fixing the bug the struct entry IDs are copied, but (as advertised in method documentation) the struct array ECInstances themselves
    // are not, so trying to access them with invalid ID raises an error.
    TestValue(*instance2, "StructArray", ECValue(/*null*/), 0);
    TestValue(*instance2, "StructArray", ECValue(/*null*/), 1);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDBufferTests, ArraysAreNotNull)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "Test", "ts", 1, 0, 0);
    ECEntityClassP ecClass;
    schema->CreateEntityClass(ecClass, "Test");
    PrimitiveArrayECPropertyP arrayProp;
    ecClass->CreatePrimitiveArrayProperty(arrayProp, "Array", PRIMITIVETYPE_String);

    StandaloneECInstancePtr instance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    bool isNull = false;
    EXPECT_EQ(ECObjectsStatus::Success, instance->IsPropertyNull(isNull, "Array"));
    EXPECT_FALSE(isNull);

    ECValue v;
    EXPECT_EQ(ECObjectsStatus::Success, instance->GetValue(v, "Array"));
    EXPECT_FALSE(v.IsNull());
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

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyIndexTests : ECTestFixture
    {

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyIndexTests, FlatteningIterator)
    {
    static const PrimitiveType testTypes[] = {PRIMITIVETYPE_Integer, PRIMITIVETYPE_String};
    for (size_t typeIndex = 0; typeIndex < _countof(testTypes); typeIndex++)
        {
        auto primType = testTypes[typeIndex];
        // Create an ECClass with nested structs like so:
        //  1
        //  2
        //      3
        //      4
        //          5
        //  6 { empty struct }
        //  7
        //      8
        //  9
        ECSchemaPtr schema;
        ECSchema::CreateSchema(schema, "Schema", "ts", 1, 0, 0);
        PrimitiveECPropertyP primProp;
        StructECPropertyP structProp;

        ECStructClassP s4;
        schema->CreateStructClass(s4, "S4");
        EXPECT_SUCCESS(s4->CreatePrimitiveProperty(primProp, "P5", primType));

        ECStructClassP s2;
        schema->CreateStructClass(s2, "S2");
        EXPECT_SUCCESS(s2->CreatePrimitiveProperty(primProp, "P3", primType));
        EXPECT_SUCCESS(s2->CreateStructProperty(structProp, "P4", *s4));

        ECStructClassP s6;
        schema->CreateStructClass(s6, "S6");

        ECStructClassP s7;
        schema->CreateStructClass(s7, "S7");
        EXPECT_SUCCESS(s7->CreatePrimitiveProperty(primProp, "P8", primType));

        ECEntityClassP ecClass;
        schema->CreateEntityClass(ecClass, "MyClass");
        EXPECT_SUCCESS(ecClass->CreatePrimitiveProperty(primProp, "P1", primType));
        EXPECT_SUCCESS(ecClass->CreateStructProperty(structProp, "P2", *s2));
        EXPECT_SUCCESS(ecClass->CreateStructProperty(structProp, "P6", *s6));
        EXPECT_SUCCESS(ecClass->CreateStructProperty(structProp, "P7", *s7));
        EXPECT_SUCCESS(ecClass->CreatePrimitiveProperty(primProp, "P9", primType));

        // Expect property indices returned using depth-first traversal of struct members
        // Expect indices of struct properties are not returned
        // Note that order in which property indices are assigned and returned depends on fixed-sized vs variable-sized property types.
        Utf8CP expect[] = {"P1", "P2.P3", "P2.P4.P5", "P7.P8", "P9"};

        auto const& enabler = *ecClass->GetDefaultStandaloneEnabler();
        uint32_t propIdx;
        bset<Utf8CP> matched;
        for (PropertyIndexFlatteningIterator iter(enabler); iter.GetCurrent(propIdx); iter.MoveNext())
            {
            Utf8CP accessString = nullptr;
            EXPECT_SUCCESS(enabler.GetAccessString(accessString, propIdx));
            bool foundMatch = false;
            for (size_t i = 0; i < _countof(expect); i++)
                {
                if (0 == strcmp(expect[i], accessString))
                    {
                    EXPECT_TRUE(matched.end() == matched.find(expect[i]));
                    matched.insert(expect[i]);
                    foundMatch = true;
                    break;
                    }
                }

            EXPECT_TRUE(foundMatch);
            }

        EXPECT_EQ(matched.size(), _countof(expect));
        }
    }

END_BENTLEY_ECN_TEST_NAMESPACE
