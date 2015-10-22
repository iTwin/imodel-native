/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/CustomAttributeTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"

#include "TestFixture.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

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

    PrimitiveECPropertyP stringProp = NULL;
    EXPECT_EQ(ECOBJECTS_STATUS_Success, classWithProperties->CreatePrimitiveProperty(stringProp, L"StringMember", PRIMITIVETYPE_String));
    EXPECT_EQ(baseStringProp, stringProp->GetBaseProperty());

    return schema;
    }                                    

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
void VerifyCustomAttributeTestSchema (ECSchemaPtr customAttributeSchema)
    {
    EXPECT_TRUE (customAttributeSchema.IsValid ()) << L"Custom attribute schema is null.";
    EXPECT_STREQ (L"TestSchema.05.05", customAttributeSchema->GetFullSchemaName ().c_str ()) << L"Custom attribute schema full name mismatches.";
    
    WCharCP className = L"CustomAttribClass";
    EXPECT_TRUE (customAttributeSchema->GetClassCP (className) != NULL) << L"Class " << className << L" not found in custom attribute schema.";

    className = L"CustomAttribClass2";
    EXPECT_TRUE (customAttributeSchema->GetClassCP (className) != NULL) << L"Class " << className << L" not found in custom attribute schema.";

    className = L"CustomAttribClass3";
    EXPECT_TRUE (customAttributeSchema->GetClassCP (className) != NULL) << L"Class " << className << L" not found in custom attribute schema.";

    className = L"CustomAttribClass4";
    EXPECT_TRUE (customAttributeSchema->GetClassCP (className) != NULL) << L"Class " << className << L" not found in custom attribute schema.";

    WCharCP baseClassName = L"BaseClass";
    ECClassCP ecClass = customAttributeSchema->GetClassCP (baseClassName);
    ASSERT_TRUE (ecClass != NULL) << L"Class " << baseClassName << L" not found in custom attribute schema.";
    WCharCP propertyName = L"StringMember";
    EXPECT_TRUE (ecClass->GetPropertyP (propertyName, false) != NULL) << L"Property " << propertyName << L" not found in class " << baseClassName;
    
    className = L"TestClass";
    ecClass = customAttributeSchema->GetClassCP (className);
    ASSERT_TRUE (ecClass != NULL) << L"Class " << className << L" not found in custom attribute schema.";
    const ECBaseClassesList& baseClassList = ecClass->GetBaseClasses ();
    EXPECT_EQ (1, baseClassList.size ()) << L"Class " << className << L"is expected to only have one base class";
    EXPECT_STREQ (baseClassName, baseClassList[0]->GetName ().c_str ()) << L"Unexpected base class of class " << className;

    propertyName = L"StringMember";
    EXPECT_TRUE (ecClass->GetPropertyP (propertyName, true) != NULL) << L"Inherited property " << propertyName << L" not found in class " << className;

    className = L"ClassWithProperties";
    ecClass = customAttributeSchema->GetClassCP (className);
    ASSERT_TRUE (ecClass != NULL) << L"Class " << className << L" not found in custom attribute schema.";
    const ECBaseClassesList& baseClassList2 = ecClass->GetBaseClasses ();
    EXPECT_EQ (1, baseClassList2.size ()) << L"Class " << className << L"is expected to only have one base class";
    EXPECT_STREQ (baseClassName, baseClassList2[0]->GetName ().c_str ()) << L"Unexpected base class of class " << className;

    propertyName = L"StringMember";
    EXPECT_TRUE (ecClass->GetPropertyP (propertyName, false) != NULL) << L"Overridden property " << propertyName << L" not found in class " << className;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
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


    EXPECT_EQ(ECOBJECTS_STATUS_Success, schema->GetCustomAttributeContainer().SetCustomAttribute(*instance));
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
    EXPECT_FALSE(containerClass->IsDefined(L"TestSchema", L"CustomAttribClass"));
    EXPECT_FALSE(containerClass->IsDefined(*customAttributeClass));

    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(*instance));
    EXPECT_TRUE(containerClass->IsDefined(L"TestSchema", L"CustomAttribClass"));
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
    EXPECT_FALSE(containerClass->IsDefined(L"TestSchema", L"CustomAttribClass"));
    EXPECT_FALSE(containerClass->IsDefined(*customAttributeClass));

    EXPECT_EQ(ECOBJECTS_STATUS_Success, baseClass->SetCustomAttribute(*instance));
    EXPECT_TRUE(baseClass->IsDefined(L"TestSchema", L"CustomAttribClass"));
    EXPECT_TRUE(baseClass->IsDefined(*customAttributeClass));
    EXPECT_TRUE(containerClass->IsDefined(L"TestSchema", L"CustomAttribClass"));
    EXPECT_TRUE(containerClass->IsDefined(*customAttributeClass));
    }

