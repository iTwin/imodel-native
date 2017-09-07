/*--------------------------------------------------------------------------------------+
|
|     $Source: RoadRailCategory.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/RoadRailCategory.h>

HANDLER_DEFINE_MEMBERS(RoadRailCategoryModelHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadRailCategoryModel::SetUp(DgnDbR db)
    {
    DgnDbStatus status;
    auto categoryPartitionPtr = DefinitionPartition::Create(*db.Elements().GetRootSubject(), GetPartitionName());
    if (categoryPartitionPtr->Insert(&status).IsNull())
        {
        BeAssert(false);
        }

    auto modelPtr = RoadRailCategoryModel::Create(RoadRailCategoryModel::CreateParams(db, categoryPartitionPtr->GetElementId()));

    if (!modelPtr.IsValid() || (DgnDbStatus::Success != modelPtr->Insert()))
        {
        BeAssert(false);
        }

    RoadRailCategory::InsertDomainCategories(db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId RoadRailCategoryModel::GetModelId(DgnDbR db)
    {
    DgnCode partitionCode = DefinitionPartition::CreateCode(*db.Elements().GetRootSubject(), GetPartitionName());
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadRailCategory::InsertDomainCategories(DgnDbR db)
    {
    auto modelPtr = RoadRailCategoryModel::GetModel(db);
    if (modelPtr.IsNull())
        {
        BeAssert(false);
        }

    InsertCategory(*modelPtr, BRRP_CATEGORY_Road, ColorDef::MediumGrey());
    InsertCategory(*modelPtr, BRRP_CATEGORY_Track, ColorDef::MediumGrey());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadRailCategory::InsertCategory(DefinitionModelR model, Utf8CP codeValue, Dgn::ColorDef const& color)
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
DgnCategoryId RoadRailCategory::QueryCategoryId(DgnDbR db, Utf8CP codeValue)
    {
    auto modelPtr = RoadRailCategoryModel::GetModel(db);
    if (modelPtr.IsNull())
        return DgnCategoryId();

    DgnCategoryId categoryId = SpatialCategory::QueryCategoryId(*modelPtr, codeValue);
    BeAssert(categoryId.IsValid());
    return categoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId RoadRailCategory::GetRoad(DgnDbR db)  { DgnCategoryId categoryId = QueryCategoryId(db, BRRP_CATEGORY_Road);    BeAssert(categoryId.IsValid()); return categoryId; }
DgnCategoryId RoadRailCategory::GetTrack(DgnDbR db) { DgnCategoryId categoryId = QueryCategoryId(db, BRRP_CATEGORY_Track);   BeAssert(categoryId.IsValid()); return categoryId; }