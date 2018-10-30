#include "ProfilesDomainTestFixture.h"
#include <BeJsonCpp/BeJsonUtilities.h>
#include <BeSQLite/BeSQLite.h>
#include <Json/Json.h>
#include <Bentley\BeAssert.h>

#include <Profiles/ProfilesApi.h>

using namespace Dgn;
USING_NAMESPACE_BENTLEY_PROFILES


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ProfilesDomainTestsFixture, EnsureDomainsAreRegistered)
    {
    BentleyStatus registrationStatus = Dgn::DgnDomains::RegisterDomain(ProfilesDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::No);
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
