/*--------------------------------------------------------------------------------------+
|
|     $Source: AlignmentCategory.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailAlignmentInternal.h>

HANDLER_DEFINE_MEMBERS(AlignmentCategoryModelHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentCategoryModel::SetUp(DgnDbR db)
    {
    DgnDbStatus status;
    auto domainCategoryPartitionPtr = DefinitionPartition::Create(*db.Elements().GetRootSubject(), GetDomainPartitionName());
    if (domainCategoryPartitionPtr->Insert(&status).IsNull())
        {
        BeAssert(false);
        }

    auto modelPtr = AlignmentCategoryModel::Create(AlignmentCategoryModel::CreateParams(db, domainCategoryPartitionPtr->GetElementId()));
    if (!modelPtr.IsValid() || (DgnDbStatus::Success != modelPtr->Insert()))
        {
        BeAssert(false);
        }

    AlignmentCategory::InsertDomainCategories(db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId AlignmentCategoryModel::GetDomainModelId(DgnDbR db)
    {
    DgnCode partitionCode = DefinitionPartition::CreateCode(*db.Elements().GetRootSubject(), GetDomainPartitionName());
    return db.Models().QuerySubModelId(partitionCode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentCategoryModelPtr AlignmentCategoryModel::GetDomainModel(DgnDbR db)
    {
    AlignmentCategoryModelPtr model = db.Models().Get<AlignmentCategoryModel>(AlignmentCategoryModel::GetDomainModelId(db));
    BeAssert(model.IsValid());
    return model;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentCategory::InsertDomainCategories(DgnDbR db)
    {
    auto domainCategoryModelPtr = AlignmentCategoryModel::GetDomainModel(db);

    DgnSubCategory::Appearance appearance;
    appearance.SetColor(ColorDef::MediumGrey());

    SpatialCategory alignmentCategory(*domainCategoryModelPtr, BRRA_CATEGORY_Alignment, DgnCategory::Rank::Domain);
    alignmentCategory.Insert(appearance);
    BeAssert(alignmentCategory.GetCategoryId().IsValid());

    DrawingCategory horizontalCategory(*domainCategoryModelPtr, BRRA_CATEGORY_HorizontalAlignment, DgnCategory::Rank::Domain);
    horizontalCategory.Insert(appearance);
    BeAssert(horizontalCategory.GetCategoryId().IsValid());

    DrawingCategory verticalCategory(*domainCategoryModelPtr, BRRA_CATEGORY_VerticalAlignment, DgnCategory::Rank::Domain);
    verticalCategory.Insert(appearance);
    BeAssert(verticalCategory.GetCategoryId().IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId AlignmentCategory::QueryDomainCategoryId(DgnDbR db, Utf8CP codeValue, bool isSpatial)
    {
    auto modelPtr = AlignmentCategoryModel::GetDomainModel(db);
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
DgnCategoryId AlignmentCategory::Get(DgnDbR db)
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

