#include "ProfilesDomainTestFixture.h"
#include <Profiles/ProfilesApi.h>

using namespace Dgn;
USING_NAMESPACE_BENTLEY_PROFILES


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ProfilesDomainTestsFixture, EnsureDomainsAreRegistered)
    {
    BentleyStatus registrationStatus = DgnDomains::RegisterDomain(ProfilesDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::No);
    ASSERT_TRUE(BentleyStatus::SUCCESS == registrationStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ProfilesDomainTestsFixture, EnsureBimCanBeCreated)
    {
    DgnDbPtr db = CreateDgnDb();
    ASSERT_TRUE(db.IsValid());

    DgnDomainCP profilesDomain = db->Domains().FindDomain(Profiles::ProfilesDomain::GetDomain().GetDomainName());
    ASSERT_TRUE(NULL != profilesDomain);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ProfilesDomainTestsFixture, ValidateSchema)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext(true, true);
    context->AddSchemaLocater((*db).GetSchemaLocater());

    ECN::SchemaKey refKey(PRF_SCHEMA_NAME, 1, 0);

    ECN::ECSchemaPtr refSchema = context->LocateSchema(refKey, ECN::SchemaMatchType::LatestWriteCompatible);
    ASSERT_TRUE(refSchema.IsValid());

    ASSERT_TRUE(refSchema->Validate());
    }
