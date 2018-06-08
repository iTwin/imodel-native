/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/CustomAttributeConversionsTest.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include <Bentley/BeTest.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct RelationshipConversionTest : ECTestFixture {};
struct ECDbMappingConversionTests :ECTestFixture {};

struct PropertyPriorityCustomAttributeConversionTest : ECTestFixture
    {
    ECSchemaPtr m_becaSchema;

    void SetUp() override
        {
        ECTestFixture::SetUp();
        ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
        SchemaKey key("EditorCustomAttributes", 1, 3);
        m_becaSchema = ECSchema::LocateSchema(key, *schemaContext);
        ASSERT_TRUE(m_becaSchema.IsValid());
        }

    ECClassCP GetPropertyPriorityClass() const
        {
        if (!m_becaSchema.IsValid())
            return nullptr;
        return m_becaSchema->GetClassCP("PropertyPriority");
        }

    void CheckForPropertyPriorityCALocally(ECPropertyCP prop, bool shouldCAExist)
        {
        EXPECT_EQ(shouldCAExist, prop->IsDefinedLocal(*GetPropertyPriorityClass())) << "The property " << prop->GetClass().GetFullName() << "." << prop->GetName().c_str() << " didn't validate properly.";
        }

    void CheckForPropertyPriorityCA(ECPropertyCP prop, bool shouldCAExist)
        {
        auto caInstance = prop->GetCustomAttribute(*GetPropertyPriorityClass());
        EXPECT_EQ(shouldCAExist, caInstance.IsValid()) << "The property " << prop->GetClass().GetFullName() << "." << prop->GetName().c_str() << " didn't validate properly.";
        }
    };

