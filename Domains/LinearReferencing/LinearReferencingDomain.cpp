/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "LinearReferencingInternal.h"
#include <LinearReferencing/LinearReferencingDomain.h>
#include <LinearReferencing/LinearlyLocated.h>
#include <LinearReferencing/LinearlyReferencedLocation.h>
#include <DgnPlatform/DgnPlatformLib.h>

DOMAIN_DEFINE_MEMBERS(LinearReferencingDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearReferencingDomain::LinearReferencingDomain() : DgnDomain(BLR_SCHEMA_NAME, "Bentley LinearReferencing Domain", 2)
    {
    }
