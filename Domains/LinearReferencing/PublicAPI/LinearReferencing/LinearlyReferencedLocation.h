/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/LinearReferencing/LinearlyReferencedLocation.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "LinearReferencingApi.h"

BEGIN_BENTLEY_LINEARREFERENCING_NAMESPACE

//=======================================================================================
//! Interface implemented by attribution-elements, representing values of a property
//! of an ILinearElementSource which may apply to only part of it.
//=======================================================================================
struct LinearlyReferencedLocation : Dgn::DgnElement::MultiAspect
{
DEFINE_T_SUPER(Dgn::DgnElement::MultiAspect)

protected:
    LinearlyReferencedLocation();

    virtual Utf8CP _GetECSchemaName() const override { return BLR_SCHEMA_NAME; }
}; // LinearlyReferencedLocation

//=======================================================================================
//! Interface implemented by attribution-elements, representing values of a property
//! of an ILinearElementSource which may apply to only part of it.
//=======================================================================================
struct LinearlyReferencedAtLocation : LinearlyReferencedLocation
{
DEFINE_T_SUPER(LinearlyReferencedAtLocation)

private:
    DistanceExpression m_atPosition;

protected:
    LinearlyReferencedAtLocation();
    LinearlyReferencedAtLocation(DistanceExpressionCR atPosition);

    virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el) override;
    virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR el) override;

public:
    LINEARREFERENCING_EXPORT DistanceExpressionCR GetAtPosition() const { return m_atPosition; }

    LINEARREFERENCING_EXPORT static LinearlyReferencedAtLocationPtr Create(DistanceExpressionCR atPosition);
}; // LinearlyReferencedAtLocation

//=======================================================================================
//! Interface implemented by attribution-elements, representing values of a property
//! of an ILinearElementSource which may apply to only part of it.
//=======================================================================================
struct LinearlyReferencedFromToLocation : LinearlyReferencedLocation
{
DEFINE_T_SUPER(LinearlyReferencedFromToLocation)

private:
    DistanceExpression m_fromPosition, m_toPosition;

protected:
    LinearlyReferencedFromToLocation();
    LinearlyReferencedFromToLocation(DistanceExpressionCR fromPosition, DistanceExpressionCR toPosition);

    virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el) override;
    virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR el) override;

public:
    LINEARREFERENCING_EXPORT DistanceExpressionCR GetFromPosition() const { return m_fromPosition; }
    LINEARREFERENCING_EXPORT DistanceExpressionCR GetToPosition() const { return m_toPosition; }

    LINEARREFERENCING_EXPORT static LinearlyReferencedFromToLocationPtr Create(DistanceExpressionCR fromPosition, DistanceExpressionCR toPosition);
}; // LinearlyReferencedFromToLocation

END_BENTLEY_LINEARREFERENCING_NAMESPACE