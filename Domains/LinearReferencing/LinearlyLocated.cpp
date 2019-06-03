/*--------------------------------------------------------------------------------------+
|
|     $Source: LinearlyLocated.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "LinearReferencingInternal.h"
#include <LinearReferencing/LinearlyLocated.h>

HANDLER_DEFINE_MEMBERS(LinearLocationHandler)
HANDLER_DEFINE_MEMBERS(LinearLocationElementHandler)
HANDLER_DEFINE_MEMBERS(LinearPhysicalElementHandler)
HANDLER_DEFINE_MEMBERS(LinearlyLocatedAttributionHandler)
HANDLER_DEFINE_MEMBERS(ReferentElementHandler)
HANDLER_DEFINE_MEMBERS(ReferentHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ReferentPtr Referent::Create(Dgn::SpatialElementCR referencedElement, CreateAtParams const& atParams)
    {
    return nullptr;
    }