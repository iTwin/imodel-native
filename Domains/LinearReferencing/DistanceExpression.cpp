/*--------------------------------------------------------------------------------------+
|
|     $Source: DistanceExpression.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <LinearReferencingInternal.h>

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