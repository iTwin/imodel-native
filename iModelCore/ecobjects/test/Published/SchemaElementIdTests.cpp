/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/SchemaElementIdTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include "BeXml/BeXml.h"

#include <iostream>
#include <fstream>

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE


struct SchemaElementIdTests : ECTestFixture {};


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaElementIdTests, IdsAreNotSetAutomatically)
    {
    ECSchemaPtr schema;
    ECEntityClassP entityClass;
    ECStructClassP structClass;
    ECCustomAttributeClassP customAttributeClass;
    ECRelationshipClassP relationshipClass;
    ECEnumerationP enumeration;
    KindOfQuantityP koq;
    uint64_t id(42);

    ECSchema::CreateSchema (schema, "TestSchema", 5, 5);
    ASSERT_TRUE (schema.IsValid ());
    EXPECT_FALSE(schema->HasId()) << "Expected ECSchemaId to be unset when created via ECSchema::CreateSchema";
    schema->SetId(ECSchemaId(id));
    EXPECT_TRUE(schema->HasId()) << "Expected ECSchemaId to be set after calling ECSchema::SetId";
    EXPECT_EQ(id, schema->GetId().GetValue()) << "Expected ECSchemaId to be set to 42";

    //Create Domain Class
    schema->CreateEntityClass (entityClass, "EntityClass");
    ASSERT_TRUE (entityClass != NULL);
    EXPECT_FALSE(entityClass->HasId()) << "Expected ECClassId to be unset when creating via ECSchema::CreateEntityClass";
    entityClass->SetId(ECClassId(id));
    EXPECT_TRUE(entityClass->HasId()) << "Expected ECClassId to be set after calling ECClass::SetId";
    EXPECT_EQ(id, entityClass->GetId().GetValue()) << "Expected ECClassId to be set to 42";

    //Create Struct
    schema->CreateStructClass (structClass, "StructClass");
    ASSERT_TRUE (structClass != NULL);
    EXPECT_FALSE(structClass->HasId()) << "Expected ECClassId to be unset when creating via ECSchema::CreateStructClass";
    structClass->SetId(ECClassId(id));
    EXPECT_TRUE(structClass->HasId()) << "Expected ECClassId to be set on struct after calling ECClass::SetId";
    EXPECT_EQ(id, structClass->GetId().GetValue()) << "Expected ECClassId to be set to 42 on struct";

    //Create customAttributeClass
    schema->CreateCustomAttributeClass (customAttributeClass, "CustomAttributeClass");
    ASSERT_TRUE (customAttributeClass != NULL);
    EXPECT_FALSE(customAttributeClass->HasId()) << "Expected ECClassId to be unset when creating via ECSchema::CreateCustomAttributeClass";
    customAttributeClass->SetId(ECClassId(id));
    EXPECT_TRUE(customAttributeClass->HasId()) << "Expected ECClassId to be set on custom attribute after calling ECClass::SetId";
    EXPECT_EQ(id, customAttributeClass->GetId().GetValue()) << "Expected ECClassId to be set to 42 on custom attribute";

    //Create RelationshipClass
    schema->CreateRelationshipClass(relationshipClass, "RelationshipClass");
    ASSERT_TRUE(relationshipClass != NULL);
    relationshipClass->GetSource().AddClass(*entityClass);
    relationshipClass->GetTarget().AddClass(*entityClass);
    EXPECT_FALSE(relationshipClass->HasId()) << "Expected ECClassId to be unset when creating via ECSchema::CreateRelationshipClass";
    relationshipClass->SetId(ECClassId(id));
    EXPECT_TRUE(relationshipClass->HasId()) << "Expected ECClassId to be set on relationship after calling ECClass::SetId";
    EXPECT_EQ(id, relationshipClass->GetId().GetValue()) << "Expected ECClassId to be set to 42 on relationship";

    //Create Enumeration
    schema->CreateEnumeration(enumeration, "Enumeration", PrimitiveType::PRIMITIVETYPE_Integer);
    ASSERT_TRUE(enumeration != nullptr);
    EXPECT_FALSE(enumeration->HasId()) << "Expected ECEnumerationId to be unset when creating via ECSchema::CreateEnumeration";
    enumeration->SetId(ECEnumerationId(id));
    EXPECT_TRUE(enumeration->HasId()) << "Expected ECEnumerationId to be set after calling ECEnumeration::SetId";
    EXPECT_EQ(id, enumeration->GetId().GetValue()) << "Expected ECEnumerationId to be set to 42";

    //Create KindOfQuantity
    schema->CreateKindOfQuantity(koq, "KindOfQuantity");
    ASSERT_TRUE(koq != nullptr);
    EXPECT_FALSE(koq->HasId()) << "Expected KindOfQuantityId to be unset when creating via ECSchema::CreateKindOfQuantity";
    koq->SetId(KindOfQuantityId(id));
    EXPECT_TRUE(koq->HasId()) << "Expected KindOfQuantityId to be set after calling ECEnumeration::SetId";
    EXPECT_EQ(id, koq->GetId().GetValue()) << "Expected KindOfQuantityId to be set to 42";

    //Add Property of primitive type to entity class
    PrimitiveECPropertyP primitiveProperty;
    entityClass->CreatePrimitiveProperty(primitiveProperty, "PrimitiveProperty", PrimitiveType::PRIMITIVETYPE_Boolean);
    ASSERT_TRUE(primitiveProperty != nullptr);
    EXPECT_FALSE(primitiveProperty->HasId()) << "Expected ECPropertyId to be unset when creating  ECClass::CreatePrimitiveProperty";
    primitiveProperty->SetId(ECPropertyId(id));
    EXPECT_TRUE(primitiveProperty->HasId()) << "Expected ECPropertyId to be set on primitive after calling ECProperty::SetId";
    EXPECT_EQ(id, primitiveProperty->GetId().GetValue()) << "Expected ECPropertyId to be set to 42 on primitive";

    //Add Property of Array type to structClass
    ArrayECPropertyP MyArrayProp;
    structClass->CreateArrayProperty (MyArrayProp, "ArrayProperty");
    ASSERT_TRUE (MyArrayProp != NULL);
    EXPECT_FALSE(MyArrayProp->HasId()) << "Expected ECPropertyId to be unset when creating  ECClass::CreateArrayProperty";
    MyArrayProp->SetId(ECPropertyId(id));
    EXPECT_TRUE(MyArrayProp->HasId()) << "Expected ECPropertyId to be set on array after calling ECProperty::SetId";
    EXPECT_EQ(id, MyArrayProp->GetId().GetValue()) << "Expected ECPropertyId to be set to 42 on array";

    //Add Property Of Struct type to custom attribute
    StructECPropertyP PropertyOfCustomAttribute;
    customAttributeClass->CreateStructProperty (PropertyOfCustomAttribute, "PropertyOfCustomAttribute", *structClass);
    ASSERT_TRUE (PropertyOfCustomAttribute != NULL);
    EXPECT_FALSE(PropertyOfCustomAttribute->HasId()) << "Expected ECPropertyId to be unset when creating  ECClass::CreateStructProperty";
    PropertyOfCustomAttribute->SetId(ECPropertyId(id));
    EXPECT_TRUE(PropertyOfCustomAttribute->HasId()) << "Expected ECPropertyId to be set on struct after calling ECProperty::SetId";
    EXPECT_EQ(id, PropertyOfCustomAttribute->GetId().GetValue()) << "Expected ECPropertyId to be set to 42 on struct";

    //Add Navgation property to entity class
    NavigationECPropertyP navigationProperty;
    entityClass->CreateNavigationProperty(navigationProperty, "NavigationProperty", *relationshipClass, ECRelatedInstanceDirection::Forward);
    ASSERT_TRUE(navigationProperty != nullptr);
    EXPECT_FALSE(navigationProperty->HasId()) << "Expected ECPropertyId to be unset when creating  ECClass::CreateNavigationProperty";
    navigationProperty->SetId(ECPropertyId(id));
    EXPECT_TRUE(navigationProperty->HasId()) << "Expected ECPropertyId to be set on navigation property after calling ECProperty::SetId";
    EXPECT_EQ(id, navigationProperty->GetId().GetValue()) << "Expected ECPropertyId to be set to 42 on navigation property";
    }


END_BENTLEY_ECN_TEST_NAMESPACE
