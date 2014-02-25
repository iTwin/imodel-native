/*--------------------------------------------------------------------------------------+
|
|     $Source: tests/Published/CustomAttributeTests.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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
//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeTest, ExpectFailureWhenSetNonCustomAttributeClass)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (NULL != containerClass);

    IECInstancePtr instance = GetInstanceForClass(L"BaseClass", *schema);
    EXPECT_EQ(ECOBJECTS_STATUS_NotCustomAttributeClass, containerClass->SetCustomAttribute(*instance));
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeTest, ExpectSuccessWhenAddDuplicateCustomAttribute)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (NULL != containerClass);

    IECInstancePtr instance = GetInstanceForClass(L"CustomAttribClass", *schema);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(*instance));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(*instance));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeTest, ExpectCanGetCustomAttribute)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (NULL != containerClass);

    IECInstancePtr instance = GetInstanceForClass(L"CustomAttribClass", *schema);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(*instance));

    IECInstancePtr gotInstance = containerClass->GetCustomAttribute(L"CustomAttribClass");
    EXPECT_TRUE(gotInstance.IsValid());
    EXPECT_TRUE(instance.get() == gotInstance.get());

    ECClassP caClass = schema->GetClassP(L"CustomAttribClass");
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CustomAttributeTest, ExpectCanRemoveCustomAttribute)
    {
    
    ECSchemaPtr         schema = CreateCustomAttributeTestSchema();

    ECClassP containerClass = schema->GetClassP (L"TestClass");
    ASSERT_TRUE (NULL != containerClass);

    IECInstancePtr instance = GetInstanceForClass(L"CustomAttribClass", *schema);
    EXPECT_EQ(ECOBJECTS_STATUS_Success, containerClass->SetCustomAttribute(*instance));

    IECInstancePtr gotInstance = containerClass->GetCustomAttribute(L"CustomAttribClass");
    EXPECT_TRUE(gotInstance.IsValid());
    EXPECT_TRUE(instance.get() == gotInstance.get());

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
//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
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
    EXPECT_TRUE (ecclass->IsDefined (L"DontShowNullProperties"));

    EXPECT_EQ (ECOBJECTS_STATUS_Success, meta.SetIgnoreZ (*pointProp));
    EXPECT_TRUE (pointProp->IsDefined (L"IgnoreZ"));

    EXPECT_EQ (ECOBJECTS_STATUS_Success, meta.SetMembersIndependent (*arrayProp));
    EXPECT_TRUE (arrayProp->IsDefined (L"MembersIndependent"));

    ECValue v;
    EXPECT_EQ (ECOBJECTS_STATUS_Success, meta.SetPriority (*primProp, 1234));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, primProp->GetCustomAttribute (L"PropertyPriority")->GetValue (v, L"Priority"));
    EXPECT_EQ (1234, v.GetInteger());

    EXPECT_EQ (ECOBJECTS_STATUS_Success, meta.SetAlwaysExpand (*arrayProp, true));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, arrayProp->GetCustomAttribute (L"AlwaysExpand")->GetValue (v, L"ArrayMembers"));
    EXPECT_TRUE (v.GetBoolean());

    EXPECT_EQ (ECOBJECTS_STATUS_Success, meta.SetExtendedType (*primProp, 1));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, primProp->GetCustomAttribute (L"ExtendType")->GetValue (v, L"Standard"));
    EXPECT_EQ (1, v.GetInteger());

    WString str (L"CustomType");
    EXPECT_EQ (ECOBJECTS_STATUS_Success, meta.SetExtendedType (*primProp, str.c_str()));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, primProp->GetCustomAttribute (L"ExtendType")->GetValue (v, L"Name"));
    EXPECT_TRUE (str.Equals (v.GetString()));

    EXPECT_EQ (ECOBJECTS_STATUS_Success, meta.SetMemberExtendedType (*arrayProp, 1));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, arrayProp->GetCustomAttribute (L"MemberExtendedType")->GetValue (v, L"Standard"));
    EXPECT_EQ (1, v.GetInteger());

    EXPECT_EQ (ECOBJECTS_STATUS_Success, meta.SetMemberExtendedType (*arrayProp, str.c_str()));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, arrayProp->GetCustomAttribute (L"MemberExtendedType")->GetValue (v, L"Name"));
    EXPECT_TRUE (str.Equals (v.GetString()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (CustomAttributeTest, SerializeSchemaToXmlUtfString)
    {
#ifdef _WIN32
    WCharCP const testClassName = L"TestClass";

    WCharCP const caClassName = L"CustomAttribClass";
    WCharCP const caWCharStringPropName = L"WCharStringProperty";
    WCharCP const caUtf8StringPropName = L"Utf8StringProperty";
    ECSchemaPtr expectedSchema = CreateCustomAttributeTestSchema ();
    ECClassP caClass = expectedSchema->GetClassP (caClassName);
    EXPECT_TRUE (caClass != NULL) << L"Test custom attribute class " << caClassName << L" not found";
    PrimitiveECPropertyP stringProp = NULL;
    caClass->CreatePrimitiveProperty (stringProp, caWCharStringPropName, PRIMITIVETYPE_String);
    caClass->CreatePrimitiveProperty (stringProp, caUtf8StringPropName, PRIMITIVETYPE_String);

        // *** WIP_PORTABILITY: Use an escape such as \u here. Don't try to use extended ascii directly
    WCharCP caPropValueString = L"äöüßá³µ";
    Utf8String expectedCAPropValueUtf8String;
    EXPECT_EQ (SUCCESS, BeStringUtilities::WCharToUtf8 (expectedCAPropValueUtf8String, caPropValueString));

    IECInstancePtr caInstance = GetInstanceForClass (caClassName, *expectedSchema);
    ECValue expectedWCharValue (caPropValueString);
    EXPECT_EQ (ECOBJECTS_STATUS_Success, caInstance->SetValue (caWCharStringPropName, expectedWCharValue)) << L"Assigning value to " << caWCharStringPropName << L" in custom attribute instance failed.";

    ECValue expectedUtf8Value (expectedCAPropValueUtf8String.c_str ());
    EXPECT_EQ (ECOBJECTS_STATUS_Success, caInstance->SetValue (caUtf8StringPropName, expectedUtf8Value)) << L"Assigning value to " << caUtf8StringPropName << L" in custom attribute instance failed.";

    ECClassP testClass = expectedSchema->GetClassP (testClassName);
    EXPECT_TRUE (testClass != NULL) << L"Test class " << testClassName << L" not found";
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
    EXPECT_TRUE (actualCAClass->GetPropertyP (caWCharStringPropName) != NULL) << L"Property " << caWCharStringPropName << L" not found in custom attribute class.";
    //now check CA instance added in this test
    ECClassCP actualTestClass = actualSchema->GetClassCP (testClassName);

    IECInstancePtr actualCAInstance = actualTestClass->GetCustomAttribute (*actualCAClass);
    ASSERT_TRUE (actualCAInstance.IsValid ()) << L"Test class " << testClassName << L" doesn't have the expected custom attribute instance.";

    //verify UTF-8 property value
    ECValue actualUtf8Value;

    //verify wchar property value
    ECValue actualWCharValue;
    ASSERT_EQ (ECOBJECTS_STATUS_Success, actualCAInstance->GetValue (actualWCharValue, caWCharStringPropName)) << L"Property " << caWCharStringPropName << L"not found in custom attribute instance on test class.";
    EXPECT_TRUE (expectedWCharValue.Equals (actualWCharValue)) << L"Unexpected ECValue of property " << caWCharStringPropName << L" of custom attribute instance";
    EXPECT_STREQ (caPropValueString, actualWCharValue.GetString ()) << L"Unexpected string value of property " << caWCharStringPropName << L" of custom attribute instance";
#endif
    }

END_BENTLEY_ECN_TEST_NAMESPACE
