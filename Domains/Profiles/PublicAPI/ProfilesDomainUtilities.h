#pragma once

#include "..\ProfilesDomain\ProfilesDomainApi.h"
#include <Json/Json.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

struct ProfilesDomainUtilities
    {
    PROFILES_DOMAIN_EXPORT static BentleyStatus RegisterDomainHandlers();
    PROFILES_DOMAIN_EXPORT ProfileDefinitionModelPtr static CreateProfilesModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject);
    PROFILES_DOMAIN_EXPORT ProfileDefinitionModelPtr static GetProfilesModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject);
    PROFILES_DOMAIN_EXPORT static Utf8String BuildDefinitionModelCode(Utf8StringCR modelCodeName);
    };

END_BENTLEY_PROFILES_NAMESPACE
