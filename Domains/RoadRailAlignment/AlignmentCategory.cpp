/*--------------------------------------------------------------------------------------+
|
|     $Source: AlignmentCategory.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailAlignmentInternal.h"
#include <RoadRailAlignment/AlignmentCategory.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentCategory::InsertDomainCategories(ConfigurationModelR model)
    {
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(ColorDef::MediumGrey());

    SpatialCategory alignmentCategory(model, BRRA_CATEGORY_Alignment, DgnCategory::Rank::Domain);
    alignmentCategory.Insert(appearance);
    BeAssert(alignmentCategory.GetCategoryId().IsValid());

    DrawingCategory horizontalCategory(model, BRRA_CATEGORY_HorizontalAlignment, DgnCategory::Rank::Domain);
    horizontalCategory.Insert(appearance);
    BeAssert(horizontalCategory.GetCategoryId().IsValid());

    DrawingCategory verticalCategory(model, BRRA_CATEGORY_VerticalAlignment, DgnCategory::Rank::Domain);
    verticalCategory.Insert(appearance);
    BeAssert(verticalCategory.GetCategoryId().IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId AlignmentCategory::QueryDomainCategoryId(SubjectCR subject, Utf8CP codeValue, bool isSpatial)
    {
    auto modelPtr = ConfigurationModel::Query(subject);
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
DgnCategoryId AlignmentCategory::GetAlignment(SubjectCR subject)
    {
    DgnCategoryId categoryId = QueryDomainCategoryId(subject, BRRA_CATEGORY_Alignment, true);
    BeAssert(categoryId.IsValid()); 
    return categoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId AlignmentCategory::GetHorizontal(SubjectCR subject)
    {
    DgnCategoryId categoryId = QueryDomainCategoryId(subject, BRRA_CATEGORY_HorizontalAlignment, false);
    BeAssert(categoryId.IsValid());
    return categoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId AlignmentCategory::GetVertical(SubjectCR subject)
    {
    DgnCategoryId categoryId = QueryDomainCategoryId(subject, BRRA_CATEGORY_VerticalAlignment, false);
    BeAssert(categoryId.IsValid());
    return categoryId;
    }

