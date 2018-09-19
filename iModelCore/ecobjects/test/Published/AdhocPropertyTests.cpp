/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/AdhocPropertyTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

#include <ECObjects/ECInstance.h>
#include <ECObjects/StandaloneECInstance.h>
#include <ECObjects/ECValue.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
struct AdHocPropertyTest : ECTestFixture
    {
    protected:
        ECSchemaPtr         m_schema;
        ECSchemaReadContextPtr context;

        Utf8CP const s_schemaXml =
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
            "   <ECStructClass typeName = 'AdHocHolder'>"
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
            "   <ECEntityClass typeName = 'NoAdHocs'>"
            "       <ECProperty propertyName = 'NotAdHoc' typeName = 'string' />"
            "   </ECEntityClass>"
            "   <ECEntityClass typeName = 'AdHocs'>"
            "       <ECCustomAttributes>"
            "           <AdhocPropertySpecification xmlns='Bentley_Standard_CustomAttributes.01.10'>"
            "               <AdHocPropertyContainer>AdHocHolder</AdHocPropertyContainer>"
            "           </AdhocPropertySpecification>"
            "       </ECCustomAttributes>"
            "       <ECStructArrayProperty propertyName = 'AdHocHolder' typeName = 'AdHocHolder' />"
            "   </ECEntityClass>"
            "</ECSchema>";

        void SetUp()
            {
            ECTestFixture::SetUp();
            context = ECSchemaReadContext::CreateContext();
            EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(m_schema, s_schemaXml, *context));
            }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(AdHocPropertyTest, SwapAdHocProperties)
    {
    IECInstancePtr instance = m_schema->GetClassP("adHocs")->GetDefaultStandaloneEnabler()->CreateInstance();
    AdHocPropertyEdit adHocs(*instance, "AdHocHolder");
    EXPECT_TRUE(adHocs.IsSupported());
    EXPECT_EQ(0, adHocs.GetCount());

    //Add 1st AdHoc Property
    ECValue v;
    uint32_t propertyOneIdx;
    v.SetUtf8CP("property one", false);
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.Add("propertyone", v, "Property One"));
    EXPECT_EQ(1, adHocs.GetCount());
    EXPECT_TRUE(adHocs.GetPropertyIndex(propertyOneIdx, "propertyone"));

    //test metadata
    bool isReadOnly;
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.IsReadOnly(isReadOnly, propertyOneIdx));
    EXPECT_FALSE(isReadOnly);

    bool isHidden;
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.IsHidden(isHidden, propertyOneIdx));
    EXPECT_FALSE(isHidden);

    Utf8String str;
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.GetDisplayLabel(str, propertyOneIdx));
    EXPECT_TRUE(str.Equals("Property One"));
    PrimitiveType type;
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.GetPrimitiveType(type, propertyOneIdx));
    EXPECT_TRUE(PRIMITIVETYPE_String == type);
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.GetUnitName(str, propertyOneIdx));
    EXPECT_TRUE(str.empty());
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.GetExtendedTypeName(str, propertyOneIdx));
    EXPECT_TRUE(str.empty());

    //test value
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.GetValue(v, propertyOneIdx));
    EXPECT_EQ(0, strcmp(v.GetUtf8CP(), "property one"));

    //test value from instance
    EXPECT_EQ(ECObjectsStatus::PropertyNotFound, instance->GetValue(v, "propertyone"));
    EXPECT_EQ(ECObjectsStatus::Success, instance->GetValueOrAdHoc(v, "propertyone"));    // include ad-hoc properties
    EXPECT_EQ(0, strcmp(v.GetUtf8CP(), "property one"));

    //Add 2nd AdHoc Property
    ECValue v1;
    uint32_t propertyTwoIdx;
    v1.SetUtf8CP("property", false);
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.Add("property2", v));
    EXPECT_EQ(2, adHocs.GetCount());
    EXPECT_TRUE(adHocs.GetPropertyIndex(propertyTwoIdx, "property2"));

    //set additional metadata
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.SetIsReadOnly(propertyTwoIdx, true));
    EXPECT_EQ(ECObjectsStatus::OperationNotSupported, adHocs.SetIsHidden(propertyTwoIdx, false));//cross check why ishidden can't be set
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.SetDisplayLabel(propertyTwoIdx, "Property Two"));
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.SetName(propertyTwoIdx, "propertytwo"));
    v1.SetUtf8CP("property two", false);
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.SetValue(propertyTwoIdx, v1));

    //Swap properties based on index
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.Swap(propertyOneIdx, propertyTwoIdx));

    //verify display label after swap
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.GetDisplayLabel(str, propertyOneIdx));
    EXPECT_TRUE(str.Equals("Property Two"));

    //remove AdHoc property
    EXPECT_EQ(2, adHocs.GetCount());
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.Remove(propertyOneIdx));
    EXPECT_EQ(1, adHocs.GetCount());
    EXPECT_FALSE(adHocs.GetPropertyIndex(propertyOneIdx, "time"));
    EXPECT_TRUE(adHocs.GetPropertyIndex(propertyOneIdx, "propertyone"));

    //remove all AdHoc properties
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.Clear());
    EXPECT_EQ(0, adHocs.GetCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(AdHocPropertyTest, InvalidCases)
    {
    IECInstancePtr noAdHocs = m_schema->GetClassP("NoAdHocs")->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_FALSE(AdHocPropertyQuery(*noAdHocs, "NONE").IsSupported());

    IECInstancePtr instance = m_schema->GetClassP("AdHocs")->GetDefaultStandaloneEnabler()->CreateInstance();
    AdHocPropertyEdit adHocs(*instance, "AdHocHolder");
    EXPECT_TRUE(adHocs.IsSupported());
    EXPECT_EQ(0, adHocs.GetCount());

    //Try to find non-existent AdHoc
    uint32_t propIdx;
    EXPECT_FALSE(adHocs.GetPropertyIndex(propIdx, "DoesNotExist"));

    //Name must be valid
    ECValue v;
    EXPECT_EQ(ECObjectsStatus::Error, adHocs.Add(nullptr, v));
    EXPECT_EQ(ECObjectsStatus::Error, adHocs.Add("Not a Valid EC Name", v));

    //Value must be null or primitive
    v.SetStruct(m_schema->GetClassP("AdHocHolder")->GetDefaultStandaloneEnabler()->CreateInstance().get());
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, adHocs.Add("Struct", v));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(AdHocPropertyTest, AddPropertyWithOptionalMetadata)
    {
    IECInstancePtr instance = m_schema->GetClassP("AdHocs")->GetDefaultStandaloneEnabler()->CreateInstance();
    AdHocPropertyEdit adHocs(*instance, "AdHocHolder");
    EXPECT_TRUE(adHocs.IsSupported());
    EXPECT_EQ(0, adHocs.GetCount());

    // add with additional optional metadata
    ECValue v;
    uint32_t propIdx;

    v.SetDouble(1234.0);
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.Add("DoorHeight", v, "Door Height", "Meters", "Distance", true, false));
    EXPECT_EQ(1, adHocs.GetCount());

    EXPECT_TRUE(adHocs.GetPropertyIndex(propIdx, "DoorHeight"));

    Utf8String str;
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.GetName(str, propIdx));
    EXPECT_TRUE(str.Equals("DoorHeight"));

    EXPECT_EQ(ECObjectsStatus::Success, adHocs.GetDisplayLabel(str, propIdx));
    EXPECT_TRUE(str.Equals("Door Height"));

    PrimitiveType type;
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.GetPrimitiveType(type, propIdx));
    EXPECT_TRUE(PRIMITIVETYPE_Double == type);

    EXPECT_EQ(ECObjectsStatus::Success, adHocs.GetExtendedTypeName(str, propIdx));
    EXPECT_TRUE(str.Equals("Distance"));

    EXPECT_EQ(ECObjectsStatus::Success, adHocs.GetUnitName(str, propIdx));
    EXPECT_TRUE(str.Equals("Meters"));

    bool isReadOnly;
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.IsReadOnly(isReadOnly, propIdx));
    EXPECT_TRUE(isReadOnly);

    bool isHidden;
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.IsHidden(isHidden, propIdx));
    EXPECT_FALSE(isHidden);

    EXPECT_EQ(ECObjectsStatus::Success, adHocs.GetValue(v, propIdx));
    EXPECT_TRUE(v.IsDouble());
    EXPECT_EQ(v.GetDouble(), 1234.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(AdHocPropertyTest, VerifyReadOnlyAdHocProperty)
    {
    IECInstancePtr instance = m_schema->GetClassP("AdHocs")->GetDefaultStandaloneEnabler()->CreateInstance();
    AdHocPropertyEdit adHocs(*instance, "AdHocHolder");
    EXPECT_TRUE(adHocs.IsSupported());
    EXPECT_EQ(0, adHocs.GetCount());

    // add with additional optional metadata
    ECValue v;
    uint32_t propIdx;

    v.SetDouble(1234.0);
    v.SetIsReadOnly(true);
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.Add("DoorHeight", v, "Door Height", "Meters", "Distance", true));
    EXPECT_EQ(1, adHocs.GetCount());
    EXPECT_TRUE(v.IsReadOnly());

    EXPECT_TRUE(adHocs.GetPropertyIndex(propIdx, "DoorHeight"));

    //Property is read-only
    v.SetDouble(5678.0);
    EXPECT_TRUE(instance->IsPropertyOrAdHocReadOnly("DoorHeight"));
    EXPECT_EQ(ECObjectsStatus::PropertyNotFound, instance->SetValue("DoorHeight", v));
    EXPECT_EQ(ECObjectsStatus::UnableToSetReadOnlyProperty, instance->SetValueOrAdHoc("DoorHeight", v));
    EXPECT_EQ(ECObjectsStatus::PropertyNotFound, instance->ChangeValue("DoorHeight", v));
    EXPECT_EQ(ECObjectsStatus::UnableToSetReadOnlyProperty, instance->ChangeValueOrAdHoc("DoorHeight", v));

    //can set read-only property through AdHoc API
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.SetValue(propIdx, v));
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.GetValue(v, propIdx));
    EXPECT_EQ(v.GetDouble(), 5678.0);
    EXPECT_FALSE(v.IsReadOnly());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(AdHocPropertyTest, GetAdHocPropertyUsingECValueAccessor)
    {
    IECInstancePtr instance = m_schema->GetClassP("AdHocs")->GetDefaultStandaloneEnabler()->CreateInstance();
    AdHocPropertyEdit adHocs(*instance, "AdHocHolder");
    EXPECT_TRUE(adHocs.IsSupported());
    EXPECT_EQ(0, adHocs.GetCount());

    //Add 1st AdHoc Property
    ECValue v;
    uint32_t propertyOneIdx;
    v.SetUtf8CP("property one", false);
    EXPECT_EQ(ECObjectsStatus::Success, adHocs.Add("propertyone", v, "Property One"));
    EXPECT_EQ(1, adHocs.GetCount());
    EXPECT_TRUE(adHocs.GetPropertyIndex(propertyOneIdx, "propertyone"));

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
