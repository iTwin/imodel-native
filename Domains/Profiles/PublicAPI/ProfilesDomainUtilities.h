#pragma once

#include "..\ProfilesDomain\ProfilesDomainApi.h"
#include <Json/Json.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

struct ProfilesDomainUtilities
    {
    PROFILES_DOMAIN_EXPORT static BentleyStatus RegisterDomainHandlers();
    };

END_BENTLEY_PROFILES_NAMESPACE
