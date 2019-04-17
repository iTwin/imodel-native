/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailAlignmentInternal.h"
#include <RoadRailAlignment/AlignmentCategory.h>

HANDLER_DEFINE_MEMBERS(RoadRailCategoryModelHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentCategory::InsertDomainCategories(DgnDbR db)
    {
    auto modelPtr = RoadRailCategoryModel::GetModel(db);
    if (modelPtr.IsNull())
        {
        BeAssert(false);
        }

    DgnSubCategory::Appearance appearance;
    appearance.SetColor(ColorDef::MediumGrey());

    SpatialCategory alignmentCategory(*modelPtr, BRRA_CATEGORY_Alignment, DgnCategory::Rank::Domain);
    alignmentCategory.Insert(appearance);
    BeAssert(alignmentCategory.GetCategoryId().IsValid());

    DrawingCategory horizontalCategory(*modelPtr, BRRA_CATEGORY_HorizontalAlignment, DgnCategory::Rank::Domain);
    horizontalCategory.Insert(appearance);
    BeAssert(horizontalCategory.GetCategoryId().IsValid());

    DrawingCategory verticalCategory(*modelPtr, BRRA_CATEGORY_VerticalAlignment, DgnCategory::Rank::Domain);
    verticalCategory.Insert(appearance);
    BeAssert(verticalCategory.GetCategoryId().IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId AlignmentCategory::QueryDomainCategoryId(DgnDbR db, Utf8CP codeValue, bool isSpatial)
    {
    auto modelPtr = RoadRailCategoryModel::GetModel(db);
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
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId AlignmentCategory::GetHorizontal(DgnDbR db)
    {
    DgnCategoryId categoryId = QueryDomainCategoryId(db, BRRA_CATEGORY_HorizontalAlignment, false);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId RoadRailCategoryModel::GetModelId(DgnDbR db)
    {
    DgnCode partitionCode = DefinitionPartition::CreateCode(*db.Elements().GetRootSubject(), RoadRailAlignmentDomain::GetDomainCategoriesPartitionName());
    return db.Models().QuerySubModelId(partitionCode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RoadRailCategoryModelPtr RoadRailCategoryModel::GetModel(DgnDbR db)
    {
    RoadRailCategoryModelPtr model = db.Models().Get<RoadRailCategoryModel>(RoadRailCategoryModel::GetModelId(db));
    BeAssert(model.IsValid());
    return model;
    }
