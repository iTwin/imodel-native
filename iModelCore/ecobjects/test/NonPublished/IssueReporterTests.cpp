/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include <iostream>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct IssueReporterTests : ECTestFixture {};

template <typename IssueReportedCallback = void(*)(IssueSeverity, IssueCategory, IssueType, Utf8CP)>
struct TestIssueListener : public IIssueListener
    {
    IssueReportedCallback m_onIssueReported;
    TestIssueListener(IssueReportedCallback onIssueReported) : m_onIssueReported(onIssueReported) {}
private:
    virtual void _OnIssueReported(IssueSeverity severity, IssueCategory category, IssueType type, Utf8CP message) const override
        {
        m_onIssueReported(severity, category, type, message);
        }
    };

namespace
    {
    template <typename F>
    TestIssueListener<F> MakeTestIssueListener(F&& f) { return TestIssueListener<F>(std::forward<F>(f)); }
    }


//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(IssueReporterTests, SchemaXmlReaderCantResolveSchemaReference)
    {
    int testListenerReportCount = 0;
    auto testListener = MakeTestIssueListener([&](IssueSeverity severity, IssueCategory category, IssueType type, Utf8CP message){
        ++testListenerReportCount;
    });

    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="UnknownSchema" version="02.00" prefix="unk" />
            <ECClass typeName="Class1">
                <ECProperty propertyName="PropA" typeName="string" description="desc" />
            </ECClass>
        </ECSchema>
    )xml";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->Issues().AddListener(testListener);

    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::ReferencedSchemaNotFound, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    // two issues will be reported, one for unable to locate schema, another for failed to read XML from string
    // it failed to deserialize not read
    EXPECT_EQ(testListenerReportCount, 2);

    context->Issues().RemoveListener();
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(IssueReporterTests, SchemaConverterClassMapBadMapStrategyReported)
    {
    int testListenerReportCount = 0;
    Utf8String lastReportMessage = "";
    auto testListener = MakeTestIssueListener([&](IssueSeverity severity, IssueCategory category, IssueType type, Utf8CP message){
        lastReportMessage = message;
        ++testListenerReportCount;
    });

    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECDbMap" version="1.0" prefix="ecdbmap"/>
            <ECClass typeName="C" isDomainClass="true">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.01.00">
                        <MapStrategy>
                            <Strategy>TablePerHierarchy</Strategy>
                            <AppliesToSubclasses>True</AppliesToSubclasses>
                        </MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
            </ECClass>
        </ECSchema>
    )xml";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->Issues().AddListener(testListener);

    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    CustomECSchemaConverterPtr converter = CustomECSchemaConverter::Create();
    IECCustomAttributeConverterPtr classMapConv = new ECDbClassMapConverter();
    converter->AddSchemaReadContext(*context);
    converter->AddConverter(ECDbClassMapConverter::GetSchemaName(), ECDbClassMapConverter::GetClassName(), classMapConv);
    ASSERT_TRUE(converter->Convert(*schema.get(), true)) << "schema conversion should succeed (dropping the offending item)";

    EXPECT_EQ(1, testListenerReportCount);
    std::cout << lastReportMessage << std::endl;
    EXPECT_EQ("Failed to convert ECDbMap:ClassMap on TestSchema:C because the MapStrategy is not 'SharedTable with AppliesToSubclasses == true' or 'NotMapped'.  Removing and skipping.", lastReportMessage);

    context->Issues().RemoveListener();
    }

END_BENTLEY_ECN_TEST_NAMESPACE
