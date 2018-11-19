#include "ProfilesDomainTestFixture.h"
#include <Profiles/ProfilesApi.h>

using namespace Dgn;
USING_NAMESPACE_BENTLEY_PROFILES


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfilesDomainTestsFixture, EnsureDomainsAreRegistered)
    {
    BentleyStatus registrationStatus = DgnDomains::RegisterDomain (ProfilesDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::No);
    ASSERT_TRUE (BentleyStatus::SUCCESS == registrationStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bssimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfilesDomainTestsFixture, EnsureBimCanBeCreated)
    {
    DgnDomainCP profilesDomain = GetDb().Domains().FindDomain (ProfilesDomain::GetDomain().GetDomainName());
    ASSERT_TRUE (NULL != profilesDomain);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfilesDomainTestsFixture, ValidateSchema)
    {
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext (true, true);
    context->AddSchemaLocater (GetDb().GetSchemaLocater());

    ECN::SchemaKey refKey (PRF_SCHEMA_NAME, 1, 0);

    ECN::ECSchemaPtr refSchema = context->LocateSchema (refKey, ECN::SchemaMatchType::LatestWriteCompatible);
    ASSERT_TRUE (refSchema.IsValid());

    ASSERT_TRUE (refSchema->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfilesDomainTestsFixture, InsertAndUpdateIShapeProfile)
    {
    IShapeProfilePtr profilePtr = IShapeProfile::Create (GetModel());
    ASSERT_TRUE (profilePtr.IsValid());

    DgnDbStatus status;
    profilePtr->Insert (&status);
    ASSERT_TRUE (status == DgnDbStatus::Success);

    profilePtr->SetDepth (1.0);
    double depth = profilePtr->GetDepth();
    ASSERT_TRUE (depth == 1.0);

    profilePtr->Update (&status);
    ASSERT_TRUE (status == DgnDbStatus::Success);
    }
