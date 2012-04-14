/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/CustomAttributeTests.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"

#include "TestFixture.h"

BEGIN_BENTLEY_EC_NAMESPACE

struct CustomAttributeTest : ECTestFixture
    {
    virtual bool _WantSchemaLeakDetection () override { return true; }
    virtual bool _WantInstanceLeakDetection () override { return true; }
    };

ECSchemaPtr   CreateCustomAttributeTestSchema()
    {
    ECSchemaPtr schema;
    ECClassP customAttributeClass;
    ECClassP customAttributeClass2;
    ECClassP customAttributeClass3;
    ECClassP customAttributeClass4;
    ECClassP containerClass;
    ECClassP baseClass;
    ECClassP classWithProperties;
    
    ECSchema::CreateSchema(schema, L"TestSchema", 5, 5);
    schema->CreateClass(customAttributeClass, L"CustomAttribClass");
    customAttributeClass->SetIsCustomAttributeClass(true);

    schema->CreateClass(customAttributeClass2, L"CustomAttribClass2");
    customAttributeClass2->SetIsCustomAttributeClass(true);

    schema->CreateClass(customAttributeClass3, L"CustomAttribClass3");
    customAttributeClass3->SetIsCustomAttributeClass(true);

    schema->CreateClass(customAttributeClass4, L"CustomAttribClass4");
    customAttributeClass4->SetIsCustomAttributeClass(true);

    schema->CreateClass(baseClass, L"BaseClass");
    PrimitiveECPropertyP baseStringProp;
    EXPECT_EQ(ECOBJECTS_STATUS_Success, baseClass->CreatePrimitiveProperty(baseStringProp, L"StringMember", PRIMITIVETYPE_String));

    schema->CreateClass(containerClass, L"TestClass");
    containerClass->AddBaseClass(*baseClass);

    schema->CreateClass(classWithProperties, L"ClassWithProperties");
    classWithProperties->AddBaseClass(*baseClass);

    PrimitiveECPropertyP stringProp;
    EXPECT_EQ(ECOBJECTS_STATUS_Success, classWithProperties->CreatePrimitiveProperty(stringProp, L"StringMember", PRIMITIVETYPE_String));
    EXPECT_EQ(baseStringProp, stringProp->GetBaseProperty());

    return schema;
    }                                    

IECInstancePtr GetInstanceForClass(WCharCP className, ECSchemaR schema)
    {
    ECClassP ecClass = schema.GetClassP(className);
    StandaloneECEnablerPtr enabler = ecClass->GetDefaultStandaloneEnabler();
    IECInstancePtr instance = enabler->CreateInstance().get();
    return instance;
    }

#ifdef NDEBUG // avoid assert eccustomattribute.cpp line 205 stopping build
TEST_F(CustomAttributeTest, ExpectFailureWhenSetNonCustomAttributeClass)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (NULL != containerClass);

    IECInstancePtr instance = GetInstanceForClass(L"BaseClass", *schema);
    EXPECT_EQ(ECOBJECTS_STATUS_NotCustomAttributeClass, containerClass->SetCustomAttribute(*instance));
    }
#endif

TEST_F(CustomAttributeTest, CanAddSingleCustomAttribute)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP (L"ClassWithProperties");
    ASSERT_TRUE (NULL != containerClass);

    ECPropertyP p = containerClass->GetPropertyP (L"StringMember");

    IECInstancePtr instance = GetInstanceForClass(L"CustomAttribClass", *schema);


    EXPECT_EQ(ECOBJECTS_STATUS_Success, schema->SetCustomAttribute(*instance));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, p->SetCustomAttribute(*instance));

    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(*instance));
    }

TEST_F(CustomAttributeTest, CanAddMultipleCustomAttributes)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (NULL != containerClass);

    IECInstancePtr instance  = GetInstanceForClass(L"CustomAttribClass",  *schema);
    IECInstancePtr instance2 = GetInstanceForClass(L"CustomAttribClass2", *schema);
    IECInstancePtr instance3 = GetInstanceForClass(L"CustomAttribClass3", *schema);
    IECInstancePtr instance4 = GetInstanceForClass(L"CustomAttribClass4", *schema);

    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(*instance));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(*instance2));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(*instance3));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(*instance4));
    }

TEST_F(CustomAttributeTest, ExpectSuccessWhenAddDuplicateCustomAttribute)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (NULL != containerClass);

    IECInstancePtr instance = GetInstanceForClass(L"CustomAttribClass", *schema);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(*instance));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(*instance));
    }

TEST_F(CustomAttributeTest, ExpectSuccessWhenAddCustomAttributeToProperty)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP classWithProperties = schema->GetClassP (L"ClassWithProperties");
    ASSERT_TRUE (NULL != classWithProperties);
    ECPropertyP stringProperty = classWithProperties->GetPropertyP(L"StringMember");
    ASSERT_TRUE (NULL != stringProperty);

    IECInstancePtr instance = GetInstanceForClass(L"CustomAttribClass", *schema);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, stringProperty->SetCustomAttribute(*instance));
    }

