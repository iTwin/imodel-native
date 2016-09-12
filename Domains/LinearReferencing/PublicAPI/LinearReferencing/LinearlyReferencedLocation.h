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
//! Specifies where a linearly located element occurs or where a linearly located attribution applies. 
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinearlyReferencedLocation : Dgn::DgnElement::MultiAspect
{
    DEFINE_T_SUPER(Dgn::DgnElement::MultiAspect)

protected:
    LinearlyReferencedLocation();

    virtual Utf8CP _GetECSchemaName() const override { return BLR_SCHEMA_NAME; }
}; // LinearlyReferencedLocation

//=======================================================================================
//! Single location whose position is specified using linear referencing.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinearlyReferencedAtLocation : LinearlyReferencedLocation
{
    DEFINE_T_SUPER(LinearlyReferencedAtLocation)
    friend struct LinearlyReferencedAtLocationHandler;

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
//! Range location whose position is specified using linear referencing.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinearlyReferencedFromToLocation : LinearlyReferencedLocation
{
    DEFINE_T_SUPER(LinearlyReferencedFromToLocation)
    friend struct LinearlyReferencedFromToLocationHandler;

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


//=======================================================================================
//! Handler for LinearlyReferencedAtLocation Aspects
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinearlyReferencedAtLocationHandler : Dgn::dgn_AspectHandler::Aspect
{
DOMAINHANDLER_DECLARE_MEMBERS(BLR_CLASS_LinearlyReferencedAtLocation, LinearlyReferencedAtLocationHandler, Dgn::dgn_AspectHandler::Aspect, LINEARREFERENCING_EXPORT)

protected:
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new LinearlyReferencedAtLocation(); }
}; // LinearlyReferencedAtLocationHandler


//=======================================================================================
//! Handler for LinearlyReferencedFromToLocation Aspects
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinearlyReferencedFromToLocationHandler : Dgn::dgn_AspectHandler::Aspect
{
DOMAINHANDLER_DECLARE_MEMBERS(BLR_CLASS_LinearlyReferencedFromToLocation, LinearlyReferencedFromToLocationHandler, Dgn::dgn_AspectHandler::Aspect, LINEARREFERENCING_EXPORT)

protected:
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new LinearlyReferencedFromToLocation(); }
}; // LinearlyReferencedFromToLocationHandler

END_BENTLEY_LINEARREFERENCING_NAMESPACE