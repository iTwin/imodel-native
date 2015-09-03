/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/SchemaLocalizationTests.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

#include <ECObjects/ECSchema.h>
using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaLocalizationTests : ECTestFixture {};

static Utf8CP const PK = "~~";
static Utf8CP const GB = "!!";
static Utf8CP const IT = "**";

Utf8String PseudoLocalizeString (Utf8CP prePostFix, Utf8CP invariantString)
    {
    Utf8String pseudoLocString(prePostFix);
    pseudoLocString.append(invariantString);
    pseudoLocString.append(prePostFix);
    return pseudoLocString;
    }

void VerifyCaString (IECCustomAttributeContainerR caContainer, Utf8CP containerId, Utf8CP invariantString, Utf8CP caClassName, Utf8CP propertyAccessor, Utf8CP prePostFix = "")
    {
    IECInstancePtr caInstance = caContainer.GetCustomAttribute(caClassName);
    EXPECT_TRUE(caInstance.IsValid());

    ECValue stringValue;
    ECValueAccessor accessor;
    EXPECT_EQ(ECOBJECTS_STATUS_Success, ECValueAccessor::PopulateValueAccessor(accessor, *caInstance, propertyAccessor));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, caInstance->GetValueUsingAccessor(stringValue, accessor));
    EXPECT_FALSE(stringValue.IsNull()) << "Failed to get a valid value for " 
        << caClassName << "." << propertyAccessor << " on container: " << containerId << "\nInstance\n" << caInstance->ToString("  ").c_str();

    if (!Utf8String::IsNullOrEmpty(prePostFix))
        EXPECT_STREQ(PseudoLocalizeString(prePostFix, invariantString).c_str(), stringValue.GetUtf8CP())
        << "Failed to localize property " << caClassName << "." << propertyAccessor << " on container: " << containerId;// << "\nInstance\n" << caInstance->ToString(L"  ").c_str();
    else
        EXPECT_STREQ(invariantString, stringValue.GetUtf8CP())
        << "Property unexpectedly localized " << caClassName << "." << propertyAccessor << " on container: " << containerId;// << "\nInstance\n" << caInstance->ToString(L"  ").c_str();
    }

void VerifyLocalized(ECSchemaPtr testSchema, Utf8CP prePostFix)
    {
    // Test Schema level
    Utf8CP houseLabel = "House";
    Utf8CP houseDescription = "A house schema";
    EXPECT_STREQ(houseLabel, testSchema->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ(PseudoLocalizeString(prePostFix, houseLabel).c_str(), testSchema->GetDisplayLabel().c_str());
    EXPECT_STREQ(houseDescription, testSchema->GetInvariantDescription().c_str());
    EXPECT_STREQ(PseudoLocalizeString(prePostFix, houseDescription).c_str(), testSchema->GetDescription().c_str());

    // Test Class level
    ECClassCP itemClass = testSchema->GetClassCP("Item");
    Utf8CP itemLabel = "Item";
    Utf8CP itemDescription = "An Item";
    EXPECT_STREQ(itemLabel, itemClass->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ(PseudoLocalizeString(prePostFix, itemLabel).c_str(), itemClass->GetDisplayLabel().c_str());
    EXPECT_STREQ(itemDescription, itemClass->GetInvariantDescription().c_str());
    EXPECT_STREQ(PseudoLocalizeString(prePostFix, itemDescription).c_str(), itemClass->GetDescription().c_str());

    // Test Rel Class Role Labels
    Utf8CP sourceRoleLabel = "Door to room";
    Utf8CP targetRoleLabel = "Door for room";
    ECRelationshipClassCP relClass = testSchema->GetClassP("RoomHasDoor")->GetRelationshipClassCP();
    ECRelationshipConstraintR sourceConstraint = relClass->GetSource();
    ECRelationshipConstraintR targetConstraint = relClass->GetTarget();
    EXPECT_STREQ(sourceRoleLabel, sourceConstraint.GetInvariantRoleLabel().c_str());
    EXPECT_STREQ(PseudoLocalizeString(prePostFix, sourceRoleLabel).c_str(), sourceConstraint.GetRoleLabel().c_str());
    EXPECT_STREQ(targetRoleLabel, targetConstraint.GetInvariantRoleLabel().c_str());
    EXPECT_STREQ(PseudoLocalizeString(prePostFix, targetRoleLabel).c_str(), targetConstraint.GetRoleLabel().c_str());

    // Test Property level
    ECPropertyP displayNameProp = itemClass->GetPropertyP("DisplayName", false);
    Utf8CP displayNameLabel = "Display Name";
    Utf8CP displayNameDescription = "Display label for an item";
    EXPECT_STREQ(displayNameLabel, displayNameProp->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ(PseudoLocalizeString(prePostFix, displayNameLabel).c_str(), displayNameProp->GetDisplayLabel().c_str());
    EXPECT_STREQ(displayNameDescription, displayNameProp->GetInvariantDescription().c_str());
    EXPECT_STREQ(PseudoLocalizeString(prePostFix, displayNameDescription).c_str(), displayNameProp->GetDescription().c_str());

    // Test duplicate strings are resolved
    Utf8CP lengthString = "Length";
    ECPropertyP roomLengthProp = testSchema->GetClassCP("Room")->GetPropertyP(lengthString, false);
    ECPropertyP doorLengthProp = testSchema->GetClassCP("Door")->GetPropertyP(lengthString, false);
    EXPECT_STREQ(lengthString, roomLengthProp->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ(lengthString, doorLengthProp->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ(PseudoLocalizeString(prePostFix, lengthString).c_str(), roomLengthProp->GetDisplayLabel().c_str());
    EXPECT_STREQ(PseudoLocalizeString(prePostFix, lengthString).c_str(), doorLengthProp->GetDisplayLabel().c_str());

    // Test custom attribute strings
    // Schema
    VerifyCaString(*testSchema, testSchema->GetFullSchemaName().c_str(), "A whole House", "ExtendedInfo", "Purpose", prePostFix);
    // Class
    ECClassP doorClass = testSchema->GetClassP("Door");
    VerifyCaString(*doorClass, doorClass->GetFullName(), "squar", "ExtendedInfo", "ExtraInfo[1]", prePostFix);
    VerifyCaString(*doorClass, doorClass->GetFullName(), "wood", "ExtendedInfo", "ContentInfo[0].DisplayName", prePostFix);
    // Relationship Constraints
    ECRelationshipClassCP roomHasDoorRelClass = testSchema->GetClassCP("RoomHasDoor")->GetRelationshipClassCP();
    VerifyCaString(roomHasDoorRelClass->GetSource(), roomHasDoorRelClass->GetFullName(), "This is a room", "ExtendedInfo", "Purpose", prePostFix);
    VerifyCaString(roomHasDoorRelClass->GetTarget(), roomHasDoorRelClass->GetFullName(), "This is a door", "ExtendedInfo", "Purpose", prePostFix);
    // Property
    doorLengthProp = testSchema->GetClassCP("Door")->GetPropertyP(lengthString, false);
    VerifyCaString(*doorLengthProp, doorLengthProp->GetName().c_str(), "Length of door", "ExtendedInfo", "Purpose", prePostFix);
    VerifyCaString(*doorLengthProp, doorLengthProp->GetName().c_str(), "Constant", "StandardValues", "ValueMap[1].DisplayString", prePostFix);
    }

void VerifyNotLocalized(ECSchemaPtr testSchema)
    {
    // Test Schema level
    Utf8CP houseLabel = "House";
    Utf8CP houseDescription = "A house schema";
    EXPECT_STREQ(houseLabel, testSchema->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ(houseLabel, testSchema->GetDisplayLabel().c_str());
    EXPECT_STREQ(houseDescription, testSchema->GetInvariantDescription().c_str());
    EXPECT_STREQ(houseDescription, testSchema->GetDescription().c_str());

    // Test Class level
    ECClassCP itemClass = testSchema->GetClassCP("Item");
    Utf8CP itemLabel = "Item";
    Utf8CP itemDescription = "An Item";
    EXPECT_STREQ(itemLabel, itemClass->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ(itemLabel, itemClass->GetDisplayLabel().c_str());
    EXPECT_STREQ(itemDescription, itemClass->GetInvariantDescription().c_str());
    EXPECT_STREQ(itemDescription, itemClass->GetDescription().c_str());

    // Test Rel Class Role Labels
    Utf8CP sourceRoleLabel = "Door to room";
    Utf8CP targetRoleLabel = "Door for room";
    ECRelationshipClassCP relClass = testSchema->GetClassP("RoomHasDoor")->GetRelationshipClassCP();
    ECRelationshipConstraintR sourceConstraint = relClass->GetSource();
    ECRelationshipConstraintR targetConstraint = relClass->GetTarget();
    EXPECT_STREQ(sourceRoleLabel, sourceConstraint.GetInvariantRoleLabel().c_str());
    EXPECT_STREQ(sourceRoleLabel, sourceConstraint.GetRoleLabel().c_str());
    EXPECT_STREQ(targetRoleLabel, targetConstraint.GetInvariantRoleLabel().c_str());
    EXPECT_STREQ(targetRoleLabel, targetConstraint.GetRoleLabel().c_str());

    // Test Property level
    ECPropertyP displayNameProp = itemClass->GetPropertyP("DisplayName", false);
    Utf8CP displayNameLabel = "Display Name";
    Utf8CP displayNameDescription = "Display label for an item";
    EXPECT_STREQ(displayNameLabel, displayNameProp->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ(displayNameLabel, displayNameProp->GetDisplayLabel().c_str());
    EXPECT_STREQ(displayNameDescription, displayNameProp->GetInvariantDescription().c_str());
    EXPECT_STREQ(displayNameDescription, displayNameProp->GetDescription().c_str());

    // Test duplicate strings are resolved
    Utf8CP lengthString = "Length";
    ECPropertyP roomLengthProp = testSchema->GetClassCP("Room")->GetPropertyP(lengthString, false);
    ECPropertyP doorLengthProp = testSchema->GetClassCP("Door")->GetPropertyP(lengthString, false);
    EXPECT_STREQ(lengthString, roomLengthProp->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ(lengthString, doorLengthProp->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ(lengthString, roomLengthProp->GetDisplayLabel().c_str());
    EXPECT_STREQ(lengthString, doorLengthProp->GetDisplayLabel().c_str());

    // Test custom attribute strings
    VerifyCaString(*testSchema, testSchema->GetFullSchemaName().c_str(), "A whole House", "ExtendedInfo", "Purpose");
    // Class
    ECClassP doorClass = testSchema->GetClassP("Door");
    VerifyCaString(*doorClass, doorClass->GetFullName(), "squar", "ExtendedInfo", "ExtraInfo[1]");
    VerifyCaString(*doorClass, doorClass->GetFullName(), "wood", "ExtendedInfo", "ContentInfo[0].DisplayName");
    // Relationship Constraints
    ECRelationshipClassCP roomHasDoorRelClass = testSchema->GetClassCP("RoomHasDoor")->GetRelationshipClassCP();
    VerifyCaString(roomHasDoorRelClass->GetSource(), roomHasDoorRelClass->GetFullName(), "This is a room", "ExtendedInfo", "Purpose");
    VerifyCaString(roomHasDoorRelClass->GetTarget(), roomHasDoorRelClass->GetFullName(), "This is a door", "ExtendedInfo", "Purpose");
    // Property
    VerifyCaString(*doorLengthProp, doorLengthProp->GetName().c_str(), "Length of door", "ExtendedInfo", "Purpose");
    VerifyCaString(*doorLengthProp, doorLengthProp->GetName().c_str(), "Constant", "StandardValues", "ValueMap[1].DisplayString");
    }

//---------------------------------------------------------------------------------------//
// Tests that localization supplemental schemas are properly applied
// @bsimethod                                    Colin.Kerr                       04/2015
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(SchemaLocalizationTests, SupplementingLocalizationSupplemental)
    {
    ECSchemaPtr testSchema;
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->AddCulture(L"ur-PK");
    schemaContext->AddSchemaPath(ECTestFixture::GetTestDataPath(L"").c_str());
    SchemaKey key("House", 01, 00);
    testSchema = schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_Latest);
    EXPECT_TRUE(testSchema->IsSupplemented());
    
    VerifyLocalized(testSchema, PK);
    }

//---------------------------------------------------------------------------------------//
// Tests that copied schemas have the same invariant strings as the original schema ensuring 
// that the localized strings are not set as the invariant strings.  
// @bsimethod                                    Colin.Kerr                       04/2015
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(SchemaLocalizationTests, CopyingALocalizedSchema)
    {
    ECSchemaPtr testSchema;
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->AddCulture(L"en-GB");
    schemaContext->AddSchemaPath(ECTestFixture::GetTestDataPath(L"").c_str());
    SchemaKey key("House", 01, 00);
    testSchema = schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_Latest);
    EXPECT_TRUE(testSchema->IsSupplemented());
    VerifyLocalized(testSchema, GB);

    ECSchemaPtr copyTestSchema;
    EXPECT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, testSchema->CopySchema(copyTestSchema));
    EXPECT_FALSE(copyTestSchema->IsSupplemented());
    VerifyNotLocalized(copyTestSchema);
    }

//---------------------------------------------------------------------------------------//
// Tests that the invariant strings are serialized NOT the localized strings
// @bsimethod                                    Colin.Kerr                       04/2015
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(SchemaLocalizationTests, XmlSerializeALocalizedSchema)
    {
    ECSchemaPtr testSchema;
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->AddCulture(L"it-IT");
    schemaContext->AddCulture(L"it");
    schemaContext->AddSchemaPath(ECTestFixture::GetTestDataPath(L"").c_str());
    SchemaKey key("House", 01, 00);
    testSchema = schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_Latest);
    EXPECT_TRUE(testSchema->IsSupplemented());
    VerifyLocalized(testSchema, IT);

    ECSchemaPtr copyTestSchema;
    Utf8String     schemaXml;
    EXPECT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, testSchema->WriteToXmlString(schemaXml));
    ECSchemaReadContextPtr deserializedSchemaContext = ECSchemaReadContext::CreateContext();
    EXPECT_EQ(SchemaReadStatus::SCHEMA_READ_STATUS_Success, ECSchema::ReadFromXmlString(copyTestSchema, schemaXml.c_str(), *deserializedSchemaContext));
    EXPECT_FALSE(copyTestSchema->IsSupplemented());
    VerifyNotLocalized(copyTestSchema);
    }

//---------------------------------------------------------------------------------------//
// Tests that custom display strings are not overridden by the localized strings
// @bsimethod                                    Colin.Kerr                       04/2015
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(SchemaLocalizationTests, CustomStringsNotOverridden)
    {
    ECSchemaPtr testSchema;
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->AddCulture(L"ur-PK");
    schemaContext->AddCulture(L"ur");
    schemaContext->AddSchemaPath(ECTestFixture::GetTestDataPath(L"").c_str());
    SchemaKey key("House", 01, 00);
    testSchema = schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_Latest);
    EXPECT_TRUE(testSchema->IsSupplemented());
    VerifyLocalized(testSchema, PK);

    //testSchema->SetDisplayLabel(L"Banana");
    //EXPECT_STREQ(L"Banana", testSchema->GetDisplayLabel().c_str());
    //EXPECT_STREQ(L"Banana", testSchema->GetInvariantDisplayLabel().c_str());
    }

END_BENTLEY_ECN_TEST_NAMESPACE
