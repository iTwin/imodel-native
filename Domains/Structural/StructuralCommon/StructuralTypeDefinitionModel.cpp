/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralCommon/StructuralTypeDefinitionModel.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <StructuralDomain\StructuralCommon\StructuralTypeDefinitionModel.h>

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

HANDLER_DEFINE_MEMBERS(StructuralTypeDefinitionModelHandler)


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
StructuralTypeDefinitionModelPtr  StructuralTypeDefinitionModel::Create(Dgn::DefinitionPartitionCR partition)
    {
    Dgn::DgnDbR db = partition.GetDgnDb();
    Dgn::DgnElementId modeledElementId = partition.GetElementId();
    Dgn::DgnClassId classId = db.Domains().GetClassId(StructuralTypeDefinitionModelHandler::GetHandler());

    Dgn::DgnModelPtr model = StructuralTypeDefinitionModelHandler::GetHandler().Create(Dgn::DgnModel::CreateParams(db, classId, modeledElementId));
    if (!model.IsValid())
        return nullptr;

    // Insert the new model into the DgnDb
    if (Dgn::DgnDbStatus::Success != model->Insert())
        return nullptr;

    return dynamic_cast<StructuralTypeDefinitionModelP>(model.get());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::DgnDbStatus StructuralTypeDefinitionModel::_OnInsertElement(Dgn::DgnElementR element)
    {
    return T_Super::_OnInsertElement(element);
    }


END_BENTLEY_STRUCTURAL_NAMESPACE

