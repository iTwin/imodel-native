/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/LinearReferencing/ILinearElement.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "LinearReferencingApi.h"

BEGIN_BENTLEY_LINEARREFERENCING_NAMESPACE

enum class CascadeLocationChangesAction { None, OnlyIfLocationsChanged, Always };

//=======================================================================================
//! Interface implemented by elements that can be used as a scale 
//! along which linear referencing is performed.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ILinearElement
{
protected:
    virtual double _GetLength() const = 0;
    virtual double _GetStartValue() const = 0;
    virtual Dgn::DgnElementCR _ILinearElementToDgnElement() const = 0;

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(ILinearElement)
    Dgn::DgnElementCR ToElement() const { return _ILinearElementToDgnElement(); }
    Dgn::DgnElementR ToElementR() { return *const_cast<Dgn::DgnElementP>(&_ILinearElementToDgnElement()); }

    LINEARREFERENCING_EXPORT double GetLength() const { return _GetLength(); }
    LINEARREFERENCING_EXPORT double GetStartValue() const { return _GetStartValue(); }
}; // ILinearElement

//=======================================================================================
//! Interface implemented by linear-elements that have a spatial representation.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ISpatialLinearElement
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
    LINEARREFERENCING_EXPORT virtual Dgn::DgnDbStatus _PrepareCascadeChanges(ICascadeLinearLocationChangesAlgorithmR algorithm) const;
    LINEARREFERENCING_EXPORT virtual Dgn::DgnDbStatus _CommitCascadeChanges(ICascadeLinearLocationChangesAlgorithmR algorithm) const;

    virtual Dgn::DgnElementCR _ILinearElementSourceToDgnElement() const = 0;

public:
    Dgn::DgnElementCR ToElement() const { return _ILinearElementSourceToDgnElement(); }
    Dgn::DgnElementR ToElementR() { return *const_cast<Dgn::DgnElementP>(&_ILinearElementSourceToDgnElement()); }
}; // ILinearElementSource

typedef BeSQLite::EC::ECInstanceId LinearlyReferencedLocationId;

//=======================================================================================
//! Base interface for linearly-located elements and attributions.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ILinearlyLocated
{
friend struct CascadeFromToLocationChangesAlgorithm;

private:
    CascadeLocationChangesAction m_cascadeLocationChangesFlag;
    bset<LinearlyReferencedLocationId> m_accessedAtLocationIds, m_accessedFromToLocationIds;

protected:
    LINEARREFERENCING_EXPORT ILinearlyLocated();

    LINEARREFERENCING_EXPORT void _SetLinearElement(Dgn::DgnElementId elementId);
    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const = 0;
    LINEARREFERENCING_EXPORT void _AddLinearlyReferencedLocation(LinearlyReferencedLocationR);
    bset<LinearlyReferencedLocationId> const& _GetLinearlyReferencedAtLocationIdsAccessed() const { return m_accessedAtLocationIds; }
    bset<LinearlyReferencedLocationId> const& _GetLinearlyReferencedFromToLocationIdsAccessed() const { return m_accessedFromToLocationIds; }

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

    CascadeLocationChangesAction GetCascadeLocationChangesActionFlag() const { return m_cascadeLocationChangesFlag;}
    void SetCascadeLocationChangesActionFlag(CascadeLocationChangesAction flag) { m_cascadeLocationChangesFlag = flag; }
}; // ILinearlyLocated

//=======================================================================================
//! Interface implemented by algorithm classes to be applied during cascading of
//! changes on linearly-located elements.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ICascadeLinearLocationChangesAlgorithm : RefCountedBase
{
private:
    CascadeLocationChangesAction m_action;
    Dgn::DgnElementCPtr m_original, m_replacement;    
    bvector<Dgn::DgnElementPtr> m_impactedLinearlyLocatedDgnElements;
    bset<Dgn::DgnElementId> m_processedIds;

protected:
    ICascadeLinearLocationChangesAlgorithm(ILinearlyLocatedCR original, ILinearlyLocatedCR replacement, CascadeLocationChangesAction action) :
        m_original(&original.ToElement()), m_replacement(&replacement.ToElement()), m_action(action) {}

    virtual Dgn::DgnDbStatus _Prepare(ILinearElementSourceCR source) = 0;
    virtual Dgn::DgnDbStatus _Commit(ILinearElementSourceCR source) = 0;
    void _AddImpactedDgnElement(Dgn::DgnElementPtr& dgnElement) { m_impactedLinearlyLocatedDgnElements.push_back(dgnElement); }
    void _MarkAsProcessed(Dgn::DgnElementId const& id) { m_processedIds.insert(id); }
    bool _IsProcessed(Dgn::DgnElementId const& id) { return m_processedIds.find(id) != m_processedIds.end(); }
    bvector<Dgn::DgnElementPtr> _GetImpactedDgnElements() const { return m_impactedLinearlyLocatedDgnElements; }

public:
    ILinearlyLocatedCR GetOriginal() const { return *dynamic_cast<ILinearlyLocatedCP>(m_original.get()); }
    ILinearlyLocatedCR GetReplacement() const { return *dynamic_cast<ILinearlyLocatedCP>(m_replacement.get()); }
    CascadeLocationChangesAction GetAction() const { return m_action; }

    Dgn::DgnDbStatus Prepare(ILinearElementSourceCR source) { return _Prepare(source); }
    Dgn::DgnDbStatus Commit(ILinearElementSourceCR source) { return _Commit(source); }
}; // ICascadeLinearLocationChangesAlgorithm

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
    LINEARREFERENCING_EXPORT double GetFromDistanceAlong() const;
    LINEARREFERENCING_EXPORT void SetFromDistanceAlong(double newFrom);

    LINEARREFERENCING_EXPORT double GetToDistanceAlong() const;
    LINEARREFERENCING_EXPORT void SetToDistanceAlong(double newFrom);
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
    virtual double _GetRestartValue() const { return 0.0; }
    virtual Dgn::DgnElementCR _IReferentToDgnElement() const = 0;

public:
    Dgn::DgnElementCR ToElement() const { return _IReferentToDgnElement(); }
    Dgn::DgnElementR ToElementR() { return *const_cast<Dgn::DgnElementP>(&_IReferentToDgnElement()); }

    LINEARREFERENCING_EXPORT double GetRestartValue() const { return _GetRestartValue(); }    
}; // IReferent

//=======================================================================================
//! Specialization of IReferent implemented by referents that are linearly-located.
//! @see IReferent
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinearlyLocatedReferent : Dgn::SpatialLocationElement, ILinearlyLocatedElement, IReferent
{
    DGNELEMENT_DECLARE_MEMBERS(BLR_CLASS_LinearlyLocatedReferent, Dgn::SpatialLocationElement);
    friend struct LinearlyLocatedReferentHandler;

protected:
    //! @private
    explicit LinearlyLocatedReferent(CreateParams const& params) : T_Super(params) {}

    virtual ILinearlyLocatedElementCP _ToLinearlyLocatedElement() const { return nullptr; }
    virtual Dgn::DgnElementCR _IReferentToDgnElement() const override { return *this; }
    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const override { return *this; }

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(LinearlyLocatedReferent)

    LINEARREFERENCING_EXPORT ILinearlyLocatedElementCP ToLinearlyLocatedElement() const { return _ToLinearlyLocatedElement(); }
}; // LinearlyLocatedReferent

//=======================================================================================
//! Specialization of IReferent implemented by referents that are linearly-located.
//! @see IReferent
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GeometricElementAsReferent : Dgn::SpatialLocationElement, IReferent
{
    DGNELEMENT_DECLARE_MEMBERS(BLR_CLASS_GeometricElementAsReferent, Dgn::SpatialLocationElement);
    friend struct GeometricElementAsReferentHandler;

protected:
    //! @private
    explicit GeometricElementAsReferent(CreateParams const& params) : T_Super(params) {}

    virtual DPoint3d _GetSpatialLocation() const { return DPoint3d(); }
    virtual Dgn::DgnElementCR _IReferentToDgnElement() const override { return *this; }

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(GeometricElementAsReferent)

    LINEARREFERENCING_EXPORT DPoint3d GetSpatialLocation() const { return _GetSpatialLocation(); }
    LINEARREFERENCING_EXPORT Dgn::DgnElementId QueryGeometricElementId() const;
}; // GeometricElementReferent


//=================================================================================
//! ElementHandler for LinearlyLocatedReferent Element
//! @ingroup GROUP_LinearReferencing
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinearlyLocatedReferentHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
ELEMENTHANDLER_DECLARE_MEMBERS(BLR_CLASS_LinearlyLocatedReferent, LinearlyLocatedReferent, LinearlyLocatedReferentHandler, Dgn::dgn_ElementHandler::SpatialLocation, LINEARREFERENCING_EXPORT)
}; // LinearlyLocatedReferentHandler

//=================================================================================
//! ElementHandler for GeometricElementAsReferent Element
//! @ingroup GROUP_LinearReferencing
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GeometricElementAsReferentHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
ELEMENTHANDLER_DECLARE_MEMBERS(BLR_CLASS_GeometricElementAsReferent, GeometricElementAsReferent, GeometricElementAsReferentHandler, Dgn::dgn_ElementHandler::SpatialLocation, LINEARREFERENCING_EXPORT)
}; // GeometricElementAsReferentHandler

END_BENTLEY_LINEARREFERENCING_NAMESPACE