/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/AlignmentModel.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailAlignment.h"

//TODO DIEGO I'm very unclear about what parts of the model we want/need to expose to SDK consumers. Does someone who's trying to get information about an alignment CARE about the model?  Or the Alignments (breakdown) model?

BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

//=======================================================================================
//! Model to contain and manage Alignment elements
//=======================================================================================
struct AlignmentModel : Dgn::SpatialLocationModel
{
    DGNMODEL_DECLARE_MEMBERS(BRRA_CLASS_AlignmentModel, Dgn::SpatialLocationModel);
    friend struct AlignmentModelHandler;

public:
    struct CreateParams : T_Super::CreateParams
    {
    DEFINE_T_SUPER(AlignmentModel::T_Super::CreateParams);

    //! Parameters to create a new instance of an AlignmentModel.
    //! @param[in] dgndb The DgnDb for the new DgnModel
    //! @param[in] modeledElementId The DgnElementId of the element this this DgnModel is describing/modeling
    CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnElementId modeledElementId)
        : T_Super(dgndb, AlignmentModel::QueryClassId(dgndb), modeledElementId)
        {}

    //! @private
    //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
    CreateParams(DgnModel::CreateParams const& params) : T_Super(params) {}
    }; // CreateParams

protected:
    explicit AlignmentModel(CreateParams const& params) : T_Super(params) { SetIsPrivate(true); }

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(AlignmentModel)

    //! Query for the InformationModel containing all of the HorizontalAlignments
    ROADRAILALIGNMENT_EXPORT HorizontalAlignmentsCPtr QueryHorizontalPartition() const;
    
    //! Query for DgnElementSet containing all of the AlignmentIds in this AlignmentModel
    ROADRAILALIGNMENT_EXPORT Dgn::DgnElementIdSet QueryAlignmentIds() const;
    ROADRAILALIGNMENT_EXPORT Dgn::SubjectCPtr GetParentSubject() const;

    ROADRAILALIGNMENT_EXPORT static AlignmentModelPtr Query(Dgn::SubjectCR parentSubject, Utf8CP partitionName);


    //! @private
    static AlignmentModelPtr Create(CreateParams const& params) { return new AlignmentModel(params); }


    static AlignmentModelCPtr Get(Dgn::DgnDbR db, Dgn::DgnModelId id) { return db.Models().Get< AlignmentModel >(id); }    
}; // AlignmentModel

//=======================================================================================
//! Model to contain and manage a Horizontal Alignment element
//=======================================================================================
struct HorizontalAlignmentModel : Dgn::SpatialLocationModel
{
    DGNMODEL_DECLARE_MEMBERS(BRRA_CLASS_HorizontalAlignmentModel, Dgn::SpatialLocationModel);
    friend struct HorizontalAlignmentModelHandler;

public:
    struct CreateParams : T_Super::CreateParams
    {
    DEFINE_T_SUPER(HorizontalAlignmentModel::T_Super::CreateParams);

    //! Parameters to create a new instance of a HorizontalAlignmentModel.
    //! @param[in] dgndb The DgnDb for the new DgnModel
    //! @param[in] modeledElementId The DgnElementId of the element this this DgnModel is describing/modeling
    CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnElementId modeledElementId)
        : T_Super(dgndb, HorizontalAlignmentModel::QueryClassId(dgndb), modeledElementId)
        {}

    //! @private
    //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
    CreateParams(DgnModel::CreateParams const& params) : T_Super(params) {}
    }; // CreateParams

protected:
    explicit HorizontalAlignmentModel(CreateParams const& params) : T_Super(params) { SetIsPrivate(true); }

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(HorizontalAlignmentModel)

    static HorizontalAlignmentModelPtr Create(CreateParams const& params) { return new HorizontalAlignmentModel(params); }
    static HorizontalAlignmentModelCPtr Get(Dgn::DgnDbR db, Dgn::DgnModelId id) { return db.Models().Get<HorizontalAlignmentModel>(id); }
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnModelId QueryBreakDownModelId(AlignmentModelCR model);
}; // HorizontalAlignmentModel

//=======================================================================================
//! Model to contain and manage Vertical Alignment elements
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

    static VerticalAlignmentModelPtr Create(CreateParams const& params) { return new VerticalAlignmentModel(params); }
    static VerticalAlignmentModelCPtr Get(Dgn::DgnDbR db, Dgn::DgnModelId id) { return db.Models().Get<VerticalAlignmentModel>(id); }
    static VerticalAlignmentModelPtr GetForEdit(Dgn::DgnDbR db, Dgn::DgnModelId id) { return db.Models().Get<VerticalAlignmentModel>(id); }
}; // VerticalAlignmentModel


//=======================================================================================
//! The ModelHandler for AlignmentModel
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AlignmentModelHandler : Dgn::dgn_ModelHandler::SpatialLocation
{
    MODELHANDLER_DECLARE_MEMBERS(BRRA_CLASS_AlignmentModel, AlignmentModel, AlignmentModelHandler, Dgn::dgn_ModelHandler::SpatialLocation, ROADRAILALIGNMENT_EXPORT)
}; // AlignmentModelHandler

//=======================================================================================
//! The ModelHandler for HorizontalAlignmentModel
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE HorizontalAlignmentModelHandler : Dgn::dgn_ModelHandler::SpatialLocation
{
    MODELHANDLER_DECLARE_MEMBERS(BRRA_CLASS_HorizontalAlignmentModel, HorizontalAlignmentModel, HorizontalAlignmentModelHandler, Dgn::dgn_ModelHandler::SpatialLocation, ROADRAILALIGNMENT_EXPORT)
}; // HorizontalAlignmentModelHandler

//=======================================================================================
//! The ModelHandler for VerticalAlignmentModel
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE VerticalAlignmentModelHandler : Dgn::dgn_ModelHandler::Geometric2d
{
    MODELHANDLER_DECLARE_MEMBERS(BRRA_CLASS_VerticalAlignmentModel, VerticalAlignmentModel, VerticalAlignmentModelHandler, Dgn::dgn_ModelHandler::Geometric2d, ROADRAILALIGNMENT_EXPORT)
}; // VerticalAlignmentModelHandler

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE