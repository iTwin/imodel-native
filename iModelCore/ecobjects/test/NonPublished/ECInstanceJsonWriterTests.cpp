/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include <initializer_list>
#include <unordered_map>
#include <unordered_set>

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

USING_NAMESPACE_BENTLEY_EC

//=======================================================================================
// @bsistruct
//=======================================================================================
struct ECClassLocater : IECClassLocater
    {
    ECSchemaCR m_schema;
    ECClassLocater(ECSchemaCR schema) : m_schema(schema) {}

protected:
    ECClassCP _LocateClass(ECClassId const& classId) override
        {
        auto const& ecClasses = m_schema.GetClasses();

         auto const it = std::find_if(std::begin(ecClasses), std::end(ecClasses),
               [&classId] (auto const& ecClass)
                  {
                  return nullptr != ecClass && ecClass->HasId() && ecClass->GetId() == classId;
                  });

         if (std::end(ecClasses) == it)
               return nullptr;
         return *it;
        }

    ECClassCP _LocateClass(Utf8CP schemaName, Utf8CP className) override
        {
        return m_schema.LookupClass(className);
        }
    };

struct ECInstanceBuilder;

//=======================================================================================
// @bsistruct
//=======================================================================================
struct Value : NonCopyableClass
    {
    friend ECInstanceBuilder;

private:
    ECValue m_ecValue;
    bool m_isArrayMember;
    int m_arrayCount;
    int m_arrayIndex;

    std::unordered_multimap<Utf8String, std::unique_ptr<Value>>* m_instanceProperties;

public:
    Value(Value &&) = default;
    Value& operator=(Value &&) = default;

    template <typename T, typename = std::enable_if_t<!std::is_base_of<T, ECInstanceBuilder>::value>> Value(T&& value) :
        m_ecValue(value), m_isArrayMember(false), m_instanceProperties(nullptr) {}
    template <typename T, typename = std::enable_if_t<!std::is_base_of<T, ECInstanceBuilder>::value>> Value(T&& value, int count, int index) :
        m_ecValue(value), m_isArrayMember(true), m_arrayCount(count), m_arrayIndex(index), m_instanceProperties(nullptr) {}
    template <typename T, typename = std::enable_if_t<std::is_base_of<T, ECInstanceBuilder>::value>> Value(T& value, int count, int index) :
        m_isArrayMember(true), m_arrayCount(count), m_arrayIndex(index), m_instanceProperties(nullptr) { m_ecValue.SetStruct(value.BuildInstance().get()); }
    };

//=======================================================================================
// @bsistruct
//=======================================================================================
struct ECInstanceBuilder
    {
private:
    std::unordered_multimap<Utf8String, std::unique_ptr<Value>> m_instanceProperties;
    std::unordered_set<Utf8String> m_initializedArrays;
    ECClassCR m_ecClass;

public:
    ECInstanceBuilder(ECClassCR ecClass) : m_ecClass(ecClass) {}

    template <typename TValue> void EmplaceProperty(Utf8String name, TValue value)
        {
        m_instanceProperties.emplace(std::make_pair(name, std::make_unique<Value>(value)));
        }

    template <typename TValue> void EmplaceProperty(Utf8String name, TValue value, int arrayCount, int arrayIndex)
        {
        m_instanceProperties.emplace(std::make_pair(name, std::make_unique<Value>(value, arrayCount, arrayIndex)));
        }

    void EmplaceProperty(Utf8String name, ECInstanceBuilder& structInstanceBuilder)
        {
        for (auto& property : structInstanceBuilder.m_instanceProperties)
            {
            std::unique_ptr<Value> propertyValue(property.second.release()); // Fix for build on linuxx64
            m_instanceProperties.emplace(std::make_pair(name + "." + property.first, std::move(propertyValue)));
            }
        }

    void EmplaceProperty(Utf8String name, ECInstanceBuilder& structInstanceBuilder, int arrayCount, int arrayIndex)
        {
        m_instanceProperties.emplace(std::make_pair(name, std::make_unique<Value>(structInstanceBuilder, arrayCount, arrayIndex)));
        }

    auto BuildInstance()
        {
        auto instance = m_ecClass.GetDefaultStandaloneEnabler()->CreateInstance();
        for (auto const& property : m_instanceProperties)
            {
            if (property.second->m_isArrayMember)
                {
                if (m_initializedArrays.emplace(property.first).second)
                    instance->AddArrayElements(property.first.c_str(), property.second->m_arrayCount);
                instance->SetValue(property.first.c_str(), property.second->m_ecValue, property.second->m_arrayIndex);
                }
            else
                instance->SetValue(property.first.c_str(), property.second->m_ecValue);
            }

        return instance;
        }

    };

//=======================================================================================
// @bsistruct
//=======================================================================================
struct ECInstanceJsonWriterTests : ECTestFixture
    {
    static ECSchemaPtr m_schema;

    static ECRelationshipClassP m_relationshipClass;
    static ECEntityClassP m_sourceClass;
    static ECEntityClassP m_targetClass;
    static ECStructClassP m_structClass;
    static ECStructClassP m_structStructClass;
    static ECEnumerationP m_strictEnumeration;
    static ECEnumerationP m_looseEnumeration;

    static StructECPropertyP m_sourceStructProperty;
    static NavigationECPropertyP m_sourceNavigationProperty;

    static StructECPropertyP m_targetStructProperty;

    static PrimitiveECPropertyP m_structIntProperty;
    static PrimitiveECPropertyP m_structDoubleProperty;
    static PrimitiveECPropertyP m_structStringProperty;
    static PrimitiveECPropertyP m_structGeometryProperty;
    static PrimitiveECPropertyP m_structStrictEnumerationProperty;
    static PrimitiveECPropertyP m_structLooseEnumerationProperty;
    static PrimitiveArrayECPropertyP m_structDoubleArrayProperty;
    static StructECPropertyP m_structStructProperty;
    static StructArrayECPropertyP m_structStructArrayProperty;

    static PrimitiveECPropertyP m_structStructIntProperty;
    static PrimitiveECPropertyP m_structStructDoubleProperty;
    static PrimitiveECPropertyP m_structStructStringProperty;
    static PrimitiveECPropertyP m_structStructGeometryProperty;
    static PrimitiveECPropertyP m_structStructStrictEnumerationProperty;
    static PrimitiveECPropertyP m_structStructLooseEnumerationProperty;
    static PrimitiveArrayECPropertyP m_structStructDoubleArrayProperty;

    static ECEnumeratorP m_strictEnumeratorA;
    static ECEnumeratorP m_strictEnumeratorB;

    static ECEnumeratorP m_looseEnumerator1;
    static ECEnumeratorP m_looseEnumerator2;

    static KindOfQuantityP m_angleKindOfQuantity;
    static KindOfQuantityP m_volumeKindOfQuantity;

protected:
    void SetUp() override
        {
        if (m_schema.IsValid())
            return;

        ECSchema::CreateSchema(m_schema, "Schema", "sch", 1, 0, 0);

        m_schema->AddReferencedSchema(*GetUnitsSchema());
        m_schema->AddReferencedSchema(*GetFormatsSchema());

        m_schema->CreateEntityClass(m_sourceClass, "SourceClass");
        m_schema->CreateEntityClass(m_targetClass, "TargetClass");
        m_schema->CreateRelationshipClass(m_relationshipClass, "RelationshipClass");
        m_schema->CreateStructClass(m_structClass, "OuterStructClass");
        m_schema->CreateStructClass(m_structStructClass, "InnerStructClass");
        m_schema->CreateEnumeration(m_strictEnumeration, "StrictEnumeration", PRIMITIVETYPE_String);
        m_schema->CreateEnumeration(m_looseEnumeration, "LooseEnumeration", PRIMITIVETYPE_Integer);
        m_schema->CreateKindOfQuantity(m_volumeKindOfQuantity, "VolumeKindOfQuantity");
        m_schema->CreateKindOfQuantity(m_angleKindOfQuantity, "AngleKindOfQuantity");

        m_relationshipClass->GetSource().AddClass(*m_sourceClass);
        m_relationshipClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
        m_relationshipClass->GetTarget().AddClass(*m_targetClass);
        m_relationshipClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne());
        m_relationshipClass->SetId(ECClassId(uint64_t(42)));

        m_sourceClass->CreateStructProperty(m_sourceStructProperty, "StructProperty", *m_structClass);
        m_sourceClass->CreateNavigationProperty(m_sourceNavigationProperty, "NavigationProperty", *m_relationshipClass, ECRelatedInstanceDirection::Forward);
        m_sourceClass->SetId(ECClassId(uint64_t(45)));

        m_targetClass->CreateStructProperty(m_targetStructProperty, "StructProperty", *m_structClass);
        m_targetClass->SetId(ECClassId(uint64_t(48)));

        m_structClass->CreatePrimitiveProperty(m_structIntProperty, "IntProperty", PRIMITIVETYPE_Integer);
        m_structClass->CreatePrimitiveProperty(m_structDoubleProperty, "DoubleProperty", PRIMITIVETYPE_Double);
        m_structClass->CreatePrimitiveProperty(m_structStringProperty, "StringProperty", PRIMITIVETYPE_String);
        m_structClass->CreatePrimitiveProperty(m_structGeometryProperty, "GeometryProperty", PRIMITIVETYPE_IGeometry);
        m_structClass->CreateEnumerationProperty(m_structStrictEnumerationProperty, "StrictEnumerationProperty", *m_strictEnumeration);
        m_structClass->CreateEnumerationProperty(m_structLooseEnumerationProperty, "LooseEnumerationProperty", *m_looseEnumeration);
        m_structClass->CreatePrimitiveArrayProperty(m_structDoubleArrayProperty, "DoubleArrayProperty", PRIMITIVETYPE_Double);
        m_structClass->CreateStructProperty(m_structStructProperty, "StructStructProperty", *m_structStructClass);
        m_structClass->CreateStructArrayProperty(m_structStructArrayProperty, "StructStructArrayProperty", *m_structStructClass);

        m_structStructClass->CreatePrimitiveProperty(m_structStructIntProperty, "IntProperty", PRIMITIVETYPE_Integer);
        m_structStructClass->CreatePrimitiveProperty(m_structStructDoubleProperty, "DoubleProperty", PRIMITIVETYPE_Double);
        m_structStructClass->CreatePrimitiveProperty(m_structStructStringProperty, "StringProperty", PRIMITIVETYPE_String);
        m_structStructClass->CreatePrimitiveProperty(m_structStructGeometryProperty, "GeometryProperty", PRIMITIVETYPE_IGeometry);
        m_structStructClass->CreateEnumerationProperty(m_structStructStrictEnumerationProperty, "StrictEnumerationProperty", *m_strictEnumeration);
        m_structStructClass->CreateEnumerationProperty(m_structStructLooseEnumerationProperty, "LooseEnumerationProperty", *m_looseEnumeration);
        m_structStructClass->CreatePrimitiveArrayProperty(m_structStructDoubleArrayProperty, "DoubleArrayProperty", PRIMITIVETYPE_Double);

        m_strictEnumeration->SetIsStrict(true);
        m_strictEnumeration->CreateEnumerator(m_strictEnumeratorA, "EnumeratorA");
        m_strictEnumeration->CreateEnumerator(m_strictEnumeratorB, "EnumeratorB");

        m_looseEnumeration->SetIsStrict(false);
        m_looseEnumeration->CreateEnumerator(m_looseEnumerator1, 1);
        m_looseEnumeration->CreateEnumerator(m_looseEnumerator2, 2);

        m_angleKindOfQuantity->SetPersistenceUnit(*GetUnitsSchema()->GetUnitCP("RAD"));
        m_angleKindOfQuantity->AddPresentationFormat(*GetFormatsSchema()->GetFormatCP("AngleDMS"), 2);

        m_volumeKindOfQuantity->SetPersistenceUnit(*GetUnitsSchema()->GetUnitCP("THOUSAND_GALLON"));
        m_volumeKindOfQuantity->SetDefaultPresentationFormat(*GetFormatsSchema()->GetFormatCP("DefaultRealU"), 3, GetUnitsSchema()->GetUnitCP("CUB_M"));

        m_structDoubleArrayProperty->SetKindOfQuantity(m_angleKindOfQuantity);
        m_structStructDoubleArrayProperty->SetKindOfQuantity(m_angleKindOfQuantity);

        m_structDoubleProperty->SetKindOfQuantity(m_volumeKindOfQuantity);
        m_structStructDoubleProperty->SetKindOfQuantity(m_volumeKindOfQuantity);
        }

