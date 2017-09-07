/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/LinearReferencing/LinearlyReferencedLocation.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
struct EXPORT_VTABLE_ATTRIBUTE LinearlyReferencedLocation : Dgn::DgnElement::MultiAspect
{
    DEFINE_T_SUPER(Dgn::DgnElement::MultiAspect)
    friend struct LinearlyReferencedLocationHandler;

protected:
    LinearlyReferencedLocation();

    virtual Utf8CP _GetECSchemaName() const override { return BLR_SCHEMA_NAME; }
    virtual Utf8CP _GetSuperECClassName() const override { return T_Super::_GetECClassName(); }

    virtual bool _HasChanges() const = 0;
    virtual LinearlyReferencedAtLocationCP _ToLinearlyReferencedAtLocation() const { return nullptr; }
    virtual LinearlyReferencedFromToLocationCP _ToLinearlyReferencedFromToLocation() const { return nullptr; }

    static BentleyStatus SetDistanceExpressionValue(DistanceExpressionR expression, Utf8CP ecPropertyName, ECN::ECValueCR value);
    static BentleyStatus GetECValue(ECN::ECValueR value, DistanceExpressionCR expression, Utf8CP ecPropertyName);

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(LinearlyReferencedLocation)

    bool HasChanges() const { return _HasChanges(); }
    LinearlyReferencedAtLocationCP ToLinearlyReferencedAtLocation() const { return _ToLinearlyReferencedAtLocation(); }
    LinearlyReferencedFromToLocationCP ToLinearlyReferencedFromToLocation() const { return _ToLinearlyReferencedFromToLocation(); }
}; // LinearlyReferencedLocation

//=======================================================================================
//! Single location whose position is specified using linear referencing.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinearlyReferencedAtLocation : LinearlyReferencedLocation
{
    DEFINE_T_SUPER(LinearlyReferencedLocation)
    friend struct LinearlyReferencedAtLocationHandler;

private:
    DistanceExpression m_originalAtPosition, m_atPosition;

protected:
    LinearlyReferencedAtLocation();
    LinearlyReferencedAtLocation(DistanceExpressionCR atPosition);

    virtual Utf8CP _GetECClassName() const override { return BLR_CLASS_LinearlyReferencedAtLocation; }
    virtual Utf8CP _GetSuperECClassName() const override { return T_Super::_GetECClassName(); }

    virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken) override;
    virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR el) override;

    virtual Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR value, Utf8CP propertyName, Dgn::PropertyArrayIndex const& arrayIndex) const override;
    virtual Dgn::DgnDbStatus _SetPropertyValue(Utf8CP propertyName, ECN::ECValueCR value, Dgn::PropertyArrayIndex const& arrayIndex) override;

    virtual bool _HasChanges() const override;
    virtual LinearlyReferencedAtLocationCP _ToLinearlyReferencedAtLocation() const override { return this; }

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(LinearlyReferencedAtLocation)
    DistanceExpressionCR GetAtPosition() const { return m_atPosition; }
    DistanceExpressionR GetAtPositionR() { return m_atPosition; }

    LINEARREFERENCING_EXPORT static LinearlyReferencedAtLocationPtr Create(DistanceExpressionCR atPosition);
}; // LinearlyReferencedAtLocation

//=======================================================================================
//! Range location whose position is specified using linear referencing.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinearlyReferencedFromToLocation : LinearlyReferencedLocation
{
    DEFINE_T_SUPER(LinearlyReferencedLocation)
    friend struct LinearlyReferencedFromToLocationHandler;

private:
    DistanceExpression m_originalFromPosition, m_originalToPosition;
    DistanceExpression m_fromPosition, m_toPosition;

protected:
    LinearlyReferencedFromToLocation();
    LinearlyReferencedFromToLocation(DistanceExpressionCR fromPosition, DistanceExpressionCR toPosition);

    virtual Utf8CP _GetECClassName() const override { return BLR_CLASS_LinearlyReferencedFromToLocation; }
    virtual Utf8CP _GetSuperECClassName() const override { return T_Super::_GetECClassName(); }

    virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken) override;
    virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR el) override;

    virtual Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR value, Utf8CP propertyName, Dgn::PropertyArrayIndex const& arrayIndex) const override;
    virtual Dgn::DgnDbStatus _SetPropertyValue(Utf8CP propertyName, ECN::ECValueCR value, Dgn::PropertyArrayIndex const& arrayIndex) override;

    virtual bool _HasChanges() const override;
    virtual LinearlyReferencedFromToLocationCP _ToLinearlyReferencedFromToLocation() const override { return this; }

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(LinearlyReferencedFromToLocation)
    DistanceExpressionCR GetFromPosition() const { return m_fromPosition; }
    DistanceExpressionR GetFromPositionR() { return m_fromPosition; }
    DistanceExpressionCR GetOriginalFromPosition() const { return m_originalFromPosition; }
    void SetFromPosition(DistanceExpressionCR position) { m_fromPosition = position; }
    DistanceExpressionCR GetToPosition() const { return m_toPosition; }
    DistanceExpressionR GetToPositionR() { return m_toPosition; }
    DistanceExpressionCR GetOriginalToPosition() const { return m_originalToPosition; }
    void SetToPosition(DistanceExpressionCR position) { m_toPosition = position; }

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
