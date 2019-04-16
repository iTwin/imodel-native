/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "BuildingDomainInternal.h"

BEGIN_BENTLEY_NAMESPACE

namespace ArchitecturalPhysical
	{

	HANDLER_DEFINE_MEMBERS(WallHandler)
	HANDLER_DEFINE_MEMBERS(WallTypeHandler)

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	WallPtr Wall::Create(Dgn::PhysicalModelR model)
		{
		Dgn::DgnDbR db = model.GetDgnDb();
		Dgn::DgnModelId modelId = model.GetModelId();
		Dgn::DgnCategoryId categoryId = ArchitecturalPhysicalCategory::QueryBuildingPhysicalWallCategoryId(db);
		Dgn::DgnClassId classId = db.Domains().GetClassId(WallHandler::GetHandler());

		WallPtr wall = new Wall(CreateParams(db, modelId, classId, categoryId));
		return wall;
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	WallTypePtr WallType::Create(Dgn::DefinitionModelR model)
		{
		Dgn::DgnDbR db = model.GetDgnDb();
		Dgn::DgnModelId modelId = model.GetModelId();
		Dgn::DgnClassId classId = db.Domains().GetClassId(WallTypeHandler::GetHandler());

		WallTypePtr element = new WallType(CreateParams(db, modelId, classId));
		if (!element.IsValid())
			return nullptr;

		return element;
		}

	} // End ArchitecturalPhysical namespace

END_BENTLEY_NAMESPACE

