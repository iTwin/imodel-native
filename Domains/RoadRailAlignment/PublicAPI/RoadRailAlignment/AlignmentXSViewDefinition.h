/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/AlignmentXSViewDefinition.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailAlignment.h"

BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

//=======================================================================================
// @bsiclass
// Specialized view definition that works with the AlignmentXSViewController defined in CivilControls
// The CivilControls component is expected to register its view controller factory
//=======================================================================================
struct AlignmentXSViewDefinition : Dgn::SpatialViewDefinition
{
DGNELEMENT_DECLARE_MEMBERS(BRRA_CLASS_AlignmentXSViewDefinition, Dgn::SpatialViewDefinition);
friend struct AlignmentXSViewDefinitionHandler;

public:
    typedef Dgn::ViewControllerPtr(*IViewControllerFactory)(AlignmentXSViewDefinitionCR);
    ROADRAILALIGNMENT_EXPORT static void RegisterControllerFactory(IViewControllerFactory);
    static constexpr Utf8CP SYSTEM_VIEW_NAME = "AlignmentXSViewDefinition";

private:
    static IViewControllerFactory s_factory;

protected:
    bool m_allowRotation;

protected:
    void _SetRotation(RotMatrixCR rot) override final;
    void _EnableCamera() override final {}
    bool _SupportsCamera() const override final { return false; }
    Dgn::ViewControllerPtr _SupplyController() const override final;

    //! @private
    explicit AlignmentXSViewDefinition(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(AlignmentXSViewDefinition)
    DECLARE_ROADRAILALIGNMENT_ELEMENT_BASE_METHODS(AlignmentXSViewDefinition)

    void SetAllowRotation(bool value) { m_allowRotation = value; }
    bool GetAllowRotation() const { return m_allowRotation; }

    //! Construct a SpatialViewDefinition in the specified DefinitionModel
    ROADRAILALIGNMENT_EXPORT AlignmentXSViewDefinition(Dgn::DefinitionModelR model, Utf8StringCR name, Dgn::CategorySelectorR categories, Dgn::DisplayStyle3dR displayStyle, Dgn::ModelSelectorR modelSelector);

    ROADRAILALIGNMENT_EXPORT static Dgn::DgnViewId QuerySystemViewId(Dgn::DgnDbR db);
}; // AlignmentXSViewDefinition

//=======================================================================================
//! The ElementHandler for the AlignmentXSViewDefinition class
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AlignmentXSViewDefinitionHandler : Dgn::ViewElementHandler::SpatialView
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRA_CLASS_AlignmentXSViewDefinition, AlignmentXSViewDefinition,
    AlignmentXSViewDefinitionHandler, Dgn::ViewElementHandler::SpatialView, ROADRAILALIGNMENT_EXPORT)
}; // AlignmentXSViewDefinition

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE
