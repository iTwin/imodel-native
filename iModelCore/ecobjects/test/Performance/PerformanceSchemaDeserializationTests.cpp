/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "PerformanceTestFixture.h"

#include <Bentley/BeTimeUtilities.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct ECSchemaDeserializationTests : PerformanceTestFixture {
    void DeserializeSchema(size_t numberOfCAClasses) {
        Utf8PrintfString refSchemaName("RefSchema_%zuCA", numberOfCAClasses);
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        context->GetSchemasToPrune() = bvector<Utf8String>{refSchemaName};
        context->SetResolveConflicts(true);

        ECSchemaPtr refSchema;
        Utf8String refSchemaXml = GenerateRefSchema(numberOfCAClasses);
        StopWatch refSchemaTimer("Deserialization", true);
        ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refSchemaXml.c_str(), *context));
        refSchemaTimer.Stop();
        ASSERT_TRUE(refSchema.IsValid());
        PERFORMANCELOG.infov("Time to load the reference Schema with %zu custom attribute classes: %.17g", numberOfCAClasses, refSchemaTimer.GetElapsedSeconds());
        LOGPERFDB(TEST_FIXTURE_NAME, refSchema->GetDescription().c_str(), refSchemaTimer.GetElapsedSeconds(), Utf8PrintfString("Loading ref schema having %zu custom attributes", numberOfCAClasses).c_str());

        ECSchemaPtr schema;
        Utf8String schemaXml = GenerateMainSchema(numberOfCAClasses);
        StopWatch schemaTimer("Deserialization", true);
        ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *context));
        schemaTimer.Stop();
        ASSERT_TRUE(schema.IsValid());
        PERFORMANCELOG.infov("Time to load the Schema with %zu custom attribute classes: %.17g; However CA schema is in pruned achemas list", numberOfCAClasses, schemaTimer.GetElapsedSeconds());
        LOGPERFDB(TEST_FIXTURE_NAME, schema->GetDescription().c_str(), schemaTimer.GetElapsedSeconds(), Utf8PrintfString("Loading schema having %zu custom attributes from pruned schema", numberOfCAClasses).c_str());
    }

    Utf8String GenerateRefSchema(size_t numberOfCAClasses) {
        Utf8PrintfString schemaName("RefSchema_%zuCA", numberOfCAClasses);
        Utf8PrintfString description("Ref schema with %zu Custom Attribute Classes", numberOfCAClasses);
        Utf8String innerXml = "";
        for (size_t i = 0; i < numberOfCAClasses; i++) {
            Utf8PrintfString caClassName("CustomAttrClass%zu", i);
            innerXml += "<ECCustomAttributeClass typeName=\"" + caClassName + "\" />\n";
        }

        Utf8PrintfString schemaXml = Utf8PrintfString(
            R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="%s" description="%s" alias="RefS" version="01.01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            %s
            </ECSchema>
        )xml",
            schemaName.c_str(), description.c_str(), innerXml.c_str());

        return schemaXml;
    }

    Utf8String GenerateMainSchema(size_t numberOfCAClasses) {
        Utf8PrintfString schemaName("TestSchema_%zuCA", numberOfCAClasses);
        Utf8PrintfString refSchemaName("RefSchema_%zuCA", numberOfCAClasses);
        Utf8PrintfString description("Test schema with %zu Custom Attribute Classes", numberOfCAClasses);
        Utf8String innerXml = "";
        for (size_t i = 0; i < numberOfCAClasses; i++) {
            Utf8PrintfString caClassName("CustomAttrClass%zu", i);
            innerXml = innerXml + "<" + caClassName + " xmlns=\"" + refSchemaName + ".01.01.01\" />\n";
        }

        Utf8PrintfString schemaXml = Utf8PrintfString(
            R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="%s" description="%s" alias="ts" version="01.01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="%s" version="01.01.01" alias="RefS" />
                <ECCustomAttributes>
                    %s
                </ECCustomAttributes>
            </ECSchema>
        )xml",
            schemaName.c_str(), description.c_str(), refSchemaName.c_str(), innerXml.c_str());

        return schemaXml;
    }
};

/*---------------------------------------------------------------------------------**/ /**
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSchemaDeserializationTests, DeserializeHavingCAFromPrunedSchema) {
    DeserializeSchema(1 /*=numberOfCAClasses*/);
    DeserializeSchema(10 /*=numberOfCAClasses*/);
    DeserializeSchema(100 /*=numberOfCAClasses*/);
    DeserializeSchema(1000 /*=numberOfCAClasses*/);
    DeserializeSchema(5000 /*=numberOfCAClasses*/);
    DeserializeSchema(8000 /*=numberOfCAClasses*/);
    DeserializeSchema(10000 /*=numberOfCAClasses*/);
}

END_BENTLEY_ECN_TEST_NAMESPACE
