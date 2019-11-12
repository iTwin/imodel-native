/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ElectricalDomainInternal.h"


BEGIN_BENTLEY_NAMESPACE

namespace ElectricalPhysical
	{

	DOMAIN_DEFINE_MEMBERS(ElectricalPhysicalDomain)

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	ElectricalPhysicalDomain::ElectricalPhysicalDomain() : Dgn::DgnDomain(BENTLEY_ELECTRICAL_PHYSICAL_SCHEMA_NAME, "Bentley Electrical Physical Domain", 1)
		{
		RegisterHandler(DeviceHandler::GetHandler());
		RegisterHandler(ElectricalPhysicalModelHandler::GetHandler());
		RegisterHandler(ElectricalTypeDefinitionModelHandler::GetHandler());
		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	void ElectricalPhysicalDomain::InsertDomainCodeSpecs(Dgn::DgnDbR db)
		{
		Dgn::CodeSpecPtr codeSpec = Dgn::CodeSpec::Create(db, BENTLEY_ELECTRICAL_PHYSICAL_AUTHORITY, Dgn::CodeScopeSpec::CreateModelScope());
		if (codeSpec.IsValid())
			codeSpec->Insert();
		}

	//----------------------------------------------------------------------------------------
	//@bsimethod                                   Bentley.Systems
	//+---------------+---------------+---------------+---------------+---------------+-------
	void ElectricalPhysicalDomain::_OnSchemaImported(Dgn::DgnDbR dgndb) const
		{
		Dgn::DgnSubCategory::Appearance defaultApperance;
		defaultApperance.SetInvisible(false);

		ElectricalPhysicalCategory::InsertDomainCategories(dgndb);

		InsertDomainCodeSpecs(dgndb);
		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                                    Bentley.Systems
	//---------------------------------------------------------------------------------------
	void ElectricalPhysicalDomain::_OnDgnDbOpened(Dgn::DgnDbR db) const
		{

		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	void ElectricalPhysicalCategory::InsertDomainCategories(Dgn::DgnDbR db)
		{
		Dgn::DgnCategoryId    deviceCategoryId       = InsertCategory(db, ELECTRICAL_PHYSICAL_CATEGORY_Devices, Dgn::ColorDef::White());
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	Dgn::DgnCategoryId ElectricalPhysicalCategory::InsertCategory(Dgn::DgnDbR db, Utf8CP codeValue, Dgn::ColorDef const& color)
		{
		Dgn::DgnSubCategory::Appearance appearance;
		appearance.SetColor(color);

		Dgn::SpatialCategory category(db.GetDictionaryModel(), codeValue, Dgn::DgnCategory::Rank::Domain);
		category.Insert(appearance);
		return category.GetCategoryId();
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	Dgn::DgnSubCategoryId ElectricalPhysicalCategory::InsertSubCategory(Dgn::DgnDbR db, Dgn::DgnCategoryId categoryId, Utf8CP name, Dgn::ColorDef const& color)
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
	Dgn::DgnCategoryId ElectricalPhysicalCategory::QueryElectricalPhysicalDeviceCategoryId(Dgn::DgnDbR db)
		{
		return Dgn::DgnCategory::QueryCategoryId(db, Dgn::SpatialCategory::CreateCode(db.GetDictionaryModel(), ELECTRICAL_PHYSICAL_CATEGORY_Devices));
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	Dgn::DgnCategoryId ElectricalPhysicalCategory::QueryElectricalPhysicalCategoryId(Dgn::DgnDbR db, Utf8CP categoryName)
		{
		Dgn::DgnCategoryId id = Dgn::DgnCategory::QueryCategoryId(db, Dgn::SpatialCategory::CreateCode(db.GetDictionaryModel(), categoryName));

		// Create it if is does not exist.

		if (!id.IsValid())
			{
			id = InsertCategory(db, categoryName, Dgn::ColorDef::White());
			}
		return id;
		}

	} // End ElectricalPhysical namespace

END_BENTLEY_NAMESPACE

