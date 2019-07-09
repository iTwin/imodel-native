/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/LinearReferencing/LinearlyLocated.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

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
struct LinearLocationElement : Dgn::SpatialLocationElement, LinearReferencing::ILinearLocationElement
{
DGNELEMENT_DECLARE_MEMBERS(BLR_CLASS_LinearLocationElement, Dgn::SpatialLocationElement);
friend struct LinearLocationElementHandler;

protected:
    //! @private
    explicit LinearLocationElement(CreateParams const& params): T_Super(params) {}

    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const { return *this; }

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(LinearLocationElement)
}; // LinearLocationElement

//=======================================================================================
//! Base class for ILinearLocationElement-implementations that are subclasses of 
//! bis:PhysicalElement.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct LinearPhysicalElement : Dgn::PhysicalElement, LinearReferencing::ILinearLocationElement
{
DGNELEMENT_DECLARE_MEMBERS(BLR_CLASS_LinearPhysicalElement, Dgn::PhysicalElement);
friend struct LinearPhysicalElementHandler;

protected:
    //! @private
    explicit LinearPhysicalElement(CreateParams const& params): T_Super(params) {}

    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const { return *this; }

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(LinearPhysicalElement)
}; // LinearPhysicalElement

//=======================================================================================
//! Base class for ILinearLocationElement-implementations that are subclasses of 
//! bis:SpatialLocationElement.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct LinearLocation : LinearLocationElement
{
DGNELEMENT_DECLARE_MEMBERS(BLR_CLASS_LinearLocation, LinearLocationElement);
friend struct LinearLocationHandler;

protected:
    //! @private
    explicit LinearLocation(CreateParams const& params): T_Super(params) {}

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(LinearLocation)
    DECLARE_LINEARREFERENCING_ELEMENT_GET_METHODS(LinearLocation)
    DECLARE_LINEARREFERENCING_LINEARLYLOCATED_SET_METHODS(LinearLocation)
}; // LinearLocation

//=======================================================================================
//! Base class for ILinearlyLocatedAttribution-implementations that are 
//! bis:SpatialLocationElement.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct LinearlyLocatedAttribution : Dgn::SpatialLocationElement, LinearReferencing::ILinearlyLocatedAttribution
{
DGNELEMENT_DECLARE_MEMBERS(BLR_CLASS_LinearlyLocatedAttribution, Dgn::SpatialLocationElement);
friend struct LinearlyLocatedAttributionHandler;

protected:
    //! @private
    explicit LinearlyLocatedAttribution(CreateParams const& params): T_Super(params) {}

    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const { return *this; }

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(LinearlyLocatedAttribution)
}; // LinearlyLocatedAttribution

//=======================================================================================
//! Base class for IReferent-implementations that are subclasses of 
//! bis:SpatialLocationElement.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct ReferentElement : Dgn::SpatialLocationElement, IReferent, ILinearlyLocatedSingleAt
{
DGNELEMENT_DECLARE_MEMBERS(BLR_CLASS_ReferentElement, Dgn::SpatialLocationElement);
friend struct ReferentElementHandler;

protected:
    //! @private
    explicit ReferentElement(CreateParams const& params): T_Super(params) {}

    //! @private
    explicit ReferentElement(CreateParams const& params, CreateAtParams const& atParams): T_Super(params), ILinearlyLocatedSingleAt(atParams) {}

    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const { return *this; }

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
DGNELEMENT_DECLARE_MEMBERS(BLR_CLASS_Referent, ReferentElement);
friend struct ReferentHandler;

protected:
    //! @private
    explicit Referent(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit Referent(CreateParams const& params, CreateAtParams const& atParams): T_Super(params, atParams) {}

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(Referent)
    DECLARE_LINEARREFERENCING_ELEMENT_GET_METHODS(Referent)
    DECLARE_LINEARREFERENCING_LINEARLYLOCATED_SET_METHODS(Referent)

    LINEARREFERENCING_EXPORT static ReferentPtr Create(Dgn::SpatialElementCR referencedElement, CreateAtParams const& atParams);
}; // Referent


//=================================================================================
//! ElementHandler for LinearlyLocated Attributions
//! @ingroup GROUP_RoadRailAlignment
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinearlyLocatedAttributionHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
ELEMENTHANDLER_DECLARE_MEMBERS(BLR_CLASS_LinearlyLocatedAttribution, LinearlyLocatedAttribution, LinearlyLocatedAttributionHandler, Dgn::dgn_ElementHandler::SpatialLocation, LINEARREFERENCING_EXPORT)
}; // LinearlyLocatedAttributionHandler

//=================================================================================
//! ElementHandler for LinearLocation Elements
//! @ingroup GROUP_RoadRailAlignment
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinearLocationElementHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
ELEMENTHANDLER_DECLARE_MEMBERS(BLR_CLASS_LinearLocationElement, LinearLocationElement, LinearLocationElementHandler, Dgn::dgn_ElementHandler::SpatialLocation, LINEARREFERENCING_EXPORT)
}; // LinearLocationElementHandler

//=================================================================================
//! ElementHandler for LinearLocation Elements
//! @ingroup GROUP_RoadRailAlignment
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinearLocationHandler : LinearLocationElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BLR_CLASS_LinearLocation, LinearLocation, LinearLocationHandler, LinearLocationElementHandler, LINEARREFERENCING_EXPORT)
}; // LinearLocationHandler

//=================================================================================
//! ElementHandler for LinearLocation Elements
//! @ingroup GROUP_RoadRailAlignment
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinearPhysicalElementHandler : Dgn::dgn_ElementHandler::Physical
{
ELEMENTHANDLER_DECLARE_MEMBERS(BLR_CLASS_LinearPhysicalElement, LinearPhysicalElement, LinearPhysicalElementHandler, Dgn::dgn_ElementHandler::Physical, LINEARREFERENCING_EXPORT)
}; // LinearPhysicalElementHandler

//=================================================================================
//! ElementHandler for Referent Elements
//! @ingroup GROUP_RoadRailAlignment
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ReferentElementHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
ELEMENTHANDLER_DECLARE_MEMBERS(BLR_CLASS_ReferentElement, ReferentElement, ReferentElementHandler, Dgn::dgn_ElementHandler::SpatialLocation, LINEARREFERENCING_EXPORT)
}; // ReferentElementHandler

//=================================================================================
//! ElementHandler for Referent Elements
//! @ingroup GROUP_RoadRailAlignment
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ReferentHandler : ReferentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BLR_CLASS_Referent, Referent, ReferentHandler, ReferentElementHandler, LINEARREFERENCING_EXPORT)
}; // ReferentHandler

END_BENTLEY_LINEARREFERENCING_NAMESPACE