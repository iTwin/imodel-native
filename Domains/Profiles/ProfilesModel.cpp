/*--------------------------------------------------------------------------------------+
|
|     $Source: ProfilesModel.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI\ProfilesDomain\ProfilesModel.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(ProfilesModelHandler)

//---------------------------------------------------------------------------------------
// @bsimethod                                Arturas.Mizaras                    12/2017
//---------------------------------------------------------------------------------------
Dgn::DgnDbStatus ProfilesModel::_OnInsertElement(Dgn::DgnElementR element)
    {
    return T_Super::_OnInsertElement(element);
    }

ProfilesModelPtr ProfilesModel::Create(Dgn::DefinitionPartitionCR partition)
    {
    Dgn::DgnDbR db = partition.GetDgnDb();
    Dgn::DgnElementId modeledElementId = partition.GetElementId();
    Dgn::DgnClassId classId = db.Domains().GetClassId(ProfilesModelHandler::GetHandler());

    Dgn::DgnModelPtr model = ProfilesModelHandler::GetHandler().Create(Dgn::DgnModel::CreateParams(db, classId, modeledElementId));
    if (!model.IsValid())
        return nullptr;

    // Insert the new model into the DgnDb
    if (Dgn::DgnDbStatus::Success != model->Insert())
        return nullptr;

    return dynamic_cast<ProfilesModelP>(model.get());
    }

END_BENTLEY_PROFILES_NAMESPACE