struct StandardCustomAttributeConversionTests : ECTestFixture 
    {
    ECSchemaPtr m_coreCASchema;
    ECSchemaPtr m_bscaSchema;

    Utf8String GetDateTimeInfoValue(IECInstancePtr instancePtr, Utf8CP name);

    void SetUp() override
        {
        ECTestFixture::SetUp();

        ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

        SchemaKey key("Bentley_Standard_CustomAttributes", 1, 6);
        m_bscaSchema = ECSchema::LocateSchema(key, *schemaContext);
        ASSERT_TRUE(m_bscaSchema.IsValid());

        SchemaKey coreCAKey("CoreCustomAttributes", 1, 0, 0);
        m_coreCASchema = ECSchema::LocateSchema(coreCAKey, *schemaContext);
        ASSERT_TRUE(m_coreCASchema.IsValid());
        }
    };

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct CustomAttributeRemovalTest : ECTestFixture
    {
    ECSchemaReadContextPtr m_readContext;
    Utf8String m_customAttributeName;
    Utf8String m_customAttributeSchemaName;
    ECSchemaPtr m_schema;
    ECSchemaPtr m_refSchema;

    void SetUp() override
        {
        ECTestFixture::SetUp();
        m_readContext = ECSchemaReadContext::CreateContext();
        }

    CustomAttributeRemovalTest(Utf8String schemaName, Utf8String customAttributeName)
        :m_customAttributeName(customAttributeName), m_customAttributeSchemaName(schemaName){}

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

    bool HasCustomAttribute(ECSchemaR schema, Utf8StringCR schemaName, Utf8StringCR customAttributeName)
        {
        for (auto ecClass : schema.GetClasses())
            {
            for (auto ecProp : ecClass->GetProperties(false))
                {
                auto instance = ecProp->GetPrimaryCustomAttributeLocal(schemaName, customAttributeName);
                if (instance.IsValid())
                    return true;
                }
            }
        return false;
        }

    void ValidateSchema(ECSchemaR schema, bool useFreshReadContext = true)
        {
        EXPECT_FALSE(HasCustomAttribute(*m_schema, m_customAttributeSchemaName, m_customAttributeName)) << "Schema still has " << m_customAttributeSchemaName << "." << m_customAttributeName << "CustomAttribute !!";
        }
    };

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct StandardValueToEnumConversionTest : CustomAttributeRemovalTest
    {
    ECSchemaReadContextPtr   m_validationReadContext;

    void SetUp() override
        {
        CustomAttributeRemovalTest::SetUp();
        m_validationReadContext = ECSchemaReadContext::CreateContext();
        }

    StandardValueToEnumConversionTest()
        :CustomAttributeRemovalTest("EditorCustomAttributes", "StandardValues") {}

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
        EXPECT_EQ(SchemaWriteStatus::Success, schema.WriteToXmlString(out, ECVersion::Latest));

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

    void ValidateConversion(ECSchemaR origSchema, ECSchemaR convSchema)
        {
        for (auto const& origClass : origSchema.GetClasses())
            {
            ECClassP convClass = convSchema.GetClassP(origClass->GetName().c_str());
            EXPECT_NE(nullptr, convClass) << "The ECClass, " << origClass->GetFullName() << ", does not exist in the converted schema.";
            if (convClass == nullptr)
                continue;

            for (auto const& origProp : origClass->GetProperties(false))
                {
                auto customAttr = origProp->GetCustomAttributeLocal("EditorCustomAttributes", "StandardValues");
                if (!customAttr.IsValid())
                    continue;

                ECPropertyP convProp = convClass->GetPropertyP(origProp->GetName(), false);

                // Check for failed conversion
                auto convCustomAttr = convProp->GetCustomAttributeLocal("EditorCustomAttributes", "StandardValues");
                EXPECT_FALSE(convCustomAttr.IsValid()) << "The standard values for ECProperty, " << convProp->GetName() << ", were not converted." ;
                if (convCustomAttr.IsValid())
                    continue;

                // Extract Standard Value information from ECInstance
                bool mustBeFromList = true;
                bmap<int, Utf8String> valuesMap;

                ECValue value;
                if (ECObjectsStatus::Success == customAttr->GetValue(value, "MustBeFromList")
                    && !value.IsNull() && value.IsBoolean())
                    {
                    mustBeFromList = value.GetBoolean();
                    }

                Utf8String accessString = "ValueMap";
                ECObjectsStatus status;
                status = customAttr->GetValue(value, accessString.c_str());
                EXPECT_EQ(ECObjectsStatus::Success, status) << "No ValueMap found in " << origClass->GetFullName() << "." << origProp->GetName() << "'s StandardValues Custom Attribute.";

                uint32_t arraySize = value.GetArrayInfo().GetCount();
                for (uint32_t i = 0; i < arraySize; i++)
                    {
                    status = customAttr->GetValue(value, accessString.c_str(), i);
                    EXPECT_EQ(ECObjectsStatus::Success, status);
                    EXPECT_TRUE(value.IsStruct());

                    IECInstancePtr  structInstance = value.GetStruct();
                    EXPECT_TRUE(structInstance.IsValid());

                    status = structInstance->GetValue(value, "Value");
                    EXPECT_EQ(ECObjectsStatus::Success, status);
                    int index = value.GetInteger();

                    status = structInstance->GetValue(value, "DisplayString");
                    EXPECT_EQ(ECObjectsStatus::Success, status);
                    valuesMap[index] = value.ToString();
                    }
                      
                // Look for the original Standard Value in the converted schema enumerations
                bool foundEnum = false;
                for (auto ecEnum : convSchema.GetEnumerations())
                    {
                    if (mustBeFromList != ecEnum->GetIsStrict())
                        continue;

                    // Must have the exact number of elements if mustBeFromList is true
                    if (mustBeFromList && valuesMap.size() != ecEnum->GetEnumeratorCount())
                        continue; 

                    // Now we assume the enum exists until proven otherwise
                    foundEnum = true;
                    for (auto const& pair : valuesMap)
                        {
                        ECEnumeratorP enumerator = ecEnum->FindEnumerator(pair.first);
                        if (enumerator == nullptr || enumerator->GetDisplayLabel() != pair.second)
                            {
                            foundEnum = false;
                            break;
                            }
                        }

                    if (foundEnum)
                        break;
                    }

                EXPECT_TRUE(foundEnum) << "No Enumeration found in converted schema for the Standard Values in the ECProperty, " << origClass->GetFullName() << "." << origProp->GetName() << ".";
                }
            }
        }

    void CheckTypeName(Utf8CP typeName, ECSchemaR schema, Utf8CP propertyName, bvector<Utf8CP> classes)
        {
        for (auto className : classes)
            {
            Utf8String propertyTypeName = schema.GetClassCP(className)->GetPropertyP(propertyName, false)->GetTypeName().c_str();
            EXPECT_STREQ(typeName, propertyTypeName.c_str()) << "Property Type for property " << className << "." << propertyName << " should have been " << typeName;
            }
        }

    void CreateEditorSchema(ECSchemaPtr& editorSchema)
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
        EXPECT_EQ(displayLabel, sdValues[i]) << "Enumerator displaylabel does not match StandardValue's Display String";
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
    EXPECT_EQ(true, ecEnum->GetIsStrict()) << "Name_Title2 has MustBeFromList set to true so Enumeration should be strict";

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
    ASSERT_NE(nullptr, ecEnum = m_schema->GetEnumerationCP(enumName.c_str())) << "Enumeration " << enumName << " should have been created";
    EXPECT_EQ(1, ecEnum->GetEnumeratorCount());
    EXPECT_EQ(false, ecEnum->GetIsStrict()) << "Title1 is derived from base class property which has no StandardValues CA so GetIsStrict() should return false";

    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Basanta.Kharel                 01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, DuplicateSDValues)
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
TEST_F(StandardValueToEnumConversionTest, NOTDuplicateSDValues)
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
    //even though the two values are same it still creates two enums.
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
TEST_F(StandardValueToEnumConversionTest, MultipleInheritedSDValues_Supplemental_ConversionSuccess)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Trap' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECClass typeName='C' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='A' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>B</BaseClass>"
        "       <BaseClass>C</BaseClass>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title' />"
        "   </ECClass>"
        "   <ECClass typeName='B' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title' />"
        "   </ECClass>"
        "</ECSchema>";
    Utf8CP supSchemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Trap_Supplemental_StandardValues' version='78.00' nameSpacePrefix='tr_sv' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.13' prefix='beca' />"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECCustomAttributes>"
        "       <SupplementalSchemaMetaData xmlns='Bentley_Standard_CustomAttributes.01.05'>"
        "           <PrimarySchemaName>Trap</PrimarySchemaName>"
        "           <PrimarySchemaMajorVersion>78</PrimarySchemaMajorVersion>"
        "           <PrimarySchemaMinorVersion>0</PrimarySchemaMinorVersion>"
        "           <Precedence>400</Precedence>"
        "       </SupplementalSchemaMetaData>"
        "   </ECCustomAttributes>"
        "   <ECClass typeName='A' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='TitleA' typeName='string' displayLabel='Title'>"
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

    ReadSchema(schemaXML);
    ECSchemaPtr supSchema;
    ECSchema::ReadFromXmlString(supSchema, supSchemaXml, *m_readContext);
    bvector<ECSchemaP> supSchemas;
    supSchemas.push_back(supSchema.get());
    SupplementedSchemaBuilder builder;
    ASSERT_EQ(SupplementedSchemaStatus::Success, builder.UpdateSchema(*m_schema.get(), supSchemas, true)) << "Failed to supplement schema";
    EXPECT_TRUE(ECSchemaConverter::Convert(*m_schema.get())) << "Schema conversion failed";

    ECEnumerationCP ecEnum;
    Utf8String enumName = "B_TitleA";
    CheckTypeName(enumName.c_str(), *m_schema, "TitleA", { "A", "B", "C" });

    ASSERT_NE(nullptr, ecEnum = m_schema->GetEnumerationCP(enumName.c_str())) << "Enumeration " << enumName << " should have been created";
    EXPECT_EQ(1, ecEnum->GetEnumeratorCount());

    ASSERT_FALSE(m_schema->GetClassP("A")->GetPropertyP("TitleA")->GetCustomAttribute("StandardValues").IsValid()) << "Failed to remove the StandardValues CA";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Basanta.Kharel                 01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, MultipleInheritedSDValues_ConversionSuccess)
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
    CheckTypeName(enumName.c_str(), *m_schema, "TitleA", {"A", "B", "C"});

    ASSERT_NE(nullptr, ecEnum = m_schema->GetEnumerationCP(enumName.c_str())) << "Enumeration " << enumName << " should have been created";
    EXPECT_EQ(1, ecEnum->GetEnumeratorCount());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Basanta.Kharel                 01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, MultipleInheritedSDValues_ConversionSuccess2)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Trap' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='C' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "       %s"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='A' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>B</BaseClass>"
        "       <BaseClass>C</BaseClass>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "       %s"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='B' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "       %s"
        "       </ECProperty>"
        "   </ECClass>"
        "</ECSchema>";

    Utf8CP instance1Xml = "<ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <MustBeFromList>false</MustBeFromList>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Mr.</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                   </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>";

    Utf8CP instance2Xml = "<ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <MustBeFromList>false</MustBeFromList>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Sensei</DisplayString>"
        "                           <Value>1</Value>"
        "                       </ValueMap>"
        "                   </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>";

    //C and B are parent of A
    //covers combination of having same CA for two but differnet for the third i.e C, B have instance1 and A instance2
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    Utf8PrintfString schema1Xml(schemaXml, instance1Xml, instance1Xml, instance2Xml);
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schema1Xml.c_str(), *context));

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema.get())) << "Schema conversion should have succeeded";
    CheckTypeName("B_TitleA", *schema, "TitleA", {"A", "B", "C"});
    EXPECT_EQ(1, schema->GetEnumerationCount()) << "Conversion should have created 1 enum";
    ValidateSchema(*schema);

    ECSchemaPtr schema2;
    context = ECSchemaReadContext::CreateContext();
    Utf8PrintfString schema2Xml(schemaXml, instance2Xml, instance1Xml, instance1Xml);
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema2, schema2Xml.c_str(), *context));

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema2.get())) << "Schema conversion should have succeeded";
    CheckTypeName("B_TitleA", *schema2, "TitleA", {"A", "B", "C"});
    EXPECT_EQ(1, schema2->GetEnumerationCount()) << "Conversion should have created 1 enum";
    ValidateSchema(*schema2);

    ECSchemaPtr schema3;
    context = ECSchemaReadContext::CreateContext();
    Utf8PrintfString schema3Xml(schemaXml, instance1Xml, instance2Xml, instance1Xml);
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema3, schema3Xml.c_str(), *context));

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema3.get())) << "Schema conversion should have succeeded";
    CheckTypeName("B_TitleA", *schema3, "TitleA", {"A", "B", "C"});
    EXPECT_EQ(1, schema3->GetEnumerationCount()) << "Conversion should have created 1 enum";
    ValidateSchema(*schema3);

    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                     09/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, MultipleInheritedSDValues_ConversionFailure)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Trap' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='C' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "       %s"      
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='A' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>B</BaseClass>"
        "       <BaseClass>C</BaseClass>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "       %s"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='B' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "       %s"
        "       </ECProperty>"
        "   </ECClass>"
        "</ECSchema>";

    Utf8CP instance1Xml = "<ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <MustBeFromList>true</MustBeFromList>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Mr.</DisplayString>"
        "                           <Value>0</Value>"
        "                       </ValueMap>"
        "                   </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>";

    Utf8CP instance2Xml = "<ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <MustBeFromList>false</MustBeFromList>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <DisplayString>Sensei</DisplayString>"
        "                           <Value>1</Value>"
        "                       </ValueMap>"
        "                   </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>";

    Utf8PrintfString schema1Xml(schemaXml, instance1Xml, instance2Xml, instance1Xml);
    // B and C should have the strict Std Value which will cause the derived properties to be renamed 
    // B and A will have different Std Values but since B is strict it won't be able to convert its derived class, A, to the enum
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schema1Xml.c_str(), *context));
    
    EXPECT_TRUE(ECSchemaConverter::Convert(*schema.get())) << "Schema conversion should have succeeded by renaming the properties";
    CheckTypeName("B_TitleA", *schema, "TitleA", {"B","C"});
    EXPECT_EQ(2, schema->GetEnumerationCount()) << "Conversion should have created two enums";
    
    // C and A have the same SD Value but since B is not strict and A can't be strict since it's a base property they should create an enum
    // It will try to convert C to the enum and fail, so C then attempts to creates its own enum but also fails and reverts to int
    ECSchemaPtr schema2;
    context = ECSchemaReadContext::CreateContext();
    Utf8PrintfString schema2Xml(schemaXml, instance1Xml, instance1Xml, instance2Xml);
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema2, schema2Xml.c_str(), *context));

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema2.get())) << "Schema conversion should have succeeded by renaming the properties";
    EXPECT_EQ(2, schema2->GetEnumerationCount()) << "Conversion should have created two enums";
    CheckTypeName("A_tr_TitleA_", *schema2, "tr_TitleA_", {"A"});
    CheckTypeName("A_tr_TitleA_", *schema2, "TitleA", {"C"});
    CheckTypeName("B_TitleA", *schema2, "TitleA", {"B"});

    ECSchemaPtr schema3;
    Utf8PrintfString schema3Xml(schemaXml, instance2Xml, instance1Xml, instance1Xml);
    context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema3, schema3Xml.c_str(), *context));

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema3.get())) << "Schema conversion should have succeeded";
    EXPECT_EQ(2, schema3->GetEnumerationCount()) << "Conversion should have created two enums";
    CheckTypeName("A_tr_TitleA_", *schema2, "tr_TitleA_", {"A"});
    CheckTypeName("A_tr_TitleA_", *schema2, "TitleA", {"C"});
    CheckTypeName("B_TitleA", *schema2, "TitleA", {"B"});
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Basanta.Kharel                 01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, InheritedSDValues_ConversionSuccess)
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

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema1.get())) << "Schema conversion should not have failed";
    CheckTypeName("A_tr_TitleA_", *schema1, "tr_TitleA_", {"A"});
    CheckTypeName("B_TitleA", *schema1, "TitleA", {"B"});
    EXPECT_EQ(2, schema1->GetEnumerationCount()) << "Conversion should have created two enums";
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
        "                  <MustBeFromList>False</MustBeFromList>"
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
        "                  <MustBeFromList>False</MustBeFromList>"
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

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema2.get())) << "Schema conversion should have passed";
    CheckTypeName("B_TitleA", *schema2, "TitleA", { "A", "B", "C" });
    EXPECT_EQ(1, schema2->GetEnumerationCount()) << "Conversion should have created one enum";
    ValidateSchema(*schema2);

    Utf8CP schemaXML3 = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='TrapC' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
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

    ECSchemaPtr schema3;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema3, schemaXML3, *m_readContext));

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema3.get())) << "Schema conversion should have failed to convert";
    CheckTypeName("A_tr_TitleA_", *schema1, "tr_TitleA_", {"A"});
    CheckTypeName("B_TitleA", *schema3, "TitleA", {"B", "C"});
    EXPECT_EQ(2, schema3->GetEnumerationCount()) << "Conversion should have created two enums";
    ValidateSchema(*schema3);

    Utf8CP schemaXML4 = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='TrapD' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='C' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <BaseClass>B</BaseClass>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <MustBeFromList>False</MustBeFromList>"
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
        "   <ECClass typeName='A' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
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

    ECSchemaPtr schema4;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema4, schemaXML4, *m_readContext));

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema4.get())) << "Schema conversion failed";
    CheckTypeName("B_TitleA", *schema4, "TitleA", {"A", "B", "C"});
    EXPECT_EQ(1, schema4->GetEnumerationCount()) << "Conversion should have created 1 enum";
    ValidateSchema(*schema4);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Basanta.Kharel                 01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, InheritedSDValues_Conversion_MultipleSchemas)
    {
    Utf8CP schemaXMLRef = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='TrapRef' version='78.00' nameSpacePrefix='trRef' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='D' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                   <MustBeFromList>False</MustBeFromList>"
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
        "                   <MustBeFromList>False</MustBeFromList>"
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
        "                   <MustBeFromList>False</MustBeFromList>"
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

    EXPECT_TRUE(ECSchemaConverter::Convert(*m_schema.get())) << "Schema conversion should have passed.";
    CheckTypeName("trRef:D_TitleA", *m_schema, "TitleA", { "A", "B", "C" });
    CheckTypeName("D_TitleA", *m_refSchema, "TitleA", { "D" });
    EXPECT_EQ(0, m_schema->GetEnumerationCount()) << "Conversion should not have created any enums in the schema";
    EXPECT_EQ(1, m_refSchema->GetEnumerationCount()) << "Conversion should have created an enum in the reference schema";
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
    EXPECT_EQ(0, m_schema->GetEnumerationCount()) << "Enumeration should have been created in refschema";
    EXPECT_EQ(1, m_refSchema->GetEnumerationCount()) << "Enumeration should not have been created in schema";
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
//@bsimethod                                    Caleb.Shafer                    12/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, PrimitiveArraySupport)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='test' version='1.0' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='TestClass' isDomainClass='True'>"
        "       <ECArrayProperty propertyName='testArray' typeName='int'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <Value>0</Value>"
        "                           <DisplayString>value0</DisplayString>"
        "                       </ValueMap>"
        "                       <ValueMap>"
        "                           <Value>1</Value>"
        "                           <DisplayString>value1</DisplayString>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECArrayProperty>"
        "   </ECClass>"
        "   <ECClass typeName='TestClass2' isDomainClass='True'>"
        "       <ECProperty propertyName='testProp' typeName='int'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <Value>0</Value>"
        "                           <DisplayString>value0</DisplayString>"
        "                       </ValueMap>"
        "                       <ValueMap>"
        "                           <Value>1</Value>"
        "                           <DisplayString>value1</DisplayString>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "</ECSchema>";

    ReadSchema(schemaXML);

    EXPECT_TRUE(ECSchemaConverter::Convert(*m_schema.get())) << "Schema conversion failed";

    ASSERT_EQ(1, m_schema->GetEnumerationCount()) << "The number of enumerations created is not as expected.";

    CheckTypeName("TestClass_testArray", *m_schema, "testArray", {"TestClass"});
    CheckTypeName("TestClass_testArray", *m_schema, "testProp", {"TestClass2"});
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                    01/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, UseBasePropertyStandardValueIfSubset)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='test' version='1.0' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='TestClass' isDomainClass='True'>"
        "       <ECProperty propertyName='testProp' typeName='int'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <Value>0</Value>"
        "                           <DisplayString>value0</DisplayString>"
        "                       </ValueMap>"
        "                       <ValueMap>"
        "                           <Value>1</Value>"
        "                           <DisplayString>value1</DisplayString>"
        "                       </ValueMap>"
        "                       <ValueMap>"
        "                           <Value>2</Value>"
        "                           <DisplayString>value2</DisplayString>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='DerivedClassWithSubset' isDomainClass='True'>"
        "       <BaseClass>TestClass</BaseClass>"
        "       <ECProperty propertyName='testProp' typeName='int'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                   <MustBeFromList>False</MustBeFromList>"
        "                   <ValueMap>"
        "                       <ValueMap>"
        "                           <Value>0</Value>"
        "                           <DisplayString>value0</DisplayString>"
        "                       </ValueMap>"
        "                       <ValueMap>"
        "                           <Value>1</Value>"
        "                           <DisplayString>value1</DisplayString>"
        "                       </ValueMap>"
        "                   </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "   <ECClass typeName='DerivedClassWithExact' isDomainClass='True'>"
        "       <BaseClass>TestClass</BaseClass>"
        "       <ECProperty propertyName='testProp' typeName='int'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                   <MustBeFromList>False</MustBeFromList>"
        "                   <ValueMap>"
        "                       <ValueMap>"
        "                           <Value>0</Value>"
        "                           <DisplayString>value0</DisplayString>"
        "                       </ValueMap>"
        "                       <ValueMap>"
        "                           <Value>1</Value>"
        "                           <DisplayString>value1</DisplayString>"
        "                       </ValueMap>"
        "                       <ValueMap>"
        "                           <Value>2</Value>"
        "                           <DisplayString>value2</DisplayString>"
        "                       </ValueMap>"
        "                   </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "</ECSchema>";

    ReadSchema(schemaXML);

    EXPECT_TRUE(ECSchemaConverter::Convert(*m_schema.get())) << "Schema conversion failed";

    ASSERT_EQ(1, m_schema->GetEnumerationCount()) << "The number of enumerations created is not as expected.";

    CheckTypeName("TestClass_testProp", *m_schema, "testProp", {"TestClass", "DerivedClassWithSubset", "DerivedClassWithExact"});
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                    03/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardValueToEnumConversionTest, TestRootClassInSeparateSchema)
    {
    Utf8CP baseSchemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='baseTest' version='1.0' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECClass typeName='TestClass' isDomainClass='True'>"
        "       <ECProperty propertyName='testProp' typeName='int'/>"
        "   </ECClass>"
        "</ECSchema>";
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='test' version='1.0' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "   <ECSchemaReference name='baseTest' version='01.00' prefix='base'/>"
        "   <ECClass typeName='TestClass' isDomainClass='True'>"
        "       <BaseClass>base:TestClass</BaseClass>"
        "       <ECProperty propertyName='testProp' typeName='int'>"
        "           <ECCustomAttributes>"
        "               <StandardValues xmlns='EditorCustomAttributes.01.00'>"
        "                  <ValueMap>"
        "                       <ValueMap>"
        "                           <Value>0</Value>"
        "                           <DisplayString>value0</DisplayString>"
        "                       </ValueMap>"
        "                       <ValueMap>"
        "                           <Value>1</Value>"
        "                           <DisplayString>value1</DisplayString>"
        "                       </ValueMap>"
        "                       <ValueMap>"
        "                           <Value>2</Value>"
        "                           <DisplayString>value2</DisplayString>"
        "                       </ValueMap>"
        "                  </ValueMap>"
        "               </StandardValues>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "   </ECClass>"
        "</ECSchema>";

    ECSchemaPtr baseSchema;
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success ,ECSchema::ReadFromXmlString(baseSchema, baseSchemaXml, *context));
    ASSERT_TRUE(baseSchema.IsValid());
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema.get())) << "Schema conversion failed";

    ASSERT_EQ(0, schema->GetEnumerationCount()) << "The number of enumerations created is not as expected.";
    ASSERT_EQ(1, baseSchema->GetEnumerationCount()) << "The number of enumerations created is not as expected.";

    CheckTypeName("base:TestClass_testProp", *schema, "testProp", {"TestClass"});
    CheckTypeName("TestClass_testProp", *baseSchema, "testProp", {"TestClass"});
    }

Utf8String StandardCustomAttributeConversionTests::GetDateTimeInfoValue(IECInstancePtr instancePtr, Utf8CP name)
    {
    ECValue value;
    instancePtr->GetValue(value, name);
    EXPECT_FALSE(value.IsNull()) << "The value of the property, '" << name << "', is null when it shouldn't be.";
    EXPECT_TRUE(value.IsString()) << "Property: " << name << " is supposed to be a string";

    return value.GetUtf8CP();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Andreas.Kurka                 04 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeConversionTests, TestDateTimeAndClassHasCurrentTimeStampPropertyConversion)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"CAConversionTestSchema.01.00.ecschema.xml").c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);

    ASSERT_EQ(true, ECSchemaConverter::Convert(*schema)) << "Failed to convert " << schema->GetFullSchemaName().c_str() << ".";

    ASSERT_TRUE(ECSchema::IsSchemaReferenced(*schema, *m_coreCASchema)) << "Converted schema is missing schema reference to CoreCustomAttributes";
    
    ECClassP classAP = schema->GetClassP("TestClassA");
    EXPECT_TRUE(classAP != nullptr) << "Could not find TestClassA in schema";
    ECClassP classBP = schema->GetClassP("TestClassB");
    EXPECT_TRUE(classBP != nullptr) << "Could not find TestClassB in schema";
    ECClassP classCP = schema->GetClassP("TestClassC");
    EXPECT_TRUE(classCP != nullptr) << "Could not find TestClassC in schema";

    ECPropertyP lastModPropP = classAP->GetPropertyP("LastMod");
    EXPECT_TRUE(lastModPropP != nullptr) << "TestClassA is supposed to have property LastMod";

    ECPropertyP nextModPropP = classBP->GetPropertyP("NextMod");
    EXPECT_TRUE(nextModPropP != nullptr) << "TestClassB is supposed to have property LastMod";

    IECInstancePtr dateTimeInfoAPtr = lastModPropP->GetCustomAttribute("DateTimeInfo");
    EXPECT_TRUE(dateTimeInfoAPtr != nullptr) << "Could not find DateTimeInfo on LastMod property of TestClassA";
    EXPECT_EQ(m_coreCASchema->GetSchemaKey(), dateTimeInfoAPtr->GetClass().GetSchema().GetSchemaKey()) << "The custom attribute, DateTimeInfo, on property " << lastModPropP->GetName().c_str() << " was not converted to use the new CoreCA custom attribute";
    EXPECT_STREQ("Utc", GetDateTimeInfoValue(dateTimeInfoAPtr, "DateTimeKind").c_str()) << "DateTimeKind of TestClassA does not have expected value.";
    EXPECT_STREQ("DateTime", GetDateTimeInfoValue(dateTimeInfoAPtr, "DateTimeComponent").c_str()) << "DateTimeComponent of TestClassA does not have expected value.";

    IECInstancePtr dateTimeInfoBPtr = nextModPropP->GetCustomAttribute("DateTimeInfo");
    EXPECT_TRUE(dateTimeInfoBPtr != nullptr) << "Could not find DateTimeInfo on NextMod property of TestClassB";
    EXPECT_EQ(m_coreCASchema->GetSchemaKey(), dateTimeInfoBPtr->GetClass().GetSchema().GetSchemaKey()) << "The custom attribute, DateTimeInfo, on property " << nextModPropP->GetName().c_str() << " was not converted to use the new CoreCA custom attribute";
    EXPECT_STREQ("Unspecified", GetDateTimeInfoValue(dateTimeInfoBPtr, "DateTimeKind").c_str()) << "DateTimeKind of TestClassB does not have expected value.";
    EXPECT_STREQ("Date", GetDateTimeInfoValue(dateTimeInfoBPtr, "DateTimeComponent").c_str()) << "DateTimeComponent of TestClassB does not have expected value.";

    IECInstancePtr timeStampInstancePtr = classCP->GetCustomAttribute("ClassHasCurrentTimeStampProperty");
    EXPECT_TRUE(timeStampInstancePtr != nullptr) << "Could not get ClassHasCurrentTimeStampProperty CA from TestClassC";
    EXPECT_EQ(m_coreCASchema->GetSchemaKey(), dateTimeInfoAPtr->GetClass().GetSchema().GetSchemaKey()) << "The custom attribute, ClassHasCurrentTimeStampProperty, on class " << classCP->GetFullName() << " was not converted to use the new CoreCA custom attribute";
    ECValue checkValue;
    timeStampInstancePtr->GetValue(checkValue, "PropertyName");
    EXPECT_TRUE(checkValue.IsString()) << "The value in the property PropertyName of CustomAttribute ClassHasCurrentTimeStampProperty is not of the expected type";
    EXPECT_STREQ("TimeStampProp", checkValue.GetUtf8CP()) << "The value of CustomAttribute ClassHasCurrentTimeStampProperty.PropertyName is not as expected.";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Colin.Kerr                          06/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeConversionTests, TestDynamicSchemaCAConversion)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='ICanHaveAnyStuffAndThings' namespacePrefix='ichasat' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='1.13' prefix='bsca'/>"
        "   <ECCustomAttributes>"
        "       <DynamicSchema xmlns='Bentley_Standard_CustomAttributes.01.13' />"
        "   </ECCustomAttributes>"
        "</ECSchema>";

    ECSchemaPtr dynamicSchema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(dynamicSchema, schemaXml, *context);
    ASSERT_TRUE(dynamicSchema.IsValid());
    ASSERT_TRUE(ECSchemaConverter::Convert(*dynamicSchema));

    EXPECT_TRUE(dynamicSchema->IsDefined("CoreCustomAttributes", "DynamicSchema"));
    EXPECT_FALSE(dynamicSchema->IsDefined("Bentley_Standard_CustomAttributes", "DynamicSchema"));
    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*dynamicSchema, *m_coreCASchema));
    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*dynamicSchema, *m_bscaSchema));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                 02/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeConversionTests, TestSupplementalSchemaMetaDataConversion)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='suppSchema_Supplemental_Testing' namespacePrefix='sup' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='1.13' prefix='bsca'/>"
        "   <ECCustomAttributes>"
        "       <SupplementalSchemaMetaData xmlns='Bentley_Standard_CustomAttributes.01.13'>"
        "           <PrimarySchemaName>TestSchema</PrimarySchemaName>"
        "           <PrimarySchemaMajorVersion>1</PrimarySchemaMajorVersion>"
        "           <PrimarySchemaMinorVersion>0</PrimarySchemaMinorVersion>"
        "           <Precedence>200</Precedence>"
        "           <Purpose>Testing</Purpose>"
        "           <IsUserSpecific>False</IsUserSpecific>"
        "       </SupplementalSchemaMetaData>"
        "   </ECCustomAttributes>"
        "</ECSchema>";

    ECSchemaPtr suppSchema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(suppSchema, schemaXml, *context);
    ASSERT_TRUE(suppSchema.IsValid());
    ASSERT_TRUE(ECSchemaConverter::Convert(*suppSchema));

    EXPECT_TRUE(suppSchema->IsSupplementalSchema());
    EXPECT_TRUE(suppSchema->IsDefined("CoreCustomAttributes", "SupplementalSchema"));
    EXPECT_FALSE(suppSchema->IsDefined("Bentley_Standard_CustomAttributes", "SupplementalSchemaMetaData"));
    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*suppSchema, *m_coreCASchema));
    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*suppSchema, *m_bscaSchema));

    IECInstancePtr metaDataInstance = suppSchema->GetCustomAttribute("CoreCustomAttributes", "SupplementalSchema");
    ASSERT_TRUE(metaDataInstance.IsValid());
    SupplementalSchemaMetaData metaData(*metaDataInstance);
    EXPECT_STREQ("TestSchema", metaData.GetPrimarySchemaName().c_str());
    EXPECT_EQ(1, metaData.GetPrimarySchemaReadVersion());
    EXPECT_EQ(0, metaData.GetPrimarySchemaMinorVersion());
    EXPECT_EQ(200, metaData.GetSupplementalSchemaPrecedence());
    EXPECT_STREQ("Testing", metaData.GetSupplementalSchemaPurpose().c_str());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                 02/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeConversionTests, TestSupplementedSchemaConversion)
    {
    Utf8CP supSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema_Supplemental_Testing" namespacePrefix="sup" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="Bentley_Standard_CustomAttributes" version="1.13" prefix="bsca"/>
            <ECCustomAttributes>
                <SupplementalSchemaMetaData xmlns="Bentley_Standard_CustomAttributes.01.13">
                    <PrimarySchemaName>TestSchema</PrimarySchemaName>
                    <PrimarySchemaMajorVersion>1</PrimarySchemaMajorVersion>
                    <PrimarySchemaMinorVersion>0</PrimarySchemaMinorVersion>
                    <Precedence>200</Precedence>
                    <Purpose>Testing</Purpose>
                    <IsUserSpecific>False</IsUserSpecific>
                </SupplementalSchemaMetaData>
            </ECCustomAttributes>
        </ECSchema>)xml";

    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" isDomainClass="false"/>
        </ECSchema>)xml";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ECSchemaPtr supSchema;
    ECSchema::ReadFromXmlString(supSchema, supSchemaXml, *context);
    bvector<ECSchemaP> supSchemas;
    supSchemas.push_back(supSchema.get());
    SupplementedSchemaBuilder builder;
    ASSERT_EQ(SupplementedSchemaStatus::Success, builder.UpdateSchema(*schema.get(), supSchemas, false)) << "Failed to supplement schema";
    EXPECT_TRUE(ECSchemaConverter::Convert(*schema.get())) << "Schema conversion failed";

    EXPECT_TRUE(schema->IsSupplemented());
    EXPECT_FALSE(schema->IsSupplementalSchema());
    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schema, *m_coreCASchema));
    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*schema, *m_bscaSchema));

    SupplementalSchemaInfoPtr suppInfo = schema->GetSupplementalInfo();
    ASSERT_TRUE(suppInfo.IsValid());
    bvector<Utf8String> suppSchemaNames;
    EXPECT_EQ(ECObjectsStatus::Success, suppInfo->GetSupplementalSchemaNames(suppSchemaNames));
    EXPECT_EQ(1, suppSchemaNames.size());
    EXPECT_STREQ("TestSchema_Supplemental_Testing.01.00.00", suppSchemaNames[0].c_str());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                 06/2017
//+---------------+---------------+---------------+---------------+---------------+------
void TestDropAllOldCustomAttributesWithAConversion(ECSchemaCR oldStandardSchema, Utf8CP caClassName, bool shouldBeRemoved = true)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECSchemaReference name="Bentley_Standard_CustomAttributes" version="1.13" prefix="bsca"/>
                <ECClass typeName="A" isDomainClass="true"/>
                <ECClass typeName="B" isDomainClass="true"/>
                <ECRelationshipClass typeName="ARelB" isDomainClass="true" strength="referencing" strengthDirection="forward">
                    <ECCustomAttributes>
                        <ReferenceTypeRelationship xmlns="Bentley_Standard_CustomAttributes.01.13"/>
                    </ECCustomAttributes>
                    <Source cardinality="(1,1)" polymorphic="true">
                        <Class class="A"/>
                    </Source>
                    <Target cardinality="(1,1)" polymorphic="true">
                        <Class class="B"/>
                    </Target>
                </ECRelationshipClass>
            </ECSchema>
        )xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());
    EXPECT_EQ(1, schema->GetReferencedSchemas().size());
    ECClassCP relClass = schema->GetClassCP("ARelB");
    EXPECT_TRUE(nullptr != relClass);

    ECClassCP caClass = oldStandardSchema.GetClassCP(caClassName);
    EXPECT_TRUE(nullptr != caClass);
    EXPECT_TRUE(relClass->IsDefinedLocal(*caClass));

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema));
    EXPECT_EQ(0, schema->GetReferencedSchemas().size());
    ECClassCP relClassAfterConv = schema->GetClassCP("ARelB");
    EXPECT_TRUE(nullptr != relClassAfterConv);
    EXPECT_FALSE(relClassAfterConv->IsDefinedLocal(*caClass));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                 06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeConversionTests, DropAllOldCustomAttributesWithoutAConversion)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECSchemaReference name="Bentley_Standard_CustomAttributes" version="1.13" prefix="bsca"/>
                <ECClass typeName="A" isDomainClass="true"/>
                <ECClass typeName="B" isDomainClass="true"/>
                <ECRelationshipClass typeName="ARelB" isDomainClass="true" strength="referencing" strengthDirection="forward">
                    <ECCustomAttributes>
                        <ReferenceTypeRelationship xmlns="Bentley_Standard_CustomAttributes.01.13"/>
                    </ECCustomAttributes>
                    <Source cardinality="(1,1)" polymorphic="true">
                        <Class class="A"/>
                    </Source>
                    <Target cardinality="(1,1)" polymorphic="true">
                        <Class class="B"/>
                    </Target>
                </ECRelationshipClass>
            </ECSchema>
        )xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());
    EXPECT_EQ(1, schema->GetReferencedSchemas().size());
    ECClassCP relClass = schema->GetClassCP("ARelB");
    EXPECT_TRUE(nullptr != relClass);

    ECClassCP caClass = StandardCustomAttributeHelper::GetCustomAttributeClass("ReferenceTypeRelationship");
    EXPECT_TRUE(nullptr != caClass);
    EXPECT_TRUE(relClass->IsDefinedLocal(*caClass));

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema));
    EXPECT_EQ(0, schema->GetReferencedSchemas().size());
    ECClassCP relClassAfterConv = schema->GetClassCP("ARelB");
    EXPECT_TRUE(nullptr != relClassAfterConv);
    EXPECT_FALSE(relClassAfterConv->IsDefinedLocal(*caClass));
    }

void propertyCategoryHasSameValuesAsCategoryCA (IECInstanceCP categoryCA, PropertyCategoryCP propertyCategory)
    {
    ECValue value;
    categoryCA->GetValue(value, "Name");
    EXPECT_STREQ(value.GetUtf8CP(), propertyCategory->GetName().c_str()) << "Category 'Name' doesn't match converted PropertyCategory.Name";
    categoryCA->GetValue(value, "DisplayLabel");
    EXPECT_STREQ(value.GetUtf8CP(), propertyCategory->GetDisplayLabel().c_str()) << "Category 'DisplayLabel' doesn't match converted PropertyCategory.DisplayLabel";
    categoryCA->GetValue(value, "Description");
    EXPECT_STREQ(value.GetUtf8CP(), propertyCategory->GetDescription().c_str()) << "Category 'Description' doesn't match converted PropertyCategory.Description";
    categoryCA->GetValue(value, "Priority");
    EXPECT_EQ(value.GetInteger(), propertyCategory->GetPriority()) << "Category 'Priority' doesn't match converted PropertyCategory.Priority";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Colin.Kerr                 06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeConversionTests, CategoryCustomAttribute_NoConflicts)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECSchemaReference name="EditorCustomAttributes" version="1.03" prefix="beca"/>
                <ECClass typeName="A" isDomainClass="true">
                    <ECProperty propertyName="A1" typeName="string">
                        <ECCustomAttributes>
                            <Category xmlns="EditorCustomAttributes.01.03">
                                <Name>Banana</Name>
                                <DisplayLabel>Banana Info</DisplayLabel>
                                <Description>Banana Properties</Description>
                                <Priority>1</Priority>
                            </Category>
                        </ECCustomAttributes>
                    </ECProperty>
                    <ECProperty propertyName="A2" typeName="string">
                        <ECCustomAttributes>
                            <Category xmlns="EditorCustomAttributes.01.03">
                                <Name>Apple</Name>
                                <DisplayLabel>Apple Info</DisplayLabel>
                                <Description>Apple Properties</Description>
                                <Priority>42</Priority>
                            </Category>
                        </ECCustomAttributes>
                    </ECProperty>
                </ECClass>
            </ECSchema>
        )xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());
    EXPECT_EQ(1, schema->GetReferencedSchemas().size());
    ECClassCP aClass = schema->GetClassCP("A");
    ASSERT_TRUE(nullptr != aClass);
    ECPropertyCP a1Prop = aClass->GetPropertyP("A1");
    ASSERT_TRUE(nullptr != a1Prop);
    IECInstancePtr bananaCatCA = a1Prop->GetCustomAttribute("EditorCustomAttributes", "Category");

    ECPropertyCP a2Prop = aClass->GetPropertyP("A2");
    ASSERT_TRUE(nullptr != a2Prop);
    IECInstancePtr appleCatCA = a2Prop->GetCustomAttribute("EditorCustomAttributes", "Category");

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema));
    EXPECT_EQ(0, schema->GetReferencedSchemas().size());
    ECClassCP aConvClass = schema->GetClassCP("A");
    EXPECT_TRUE(nullptr != aConvClass);

    EXPECT_EQ(2, schema->GetPropertyCategoryCount());

    PropertyCategoryCP bCat = schema->GetPropertyCategoryCP("Banana");
    ASSERT_NE(nullptr, bCat);
    PropertyCategoryCP aCat = schema->GetPropertyCategoryCP("Apple");
    ASSERT_NE(nullptr, aCat);

    propertyCategoryHasSameValuesAsCategoryCA(bananaCatCA.get(), bCat);
    propertyCategoryHasSameValuesAsCategoryCA(appleCatCA.get(), aCat);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Colin.Kerr                 06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeConversionTests, CategoryCustomAttribute_ConflictingCategories)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECSchemaReference name="EditorCustomAttributes" version="1.03" prefix="beca"/>
                <ECClass typeName="A" isDomainClass="true">
                    <ECProperty propertyName="A1" typeName="string">
                        <ECCustomAttributes>
                            <Category xmlns="EditorCustomAttributes.01.03">
                                <Name>Banana</Name>
                                <DisplayLabel>Banana Info</DisplayLabel>
                                <Description>Banana Properties</Description>
                                <Priority>1</Priority>
                            </Category>
                        </ECCustomAttributes>
                    </ECProperty>
                    <ECProperty propertyName="A2" typeName="string">
                        <ECCustomAttributes>
                            <Category xmlns="EditorCustomAttributes.01.03">
                                <Name>Banana</Name>
                                <DisplayLabel>Apple Info</DisplayLabel>
                                <Description>Apple Properties</Description>
                                <Priority>42</Priority>
                            </Category>
                        </ECCustomAttributes>
                    </ECProperty>
                </ECClass>
            </ECSchema>
        )xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());
    EXPECT_EQ(1, schema->GetReferencedSchemas().size());
    ECClassCP aClass = schema->GetClassCP("A");
    ASSERT_TRUE(nullptr != aClass);
    ECPropertyCP a1Prop = aClass->GetPropertyP("A1");
    ASSERT_TRUE(nullptr != a1Prop);
    IECInstancePtr bananaCatCA = a1Prop->GetCustomAttribute("EditorCustomAttributes", "Category");

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema));
    EXPECT_EQ(0, schema->GetReferencedSchemas().size());
    ECClassCP aConvClass = schema->GetClassCP("A");
    EXPECT_TRUE(nullptr != aConvClass);

    EXPECT_EQ(1, schema->GetPropertyCategoryCount()) << "Expected categories to be merged because their names were the same";

    PropertyCategoryCP bCat = schema->GetPropertyCategoryCP("Banana");
    ASSERT_NE(nullptr, bCat);

    propertyCategoryHasSameValuesAsCategoryCA(bananaCatCA.get(), bCat);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Colin.Kerr                 06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeConversionTests, CategoryCustomAttribute_CategoryNameConflictsWithOtherElement)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECSchemaReference name="EditorCustomAttributes" version="1.03" prefix="beca"/>
                <ECClass typeName="A" isDomainClass="true">
                    <ECProperty propertyName="A1" typeName="string">
                        <ECCustomAttributes>
                            <Category xmlns="EditorCustomAttributes.01.03">
                                <Name>Banana</Name>
                                <DisplayLabel>Banana Info</DisplayLabel>
                                <Description>Banana Properties</Description>
                                <Priority>1</Priority>
                            </Category>
                        </ECCustomAttributes>
                    </ECProperty>
                    <ECProperty propertyName="A2" typeName="string">
                        <ECCustomAttributes>
                            <Category xmlns="EditorCustomAttributes.01.03">
                                <Name>Banana</Name>
                                <DisplayLabel>Apple Info</DisplayLabel>
                                <Description>Apple Properties</Description>
                                <Priority>42</Priority>
                            </Category>
                        </ECCustomAttributes>
                    </ECProperty>
                </ECClass>
                <ECClass typeName="Banana" isDomainClass="true" />
            </ECSchema>
        )xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());
    EXPECT_EQ(1, schema->GetReferencedSchemas().size());
    ECClassCP aClass = schema->GetClassCP("A");
    ASSERT_TRUE(nullptr != aClass);
    ECPropertyCP a1Prop = aClass->GetPropertyP("A1");
    ASSERT_TRUE(nullptr != a1Prop);
    IECInstancePtr bananaCatCA = a1Prop->GetCustomAttribute("EditorCustomAttributes", "Category");

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema));
    EXPECT_EQ(0, schema->GetReferencedSchemas().size());
    ECClassCP aConvClass = schema->GetClassCP("A");
    EXPECT_TRUE(nullptr != aConvClass);

    EXPECT_EQ(1, schema->GetPropertyCategoryCount()) << "Expected categories to be merged because their names were the same";

    PropertyCategoryCP bCat = schema->GetPropertyCategoryCP("Banana_Category");
    ASSERT_NE(nullptr, bCat);

    // Modify Banana category CA to match the rename
    ECValue nameValue("Banana_Category");
    bananaCatCA->SetValue("Name", nameValue);
    propertyCategoryHasSameValuesAsCategoryCA(bananaCatCA.get(), bCat);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Colin.Kerr                 06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeConversionTests, CategoryCustomAttribute_NameNotValid)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECSchemaReference name="EditorCustomAttributes" version="1.03" prefix="beca"/>
                <ECClass typeName="A" isDomainClass="true">
                    <ECProperty propertyName="A1" typeName="string">
                        <ECCustomAttributes>
                            <Category xmlns="EditorCustomAttributes.01.03">
                                <Name>Banana Space</Name>
                                <DisplayLabel>Banana Info</DisplayLabel>
                                <Description>Banana Properties</Description>
                                <Priority>1</Priority>
                            </Category>
                        </ECCustomAttributes>
                    </ECProperty>
                </ECClass>
            </ECSchema>
        )xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());
    EXPECT_EQ(1, schema->GetReferencedSchemas().size());
    ECClassCP aClass = schema->GetClassCP("A");
    ASSERT_TRUE(nullptr != aClass);
    ECPropertyCP a1Prop = aClass->GetPropertyP("A1");
    ASSERT_TRUE(nullptr != a1Prop);
    IECInstancePtr bananaCatCA = a1Prop->GetCustomAttribute("EditorCustomAttributes", "Category");

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema));
    EXPECT_EQ(0, schema->GetReferencedSchemas().size());
    ECClassCP aConvClass = schema->GetClassCP("A");
    EXPECT_TRUE(nullptr != aConvClass);

    EXPECT_EQ(1, schema->GetPropertyCategoryCount()) << "Expected categories to be merged because their names were the same";

    Utf8String encodedName = ECNameValidation::EncodeToValidName("Banana Space");
    PropertyCategoryCP bCat = schema->GetPropertyCategoryCP(encodedName.c_str());
    ASSERT_NE(nullptr, bCat);

    ECValue nameValue(encodedName.c_str());
    bananaCatCA->SetValue("Name", nameValue);
    propertyCategoryHasSameValuesAsCategoryCA(bananaCatCA.get(), bCat);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                 06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyPriorityCustomAttributeConversionTest, EmptyPropertyPriorityIsRemovedAndPriorityAttributeNotSet)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="EditorCustomAttributes" version="1.03" prefix="beca"/>
            <ECClass typeName="A" isDomainClass="true">
                <ECProperty propertyName="propWithPriority" typeName="string">
                    <ECCustomAttributes>
                        <PropertyPriority xmlns="EditorCustomAttributes.01.03" />
                    </ECCustomAttributes>
                </ECProperty>
            </ECClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());
    EXPECT_EQ(1, schema->GetReferencedSchemas().size());

    ECClassCP ecClass = schema->GetClassCP("A");
    ECPropertyCP ecProp = ecClass->GetPropertyP("propWithPriority");
    CheckForPropertyPriorityCALocally(ecProp, true);

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema));
    EXPECT_EQ(0, schema->GetReferencedSchemas().size());

    ECClassCP afterConv = schema->GetClassCP("A");
    ECPropertyCP afterConvProp = afterConv->GetPropertyP("propWithPriority");
    EXPECT_FALSE(afterConvProp->IsPriorityLocallyDefined());
    CheckForPropertyPriorityCALocally(afterConvProp, false);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                 06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyPriorityCustomAttributeConversionTest, LocallyDefinedPropertyPriority)
    {
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="EditorCustomAttributes" version="1.03" prefix="beca"/>
            <ECClass typeName="A" isDomainClass="true">
                <ECProperty propertyName="propWithPriority" typeName="string">
                    <ECCustomAttributes>
                        <PropertyPriority xmlns="EditorCustomAttributes.01.03">
                            <Priority>3</Priority>
                        </PropertyPriority>
                    </ECCustomAttributes>
                </ECProperty>
            </ECClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());
    EXPECT_EQ(1, schema->GetReferencedSchemas().size());

    ECClassCP ecClass = schema->GetClassCP("A");
    ECPropertyCP ecProp = ecClass->GetPropertyP("propWithPriority");
    CheckForPropertyPriorityCALocally(ecProp, true);

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema));
    EXPECT_EQ(0, schema->GetReferencedSchemas().size());

    ECClassCP afterConv = schema->GetClassCP("A");
    ECPropertyCP afterConvProp = afterConv->GetPropertyP("propWithPriority");
    EXPECT_EQ(3, afterConvProp->GetPriority());
    CheckForPropertyPriorityCALocally(afterConvProp, false);
    }
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="EditorCustomAttributes" version="1.03" prefix="beca"/>
            <ECClass typeName="A" isDomainClass="true">
                <ECProperty propertyName="propWithPriority" typeName="string">
                    <ECCustomAttributes>
                        <PropertyPriority xmlns="EditorCustomAttributes.01.03">
                            <Priority>-3</Priority>
                        </PropertyPriority>
                    </ECCustomAttributes>
                </ECProperty>
            </ECClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());
    EXPECT_EQ(1, schema->GetReferencedSchemas().size());

    ECClassCP ecClass = schema->GetClassCP("A");
    ECPropertyCP ecProp = ecClass->GetPropertyP("propWithPriority");
    CheckForPropertyPriorityCALocally(ecProp, true);

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema));
    EXPECT_EQ(0, schema->GetReferencedSchemas().size());

    ECClassCP afterConv = schema->GetClassCP("A");
    ECPropertyCP afterConvProp = afterConv->GetPropertyP("propWithPriority");
    CheckForPropertyPriorityCALocally(ecProp, false);
    EXPECT_EQ(-3, afterConvProp->GetPriority());
    }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                 06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyPriorityCustomAttributeConversionTest, PropertyPriorityOverride)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="EditorCustomAttributes" version="1.03" prefix="beca"/>
            <ECClass typeName="A" isDomainClass="true">
                <ECProperty propertyName="propWithPriority" typeName="string">
                    <ECCustomAttributes>
                        <PropertyPriority xmlns="EditorCustomAttributes.01.03">
                            <Priority>3</Priority>
                        </PropertyPriority>
                    </ECCustomAttributes>
                </ECProperty>
            </ECClass>
            <ECClass typeName="B" isDomainClass="true">
                <BaseClass>A</BaseClass>
                <ECProperty propertyName="propWithPriority" typeName="string">
                    <ECCustomAttributes>
                        <PropertyPriority xmlns="EditorCustomAttributes.01.03">
                            <Priority>-4</Priority>
                        </PropertyPriority>
                    </ECCustomAttributes>
                </ECProperty>
            </ECClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());
    EXPECT_EQ(1, schema->GetReferencedSchemas().size());

    ECClassCP classA = schema->GetClassCP("A");
    ECPropertyCP propA = classA->GetPropertyP("propWithPriority");
    CheckForPropertyPriorityCALocally(propA, true);

    ECClassCP classB = schema->GetClassCP("B");
    ECPropertyCP propB = classB->GetPropertyP("propWithPriority");
    CheckForPropertyPriorityCALocally(propB, true);

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema));
    EXPECT_EQ(0, schema->GetReferencedSchemas().size());

    ECClassCP afterConvClassA = schema->GetClassCP("A");
    ECPropertyCP afterConvPropA = afterConvClassA->GetPropertyP("propWithPriority");
    CheckForPropertyPriorityCALocally(afterConvPropA, false);
    EXPECT_EQ(3, afterConvPropA->GetPriority());

    ECClassCP afterConvClassB = schema->GetClassCP("B");
    ECPropertyCP afterConvPropB = afterConvClassB->GetPropertyP("propWithPriority");
    CheckForPropertyPriorityCALocally(afterConvPropB, false);
    EXPECT_EQ(-4, afterConvPropB->GetPriority());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                 06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyPriorityCustomAttributeConversionTest, PropertyPriorityInherited)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="EditorCustomAttributes" version="1.03" prefix="beca"/>
            <ECClass typeName="A" isDomainClass="true">
                <ECProperty propertyName="propWithPriority" typeName="string">
                    <ECCustomAttributes>
                        <PropertyPriority xmlns="EditorCustomAttributes.01.03">
                            <Priority>3</Priority>
                        </PropertyPriority>
                    </ECCustomAttributes>
                </ECProperty>
            </ECClass>
            <ECClass typeName="B" isDomainClass="true">
                <BaseClass>A</BaseClass>
                <ECProperty propertyName="propWithPriority" typeName="string"/>
            </ECClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());
    EXPECT_EQ(1, schema->GetReferencedSchemas().size());

    {
    ECClassCP classA = schema->GetClassCP("A");
    ECPropertyP propA = classA->GetPropertyP("propWithPriority");
    CheckForPropertyPriorityCALocally(propA, true);

    ECClassCP classB = schema->GetClassCP("B");
    ECPropertyP propB = classB->GetPropertyP("propWithPriority");
    CheckForPropertyPriorityCALocally(propB, false);
    }

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema));
    EXPECT_EQ(0, schema->GetReferencedSchemas().size());

    {
    ECClassCP classA = schema->GetClassCP("A");
    ECPropertyP propA = classA->GetPropertyP("propWithPriority");
    CheckForPropertyPriorityCALocally(propA, false);
    EXPECT_TRUE(propA->IsPriorityLocallyDefined());
    EXPECT_EQ(3, propA->GetPriority());

    ECClassCP classB = schema->GetClassCP("B");
    ECPropertyP propB = classB->GetPropertyP("propWithPriority");
    CheckForPropertyPriorityCALocally(propB, false);
    EXPECT_FALSE(propB->IsPriorityLocallyDefined());
    EXPECT_EQ(3, propB->GetPriority());
    }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                 06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyPriorityCustomAttributeConversionTest, PropertyPriorityInherited_Supplemental)
    {
    Utf8CP schemaXML = R"xml(<?xml version='1.0' encoding='UTF-8'?>
        <ECSchema schemaName='Test' version='78.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>
            <ECClass typeName='A' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>
                <ECProperty propertyName='propWithPriority' typeName='int' displayLabel='Title' />
            </ECClass>
            <ECClass typeName='B' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>
                <BaseClass>A</BaseClass>
                <ECProperty propertyName='propWithPriority' typeName='int' displayLabel='Title' />
            </ECClass>
        </ECSchema>)xml";
    Utf8CP supSchemaXml = R"xml(<?xml version='1.0' encoding='UTF-8'?>
        <ECSchema schemaName='Test_Supplemental_PropertyPriority' version='78.00' nameSpacePrefix='tr_sv' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>
           <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.13' prefix='bsca' />
           <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />
           <ECCustomAttributes>
               <SupplementalSchemaMetaData xmlns='Bentley_Standard_CustomAttributes.01.05'>
                   <PrimarySchemaName>Test</PrimarySchemaName>
                   <PrimarySchemaMajorVersion>78</PrimarySchemaMajorVersion>
                   <PrimarySchemaMinorVersion>0</PrimarySchemaMinorVersion>
                   <Precedence>400</Precedence>
               </SupplementalSchemaMetaData>
           </ECCustomAttributes>
           <ECClass typeName='A' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>
               <ECProperty propertyName='propWithPriority' typeName='string' displayLabel='Title'>
                   <ECCustomAttributes>
                       <PropertyPriority xmlns="EditorCustomAttributes.01.03">
                            <Priority>3</Priority>
                        </PropertyPriority>
                   </ECCustomAttributes>
               </ECProperty>
           </ECClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXML, *context));
    ECSchemaPtr supSchema;
    ECSchema::ReadFromXmlString(supSchema, supSchemaXml, *context);
    bvector<ECSchemaP> supSchemas;
    supSchemas.push_back(supSchema.get());
    SupplementedSchemaBuilder builder;
    ASSERT_EQ(SupplementedSchemaStatus::Success, builder.UpdateSchema(*schema.get(), supSchemas, true)) << "Failed to supplement schema";
    ASSERT_TRUE(schema->IsSupplemented()) << "Schema returned success but was not supplemented";

    {
    ECClassCP classA = schema->GetClassCP("A");
    ECPropertyCP propA = classA->GetPropertyP("propWithPriority");
    CheckForPropertyPriorityCA(propA, true); // Don't check locally because it is defined in a supplemental schema

    ECClassCP classB = schema->GetClassCP("B");
    ECPropertyCP propB = classB->GetPropertyP("propWithPriority");
    CheckForPropertyPriorityCALocally(propB, false);
    }

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema));
    EXPECT_EQ(1, schema->GetReferencedSchemas().size()); //CoreCustomAttributes schema referenced for new Supplemental info

    {
    ECClassCP classA = schema->GetClassCP("A");
    ECPropertyP propA = classA->GetPropertyP("propWithPriority");
    CheckForPropertyPriorityCALocally(propA, false);
    EXPECT_TRUE(propA->IsPriorityLocallyDefined());
    EXPECT_EQ(3, propA->GetPriority());

    ECClassCP classB = schema->GetClassCP("B");
    ECPropertyP propB = classB->GetPropertyP("propWithPriority", false);
    EXPECT_TRUE(nullptr == propB);
    }
    }

void verifyHidden(ECPropertyCP ecProperty)
    {
    IECInstancePtr hiddenPropertyCA = ecProperty->GetCustomAttribute("CoreCustomAttributes", "HiddenProperty");
    EXPECT_TRUE(hiddenPropertyCA.IsValid()) << "Expected to find the 'HiddenProperty' Custom Attribute on " <<
        ecProperty->GetClass().GetFullName() << "." << ecProperty->GetName().c_str();

    ECValue value;
    EXPECT_EQ(ECObjectsStatus::Success, hiddenPropertyCA->GetValue(value, "Show"))
        << "Failed to get the 'Show' value for " << ecProperty->GetClass().GetFullName() << "." << ecProperty->GetName().c_str();

    EXPECT_TRUE(value.IsNull() || !value.GetBoolean()) <<
        "Expected 'HiddenProperty.Show' CA value on " << ecProperty->GetClass().GetFullName() << "." << ecProperty->GetName().c_str() <<
        " to be unset or set to false to be considered hidden";
    }

void verifyShown(ECPropertyCP ecProperty)
    {
    IECInstancePtr hiddenPropertyCA = ecProperty->GetCustomAttribute("CoreCustomAttributes", "HiddenProperty");
    EXPECT_TRUE(hiddenPropertyCA.IsValid()) << "Expected to find the 'HiddenProperty' Custom Attribute on " <<
        ecProperty->GetClass().GetFullName() << "." << ecProperty->GetName().c_str();

    ECValue value;
    EXPECT_EQ(ECObjectsStatus::Success, hiddenPropertyCA->GetValue(value, "Show"))
        << "Failed to get the 'Show' value for " << ecProperty->GetClass().GetFullName() << "." << ecProperty->GetName().c_str();

    EXPECT_TRUE(!value.IsNull() || value.GetBoolean()) <<
        "Expected 'HiddenProperty.Show' CA value on " << ecProperty->GetClass().GetFullName() << "." << ecProperty->GetName().c_str() <<
        " to be set to true to be considered shown";
    }

void verifySchemaReferencesOnlyCoreCAs(ECSchemaCP convertedSchema)
    {
    EXPECT_EQ(1, convertedSchema->GetReferencedSchemas().size()) << "Expected only one schema reference";
    EXPECT_STREQ("CoreCustomAttributes", convertedSchema->GetReferencedSchemas().begin()->second->GetName().c_str()) <<
        "The only referenced schema does not have the correct name.";
    }

void verifyHiddenPropertyAppliedCorrectly(ECSchemaCP convertedSchema)
    {
    verifySchemaReferencesOnlyCoreCAs(convertedSchema);

    for (auto ecClass : convertedSchema->GetClasses())
        {
        for (auto ecProperty : ecClass->GetProperties(false))
            {
            if (ecProperty->GetDescription().Equals("Hide"))
                verifyHidden(ecProperty);
            else if (ecProperty->GetDescription().Equals("Show"))
                verifyShown(ecProperty);
            else
                EXPECT_TRUE(false) << "The property " << ecClass->GetFullName() << "." << ecProperty->GetName().c_str() << " does not specify 'Hide' or a 'Show' property its description";
            }
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Colin.Kerr                 06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeConversionTests, HidePropertyCustomAttribute)
    {
    // Property description defines if we expect the property to be shown or hidden
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECSchemaReference name="EditorCustomAttributes" version="1.03" prefix="beca"/>
                <ECClass typeName="A" isDomainClass="true">
                    <ECProperty propertyName="A1" typeName="string" description="Hide">
                        <ECCustomAttributes>
                            <HideProperty xmlns="EditorCustomAttributes.01.03"/>
                        </ECCustomAttributes>
                    </ECProperty>
                    <ECProperty propertyName="A2" typeName="string" description="Hide">
                        <ECCustomAttributes>
                            <HideProperty xmlns="EditorCustomAttributes.01.03">
                                <If2D>true</If2D>
                                <If3D>true</If3D>
                            </HideProperty>
                        </ECCustomAttributes>
                    </ECProperty>
                    <ECProperty propertyName="A3" typeName="string" description="Hide">
                        <ECCustomAttributes>
                            <HideProperty xmlns="EditorCustomAttributes.01.03">
                                <If2D>true</If2D>
                                <If3D>false</If3D>
                            </HideProperty>
                        </ECCustomAttributes>
                    </ECProperty>
                    <ECProperty propertyName="A4" typeName="string" description="Hide">
                        <ECCustomAttributes>
                            <HideProperty xmlns="EditorCustomAttributes.01.03">
                                <If2D>false</If2D>
                                <If3D>true</If3D>
                            </HideProperty>
                        </ECCustomAttributes>
                    </ECProperty>
                    <ECProperty propertyName="A5" typeName="string" description="Hide">
                        <ECCustomAttributes>
                            <HideProperty xmlns="EditorCustomAttributes.01.03">
                                <If>some expression</If>
                            </HideProperty>
                        </ECCustomAttributes>
                    </ECProperty>
                    <ECProperty propertyName="A6" typeName="string" description="Show">
                        <ECCustomAttributes>
                            <HideProperty xmlns="EditorCustomAttributes.01.03">
                                <If2D>false</If2D>
                                <If3D>false</If3D>
                            </HideProperty>
                        </ECCustomAttributes>
                    </ECProperty>
                </ECClass>
                <ECClass typeName="B" isDomainClass="true">
                    <BaseClass>A</BaseClass>
                    <ECProperty propertyName="A1" typeName="string" description="Show">
                        <ECCustomAttributes>
                            <HideProperty xmlns="EditorCustomAttributes.01.03">
                                <If2D>false</If2D>
                                <If3D>false</If3D>
                            </HideProperty>
                        </ECCustomAttributes>
                    </ECProperty>
                    <ECProperty propertyName="A2" typeName="string" description="Show">
                        <ECCustomAttributes>
                            <HideProperty xmlns="EditorCustomAttributes.01.03">
                                <If3D>false</If3D>
                            </HideProperty>
                        </ECCustomAttributes>
                    </ECProperty>
                    <ECProperty propertyName="A3" typeName="string" description="Show">
                        <ECCustomAttributes>
                            <HideProperty xmlns="EditorCustomAttributes.01.03">
                                <If2D>false</If2D>
                            </HideProperty>
                        </ECCustomAttributes>
                    </ECProperty>
                    <ECProperty propertyName="A6" typeName="string" description="Hide">
                        <ECCustomAttributes>
                            <HideProperty xmlns="EditorCustomAttributes.01.03">
                                <If2D>true</If2D>
                            </HideProperty>
                        </ECCustomAttributes>
                    </ECProperty>
                </ECClass>
                <ECRelationshipClass typeName="ARelB" isDomainClass="true" strength="referencing" strengthDirection="forward">
                    <Source cardinality="(1,1)" polymorphic="true">
                        <Class class="A"/>
                    </Source>
                    <Target cardinality="(1,1)" polymorphic="true">
                        <Class class="B"/>
                    </Target>
                    <ECProperty propertyName="AB1" typeName="string" description="Hide">
                        <ECCustomAttributes>
                            <HideProperty xmlns="EDitorCustomAttributes.01.03"/>
                        </ECCustomAttributes>
                    </ECProperty>
                </ECRelationshipClass>
            </ECSchema>
        )xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());
    EXPECT_EQ(1, schema->GetReferencedSchemas().size());

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema));

    verifyHiddenPropertyAppliedCorrectly(schema.get());
    }

void verifyHiddenSchemaAppliedCorrectly(Utf8CP schemaXml)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());
    EXPECT_EQ(1, schema->GetReferencedSchemas().size());

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema));

    IECInstancePtr hiddenSchemaCA = schema->GetCustomAttribute("CoreCustomAttributes", "HiddenSchema");
    if (schema->GetDescription().Equals("Hide"))
        {
        verifySchemaReferencesOnlyCoreCAs(schema.get());
        ASSERT_TRUE(hiddenSchemaCA.IsValid()) << schema->GetName().c_str() << " expected to be hidden but 'HiddenSchema' CA not found";
        ECValue showClassesValue;
        ASSERT_EQ(ECObjectsStatus::Success, hiddenSchemaCA->GetValue(showClassesValue, "ShowClasses"));
        EXPECT_TRUE(showClassesValue.IsNull() || !showClassesValue.GetBoolean()) <<
            "Schema should be hidden based on the legacy DisplayOptions CA, we lack the information to ever set 'ShowClasses' to true so it should be unset or set to false";
        }
    else if (schema->GetDescription().Equals("Show"))
        {
        EXPECT_EQ(0, schema->GetReferencedSchemas().size());
        EXPECT_FALSE(hiddenSchemaCA.IsValid()) << schema->GetName().c_str() << " expected to be shown but 'HiddenSchema' CA found";
        }
    else
        EXPECT_TRUE(false) << schema->GetName().c_str() << " schema does not specify 'Hide' or 'Show' in its description";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Colin.Kerr                 06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeConversionTests, DisplayOptionsCustomAttribute_AppliedToSchema)
    {
    // Schema description defines if we expect the property to be shown or hidden
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0" description="Hide">
            <ECSchemaReference name="Bentley_Standard_CustomAttributes" version="1.13" prefix="bsca"/>
            <ECCustomAttributes>
                <DisplayOptions xmlns="Bentley_Standard_CustomAttributes.01.13">
                    <Hidden>true</Hidden>
                    <HideInstances>true</HideInstances>
                </DisplayOptions>
            </ECCustomAttributes>
            <ECClass typeName="A" isDomainClass="true">
                <ECProperty propertyName="A1" typeName="string"/>
                <ECProperty propertyName="A2" typeName="string"/>
            </ECClass>
        </ECSchema>
    )xml";
    verifyHiddenSchemaAppliedCorrectly(schemaXml);

    Utf8CP schemaXml2 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0" description="Hide">
            <ECSchemaReference name="Bentley_Standard_CustomAttributes" version="1.13" prefix="bsca"/>
            <ECCustomAttributes>
                <DisplayOptions xmlns="Bentley_Standard_CustomAttributes.01.13">
                    <Hidden>true</Hidden>
                </DisplayOptions>
            </ECCustomAttributes>
            <ECClass typeName="A" isDomainClass="true">
                <ECProperty propertyName="A1" typeName="string"/>
                <ECProperty propertyName="A2" typeName="string"/>
            </ECClass>
        </ECSchema>
    )xml";
    verifyHiddenSchemaAppliedCorrectly(schemaXml2);

    Utf8CP schemaXml3 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0" description="Hide">
            <ECSchemaReference name="Bentley_Standard_CustomAttributes" version="1.13" prefix="bsca"/>
            <ECCustomAttributes>
                <DisplayOptions xmlns="Bentley_Standard_CustomAttributes.01.13">
                    <HideInstances>true</HideInstances>
                </DisplayOptions>
            </ECCustomAttributes>
            <ECClass typeName="A" isDomainClass="true">
                <ECProperty propertyName="A1" typeName="string"/>
                <ECProperty propertyName="A2" typeName="string"/>
            </ECClass>
        </ECSchema>
    )xml";
    verifyHiddenSchemaAppliedCorrectly(schemaXml3);

    // This is imperfect but follows how presentation rules handles the legacy CA.
    Utf8CP schemaXml4 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0" description="Hide">
            <ECSchemaReference name="Bentley_Standard_CustomAttributes" version="1.13" prefix="bsca"/>
            <ECCustomAttributes>
                <DisplayOptions xmlns="Bentley_Standard_CustomAttributes.01.13">
                    <HideInstances>true</HideInstances>
                    <Hidden>false</Hidden>
                </DisplayOptions>
            </ECCustomAttributes>
            <ECClass typeName="A" isDomainClass="true">
                <ECProperty propertyName="A1" typeName="string"/>
                <ECProperty propertyName="A2" typeName="string"/>
            </ECClass>
        </ECSchema>
    )xml";
    verifyHiddenSchemaAppliedCorrectly(schemaXml4);

    // This is imperfect but follows how presentation rules handles the legacy CA.
    Utf8CP schemaXml5 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0" description="Hide">
            <ECSchemaReference name="Bentley_Standard_CustomAttributes" version="1.13" prefix="bsca"/>
            <ECCustomAttributes>
                <DisplayOptions xmlns="Bentley_Standard_CustomAttributes.01.13">
                    <HideInstances>false</HideInstances>
                    <Hidden>true</Hidden>
                </DisplayOptions>
            </ECCustomAttributes>
            <ECClass typeName="A" isDomainClass="true">
                <ECProperty propertyName="A1" typeName="string"/>
                <ECProperty propertyName="A2" typeName="string"/>
            </ECClass>
        </ECSchema>
    )xml";
    verifyHiddenSchemaAppliedCorrectly(schemaXml5);

    Utf8CP schemaXml6 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0" description="Show">
            <ECSchemaReference name="Bentley_Standard_CustomAttributes" version="1.13" prefix="bsca"/>
            <ECCustomAttributes>
                <DisplayOptions xmlns="Bentley_Standard_CustomAttributes.01.13">
                    <HideInstances>false</HideInstances>
                    <Hidden>false</Hidden>
                </DisplayOptions>
            </ECCustomAttributes>
            <ECClass typeName="A" isDomainClass="true">
                <ECProperty propertyName="A1" typeName="string"/>
                <ECProperty propertyName="A2" typeName="string"/>
            </ECClass>
        </ECSchema>
    )xml";
    verifyHiddenSchemaAppliedCorrectly(schemaXml6);

    // Hide related is ignored by presentation rules so we ignore it during conversion.
    Utf8CP schemaXml7 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0" description="Show">
            <ECSchemaReference name="Bentley_Standard_CustomAttributes" version="1.13" prefix="bsca"/>
            <ECCustomAttributes>
                <DisplayOptions xmlns="Bentley_Standard_CustomAttributes.01.13">
                    <HideInstances>false</HideInstances>
                    <Hidden>false</Hidden>
                    <HideRelated>true</HideRelated>
                </DisplayOptions>
            </ECCustomAttributes>
            <ECClass typeName="A" isDomainClass="true">
                <ECProperty propertyName="A1" typeName="string"/>
                <ECProperty propertyName="A2" typeName="string"/>
            </ECClass>
        </ECSchema>
    )xml";
    verifyHiddenSchemaAppliedCorrectly(schemaXml7);

    Utf8CP schemaXml8 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0" description="Show">
            <ECSchemaReference name="Bentley_Standard_CustomAttributes" version="1.13" prefix="bsca"/>
            <ECClass typeName="A" isDomainClass="true">
                <ECProperty propertyName="A1" typeName="string"/>
                <ECProperty propertyName="A2" typeName="string"/>
            </ECClass>
        </ECSchema>
    )xml";
    verifyHiddenSchemaAppliedCorrectly(schemaXml8);
    }

void verifyClassHidden(ECClassCP ecClass)
    {
    IECInstancePtr hiddenClassCA = ecClass->GetCustomAttribute("CoreCustomAttributes", "HiddenClass");
    ASSERT_TRUE(hiddenClassCA.IsValid()) << ecClass->GetName().c_str() << " should be hidden but does not have the 'HiddenClass' CA";
    ECValue showValue;
    EXPECT_EQ(ECObjectsStatus::Success, hiddenClassCA->GetValue(showValue, "Show"));
    EXPECT_TRUE(showValue.IsNull() || !showValue.GetBoolean()) << "Expected class '" << ecClass->GetName().c_str() <<
        "' to be hidden but the HiddenClass.Show value was set to true.";
    }

void verifyClassNotHidden(ECClassCP ecClass)
    {
    IECInstancePtr hiddenClassCA = ecClass->GetCustomAttribute("CoreCustomAttributes", "HiddenClass");
    if (hiddenClassCA.IsNull() || !hiddenClassCA.IsValid())
        return;

    ECValue showValue;
    EXPECT_EQ(ECObjectsStatus::Success, hiddenClassCA->GetValue(showValue, "Show"));
    EXPECT_TRUE(!showValue.IsNull() && showValue.GetBoolean()) << "Expected class '" << ecClass->GetName().c_str() <<
        "' to be shown but the HiddenClass.Show value was not set or set to false.";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Colin.Kerr                 06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeConversionTests, DisplayOptionsCustomAttribute_AppliedToClass)
    {
    // Class description defines if we expect the property to be shown or hidden
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="Bentley_Standard_CustomAttributes" version="1.13" prefix="bsca"/>
            <ECClass typeName="A" isDomainClass="true" description="Hide">
                <ECCustomAttributes>
                    <DisplayOptions xmlns="Bentley_Standard_CustomAttributes.01.13">
                        <Hidden>true</Hidden>
                        <HideInstances>true</HideInstances>
                    </DisplayOptions>
                </ECCustomAttributes>
            </ECClass>
            <ECClass typeName="B" isDomainClass="true" description="Show">
                <BaseClass>A</BaseClass>
                <ECCustomAttributes>
                    <DisplayOptions xmlns="Bentley_Standard_CustomAttributes.01.13">
                        <Hidden>false</Hidden>
                        <HideInstances>false</HideInstances>
                    </DisplayOptions>
                </ECCustomAttributes>
            </ECClass>
            <ECClass typeName="C" isDomainClass="true" description="Show">
                <BaseClass>A</BaseClass>
                <ECCustomAttributes>
                    <DisplayOptions xmlns="Bentley_Standard_CustomAttributes.01.13">
                        <Hidden>false</Hidden>
                    </DisplayOptions>
                </ECCustomAttributes>
            </ECClass>
            <ECClass typeName="D" isDomainClass="true" description="Show">
                <BaseClass>A</BaseClass>
                <ECCustomAttributes>
                    <DisplayOptions xmlns="Bentley_Standard_CustomAttributes.01.13">
                        <HideInstances>false</HideInstances>
                    </DisplayOptions>
                </ECCustomAttributes>
            </ECClass>
            <ECClass typeName="E" isDomainClass="true" description="Hide">
                <ECCustomAttributes>
                    <DisplayOptions xmlns="Bentley_Standard_CustomAttributes.01.13">
                        <Hidden>true</Hidden>
                        <HideInstances>true</HideInstances>
                        <HideRelated>false</HideRelated>
                    </DisplayOptions>
                </ECCustomAttributes>
            </ECClass>
        </ECSchema>
    )xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());
    EXPECT_EQ(1, schema->GetReferencedSchemas().size());

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema));
    verifySchemaReferencesOnlyCoreCAs(schema.get());

    for (const auto& ecClass : schema->GetClasses())
        {
        if (ecClass->GetDescription().Equals("Hide"))
            verifyClassHidden(ecClass);
        else if (ecClass->GetDescription().Equals("Show"))
            verifyClassNotHidden(ecClass);
        else
            EXPECT_TRUE(false) << ecClass->GetName().c_str() <<
            " class must specify 'Hide' or 'Show' in its description to identify it as a class that should be hidden or shown";
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                 06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipConversionTest, BaseClassHasConstraintClasses)
    {
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECClass typeName="A" isDomainClass="true"/>
                <ECClass typeName="B" isDomainClass="true"/>
                <ECRelationshipClass typeName="ARelB" isDomainClass="true" strength="referencing" strengthDirection="forward">
                    <Source cardinality="(1,1)" polymorphic="true">
                        <Class class="A"/>
                    </Source>
                    <Target cardinality="(1,1)" polymorphic="true">
                        <Class class="B"/>
                    </Target>
                </ECRelationshipClass>
                <ECRelationshipClass typeName="DerivedARelB" isDomainClass="true" strength="referencing" strengthDirection="forward">
                    <BaseClass>ARelB</BaseClass>
                    <Source cardinality="(1,1)" polymorphic="true">
                    </Source>
                    <Target cardinality="(1,1)" polymorphic="true">
                    </Target>
                </ECRelationshipClass>
            </ECSchema>
        )xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema));
    ECClassCP ecClass = schema->GetClassCP("DerivedARelB");
    ECRelationshipClassCP derivedRelClass = ecClass->GetRelationshipClassCP();
    EXPECT_EQ(1, derivedRelClass->GetSource().GetConstraintClasses().size());
    EXPECT_STREQ("A", derivedRelClass->GetSource().GetConstraintClasses()[0]->GetName().c_str());
    EXPECT_EQ(1, derivedRelClass->GetTarget().GetConstraintClasses().size());
    EXPECT_STREQ("B", derivedRelClass->GetTarget().GetConstraintClasses()[0]->GetName().c_str());
    }
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECClass typeName="A" isDomainClass="true"/>
                <ECClass typeName="B" isDomainClass="true"/>
                <ECClass typeName="C" isDomainClass="true">
                    <BaseClass>A</BaseClass>
                </ECClass>
                <ECClass typeName="D" isDomainClass="true">
                    <BaseClass>B</BaseClass>
                </ECClass>
                <ECRelationshipClass typeName="ARelB" isDomainClass="true" strength="referencing" strengthDirection="forward">
                    <Source cardinality="(1,1)" polymorphic="true">
                        <Class class="A"/>
                        <Class class="C"/>
                    </Source>
                    <Target cardinality="(1,1)" polymorphic="true">
                        <Class class="B"/>
                        <Class class="D"/>
                    </Target>
                </ECRelationshipClass>
                <ECRelationshipClass typeName="DerivedARelB" isDomainClass="true" strength="referencing" strengthDirection="forward">
                    <BaseClass>ARelB</BaseClass>
                    <Source cardinality="(1,1)" polymorphic="true">
                    </Source>
                    <Target cardinality="(1,1)" polymorphic="true">
                    </Target>
                </ECRelationshipClass>
            </ECSchema>
        )xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema));
    ECClassCP ecClass = schema->GetClassCP("DerivedARelB");
    ECRelationshipClassCP derivedRelClass = ecClass->GetRelationshipClassCP();
    EXPECT_EQ(2, derivedRelClass->GetSource().GetConstraintClasses().size());
    EXPECT_STREQ("A", derivedRelClass->GetSource().GetAbstractConstraint()->GetName().c_str());
    EXPECT_EQ(2, derivedRelClass->GetTarget().GetConstraintClasses().size());
    EXPECT_STREQ("B", derivedRelClass->GetTarget().GetAbstractConstraint()->GetName().c_str());
    }
    }


void validateClassMapConvertedCorrectly(Utf8CP schemaXml, bool expectSuccess, Utf8CP expectedMappingStrategy)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext(true, false);
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    BeFileName ecdbSchemaDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(ecdbSchemaDir);
    ecdbSchemaDir.AppendToPath(L"SeedData");
    context->AddSchemaPath(ecdbSchemaDir);

    CustomECSchemaConverterPtr schemaConverter = CustomECSchemaConverter::Create();
    schemaConverter->AddSchemaReadContext(*context);
    IECCustomAttributeConverterPtr classMapConverter = new ECDbClassMapConverter();
    schemaConverter->AddConverter(ECDbClassMapConverter::GetSchemaName(), ECDbClassMapConverter::GetClassName(), classMapConverter);

    schemaConverter->Convert(*schema); // Converter doesn't return an error when it hits a mapping strategy it doesn't understand
    IECInstancePtr classMapCA = schema->GetClassCP("C")->GetCustomAttribute("ClassMap");
    ASSERT_EQ(expectSuccess, classMapCA.IsValid());
    if (expectSuccess)
        {
        EXPECT_EQ(2, classMapCA->GetClass().GetSchema().GetVersionRead());
        ECValue actualMappingStrategy;
        ASSERT_EQ(ECObjectsStatus::Success, classMapCA->GetValue(actualMappingStrategy, "MapStrategy"));
        EXPECT_STREQ(expectedMappingStrategy, actualMappingStrategy.GetUtf8CP());
    
        EXPECT_EQ(1, schema->GetReferencedSchemas().size()) << "Expected only one schema reference on success";
        Utf8String schemaReferenceFullName = schema->GetReferencedSchemas().begin()->second->GetFullSchemaName();
        EXPECT_STREQ("ECDbMap.02.00.00", schemaReferenceFullName.c_str()) <<
            "The only referenced schema does not have the correct name and version.";
        }
    else
        EXPECT_EQ(0, schema->GetReferencedSchemas().size()) << "Expected no schema references on failure";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Colin.Kerr                  01/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingConversionTests, ClassMap_SharedTableToTablePerHierarchy)
    {
    Utf8CP schemaXmlCanConvert0 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECSchemaReference name="ECDbMap" version="1.0" prefix="ecdbmap"/>
                <ECClass typeName="C" isDomainClass="true">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.01.00">
                            <MapStrategy>
                                <Strategy>SharedTable</Strategy>
                                <AppliesToSubclasses>True</AppliesToSubclasses>
                            </MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                </ECClass>
            </ECSchema>
        )xml";

    Utf8CP schemaXmlCanConvert1 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECSchemaReference name="ECDbMap" version="1.1" prefix="ecdbmap"/>
                <ECClass typeName="C" isDomainClass="true">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.01.01">
                            <MapStrategy>
                                <Strategy>SharedTable</Strategy>
                                <AppliesToSubclasses>True</AppliesToSubclasses>
                            </MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                </ECClass>
            </ECSchema>
        )xml";

    Utf8CP schemaXmlCanNotConvert0 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECSchemaReference name="ECDbMap" version="1.0" prefix="ecdbmap"/>
                <ECClass typeName="C" isDomainClass="true">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.01.00">
                            <MapStrategy>
                                <Strategy>SharedTable</Strategy>
                                <AppliesToSubclasses>False</AppliesToSubclasses>
                            </MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                </ECClass>
            </ECSchema>
        )xml";

    Utf8CP schemaXmlCanNotConvert1 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECSchemaReference name="ECDbMap" version="1.0" prefix="ecdbmap"/>
                <ECClass typeName="C" isDomainClass="true">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.01.00">
                            <MapStrategy>
                                <Strategy>SharedTable</Strategy>
                            </MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                </ECClass>
            </ECSchema>
        )xml";

    validateClassMapConvertedCorrectly(schemaXmlCanConvert0, true, "TablePerHierarchy");
    validateClassMapConvertedCorrectly(schemaXmlCanConvert1, true, "TablePerHierarchy");
    validateClassMapConvertedCorrectly(schemaXmlCanNotConvert0, false, "");
    validateClassMapConvertedCorrectly(schemaXmlCanNotConvert1, false, "");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Colin.Kerr                  01/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingConversionTests, ClassMap_NotMapped)
    {
    Utf8CP schemaXmlCanConvert = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECSchemaReference name="ECDbMap" version="1.0" prefix="ecdbmap"/>
                <ECClass typeName="C" isDomainClass="true">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.01.00">
                            <MapStrategy>
                                <Strategy>NotMapped</Strategy>
                            </MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                </ECClass>
            </ECSchema>
        )xml";

    Utf8CP schemaXmlCanNotConvert0 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECSchemaReference name="ECDbMap" version="1.0" prefix="ecdbmap"/>
                <ECClass typeName="C" isDomainClass="true">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.01.00">
                            <MapStrategy>
                                <Strategy>Banana</Strategy>
                            </MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                </ECClass>
            </ECSchema>
        )xml";

    validateClassMapConvertedCorrectly(schemaXmlCanConvert, true, "NotMapped");
    validateClassMapConvertedCorrectly(schemaXmlCanNotConvert0, false, "");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Colin.Kerr                  01/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeConversionTests, RemovingStandardCustomAttributesIsDisabledByDefaultInCustomConverter)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECSchemaReference name="Bentley_Standard_CustomAttributes" version="1.13" prefix="bsca"/>
                <ECClass typeName="C" isDomainClass="true">
                    <ECProperty propertyName="AppStartDate" typeName="dateTime" displayLabel="Application Start Date">
                        <ECCustomAttributes>
                            <DateTimeInfo xmlns="Bentley_Standard_CustomAttributes.01.13">
                                <DateTimeKind>Utc</DateTimeKind>
                            </DateTimeInfo>
                        </ECCustomAttributes>
                    </ECProperty>
                </ECClass>
            </ECSchema>
        )xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    auto converter = CustomECSchemaConverter::Create();
    ASSERT_EQ(true, converter->Convert(*schema, true));
    EXPECT_EQ(1, schema->GetReferencedSchemas().size());
    EXPECT_TRUE(schema->GetClassP("C")->GetPropertyP("AppStartDate")->GetCustomAttribute("DateTimeInfo").IsValid()) << "DateTimeInfo CA should not have been removed";
    }

END_BENTLEY_ECN_TEST_NAMESPACE