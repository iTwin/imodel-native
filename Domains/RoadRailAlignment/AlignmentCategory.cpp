/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailAlignmentInternal.h"
#include <RoadRailAlignment/AlignmentCategory.h>
#include <RoadRailAlignment/RoadRailAlignmentDomain.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentCategory::InsertDomainCategories(DgnDbR db)
    {
    auto modelPtr = RoadRailAlignmentDomain::QueryCategoryModel(db);
    if (modelPtr.IsNull())
        {
        BeAssert(false);
        }

    DgnSubCategory::Appearance appearance;
    appearance.SetColor(ColorDef::MediumGrey());

    SpatialCategory alignmentCategory(*modelPtr, BRRA_CATEGORY_Alignment, DgnCategory::Rank::Domain);
    alignmentCategory.Insert(appearance);
    BeAssert(alignmentCategory.GetCategoryId().IsValid());

    SpatialCategory linearCategory(*modelPtr, BRRA_CATEGORY_Linear, DgnCategory::Rank::Domain);
    linearCategory.Insert(appearance);
    BeAssert(linearCategory.GetCategoryId().IsValid());

    DrawingCategory verticalCategory(*modelPtr, BRRA_CATEGORY_VerticalAlignment, DgnCategory::Rank::Domain);
    verticalCategory.Insert(appearance);
    BeAssert(verticalCategory.GetCategoryId().IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId AlignmentCategory::QueryDomainCategoryId(DgnDbR db, Utf8CP codeValue, bool isSpatial)
    {
    auto modelPtr = RoadRailAlignmentDomain::QueryCategoryModel(db);
    if (!modelPtr.IsValid())
        return DgnCategoryId();

    DgnCategoryId categoryId = (isSpatial) ? 
        SpatialCategory::QueryCategoryId(*modelPtr, codeValue) : 
        DrawingCategory::QueryCategoryId(*modelPtr, codeValue);

    BeAssert(categoryId.IsValid());
    return categoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId AlignmentCategory::GetAlignment(DgnDbR db)
    {
    DgnCategoryId categoryId = QueryDomainCategoryId(db, BRRA_CATEGORY_Alignment, true);
    BeAssert(categoryId.IsValid()); 
    return categoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId AlignmentCategory::GetLinear(DgnDbR db)
    {
    DgnCategoryId categoryId = QueryDomainCategoryId(db, BRRA_CATEGORY_Linear, true);
    BeAssert(categoryId.IsValid());
    return categoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId AlignmentCategory::GetVertical(DgnDbR db)
    {
    DgnCategoryId categoryId = QueryDomainCategoryId(db, BRRA_CATEGORY_VerticalAlignment, false);
    BeAssert(categoryId.IsValid());
    return categoryId;
    }