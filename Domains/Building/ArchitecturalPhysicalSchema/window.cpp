/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "BuildingDomainInternal.h"


BEGIN_BENTLEY_NAMESPACE

namespace ArchitecturalPhysical
	{

	HANDLER_DEFINE_MEMBERS(WindowHandler)
	HANDLER_DEFINE_MEMBERS(WindowTypeHandler)

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	WindowPtr Window::Create(Dgn::PhysicalModelR model)
		{
		Dgn::DgnDbR db = model.GetDgnDb();
		Dgn::DgnModelId modelId = model.GetModelId();
		Dgn::DgnCategoryId categoryId = ArchitecturalPhysical::ArchitecturalPhysicalCategory::QueryBuildingPhysicalWindowCategoryId(db);
		Dgn::DgnClassId classId = db.Domains().GetClassId(WindowHandler::GetHandler());

		WindowPtr window = new Window(CreateParams(db, modelId, classId, categoryId));
		return window;
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	WindowTypePtr WindowType::Create(Dgn::DefinitionModelR model)
		{
		Dgn::DgnDbR db = model.GetDgnDb();
		Dgn::DgnModelId modelId = model.GetModelId();
		//DgnCategoryId categoryId = ToyTileCategory::QueryToyTileCategoryId(db);
		Dgn::DgnClassId classId = db.Domains().GetClassId(ArchitecturalPhysical::WindowTypeHandler::GetHandler());

		WindowTypePtr element = new WindowType(CreateParams(db, modelId, classId));
		if (!element.IsValid())
			return nullptr;

		return element;
		}

	} // End ArchitecturalPhysical namespace

END_BENTLEY_NAMESPACE

