/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/MemoryLayoutTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"


#include <ECObjects/ECInstance.h>
#include <ECObjects/StandaloneECInstance.h>
#include <ECObjects/ECValue.h>
#include <Bentley/BeTimeUtilities.h>

#define N_FINAL_STRING_PROPS_IN_FAKE_CLASS 48

#define FIXED_COUNT_ARRAYS_ARE_SUPPORTED 0

using namespace std;
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct MemoryLayoutTests : ECTestFixture
    {
    protected:
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

                ECObjectsStatus status = instance.SetValue(accessString, v, i);
                EXPECT_TRUE(ECObjectsStatus::Success == status);
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

                ECObjectsStatus status = instance.SetValue(accessString, v, i);
                EXPECT_TRUE(ECObjectsStatus::Success == status);
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
                "    <ECClass typeName=\"ArrayTest\" isStruct=\"True\" isDomainClass=\"True\">"
                "        <ECArrayProperty propertyName=\"SomeStrings\" typeName=\"string\" />"
                "        <ECArrayProperty propertyName=\"SomeInts\"    typeName=\"int\" />"
                "        <ECArrayProperty propertyName=\"SomePoint3ds\"    typeName=\"point3d\" />"
                "        <ECArrayProperty propertyName=\"SomePoint2ds\"    typeName=\"point2d\" />"
                "        <ECArrayProperty propertyName=\"SomeDoubles\"     typeName=\"double\"  />"
                "        <ECArrayProperty propertyName=\"SomeDateTimes\"   typeName=\"dateTime\"  />"
                "        <ECArrayProperty propertyName=\"SomeBooleans\"    typeName=\"boolean\"  />"
                "        <ECArrayProperty propertyName=\"SomeLongs\"       typeName=\"long\"  />"
                "        <ECArrayProperty propertyName=\"SomeBinaries\"    typeName=\"binary\"  />"
                "        <ECArrayProperty propertyName=\"FixedArrayFixedElement\" typeName=\"int\" minOccurs=\"10\" maxOccurs=\"10\"/>"
                "        <ECArrayProperty propertyName=\"FixedArrayVariableElement\" typeName=\"string\" minOccurs=\"12\" maxOccurs=\"12\"/>"
                "        <ECArrayProperty propertyName=\"ManufacturerArray\" typeName=\"Manufacturer\" />"
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
                "    <ECClass typeName=\"Address\" isStruct=\"True\" isDomainClass=\"True\">"
                "        <ECProperty propertyName=\"HouseNumber\"  typeName=\"string\" />"
                "        <ECProperty propertyName=\"Street\"       typeName=\"string\" />"
                "        <ECProperty propertyName=\"Town\"         typeName=\"string\" />"
                "        <ECProperty propertyName=\"State\"        typeName=\"string\" />"
                "        <ECProperty propertyName=\"Zip\"          typeName=\"int\" />"
                "    </ECClass>"
                "    <ECClass typeName=\"PhoneNumber\" isStruct=\"True\" isDomainClass=\"True\">"
                "        <ECProperty propertyName=\"AreaCode\"     typeName=\"int\" />"
                "        <ECProperty propertyName=\"Number\"       typeName=\"int\" />"
                "    </ECClass>"
                "    <ECClass typeName=\"ContactInfo\" isStruct=\"True\" isDomainClass=\"True\">"
                "        <ECStructProperty propertyName=\"PhoneNumber\" typeName=\"PhoneNumber\" />"
                "        <ECStructProperty propertyName=\"Address\"     typeName=\"Address\" />"
                "        <ECProperty       propertyName=\"Email\"       typeName=\"string\" />"
                "    </ECClass>"
                "    <ECClass typeName=\"Employee\" isStruct=\"True\" isDomainClass=\"True\">"
                "        <ECProperty       propertyName=\"Name\"       typeName=\"string\" />"
                "        <ECStructProperty propertyName=\"Home\"       typeName=\"ContactInfo\" />"
                "        <ECStructProperty propertyName=\"Work\"       typeName=\"ContactInfo\" />"
                "    </ECClass>"
                "    <ECClass typeName=\"EmployeeDirectory\" isDomainClass=\"True\">"
                "        <ECArrayProperty propertyName=\"Employees\" typeName=\"Employee\"  minOccurs=\"0\" maxOccurs=\"unbounded\" />"
                "    </ECClass>"
                "    <ECClass typeName=\"Car\" isStruct=\"True\" isDomainClass=\"True\">"
                "        <ECProperty       propertyName=\"Name\"       typeName=\"string\"/>"
                "        <ECProperty       propertyName=\"Wheels\"     typeName=\"int\"  readOnly=\"True\"/>"
                "    </ECClass>"
                "  <ECClass typeName=\"StructClass\" isStruct=\"True\" isDomainClass=\"False\">"
                "    <ECProperty propertyName=\"StringProperty\" typeName=\"string\" /> "
                "    <ECProperty propertyName=\"IntProperty\" typeName=\"int\" /> "
                "    <ECArrayProperty propertyName=\"ArrayProperty\" typeName=\"string\" minOccurs=\"1\" maxOccurs=\"1\" /> "
                "    </ECClass>"
                "  <ECClass typeName=\"ComplexClass\" isDomainClass=\"True\">"
                "    <ECProperty propertyName=\"IntProperty\" typeName=\"int\" />"
                "    <ECProperty propertyName=\"StringProperty\" typeName=\"string\" /> "
                "    <ECProperty propertyName=\"DoubleProperty\" typeName=\"double\" /> "
                "    <ECProperty propertyName=\"DateTimeProperty\" typeName=\"dateTime\" />"
                "    <ECProperty propertyName=\"BooleanProperty\" typeName=\"boolean\" />"
                "    <ECArrayProperty propertyName=\"SimpleArrayProperty\" typeName=\"string\" minOccurs=\"1\" maxOccurs=\"1\" />"
                "    <ECArrayProperty propertyName=\"StructArrayProperty\" typeName=\"StructClass\" minOccurs=\"1\" maxOccurs=\"1\" isStruct=\"True\" />"
                "    <ECStructProperty propertyName=\"StructProperty\" typeName=\"StructClass\" />"
                "  </ECClass>"
                "</ECSchema>";

            Utf8String buff;
            buff.Sprintf(fmt, schemaName, versionMajor, versionMinor, className);

            return buff;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    JoshSchifter    12/09
        +---------------+---------------+---------------+---------------+---------------+------*/
        ECSchemaPtr       CreateTestSchema()
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
        ECSchemaPtr     CreateProfilingSchema(int nStrings)
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
            largeString[0] = '\0';
            for (int i = 0; i < 20; i++)
                strcat(largeString, "S2345678901234567890123456789012");

            SetAndVerifyString(instance, v, "S", largeString);
            for (int i = 0; i < N_FINAL_STRING_PROPS_IN_FAKE_CLASS; i++)
                {
                Utf8String propertyName;
                propertyName.Sprintf("Property%i", i);
                SetAndVerifyString(instance, v, propertyName.c_str(), valueForFinalStrings);
                }

            VerifyArrayInfo(instance, v, "BeginningArray", 0, false);
            VerifyArrayInfo(instance, v, "VariableArrayFixedElement", 0, false);
            VerifyArrayInfo(instance, v, "VariableArrayVariableElement", 0, false);
            VerifyArrayInfo(instance, v, "EndingArray", 0, false);

#if FIXED_COUNT_ARRAYS_ARE_SUPPORTED
            // We cannot honor min/maxOccurs attributes of ArrayECProperty, so arrays are always unbounded
            VerifyArrayInfo(instance, v, "FixedArrayFixedElement", 10, true);
            VerifyArrayInfo(instance, v, "FixedArrayVariableElement", 12, true);
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
#else
            // Replace #ifdef'ed out section above
            VerifyArrayInfo(instance, v, "FixedArrayFixedElement", 0, false);
            VerifyArrayInfo(instance, v, "FixedArrayVariableElement", 0, false);
            instance.AddArrayElements("FixedArrayFixedElement", 10);                       // if we supported fixed count arrays, the elements would already have been allocated and this would be unnecessary
            VerifyIsNullArrayElements(instance, v, "FixedArrayFixedElement", 0, 10, true);
            SetAndVerifyIntegerArray(instance, v, "FixedArrayFixedElement", 172, 10);
            VerifyIsNullArrayElements(instance, v, "FixedArrayFixedElement", 0, 10, false);
            SetAndVerifyIntegerArray(instance, v, "FixedArrayFixedElement", 283, 10);

            instance.AddArrayElements("FixedArrayVariableElement", 12);                    // if we supported fixed count arrays, the elements would already have been allocated and this would be unnecessary
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

#ifdef OLD_WAY    
            ClassLayoutP manufClassLayout = ClassLayout::BuildFromClass(*manufacturerClass, 43, 24);
            StandaloneECEnablerPtr manufEnabler = StandaloneECEnabler::CreateEnabler(*manufacturerClass, *manufClassLayout, true);
#endif
            StandaloneECEnablerPtr manufEnabler = instance.GetEnablerR().GetEnablerForStructArrayMember(manufacturerClass->GetSchema().GetSchemaKey(), manufacturerClass->GetName().c_str());
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
            VerifyArrayInfo(instance, v, "VariableArrayVariableElement", 14, false);
            VerifyIsNullArrayElements(instance, v, "VariableArrayVariableElement", 0, 14, false);
            VerifyStringArray(instance, v, "VariableArrayVariableElement", "Var+Var", 0, 14);
            VerifyArrayInfo(instance, v, "EndingArray", 14, false);
            VerifyIsNullArrayElements(instance, v, "EndingArray", 0, 14, false);
            VerifyStringArray(instance, v, "EndingArray", "EArray", 0, 14);
            VerifyVariableCountManufacturerArray(instance, v, "ManufacturerArray");

#if FIXED_COUNT_ARRAYS_ARE_SUPPORTED
            VerifyArrayInfo(instance, v, "FixedArrayFixedElement", 10, true);
            VerifyIntegerArray(instance, v, "FixedArrayFixedElement", 283, 0, 10);
            VerifyArrayInfo(instance, v, "FixedArrayVariableElement", 12, true);
            VerifyIsNullArrayElements(instance, v, "FixedArrayVariableElement", 0, 12, false);
            VerifyStringArray(instance, v, "FixedArrayVariableElement", "XString", 0, 12);
            VerifyArrayInfo(instance, v, "VariableArrayFixedElement", 14, false);
            VerifyIsNullArrayElements(instance, v, "VariableArrayFixedElement", 0, 14, false);
            VerifyIntegerArray(instance, v, "VariableArrayFixedElement", 57, 0, 14);
#endif
            instance.ToString("").c_str();
            }
    };
//static
std::vector<Utf8String> MemoryLayoutTests::s_propertyNames;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MemoryLayoutTests, GetValuesUsingInteropHelper_PropertyAccessor)
    {
    ECSchemaPtr schema = CreateTestSchema();
    ASSERT_TRUE(schema.IsValid());

    ECClassP ecClass = schema->GetClassP("AllPrimitives");
    ASSERT_TRUE(ecClass);

    StandaloneECEnablerPtr enabler = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    const Byte binaryInput[] = {0x01, 0x23, 0x26, 0x78};
    const size_t sizeInput = 4;
    ECValue      valueInput;
    valueInput.SetBinary(binaryInput, sizeInput, false);
    const Byte*  binaryOutput;
    size_t       sizeOutput;
    ECValue      valueOutput;

    // GetValue
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetValue(*instance, "ABinary", valueInput));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetValue(*instance, valueOutput, "ABinary"));
    binaryOutput = valueOutput.GetBinary(sizeOutput);
    EXPECT_EQ(sizeInput, sizeOutput);
    for (int i = 0; i < (int) sizeInput; i++)
        EXPECT_EQ(binaryInput[i], binaryOutput[i]);

    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetValue(*instance, "SomeBinaries[0]", valueInput));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetValue(*instance, valueOutput, "SomeBinaries[0]"));
    binaryOutput = valueOutput.GetBinary(sizeOutput);
    EXPECT_EQ(sizeInput, sizeOutput);
    for (int i = 0; i < (int) sizeInput; i++)
        EXPECT_EQ(binaryInput[i], binaryOutput[i]);

    // GetInteger
    int intVal = 0;
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetIntegerValue(*instance, "AnInt", 10));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetInteger(*instance, intVal, "AnInt"));
    EXPECT_EQ(10, intVal);
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetIntegerValue(*instance, "SomeInts[0]", 20));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetInteger(*instance, intVal, "SomeInts[0]"));
    EXPECT_EQ(20, intVal);

    // GetDouble
    double doubleVal = 0.0;
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetDoubleValue(*instance, "ADouble", 1.0));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetDouble(*instance, doubleVal, "ADouble"));
    EXPECT_EQ(1.0, doubleVal);
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetDoubleValue(*instance, "SomeDoubles[0]", 2.0));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetDouble(*instance, doubleVal, "SomeDoubles[0]"));
    EXPECT_EQ(2.0, doubleVal);

    // GetLong
    int64_t longVal = 0;
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetLongValue(*instance, "ALong", 50));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetLong(*instance, longVal, "ALong"));
    EXPECT_EQ(50, longVal);
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetLongValue(*instance, "SomeLongs[0]", 100));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetLong(*instance, longVal, "SomeLongs[0]"));
    EXPECT_EQ(100, longVal);

    // GetString
    Utf8String  stringVal;
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetStringValue(*instance, "AString", "teststring1"));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetString(*instance, stringVal, "AString"));
    EXPECT_STREQ("teststring1", stringVal.c_str());
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetStringValue(*instance, "SomeStrings[0]", "teststring2"));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetString(*instance, stringVal, "SomeStrings[0]"));
    EXPECT_STREQ("teststring2", stringVal.c_str());

    // GetBool
    bool boolVal = false;
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetBooleanValue(*instance, "ABoolean", true));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetBoolean(*instance, boolVal, "ABoolean"));
    EXPECT_TRUE(boolVal);
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetBooleanValue(*instance, "SomeBooleans[0]", false));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetBoolean(*instance, boolVal, "SomeBooleans[0]"));
    EXPECT_FALSE(boolVal);

    // GetPoint2d
    DPoint2d   point2dInput = {1.0, 2.0};
    DPoint2d   point2dOutput = {0.0, 0.0};
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetPoint2DValue(*instance, "APoint2d", point2dInput));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetPoint2D(*instance, point2dOutput, "APoint2d"));
    EXPECT_TRUE(point2dInput.x == point2dOutput.x && point2dInput.y == point2dOutput.y);
    point2dInput = {3.0, 4.0};
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetPoint2DValue(*instance, "SomePoint2ds[0]", point2dInput));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetPoint2D(*instance, point2dOutput, "SomePoint2ds[0]"));
    EXPECT_TRUE(point2dInput.x == point2dOutput.x && point2dInput.y == point2dOutput.y);

    // GetPoint3d
    DPoint3d   point3dInput = {1.0, 2.0, 3.0};
    DPoint3d   point3dOutput = {0.0, 0.0, 0.0};
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetPoint3DValue(*instance, "APoint3d", point3dInput));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetPoint3D(*instance, point3dOutput, "APoint3d"));
    EXPECT_TRUE(point3dInput.x == point3dOutput.x && point3dInput.y == point3dOutput.y && point3dInput.z == point3dOutput.z);
    point3dInput = {4.0, 5.0, 6.0};
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetPoint3DValue(*instance, "SomePoint3ds[0]", point3dInput));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetPoint3D(*instance, point3dOutput, "SomePoint3ds[0]"));
    EXPECT_TRUE(point3dInput.x == point3dOutput.x && point3dInput.y == point3dOutput.y && point3dInput.z == point3dOutput.z);

    // GetDateTime
    DateTime timeInput = DateTime::GetCurrentTimeUtc();
    DateTime timeOutput;
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetDateTimeValue(*instance, "ADateTime", timeInput));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetDateTime(*instance, timeOutput, "ADateTime"));
    EXPECT_TRUE(timeInput.Equals(timeOutput, true));
    timeInput = DateTime::GetCurrentTimeUtc();
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetDateTimeValue(*instance, "SomeDateTimes[0]", timeInput));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetDateTime(*instance, timeOutput, "SomeDateTimes[0]"));
    EXPECT_TRUE(timeInput.Equals(timeOutput, true));

    //GetDateTimeTicks
    int64_t    ticksInput = 634027121070910000;
    int64_t    ticksOutput = 0;
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetDateTimeTicks(*instance, "ADateTime", ticksInput));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetDateTimeTicks(*instance, ticksOutput, "ADateTime"));
    EXPECT_TRUE(ticksInput == ticksOutput);

    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetDateTimeTicks(*instance, "SomeDateTimes[1]", ticksInput));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetDateTimeTicks(*instance, ticksOutput, "SomeDateTimes[1]"));
    EXPECT_TRUE(ticksInput == ticksOutput);
    };

