/*--------------------------------------------------------------------------------------+
|
|     $Source: RoadRailCategory.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailPhysicalInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadRailCategory::InsertDomainCategories(DgnDbR db)
    {
    InsertCategory(db, ColorDef::MediumGrey(), BRRP_CATEGORY_Road);
    InsertCategory(db, ColorDef::MediumGrey(), BRRP_CATEGORY_Track);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadRailCategory::InsertCategory(DgnDbR db, ColorDef const& color, Utf8CP codeValue)
    {
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);

    SpatialCategory category(db, SpatialCategory::CreateCode(codeValue, BRRP_SCHEMA_NAME), DgnCategory::Rank::Domain);
    category.Insert(appearance);

    BeAssert(category.GetCategoryId().IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId RoadRailCategory::QueryCategoryId(DgnDbR db, Utf8CP codeValue)
    {
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(db, SpatialCategory::CreateCode(codeValue, BRRP_SCHEMA_NAME));
    BeAssert(categoryId.IsValid());
    return categoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId RoadRailCategory::GetRoad(DgnDbR db)  { DgnCategoryId categoryId = QueryCategoryId(db, BRRP_CATEGORY_Road);    BeAssert(categoryId.IsValid()); return categoryId; }
DgnCategoryId RoadRailCategory::GetTrack(DgnDbR db) { DgnCategoryId categoryId = QueryCategoryId(db, BRRP_CATEGORY_Track);   BeAssert(categoryId.IsValid()); return categoryId; }