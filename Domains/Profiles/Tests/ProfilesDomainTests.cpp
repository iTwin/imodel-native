#include "ProfilesDomainTestFixture.h"
#include "ProfilesDomain\ProfilesDomainUtilities.h"
#include <BeJsonCpp/BeJsonUtilities.h>
#include <BeSQLite/BeSQLite.h>
#include <Json/Json.h>
#include <Bentley\BeAssert.h>

#include <ProfilesDomain\ConstantProfile.h>
#include <ProfilesDomain\BuiltUpProfile.h>
#include <ProfilesDomain\ParametricProfile.h>
#include <ProfilesDomain\BuiltUpProfileComponent.h>
#include <ProfilesDomain\ProfilesDomainDefinitions.h>
#include "ProfilesTestUtils.h"


#define MODEL_TEST_NAME              "SampleModel"
#define MODEL_TEST_NAME1             "SampleModelAAA"
#define MODEL_TEST_NAME2             "SampleModelBBB"
#define MODEL_TEST_NAME3             "SampleModelCCC"
#define DYNAMIC_SCHEMA_NAME          "SampleDynamic" 

BE_JSON_NAME(ProfilesDomain)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras             11/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ProfilesDomainTestsFixture, EnsureDomainsAreRegistered)
    {
    DgnDbPtr db = CreateDgnDb();

    ASSERT_TRUE(db.IsValid());

    // //This should create a DGN db with building domain.
    Profiles::ProfilesDomainUtilities::RegisterDomainHandlers();

    DgnDomainCP profilesDomain = db->Domains().FindDomain(Profiles::ProfilesDomain::GetDomain().GetDomainName());
    ASSERT_TRUE(NULL != profilesDomain);
    }

TEST_F(ProfilesDomainTestsFixture, ValidateSchema)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext(true, true);
    context->AddSchemaLocater((*db).GetSchemaLocater());

    ECN::SchemaKey refKey = ECN::SchemaKey(BENTLEY_PROFILES_SCHEMA_NAME, 1, 0);

    ECN::ECSchemaPtr refSchema = context->LocateSchema(refKey, ECN::SchemaMatchType::LatestWriteCompatible);
    ASSERT_TRUE(refSchema.IsValid());

    ASSERT_TRUE(refSchema->Validate());
    }

TEST_F(ProfilesDomainTestsFixture, CreateModel)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Profiles::ProfileDefinitionModelCPtr profilesModel = ProfilesTestUtils::CreateProfilesModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(profilesModel.IsValid());
    }

TEST_F(ProfilesDomainTestsFixture, ConstantProfileTest)
    {
    Utf8String codeValue = "CONSTANTPROFILETEST-1";
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Profiles::ProfileDefinitionModelCPtr profilesModel = ProfilesTestUtils::GetProfilesModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(profilesModel.IsValid());

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_PROFILES_AUTHORITY, *profilesModel, codeValue);

    Profiles::ConstantProfilePtr constantProfile = Profiles::ConstantProfile::Create(profilesModel);
    ASSERT_TRUE(constantProfile.IsValid());
    constantProfile->SetCode(code);

    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr persistentElement = constantProfile->Insert(&status);
    ASSERT_TRUE(persistentElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::DefinitionElementPtr queriedElement = ProfilesTestUtils::QueryByCodeValue<Dgn::DefinitionElement>(*profilesModel, codeValue);
    ASSERT_TRUE(queriedElement.IsValid());
    }

TEST_F(ProfilesDomainTestsFixture, BuiltUpProfileTest)
    {
    Utf8String codeValue = "BUILTUPPROFILETEST-1";
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Profiles::ProfileDefinitionModelCPtr profilesModel = ProfilesTestUtils::GetProfilesModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(profilesModel.IsValid());

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_PROFILES_AUTHORITY, *profilesModel, codeValue);

    Profiles::BuiltUpProfilePtr profile = Profiles::BuiltUpProfile::Create(profilesModel);
    ASSERT_TRUE(profile.IsValid());
    profile->SetCode(code);

    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr persistentElement = profile->Insert(&status);
    ASSERT_TRUE(persistentElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::DefinitionElementPtr queriedElement = ProfilesTestUtils::QueryByCodeValue<Dgn::DefinitionElement>(*profilesModel, codeValue);
    ASSERT_TRUE(queriedElement.IsValid());
    }

TEST_F(ProfilesDomainTestsFixture, ParametricProfileTest)
    {
    Utf8String codeValue = "PARAMETRICPROFILETEST-1";
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Profiles::ProfileDefinitionModelCPtr profilesModel = ProfilesTestUtils::GetProfilesModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(profilesModel.IsValid());

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_PROFILES_AUTHORITY, *profilesModel, codeValue);

    Profiles::ParametricProfilePtr profile = Profiles::ParametricProfile::Create(profilesModel);
    ASSERT_TRUE(profile.IsValid());
    profile->SetCode(code);

    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr persistentElement = profile->Insert(&status);
    ASSERT_TRUE(persistentElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::DefinitionElementPtr queriedElement = ProfilesTestUtils::QueryByCodeValue<Dgn::DefinitionElement>(*profilesModel, codeValue);
    ASSERT_TRUE(queriedElement.IsValid());
    }

/*TEST_F(ProfilesDomainTestsFixture, PublishedProfileTest)
    {
    Utf8String codeValue = "PUBLISHEDPROFILETEST-1";
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Profiles::ProfileDefinitionModelCPtr profilesModel = ProfilesTestUtils::GetProfilesModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(profilesModel.IsValid());

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_PROFILES_AUTHORITY, *profilesModel, codeValue);

    Profiles::PublishedProfilePtr profile = Profiles::PublishedProfile::Create(profilesModel);
    ASSERT_TRUE(profile.IsValid());
    profile->SetCode(code);

    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr persistentElement = profile->Insert(&status);
    ASSERT_TRUE(persistentElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::DefinitionElementPtr queriedElement = ProfilesTestUtils::QueryByCodeValue<Dgn::DefinitionElement>(*profilesModel, codeValue);
    ASSERT_TRUE(queriedElement.IsValid());
    }*/


TEST_F(ProfilesDomainTestsFixture, BuiltUpProfileComponentUsesConstantProfileTest)
    {
    Utf8String codeValue = "BUILTUPPROFILECOMPONENTUSESCONSTANTPROFILETEST-1";
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Profiles::ProfileDefinitionModelCPtr profilesModel = ProfilesTestUtils::GetProfilesModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(profilesModel.IsValid());

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_PROFILES_AUTHORITY, *profilesModel, codeValue);

    Profiles::ConstantProfilePtr constantProfile = Profiles::ConstantProfile::Create(profilesModel);
    ASSERT_TRUE(constantProfile.IsValid());
    constantProfile->SetCode(code);

    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr persistentElement = constantProfile->Insert(&status);
    ASSERT_TRUE(persistentElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::DefinitionElementPtr queriedElement = ProfilesTestUtils::QueryByCodeValue<Dgn::DefinitionElement>(*profilesModel, codeValue);
    ASSERT_TRUE(queriedElement.IsValid());


    ECN::ECClassCP  aspectClass = db->Schemas().GetClass(BENTLEY_PROFILES_SCHEMA_NAME, PROFILES_CLASS_BuiltUpProfileComponent);
    RefCountedPtr<DgnElement::MultiAspect> aspect = DgnElement::MultiAspect::CreateAspect(*db, *aspectClass);
    DgnElement::MultiAspect::AddAspect(*queriedElement, *aspect);

    ASSERT_TRUE(queriedElement->Update().IsValid());
    }

TEST_F(ProfilesDomainTestsFixture, CardinalPointsTestsPublishedProfile)
    {
    Utf8String codeValue = "CARDINALPOINTSTESTSPUBLISHEDPROFILE-1";
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Profiles::ProfileDefinitionModelCPtr profilesModel = ProfilesTestUtils::GetProfilesModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(profilesModel.IsValid());

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_PROFILES_AUTHORITY, *profilesModel, codeValue);
    ASSERT_TRUE(code.IsValid());

    Dgn::DgnDbStatus status;
    ECN::ECObjectsStatus objectStatus;
    BentleyStatus bentleyStatus;

    Profiles::ConstantProfilePtr publishedProfile = Profiles::ConstantProfile::Create(profilesModel);
    ASSERT_TRUE(publishedProfile.IsValid());

    status = publishedProfile->SetCode(code);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    DPoint2d test1 = { 0.0 };

    test1.x = 0.14000;
    test1.y = 0.14000;

    bool bret = publishedProfile->AddCardinalPoint("Test_1", test1);
    ASSERT_TRUE(false != bret);

    test1.x = 0.14000;
    test1.y = 0.15002;

    bret = publishedProfile->AddCardinalPoint("Test_2", test1);
    ASSERT_TRUE(false != bret);

    bret = publishedProfile->AddCardinalPoint("Test_2", test1);
    ASSERT_TRUE(false == bret);

    bret = publishedProfile->RemoveCardinalPoint("Test_1");
    ASSERT_TRUE(false != bret);

    bret = publishedProfile->RemoveCardinalPoint("dummy");
    ASSERT_TRUE(false == bret);

    test1.x = 6.14000;
    test1.y = 61.15002;

    bret = publishedProfile->SetCardinalPoint("Test_2", test1);
    ASSERT_TRUE(false != bret);

    bret = publishedProfile->SetCardinalPoint("Test_2_dummy", test1);
    ASSERT_TRUE(false == bret);


    bret = Profiles::Profile::IsStandardCardinalPointName("custom name");
    ASSERT_TRUE(false == bret);

    bret = Profiles::Profile::IsStandardCardinalPointName("LeftBottom");
    ASSERT_TRUE(false != bret);

    Dgn::DgnElementCPtr persistentElement = publishedProfile->Insert(&status);
    ASSERT_TRUE(persistentElement.IsValid());

    Dgn::DefinitionElementPtr queriedElement = ProfilesTestUtils::QueryByCodeValue<Dgn::DefinitionElement>(BENTLEY_PROFILES_AUTHORITY, *profilesModel, codeValue);
    ASSERT_TRUE(queriedElement.IsValid());
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    ECN::ECValue CardinalPointsValue;
    status = queriedElement->GetPropertyValue(CardinalPointsValue, "CardinalPoints");
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    ECN::ArrayInfo arrayInfo = CardinalPointsValue.GetArrayInfo();
    ASSERT_TRUE(1 == arrayInfo.GetCount());
    for (uint32_t i = 0; i < arrayInfo.GetCount(); ++i)
        {
        ECN::IECInstancePtr checkInstance;
        ECN::ECValue checkValue;
        status = queriedElement->GetPropertyValue(checkValue, "CardinalPoints", PropertyArrayIndex(i));
        ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

        if (!checkValue.IsNull())
            {
            ECN::ECValue value;
            checkInstance = checkValue.GetStruct();
            ASSERT_TRUE(checkInstance.IsValid());

            objectStatus = checkInstance->GetValue(value, "Name");
            ASSERT_TRUE(ECN::ECObjectsStatus::Success == objectStatus);

            Utf8CP name = value.GetUtf8CP();

            value.Clear();
            objectStatus = checkInstance->GetValue(value, "Coordinates");
            ASSERT_TRUE(ECN::ECObjectsStatus::Success == objectStatus);

            DPoint2d point = value.GetPoint2d();
            }
        }
    }

