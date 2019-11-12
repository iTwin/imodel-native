/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailAlignment.h"

BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

//=======================================================================================
// @bsiclass
// Specialized view definition that works with the AlignmentXSViewController defined in CivilControls
// The CivilControls component is expected to register its view controller factory
//=======================================================================================
struct ClipPlanesViewDefinition : Dgn::SpatialViewDefinition
{
DGNELEMENT_DECLARE_MEMBERS(BRRA_CLASS_ClipPlanesViewDefinition, Dgn::SpatialViewDefinition);
friend struct ClipPlanesViewDefinitionHandler;

public:
    typedef Dgn::ViewControllerPtr(*IViewControllerFactory)(ClipPlanesViewDefinitionCR);
    ROADRAILALIGNMENT_EXPORT static void RegisterControllerFactory(IViewControllerFactory);

    //This has to stay AlignmentXSViewDefinition as that is the old name of the class, and the one used in the schema
    //Changing this to be the proper name "ClipPlanesViewDefinition" would require a change to platform and all user files
    static constexpr Utf8CP SYSTEM_VIEW_NAME = "AlignmentXSViewDefinition"; 

private:
    static IViewControllerFactory s_factory;

protected:
    bool m_allowRotation;
    Json::Value m_targetPlane;

protected:
    void _SetRotation(RotMatrixCR rot) override final;
    void _EnableCamera() override final {   }
    bool _SupportsCamera() const override final { return false; }
    Dgn::ViewControllerPtr _SupplyController() const override final;
    bool _ViewsModel(Dgn::DgnModelId modelId) override final;

    //! @private
    explicit ClipPlanesViewDefinition(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(ClipPlanesViewDefinition)
    DECLARE_ROADRAILALIGNMENT_ELEMENT_BASE_METHODS(ClipPlanesViewDefinition)

    void SetAllowRotation(bool value) { m_allowRotation = value; }
    bool GetAllowRotation() const { return m_allowRotation; }
    ROADRAILALIGNMENT_EXPORT void SetTargetPlane(JsonValueCR target) { m_targetPlane = target; }
    ROADRAILALIGNMENT_EXPORT Json::Value GetTargetPlane() const { return m_targetPlane; }



    //! Construct a SpatialViewDefinition in the specified DefinitionModel
    ROADRAILALIGNMENT_EXPORT ClipPlanesViewDefinition(ConfigurationModelCR model, Utf8StringCR name, Dgn::CategorySelectorR categories, Dgn::DisplayStyle3dR displayStyle, Dgn::ModelSelectorR modelSelector);

    ROADRAILALIGNMENT_EXPORT static Dgn::DgnViewId QuerySystemViewId(Dgn::SubjectCR subject);
}; // ClipPlanesViewDefinition


//__PUBLISH_SECTION_END__
//=======================================================================================
//! The ElementHandler for the ClipPlanesViewDefinition class
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ClipPlanesViewDefinitionHandler : Dgn::ViewElementHandler::SpatialView
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRA_CLASS_ClipPlanesViewDefinition, ClipPlanesViewDefinition,
    ClipPlanesViewDefinitionHandler, Dgn::ViewElementHandler::SpatialView, ROADRAILALIGNMENT_EXPORT)
}; // ClipPlanesViewDefinition

//__PUBLISH_SECTION_START__
END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE
