/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/NonPublishedSchemaTests.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include <comdef.h>
#include "StopWatch.h"
#include "TestFixture.h"

#include <ECObjects\ECInstance.h>
#include <ECObjects\StandaloneECInstance.h>
#include <ECObjects\ECValue.h>
#include <ECObjects\ECSchema.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

using namespace std;

struct SchemaTest : ECTestFixture {};
TEST_F(SchemaTest, ShouldBeAbleToIterateOverECClassContainer)
    {
    ECSchemaPtr schema;
    ECClassP foo;
    ECClassP bar;
    
    ECSchema::CreateSchema(schema, L"TestSchema", 5, 5);
    schema->CreateClass(foo, L"foo");
    schema->CreateClass(bar, L"bar");

    ClassMap classMap;
    classMap.insert (bpair<WCharCP, ECClassP> (foo->GetName().c_str(), foo));
    classMap.insert (bpair<WCharCP, ECClassP> (bar->GetName().c_str(), bar));
    
    int count = 0;
    ECClassContainer container(classMap);
    for (ECClassContainer::const_iterator cit = container.begin(); cit != container.end(); ++cit)
        {
        ECClassCP ecClass = *cit;
        WString name = ecClass->GetName();
        wprintf(L"ECClass=0x%x, name=%s\n", ecClass, name.c_str());
        count++;
        }
    ASSERT_EQ(2, count);

    FOR_EACH (ECClassCP ecClass, container)
        {
        WString name = ecClass->GetName();
        wprintf(L"ECClass=0x%x, name=%s\n", ecClass, name.c_str());
        count++;
        }
    ASSERT_EQ(4, count);
    }
END_BENTLEY_ECOBJECT_NAMESPACE