public:
    static void SetStructStructProperties(ECInstanceBuilder& innerStructBuilder, Nullable<int> const& intProperty, Nullable<double> const& doubleProperty, Utf8CP stringProperty,
        IGeometryPtr geometryProperty, Utf8CP strictEnumerationProperty, Nullable<int> const& looseEnumerationProperty, int doubleArrayCount, std::initializer_list<double> doubleArray)
        {
        SetCommonStructProperties(innerStructBuilder, intProperty, doubleProperty, stringProperty, geometryProperty, strictEnumerationProperty, looseEnumerationProperty, doubleArrayCount, doubleArray);
        }

    static void SetStructProperties(ECInstanceBuilder& outerStructBuilder, Nullable<int> const& intProperty, Nullable<double> const& doubleProperty, Utf8CP stringProperty,
        IGeometryPtr geometryProperty, Utf8CP strictEnumerationProperty, Nullable<int> const& looseEnumerationProperty, int doubleArrayCount, std::initializer_list<double> doubleArray,
        ECInstanceBuilder* innerStructBuilder, int innerStructArrayCount, std::initializer_list<ECInstanceBuilder*> innerStructArray)
        {
        SetCommonStructProperties(outerStructBuilder, intProperty, doubleProperty, stringProperty,
            geometryProperty, strictEnumerationProperty, looseEnumerationProperty, doubleArrayCount, doubleArray);

        if (nullptr != innerStructBuilder)
            outerStructBuilder.EmplaceProperty("StructStructProperty", *innerStructBuilder);

        auto pos = -1;
        for (auto& builder : innerStructArray)
            outerStructBuilder.EmplaceProperty("StructStructArrayProperty", *builder, innerStructArrayCount, ++pos);

        ASSERT_EQ(++pos, innerStructArrayCount) << "Wrong struct array count passed to the SetStructProperties() function.";
        }

private:
    static void SetCommonStructProperties(ECInstanceBuilder& structBuilder, Nullable<int> const& intProperty, Nullable<double> const& doubleProperty, Utf8CP stringProperty,
        IGeometryPtr geometryProperty, Utf8CP strictEnumerationProperty, Nullable<int> const& looseEnumerationProperty, int doubleArrayCount, std::initializer_list<double> doubleArray)
        {
        if(intProperty.IsValid())
            structBuilder.EmplaceProperty("IntProperty", intProperty.Value());
        if(doubleProperty.IsValid())
            structBuilder.EmplaceProperty("DoubleProperty", doubleProperty.Value());
        if (looseEnumerationProperty.IsValid())
            structBuilder.EmplaceProperty("LooseEnumerationProperty", looseEnumerationProperty.Value());
        if (geometryProperty.IsValid())
         structBuilder.EmplaceProperty("GeometryProperty", *geometryProperty);

        structBuilder.EmplaceProperty("StringProperty", stringProperty);
        structBuilder.EmplaceProperty("StrictEnumerationProperty", strictEnumerationProperty);

        auto pos = -1;
        for (auto const& value : doubleArray)
            structBuilder.EmplaceProperty("DoubleArrayProperty", value, doubleArrayCount, ++pos);

        ASSERT_EQ(++pos, doubleArrayCount) << "Wrong double array count passed to the SetStructProperties() or SetStructStructProperties() function.";
        }
    };

// Static member definitions

ECSchemaPtr ECInstanceJsonWriterTests::m_schema;
ECRelationshipClassP ECInstanceJsonWriterTests::m_relationshipClass;
ECEntityClassP ECInstanceJsonWriterTests::m_sourceClass;
ECEntityClassP ECInstanceJsonWriterTests::m_targetClass;
ECStructClassP ECInstanceJsonWriterTests::m_structClass;
ECStructClassP ECInstanceJsonWriterTests::m_structStructClass;
ECEnumerationP ECInstanceJsonWriterTests::m_strictEnumeration;
ECEnumerationP ECInstanceJsonWriterTests::m_looseEnumeration;
StructECPropertyP ECInstanceJsonWriterTests::m_sourceStructProperty;
NavigationECPropertyP ECInstanceJsonWriterTests::m_sourceNavigationProperty;
StructECPropertyP ECInstanceJsonWriterTests::m_targetStructProperty;
PrimitiveECPropertyP ECInstanceJsonWriterTests::m_structIntProperty;
PrimitiveECPropertyP ECInstanceJsonWriterTests::m_structDoubleProperty;
PrimitiveECPropertyP ECInstanceJsonWriterTests::m_structStringProperty;
PrimitiveECPropertyP ECInstanceJsonWriterTests::m_structGeometryProperty;
PrimitiveECPropertyP ECInstanceJsonWriterTests::m_structStrictEnumerationProperty;
PrimitiveECPropertyP ECInstanceJsonWriterTests::m_structLooseEnumerationProperty;
PrimitiveArrayECPropertyP ECInstanceJsonWriterTests::m_structDoubleArrayProperty;
StructECPropertyP ECInstanceJsonWriterTests::m_structStructProperty;
StructArrayECPropertyP ECInstanceJsonWriterTests::m_structStructArrayProperty;
PrimitiveECPropertyP ECInstanceJsonWriterTests::m_structStructIntProperty;
PrimitiveECPropertyP ECInstanceJsonWriterTests::m_structStructDoubleProperty;
PrimitiveECPropertyP ECInstanceJsonWriterTests::m_structStructStringProperty;
PrimitiveECPropertyP ECInstanceJsonWriterTests::m_structStructGeometryProperty;
PrimitiveECPropertyP ECInstanceJsonWriterTests::m_structStructStrictEnumerationProperty;
PrimitiveECPropertyP ECInstanceJsonWriterTests::m_structStructLooseEnumerationProperty;
PrimitiveArrayECPropertyP ECInstanceJsonWriterTests::m_structStructDoubleArrayProperty;
ECEnumeratorP ECInstanceJsonWriterTests::m_strictEnumeratorA;
ECEnumeratorP ECInstanceJsonWriterTests::m_strictEnumeratorB;
ECEnumeratorP ECInstanceJsonWriterTests::m_looseEnumerator1;
ECEnumeratorP ECInstanceJsonWriterTests::m_looseEnumerator2;
KindOfQuantityP ECInstanceJsonWriterTests::m_angleKindOfQuantity;
KindOfQuantityP ECInstanceJsonWriterTests::m_volumeKindOfQuantity;

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECInstanceJsonWriterTests, WriteEmbeddedStructValueTest)
    {
    // Arrange

    ECInstanceBuilder
        sourceInstanceBuilder(*m_sourceClass),
        structBuilder(*m_structClass),
        structStructBuilder(*m_structStructClass),
        structStructArrayMemberBuilder1(*m_structStructClass),
        structStructArrayMemberBuilder2(*m_structStructClass);

    SetStructStructProperties(structStructBuilder, 0, 3.14, "Pi 3 digits", nullptr, nullptr, 1, 4, { 1.0, 2.0, 4.0, 9.2 });
    SetStructStructProperties(structStructArrayMemberBuilder1, +1234567890, 3.1415926535897932384626433832795028841971, u8"π 41 digits", nullptr, "EnumeratorB", 2, 2, { 16, 25 });
    SetStructStructProperties(structStructArrayMemberBuilder2, -1234567890, 2.7182818284590452353602874713526624977572, "e 41 digits", nullptr, "EnumeratorB", 2, 0, {});
    SetStructProperties(structBuilder, 1, 2.0, "2.0", nullptr, "EnumeratorA", 1, 0, {}, &structStructBuilder, 2, { &structStructArrayMemberBuilder1, &structStructArrayMemberBuilder2 });
    sourceInstanceBuilder.EmplaceProperty("StructProperty", structBuilder);

    auto const sourceInstance = sourceInstanceBuilder.BuildInstance();

    auto expectedJsonValue12 = Json::Value::From(u8R"(
        {
           "StructProperty" : {
              "IntProperty" : 1,
              "DoubleProperty" : 2.0,
              "StringProperty" : "2.0",
              "StrictEnumerationProperty" : "EnumeratorA",
              "LooseEnumerationProperty" : 1,
              "StructStructProperty" : {
                 "IntProperty" : 0,
                 "DoubleProperty" : 3.14,
                 "StringProperty" : "Pi 3 digits",
                 "LooseEnumerationProperty" : 1,
                 "DoubleArrayProperty" : [ 1, 2, 4, 9.2 ]
              },
              "StructStructArrayProperty" : [
                 {
                    "IntProperty" : 1234567890,
                    "DoubleProperty" : 3.1415926535897932384626433832795028841971,
                    "StringProperty" : "π 41 digits",
                    "StrictEnumerationProperty" : "EnumeratorB",
                    "LooseEnumerationProperty" : 2,
                    "DoubleArrayProperty" : [ 16, 25 ]
                 },
                 {
                    "IntProperty" : -1234567890,
                    "DoubleProperty" : 2.7182818284590452353602874713526624977572,
                    "StringProperty" : "e 41 digits",
                    "StrictEnumerationProperty" : "EnumeratorB",
                    "LooseEnumerationProperty" : 2
                 }
              ]
           }
        })");

    auto expectedJsonValue34 = Json::Value::From(u8R"(
        {
           "StructStructProperty" : {
              "IntProperty" : 0,
              "DoubleProperty" : 3.14,
              "StringProperty" : "Pi 3 digits",
              "LooseEnumerationProperty" : 1,
              "DoubleArrayProperty" : [ 1, 2, 4, 9.2  ]
           }
        })");

    auto expectedJsonValue5 = Json::Value::From(u8R"(
        {
           "StructStructProperty" : {
              "IntProperty" : 0,
              "DoubleProperty" : 3.14,
              "StringProperty" : "Pi 3 digits",
              "LooseEnumerationProperty" : 1,
              "DoubleArrayProperty" : [ 1, 2, 4, 9.2 ]
           },
           "StructProperty" : {
              "IntProperty" : 1,
              "DoubleProperty" : 2.0,
              "StringProperty" : "2.0",
              "StrictEnumerationProperty" : "EnumeratorA",
              "LooseEnumerationProperty" : 1,
              "StructStructProperty" : {
                 "IntProperty" : 0,
                 "DoubleProperty" : 3.14,
                 "StringProperty" : "Pi 3 digits",
                 "LooseEnumerationProperty" : 1,
                 "DoubleArrayProperty" : [ 1, 2, 4, 9.2 ]
              },
              "StructStructArrayProperty" : [
                 {
                    "IntProperty" : 1234567890,
                    "DoubleProperty" : 3.1415926535897932384626433832795028841971,
                    "StringProperty" : "π 41 digits",
                    "StrictEnumerationProperty" : "EnumeratorB",
                    "LooseEnumerationProperty" : 2,
                    "DoubleArrayProperty" : [ 16, 25 ]
                 },
                 {
                    "IntProperty" : -1234567890,
                    "DoubleProperty" : 2.7182818284590452353602874713526624977572,
                    "StringProperty" : "e 41 digits",
                    "StrictEnumerationProperty" : "EnumeratorB",
                    "LooseEnumerationProperty" : 2
                 }
              ]
           }
        })");

    // Act

    Json::Value actualJsonValue1;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteEmbeddedStructValue(actualJsonValue1, *m_sourceStructProperty, *sourceInstance, nullptr));

    Json::Value actualJsonValue2;
    Utf8String baseAccessString2("");
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteEmbeddedStructValue(actualJsonValue2, *m_sourceStructProperty, *sourceInstance, baseAccessString2.c_str()));

    Json::Value actualJsonValue3;
    Utf8String baseAccessString3("StructProperty");
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteEmbeddedStructValue(actualJsonValue3, *m_structStructProperty, *sourceInstance, baseAccessString3.c_str()));

    Json::Value actualJsonValue4;
    Utf8String baseAccessString4("StructProperty.");
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteEmbeddedStructValue(actualJsonValue4, *m_structStructProperty, *sourceInstance, baseAccessString4.c_str()));

    Json::Value actualJsonValue5;
    Utf8String baseAccessString5("StructProperty");
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteEmbeddedStructValue(actualJsonValue5, *m_structStructProperty, *sourceInstance, baseAccessString5.c_str()));
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteEmbeddedStructValue(actualJsonValue5, *m_sourceStructProperty, *sourceInstance, nullptr));

    // Assert

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue12, actualJsonValue1)) <<
        "Expected:\n" + expectedJsonValue12.ToString() + "\nBut was:\n" + actualJsonValue1.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue12, actualJsonValue2)) <<
        "Expected:\n" + expectedJsonValue12.ToString() + "\nBut was:\n" + actualJsonValue2.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue34, actualJsonValue3)) <<
        "Expected:\n" + expectedJsonValue34.ToString() + "\nBut was:\n" + actualJsonValue3.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue34, actualJsonValue4)) <<
        "Expected:\n" + expectedJsonValue34.ToString() + "\nBut was:\n" + actualJsonValue4.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue5, actualJsonValue5)) <<
        "Expected:\n" + expectedJsonValue5.ToString() + "\nBut was:\n" + actualJsonValue5.ToString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECInstanceJsonWriterTests, WriteEmbeddedStructValueForPresentationTest)
    {
    // Arrange

    ECInstanceBuilder
        sourceInstanceBuilder(*m_sourceClass),
        structBuilder(*m_structClass),
        structStructBuilder(*m_structStructClass),
        structStructArrayMemberBuilder1(*m_structStructClass),
        structStructArrayMemberBuilder2(*m_structStructClass);

    SetStructStructProperties(structStructBuilder, nullptr, nullptr, u8"α β γ δ ε ζ η θ ι κ λ μ ν ξ ο π ρ ς σ τ υ φ χ ψ ω", nullptr, "EnumeratorB", nullptr, 1, { 7 });
    SetStructStructProperties(structStructArrayMemberBuilder1, 333333, 3.333333, nullptr, nullptr, nullptr, 2, 1, { 9.0 });
    SetStructStructProperties(structStructArrayMemberBuilder2, nullptr, nullptr, nullptr, nullptr, nullptr/*"EnumeratorInvalid"*/, nullptr, 0, { });
    SetStructProperties(structBuilder, nullptr, 1.111111, "1.111111", nullptr, nullptr, 1, 0, { }, &structStructBuilder, 2, { &structStructArrayMemberBuilder1, &structStructArrayMemberBuilder2 });
    sourceInstanceBuilder.EmplaceProperty("StructProperty", structBuilder);

    auto const sourceInstance = sourceInstanceBuilder.BuildInstance();

    auto expectedJsonValue12 = Json::Value::From(u8R"*(
        {
           "StructProperty" : {
              "StringProperty" : "1.111111",
              "DoubleProperty" : {
                 "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
                 "formattedValue" : "4.206 m³",
                 "rawValue" : 1.111111
              },
              "LooseEnumerationProperty" : 1,
              "StructStructProperty" : {
                 "StringProperty" : "α β γ δ ε ζ η θ ι κ λ μ ν ξ ο π ρ ς σ τ υ φ χ ψ ω",
                 "StrictEnumerationProperty" : "EnumeratorB",
                 "DoubleArrayProperty" : [
                    {
                       "currentUnit" : "AngleDMS(2)",
                       "formattedValue" : "401° 4' 13.64\"",
                       "rawValue" : 7
                    }
                 ]
              },
              "StructStructArrayProperty" : [
                 {
                    "IntProperty" : 333333,
                    "DoubleProperty" : {
                       "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
                       "formattedValue" : "12.618 m³",
                       "rawValue" : 3.333333
                    },
                    "LooseEnumerationProperty" : 2,
                    "DoubleArrayProperty" : [
                       {
                          "currentUnit" : "AngleDMS(2)",
                          "formattedValue" : "515° 39' 43.26\"",
                          "rawValue" : 9
                       }
                    ]
                 },
                 {}
              ]
           }
        })*");

    auto expectedJsonValue34 = Json::Value::From(u8R"*(
        {
           "StructStructProperty" : {
              "StringProperty" : "α β γ δ ε ζ η θ ι κ λ μ ν ξ ο π ρ ς σ τ υ φ χ ψ ω",
              "StrictEnumerationProperty" : "EnumeratorB",
              "DoubleArrayProperty" : [
                 {
                    "currentUnit" : "AngleDMS(2)",
                    "formattedValue" : "401° 4' 13.64\"",
                    "rawValue" : 7
                 }
              ]
           }
        })*");

    auto expectedJsonValue5 = Json::Value::From(u8R"*(
        {
           "StructStructProperty" : {
              "StringProperty" : "α β γ δ ε ζ η θ ι κ λ μ ν ξ ο π ρ ς σ τ υ φ χ ψ ω",
              "StrictEnumerationProperty" : "EnumeratorB",
              "DoubleArrayProperty" : [
                 {
                    "currentUnit" : "AngleDMS(2)",
                    "formattedValue" : "401° 4' 13.64\"",
                    "rawValue" : 7
                 }
              ]
           },
           "StructProperty" : {
              "StringProperty" : "1.111111",
              "DoubleProperty" : {
                 "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
                 "formattedValue" : "4.206 m³",
                 "rawValue" : 1.111111
              },
              "LooseEnumerationProperty" : 1,
              "StructStructProperty" : {
                 "StringProperty" : "α β γ δ ε ζ η θ ι κ λ μ ν ξ ο π ρ ς σ τ υ φ χ ψ ω",
                 "StrictEnumerationProperty" : "EnumeratorB",
                 "DoubleArrayProperty" : [
                    {
                       "currentUnit" : "AngleDMS(2)",
                       "formattedValue" : "401° 4' 13.64\"",
                       "rawValue" : 7
                    }
                 ]
              },
              "StructStructArrayProperty" : [
                 {
                    "IntProperty" : 333333,
                    "DoubleProperty" : {
                       "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
                       "formattedValue" : "12.618 m³",
                       "rawValue" : 3.333333
                    },
                    "LooseEnumerationProperty" : 2,
                    "DoubleArrayProperty" : [
                       {
                          "currentUnit" : "AngleDMS(2)",
                          "formattedValue" : "515° 39' 43.26\"",
                          "rawValue" : 9
                       }
                    ]
                 },
                 {}
              ]
           }
        })*");

    // Act

    Json::Value actualJsonValue1;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteEmbeddedStructValueForPresentation(actualJsonValue1, *m_sourceStructProperty, *sourceInstance, nullptr));

    Json::Value actualJsonValue2;
    Utf8String baseAccessString2("");
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteEmbeddedStructValueForPresentation(actualJsonValue2, *m_sourceStructProperty, *sourceInstance, baseAccessString2.c_str()));

    Json::Value actualJsonValue3;
    Utf8String baseAccessString3("StructProperty");
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteEmbeddedStructValueForPresentation(actualJsonValue3, *m_structStructProperty, *sourceInstance, baseAccessString3.c_str()));

    Json::Value actualJsonValue4;
    Utf8String baseAccessString4("StructProperty.");
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteEmbeddedStructValueForPresentation(actualJsonValue4, *m_structStructProperty, *sourceInstance, baseAccessString4.c_str()));

    Json::Value actualJsonValue5;
    Utf8String baseAccessString5("StructProperty");
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteEmbeddedStructValueForPresentation(actualJsonValue5, *m_structStructProperty, *sourceInstance, baseAccessString5.c_str()));
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteEmbeddedStructValueForPresentation(actualJsonValue5, *m_sourceStructProperty, *sourceInstance, nullptr));

    // Assert

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue12, actualJsonValue1)) <<
        "Expected:\n" + expectedJsonValue12.ToString() + "\nBut was:\n" + actualJsonValue1.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue12, actualJsonValue2)) <<
        "Expected:\n" + expectedJsonValue12.ToString() + "\nBut was:\n" + actualJsonValue2.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue34, actualJsonValue3)) <<
        "Expected:\n" + expectedJsonValue34.ToString() + "\nBut was:\n" + actualJsonValue3.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue34, actualJsonValue4)) <<
        "Expected:\n" + expectedJsonValue34.ToString() + "\nBut was:\n" + actualJsonValue4.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue5, actualJsonValue5)) <<
        "Expected:\n" + expectedJsonValue5.ToString() + "\nBut was:\n" + actualJsonValue5.ToString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECInstanceJsonWriterTests, WritePrimitiveValueTest)
    {
    // Arrange

    ECInstanceBuilder
        sourceInstanceBuilder(*m_sourceClass),
        structBuilder(*m_structClass),
        structStructBuilder(*m_structStructClass),
        structStructArrayMemberBuilder1(*m_structStructClass),
        structStructArrayMemberBuilder2(*m_structStructClass);

    SetStructStructProperties(structStructBuilder, nullptr, nullptr, u8"α β γ δ ε ζ η θ ι κ λ μ ν ξ ο π ρ ς σ τ υ φ χ ψ ω", nullptr, "EnumeratorB", nullptr, 1, {7});
    SetStructStructProperties(structStructArrayMemberBuilder1, 333333, 3.333333, nullptr, nullptr, nullptr, 2, 1, {9.0});
    SetStructStructProperties(structStructArrayMemberBuilder2, nullptr, nullptr, nullptr, nullptr, nullptr, 5, 0, { });
    SetStructProperties(structBuilder, nullptr, 1.111111, "1.111111", nullptr, nullptr, 7, 0, { }, &structStructBuilder, 2, { &structStructArrayMemberBuilder1, &structStructArrayMemberBuilder2 });
    sourceInstanceBuilder.EmplaceProperty("StructProperty", structBuilder);

    auto const sourceInstance = sourceInstanceBuilder.BuildInstance();

    auto expectedJsonValue2 = Json::Value::From(u8R"(
        {
           "DoubleProperty" : 1.111111
        })");

    auto expectedJsonValue3 = Json::Value::From(u8R"(
        {
           "StringProperty" : "1.111111"
        })");

    auto expectedJsonValue5 = Json::Value::From(u8R"(
        {
           "LooseEnumerationProperty" : 7
        })");

    auto expectedJsonValue8 = Json::Value::From(u8R"(
        {
           "StringProperty" : "α β γ δ ε ζ η θ ι κ λ μ ν ξ ο π ρ ς σ τ υ φ χ ψ ω"
        })");

    auto expectedJsonValue9 = Json::Value::From(u8R"(
        {
           "StrictEnumerationProperty" : "EnumeratorB"
        })");

    auto expectedJsonValue11 = Json::Value::From(u8R"(
        {
           "DoubleProperty" : 1.111111,
           "StringProperty" : "1.111111",
           "LooseEnumerationProperty" : 7,
           "StrictEnumerationProperty" : "EnumeratorB"
        })");

    auto expectedJsonValue12 = Json::Value::From(u8R"(
        {
           "DoubleProperty" : 1.111111,
           "StringProperty" : "α β γ δ ε ζ η θ ι κ λ μ ν ξ ο π ρ ς σ τ υ φ χ ψ ω",
           "LooseEnumerationProperty" : 7,
           "StrictEnumerationProperty" : "EnumeratorB"
        })");

    // Act

    Utf8String structPropertyBaseAccessString("StructProperty");
    Utf8String structStructPropertyBaseAccessString("StructProperty.StructStructProperty");

    Json::Value actualJsonValue1;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValue(actualJsonValue1, *m_structIntProperty, *sourceInstance, structPropertyBaseAccessString.c_str()));

    Json::Value actualJsonValue2;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValue(actualJsonValue2, *m_structDoubleProperty, *sourceInstance, structPropertyBaseAccessString.c_str()));

    Json::Value actualJsonValue3;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValue(actualJsonValue3, *m_structStringProperty, *sourceInstance, structPropertyBaseAccessString.c_str()));

    Json::Value actualJsonValue4;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValue(actualJsonValue4, *m_structStrictEnumerationProperty, *sourceInstance, structPropertyBaseAccessString.c_str()));

    Json::Value actualJsonValue5;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValue(actualJsonValue5, *m_structLooseEnumerationProperty, *sourceInstance, structPropertyBaseAccessString.c_str()));

    Json::Value actualJsonValue6;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValue(actualJsonValue6, *m_structStructIntProperty, *sourceInstance, structStructPropertyBaseAccessString.c_str()));

    Json::Value actualJsonValue7;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValue(actualJsonValue7, *m_structStructDoubleProperty, *sourceInstance, structStructPropertyBaseAccessString.c_str()));

    Json::Value actualJsonValue8;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValue(actualJsonValue8, *m_structStructStringProperty, *sourceInstance, structStructPropertyBaseAccessString.c_str()));

    Json::Value actualJsonValue9;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValue(actualJsonValue9, *m_structStructStrictEnumerationProperty, *sourceInstance, structStructPropertyBaseAccessString.c_str()));

    Json::Value actualJsonValue10;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValue(actualJsonValue10, *m_structStructLooseEnumerationProperty, *sourceInstance, structStructPropertyBaseAccessString.c_str()));

    Json::Value actualJsonValue11;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValue(actualJsonValue11, *m_structDoubleProperty, *sourceInstance, structPropertyBaseAccessString.c_str()));
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValue(actualJsonValue11, *m_structStringProperty, *sourceInstance, structPropertyBaseAccessString.c_str()));
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValue(actualJsonValue11, *m_structLooseEnumerationProperty, *sourceInstance, structPropertyBaseAccessString.c_str()));
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValue(actualJsonValue11, *m_structStructStrictEnumerationProperty, *sourceInstance, structStructPropertyBaseAccessString.c_str()));

    Json::Value actualJsonValue12;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValue(actualJsonValue12, *m_structDoubleProperty, *sourceInstance, structPropertyBaseAccessString.c_str()));
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValue(actualJsonValue12, *m_structStringProperty, *sourceInstance, structPropertyBaseAccessString.c_str()));
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValue(actualJsonValue12, *m_structLooseEnumerationProperty, *sourceInstance, structPropertyBaseAccessString.c_str()));
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValue(actualJsonValue12, *m_structStructStringProperty, *sourceInstance, structStructPropertyBaseAccessString.c_str()));
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValue(actualJsonValue12, *m_structStructStrictEnumerationProperty, *sourceInstance, structStructPropertyBaseAccessString.c_str()));

    // Assert

    Json::Value nullValue;

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(nullValue, actualJsonValue1)) <<
        "Expected:\n" + nullValue.ToString() + "\nBut was:\n" + actualJsonValue1.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue2, actualJsonValue2)) <<
        "Expected:\n" + expectedJsonValue2.ToString() + "\nBut was:\n" + actualJsonValue2.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue3, actualJsonValue3)) <<
        "Expected:\n" + expectedJsonValue3.ToString() + "\nBut was:\n" + actualJsonValue3.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(nullValue, actualJsonValue4)) <<
        "Expected:\n" + nullValue.ToString() + "\nBut was:\n" + actualJsonValue4.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue5, actualJsonValue5)) <<
        "Expected:\n" + expectedJsonValue5.ToString() + "\nBut was:\n" + actualJsonValue5.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(nullValue, actualJsonValue6)) <<
        "Expected:\n" + nullValue.ToString() + "\nBut was:\n" + actualJsonValue6.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(nullValue, actualJsonValue7)) <<
        "Expected:\n" + nullValue.ToString() + "\nBut was:\n" + actualJsonValue7.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue8, actualJsonValue8)) <<
        "Expected:\n" + expectedJsonValue8.ToString() + "\nBut was:\n" + actualJsonValue8.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue9, actualJsonValue9)) <<
        "Expected:\n" + expectedJsonValue9.ToString() + "\nBut was:\n" + actualJsonValue9.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(nullValue, actualJsonValue10)) <<
        "Expected:\n" + nullValue.ToString() + "\nBut was:\n" + actualJsonValue10.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue11, actualJsonValue11)) <<
        "Expected:\n" + expectedJsonValue11.ToString() + "\nBut was:\n" + actualJsonValue11.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue12, actualJsonValue12)) <<
        "Expected:\n" + expectedJsonValue12.ToString() + "\nBut was:\n" + actualJsonValue12.ToString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECInstanceJsonWriterTests, WritePrimitiveValueForPresentationTest)
    {
    // Arrange

    ECInstanceBuilder
        sourceInstanceBuilder(*m_sourceClass),
        structBuilder(*m_structClass),
        structStructBuilder(*m_structStructClass),
        structStructArrayMemberBuilder1(*m_structStructClass),
        structStructArrayMemberBuilder2(*m_structStructClass);

    SetStructStructProperties(structStructBuilder, nullptr, 3.14, u8"π", nullptr, "EnumeratorB", nullptr, 1, {7});
    SetStructStructProperties(structStructArrayMemberBuilder1, 333333, 3.333333, nullptr, nullptr, nullptr, 2, 1, {9.0});
    SetStructStructProperties(structStructArrayMemberBuilder2, nullptr, nullptr, nullptr, nullptr, nullptr, 5, 0, { });
    SetStructProperties(structBuilder, nullptr, 1.111111, "1.111111", nullptr, nullptr, 7, 0, { }, &structStructBuilder, 2, { &structStructArrayMemberBuilder1, &structStructArrayMemberBuilder2 });
    sourceInstanceBuilder.EmplaceProperty("StructProperty", structBuilder);

    auto const sourceInstance = sourceInstanceBuilder.BuildInstance();

    auto expectedJsonValue2 = Json::Value::From(u8R"(
        {
           "DoubleProperty" : {
              "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
              "formattedValue" : "4.206 m³",
              "rawValue" : 1.111111
           }
        })");

    auto expectedJsonValue3 = Json::Value::From(u8R"(
        {
           "StringProperty" : "1.111111"
        })");

    auto expectedJsonValue5 = Json::Value::From(u8R"(
        {
           "LooseEnumerationProperty" : 7
        })");

    auto expectedJsonValue7 = Json::Value::From(u8R"(
        {
           "DoubleProperty" : {
              "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
              "formattedValue" : "11.886 m³",
              "rawValue" : 3.14
           }
        })");

    auto expectedJsonValue8 = Json::Value::From(u8R"(
        {
           "StringProperty" : "π"
        })");

    auto expectedJsonValue9 = Json::Value::From(u8R"(
        {
           "StrictEnumerationProperty" : "EnumeratorB"
        })");

    auto expectedJsonValue11 = Json::Value::From(u8R"(
        {
           "DoubleProperty" : {
              "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
              "formattedValue" : "4.206 m³",
              "rawValue" : 1.111111
           },
           "StringProperty" : "1.111111",
           "LooseEnumerationProperty" : 7,
           "StrictEnumerationProperty" : "EnumeratorB"
        })");

    auto expectedJsonValue12 = Json::Value::From(u8R"(
        {
           "DoubleProperty" : {
              "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
              "formattedValue" : "11.886 m³",
              "rawValue" : 3.14
           },
           "StringProperty" : "π",
           "LooseEnumerationProperty" : 7,
           "StrictEnumerationProperty" : "EnumeratorB"
        })");

    // Act

    Utf8String structPropertyBaseAccessString("StructProperty");
    Utf8String structStructPropertyBaseAccessString("StructProperty.StructStructProperty");

    Json::Value actualJsonValue1;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValueForPresentation(actualJsonValue1, *m_structIntProperty, *sourceInstance, structPropertyBaseAccessString.c_str()));

    Json::Value actualJsonValue2;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValueForPresentation(actualJsonValue2, *m_structDoubleProperty, *sourceInstance, structPropertyBaseAccessString.c_str()));

    Json::Value actualJsonValue3;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValueForPresentation(actualJsonValue3, *m_structStringProperty, *sourceInstance, structPropertyBaseAccessString.c_str()));

    Json::Value actualJsonValue4;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValueForPresentation(actualJsonValue4, *m_structStrictEnumerationProperty, *sourceInstance, structPropertyBaseAccessString.c_str()));

    Json::Value actualJsonValue5;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValueForPresentation(actualJsonValue5, *m_structLooseEnumerationProperty, *sourceInstance, structPropertyBaseAccessString.c_str()));

    Json::Value actualJsonValue6;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValueForPresentation(actualJsonValue6, *m_structStructIntProperty, *sourceInstance, structStructPropertyBaseAccessString.c_str()));

    Json::Value actualJsonValue7;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValueForPresentation(actualJsonValue7, *m_structStructDoubleProperty, *sourceInstance, structStructPropertyBaseAccessString.c_str()));

    Json::Value actualJsonValue8;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValueForPresentation(actualJsonValue8, *m_structStructStringProperty, *sourceInstance, structStructPropertyBaseAccessString.c_str()));

    Json::Value actualJsonValue9;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValueForPresentation(actualJsonValue9, *m_structStructStrictEnumerationProperty, *sourceInstance, structStructPropertyBaseAccessString.c_str()));

    Json::Value actualJsonValue10;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValueForPresentation(actualJsonValue10, *m_structStructLooseEnumerationProperty, *sourceInstance, structStructPropertyBaseAccessString.c_str()));

    Json::Value actualJsonValue11;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValueForPresentation(actualJsonValue11, *m_structDoubleProperty, *sourceInstance, structPropertyBaseAccessString.c_str()));
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValueForPresentation(actualJsonValue11, *m_structStringProperty, *sourceInstance, structPropertyBaseAccessString.c_str()));
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValueForPresentation(actualJsonValue11, *m_structLooseEnumerationProperty, *sourceInstance, structPropertyBaseAccessString.c_str()));
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValueForPresentation(actualJsonValue11, *m_structStructStrictEnumerationProperty, *sourceInstance, structStructPropertyBaseAccessString.c_str()));

    Json::Value actualJsonValue12;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValueForPresentation(actualJsonValue12, *m_structDoubleProperty, *sourceInstance, structPropertyBaseAccessString.c_str()));
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValueForPresentation(actualJsonValue12, *m_structStringProperty, *sourceInstance, structPropertyBaseAccessString.c_str()));
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValueForPresentation(actualJsonValue12, *m_structLooseEnumerationProperty, *sourceInstance, structPropertyBaseAccessString.c_str()));
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValueForPresentation(actualJsonValue12, *m_structStructDoubleProperty, *sourceInstance, structStructPropertyBaseAccessString.c_str()));
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValueForPresentation(actualJsonValue12, *m_structStructStringProperty, *sourceInstance, structStructPropertyBaseAccessString.c_str()));
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePrimitiveValueForPresentation(actualJsonValue12, *m_structStructStrictEnumerationProperty, *sourceInstance, structStructPropertyBaseAccessString.c_str()));

    // Assert

    Json::Value nullValue;

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(nullValue, actualJsonValue1)) <<
        "Expected:\n" + nullValue.ToString() + "\nBut was:\n" + actualJsonValue1.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue2, actualJsonValue2)) <<
        "Expected:\n" + expectedJsonValue2.ToString() + "\nBut was:\n" + actualJsonValue2.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue3, actualJsonValue3)) <<
        "Expected:\n" + expectedJsonValue3.ToString() + "\nBut was:\n" + actualJsonValue3.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(nullValue, actualJsonValue4)) <<
        "Expected:\n" + nullValue.ToString() + "\nBut was:\n" + actualJsonValue4.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue5, actualJsonValue5)) <<
        "Expected:\n" + expectedJsonValue5.ToString() + "\nBut was:\n" + actualJsonValue5.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(nullValue, actualJsonValue6)) <<
        "Expected:\n" + nullValue.ToString() + "\nBut was:\n" + actualJsonValue6.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue7, actualJsonValue7)) <<
        "Expected:\n" + expectedJsonValue7.ToString() + "\nBut was:\n" + actualJsonValue7.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue8, actualJsonValue8)) <<
        "Expected:\n" + expectedJsonValue8.ToString() + "\nBut was:\n" + actualJsonValue8.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue9, actualJsonValue9)) <<
        "Expected:\n" + expectedJsonValue9.ToString() + "\nBut was:\n" + actualJsonValue9.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(nullValue, actualJsonValue10)) <<
        "Expected:\n" + nullValue.ToString() + "\nBut was:\n" + actualJsonValue10.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue11, actualJsonValue11)) <<
        "Expected:\n" + expectedJsonValue11.ToString() + "\nBut was:\n" + actualJsonValue11.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue12, actualJsonValue12)) <<
        "Expected:\n" + expectedJsonValue12.ToString() + "\nBut was:\n" + actualJsonValue12.ToString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECInstanceJsonWriterTests, WriteInstanceToJsonTest)
    {
    // Arrange

    ECInstanceBuilder
        sourceInstanceBuilder(*m_sourceClass),
        targetInstanceBuilder(*m_targetClass),
        structBuilder(*m_structClass),
        structStructBuilder(*m_structStructClass),
        structStructArrayMemberBuilder1(*m_structStructClass),
        structStructArrayMemberBuilder2(*m_structStructClass);

    SetStructStructProperties(structStructBuilder, nullptr, 3.14, "Pi 3 digits", nullptr, nullptr, 1, 4, {1.0, 2.0, 4.0, 9.0});
    SetStructStructProperties(structStructArrayMemberBuilder1, +1, 3.1415926535897932384626433832795028841971, u8"π 41 digits", nullptr, "EnumeratorB", 2, 2, {16, 25});
    SetStructStructProperties(structStructArrayMemberBuilder2, -1, 2.7182818284590452353602874713526624977572, "e 41 digits", nullptr, "EnumeratorB", 2, 0, {});
    SetStructProperties(structBuilder, nullptr, 2.0, "2.0", nullptr, "EnumeratorA", 1, 0, {}, &structStructBuilder, 2, { &structStructArrayMemberBuilder1, &structStructArrayMemberBuilder2 });
    sourceInstanceBuilder.EmplaceProperty("StructProperty", structBuilder);

    auto const sourceInstance = sourceInstanceBuilder.BuildInstance();
    sourceInstance->SetInstanceId("sourceInstanceId");
    auto const targetInstance = targetInstanceBuilder.BuildInstance();
    targetInstance->SetInstanceId("targetInstanceId");

    auto expectedJsonValue1 = Json::Value::From(u8R"(
        {
           "SourceClass" : {
              "NavigationProperty" : {
                 "id" : "0xd",
                 "relClassName" : "Schema.RelationshipClass"
              },
              "StructProperty" : {
                 "DoubleProperty" : 2.0,
                 "StringProperty" : "2.0",
                 "StrictEnumerationProperty" : "EnumeratorA",
                 "LooseEnumerationProperty" : 1,
                 "StructStructProperty" : {
                    "DoubleProperty" : 3.14,
                    "StringProperty" : "Pi 3 digits",
                    "LooseEnumerationProperty" : 1,
                    "DoubleArrayProperty" : [ 1, 2, 4, 9 ]
                 },
                 "StructStructArrayProperty" : [
                    {
                       "IntProperty" : 1,
                       "DoubleProperty" : 3.1415926535897932384626433832795028841971,
                       "StringProperty" : "π 41 digits",
                       "StrictEnumerationProperty" : "EnumeratorB",
                       "LooseEnumerationProperty" : 2,
                       "DoubleArrayProperty" : [ 16, 25 ]
                    },
                    {
                       "IntProperty" : -1,
                       "DoubleProperty" : 2.7182818284590452353602874713526624977572,
                       "StringProperty" : "e 41 digits",
                       "StrictEnumerationProperty" : "EnumeratorB",
                       "LooseEnumerationProperty" : 2
                    }
                 ]
              }
           },
           "ecClass" : "SourceClass",
           "ecSchema" : "Schema.01.00",
           "instanceId" : "sourceInstanceId"
        })");

    auto expectedJsonValue2 = Json::Value::From(u8R"(
        {
           "MyClass" : {
              "NavigationProperty" : {
                 "id" : "0xd",
                 "relClassName" : "Schema.RelationshipClass"
              },
              "StructProperty" : {
                 "IntProperty" : null,
                 "DoubleProperty" : 2.0,
                 "StringProperty" : "2.0",
                 "GeometryProperty": null,
                 "StrictEnumerationProperty" : "EnumeratorA",
                 "LooseEnumerationProperty" : 1,
                 "DoubleArrayProperty" : [],
                 "StructStructProperty" : {
                    "IntProperty" : null,
                    "DoubleProperty" : 3.14,
                    "StringProperty" : "Pi 3 digits",
                    "GeometryProperty": null,
                    "StrictEnumerationProperty" : null,
                    "LooseEnumerationProperty" : 1,
                    "DoubleArrayProperty" : [ 1, 2, 4, 9 ]
                 },
                 "StructStructArrayProperty" : [
                    {
                       "IntProperty" : 1,
                       "DoubleProperty" : 3.1415926535897932384626433832795028841971,
                       "StringProperty" : "π 41 digits",
                       "GeometryProperty": null,
                       "StrictEnumerationProperty" : "EnumeratorB",
                       "LooseEnumerationProperty" : 2,
                       "DoubleArrayProperty" : [ 16, 25 ]
                    },
                    {
                       "IntProperty" : -1,
                       "DoubleProperty" : 2.7182818284590452353602874713526624977572,
                       "StringProperty" : "e 41 digits",
                       "GeometryProperty": null,
                       "StrictEnumerationProperty" : "EnumeratorB",
                       "LooseEnumerationProperty" : 2,
                       "DoubleArrayProperty" : []
                    }
                 ]
              }
           },
           "ecClass" : "SourceClass",
           "ecSchema" : "Schema.01.00"
        })");

    auto expectedJsonValue3 = Json::Value::From(u8R"(
        {
           "SourceClass" : {
              "NavigationProperty" : {
                 "id" : "0xe"
              },
              "StructProperty" : {
                 "IntProperty" : null,
                 "DoubleProperty" : 2.0,
                 "StringProperty" : "2.0",
                 "GeometryProperty": null,
                 "StrictEnumerationProperty" : "EnumeratorA",
                 "LooseEnumerationProperty" : 1,
                 "DoubleArrayProperty" : [],
                 "StructStructProperty" : {
                    "DoubleArrayProperty" : [ 1, 2, 4, 9 ],
                    "DoubleProperty" : 3.1400000000000001,
                    "IntProperty" : null,
                    "LooseEnumerationProperty" : 1,
                    "StrictEnumerationProperty" : null,
                    "StringProperty" : "Pi 3 digits",
                    "GeometryProperty": null
                 },
                 "StructStructArrayProperty" : [
                    {
                       "IntProperty" : 1,
                       "DoubleProperty" : 3.1415926535897931,
                       "StringProperty" : "π 41 digits",
                       "GeometryProperty": null,
                       "StrictEnumerationProperty" : "EnumeratorB",
                       "LooseEnumerationProperty" : 2,
                       "DoubleArrayProperty" : [ 16, 25 ]
                    },
                    {
                       "IntProperty" : -1,
                       "DoubleProperty" : 2.7182818284590451,
                       "StringProperty" : "e 41 digits",
                       "GeometryProperty": null,
                       "StrictEnumerationProperty" : "EnumeratorB",
                       "LooseEnumerationProperty" : 2,
                       "DoubleArrayProperty" : []
                    }
                 ]
              }
           },
           "ecClass" : "SourceClass",
           "ecSchema" : "Schema.01.00",
           "instanceId" : "sourceInstanceId"
        })");

    auto expectedJsonValue4 = Json::Value::From(u8R"(
        {
           "SourceClass" : {
              "NavigationProperty" : {
                 "id" : "0xf",
                 "relClassName" : "Schema.RelationshipClass"
              },
              "StructProperty" : {
                 "DoubleProperty" : 2.0,
                 "StringProperty" : "2.0",
                 "StrictEnumerationProperty" : "EnumeratorA",
                 "LooseEnumerationProperty" : 1,
                 "StructStructProperty" : {
                    "DoubleProperty" : 3.14,
                    "StringProperty" : "Pi 3 digits",
                    "LooseEnumerationProperty" : 1,
                    "DoubleArrayProperty" : [ 1, 2, 4, 9 ]
                 },
                 "StructStructArrayProperty" : [
                    {
                       "IntProperty" : 1,
                       "DoubleProperty" : 3.1415926535897931,
                       "StringProperty" : "π 41 digits",
                       "StrictEnumerationProperty" : "EnumeratorB",
                       "LooseEnumerationProperty" : 2,
                       "DoubleArrayProperty" : [ 16, 25 ]
                    },
                    {
                       "IntProperty" : -1,
                       "DoubleProperty" : 2.7182818284590451,
                       "StringProperty" : "e 41 digits",
                       "StrictEnumerationProperty" : "EnumeratorB",
                       "LooseEnumerationProperty" : 2
                    }
                 ]
              }
           },
           "ecClass" : "SourceClass",
           "ecSchema" : "Schema.01.00"
        })");

    auto expectedJsonValue5 = Json::Value::From(u8R"(
        {
           "AnotherSourceClass" : {
              "NavigationProperty" : {
                 "id" : "0xf",
                 "relClassName" : "Schema.RelationshipClass"
              },
              "StructProperty" : {
                 "DoubleProperty" : 2.0,
                 "LooseEnumerationProperty" : 1,
                 "StrictEnumerationProperty" : "EnumeratorA",
                 "StringProperty" : "2.0",
                 "StructStructArrayProperty" : [
                    {
                       "DoubleArrayProperty" : [ 16, 25 ],
                       "DoubleProperty" : 3.1415926535897931,
                       "IntProperty" : 1,
                       "LooseEnumerationProperty" : 2,
                       "StrictEnumerationProperty" : "EnumeratorB",
                       "StringProperty" : "π 41 digits"
                    },
                    {
                       "DoubleProperty" : 2.7182818284590451,
                       "IntProperty" : -1,
                       "LooseEnumerationProperty" : 2,
                       "StrictEnumerationProperty" : "EnumeratorB",
                       "StringProperty" : "e 41 digits"
                    }
                 ],
                 "StructStructProperty" : {
                    "DoubleArrayProperty" : [ 1, 2, 4, 9 ],
                    "DoubleProperty" : 3.1400000000000001,
                    "LooseEnumerationProperty" : 1,
                    "StringProperty" : "Pi 3 digits"
                 }
              }
           },
           "SourceClass" : {
              "NavigationProperty" : {
                 "id" : "0xf",
                 "relClassName" : "Schema.RelationshipClass"
              },
              "StructProperty" : {
                 "DoubleArrayProperty" : [],
                 "DoubleProperty" : 2.0,
                 "IntProperty" : null,
                 "LooseEnumerationProperty" : 1,
                 "StrictEnumerationProperty" : "EnumeratorA",
                 "StringProperty" : "2.0",
                 "GeometryProperty": null,
                 "StructStructArrayProperty" : [
                    {
                       "DoubleArrayProperty" : [ 16, 25 ],
                       "DoubleProperty" : 3.1415926535897931,
                       "IntProperty" : 1,
                       "LooseEnumerationProperty" : 2,
                       "StrictEnumerationProperty" : "EnumeratorB",
                       "StringProperty" : "π 41 digits",
                       "GeometryProperty": null
                    },
                    {
                       "DoubleArrayProperty" : [],
                       "DoubleProperty" : 2.7182818284590451,
                       "IntProperty" : -1,
                       "LooseEnumerationProperty" : 2,
                       "StrictEnumerationProperty" : "EnumeratorB",
                       "StringProperty" : "e 41 digits",
                       "GeometryProperty": null
                    }
                 ],
                 "StructStructProperty" : {
                    "DoubleArrayProperty" : [ 1, 2, 4, 9 ],
                    "DoubleProperty" : 3.1400000000000001,
                    "IntProperty" : null,
                    "LooseEnumerationProperty" : 1,
                    "StrictEnumerationProperty" : null,
                    "StringProperty" : "Pi 3 digits",
                    "GeometryProperty": null
                 }
              }
           },
           "TargetClass" : {},
           "ecClass" : "TargetClass",
           "ecSchema" : "Schema.01.00",
           "instanceId" : "targetInstanceId"
        })");

    auto expectedJsonValue6 = Json::Value::From(u8R"(
        {
           "ecClass" : "SourceClass",
           "ecSchema" : "Schema.01.00"
        })");

    // Act

    ECClassLocater classLocator(*m_schema.get());

    sourceInstance->SetValue("NavigationProperty", ECValue(BeInt64Id(13), m_relationshipClass));

    Json::Value actualJsonValue1;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteInstanceToJson(actualJsonValue1, *sourceInstance, nullptr, true));

    Json::Value actualJsonValue2;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteInstanceToJson(actualJsonValue2, *sourceInstance, "MyClass", false, true));

    sourceInstance->SetValue("NavigationProperty", ECValue(BeInt64Id(14)));

    Json::Value actualJsonValue3;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteInstanceToJson(actualJsonValue3, *sourceInstance, nullptr, true, true));

    sourceInstance->SetValue("NavigationProperty", ECValue(BeInt64Id(15), m_relationshipClass->GetId()));

    Json::Value actualJsonValue4;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteInstanceToJson(actualJsonValue4, *sourceInstance, nullptr, false, false, &classLocator));

    Json::Value actualJsonValue5;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteInstanceToJson(actualJsonValue5, *sourceInstance, nullptr, true, true, &classLocator));
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteInstanceToJson(actualJsonValue5, *sourceInstance, "AnotherSourceClass", false, false, &classLocator));
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteInstanceToJson(actualJsonValue5, *targetInstance, nullptr, true, false, &classLocator));

    Json::Value actualJsonValue6;
        {
        DISABLE_ASSERTS
        ASSERT_EQ(BSIERROR, JsonEcInstanceWriter::WriteInstanceToJson(actualJsonValue6, *sourceInstance, nullptr, false));
        }

    // Assert

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue1, actualJsonValue1)) <<
        "Expected:\n" + expectedJsonValue1.ToString() + "\nBut was:\n" + actualJsonValue1.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue2, actualJsonValue2)) <<
        "Expected:\n" + expectedJsonValue2.ToString() + "\nBut was:\n" + actualJsonValue2.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue3, actualJsonValue3)) <<
        "Expected:\n" + expectedJsonValue3.ToString() + "\nBut was:\n" + actualJsonValue3.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue4, actualJsonValue4)) <<
        "Expected:\n" + expectedJsonValue4.ToString() + "\nBut was:\n" + actualJsonValue4.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue5, actualJsonValue5)) <<
        "Expected:\n" + expectedJsonValue5.ToString() + "\nBut was:\n" + actualJsonValue5.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue6, actualJsonValue6)) <<
        "Expected:\n" + expectedJsonValue6.ToString() + "\nBut was:\n" + actualJsonValue6.ToString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECInstanceJsonWriterTests, WriteInstanceToPresentationJsonTest)
    {
    // Arrange

    ECInstanceBuilder
        sourceInstanceBuilder(*m_sourceClass),
        targetInstanceBuilder(*m_targetClass),
        structBuilder(*m_structClass),
        structStructBuilder(*m_structStructClass),
        structStructArrayMemberBuilder1(*m_structStructClass),
        structStructArrayMemberBuilder2(*m_structStructClass);

    SetStructStructProperties(structStructBuilder, nullptr, 3.14, "Pi 3 digits", nullptr, nullptr, 1, 1, { 7.0 });
    SetStructStructProperties(structStructArrayMemberBuilder1, +1, 3.1415926535897932384626433832795028841971, u8"π 41 digits", nullptr, "EnumeratorB", 2, 1, { 25 });
    SetStructStructProperties(structStructArrayMemberBuilder2, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0, { });
    SetStructProperties(structBuilder, nullptr, 2.0, "2.0", nullptr, "EnumeratorA", 1, 0, { }, &structStructBuilder, 2, { &structStructArrayMemberBuilder1, &structStructArrayMemberBuilder2 });
    sourceInstanceBuilder.EmplaceProperty("StructProperty", structBuilder);

    auto const sourceInstance = sourceInstanceBuilder.BuildInstance();
    sourceInstance->SetInstanceId("sourceInstanceId");
    auto const targetInstance = targetInstanceBuilder.BuildInstance();
    targetInstance->SetInstanceId("targetInstanceId");

    auto expectedJsonValue1 = Json::Value::From(u8R"*(
        {
           "SourceClass" : {
              "NavigationProperty" : {
                 "id" : "0xd",
                 "relClassName" : "Schema.RelationshipClass"
              },
              "StructProperty" : {
                 "DoubleProperty" : {
                    "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
                    "formattedValue" : "7.571 m³",
                    "rawValue" : 2.0
                 },
                 "StringProperty" : "2.0",
                 "StrictEnumerationProperty" : "EnumeratorA",
                 "LooseEnumerationProperty" : 1,
                 "StructStructProperty" : {
                    "DoubleProperty" : {
                       "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
                       "formattedValue" : "11.886 m³",
                       "rawValue" : 3.14
                    },
                    "StringProperty" : "Pi 3 digits",
                    "LooseEnumerationProperty" : 1,
                    "DoubleArrayProperty" : [
                       {
                          "currentUnit" : "AngleDMS(2)",
                          "formattedValue":"401° 4' 13.64\"",
                          "rawValue" : 7
                       }
                    ]
                 },
                 "StructStructArrayProperty" : [
                    {
                       "IntProperty" : 1,
                       "DoubleProperty" : {
                          "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
                          "formattedValue" : "11.892 m³",
                          "rawValue" : 3.1415926535897932384626433832795028841971
                       },
                       "StringProperty" : "π 41 digits",
                       "StrictEnumerationProperty" : "EnumeratorB",
                       "LooseEnumerationProperty" : 2,
                       "DoubleArrayProperty" : [
                          {
                             "currentUnit" : "AngleDMS(2)",
                             "formattedValue":"1432° 23' 40.16\"",
                             "rawValue" : 25
                          }
                       ]
                    },
                    {}
                 ]
              }
           },
           "ecClass" : "SourceClass",
           "ecSchema" : "Schema.01.00",
           "instanceId" : "sourceInstanceId"
        })*");

    auto expectedJsonValue2 = Json::Value::From(u8R"*(
        {
           "SourceClass" : {
              "NavigationProperty" : {
                 "id" : "0xe"
              },
              "StructProperty" : {
                 "DoubleProperty" : {
                    "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
                    "formattedValue" : "7.571 m³",
                    "rawValue" : 2.0
                 },
                 "StringProperty" : "2.0",
                 "StrictEnumerationProperty" : "EnumeratorA",
                 "LooseEnumerationProperty" : 1,
                 "StructStructArrayProperty" : [
                    {
                       "IntProperty" : 1,
                       "DoubleProperty" : {
                          "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
                          "formattedValue" : "11.892 m³",
                          "rawValue" : 3.1415926535897932384626433832795028841971
                       },
                       "StringProperty" : "π 41 digits",
                       "StrictEnumerationProperty" : "EnumeratorB",
                       "LooseEnumerationProperty" : 2,
                       "DoubleArrayProperty" : [
                          {
                             "currentUnit" : "AngleDMS(2)",
                             "formattedValue" : "1432° 23' 40.16\"",
                             "rawValue" : 25
                          }
                       ]
                    },
                    {}
                 ],
                 "StructStructProperty" : {
                    "DoubleProperty" : {
                       "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
                       "formattedValue" : "11.886 m³",
                       "rawValue" : 3.14
                    },
                    "StringProperty" : "Pi 3 digits",
                    "LooseEnumerationProperty" : 1,
                    "DoubleArrayProperty" : [
                       {
                          "currentUnit" : "AngleDMS(2)",
                          "formattedValue" : "401° 4' 13.64\"",
                          "rawValue" : 7
                       }
                    ]
                 }
              }
           },
           "ecClass" : "SourceClass",
           "ecSchema" : "Schema.01.00"
        })*");

    auto expectedJsonValue3 = Json::Value::From(u8R"*(
        {
           "SourceClass" : {
              "NavigationProperty" : {
                 "id" : "0xf",
                 "relClassName" : "Schema.RelationshipClass"
              },
              "StructProperty" : {
                 "DoubleProperty" : {
                    "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
                    "formattedValue" : "7.571 m³",
                    "rawValue" : 2.0
                 },
                 "StringProperty" : "2.0",
                 "StrictEnumerationProperty" : "EnumeratorA",
                 "LooseEnumerationProperty" : 1,
                 "StructStructArrayProperty" : [
                    {
                       "IntProperty" : 1,
                       "DoubleProperty" : {
                          "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
                          "formattedValue" : "11.892 m³",
                          "rawValue" : 3.1415926535897932384626433832795028841971
                       },
                       "StringProperty" : "π 41 digits",
                       "StrictEnumerationProperty" : "EnumeratorB",
                       "LooseEnumerationProperty" : 2,
                       "DoubleArrayProperty" : [
                          {
                             "currentUnit" : "AngleDMS(2)",
                             "formattedValue" : "1432° 23' 40.16\"",
                             "rawValue" : 25
                          }
                       ]
                    },
                    {}
                 ],
                 "StructStructProperty" : {
                    "DoubleProperty" : {
                       "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
                       "formattedValue" : "11.886 m³",
                       "rawValue" : 3.14
                    },
                    "StringProperty" : "Pi 3 digits",
                    "LooseEnumerationProperty" : 1,
                    "DoubleArrayProperty" : [
                       {
                          "currentUnit" : "AngleDMS(2)",
                          "formattedValue" : "401° 4' 13.64\"",
                          "rawValue" : 7
                       }
                    ]
                 }
              }
           },
           "ecClass" : "SourceClass",
           "ecSchema" : "Schema.01.00"
        })*");

    auto expectedJsonValue4 = Json::Value::From(u8R"*(
        {
           "AnotherSourceClass" : {
              "NavigationProperty" : {
                 "id" : "0xf",
                 "relClassName" : "Schema.RelationshipClass"
              },
              "StructProperty" : {
                 "DoubleProperty" : {
                    "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
                    "formattedValue" : "7.571 m³",
                    "rawValue" : 2.0
                 },
                 "StringProperty" : "2.0",
                 "StrictEnumerationProperty" : "EnumeratorA",
                 "LooseEnumerationProperty" : 1,
                 "StructStructArrayProperty" : [
                    {
                       "IntProperty" : 1,
                       "DoubleProperty" : {
                          "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
                          "formattedValue" : "11.892 m³",
                          "rawValue" : 3.1415926535897932384626433832795028841971
                       },
                       "StringProperty" : "π 41 digits",
                       "StrictEnumerationProperty" : "EnumeratorB",
                       "LooseEnumerationProperty" : 2,
                       "DoubleArrayProperty" : [
                          {
                             "currentUnit" : "AngleDMS(2)",
                             "formattedValue" : "1432° 23' 40.16\"",
                             "rawValue" : 25
                          }
                       ]
                    },
                    {}
                 ],
                 "StructStructProperty" : {
                    "DoubleProperty" : {
                       "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
                       "formattedValue" : "11.886 m³",
                       "rawValue" : 3.14
                    },
                    "StringProperty" : "Pi 3 digits",
                    "LooseEnumerationProperty" : 1,
                    "DoubleArrayProperty" : [
                       {
                          "currentUnit" : "AngleDMS(2)",
                          "formattedValue" : "401° 4' 13.64\"",
                          "rawValue" : 7
                       }
                    ]
                 }
              }
           },
           "SourceClass" : {
              "NavigationProperty" : {
                 "id" : "0xf",
                 "relClassName" : "Schema.RelationshipClass"
              },
              "StructProperty" : {
                 "DoubleProperty" : {
                    "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
                    "formattedValue" : "7.571 m³",
                    "rawValue" : 2.0
                 },
                 "StringProperty" : "2.0",
                 "StrictEnumerationProperty" : "EnumeratorA",
                 "LooseEnumerationProperty" : 1,
                 "StructStructArrayProperty" : [
                    {
                       "IntProperty" : 1,
                       "DoubleProperty" : {
                          "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
                          "formattedValue" : "11.892 m³",
                          "rawValue" : 3.1415926535897932384626433832795028841971
                       },
                       "StringProperty" : "π 41 digits",
                       "StrictEnumerationProperty" : "EnumeratorB",
                       "LooseEnumerationProperty" : 2,
                       "DoubleArrayProperty" : [
                          {
                             "currentUnit" : "AngleDMS(2)",
                             "formattedValue" : "1432° 23' 40.16\"",
                             "rawValue" : 25
                          }
                       ]
                    },
                    {}
                 ],
                 "StructStructProperty" : {
                    "DoubleProperty" : {
                       "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
                       "formattedValue" : "11.886 m³",
                       "rawValue" : 3.14
                    },
                    "StringProperty" : "Pi 3 digits",
                    "LooseEnumerationProperty" : 1,
                    "DoubleArrayProperty" : [
                       {
                          "currentUnit" : "AngleDMS(2)",
                          "formattedValue" : "401° 4' 13.64\"",
                          "rawValue" : 7
                       }
                    ]
                 }
              }
           },
           "TargetClass" : {},
           "ecClass" : "TargetClass",
           "ecSchema" : "Schema.01.00",
           "instanceId" : "targetInstanceId"
        })*");

    auto expectedJsonValue5 = Json::Value::From(u8R"(
        {
           "ecClass" : "SourceClass",
           "ecSchema" : "Schema.01.00"
        })");

    // Act

    ECClassLocater classLocator(*m_schema.get());

    sourceInstance->SetValue("NavigationProperty", ECValue(BeInt64Id(13), m_relationshipClass));

    Json::Value actualJsonValue1;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteInstanceToPresentationJson(actualJsonValue1, *sourceInstance, nullptr, true));

    sourceInstance->SetValue("NavigationProperty", ECValue(BeInt64Id(14)));

    Json::Value actualJsonValue2;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteInstanceToPresentationJson(actualJsonValue2, *sourceInstance, nullptr, false));

    sourceInstance->SetValue("NavigationProperty", ECValue(BeInt64Id(15), m_relationshipClass->GetId()));

    Json::Value actualJsonValue3;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteInstanceToPresentationJson(actualJsonValue3, *sourceInstance, nullptr, false, &classLocator));

    Json::Value actualJsonValue4;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteInstanceToPresentationJson(actualJsonValue4, *sourceInstance, nullptr, true, &classLocator));
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteInstanceToPresentationJson(actualJsonValue4, *sourceInstance, "AnotherSourceClass", false, &classLocator));
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteInstanceToPresentationJson(actualJsonValue4, *targetInstance, nullptr, true, &classLocator));

    Json::Value actualJsonValue5;
        {
        DISABLE_ASSERTS
        ASSERT_EQ(BSIERROR, JsonEcInstanceWriter::WriteInstanceToPresentationJson(actualJsonValue5, *sourceInstance, nullptr, false));
        }

    // Assert

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue1, actualJsonValue1)) <<
        "Expected:\n" + expectedJsonValue1.ToString() + "\nBut was:\n" + actualJsonValue1.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue2, actualJsonValue2)) <<
        "Expected:\n" + expectedJsonValue2.ToString() + "\nBut was:\n" + actualJsonValue2.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue3, actualJsonValue3)) <<
        "Expected:\n" + expectedJsonValue3.ToString() + "\nBut was:\n" + actualJsonValue3.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue4, actualJsonValue4)) <<
        "Expected:\n" + expectedJsonValue4.ToString() + "\nBut was:\n" + actualJsonValue4.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue5, actualJsonValue5)) <<
        "Expected:\n" + expectedJsonValue5.ToString() + "\nBut was:\n" + actualJsonValue5.ToString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECInstanceJsonWriterTests, WriteInstanceToSchemaJsonTest)
    {
    // Arrange

    ECInstanceBuilder
        sourceInstanceBuilder(*m_sourceClass),
        targetInstanceBuilder(*m_targetClass),
        structBuilder(*m_structClass),
        structStructBuilder(*m_structStructClass),
        structStructArrayMemberBuilder1(*m_structStructClass),
        structStructArrayMemberBuilder2(*m_structStructClass);

    SetStructStructProperties(structStructBuilder, nullptr, nullptr, "No number", nullptr, nullptr, nullptr, 0, { });
    SetStructStructProperties(structStructArrayMemberBuilder1, nullptr, 3.1415926535897932384626433832795028841971, u8"π 41 digits", nullptr, "EnumeratorB", nullptr, 1, { 25 });
    SetStructStructProperties(structStructArrayMemberBuilder2, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0, { });
    SetStructProperties(structBuilder, nullptr, nullptr, nullptr, nullptr, "EnumeratorA", 1, 0, { }, &structStructBuilder, 2, { &structStructArrayMemberBuilder1, &structStructArrayMemberBuilder2 });
    sourceInstanceBuilder.EmplaceProperty("StructProperty", structBuilder);

    auto const sourceInstance = sourceInstanceBuilder.BuildInstance();
    auto const targetInstance = targetInstanceBuilder.BuildInstance();

    auto expectedJsonValue1 = Json::Value::From(u8R"*(
        {
           "NavigationProperty" : {
              "id" : "0xd"
           },
           "StructProperty" : {
              "StrictEnumerationProperty" : "EnumeratorA",
              "LooseEnumerationProperty" : 1,
              "StructStructArrayProperty" : [
                 {
                    "StringProperty" : "π 41 digits",
                    "DoubleProperty" : {
                       "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
                       "formattedValue" : "11.892 m³",
                       "rawValue" : 3.1415926535897932384626433832795028841971
                    },
                    "StrictEnumerationProperty" : "EnumeratorB",
                    "DoubleArrayProperty" : [
                       {
                          "currentUnit" : "AngleDMS(2)",
                          "formattedValue" : "1432° 23' 40.16\"",
                          "rawValue" : 25
                       }
                    ]
                 },
                 {}
              ],
              "StructStructProperty" : {
                 "StringProperty" : "No number"
              }
           },
           "className" : "Schema.SourceClass"
        })*");

    auto expectedJsonValue23 = Json::Value::From(u8R"*(
        {
           "NavigationProperty" : {
              "id" : "0xe",
              "relClassName" : "Schema.RelationshipClass"
           },
           "StructProperty" : {
              "StrictEnumerationProperty" : "EnumeratorA",
              "LooseEnumerationProperty" : 1,
              "StructStructArrayProperty" : [
                 {
                    "StringProperty" : "π 41 digits",
                    "DoubleProperty" : {
                       "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
                       "formattedValue" : "11.892 m³",
                       "rawValue" : 3.1415926535897932384626433832795028841971
                    },
                    "StrictEnumerationProperty" : "EnumeratorB",
                    "DoubleArrayProperty" : [
                       {
                          "currentUnit" : "AngleDMS(2)",
                          "formattedValue" : "1432° 23' 40.16\"",
                          "rawValue" : 25
                       }
                    ]
                 },
                 {}
              ],
              "StructStructProperty" : {
                 "StringProperty" : "No number"
              }
           },
           "className" : "Schema.SourceClass"
        })*");

    auto expectedJsonValue4 = Json::Value::From(u8R"*(
        {
           "NavigationProperty" : {
              "id" : "0xe",
              "relClassName" : "Schema.RelationshipClass"
           },
           "StructProperty" : {
              "StrictEnumerationProperty" : "EnumeratorA",
              "LooseEnumerationProperty" : 1,
              "StructStructArrayProperty" : [
                 {
                    "StringProperty" : "π 41 digits",
                    "DoubleProperty" : {
                       "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
                       "formattedValue" : "11.892 m³",
                       "rawValue" : 3.1415926535897932384626433832795028841971
                    },
                    "StrictEnumerationProperty" : "EnumeratorB",
                    "DoubleArrayProperty" : [
                       {
                          "currentUnit" : "AngleDMS(2)",
                          "formattedValue" : "1432° 23' 40.16\"",
                          "rawValue" : 25
                       }
                    ]
                 },
                 {}
              ],
              "StructStructProperty" : {
                 "StringProperty" : "No number"
              }
           },
           "className" : "Schema.TargetClass"
        })*");

    auto expectedJsonValue5 = Json::Value::From(u8R"*(
        {
           "NavigationProperty" : {
              "id" : "0xe"
           },
           "StructProperty" : {
              "StrictEnumerationProperty" : "EnumeratorA",
              "LooseEnumerationProperty" : 1,
              "StructStructArrayProperty" : [
                 {
                    "StringProperty" : "π 41 digits",
                    "DoubleProperty" : {
                       "currentUnit" : "DefaultRealU(3)[u:CUB_M]",
                       "formattedValue" : "11.892 m³",
                       "rawValue" : 3.1415926535897932384626433832795028841971
                    },
                    "StrictEnumerationProperty" : "EnumeratorB",
                    "DoubleArrayProperty" : [
                       {
                          "currentUnit" : "AngleDMS(2)",
                          "formattedValue" : "1432° 23' 40.16\"",
                          "rawValue" : 25
                       }
                    ]
                 },
                 {}
              ],
              "StructStructProperty" : {
                 "StringProperty" : "No number"
              }
           }
        })*");

    // Act

    ECClassLocater classLocator(*m_schema.get());

    sourceInstance->SetValue("NavigationProperty", ECValue(BeInt64Id(13)));

    Json::Value actualJsonValue1;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteInstanceToSchemaJson(actualJsonValue1, *sourceInstance));

    sourceInstance->SetValue("NavigationProperty", ECValue(BeInt64Id(14), m_relationshipClass));

    Json::Value actualJsonValue2;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteInstanceToSchemaJson(actualJsonValue2, *sourceInstance));

    sourceInstance->SetValue("NavigationProperty", ECValue(BeInt64Id(14), m_relationshipClass->GetId()));

    Json::Value actualJsonValue3;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteInstanceToSchemaJson(actualJsonValue3, *sourceInstance, &classLocator));

    Json::Value actualJsonValue4;
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteInstanceToSchemaJson(actualJsonValue4, *sourceInstance, &classLocator));
    ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteInstanceToSchemaJson(actualJsonValue4, *targetInstance, &classLocator));

    Json::Value actualJsonValue5;
        {
        DISABLE_ASSERTS
        ASSERT_EQ(BSIERROR, JsonEcInstanceWriter::WriteInstanceToSchemaJson(actualJsonValue5, *sourceInstance));
        }

    // Assert

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue1, actualJsonValue1)) <<
        "Expected:\n" + expectedJsonValue1.ToString() + "\nBut was:\n" + actualJsonValue1.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue23, actualJsonValue2)) <<
        "Expected:\n" + expectedJsonValue23.ToString() + "\nBut was:\n" + actualJsonValue2.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue23, actualJsonValue3)) <<
        "Expected:\n" + expectedJsonValue23.ToString() + "\nBut was:\n" + actualJsonValue3.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue4, actualJsonValue4)) <<
        "Expected:\n" + expectedJsonValue4.ToString() + "\nBut was:\n" + actualJsonValue4.ToString();

    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue5, actualJsonValue5)) <<
        "Expected:\n" + expectedJsonValue5.ToString() + "\nBut was:\n" + actualJsonValue5.ToString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECInstanceJsonWriterTests, WritePartialWritesIModelJsonGeometry)
   {
   // Arrange

   ECInstanceBuilder sourceInstanceBuilder(*m_sourceClass), structBuilder(*m_structClass);

   IGeometryPtr geom = IGeometry::Create(ICurvePrimitive::CreateArc(DEllipse3d::From(1, 1, 1,   0, 0, 2,   0, 3, 0,   0.0, Angle::TwoPi())));
   SetStructProperties(structBuilder, nullptr, nullptr, nullptr, geom, nullptr, nullptr, 0, { }, nullptr, 0, { });
   sourceInstanceBuilder.EmplaceProperty("StructProperty", structBuilder);

   auto const sourceInstance = sourceInstanceBuilder.BuildInstance();

   auto expectedJsonValue1 = Json::Value::From(u8R"(
      {
         "StructProperty": {
            "GeometryProperty": {
               "arc": {
               "center": [ 1, 1, 1 ],
               "sweepStartEnd": [ 0, 360 ],
               "vectorX": [ 0, 0, 2 ],
               "vectorY": [ 0, 3, 0 ]
               }
            }
         }
      })");

   // Act

   Json::Value actualJsonValue1;
   ASSERT_EQ(BSISUCCESS, JsonEcInstanceWriter::WritePartialInstanceToJson(actualJsonValue1, *sourceInstance, JsonEcInstanceWriter::MemberNameCasing::KeepOriginal, nullptr));

   // Assert

   ASSERT_TRUE(ECTestUtility::JsonDeepEqual(expectedJsonValue1, actualJsonValue1)) <<
       "Expected:\n" + expectedJsonValue1.ToString() + "\nBut was:\n" + actualJsonValue1.ToString();
   }

END_BENTLEY_ECN_TEST_NAMESPACE