#define ACCESSOR propertyValue.GetValueAccessor ()
#define GetAccessor_start(valToMatch)             \
        for (ECPropertyValueCR propertyValue : *collection) \
                            { \
                if (valToMatch == propertyValue.GetValueAccessor ().GetPropertyName ()) \
                                        { 

#define GetAccessor_end()   \
                break;\
                    } \
                }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MemoryLayoutTests, GetValuesUsingInteropHelper_ECValueAccessor)
    {
    ECSchemaPtr schema = CreateTestSchema();
    ASSERT_TRUE(schema.IsValid());

    ECClassP ecClass = schema->GetClassP("AllPrimitives");
    ASSERT_TRUE(ecClass);

    StandaloneECEnablerPtr enabler = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    ECValuesCollectionPtr collection = ECValuesCollection::Create(*instance);

    // GetValue
    const Byte binaryInput[] = {0x01, 0x23, 0x26, 0x78};
    const size_t sizeInput = 4;
    ECValue      valueInput;
    valueInput.SetBinary(binaryInput, sizeInput, false);
    const Byte*  binaryOutput;
    size_t       sizeOutput;
    ECValue      valueOutput;

    GetAccessor_start("ABinary")
        EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetValue(*instance, ACCESSOR, valueInput));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetValue(*instance, valueOutput, "ABinary"));
    binaryOutput = valueOutput.GetBinary(sizeOutput);
    EXPECT_EQ(sizeInput, sizeOutput);
    for (int i = 0; i < (int) sizeInput; i++)
        EXPECT_EQ(binaryInput[i], binaryOutput[i]);
    GetAccessor_end()

        // GetInteger
        GetAccessor_start("AnInt")
        int intVal = 0;
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetIntegerValue(*instance, ACCESSOR, 10));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetInteger(*instance, intVal, ACCESSOR));
    EXPECT_EQ(10, intVal);
    GetAccessor_end()

        // GetDouble
        GetAccessor_start("ADouble")
        double doubleVal = 0.0;
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetDoubleValue(*instance, ACCESSOR, 1.0));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetDouble(*instance, doubleVal, ACCESSOR));
    EXPECT_EQ(1.0, doubleVal);
    GetAccessor_end()

        // GetLong
        GetAccessor_start("ALong")
        int64_t longVal = 0;
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetLongValue(*instance, ACCESSOR, 50));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetLong(*instance, longVal, ACCESSOR));
    EXPECT_EQ(50, longVal);
    GetAccessor_end()

        // GetString
        GetAccessor_start("AString")
        Utf8String  stringVal;
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetStringValue(*instance, ACCESSOR, "teststring1"));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetString(*instance, stringVal, ACCESSOR));
    EXPECT_STREQ("teststring1", stringVal.c_str());
    GetAccessor_end()

        // GetBool
        GetAccessor_start("ABoolean")
        bool boolVal = false;
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetBooleanValue(*instance, ACCESSOR, true));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetBoolean(*instance, boolVal, ACCESSOR));
    EXPECT_TRUE(boolVal);
    GetAccessor_end()

        // GetPoint2d
        GetAccessor_start("APoint2d")
        DPoint2d   point2dInput = {1.0, 2.0};
    DPoint2d   point2dOutput = {0.0, 0.0};
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetPoint2DValue(*instance, ACCESSOR, point2dInput));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetPoint2D(*instance, point2dOutput, ACCESSOR));
    EXPECT_TRUE(point2dInput.x == point2dOutput.x && point2dInput.y == point2dOutput.y);
    GetAccessor_end()

        // GetPoint3d
        GetAccessor_start("APoint3d")
        DPoint3d   point3dInput = {1.0, 2.0, 3.0};
    DPoint3d   point3dOutput = {0.0, 0.0, 0.0};
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetPoint3DValue(*instance, ACCESSOR, point3dInput));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetPoint3D(*instance, point3dOutput, ACCESSOR));
    EXPECT_TRUE(point3dInput.x == point3dOutput.x && point3dInput.y == point3dOutput.y && point3dInput.z == point3dOutput.z);
    GetAccessor_end()

        // GetDateTime
        GetAccessor_start("ADateTime")
        DateTime timeInput = DateTime::GetCurrentTimeUtc();
    DateTime timeOutput;
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetDateTimeValue(*instance, ACCESSOR, timeInput));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetDateTime(*instance, timeOutput, ACCESSOR));
    EXPECT_TRUE(timeInput.Equals(timeOutput, true));
    GetAccessor_end()

        //GetDateTimeTicks
        GetAccessor_start("ADateTime")
        int64_t    ticksInput = 634027121070910000;
    int64_t    ticksOutput = 0;
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetDateTimeTicks(*instance, ACCESSOR, ticksInput));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetDateTimeTicks(*instance, ticksOutput, ACCESSOR));
    EXPECT_TRUE(ticksInput == ticksOutput);
    GetAccessor_end();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MemoryLayoutTests, AddRemoveArrayUsingInteropHelper)
    {
    ECSchemaPtr schema = CreateTestSchema();
    ASSERT_TRUE(schema.IsValid());

    ECClassP ecClass = schema->GetClassP("TestClass");
    ASSERT_TRUE(ecClass);

    StandaloneECEnablerPtr enabler = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    int intVal = 0;
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::AddArrayElements(*instance, "FixedArrayFixedElement", 1));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetIntegerValue(*instance, "FixedArrayFixedElement[0]", 97));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetInteger(*instance, intVal, "FixedArrayFixedElement[0]"));
    EXPECT_TRUE(97 == intVal);

    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::AddArrayElements(*instance, "VariableArrayFixedElement", 1));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetIntegerValue(*instance, "VariableArrayFixedElement[1]", 101));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetInteger(*instance, intVal, "VariableArrayFixedElement[1]"));
    EXPECT_TRUE(101 == intVal);

    Utf8String testString = "Charmed";
    Utf8String stringValueP = NULL;

    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::AddArrayElements(*instance, "ManufacturerArray", 1));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetStringValue(*instance, "ManufacturerArray[1].Name", testString.c_str()));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetString(*instance, stringValueP, "ManufacturerArray[1].Name"));
    EXPECT_STREQ(testString.c_str(), stringValueP.c_str());

    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::RemoveArrayElement(*instance, "FixedArrayFixedElement", 0));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::RemoveArrayElement(*instance, "VariableArrayFixedElement", 1));
    EXPECT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::RemoveArrayElement(*instance, "ManufacturerArray", 1));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MemoryLayoutTests, GetStructArraysUsingInteropHelper)
    {
    ECSchemaPtr      schema = CreateTestSchema();
    ASSERT_TRUE(schema.IsValid());

    ECClassP ecClass = schema->GetClassP("ClassWithStructArray");
    ASSERT_TRUE(NULL != ecClass);

    StandaloneECEnablerPtr enabler = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();
    //Testing array of structs
    ECValue structArrayValueInput(42);
    EXPECT_TRUE(ECObjectsStatus::Success == ECInstanceInteropHelper::SetValue(*instance, "StructArray[1].AnInt", structArrayValueInput));
    EXPECT_TRUE(ECObjectsStatus::Success == ECInstanceInteropHelper::SetValue(*instance, "StructArray[0].AnInt", structArrayValueInput));

    ECValue structArrayValueOutput;
    EXPECT_TRUE(ECObjectsStatus::Success == ECInstanceInteropHelper::GetValue(*instance, structArrayValueOutput, "StructArray[0].AnInt"));
    EXPECT_TRUE(structArrayValueInput.GetInteger() == structArrayValueOutput.GetInteger());

    //Just seeing if it's possible to set a struct array element directly using the interop helper.
    ECClassP structClass = schema->GetClassP("AllPrimitives");
    ASSERT_TRUE(NULL != structClass);

    StandaloneECEnablerPtr structEnabler = structClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr newStructInstance = structEnabler->CreateInstance();

    ECValue manualIntEntry(64);
    EXPECT_TRUE(ECObjectsStatus::Success == ECInstanceInteropHelper::SetValue(*newStructInstance, "AnInt", manualIntEntry));

    ECValue newStructValue;
    newStructValue.SetStruct(newStructInstance.get());
    EXPECT_TRUE(ECObjectsStatus::Success == ECInstanceInteropHelper::SetValue(*instance, "StructArray[2]", newStructValue));

    ECValue manualIntEntryOutput;
    EXPECT_TRUE(ECObjectsStatus::Success == ECInstanceInteropHelper::GetValue(*instance, manualIntEntryOutput, "StructArray[2].AnInt"));
    EXPECT_TRUE(manualIntEntryOutput.GetInteger() == manualIntEntry.GetInteger());
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MemoryLayoutTests, ChangeSizeOfBinaryArrayEntries)
    {
    ECSchemaPtr      schema = CreateTestSchema();
    ASSERT_TRUE(schema.IsValid());

    ECClassP ecClass = schema->GetClassP("AllPrimitives");
    ASSERT_TRUE(ecClass != NULL);
    ClassLayoutPtr classLayout = ClassLayout::BuildFromClass(*ecClass);
    StandaloneECEnablerPtr enabler = StandaloneECEnabler::CreateEnabler(*ecClass, *classLayout, NULL);

    ECN::StandaloneECInstancePtr wipInstance = enabler->CreateInstance();
    // Previously there was a bug which prevented changing the size of a binary value once it had been set. This has since been fixed.
    const Byte bugTestBinary1[8] = {0x21, 0xa9, 0x84, 0x9d, 0xca, 0x1d, 0x8a, 0x78};
    const Byte bugTestBinary2[4] = {0x12, 0x79, 0xca, 0xde};

    ECValue bugTestValue0(bugTestBinary2, 4);
    ASSERT_TRUE(ECObjectsStatus::Success == wipInstance->InsertArrayElements("SomeBinaries", 0, 1));
    ASSERT_TRUE(ECObjectsStatus::Success == ECN::ECInstanceInteropHelper::SetValue(*wipInstance, "SomeBinaries[0]", bugTestValue0));

    ECValue bugTestValue1(bugTestBinary1, 8);
    ASSERT_TRUE(ECObjectsStatus::Success == ECN::ECInstanceInteropHelper::SetValue(*wipInstance, "SomeBinaries[0]", bugTestValue1));

    ECN::ECValue bugTestValue2;
    ASSERT_TRUE(ECObjectsStatus::Success == ECN::ECInstanceInteropHelper::GetValue(*wipInstance, bugTestValue2, "SomeBinaries[0]"));
    size_t size1 = -1;
    size_t size2 = -2;
    const Byte* data1 = bugTestValue1.GetBinary(size1);
    const Byte* data2 = bugTestValue2.GetBinary(size2);

    ASSERT_TRUE(NULL != data1 && NULL != data2);
    ASSERT_TRUE(size1 == 8);
    ASSERT_TRUE(size2 == 8);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MemoryLayoutTests, GetSetValueByIndexUsingInteropHelper)
    {
    ECSchemaPtr schema = CreateTestSchema();
    ASSERT_TRUE(schema.IsValid());

    ECClassP ecClass = schema->GetClassP("AllPrimitives");
    ASSERT_TRUE(ecClass);

    StandaloneECEnablerPtr enabler = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    Utf8CP accessString;
    uint32_t propertyIndex;

    // set, get integer value by index
    ASSERT_EQ (ECObjectsStatus::Success, enabler->GetPropertyIndex (propertyIndex, "AnInt"));
    ASSERT_EQ (ECObjectsStatus::Success, enabler->GetAccessString (accessString, propertyIndex));// verify propertyIndex
    ASSERT_STREQ ("AnInt", accessString);
    ASSERT_FALSE (ECInstanceInteropHelper::IsCalculatedECProperty (*instance, (int)propertyIndex));

    ECValue intVal;
    ECValue outputInt;
    intVal.SetInteger (10);
    ASSERT_EQ (ECObjectsStatus::Success, ECInstanceInteropHelper::SetValueByIndex (*instance, (int)propertyIndex, -1, intVal));
    ASSERT_EQ (ECObjectsStatus::Success, ECInstanceInteropHelper::GetValueByIndex (outputInt, *instance, (int)propertyIndex, -1));
    ASSERT_EQ (10, (int)outputInt.GetInteger ());

    // set, get IntegerArray value by index
    ASSERT_EQ(ECObjectsStatus::Success, enabler->GetPropertyIndex(propertyIndex, "SomeInts"));
    ASSERT_EQ (ECObjectsStatus::Success, enabler->GetAccessString (accessString, propertyIndex));// verify propertyIndex
    ASSERT_STREQ ("SomeInts", accessString);
    ASSERT_FALSE (ECInstanceInteropHelper::IsCalculatedECProperty (*instance, (int)propertyIndex));

    intVal.SetInteger(10);
    ASSERT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetValueByIndex(*instance, (int) propertyIndex, 0, intVal));
    ASSERT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::GetValueByIndex(outputInt, *instance, (int) propertyIndex, 0));
    ASSERT_EQ(10, (int) outputInt.GetInteger());

    // set, get DataTimeArray value by index
    ASSERT_EQ(ECObjectsStatus::Success, enabler->GetPropertyIndex(propertyIndex, "SomeDateTimes"));
    ASSERT_EQ (ECObjectsStatus::Success, enabler->GetAccessString (accessString, propertyIndex));// verify propertyIndex
    ASSERT_STREQ ("SomeDateTimes", accessString);
    ASSERT_FALSE (ECInstanceInteropHelper::IsCalculatedECProperty (*instance, (int)propertyIndex));

    ECValue dateTimeVal;
    dateTimeVal.SetDateTime (DateTime::GetCurrentTimeUtc ());
    ECValue dateTimeOutput;
    ASSERT_EQ(ECObjectsStatus::Success, ECInstanceInteropHelper::SetValueByIndex(*instance, (int)propertyIndex, 1, dateTimeVal));
    ASSERT_EQ (ECObjectsStatus::Success, ECInstanceInteropHelper::GetValueByIndex (dateTimeOutput, *instance, (int)propertyIndex, 1));
    ASSERT_TRUE (dateTimeVal.GetDateTime ().Equals (dateTimeOutput.GetDateTime (), true));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (MemoryLayoutTests, GetNullValueUsingInteropHelper)
    {
    ECSchemaPtr schema = CreateTestSchema ();
    ASSERT_TRUE (schema.IsValid ());

    ECClassP ecClass = schema->GetClassP ("AllPrimitives");
    ASSERT_TRUE (ecClass);

    StandaloneECEnablerPtr enabler = ecClass->GetDefaultStandaloneEnabler ();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance ();

    ECValuesCollectionPtr collection = ECValuesCollection::Create (*instance);

    GetAccessor_start ("ABinary")
    ECInstanceInteropHelper::SetToNull (*instance, ACCESSOR);
    ASSERT_TRUE (ECInstanceInteropHelper::IsNull (*instance, ACCESSOR));
    GetAccessor_end ()
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, ECValueEqualsMethod)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP ("AllPrimitives");
    ASSERT_TRUE (NULL != ecClass);

    StandaloneECEnablerPtr enabler =  ecClass->GetDefaultStandaloneEnabler();

    ECValue v1, v2;
    EXPECT_TRUE   (v1.Equals(v2));
    v1.SetInteger (3425);
    v2.SetInteger (6548);
    EXPECT_FALSE  (v1.Equals (v2));
    v2.SetInteger (v1.GetInteger());
    EXPECT_TRUE   (v1.Equals (v2));

    v1.SetUtf8CP  ("Something");
    v2.SetUtf8CP  ("Something else");
    EXPECT_FALSE  (v1.Equals (v2));
    v2.SetUtf8CP  (v1.GetUtf8CP());
    EXPECT_TRUE   (v1.Equals (v2));

    //Conflicting types
    v2.SetInteger (3425);
    EXPECT_FALSE  (v1.Equals (v2));

    v1.SetDouble  (1.0);
    v2.SetDouble  (1.0);
    EXPECT_TRUE   (v1.Equals (v2));
    v2.SetDouble  (2.0);
    EXPECT_FALSE  (v1.Equals (v2));

    v1.SetLong    ((int64_t)345);
    v2.SetLong    ((int64_t)345);
    EXPECT_TRUE   (v1.Equals (v2));
    v2.SetLong    ((int64_t)345345);
    EXPECT_FALSE  (v1.Equals (v2));

    v1.SetBoolean (false);
    v2.SetBoolean (false);
    EXPECT_TRUE   (v1.Equals (v2));
    v2.SetBoolean (true);
    EXPECT_FALSE  (v1.Equals (v2));

    DateTime timeInput = DateTime::GetCurrentTimeUtc ();
    v1.SetDateTime(timeInput);
    v2.SetDateTime(timeInput);
    EXPECT_TRUE   (v1.Equals (v2));
    DateTime timeInput2 (timeInput.GetInfo ().GetKind (), timeInput.GetYear () + 1, timeInput.GetMonth (), timeInput.GetDay (), timeInput.GetHour (), timeInput.GetMinute (), timeInput.GetSecond (), timeInput.GetHectoNanosecond ());
    v2.SetDateTime(timeInput2);
    EXPECT_FALSE  (v1.Equals (v2));

    v1.SetDateTimeTicks((int64_t)633487865666864601);
    v2.SetDateTimeTicks((int64_t)633487865666864601);
    EXPECT_TRUE   (v1.Equals (v2));
    v2.SetDateTimeTicks((int64_t)633487865666866601);
    EXPECT_FALSE  (v1.Equals (v2));

    const static bool HOLD_AS_DUPLICATE = true;
    const Byte binaryValue0[4] = {0x00, 0x01, 0x02, 0x03};
    const Byte binaryValue1[4] = {0x10, 0x11, 0x12, 0x13};
    EXPECT_EQ (sizeof(binaryValue0), 4);
    EXPECT_EQ (sizeof(binaryValue0), 4);
    v1.SetBinary(binaryValue0, sizeof(binaryValue0), HOLD_AS_DUPLICATE);
    v2.SetBinary(binaryValue0, sizeof(binaryValue0), HOLD_AS_DUPLICATE);
    EXPECT_TRUE   (v1.Equals (v2));
    v2.SetBinary(binaryValue1, sizeof(binaryValue1), HOLD_AS_DUPLICATE);
    EXPECT_FALSE  (v1.Equals (v2));

    DPoint2d   point2dInput0 = {1.0, 2.0};
    DPoint2d   point2dInput1 = {3.0, 4.0};
    v1.SetPoint2D (point2dInput0);
    v2.SetPoint2D (point2dInput0);
    EXPECT_TRUE   (v1.Equals (v2));
    v2.SetPoint2D (point2dInput1);
    EXPECT_FALSE  (v1.Equals (v2));

    DPoint3d   point3dInput0 = {1.0, 2.0, -10.0};
    DPoint3d   point3dInput1 = {3.0, 4.0, -123.0};
    v1.SetPoint3D (point3dInput0);
    v2.SetPoint3D (point3dInput0);
    EXPECT_TRUE   (v1.Equals (v2));
    v2.SetPoint3D (point3dInput1);
    EXPECT_FALSE  (v1.Equals (v2));

    ECN::StandaloneECInstancePtr testInstance0 = enabler->CreateInstance();
    ECN::StandaloneECInstancePtr testInstance1 = enabler->CreateInstance();
    v1.SetStruct  (testInstance0.get());
    v2.SetStruct  (testInstance0.get());
    EXPECT_TRUE   (v1.Equals(v2));
    v2.SetStruct  (testInstance1.get());
    EXPECT_FALSE  (v1.Equals (v2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, GetEnablerPropertyInformation)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP ("AllPrimitives");
    ASSERT_TRUE (NULL != ecClass);

    StandaloneECEnablerPtr enabler = ecClass->GetDefaultStandaloneEnabler();

    const int expectedPropertyCount = 19;

/** -- Can't test this method via published API -- tested indirectly below
    UInt32 propertyCount = enabler->GetClassLayout().GetPropertyCount();
    EXPECT_EQ (expectedPropertyCount, propertyCount);
**/

    Utf8Char* expectedProperties [expectedPropertyCount] = 
        {
        "",
        "AString",
        "AnInt",
        "APoint3d",
        "APoint2d",
        "ADouble",
        "ADateTime",
        "ABoolean",
        "ALong",
        "ABinary",
        "SomeStrings",
        "SomeInts",
        "SomePoint3ds",
        "SomePoint2ds",
        "SomeDoubles",
        "SomeDateTimes",
        "SomeBooleans",
        "SomeLongs",
        "SomeBinaries"
        };

    for (uint32_t i=0; i < expectedPropertyCount; i++)
        {
        Utf8CP expectedPropertyName = expectedProperties [i];
        Utf8CP propertyName         = NULL;
        uint32_t propertyIndex          = 0;

        EXPECT_TRUE (ECObjectsStatus::Success == enabler->GetPropertyIndex (propertyIndex, expectedPropertyName));
        EXPECT_TRUE (ECObjectsStatus::Success == enabler->GetAccessString  (propertyName,  propertyIndex));

        EXPECT_STREQ (expectedPropertyName, propertyName);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
static void     printfIndent (uint32_t indentDepth)
    {
    for (uint32_t i = 0; i < indentDepth; i++)
        printf ("  ");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
static void     dumpPropertyValues (ECValuesCollectionR collection, bool isArray, uint32_t indentDepth)
    {
    uint32_t arrayIndex = 0;

    for (ECPropertyValueCR propertyValue : collection)
        {
        ECValueCR v = propertyValue.GetValue();

        printfIndent (indentDepth);
        ECValueAccessorCR   accessor = propertyValue.GetValueAccessor();
        uint32_t accessorDepth = accessor.GetDepth();
        Utf8CP accessString = accessor.GetAccessString (accessorDepth- 1);

        if (isArray)
            {
            printf ("Array Member [%u] %s (depth=%u) = %s\n", arrayIndex++, accessString, accessorDepth, v.ToString().c_str());
            }
        else
            {
            printf ("%s (depth=%u)", accessString, accessorDepth);
            if ( ! v.IsStruct())
                printf (" = %s", v.ToString().c_str());

            printf ("\n");
            }

        if (propertyValue.HasChildValues ())
            {
            ECValuesCollectionPtr children = propertyValue.GetChildValues();
            dumpPropertyValues (*children, v.IsArray(), indentDepth+1);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
static void     dumpLoadedPropertyValues (ECValuesCollectionR collection, bool isArray, uint32_t indentDepth, bool printValues, int& count)
    {
    uint32_t arrayIndex = 0;

    for (ECPropertyValueCR propertyValue : collection)
        {
        ECValueCR v = propertyValue.GetValue();
        if (!v.IsLoaded())
            continue;

        count++;
        if (printValues)
            {
            printfIndent (indentDepth);
            ECValueAccessorCR   accessor = propertyValue.GetValueAccessor();
            uint32_t accessorDepth = accessor.GetDepth();
            Utf8CP accessString = accessor.GetAccessString (accessorDepth- 1);

            if (isArray)
                {
                printf ("Array Member [%u] %s (depth=%u) = %s\n", arrayIndex++, accessString, accessorDepth, v.ToString().c_str());
                }
            else
                {
                printf ("%s (depth=%u)", accessString, accessorDepth);
                if ( ! v.IsStruct())
                    printf (" = %s", v.ToString().c_str());

                printf ("\n");
                }
            }

        if (propertyValue.HasChildValues ())
            {
            ECValuesCollectionPtr children = propertyValue.GetChildValues();
            dumpLoadedPropertyValues (*children, v.IsArray(), indentDepth+1, printValues, count);
            }
        }
    }

typedef bpair<Utf8String, ECValue>  AccessStringValuePair;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
static void     verifyECValueEnumeration (ECValuesCollectionR collection, bvector <AccessStringValuePair>& expectedValues, uint32_t& iValue, bool isDup)
    {
    for (ECPropertyValueCR propertyValue : collection)
        {
        Utf8String   foundAccessString    = propertyValue.GetValueAccessor().GetManagedAccessString(); 
        Utf8String   expectedAccessString = expectedValues[iValue].first;

        EXPECT_STREQ (expectedAccessString.c_str(), foundAccessString.c_str());

        ECValueCR foundValue    = propertyValue.GetValue();
        ECValueCR expectedValue = expectedValues[iValue].second;

        if ( ! isDup || ! foundValue.IsStruct())
            {
            EXPECT_TRUE (foundValue.Equals (expectedValue));
            }
        else
            {
            // If we are enumerating a duplicate, it will have its own struct instances
            // and we expect the struct pointers to be different so we can't call Equals
            EXPECT_TRUE (foundValue.IsNull()   == expectedValue.IsNull());
            EXPECT_TRUE (foundValue.IsStruct() == expectedValue.IsStruct());
            }

        iValue++;;

        if (propertyValue.HasChildValues ())
            {
            ECValuesCollectionPtr children = propertyValue.GetChildValues();
            verifyECValueEnumeration (*children, expectedValues, iValue, isDup);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      5/11
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, RecursiveECValueEnumeration_EmptyInstance)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());


    StandaloneECEnablerPtr enabler = schema->GetClassP("EmptyClass")->GetDefaultStandaloneEnabler ();

    ASSERT_TRUE (enabler.IsValid());

    /*--------------------------------------------------------------------------
        Create an empty instance
    --------------------------------------------------------------------------*/
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    ECN::ECValuesCollectionPtr collection = ECN::ECValuesCollection::Create (*instance);

    /*--------------------------------------------------------------------------
        Iterate through its values - shouldn't find any
    --------------------------------------------------------------------------*/
    uint32_t foundValues = 0;
    for (ECPropertyValueCR propertyValue : *collection)
        {
        propertyValue.HasChildValues(); // Use it to avoid warning about unused propertyValue object
        foundValues++;
        }
    EXPECT_TRUE (0 == foundValues);

    /*--------------------------------------------------------------------------
        Duplicate the instance and verify the duplicate.
    --------------------------------------------------------------------------*/
    StandaloneECInstancePtr standAloneInstance = StandaloneECInstance::Duplicate(*instance);

    collection = ECValuesCollection::Create (*standAloneInstance);
    foundValues = 0;
    for (ECPropertyValueCR propertyValue : *collection)
        {
        propertyValue.HasChildValues(); // Use it to avoid warning about unused propertyValue object
        foundValues++;
        }
    EXPECT_TRUE (0 == foundValues);
    }

TEST_F(MemoryLayoutTests, MergeArrayPropertyWithSmallerArray)
    {
    ECSchemaPtr testSchema;
    ECSchema::CreateSchema(testSchema, "TestSchema", 1, 2);
    ECEntityClassP class1;
    testSchema->CreateEntityClass(class1, "TestClass");
    ArrayECPropertyP primitiveArrayProp;
    class1->CreateArrayProperty(primitiveArrayProp, "PrimitiveArray");
    primitiveArrayProp->SetPrimitiveElementType (PRIMITIVETYPE_Long);

    StandaloneECEnablerPtr enabler       = class1->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    instance->AddArrayElements("PrimitiveArray", 3);
    ECValue v;
    v.SetLong(0);
    instance->SetValue("PrimitiveArray", v, 0);

    v.SetLong(1);
    instance->SetValue("PrimitiveArray", v, 1);

    v.SetLong(2);
    instance->SetValue("PrimitiveArray", v, 2);

    ECN::StandaloneECInstancePtr secondInstance = enabler->CreateInstance();
    secondInstance->AddArrayElements("PrimitiveArray", 2);
    v.SetLong(3);
    secondInstance->SetValue("PrimitiveArray", v, 0);

    v.SetLong(4);
    secondInstance->SetValue("PrimitiveArray", v, 1);

    EXPECT_TRUE (ECObjectsStatus::Success == secondInstance->GetValue (v, "PrimitiveArray"));
    EXPECT_EQ (2, v.GetArrayInfo().GetCount());

    v.Clear();

    MemoryECInstanceBase* mbInstance = instance->GetAsMemoryECInstanceP ();
    mbInstance->MergePropertiesFromInstance (*secondInstance);

    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (v, "PrimitiveArray"));

    // TFS#128233
    EXPECT_EQ (2, v.GetArrayInfo().GetCount()); // CGM: This line fails because it merges the array values instead of overwriting

    instance->GetValue(v, "PrimitiveArray", 0);
    EXPECT_EQ(3, v.GetLong());

    instance->GetValue(v, "PrimitiveArray", 1);
    EXPECT_EQ(4, v.GetLong());

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, RecursiveECValueEnumeration_PrimitiveProperties)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    StandaloneECEnablerPtr enabler = schema->GetClassP("CadData")->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (enabler.IsValid());

    /*--------------------------------------------------------------------------
        Build the instance
    --------------------------------------------------------------------------*/
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    instance->SetValue("Name",         ECValue ("My Name"));
    instance->SetValue("Count",        ECValue (14));
    instance->SetValue("Length",       ECValue (142.5));
    instance->SetValue("Field_Tested", ECValue (true));

    /*--------------------------------------------------------------------------
        Build the vector of expected values.
        Note: The order does not match the class it matches the classLayout
    --------------------------------------------------------------------------*/
    bvector <AccessStringValuePair> expectedValues;

    expectedValues.push_back (AccessStringValuePair ("Count",          ECValue(14)));
    expectedValues.push_back (AccessStringValuePair ("StartPoint",     ECValue ()));
    expectedValues.push_back (AccessStringValuePair ("EndPoint",       ECValue ()));
    expectedValues.push_back (AccessStringValuePair ("Size",           ECValue ()));
    expectedValues.push_back (AccessStringValuePair ("Length",         ECValue (142.5)));
    expectedValues.push_back (AccessStringValuePair ("Install_Date",   ECValue ()));
    expectedValues.push_back (AccessStringValuePair ("Service_Date",   ECValue ()));
    expectedValues.push_back (AccessStringValuePair ("Field_Tested",   ECValue (true)));
    expectedValues.push_back (AccessStringValuePair ("Name",           ECValue ("My Name")));

    /*--------------------------------------------------------------------------
        Verify that the values returned from the instance match the expected ones.
    --------------------------------------------------------------------------*/
    ECValuesCollectionPtr   collection = ECValuesCollection::Create (*instance);
    uint32_t                iValue = 0;

    verifyECValueEnumeration (*collection, expectedValues, iValue, false);
    //dumpPropertyValues (*collection, false, 0);

    EXPECT_TRUE (expectedValues.size() == iValue);

    /*--------------------------------------------------------------------------
        Duplicate the instance and verify the duplicate.
    --------------------------------------------------------------------------*/
    StandaloneECInstancePtr standAloneInstance = StandaloneECInstance::Duplicate(*instance);

    collection = ECValuesCollection::Create (*standAloneInstance);
    iValue = 0;
    verifyECValueEnumeration (*collection, expectedValues, iValue, true);
    //dumpPropertyValues (*collection, false, 0);

    EXPECT_TRUE (expectedValues.size() == iValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, CopyInstanceProperties)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.get() != NULL);

    StandaloneECEnablerPtr enabler = schema->GetClassP("CadData")->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (enabler.IsValid());

    /*--------------------------------------------------------------------------
        Build the instance
    --------------------------------------------------------------------------*/
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    instance->SetValue("Name",         ECValue ("My Name"));
    instance->SetValue("Count",        ECValue (14));
    instance->SetValue("Length",       ECValue (142.5));
    instance->SetValue("Field_Tested", ECValue (true));

    /*--------------------------------------------------------------------------
        Build the vector of expected values.
        Note: The order does not match the class it matches the classLayout
    --------------------------------------------------------------------------*/
    bvector <AccessStringValuePair> expectedValues;

    expectedValues.push_back (AccessStringValuePair ("Count",          ECValue(14)));
    expectedValues.push_back (AccessStringValuePair ("StartPoint",     ECValue ()));
    expectedValues.push_back (AccessStringValuePair ("EndPoint",       ECValue ()));
    expectedValues.push_back (AccessStringValuePair ("Size",           ECValue ()));
    expectedValues.push_back (AccessStringValuePair ("Length",         ECValue (142.5)));
    expectedValues.push_back (AccessStringValuePair ("Install_Date",   ECValue ()));
    expectedValues.push_back (AccessStringValuePair ("Service_Date",   ECValue ()));
    expectedValues.push_back (AccessStringValuePair ("Field_Tested",   ECValue (true)));
    expectedValues.push_back (AccessStringValuePair ("Name",           ECValue ("My Name")));

    /*--------------------------------------------------------------------------
        Verify that the values returned from the instance match the expected ones.
    --------------------------------------------------------------------------*/
    ECValuesCollectionPtr   collection = ECValuesCollection::Create (*instance);
    uint32_t                iValue = 0;

    verifyECValueEnumeration (*collection, expectedValues, iValue, false);
    //dumpPropertyValues (*collection, false, 0);

    EXPECT_TRUE (expectedValues.size() == iValue);

    /*--------------------------------------------------------------------------
        Duplicate the instance and verify the duplicate.
    --------------------------------------------------------------------------*/
    ECN::StandaloneECInstancePtr duplicateInstance = enabler->CreateInstance();

    ECObjectsStatus copyStatus = duplicateInstance->CopyValues (*instance);
    EXPECT_TRUE (ECObjectsStatus::Success == copyStatus);

    collection = ECValuesCollection::Create (*duplicateInstance);
    iValue = 0;
    verifyECValueEnumeration (*collection, expectedValues, iValue, true);
    //dumpPropertyValues (*collection, false, 0);

    EXPECT_TRUE (expectedValues.size() == iValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, MergeInstanceProperties)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.get() != NULL);

    StandaloneECEnablerPtr enabler = schema->GetClassP("CadData")->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (enabler.IsValid());

    /*--------------------------------------------------------------------------
        Build the base instance
    --------------------------------------------------------------------------*/
    ECN::StandaloneECInstancePtr mergeToInstance = enabler->CreateInstance();

    mergeToInstance->SetValue("Name",         ECValue ("base"));
    mergeToInstance->SetValue("Length",       ECValue (142.5));
    mergeToInstance->SetValue("Field_Tested", ECValue (true));

    /*--------------------------------------------------------------------------
        Build the instance with data to merge
    --------------------------------------------------------------------------*/
    ECN::StandaloneECInstancePtr mergeFromInstance = enabler->CreateInstance();

    DPoint2d   tstSize = {10.5, 22.3};

    ECValue nullBool (ECN::PRIMITIVETYPE_Boolean);

    mergeFromInstance->SetValue("Name",         ECValue ("merge"));
    mergeFromInstance->SetValue("Count",        ECValue (14));
    mergeFromInstance->SetValue("Field_Tested", nullBool);
    mergeFromInstance->SetValue ("Size",        ECValue (tstSize));

    MemoryECInstanceBase* mbInstance = mergeToInstance->GetAsMemoryECInstanceP ();
    mbInstance->MergePropertiesFromInstance (*mergeFromInstance);

    bvector <AccessStringValuePair> expectedValues;

    expectedValues.push_back (AccessStringValuePair ("Count",          ECValue(14)));
    expectedValues.push_back (AccessStringValuePair ("StartPoint",     ECValue ()));
    expectedValues.push_back (AccessStringValuePair ("EndPoint",       ECValue ()));
    expectedValues.push_back (AccessStringValuePair ("Size",           ECValue (tstSize)));
    expectedValues.push_back (AccessStringValuePair ("Length",         ECValue (142.5)));
    expectedValues.push_back (AccessStringValuePair ("Install_Date",   ECValue ()));
    expectedValues.push_back (AccessStringValuePair ("Service_Date",   ECValue ()));
    expectedValues.push_back (AccessStringValuePair ("Field_Tested",   nullBool));
    expectedValues.push_back (AccessStringValuePair ("Name",           ECValue ("merge")));

    ECValuesCollectionPtr   collection = ECValuesCollection::Create (*mergeToInstance);
//    dumpPropertyValues (*collection, false, 0);

    uint32_t                iValue = 0;
    verifyECValueEnumeration (*collection, expectedValues, iValue, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, RecursiveECValueEnumeration_PrimitiveArray)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    StandaloneECEnablerPtr enabler = schema->GetClassP("AllPrimitives")->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (enabler.IsValid());

    /*--------------------------------------------------------------------------
        Build the instance
    --------------------------------------------------------------------------*/
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    instance->SetValue("AString",  ECValue ("Happy String"));
    instance->SetValue("AnInt",    ECValue (6));

    instance->AddArrayElements("SomeStrings", 5);

    instance->SetValue("SomeStrings", ECValue ("ArrayMember 1"), 0);
    instance->SetValue("SomeStrings", ECValue ("ArrayMember 2"), 2);
    instance->SetValue("SomeStrings", ECValue ("ArrayMember 3"), 4);

    /*--------------------------------------------------------------------------
        Build the vector of expected values.
        Note: The order does not match the class it matches the classLayout
    --------------------------------------------------------------------------*/
    ECValue arrayValue;
    bvector <AccessStringValuePair> expectedValues;

    expectedValues.push_back (AccessStringValuePair ("AnInt", ECValue (6)));
    expectedValues.push_back (AccessStringValuePair ("APoint3d", ECValue ()));
    expectedValues.push_back (AccessStringValuePair ("APoint2d", ECValue ()));
    expectedValues.push_back (AccessStringValuePair ("ADouble", ECValue ()));
    expectedValues.push_back (AccessStringValuePair ("ADateTime", ECValue ()));
    expectedValues.push_back (AccessStringValuePair ("ABoolean", ECValue ()));
    expectedValues.push_back (AccessStringValuePair ("ALong", ECValue ()));
    expectedValues.push_back (AccessStringValuePair ("AString", ECValue ("Happy String")));
    expectedValues.push_back (AccessStringValuePair ("ABinary", ECValue ()));

    arrayValue.Clear();
    arrayValue.SetPrimitiveArrayInfo (PRIMITIVETYPE_String, 5, false);
    expectedValues.push_back (AccessStringValuePair ("SomeStrings", arrayValue));

    expectedValues.push_back (AccessStringValuePair ("SomeStrings[0]", ECValue ("ArrayMember 1")));
    expectedValues.push_back (AccessStringValuePair ("SomeStrings[1]", ECValue ()));
    expectedValues.push_back (AccessStringValuePair ("SomeStrings[2]", ECValue ("ArrayMember 2")));
    expectedValues.push_back (AccessStringValuePair ("SomeStrings[3]", ECValue ()));
    expectedValues.push_back (AccessStringValuePair ("SomeStrings[4]", ECValue ("ArrayMember 3")));

    arrayValue.SetPrimitiveArrayInfo (PRIMITIVETYPE_Integer, 0, false);
    expectedValues.push_back (AccessStringValuePair ("SomeInts",     arrayValue));

    arrayValue.SetPrimitiveArrayInfo (PRIMITIVETYPE_Point3D, 0, false);
    expectedValues.push_back (AccessStringValuePair ("SomePoint3ds", arrayValue));

    arrayValue.SetPrimitiveArrayInfo (PRIMITIVETYPE_Point2D, 0, false);
    expectedValues.push_back (AccessStringValuePair ("SomePoint2ds", arrayValue));

    arrayValue.SetPrimitiveArrayInfo (PRIMITIVETYPE_Double, 0, false);
    expectedValues.push_back (AccessStringValuePair ("SomeDoubles",  arrayValue));

    arrayValue.SetPrimitiveArrayInfo (PRIMITIVETYPE_DateTime, 0, false);
    expectedValues.push_back (AccessStringValuePair ("SomeDateTimes",arrayValue));

    arrayValue.SetPrimitiveArrayInfo (PRIMITIVETYPE_Boolean, 0, false);
    expectedValues.push_back (AccessStringValuePair ("SomeBooleans", arrayValue));

    arrayValue.SetPrimitiveArrayInfo (PRIMITIVETYPE_Long, 0, false);
    expectedValues.push_back (AccessStringValuePair ("SomeLongs",    arrayValue));

    arrayValue.SetPrimitiveArrayInfo (PRIMITIVETYPE_Binary, 0, false);
    expectedValues.push_back (AccessStringValuePair ("SomeBinaries", arrayValue));

    /*--------------------------------------------------------------------------
        Verify that the values returned from the instance match the expected ones.
    --------------------------------------------------------------------------*/
    ECValuesCollectionPtr   collection = ECValuesCollection::Create (*instance);
    uint32_t                iValue = 0;

    verifyECValueEnumeration (*collection, expectedValues, iValue, false);
    //dumpPropertyValues (*collection, false, 0);

    EXPECT_TRUE (expectedValues.size() == iValue);

    /*--------------------------------------------------------------------------
        Duplicate the instance and verify the duplicate.
    --------------------------------------------------------------------------*/
    StandaloneECInstancePtr standAloneInstance = StandaloneECInstance::Duplicate(*instance);

    collection = ECValuesCollection::Create (*standAloneInstance);
    iValue = 0;
    verifyECValueEnumeration (*collection, expectedValues, iValue, true);
    //dumpPropertyValues (*collection, false, 0);

    EXPECT_TRUE (expectedValues.size() == iValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String  buildAccessString
(
Utf8CP  accessPrefix,
Utf8CP  propertyString
)
    {
    Utf8String accessString;

    if (accessPrefix && 0 < strlen (accessPrefix))
        {
        accessString.append (accessPrefix);
        accessString.append (".");
        }

    accessString.append (propertyString);
    
    return accessString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
static void     setValue
(
Utf8CP  accessPrefix,
Utf8CP  propertyString,
ECValueCR       ecValue,
IECInstanceR    instance
)
    {
    Utf8String accessString = buildAccessString (accessPrefix, propertyString);

    instance.SetValue (accessString.c_str(), ecValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void     setPartialContactInfo
(
bool            skipPhoneNumberData,
Utf8CP         prefix,
int             areaCode,
int             phoneNumber,
Utf8CP houseNumber,
Utf8CP street,
Utf8CP town,
Utf8CP state,
int             zip,
Utf8CP email,
IECInstanceR    instance
)
    {
    if (!skipPhoneNumberData)
        {
        setValue (prefix, "PhoneNumber.AreaCode",  ECValue (areaCode),     instance);
        setValue (prefix, "PhoneNumber.AreaCode",  ECValue (areaCode),     instance);
        setValue (prefix, "PhoneNumber.Number",    ECValue (phoneNumber),  instance);
        setValue (prefix, "Address.HouseNumber",   ECValue (houseNumber),  instance);
        }

    setValue (prefix, "Address.Street",        ECValue (street),       instance);
    setValue (prefix, "Address.Town",          ECValue (town),         instance);
    setValue (prefix, "Address.State",         ECValue (state),        instance);
    setValue (prefix, "Address.Zip",           ECValue (zip),          instance);
    setValue (prefix, "Email",                 ECValue (email),        instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
static void     setContactInfo
(
Utf8CP prefix,
int             areaCode,
int             phoneNumber,
Utf8CP houseNumber,
Utf8CP street,
Utf8CP town,
Utf8CP state,
int             zip,
Utf8CP email,
IECInstanceR    instance
)
    {
    setValue (prefix, "PhoneNumber.AreaCode",  ECValue (areaCode),     instance);
    setValue (prefix, "PhoneNumber.AreaCode",  ECValue (areaCode),     instance);
    setValue (prefix, "PhoneNumber.Number",    ECValue (phoneNumber),  instance);
    setValue (prefix, "Address.HouseNumber",   ECValue (houseNumber),  instance);
    setValue (prefix, "Address.Street",        ECValue (street),       instance);
    setValue (prefix, "Address.Town",          ECValue (town),         instance);
    setValue (prefix, "Address.State",         ECValue (state),        instance);
    setValue (prefix, "Address.Zip",           ECValue (zip),          instance);
    setValue (prefix, "Email",                 ECValue (email),        instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
static void     addValue
(
Utf8CP  accessPrefix,
Utf8CP  propertyString,
ECValueCR       ecValue,
bvector <AccessStringValuePair>& expectedValues
)
    {
    Utf8String accessString = buildAccessString (accessPrefix, propertyString);

    expectedValues.push_back (AccessStringValuePair (accessString.c_str(), ecValue));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
static void     addExpectedContactInfo
(
Utf8CP prefix,
int             areaCode,
int             phoneNumber,
Utf8CP houseNumber,
Utf8CP street,
Utf8CP town,
Utf8CP state,
int             zip,
Utf8CP email,
bvector <AccessStringValuePair>& expectedValues
)
    {
    if (NULL != prefix  && 0 < strlen (prefix))
        expectedValues.push_back (AccessStringValuePair (prefix, ECValue (VALUEKIND_Struct)));

    addValue (prefix, "PhoneNumber",           ECValue (VALUEKIND_Struct), expectedValues);
    addValue (prefix, "PhoneNumber.AreaCode",  ECValue (areaCode),         expectedValues);
    addValue (prefix, "PhoneNumber.Number",    ECValue (phoneNumber),      expectedValues);
    addValue (prefix, "Address",               ECValue (VALUEKIND_Struct), expectedValues);
    addValue (prefix, "Address.Zip",           ECValue (zip),              expectedValues);

    addValue (prefix, "Address.HouseNumber",   ECValue (houseNumber),     expectedValues);
    addValue (prefix, "Address.Street",        ECValue (street),          expectedValues);
    addValue (prefix, "Address.Town",          ECValue (town),            expectedValues);
    addValue (prefix, "Address.State",         ECValue (state),           expectedValues);
    addValue (prefix, "Email",                 ECValue (email),           expectedValues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, RecursiveECValueEnumeration_EmbeddedStructs)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    StandaloneECEnablerPtr enabler = schema->GetClassP("ContactInfo")->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (enabler.IsValid());

    /*--------------------------------------------------------------------------
        Build the instance
    --------------------------------------------------------------------------*/
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();
    setContactInfo ("", 610, 1234567, "123-4", "Main Street", "Exton", "PA", 12345, "nobody@nowhere.com", *instance);

    /*--------------------------------------------------------------------------
        Build the vector of expected values.
        Note: The order does not match the class it matches the classLayout
    --------------------------------------------------------------------------*/
    bvector <AccessStringValuePair> expectedValues;

    addExpectedContactInfo ("", 610, 1234567, "123-4", "Main Street", "Exton", "PA", 12345, "nobody@nowhere.com", expectedValues);

    /*--------------------------------------------------------------------------
        Verify that the values returned from the instance match the expected ones.
    --------------------------------------------------------------------------*/
    ECValuesCollectionPtr   collection = ECValuesCollection::Create (*instance);
    uint32_t                iValue = 0;

    verifyECValueEnumeration (*collection, expectedValues, iValue, false);
    //dumpPropertyValues (*collection, false, 0);

    EXPECT_TRUE (expectedValues.size() == iValue);

    /*--------------------------------------------------------------------------
        Duplicate the instance and verify the duplicate.
    --------------------------------------------------------------------------*/
    StandaloneECInstancePtr standAloneInstance = StandaloneECInstance::Duplicate(*instance);

    collection = ECValuesCollection::Create (*standAloneInstance);
    iValue = 0;
    verifyECValueEnumeration (*collection, expectedValues, iValue, true);
    //dumpPropertyValues (*collection, false, 0);

    EXPECT_TRUE (expectedValues.size() == iValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, RecursiveECValueEnumeration_StructArray)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    StandaloneECEnablerPtr enabler = schema->GetClassP("EmployeeDirectory")->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (enabler.IsValid());

    /*--------------------------------------------------------------------------
        Build the instance
    --------------------------------------------------------------------------*/
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();
    instance->AddArrayElements("Employees", 2);

    StandaloneECEnablerPtr arrayMemberEnabler = schema->GetClassP("Employee")->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (enabler.IsValid());

    ECValue v;
    ECN::StandaloneECInstancePtr arrayMemberInstance1 = arrayMemberEnabler->CreateInstance();
    arrayMemberInstance1->SetValue("Name", ECValue ("John Smith"));

    setContactInfo ("Home",   610, 7654321, "175",   "Oak Lane",    "Wayne", "PA", 12348, "jsmith@home.com", *arrayMemberInstance1);
    setContactInfo ("Work",   610, 1234567, "123-4", "Main Street", "Exton", "PA", 12345, "jsmith@work.com", *arrayMemberInstance1);
    v.SetStruct(arrayMemberInstance1.get());
    instance->SetValue ("Employees", v, 0);

    ECN::StandaloneECInstancePtr arrayMemberInstance2 = arrayMemberEnabler->CreateInstance();
    arrayMemberInstance2->SetValue("Name", ECValue ("Jane Doe"));
    setContactInfo ("Home",   555, 1122334, "1600", "Pennsylvania Ave", "Washington", "DC", 10001, "prez@gmail.com", *arrayMemberInstance2);
    setContactInfo ("Work",   555, 1000000, "1600", "Pennsylvania Ave", "Washington", "DC", 10001, "president@whitehouse.gov", *arrayMemberInstance2);
    v.SetStruct(arrayMemberInstance2.get());
    instance->SetValue ("Employees", v, 1);

    /*--------------------------------------------------------------------------
        Build the vector of expected values.
        Note: The order does not match the class it matches the classLayout
    --------------------------------------------------------------------------*/
    bvector <AccessStringValuePair> expectedValues;

    ECValue arrayValue;
    arrayValue.SetStructArrayInfo (2, false);
    expectedValues.push_back (AccessStringValuePair ("Employees", arrayValue));

    ECValue structValue;
    structValue.SetStruct (arrayMemberInstance1.get());
    expectedValues.push_back (AccessStringValuePair ("Employees[0]", structValue));

    addExpectedContactInfo ("Employees[0].Home", 610, 7654321, "175",   "Oak Lane",    "Wayne", "PA", 12348, "jsmith@home.com", expectedValues);
    addExpectedContactInfo ("Employees[0].Work", 610, 1234567, "123-4", "Main Street", "Exton", "PA", 12345, "jsmith@work.com", expectedValues);

    expectedValues.push_back (AccessStringValuePair ("Employees[0].Name", ECValue ("John Smith")));

    structValue.SetStruct (arrayMemberInstance2.get());
    expectedValues.push_back (AccessStringValuePair ("Employees[1]", structValue));

    addExpectedContactInfo ("Employees[1].Home", 555, 1122334, "1600", "Pennsylvania Ave", "Washington", "DC", 10001, "prez@gmail.com", expectedValues);
    addExpectedContactInfo ("Employees[1].Work", 555, 1000000, "1600", "Pennsylvania Ave", "Washington", "DC", 10001, "president@whitehouse.gov", expectedValues);

    expectedValues.push_back (AccessStringValuePair ("Employees[1].Name", ECValue ("Jane Doe")));

    /*--------------------------------------------------------------------------
        Verify that the values returned from the instance match the expected ones.
    --------------------------------------------------------------------------*/
    ECValuesCollectionPtr   collection = ECValuesCollection::Create (*instance);
    uint32_t                iValue = 0;

    verifyECValueEnumeration (*collection, expectedValues, iValue, false);
    //dumpPropertyValues (*collection, false, 0);

    EXPECT_TRUE (expectedValues.size() == iValue);

    /*--------------------------------------------------------------------------
        Duplicate the instance and verify the duplicate.
    --------------------------------------------------------------------------*/
    StandaloneECInstancePtr standAloneInstance = StandaloneECInstance::Duplicate(*instance);

    collection = ECValuesCollection::Create (*standAloneInstance);
    iValue = 0;
    verifyECValueEnumeration (*collection, expectedValues, iValue, true);
    //dumpPropertyValues (*collection, false, 0);

    EXPECT_TRUE (expectedValues.size() == iValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, MergeStructArray)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    StandaloneECEnablerPtr enabler = schema->GetClassP("EmployeeDirectory")->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (enabler.IsValid());

    /*--------------------------------------------------------------------------
        Build the instance
    --------------------------------------------------------------------------*/
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();
    instance->AddArrayElements("Employees", 2);

    StandaloneECEnablerPtr arrayMemberEnabler = schema->GetClassP("Employee")->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (enabler.IsValid());

    ECValue v;
    ECN::StandaloneECInstancePtr arrayMemberInstance1 = arrayMemberEnabler->CreateInstance();
    arrayMemberInstance1->SetValue("Name", ECValue ("John Smith"));

    setContactInfo ("Home",   610, 7654321, "175",   "Oak Lane",    "Wayne", "PA", 12348, "jsmith@home.com", *arrayMemberInstance1);
    setPartialContactInfo (true, "Work",   610, 1234567, "123-4", "Main Street", "Exton", "PA", 12345, "jsmith@work.com", *arrayMemberInstance1);
    v.SetStruct(arrayMemberInstance1.get());
    instance->SetValue ("Employees", v, 0);

    ECN::StandaloneECInstancePtr arrayMemberInstance2 = arrayMemberEnabler->CreateInstance();
    arrayMemberInstance2->SetValue("Name", ECValue ("Jane Doe"));
    setPartialContactInfo (false, "Home",   555, 1122334, "1600", "Pennsylvania Ave", "Washington", "DC", 10001, "prez@gmail.com", *arrayMemberInstance2);
    setPartialContactInfo (true, "Work",   555, 1000000, "1600", "Pennsylvania Ave", "Washington", "DC", 10001, "president@whitehouse.gov", *arrayMemberInstance2);
    v.SetStruct(arrayMemberInstance2.get());
    instance->SetValue ("Employees", v, 1);

    ECValuesCollectionPtr   collection = ECValuesCollection::Create (*instance);
    int originalCount = 0;
    int count = 0;

    dumpLoadedPropertyValues  (*collection, false, 0, false, originalCount);

    ECN::StandaloneECInstancePtr toInstance = enabler->CreateInstance();

    MemoryECInstanceBase* mbInstance = toInstance->GetAsMemoryECInstanceP ();
    mbInstance->MergePropertiesFromInstance (*instance);

    collection = ECValuesCollection::Create (*toInstance);

    dumpLoadedPropertyValues  (*collection, false, 0, false, count);
    ASSERT_TRUE (count==originalCount);
    ASSERT_TRUE (count==41);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, MergeStruct)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    StandaloneECEnablerPtr enabler = schema->GetClassP("Employee")->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (enabler.IsValid());

    ECValue v;
    ECN::StandaloneECInstancePtr employeeInstance = enabler->CreateInstance();
    employeeInstance->SetValue("Name", ECValue ("John Smith"));

    setPartialContactInfo (false, "Home",   610, 7654321, "175",   "Oak Lane",    "Wayne", "PA", 12348, "jsmith@home.com", *employeeInstance);
    setPartialContactInfo (true, "Work",   610, 1234567, "123-4", "Main Street", "Exton", "PA", 12345, "jsmith@work.com", *employeeInstance);

    int originalCount = 0;
    int count = 0;

    ECValuesCollectionPtr   collection = ECValuesCollection::Create (*employeeInstance);
    dumpLoadedPropertyValues  (*collection, false, 0, false, originalCount);

    ECN::StandaloneECInstancePtr toInstance = enabler->CreateInstance();

    MemoryECInstanceBase* mbInstance = toInstance->GetAsMemoryECInstanceP ();
    mbInstance->MergePropertiesFromInstance (*employeeInstance);

    collection = ECValuesCollection::Create (*toInstance);
    dumpLoadedPropertyValues  (*collection, false, 0, false, count);
    ASSERT_TRUE (count==originalCount);
    ASSERT_TRUE (count==19);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, SimpleMergeTwoInstances)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP primitiveClass = schema->GetClassP ("AllPrimitives");
    ASSERT_TRUE (NULL != primitiveClass);

    StandaloneECEnablerPtr enabler = primitiveClass->GetDefaultStandaloneEnabler();
    
    ECN::StandaloneECInstancePtr sourceInstance0 = enabler->CreateInstance();
    ECN::StandaloneECInstancePtr sourceInstance1 = enabler->CreateInstance();
    ECN::StandaloneECInstancePtr targetInstance  = enabler->CreateInstance();

    ECValue v;
    v.SetDouble(1.0/3.0);
    sourceInstance0->SetValue("ADouble", v);
    v.SetUtf8CP("Weaker source instance");
    sourceInstance0->SetValue("AString", v);
    v.SetInteger(234);
    sourceInstance0->SetValue("AnInt", v);
    v.SetInteger(50);
    sourceInstance0->AddArrayElements("SomeInts", 4);
    sourceInstance0->SetValue("SomeInts", v, 0);
    v.SetInteger(60);
    sourceInstance0->SetValue("SomeInts", v, 1);
    v.SetInteger(70);
    sourceInstance0->SetValue("SomeInts", v, 2);
    v.SetInteger(80);
    sourceInstance0->SetValue("SomeInts", v, 3);

    v.SetDouble(10.0/3.0);
    sourceInstance1->SetValue("ADouble", v);
    v.SetLong((int64_t)2345978);
    sourceInstance1->SetValue("ALong", v);
    v.SetUtf8CP("Dominant source instance");
    sourceInstance1->SetValue("AString", v);
    v.SetInteger(99999999);
    sourceInstance1->AddArrayElements("SomeInts", 4);
    sourceInstance1->SetValue("SomeInts", v, 1);

    /*
    Merging two instances into a third instance:
    In this example, values from sourceInstance 1 will take precedence over 
    values in sourceInstance0 in the even that neither are null.
    Note that in Options::Create (), the second flag is set to true: in this
    case, it is wise to include accessors that have null values.
    */
    ECValuesCollectionPtr collection = ECValuesCollection::Create (*sourceInstance1);

    for (ECPropertyValueCR propertyValue : *collection)
        {
        ECValue             localValue;
        ECValueCP           ecValue  = &propertyValue.GetValue();

        if ( ! ecValue->IsPrimitive())
            continue;

        ECValueAccessorCR   accessor = propertyValue.GetValueAccessor();

        // If the value from instance1 is NULL, try to get it from instance0
        if (ecValue->IsNull())
            {
            sourceInstance0->GetValueUsingAccessor (localValue, accessor);
            ecValue = &localValue;
            }
            
        //set the value to target instance
        if(!ecValue->IsNull())
            targetInstance->SetValueUsingAccessor (accessor, *ecValue);
        }

    int valuesCounted = 0;
    ECValuesCollectionPtr targetCollection = ECValuesCollection::Create (*targetInstance);

    for (ECPropertyValueCR propertyValue : *targetCollection)
        {
        if ( ! propertyValue.GetValue().IsPrimitive())
            continue;

        valuesCounted++;
        //wprintf("%ls: %ls\n", propertyValue.GetValueAccessor().GetManagedAccessString(), propertyValue.GetValue().ToString());
        }

    //Verify that the merge succeeded
    EXPECT_EQ (9, valuesCounted);
    targetInstance->GetValue (v, "AnInt");    //Came from sourceInstance0
    EXPECT_EQ (234, v.GetInteger());
    targetInstance->GetValue (v, "ADouble");  //Came from sourceInstance1
    EXPECT_EQ (10.0/3.0, v.GetDouble());
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, InstantiateStandaloneInstance)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP ("TestClass");
    ASSERT_TRUE (NULL != ecClass);

    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();
    Utf8String instanceId = instance->GetInstanceId();
    instance->ToString("").c_str();
    ExerciseInstance (*instance, "Test");

    // instance.Compact()... then check values again
    
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, InstantiateInstanceWithNoProperties)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP ("EmptyClass");
    ASSERT_TRUE (NULL != ecClass);

    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();
    Utf8String instanceId = instance->GetInstanceId();

    instance->ToString("").c_str();

    // instance.Compact()... then check values again
    
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, DirectSetStandaloneInstance)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP ("CadData");
    ASSERT_TRUE (NULL != ecClass);
    
    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    DPoint2d   inSize = {10.5, 22.3};
    DPoint3d   inPoint1 = {10.10, 11.11, 12.12};
    DPoint3d   inPoint2 ={200.100, 210.110, 220.120};
    DateTime   inTimeUtc = DateTime::GetCurrentTimeUtc ();
    int        inCount = 100;
    double     inLength = 432.178;
    bool       inTest = true;
    int64_t    inTicks = 634027121070910000;

    instance->SetValue ("Count",        ECValue (inCount));
    instance->SetValue ("Name",         ECValue ("Test"));
    instance->SetValue ("Length",       ECValue (inLength));
    instance->SetValue ("Field_Tested", ECValue (inTest));
    instance->SetValue ("Size",         ECValue (inSize));
    instance->SetValue ("StartPoint",   ECValue (inPoint1));
    instance->SetValue ("EndPoint",     ECValue (inPoint2));
    instance->SetValue ("Service_Date", ECValue (inTimeUtc));

    ECValue ecValue;
    ecValue.SetDateTimeTicks(inTicks);
    instance->SetValue ("Install_Date", ecValue);

    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "Count"));
    EXPECT_TRUE (ecValue.GetInteger() == inCount);
    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "Name"));
    EXPECT_STREQ (ecValue.GetUtf8CP(), "Test");
    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "Length"));
    EXPECT_TRUE (ecValue.GetDouble() == inLength);
    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "Field_Tested"));
    EXPECT_TRUE (ecValue.GetBoolean() == inTest);
    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "Size"));
    DPoint2d    point2d = ecValue.GetPoint2D ();
    EXPECT_TRUE (SUCCESS == memcmp (&inSize, &point2d, sizeof(DPoint2d)));
    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "StartPoint"));
    DPoint3d    point3d = ecValue.GetPoint3D ();
    EXPECT_TRUE (SUCCESS == memcmp (&inPoint1, &point3d, sizeof(DPoint3d)));
    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "EndPoint"));
    point3d = ecValue.GetPoint3D ();
    EXPECT_TRUE (SUCCESS == memcmp (&inPoint2, &point3d, sizeof(DPoint3d)));
    //in absence of the DateTimeInfo custom attribute on Service_Date the retrieved
    //date time will always be of kind Unspecified, i.e. the original kind (here Utc)
    //gets lost
    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "Service_Date"));
    DateTime  sysTime = ecValue.GetDateTime ();
    EXPECT_TRUE (inTimeUtc.Equals (sysTime, true));
    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "Install_Date"));
    EXPECT_TRUE (ecValue.GetDateTimeTicks() == inTicks);

    // instance.Compact()... then check values again
    
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, GetSetValuesByIndex)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP ("TestClass");
    ASSERT_TRUE (NULL != ecClass);
    
    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    Utf8CP accessString = "Property34";

    //UInt32          intValue = 12345;
    Utf8CP stringValue = "Xyz";

    //instance->SetValue  (accessString, ECValue (intValue));
    instance->SetValue  (accessString, ECValue (stringValue));

    ECValue value;
    uint32_t propertyIndex = 0;

    EXPECT_TRUE (ECObjectsStatus::Success == enabler->GetPropertyIndex (propertyIndex, accessString));
    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (value, propertyIndex));
    //EXPECT_TRUE (intValue == value.GetInteger());
    EXPECT_STREQ (stringValue, value.GetUtf8CP());

