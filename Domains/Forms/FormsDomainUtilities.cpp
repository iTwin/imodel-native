/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "FormsDomain\FormsDomainUtilities.h"

BE_JSON_NAME(FormsDomain)

BEGIN_BENTLEY_FORMS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus FormsDomainUtilities::RegisterDomainHandlers()
    {
    
    BentleyStatus registrationStatus = Dgn::DgnDomains::RegisterDomain(FormsDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No);
    BeAssert(BentleyStatus::SUCCESS == registrationStatus);

    return registrationStatus;
    }


END_BENTLEY_FORMS_NAMESPACE
