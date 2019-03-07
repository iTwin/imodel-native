/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/SchemaParseUtilsTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include "../../PrivateApi/ECObjects/SchemaParseUtils.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaParseUtilsTest : ECTestFixture
    {
    };

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaParseUtilsTest, ParseBooleanXmlString)
    {
    bool bval;

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseBooleanXmlString(bval, "True"));
    EXPECT_TRUE(bval);
    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseBooleanXmlString(bval, "true"));
    EXPECT_TRUE(bval);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseBooleanXmlString(bval, "False"));
    EXPECT_FALSE(bval);
    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseBooleanXmlString(bval, "false"));
    EXPECT_FALSE(bval);

    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParseBooleanXmlString(bval, ""));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaParseUtilsTest, ParseCardinalityString__LegacyStrings)
    {
    uint32_t lowerLimit, upperLimit;

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseCardinalityString(lowerLimit, upperLimit, "1"));
    EXPECT_EQ(1, lowerLimit);
    EXPECT_EQ(1, upperLimit);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseCardinalityString(lowerLimit, upperLimit, "UNBOUNDED"));
    EXPECT_EQ(0, lowerLimit);
    EXPECT_EQ(INT_MAX, upperLimit);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseCardinalityString(lowerLimit, upperLimit, "Unbounded"));
    EXPECT_EQ(0, lowerLimit);
    EXPECT_EQ(INT_MAX, upperLimit);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseCardinalityString(lowerLimit, upperLimit, "unbounded"));
    EXPECT_EQ(0, lowerLimit);
    EXPECT_EQ(INT_MAX, upperLimit);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseCardinalityString(lowerLimit, upperLimit, "n"));
    EXPECT_EQ(0, lowerLimit);
    EXPECT_EQ(INT_MAX, upperLimit);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseCardinalityString(lowerLimit, upperLimit, "N"));
    EXPECT_EQ(0, lowerLimit);
    EXPECT_EQ(INT_MAX, upperLimit);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseCardinalityString(lowerLimit, upperLimit, "5"));
    EXPECT_EQ(0, lowerLimit);
    EXPECT_EQ(5, upperLimit);
    }

