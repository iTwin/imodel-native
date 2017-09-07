/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/LinearReferencing/DistanceExpression.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "LinearReferencing.h"

BEGIN_BENTLEY_LINEARREFERENCING_NAMESPACE

//=======================================================================================
//! Data structure used as a data-holder of a measured value which defines 
//! the location along the linear element, and optionally offset from it.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct DistanceExpression
{
    friend struct LinearlyReferencedAtLocation;
    friend struct LinearlyReferencedFromToLocation;

private:
    double m_distanceAlong;
    NullableDouble m_lateralOffset, m_verticalOffset, m_distanceAlongFromReferent;
    Dgn::DgnElementId m_fromReferentId;

    DistanceExpression(double distanceAlong, NullableDouble lateralOffset, NullableDouble verticalOffset,
        Dgn::DgnElementId fromReferentId, NullableDouble distanceAlongFromReferent);

public:
    LINEARREFERENCING_EXPORT DistanceExpression();
    LINEARREFERENCING_EXPORT DistanceExpression(double distanceAlong, NullableDouble lateralOffset = NullableDouble(), NullableDouble verticalOffset = NullableDouble(), 
        IReferentCP fromReferentId = nullptr, NullableDouble distanceAlongFromReferent = NullableDouble());

    double GetDistanceAlongFromStart() const { return m_distanceAlong; }    
    NullableDouble GetLateralOffsetFromILinearElement() const { return m_lateralOffset; }    
    NullableDouble GetVerticalOffsetFromILinearElement() const { return m_verticalOffset; }
    NullableDouble GetDistanceAlongFromReferent() const { return m_distanceAlongFromReferent; }
    Dgn::DgnElementId GetFromReferentId() const { return m_fromReferentId; }
    
    void SetDistanceAlongFromStart(double newVal) { m_distanceAlong = newVal; }
    void SetLateralOffsetFromILinearElement(NullableDouble newVal) { m_lateralOffset = newVal; }
    void SetVerticalOffsetFromILinearElement(NullableDouble newVal) { m_verticalOffset = newVal; }
    void SetDistanceAlongFromReferent(NullableDouble newVal) { m_distanceAlongFromReferent = newVal; }    
    LINEARREFERENCING_EXPORT void SetFromReferent(IReferentCP fromReferent);
}; // DistanceExpression

BEGIN_UNNAMED_NAMESPACE
inline bool Compare(const NullableDouble& lhs, const NullableDouble& rhs)
    {
    return (lhs.IsNull() == rhs.IsNull() ||
            (lhs.IsValid() && rhs.IsValid() && fabs(lhs.Value() - rhs.Value()) < DBL_EPSILON));
    }
END_UNNAMED_NAMESPACE

inline bool operator==(const DistanceExpression& lhs, const DistanceExpression& rhs) 
    {
    return (fabs(lhs.GetDistanceAlongFromStart() - rhs.GetDistanceAlongFromStart()) < DBL_EPSILON && 
        Compare(lhs.GetLateralOffsetFromILinearElement(), rhs.GetLateralOffsetFromILinearElement()) &&
        Compare(lhs.GetVerticalOffsetFromILinearElement(), rhs.GetVerticalOffsetFromILinearElement()) &&
        Compare(lhs.GetDistanceAlongFromReferent(), rhs.GetDistanceAlongFromReferent()) &&
        lhs.GetFromReferentId() == rhs.GetFromReferentId());
    }

inline bool operator!=(const DistanceExpression& lhs, const DistanceExpression& rhs)
    {
    return (fabs(lhs.GetDistanceAlongFromStart() - rhs.GetDistanceAlongFromStart()) > DBL_EPSILON ||
        !Compare(lhs.GetLateralOffsetFromILinearElement(), rhs.GetLateralOffsetFromILinearElement()) ||
        !Compare(lhs.GetVerticalOffsetFromILinearElement(), rhs.GetVerticalOffsetFromILinearElement()) ||
        !Compare(lhs.GetDistanceAlongFromReferent(), rhs.GetDistanceAlongFromReferent()) ||
        lhs.GetFromReferentId() != rhs.GetFromReferentId());
    }

END_BENTLEY_LINEARREFERENCING_NAMESPACE
