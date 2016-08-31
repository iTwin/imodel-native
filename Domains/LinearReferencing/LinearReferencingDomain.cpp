/*--------------------------------------------------------------------------------------+
|
|     $Source: LinearReferencingDomain.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <LinearReferencingInternal.h>

DOMAIN_DEFINE_MEMBERS(LinearReferencingDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearReferencingDomain::LinearReferencingDomain() : DgnDomain(BLR_SCHEMA_NAME, "Bentley LinearReferencing Domain", 1)
    {
    RegisterHandler(LinearlyReferencedAtLocationHandler::GetHandler());
    RegisterHandler(LinearlyReferencedFromToLocationHandler::GetHandler());
    }
