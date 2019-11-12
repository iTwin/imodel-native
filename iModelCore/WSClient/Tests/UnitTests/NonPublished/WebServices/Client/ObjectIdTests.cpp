/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../../Utils/WebServicesTestsHelper.h"
#include <WebServices/Client/ObjectId.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

struct ObjectIdTests : WSClientBaseTest {};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ObjectIdTests, LessThan_EmptyEqual_False)
    {
    EXPECT_FALSE(ObjectId() < ObjectId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ObjectIdTests, LessThan_Equal_False)
    {
    EXPECT_FALSE(ObjectId("Schema", "Foo", "Boo") < ObjectId("Schema", "Foo", "Boo"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ObjectIdTests, LessThan_SchemaNameLessThanOther_True)
    {
    EXPECT_TRUE(ObjectId("ASchemaName", "ClassName", "Foo") < ObjectId("BSchemaName", "ClassName", "Foo"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ObjectIdTests, LessThan_ClassNameLessThanOther_True)
    {
    EXPECT_TRUE(ObjectId("AClassName", "Foo") < ObjectId("BClassName", "Foo"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ObjectIdTests, LessThan_RemoteIdLessThanOther_True)
    {
    EXPECT_TRUE(ObjectId("Foo", "ARemoteId") < ObjectId("Foo", "BRemoteId"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ObjectIdTests, Ctor_ClassKeyContainsOnlyClass_OnlyClassIsSetAndSchemaIsEmpty)
    {
    ObjectId objectId("Class", "Id");
    EXPECT_EQ("", objectId.schemaName);
    EXPECT_EQ("Class", objectId.className);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ObjectIdTests, Ctor_ClassKeyContainsSchemaAndClass_SchemaAndClassSet)
    {
    ObjectId objectId("Schema.Class", "Id");
    EXPECT_EQ("Schema", objectId.schemaName);
    EXPECT_EQ("Class", objectId.className);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ObjectIdTests, Ctor_ECClassPassedWithRemoteId_SchemaAndClassSet)
    {
    Utf8String schemaXml =
        R"xml(<ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" />
        </ECSchema>)xml";

    BentleyApi::ECN::ECSchemaPtr schema;
    BentleyApi::ECN::ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *BentleyApi::ECN::ECSchemaReadContext::CreateContext());
    BentleyApi::ECN::ECClassCP ecClass = schema->GetClassCP ("TestClass");

    ObjectId objectId(*ecClass, "Id");

    EXPECT_EQ("TestSchema", objectId.schemaName);
    EXPECT_EQ("TestClass", objectId.className);
    EXPECT_EQ("Id", objectId.remoteId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ObjectIdTests, Ctor_ECClassPassed_SchemaAndClassSetWithEmptyRemoteId)
    {
    Utf8String schemaXml =
        R"xml(<ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" />
        </ECSchema>)xml";

    BentleyApi::ECN::ECSchemaPtr schema;
    BentleyApi::ECN::ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *BentleyApi::ECN::ECSchemaReadContext::CreateContext());
    BentleyApi::ECN::ECClassCP ecClass = schema->GetClassCP ("TestClass");

    ObjectId objectId(*ecClass);

    EXPECT_EQ("TestSchema", objectId.schemaName);
    EXPECT_EQ("TestClass", objectId.className);
    EXPECT_EQ("", objectId.remoteId);
    }
