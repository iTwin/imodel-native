/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/Pathway.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysical.h"
#include "RoadRailCategory.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! An ILinearElementSource providing an ILinearElement considered as the main 
//! linear-referencing axis for Road/Rail purposes.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct IMainLinearElementSource : LinearReferencing::ILinearElementSource
{
    Dgn::DgnElementId GetMainLinearElementId() const { return _ILinearElementSourceToDgnElement().GetPropertyValueId<Dgn::DgnElementId>("MainLinearElement"); }

    template <class T> RefCountedCPtr<T> GetMainLinearElementAs() const { return _ILinearElementSourceToDgnElement().GetDgnDb().Elements().Get<T>(GetMainLinearElementId()); }
    ROADRAILPHYSICAL_EXPORT Dgn::DgnDbStatus SetMainLinearElement(LinearReferencing::ILinearElementCP linearElement);
}; // IMainLinearElementSource

//=======================================================================================
//! Base class for Road and Rail range of physical segments.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct PathwayElement : Dgn::PhysicalElement, IMainLinearElementSource
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_PathwayElement, Dgn::PhysicalElement);
    friend struct PathwayElementHandler;

protected:
    //! @private
    explicit PathwayElement(CreateParams const& params) : T_Super(params) {}

    virtual Dgn::DgnElementCR _ILinearElementSourceToDgnElement() const override { return *this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(PathwayElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(PathwayElement)

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus AddRepresentedBy(PathwayElementCR pathway, Dgn::DgnElementCR representedBy);
}; // PathwayElement

//=======================================================================================
//! Physical range over a Road that can be segmented.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct Roadway : PathwayElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_Roadway, PathwayElement);
    friend struct RoadwayHandler;

protected:
    //! @private
    explicit Roadway(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(Roadway)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(Roadway)
    ROADRAILPHYSICAL_EXPORT static RoadwayPtr Create(Dgn::PhysicalModelR model);
}; // Roadway

//=======================================================================================
//! Physical range over a Rail that can be segmented.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct Railway : PathwayElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_Railway, PathwayElement);
    friend struct RailwayHandler;

protected:
    //! @private
    explicit Railway(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit Railway(CreateParams const& params, RoadRailAlignment::AlignmentCR alignment);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(Railway)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(Railway)
    ROADRAILPHYSICAL_EXPORT static RailwayPtr Create(Dgn::PhysicalModelR model);
}; // Railway

//=======================================================================================
//! Physical Portion of a Pathway.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct PathwayPortionElement : Dgn::PhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_PathwayPortionElement, Dgn::PhysicalElement);
    friend struct PathwayPortionElementHandler;

protected:
    //! @private
    explicit PathwayPortionElement(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(PathwayPortionElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(PathwayPortionElement)
}; // PathwayPortionElement

//=======================================================================================
//! A PathwayPortion specially constructed for continuous travel of a particular type.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct ThruTravelComposite : PathwayPortionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_ThruTravelComposite, PathwayPortionElement);
    friend struct ThruTravelCompositeHandler;

protected:
    //! @private
    explicit ThruTravelComposite(CreateParams const& params) : T_Super(params) {}

public:
    enum class SideOnPathway { SingleComposite = 0, Left = 1, Right = 2 };

    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(ThruTravelComposite)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(ThruTravelComposite)
}; // ThruTravelComposite

//=======================================================================================
//! A PathwayPortion specially constructed to separate two portions of continuous travel.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct ThruTravelSeparationComposite : PathwayPortionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_ThruTravelSeparationComposite, PathwayPortionElement);
    friend struct ThruTravelSeparationCompositeHandler;

protected:
    //! @private
    explicit ThruTravelSeparationComposite(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(ThruTravelSeparationComposite)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(ThruTravelSeparationComposite)
}; // ThruTravelSeparationComposite

//=======================================================================================
//! A ThruTravelComposite specified using linear-referencing methods.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct ThruwayComposite : ThruTravelComposite, IMainLinearElementSource
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_ThruwayComposite, ThruTravelComposite);
    friend struct ThruwayCompositeHandler;

