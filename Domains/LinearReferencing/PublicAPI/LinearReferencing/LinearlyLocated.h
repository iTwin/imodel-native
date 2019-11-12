/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "LinearReferencing.h"
#include "ILinearElement.h"

BEGIN_BENTLEY_LINEARREFERENCING_NAMESPACE

//=======================================================================================
//! Base class for ILinearLocationElement-implementations that are subclasses of 
//! bis:SpatialLocationElement.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct LinearLocationElement : GeometricElementWrapper<Dgn::SpatialLocationElement>, LinearReferencing::ILinearLocationElement
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(GeometricElementWrapper, Dgn::SpatialLocationElement)

protected:
    //! @private
    explicit LinearLocationElement(Dgn::SpatialLocationElement const& element) : T_Super(element) {}
    explicit LinearLocationElement(Dgn::SpatialLocationElement& element) : T_Super(element) {}

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(LinearLocationElement)
}; // LinearLocationElement

//=======================================================================================
//! Base class for ILinearLocationElement-implementations that are subclasses of 
//! bis:PhysicalElement.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct LinearPhysicalElement : GeometricElementWrapper<Dgn::PhysicalElement>, LinearReferencing::ILinearLocationElement
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(GeometricElementWrapper, Dgn::PhysicalElement)

protected:
    //! @private
    explicit LinearPhysicalElement(Dgn::PhysicalElement const& element) : T_Super(element) {}
    explicit LinearPhysicalElement(Dgn::PhysicalElement& element) : T_Super(element) {}

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(LinearPhysicalElement)
}; // LinearPhysicalElement

//=======================================================================================
//! Base class for ILinearLocationElement-implementations that are subclasses of 
//! bis:SpatialLocationElement.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct LinearLocation : LinearLocationElement, ILinearlyLocatedSingleAt, ILinearlyLocatedSingleFromTo
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(LinearLocationElement, Dgn::SpatialLocationElement)

private:
    mutable Dgn::DgnElementId m_cachedLocatedElementId;

    //! @private
    LINEARREFERENCING_EXPORT Dgn::DgnDbStatus _InsertLocatedElementRelationship();

protected:
    //! @private
    explicit LinearLocation(Dgn::SpatialLocationElement const& element) : T_Super(element) {}
    explicit LinearLocation(Dgn::SpatialLocationElement& element) : T_Super(element) {}
    //! @private
    explicit LinearLocation(Dgn::SpatialLocationElementR element, Dgn::DgnElementCR locatedElement, CreateAtParams const& atParams);
    explicit LinearLocation(Dgn::SpatialLocationElementR element, Dgn::DgnElementCR locatedElement, CreateFromToParams const& atParams);

    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const override { return *get(); }

    //! @private
    void _SetLocatedElement(Dgn::DgnElementId elementId) { m_cachedLocatedElementId = elementId; }

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(LinearLocation)
    DECLARE_LINEARREFERENCING_ELEMENT_GET_METHODS(LinearLocation, Dgn::SpatialLocationElement)

    LINEARREFERENCING_EXPORT static LinearLocationPtr Create(Dgn::DgnElementCR locatedElement, Dgn::DgnCategoryId const& categoryId, CreateAtParams const& atParams);
    LINEARREFERENCING_EXPORT static LinearLocationPtr Create(Dgn::DgnElementCR locatedElement, Dgn::DgnCategoryId const& categoryId, CreateFromToParams const& atParams);
    LINEARREFERENCING_EXPORT static Dgn::DgnElementId Query(Dgn::DgnElementCR locatedElement);

    LINEARREFERENCING_EXPORT LinearLocationCPtr Insert(Dgn::DgnDbStatus* stat = nullptr);
    LINEARREFERENCING_EXPORT LinearLocationCPtr Update(Dgn::DgnDbStatus* stat = nullptr);
}; // LinearLocation

//=======================================================================================
//! Base class for ILinearlyLocatedAttribution-implementations that are 
//! bis:SpatialLocationElement.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct LinearlyLocatedAttribution : GeometricElementWrapper<Dgn::SpatialLocationElement>, LinearReferencing::ILinearlyLocatedAttribution
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(GeometricElementWrapper, Dgn::SpatialLocationElement)

protected:
    //! @private
    explicit LinearlyLocatedAttribution(Dgn::SpatialLocationElement const& element) : T_Super(element) {}
    explicit LinearlyLocatedAttribution(Dgn::SpatialLocationElement& element) : T_Super(element) {}

    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const { return *get(); }

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(LinearlyLocatedAttribution)
}; // LinearlyLocatedAttribution

//=======================================================================================
//! Base class for IReferent-implementations that are subclasses of 
//! bis:SpatialLocationElement.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct ReferentElement : GeometricElementWrapper<Dgn::SpatialLocationElement>, IReferent, ILinearlyLocatedSingleAt
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(GeometricElementWrapper, Dgn::SpatialLocationElement)

protected:
    //! @private
    explicit ReferentElement(Dgn::SpatialLocationElement const& element) : T_Super(element) {}
    explicit ReferentElement(Dgn::SpatialLocationElement& element) : T_Super(element) {}

    //! @private
    explicit ReferentElement(Dgn::SpatialLocationElement& element, CreateAtParams const& atParams): T_Super(element), ILinearlyLocatedSingleAt(atParams) {}

    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const { return *get(); }

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(ReferentElement)
}; // ReferentElement

//=======================================================================================
//! IReferent-implementation turning any bis:SpatialElement not inherently 
//! Linearly-Referenced into a Referent for Linear-Referencing purposes.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct Referent : ReferentElement
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(ReferentElement, Dgn::SpatialLocationElement)

protected:
    //! @private
    explicit Referent(Dgn::SpatialLocationElement const& element) : T_Super(element) {}
    explicit Referent(Dgn::SpatialLocationElement& element) : T_Super(element) {}

    //! @private
    explicit Referent(Dgn::SpatialLocationElement& element, CreateAtParams const& atParams): T_Super(element, atParams) {}

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(Referent)
    DECLARE_LINEARREFERENCING_ELEMENT_GET_METHODS(Referent, Dgn::SpatialLocationElement)
    DECLARE_LINEARREFERENCING_LINEARLYLOCATED_SET_METHODS(Referent, Dgn::SpatialLocationElement)

    LINEARREFERENCING_EXPORT static ReferentPtr Create(Dgn::GeometricElement3dCR referencedElement, CreateAtParams const& atParams);
}; // Referent

END_BENTLEY_LINEARREFERENCING_NAMESPACE