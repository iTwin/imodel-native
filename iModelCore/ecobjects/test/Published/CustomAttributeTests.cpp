
/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/nativeatp/Published/CustomAttributeTests.cpp $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"

#include "TestFixture.h"

BEGIN_BENTLEY_EC_NAMESPACE

ECSchemaPtr CreateCustomAttributeTestSchema()
    {
    ECSchemaPtr schema;
    ECClassP customAttributeClass;
    ECClassP customAttributeClass2;
    ECClassP customAttributeClass3;
    ECClassP customAttributeClass4;
    ECClassP containerClass;
    ECClassP baseClass;
    ECClassP classWithProperties;
    
    ECSchema::CreateSchema(schema, L"TestSchema");
    schema->CreateClass(customAttributeClass, L"CustomAttribClass");
    customAttributeClass->IsCustomAttributeClass = true;

    schema->CreateClass(customAttributeClass2, L"CustomAttribClass2");
    customAttributeClass2->IsCustomAttributeClass = true;

    schema->CreateClass(customAttributeClass3, L"CustomAttribClass3");
    customAttributeClass3->IsCustomAttributeClass = true;

    schema->CreateClass(customAttributeClass4, L"CustomAttribClass4");
    customAttributeClass4->IsCustomAttributeClass = true;

    schema->CreateClass(baseClass, L"BaseClass");
    PrimitiveECPropertyP baseStringProp;
    EXPECT_EQ(ECOBJECTS_STATUS_Success, baseClass->CreatePrimitiveProperty(baseStringProp, L"StringMember", PRIMITIVETYPE_String));

    schema->CreateClass(containerClass, L"TestClass");
    containerClass->AddBaseClass(*baseClass);

    schema->CreateClass(classWithProperties, L"ClassWithProperties");
    classWithProperties->AddBaseClass(*baseClass);

    PrimitiveECPropertyP stringProp;
    EXPECT_EQ(ECOBJECTS_STATUS_Success, classWithProperties->CreatePrimitiveProperty(stringProp, L"StringMember", PRIMITIVETYPE_String));
    EXPECT_EQ(baseStringProp, stringProp->BaseProperty);

    return schema;
    }

IECInstancePtr GetInstanceForClass(const wchar_t *className, ECSchemaPtr schema)
    {
    ECClassP ecClass = schema->GetClassP (className);
    ClassLayoutP classLayout = ClassLayout::BuildFromClass (*ecClass, 0, 0);
    StandaloneECEnablerPtr enabler = StandaloneECEnabler::CreateEnabler (*ecClass, *classLayout);        
    
    IECInstancePtr instance = enabler->CreateInstance().get();
    return instance;
    }

TEST(CustomAttributeTest, ExpectFailureWhenSetNonCustomAttributeClass)
    {
    ECSchemaPtr schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (containerClass);

    IECInstancePtr instance = GetInstanceForClass(L"BaseClass", schema);
    EXPECT_EQ(ECOBJECTS_STATUS_NotCustomAttributeClass, containerClass->SetCustomAttribute(instance));
    }

TEST(CustomAttributeTest, CanAddSingleCustomAttribute)
    {
    ECSchemaPtr schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (containerClass);

    IECInstancePtr instance = GetInstanceForClass(L"CustomAttribClass", schema);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(instance));
    }

TEST(CustomAttributeTest, CanAddMultipleCustomAttributes)
    {
    ECSchemaPtr schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (containerClass);

    IECInstancePtr instance = GetInstanceForClass(L"CustomAttribClass", schema);
    IECInstancePtr instance2 = GetInstanceForClass(L"CustomAttribClass2", schema);
    IECInstancePtr instance3 = GetInstanceForClass(L"CustomAttribClass3", schema);
    IECInstancePtr instance4 = GetInstanceForClass(L"CustomAttribClass4", schema);

    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(instance));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(instance2));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(instance3));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(instance4));
    }

TEST(CustomAttributeTest, ExpectSuccessWhenAddDuplicateCustomAttribute)
    {
    ECSchemaPtr schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (containerClass);

    IECInstancePtr instance = GetInstanceForClass(L"CustomAttribClass", schema);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(instance));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(instance));
    }

TEST(CustomAttributeTest, ExpectSuccessWhenAddCustomAttributeToProperty)
    {
    ECSchemaPtr schema = CreateCustomAttributeTestSchema();

    ECClassP classWithProperties = schema->GetClassP (L"ClassWithProperties");
    ASSERT_TRUE (classWithProperties);
    ECPropertyP stringProperty = classWithProperties->GetPropertyP(L"StringMember");
    ASSERT_TRUE(stringProperty);

    IECInstancePtr instance = GetInstanceForClass(L"CustomAttribClass", schema);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, stringProperty->SetCustomAttribute(instance));
    }

TEST(CustomAttributeTest, ExpectIsDefined)
    {
    ECSchemaPtr schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (containerClass);
    ECClassP customAttributeClass = schema->GetClassP (L"CustomAttribClass");

    IECInstancePtr instance = GetInstanceForClass(L"CustomAttribClass", schema);
    EXPECT_FALSE(containerClass->IsDefined(L"CustomAttribClass"));
    EXPECT_FALSE(containerClass->IsDefined(*customAttributeClass));

    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(instance));
    EXPECT_TRUE(containerClass->IsDefined(L"CustomAttribClass"));
    EXPECT_TRUE(containerClass->IsDefined(*customAttributeClass));
    }

TEST(CustomAttributeTest, ExpectIsDefinedOnBaseClass)
    {
    ECSchemaPtr schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (containerClass);
    ECClassP customAttributeClass = schema->GetClassP (L"CustomAttribClass");
    ASSERT_TRUE (customAttributeClass);
    ECClassP baseClass = schema->GetClassP (L"BaseClass");
    ASSERT_TRUE (baseClass);

    IECInstancePtr instance = GetInstanceForClass(L"CustomAttribClass", schema);
    EXPECT_FALSE(containerClass->IsDefined(L"CustomAttribClass"));
    EXPECT_FALSE(containerClass->IsDefined(*customAttributeClass));

    EXPECT_EQ(ECOBJECTS_STATUS_Success, baseClass->SetCustomAttribute(instance));
    EXPECT_TRUE(baseClass->IsDefined(L"CustomAttribClass"));
    EXPECT_TRUE(baseClass->IsDefined(*customAttributeClass));
    EXPECT_TRUE(containerClass->IsDefined(L"CustomAttribClass"));
    EXPECT_TRUE(containerClass->IsDefined(*customAttributeClass));
    }

TEST(CustomAttributeTest, ExpectCanGetCustomAttribute)
    {
    ECSchemaPtr schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (containerClass);

    IECInstancePtr instance = GetInstanceForClass(L"CustomAttribClass", schema);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(instance));

    IECInstancePtr gotInstance = containerClass->GetCustomAttribute(L"CustomAttribClass");
    EXPECT_TRUE(gotInstance.IsValid());
    EXPECT_TRUE(instance == gotInstance);

    ECClassP caClass = schema->GetClassP(L"CustomAttribClass");
    ASSERT_TRUE(caClass);
    gotInstance = containerClass->GetCustomAttribute(*caClass);
    EXPECT_TRUE(gotInstance.IsValid());
    EXPECT_TRUE(instance == gotInstance);
    }

TEST(CustomAttributeTest, ExpectCanGetAllCustomAttributes)
    {
    ECSchemaPtr schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (containerClass);

    ECClassP baseClass = schema->GetClassP (L"BaseClass");
    ASSERT_TRUE (baseClass);

    IECInstancePtr instance = GetInstanceForClass(L"CustomAttribClass", schema);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(instance));

    IECInstancePtr instance2 = GetInstanceForClass(L"CustomAttribClass2", schema);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(instance2));

    IECInstancePtr instance3 = GetInstanceForClass(L"CustomAttribClass3", schema);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, baseClass->SetCustomAttribute(instance3));

    bool foundCustomAttrib = false;
    bool foundCustomAttrib2 = false;
    bool foundCustomAttrib3 = false;
    for each (IECInstancePtr testInstance in containerClass->GetCustomAttributes(false))
        {
        if (testInstance->GetClass().Name.compare(L"CustomAttribClass") == 0)
            foundCustomAttrib = true;
        else if (testInstance->GetClass().Name.compare(L"CustomAttribClass2") == 0)
            foundCustomAttrib2 = true;
        else if (testInstance->GetClass().Name.compare(L"CustomAttribClass3") == 0)
            foundCustomAttrib3 = true;
        }
    EXPECT_TRUE(foundCustomAttrib);
    EXPECT_TRUE(foundCustomAttrib2);
    EXPECT_FALSE(foundCustomAttrib3);

    foundCustomAttrib = false;
    foundCustomAttrib2 = false;
    foundCustomAttrib3 = false;
    for each (IECInstancePtr testInstance in containerClass->GetCustomAttributes(true))
        {
        if (testInstance->GetClass().Name.compare(L"CustomAttribClass") == 0)
            foundCustomAttrib = true;
        else if (testInstance->GetClass().Name.compare(L"CustomAttribClass2") == 0)
            foundCustomAttrib2 = true;
        else if (testInstance->GetClass().Name.compare(L"CustomAttribClass3") == 0)
            foundCustomAttrib3 = true;
        }
    EXPECT_TRUE(foundCustomAttrib);
    EXPECT_TRUE(foundCustomAttrib2);
    EXPECT_TRUE(foundCustomAttrib3);
    }

TEST(CustomAttributeTest, ExpectCanRemoveCustomAttribute)
    {
    ECSchemaPtr schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (containerClass);

    IECInstancePtr instance = GetInstanceForClass(L"CustomAttribClass", schema);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(instance));

    IECInstancePtr gotInstance = containerClass->GetCustomAttribute(L"CustomAttribClass");
    EXPECT_TRUE(gotInstance.IsValid());
    EXPECT_TRUE(instance == gotInstance);

    EXPECT_TRUE(containerClass->RemoveCustomAttribute(L"CustomAttribClass"));
    IECInstancePtr gotInstance2 = containerClass->GetCustomAttribute(L"CustomAttribClass");
    EXPECT_FALSE(gotInstance2.IsValid());

    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(instance));
    ECClassP caClass = schema->GetClassP (L"CustomAttribClass");
    ASSERT_TRUE (caClass);
    EXPECT_TRUE(containerClass->RemoveCustomAttribute(*caClass));
    IECInstancePtr gotInstance3 = containerClass->GetCustomAttribute(*caClass);
    EXPECT_FALSE(gotInstance3.IsValid());


    }

END_BENTLEY_EC_NAMESPACE