#define BEGRUDGINGLY_EXPECT_EQ EXPECT_EQ
//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaParseUtilsTest, ParseCardinalityString__NonLegacyStrings)
    {
    uint32_t lowerLimit, upperLimit;

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseCardinalityString(lowerLimit, upperLimit, "(3,12)"));
    EXPECT_EQ(3, lowerLimit);
    EXPECT_EQ(12, upperLimit);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseCardinalityString(lowerLimit, upperLimit, "(5, 13)"));
    EXPECT_EQ(5, lowerLimit);
    EXPECT_EQ(13, upperLimit);

    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParseCardinalityString(lowerLimit, upperLimit, "foo"));
    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParseCardinalityString(lowerLimit, upperLimit, "(foo,12)"));

    // Bugs that we are declaring as not a problem because it's not worth making potentially breaking changes to
    // fix them. Although these are semantically incorrect, we aren't touching the code with a 10 foot pole for
    // fear of breaking existing schemas, flooding the Delaware River, retriggering Y2K, and raising the dead.
    BEGRUDGINGLY_EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseCardinalityString(lowerLimit, upperLimit, ""));
    BEGRUDGINGLY_EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseCardinalityString(lowerLimit, upperLimit, "(5)"));
    BEGRUDGINGLY_EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseCardinalityString(lowerLimit, upperLimit, "(3..12)"));
    BEGRUDGINGLY_EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseCardinalityString(lowerLimit, upperLimit, "(3,foo)"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaParseUtilsTest, ParseContainerString)
    {
    CustomAttributeContainerType type;

    // Case sensistive
    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "Schema"));
    EXPECT_EQ(CustomAttributeContainerType::Schema, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "EntityClass"));
    EXPECT_EQ(CustomAttributeContainerType::EntityClass, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "CustomAttributeClass"));
    EXPECT_EQ(CustomAttributeContainerType::CustomAttributeClass, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "StructClass"));
    EXPECT_EQ(CustomAttributeContainerType::StructClass, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "RelationshipClass"));
    EXPECT_EQ(CustomAttributeContainerType::RelationshipClass, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "AnyClass"));
    EXPECT_EQ(CustomAttributeContainerType::AnyClass, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "PrimitiveProperty"));
    EXPECT_EQ(CustomAttributeContainerType::PrimitiveProperty, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "StructProperty"));
    EXPECT_EQ(CustomAttributeContainerType::StructProperty, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "ArrayProperty"));
    EXPECT_EQ(CustomAttributeContainerType::PrimitiveArrayProperty, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "StructArrayProperty"));
    EXPECT_EQ(CustomAttributeContainerType::StructArrayProperty, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "NavigationProperty"));
    EXPECT_EQ(CustomAttributeContainerType::NavigationProperty, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "AnyProperty"));
    EXPECT_EQ(CustomAttributeContainerType::AnyProperty, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "SourceRelationshipConstraint"));
    EXPECT_EQ(CustomAttributeContainerType::SourceRelationshipConstraint, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "TargetRelationshipConstraint"));
    EXPECT_EQ(CustomAttributeContainerType::TargetRelationshipConstraint, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "AnyRelationshipConstraint"));
    EXPECT_EQ(CustomAttributeContainerType::AnyRelationshipConstraint, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "Any"));
    EXPECT_EQ(CustomAttributeContainerType::Any, type);

    // Case insensitive
    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "scHEmA"));
    EXPECT_EQ(CustomAttributeContainerType::Schema, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "enTiTYClASS"));
    EXPECT_EQ(CustomAttributeContainerType::EntityClass, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "cUSToMATtRibUTECLASS"));
    EXPECT_EQ(CustomAttributeContainerType::CustomAttributeClass, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "StrUctcLass"));
    EXPECT_EQ(CustomAttributeContainerType::StructClass, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "rELATioNShIPClaSs"));
    EXPECT_EQ(CustomAttributeContainerType::RelationshipClass, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "AnYcLAsS"));
    EXPECT_EQ(CustomAttributeContainerType::AnyClass, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "primItivePROpERtY"));
    EXPECT_EQ(CustomAttributeContainerType::PrimitiveProperty, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "sTRUctPRoperty"));
    EXPECT_EQ(CustomAttributeContainerType::StructProperty, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "aRrAYpRoPertY"));
    EXPECT_EQ(CustomAttributeContainerType::PrimitiveArrayProperty, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "STRuCTArraYpROPeRty"));
    EXPECT_EQ(CustomAttributeContainerType::StructArrayProperty, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "nAviGATiOnPROpeRty"));
    EXPECT_EQ(CustomAttributeContainerType::NavigationProperty, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "AnYpRoPeRty"));
    EXPECT_EQ(CustomAttributeContainerType::AnyProperty, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "sOurCereLATioNsHipconSTrAINt"));
    EXPECT_EQ(CustomAttributeContainerType::SourceRelationshipConstraint, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "TaRgeTReLATIONShIpcOnSTraiNt"));
    EXPECT_EQ(CustomAttributeContainerType::TargetRelationshipConstraint, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "anyReLatioNshiPcoNsTRaInt"));
    EXPECT_EQ(CustomAttributeContainerType::AnyRelationshipConstraint, type);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "aNy"));
    EXPECT_EQ(CustomAttributeContainerType::Any, type);

    // Test different token separators
    CustomAttributeContainerType targetParsedContainerType = CustomAttributeContainerType::Schema | CustomAttributeContainerType::EntityClass | CustomAttributeContainerType::PrimitiveArrayProperty | CustomAttributeContainerType::TargetRelationshipConstraint;

    CustomAttributeContainerType commaParse;
    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(commaParse, "Schema,EntityClass,ArrayProperty,TargetRelationshipConstraint"));
    EXPECT_EQ(targetParsedContainerType, commaParse);

    CustomAttributeContainerType semicolonParse;
    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(semicolonParse, "Schema;EntityClass;ArrayProperty;TargetRelationshipConstraint"));
    EXPECT_EQ(targetParsedContainerType, semicolonParse);

    CustomAttributeContainerType pipeParse;
    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(pipeParse, "Schema|EntityClass|ArrayProperty|TargetRelationshipConstraint"));
    EXPECT_EQ(targetParsedContainerType, pipeParse);

    CustomAttributeContainerType mixedParse;
    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(mixedParse, "Schema,EntityClass;ArrayProperty|TargetRelationshipConstraint"));
    EXPECT_EQ(targetParsedContainerType, mixedParse);

    // Test invalid strings
    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, ""));
    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(type, "foo"));
    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParseContainerString(commaParse, "Schema EntityClass"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaParseUtilsTest, ParseDirectionString)
    {
    ECRelatedInstanceDirection direction;

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseDirectionString(direction, "forward"));
    EXPECT_EQ(ECRelatedInstanceDirection::Forward, direction);
    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseDirectionString(direction, "Forward"));
    EXPECT_EQ(ECRelatedInstanceDirection::Forward, direction);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseDirectionString(direction, "backward"));
    EXPECT_EQ(ECRelatedInstanceDirection::Backward, direction);
    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseDirectionString(direction, "Backward"));
    EXPECT_EQ(ECRelatedInstanceDirection::Backward, direction);

    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParseDirectionString(direction, ""));
    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParseDirectionString(direction, "foo"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaParseUtilsTest, ParseXmlFullyQualifiedName)
    {
    Utf8String alias;
    Utf8String typeName;

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseXmlFullyQualifiedName(alias, typeName, "Foo:Bar"));
    EXPECT_STREQ("Foo", alias.c_str());
    EXPECT_STREQ("Bar", typeName.c_str());

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseXmlFullyQualifiedName(alias, typeName, "Baz"));
    EXPECT_STREQ("", alias.c_str());
    EXPECT_STREQ("Baz", typeName.c_str());

    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParseXmlFullyQualifiedName(alias, typeName, ""));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaParseUtilsTest, ParseModifierXmlString)
    {
    ECClassModifier modifier;

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseModifierXmlString(modifier, "Abstract"));
    EXPECT_EQ(ECClassModifier::Abstract, modifier);
    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseModifierXmlString(modifier, "abstract"));
    EXPECT_EQ(ECClassModifier::Abstract, modifier);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseModifierXmlString(modifier, "Sealed"));
    EXPECT_EQ(ECClassModifier::Sealed, modifier);
    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseModifierXmlString(modifier, "sealed"));
    EXPECT_EQ(ECClassModifier::Sealed, modifier);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseModifierXmlString(modifier, "None"));
    EXPECT_EQ(ECClassModifier::None, modifier);
    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseModifierXmlString(modifier, "none"));
    EXPECT_EQ(ECClassModifier::None, modifier);

    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParseModifierXmlString(modifier, ""));
    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParseModifierXmlString(modifier, "foo"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaParseUtilsTest, ParseLegacyMultiplicityString)
    {
    uint32_t lowerLimit, upperLimit;

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseLegacyMultiplicityString(lowerLimit, upperLimit, "(3..12)"));
    EXPECT_EQ(3, lowerLimit);
    EXPECT_EQ(12, upperLimit);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseLegacyMultiplicityString(lowerLimit, upperLimit, "(5..*)"));
    EXPECT_EQ(5, lowerLimit);
    EXPECT_EQ(INT_MAX, upperLimit);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseLegacyMultiplicityString(lowerLimit, upperLimit, "(42 .. 120)"));
    EXPECT_EQ(42, lowerLimit);
    EXPECT_EQ(120, upperLimit);

    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParseLegacyMultiplicityString(lowerLimit, upperLimit, ""));
    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParseLegacyMultiplicityString(lowerLimit, upperLimit, "foo"));
    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParseLegacyMultiplicityString(lowerLimit, upperLimit, "(foo..12)"));

    // The tests below exist for a known bug in SchemaParseUtils::ParseLegacyMultiplicityString, where a multiplicity
    // string parsed with the scanf format string "(%d..%d)" will successfully parse the first integer, fail to
    // parse the second integer, and assume the second integer was inteded to be "*". As a result of this bug strings
    // such as
    //      "(3..N)"
    //      "(3,N)"
    //      "(3banana)"
    // would all be parsed as if they were "(3..*)".
    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseLegacyMultiplicityString(lowerLimit, upperLimit, "(3,12)"));
    EXPECT_EQ(3, lowerLimit);
    EXPECT_EQ(INT_MAX, upperLimit);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseLegacyMultiplicityString(lowerLimit, upperLimit, "(5,N)"));
    EXPECT_EQ(5, lowerLimit);
    EXPECT_EQ(INT_MAX, upperLimit);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseLegacyMultiplicityString(lowerLimit, upperLimit, "(7..N)"));
    EXPECT_EQ(7, lowerLimit);
    EXPECT_EQ(INT_MAX, upperLimit);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseLegacyMultiplicityString(lowerLimit, upperLimit, "(5)"));
    EXPECT_EQ(5, lowerLimit);
    EXPECT_EQ(INT_MAX, upperLimit);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseLegacyMultiplicityString(lowerLimit, upperLimit, "(7banana)"));
    EXPECT_EQ(7, lowerLimit);
    EXPECT_EQ(INT_MAX, upperLimit);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaParseUtilsTest, ParseMultiplicityString)
    {
    uint32_t lowerLimit, upperLimit;

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseMultiplicityString(lowerLimit, upperLimit, "(3..12)"));
    EXPECT_EQ(3, lowerLimit);
    EXPECT_EQ(12, upperLimit);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseMultiplicityString(lowerLimit, upperLimit, "(5..*)"));
    EXPECT_EQ(5, lowerLimit);
    EXPECT_EQ(INT_MAX, upperLimit);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseMultiplicityString(lowerLimit, upperLimit, "(42 .. 120)"));
    EXPECT_EQ(42, lowerLimit);
    EXPECT_EQ(120, upperLimit);

    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParseMultiplicityString(lowerLimit, upperLimit, ""));
    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParseMultiplicityString(lowerLimit, upperLimit, "foo"));
    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParseMultiplicityString(lowerLimit, upperLimit, "(foo..12)"));

    // Should fail to parse the old, nasty multiplicity strings
    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParseMultiplicityString(lowerLimit, upperLimit, "(3,12)"));
    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParseMultiplicityString(lowerLimit, upperLimit, "(5,N)"));
    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParseMultiplicityString(lowerLimit, upperLimit, "(7..N)"));
    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParseMultiplicityString(lowerLimit, upperLimit, "(5)"));
    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParseMultiplicityString(lowerLimit, upperLimit, "(7banana)"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaParseUtilsTest, ParsePrimitiveType)
    {
    PrimitiveType primType;

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParsePrimitiveType(primType, "string"));
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_String, primType);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParsePrimitiveType(primType, "int"));
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_Integer, primType);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParsePrimitiveType(primType, "long"));
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_Long, primType);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParsePrimitiveType(primType, "boolean"));
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_Boolean, primType);
    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParsePrimitiveType(primType, "bool"));
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_Boolean, primType);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParsePrimitiveType(primType, "double"));
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_Double, primType);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParsePrimitiveType(primType, "point2d"));
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_Point2d, primType);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParsePrimitiveType(primType, "point3d"));
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_Point3d, primType);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParsePrimitiveType(primType, "dateTime"));
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_DateTime, primType);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParsePrimitiveType(primType, "Bentley.GeometryNET.Common"));
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_IGeometry, primType);
    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParsePrimitiveType(primType, "Bentley.Geometry.Common"));
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_IGeometry, primType);
    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParsePrimitiveType(primType, "Bentley.Geometry.Common.IGeometry"));
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_IGeometry, primType);

    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParsePrimitiveType(primType, ""));
    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParsePrimitiveType(primType, "foo"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaParseUtilsTest, ParseStrengthType)
    {
    StrengthType strength;

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseStrengthType(strength, "referencing"));
    EXPECT_EQ(StrengthType::Referencing, strength);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseStrengthType(strength, "holding"));
    EXPECT_EQ(StrengthType::Holding, strength);

    EXPECT_EQ(ECObjectsStatus::Success, SchemaParseUtils::ParseStrengthType(strength, "embedding"));
    EXPECT_EQ(StrengthType::Embedding, strength);

    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParseStrengthType(strength, ""));
    EXPECT_NE(ECObjectsStatus::Success, SchemaParseUtils::ParseStrengthType(strength, "foo"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaParseUtilsTest, DirectionToString)
    {
    EXPECT_STREQ("forward", SchemaParseUtils::DirectionToXmlString(ECRelatedInstanceDirection::Forward));
    EXPECT_STREQ("backward", SchemaParseUtils::DirectionToXmlString(ECRelatedInstanceDirection::Backward));

    EXPECT_STREQ("Forward", SchemaParseUtils::DirectionToJsonString(ECRelatedInstanceDirection::Forward));
    EXPECT_STREQ("Backward", SchemaParseUtils::DirectionToJsonString(ECRelatedInstanceDirection::Backward));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaParseUtilsTest, ModifierToString)
    {
    EXPECT_STREQ("Abstract", SchemaParseUtils::ModifierToString(ECClassModifier::Abstract));
    EXPECT_STREQ("Sealed", SchemaParseUtils::ModifierToString(ECClassModifier::Sealed));
    EXPECT_STREQ("None", SchemaParseUtils::ModifierToString(ECClassModifier::None));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaParseUtilsTest, PrimitiveTypeToString)
    {
    EXPECT_STREQ("binary", SchemaParseUtils::PrimitiveTypeToString(PrimitiveType::PRIMITIVETYPE_Binary));
    EXPECT_STREQ("boolean", SchemaParseUtils::PrimitiveTypeToString(PrimitiveType::PRIMITIVETYPE_Boolean));
    EXPECT_STREQ("dateTime", SchemaParseUtils::PrimitiveTypeToString(PrimitiveType::PRIMITIVETYPE_DateTime));
    EXPECT_STREQ("double", SchemaParseUtils::PrimitiveTypeToString(PrimitiveType::PRIMITIVETYPE_Double));
    EXPECT_STREQ("int", SchemaParseUtils::PrimitiveTypeToString(PrimitiveType::PRIMITIVETYPE_Integer));
    EXPECT_STREQ("long", SchemaParseUtils::PrimitiveTypeToString(PrimitiveType::PRIMITIVETYPE_Long));
    EXPECT_STREQ("point2d", SchemaParseUtils::PrimitiveTypeToString(PrimitiveType::PRIMITIVETYPE_Point2d));
    EXPECT_STREQ("point3d", SchemaParseUtils::PrimitiveTypeToString(PrimitiveType::PRIMITIVETYPE_Point3d));
    EXPECT_STREQ("string", SchemaParseUtils::PrimitiveTypeToString(PrimitiveType::PRIMITIVETYPE_String));
    EXPECT_STREQ("Bentley.Geometry.Common.IGeometry", SchemaParseUtils::PrimitiveTypeToString(PrimitiveType::PRIMITIVETYPE_IGeometry));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaParseUtilsTest, StrengthToString)
    {
    EXPECT_STREQ("referencing", SchemaParseUtils::StrengthToXmlString(StrengthType::Referencing));
    EXPECT_STREQ("embedding", SchemaParseUtils::StrengthToXmlString(StrengthType::Embedding));
    EXPECT_STREQ("holding", SchemaParseUtils::StrengthToXmlString(StrengthType::Holding));

    EXPECT_STREQ("Referencing", SchemaParseUtils::StrengthToJsonString(StrengthType::Referencing));
    EXPECT_STREQ("Embedding", SchemaParseUtils::StrengthToJsonString(StrengthType::Embedding));
    EXPECT_STREQ("Holding", SchemaParseUtils::StrengthToJsonString(StrengthType::Holding));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaParseUtilsTest, ContainerTypeToString)
    {
    EXPECT_STREQ("Schema", SchemaParseUtils::ContainerTypeToString(CustomAttributeContainerType::Schema).c_str());
    EXPECT_STREQ("EntityClass", SchemaParseUtils::ContainerTypeToString(CustomAttributeContainerType::EntityClass).c_str());
    EXPECT_STREQ("CustomAttributeClass", SchemaParseUtils::ContainerTypeToString(CustomAttributeContainerType::CustomAttributeClass).c_str());
    EXPECT_STREQ("StructClass", SchemaParseUtils::ContainerTypeToString(CustomAttributeContainerType::StructClass).c_str());
    EXPECT_STREQ("RelationshipClass", SchemaParseUtils::ContainerTypeToString(CustomAttributeContainerType::RelationshipClass).c_str());
    EXPECT_STREQ("AnyClass", SchemaParseUtils::ContainerTypeToString(CustomAttributeContainerType::AnyClass).c_str());
    EXPECT_STREQ("PrimitiveProperty", SchemaParseUtils::ContainerTypeToString(CustomAttributeContainerType::PrimitiveProperty).c_str());
    EXPECT_STREQ("StructProperty", SchemaParseUtils::ContainerTypeToString(CustomAttributeContainerType::StructProperty).c_str());
    EXPECT_STREQ("ArrayProperty", SchemaParseUtils::ContainerTypeToString(CustomAttributeContainerType::PrimitiveArrayProperty).c_str());
    EXPECT_STREQ("StructArrayProperty", SchemaParseUtils::ContainerTypeToString(CustomAttributeContainerType::StructArrayProperty).c_str());
    EXPECT_STREQ("NavigationProperty", SchemaParseUtils::ContainerTypeToString(CustomAttributeContainerType::NavigationProperty).c_str());
    EXPECT_STREQ("AnyProperty", SchemaParseUtils::ContainerTypeToString(CustomAttributeContainerType::AnyProperty).c_str());
    EXPECT_STREQ("SourceRelationshipConstraint", SchemaParseUtils::ContainerTypeToString(CustomAttributeContainerType::SourceRelationshipConstraint).c_str());
    EXPECT_STREQ("TargetRelationshipConstraint", SchemaParseUtils::ContainerTypeToString(CustomAttributeContainerType::TargetRelationshipConstraint).c_str());
    EXPECT_STREQ("AnyRelationshipConstraint", SchemaParseUtils::ContainerTypeToString(CustomAttributeContainerType::AnyRelationshipConstraint).c_str());
    EXPECT_STREQ("Any", SchemaParseUtils::ContainerTypeToString(CustomAttributeContainerType::Any).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaParseUtilsTest, MultiplicityToLegacyString)
    {
    EXPECT_EQ("(0,N)", SchemaParseUtils::MultiplicityToLegacyString(RelationshipMultiplicity(0)));
    EXPECT_EQ("(0,1)", SchemaParseUtils::MultiplicityToLegacyString(RelationshipMultiplicity(0, 1)));
    EXPECT_EQ("(3,N)", SchemaParseUtils::MultiplicityToLegacyString(RelationshipMultiplicity(3)));
    EXPECT_EQ("(3,5)", SchemaParseUtils::MultiplicityToLegacyString(RelationshipMultiplicity(3, 5)));
    }

END_BENTLEY_ECN_TEST_NAMESPACE
