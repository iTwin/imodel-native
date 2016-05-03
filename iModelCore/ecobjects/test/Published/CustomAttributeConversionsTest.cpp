/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/CustomAttributeConversionsTest.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct CustomAttributeRemovalTest : ECTestFixture
    {
    ECSchemaReadContextPtr   m_readContext = ECSchemaReadContext::CreateContext();
    Utf8String m_customAttributeName;
    ECSchemaPtr m_schema;
    ECSchemaPtr m_refSchema;

    CustomAttributeRemovalTest(Utf8String customAttributeName)
        :m_customAttributeName(customAttributeName) {}
    
    ~CustomAttributeRemovalTest()
        {
        if (m_refSchema.IsValid())
            ValidateSchema(*m_refSchema, false);

        if (m_schema.IsValid())
            ValidateSchema(*m_schema, false);
        }

    void ReadSchema(Utf8CP schemaString)
        {
        ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(m_schema, schemaString, *m_readContext));
        }

    void ReadSchemaWithRef(Utf8CP refSchemaString, Utf8CP schemaString)
        {
        ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(m_refSchema, refSchemaString, *m_readContext));
        ReadSchema(schemaString);
        }

    bool HasCustomAttribute(ECSchemaR schema, Utf8StringCR customAttributeName)
        {
        for (auto ecClass : schema.GetClasses())
            {
            for (auto ecProp : ecClass->GetProperties(false))
                {
                auto instance = ecProp->GetPrimaryCustomAttributeLocal(customAttributeName);
                if (instance.IsValid())
                    return true;
                }
            }
        return false;
        }

    void ValidateSchema(ECSchemaR schema, bool useFreshReadContext = true)
        {
        EXPECT_FALSE(HasCustomAttribute(*m_schema, m_customAttributeName)) << "Schema still has " << m_customAttributeName << "CustomAttribute !!";
        }
    };

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct StandardValueToEnumConversionTest : CustomAttributeRemovalTest
    {
    ECSchemaReadContextPtr   m_validationReadContext = ECSchemaReadContext::CreateContext();

    StandardValueToEnumConversionTest()
        :CustomAttributeRemovalTest("StandardValues") {}

    ~StandardValueToEnumConversionTest()
        {
        if (m_refSchema.IsValid())
            ValidateSchema(*m_refSchema, false);

        if (m_schema.IsValid())
            ValidateSchema(*m_schema, false);
        }


    void ValidateSchema(ECSchemaR schema, bool useFreshReadContext = true)
        {
        Utf8String out;
        EXPECT_EQ(SchemaWriteStatus::Success, schema.WriteToXmlString(out, 3));

        ECSchemaPtr schemaCopy;
        if (useFreshReadContext)
            {
            ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
            EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopy, out.c_str(), *readContext));
            }
        else
            EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopy, out.c_str(), *m_validationReadContext));

        EXPECT_EQ(schema.GetEnumerationCount(), schemaCopy->GetEnumerationCount()) << "Enumeration count doesnot match";
        for (auto ecEnum : schema.GetEnumerations())
            {
            ECEnumerationCP copy = schemaCopy->GetEnumerationCP(ecEnum->GetName().c_str());
            EXPECT_NE(nullptr, copy);
            EXPECT_EQ(ecEnum->GetEnumeratorCount(), copy->GetEnumeratorCount());
            }
        }



    void CheckTypeName(Utf8CP typeName, ECSchemaR schema, Utf8CP propertyName, bvector<Utf8CP> classes)
        {
        for (auto className : classes)
            {
            EXPECT_EQ(typeName, schema.GetClassCP(className)->GetPropertyP(propertyName, false)->GetTypeName()) << "Property Type should have been " << typeName;
            }
        }

    void CreatEditorSchema(ECSchemaPtr& editorSchema)
        {
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        SchemaKey schemaKey("EditorCustomAttributes", 1, 0);
        editorSchema = context->LocateSchema(schemaKey, SchemaMatchType::Latest);
        }

    IECInstancePtr GetStandardValuesInstance(std::map<int, Utf8String> valueMap, ECSchemaPtr& customAttributesSchema, bool mustBedefined = false, bool mustBeFromList = true)
        {
        StandaloneECEnablerP customAttributeEnabler = customAttributesSchema->GetClassP("StandardValues")->GetDefaultStandaloneEnabler();
        StandaloneECEnablerP valueMapEnabler = customAttributesSchema->GetClassP("ValueMap")->GetDefaultStandaloneEnabler();

        StandaloneECInstancePtr sdValueAttr = customAttributeEnabler->CreateInstance();
        sdValueAttr->AddArrayElements("ValueMap", (uint32_t)valueMap.size());
        if (mustBedefined)
            sdValueAttr->SetValue("MustBeFromList", ECValue(mustBeFromList));
        int i = 0;
        for (auto const& pair : valueMap)
            {
            IECInstancePtr valueMapAttr = valueMapEnabler->CreateInstance();
            valueMapAttr->SetValue("DisplayString", ECValue(pair.second.c_str()));
            valueMapAttr->SetValue("Value", ECValue(pair.first));
            ECValue structValue;
            structValue.SetStruct(valueMapAttr.get());
            sdValueAttr->SetValue("ValueMap", structValue, i);
            i++;
            }

        IECInstancePtr instance = sdValueAttr->GetAsIECInstanceP();
        return instance;
        }
    };


//---------------------------------------------------------------------------------------
//@bsimethod                                    Basanta.Kharel                 01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, StandardValuesTest)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Trap' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "  <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "  <ECClass typeName='Name' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "    <ECProperty propertyName='Title' typeName='int' displayLabel='Title'>"
        "      <ECCustomAttributes>"
        "        <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "          <MustBeFromList>True</MustBeFromList>"
        "          <ValueMap>"
        "            <ValueMap>"
        "              <DisplayString>Mr.</DisplayString>"
        "              <Value>0</Value>"
        "            </ValueMap>"
        "            <ValueMap>"
        "              <DisplayString>Ms.</DisplayString>"
        "              <Value>1</Value>"
        "            </ValueMap>"
        "            <ValueMap>"
        "              <!--Mr. with Value 1 overrides Ms. with Value 1 -->"
        "              <DisplayString>Mr.</DisplayString>"
        "              <Value>1</Value>"
        "            </ValueMap>"
        "            <ValueMap>"
        "              <!--Ms. with Value 0 overrides Mr. with Value 0 -->"
        "              <DisplayString>Ms.</DisplayString>"
        "              <Value>0</Value>"
        "            </ValueMap>"
        "            <ValueMap>"
        "              <DisplayString>Mr.</DisplayString>"
        "              <Value>-10</Value>"
        "            </ValueMap>"
        "            <ValueMap>"
        "              <DisplayString>Ms.</DisplayString>"
        "              <Value>11</Value>"
        "            </ValueMap>"
        "            <ValueMap>"
        "              <DisplayString>Miss</DisplayString>"
        "              <Value>2</Value>"
        "            </ValueMap>"
        "            <ValueMap>"
        "              <!--Sensei with Value 2 overrides Miss -->"
        "              <DisplayString>Sensei</DisplayString>"
        "              <Value>2</Value>"
        "            </ValueMap>"
        "          </ValueMap>"
        "        </StandardValues>"
        "      </ECCustomAttributes>"
        "    </ECProperty>"
        "  </ECClass>"
        "</ECSchema>";

    ReadSchema(schemaXML);

    ASSERT_TRUE(ECSchemaConverter::Convert(*m_schema.get())) << "Failed to convert schema";
    ECEnumerationCP ecEnum;
    ASSERT_NE(nullptr, ecEnum = m_schema->GetEnumerationCP("Name_Title")) << "Failed to Create Name_Title Enum";
    EXPECT_EQ(5, ecEnum->GetEnumeratorCount());

    std::map <int, Utf8String> sdValues = { { 1, "Mr." },{ 0, "Ms." },{ -10, "Mr." },{ 11, "Ms." },{ 2, "Sensei" } };
    for (auto enumerator : ecEnum->GetEnumerators())
        {
        int i = enumerator->GetInteger();
        Utf8String displayLabel = enumerator->GetDisplayLabel();
        EXPECT_EQ(displayLabel, sdValues[i]) << "Enumrator displaylabel doesnot match StandardValue's Display String";
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Basanta.Kharel                 01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, StrictTestSimple)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Trap' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "<ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='Name' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='Title1' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <MustBeFromList>False</MustBeFromList>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Mr.</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                   </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "       <ECProperty propertyName='Title2' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <MustBeFromList>True</MustBeFromList>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Ms.</DisplayString>"
        "                           <Value>1</Value>"
        "                       </ValueMap>"
        "                   </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "       <ECProperty propertyName='Title3' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Miss</DisplayString>"
        "                           <Value>1</Value>"
        "                       </ValueMap>"
        "                   </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "</ECSchema>";

    ReadSchema(schemaXML);

    ASSERT_TRUE(ECSchemaConverter::Convert(*m_schema.get())) << "Failed to convert schema";
    ASSERT_EQ(3, m_schema->GetEnumerationCount());

    ECEnumerationCP ecEnum;
    ASSERT_NE(nullptr, ecEnum = m_schema->GetEnumerationCP("Name_Title1")) << "Enumeration Name_Title1 should have been created";
    EXPECT_EQ(1, ecEnum->GetEnumeratorCount());
    EXPECT_EQ(false, ecEnum->GetIsStrict()) << "Name_Title1 has MustBeFromList set to false so GetIsStrict() should return false";

    ASSERT_NE(nullptr, ecEnum = m_schema->GetEnumerationCP("Name_Title2")) << "Enumeration Name_Title2 should have been created";
    EXPECT_EQ(1, ecEnum->GetEnumeratorCount());
    EXPECT_EQ(true, ecEnum->GetIsStrict()) << "Name_Title2 has MustBeFromList set to true so GetIsStrict() should return true";

    ASSERT_NE(nullptr, ecEnum = m_schema->GetEnumerationCP("Name_Title3")) << "Enumeration Name_Title3 should have been created";
    EXPECT_EQ(1, ecEnum->GetEnumeratorCount());
    EXPECT_EQ(true, ecEnum->GetIsStrict()) << "Name_Title3 has not set MustBeFromList. Default is true so GetIsStrict() should return true";

    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Basanta.Kharel                 01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, StrictTestInheritence_ParentPropertyStrictnessWins)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Trap' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "<ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='Name' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>BaseName</BaseClass>"
        "       <ECProperty propertyName='Title1' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <MustBeFromList>False</MustBeFromList>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Mr.</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                   </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='BaseName' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='Title1' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <MustBeFromList>True</MustBeFromList>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Mr.</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                   </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "</ECSchema>";

    ReadSchema(schemaXML);

    ASSERT_TRUE(ECSchemaConverter::Convert(*m_schema.get())) << "Failed to convert schema";
    ASSERT_EQ(1, m_schema->GetEnumerationCount());

    ECEnumerationCP ecEnum;
    Utf8String enumName = "BaseName_Title1";
    ASSERT_NE(nullptr, ecEnum = m_schema->GetEnumerationCP(enumName.c_str())) << "Enumeration" << enumName << "should have been created";
    EXPECT_EQ(1, ecEnum->GetEnumeratorCount());
    EXPECT_EQ(true, ecEnum->GetIsStrict()) << enumName << " should be strict";

    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Basanta.Kharel                 01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, StrictTestInheritence_ParentPropertyWithNoCAMeansNotStrict)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Trap' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "<ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='Name' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>BaseName</BaseClass>"
        "       <ECProperty propertyName='Title1' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <MustBeFromList>False</MustBeFromList>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Mr.</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                   </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='BaseName' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='Title1' typeName='int' displayLabel='Title'>"
        "       </ECProperty>"
        "   </ECClass>"
        "</ECSchema>";

    ReadSchema(schemaXML);

    ASSERT_TRUE(ECSchemaConverter::Convert(*m_schema.get())) << "Failed to convert schema";
    ASSERT_EQ(1, m_schema->GetEnumerationCount());

    ECEnumerationCP ecEnum;
    Utf8String enumName = "BaseName_Title1";
    ASSERT_NE(nullptr, ecEnum = m_schema->GetEnumerationCP(enumName.c_str())) << "Enumeration " << enumName <<" should have been created";
    EXPECT_EQ(1, ecEnum->GetEnumeratorCount());
    EXPECT_EQ(false, ecEnum->GetIsStrict()) << "Title1 is derived from base class property which has no StandardValues CA so GetIsStrict() should return false";

    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Basanta.Kharel                 01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, DuplicatSDValues)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Trap' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='C' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'/>"
        "       <ECProperty propertyName='TitleB' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <MustBeFromList>False</MustBeFromList>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Sensei</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "       <ECProperty propertyName='TitleC' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <MustBeFromList>False</MustBeFromList>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Sensei</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='D' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <MustBeFromList>False</MustBeFromList>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Mr.</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Ms.</DisplayString>"
        "                           <Value>1</Value>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='A' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>C</BaseClass>"
        "       <ECProperty propertyName='TitleB' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <MustBeFromList>False</MustBeFromList>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Sensei</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='B' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>C</BaseClass>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <MustBeFromList>False</MustBeFromList>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Mr.</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Ms.</DisplayString>"
        "                           <Value>1</Value>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='E' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'/>"
        "   </ECClass>"
        "</ECSchema>";

    ReadSchema(schemaXML);

    ASSERT_TRUE(ECSchemaConverter::Convert(*m_schema.get())) << "Failed to convert schema";
    ASSERT_EQ(2, m_schema->GetEnumerationCount());

    ECEnumerationCP ecEnum;
    //with duplicate sd values only one enumeration is created as classname + propertyname

    Utf8String enumName1 = "C_TitleB";
    //within a class if different property have same enumeration, propertyB trumps propertyC if propertyB < propertyC
    ASSERT_NE(nullptr, ecEnum = m_schema->GetEnumerationCP(enumName1.c_str())) << "Enumeration " << enumName1 << " should have been created";
    EXPECT_EQ(nullptr, ecEnum = m_schema->GetEnumerationCP("C_TitleC")) << "Enumeration C_TitleC should NOT have been created";
    EXPECT_EQ(enumName1, m_schema->GetClassCP("C")->GetPropertyP("TitleC", false)->GetTypeName()) << "Enum type should have been " << enumName1;

    //with same class(including child,parent) and property name, baseclass name trumps
    EXPECT_EQ(nullptr, ecEnum = m_schema->GetEnumerationCP("A_TitleB")) << "Enumeration A_TitleB should NOT have been created";
    EXPECT_EQ(enumName1, m_schema->GetClassCP("A")->GetPropertyP("TitleB", false)->GetTypeName()) << "Enum type should have been " << enumName1;

    Utf8String enumName2 = "C_TitleA";
    CheckTypeName(enumName2.c_str(), *m_schema, "TitleA", { "D", "B", "C" });

    //with same class(including child,parent) and property name, baseclass name trumps even if base class property has no CA attached to it
    ASSERT_NE(nullptr, ecEnum = m_schema->GetEnumerationCP(enumName2.c_str())) << "Enumeration" << enumName2 << "should have been created";
    EXPECT_EQ(nullptr, ecEnum = m_schema->GetEnumerationCP("B_TitleA")) << "Enumeration B_TitleA should NOT have been created";

    //with different classes having same sd values, classA trumps classB if classA < classB 
    EXPECT_EQ(nullptr, ecEnum = m_schema->GetEnumerationCP("D_TitleA")) << "Enumeration D_TitleA should NOT have been created";

    //should not be an enum
    EXPECT_EQ("int", m_schema->GetClassCP("E")->GetPropertyP("TitleA", false)->GetTypeName()) << "Should be a regular int type";

    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Basanta.Kharel                 01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, DuplicatSDValues_MultipleSchemas_ExpectedBehavior)
    {
    Utf8CP schemaXML1 = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Trap' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='C' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='TitleC' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <MustBeFromList>False</MustBeFromList>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Sensei</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "</ECSchema>";

    Utf8CP schemaXML2 = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Trap2' version='78.00' nameSpacePrefix='tr2' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='B' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='TitleB' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <MustBeFromList>False</MustBeFromList>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Sensei</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "</ECSchema>";
    ECSchemaPtr schema1, schema2;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema1, schemaXML1, *m_readContext));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema2, schemaXML2, *m_readContext));

    ASSERT_TRUE(ECSchemaConverter::Convert(*schema1.get())) << "Failed to convert schema";
    EXPECT_EQ(1, schema1->GetEnumerationCount());

    ASSERT_TRUE(ECSchemaConverter::Convert(*schema2.get())) << "Failed to convert schema";
    EXPECT_EQ(1, schema2->GetEnumerationCount());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Basanta.Kharel                 01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, NOTDuplicatSDValues)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Trap' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "  <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "  <ECClass typeName='Name' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "    <ECProperty propertyName='Title' typeName='int' displayLabel='Title'>"
        "      <ECCustomAttributes>"
        "        <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "          <MustBeFromList>True</MustBeFromList>"
        "          <ValueMap>"
        "            <ValueMap>"
        "              <DisplayString>Mr.</DisplayString>"
        "              <Value>0</Value>"
        "            </ValueMap>"
        "            <ValueMap>"
        "              <DisplayString>Ms.</DisplayString>"
        "              <Value>1</Value>"
        "            </ValueMap>"
        "          </ValueMap>"
        "        </StandardValues>"
        "      </ECCustomAttributes>"
        "    </ECProperty>"
        "    <ECProperty propertyName='NotDuplicateDifferentMustBeFromListValue' typeName='int' displayLabel='Title'>"
        "      <ECCustomAttributes>"
        "        <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "          <MustBeFromList>False</MustBeFromList>"
        "          <ValueMap>"
        "            <ValueMap>"
        "              <DisplayString>Mr.</DisplayString>"
        "              <Value>0</Value>"
        "            </ValueMap>"
        "            <ValueMap>"
        "              <DisplayString>Ms.</DisplayString>"
        "              <Value>1</Value>"
        "            </ValueMap>"
        "          </ValueMap>"
        "        </StandardValues>"
        "      </ECCustomAttributes>"
        "    </ECProperty>"
        "    <ECProperty propertyName='NotDuplicate' typeName='int' displayLabel='Title'>"
        "      <ECCustomAttributes>"
        "        <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "          <MustBeFromList>True</MustBeFromList>"
        "          <ValueMap>"
        "            <ValueMap>"
        "              <DisplayString>Mr.</DisplayString>"
        "              <Value>10</Value>"
        "            </ValueMap>"
        "            <ValueMap>"
        "              <DisplayString>Ms.</DisplayString>"
        "              <Value>11</Value>"
        "            </ValueMap>"
        "          </ValueMap>"
        "        </StandardValues>"
        "      </ECCustomAttributes>"
        "    </ECProperty>"
        "    <ECProperty propertyName='NotDuplicateEither' typeName='int' displayLabel='Title'>"
        "      <ECCustomAttributes>"
        "        <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "          <ValueMap>"
        "            <ValueMap>"
        "              <DisplayString>Mr.</DisplayString>"
        "              <Value>0</Value>"
        "            </ValueMap>"
        "            <ValueMap>"
        "              <DisplayString>Ms.</DisplayString>"
        "              <Value>1</Value>"
        "            </ValueMap>"
        "            <ValueMap>"
        "              <DisplayString>Miss</DisplayString>"
        "              <Value>2</Value>"
        "            </ValueMap>"
        "          </ValueMap>"
        "        </StandardValues>"
        "      </ECCustomAttributes>"
        "    </ECProperty>"
        "    <ECProperty propertyName='AlsoNotDuplicate' typeName='int' displayLabel='Adds one more SD'>"
        "      <ECCustomAttributes>"
        "        <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "          <MustBeFromList>True</MustBeFromList>"
        "          <ValueMap>"
        "            <ValueMap>"
        "              <DisplayString>Mr.</DisplayString>"
        "              <Value>0</Value>"
        "            </ValueMap>"
        "            <ValueMap>"
        "              <DisplayString>Ms.</DisplayString>"
        "              <Value>1</Value>"
        "            </ValueMap>"
        "            <ValueMap>"
        "              <DisplayString>Sensei</DisplayString>"
        "              <Value>2</Value>"
        "            </ValueMap>"
        "          </ValueMap>"
        "        </StandardValues>"
        "      </ECCustomAttributes>"
        "    </ECProperty>"
        "  </ECClass>"
        "</ECSchema>";

    ReadSchema(schemaXML);
    ASSERT_TRUE(ECSchemaConverter::Convert(*m_schema.get())) << "Failed to convert schema";
    ASSERT_EQ(5, m_schema->GetEnumerationCount());

    ECEnumerationCP ecEnum;
    ASSERT_NE(nullptr, ecEnum = m_schema->GetEnumerationCP("Name_Title")) << "Failed to Create Name_Title Enum";
    EXPECT_EQ(2, ecEnum->GetEnumeratorCount());

    ASSERT_NE(nullptr, ecEnum = m_schema->GetEnumerationCP("Name_NotDuplicateDifferentMustBeFromListValue")) << "Failed to Create Name_NotDuplicateDifferentMustBeFromListValue Enum";
    EXPECT_EQ(2, ecEnum->GetEnumeratorCount());

    ASSERT_NE(nullptr, ecEnum = m_schema->GetEnumerationCP("Name_NotDuplicate")) << "Failed to Create Name_NotDuplicate Enum";
    EXPECT_EQ(2, ecEnum->GetEnumeratorCount());

    ASSERT_NE(nullptr, ecEnum = m_schema->GetEnumerationCP("Name_NotDuplicateEither")) << "Failed to Create Name_NotDuplicateEither Enum";
    EXPECT_EQ(3, ecEnum->GetEnumeratorCount());

    ASSERT_NE(nullptr, ecEnum = m_schema->GetEnumerationCP("Name_AlsoNotDuplicate")) << "Failed to Create Name_AlsoNotDuplicate Enum";
    EXPECT_EQ(3, ecEnum->GetEnumeratorCount());

    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Basanta.Kharel                 01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, Strict_Duplicate_Inherited_Combo_ExpectTheUnexpected1)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Trap' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "<ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='A' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='Title1' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <MustBeFromList>True</MustBeFromList>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Mr.</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                   </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='Name' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>BaseName</BaseClass>"
        "       <ECProperty propertyName='Title1' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <MustBeFromList>True</MustBeFromList>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Mr.</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                   </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='BaseName' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='Title1' typeName='int' displayLabel='Title'>"
        "       </ECProperty>"
        "   </ECClass>"
        "</ECSchema>";

    ReadSchema(schemaXML);

    ASSERT_TRUE(ECSchemaConverter::Convert(*m_schema.get())) << "Failed to convert schema";
    //event though the two values are same it still creates two enums.
    //Name inherits from basename, which has no CA, so mustbelist is false
    ASSERT_EQ(2, m_schema->GetEnumerationCount());

    ECEnumerationCP ecEnum;
    Utf8String enumName = "BaseName_Title1";
    ASSERT_NE(nullptr, ecEnum = m_schema->GetEnumerationCP(enumName.c_str())) << "Enumeration " << enumName << " should have been created";
    EXPECT_EQ(1, ecEnum->GetEnumeratorCount());
    EXPECT_EQ(false, ecEnum->GetIsStrict()) << "Title1 is derived from base class property which has no StandardValues CA so GetIsStrict() should return false";

    ASSERT_NE(nullptr, ecEnum = m_schema->GetEnumerationCP("A_Title1")) << "Enumeration A_Title1 should have been created";
    EXPECT_EQ(1, ecEnum->GetEnumeratorCount());
    EXPECT_EQ(true, ecEnum->GetIsStrict()) << "A.Title1 defines mustbefromlist true, so GetIsStrict() should return true";

    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Basanta.Kharel                 01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, Strict_Duplicate_Inherited_Combo_ExpectTheUnexpected2)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Trap' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "<ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='A' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='Title1' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <MustBeFromList>False</MustBeFromList>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Mr.</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                   </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='Name' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>BaseName</BaseClass>"
        "       <ECProperty propertyName='Title1' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <MustBeFromList>True</MustBeFromList>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Mr.</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                   </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='BaseName' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='Title1' typeName='int' displayLabel='Title'>"
        "       </ECProperty>"
        "   </ECClass>"
        "</ECSchema>";

    ReadSchema(schemaXML);

    ASSERT_TRUE(ECSchemaConverter::Convert(*m_schema.get())) << "Failed to convert schema";
    //even though the two values are different, as mustbelist values are different
    //Name inherits from basename, which has no ca, making mustbelist false
    ASSERT_EQ(1, m_schema->GetEnumerationCount());

    ECEnumerationCP ecEnum;
    Utf8String enumName = "A_Title1";
    ASSERT_NE(nullptr, ecEnum = m_schema->GetEnumerationCP(enumName.c_str())) << "Enumeration " << enumName << " should have been created";
    EXPECT_EQ(1, ecEnum->GetEnumeratorCount());
    EXPECT_EQ(false, ecEnum->GetIsStrict()) << "Title1 is derived from base class property which has no StandardValues CA so GetIsStrict() should return false";
    
    }


//---------------------------------------------------------------------------------------
//@bsimethod                                    Basanta.Kharel                 01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, MultipleInheritedSDValues_ConversionSucess)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Trap' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='C' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='A' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>B</BaseClass>"
        "       <BaseClass>C</BaseClass>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Sensei</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='B' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "       </ECProperty>"
        "   </ECClass>"
        "</ECSchema>";

    ReadSchema(schemaXML);
    EXPECT_TRUE(ECSchemaConverter::Convert(*m_schema.get())) << "Schema conversion failed";

    ECEnumerationCP ecEnum;
    Utf8String enumName = "B_TitleA";
    CheckTypeName(enumName.c_str(), *m_schema, "TitleA", { "A", "B", "C" });

    ASSERT_NE(nullptr, ecEnum = m_schema->GetEnumerationCP(enumName.c_str())) << "Enumeration " << enumName << " should have been created";
    EXPECT_EQ(1, ecEnum->GetEnumeratorCount());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Basanta.Kharel                 01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, MultipleInheritedSDValues_ConversionWarnings)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Trap' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='C' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='A' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>B</BaseClass>"
        "       <BaseClass>C</BaseClass>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='B' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "       </ECProperty>"
        "   </ECClass>"
        "</ECSchema>";

    ReadSchema(schemaXML);

    bvector<ECClassP> classes = ECSchemaConverter::GetHierarchicallySortedClasses(*m_schema);
    ECSchemaPtr customAttributesSchema;
    CreatEditorSchema(customAttributesSchema);

    IECInstancePtr instance1 = GetStandardValuesInstance({ {0, "Mr."} }, customAttributesSchema);
    IECInstancePtr instance2 = GetStandardValuesInstance({ { 1, "Sensei" } }, customAttributesSchema);
    //C and B are parent of A
    //covers combination of having same CA for two but differnet for the third i.e C, B have instance1 and A instance2
    for (int i = 0; i < 3; i++)
        {
        int j = (i + 1) % 3;
        int k = (i + 2) % 3;
        classes[i]->GetPropertyP("TitleA", false)->SetCustomAttribute(*instance1);
        classes[j]->GetPropertyP("TitleA", false)->SetCustomAttribute(*instance1);
        classes[k]->GetPropertyP("TitleA", false)->SetCustomAttribute(*instance2);
        EXPECT_TRUE(ECSchemaConverter::Convert(*m_schema.get())) << "Schema conversion should have passed with warnings";
        CheckTypeName("int", *m_schema, "TitleA", { "A", "B", "C" });
        EXPECT_EQ(0, m_schema->GetEnumerationCount()) << "Conversion should not have created any enums";
        ValidateSchema(*m_schema);
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Basanta.Kharel                 01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, InheritedSDValues_ConversionSucess)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Trap' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='C' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'/>"
        "   </ECClass>"
        "   <ECClass typeName='A' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>B</BaseClass>"
        "       <ECProperty propertyName = 'TitleA' typeName = 'int' displayLabel = 'Title'/>"
        "   </ECClass>"
        "   <ECClass typeName='B' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>C</BaseClass>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Mr.</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Ms.</DisplayString>"
        "                           <Value>1</Value>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "</ECSchema>";

    ReadSchema(schemaXML);
    ASSERT_TRUE(ECSchemaConverter::Convert(*m_schema.get())) << "Failed to convert schema";
    ASSERT_EQ(1, m_schema->GetEnumerationCount());

    Utf8String enumName = "C_TitleA";
    CheckTypeName(enumName.c_str(), *m_schema, "TitleA", { "B","C","A" });
    ASSERT_NE(nullptr, m_schema->GetEnumerationCP(enumName.c_str())) << "Enumeration " << enumName << "should have been created";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Basanta.Kharel                 01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, InheritedSDValues_ConversionWithWarning)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Trap' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='A' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>B</BaseClass>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Mr.</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='B' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Sensei</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "</ECSchema>";

    ECSchemaPtr schema1;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema1, schemaXML, *m_readContext));

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema1.get())) << "Schema conversion should have passed with warnings";
    CheckTypeName("int", *schema1, "TitleA", { "A", "B" });
    EXPECT_EQ(0, schema1->GetEnumerationCount()) << "Conversion should not have created any enums";
    ValidateSchema(*schema1);

    Utf8CP schemaXML2 = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='TrapB' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='C' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>B</BaseClass>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='A' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>C</BaseClass>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Mr.</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='B' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Sensei</DisplayString>"
        "                           <Value>1</Value>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "</ECSchema>";

    ECSchemaPtr schema2;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema2, schemaXML2, *m_readContext));

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema2.get())) << "Schema conversion should have passed with warnings";
    CheckTypeName("int", *schema2, "TitleA", { "A", "B", "C" });
    EXPECT_EQ(0, schema2->GetEnumerationCount()) << "Conversion should not have created any enums";
    ValidateSchema(*schema2);

    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Basanta.Kharel                 01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, InheritedSDValues_ConversionWithWarning_MultipleSchemas)
    {
    Utf8CP schemaXMLRef = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='TrapRef' version='78.00' nameSpacePrefix='trRef' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='D' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Mr.</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "</ECSchema>";

    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='TrapB' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECSchemaReference name='TrapRef' version='78.00' prefix='trRef' />"
        "   <ECClass typeName='C' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>B</BaseClass>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='A' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>C</BaseClass>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Mr.</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='B' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>trRef:D</BaseClass>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Sensei</DisplayString>"
        "                           <Value>1</Value>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "</ECSchema>";

    ReadSchemaWithRef(schemaXMLRef, schemaXML);
   
    EXPECT_TRUE(ECSchemaConverter::Convert(*m_schema.get())) << "Schema conversion should have passed with warnings";
    CheckTypeName("int", *m_schema, "TitleA", { "A", "B", "C" });
    CheckTypeName("int", *m_refSchema, "TitleA", { "D" });
    EXPECT_EQ(0, m_schema->GetEnumerationCount()) << "Conversion should not have created any enums";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Basanta.Kharel                 01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, InheritedSDValues_ConversionSucess_MultipleSchemas_DuplicateEnumerationsExpected)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Trap' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECSchemaReference name='TrapRef' version='78.00' prefix='trRef' />"
        "   <ECClass typeName='C' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>trRef:D</BaseClass>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'/>"
        "   </ECClass>"
        "   <ECClass typeName='A' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Sensei</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "</ECSchema>";

    Utf8CP schemaXMLRef = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='TrapRef' version='78.00' nameSpacePrefix='trRef' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='D' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Sensei</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "</ECSchema>";

    ReadSchemaWithRef(schemaXMLRef, schemaXML);
    EXPECT_TRUE(ECSchemaConverter::Convert(*m_refSchema.get())) << "Schema conversion failed";
    EXPECT_TRUE(ECSchemaConverter::Convert(*m_schema.get())) << "Schema conversion failed";

    CheckTypeName("trRef:D_TitleA", *m_schema, "TitleA", { "C" });
    CheckTypeName("D_TitleA", *m_refSchema, "TitleA", { "D" });
    //even thought sd value is same as trRef:DTitleA since it is in different schema it creates a new enum
    CheckTypeName("A_TitleA", *m_schema, "TitleA", { "A" });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Basanta.Kharel                 01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, InheritedSDValues_ConversionSucess_MultipleSchemas_Simple)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Trap' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECSchemaReference name='TrapRef' version='78.00' prefix='trRef' />"
        "   <ECClass typeName='A' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>trRef:D</BaseClass>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Sensei</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "</ECSchema>";

    Utf8CP schemaXMLRef = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='TrapRef' version='78.00' nameSpacePrefix='trRef' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='D' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Sensei</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "</ECSchema>";

    ReadSchemaWithRef(schemaXMLRef, schemaXML);

    EXPECT_TRUE(ECSchemaConverter::Convert(*m_schema.get())) << "Schema conversion failed";

    CheckTypeName("trRef:D_TitleA", *m_schema, "TitleA", { "A" });
    CheckTypeName("D_TitleA", *m_refSchema, "TitleA", { "D" });
    EXPECT_EQ(1, m_refSchema->GetEnumerationCount()) << "Enumeration should have been created in refschema";
    EXPECT_EQ(0, m_schema->GetEnumerationCount()) << "Enumeration should not have been created in schema";
    }
//---------------------------------------------------------------------------------------
//@bsimethod                                    Basanta.Kharel                 01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, SortedClasses)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Trap' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='C' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "   </ECClass>"
        "   <ECClass typeName='A' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>B</BaseClass>"
        "       <BaseClass>C</BaseClass>"
        "   </ECClass>"
        "   <ECClass typeName='B' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "   </ECClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXML, *m_readContext));
    bvector<ECClassP> classes = ECSchemaConverter::GetHierarchicallySortedClasses(*schema);
    bvector<Utf8String> order = { "B", "C", "A" };
    for (size_t i = 0; i < order.size(); i++)
        EXPECT_EQ(order[i], classes[i]->GetName()) << "Class Order is not Hierarcical";

    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Basanta.Kharel                 01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, IsBaseClassTest)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Trap' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='D' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "   </ECClass>"
        "   <ECClass typeName='C' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>D</BaseClass>"
        "   </ECClass>"
        "   <ECClass typeName='Aa' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>B</BaseClass>"
        "       <BaseClass>C</BaseClass>"
        "   </ECClass>"
        "   <ECClass typeName='Ab' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECClass>"
        "   <ECClass typeName='B' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>C</BaseClass>"
        "       <BaseClass>Ba</BaseClass>"
        "   </ECClass>"
        "   <ECClass typeName='Ba' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "   </ECClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXML, *m_readContext));

    Utf8CP schemaXML2 = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Trap2' version='78.00' nameSpacePrefix='tr2' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='D' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "   </ECClass>"
        "</ECSchema>";

    ECSchemaPtr schema2;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema2, schemaXML2, *m_readContext));
    EXPECT_FALSE(ECSchemaConverter::IsBaseClass(schema->GetClassP("Aa"), schema2->GetClassP("D")))<< "Class D of Trap2 Schema is not  base class of Aa";
    EXPECT_TRUE(ECSchemaConverter::IsBaseClass(schema->GetClassP("Aa"), schema->GetClassP("D"))) << "Class D of Trap Schema should be a base class of Aa";
    EXPECT_TRUE(ECSchemaConverter::IsBaseClass(schema->GetClassP("Aa"), schema->GetClassP("B"))) << "Class B of Trap Schema should be base class of Aa";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Basanta.Kharel                 01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, EnumName_NamingConvention)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='TrapRef' version='78.00' nameSpacePrefix='trRef' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECEnumeration typeName='Class_PropertyA' backingTypeName='int' isStrict='true'>"
        "        <ECEnumerator value='0' displayLabel='Sensei'/>"
        "   </ECEnumeration>"
        "   <ECEnumeration typeName='Class_PropertyB' backingTypeName='int' isStrict='true'>"
        "        <ECEnumerator value='0' displayLabel='Sensei'/>"
        "   </ECEnumeration>"
        "   <ECEnumeration typeName='Class_PropertyB_1' backingTypeName='int' isStrict='true'>"
        "        <ECEnumerator value='0' displayLabel='Sensei'/>"
        "   </ECEnumeration>"
        "   <ECClass typeName='Computer' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='ComputerType' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Wakata</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='Monitor' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='Type' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Akinamiro</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='Class' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='PropertyA' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Arigato</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "       <ECProperty propertyName='PropertyB' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>What</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "</ECSchema>";

    ReadSchema(schemaXML);

    EXPECT_TRUE(ECSchemaConverter::Convert(*m_schema.get())) << "Schema conversion failed";

    ASSERT_EQ(7, m_schema->GetEnumerationCount());
    //enum name is combined of className and property name with _ in between
    CheckTypeName("Monitor_Type", *m_schema, "Type", { "Monitor" });
    CheckTypeName("Computer_ComputerType", *m_schema, "ComputerType", { "Computer" });

    //if enumName exists a number is appended starting with 1 
    CheckTypeName("Class_PropertyA_1", *m_schema, "PropertyA", { "Class" });
    //increment number until enumName is not in schema
    CheckTypeName("Class_PropertyB_2", *m_schema, "PropertyB", { "Class" });

    }
//---------------------------------------------------------------------------------------
//@bsimethod                                    Andreas.Kurka                 04 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, SomeTest)
{
	ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
	ECSchemaPtr schema;
	SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"CAConversionTestSchema.01.00.ecschema.xml").c_str(), *schemaContext);
	ASSERT_EQ(SchemaReadStatus::Success, status );

	bool result = ECSchemaConverter::Convert(*schema);
	ASSERT_EQ(true, result);

	SchemaWriteStatus status2 = schema->WriteToXmlFile(ECTestFixture::GetTempDataPath(L"CAConversionTestSchema.01.00.ecschema-out.xml").c_str());
	ASSERT_EQ(SchemaWriteStatus::Success, status2);
}

END_BENTLEY_ECN_TEST_NAMESPACE