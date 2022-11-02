/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct  Struct1
    {
    bool    struct1BoolMember;
    int     struct1IntMember;
    };

struct  Struct2
    {
    bool            struct2StringMemberNull;
    Utf8CP          struct2StringMember;
    bool            struct2DoubleMemberNull;
    double          struct2DoubleMember;
    Struct1*        nestedArray;
    uint32_t        arraySize;
    };

struct InSchemaClassLocater final : ECN::IECClassLocater
    {
    private:
        ECSchemaCR m_schema;

        ECClassCP _LocateClass(Utf8CP schemaName, Utf8CP className) override { return m_schema.GetClassCP(className); }
    public:
        explicit InSchemaClassLocater(ECSchemaCR schema) : m_schema(schema) {}
    };

struct InstanceTests;
struct CompressInstanceTests;
struct PropertyTests;
struct StringEncodingTests;
struct PropertyIndexTests : ECTestFixture{};

Utf8Char s_schemaXml[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.01\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                "    <ECSchemaReference name=\"Bentley_Standard_CustomAttributes\" version=\"01.06\" prefix=\"besc\" />"
                "    <ECClass typeName=\"Manufacturer\" isStruct=\"True\" isDomainClass=\"True\">"
                "        <ECProperty propertyName=\"Name\" typeName=\"string\" />"
                "    </ECClass>"
                "    <ECClass typeName=\"TestClass\" isDomainClass=\"True\">"
                "        <ECArrayProperty propertyName=\"StringArray\" typeName=\"string\" />"
                "        <ECProperty propertyName=\"String\" typeName=\"string\" />"
                "        <ECStructProperty propertyName=\"Struct\" typeName=\"Manufacturer\" />"
                "        <ECArrayProperty propertyName=\"StructArray\" typeName=\"Manufacturer\" isStruct=\"True\" />"
                "    </ECClass>"
                "    <ECClass typeName=\"TestUtf8Class\" isDomainClass=\"True\">"
                "        <ECCustomAttributes>"
                "            <PersistStringsAsUtf8 xmlns=\"Bentley_Standard_CustomAttributes.01.00\" />"
                "        </ECCustomAttributes>"
                "        <ECProperty propertyName=\"String\" typeName=\"string\" />"
                "    </ECClass>"
                "</ECSchema>";


/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceTests : ECTestFixture
    {
    ECSchemaPtr          m_schema;
    ECEntityClassP             m_ecClass;
    IECInstancePtr       m_instance;
    uint32_t             propIndex;

    void CreateSchema (Utf8String schemaName = "TestSchema", Utf8String className = "TestClass", Utf8String alias = "TestAlias")
        {
        ECSchema::CreateSchema (m_schema, schemaName, alias, 1, 0, 0);
        m_schema->CreateEntityClass (m_ecClass, className);
        }

    void CreateInstance ()
        {
        m_instance = m_ecClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
        }

    template <typename T>
    ECEnumerationP CreateEnumeration(Utf8CP name, PrimitiveType primitiveType, bool isStrict , std::initializer_list<T> enumerators)
        {
        ECEnumerationP enumeration;
        m_schema->CreateEnumeration(enumeration, name, primitiveType);
        enumeration->SetIsStrict(isStrict);

        ECEnumeratorP enumerator;
        for (auto const& value : enumerators)
            enumeration->CreateEnumerator(enumerator, value);

        return enumeration;
        }

    PrimitiveECPropertyP CreateProperty (Utf8CP name, PrimitiveType primitiveType)
        {
        PrimitiveECPropertyP prop;
        m_ecClass->CreatePrimitiveProperty (prop, name, primitiveType);
        m_ecClass->GetDefaultStandaloneEnabler ()->GetPropertyIndex (propIndex, name);
        return prop;
        }

    PrimitiveECPropertyP CreateProperty (Utf8CP name)
        {
        PrimitiveECPropertyP prop;
        m_ecClass->CreatePrimitiveProperty (prop, name, PRIMITIVETYPE_String);
        m_ecClass->GetDefaultStandaloneEnabler ()->GetPropertyIndex (propIndex, name);
        return prop;
        }

    PrimitiveECPropertyP CreateEnumerationProperty (Utf8CP name, ECEnumerationP enumerationType)
        {
        PrimitiveECPropertyP prop;
        m_ecClass->CreateEnumerationProperty(prop, name, *enumerationType);
        return prop;
        }

    PrimitiveArrayECPropertyP CreateArrayProperty (Utf8CP name)
        {
        PrimitiveArrayECPropertyP prop;
        m_ecClass->CreatePrimitiveArrayProperty (prop, name);
        m_ecClass->GetDefaultStandaloneEnabler ()->GetPropertyIndex (propIndex, name);
        return prop;
        }
    };

struct PropertyTests : InstanceTests {};

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct StringEncodingTests : ECTestFixture
    {
    ECSchemaPtr         m_schema;

    void SetUp() override
        {
        ECTestFixture::SetUp();
        ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();
        ECSchemaPtr schema;
        EXPECT_EQ (SchemaReadStatus::Success, ECSchema::ReadFromXmlString (m_schema, s_schemaXml, *schemaContext));  
        }

    void TearDown() override
        {
        // Resetting string encoding to the anticipated default
        // The static variable does not get cleaned after the test fixture is finished so need to manually reset it.
        ECDBuffer::SetDefaultStringEncoding(ECDBuffer::StringEncoding::StringEncoding_Utf8);
        }

    struct          Utf16String
        {
    private:
        Utf16Buffer         m_utf16;
    public:
        Utf16String (WCharCP wc)
            {
            BeStringUtilities::WCharToUtf16 (m_utf16, wc);
            }

        Utf16CP c_str() const   { return &m_utf16[0]; }
        };

    void            Compare (ECValueCR v1, ECValueCR v2, bool expectMatch)
        {
        EXPECT_EQ (expectMatch, v1.Equals (v2)) << v1.ToString().c_str() << "\n" << v2.ToString().c_str();
        EXPECT_EQ (expectMatch, v2.Equals (v1)) << v1.ToString().c_str() << "\n" << v2.ToString().c_str();
        }

    template<bool expectMatch> void Compare (WCharCP wc, Utf8CP u8, Utf16CP u16)
        {
        ECValue vw (wc), v8 (u8), v16 (u16);
        Compare (vw, v8, expectMatch);
        Compare (vw, v16, expectMatch);
        Compare (v8, v16, expectMatch);
        }

    void                            Convert (ECValueCR v, WCharCP str)
        {
        WCharCP wc = v.GetWCharCP();
        EXPECT_EQ (0, wcscmp (wc, str));
        Utf8CP u8 = v.GetUtf8CP();
        EXPECT_TRUE (0 == Utf8String (str).compare (u8));
        Utf16CP u16 = v.GetUtf16CP();
        EXPECT_EQ (0, BeStringUtilities::CompareUtf16WChar (u16, str));
        }

    StandaloneECInstancePtr CreateInstance (Utf8CP classname, ECDBuffer::StringEncoding encoding)
        {
        // Note setting the global default string encoding isn't a typical workflow.
        // We do it here so we can test instances with different encodings.
        ECDBuffer::SetDefaultStringEncoding (encoding);
        ECClassP ecClass = m_schema->GetClassP (classname);
        return ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
        }
    
    StandaloneECInstancePtr CreatePrimaryInstance (ECDBuffer::StringEncoding encoding, Utf8CP strVal)
        {
        StandaloneECInstancePtr instance = CreateInstance ("TestClass", encoding);
        ECValue v (strVal);
        EXPECT_EQ (ECObjectsStatus::Success, instance->SetValue ("String", v));
        EXPECT_EQ (ECObjectsStatus::Success, instance->SetValue ("Struct.Name", v));
        EXPECT_EQ (ECObjectsStatus::Success, instance->AddArrayElements ("StringArray", 5));
        for (uint32_t i = 0; i < 3; i++)
            EXPECT_EQ (ECObjectsStatus::Success, instance->SetValue ("StringArray", v, i));

        EXPECT_EQ (encoding, instance->GetStringEncoding());
        return instance;
        }

    StandaloneECInstancePtr CreateStructInstance (ECDBuffer::StringEncoding encoding, Utf8CP name, IECInstanceR parent)
        {
        ECValue v;
        EXPECT_EQ (ECObjectsStatus::Success, parent.GetValue (v, "StructArray"));
        EXPECT_EQ (ECObjectsStatus::Success, parent.AddArrayElements ("StructArray", 1));

        StandaloneECInstancePtr instance = CreateInstance ("Manufacturer", encoding);
        EXPECT_EQ (ECObjectsStatus::Success, instance->SetValue ("Name", ECValue (name)));

        ECValue structV;
        structV.SetStruct (instance.get());
        EXPECT_EQ (ECObjectsStatus::Success, parent.SetValue ("StructArray", structV, v.GetArrayInfo().GetCount()));
        EXPECT_EQ (encoding, instance->GetStringEncoding());
        return instance;
        }

    bool                    CompareInstances (ECValuesCollectionCR aVals, StandaloneECInstancePtr b, bool outputDifferences = false)
        {
        for (ECPropertyValueCR aVal: aVals)
            {
            ECValueCR aV = aVal.GetValue();
            ECValue bV;
            EXPECT_EQ (ECObjectsStatus::Success, b->GetValueUsingAccessor (bV, aVal.GetValueAccessor()));
            if (aVal.HasChildValues())
                {
                if (!CompareInstances (*aVal.GetChildValues(), b))
                    return false;
                }
            else if (!bV.Equals (aV))
                {
                if (outputDifferences)
                    printf ("%s differs: %s vs. %s\n", aVal.GetValueAccessor().GetAccessString(), aV.ToString().c_str(), bV.ToString().c_str());

                return false;
                }
            }
            
        return true;
        }

    template <bool expectMatch>
    void                    CompareInstances (StandaloneECInstancePtr a, StandaloneECInstancePtr b)
        {
        ECValuesCollectionPtr aVals = ECValuesCollection::Create (*a);
        EXPECT_EQ (expectMatch, CompareInstances (*aVals, b, expectMatch));
        }
    };

struct CompressInstanceTests : ECTestFixture
    {
    ECSchemaPtr m_schema;
    Utf8CP kitchenSinkSchemaXML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"KitchenSink\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "    <ECClass typeName=\"Manufacturer\" isStruct=\"True\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"Name\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"AccountNo\" typeName=\"int\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"Complicated\" isStruct=\"True\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"ExtName\" typeName=\"string\" />"
        "        <ECStructProperty propertyName=\"ExtStruct\" typeName=\"Manufacturer\" />"
        "        <ECArrayProperty propertyName=\"ExtStructs\" typeName=\"Manufacturer\" />"
        "    </ECClass>"

        "    <ECClass typeName=\"FixedSizeArrayTester\" isStruct=\"True\" isDomainClass=\"True\">"
        "        <ECArrayProperty propertyName=\"FixedString5\"     typeName=\"string\"            minOccurs=\"5\"  maxOccurs=\"5\" />"
        "        <ECArrayProperty propertyName=\"FixedInt5\"        typeName=\"int\"               minOccurs=\"5\"  maxOccurs=\"5\" />"
        "        <ECArrayProperty propertyName=\"Manufacturer5\"    typeName=\"Manufacturer\"      minOccurs=\"5\"  maxOccurs=\"5\" />"
        "    </ECClass>"

        "    <ECClass typeName=\"KitchenSink\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"myString\" typeName=\"string\" />"
        "        <ECArrayProperty propertyName=\"myStringArray\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"myInt\" typeName=\"int\" />"
        "        <ECArrayProperty propertyName=\"myIntArray\" typeName=\"int\" />"
        "        <ECProperty propertyName=\"my3dPoint\"     typeName=\"point3d\" />"
        "        <ECArrayProperty propertyName=\"my3dPointArray\" typeName=\"point3d\"/>"
        "        <ECProperty propertyName=\"myDate\" typeName=\"dateTime\"  />"
        "        <ECArrayProperty propertyName=\"myDateArray\" typeName=\"dateTime\" />"
        "        <ECProperty propertyName=\"myLong\" typeName=\"long\" />"
        "        <ECArrayProperty propertyName=\"myLongArray\" typeName=\"long\" />"
        "        <ECProperty propertyName=\"myDouble\" typeName=\"double\" />"
        "        <ECArrayProperty propertyName=\"myDoubleArray\" typeName=\"double\" />"
        "        <ECProperty propertyName=\"myBool\" typeName=\"boolean\"  />"
        "        <ECArrayProperty propertyName=\"myBoolArray\" typeName=\"boolean\" />"
        "        <ECProperty propertyName=\"my2dPoint\" typeName=\"point2d\" />"
        "        <ECArrayProperty propertyName=\"my2dPointArray\" typeName=\"point2d\" />"
        "        <ECStructProperty propertyName=\"myManufacturerStruct\" typeName=\"Manufacturer\" />"
        "        <ECArrayProperty propertyName=\"myManufacturerStructArray\" typeName=\"Manufacturer\"/>"
        "        <ECStructProperty propertyName=\"myComplicated\" typeName=\"Complicated\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"PointArrayTest\" isStruct=\"True\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"myString\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"myInt\" typeName=\"int\" />"
        "        <ECArrayProperty propertyName=\"my3dPointArray\" typeName=\"point3d\"/>"
        "    </ECClass>"
        "</ECSchema>";

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ReadKitchenSinkSchemaFromXml ()
        {
        ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext (false);
        EXPECT_EQ (SchemaReadStatus::Success, ECSchema::ReadFromXmlString (m_schema, kitchenSinkSchemaXML, *schemaContext));
        EXPECT_TRUE (m_schema.IsValid ());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyArrayInfo (IECInstanceR instance, ECValueR v, Utf8CP accessString, uint32_t count, bool isFixedCount)
        {
        v.Clear ();
        EXPECT_TRUE (ECObjectsStatus::Success == instance.GetValue (v, accessString));
        EXPECT_EQ (count, v.GetArrayInfo ().GetCount ());
        EXPECT_EQ (isFixedCount, v.GetArrayInfo ().IsFixedCount ());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyIsNullArrayElements (IECInstanceR instance, ECValueR v, Utf8CP accessString, uint32_t start, uint32_t count, bool isNull)
        {
        for (uint32_t i = start; i < start + count; i++)
            {
            v.Clear ();
            EXPECT_TRUE (ECObjectsStatus::Success == instance.GetValue (v, accessString, i));
            EXPECT_TRUE (isNull == v.IsNull ());
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyString (IECInstanceR instance, ECValueR v, Utf8CP accessString, bool useIndex, uint32_t index, Utf8CP value)
        {
        v.Clear ();
        if (useIndex)
            EXPECT_TRUE (ECObjectsStatus::Success == instance.GetValue (v, accessString, index));
        else
            EXPECT_TRUE (ECObjectsStatus::Success == instance.GetValue (v, accessString));
        EXPECT_STREQ (value, v.GetUtf8CP ());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyString (IECInstanceR instance, ECValueR v, Utf8CP accessString, Utf8CP value)
        {
        return VerifyString (instance, v, accessString, false, 0, value);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetAndVerifyString (IECInstanceR instance, ECValueR v, Utf8CP accessString, Utf8CP value)
        {
        v.SetUtf8CP (value);
        EXPECT_TRUE (ECObjectsStatus::Success == instance.SetValue (accessString, v));
        VerifyString (instance, v, accessString, value);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyInteger (IECInstanceR instance, ECValueR v, Utf8CP accessString, bool useIndex, uint32_t index, uint32_t value)
        {
        v.Clear ();
        if (useIndex)
            EXPECT_TRUE (ECObjectsStatus::Success == instance.GetValue (v, accessString, index));
        else
            EXPECT_TRUE (ECObjectsStatus::Success == instance.GetValue (v, accessString));
        EXPECT_EQ (value, v.GetInteger ());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyInteger (IECInstanceR instance, ECValueR v, Utf8CP accessString, uint32_t value)
        {
        return VerifyInteger (instance, v, accessString, false, 0, value);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetAndVerifyInteger (IECInstanceR instance, ECValueR v, Utf8CP accessString, uint32_t value)
        {
        v.SetInteger (value);
        EXPECT_TRUE (ECObjectsStatus::Success == instance.SetValue (accessString, v));
        VerifyInteger (instance, v, accessString, value);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void validateArrayCount (ECN::StandaloneECInstanceCR instance, Utf8CP propertyName, uint32_t expectedCount)
        {
        ECValue varray;
        EXPECT_TRUE (ECObjectsStatus::Success == instance.GetValue (varray, propertyName));
        uint32_t count = varray.GetArrayInfo ().GetCount ();
        EXPECT_TRUE (count == expectedCount);

        ECValue ventry;
        for (uint32_t i = 0; i < count; i++)
            {
            EXPECT_TRUE (ECObjectsStatus::Success == instance.GetValue (ventry, propertyName, i));
            }
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (InstanceTests, TestIsNullPrimitive)
    {
    CreateSchema ();
    CreateProperty ("PropertyString", PRIMITIVETYPE_String);
    CreateInstance ();

    bool isNull = NULL;

    EXPECT_EQ (m_instance->IsPropertyNull (isNull, "PropertyString"), ECObjectsStatus::Success);
    EXPECT_TRUE (isNull);
    EXPECT_EQ (m_instance->IsPropertyNull (isNull, propIndex), ECObjectsStatus::Success);
    EXPECT_TRUE (isNull);

    EXPECT_EQ (m_instance->SetValue ("PropertyString", ECValue ("Some value")), ECObjectsStatus::Success);

    EXPECT_EQ (m_instance->IsPropertyNull (isNull, "PropertyString"), ECObjectsStatus::Success);
    EXPECT_FALSE (isNull);
    EXPECT_EQ (m_instance->IsPropertyNull (isNull, propIndex), ECObjectsStatus::Success);
    EXPECT_FALSE (isNull);
    }

/*---------------------------------------------------------------------------------**//**
* The array property value itself is *never* null.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (InstanceTests, TestIsNullArray)
    {
    CreateSchema ();
    CreateArrayProperty ("PropertyArray");
    CreateInstance ();

    bool isNull = false;

    EXPECT_EQ (m_instance->IsPropertyNull (isNull, "PropertyArray"), ECObjectsStatus::Success);
    EXPECT_FALSE (isNull);
    EXPECT_EQ (m_instance->IsPropertyNull (isNull, propIndex), ECObjectsStatus::Success);
    EXPECT_FALSE (isNull);

    EXPECT_EQ (m_instance->AddArrayElements ("PropertyArray", 15), ECObjectsStatus::Success);
    EXPECT_EQ (m_instance->SetValue ("PropertyArray", ECValue ("Some value"), 3), ECObjectsStatus::Success);

    //Strangelly array property doesn't exist as PrimitiveProperty
    EXPECT_EQ (m_instance->IsPropertyNull (isNull, "PropertyArray"), ECObjectsStatus::Success);
    EXPECT_FALSE (isNull);
    EXPECT_EQ (m_instance->IsPropertyNull (isNull, propIndex), ECObjectsStatus::Success);
    EXPECT_FALSE (isNull);
    //-------------------------------------------------------------

    EXPECT_EQ (m_instance->IsPropertyNull (isNull, "PropertyArray", 3), ECObjectsStatus::Success);
    EXPECT_FALSE (isNull);
    EXPECT_EQ (m_instance->IsPropertyNull (isNull, propIndex, 3), ECObjectsStatus::Success);
    EXPECT_FALSE (isNull);

    EXPECT_EQ (m_instance->IsPropertyNull (isNull, "PropertyArray", 13), ECObjectsStatus::Success);
    EXPECT_TRUE (isNull);
    EXPECT_EQ (m_instance->IsPropertyNull (isNull, propIndex, 13), ECObjectsStatus::Success);
    EXPECT_TRUE (isNull);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(InstanceTests, TestGetNullPropertyValues)
    {
    CreateSchema();
    CreateProperty("GeomProp", PrimitiveType::PRIMITIVETYPE_IGeometry);
    CreateProperty("BinaryProp", PrimitiveType::PRIMITIVETYPE_Binary);
    CreateProperty("BoolProp", PrimitiveType::PRIMITIVETYPE_Boolean);
    CreateProperty("DateProp", PrimitiveType::PRIMITIVETYPE_DateTime);
    CreateProperty("DoubleProp", PrimitiveType::PRIMITIVETYPE_Double);
    CreateProperty("IntProp", PrimitiveType::PRIMITIVETYPE_Integer);
    CreateProperty("LongProp", PrimitiveType::PRIMITIVETYPE_Long);
    CreateProperty("Point2dProp", PrimitiveType::PRIMITIVETYPE_Point2d);
    CreateProperty("Point3dProp", PrimitiveType::PRIMITIVETYPE_Point3d);
    CreateProperty("StringProp", PrimitiveType::PRIMITIVETYPE_String);

    CreateInstance();

    ECValue testValue;

    // IGeometry
    m_instance->GetValue(testValue, "GeomProp");
    EXPECT_TRUE(testValue.IsNull());
    IGeometryPtr storedGeometryPtr = testValue.GetIGeometry();
    EXPECT_TRUE(storedGeometryPtr.IsNull());
    EXPECT_FALSE(storedGeometryPtr.IsValid());

    size_t size = 0;
    const Byte * igeom = testValue.GetIGeometry(size);
    EXPECT_EQ(nullptr, igeom);
    EXPECT_EQ(0, size);

    size = 0;
    const Byte* igeomBlob = testValue.GetBinary(size);
    EXPECT_EQ(nullptr, igeomBlob);
    EXPECT_EQ(0, size);

    testValue.Clear();

    // Binary
    m_instance->GetValue(testValue, "BinaryProp");
    EXPECT_TRUE(testValue.IsNull());
    size = 0;
    const Byte* binaryBlob = testValue.GetBinary(size);
    EXPECT_EQ(nullptr, binaryBlob);
    EXPECT_EQ(0, size);

    testValue.Clear();

    // Bool

    m_instance->GetValue(testValue, "BoolProp");
    EXPECT_TRUE(testValue.IsNull());
    { // Precondition within the GetBoolean to assert if null.
    DISABLE_ASSERTS
    EXPECT_FALSE(testValue.GetBoolean());
    }
    
    testValue.Clear();

    // Date
    m_instance->GetValue(testValue, "DateProp");
    EXPECT_TRUE(testValue.IsNull());
    { // Precondition within the GetBoolean to assert if null.
    DISABLE_ASSERTS
    DateTime time = testValue.GetDateTime();
    EXPECT_FALSE(time.IsValid());

    int64_t test = testValue.GetDateTimeTicks();
    EXPECT_EQ(0, test);

    int64_t test2 = testValue.GetDateTimeUnixMillis();
    EXPECT_EQ(0, test2);
    }

    testValue.Clear();

    // Double
    m_instance->GetValue(testValue, "DoubleProp");
    { // Precondition within the GetBoolean to assert if null.
    DISABLE_ASSERTS
    EXPECT_TRUE(std::isnan(testValue.GetDouble()));
    }

    testValue.Clear();

    // Integer
    m_instance->GetValue(testValue, "IntProp");
    { // Precondition within the GetBoolean to assert if null.
    DISABLE_ASSERTS
    EXPECT_EQ(0, testValue.GetInteger());
    }

    testValue.Clear();

    // Long
    m_instance->GetValue(testValue, "LongProp");
    { // Precondition within the GetBoolean to assert if null.
    DISABLE_ASSERTS
    EXPECT_EQ(0, testValue.GetLong());
    }

    testValue.Clear();

    // Point2dProp
    m_instance->GetValue(testValue, "Point2dProp");
    { // Precondition within the GetBoolean to assert if null.
    DISABLE_ASSERTS
    DPoint2d pt2 = testValue.GetPoint2d();
    EXPECT_EQ(DPoint2d::FromZero(), pt2);
    }

    testValue.Clear();

    // Point3dProp
    m_instance->GetValue(testValue, "Point3dProp");
    { // Precondition within the GetBoolean to assert if null.
    DISABLE_ASSERTS
    DPoint3d pt3 = testValue.GetPoint3d();
    EXPECT_EQ(DPoint3d::FromZero(), pt3);
    }

    testValue.Clear();

    // String
    m_instance->GetValue(testValue, "StringProp");
    EXPECT_TRUE(testValue.IsNull());
    EXPECT_EQ(nullptr, testValue.GetUtf16CP());
    EXPECT_EQ(nullptr, testValue.GetUtf8CP());
    EXPECT_EQ(nullptr, testValue.GetWCharCP());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (InstanceTests, TestArraySize)
    {
    CreateSchema ();
    CreateArrayProperty ("PropertyArray");
    CreateInstance ();

    ECValue arrayVal;

    EXPECT_EQ (m_instance->GetValue (arrayVal, "PropertyArray"), ECObjectsStatus::Success);
    EXPECT_TRUE (arrayVal.IsArray ());
    EXPECT_EQ (arrayVal.GetArrayInfo ().GetCount (), 0);

    EXPECT_EQ (m_instance->AddArrayElements ("PropertyArray", 13), ECObjectsStatus::Success);

    EXPECT_EQ (m_instance->GetValue (arrayVal, "PropertyArray"), ECObjectsStatus::Success);
    EXPECT_TRUE (arrayVal.IsArray ());
    EXPECT_EQ (arrayVal.GetArrayInfo ().GetCount (), 13);

    EXPECT_EQ (m_instance->ClearArray ("PropertyArray"), ECObjectsStatus::Success);

    EXPECT_EQ (m_instance->GetValue (arrayVal, "PropertyArray"), ECObjectsStatus::Success);
    EXPECT_TRUE (arrayVal.IsArray ());
    EXPECT_EQ (arrayVal.GetArrayInfo ().GetCount (), 0);

    EXPECT_EQ (m_instance->AddArrayElements (propIndex, 42), ECObjectsStatus::Success);

    EXPECT_EQ (m_instance->GetValue (arrayVal, propIndex), ECObjectsStatus::Success);
    EXPECT_TRUE (arrayVal.IsArray ());
    EXPECT_EQ (arrayVal.GetArrayInfo ().GetCount (), 42);

    EXPECT_EQ (m_instance->ClearArray (propIndex), ECObjectsStatus::Success);

    EXPECT_EQ (m_instance->GetValue (arrayVal, propIndex), ECObjectsStatus::Success);
    EXPECT_TRUE (arrayVal.IsArray ());
    EXPECT_EQ (arrayVal.GetArrayInfo ().GetCount (), 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (InstanceTests, TestSetValueString)
    {
    CreateSchema ();
    CreateProperty ("Property_1");
    CreateInstance ();

    ECValue value;

    EXPECT_EQ (m_instance->SetValue ("Property_1", ECValue ("Some value 1")), ECObjectsStatus::Success);
    EXPECT_EQ (m_instance->GetValue (value, "Property_1"), ECObjectsStatus::Success);
    EXPECT_STREQ (value.GetUtf8CP (), "Some value 1");

    EXPECT_EQ (m_instance->SetValue (propIndex, ECValue ("Some value 2")), ECObjectsStatus::Success);
    EXPECT_EQ (m_instance->GetValue (value, propIndex), ECObjectsStatus::Success);
    EXPECT_STREQ (value.GetUtf8CP (), "Some value 2");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (InstanceTests, TestSetDisplayLabel)
    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext ();
    SchemaKey schemaKey ("Bentley_Standard_CustomAttributes", 1, 5);
    ECSchemaPtr customAttributesSchema = context->LocateSchema (schemaKey, SchemaMatchType::Latest);

    StandaloneECEnablerPtr m_customAttributeEnabler = customAttributesSchema->GetClassP ("InstanceLabelSpecification")->GetDefaultStandaloneEnabler ();
    CreateSchema ();
    m_schema->AddReferencedSchema (*customAttributesSchema);

    PrimitiveECPropertyP prop;
    m_ecClass->CreatePrimitiveProperty (prop, "InstanceLabel", PRIMITIVETYPE_String);

    IECInstancePtr labelAttr = m_customAttributeEnabler->CreateInstance ();
    labelAttr->SetValue ("PropertyName", ECValue ("InstanceLabel"));
    m_ecClass->SetCustomAttribute (*labelAttr);

    IECInstancePtr m_instance = m_ecClass->GetDefaultStandaloneEnabler ()->CreateInstance ();

    Utf8String displayLabel;
    EXPECT_EQ (m_instance->GetDisplayLabel (displayLabel), ECObjectsStatus::Success);
    EXPECT_TRUE (displayLabel.Equals (m_ecClass->GetDisplayLabel ()));

    EXPECT_EQ (m_instance->SetDisplayLabel ("Some fancy instance label"), ECObjectsStatus::Success);

    EXPECT_EQ (m_instance->GetDisplayLabel (displayLabel), ECObjectsStatus::Success);
    EXPECT_STREQ (displayLabel.c_str (), "Some fancy instance label");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceTests, GetInstanceAttributesUsingInstanceInterface)
    {
    CreateSchema();
    CreateProperty("StringProperty", PRIMITIVETYPE_String);
    CreateInstance();

    ASSERT_EQ(ECObjectsStatus::Success, m_instance->SetValue("StringProperty", ECValue("Some value")));

    ECN::ECInstanceInterface instanceInterface(*m_instance);

    //get value
    ECValue stringVal;
    ASSERT_EQ(ECObjectsStatus::Success, instanceInterface.GetInstanceValue(stringVal, "StringProperty"));
    ASSERT_STREQ("Some value", stringVal.GetUtf8CP());

    //get instance class name
    ECClassCP instanceClass = instanceInterface.GetInstanceClass();
    ASSERT_STREQ("TestSchema:TestClass", instanceClass->GetFullName());

    //get instanceId
    Utf8String instanceId = instanceInterface.GetInstanceId();
    ASSERT_STREQ(instanceId.c_str(), m_instance->GetInstanceId().c_str());

    //Get IECInstance
    IECInstanceCP instance = instanceInterface.ObtainECInstance();
    ASSERT_TRUE(instance != nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceTests, GetInstanceOffSet)
    {
    CreateSchema();
    CreateProperty("StringProperty", PRIMITIVETYPE_String);
    CreateInstance();

    ASSERT_EQ(ECObjectsStatus::Success, m_instance->SetValue("StringProperty", ECValue("Some value")));

    size_t offSet = m_instance->GetOffsetToIECInstance();
    ASSERT_TRUE(offSet == 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (PropertyTests, SetReadOnlyAndSetValue)
    {
    CreateSchema ();
    CreateProperty ("PropertyString", PRIMITIVETYPE_String);
    m_ecClass->GetPropertyP ("PropertyString")->SetIsReadOnly (true);
    CreateInstance ();

    EXPECT_TRUE (m_ecClass->GetPropertyP ("PropertyString")->GetIsReadOnly ());

    //since the instance has no value initially, it can be set.This was done so that instances could be deserialized even if they had readonly property.This is the same as in the managed API.
    EXPECT_EQ (ECObjectsStatus::Success, m_instance->SetValue ("PropertyString", ECValue ("Some value")));
    ECValue getValue;
    EXPECT_EQ (m_instance->GetValue (getValue, "PropertyString"), ECObjectsStatus::Success);
    EXPECT_STREQ (getValue.GetUtf8CP (), "Some value");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (PropertyTests, SetReadOnlyAndChangeValue)
    {
    CreateSchema ();
    CreateProperty ("PropertyString", PRIMITIVETYPE_String);
    m_ecClass->GetPropertyP ("PropertyString")->SetIsReadOnly (true);
    CreateInstance ();

    EXPECT_TRUE (m_ecClass->GetPropertyP ("PropertyString")->GetIsReadOnly ());

    //since the instance has no value initially, it can be set.This was done so that instances could be deserialized even if they had readonly property.This is the same as in the managed API.
    EXPECT_EQ (ECObjectsStatus::Success, m_instance->ChangeValue ("PropertyString", ECValue ("Other value")));
    ECValue getValue;
    EXPECT_EQ (m_instance->GetValue (getValue, "PropertyString"), ECObjectsStatus::Success);
    EXPECT_STREQ (getValue.GetUtf8CP (), "Other value");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (PropertyTests, SetAndChangePropertyValue)
    {
    CreateSchema ();
    CreateProperty ("PropertyString", PRIMITIVETYPE_String);
    CreateInstance ();

    EXPECT_EQ (ECObjectsStatus::Success, m_instance->SetValue ("PropertyString", ECValue ("init value")));
    ECValue getValue;
    EXPECT_EQ (m_instance->GetValue (getValue, "PropertyString"), ECObjectsStatus::Success);
    EXPECT_STREQ (getValue.GetUtf8CP (), "init value");

    EXPECT_EQ (ECObjectsStatus::Success, m_instance->ChangeValue ("PropertyString", ECValue ("Other value")));
    
    EXPECT_EQ (m_instance->GetValue (getValue, "PropertyString"), ECObjectsStatus::Success);
    EXPECT_STREQ (getValue.GetUtf8CP (), "Other value");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (PropertyTests, GetValueFromInstance)
    {
    CreateSchema ();
    CreateProperty ("Property_1");
    CreateInstance ();

    m_instance->SetValue ("Property_1", ECValue ("Some value"));
    ECPropertyValuePtr propValue = ECPropertyValue::GetPropertyValue (*m_instance, "Property_1");

    IECInstanceCR instance = propValue->GetInstance ();
    EXPECT_STREQ (m_instance->GetInstanceId ().c_str (), instance.GetInstanceId ().c_str ());
    ECValueCR value = propValue->GetValue ();
    EXPECT_STREQ (value.ToString ().c_str (), "Some value");
    ECValueAccessorCR accessor = propValue->GetValueAccessor ();
    EXPECT_STREQ (accessor.GetAccessString (), "Property_1");
    EXPECT_FALSE (propValue->HasChildValues ());
    }

TEST_F (PropertyTests, GetEnumeratorValueFromInstance)
    {
    // Arrange
    CreateSchema();
    
    auto const strictStringEnum = CreateEnumeration("StrictStringEnum", PRIMITIVETYPE_String, true, { "Strict1", "Strict2" });
    auto const looseStringEnum = CreateEnumeration("LooseStringEnum", PRIMITIVETYPE_String, false, { "Loose3", "Loose4" });
    
    auto const strictIntegerEnum = CreateEnumeration("StrictIntegerEnum", PRIMITIVETYPE_Integer, true, { 1, 2 });
    auto const looseIntegerEnum = CreateEnumeration("LooseIntegerEnum", PRIMITIVETYPE_Integer, false, { 3, 4 });

    CreateEnumerationProperty("StrictStringEnum_ValidValue", strictStringEnum);
    CreateEnumerationProperty("StrictStringEnum_InvalidValue", strictStringEnum);

    CreateEnumerationProperty("LooseStringEnum_ValidValue", looseStringEnum);
    CreateEnumerationProperty("LooseStringEnum_InvalidValue", looseStringEnum);

    CreateEnumerationProperty("StrictIntegerEnum_ValidValue", strictIntegerEnum);
    CreateEnumerationProperty("StrictIntegerEnum_InvalidValue", strictIntegerEnum);

    CreateEnumerationProperty("LooseIntegerEnum_ValidValue", looseIntegerEnum);
    CreateEnumerationProperty("LooseIntegerEnum_InvalidValue", looseIntegerEnum);

    // Act

    CreateInstance();

    m_instance->SetValue("StrictStringEnum_ValidValue", ECValue("Strict1"));
    m_instance->SetValue("StrictStringEnum_InvalidValue", ECValue("StrictInvalid"));

    m_instance->SetValue("LooseStringEnum_ValidValue", ECValue("Loose3"));
    m_instance->SetValue("LooseStringEnum_InvalidValue", ECValue("LooseInvalid"));

    m_instance->SetValue("StrictIntegerEnum_ValidValue", ECValue(2));
    m_instance->SetValue("StrictIntegerEnum_InvalidValue", ECValue(12));

    m_instance->SetValue("LooseIntegerEnum_ValidValue", ECValue(4));
    m_instance->SetValue("LooseIntegerEnum_InvalidValue", ECValue(34));

    // Assert

    ECValue actualValue;
    ASSERT_EQ(ECObjectsStatus::Success, m_instance->GetValue(actualValue, "StrictStringEnum_ValidValue"));
    EXPECT_STREQ("Strict1", actualValue.GetUtf8CP()) << actualValue.GetUtf8CP();

    ASSERT_EQ(ECObjectsStatus::Success, m_instance->GetValue(actualValue, "StrictStringEnum_InvalidValue"));
    ASSERT_TRUE(actualValue.IsNull());

    ASSERT_EQ(ECObjectsStatus::Success, m_instance->GetValue(actualValue, "LooseStringEnum_ValidValue"));
    EXPECT_STREQ("Loose3", actualValue.GetUtf8CP());

    ASSERT_EQ(ECObjectsStatus::Success, m_instance->GetValue(actualValue, "LooseStringEnum_InvalidValue"));
    EXPECT_STREQ("LooseInvalid", actualValue.GetUtf8CP());

    ASSERT_EQ(ECObjectsStatus::Success, m_instance->GetValue(actualValue, "StrictIntegerEnum_ValidValue"));
    ASSERT_EQ(2, actualValue.GetInteger());

    ASSERT_EQ(ECObjectsStatus::Success, m_instance->GetValue(actualValue, "StrictIntegerEnum_InvalidValue"));
    ASSERT_TRUE(actualValue.IsNull());

    ASSERT_EQ(ECObjectsStatus::Success, m_instance->GetValue(actualValue, "LooseIntegerEnum_ValidValue"));
    ASSERT_EQ(4, actualValue.GetInteger());

    ASSERT_EQ(ECObjectsStatus::Success, m_instance->GetValue(actualValue, "LooseIntegerEnum_InvalidValue"));
    ASSERT_EQ(34, actualValue.GetInteger());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (PropertyTests, SetEnumerationThroughECValue)
    {
    // Arrange
    CreateSchema();

    auto const strictStringEnum = CreateEnumeration("StrictStringEnum", PRIMITIVETYPE_String, true, { "Strict1", "Strict2" });
    auto const looseStringEnum = CreateEnumeration("LooseStringEnum", PRIMITIVETYPE_String, false, { "Loose3", "Loose4" });
    
    auto const strictIntegerEnum = CreateEnumeration("StrictIntegerEnum", PRIMITIVETYPE_Integer, true, { 1, 2 });
    auto const looseIntegerEnum = CreateEnumeration("LooseIntegerEnum", PRIMITIVETYPE_Integer, false, { 3, 4 });

    CreateEnumerationProperty("StrictStringEnum_ValidValue", strictStringEnum);
    CreateEnumerationProperty("StrictStringEnum_InvalidValue", strictStringEnum);

    CreateEnumerationProperty("LooseStringEnum_ValidValue", looseStringEnum);
    CreateEnumerationProperty("LooseStringEnum_InvalidValue", looseStringEnum);

    CreateEnumerationProperty("StrictIntegerEnum_ValidValue", strictIntegerEnum);
    CreateEnumerationProperty("StrictIntegerEnum_InvalidValue", strictIntegerEnum);

    CreateEnumerationProperty("LooseIntegerEnum_ValidValue", looseIntegerEnum);
    CreateEnumerationProperty("LooseIntegerEnum_InvalidValue", looseIntegerEnum);

    // set enum
    CreateInstance();

    ECValue strictStringValue;
    EXPECT_EQ(BentleyStatus::SUCCESS, strictStringValue.SetEnumeration(*strictStringEnum, "Strict1"));
    EXPECT_STREQ("Strict1", strictStringValue.GetUtf8CP());
    m_instance->SetValue("StrictStringEnum_ValidValue", strictStringValue);

    ECValue invalidStrictStringValue;
    EXPECT_EQ(BentleyStatus::ERROR, invalidStrictStringValue.SetEnumeration(*strictStringEnum, "StrictInvalid"));
    EXPECT_TRUE(invalidStrictStringValue.IsNull());
    m_instance->SetValue("strictStringEnum_InvalidValue", invalidStrictStringValue);

    ECValue looseStringValue;
    EXPECT_EQ(BentleyStatus::SUCCESS, looseStringValue.SetEnumeration(*looseStringEnum, "Loose3"));
    EXPECT_STREQ("Loose3", looseStringValue.GetUtf8CP());
    m_instance->SetValue("LooseStringEnum_ValidValue", looseStringValue);

    ECValue invalidLooseStringValue;
    EXPECT_EQ(BentleyStatus::SUCCESS, invalidLooseStringValue.SetEnumeration(*looseStringEnum, "LooseInvalid"));
    EXPECT_STREQ("LooseInvalid", invalidLooseStringValue.GetUtf8CP());
    m_instance->SetValue("LooseStringEnum_InvalidValue", invalidLooseStringValue);

    ECValue strictIntegerValue;
    EXPECT_EQ(BentleyStatus::SUCCESS, strictIntegerValue.SetEnumeration(*strictIntegerEnum, 2));
    EXPECT_EQ(2, strictIntegerValue.GetInteger());
    m_instance->SetValue("StrictIntegerEnum_ValidValue", strictIntegerValue);

    ECValue invalidStrictIntegerValue;
    EXPECT_EQ(BentleyStatus::ERROR, invalidStrictIntegerValue.SetEnumeration(*strictIntegerEnum, 12));
    EXPECT_TRUE(invalidStrictIntegerValue.IsNull());
    m_instance->SetValue("StrictIntegerEnum_InvalidValue", invalidStrictIntegerValue);

    ECValue looseIntegerValue;
    EXPECT_EQ(BentleyStatus::SUCCESS, looseIntegerValue.SetEnumeration(*looseIntegerEnum, 4));
    EXPECT_EQ(4, looseIntegerValue.GetInteger());
    m_instance->SetValue("LooseIntegerEnum_ValidValue", looseIntegerValue);

    ECValue invalidLooseIntegerValue;
    EXPECT_EQ(BentleyStatus::SUCCESS, invalidLooseIntegerValue.SetEnumeration(*looseIntegerEnum, 34));
    EXPECT_EQ(34, invalidLooseIntegerValue.GetInteger());
    m_instance->SetValue("LooseIntegerEnum_InvalidValue", invalidLooseIntegerValue);

    // assert property set
    ECValue actualValue;
    ASSERT_EQ(ECObjectsStatus::Success, m_instance->GetValue(actualValue, "StrictStringEnum_ValidValue"));
    EXPECT_STREQ("Strict1", actualValue.GetUtf8CP()) << actualValue.GetUtf8CP();

    ASSERT_EQ(ECObjectsStatus::Success, m_instance->GetValue(actualValue, "StrictStringEnum_InvalidValue"));
    ASSERT_TRUE(actualValue.IsNull());

    ASSERT_EQ(ECObjectsStatus::Success, m_instance->GetValue(actualValue, "LooseStringEnum_ValidValue"));
    EXPECT_STREQ("Loose3", actualValue.GetUtf8CP());

    ASSERT_EQ(ECObjectsStatus::Success, m_instance->GetValue(actualValue, "LooseStringEnum_InvalidValue"));
    EXPECT_STREQ("LooseInvalid", actualValue.GetUtf8CP());

    ASSERT_EQ(ECObjectsStatus::Success, m_instance->GetValue(actualValue, "StrictIntegerEnum_ValidValue"));
    ASSERT_EQ(2, actualValue.GetInteger());

    ASSERT_EQ(ECObjectsStatus::Success, m_instance->GetValue(actualValue, "StrictIntegerEnum_InvalidValue"));
    ASSERT_TRUE(actualValue.IsNull());

    ASSERT_EQ(ECObjectsStatus::Success, m_instance->GetValue(actualValue, "LooseIntegerEnum_ValidValue"));
    ASSERT_EQ(4, actualValue.GetInteger());

    ASSERT_EQ(ECObjectsStatus::Success, m_instance->GetValue(actualValue, "LooseIntegerEnum_InvalidValue"));
    ASSERT_EQ(34, actualValue.GetInteger());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (StringEncodingTests, TestComparisons)
    {
    WCharCP wc = L"Testing";
    Compare<true> (wc, Utf8String(wc).c_str(), Utf16String(wc).c_str());
    Compare<false> (wc, Utf8String ("abcdefg").c_str(), Utf16String(L"blarg").c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (StringEncodingTests, TestConversions)
    {
    WCharCP wc = L"Testing";
    Convert (ECValue (wc), wc);

    Utf8String u8 (wc);
    ECValue v8 (u8.c_str());
    Convert (v8, wc);

    Utf16String u16 (wc);
    ECValue v16 (u16.c_str());
    Convert (v16, wc);

    Convert (ECValue (wc, false), wc);
    Convert (ECValue (u8.c_str(), false), wc);
    Convert (ECValue (u16.c_str(), false), wc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (StringEncodingTests, CompareBuffersWithDifferentEncodings)
    {
    StandaloneECInstancePtr a = CreatePrimaryInstance (ECDBuffer::StringEncoding_Utf8, "testing"),
                            b = CreatePrimaryInstance (ECDBuffer::StringEncoding_Utf16, "no match");
    CompareInstances<false> (a, b);

    b = CreatePrimaryInstance (ECDBuffer::StringEncoding_Utf16, "testing");
    CompareInstances<true> (a, b);

    // create a struct array instance with a different encoding than it's parent instance. Useful? not really. But nothing prohibits it.
    StandaloneECInstancePtr structA0 = CreateStructInstance (ECDBuffer::StringEncoding_Utf16, "child", *a),
                            structA1 = CreateStructInstance (ECDBuffer::StringEncoding_Utf8, "child", *a),
                            structB0 = CreateStructInstance (ECDBuffer::StringEncoding_Utf16, "child", *b),
                            structB1 = CreateStructInstance (ECDBuffer::StringEncoding_Utf8, "child", *b);

    CompareInstances<true> (structA0, structB0);
    CompareInstances<true> (structA0, structA1);
    CompareInstances<true> (structA1, structB0);
    CompareInstances<true> (a, b);

    structB1->SetValue ("Name", ECValue ("grandkid"));
    ECValue structV;
    structV.SetStruct (structB1.get());
    b->SetValue ("StructArray", structV, 1);
    CompareInstances<false> (structB1, structB0);
    CompareInstances<false> (structB1, structA1);
    CompareInstances<false> (a, b);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (StringEncodingTests, CopyBuffersWithDifferentEncodings)
    {
    // a and b use different encodings
    StandaloneECInstancePtr a = CreatePrimaryInstance (ECDBuffer::StringEncoding_Utf8, "slartibartfast");
    ECDBuffer::SetDefaultStringEncoding (ECDBuffer::StringEncoding_Utf16);
    StandaloneECInstancePtr b = m_schema->GetClassP ("TestClass")->GetDefaultStandaloneEnabler()->CreateInstance();

    EXPECT_EQ (ECDBuffer::StringEncoding_Utf16, b->GetStringEncoding());

    // Copying instances copies the entire buffer - including the encoding flag and the strings in their original encodings.
    EXPECT_EQ (ECObjectsStatus::Success, b->CopyValues (*a));

    // a and b should now have the same encoding
    EXPECT_EQ (ECDBuffer::StringEncoding_Utf8, b->GetStringEncoding());

    CompareInstances<true> (a, b);

    b->SetValue ("Struct.Name", ECValue ("finnegan"));
    CompareInstances<false> (a, b);
    }

/*---------------------------------------------------------------------------------**//**
* If the ECClass has the custom attribute "PersistStringsAsUtf8", instances will always
* use Utf-8 encoding.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (StringEncodingTests, ForceUtf8)
    {
    ECDBuffer::SetDefaultStringEncoding (ECDBuffer::StringEncoding_Utf16);
    StandaloneECInstancePtr a = m_schema->GetClassP ("TestUtf8Class")->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_EQ (ECDBuffer::StringEncoding_Utf8, a->GetStringEncoding());

    ECDBuffer::SetDefaultStringEncoding (ECDBuffer::StringEncoding_Utf8);
    StandaloneECInstancePtr b = m_schema->GetClassP ("TestUtf8Class")->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_EQ (ECDBuffer::StringEncoding_Utf8, b->GetStringEncoding());

    ECDBuffer::SetDefaultStringEncoding (ECDBuffer::StringEncoding_Utf16);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CompressInstanceTests, CheckVariableSizedPropertyAfterCallingCompress)
    {
    ReadKitchenSinkSchemaFromXml ();
    ECClassP ecClass = m_schema->GetClassP ("KitchenSink");
    ASSERT_TRUE (NULL != ecClass);

    StandaloneECEnablerPtr enabler = ecClass->GetDefaultStandaloneEnabler ();
    ECN::StandaloneECInstancePtr instance = enabler->CreateInstance ();

    int        inCount = 100;
    double     inLength = 432.178;
    bool       inTest = true;

    ASSERT_EQ (ECObjectsStatus::Success, instance->SetValue ("myInt", ECValue (inCount)));
    ASSERT_EQ (ECObjectsStatus::Success, instance->SetValue ("myString", ECValue ("Test")));
    ASSERT_EQ (ECObjectsStatus::Success, instance->SetValue ("myDouble", ECValue (inLength)));
    ASSERT_EQ (ECObjectsStatus::Success, instance->SetValue ("myBool", ECValue (inTest)));

    ECValue ecValue;

    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "myInt"));
    EXPECT_TRUE (ecValue.GetInteger () == inCount);

    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "myString"));
    EXPECT_STREQ (ecValue.GetUtf8CP (), "Test");

    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "myDouble"));
    EXPECT_TRUE (ecValue.GetDouble () == inLength);

    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "myBool"));
    EXPECT_TRUE (ecValue.GetBoolean () == inTest);

    instance->Compress ();

    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "myInt"));
    EXPECT_TRUE (ecValue.GetInteger () == inCount);

    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "myString"));
    EXPECT_STREQ (ecValue.GetUtf8CP (), "Test");

    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "myDouble"));
    EXPECT_TRUE (ecValue.GetDouble () == inLength);

    EXPECT_TRUE (ECObjectsStatus::Success == instance->GetValue (ecValue, "myBool"));
    EXPECT_TRUE (ecValue.GetBoolean () == inTest);

    // define struct array
    StandaloneECEnablerPtr manufacturerEnabler = instance->GetEnablerR ().GetEnablerForStructArrayMember (m_schema->GetSchemaKey (), "Manufacturer");
    EXPECT_TRUE (manufacturerEnabler.IsValid ());

    ECValue v;
    ASSERT_TRUE (ECObjectsStatus::Success == instance->AddArrayElements ("myManufacturerStructArray", 4));
    instance->Compress ();
    VerifyArrayInfo (*instance, v, "myManufacturerStructArray", 4, false);
    instance->Compress ();
    VerifyIsNullArrayElements (*instance, v, "myManufacturerStructArray", 0, 4, true);

    IECInstancePtr manufInst = manufacturerEnabler->CreateInstance ().get ();

    SetAndVerifyString (*manufInst, v, "Name", "Nissan");
    instance->Compress ();
    SetAndVerifyInteger (*manufInst, v, "AccountNo", 3475);
    instance->Compress ();
    v.SetStruct (manufInst.get ());
    ASSERT_TRUE (ECObjectsStatus::Success == instance->SetValue ("myManufacturerStructArray", v, 0));

    manufInst = manufacturerEnabler->CreateInstance ().get ();
    SetAndVerifyString (*manufInst, v, "Name", "Kia");
    SetAndVerifyInteger (*manufInst, v, "AccountNo", 1791);
    v.SetStruct (manufInst.get ());
    ASSERT_TRUE (ECObjectsStatus::Success == instance->SetValue ("myManufacturerStructArray", v, 1));

    manufInst = manufacturerEnabler->CreateInstance ().get ();
    SetAndVerifyString (*manufInst, v, "Name", "Honda");
    SetAndVerifyInteger (*manufInst, v, "AccountNo", 1592);
    v.SetStruct (manufInst.get ());
    ASSERT_TRUE (ECObjectsStatus::Success == instance->SetValue ("myManufacturerStructArray", v, 2));

    manufInst = manufacturerEnabler->CreateInstance ().get ();
    SetAndVerifyString (*manufInst, v, "Name", "Chevy");
    SetAndVerifyInteger (*manufInst, v, "AccountNo", 19341);
    v.SetStruct (manufInst.get ());
    ASSERT_TRUE (ECObjectsStatus::Success == instance->SetValue ("myManufacturerStructArray", v, 3));
    instance->Compress ();
    VerifyIsNullArrayElements (*instance, v, "myManufacturerStructArray", 0, 4, false);

    // remove struct array element
    instance->RemoveArrayElement ("myManufacturerStructArray", 2);
    instance->Compress ();
    validateArrayCount (*instance, "myManufacturerStructArray", 3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyIndexTests, FlatteningIterator)
    {
    static const PrimitiveType testTypes[] = {PRIMITIVETYPE_Integer, PRIMITIVETYPE_String};
    for (size_t typeIndex = 0; typeIndex < _countof(testTypes); typeIndex++)
        {
        auto primType = testTypes[typeIndex];
        // Create an ECClass with nested structs like so:
        //  1
        //  2
        //      3
        //      4
        //          5
        //  6 { empty struct }
        //  7
        //      8
        //  9
        ECSchemaPtr schema;
        ECSchema::CreateSchema(schema, "Schema", "ts", 1, 0, 0);
        PrimitiveECPropertyP primProp;
        StructECPropertyP structProp;

        ECStructClassP s4;
        schema->CreateStructClass(s4, "S4");
        EXPECT_EQ(ECObjectsStatus::Success, s4->CreatePrimitiveProperty(primProp, "P5", primType));

        ECStructClassP s2;
        schema->CreateStructClass(s2, "S2");
        EXPECT_EQ(ECObjectsStatus::Success, s2->CreatePrimitiveProperty(primProp, "P3", primType));
        EXPECT_EQ(ECObjectsStatus::Success, s2->CreateStructProperty(structProp, "P4", *s4));

        ECStructClassP s6;
        schema->CreateStructClass(s6, "S6");

        ECStructClassP s7;
        schema->CreateStructClass(s7, "S7");
        EXPECT_EQ(ECObjectsStatus::Success, s7->CreatePrimitiveProperty(primProp, "P8", primType));

        ECEntityClassP ecClass;
        schema->CreateEntityClass(ecClass, "MyClass");
        EXPECT_EQ(ECObjectsStatus::Success, ecClass->CreatePrimitiveProperty(primProp, "P1", primType));
        EXPECT_EQ(ECObjectsStatus::Success, ecClass->CreateStructProperty(structProp, "P2", *s2));
        EXPECT_EQ(ECObjectsStatus::Success, ecClass->CreateStructProperty(structProp, "P6", *s6));
        EXPECT_EQ(ECObjectsStatus::Success, ecClass->CreateStructProperty(structProp, "P7", *s7));
        EXPECT_EQ(ECObjectsStatus::Success, ecClass->CreatePrimitiveProperty(primProp, "P9", primType));

        // Expect property indices returned using depth-first traversal of struct members
        // Expect indices of struct properties are not returned
        // Note that order in which property indices are assigned and returned depends on fixed-sized vs variable-sized property types.
        Utf8CP expect[] = {"P1", "P2.P3", "P2.P4.P5", "P7.P8", "P9"};

        auto const& enabler = *ecClass->GetDefaultStandaloneEnabler();
        uint32_t propIdx;
        bset<Utf8CP> matched;
        for (PropertyIndexFlatteningIterator iter(enabler); iter.GetCurrent(propIdx); iter.MoveNext())
            {
            Utf8CP accessString = nullptr;
            EXPECT_EQ(ECObjectsStatus::Success, enabler.GetAccessString(accessString, propIdx));
            bool foundMatch = false;
            for (size_t i = 0; i < _countof(expect); i++)
                {
                if (0 == strcmp(expect[i], accessString))
                    {
                    EXPECT_TRUE(matched.end() == matched.find(expect[i]));
                    matched.insert(expect[i]);
                    foundMatch = true;
                    break;
                    }
                }

            EXPECT_TRUE(foundMatch);
            }

        EXPECT_EQ(matched.size(), _countof(expect));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(InstanceTests, TestJsonAndXmlInstanceCompatibility)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECTestFixture::DeserializeSchema(schema, *context, SchemaItem::CreateForFile("BasicTest.ecschema.xml"));

    StandaloneECEnablerPtr enabler = schema->GetClassCP("Company")->GetDefaultStandaloneEnabler();
    IECInstancePtr testInstanceXml = enabler->CreateInstance();

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext(*schema);
    InstanceReadStatus instanceStatus = IECInstance::ReadFromXmlFile(testInstanceXml, GetTestDataPath(L"BasicTest_Instance1.xml").c_str(), *instanceContext);
    EXPECT_EQ(InstanceReadStatus::Success, instanceStatus);

    IECInstancePtr testInstanceJson = enabler->CreateInstance();
    Json::Value instance;
    BeFileName instanceJson(GetTestDataPath(L"BasicTest_Instance1.json").c_str());
    ASSERT_EQ(BentleyStatus::SUCCESS, ECTestUtility::ReadJsonInputFromFile(instance, instanceJson));

    InSchemaClassLocater classLocater(*schema);
    EXPECT_EQ(BentleyStatus::SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstanceJson.get(), instance["Company"], classLocater));

    EXPECT_TRUE(ECTestUtility::CompareECInstances(*testInstanceXml, *testInstanceJson));
    }

END_BENTLEY_ECN_TEST_NAMESPACE


