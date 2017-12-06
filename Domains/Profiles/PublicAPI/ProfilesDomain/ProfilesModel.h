#pragma once

//__PUBLISH_SECTION_START__
#include "ProfilesDomainDefinitions.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE ProfilesModel : Dgn::DefinitionModel
    {
    DGNMODEL_DECLARE_MEMBERS(PROFILES_ProfilesModel, Dgn::DefinitionModel);
    friend struct ProfilesModelHandler;

public:
    struct CreateParams : Dgn::DefinitionModel::CreateParams
        {
        DEFINE_T_SUPER(Dgn::DefinitionModel::CreateParams);

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
            T_Super(dgndb, ProfilesModel::QueryClassId(dgndb), modeledElementId, isPrivate)
            {}
        };

protected:
    PROFILES_DOMAIN_EXPORT Dgn::DgnDbStatus _OnInsertElement(Dgn::DgnElementR element) override;

public:
    //! Constructor
    explicit ProfilesModel(CreateParams const& params) : T_Super(params) {}

    //! Get the id of this PlanningModel 
    Dgn::DgnModelId GetId() const { return GetModelId(); }

    //! Create a new profiles model
    static ProfilesModelPtr Create(CreateParams const& params) { return new ProfilesModel(params); }
    PROFILES_DOMAIN_EXPORT static ProfilesModelPtr Create(Dgn::DefinitionPartitionCR partition);

    //! Gets the PlanningModel by Id. If the model is not loaded, it loads it, but does not fill it with contained elements. 
    static ProfilesModelPtr Get(Dgn::DgnDbCR dgndb, Dgn::DgnModelId id) { return dgndb.Models().Get<ProfilesModel>(id); }

    //! Query the DgnClassId of the planning.PlanningModel ECClass in the specified DgnDb.
    //! @note This is a static method that always returns the DgnClassId of the planning.PlanningModel class - it does @em not return the class of a specific instance.
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR dgndb) { return Dgn::DgnClassId(dgndb.Schemas().GetClassId(BENTLEY_PROFILES_SCHEMA_NAME, PROFILES_ProfilesModel)); }
    };

//=================================================================================
//! ModelHandler 
//! @ingroup GROUP_Planning
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ProfilesModelHandler : Dgn::dgn_ModelHandler::Definition
    {
    MODELHANDLER_DECLARE_MEMBERS(PROFILES_ProfilesModel, ProfilesModel, ProfilesModelHandler, Dgn::dgn_ModelHandler::Definition, PROFILES_DOMAIN_EXPORT)
    };

END_BENTLEY_PROFILES_NAMESPACE
