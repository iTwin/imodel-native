/*--------------------------------------------------------------------------------------+
|
|     $Source: LinearlyReferencedLocation.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <LinearReferencingInternal.h>

HANDLER_DEFINE_MEMBERS(LinearlyReferencedAtLocationHandler)
HANDLER_DEFINE_MEMBERS(LinearlyReferencedFromToLocationHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedLocation::LinearlyReferencedLocation()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedAtLocation::LinearlyReferencedAtLocation()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedAtLocation::LinearlyReferencedAtLocation(DistanceExpressionCR atPosition):
    m_atPosition(atPosition)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedAtLocationPtr LinearlyReferencedAtLocation::Create(DistanceExpressionCR atPosition)
    {
    return new LinearlyReferencedAtLocation(atPosition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus LinearlyReferencedAtLocation::_UpdateProperties(DgnElementCR el)
    {
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus LinearlyReferencedAtLocation::_LoadProperties(DgnElementCR el)
    {
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedFromToLocation::LinearlyReferencedFromToLocation()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedFromToLocation::LinearlyReferencedFromToLocation(DistanceExpressionCR fromPosition, DistanceExpressionCR toPosition):
    m_fromPosition(fromPosition), m_toPosition(toPosition)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedFromToLocationPtr LinearlyReferencedFromToLocation::Create(DistanceExpressionCR fromPosition, DistanceExpressionCR toPosition)
    {
    return new LinearlyReferencedFromToLocation(fromPosition, toPosition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus LinearlyReferencedFromToLocation::_UpdateProperties(DgnElementCR el)
    {
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus LinearlyReferencedFromToLocation::_LoadProperties(DgnElementCR el)
    {
    return DgnDbStatus::Success;
    }