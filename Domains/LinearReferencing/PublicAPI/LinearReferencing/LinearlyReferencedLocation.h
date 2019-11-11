/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "LinearReferencing.h"
#include "DistanceExpression.h"

BEGIN_BENTLEY_LINEARREFERENCING_NAMESPACE

//=======================================================================================
//! Specifies where a linearly located element occurs or where a linearly located attribution applies. 
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinearlyReferencedLocation : RefCountedBase, NonCopyableClass
{
protected:
    RefCountedCPtr<ECN::IECInstance> m_instanceCPtr;
    ECN::IECInstancePtr m_instancePtr;

    /// @private
    LinearlyReferencedLocation();
    LinearlyReferencedLocation(ECN::IECInstanceCR instance): m_instanceCPtr(&instance) {}
    LinearlyReferencedLocation(ECN::IECInstanceR instance) : m_instancePtr(&instance) {}

    virtual ECN::IECInstancePtr _ToECInstance(Dgn::DgnDbR dgnDb) const = 0;

    /// @private
    virtual LinearlyReferencedAtLocationCP _ToLinearlyReferencedAtLocation() const { return nullptr; }

    /// @private
    virtual LinearlyReferencedFromToLocationCP _ToLinearlyReferencedFromToLocation() const { return nullptr; }

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(LinearlyReferencedLocation)

    /// Convert this LinearlyReferencedLocation to a LinearlyReferencedAtLocation.
    LinearlyReferencedAtLocationCP ToLinearlyReferencedAtLocation() const { return _ToLinearlyReferencedAtLocation(); }

    /// Convert this LinearlyReferencedLocation to a LinearlyReferencedFromToLocation.
    LinearlyReferencedFromToLocationCP ToLinearlyReferencedFromToLocation() const { return _ToLinearlyReferencedFromToLocation(); }

    ECN::IECInstancePtr ToECInstance(Dgn::DgnDbR dgnDb) const { return _ToECInstance(dgnDb); }
}; // LinearlyReferencedLocation

//=======================================================================================
//! Single location whose position is specified using linear referencing.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinearlyReferencedAtLocation : LinearlyReferencedLocation
{
    DEFINE_T_SUPER(LinearlyReferencedLocation)

private:
    DistanceExpression m_atPosition;

    void PopulateInstance(ECN::IECInstanceR instance) const;
    DistanceExpression ToAtDistanceExpression(ECN::IECInstanceCR instance);

protected:
    /// @private
    LinearlyReferencedAtLocation();

    /// @private
    LinearlyReferencedAtLocation(DistanceExpressionCR atPosition);
    LinearlyReferencedAtLocation(ECN::IECInstanceCR instance) : T_Super(instance) { m_atPosition = ToAtDistanceExpression(instance); }
    LinearlyReferencedAtLocation(ECN::IECInstanceR instance) : T_Super(instance) { m_atPosition = ToAtDistanceExpression(instance); }

    virtual ECN::IECInstancePtr _ToECInstance(Dgn::DgnDbR dgnDb) const override;

    /// @private
    virtual LinearlyReferencedAtLocationCP _ToLinearlyReferencedAtLocation() const override { return this; }

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(LinearlyReferencedAtLocation)

    /// Get the read-only DistanceExpression that this Location represents along the alignment.
    DistanceExpressionCR GetAtPosition() const { return m_atPosition; }

    /// @private
    LINEARREFERENCING_EXPORT void SetAtPosition(DistanceExpressionCR);

    LINEARREFERENCING_EXPORT static LinearlyReferencedAtLocationPtr Create(DistanceExpressionCR atPosition);
    LINEARREFERENCING_EXPORT static LinearlyReferencedAtLocationCPtr Create(ECN::IECInstanceCR instance) { return new LinearlyReferencedAtLocation(instance); }
    LINEARREFERENCING_EXPORT static LinearlyReferencedAtLocationPtr Create(ECN::IECInstanceR instance) { return new LinearlyReferencedAtLocation(instance); }
}; // LinearlyReferencedAtLocation

//=======================================================================================
//! Range location whose position is specified using linear referencing.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinearlyReferencedFromToLocation : LinearlyReferencedLocation
{
    DEFINE_T_SUPER(LinearlyReferencedLocation)

private:
    DistanceExpression m_fromPosition, m_toPosition;

    void PopulateInstance(ECN::IECInstanceR instance) const;
    void ToFromToDistanceExpression(ECN::IECInstanceCR instance, DistanceExpressionR from, DistanceExpressionR to);

protected:
    /// @private
    LinearlyReferencedFromToLocation();

    /// @private
    LinearlyReferencedFromToLocation(DistanceExpressionCR fromPosition, DistanceExpressionCR toPosition);
    LinearlyReferencedFromToLocation(ECN::IECInstanceCR instance) : T_Super(instance) { ToFromToDistanceExpression(instance, m_fromPosition, m_toPosition); }
    LinearlyReferencedFromToLocation(ECN::IECInstanceR instance) : T_Super(instance) { ToFromToDistanceExpression(instance, m_fromPosition, m_toPosition); }

    virtual ECN::IECInstancePtr _ToECInstance(Dgn::DgnDbR dgnDb) const override;

    /// @private
    virtual LinearlyReferencedFromToLocationCP _ToLinearlyReferencedFromToLocation() const override { return this; }

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(LinearlyReferencedFromToLocation)

    /// Get the DistanceExpression that represents the "From" location
    DistanceExpressionCR GetFromPosition() const { return m_fromPosition; }

    /// @private
    DistanceExpressionR GetFromPositionR() { return m_fromPosition; }

    /// @private
    void SetFromPosition(DistanceExpressionCR position);

    /// Get the DistanceExpression that represents the "To" location
    DistanceExpressionCR GetToPosition() const { return m_toPosition; }

    /// @private
    DistanceExpressionR GetToPositionR() { return m_toPosition; }

    /// @private
    void SetToPosition(DistanceExpressionCR position);

    /// Create a new LinearlyReferencedFromToLocation.
    /// A LinearelyReferencedFromToLocation represents a directed range along an ILinearElement.
    LINEARREFERENCING_EXPORT static LinearlyReferencedFromToLocationPtr Create(DistanceExpressionCR fromPosition, DistanceExpressionCR toPosition);
    LINEARREFERENCING_EXPORT static LinearlyReferencedFromToLocationCPtr Create(ECN::IECInstanceCR instance) { return new LinearlyReferencedFromToLocation(instance); }
    LINEARREFERENCING_EXPORT static LinearlyReferencedFromToLocationPtr Create(ECN::IECInstanceR instance) { return new LinearlyReferencedFromToLocation(instance); }
}; // LinearlyReferencedFromToLocation

END_BENTLEY_LINEARREFERENCING_NAMESPACE