protected:
    //! @private
    explicit ThruwayComposite(CreateParams const& params) : T_Super(params) {}

    virtual Dgn::DgnElementCR _ILinearElementSourceToDgnElement() const override { return *this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(ThruwayComposite)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(ThruwayComposite)

    ROADRAILPHYSICAL_EXPORT static ThruwayCompositePtr Create(PathwayElementCR pathway);

    ROADRAILPHYSICAL_EXPORT ThruwayCompositeCPtr Insert(SideOnPathway side, Dgn::DgnDbStatus* status = nullptr);
}; // ThruwayComposite

//=======================================================================================
//! A ThruTravelSeparationComposite specified using linear-referencing methods.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct ThruwaySeparationComposite : ThruTravelSeparationComposite, IMainLinearElementSource
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_ThruwaySeparationComposite, ThruTravelSeparationComposite);
    friend struct ThruwaySeparationCompositeHandler;

protected:
    //! @private
    explicit ThruwaySeparationComposite(CreateParams const& params) : T_Super(params) {}

    virtual Dgn::DgnElementCR _ILinearElementSourceToDgnElement() const override { return *this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(ThruwaySeparationComposite)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(ThruwaySeparationComposite)

    ROADRAILPHYSICAL_EXPORT static ThruwaySeparationCompositePtr Create(PathwayElementCR pathway);
}; // ThruwaySeparationComposite

//=======================================================================================
//! Utility class facilitating some operations against ILinearElements in the 
//! context of the Road/Rail discipline.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct ILinearElementUtilities : NonCopyableClass
{
public:
    static Dgn::DgnClassId QueryILinearElementSourceRefersToGeneratedILinearElementsRelClassId(Dgn::DgnDbCR dgnDb)
        { return dgnDb.Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_ILinearElementSourceRefersToGeneratedILinearElements); }

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus SetAssociatedSignificantPointDef(LinearReferencing::ILinearElementCR linearElement, SignificantPointDefinitionCP significantPointDef);

}; // GeneratedILinearElementHelper


//=================================================================================
//! ElementHandler for Pathway Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PathwayElementHandler : Dgn::dgn_ElementHandler::Physical
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_PathwayElement, PathwayElement, PathwayElementHandler, Dgn::dgn_ElementHandler::Physical, ROADRAILPHYSICAL_EXPORT)
}; // PathwayElementHandler

//=================================================================================
//! ElementHandler for Roadway Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadwayHandler : PathwayElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_Roadway, Roadway, RoadwayHandler, PathwayElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RoadwayHandler

//=================================================================================
//! ElementHandler for Railway Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RailwayHandler : PathwayElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_Railway, Railway, RailwayHandler, PathwayElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RailwayHandler

//=================================================================================
//! ElementHandler for Pathway Portion Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PathwayPortionElementHandler : Dgn::dgn_ElementHandler::Physical
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_PathwayPortionElement, PathwayPortionElement, PathwayPortionElementHandler, Dgn::dgn_ElementHandler::Physical, ROADRAILPHYSICAL_EXPORT)
}; // PathwayPortionElementHandler

//=================================================================================
//! ElementHandler for ThruTravelComposite Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ThruTravelCompositeHandler : PathwayPortionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_ThruTravelComposite, ThruTravelComposite, ThruTravelCompositeHandler, PathwayPortionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // ThruTravelCompositeHandler

//=================================================================================
//! ElementHandler for ThruTravelComposite Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ThruTravelSeparationCompositeHandler : PathwayPortionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_ThruTravelSeparationComposite, ThruTravelSeparationComposite, ThruTravelSeparationCompositeHandler, PathwayPortionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // ThruTravelSeparationCompositeHandler

//=================================================================================
//! ElementHandler for ThruwayComposite Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ThruwayCompositeHandler : ThruTravelCompositeHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_ThruwayComposite, ThruwayComposite, ThruwayCompositeHandler, ThruTravelCompositeHandler, ROADRAILPHYSICAL_EXPORT)
}; // ThruwayCompositeHandler

//=================================================================================
//! ElementHandler for ThruwayComposite Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ThruwaySeparationCompositeHandler : ThruTravelSeparationCompositeHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_ThruwaySeparationComposite, ThruwaySeparationComposite, ThruwaySeparationCompositeHandler, ThruTravelSeparationCompositeHandler, ROADRAILPHYSICAL_EXPORT)
}; // ThruwayCompositeHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE