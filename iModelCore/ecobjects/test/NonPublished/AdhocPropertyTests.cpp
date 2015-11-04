/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/AdhocPropertyTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

#include <ECObjects/ECInstance.h>
#include <ECObjects/StandaloneECInstance.h>
#include <ECObjects/ECValue.h>

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE


#define EXPECT_STATUS(STATUS, EXPR) EXPECT_EQ (ECObjectsStatus:: ## STATUS , (EXPR))
#define EXPECT_SUCCESS(EXPR) EXPECT_STATUS (Success, (EXPR))

static const WCharCP s_schemaXml =
    L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    L"<ECSchema schemaName=\"AdhocSchema\" nameSpacePrefix=\"adhoc\" version=\"01.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        L"<ECSchemaReference name=\"Bentley_Standard_CustomAttributes\" version=\"01.10\" prefix=\"besc\" />"
        L"<ECClass typeName=\"AdhocHolder\" isStruct=\"True\">"
            L"<ECCustomAttributes>"
                L"<AdhocPropertyContainerDefinition xmlns=\"Bentley_Standard_CustomAttributes.01.10\">"
                    L"<NameProperty>Name</NameProperty>"
                    L"<DisplayLabelProperty>Label</DisplayLabelProperty>"
                    L"<ValueProperty>Value</ValueProperty>"
                    L"<TypeProperty>Type</TypeProperty>"
                    L"<UnitProperty>Unit</UnitProperty>"
                    L"<ExtendTypeProperty>ExtendType</ExtendTypeProperty>"
                    L"<IsReadOnlyProperty>IsReadOnly</IsReadOnlyProperty>"
                L"</AdhocPropertyContainerDefinition>"
            L"</ECCustomAttributes>"
            L"<ECProperty propertyName=\"Name\" typeName=\"string\" />"
            L"<ECProperty propertyName=\"Label\" typeName=\"string\" />"
            L"<ECProperty propertyName=\"Value\" typeName=\"string\" />"
            L"<ECProperty propertyName=\"Type\" typeName=\"int\" />"
            L"<ECProperty propertyName=\"Unit\" typeName=\"string\" />"
            L"<ECProperty propertyName=\"ExtendType\" typeName=\"string\" />"
            L"<ECProperty propertyName=\"IsReadOnly\" typeName=\"boolean\" />"
        L"</ECClass>"
        L"<ECClass typeName=\"NoAdhocs\" isStruct=\"False\" isDomainClass=\"True\">"
            L"<ECProperty propertyName=\"NotAdhoc\" typeName=\"string\" />"
        L"</ECClass>"
        L"<ECClass typeName=\"Adhocs\" isStruct=\"False\" isDomainClass=\"True\">"
            L"<ECCustomAttributes>"
                L"<AdhocPropertySpecification xmlns=\"Bentley_Standard_CustomAttributes.01.10\">"
                    L"<AdhocPropertyContainer>AdhocHolder</AdhocPropertyContainer>"
                L"</AdhocPropertySpecification>"
            L"</ECCustomAttributes>"
            L"<BaseClass>NoAdhocs</BaseClass>"
            L"<ECArrayProperty propertyName=\"AdhocHolder\" typeName=\"AdhocHolder\" isStruct=\"True\" />"
        L"</ECClass>"
    L"</ECSchema>";

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
struct AdhocPropertyTest : ECTestFixture
    {
    ECSchemaPtr         m_schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext ();
    };

/*---------------------------------------------------------------------------------**//**
* Based on ecf\ecobjects\atp\AdhocAtp.cs
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AdhocPropertyTest, AdhocInterface)
    {
    Utf8String schemaXml (s_schemaXml);
    EXPECT_EQ (SchemaReadStatus::Success, ECSchema::ReadFromXmlString (m_schema, schemaXml.c_str(), *context));

    auto noAdhocs = m_schema->GetClassP ("NoAdhocs")->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_FALSE (AdhocPropertyQuery (*noAdhocs, "NONE").IsSupported());

    auto instance = m_schema->GetClassP ("Adhocs")->GetDefaultStandaloneEnabler()->CreateInstance();
    AdhocPropertyEdit adhocs (*instance, "AdhocHolder");
    EXPECT_TRUE (adhocs.IsSupported());
    EXPECT_EQ (0, adhocs.GetCount());

    // Try to find non-existent adhoc
    uint32_t propIdx;
    EXPECT_FALSE (adhocs.GetPropertyIndex (propIdx, "DoesNotExist"));

    // Name must be valid
    ECValue v;
    EXPECT_STATUS (Error, adhocs.Add (nullptr, v));
    EXPECT_STATUS (Error, adhocs.Add ("Not a Valid EC Name", v));
    
    // Value must be null or primitive
    v.SetStruct (m_schema->GetClassP ("AdhocHolder")->GetDefaultStandaloneEnabler()->CreateInstance().get());
    EXPECT_STATUS (DataTypeMismatch, adhocs.Add ("Struct", v));

    // simple add
    v.SetUtf8CP ("one string", false);
    EXPECT_STATUS (Success, adhocs.Add ("onestring", v, "ONE STRING"));
    EXPECT_EQ (1, adhocs.GetCount());
    EXPECT_TRUE (adhocs.GetPropertyIndex (propIdx, "onestring"));

    //  -- test metadata
    bool isReadOnly;
    EXPECT_SUCCESS (adhocs.IsReadOnly (isReadOnly, propIdx));
    EXPECT_FALSE (isReadOnly);

    Utf8String str;
    EXPECT_SUCCESS (adhocs.GetDisplayLabel (str, propIdx));
    EXPECT_TRUE (str.Equals ("ONE STRING"));
    PrimitiveType type;
    EXPECT_SUCCESS (adhocs.GetPrimitiveType (type, propIdx));
    EXPECT_TRUE (PRIMITIVETYPE_String == type);
    EXPECT_SUCCESS (adhocs.GetUnitName (str, propIdx));
    EXPECT_TRUE (str.empty());
    EXPECT_SUCCESS (adhocs.GetExtendedTypeName (str, propIdx));
    EXPECT_TRUE (str.empty());

    //  -- test value
    EXPECT_SUCCESS (adhocs.GetValue (v, propIdx));
    EXPECT_EQ (0, strcmp (v.GetUtf8CP(), "one string"));

    //  -- test value from instance
    EXPECT_STATUS (PropertyNotFound, instance->GetValue (v, "onestring"));
    EXPECT_SUCCESS (instance->GetValueOrAdhoc (v, "onestring"));    // include ad-hoc properties
    EXPECT_EQ (0, strcmp (v.GetUtf8CP(), "one string"));

    // re-add the same property, which just sets the value of the existing adhoc value
    v.SetUtf8CP ("one string second time", false);
    EXPECT_SUCCESS (adhocs.Add ("onestring", v, "One String"));
    EXPECT_EQ (1, adhocs.GetCount());
    EXPECT_TRUE (adhocs.GetPropertyIndex (propIdx, "onestring"));
    EXPECT_SUCCESS (adhocs.GetValue (v, propIdx));
    EXPECT_EQ (0, strcmp (v.GetUtf8CP(), "one string second time"));

    // set the value
    v.SetUtf8CP ("set with property value", false);
    EXPECT_SUCCESS (adhocs.SetValue (propIdx, v));
    EXPECT_SUCCESS (adhocs.GetValue (v, propIdx));
    EXPECT_EQ (0, strcmp ("set with property value", v.GetUtf8CP()));

    // add with additional optional metadata
    v.SetDouble (1234.0);
    EXPECT_SUCCESS (adhocs.Add ("DoorHeight", v, "Door Height", "Meters", "Distance", true));
    EXPECT_EQ (2, adhocs.GetCount());
    EXPECT_TRUE (adhocs.GetPropertyIndex (propIdx, "DoorHeight"));
    EXPECT_SUCCESS (adhocs.GetName (str, propIdx));
    EXPECT_TRUE (str.Equals ("DoorHeight"));
    EXPECT_SUCCESS (adhocs.GetDisplayLabel (str, propIdx));
    EXPECT_TRUE (str.Equals ("Door Height"));
    EXPECT_SUCCESS (adhocs.GetPrimitiveType (type, propIdx));
    EXPECT_TRUE (PRIMITIVETYPE_Double == type);
    EXPECT_SUCCESS (adhocs.GetExtendedTypeName (str, propIdx));
    EXPECT_TRUE (str.Equals ("Distance"));
    EXPECT_SUCCESS (adhocs.GetUnitName (str, propIdx));
    EXPECT_TRUE (str.Equals ("Meters"));
    EXPECT_SUCCESS (adhocs.IsReadOnly (isReadOnly, propIdx));
    EXPECT_TRUE (isReadOnly);
    EXPECT_SUCCESS (adhocs.GetValue (v, propIdx));
    EXPECT_TRUE (v.IsDouble());
    EXPECT_EQ (v.GetDouble(), 1234.0);

    // Property is read-only
    v.SetDouble (5678.0);
    EXPECT_TRUE (instance->IsPropertyOrAdhocReadOnly ("DoorHeight"));
    EXPECT_STATUS (PropertyNotFound, instance->SetValue ("DoorHeight", v));
    EXPECT_STATUS (UnableToSetReadOnlyProperty, instance->SetValueOrAdhoc ("DoorHeight", v));
    EXPECT_STATUS (PropertyNotFound, instance->ChangeValue ("DoorHeight", v));
    EXPECT_STATUS (UnableToSetReadOnlyProperty, instance->ChangeValueOrAdhoc ("DoorHeight", v));

    // can set read-only property through adhoc API...probably?
    EXPECT_SUCCESS (adhocs.SetValue (propIdx, v));
    EXPECT_SUCCESS (adhocs.GetValue (v, propIdx));
    EXPECT_EQ (v.GetDouble(), 5678.0);

    // not clear if managed implementation stores DateTime values in a format compatible with native. We want it to use the same
    // string representation it would use in ECInstanceXML
    v.SetDateTimeTicks (1234);
    EXPECT_SUCCESS (adhocs.Add ("time", v));
    EXPECT_TRUE (adhocs.GetPropertyIndex (propIdx, "time"));
    EXPECT_SUCCESS (adhocs.GetPrimitiveType (type, propIdx));
    EXPECT_TRUE (type == PRIMITIVETYPE_DateTime);
    EXPECT_SUCCESS (adhocs.GetValue (v, propIdx));
    EXPECT_EQ (v.GetDateTimeTicks(), 1234);

    // Test accessing ad-hocs using ECValueAccessor
    ECValueAccessor va;
    EXPECT_STATUS (PropertyNotFound, ECValueAccessor::PopulateValueAccessor (va, instance->GetEnabler(), "onestring"));
    EXPECT_STATUS (PropertyNotFound, ECValueAccessor::PopulateValueAccessor (va, *instance, "onestring", false));
    EXPECT_SUCCESS (ECValueAccessor::PopulateValueAccessor (va, *instance, "onestring", true));

    EXPECT_SUCCESS (instance->SetValueUsingAccessor (va, ECValue ("set using accessor", false)));
    EXPECT_SUCCESS (instance->GetValueUsingAccessor (v, va));
    EXPECT_EQ (0, strcmp (v.GetUtf8CP(), "set using accessor"));

    // Test removing an entry
    EXPECT_EQ (3, adhocs.GetCount());
    EXPECT_SUCCESS (adhocs.Remove (propIdx));
    EXPECT_EQ (2, adhocs.GetCount());
    EXPECT_FALSE (adhocs.GetPropertyIndex (propIdx, "time"));
    EXPECT_TRUE (adhocs.GetPropertyIndex (propIdx, "onestring"));

    // Test removing all entries
    EXPECT_SUCCESS (adhocs.Clear());
    EXPECT_EQ (0, adhocs.GetCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Muhammad Hassan   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AdhocPropertyTest, WCharSchemaXml_FailsOnNonWindowPlatforms)
    {
#ifdef BENTLEY_WIN32
    EXPECT_EQ (SchemaReadStatus::Success, ECSchema::ReadFromXmlString (m_schema, s_schemaXml, *context));
#endif
    }

END_BENTLEY_ECN_TEST_NAMESPACE
