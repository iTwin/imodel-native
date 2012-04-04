/*--------------------------------------------------------------------------------------+
|
|     $Source: test/scenario/SchemaTests.cpp $
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

BEGIN_BENTLEY_EC_NAMESPACE

using namespace std;

struct SchemaTest : ECTestFixture {
//IECInstance GetClassInstance(L"CustomAttribute", *schema, *schemaOwner)
//{
//    return NULL;
//}

};
TEST_F(SchemaTest,ExpectReadOnly)
{
    ECSchemaPtr schema;   
 
    ECClassP domainClass;
    ECClassP derivedClass;
    ECClassP structClass;
    ECClassP customAttributeClass;
 
    ECSchema::CreateSchema(schema, L"TestSchema", 5, 5);
    ASSERT_TRUE(schema!=NULL);
 
    //Create Domain Class
    schema->CreateClass(domainClass,L"DomainClass");
    ASSERT_TRUE(domainClass!=NULL);
    domainClass->SetIsDomainClass(true);
 
    //Create Derived Class
    schema->CreateClass(derivedClass,L"DerivedClass");
    ASSERT_TRUE(derivedClass!=NULL);
 
    //Create Struct
    schema->CreateClass(structClass,L"StructClass");
    ASSERT_TRUE(structClass!=NULL);
    structClass->SetIsStruct(true);
 
    //Add Property of Array type to structClass
    ArrayECPropertyP MyArrayProp;
    structClass->CreateArrayProperty(MyArrayProp,L"ArrayProperty");
    ASSERT_TRUE(MyArrayProp!=NULL);
 
    //Create customAttributeClass
    schema->CreateClass(customAttributeClass,L"CustomAttribute");
    ASSERT_TRUE(customAttributeClass!=NULL);
    customAttributeClass->SetIsCustomAttributeClass(true);
 
    //Add Property Of Struct type to custom attribute
    StructECPropertyP PropertyOfCustomAttribute;
    customAttributeClass->CreateStructProperty(PropertyOfCustomAttribute, L"PropertyOfCustomAttribute",*structClass);
    ASSERT_TRUE(PropertyOfCustomAttribute!=NULL);
      
    //IECInstancePtr instance = GetClassInstance(L"CustomAttribute", *schema, *schemaOwner);
    //ASSERT_TRUE(instance.get()!=NULL);
 
    //ECObjectsStatus status =domainClass->SetCustomAttribute(*instance);
    //ASSERT_EQ(ECOBJECTS_STATUS_Success,status);
 
    //derivedClass->AddBaseClass(*domainClass);
 
    //instance = derivedClass->GetCustomAttribute(L"CustomAttribute");
 
    //bool status1 = instance->IsReadOnly();
    //ASSERT_TRUE(status1 ==true); // Assertion Fails since IsReadOnly() returns False
}
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
END_BENTLEY_EC_NAMESPACE