TEST_F(CustomAttributeTest, ExpectIsDefined)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (NULL != containerClass);
    ECClassP customAttributeClass = schema->GetClassP (L"CustomAttribClass");

    IECInstancePtr instance = GetInstanceForClass(L"CustomAttribClass", *schema);
    EXPECT_FALSE(containerClass->IsDefined(L"CustomAttribClass"));
    EXPECT_FALSE(containerClass->IsDefined(*customAttributeClass));

    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(*instance));
    EXPECT_TRUE(containerClass->IsDefined(L"CustomAttribClass"));
    EXPECT_TRUE(containerClass->IsDefined(*customAttributeClass));
    }

TEST_F(CustomAttributeTest, ExpectIsDefinedOnBaseClass)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (NULL != containerClass);
    ECClassP customAttributeClass = schema->GetClassP (L"CustomAttribClass");
    ASSERT_TRUE (NULL != customAttributeClass);
    ECClassP baseClass = schema->GetClassP (L"BaseClass");
    ASSERT_TRUE (NULL != baseClass);

    IECInstancePtr instance = GetInstanceForClass(L"CustomAttribClass", *schema);
    EXPECT_FALSE(containerClass->IsDefined(L"CustomAttribClass"));
    EXPECT_FALSE(containerClass->IsDefined(*customAttributeClass));

    EXPECT_EQ(ECOBJECTS_STATUS_Success, baseClass->SetCustomAttribute(*instance));
    EXPECT_TRUE(baseClass->IsDefined(L"CustomAttribClass"));
    EXPECT_TRUE(baseClass->IsDefined(*customAttributeClass));
    EXPECT_TRUE(containerClass->IsDefined(L"CustomAttribClass"));
    EXPECT_TRUE(containerClass->IsDefined(*customAttributeClass));
    }

TEST_F(CustomAttributeTest, ExpectCanGetCustomAttribute)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (NULL != containerClass);

    IECInstancePtr instance = GetInstanceForClass(L"CustomAttribClass", *schema);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(*instance));

    IECInstancePtr gotInstance = containerClass->GetCustomAttribute(L"CustomAttribClass");
    EXPECT_TRUE(gotInstance.IsValid());
    EXPECT_TRUE(instance == gotInstance);

    ECClassP caClass = schema->GetClassP(L"CustomAttribClass");
    ASSERT_TRUE (NULL != caClass);
    gotInstance = containerClass->GetCustomAttribute(*caClass);
    EXPECT_TRUE(gotInstance.IsValid());
    EXPECT_TRUE(instance == gotInstance);
    }

TEST_F(CustomAttributeTest, ExpectCanGetAllCustomAttributes)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (NULL != containerClass);

    ECClassP baseClass = schema->GetClassP (L"BaseClass");
    ASSERT_TRUE (NULL != baseClass);

    IECInstancePtr instance = GetInstanceForClass(L"CustomAttribClass", *schema);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(*instance));

    IECInstancePtr instance2 = GetInstanceForClass(L"CustomAttribClass2", *schema);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(*instance2));

    IECInstancePtr instance3 = GetInstanceForClass(L"CustomAttribClass3", *schema);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, baseClass->SetCustomAttribute(*instance3));

    bool foundCustomAttrib = false;
    bool foundCustomAttrib2 = false;
    bool foundCustomAttrib3 = false;
    ECCustomAttributeInstanceIterable  iterableFalse = containerClass->GetCustomAttributes (false);
    FOR_EACH (IECInstancePtr testInstance, iterableFalse)
        {
        if (testInstance->GetClass().GetName().compare(L"CustomAttribClass") == 0)
            foundCustomAttrib = true;
        else if (testInstance->GetClass().GetName().compare(L"CustomAttribClass2") == 0)
            foundCustomAttrib2 = true;
        else if (testInstance->GetClass().GetName().compare(L"CustomAttribClass3") == 0)
            foundCustomAttrib3 = true;
        }
    EXPECT_TRUE(foundCustomAttrib);
    EXPECT_TRUE(foundCustomAttrib2);
    EXPECT_FALSE(foundCustomAttrib3);

    foundCustomAttrib = false;
    foundCustomAttrib2 = false;
    foundCustomAttrib3 = false;

    ECCustomAttributeInstanceIterable  iterableTrue = containerClass->GetCustomAttributes (true);
    FOR_EACH (IECInstancePtr testInstance, iterableTrue)
        {
        if (testInstance->GetClass().GetName().compare(L"CustomAttribClass") == 0)
            foundCustomAttrib = true;
        else if (testInstance->GetClass().GetName().compare(L"CustomAttribClass2") == 0)
            foundCustomAttrib2 = true;
        else if (testInstance->GetClass().GetName().compare(L"CustomAttribClass3") == 0)
            foundCustomAttrib3 = true;
        }
    EXPECT_TRUE(foundCustomAttrib);
    EXPECT_TRUE(foundCustomAttrib2);
    EXPECT_TRUE(foundCustomAttrib3);
    }

