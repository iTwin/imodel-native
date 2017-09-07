/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/AlignmentProfileViewDefinition.h $
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
// Specialized view definition that works with the AlignmentProfileViewController defined in CivilControls
// The CivilControls component is expected to register its view controller factory
//=======================================================================================
struct AlignmentProfileViewDefinition : Dgn::SpatialViewDefinition
{
DGNELEMENT_DECLARE_MEMBERS(BRRA_CLASS_AlignmentProfileViewDefinition, Dgn::SpatialViewDefinition);
friend struct AlignmentProfileViewDefinitionHandler;

public:
    typedef Dgn::ViewControllerPtr(*IViewControllerFactory)(AlignmentProfileViewDefinitionCR);
    ROADRAILALIGNMENT_EXPORT static void RegisterControllerFactory(IViewControllerFactory);

private:
    static IViewControllerFactory s_factory;

protected:
    bool m_allowRotation;

protected:
    void _AdjustAspectRatio(double windowAspect) override final;
    void _SetRotation(RotMatrixCR rot) override final;
    void _EnableCamera() override final {}
    bool _SupportsCamera() const override final { return false; }
    Dgn::ViewControllerPtr _SupplyController() const override final;

    //! @private
    explicit AlignmentProfileViewDefinition(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(AlignmentProfileViewDefinition)
    DECLARE_ROADRAILALIGNMENT_ELEMENT_BASE_METHODS(AlignmentProfileViewDefinition)

    void SetAllowRotation(bool value) { m_allowRotation = value; }
    bool GetAllowRotation() const { return m_allowRotation; }

    //! Construct a SpatialViewDefinition in the specified DefinitionModel
    ROADRAILALIGNMENT_EXPORT AlignmentProfileViewDefinition(Dgn::DefinitionModelR model, Utf8StringCR name, Dgn::CategorySelectorR categories, Dgn::DisplayStyle3dR displayStyle, Dgn::ModelSelectorR modelSelector);

}; // AlignmentProfileViewDefinition

//=======================================================================================
//! The ElementHandler for the AlignmentProfileViewDefinition class
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AlignmentProfileViewDefinitionHandler : Dgn::ViewElementHandler::SpatialView
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRA_CLASS_AlignmentProfileViewDefinition, AlignmentProfileViewDefinition,
    AlignmentProfileViewDefinitionHandler, Dgn::ViewElementHandler::SpatialView, ROADRAILALIGNMENT_EXPORT)
}; // AlignmentProfileViewDefinitionHandler

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE
