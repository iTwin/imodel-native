/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailAlignment.h"

BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

//=======================================================================================
//! Utility class for models containing Alignment elements
//=======================================================================================
struct AlignmentModelUtilities
{
private:
    AlignmentModelUtilities() {}

public:
    //! Query for DgnElementSet containing all of the AlignmentIds in this AlignmentModel
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnElementIdSet QueryAlignmentIds(Dgn::SpatialModelCR alignmentModel);
}; // AlignmentModelUtilities

//=======================================================================================
//! Model to contain and manage Vertical Alignment elements for a particular Alignment
//=======================================================================================
struct VerticalAlignmentModel : Dgn::GeometricModel2d
{
    DGNMODEL_DECLARE_MEMBERS(BRRA_CLASS_VerticalAlignmentModel, Dgn::GeometricModel2d);
    friend struct VerticalAlignmentModelHandler;

public:
    struct CreateParams : T_Super::CreateParams
    {
    DEFINE_T_SUPER(VerticalAlignmentModel::T_Super::CreateParams);

    //! Parameters to create a new instance of a VerticalAlignmentModel.
    //! @param[in] dgndb The DgnDb for the new DgnModel
    //! @param[in] modeledElementId The DgnElementId of the element this this DgnModel is describing/modeling
    CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnElementId modeledElementId)
        : T_Super(dgndb, VerticalAlignmentModel::QueryClassId(dgndb), modeledElementId)
        {}

    //! @private
    //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
    CreateParams(DgnModel::CreateParams const& params) : T_Super(params) {}
    }; // CreateParams

protected:
    explicit VerticalAlignmentModel(CreateParams const& params) : T_Super(params) { SetIsPrivate(true); }

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(VerticalAlignmentModel)
    ROADRAILALIGNMENT_EXPORT AlignmentCPtr GetAlignment() const;

    //! @private
    static VerticalAlignmentModelPtr Create(CreateParams const& params) { return new VerticalAlignmentModel(params); }
    //! @private
    static VerticalAlignmentModelCPtr Get(Dgn::DgnDbR db, Dgn::DgnModelId id) { return db.Models().Get<VerticalAlignmentModel>(id); }
    //! @private
    static VerticalAlignmentModelPtr GetForEdit(Dgn::DgnDbR db, Dgn::DgnModelId id) { return db.Models().Get<VerticalAlignmentModel>(id); }
}; // VerticalAlignmentModel


//__PUBLISH_SECTION_END__
//=======================================================================================
//! The ModelHandler for VerticalAlignmentModel
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE VerticalAlignmentModelHandler : Dgn::dgn_ModelHandler::Geometric2d
{
    MODELHANDLER_DECLARE_MEMBERS(BRRA_CLASS_VerticalAlignmentModel, VerticalAlignmentModel, VerticalAlignmentModelHandler, Dgn::dgn_ModelHandler::Geometric2d, ROADRAILALIGNMENT_EXPORT)
}; // VerticalAlignmentModelHandler

//__PUBLISH_SECTION_START__
END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE