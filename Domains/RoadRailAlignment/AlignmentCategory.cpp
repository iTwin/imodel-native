/*--------------------------------------------------------------------------------------+
|
|     $Source: AlignmentCategory.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailAlignmentInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentCategory::InsertDomainCategories(DgnDbR db)
    {
    InsertCategory(db, ColorDef::MediumGrey(), BRRA_CATEGORY_Alignment);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentCategory::InsertCategory(DgnDbR db, ColorDef const& color, Utf8CP codeValue)
    {
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);

    SpatialCategory category(db, SpatialCategory::CreateCode(codeValue, BRRA_SCHEMA_NAME), DgnCategory::Rank::Domain);
    category.Insert(appearance);

    BeAssert(category.GetCategoryId().IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId AlignmentCategory::QueryCategoryId(DgnDbR db, Utf8CP codeValue)
    {
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(db, SpatialCategory::CreateCode(codeValue, BRRA_SCHEMA_NAME));
    BeAssert(categoryId.IsValid());
    return categoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId AlignmentCategory::Get(DgnDbR db)
    {
    DgnCategoryId categoryId = QueryCategoryId(db, BRRA_CATEGORY_Alignment);
    BeAssert(categoryId.IsValid()); 
    return categoryId;
    }