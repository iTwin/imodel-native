/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/LinearReferencing/ILinearElement.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "LinearReferencing.h"
#include "LinearlyReferencedLocation.h"

BEGIN_BENTLEY_LINEARREFERENCING_NAMESPACE

//=======================================================================================
//! Interface implemented by elements that can be used as a scale 
//! along which linear referencing is performed.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ILinearElement
{
protected:
    virtual double _GetLength() const = 0;
    virtual Dgn::DgnElementCR _ILinearElementToDgnElement() const = 0;

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(ILinearElement)
    Dgn::DgnElementCR ToElement() const { return _ILinearElementToDgnElement(); }
    Dgn::DgnElementR ToElementR() { return *const_cast<Dgn::DgnElementP>(&_ILinearElementToDgnElement()); }

    Dgn::DgnElementId GetILinearElementSource() const { return ToElement().GetPropertyValueId<Dgn::DgnElementId>("ILinearElementSource"); }
    LINEARREFERENCING_EXPORT void SetILinearElementSource(ILinearElementSourceCP);

    LINEARREFERENCING_EXPORT double GetLength() const { return _GetLength(); }
    double GetStartValue() const { return ToElement().GetPropertyValueDouble("StartValue"); }
    void SetStartValue(double newStartVal) { ToElementR().SetPropertyValue("StartValue", newStartVal); }
}; // ILinearElement

//=======================================================================================
//! Interface implemented by linear-elements that have a spatial representation.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ISpatialLinearElement : virtual ILinearElement
{
protected:
    virtual DPoint3d _ToDPoint3d(DistanceExpressionCR distanceExpression) const = 0;
    virtual DistanceExpression _ToDistanceExpression(DPoint3dCR point) const = 0;

public:
    LINEARREFERENCING_EXPORT DPoint3d ToDPoint3d(DistanceExpressionCR distanceExpression) const { return _ToDPoint3d(distanceExpression); }
    LINEARREFERENCING_EXPORT DistanceExpression ToDistanceExpression(DPoint3dCR point) const { return _ToDistanceExpression(point); }
}; // ISpatialLinearElement

//=======================================================================================
//! Interface implemented by elements that can realize Linear-Elements along which
//! linear referencing is performed.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ILinearElementSource
{
protected:
    virtual Dgn::DgnElementCR _ILinearElementSourceToDgnElement() const = 0;

public:
    Dgn::DgnElementCR ToElement() const { return _ILinearElementSourceToDgnElement(); }
    Dgn::DgnElementR ToElementR() { return *const_cast<Dgn::DgnElementP>(&_ILinearElementSourceToDgnElement()); }
    LINEARREFERENCING_EXPORT bset<Dgn::DgnElementId> QueryLinearElements() const;
}; // ILinearElementSource

typedef BeSQLite::EC::ECInstanceId LinearlyReferencedLocationId;

//=======================================================================================
//! Base interface for linearly-located elements and attributions.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ILinearlyLocated
{
protected:
    LINEARREFERENCING_EXPORT ILinearlyLocated();

    LINEARREFERENCING_EXPORT void _SetLinearElement(Dgn::DgnElementId elementId);
    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const = 0;
    LINEARREFERENCING_EXPORT void _AddLinearlyReferencedLocation(LinearlyReferencedLocationR);
    
public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(ILinearlyLocated)
    Dgn::DgnElementCR ToElement() const { return _ILinearlyLocatedToDgnElement(); }
    Dgn::DgnElementR ToElementR() { return *const_cast<Dgn::DgnElementP>(&_ILinearlyLocatedToDgnElement()); }

    LINEARREFERENCING_EXPORT Dgn::DgnElementId GetLinearElementId() const;
    LINEARREFERENCING_EXPORT ILinearElementCP GetLinearElement() const;
    LINEARREFERENCING_EXPORT bvector<LinearlyReferencedLocationId> QueryLinearlyReferencedLocationIds() const;
    LINEARREFERENCING_EXPORT LinearlyReferencedLocationCP GetLinearlyReferencedLocation(LinearlyReferencedLocationId) const;
    LINEARREFERENCING_EXPORT LinearlyReferencedLocationP GetLinearlyReferencedLocationP(LinearlyReferencedLocationId);
    LINEARREFERENCING_EXPORT LinearlyReferencedAtLocationCP GetLinearlyReferencedAtLocation(LinearlyReferencedLocationId) const;
    LINEARREFERENCING_EXPORT LinearlyReferencedAtLocationP GetLinearlyReferencedAtLocationP(LinearlyReferencedLocationId);
    LINEARREFERENCING_EXPORT LinearlyReferencedFromToLocationCP GetLinearlyReferencedFromToLocation(LinearlyReferencedLocationId) const;
    LINEARREFERENCING_EXPORT LinearlyReferencedFromToLocationP GetLinearlyReferencedFromToLocationP(LinearlyReferencedLocationId);
}; // ILinearlyLocated

//=======================================================================================
//! Interface implemented by attribution-elements, representing values of a property
//! of an ILinearElementSource which may apply to only part of it.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ILinearlyLocatedAttribution : ILinearlyLocated
{
    DEFINE_T_SUPER(ILinearlyLocated)

protected:
    LINEARREFERENCING_EXPORT ILinearlyLocatedAttribution();
}; // ILinearlyLocatedAttribution

//=======================================================================================
//! Interface implemented by elements lineraly located existing along 
//! an ILinearElementSource.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ILinearlyLocatedElement : ILinearlyLocated
{
    DEFINE_T_SUPER(ILinearlyLocated)

protected:
    LINEARREFERENCING_EXPORT ILinearlyLocatedElement();
}; // ILinearlyLocatedElement

//=======================================================================================
//! Helper interface to be implemented by linearly located attribution and elements
//! only accepting and exposing one "at" linearly referenced location.
//! Concrete subclasses are expected to explicitely add the single aspect on their
//! constructor as follows:
//! _AddLinearlyReferencedLocation(*_GetUnpersistedAtLocation());
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct ILinearlyLocatedSingleAt
{
private:
    mutable LinearReferencing::LinearlyReferencedLocationId m_atLocationAspectId;
    LinearReferencing::LinearlyReferencedAtLocationPtr m_unpersistedAtLocationPtr;

    virtual Dgn::DgnElementCR ToElement() const { return *dynamic_cast<Dgn::DgnElementCP>(this); }
    virtual Dgn::DgnElementR ToElementR() { return *dynamic_cast<Dgn::DgnElementP>(this); }
    virtual LinearReferencing::ILinearlyLocatedCR ToLinearlyLocated() const { return *dynamic_cast<LinearReferencing::ILinearlyLocatedCP>(this); }
    virtual LinearReferencing::ILinearlyLocatedR ToLinearlyLocatedR() { return *dynamic_cast<LinearReferencing::ILinearlyLocatedP>(this); }

protected:
    LINEARREFERENCING_EXPORT ILinearlyLocatedSingleAt() {}

    LINEARREFERENCING_EXPORT ILinearlyLocatedSingleAt(double atDistanceAlong);

    LinearReferencing::LinearlyReferencedAtLocationPtr _GetUnpersistedAtLocation() const { return m_unpersistedAtLocationPtr; }

public:
    LINEARREFERENCING_EXPORT double GetAtDistanceAlongFromStart() const;
    LINEARREFERENCING_EXPORT void SetAtDistanceAlongFromStart(double newAt);
}; // ILinearlyLocatedSingleAt


//=======================================================================================
//! Helper interface to be implemented by linearly located attribution and elements
//! only accepting and exposing one from-to linearly referenced location.
//! Concrete subclasses are expected to explicitely add the single aspect on their
//! constructor as follows:
//! _AddLinearlyReferencedLocation(*_GetUnpersistedFromToLocation());
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct ILinearlyLocatedSingleFromTo
{
private:
    mutable LinearReferencing::LinearlyReferencedLocationId m_fromToLocationAspectId;
    LinearReferencing::LinearlyReferencedFromToLocationPtr m_unpersistedFromToLocationPtr;

    virtual Dgn::DgnElementCR ToElement() const { return *dynamic_cast<Dgn::DgnElementCP>(this); }
    virtual Dgn::DgnElementR ToElementR() { return *dynamic_cast<Dgn::DgnElementP>(this); }
    virtual LinearReferencing::ILinearlyLocatedCR ToLinearlyLocated() const { return *dynamic_cast<LinearReferencing::ILinearlyLocatedCP>(this); }
    virtual LinearReferencing::ILinearlyLocatedR ToLinearlyLocatedR() { return *dynamic_cast<LinearReferencing::ILinearlyLocatedP>(this); }

protected:
    LINEARREFERENCING_EXPORT ILinearlyLocatedSingleFromTo() {}

    LINEARREFERENCING_EXPORT ILinearlyLocatedSingleFromTo(double fromDistanceAlong, double toDistanceAlong);

    LinearReferencing::LinearlyReferencedFromToLocationPtr _GetUnpersistedFromToLocation() const { return m_unpersistedFromToLocationPtr; }

public:
    LINEARREFERENCING_EXPORT double GetFromDistanceAlongFromStart() const;
    LINEARREFERENCING_EXPORT void SetFromDistanceAlongFromStart(double newFrom);

    LINEARREFERENCING_EXPORT double GetToDistanceAlongFromStart() const;
    LINEARREFERENCING_EXPORT void SetToDistanceAlongFromStart(double newFrom);
}; // ILinearlyLocatedSingleFromTo

//=======================================================================================
//! Interface implemented by elements representing known locations along 
//! an ILinearElement.
//! @see ILinearElement
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IReferent
{
protected:
    virtual NullableDouble _GetRestartValue() const = 0;
    virtual Dgn::DgnElementCR _IReferentToDgnElement() const = 0;

public:
    Dgn::DgnElementCR ToElement() const { return _IReferentToDgnElement(); }
    Dgn::DgnElementR ToElementR() { return *const_cast<Dgn::DgnElementP>(&_IReferentToDgnElement()); }

    LINEARREFERENCING_EXPORT NullableDouble GetRestartValue() const { return _GetRestartValue(); }    
}; // IReferent

END_BENTLEY_LINEARREFERENCING_NAMESPACE