/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/TypicalSectionPoint.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysical.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! Base class for point constructs in a TypicalSection portion definition.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SignificantPointDefinition : Dgn::DefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_SignificantPointDefinition, Dgn::DefinitionElement);
friend struct SignificantPointDefinitionHandler;

protected:
    //! @private
    explicit SignificantPointDefinition(CreateParams const& params);
    //! @private
    virtual TravelwaySignificantPointDefCP _ToTravelwaySignificantPointDef() const { return nullptr; }
    //! @private
    virtual TravelwaySideSignificantPointDefCP _ToTravelwaySideSignificantPointDef() const { return nullptr; }
    //! @private
    virtual TravelwayStructureSignificantPointDefCP _ToTravelwayStructureSignificantPointDef() const { return nullptr; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(SignificantPointDefinition)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(SignificantPointDefinition)

    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DefinitionModelCR scope, Utf8StringCR value);
    
    //! Query for a Significant Point Definition based on a Code value
    //! @return A Significant Point Definition or nullptr if none could be found for the given Code value
    ROADRAILPHYSICAL_EXPORT static SignificantPointDefinitionCPtr QueryByCode(Dgn::DefinitionModelCR model, Utf8StringCR pointCode);

    //! Cast this SignificantPointDefinition into a TravelwaySignificantPointDef
    //! @return A TravelwaySignificantPointDef or nullptr if this CorridorPortionElement is not a TravelwaySignificantPointDef
    TravelwaySignificantPointDefCP ToTravelwaySignificantPointDef() const { return _ToTravelwaySignificantPointDef(); }
    //! Cast this SignificantPointDefinition into a TravelwaySignificantPointDef
    //! @return A TravelwaySignificantPointDef or nullptr if this SignificantPointDefinition is not a TravelwaySignificantPointDef
    TravelwaySideSignificantPointDefCP ToTravelwaySideSignificantPointDef() const { return _ToTravelwaySideSignificantPointDef(); }
    //! Cast this SignificantPointDefinition into a TravelwayStructureSignificantPointDef
    //! @return A TravelwayStructureSignificantPointDef or nullptr if this SignificantPointDefinition is not a TravelwayStructureSignificantPointDef
    TravelwayStructureSignificantPointDefCP ToTravelwayStructureSignificantPointDef() const { return _ToTravelwayStructureSignificantPointDef(); }
}; // SignificantPointDefinition

//=======================================================================================
//! Point constructs in a Travelway portion definition.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwaySignificantPointDef : SignificantPointDefinition
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TravelwaySignificantPointDef, SignificantPointDefinition);
friend struct TravelwaySignificantPointDefHandler;

protected:
    //! @private
    explicit TravelwaySignificantPointDef(CreateParams const& params) : T_Super(params) {}    
    //! @private
    virtual TravelwaySignificantPointDefCP _ToTravelwaySignificantPointDef() const override { return this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwaySignificantPointDef)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(TravelwaySignificantPointDef)

    //! @private
    ROADRAILPHYSICAL_EXPORT static TravelwaySignificantPointDefPtr Create(Dgn::DefinitionModelCR model, Utf8StringCR pointCode, Utf8CP userLabel = nullptr);
    //! @private
    ROADRAILPHYSICAL_EXPORT static TravelwaySignificantPointDefCPtr CreateAndInsert(Dgn::DefinitionModelCR model, Utf8StringCR pointCode, Utf8CP userLabel = nullptr);
}; // TravelwaySignificantPointDef

//=======================================================================================
//! Point constructs in a TravelwaySide portion definition.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwaySideSignificantPointDef : SignificantPointDefinition
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TravelwaySideSignificantPointDef, SignificantPointDefinition);
friend struct TravelwaySideSignificantPointDefHandler;

protected:
    //! @private
    explicit TravelwaySideSignificantPointDef(CreateParams const& params) : T_Super(params) {}
    //! @private
    virtual TravelwaySideSignificantPointDefCP _ToTravelwaySideSignificantPointDef() const override { return this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwaySideSignificantPointDef)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(TravelwaySideSignificantPointDef)

    //! @private
    ROADRAILPHYSICAL_EXPORT static TravelwaySideSignificantPointDefPtr Create(Dgn::DefinitionModelCR model, Utf8StringCR pointCode, Utf8CP userLabel = nullptr);
    //! @private
    ROADRAILPHYSICAL_EXPORT static TravelwaySideSignificantPointDefCPtr CreateAndInsert(Dgn::DefinitionModelCR model, Utf8StringCR pointCode, Utf8CP userLabel = nullptr);
}; // TravelwaySideSignificantPointDef

//=======================================================================================
//! Point constructs in a TravelwayStructure portion definition.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayStructureSignificantPointDef : SignificantPointDefinition
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TravelwayStructureSignificantPointDef, SignificantPointDefinition);
friend struct TravelwayStructureSignificantPointDefHandler;

protected:
    //! @private
    explicit TravelwayStructureSignificantPointDef(CreateParams const& params) : T_Super(params) {}
    //! @private
    virtual TravelwayStructureSignificantPointDefCP _ToTravelwayStructureSignificantPointDef() const override { return this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwayStructureSignificantPointDef)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(TravelwayStructureSignificantPointDef)

    //! @private
    ROADRAILPHYSICAL_EXPORT static TravelwayStructureSignificantPointDefPtr Create(Dgn::DefinitionModelCR model, Utf8StringCR pointCode, Utf8CP userLabel = nullptr);
    //! @private
    ROADRAILPHYSICAL_EXPORT static TravelwayStructureSignificantPointDefCPtr CreateAndInsert(Dgn::DefinitionModelCR model, Utf8StringCR pointCode, Utf8CP userLabel = nullptr);
}; // TravelwayStructureSignificantPointDef


//__PUBLISH_SECTION_END__
//=================================================================================
//! ElementHandler for Typical Section Point Definitions
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SignificantPointDefinitionHandler : Dgn::dgn_ElementHandler::Definition
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_SignificantPointDefinition, SignificantPointDefinition, SignificantPointDefinitionHandler, Dgn::dgn_ElementHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // SignificantPointDefinitionHandler

//=================================================================================
//! ElementHandler for Travelway Point Definitions
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwaySignificantPointDefHandler : SignificantPointDefinitionHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwaySignificantPointDef, TravelwaySignificantPointDef, TravelwaySignificantPointDefHandler, SignificantPointDefinitionHandler, ROADRAILPHYSICAL_EXPORT)
}; // TravelwaySignificantPointDefHandler

//=================================================================================
//! ElementHandler for TravelwaySide Point Definitions
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwaySideSignificantPointDefHandler : SignificantPointDefinitionHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwaySideSignificantPointDef, TravelwaySideSignificantPointDef, TravelwaySideSignificantPointDefHandler, SignificantPointDefinitionHandler, ROADRAILPHYSICAL_EXPORT)
}; // TravelwaySideSignificantPointDefHandler

//=================================================================================
//! ElementHandler for TravelwayStructure Point Definitions
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayStructureSignificantPointDefHandler : SignificantPointDefinitionHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwayStructureSignificantPointDef, TravelwayStructureSignificantPointDef, TravelwayStructureSignificantPointDefHandler, SignificantPointDefinitionHandler, ROADRAILPHYSICAL_EXPORT)
}; // TravelwayStructureSignificantPointDefHandler

//__PUBLISH_SECTION_START__
END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE