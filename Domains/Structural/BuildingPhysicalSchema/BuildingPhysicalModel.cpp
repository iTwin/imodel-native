/*--------------------------------------------------------------------------------------+
|
|     $Source: BuildingPhysicalSchema/BuildingPhysicalModel.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BuildingPhysicalSchemaInternal.h"


BEGIN_BENTLEY_BUILDING_PHYSICAL_NAMESPACE

HANDLER_DEFINE_MEMBERS(BuildingTypeDefinitionModelHandler)
HANDLER_DEFINE_MEMBERS(BuildingPhysicalModelHandler)


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BuildingPhysicalModelPtr BuildingPhysicalModel::Create(PhysicalPartitionCR partition)
    {
    DgnDbR db = partition.GetDgnDb();
    DgnElementId modeledElementId = partition.GetElementId();
    DgnClassId classId = db.Domains().GetClassId(BuildingPhysicalModelHandler::GetHandler());

    DgnModelPtr model = BuildingPhysicalModelHandler::GetHandler().Create(DgnModel::CreateParams(db, classId, modeledElementId));
    if (!model.IsValid())
        return nullptr;

    // Insert the new model into the DgnDb
    if (DgnDbStatus::Success != model->Insert())
        return nullptr;


    return dynamic_cast<BuildingPhysicalModelP>(model.get());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
DgnDbStatus BuildingPhysicalModel::_OnInsertElement(DgnElementR element)
    {
  //  if (nullptr == dynamic_cast<ArchitecturalBaseElementCP>(&element))
  //      return DgnDbStatus::WrongElement;

    return T_Super::_OnInsertElement(element);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BuildingTypeDefinitionModelPtr  BuildingTypeDefinitionModel::Create(DefinitionPartitionCR partition)
    {
    DgnDbR db = partition.GetDgnDb();
    DgnElementId modeledElementId = partition.GetElementId();
    DgnClassId classId = db.Domains().GetClassId(BuildingTypeDefinitionModelHandler::GetHandler());

    DgnModelPtr model = BuildingTypeDefinitionModelHandler::GetHandler().Create(DgnModel::CreateParams(db, classId, modeledElementId));
    if (!model.IsValid())
        return nullptr;

    // Insert the new model into the DgnDb
    if (DgnDbStatus::Success != model->Insert())
        return nullptr;

    return dynamic_cast<BuildingTypeDefinitionModelP>(model.get());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
DgnDbStatus BuildingTypeDefinitionModel::_OnInsertElement(DgnElementR element)
    {
  //  if (nullptr == dynamic_cast<DoorTypeCP>(&element))
  //      return DgnDbStatus::WrongElement;

    return T_Super::_OnInsertElement(element);
    }


END_BENTLEY_BUILDING_PHYSICAL_NAMESPACE