TEST_F(CustomAttributeTest, ExpectCanGetCustomAttribute)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (NULL != containerClass);

    IECInstancePtr instance = GetInstanceForClass(L"CustomAttribClass", *schema);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(*instance));

    IECInstancePtr gotInstance = containerClass->GetCustomAttribute(L"TestSchema", L"CustomAttribClass");
    EXPECT_TRUE(gotInstance.IsValid());
    EXPECT_TRUE(instance.get() == gotInstance.get());

    ECClassP caClass = schema->GetClassP(L"CustomAttribClass");
    ASSERT_TRUE (NULL != caClass);
    gotInstance = containerClass->GetCustomAttribute(*caClass);
    EXPECT_TRUE(gotInstance.IsValid());
    EXPECT_TRUE(instance.get() == gotInstance.get());
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
    for (IECInstancePtr testInstance: iterableFalse)
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
    for (IECInstancePtr testInstance: iterableTrue)
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

    IECInstancePtr gotInstance = containerClass->GetCustomAttribute(L"TestSchema", L"CustomAttribClass");
    EXPECT_TRUE(gotInstance.IsValid());
    EXPECT_TRUE(instance.get() == gotInstance.get());

    EXPECT_TRUE(containerClass->RemoveCustomAttribute(L"TestSchema", L"CustomAttribClass"));
    IECInstancePtr gotInstance2 = containerClass->GetCustomAttribute(L"TestSchema", L"CustomAttribClass");
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

TEST_F (CustomAttributeTest, PresentationMetadataHelper)
    {
    ECSchemaPtr             schema;
    ECClassP                ecclass;
    PrimitiveECPropertyP    primProp;
    ArrayECPropertyP        arrayProp;
    PrimitiveECPropertyP    pointProp;

    ECSchema::CreateSchema (schema, L"TestSchema", 1, 2);
    schema->CreateClass (ecclass, L"TestClass");
    ecclass->CreatePrimitiveProperty (primProp, L"PrimitiveProperty", PRIMITIVETYPE_String);
    ecclass->CreateArrayProperty (arrayProp, L"ArrayProperty", PRIMITIVETYPE_String);
    ecclass->CreatePrimitiveProperty (pointProp, L"PointProperty", PRIMITIVETYPE_Point3D);

    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();

    PresentationMetadataHelper meta (*readContext);

    EXPECT_EQ (ECOBJECTS_STATUS_Success, meta.SetHideNullProperties (*ecclass));
    EXPECT_TRUE (ecclass->IsDefined (L"EditorCustomAttributes", L"DontShowNullProperties"));

    EXPECT_EQ (ECOBJECTS_STATUS_Success, meta.SetIgnoreZ (*pointProp));
    EXPECT_TRUE (pointProp->IsDefined (L"EditorCustomAttributes", L"IgnoreZ"));

    EXPECT_EQ (ECOBJECTS_STATUS_Success, meta.SetMembersIndependent (*arrayProp));
    EXPECT_TRUE (arrayProp->IsDefined (L"EditorCustomAttributes", L"MembersIndependent"));

    ECValue v;
    EXPECT_EQ (ECOBJECTS_STATUS_Success, meta.SetPriority (*primProp, 1234));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, primProp->GetCustomAttribute (L"EditorCustomAttributes", L"PropertyPriority")->GetValue (v, L"Priority"));
    EXPECT_EQ (1234, v.GetInteger());

    EXPECT_EQ (ECOBJECTS_STATUS_Success, meta.SetAlwaysExpand (*arrayProp, true));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, arrayProp->GetCustomAttribute (L"EditorCustomAttributes", L"AlwaysExpand")->GetValue (v, L"ArrayMembers"));
    EXPECT_TRUE (v.GetBoolean());

    EXPECT_EQ (ECOBJECTS_STATUS_Success, meta.SetExtendedType (*primProp, 1));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, primProp->GetCustomAttribute (L"EditorCustomAttributes", L"ExtendType")->GetValue (v, L"Standard"));
    EXPECT_EQ (1, v.GetInteger());

    WString str (L"CustomType");
    EXPECT_EQ (ECOBJECTS_STATUS_Success, meta.SetExtendedType (*primProp, str.c_str()));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, primProp->GetCustomAttribute (L"EditorCustomAttributes", L"ExtendType")->GetValue (v, L"Name"));
    EXPECT_TRUE (str.Equals (v.GetString()));

    EXPECT_EQ (ECOBJECTS_STATUS_Success, meta.SetMemberExtendedType (*arrayProp, 1));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, arrayProp->GetCustomAttribute (L"EditorCustomAttributes", L"MemberExtendedType")->GetValue (v, L"Standard"));
    EXPECT_EQ (1, v.GetInteger());

    EXPECT_EQ (ECOBJECTS_STATUS_Success, meta.SetMemberExtendedType (*arrayProp, str.c_str()));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, arrayProp->GetCustomAttribute (L"EditorCustomAttributes", L"MemberExtendedType")->GetValue (v, L"Name"));
    EXPECT_TRUE (str.Equals (v.GetString()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (CustomAttributeTest, SerializeSchemaToXmlUtfString)
    {
    WCharCP const testClassName = L"TestClass";

    WCharCP const caClassName = L"CustomAttribClass";
    WCharCP const caStringPropName = L"StringProperty";
    ECSchemaPtr expectedSchema = CreateCustomAttributeTestSchema ();
    ECClassP caClass = expectedSchema->GetClassP (caClassName);
    EXPECT_TRUE (caClass != NULL) << L"Test custom attribute class " << caClassName << L" not found";
    PrimitiveECPropertyP stringProp = NULL;
    caClass->CreatePrimitiveProperty (stringProp, caStringPropName, PRIMITIVETYPE_String);

    Utf8String expectedCAString;
    EXPECT_EQ (SUCCESS, BeStringUtilities::WCharToUtf8 (expectedCAString, L"äöüßá³µ"));
    IECInstancePtr caInstance = GetInstanceForClass (caClassName, *expectedSchema);
    ECValue expectedUtf8Value (expectedCAString.c_str ());
    EXPECT_EQ (ECOBJECTS_STATUS_Success, caInstance->SetValue (caStringPropName, expectedUtf8Value)) << L"Assigning property value to custom attribute instance failed.";

    ECClassP testClass = expectedSchema->GetClassP (testClassName);
    EXPECT_TRUE (testClass != NULL) << L"Test class " << testClass << L" not found";
    EXPECT_EQ (ECOBJECTS_STATUS_Success, testClass->SetCustomAttribute (*caInstance)) << L"Assigning the custom attribute instance to class " << testClassName << L" failed.";

    //serializing
    Utf8String schemaUtfXml;
    SchemaWriteStatus serializeStat = expectedSchema->WriteToXmlString (schemaUtfXml);
    EXPECT_EQ (SCHEMA_WRITE_STATUS_Success, serializeStat) << L"Serializing the schema to a UTF-8 XML string failed.";

    //deserializing
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr actualSchema = NULL;
    SchemaReadStatus deserializeStat = ECSchema::ReadFromXmlString (actualSchema, schemaUtfXml.c_str (), *schemaContext);
    EXPECT_EQ (SCHEMA_READ_STATUS_Success, deserializeStat) << L"Deserializing the schema from a UTF-8 XML string failed.";

    //base check of actual vs expected schema
    VerifyCustomAttributeTestSchema (actualSchema);
    //now check CA added in this test
    ECClassCP actualCAClass = actualSchema->GetClassCP (caClassName);
    EXPECT_TRUE (actualCAClass->GetPropertyP (caStringPropName) != NULL) << L"Property " << caStringPropName << L" not found in custom attribute class.";
    //now check CA instance added in this test
    ECClassCP actualTestClass = actualSchema->GetClassCP (testClassName);

    IECInstancePtr actualCAInstance = actualTestClass->GetCustomAttribute (*actualCAClass);
    ASSERT_TRUE (actualCAInstance.IsValid ()) << L"Test class " << testClassName << L" doesn't have the expected custom attribute instance.";

    ECValue actualUtf8Value;
    ASSERT_EQ (ECOBJECTS_STATUS_Success, actualCAInstance->GetValue (actualUtf8Value, caStringPropName)) << L"Property " << caStringPropName << L"not found in custom attribute instance on test class.";
    EXPECT_TRUE (expectedUtf8Value.Equals (actualUtf8Value)) << L"Unexpected string property value of custom attribute instance";
    EXPECT_STREQ (expectedCAString.c_str (), actualUtf8Value.GetUtf8CP ()) << L"Unexpected string value of custom attribute property value";
    }
		
TEST_F(CustomAttributeTest, TestCustomAttributesWithSameNameInDifferentSchemas)
    {
    ECSchemaPtr schema1;
    ECSchemaPtr schema2;
    ECSchemaPtr testSchema;
    ECClassP customAttributeClass1;
    ECClassP customAttributeClass2;

    ECSchema::CreateSchema(schema1, L"CASchema1", 5, 5);
    schema1->CreateClass(customAttributeClass1, L"CustomAttribClass");
    customAttributeClass1->SetIsCustomAttributeClass(true);

		ECSchema::CreateSchema(schema2, L"CASchema2", 5, 5);
    schema2->CreateClass(customAttributeClass2, L"CustomAttribClass");
    customAttributeClass2->SetIsCustomAttributeClass(true);
    
		IECInstancePtr caInstance1 = GetInstanceForClass(L"CustomAttribClass", *schema1);
		IECInstancePtr caInstance2 = GetInstanceForClass(L"CustomAttribClass", *schema2);
		
		ECSchema::CreateSchema(testSchema, L"TestSchema", 5, 5);
		EXPECT_EQ (ECOBJECTS_STATUS_Success, testSchema->SetCustomAttribute(*caInstance1)) << L"Assigning the custom attribute instance to class CASchema1.CustomAttribClass failed.";
		EXPECT_EQ (ECOBJECTS_STATUS_Success, testSchema->SetCustomAttribute(*caInstance2)) << L"Assigning the custom attribute instance to class CASchema2.CustomAttribClass failed.";

		EXPECT_TRUE(containerClass->IsDefined(L"TestSchema1", L"CustomAttribClass")) << L"CustomAttribute CATestSchema1.TestClass couldn't be found on TestSchema.";
		EXPECT_TRUE(containerClass->IsDefined(L"TestSchema2", L"CustomAttribClass")) << L"CustomAttribute CATestSchema2.TestClass couldn't be found on TestSchema.";
		EXPECT_TRUE(containerClass->IsDefined(L"CustomAttribClass")) << L"CustomAttribute TestClass couldn't be found on TestSchema."; // LEGENCY TEST
		
    ASSERT_TRUE (testSchema->GetCustomAttribute (L"TestSchema1", L"CustomAttribClass").IsValid ());
    ASSERT_TRUE (testSchema->GetCustomAttribute (L"TestSchema2", L"CustomAttribClass").IsValid ());
				
		EXPECT_TRUE(containerClass->RemoveCustomAttribute(L"TestSchema2", L"CustomAttribClass"));
		EXPECT_FALSE(containerClass->IsDefined(L"TestSchema2", L"CustomAttribClass")) << L"CustomAttribute CATestSchema2.TestClass was still found although it should have been removed.";
		EXPECT_TRUE(containerClass->IsDefined(L"CustomAttribClass"));
		
		EXPECT_TRUE(containerClass->RemoveCustomAttribute(L"TestSchema1", L"CustomAttribClass"));
		EXPECT_FALSE(containerClass->IsDefined(L"TestSchema1", L"CustomAttribClass")) << L"CustomAttribute CATestSchema1.TestClass was still found although it should have been removed.";
		EXPECT_FALSE(containerClass->IsDefined(L"CustomAttribClass"));
    }

END_BENTLEY_ECOBJECT_NAMESPACE
