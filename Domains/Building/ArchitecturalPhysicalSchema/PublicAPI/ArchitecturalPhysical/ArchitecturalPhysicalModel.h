/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/PublicAPI/ArchitecturalPhysical/ArchitecturalPhysicalModel.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "PlanningDefinitions.h"

BEGIN_BENTLEY_PLANNING_NAMESPACE

//=======================================================================================
//! A model that contains all elements within one and only plan - the root Plan element, 
//! the child WorkBreakdown elements, and the leaf Activity elements. 
//! @ingroup GROUP_Planning
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PlanningModel : Dgn::InformationModel
{
DGNMODEL_DECLARE_MEMBERS(BP_CLASS_PlanningModel, Dgn::InformationModel);
friend struct PlanningModelHandler;

public:
    struct CreateParams : Dgn::InformationModel::CreateParams
    {
    DEFINE_T_SUPER(Dgn::InformationModel::CreateParams);

    protected:
        CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnClassId classId, bool isPrivate = false)
            : T_Super(dgndb, classId, Dgn::DgnElementId() /* WIP: Which element? */, isPrivate)
            {}

    public:
        //! @private
        //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
        CreateParams(Dgn::DgnModel::CreateParams const& params) : T_Super(params) {}

        //! Parameters to create a new instance of a PlanningModel.
        //! @param[in] dgndb The DgnDb for the new DgnModel
        //! @param[in] modeledElementId The DgnElementId of the element this this DgnModel is describing/modeling
        //! @param[in] isPrivate Optional parameter specifying that this model should @em not appear in lists shown to the user
        CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnElementId modeledElementId, bool isPrivate = false) :
            T_Super(dgndb, PlanningModel::QueryClassId(dgndb), modeledElementId, isPrivate)
            {}
    };

protected:
    PLANNING_EXPORT Dgn::DgnDbStatus _OnInsertElement(Dgn::DgnElementR element) override;

public:
    //! Constructor
    explicit PlanningModel(CreateParams const& params) : T_Super(params) {}

    //! Get the id of this PlanningModel 
    PlanningModelId GetId() const { return PlanningModelId(GetModelId().GetValue()); }

    //! Create a new planning model
    static PlanningModelPtr Create(CreateParams const& params) { return new PlanningModel(params); }

    //! Gets the PlanningModel by Id. If the model is not loaded, it loads it, but does not fill it with contained elements. 
    static PlanningModelPtr Get(Dgn::DgnDbCR dgndb, Dgn::DgnModelId id) { return dgndb.Models().Get<PlanningModel>(id); }

    //! Query the DgnClassId of the planning.PlanningModel ECClass in the specified DgnDb.
    //! @note This is a static method that always returns the DgnClassId of the planning.PlanningModel class - it does @em not return the class of a specific instance.
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR dgndb) { return Dgn::DgnClassId(dgndb.Schemas().GetClassId(BENTLEY_PLANNING_SCHEMA_NAME, BP_CLASS_PlanningModel)); }
};

//=================================================================================
//! ModelHandler 
//! @ingroup GROUP_Planning
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PlanningModelHandler : Dgn::dgn_ModelHandler::Information
{
    MODELHANDLER_DECLARE_MEMBERS(BP_CLASS_PlanningModel, PlanningModel, PlanningModelHandler, Dgn::dgn_ModelHandler::Information, PLANNING_EXPORT)
};

END_BENTLEY_PLANNING_NAMESPACE