TEST_F(CustomAttributeTest, ExpectCanRemoveCustomAttribute)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (NULL != containerClass);

    IECInstancePtr instance = GetInstanceForClass(L"CustomAttribClass", *schema);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(*instance));

    IECInstancePtr gotInstance = containerClass->GetCustomAttribute(L"CustomAttribClass");
    EXPECT_TRUE(gotInstance.IsValid());
    EXPECT_TRUE(instance == gotInstance);

    EXPECT_TRUE(containerClass->RemoveCustomAttribute(L"CustomAttribClass"));
    IECInstancePtr gotInstance2 = containerClass->GetCustomAttribute(L"CustomAttribClass");
    EXPECT_FALSE(gotInstance2.IsValid());

    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(*instance));
    ECClassP caClass = schema->GetClassP (L"CustomAttribClass");
    ASSERT_TRUE (NULL != caClass);
    EXPECT_TRUE(containerClass->RemoveCustomAttribute(*caClass));
    IECInstancePtr gotInstance3 = containerClass->GetCustomAttribute(*caClass);
    EXPECT_FALSE(gotInstance3.IsValid());


    }

#ifdef NDEBUG // avoid assert eccustomattribute.cpp line 205 stopping build
TEST_F(CustomAttributeTest, ExpectFailureWithUnreferencedCustomAttribute)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECSchemaPtr refSchema;
    ECSchema::CreateSchema(refSchema, L"RefSchema", 5, 5);

    ECClassP refClass;
    refSchema->CreateClass(refClass, L"RefClass");
    refClass->SetIsCustomAttributeClass(true);

    ECClassP containerClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (NULL != containerClass);

    IECInstancePtr instance = GetInstanceForClass(L"RefClass", *refSchema);

    EXPECT_EQ(ECOBJECTS_STATUS_SchemaNotFound, containerClass->SetCustomAttribute(*instance));
    schema->AddReferencedSchema(*refSchema);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(*instance));
    }
#endif

#ifdef TEST_DEFECT_D_88458 
// This is a nonsensical scenario, so the defect is being deferred.  But the code is
// here if the defect ever gets reopened 
TEST_F(CustomAttributeTest, ExpectSuccessWhenAddingCircularStructPropertiesToCustomAttributeClass)
    {
    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create(); 
    ECSchemaP schema;    

    //ECObjectsStatus structStatus;
    ECClassP struct1;
    ECClassP struct2;
    ECClassP customAttributeClass;
    ECClassP testClass;

    ECSchema::CreateSchema(schema, L"TestSchema", 5, 5);
    ASSERT_TRUE(schema!=NULL);

    schema->CreateClass(struct1,L"Struct1");
    ASSERT_TRUE(struct1!=NULL);
    schema->CreateClass(struct2,L"Struct2");
    ASSERT_TRUE(struct2!=NULL);
    struct1->SetIsStruct(true);
    struct2->SetIsStruct(true);

    StructECPropertyP P1;
    StructECPropertyP P2;

    struct1->CreateStructProperty(P1, L"P1",*struct2);
    ASSERT_TRUE(P1!=NULL);
    struct2->CreateStructProperty(P2,L"P2",*struct1);
    ASSERT_TRUE(P2!=NULL);

    StructECPropertyP PropertyOfCustomAttribute;

    schema->CreateClass(customAttributeClass,L"MyCustomAttribute");
    ASSERT_TRUE(customAttributeClass!=NULL);
    customAttributeClass->SetIsCustomAttributeClass(true);


    customAttributeClass->CreateStructProperty(PropertyOfCustomAttribute, L"PropertyOfCustomAttribute",*struct1);
    ASSERT_TRUE(PropertyOfCustomAttribute!=NULL);
    //If we comment out the struct property added to custom attribute. It works fine.
    IECInstancePtr instance = GetInstanceForClass(L"MyCustomAttribute", *schema);
    ASSERT_TRUE(instance.get()!=NULL);

    schema->CreateClass(testClass,L"TestClass");
    ASSERT_TRUE(testClass!=NULL);

    ECClassP tempClass = schema->GetClassP(L"TestClass");
    ASSERT_TRUE(tempClass!=NULL);
    ECObjectsStatus status =tempClass->SetCustomAttribute(*instance);
    ASSERT_EQ(ECOBJECTS_STATUS_Success,status);
    }
#endif
END_BENTLEY_EC_NAMESPACE