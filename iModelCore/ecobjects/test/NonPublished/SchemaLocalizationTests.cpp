/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/SchemaLocalizationTests.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include "TestFixture.h"

#include <ECObjects\ECSchema.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

struct SchemaLocalizationTests : ECTestFixture {};

static WCharCP const PK = L"~~";
static WCharCP const GB = L"!!";
static WCharCP const IT = L"**";

WString PsudoLocalizeString (WCharCP prePostFix, WCharCP invariantString)
    {
    WString psudoLocString(prePostFix);
    psudoLocString.append(invariantString);
    psudoLocString.append(prePostFix);
    return psudoLocString;
    }

void VerifyCaString (IECCustomAttributeContainerR caContainer, WCharCP containerId, WCharCP invariantString, WCharCP caClassName, WCharCP propertyAccessor, WCharCP prePostFix = L"")
    {
    IECInstancePtr caInstance = caContainer.GetCustomAttribute(caClassName);
    EXPECT_TRUE(caInstance.IsValid());

    ECValue stringValue;
    ECValueAccessor accessor;
    EXPECT_EQ(ECOBJECTS_STATUS_Success, ECValueAccessor::PopulateValueAccessor(accessor, *caInstance, propertyAccessor));
    EXPECT_EQ(ECOBJECTS_STATUS_Success, caInstance->GetValueUsingAccessor(stringValue, accessor));
    EXPECT_FALSE(stringValue.IsNull()) << "Failed to get a valid value for " 
        << caClassName << "." << propertyAccessor << " on container: " << containerId << "\nInstance\n" << caInstance->ToString(L"  ").c_str();

    if (!WString::IsNullOrEmpty(prePostFix))
        EXPECT_STREQ(PsudoLocalizeString(prePostFix, invariantString).c_str(), stringValue.GetString())
        << "Failed to localize property " << caClassName << "." << propertyAccessor << " on container: " << containerId;// << "\nInstance\n" << caInstance->ToString(L"  ").c_str();
    else
        EXPECT_STREQ(invariantString, stringValue.GetString())
        << "Property unexpectedly localized " << caClassName << "." << propertyAccessor << " on container: " << containerId;// << "\nInstance\n" << caInstance->ToString(L"  ").c_str();
    }

void VerifyLocalized(ECSchemaPtr testSchema, WCharCP prePostFix)
    {
    // Test Schema level
    WCharCP houseLabel = L"House";
    WCharCP houseDescription = L"A house schema";
    EXPECT_STREQ(houseLabel, testSchema->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ(PsudoLocalizeString(prePostFix, houseLabel).c_str(), testSchema->GetDisplayLabel().c_str());
    EXPECT_STREQ(houseDescription, testSchema->GetInvariantDescription().c_str());
    EXPECT_STREQ(PsudoLocalizeString(prePostFix, houseDescription).c_str(), testSchema->GetDescription().c_str());

    // Test Class level
    ECClassCP itemClass = testSchema->GetClassCP(L"Item");
    WCharCP itemLabel = L"Item";
    WCharCP itemDescription = L"An Item";
    EXPECT_STREQ(itemLabel, itemClass->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ(PsudoLocalizeString(prePostFix, itemLabel).c_str(), itemClass->GetDisplayLabel().c_str());
    EXPECT_STREQ(itemDescription, itemClass->GetInvariantDescription().c_str());
    EXPECT_STREQ(PsudoLocalizeString(prePostFix, itemDescription).c_str(), itemClass->GetDescription().c_str());

    // Test Rel Class Role Labels
    WCharCP sourceRoleLabel = L"Door to room";
    WCharCP targetRoleLabel = L"Door for room";
    ECRelationshipClassCP relClass = testSchema->GetClassP(L"RoomHasDoor")->GetRelationshipClassCP();
    ECRelationshipConstraintR sourceConstraint = relClass->GetSource();
    ECRelationshipConstraintR targetConstraint = relClass->GetTarget();
    EXPECT_STREQ(sourceRoleLabel, sourceConstraint.GetInvariantRoleLabel().c_str());
    EXPECT_STREQ(PsudoLocalizeString(prePostFix, sourceRoleLabel).c_str(), sourceConstraint.GetRoleLabel().c_str());
    EXPECT_STREQ(targetRoleLabel, targetConstraint.GetInvariantRoleLabel().c_str());
    EXPECT_STREQ(PsudoLocalizeString(prePostFix, targetRoleLabel).c_str(), targetConstraint.GetRoleLabel().c_str());

    // Test Property level
    ECPropertyP displayNameProp = itemClass->GetPropertyP(L"DisplayName", false);
    WCharCP displayNameLabel = L"Display Name";
    WCharCP displayNameDescription = L"Display label for an item";
    EXPECT_STREQ(displayNameLabel, displayNameProp->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ(PsudoLocalizeString(prePostFix, displayNameLabel).c_str(), displayNameProp->GetDisplayLabel().c_str());
    EXPECT_STREQ(displayNameDescription, displayNameProp->GetInvariantDescription().c_str());
    EXPECT_STREQ(PsudoLocalizeString(prePostFix, displayNameDescription).c_str(), displayNameProp->GetDescription().c_str());

    // Test duplicate strings are resolved
    WCharCP lengthString = L"Length";
    ECPropertyP roomLengthProp = testSchema->GetClassCP(L"Room")->GetPropertyP(lengthString, false);
    ECPropertyP doorLengthProp = testSchema->GetClassCP(L"Door")->GetPropertyP(lengthString, false);
    EXPECT_STREQ(lengthString, roomLengthProp->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ(lengthString, doorLengthProp->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ(PsudoLocalizeString(prePostFix, lengthString).c_str(), roomLengthProp->GetDisplayLabel().c_str());
    EXPECT_STREQ(PsudoLocalizeString(prePostFix, lengthString).c_str(), doorLengthProp->GetDisplayLabel().c_str());

    // Test custom attribute strings
    // Schema
    VerifyCaString(*testSchema, testSchema->GetFullSchemaName().c_str(), L"A whole House", L"ExtendedInfo", L"Purpose", prePostFix);
    // Class
    ECClassP doorClass = testSchema->GetClassP(L"Door");
    VerifyCaString(*doorClass, doorClass->GetFullName(), L"squar", L"ExtendedInfo", L"ExtraInfo[1]", prePostFix);
    VerifyCaString(*doorClass, doorClass->GetFullName(), L"wood", L"ExtendedInfo", L"ContentInfo[0].DisplayName", prePostFix);
    // Relationship Constraints
    ECRelationshipClassCP roomHasDoorRelClass = testSchema->GetClassCP(L"RoomHasDoor")->GetRelationshipClassCP();
    VerifyCaString(roomHasDoorRelClass->GetSource(), roomHasDoorRelClass->GetFullName(), L"This is a room", L"ExtendedInfo", L"Purpose", prePostFix);
    VerifyCaString(roomHasDoorRelClass->GetTarget(), roomHasDoorRelClass->GetFullName(), L"This is a door", L"ExtendedInfo", L"Purpose", prePostFix);
    // Property
    doorLengthProp = testSchema->GetClassCP(L"Door")->GetPropertyP(lengthString, false);
    VerifyCaString(*doorLengthProp, doorLengthProp->GetName().c_str(), L"Length of door", L"ExtendedInfo", L"Purpose", prePostFix);
    VerifyCaString(*doorLengthProp, doorLengthProp->GetName().c_str(), L"Constant", L"StandardValues", L"ValueMap[1].DisplayString", prePostFix);
    }

void VerifyNotLocalized(ECSchemaPtr testSchema)
    {
    // Test Schema level
    WCharCP houseLabel = L"House";
    WCharCP houseDescription = L"A house schema";
    EXPECT_STREQ(houseLabel, testSchema->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ(houseLabel, testSchema->GetDisplayLabel().c_str());
    EXPECT_STREQ(houseDescription, testSchema->GetInvariantDescription().c_str());
    EXPECT_STREQ(houseDescription, testSchema->GetDescription().c_str());

    // Test Class level
    ECClassCP itemClass = testSchema->GetClassCP(L"Item");
    WCharCP itemLabel = L"Item";
    WCharCP itemDescription = L"An Item";
    EXPECT_STREQ(itemLabel, itemClass->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ(itemLabel, itemClass->GetDisplayLabel().c_str());
    EXPECT_STREQ(itemDescription, itemClass->GetInvariantDescription().c_str());
    EXPECT_STREQ(itemDescription, itemClass->GetDescription().c_str());

    // Test Rel Class Role Labels
    WCharCP sourceRoleLabel = L"Door to room";
    WCharCP targetRoleLabel = L"Door for room";
    ECRelationshipClassCP relClass = testSchema->GetClassP(L"RoomHasDoor")->GetRelationshipClassCP();
    ECRelationshipConstraintR sourceConstraint = relClass->GetSource();
    ECRelationshipConstraintR targetConstraint = relClass->GetTarget();
    EXPECT_STREQ(sourceRoleLabel, sourceConstraint.GetInvariantRoleLabel().c_str());
    EXPECT_STREQ(sourceRoleLabel, sourceConstraint.GetRoleLabel().c_str());
    EXPECT_STREQ(targetRoleLabel, targetConstraint.GetInvariantRoleLabel().c_str());
    EXPECT_STREQ(targetRoleLabel, targetConstraint.GetRoleLabel().c_str());

    // Test Property level
    ECPropertyP displayNameProp = itemClass->GetPropertyP(L"DisplayName", false);
    WCharCP displayNameLabel = L"Display Name";
    WCharCP displayNameDescription = L"Display label for an item";
    EXPECT_STREQ(displayNameLabel, displayNameProp->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ(displayNameLabel, displayNameProp->GetDisplayLabel().c_str());
    EXPECT_STREQ(displayNameDescription, displayNameProp->GetInvariantDescription().c_str());
    EXPECT_STREQ(displayNameDescription, displayNameProp->GetDescription().c_str());

    // Test duplicate strings are resolved
    WCharCP lengthString = L"Length";
    ECPropertyP roomLengthProp = testSchema->GetClassCP(L"Room")->GetPropertyP(lengthString, false);
    ECPropertyP doorLengthProp = testSchema->GetClassCP(L"Door")->GetPropertyP(lengthString, false);
    EXPECT_STREQ(lengthString, roomLengthProp->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ(lengthString, doorLengthProp->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ(lengthString, roomLengthProp->GetDisplayLabel().c_str());
    EXPECT_STREQ(lengthString, doorLengthProp->GetDisplayLabel().c_str());

    // Test custom attribute strings
    VerifyCaString(*testSchema, testSchema->GetFullSchemaName().c_str(), L"A whole House", L"ExtendedInfo", L"Purpose");
    // Class
    ECClassP doorClass = testSchema->GetClassP(L"Door");
    VerifyCaString(*doorClass, doorClass->GetFullName(), L"squar", L"ExtendedInfo", L"ExtraInfo[1]");
    VerifyCaString(*doorClass, doorClass->GetFullName(), L"wood", L"ExtendedInfo", L"ContentInfo[0].DisplayName");
    // Relationship Constraints
    ECRelationshipClassCP roomHasDoorRelClass = testSchema->GetClassCP(L"RoomHasDoor")->GetRelationshipClassCP();
    VerifyCaString(roomHasDoorRelClass->GetSource(), roomHasDoorRelClass->GetFullName(), L"This is a room", L"ExtendedInfo", L"Purpose");
    VerifyCaString(roomHasDoorRelClass->GetTarget(), roomHasDoorRelClass->GetFullName(), L"This is a door", L"ExtendedInfo", L"Purpose");
    // Property
    VerifyCaString(*doorLengthProp, doorLengthProp->GetName().c_str(), L"Length of door", L"ExtendedInfo", L"Purpose");
    VerifyCaString(*doorLengthProp, doorLengthProp->GetName().c_str(), L"Constant", L"StandardValues", L"ValueMap[1].DisplayString");
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
    SchemaKey key(L"House", 01, 00);
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
    SchemaKey key(L"House", 01, 00);
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
    SchemaKey key(L"House", 01, 00);
    testSchema = schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_Latest);
    EXPECT_TRUE(testSchema->IsSupplemented());
    VerifyLocalized(testSchema, IT);

    ECSchemaPtr copyTestSchema;
    WString     schemaXml;
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
    SchemaKey key(L"House", 01, 00);
    testSchema = schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_Latest);
    EXPECT_TRUE(testSchema->IsSupplemented());

	VerifyLocalized(testSchema, PK);

	testSchema->SetDisplayLabel(L"Banana");
	EXPECT_STREQ(L"Banana", testSchema->GetDisplayLabel().c_str());
	EXPECT_STREQ(L"Banana", testSchema->GetInvariantDisplayLabel().c_str());
    }

END_BENTLEY_ECOBJECT_NAMESPACE
