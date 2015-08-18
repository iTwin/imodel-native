/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Util/ECExpressionHelperTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECExpressionHelperTests.h"

#include <WebServices/Cache/Util/ECExpressionHelper.h>
#include "../CachingTestsHelper.h"

using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

TEST_F(ECExpressionHelperTests, GetRequiredProperties_EmptyString_ReturnsEmpty)
    {
    auto schema = ParseSchema(
        R"xml(<ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" >
                <ECProperty propertyName="TestProperty" typeName="string" />
            </ECClass>
        </ECSchema>)xml");

    auto ecClass = schema->GetClassCP("TestClass");
    auto properties = ECExpressionHelper::GetRequiredProperties("", *ecClass);
    EXPECT_EQ(0, properties.size());
    }

TEST_F(ECExpressionHelperTests, GetRequiredProperties_StringWithPropertyAndVariousSymbols_ReturnsProperty)
    {
    auto schema = ParseSchema(
        R"xml(<ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" >
                <ECProperty propertyName="TestProperty" typeName="string" />
            </ECClass>
        </ECSchema>)xml");

    auto ecClass = schema->GetClassCP("TestClass");
    auto ecProperty = ecClass->GetPropertyP("TestProperty");

    auto properties = ECExpressionHelper::GetRequiredProperties("this.TestProperty", *ecClass);
    EXPECT_CONTAINS(properties, ecProperty);
    properties = ECExpressionHelper::GetRequiredProperties("this.TestProperty ", *ecClass);
    EXPECT_CONTAINS(properties, ecProperty);
    properties = ECExpressionHelper::GetRequiredProperties(" this.TestProperty", *ecClass);
    EXPECT_CONTAINS(properties, ecProperty);
    properties = ECExpressionHelper::GetRequiredProperties("this.TestProperty!", *ecClass);
    EXPECT_CONTAINS(properties, ecProperty);
    properties = ECExpressionHelper::GetRequiredProperties("this.TestProperty.Foo", *ecClass);
    EXPECT_CONTAINS(properties, ecProperty);
    properties = ECExpressionHelper::GetRequiredProperties("(this.TestProperty)", *ecClass);
    EXPECT_CONTAINS(properties, ecProperty);
    properties = ECExpressionHelper::GetRequiredProperties("(this.TestProperty,", *ecClass);
    EXPECT_CONTAINS(properties, ecProperty);
    }

TEST_F(ECExpressionHelperTests, GetRequiredProperties_StringWithMultipleProperties_ReturnsProperties)
    {
    auto schema = ParseSchema(
        R"xml(<ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" >
                <ECProperty propertyName="A" typeName="string" />
                <ECProperty propertyName="B" typeName="string" />
                <ECProperty propertyName="C" typeName="string" />
            </ECClass>
        </ECSchema>)xml");

    auto ecClass = schema->GetClassCP("TestClass");
    auto properties = ECExpressionHelper::GetRequiredProperties("this.A this.B this.C", *ecClass);
    EXPECT_EQ(3, properties.size());
    EXPECT_CONTAINS(properties, schema->GetClassCP("TestClass")->GetPropertyP("A"));
    EXPECT_CONTAINS(properties, schema->GetClassCP("TestClass")->GetPropertyP("B"));
    EXPECT_CONTAINS(properties, schema->GetClassCP("TestClass")->GetPropertyP("C"));
    }


TEST_F(ECExpressionHelperTests, GetRequiredProperties_StringWithFakeProperties_ReturnsOnlyValidProperties)
    {
    auto schema = ParseSchema(
        R"xml(<ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" >
                <ECProperty propertyName="A" typeName="string" />
                <ECProperty propertyName="B" typeName="string" />
            </ECClass>
        </ECSchema>)xml");

    auto ecClass = schema->GetClassCP("TestClass");
    auto properties = ECExpressionHelper::GetRequiredProperties("this.A this.Foo this.B", *ecClass);
    EXPECT_EQ(2, properties.size());
    EXPECT_CONTAINS(properties, schema->GetClassCP("TestClass")->GetPropertyP("A"));
    EXPECT_CONTAINS(properties, schema->GetClassCP("TestClass")->GetPropertyP("B"));
    }
