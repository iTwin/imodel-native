/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/window.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ArchitecturalPhysicalSchemaInternal.h"
#include <BuildingPhysical/BuildingPhysicalApi.h>


BEGIN_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE

HANDLER_DEFINE_MEMBERS(WindowHandler)
HANDLER_DEFINE_MEMBERS(WindowTypeHandler)

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
WindowPtr Window::Create(BuildingPhysical::BuildingPhysicalModelR model)
    {
    DgnDbR db = model.GetDgnDb();
    DgnModelId modelId = model.GetModelId();
    DgnCategoryId categoryId = ArchitecturalPhysicalCategory::QueryBuildingPhysicalWindowCategoryId(db);
    DgnClassId classId = db.Domains().GetClassId(WindowHandler::GetHandler());

    WindowPtr window = new Window(CreateParams(db, modelId, classId, categoryId));
    return window;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
WindowTypePtr WindowType::Create(BuildingPhysical::BuildingTypeDefinitionModelR model)
    {
    DgnDbR db = model.GetDgnDb();
    DgnModelId modelId = model.GetModelId();
    //DgnCategoryId categoryId = ToyTileCategory::QueryToyTileCategoryId(db);
    DgnClassId classId = db.Domains().GetClassId(WindowTypeHandler::GetHandler());

    WindowTypePtr element = new WindowType(CreateParams(db, modelId, classId));
    if (!element.IsValid())
        return nullptr;

    return element;
    }

END_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE

