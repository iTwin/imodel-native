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

    virtual TravelwaySignificantPointDefCP _ToTravelwaySignificantPointDef() const { return nullptr; }
    virtual TravelwaySideSignificantPointDefCP _ToTravelwaySideSignificantPointDef() const { return nullptr; }
    virtual TravelwayStructureSignificantPointDefCP _ToTravelwayStructureSignificantPointDef() const { return nullptr; }

public:
    enum class ExpectedAtSurface : int32_t { Internal = 0, Top = 1, Bottom = 2, TopAndBottom = 4 };

    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(SignificantPointDefinition)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(SignificantPointDefinition)

    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnModelCR scope, Utf8StringCR value);
    ROADRAILPHYSICAL_EXPORT static SignificantPointDefinitionCPtr QueryByCode(Dgn::DefinitionModelCR model, Utf8StringCR pointCode);

    ExpectedAtSurface GetExpectedAtSurface() const { return static_cast<ExpectedAtSurface>(GetPropertyValueInt32("ExpectedAtSurface")); }
    void SetExpectedAtSurface(ExpectedAtSurface newVal) { SetPropertyValue("ExpectedAtSurface", static_cast<int32_t>(newVal)); }

    bool GetGenerateLinearElement() const { return GetPropertyValueBoolean("GenerateLinearElement"); }
    void SetGenerateLinearElement(bool newVal) { SetPropertyValue("GenerateLinearElement", newVal); }

    TravelwaySignificantPointDefCP ToTravelwaySignificantPointDef() const { return _ToTravelwaySignificantPointDef(); }
    TravelwaySideSignificantPointDefCP ToTravelwaySideSignificantPointDef() const { return _ToTravelwaySideSignificantPointDef(); }
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

    virtual TravelwaySignificantPointDefCP _ToTravelwaySignificantPointDef() const override { return this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwaySignificantPointDef)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(TravelwaySignificantPointDef)

    ROADRAILPHYSICAL_EXPORT static TravelwaySignificantPointDefPtr Create(Dgn::DefinitionModelCR model, Utf8StringCR pointCode, Utf8CP userLabel = nullptr);
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

    virtual TravelwaySideSignificantPointDefCP _ToTravelwaySideSignificantPointDef() const override { return this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwaySideSignificantPointDef)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(TravelwaySideSignificantPointDef)

    ROADRAILPHYSICAL_EXPORT static TravelwaySideSignificantPointDefPtr Create(Dgn::DefinitionModelCR model, Utf8StringCR pointCode, Utf8CP userLabel = nullptr);
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

    virtual TravelwayStructureSignificantPointDefCP _ToTravelwayStructureSignificantPointDef() const override { return this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwayStructureSignificantPointDef)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(TravelwayStructureSignificantPointDef)

    ROADRAILPHYSICAL_EXPORT static TravelwayStructureSignificantPointDefPtr Create(Dgn::DefinitionModelCR model, Utf8StringCR pointCode, Utf8CP userLabel = nullptr);
    ROADRAILPHYSICAL_EXPORT static TravelwayStructureSignificantPointDefCPtr CreateAndInsert(Dgn::DefinitionModelCR model, Utf8StringCR pointCode, Utf8CP userLabel = nullptr);
}; // TravelwayStructureSignificantPointDef


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
END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE