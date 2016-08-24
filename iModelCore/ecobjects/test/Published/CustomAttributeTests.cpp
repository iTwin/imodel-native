/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/CustomAttributeTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct CustomAttributeTest : ECTestFixture
    {
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
ECSchemaPtr   CreateCustomAttributeTestSchema()
    {
    ECSchemaPtr schema;
    ECCustomAttributeClassP customAttributeClass;
    ECCustomAttributeClassP customAttributeClass2;
    ECCustomAttributeClassP customAttributeClass3;
    ECCustomAttributeClassP customAttributeClass4;
    ECEntityClassP containerClass;
    ECEntityClassP baseClass;
    ECEntityClassP classWithProperties;
    
    ECSchema::CreateSchema(schema, "TestSchema", "test", 5, 0, 5);
    schema->CreateCustomAttributeClass(customAttributeClass, "CustomAttribClass");
    schema->CreateCustomAttributeClass(customAttributeClass2, "CustomAttribClass2");
    schema->CreateCustomAttributeClass(customAttributeClass3, "CustomAttribClass3");
    schema->CreateCustomAttributeClass(customAttributeClass4, "CustomAttribClass4");

    schema->CreateEntityClass(baseClass, "BaseClass");
    PrimitiveECPropertyP baseStringProp;
    EXPECT_EQ(ECObjectsStatus::Success, baseClass->CreatePrimitiveProperty(baseStringProp, "StringMember", PRIMITIVETYPE_String));

    schema->CreateEntityClass(containerClass, "TestClass");
    containerClass->AddBaseClass(*baseClass);

    schema->CreateEntityClass(classWithProperties, "ClassWithProperties");
    classWithProperties->AddBaseClass(*baseClass);

    PrimitiveECPropertyP stringProp = NULL;
    EXPECT_EQ(ECObjectsStatus::Success, classWithProperties->CreatePrimitiveProperty(stringProp, "StringMember", PRIMITIVETYPE_String));
    EXPECT_EQ(baseStringProp, stringProp->GetBaseProperty());

    return schema;
    }                                    

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
void VerifyCustomAttributeTestSchema (ECSchemaPtr customAttributeSchema)
    {
    EXPECT_TRUE (customAttributeSchema.IsValid ()) << "Custom attribute schema is null.";
    EXPECT_STREQ ("TestSchema.05.00.05", customAttributeSchema->GetFullSchemaName ().c_str ()) << "Custom attribute schema full name mismatches.";
    
    Utf8CP className = "CustomAttribClass";
    EXPECT_TRUE (customAttributeSchema->GetClassCP (className) != NULL) << "Class " << className << " not found in custom attribute schema.";

    className = "CustomAttribClass2";
    EXPECT_TRUE (customAttributeSchema->GetClassCP (className) != NULL) << "Class " << className << " not found in custom attribute schema.";

    className = "CustomAttribClass3";
    EXPECT_TRUE (customAttributeSchema->GetClassCP (className) != NULL) << "Class " << className << " not found in custom attribute schema.";

    className = "CustomAttribClass4";
    EXPECT_TRUE (customAttributeSchema->GetClassCP (className) != NULL) << "Class " << className << " not found in custom attribute schema.";

    Utf8CP baseClassName = "BaseClass";
    ECClassCP ecClass = customAttributeSchema->GetClassCP (baseClassName);
    ASSERT_TRUE (ecClass != NULL) << "Class " << baseClassName << " not found in custom attribute schema.";
    Utf8CP propertyName = "StringMember";
    EXPECT_TRUE (ecClass->GetPropertyP (propertyName, false) != NULL) << "Property " << propertyName << " not found in class " << baseClassName;
    
    className = "TestClass";
    ecClass = customAttributeSchema->GetClassCP (className);
    ASSERT_TRUE (ecClass != NULL) << "Class " << className << " not found in custom attribute schema.";
    const ECBaseClassesList& baseClassList = ecClass->GetBaseClasses ();
    EXPECT_EQ (1, baseClassList.size ()) << "Class " << className << "is expected to only have one base class";
    EXPECT_STREQ (baseClassName, baseClassList[0]->GetName ().c_str ()) << "Unexpected base class of class " << className;

    propertyName = "StringMember";
    EXPECT_TRUE (ecClass->GetPropertyP (propertyName, true) != NULL) << "Inherited property " << propertyName << " not found in class " << className;

    className = "ClassWithProperties";
    ecClass = customAttributeSchema->GetClassCP (className);
    ASSERT_TRUE (ecClass != NULL) << "Class " << className << " not found in custom attribute schema.";
    const ECBaseClassesList& baseClassList2 = ecClass->GetBaseClasses ();
    EXPECT_EQ (1, baseClassList2.size ()) << "Class " << className << "is expected to only have one base class";
    EXPECT_STREQ (baseClassName, baseClassList2[0]->GetName ().c_str ()) << "Unexpected base class of class " << className;

    propertyName = "StringMember";
    EXPECT_TRUE (ecClass->GetPropertyP (propertyName, false) != NULL) << "Overridden property " << propertyName << " not found in class " << className;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
IECInstancePtr GetInstanceForClass(Utf8CP className, ECSchemaR schema)
    {
    ECClassP ecClass = schema.GetClassP(className);
    StandaloneECEnablerPtr enabler = ecClass->GetDefaultStandaloneEnabler();
    IECInstancePtr instance = enabler->CreateInstance().get();
    return instance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeTest, ExpectFailureWhenSetNonCustomAttributeClass)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP ("TestClass");
    ASSERT_TRUE (NULL != containerClass);

    IECInstancePtr instance = GetInstanceForClass("BaseClass", *schema);
        {
        DISABLE_ASSERTS
        EXPECT_EQ(ECObjectsStatus::NotCustomAttributeClass, containerClass->SetCustomAttribute(*instance));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                       Colin.Kerr 4/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeTest, ExpectFailureWhenContainerTypesNotCompatible)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "Test", "T111", 1, 1, 1));

    ECCustomAttributeClassP caClass;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateCustomAttributeClass(caClass, "CAClass"));
    caClass->SetContainerType(CustomAttributeContainerType::EntityClass);

    ECEntityClassP entityClass;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(entityClass, "EntityClass"));

    ECStructClassP structClass;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateStructClass(structClass, "StructClass"));

    IECInstancePtr caInstance = GetInstanceForClass("CAClass", *schema);
    EXPECT_EQ(ECObjectsStatus::Success, entityClass->SetCustomAttribute(*caInstance)) << "Should have been able to set custom attribute on entity class because container types match";

        {
        DISABLE_ASSERTS
        EXPECT_EQ(ECObjectsStatus::CustomAttributeContainerTypesNotCompatible, structClass->SetCustomAttribute(*caInstance)) << "Should not have been able to set custom attribute on struct class because contaienr types do not match";
        }
    
    ECRelationshipClassP relClass;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateRelationshipClass(relClass, "RelClass"));

    caClass->SetContainerType(CustomAttributeContainerType::SourceRelationshipConstraint);

    EXPECT_EQ(ECObjectsStatus::Success, relClass->GetSource().SetCustomAttribute(*caInstance)) << "Should have been able to set custom attribute on source constraint because container types match";

        {
        DISABLE_ASSERTS
        EXPECT_EQ(ECObjectsStatus::CustomAttributeContainerTypesNotCompatible, relClass->GetTarget().SetCustomAttribute(*caInstance)) << "Should not have been able to set custom attribute on struct class because contaienr types do not match";
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeTest, CanAddSingleCustomAttribute)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP ("ClassWithProperties");
    ASSERT_TRUE (NULL != containerClass);

    ECPropertyP p = containerClass->GetPropertyP ("StringMember");

    IECInstancePtr instance = GetInstanceForClass("CustomAttribClass", *schema);


    EXPECT_EQ(ECObjectsStatus::Success, schema->GetCustomAttributeContainer().SetCustomAttribute(*instance));
    EXPECT_EQ(ECObjectsStatus::Success, p->SetCustomAttribute(*instance));

    EXPECT_EQ(ECObjectsStatus::Success, containerClass->SetCustomAttribute(*instance));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeTest, CanAddMultipleCustomAttributes)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP ("TestClass");
    ASSERT_TRUE (NULL != containerClass);

    IECInstancePtr instance  = GetInstanceForClass("CustomAttribClass",  *schema);
    IECInstancePtr instance2 = GetInstanceForClass("CustomAttribClass2", *schema);
    IECInstancePtr instance3 = GetInstanceForClass("CustomAttribClass3", *schema);
    IECInstancePtr instance4 = GetInstanceForClass("CustomAttribClass4", *schema);

    EXPECT_EQ(ECObjectsStatus::Success, containerClass->SetCustomAttribute(*instance));
    EXPECT_EQ(ECObjectsStatus::Success, containerClass->SetCustomAttribute(*instance2));
    EXPECT_EQ(ECObjectsStatus::Success, containerClass->SetCustomAttribute(*instance3));
    EXPECT_EQ(ECObjectsStatus::Success, containerClass->SetCustomAttribute(*instance4));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeTest, ExpectSuccessWhenAddDuplicateCustomAttribute)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP ("TestClass");
    ASSERT_TRUE (NULL != containerClass);

    IECInstancePtr instance = GetInstanceForClass("CustomAttribClass", *schema);
    EXPECT_EQ(ECObjectsStatus::Success, containerClass->SetCustomAttribute(*instance));
    EXPECT_EQ(ECObjectsStatus::Success, containerClass->SetCustomAttribute(*instance));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeTest, ExpectSuccessWhenAddCustomAttributeToProperty)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP classWithProperties = schema->GetClassP ("ClassWithProperties");
    ASSERT_TRUE (NULL != classWithProperties);
    ECPropertyP stringProperty = classWithProperties->GetPropertyP("StringMember");
    ASSERT_TRUE (NULL != stringProperty);

    IECInstancePtr instance = GetInstanceForClass("CustomAttribClass", *schema);
    EXPECT_EQ(ECObjectsStatus::Success, stringProperty->SetCustomAttribute(*instance));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeTest, ExpectIsDefined)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP ("TestClass");
    ASSERT_TRUE (NULL != containerClass);
    ECClassP customAttributeClass = schema->GetClassP ("CustomAttribClass");

    IECInstancePtr instance = GetInstanceForClass("CustomAttribClass", *schema);
    EXPECT_FALSE(containerClass->IsDefined("TestSchema", "CustomAttribClass"));
    EXPECT_FALSE(containerClass->IsDefined(*customAttributeClass));

    EXPECT_EQ(ECObjectsStatus::Success, containerClass->SetCustomAttribute(*instance));
    EXPECT_TRUE(containerClass->IsDefined("TestSchema", "CustomAttribClass"));
    EXPECT_TRUE(containerClass->IsDefined(*customAttributeClass));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeTest, ExpectIsDefinedOnBaseClass)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP ("TestClass");
    ASSERT_TRUE (NULL != containerClass);
    ECClassP customAttributeClass = schema->GetClassP ("CustomAttribClass");
    ASSERT_TRUE (NULL != customAttributeClass);
    ECClassP baseClass = schema->GetClassP ("BaseClass");
    ASSERT_TRUE (NULL != baseClass);

    IECInstancePtr instance = GetInstanceForClass("CustomAttribClass", *schema);
    EXPECT_FALSE(containerClass->IsDefined("TestSchema", "CustomAttribClass"));
    EXPECT_FALSE(containerClass->IsDefined(*customAttributeClass));

    EXPECT_EQ(ECObjectsStatus::Success, baseClass->SetCustomAttribute(*instance));
    EXPECT_TRUE(baseClass->IsDefined("TestSchema", "CustomAttribClass"));
    EXPECT_TRUE(baseClass->IsDefined(*customAttributeClass));
    EXPECT_TRUE(containerClass->IsDefined("TestSchema", "CustomAttribClass"));
    EXPECT_TRUE(containerClass->IsDefined(*customAttributeClass));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeTest, ExpectCanGetCustomAttribute)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP ("TestClass");
    ASSERT_TRUE (NULL != containerClass);

    IECInstancePtr instance = GetInstanceForClass("CustomAttribClass", *schema);
    EXPECT_EQ(ECObjectsStatus::Success, containerClass->SetCustomAttribute(*instance));

    IECInstancePtr gotInstance = containerClass->GetCustomAttribute("TestSchema", "CustomAttribClass");
    EXPECT_TRUE(gotInstance.IsValid());
    EXPECT_TRUE(instance.get() == gotInstance.get());

    ECClassP caClass = schema->GetClassP("CustomAttribClass");
    ASSERT_TRUE (NULL != caClass);
    gotInstance = containerClass->GetCustomAttribute(*caClass);
    EXPECT_TRUE(gotInstance.IsValid());
    EXPECT_TRUE(instance.get() == gotInstance.get());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeTest, ExpectCanGetAllCustomAttributes)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP ("TestClass");
    ASSERT_TRUE (NULL != containerClass);

    ECClassP baseClass = schema->GetClassP ("BaseClass");
    ASSERT_TRUE (NULL != baseClass);

    IECInstancePtr instance = GetInstanceForClass("CustomAttribClass", *schema);
    EXPECT_EQ(ECObjectsStatus::Success, containerClass->SetCustomAttribute(*instance));

    IECInstancePtr instance2 = GetInstanceForClass("CustomAttribClass2", *schema);
    EXPECT_EQ(ECObjectsStatus::Success, containerClass->SetCustomAttribute(*instance2));

    IECInstancePtr instance3 = GetInstanceForClass("CustomAttribClass3", *schema);
    EXPECT_EQ(ECObjectsStatus::Success, baseClass->SetCustomAttribute(*instance3));

    bool foundCustomAttrib = false;
    bool foundCustomAttrib2 = false;
    bool foundCustomAttrib3 = false;
    ECCustomAttributeInstanceIterable  iterableFalse = containerClass->GetCustomAttributes (false);
    for (IECInstancePtr testInstance: iterableFalse)
        {
        if (testInstance->GetClass().GetName().compare("CustomAttribClass") == 0)
            foundCustomAttrib = true;
        else if (testInstance->GetClass().GetName().compare("CustomAttribClass2") == 0)
            foundCustomAttrib2 = true;
        else if (testInstance->GetClass().GetName().compare("CustomAttribClass3") == 0)
            foundCustomAttrib3 = true;
        }
    EXPECT_TRUE(foundCustomAttrib);
    EXPECT_TRUE(foundCustomAttrib2);
    EXPECT_FALSE(foundCustomAttrib3);

    foundCustomAttrib = false;
    foundCustomAttrib2 = false;
    foundCustomAttrib3 = false;

    ECCustomAttributeInstanceIterable  iterableTrue = containerClass->GetCustomAttributes (true);
    for (IECInstancePtr testInstance: iterableTrue)
        {
        if (testInstance->GetClass().GetName().compare("CustomAttribClass") == 0)
            foundCustomAttrib = true;
        else if (testInstance->GetClass().GetName().compare("CustomAttribClass2") == 0)
            foundCustomAttrib2 = true;
        else if (testInstance->GetClass().GetName().compare("CustomAttribClass3") == 0)
            foundCustomAttrib3 = true;
        }
    EXPECT_TRUE(foundCustomAttrib);
    EXPECT_TRUE(foundCustomAttrib2);
    EXPECT_TRUE(foundCustomAttrib3);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeTest, ExpectCanRemoveCustomAttribute)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP ("TestClass");
    ASSERT_TRUE (NULL != containerClass);

    IECInstancePtr instance = GetInstanceForClass("CustomAttribClass", *schema);
    EXPECT_EQ(ECObjectsStatus::Success, containerClass->SetCustomAttribute(*instance));

    IECInstancePtr gotInstance = containerClass->GetCustomAttribute("TestSchema", "CustomAttribClass");
    EXPECT_TRUE(gotInstance.IsValid());
    EXPECT_TRUE(instance.get() == gotInstance.get());

    EXPECT_TRUE(containerClass->RemoveCustomAttribute("TestSchema", "CustomAttribClass"));
    IECInstancePtr gotInstance2 = containerClass->GetCustomAttribute("TestSchema", "CustomAttribClass");
    EXPECT_FALSE(gotInstance2.IsValid());

    EXPECT_EQ(ECObjectsStatus::Success, containerClass->SetCustomAttribute(*instance));
    ECClassP caClass = schema->GetClassP ("CustomAttribClass");
    ASSERT_TRUE (NULL != caClass);
    EXPECT_TRUE(containerClass->RemoveCustomAttribute(*caClass));
    IECInstancePtr gotInstance3 = containerClass->GetCustomAttribute(*caClass);
    EXPECT_FALSE(gotInstance3.IsValid());


    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeTest, ExpectFailureWithUnreferencedCustomAttribute)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECSchemaPtr refSchema;
    ECSchema::CreateSchema(refSchema, "RefSchema", "ts", 5, 5, 5);

    ECCustomAttributeClassP refClass;
    refSchema->CreateCustomAttributeClass(refClass, "RefClass");

    ECClassP containerClass = schema->GetClassP ("TestClass");
    ASSERT_TRUE (NULL != containerClass);

    IECInstancePtr instance = GetInstanceForClass("RefClass", *refSchema);

    {
    DISABLE_ASSERTS
    EXPECT_EQ(ECObjectsStatus::SchemaNotFound, containerClass->SetCustomAttribute(*instance));
    }
    schema->AddReferencedSchema(*refSchema);
    EXPECT_EQ(ECObjectsStatus::Success, containerClass->SetCustomAttribute(*instance));
    }


#ifdef TEST_DEFECT_D_88458 
//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
// This is a nonsensical scenario, so the defect is being deferred.  But the code is
// here if the defect ever gets reopened 
TEST_F(CustomAttributeTest, ExpectSuccessWhenAddingCircularStructPropertiesToCustomAttributeClass)
    {
    ECSchemaCachePtr schemaOwner = ECSchemaCache::Create(); 
    ECSchemaP schema;    

    //ECObjectsStatus structStatus;
    ECStructClassP struct1;
    ECStructClassP struct2;
    ECCustomAttributeClassP customAttributeClass;
    ECEntityClassP testClass;

    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    ASSERT_TRUE(schema!=NULL);

    schema->CreateStructClass(struct1,"Struct1");
    ASSERT_TRUE(struct1!=NULL);
    schema->CreateStructClass(struct2,"Struct2");
    ASSERT_TRUE(struct2!=NULL);

    StructECPropertyP P1;
    StructECPropertyP P2;

    struct1->CreateStructProperty(P1, "P1",*struct2);
    ASSERT_TRUE(P1!=NULL);
    struct2->CreateStructProperty(P2,"P2",*struct1);
    ASSERT_TRUE(P2!=NULL);

    StructECPropertyP PropertyOfCustomAttribute;

    schema->CreateCustomAttributeClass(customAttributeClass,"MyCustomAttribute");
    ASSERT_TRUE(customAttributeClass!=NULL);


    customAttributeClass->CreateStructProperty(PropertyOfCustomAttribute, "PropertyOfCustomAttribute",*struct1);
    ASSERT_TRUE(PropertyOfCustomAttribute!=NULL);
    //If we comment out the struct property added to custom attribute. It works fine.
    IECInstancePtr instance = GetInstanceForClass("MyCustomAttribute", *schema);
    ASSERT_TRUE(instance.get()!=NULL);

    schema->CreateEntityClass(testClass,"TestClass");
    ASSERT_TRUE(testClass!=NULL);

    ECClassP tempClass = schema->GetClassP("TestClass");
    ASSERT_TRUE(tempClass!=NULL);
    ECObjectsStatus status =tempClass->SetCustomAttribute(*instance);
    ASSERT_EQ(ECObjectsStatus::Success,status);
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (CustomAttributeTest, PresentationMetadataHelper)
    {
    ECSchemaPtr             schema;
    ECEntityClassP                ecclass;
    PrimitiveECPropertyP    primProp;
    ArrayECPropertyP        arrayProp;
    PrimitiveECPropertyP    pointProp;

    ECSchema::CreateSchema (schema, "TestSchema", "ts", 1, 0, 2);
    schema->CreateEntityClass (ecclass, "TestClass");
    ecclass->CreatePrimitiveProperty (primProp, "PrimitiveProperty", PRIMITIVETYPE_String);
    ecclass->CreateArrayProperty (arrayProp, "ArrayProperty", PRIMITIVETYPE_String);
    ecclass->CreatePrimitiveProperty (pointProp, "PointProperty", PRIMITIVETYPE_Point3D);

    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();

    PresentationMetadataHelper meta (*readContext);

    EXPECT_EQ (ECObjectsStatus::Success, meta.SetHideNullProperties (*ecclass));
    EXPECT_TRUE (ecclass->IsDefined ("EditorCustomAttributes", "DontShowNullProperties"));

    EXPECT_EQ (ECObjectsStatus::Success, meta.SetIgnoreZ (*pointProp));
    EXPECT_TRUE (pointProp->IsDefined ("EditorCustomAttributes", "IgnoreZ"));

    EXPECT_EQ (ECObjectsStatus::Success, meta.SetMembersIndependent (*arrayProp));
    EXPECT_TRUE (arrayProp->IsDefined ("EditorCustomAttributes", "MembersIndependent"));

    ECValue v;
    EXPECT_EQ (ECObjectsStatus::Success, meta.SetPriority (*primProp, 1234));
    EXPECT_EQ (ECObjectsStatus::Success, primProp->GetCustomAttribute ("EditorCustomAttributes", "PropertyPriority")->GetValue (v, "Priority"));
    EXPECT_EQ (1234, v.GetInteger());

    EXPECT_EQ (ECObjectsStatus::Success, meta.SetAlwaysExpand (*arrayProp, true));
    EXPECT_EQ (ECObjectsStatus::Success, arrayProp->GetCustomAttribute ("EditorCustomAttributes", "AlwaysExpand")->GetValue (v, "ArrayMembers"));
    EXPECT_TRUE (v.GetBoolean());

    EXPECT_EQ (ECObjectsStatus::Success, meta.SetExtendedType (*primProp, 1));
    EXPECT_EQ (ECObjectsStatus::Success, primProp->GetCustomAttribute ("EditorCustomAttributes", "ExtendType")->GetValue (v, "Standard"));
    EXPECT_EQ (1, v.GetInteger());

    Utf8String str ("CustomType");
    EXPECT_EQ (ECObjectsStatus::Success, meta.SetExtendedType (*primProp, str.c_str()));
    EXPECT_EQ (ECObjectsStatus::Success, primProp->GetCustomAttribute ("EditorCustomAttributes", "ExtendType")->GetValue (v, "Name"));
    EXPECT_TRUE (str.Equals (v.GetUtf8CP()));

    EXPECT_EQ (ECObjectsStatus::Success, meta.SetMemberExtendedType (*arrayProp, 1));
    EXPECT_EQ (ECObjectsStatus::Success, arrayProp->GetCustomAttribute ("MemberExtendedType")->GetValue (v, "Standard"));
    EXPECT_EQ (1, v.GetInteger());

    EXPECT_EQ (ECObjectsStatus::Success, meta.SetMemberExtendedType (*arrayProp, str.c_str()));
    EXPECT_EQ (ECObjectsStatus::Success, arrayProp->GetCustomAttribute ("EditorCustomAttributes", "MemberExtendedType")->GetValue (v, "Name"));
    EXPECT_TRUE (str.Equals (v.GetUtf8CP()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (CustomAttributeTest, SerializeSchemaToXmlUtfString)
    {
#ifdef _WIN32
    Utf8CP const testClassName = "TestClass";

    Utf8CP const caClassName = "CustomAttribClass";
    Utf8CP const caWCharStringPropName = "WCharStringProperty";
    Utf8CP const caUtf8StringPropName = "Utf8StringProperty";
    ECSchemaPtr expectedSchema = CreateCustomAttributeTestSchema ();
    ECClassP caClass = expectedSchema->GetClassP (caClassName);
    EXPECT_TRUE (caClass != NULL) << "Test custom attribute class " << caClassName << " not found";
    PrimitiveECPropertyP stringProp = NULL;
    caClass->CreatePrimitiveProperty (stringProp, caWCharStringPropName, PRIMITIVETYPE_String);
    caClass->CreatePrimitiveProperty (stringProp, caUtf8StringPropName, PRIMITIVETYPE_String);

        // *** WIP_PORTABILITY: Use an escape such as \u here. Don't try to use extended ascii directly
    WCharCP caPropValueString = L"äöüßá³µ";
    Utf8String expectedCAPropValueUtf8String;
    EXPECT_EQ (SUCCESS, BeStringUtilities::WCharToUtf8 (expectedCAPropValueUtf8String, caPropValueString));

    IECInstancePtr caInstance = GetInstanceForClass (caClassName, *expectedSchema);
    ECValue expectedWCharValue (caPropValueString);
    EXPECT_EQ (ECObjectsStatus::Success, caInstance->SetValue (caWCharStringPropName, expectedWCharValue)) << "Assigning value to " << caWCharStringPropName << " in custom attribute instance failed.";

    ECValue expectedUtf8Value (expectedCAPropValueUtf8String.c_str ());
    EXPECT_EQ (ECObjectsStatus::Success, caInstance->SetValue (caUtf8StringPropName, expectedUtf8Value)) << "Assigning value to " << caUtf8StringPropName << " in custom attribute instance failed.";

    ECClassP testClass = expectedSchema->GetClassP (testClassName);
    EXPECT_TRUE (testClass != NULL) << "Test class " << testClassName << " not found";
    EXPECT_EQ (ECObjectsStatus::Success, testClass->SetCustomAttribute (*caInstance)) << "Assigning the custom attribute instance to class " << testClassName << " failed.";

    //serializing
    Utf8String schemaUtfXml;
    SchemaWriteStatus serializeStat = expectedSchema->WriteToXmlString (schemaUtfXml);
    EXPECT_EQ (SchemaWriteStatus::Success, serializeStat) << "Serializing the schema to a UTF-8 XML string failed.";

    //deserializing
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr actualSchema = NULL;
    SchemaReadStatus deserializeStat = ECSchema::ReadFromXmlString (actualSchema, schemaUtfXml.c_str (), *schemaContext);
    EXPECT_EQ (SchemaReadStatus::Success, deserializeStat) << "Deserializing the schema from a UTF-8 XML string failed.";

    //base check of actual vs expected schema
    VerifyCustomAttributeTestSchema (actualSchema);
    //now check CA added in this test
    ECClassCP actualCAClass = actualSchema->GetClassCP (caClassName);
    EXPECT_TRUE (actualCAClass->GetPropertyP (caWCharStringPropName) != NULL) << "Property " << caWCharStringPropName << " not found in custom attribute class.";
    //now check CA instance added in this test
    ECClassCP actualTestClass = actualSchema->GetClassCP (testClassName);

    IECInstancePtr actualCAInstance = actualTestClass->GetCustomAttribute (*actualCAClass);
    ASSERT_TRUE (actualCAInstance.IsValid ()) << "Test class " << testClassName << " doesn't have the expected custom attribute instance.";

    //verify UTF-8 property value
    ECValue actualUtf8Value;
    ASSERT_EQ (ECObjectsStatus::Success, actualCAInstance->GetValue (actualUtf8Value, caUtf8StringPropName)) << "Property " << caUtf8StringPropName << "not found in custom attribute instance on test class.";
    EXPECT_TRUE (expectedUtf8Value.Equals (actualUtf8Value)) << "Unexpected ECValue of property " << caUtf8StringPropName << " of custom attribute instance";
    EXPECT_STREQ (expectedCAPropValueUtf8String.c_str(), actualUtf8Value.GetUtf8CP ()) << "Unexpected string value of property " << caUtf8StringPropName << " of custom attribute instance";
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CustomAttributeTest, ContainerTypes)
    {
    CustomAttributeContainerType schema = CustomAttributeContainerType::Schema;
    CustomAttributeContainerType entityAndSchema = CustomAttributeContainerType::Schema | CustomAttributeContainerType::EntityClass;

    ASSERT_TRUE(CustomAttributeContainerType::Schema == (schema & CustomAttributeContainerType::Schema));
    ASSERT_TRUE(CustomAttributeContainerType::Schema != (schema & CustomAttributeContainerType::EntityClass));
    ASSERT_TRUE(CustomAttributeContainerType::EntityClass == (entityAndSchema & CustomAttributeContainerType::EntityClass));

    int schemaAsInt = static_cast<int>(schema);
    ASSERT_TRUE(CustomAttributeContainerType::Schema == (static_cast<CustomAttributeContainerType>(schemaAsInt) & CustomAttributeContainerType::Schema));

    int entityAndSchemaAsInt = static_cast<int>(entityAndSchema);
    ASSERT_TRUE(CustomAttributeContainerType::EntityClass == (static_cast<CustomAttributeContainerType>(entityAndSchemaAsInt) & CustomAttributeContainerType::EntityClass));

    ASSERT_TRUE(CustomAttributeContainerType::EntityClass == (CustomAttributeContainerType::EntityClass & CustomAttributeContainerType::Any));
    ASSERT_TRUE(CustomAttributeContainerType::EntityClass == (CustomAttributeContainerType::EntityClass & CustomAttributeContainerType::AnyClass));
    ASSERT_TRUE(CustomAttributeContainerType::EntityClass != (CustomAttributeContainerType::EntityClass & CustomAttributeContainerType::AnyProperty));
    }

bool TestValue(CustomAttributeContainerType compareType, CustomAttributeContainerType myType)
    {
    if (compareType == (compareType & myType))
        return true;
    return false;
    }

TEST_F(CustomAttributeTest, FailsToLoadSchemaWithInvalidCustomAttributes_EC3)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;

    Utf8Char schemaXML[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "    <ECCustomAttributes>"
        "        <AppliesToClass xmlns=\"TestSchema.01.00\"/>"
        "    </ECCustomAttributes>"
        "    <ECCustomAttributeClass typeName=\"AppliesToClass\" appliesTo=\"AnyClass\" />"
        "</ECSchema>";

    SchemaReadStatus status;
    {
    DISABLE_ASSERTS
    status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);
    }
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "Expected deserialization to fail when custom attribute is applied to a container that conflicts with the CAs appliesTo attribute";

    Utf8Char schemaXML2[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "    <ECCustomAttributes>"
        "        <DoesNotExist xmlns=\"TestSchema.01.00\"/>"
        "    </ECCustomAttributes>"
        "    <ECCustomAttributeClass typeName=\"AppliesToClass\" appliesTo=\"AnyClass\" />"
        "</ECSchema>";
    {
    DISABLE_ASSERTS
    status = ECSchema::ReadFromXmlString(schema, schemaXML2, *schemaContext);
    }
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "Expected deserialization to fail when custom attribute is applied but it's class cannot be found";
    }

TEST_F(CustomAttributeTest, CanLoadSchemaWithInvalidCustomAttributes_EC2)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;

    Utf8Char schemaXML[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "    <ECCustomAttributes>"
        "        <DoesNotExist xmlns=\"TestSchema.01.00\"/>"
        "    </ECCustomAttributes>"
        "    <ECClass typeName=\"AppliesToClass\"/>"
        "</ECSchema>";

    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "Expected deserialization to succeed when custom attribute is applied but it's class cannot be found";
    }

TEST_F(CustomAttributeTest, InvalidContainerTypeDeserialization)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;

    Utf8Char schemaXML[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "    <ECCustomAttributeClass typeName=\"AppliesToGarbage\" appliesTo=\"Bad\" />"
        "</ECSchema>";
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "Expected deserialization to fail when appliesTo attribute is an invalid value";


    Utf8Char schemaXML2[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "    <ECCustomAttributeClass typeName=\"AppliesToGarbage\" appliesTo=\"\" />"
        "</ECSchema>";
    status = ECSchema::ReadFromXmlString(schema, schemaXML2, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "Expected deserialization to fail when appliesTo attribute is an empty value";
    }

TEST_F(CustomAttributeTest, ContainerTypeSerialization)
    {
    Utf8Char schemaXML[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "    <ECCustomAttributeClass typeName=\"AppliesToSchema\" appliesTo=\"Schema\" />"
        "    <ECCustomAttributeClass typeName=\"AppliesToAnyClass\" appliesTo=\"AnyClass\" />"
        "    <ECCustomAttributeClass typeName=\"AppliesToAny\" appliesTo=\"Any\" />"
        "    <ECCustomAttributeClass typeName=\"AppliesToSchemaAndEntity\" appliesTo=\"Schema,EntityClass\" />"
        "    <ECCustomAttributeClass typeName=\"AppliesToSchemaEntityClassPrimitiveProperty\" appliesTo=\"Schema,EntityClass,PrimitiveProperty\" />"
        "    <ECCustomAttributeClass typeName=\"AppliesToNotSet\" />"
        "</ECSchema>";

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);

    ECCustomAttributeClassCP appliesToSchema = schema->GetClassCP("AppliesToSchema")->GetCustomAttributeClassCP();
    ECCustomAttributeClassCP appliesToAnyClass = schema->GetClassCP("AppliesToAnyClass")->GetCustomAttributeClassCP();
    ECCustomAttributeClassCP appliesToAny = schema->GetClassCP("AppliesToAny")->GetCustomAttributeClassCP();
    ECCustomAttributeClassCP appliesToSchemaAndEntity = schema->GetClassCP("AppliesToSchemaAndEntity")->GetCustomAttributeClassCP();
    ECCustomAttributeClassCP appliesToSchemaEntityClassPrimitiveProperty = schema->GetClassCP("AppliesToSchemaEntityClassPrimitiveProperty")->GetCustomAttributeClassCP();
    ECCustomAttributeClassCP appliesToNotSet = schema->GetClassCP("AppliesToNotSet")->GetCustomAttributeClassCP();

    EXPECT_TRUE(TestValue(CustomAttributeContainerType::Schema, appliesToSchema->GetContainerType()));
    EXPECT_FALSE(TestValue(CustomAttributeContainerType::Any, appliesToSchema->GetContainerType()));
    EXPECT_FALSE(TestValue(CustomAttributeContainerType::EntityClass, appliesToSchema->GetContainerType()));

    EXPECT_TRUE(TestValue(CustomAttributeContainerType::AnyClass, appliesToAnyClass->GetContainerType()));
    EXPECT_TRUE(TestValue(CustomAttributeContainerType::EntityClass, appliesToAnyClass->GetContainerType()));
    EXPECT_TRUE(TestValue(CustomAttributeContainerType::RelationshipClass, appliesToAnyClass->GetContainerType()));
    EXPECT_FALSE(TestValue(CustomAttributeContainerType::Schema, appliesToAnyClass->GetContainerType()));

    EXPECT_TRUE(TestValue(CustomAttributeContainerType::Any, appliesToAny->GetContainerType()));
    EXPECT_TRUE(TestValue(CustomAttributeContainerType::AnyClass, appliesToAny->GetContainerType()));
    EXPECT_TRUE(TestValue(CustomAttributeContainerType::EntityClass, appliesToAny->GetContainerType()));
    EXPECT_TRUE(TestValue(CustomAttributeContainerType::RelationshipClass, appliesToAny->GetContainerType()));

    EXPECT_TRUE(TestValue(CustomAttributeContainerType::Schema, appliesToSchemaAndEntity->GetContainerType()));
    EXPECT_TRUE(TestValue(CustomAttributeContainerType::EntityClass, appliesToSchemaAndEntity->GetContainerType()));
    EXPECT_FALSE(TestValue(CustomAttributeContainerType::RelationshipClass, appliesToSchemaAndEntity->GetContainerType()));

    EXPECT_TRUE(TestValue(CustomAttributeContainerType::Schema, appliesToSchemaEntityClassPrimitiveProperty->GetContainerType()));
    EXPECT_TRUE(TestValue(CustomAttributeContainerType::EntityClass, appliesToSchemaEntityClassPrimitiveProperty->GetContainerType()));
    EXPECT_TRUE(TestValue(CustomAttributeContainerType::PrimitiveProperty, appliesToSchemaEntityClassPrimitiveProperty->GetContainerType()));
    EXPECT_FALSE(TestValue(CustomAttributeContainerType::RelationshipClass, appliesToSchemaEntityClassPrimitiveProperty->GetContainerType()));

    EXPECT_TRUE(TestValue(CustomAttributeContainerType::Any, appliesToNotSet->GetContainerType()));

    Utf8String serializedStr;
    SchemaWriteStatus status2 = schema->WriteToXmlString(serializedStr, 3, 0);
    EXPECT_EQ(SchemaWriteStatus::Success, status2);
    }

TEST_F(CustomAttributeTest, TestCustomAttributesWithSameNameInDifferentSchemas)
    {
    ECSchemaPtr schema1;
    ECSchemaPtr schema2;
    ECSchemaPtr testSchema;
    ECCustomAttributeClassP customAttributeClass1;
    ECCustomAttributeClassP customAttributeClass2;

    ECSchema::CreateSchema(schema1, "CASchema1", "ts", 5, 5, 5);
    schema1->CreateCustomAttributeClass(customAttributeClass1, "CustomAttribClass");

    ECSchema::CreateSchema(schema2, "CASchema2", "ts", 5, 5, 5);
    schema2->CreateCustomAttributeClass(customAttributeClass2, "CustomAttribClass");
    
    IECInstancePtr caInstance1 = GetInstanceForClass("CustomAttribClass", *schema1);
    IECInstancePtr caInstance2 = GetInstanceForClass("CustomAttribClass", *schema2);
        
    ECSchema::CreateSchema(testSchema, "TestSchema", "ts", 5, 5, 5);
    testSchema->AddReferencedSchema(*schema1);
    testSchema->AddReferencedSchema(*schema2);
    IECCustomAttributeContainer& schemaCustomAttributeContainer = testSchema->GetCustomAttributeContainer();
        
    EXPECT_EQ (ECObjectsStatus::Success, schemaCustomAttributeContainer.SetCustomAttribute(*caInstance1)) << "Assigning the custom attribute instance to class CASchema1.CustomAttribClass failed.";
    EXPECT_EQ (ECObjectsStatus::Success, schemaCustomAttributeContainer.SetCustomAttribute(*caInstance2)) << "Assigning the custom attribute instance to class CASchema2.CustomAttribClass failed.";

    EXPECT_TRUE(schemaCustomAttributeContainer.IsDefined("CASchema1", "CustomAttribClass")) << "CustomAttribute CATestSchema1.TestClass couldn't be found on TestSchema.";
    EXPECT_TRUE(schemaCustomAttributeContainer.IsDefined("CASchema2", "CustomAttribClass")) << "CustomAttribute CATestSchema2.TestClass couldn't be found on TestSchema.";
    EXPECT_TRUE(schemaCustomAttributeContainer.IsDefined("CustomAttribClass")) << "CustomAttribute TestClass couldn't be found on TestSchema."; // LEGENCY TEST
        
    ASSERT_TRUE (schemaCustomAttributeContainer.GetCustomAttribute ("CASchema1", "CustomAttribClass").IsValid ());
    ASSERT_TRUE (schemaCustomAttributeContainer.GetCustomAttribute ("CASchema2", "CustomAttribClass").IsValid ());
                
    EXPECT_TRUE(schemaCustomAttributeContainer.RemoveCustomAttribute("CASchema2", "CustomAttribClass"));
    EXPECT_FALSE(schemaCustomAttributeContainer.IsDefined("CASchema2", "CustomAttribClass")) << "CustomAttribute CATestSchema2.TestClass was still found although it should have been removed.";
    EXPECT_TRUE(schemaCustomAttributeContainer.IsDefined("CustomAttribClass"));
        
    EXPECT_TRUE(schemaCustomAttributeContainer.RemoveCustomAttribute("CASchema1", "CustomAttribClass"));
    EXPECT_FALSE(schemaCustomAttributeContainer.IsDefined("CASchema1", "CustomAttribClass")) << "CustomAttribute CATestSchema1.TestClass was still found although it should have been removed.";
    EXPECT_FALSE(schemaCustomAttributeContainer.IsDefined("CustomAttribClass"));
    }
        
END_BENTLEY_ECN_TEST_NAMESPACE
