/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/wall.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ArchitecturalPhysicalSchemaInternal.h"
#include <BuildingPhysical/BuildingPhysicalApi.h>


BEGIN_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE

HANDLER_DEFINE_MEMBERS(WallHandler)
HANDLER_DEFINE_MEMBERS(WallTypeHandler)

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
WallPtr Wall::Create(BuildingPhysical::BuildingPhysicalModelR model)
    {
    DgnDbR db = model.GetDgnDb();
    DgnModelId modelId = model.GetModelId();
    DgnCategoryId categoryId = ArchitecturalPhysicalCategory::QueryBuildingPhysicalWallCategoryId(db);
    DgnClassId classId = db.Domains().GetClassId(WallHandler::GetHandler());

    WallPtr wall = new Wall(CreateParams(db, modelId, classId, categoryId));
    return wall;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
WallTypePtr WallType::Create(BuildingPhysical::BuildingTypeDefinitionModelR model)
    {
    DgnDbR db = model.GetDgnDb();
    DgnModelId modelId = model.GetModelId();
    DgnClassId classId = db.Domains().GetClassId(WallTypeHandler::GetHandler());

    WallTypePtr element = new WallType(CreateParams(db, modelId, classId));
    if (!element.IsValid())
        return nullptr;

    return element;
    }

END_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE

