/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/JsonNavNodeTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include "TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

#define TEST_SCHEMA "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"                                                                                    \
                    "<ECSchema schemaName=\"TestSchema\" alias=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"  \
                    "    <ECEntityClass typeName=\"TestClass\">"                                                                                    \
                    "        <ECProperty propertyName=\"Name\" typeName=\"string\" />"                                                              \
                    "    </ECEntityClass>"                                                                                                          \
                    "</ECSchema>"

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2015
+===============+===============+===============+===============+===============+======*/
struct JsonNavNodeTests : ::testing::Test
    {
    JsonNavNodesFactory m_factory;
    ECSchemaReadContextPtr m_schemaContext;
    ECSchemaPtr m_schema;
    
    void SetUp() override
        {
        m_schemaContext = ECSchemaReadContext::CreateContext();
        ECSchema::ReadFromXmlString(m_schema, TEST_SCHEMA, *m_schemaContext);
        }
    };