#if defined (TIMING_ACCESS_BYINDEX)
    uint32_t    numAccesses = 10000000;

    double      elapsedTime1 = 0.0;
    StopWatch   timer1 ("Time getting values using index", true);

    for (uint32_t i = 0; i < numAccesses; i++)
        {
        timer1.Start();
        instance->GetValue (value, propertyIndex);
        timer1.Stop();

        elapsedTime1 += timer1.GetElapsedSeconds();
        }

    double      elapsedTime2 = 0.0;
    StopWatch   timer2 ("Time getting values using accessString", true);

    for (uint32_t i = 0; i < numAccesses; i++)
        {
        timer2.Start();
        instance->GetValue (value, accessString);
        timer2.Stop();

        elapsedTime2 += timer2.GetElapsedSeconds();
        }

    printf ("Time to set %d values by: accessString = %.4f, index = %.4f\n", numAccesses, elapsedTime1, elapsedTime2);
#endif

    // instance.Compact()... then check values again
    
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryLayoutTests, ExpectErrorsWhenViolatingArrayConstraints)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP ("TestClass");
    ASSERT_TRUE (NULL != ecClass);    
    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    {
    DISABLE_ASSERTS

#if FIXED_COUNT_ARRAYS_ARE_SUPPORTED
    // verify we can not change the size of fixed arrays        
    ASSERT_EQ (FIXED_COUNT_ARRAYS_ARE_SUPPORTED ? true : false, ECObjectsStatus::Success != instance->InsertArrayElements ("FixedArrayFixedElement", 0, 1));
    ASSERT_EQ (FIXED_COUNT_ARRAYS_ARE_SUPPORTED ? true : false, ECObjectsStatus::Success != instance->InsertArrayElements ("FixedArrayFixedElement", 10, 1));
    ASSERT_EQ (FIXED_COUNT_ARRAYS_ARE_SUPPORTED ? true : false, ECObjectsStatus::Success != instance->AddArrayElements    ("FixedArrayFixedElement", 1));
    ASSERT_EQ (FIXED_COUNT_ARRAYS_ARE_SUPPORTED ? true : false, ECObjectsStatus::Success != instance->InsertArrayElements ("FixedArrayVariableElement", 0, 1));
    ASSERT_EQ (FIXED_COUNT_ARRAYS_ARE_SUPPORTED ? true : false, ECObjectsStatus::Success != instance->InsertArrayElements ("FixedArrayVariableElement", 12, 1));
    ASSERT_EQ (FIXED_COUNT_ARRAYS_ARE_SUPPORTED ? true : false, ECObjectsStatus::Success != instance->AddArrayElements    ("FixedArrayVariableElement", 1));
#endif

    // verify constraints of array insertion are enforced
    ASSERT_TRUE (ECObjectsStatus::Success != instance->InsertArrayElements ("NonExistArray", 0, 1));
    ASSERT_TRUE (ECObjectsStatus::Success != instance->InsertArrayElements ("BeginningArray", 2, 1)); // insert index is invalid    
    ASSERT_TRUE (ECObjectsStatus::Success != instance->InsertArrayElements ("BeginningArray", 0, 0)); // insert count is invalid    
    }
    
    ECValue v;
    VerifyOutOfBoundsError (*instance, v, "BeginningArray", 0);
    VerifyOutOfBoundsError (*instance, v, "FixedArrayFixedElement", 10);
    VerifyOutOfBoundsError (*instance, v, "VariableArrayFixedElement", 0);
    VerifyOutOfBoundsError (*instance, v, "FixedArrayVariableElement", 12);
    VerifyOutOfBoundsError (*instance, v, "VariableArrayVariableElement", 0);
    VerifyOutOfBoundsError (*instance, v, "EndingArray", 0);                     
    };    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (MemoryLayoutTests, Values) // move it!
    {
    ECValue i(3);
    EXPECT_TRUE (i.IsInteger());
    EXPECT_TRUE (!i.IsNull());
    EXPECT_EQ (3, i.GetInteger());    
    i.SetInteger(4);
    EXPECT_EQ (4, i.GetInteger());
    
    i.SetUtf8CP("Type changed to string");
    EXPECT_TRUE (i.IsString());
    EXPECT_TRUE (!i.IsNull());
    EXPECT_STREQ ("Type changed to string", i.GetUtf8CP());    
    
    i.Clear();
    EXPECT_TRUE (i.IsUninitialized());
    EXPECT_TRUE (i.IsNull());
    
    ECValue v;
    EXPECT_TRUE (v.IsUninitialized());
    EXPECT_TRUE (v.IsNull());
    
    double doubleValue = 1./3.;
    v.SetDouble(doubleValue);
    EXPECT_TRUE (v.IsDouble());
    EXPECT_EQ (doubleValue, v.GetDouble());
    
    ECValue nullInt (ECN::PRIMITIVETYPE_Integer);
    EXPECT_TRUE (nullInt.IsNull());
    EXPECT_TRUE (nullInt.IsInteger());

    ECValue long64 ((::int64_t)3);
    EXPECT_TRUE (!long64.IsNull());
    EXPECT_TRUE (long64.IsLong());
    EXPECT_EQ (3, long64.GetLong());

    ECValue s("Hello");
    EXPECT_TRUE (s.IsString());
    EXPECT_TRUE (!s.IsNull());
    EXPECT_STREQ ("Hello", s.GetUtf8CP());
    const Utf8String ws = s.GetUtf8CP();
    
    s.SetUtf8CP("Nice one");
    EXPECT_STREQ ("Nice one", s.GetUtf8CP());
    
    s.SetUtf8CP(NULL);
    EXPECT_TRUE (s.IsNull());
    EXPECT_TRUE (NULL == s.GetUtf8CP());
    
    ECValue snull((wchar_t*)NULL);
    EXPECT_TRUE (snull.IsString());
    EXPECT_TRUE (snull.IsNull());
    //WCharCP wcnull = snull.GetString();
    EXPECT_EQ (NULL, s.GetUtf8CP());
    
    //bool
    ECValue boolVal(true);
    EXPECT_TRUE (boolVal.IsBoolean());
    EXPECT_TRUE (boolVal.GetBoolean());

    //DPoint3d
    DPoint3d inPoint3 = {10.0, 100.0, 1000.0};
    ECValue pntVal3(inPoint3);
    DPoint3d outPoint3 = pntVal3.GetPoint3D ();
    EXPECT_TRUE (pntVal3.IsPoint3D());
    EXPECT_TRUE (0 == memcmp(&inPoint3, &outPoint3, sizeof(outPoint3)));
    Utf8String point3Str = pntVal3.ToString();
    EXPECT_TRUE (0 == point3Str.compare ("10,100,1000"));

    //DPoint2d
    DPoint2d inPoint2 = {10.0, 100.0};
    ECValue pntVal2 (inPoint2);
    EXPECT_TRUE (pntVal2.IsPoint2D());
    DPoint2d outPoint2 = pntVal2.GetPoint2D ();
    EXPECT_TRUE (0 == memcmp(&inPoint2, &outPoint2, sizeof(outPoint2)));
    Utf8String point2Str = pntVal2.ToString();
    EXPECT_TRUE (0 == point2Str.compare ("10,100"));

    // DateTime
    DateTime nowUtc = DateTime::GetCurrentTimeUtc ();
    ECValue dateValue (nowUtc);
    EXPECT_TRUE (dateValue.IsDateTime());
    DateTime nowToo = dateValue.GetDateTime ();
    EXPECT_TRUE (nowToo.Equals (nowUtc, true));

    ECValue fixedDate;
    fixedDate.SetDateTimeTicks (634027121070910000);
    Utf8String dateStr = fixedDate.ToString();
    EXPECT_TRUE (0 == dateStr.compare ("2010-02-25T16:28:27.091")) << "Expected date: " << fixedDate.GetDateTime ().ToString ().c_str ();
    };
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (MemoryLayoutTests, TestSetGetNull)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP ("TestClass");
    ASSERT_TRUE (NULL != ecClass);
        
    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();
    ECValue v;
    
    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (v, "D"));
    EXPECT_TRUE (v.IsNull());
    
    double doubleValue = 1.0/3.0;
    SetAndVerifyDouble (*instance, v, "D", doubleValue);
    EXPECT_TRUE (!v.IsNull());    
    
    v.SetToNull();
    EXPECT_TRUE (ECObjectsStatus::Success == instance->SetValue ("D", v));
    v.SetUtf8CP("Just making sure that it is not NULL before calling GetValue in the next line.");
    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (v, "D"));
    EXPECT_TRUE (v.IsNull());
        
    SetAndVerifyString (*instance, v, "S", "Yo!");

    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (v, "D"));
    EXPECT_TRUE (v.IsNull());    
    
    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (v, "S"));
    EXPECT_FALSE (v.IsNull());     
    };

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (MemoryLayoutTests, TestPropertyReadOnly)
    {
    //"    <ECClass typeName=\"Car\" isStruct=\"True\" isDomainClass=\"True\">"
    //"        <ECProperty       propertyName=\"Name\"       typeName=\"string\"/>"
    //"        <ECProperty       propertyName=\"Wheels\"     typeName=\"int\"  readOnly=\"True\"/>"
    //"    </ECClass>"

    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP ("Car");
    ASSERT_TRUE (NULL != ecClass);
        
    StandaloneECEnablerPtr enabler = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();
    
    Utf8CP nameAccessString = "Name";
    Utf8CP wheelsAccessString = "Wheels";
    uint32_t namePropertyIndex = 9999;
    uint32_t wheelsPropertyIndex = 9998;
    EXPECT_TRUE (ECObjectsStatus::Success == enabler->GetPropertyIndex (namePropertyIndex, nameAccessString));
    EXPECT_TRUE (ECObjectsStatus::Success == enabler->GetPropertyIndex (wheelsPropertyIndex, wheelsAccessString));

    EXPECT_FALSE (instance->IsPropertyReadOnly (nameAccessString));
    EXPECT_FALSE (instance->IsPropertyReadOnly (namePropertyIndex));

    EXPECT_TRUE  (instance->IsPropertyReadOnly (wheelsAccessString));
    EXPECT_TRUE  (instance->IsPropertyReadOnly (wheelsPropertyIndex));  

    ECValue v;
    v.SetInteger(610);
    EXPECT_TRUE (ECObjectsStatus::Success == instance->SetValue (wheelsAccessString, v));  // should work since original value is NULL
    v.SetInteger(512);
    EXPECT_TRUE (ECObjectsStatus::UnableToSetReadOnlyProperty == instance->SetValue (wheelsAccessString, v));  // should fail since read only and value is not NULL

    // make sure we can copy an instance contains read only properties
    StandaloneECInstancePtr  copyInstance =  StandaloneECInstance::Duplicate (*instance);
    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (v, wheelsAccessString));
    EXPECT_TRUE (610 == v.GetInteger());

    // make sure we can deserialize and instance from XML that contains read only properties
    Utf8String ecInstanceXml;
    instance->WriteToXmlString (ecInstanceXml, true, false);
    ECN::IECInstancePtr deserializedInstance = NULL;
    ECN::ECInstanceReadContextPtr instanceContext = ECN::ECInstanceReadContext::CreateContext (*schema);
    EXPECT_TRUE (InstanceReadStatus::Success == IECInstance::ReadFromXmlString(deserializedInstance, ecInstanceXml.c_str(), *instanceContext));
    EXPECT_TRUE (ECObjectsStatus::Success == deserializedInstance->GetValue (v, wheelsAccessString));
    EXPECT_TRUE (610 == v.GetInteger());
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (MemoryLayoutTests, TestBinarySetGet)
    {
    const static bool HOLD_AS_DUPLICATE = true;
    const Byte binaryValue0[4] = {0x00, 0x01, 0x02, 0x03};
    const Byte binaryValue1[2] = {0x99, 0x88};

    EXPECT_EQ (sizeof(binaryValue0), 4);
    EXPECT_EQ (sizeof(binaryValue1), 2);

    ECValue v0In;
    ECValue v0Out;
    ECValue v1In;
    ECValue v1Out;

    v0In.SetBinary(binaryValue0, sizeof(binaryValue0), HOLD_AS_DUPLICATE);
    v1In.SetBinary(binaryValue1, sizeof(binaryValue1), HOLD_AS_DUPLICATE);

    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP ("AllPrimitives");
    ASSERT_TRUE (NULL != ecClass);
        
    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();
    
    EXPECT_TRUE (ECObjectsStatus::Success == instance->SetValue ("ABinary", v0In));
    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (v0Out, "ABinary"));
    EXPECT_TRUE (v0In.Equals (v0Out));

    // now set it to a smaller size
    EXPECT_TRUE (ECObjectsStatus::Success == instance->SetValue ("ABinary", v1In));
    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (v1Out, "ABinary"));
    EXPECT_TRUE (v1In.Equals (v1Out));
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static void validateArrayCount  (ECN::StandaloneECInstanceCR instance, Utf8CP propertyName, uint32_t expectedCount)
    {
    ECValue varray;
    EXPECT_TRUE (ECObjectsStatus::Success == instance.GetValue (varray, propertyName));
    uint32_t count = varray.GetArrayInfo().GetCount();
    EXPECT_TRUE (count == expectedCount);

    ECValue ventry;

    for (uint32_t i=0; i<count; i++)
        {
        EXPECT_TRUE (ECObjectsStatus::Success == instance.GetValue (ventry, propertyName, i));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (MemoryLayoutTests, TestRemovingArrayEntries)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP ("ArrayTest");
    ASSERT_TRUE (NULL != ecClass);
        
    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();
    
#if FIXED_COUNT_ARRAYS_ARE_SUPPORTED
    EXPECT_TRUE (SUCCESS == instance->SetValue ("FixedArrayFixedElement",  ECValue ((int)1), 1));
    EXPECT_TRUE (SUCCESS == instance->SetValue ("FixedArrayFixedElement",  ECValue ((int)3), 3));
    EXPECT_TRUE (SUCCESS == instance->SetValue ("FixedArrayFixedElement",  ECValue ((int)5), 5));
    EXPECT_TRUE (SUCCESS == instance->SetValue ("FixedArrayFixedElement",  ECValue ((int)7), 7));
    EXPECT_TRUE (SUCCESS == instance->SetValue ("FixedArrayFixedElement",  ECValue ((int)9), 9));

    {
    DISABLE_ASSERTS    
    EXPECT_TRUE (ECObjectsStatus::Success != instance->RemoveArrayElement("FixedArrayFixedElement", 2));
    }

    EXPECT_TRUE (SUCCESS == instance->SetValue ("FixedArrayVariableElement", ECValue ("ArrayMember 1"), 1));
    EXPECT_TRUE (SUCCESS == instance->SetValue ("FixedArrayVariableElement", ECValue ("ArrayMember 3"), 3));
    EXPECT_TRUE (SUCCESS == instance->SetValue ("FixedArrayVariableElement", ECValue ("ArrayMember 5"), 5));
    EXPECT_TRUE (SUCCESS == instance->SetValue ("FixedArrayVariableElement", ECValue ("ArrayMember 7"), 7));
    EXPECT_TRUE (SUCCESS == instance->SetValue ("FixedArrayVariableElement", ECValue ("ArrayMember 9"), 9));
    EXPECT_TRUE (SUCCESS == instance->SetValue ("FixedArrayVariableElement", ECValue ("ArrayMember 11"), 11));

    {
    DISABLE_ASSERTS    
    EXPECT_TRUE (ECObjectsStatus::Success != instance->RemoveArrayElement("FixedArrayVariableElement", 2));
    }
#endif

    instance->AddArrayElements("SomeStrings", 5);

    EXPECT_TRUE (ECObjectsStatus::Success == instance->SetValue ("SomeStrings",  ECValue ("ArrayMember 0"), 0));
    EXPECT_TRUE (ECObjectsStatus::Success == instance->SetValue ("SomeStrings",  ECValue ("ArrayMember 1"), 1));
    EXPECT_TRUE (ECObjectsStatus::Success == instance->SetValue ("SomeStrings",  ECValue ("ArrayMember 2"), 2));
    // leave index 3 null
    EXPECT_TRUE (ECObjectsStatus::Success == instance->SetValue ("SomeStrings",  ECValue ("ArrayMember 4"), 4));

    validateArrayCount (*instance, "SomeStrings", 5); 

    instance->AddArrayElements("SomeInts", 6);

    EXPECT_TRUE (ECObjectsStatus::Success == instance->SetValue ("SomeInts",  ECValue ((int)0), 0));
    EXPECT_TRUE (ECObjectsStatus::Success == instance->SetValue ("SomeInts",  ECValue ((int)1), 1));
    EXPECT_TRUE (ECObjectsStatus::Success == instance->SetValue ("SomeInts",  ECValue ((int)2), 2));
    // leave index 3 null
    EXPECT_TRUE (ECObjectsStatus::Success == instance->SetValue ("SomeInts",  ECValue ((int)4), 4));
    EXPECT_TRUE (ECObjectsStatus::Success == instance->SetValue ("SomeInts",  ECValue ((int)5), 5));

    validateArrayCount (*instance, "SomeInts", 6); 

    // define struct array
    StandaloneECEnablerPtr manufacturerEnabler = instance->GetEnablerR().GetEnablerForStructArrayMember (schema->GetSchemaKey(), "Manufacturer"); 
    EXPECT_TRUE (manufacturerEnabler.IsValid());

    ECValue v;
    ASSERT_TRUE (ECObjectsStatus::Success == instance->AddArrayElements ("ManufacturerArray", 4));
    VerifyArrayInfo (*instance, v, "ManufacturerArray", 4, false);
    VerifyIsNullArrayElements (*instance, v, "ManufacturerArray", 0, 4, true);

    IECInstancePtr manufInst = manufacturerEnabler->CreateInstance().get();    

    SetAndVerifyString (*manufInst, v, "Name", "Nissan");
    SetAndVerifyInteger (*manufInst, v, "AccountNo", 3475);
    v.SetStruct (manufInst.get());
    ASSERT_TRUE (ECObjectsStatus::Success == instance->SetValue ("ManufacturerArray", v, 0));

    manufInst = manufacturerEnabler->CreateInstance().get();    
    SetAndVerifyString (*manufInst, v, "Name", "Kia");
    SetAndVerifyInteger (*manufInst, v, "AccountNo", 1791);
    v.SetStruct (manufInst.get());
    ASSERT_TRUE (ECObjectsStatus::Success == instance->SetValue ("ManufacturerArray", v, 1));

    manufInst = manufacturerEnabler->CreateInstance().get();    
    SetAndVerifyString (*manufInst, v, "Name", "Honda");
    SetAndVerifyInteger (*manufInst, v, "AccountNo", 1592);
    v.SetStruct (manufInst.get());
    ASSERT_TRUE (ECObjectsStatus::Success == instance->SetValue ("ManufacturerArray", v, 2));

    manufInst = manufacturerEnabler->CreateInstance().get();    
    SetAndVerifyString (*manufInst, v, "Name", "Chevy");
    SetAndVerifyInteger (*manufInst, v, "AccountNo", 19341);
    v.SetStruct (manufInst.get());
    ASSERT_TRUE (ECObjectsStatus::Success == instance->SetValue ("ManufacturerArray", v, 3));

    VerifyIsNullArrayElements (*instance, v, "ManufacturerArray", 0, 4, false);    

    // remove from start of array
    instance->RemoveArrayElement("SomeStrings", 0);
    validateArrayCount (*instance, "SomeStrings", 4); 

    // remove from middle of array
    instance->RemoveArrayElement("SomeStrings", 2);
    validateArrayCount (*instance, "SomeStrings", 3); 

    // remove from end of array
    instance->RemoveArrayElement("SomeInts", 2);
    validateArrayCount (*instance, "SomeInts", 5);

    // remove struct array element
    instance->RemoveArrayElement("ManufacturerArray", 2);
    validateArrayCount (*instance, "ManufacturerArray", 3);
    }

TEST_F (MemoryLayoutTests, IterateCompleClass)
    {
    ECSchemaPtr        schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP ecClass = schema->GetClassP ("ComplexClass");
    ASSERT_TRUE (NULL != ecClass);
        
    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();

    ECValue b(true);
    ECValue s1("719372644");
    ECValue s2("asasdasd");
    ECValue s3("1338164264");
    ECValue s4("string val");
    ECValue s5("asdasdas");
    ECValue s6("392010267");
    ECValue i1((int)1683483880);
    ECValue i2((int)1367822242);
    ECValue i3((int)32323);
    ECValue d1(0.71266461290077521);

    EXPECT_TRUE (ECObjectsStatus::Success == instance->SetValue ("StringProperty", s4));
    EXPECT_TRUE (ECObjectsStatus::Success == instance->SetValue ("IntProperty", i2));

    StandaloneECEnablerPtr structArrayEnabler = schema->GetClassP("StructClass")->GetDefaultStandaloneEnabler ();
    ECN::StandaloneECInstancePtr structInstance = structArrayEnabler->CreateInstance();

    EXPECT_TRUE (ECObjectsStatus::Success == instance->SetValue ("BooleanProperty", b));
    EXPECT_TRUE (ECObjectsStatus::PropertyValueMatchesNoChange == instance->ChangeValue ("BooleanProperty", b));
#if !FIXED_COUNT_ARRAYS_ARE_SUPPORTED
    EXPECT_TRUE (ECObjectsStatus::Success == instance->AddArrayElements ("SimpleArrayProperty", 1));
#endif
    EXPECT_TRUE (ECObjectsStatus::Success == instance->SetValue ("SimpleArrayProperty", s1, 0));
    EXPECT_TRUE (ECObjectsStatus::Success == instance->SetValue ("StructProperty.StringProperty", s2));
    EXPECT_TRUE (ECObjectsStatus::PropertyValueMatchesNoChange == instance->ChangeValue ("StructProperty.StringProperty", s2));
    EXPECT_TRUE (ECObjectsStatus::Success == instance->SetValue ("StructProperty.IntProperty", i1));
    EXPECT_TRUE (ECObjectsStatus::PropertyValueMatchesNoChange == instance->ChangeValue ("StructProperty.IntProperty", i1));
#if !FIXED_COUNT_ARRAYS_ARE_SUPPORTED
    EXPECT_TRUE (ECObjectsStatus::Success == instance->AddArrayElements ("StructProperty.ArrayProperty", 1));
#endif
    EXPECT_TRUE (ECObjectsStatus::Success == instance->SetValue ("StructProperty.ArrayProperty", s3, 0));
    EXPECT_TRUE (ECObjectsStatus::Success == instance->SetValue ("DoubleProperty", d1));
    EXPECT_TRUE (ECObjectsStatus::PropertyValueMatchesNoChange == instance->ChangeValue ("DoubleProperty", d1));

    EXPECT_TRUE (ECObjectsStatus::Success == structInstance->SetValue ("StringProperty", s5));
    EXPECT_TRUE (ECObjectsStatus::Success == structInstance->SetValue ("IntProperty", i3));
#if !FIXED_COUNT_ARRAYS_ARE_SUPPORTED
    EXPECT_TRUE (ECObjectsStatus::Success == structInstance->AddArrayElements ("ArrayProperty", 1));
#endif
    EXPECT_TRUE (ECObjectsStatus::Success == structInstance->SetValue ("ArrayProperty", s6, 0));

#if FIXED_COUNT_ARRAYS_ARE_SUPPORTED
    // This is a fixed-size struct array so we don't have to insert members
#else
    EXPECT_TRUE (ECObjectsStatus::Success == instance->AddArrayElements ("StructArrayProperty", 1));
#endif
    ECValue structVal;
    structVal.SetStruct (structInstance.get());
    EXPECT_TRUE (ECObjectsStatus::Success == instance->SetValue ("StructArrayProperty", structVal, 0));

    // ensure we can walk the properties
    ECValuesCollectionPtr   collection = ECValuesCollection::Create (*instance);
    //dumpPropertyValues (*collection, false, 0);
    }

TEST_F (MemoryLayoutTests, ProfileSettingValues)
    {
    int nStrings = 100;
    int nInstances = 1000;

    ECSchemaPtr         schema      = CreateProfilingSchema(nStrings);
    ECClassP           ecClass     = schema->GetClassP ("Pidget");
    ASSERT_TRUE (NULL != ecClass);
        
    StandaloneECEnablerPtr enabler       = ecClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance();
    
    //UInt32 slack = 0;
    double elapsedSeconds = 0.0;
    StopWatch timer ("Time setting of values in a new StandaloneECInstance", true);
    for (int i = 0; i < nInstances; i++)
        {
        timer.Start();
        SetValuesForProfiling (*instance);
        timer.Stop();
        
        elapsedSeconds += timer.GetElapsedSeconds();
        instance->GetAsMemoryECInstanceP()->ClearValues();
        }
    
    //printf ("  %d StandaloneECInstances with %d string properties initialized in %.4f seconds.\n", nInstances, nStrings, elapsedSeconds);
    }
    

TEST_F (MemoryLayoutTests, TestValueAccessor)
    {
    ECValueAccessor m_accessor;
    }

TEST_F (MemoryLayoutTests, GeometrySetGet)
    {
    ECSchemaPtr testSchema;
    ECSchema::CreateSchema(testSchema, "GeometrySchema", 1, 0);
    ECEntityClassP geomClass;
    testSchema->CreateEntityClass(geomClass, "GeometryStore");

    PrimitiveECPropertyP stringProp;
    geomClass->CreatePrimitiveProperty(stringProp, "Name");

    PrimitiveECPropertyP geomProperty;
    geomClass->CreatePrimitiveProperty(geomProperty, "MyGeometry");
    geomProperty->SetTypeName ("Bentley.Geometry.Common.IGeometry");

    IECInstancePtr instance = geomClass->GetDefaultStandaloneEnabler()->CreateInstance();

    DEllipse3d ellipse;
    ellipse.Init (0.0, 0.0, 0.0, 10000, 0.0, 0.0, 0.0, 10000, 0.0, 0.0, msGeomConst_2pi);
    ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc (ellipse);
    IGeometryPtr geometryPtr = IGeometry::Create (arc);
    ECValue v;
    BentleyStatus vStatus = v.SetIGeometry (*geometryPtr);
    ASSERT_EQ(SUCCESS ,vStatus);

    IGeometryPtr storedGeometryPtr1 = v.GetIGeometry ();
    ASSERT_TRUE(storedGeometryPtr1.IsValid());
    ECObjectsStatus status = instance->SetValue ("MyGeometry", v);
    ASSERT_EQ(ECObjectsStatus::Success,status);

    }

/*---------------------------------------------------------------------------------**//**
* TFS#290587: When you modify a property (e.g. by changing its read-only flag, type, etc)
* its containing ECClass's cached ClassLayout should be updated to reflect the change.
* @bsistruct                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct DefaultStandaloneEnablerTests : ECTestFixture
{
    ECSchemaPtr m_schema;
    ECEntityClassP    m_class;

    DefaultStandaloneEnablerTests() : m_class(nullptr)
        {
        ECSchema::CreateSchema(m_schema, "TestSchema", 1, 0);
        m_schema->CreateEntityClass(m_class, "TestClass");
        }

    ECStructClassP CreateStructClass(Utf8StringCR structName, Utf8StringCR propName)
        {
        ECStructClassP structClass;
        m_schema->CreateStructClass(structClass, structName);
        PrimitiveECPropertyP prop;
        structClass->CreatePrimitiveProperty(prop, propName);
        return structClass;
        }

    StandaloneECInstancePtr CreateInstance() const { return m_class->GetDefaultStandaloneEnabler()->CreateInstance(); }

    StandaloneECInstancePtr CreateInstanceWithArray(Utf8CP arrayAccessString) const
        {
        auto instance = CreateInstance();
        EXPECT_EQ(ECObjectsStatus::Success, instance->AddArrayElements(arrayAccessString, 1));
        return instance;
        }

    template<typename T> ECObjectsStatus SetValue(Utf8CP accessString, T const& value, uint32_t arrayIndex = -1) const
        {
        ECValue v(value);
        auto instance = -1 == arrayIndex ? CreateInstance() : CreateInstanceWithArray(accessString);
        return -1 == arrayIndex ? instance->SetValue(accessString, v) : instance->SetValue(accessString, v, arrayIndex);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultStandaloneEnablerTests, ReadOnly)
    {
    PrimitiveECPropertyP prop;
    m_class->CreatePrimitiveProperty(prop, "Prop");
    EXPECT_EQ(ECObjectsStatus::Success, SetValue("Prop", "abc"));
    prop->SetIsReadOnly(true);
        {
        // An initial SetValue() will succeed, because the property is null
        auto instance = CreateInstance();
        EXPECT_EQ(ECObjectsStatus::Success, instance->SetValue("Prop", ECValue("def", false)));
        // Setting a non-null read-only property will fail
        EXPECT_EQ(ECObjectsStatus::UnableToSetReadOnlyProperty, instance->SetValue("Prop", ECValue("uvw", false)));
        }

    prop->SetIsReadOnly(false);
        {
        auto instance = CreateInstance();
        EXPECT_EQ(ECObjectsStatus::Success, instance->SetValue("Prop", ECValue("xyz", false)));  // set from null
        EXPECT_EQ(ECObjectsStatus::Success, instance->SetValue("Prop", ECValue("abc", false)));  // set from non-null
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultStandaloneEnablerTests, Type)
    {
    PrimitiveECPropertyP primProp;
    m_class->CreatePrimitiveProperty(primProp, "Prim");
    EXPECT_EQ(ECObjectsStatus::Success, SetValue("Prim", "abc"));
    primProp->SetType(PRIMITIVETYPE_Integer);
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, SetValue("Prim", "def"));
    EXPECT_EQ(ECObjectsStatus::Success, SetValue("Prim", 123));

    ECStructClassP oldStruct = CreateStructClass("OldStruct", "Old");
    StructECPropertyP structProp;
    m_class->CreateStructProperty(structProp, "Struct", *oldStruct);
    EXPECT_EQ(ECObjectsStatus::Success, SetValue("Struct.Old", "abc"));
    structProp->SetType(*CreateStructClass("NewStruct", "New"));
    EXPECT_EQ(ECObjectsStatus::PropertyNotFound, SetValue("Struct.Old", "def"));
    EXPECT_EQ(ECObjectsStatus::Success, SetValue("Struct.New", "xyz"));

    ArrayECPropertyP arrayProp;
    m_class->CreateArrayProperty(arrayProp, "Array", PRIMITIVETYPE_String);
    EXPECT_EQ(ECObjectsStatus::Success, SetValue("Array", "abc", 0));
    arrayProp->SetPrimitiveElementType(PRIMITIVETYPE_Integer);
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, SetValue("Array", "def", 0));
    EXPECT_EQ(ECObjectsStatus::Success, SetValue("Array", 123, 0));
    }

END_BENTLEY_ECN_TEST_NAMESPACE
