/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/Door.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ArchitecturalPhysicalSchemaInternal.h"
#include <BuildingPhysical/BuildingPhysicalApi.h>


BEGIN_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE

HANDLER_DEFINE_MEMBERS(DoorHandler)
HANDLER_DEFINE_MEMBERS(DoorTypeHandler)
HANDLER_DEFINE_MEMBERS(ArchitecturalBaseElementHandler)

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
DoorPtr Door::Create(BuildingPhysical::BuildingPhysicalModelR model)
    {
    DgnDbR db = model.GetDgnDb();
    DgnModelId modelId = model.GetModelId();
    DgnCategoryId categoryId = ArchitecturalPhysicalCategory::QueryBuildingPhysicalDoorCategoryId(db);
    DgnClassId classId = db.Domains().GetClassId(DoorHandler::GetHandler());

    DoorPtr door = new Door(CreateParams(db, modelId, classId, categoryId));
    return door;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
DoorTypePtr DoorType::Create(BuildingPhysical::BuildingTypeDefinitionModelR model)
    {
    DgnDbR db = model.GetDgnDb();
    DgnModelId modelId = model.GetModelId();
    //DgnCategoryId categoryId = ToyTileCategory::QueryToyTileCategoryId(db);
    DgnClassId classId = db.Domains().GetClassId(DoorTypeHandler::GetHandler());

    DoorTypePtr element = new DoorType(CreateParams(db, modelId, classId));
    if (!element.IsValid())
        return nullptr;

    return element;
    }

END_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE

