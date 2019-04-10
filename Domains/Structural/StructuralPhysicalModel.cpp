/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralPhysicalModel.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI\StructuralPhysicalModel.h"

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

HANDLER_DEFINE_MEMBERS(StructuralPhysicalModelHandler)


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
StructuralPhysicalModelPtr StructuralPhysicalModel::Create(Dgn::PhysicalPartitionCR partition)
    {
    Dgn::DgnDbR db = partition.GetDgnDb();
    Dgn::DgnElementId modeledElementId = partition.GetElementId();
    Dgn::DgnClassId classId = db.Domains().GetClassId(StructuralPhysicalModelHandler::GetHandler());

    Dgn::DgnModelPtr model = StructuralPhysicalModelHandler::GetHandler().Create(Dgn::DgnModel::CreateParams(db, classId, modeledElementId));
    if (!model.IsValid())
        return nullptr;

    // Insert the new model into the DgnDb
    if (Dgn::DgnDbStatus::Success != model->Insert())
        return nullptr;

    return dynamic_cast<StructuralPhysicalModelP>(model.get());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::DgnDbStatus StructuralPhysicalModel::_OnInsertElement(Dgn::DgnElementR element)
    {
    return T_Super::_OnInsertElement(element);
    }


END_BENTLEY_STRUCTURAL_NAMESPACE

