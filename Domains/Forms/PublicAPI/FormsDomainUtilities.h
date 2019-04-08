#pragma once

#include "..\FormsDomain\FormsDomainApi.h"
#include <Json/Json.h>

BEGIN_BENTLEY_FORMS_NAMESPACE

struct FormsDomainUtilities
    {
    FORMS_DOMAIN_EXPORT static BentleyStatus RegisterDomainHandlers();
    };

END_BENTLEY_FORMS_NAMESPACE
