/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/RoadDesignSpeed.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysical.h"
#include "Pathway.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! Model for Design-Speed Definition Table elements.
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadwayStandardsModel : Dgn::DefinitionModel
{
DGNMODEL_DECLARE_MEMBERS(BRRP_CLASS_RoadwayStandardsModel, Dgn::DefinitionModel);
friend struct RoadwayStandardsModelHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(RoadwayStandardsModel::T_Super::CreateParams);

        //! Parameters to create a new instance of an RoadwayStandardsModel.
        //! @param[in] dgndb The DgnDb for the new DgnModel
        //! @param[in] modeledElementId The DgnElementId of the element this this DgnModel is describing/modeling
        CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnElementId modeledElementId)
            : T_Super(dgndb, RoadwayStandardsModel::QueryClassId(dgndb), modeledElementId)
            {}

        //! @private
        //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
        CreateParams(DgnModel::CreateParams const& params) : T_Super(params) {}
        }; // CreateParams

protected:
    explicit RoadwayStandardsModel(CreateParams const& params) : T_Super(params) { }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadwayStandardsModel)

    static RoadwayStandardsModelPtr Create(CreateParams const& params) { return new RoadwayStandardsModel(params); }
    static RoadwayStandardsModelCPtr Get(Dgn::DgnDbR db, Dgn::DgnModelId id) { return db.Models().Get< RoadwayStandardsModel >(id); }

    ROADRAILPHYSICAL_EXPORT static RoadwayStandardsModelPtr Query(Dgn::SubjectCR parentSubject, Utf8CP modelName = nullptr);
}; // RoadwayStandardsModel


//=======================================================================================
//! The ModelHandler for RoadwayStandards Models
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadwayStandardsModelHandler : Dgn::dgn_ModelHandler::Definition
{
MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadwayStandardsModel, RoadwayStandardsModel, RoadwayStandardsModelHandler, Dgn::dgn_ModelHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // RoadwayStandardsModelHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE