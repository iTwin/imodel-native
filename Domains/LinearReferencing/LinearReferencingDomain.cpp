/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "LinearReferencingInternal.h"
#include <LinearReferencing/LinearReferencingDomain.h>
#include <LinearReferencing/LinearlyLocated.h>
#include <LinearReferencing/LinearlyReferencedLocation.h>

DOMAIN_DEFINE_MEMBERS(LinearReferencingDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearReferencingDomain::LinearReferencingDomain() : DgnDomain(BLR_SCHEMA_NAME, "Bentley LinearReferencing Domain", 2)
    {
    RegisterHandler(LinearlyReferencedAtLocationHandler::GetHandler());
    RegisterHandler(LinearlyReferencedFromToLocationHandler::GetHandler());

    RegisterHandler(LinearlyLocatedAttributionHandler::GetHandler());
    RegisterHandler(LinearLocationElementHandler::GetHandler());
    RegisterHandler(LinearLocationHandler::GetHandler());
    RegisterHandler(LinearPhysicalElementHandler::GetHandler());

    RegisterHandler(ReferentElementHandler::GetHandler());
    RegisterHandler(ReferentHandler::GetHandler());
    }