/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "LinearReferencing.h"
#include "DistanceExpression.h"
#include "LinearlyReferencedLocation.h"

BEGIN_BENTLEY_LINEARREFERENCING_NAMESPACE

typedef BeSQLite::EC::ECInstanceId LinearlyReferencedLocationId;

//=======================================================================================
//! Reference to a LinearlyReferencedLocation used to return the result of
//! querying for locations along an ILinearElement.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct LinearLocationReference
{
friend struct ILinearElement;

private:
    double m_startDistanceAlong, m_stopDistanceAlong;
    Dgn::DgnElementId m_linearlyLocatedId;
    Dgn::DgnClassId m_linearlyLocatedClassId;
    LinearlyReferencedLocationId m_locationId;

    LinearLocationReference() :  m_startDistanceAlong(0), m_stopDistanceAlong(0),
        m_linearlyLocatedId(Dgn::DgnElementId()), m_linearlyLocatedClassId(Dgn::DgnClassId()), m_locationId(LinearlyReferencedLocationId()) {}

public:
    LinearLocationReference(Dgn::DgnElementId linearlyLocatedId, Dgn::DgnClassId linearlyLocatedClassId, double startDistanceAlong, double stopDistanceAlong,
        LinearlyReferencedLocationId locationId) :
        m_linearlyLocatedId(linearlyLocatedId), m_linearlyLocatedClassId(linearlyLocatedClassId),
        m_startDistanceAlong(startDistanceAlong), m_stopDistanceAlong(stopDistanceAlong), m_locationId(locationId)
        {}

    LinearLocationReference(ILinearlyLocatedCR linearlyLocated, double startDistanceAlong, double stopDistanceAlong);

    double GetStartDistanceAlong() const { return m_startDistanceAlong; }
    double GetStopDistanceAlong() const { return m_stopDistanceAlong; }
    Dgn::DgnElementId GetILinearlyLocatedId() const { return m_linearlyLocatedId; }
    Dgn::DgnClassId GetILinearlyLocatedClassId() const { return m_linearlyLocatedClassId; }
    LinearlyReferencedLocationId GetLinearlyReferencedLocationId() const { return m_locationId; }

    LINEARREFERENCING_EXPORT bool operator==(LinearLocationReferenceCR rhs) const;
    LINEARREFERENCING_EXPORT bool operator!=(LinearLocationReferenceCR rhs) const { return !(*this == rhs); }
}; // LinearLocationReference

//=======================================================================================
//! Interface implemented by elements that can be used as a scale 
//! along which linear referencing is performed.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ILinearElement
{
public:
    struct QueryParams
    {
    public:
        enum class ComparisonOption { Inclusive, Exclusive };
        enum class LinearlyReferencedLocationType { At = 1, FromTo = 2, Any = 3 };

    private:
        void Init()
            {
            m_linearlyReferencedLocationTypeFilter = LinearlyReferencedLocationType::Any;
            }

    public:
        QueryParams(NullableDouble fromDistanceAlong, NullableDouble toDistanceAlong) :
            m_fromDistanceAlong(fromDistanceAlong), m_toDistanceAlong(toDistanceAlong),
            m_fromComparisonOption(ComparisonOption::Inclusive),
            m_toComparisonOption(ComparisonOption::Inclusive) { Init(); }

        QueryParams(NullableDouble fromDistanceAlong, ComparisonOption fromComparison,
            NullableDouble toDistanceAlong, ComparisonOption toComparison) :
            m_fromDistanceAlong(fromDistanceAlong), m_toDistanceAlong(toDistanceAlong),
            m_fromComparisonOption(fromComparison),
            m_toComparisonOption(toComparison) { Init(); }

        QueryParams(bset<Dgn::DgnClassId> const& iLinearlyLocatedClassIds) :
            m_iLinearlyLocatedClassIds(iLinearlyLocatedClassIds),
            m_fromComparisonOption(ComparisonOption::Inclusive),
            m_toComparisonOption(ComparisonOption::Inclusive) { Init(); }

        QueryParams(bset<Dgn::DgnClassId> const& iLinearlyLocatedClassIds,
            NullableDouble fromDistanceAlong, NullableDouble toDistanceAlong) :
            m_iLinearlyLocatedClassIds(iLinearlyLocatedClassIds),
            m_fromDistanceAlong(fromDistanceAlong), m_toDistanceAlong(toDistanceAlong),
            m_fromComparisonOption(ComparisonOption::Inclusive),
            m_toComparisonOption(ComparisonOption::Inclusive) { Init(); }

        QueryParams(bset<Dgn::DgnClassId> const& iLinearlyLocatedClassIds,
            NullableDouble fromDistanceAlong, ComparisonOption fromComparison,
            NullableDouble toDistanceAlong, ComparisonOption toComparison) :
            m_iLinearlyLocatedClassIds(iLinearlyLocatedClassIds),
            m_fromDistanceAlong(fromDistanceAlong), m_toDistanceAlong(toDistanceAlong),
            m_fromComparisonOption(fromComparison),
            m_toComparisonOption(toComparison) { Init(); }

        bset<Dgn::DgnClassId> m_iLinearlyLocatedClassIds;
        NullableDouble m_fromDistanceAlong;
        ComparisonOption m_fromComparisonOption;
        NullableDouble m_toDistanceAlong;
        ComparisonOption m_toComparisonOption;
        LinearlyReferencedLocationType m_linearlyReferencedLocationTypeFilter;
    }; // QueryParams

protected:
    //! @private
    virtual Dgn::DgnElementCR _ILinearElementToDgnElement() const = 0;

    //! @private
    LINEARREFERENCING_EXPORT virtual bvector<LinearLocationReference> _QueryLinearLocations(QueryParams const& params) const;

    //! @private
    void _SetLength(double newVal) { ToElementR().SetPropertyValue(BLR_PROP_ILinearElement_LengthValue, newVal); }

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(ILinearElement)
    //! Get a const reference to the DgnElement of this ILinearElement
    Dgn::DgnElementCR ToElement() const { return _ILinearElementToDgnElement(); }
    
    //__PUBLISH_SECTION_END__
    Dgn::DgnElementR ToElementR() { return *const_cast<Dgn::DgnElementP>(&_ILinearElementToDgnElement()); }
    //__PUBLISH_SECTION_START__

    //! Get the length of the ILinearElement
    //! @return The length of this ILinearElement, in meters.
    double GetLength() const { return ToElement().GetPropertyValueDouble(BLR_PROP_ILinearElement_LengthValue); }

    //! Get the start value of the ILinearElement
    double GetStartValue() const { return ToElement().GetPropertyValueDouble(BLR_PROP_ILinearElement_StartValue); }

    //! Get the source element providing this ILinearElement
    Dgn::DgnElementId GetSource() const { return ToElement().GetPropertyValueId<Dgn::DgnElementId>(BLR_PROP_ILinearElement_Source); }

    //! Query Linearly-Located elements/attribution along this LinearElement
    LINEARREFERENCING_EXPORT bvector<LinearLocationReference> QueryLinearLocations(QueryParams const& params) const { return _QueryLinearLocations(params); }

    //__PUBLISH_SECTION_END__
    void SetStartValue(double newStartVal) { ToElementR().SetPropertyValue(BLR_PROP_ILinearElement_StartValue, newStartVal); }
    LINEARREFERENCING_EXPORT void SetSource(ILinearElementSourceCP source);
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

//=======================================================================================
//! Base interface for linearly-located elements and attributions.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ILinearlyLocated
{
private:
    mutable Dgn::DgnElementId m_cachedLinearElementId;

protected:
    //! @private
    LINEARREFERENCING_EXPORT ILinearlyLocated();

    //! @private
    void _SetLinearElement(Dgn::DgnElementId elementId) { m_cachedLinearElementId = elementId; }

    //! @private
    LINEARREFERENCING_EXPORT Dgn::DgnDbStatus _InsertLinearElementRelationship();

    //! @private
    LINEARREFERENCING_EXPORT Dgn::DgnDbStatus _UpdateLinearElementRelationship();
    
    //! @private
    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const = 0;

    //! @private
    LINEARREFERENCING_EXPORT void _AddLinearlyReferencedLocation(LinearlyReferencedLocationR);
    
public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(ILinearlyLocated)

    //! Get a const reference to the DgnElement of this ILinearlyLocated
    Dgn::DgnElementCR ToElement() const { return _ILinearlyLocatedToDgnElement(); }

    //! Query the DgnElementId of the LinearElement
    LINEARREFERENCING_EXPORT Dgn::DgnElementId GetLinearElementId() const;

    //! Query the LinearElement
    LINEARREFERENCING_EXPORT ILinearElementCP GetLinearElement() const;

    //! Obtain all of the LinearlyReferencedLocationIds related to this element.
    LINEARREFERENCING_EXPORT bvector<LinearlyReferencedLocationId> QueryLinearlyReferencedLocationIds() const;

    //! Given a LinearlyReferencedLocationId (see QueryLinearlyReferencedLocationIds()), obtain its LinearlyReferencedLocation.
    LINEARREFERENCING_EXPORT LinearlyReferencedLocationCP GetLinearlyReferencedLocation(LinearlyReferencedLocationId) const;
    
    //__PUBLISH_SECTION_END__
    Dgn::DgnElementR ToElementR() { return *const_cast<Dgn::DgnElementP>(&_ILinearlyLocatedToDgnElement()); }    

    LINEARREFERENCING_EXPORT LinearlyReferencedLocationP GetLinearlyReferencedLocationP(LinearlyReferencedLocationId);    
    //__PUBLISH_SECTION_START__
    
}; // ILinearlyLocated

//=======================================================================================
//! Interface implemented by attribution-elements, representing values of a property
//! of an ILinearElementSource which may apply to only part of it.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ILinearlyLocatedAttribution : virtual ILinearlyLocated
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
struct EXPORT_VTABLE_ATTRIBUTE ILinearLocationElement : virtual ILinearlyLocated
{
    DEFINE_T_SUPER(ILinearlyLocated)

protected:
    LINEARREFERENCING_EXPORT ILinearLocationElement();

public:
    Dgn::DgnElementId GetLocatedElementId() const { return ToElement().GetPropertyValueId<Dgn::DgnElementId>(BLR_PROP_ILinearlyLocated_LocatedElement); }
}; // ILinearLocationElement

//=======================================================================================
//! Helper interface to be implemented by linearly located attribution and elements
//! only accepting and exposing multiple "at" linearly referenced locations.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ILinearlyLocatedMultipleAt : virtual ILinearlyLocated
{
    //! Given a LinearlyReferencedLocationId (see QueryLinearlyReferencedLocationIds()), get its LinearlyReferencedAtLocation.
    LINEARREFERENCING_EXPORT LinearlyReferencedAtLocationCP GetAtLocation(LinearlyReferencedLocationId) const;
    //__PUBLISH_SECTION_END__

    LINEARREFERENCING_EXPORT LinearlyReferencedAtLocationP GetAtLocationP(LinearlyReferencedLocationId);

    //__PUBLISH_SECTION_START__
}; // ILinearlyLocatedAt

//=======================================================================================
//! Helper interface to be implemented by linearly located attribution and elements
//! only accepting and exposing one "at" linearly referenced location.
//! Concrete subclasses are expected to explicitely add the single aspect on their
//! constructor as follows:
//! _AddLinearlyReferencedLocation(*_GetUnpersistedAtLocation());
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct ILinearlyLocatedSingleAt : virtual ILinearlyLocated
{
private:
    mutable LinearReferencing::LinearlyReferencedLocationId m_atLocationAspectId;
    LinearReferencing::LinearlyReferencedAtLocationPtr m_unpersistedAtLocationPtr;
    
public:
    struct CreateAtParams
    {
        DistanceExpression m_atPosition;
        Dgn::DgnElementCPtr m_linearElementCPtr;

        CreateAtParams(ILinearElementCR linearElement, DistanceExpressionCR atPosition): m_linearElementCPtr(&linearElement.ToElement()), m_atPosition(atPosition) {}
    }; // CreateAtParams

protected:
    //! @private
    LINEARREFERENCING_EXPORT ILinearlyLocatedSingleAt() {}

    //! @private
    LINEARREFERENCING_EXPORT ILinearlyLocatedSingleAt(CreateAtParams const& params);

    //! @private
    LinearReferencing::LinearlyReferencedAtLocationPtr _GetUnpersistedAtLocation() const { return m_unpersistedAtLocationPtr; }

    static bool ValidateParams(CreateAtParams const& params) { return params.m_linearElementCPtr.IsValid() && params.m_linearElementCPtr->GetElementId().IsValid(); }

public:
    LINEARREFERENCING_EXPORT LinearReferencing::LinearlyReferencedAtLocationCP GetAtLocation() const;
    LINEARREFERENCING_EXPORT LinearReferencing::LinearlyReferencedAtLocationP GetAtLocationP();

    //! Get the distance along of this LinearLocated from the start of the ILinearElement
    LINEARREFERENCING_EXPORT double GetAtDistanceAlongFromStart() const;

    //! @private
    LINEARREFERENCING_EXPORT void SetAtDistanceAlongFromStart(double newAt);
}; // ILinearlyLocatedSingleAt

//=======================================================================================
//! Helper interface to be implemented by linearly located attribution and elements
//! only accepting and exposing multiple "from-to" linearly referenced locations.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ILinearlyLocatedMultipleFromTo : virtual ILinearlyLocated
{
    //! Given a LinearlyReferencedLocationId (see QueryLinearlyReferencedLocationIds()), get its LinearlyReferencedLocationId.
    LINEARREFERENCING_EXPORT LinearlyReferencedFromToLocationCP GetFromToLocation(LinearlyReferencedLocationId) const;
    //__PUBLISH_SECTION_END__

    LINEARREFERENCING_EXPORT LinearlyReferencedFromToLocationP GetFromToLocationP(LinearlyReferencedLocationId);
    //__PUBLISH_SECTION_START__
}; // ILinearlyLocatedAt

//=======================================================================================
//! Helper interface to be implemented by linearly located attribution and elements
//! only accepting and exposing one from-to linearly referenced location.
//! Concrete subclasses are expected to explicitely add the single aspect on their
//! constructor as follows:
//! _AddLinearlyReferencedLocation(*_GetUnpersistedFromToLocation());
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct ILinearlyLocatedSingleFromTo : virtual ILinearlyLocated
{
private:
    mutable LinearReferencing::LinearlyReferencedLocationId m_fromToLocationAspectId;
    LinearReferencing::LinearlyReferencedFromToLocationPtr m_unpersistedFromToLocationPtr;

public:
    struct CreateFromToParams
    {
        DistanceExpression m_fromPosition, m_toPosition;
        Dgn::DgnElementCPtr m_linearElementCPtr;

        CreateFromToParams(ILinearElementCR linearElement, DistanceExpressionCR fromPosition, DistanceExpressionCR toPosition):
            m_linearElementCPtr(&linearElement.ToElement()), m_fromPosition(fromPosition), m_toPosition(toPosition)
            { }
    }; // CreateFromToParams

protected:
    //! @private
    LINEARREFERENCING_EXPORT ILinearlyLocatedSingleFromTo() {}

    //! @private
    LINEARREFERENCING_EXPORT ILinearlyLocatedSingleFromTo(CreateFromToParams const& params);

    //! @private
    LinearReferencing::LinearlyReferencedFromToLocationPtr _GetUnpersistedFromToLocation() const { return m_unpersistedFromToLocationPtr; }

    static bool ValidateParams(CreateFromToParams const& params) { return params.m_linearElementCPtr.IsValid() && params.m_linearElementCPtr->GetElementId().IsValid(); }

public:
    LINEARREFERENCING_EXPORT LinearReferencing::LinearlyReferencedFromToLocationCP GetFromToLocation() const;
    LINEARREFERENCING_EXPORT LinearReferencing::LinearlyReferencedFromToLocationP GetFromToLocationP();

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
struct EXPORT_VTABLE_ATTRIBUTE IReferent : virtual ILinearlyLocated
{
public:
    //! Get the referenced elements from this Referent.
    Dgn::DgnElementId GetReferencedElementId() const { return ToElement().GetPropertyValueId<Dgn::DgnElementId>(BLR_PROP_IReferent_ReferencedElement); }

    //! @private
    void SetReferencedElement(Dgn::SpatialElementCR referencedElement) { ToElementR().SetPropertyValue(BLR_PROP_IReferent_ReferencedElement, referencedElement.GetElementId()); }
}; // IReferent

END_BENTLEY_LINEARREFERENCING_NAMESPACE