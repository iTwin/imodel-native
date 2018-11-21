#include "ProfilesTestCase.h"
#include <Profiles/ProfilesApi.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_PROFILES

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct DomainTestCase : ProfilesTestCase
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DomainTestCase, EnsureDomainsAreRegistered)
    {
    BentleyStatus registrationStatus = DgnDomains::RegisterDomain (ProfilesDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::No);
    ASSERT_TRUE (BentleyStatus::SUCCESS == registrationStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bssimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DomainTestCase, EnsureProfilesDomainIsPresentInBim)
    {
    DgnDomainCP profilesDomain = GetDb().Domains().FindDomain (ProfilesDomain::GetDomain().GetDomainName());
    ASSERT_TRUE (NULL != profilesDomain);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DomainTestCase, ValidateSchema)
    {
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext (true, true);
    context->AddSchemaLocater (GetDb().GetSchemaLocater());

    ECN::SchemaKey refKey (PRF_SCHEMA_NAME, 1, 0);

    ECN::ECSchemaPtr refSchema = context->LocateSchema (refKey, ECN::SchemaMatchType::LatestWriteCompatible);
    ASSERT_TRUE (refSchema.IsValid());

    ASSERT_TRUE (refSchema->Validate());
    }
