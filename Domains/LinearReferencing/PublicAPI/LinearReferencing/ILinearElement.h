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
    //! @private
    virtual double _GetLength() const = 0;
    //! @private
    virtual Dgn::DgnElementCR _ILinearElementToDgnElement() const = 0;

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(ILinearElement)
    //! Get a const reference to the DgnElement of this ILinearElement
    Dgn::DgnElementCR ToElement() const { return _ILinearElementToDgnElement(); }
    
    //__PUBLISH_SECTION_END__
    Dgn::DgnElementR ToElementR() { return *const_cast<Dgn::DgnElementP>(&_ILinearElementToDgnElement()); }
    //__PUBLISH_SECTION_START__

    //! Get the DgnElementId of the ILinearElementSource realizing this ILinearElement
    Dgn::DgnElementId GetILinearElementSource() const { return ToElement().GetPropertyValueId<Dgn::DgnElementId>("ILinearElementSource"); }

    //__PUBLISH_SECTION_END__
    LINEARREFERENCING_EXPORT void SetILinearElementSource(ILinearElementSourceCP);
    //__PUBLISH_SECTION_START__

    //! Get the length of the ILinearElement
    //! @return The length of this ILinearElement, in meters.
    LINEARREFERENCING_EXPORT double GetLength() const { return _GetLength(); }

    //! Get the start value of the ILinearElement
    double GetStartValue() const { return ToElement().GetPropertyValueDouble("StartValue"); }

    //__PUBLISH_SECTION_END__
    void SetStartValue(double newStartVal) { ToElementR().SetPropertyValue("StartValue", newStartVal); }
    //__PUBLISH_SECTION_START__
}; // ILinearElement

//=======================================================================================
//! Interface implemented by linear-elements that have a spatial representation.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ISpatialLinearElement : virtual ILinearElement
{
//! @privatesection
protected:
    virtual DPoint3d _ToDPoint3d(DistanceExpressionCR distanceExpression) const = 0;
    virtual DistanceExpression _ToDistanceExpression(DPoint3dCR point) const = 0;
//! @publicsection

public:
    //! Get the actual point in space of the DistanceExpression, relative to this element. 
    //! @see ToDistanceExpression
    //! @param distanceExpression The DistanceExpression whose location you want to determine.
    //! @return The DPoing3d reflecting /p distanceExpression.
    LINEARREFERENCING_EXPORT DPoint3d ToDPoint3d(DistanceExpressionCR distanceExpression) const { return _ToDPoint3d(distanceExpression); }

    //! Given a point in space, creates a DistanceExpression describing the point's relation to this ISpatialLinearElement.
    //! @see ToDPoint3d
    //! @param point The point from which to derive a DistanceExpression.
    //! @return The DistanceExpression derived from /point.
    LINEARREFERENCING_EXPORT DistanceExpression ToDistanceExpression(DPoint3dCR point) const { return _ToDistanceExpression(point); }
}; // ISpatialLinearElement

//=======================================================================================
//! Interface implemented by elements that can realize one or more ILinearElements along which
//! linear referencing is performed.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ILinearElementSource
{
protected:
    //! @private
    virtual Dgn::DgnElementCR _ILinearElementSourceToDgnElement() const = 0;

public:
    //! Get a const reference to the DgnElement of this ILinearElementSource
    Dgn::DgnElementCR ToElement() const { return _ILinearElementSourceToDgnElement(); }
    //__PUBLISH_SECTION_END__
    Dgn::DgnElementR ToElementR() { return *const_cast<Dgn::DgnElementP>(&_ILinearElementSourceToDgnElement()); }
    //__PUBLISH_SECTION_START__
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
    //! @private
    LINEARREFERENCING_EXPORT ILinearlyLocated();

    //! @private
    LINEARREFERENCING_EXPORT void _SetLinearElement(Dgn::DgnElementId elementId);
    
    //! @private
    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const = 0;

    //! @private
    LINEARREFERENCING_EXPORT void _AddLinearlyReferencedLocation(LinearlyReferencedLocationR);
    
public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(ILinearlyLocated)

    //! Get a const reference to the DgnElement of this ILinearlyLocated
    Dgn::DgnElementCR ToElement() const { return _ILinearlyLocatedToDgnElement(); }

    //! Get the DgnElementId of the LienarElement
    LINEARREFERENCING_EXPORT Dgn::DgnElementId GetLinearElementId() const;

    //! Get the LinearElement
    LINEARREFERENCING_EXPORT ILinearElementCP GetLinearElement() const;

    //! Obtain all of the LinearlyReferencedLocationIds related to this element.
    LINEARREFERENCING_EXPORT bvector<LinearlyReferencedLocationId> QueryLinearlyReferencedLocationIds() const;

    //! Given a LinearlyReferencedLocationId (see QueryLinearlyReferencedLocationIds()), obtain its LinearlyReferencedLocation.
    LINEARREFERENCING_EXPORT LinearlyReferencedLocationCP GetLinearlyReferencedLocation(LinearlyReferencedLocationId) const;
    
    //! Given a LinearlyReferencedLocationId (see QueryLinearlyReferencedLocationIds()), get its LinearlyReferencedAtLocation.
    LINEARREFERENCING_EXPORT LinearlyReferencedAtLocationCP GetLinearlyReferencedAtLocation(LinearlyReferencedLocationId) const;
    
    //! Given a LinearlyReferencedLocationId (see QueryLinearlyReferencedLocationIds()), get its LinearlyReferencedLocationId.
    LINEARREFERENCING_EXPORT LinearlyReferencedFromToLocationCP GetLinearlyReferencedFromToLocation(LinearlyReferencedLocationId) const;

    //__PUBLISH_SECTION_END__
    Dgn::DgnElementR ToElementR() { return *const_cast<Dgn::DgnElementP>(&_ILinearlyLocatedToDgnElement()); }
    LINEARREFERENCING_EXPORT LinearlyReferencedFromToLocationP GetLinearlyReferencedFromToLocationP(LinearlyReferencedLocationId);
    LINEARREFERENCING_EXPORT LinearlyReferencedAtLocationP GetLinearlyReferencedAtLocationP(LinearlyReferencedLocationId);
    LINEARREFERENCING_EXPORT LinearlyReferencedLocationP GetLinearlyReferencedLocationP(LinearlyReferencedLocationId);
    //__PUBLISH_SECTION_START__
    
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
//! Interface implemented by elements linearly located along an ILinearElement realized
//! by an ILinearElementSource.
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
    //! @private
    LINEARREFERENCING_EXPORT ILinearlyLocatedSingleAt() {}

    //! @private
    LINEARREFERENCING_EXPORT ILinearlyLocatedSingleAt(double atDistanceAlong);

    //! @private
    LinearReferencing::LinearlyReferencedAtLocationPtr _GetUnpersistedAtLocation() const { return m_unpersistedAtLocationPtr; }

public:
    //! Get the distance along of this LinearLocated from the start of the ILinearElement
    LINEARREFERENCING_EXPORT double GetAtDistanceAlongFromStart() const;

    //! @private
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
    //! @private
    LINEARREFERENCING_EXPORT ILinearlyLocatedSingleFromTo() {}

    //! @private
    LINEARREFERENCING_EXPORT ILinearlyLocatedSingleFromTo(double fromDistanceAlong, double toDistanceAlong);

    //! @private
    LinearReferencing::LinearlyReferencedFromToLocationPtr _GetUnpersistedFromToLocation() const { return m_unpersistedFromToLocationPtr; }

public:
    //! Get the "From" distance from the start.
    LINEARREFERENCING_EXPORT double GetFromDistanceAlongFromStart() const;
    
    //! Set the "From" distance from the start.
    LINEARREFERENCING_EXPORT void SetFromDistanceAlongFromStart(double newFrom);

    //! Get the "To" distance from the start.
    LINEARREFERENCING_EXPORT double GetToDistanceAlongFromStart() const;

    //! Set the "To" distance from the start.
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
    //! @private
    virtual NullableDouble _GetRestartValue() const = 0;

    //! @private
    virtual Dgn::DgnElementCR _IReferentToDgnElement() const = 0;

public:
    //! Get the DgnElement from this IReferent
    Dgn::DgnElementCR ToElement() const { return _IReferentToDgnElement(); }

    //__PUBLISH_SECTION_END__
    Dgn::DgnElementR ToElementR() { return *const_cast<Dgn::DgnElementP>(&_IReferentToDgnElement()); }
    //__PUBLISH_SECTION_START__

    //! Ge the Restart Value from this IReferent.
    LINEARREFERENCING_EXPORT NullableDouble GetRestartValue() const { return _GetRestartValue(); }    
}; // IReferent

END_BENTLEY_LINEARREFERENCING_NAMESPACE