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
    auto categoryPartitionPtr = DefinitionPartition::Create(*db.Elements().GetRootSubject(), GetPartitionName());
    if (categoryPartitionPtr->Insert(&status).IsNull())
        {
        BeAssert(false);
        }

    auto modelPtr = AlignmentCategoryModel::Create(AlignmentCategoryModel::CreateParams(db, categoryPartitionPtr->GetElementId()));

    if (!modelPtr.IsValid() || (DgnDbStatus::Success != modelPtr->Insert()))
        {
        BeAssert(false);
        }

    AlignmentCategory::InsertDomainCategories(db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId AlignmentCategoryModel::GetModelId(DgnDbR db)
    {
    DgnCode partitionCode = DefinitionPartition::CreateCode(*db.Elements().GetRootSubject(), GetPartitionName());
    return db.Models().QuerySubModelId(partitionCode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentCategoryModelPtr AlignmentCategoryModel::GetModel(DgnDbR db)
    {
    AlignmentCategoryModelPtr model = db.Models().Get<AlignmentCategoryModel>(AlignmentCategoryModel::GetModelId(db));
    BeAssert(model.IsValid());
    return model;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentCategory::InsertDomainCategories(DgnDbR db)
    {
    InsertCategory(*AlignmentCategoryModel::GetModel(db), BRRA_CATEGORY_Alignment, ColorDef::MediumGrey());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentCategory::InsertCategory(DefinitionModelR model, Utf8CP codeValue, ColorDef const& color)
    {
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);

    SpatialCategory category(model, codeValue, DgnCategory::Rank::Domain);
    category.Insert(appearance);

    BeAssert(category.GetCategoryId().IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId AlignmentCategory::QueryCategoryId(DgnDbR db, Utf8CP codeValue)
    {
    AlignmentCategoryModelPtr model = AlignmentCategoryModel::GetModel(db);
    if (!model.IsValid())
        return DgnCategoryId();

    DgnCategoryId categoryId = SpatialCategory::QueryCategoryId(*model, codeValue);
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