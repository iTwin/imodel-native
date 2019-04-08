/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/AlignmentProfileViewDefinition.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    static constexpr Utf8CP SYSTEM_VIEW_NAME = "AlignmentProfileViewDefinition";

private:
    static IViewControllerFactory s_factory;

protected:
    //! @private
    bool m_allowRotation;

protected:
    //! @private
    Dgn::ViewControllerPtr _SupplyController() const override final;

    //! @private
    void _AdjustAspectRatio(double windowAspect) override final;
   
    //! @private
    void _SetRotation(RotMatrixCR rot) override final;

    //! @private
    bool _ViewsModel(Dgn::DgnModelId modelId) override final;

    //! @private
    void _EnableCamera() override final {}
    
    //! @private
    bool _SupportsCamera() const override final { return false; }

    //! @private
    explicit AlignmentProfileViewDefinition(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(AlignmentProfileViewDefinition)
    DECLARE_ROADRAILALIGNMENT_ELEMENT_BASE_METHODS(AlignmentProfileViewDefinition)

    //! Control whether or not this ViewDefinition supports rotation
    //! @param[in] value true to allow rotation, false to disallow
    void SetAllowRotation(bool value) { m_allowRotation = value; }

    //! Check whether or not this ViewDefinition supports rotation
    //! @return true if rotation is allowed, otherwise false.
    bool GetAllowRotation() const { return m_allowRotation; }

    //! Construct a SpatialViewDefinition in the specified DefinitionModel
    ROADRAILALIGNMENT_EXPORT AlignmentProfileViewDefinition(ConfigurationModelCR model, Utf8StringCR name, Dgn::CategorySelectorR categories, Dgn::DisplayStyle3dR displayStyle, Dgn::ModelSelectorR modelSelector);

    //! Query for the ViewId of this ViewDefinition
    //! @param[in] db The DgnDb containing the View
    //! @return The DgnViewId of this ViewDefinition.  Caller must check .IsValid() on the returned ID before using it.
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnViewId QuerySystemViewId(Dgn::SubjectCR subject);
    //! @param[in] db The DgnDb containing the View
    //! @return The DgnViewId of this ViewDefinition.  Caller must check .IsValid() on the returned ID before using it.
}; // AlignmentProfileViewDefinition


//__PUBLISH_SECTION_END__
//=======================================================================================
//! The ElementHandler for the AlignmentProfileViewDefinition class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AlignmentProfileViewDefinitionHandler : Dgn::ViewElementHandler::SpatialView
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRA_CLASS_AlignmentProfileViewDefinition, AlignmentProfileViewDefinition,
    AlignmentProfileViewDefinitionHandler, Dgn::ViewElementHandler::SpatialView, ROADRAILALIGNMENT_EXPORT)
}; // AlignmentProfileViewDefinitionHandler

//__PUBLISH_SECTION_START__
END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE
