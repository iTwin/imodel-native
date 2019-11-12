/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "..\FormsDomain\FormsDomainApi.h"
#include <Json/Json.h>

BEGIN_BENTLEY_FORMS_NAMESPACE

struct FormsDomainUtilities
    {
    FORMS_DOMAIN_EXPORT static BentleyStatus RegisterDomainHandlers();
    };

END_BENTLEY_FORMS_NAMESPACE
