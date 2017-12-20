/*--------------------------------------------------------------------------------------+
|
|     $Source: ProfilesModel.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesDomain\ProfilesModel.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(ProfileDefinitionModelHandler)

//---------------------------------------------------------------------------------------
// @bsimethod                                Arturas.Mizaras                    12/2017
//---------------------------------------------------------------------------------------
Dgn::DgnDbStatus ProfileDefinitionModel::_OnInsertElement(Dgn::DgnElementR element)
    {
    return T_Super::_OnInsertElement(element);
    }

ProfileDefinitionModelPtr ProfileDefinitionModel::Create(Dgn::DefinitionPartitionCR partition)
    {
    Dgn::DgnDbR db = partition.GetDgnDb();
    Dgn::DgnElementId modeledElementId = partition.GetElementId();
    Dgn::DgnClassId classId = db.Domains().GetClassId(ProfileDefinitionModelHandler::GetHandler());

    Dgn::DgnModelPtr model = ProfileDefinitionModelHandler::GetHandler().Create(Dgn::DgnModel::CreateParams(db, classId, modeledElementId));
    if (!model.IsValid())
        return nullptr;

    // Insert the new model into the DgnDb
    if (Dgn::DgnDbStatus::Success != model->Insert())
        return nullptr;

    return dynamic_cast<ProfileDefinitionModelP>(model.get());
    }

END_BENTLEY_PROFILES_NAMESPACE
