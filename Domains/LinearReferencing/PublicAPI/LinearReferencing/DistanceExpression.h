/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
    //! Create a new instance of DistanceExpression with default settings
    LINEARREFERENCING_EXPORT DistanceExpression();

    //! Create a new instance of DistanceExpression
    //! @param distanceAlong The distance along the ILinearElement
    //! @param lateralOffset The lateral offset from the ILinearElement. Positive values on the right side.
    //! @param verticalOffset The vertical offset from the ILinearElement
    //! @param fromReferentId The id of the IReferent of interest
    //! @param distanceAlongFromReferent The distance along the alignment from \p fromReferentId
    LINEARREFERENCING_EXPORT DistanceExpression(double distanceAlong, NullableDouble lateralOffset = NullableDouble(), NullableDouble verticalOffset = NullableDouble(), 
        IReferentCP fromReferentId = nullptr, NullableDouble distanceAlongFromReferent = NullableDouble());

    //! Get the distance along from the start of the ILinearElement
    //! @return The distance along from the start
    double GetDistanceAlongFromStart() const { return m_distanceAlong; }    

    //! Get the lateral offset from the ILinearElement
    //! @return The lateral offset from the ILinearElement, or NullableDouble::IsNull() if not set. 
    NullableDouble GetLateralOffsetFromILinearElement() const { return m_lateralOffset; }    

    //! Get the vertical offset from the ILinearElement
    //! @return The vertical offset from the ILinearElement, or NullableDouble::IsNull() if not set.
    NullableDouble GetVerticalOffsetFromILinearElement() const { return m_verticalOffset; }

    //! Get the distace along from the IReferent
    //! @return The distance along fromt he IReferent, or NullableDouble::IsNull() if not set.
    NullableDouble GetDistanceAlongFromReferent() const { return m_distanceAlongFromReferent; }

    //! Get the id of the IReferent of interest, if any
    //! @return The DgnElementId of the IReferent.  Returns !DgnElementId::IsValid() if not set.
    Dgn::DgnElementId GetFromReferentId() const { return m_fromReferentId; }
    
    //! Set the distance along from the start of the ILinearElement, in meters.
    void SetDistanceAlongFromStart(double newVal) { m_distanceAlong = newVal; }

    //! Set the lateral offset from the ILinearElement, in meters.
    void SetLateralOffsetFromILinearElement(NullableDouble newVal) { m_lateralOffset = newVal; }

    //! Set the vertical offset from the ILinearElement, in meters.
    void SetVerticalOffsetFromILinearElement(NullableDouble newVal) { m_verticalOffset = newVal; }

    //! Set the distance along from the IReferent of interest, in meters.
    void SetDistanceAlongFromReferent(NullableDouble newVal) { m_distanceAlongFromReferent = newVal; }    

    //! Set the IReferent of interest.
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
