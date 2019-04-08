/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/ArchitecturalPhysicalDomain.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BuildingDomainInternal.h"


BEGIN_BENTLEY_NAMESPACE

namespace ArchitecturalPhysical
	{

	DOMAIN_DEFINE_MEMBERS(ArchitecturalPhysicalDomain)

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	ArchitecturalPhysicalDomain::ArchitecturalPhysicalDomain() : Dgn::DgnDomain(BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME, "Bentley Architectural Physical Domain", 1)
		{
/*		RegisterHandler(ArchitecturalBaseElementHandler::GetHandler());
		RegisterHandler(DoorHandler::GetHandler());
		RegisterHandler(DoorTypeHandler::GetHandler());
		RegisterHandler(WindowHandler::GetHandler());
		RegisterHandler(WindowTypeHandler::GetHandler());
		RegisterHandler(WallHandler::GetHandler());
		RegisterHandler(WallTypeHandler::GetHandler()); */
		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	void ArchitecturalPhysicalDomain::InsertDomainCodeSpecs(Dgn::DgnDbR db)
		{
		Dgn::CodeSpecPtr codeSpec = Dgn::CodeSpec::Create(db, BENTLEY_ARCHITECTURAL_PHYSICAL_AUTHORITY, Dgn::CodeScopeSpec::CreateModelScope());
		if (codeSpec.IsValid())
			codeSpec->Insert();
		}

	/*---------------------------------------------------------------------------------**//**
	* @bsimethod                                   Bentley.Systems
	+---------------+---------------+---------------+---------------+---------------+------*/
	void ArchitecturalPhysicalDomain::_OnSchemaImported(Dgn::DgnDbR dgndb) const
		{

		Dgn::DgnSubCategory::Appearance defaultApperance;
		defaultApperance.SetInvisible(false);

		ArchitecturalPhysicalCategory::InsertDomainCategories(dgndb);
	
		InsertDomainCodeSpecs( dgndb );

		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                                    Bentley.Systems
	//---------------------------------------------------------------------------------------
	void ArchitecturalPhysicalDomain::_OnDgnDbOpened(Dgn::DgnDbR db) const
		{

		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	void ArchitecturalPhysicalCategory::InsertDomainCategories(Dgn::DgnDbR db)
		{
		//Dgn::DgnCategoryId    doorCategoryId       = InsertCategory(db, ARCHITECTURAL_PHYSICAL_CATEGORY_Doors, Dgn::ColorDef::White());
		//Dgn::DgnSubCategoryId doorFrameCategoryId  = InsertSubCategory(db, doorCategoryId, ARCHITECTURAL_PHYSICAL_SUBCATEGORY_Frame, Dgn::ColorDef::White());
		//Dgn::DgnSubCategoryId doorWindowCategoryId = InsertSubCategory(db, doorCategoryId, ARCHITECTURAL_PHYSICAL_SUBCATEGORY_Panel, Dgn::ColorDef::DarkGrey());

		//Dgn::DgnCategoryId    windowCategoryId      = InsertCategory(db, ARCHITECTURAL_PHYSICAL_CATEGORY_Windows, Dgn::ColorDef::White());
		//Dgn::DgnSubCategoryId windowFrameCategoryId = InsertSubCategory(db, windowCategoryId, ARCHITECTURAL_PHYSICAL_SUBCATEGORY_Frame, Dgn::ColorDef::White());
		//Dgn::DgnSubCategoryId windowPanelCategoryId = InsertSubCategory(db, windowCategoryId, ARCHITECTURAL_PHYSICAL_SUBCATEGORY_Panel, Dgn::ColorDef::DarkGrey());

		//Dgn::DgnCategoryId    wallCategoryId = InsertCategory(db, ARCHITECTURAL_PHYSICAL_CATEGORY_Walls, Dgn::ColorDef::White());
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	Dgn::DgnCategoryId ArchitecturalPhysicalCategory::InsertCategory(Dgn::DgnDbR db, Utf8CP codeValue, Dgn::ColorDef const& color)
		{
		Dgn::DgnSubCategory::Appearance appearance;
		appearance.SetColor(color);

		Dgn::SpatialCategory category(db.GetDictionaryModel(),  codeValue, Dgn::DgnCategory::Rank::Domain);
        Dgn::DgnDbStatus status;
		category.Insert(appearance, &status);
        BeAssert(Dgn::DgnDbStatus::Success == status);
		return category.GetCategoryId();
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	Dgn::DgnSubCategoryId ArchitecturalPhysicalCategory::InsertSubCategory(Dgn::DgnDbR db, Dgn::DgnCategoryId categoryId, Utf8CP name, Dgn::ColorDef const& color)
		{
		Dgn::DgnSubCategory::Appearance appearance;
		appearance.SetColor(color);

		Dgn::DgnSubCategoryPtr newSubCategory = new Dgn::DgnSubCategory(Dgn::DgnSubCategory::CreateParams(db, categoryId, name, appearance));
		if (!newSubCategory.IsValid())
			return Dgn::DgnSubCategoryId();

		Dgn::DgnSubCategoryCPtr insertedSubCategory = newSubCategory->Insert();
		if (!insertedSubCategory.IsValid())
			return Dgn::DgnSubCategoryId();

		return insertedSubCategory->GetSubCategoryId();
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	//Dgn::DgnCategoryId ArchitecturalPhysicalCategory::QueryBuildingPhysicalDoorCategoryId(Dgn::DgnDbR db)
	//	{
	//	return Dgn::DgnCategory::QueryCategoryId(db, Dgn::SpatialCategory::CreateCode(db.GetDictionaryModel(), ARCHITECTURAL_PHYSICAL_CATEGORY_Doors));
	//	}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	//Dgn::DgnSubCategoryId ArchitecturalPhysicalCategory::QueryBuildingPhysicalDoorPanelSubCategoryId(Dgn::DgnDbR db)
	//	{
	//	return Dgn::DgnSubCategory::QuerySubCategoryId(db, Dgn::DgnSubCategory::CreateCode(db, QueryBuildingPhysicalDoorCategoryId(db), ARCHITECTURAL_PHYSICAL_SUBCATEGORY_Panel));
	//	}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	//Dgn::DgnSubCategoryId ArchitecturalPhysicalCategory::QueryBuildingPhysicalDoorFrameSubCategoryId(Dgn::DgnDbR db)
	//	{
	//	return Dgn::DgnSubCategory::QuerySubCategoryId(db, Dgn::DgnSubCategory::CreateCode(db, QueryBuildingPhysicalDoorCategoryId(db), ARCHITECTURAL_PHYSICAL_SUBCATEGORY_Frame));
	//	}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	//Dgn::DgnCategoryId ArchitecturalPhysicalCategory::QueryBuildingPhysicalWindowCategoryId(Dgn::DgnDbR db)
	//	{
	//	return Dgn::DgnCategory::QueryCategoryId(db, Dgn::SpatialCategory::CreateCode(db.GetDictionaryModel(), ARCHITECTURAL_PHYSICAL_CATEGORY_Windows));
	//	}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	//Dgn::DgnSubCategoryId ArchitecturalPhysicalCategory::QueryBuildingPhysicalWindowPanelSubCategoryId(Dgn::DgnDbR db)
	//	{
	//	return Dgn::DgnSubCategory::QuerySubCategoryId(db, Dgn::DgnSubCategory::CreateCode(db, QueryBuildingPhysicalWindowCategoryId(db), ARCHITECTURAL_PHYSICAL_SUBCATEGORY_Panel));
	//	}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	//Dgn::DgnSubCategoryId ArchitecturalPhysicalCategory::QueryBuildingPhysicalWindowFrameSubCategoryId(Dgn::DgnDbR db)
	//	{
	//	Dgn::DgnSubCategoryId windowFrameCategoryId = Dgn::DgnSubCategory::QuerySubCategoryId(db, Dgn::DgnSubCategory::CreateCode(db, QueryBuildingPhysicalWindowCategoryId(db), ARCHITECTURAL_PHYSICAL_SUBCATEGORY_Frame));
	//	return windowFrameCategoryId;
	//	}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	//Dgn::DgnCategoryId ArchitecturalPhysicalCategory::QueryBuildingPhysicalWallCategoryId(Dgn::DgnDbR db)
	//	{
	//	return Dgn::DgnCategory::QueryCategoryId(db, Dgn::SpatialCategory::CreateCode(db.GetDictionaryModel(), ARCHITECTURAL_PHYSICAL_CATEGORY_Walls));
	//	}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	Dgn::DgnCategoryId ArchitecturalPhysicalCategory::QueryBuildingPhysicalCategoryId(Dgn::DgnDbR db, Utf8CP categoryName)
		{
		Dgn::DgnCategoryId id =  Dgn::DgnCategory::QueryCategoryId(db, Dgn::SpatialCategory::CreateCode(db.GetDictionaryModel(), categoryName));

		// Create it if is does not exist.

		if (!id.IsValid())
			{
			id = InsertCategory(db, categoryName, Dgn::ColorDef::White());
			}
		return id;
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	Dgn::DgnCategoryId ArchitecturalPhysicalCategory::QueryBuildingDrawingCategoryId(Dgn::DgnDbR db, Utf8CP categoryName)
		{
		Dgn::DgnCategoryId id = Dgn::DgnCategory::QueryCategoryId(db, Dgn::DrawingCategory::CreateCode(db.GetDictionaryModel(), categoryName));

		// Create it if is does not exist.

		if (!id.IsValid())
			{

			Dgn::DgnSubCategory::Appearance appearance;
			appearance.SetColor(Dgn::ColorDef::White());

			Dgn::DrawingCategory category(db.GetDictionaryModel(), categoryName, Dgn::DgnCategory::Rank::Domain);
			category.Insert(appearance);
			id = category.GetCategoryId();

			}
		return id;
		}


	} // End ArchitecturalPhysical namespace

	END_BENTLEY_NAMESPACE

