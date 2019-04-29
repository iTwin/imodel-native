/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "LinearReferencingInternal.h"
#include <LinearReferencing/DistanceExpression.h>
#include <LinearReferencing/ILinearElement.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DistanceExpression::DistanceExpression():
    m_distanceAlong(0.0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DistanceExpression::DistanceExpression(double distanceAlong, NullableDouble lateralOffset, NullableDouble verticalOffset, 
    DgnElementId fromReferentId, NullableDouble distanceAlongFromReferent):
    m_distanceAlong(distanceAlong), m_lateralOffset(lateralOffset), m_verticalOffset(verticalOffset), 
    m_fromReferentId(fromReferentId), m_distanceAlongFromReferent(distanceAlongFromReferent)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DistanceExpression::DistanceExpression(double distanceAlong, NullableDouble lateralOffset, NullableDouble verticalOffset, 
    IReferentCP fromReferentCP, NullableDouble distanceAlongFromReferent):
    m_distanceAlong(distanceAlong), m_lateralOffset(lateralOffset), m_verticalOffset(verticalOffset), 
    m_fromReferentId(fromReferentCP == nullptr ? DgnElementId() : fromReferentCP->ToElement().GetElementId()), 
    m_distanceAlongFromReferent(distanceAlongFromReferent)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DistanceExpression::SetFromReferent(IReferentCP fromReferent)
    {
    m_fromReferentId = (fromReferent) ? fromReferent->ToElement().GetElementId() : Dgn::DgnElementId();
    }
