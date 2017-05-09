/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/ArchitecturalPhysicalDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ArchitecturalPhysicalSchemaInternal.h"

BEGIN_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE

DOMAIN_DEFINE_MEMBERS(ArchitecturalPhysicalDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ArchitecturalPhysicalDomain::ArchitecturalPhysicalDomain() : DgnDomain(BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME, "Bentley Architectural Physical Domain", 1)
    {
    RegisterHandler(ArchitecturalBaseElementHandler::GetHandler());
    RegisterHandler(DoorHandler::GetHandler());
    RegisterHandler(DoorTypeHandler::GetHandler());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void ArchitecturalPhysicalDomain::InsertDomainCodeSpecs(DgnDbR db)
    {
    CodeSpecPtr codeSpec = CodeSpec::Create(db, BENTLEY_ARCHITECTURAL_PHYSICAL_AUTHORITY, Dgn::CodeScopeSpec::CreateModelScope());
    if (codeSpec.IsValid())
        codeSpec->Insert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Bentley.Systems                   
+---------------+---------------+---------------+---------------+---------------+------*/
void ArchitecturalPhysicalDomain::_OnSchemaImported(DgnDbR dgndb) const
    {
  
    DgnSubCategory::Appearance defaultApperance;
    defaultApperance.SetInvisible(false);                        

    ArchitecturalPhysicalCategory::InsertDomainCategories(dgndb);

    InsertDomainCodeSpecs( dgndb );

    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      07/16
//---------------------------------------------------------------------------------------
void ArchitecturalPhysicalDomain::_OnDgnDbOpened(DgnDbR db) const
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void ArchitecturalPhysicalCategory::InsertDomainCategories(DgnDbR db)
    {
    DgnCategoryId categoryId = InsertCategory(db, ARCHITECTURAL_PHYSICAL_CATEGORY_Doors, ColorDef::White());
    InsertSubCategory(db, categoryId, ARCHITECTURAL_PHYSICAL_SUBCATEGORY_Frame, ColorDef::White());
    InsertSubCategory(db, categoryId, ARCHITECTURAL_PHYSICAL_SUBCATEGORY_Panel, ColorDef::DarkGrey());

    categoryId = InsertCategory(db, ARCHITECTURAL_PHYSICAL_CATEGORY_Walls, ColorDef::White());

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
DgnCategoryId ArchitecturalPhysicalCategory::InsertCategory(DgnDbR db, Utf8CP codeValue, ColorDef const& color)
    {
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);

    SpatialCategory category(db, SpatialCategory::CreateCode(db, codeValue, BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME), DgnCategory::Rank::Domain);
    category.Insert(appearance);
    return category.GetCategoryId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
DgnSubCategoryId ArchitecturalPhysicalCategory::InsertSubCategory(DgnDbR db, DgnCategoryId categoryId, Utf8CP name, ColorDef const& color)
    {
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);

    DgnSubCategoryPtr newSubCategory = new DgnSubCategory(DgnSubCategory::CreateParams(db, categoryId, name, appearance));
    if (!newSubCategory.IsValid())
        return DgnSubCategoryId();

    DgnSubCategoryCPtr insertedSubCategory = newSubCategory->Insert();
    if (!insertedSubCategory.IsValid())
        return DgnSubCategoryId();

    return insertedSubCategory->GetSubCategoryId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
DgnCategoryId ArchitecturalPhysicalCategory::QueryBuildingPhysicalDoorCategoryId(DgnDbR db)
    {
    return DgnCategory::QueryCategoryId(db, SpatialCategory::CreateCode(db, ARCHITECTURAL_PHYSICAL_CATEGORY_Doors, BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
DgnSubCategoryId ArchitecturalPhysicalCategory::QueryBuildingPhysicalDoorPanelSubCategoryId(DgnDbR db)
    {
    return DgnSubCategory::QuerySubCategoryId(db, DgnSubCategory::CreateCode(db, QueryBuildingPhysicalDoorCategoryId(db), ARCHITECTURAL_PHYSICAL_SUBCATEGORY_Panel));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
DgnSubCategoryId ArchitecturalPhysicalCategory::QueryBuildingPhysicalDoorFrameSubCategoryId(DgnDbR db)
    {
    return DgnSubCategory::QuerySubCategoryId(db, DgnSubCategory::CreateCode(db, QueryBuildingPhysicalDoorCategoryId(db), ARCHITECTURAL_PHYSICAL_SUBCATEGORY_Frame));
    }



END_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE

