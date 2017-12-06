#include "PublicAPI\ProfilesDomain\ProfilesDomainUtilities.h"

BE_JSON_NAME(ProfilesDomain)

BEGIN_BENTLEY_PROFILES_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus ProfilesDomainUtilities::RegisterDomainHandlers()
    {
    
    BentleyStatus registrationStatus = Dgn::DgnDomains::RegisterDomain(ProfilesDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No);
    BeAssert(BentleyStatus::SUCCESS == registrationStatus);

    return registrationStatus;
    };


END_BENTLEY_PROFILES_NAMESPACE
