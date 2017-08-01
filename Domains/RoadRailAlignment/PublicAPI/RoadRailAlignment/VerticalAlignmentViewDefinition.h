/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/VerticalAlignmentViewDefinition.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailAlignmentApi.h"

BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

//=======================================================================================
// @bsiclass
// This view definition is not expected to be persisted.
//=======================================================================================
struct VerticalAlignmentViewDefinition : Dgn::SpatialViewDefinition
{
DGNELEMENT_DECLARE_MEMBERS(BRRA_CLASS_VerticalAlignmentViewDefinition, Dgn::SpatialViewDefinition);
friend struct VerticalAlignmentViewDefinitionHandler;

protected:
    bool m_allowRotation;

protected:
    void _AdjustAspectRatio(double windowAspect) override final;
    void _SetRotation(RotMatrixCR rot) override final;

    //! Disable insertion, update
    Dgn::DgnDbStatus _OnInsert() override final;
    Dgn::DgnDbStatus _OnUpdate(Dgn::DgnElementCR original) override final;

    //! @private
    explicit VerticalAlignmentViewDefinition(CreateParams const& params) : T_Super(params) {}
    VerticalAlignmentViewDefinition(Dgn::SpatialViewDefinitionR def);

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(VerticalAlignmentViewDefinition)

    void SetAllowRotation(bool value) { m_allowRotation = value; }
    bool GetAllowRotation() const { return m_allowRotation; }

    // Creates a definition off an existing SpatialViewDefinition
    ROADRAILALIGNMENT_EXPORT static VerticalAlignmentViewDefinitionPtr Create(Dgn::SpatialViewDefinitionCR def);

}; // VerticalAlignmentViewDefinition

//=======================================================================================
//! The ElementHandler for the VerticalAlignmentViewDefinition class
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE VerticalAlignmentViewDefinitionHandler : Dgn::ViewElementHandler::SpatialView
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRA_CLASS_VerticalAlignmentViewDefinition, VerticalAlignmentViewDefinition,
    VerticalAlignmentViewDefinitionHandler, Dgn::ViewElementHandler::SpatialView, ROADRAILALIGNMENT_EXPORT)
}; // VerticalAlignmentViewDefinitionHandler

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE