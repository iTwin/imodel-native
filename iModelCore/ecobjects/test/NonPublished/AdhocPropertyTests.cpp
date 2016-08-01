/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/AdhocPropertyTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

#include <ECObjects/ECInstance.h>
#include <ECObjects/StandaloneECInstance.h>
#include <ECObjects/ECValue.h>

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
struct AdhocPropertyTest : ECTestFixture
    {
    protected:
        ECSchemaPtr         m_schema;
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();

        Utf8CP const s_schemaXml =
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
            "   <ECStructClass typeName = 'AdhocHolder'>"
            "       <ECCustomAttributes>"
            "           <AdhocPropertyContainerDefinition xmlns='Bentley_Standard_CustomAttributes.01.10'>"
            "               <NameProperty>Name</NameProperty>"
            "               <DisplayLabelProperty>Label</DisplayLabelProperty>"
            "               <ValueProperty>Value</ValueProperty>"
            "               <TypeProperty>Type</TypeProperty>"
            "               <UnitProperty>Unit</UnitProperty>"
            "               <ExtendTypeProperty>ExtendType</ExtendTypeProperty>"
            "               <IsReadOnlyProperty>IsReadOnly</IsReadOnlyProperty>"
            "           </AdhocPropertyContainerDefinition>"
            "       </ECCustomAttributes>"
            "       <ECProperty propertyName='Name' typeName='string' />"
            "       <ECProperty propertyName='Label' typeName='string' />"
            "       <ECProperty propertyName='Value' typeName='string' />"
            "       <ECProperty propertyName='Type' typeName='int' />"
            "       <ECProperty propertyName='Unit' typeName='string' />"
            "       <ECProperty propertyName='ExtendType' typeName='string' />"
            "       <ECProperty propertyName='IsReadOnly' typeName='boolean' />"
            "   </ECStructClass>"
            "   <ECEntityClass typeName = 'NoAdhocs'>"
            "       <ECProperty propertyName = 'NotAdhoc' typeName = 'string' />"
            "   </ECEntityClass>"
            "   <ECEntityClass typeName = 'Adhocs'>"
            "       <ECCustomAttributes>"
            "           <AdhocPropertySpecification xmlns='Bentley_Standard_CustomAttributes.01.10'>"
            "               <AdhocPropertyContainer>AdhocHolder</AdhocPropertyContainer>"
            "           </AdhocPropertySpecification>"
            "       </ECCustomAttributes>"
            "       <ECStructArrayProperty propertyName = 'AdhocHolder' typeName = 'AdhocHolder' />"
            "   </ECEntityClass>"
            "</ECSchema>";

        void SetUp()
            {
            EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(m_schema, s_schemaXml, *context));
            }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(AdhocPropertyTest, SwapAdhocProperties)
    {
    IECInstancePtr instance = m_schema->GetClassP("Adhocs")->GetDefaultStandaloneEnabler()->CreateInstance();
    AdhocPropertyEdit adhocs(*instance, "AdhocHolder");
    EXPECT_TRUE(adhocs.IsSupported());
    EXPECT_EQ(0, adhocs.GetCount());

    //Add 1st adhoc Property
    ECValue v;
    uint32_t propertyOneIdx;
    v.SetUtf8CP("property one", false);
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.Add("propertyone", v, "Property One"));
    EXPECT_EQ(1, adhocs.GetCount());
    EXPECT_TRUE(adhocs.GetPropertyIndex(propertyOneIdx, "propertyone"));

    //test metadata
    bool isReadOnly;
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.IsReadOnly(isReadOnly, propertyOneIdx));
    EXPECT_FALSE(isReadOnly);

    bool isHidden;
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.IsHidden(isHidden, propertyOneIdx));
    EXPECT_FALSE(isHidden);

    Utf8String str;
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.GetDisplayLabel(str, propertyOneIdx));
    EXPECT_TRUE(str.Equals("Property One"));
    PrimitiveType type;
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.GetPrimitiveType(type, propertyOneIdx));
    EXPECT_TRUE(PRIMITIVETYPE_String == type);
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.GetUnitName(str, propertyOneIdx));
    EXPECT_TRUE(str.empty());
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.GetExtendedTypeName(str, propertyOneIdx));
    EXPECT_TRUE(str.empty());

    //test value
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.GetValue(v, propertyOneIdx));
    EXPECT_EQ(0, strcmp(v.GetUtf8CP(), "property one"));

    //test value from instance
    EXPECT_EQ(ECObjectsStatus::PropertyNotFound, instance->GetValue(v, "propertyone"));
    EXPECT_EQ(ECObjectsStatus::Success, instance->GetValueOrAdhoc(v, "propertyone"));    // include ad-hoc properties
    EXPECT_EQ(0, strcmp(v.GetUtf8CP(), "property one"));

    //Add 2nd adhoc Property
    ECValue v1;
    uint32_t propertyTwoIdx;
    v1.SetUtf8CP("property", false);
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.Add("property2", v));
    EXPECT_EQ(2, adhocs.GetCount());
    EXPECT_TRUE(adhocs.GetPropertyIndex(propertyTwoIdx, "property2"));

    //set additional metadata
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.SetIsReadOnly(propertyTwoIdx, true));
    EXPECT_EQ(ECObjectsStatus::OperationNotSupported, adhocs.SetIsHidden(propertyTwoIdx, false));//cross check why ishidden can't be set
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.SetDisplayLabel(propertyTwoIdx, "Property Two"));
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.SetName(propertyTwoIdx, "propertytwo"));
    v1.SetUtf8CP("property two", false);
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.SetValue(propertyTwoIdx, v1));

    //Swap properties based on index
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.Swap(propertyOneIdx, propertyTwoIdx));

    //verify display label after swap
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.GetDisplayLabel(str, propertyOneIdx));
    EXPECT_TRUE(str.Equals("Property Two"));

    //remove adhoc property
    EXPECT_EQ(2, adhocs.GetCount());
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.Remove(propertyOneIdx));
    EXPECT_EQ(1, adhocs.GetCount());
    EXPECT_FALSE(adhocs.GetPropertyIndex(propertyOneIdx, "time"));
    EXPECT_TRUE(adhocs.GetPropertyIndex(propertyOneIdx, "propertyone"));

    //remove all adhoc properties
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.Clear());
    EXPECT_EQ(0, adhocs.GetCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(AdhocPropertyTest, InvalidCases)
    {
    IECInstancePtr noAdhocs = m_schema->GetClassP("NoAdhocs")->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_FALSE(AdhocPropertyQuery(*noAdhocs, "NONE").IsSupported());

    IECInstancePtr instance = m_schema->GetClassP("Adhocs")->GetDefaultStandaloneEnabler()->CreateInstance();
    AdhocPropertyEdit adhocs(*instance, "AdhocHolder");
    EXPECT_TRUE(adhocs.IsSupported());
    EXPECT_EQ(0, adhocs.GetCount());

    //Try to find non-existent adhoc
    uint32_t propIdx;
    EXPECT_FALSE(adhocs.GetPropertyIndex(propIdx, "DoesNotExist"));

    //Name must be valid
    ECValue v;
    EXPECT_EQ(ECObjectsStatus::Error, adhocs.Add(nullptr, v));
    EXPECT_EQ(ECObjectsStatus::Error, adhocs.Add("Not a Valid EC Name", v));

    //Value must be null or primitive
    v.SetStruct(m_schema->GetClassP("AdhocHolder")->GetDefaultStandaloneEnabler()->CreateInstance().get());
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, adhocs.Add("Struct", v));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(AdhocPropertyTest, AddPropertyWithOptionalMetadata)
    {
    IECInstancePtr instance = m_schema->GetClassP("Adhocs")->GetDefaultStandaloneEnabler()->CreateInstance();
    AdhocPropertyEdit adhocs(*instance, "AdhocHolder");
    EXPECT_TRUE(adhocs.IsSupported());
    EXPECT_EQ(0, adhocs.GetCount());

    // add with additional optional metadata
    ECValue v;
    uint32_t propIdx;

    v.SetDouble(1234.0);
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.Add("DoorHeight", v, "Door Height", "Meters", "Distance", true, false));
    EXPECT_EQ(1, adhocs.GetCount());

    EXPECT_TRUE(adhocs.GetPropertyIndex(propIdx, "DoorHeight"));

    Utf8String str;
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.GetName(str, propIdx));
    EXPECT_TRUE(str.Equals("DoorHeight"));

    EXPECT_EQ(ECObjectsStatus::Success, adhocs.GetDisplayLabel(str, propIdx));
    EXPECT_TRUE(str.Equals("Door Height"));

    PrimitiveType type;
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.GetPrimitiveType(type, propIdx));
    EXPECT_TRUE(PRIMITIVETYPE_Double == type);

    EXPECT_EQ(ECObjectsStatus::Success, adhocs.GetExtendedTypeName(str, propIdx));
    EXPECT_TRUE(str.Equals("Distance"));

    EXPECT_EQ(ECObjectsStatus::Success, adhocs.GetUnitName(str, propIdx));
    EXPECT_TRUE(str.Equals("Meters"));

    bool isReadOnly;
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.IsReadOnly(isReadOnly, propIdx));
    EXPECT_TRUE(isReadOnly);

    bool isHidden;
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.IsHidden(isHidden, propIdx));
    EXPECT_FALSE(isHidden);

    EXPECT_EQ(ECObjectsStatus::Success, adhocs.GetValue(v, propIdx));
    EXPECT_TRUE(v.IsDouble());
    EXPECT_EQ(v.GetDouble(), 1234.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(AdhocPropertyTest, VerifyReadOnlyAdhocProperty)
    {
    IECInstancePtr instance = m_schema->GetClassP("Adhocs")->GetDefaultStandaloneEnabler()->CreateInstance();
    AdhocPropertyEdit adhocs(*instance, "AdhocHolder");
    EXPECT_TRUE(adhocs.IsSupported());
    EXPECT_EQ(0, adhocs.GetCount());

    // add with additional optional metadata
    ECValue v;
    uint32_t propIdx;

    v.SetDouble(1234.0);
    v.SetIsReadOnly(true);
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.Add("DoorHeight", v, "Door Height", "Meters", "Distance", true));
    EXPECT_EQ(1, adhocs.GetCount());
    EXPECT_TRUE(v.IsReadOnly());

    EXPECT_TRUE(adhocs.GetPropertyIndex(propIdx, "DoorHeight"));

    //Property is read-only
    v.SetDouble(5678.0);
    EXPECT_TRUE(instance->IsPropertyOrAdhocReadOnly("DoorHeight"));
    EXPECT_EQ(ECObjectsStatus::PropertyNotFound, instance->SetValue("DoorHeight", v));
    EXPECT_EQ(ECObjectsStatus::UnableToSetReadOnlyProperty, instance->SetValueOrAdhoc("DoorHeight", v));
    EXPECT_EQ(ECObjectsStatus::PropertyNotFound, instance->ChangeValue("DoorHeight", v));
    EXPECT_EQ(ECObjectsStatus::UnableToSetReadOnlyProperty, instance->ChangeValueOrAdhoc("DoorHeight", v));

    //can set read-only property through adhoc API
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.SetValue(propIdx, v));
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.GetValue(v, propIdx));
    EXPECT_EQ(v.GetDouble(), 5678.0);
    EXPECT_FALSE(v.IsReadOnly());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(AdhocPropertyTest, GetAdhocPropertyUsingECValueAccessor)
    {
    IECInstancePtr instance = m_schema->GetClassP("Adhocs")->GetDefaultStandaloneEnabler()->CreateInstance();
    AdhocPropertyEdit adhocs(*instance, "AdhocHolder");
    EXPECT_TRUE(adhocs.IsSupported());
    EXPECT_EQ(0, adhocs.GetCount());

    //Add 1st adhoc Property
    ECValue v;
    uint32_t propertyOneIdx;
    v.SetUtf8CP("property one", false);
    EXPECT_EQ(ECObjectsStatus::Success, adhocs.Add("propertyone", v, "Property One"));
    EXPECT_EQ(1, adhocs.GetCount());
    EXPECT_TRUE(adhocs.GetPropertyIndex(propertyOneIdx, "propertyone"));

    // Test accessing ad-hocs using ECValueAccessor
    ECValueAccessor va;
    EXPECT_EQ(ECObjectsStatus::PropertyNotFound, ECValueAccessor::PopulateValueAccessor(va, instance->GetEnabler(), "propertyone"));
    EXPECT_EQ(ECObjectsStatus::PropertyNotFound, ECValueAccessor::PopulateValueAccessor(va, *instance, "propertyone", false));
    EXPECT_EQ(ECObjectsStatus::Success, ECValueAccessor::PopulateValueAccessor(va, *instance, "propertyone", true));

    EXPECT_EQ(ECObjectsStatus::Success, instance->SetValueUsingAccessor(va, ECValue("set using accessor", false)));
    EXPECT_EQ(ECObjectsStatus::Success, instance->GetValueUsingAccessor(v, va));
    EXPECT_EQ(0, strcmp(v.GetUtf8CP(), "set using accessor"));
    }

END_BENTLEY_ECN_TEST_NAMESPACE
