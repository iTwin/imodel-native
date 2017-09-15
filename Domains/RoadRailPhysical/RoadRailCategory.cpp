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

    InsertSpatialCategory(*modelPtr, BRRP_CATEGORY_Road, ColorDef::MediumGrey());
    InsertSpatialCategory(*modelPtr, BRRP_CATEGORY_Track, ColorDef::MediumGrey());

    InsertDrawingCategory(*modelPtr, BRRP_CATEGORY_TravelwayDefComponent, ColorDef::DarkGreen());
    InsertDrawingCategory(*modelPtr, BRRP_CATEGORY_TravelwaySideDefComponent, ColorDef::MediumGrey());
    InsertDrawingCategory(*modelPtr, BRRP_CATEGORY_TravelwayStructureDefComponent, ColorDef::Green());
    InsertDrawingCategory(*modelPtr, BRRP_CATEGORY_TypicalSectionPoint, ColorDef::Red());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadRailCategory::InsertSpatialCategory(DefinitionModelR model, Utf8CP codeValue, Dgn::ColorDef const& color)
    {
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);

    SpatialCategory category(model, codeValue, DgnCategory::Rank::Domain);
    category.Insert(appearance);

    BeAssert(category.GetCategoryId().IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadRailCategory::InsertDrawingCategory(DefinitionModelR model, Utf8CP codeValue, Dgn::ColorDef const& color)
    {
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);

    DrawingCategory category(model, codeValue, DgnCategory::Rank::Domain);
    category.Insert(appearance);

    BeAssert(category.GetCategoryId().IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId RoadRailCategory::QuerySpatialCategoryId(DgnDbR db, Utf8CP codeValue)
    {
    auto modelPtr = RoadRailCategoryModel::GetModel(db);
    if (modelPtr.IsNull())
        return DgnCategoryId();

    DgnCategoryId categoryId = SpatialCategory::QueryCategoryId(*modelPtr, codeValue);
    BeAssert(categoryId.IsValid());
    return categoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId RoadRailCategory::QueryDrawingCategoryId(DgnDbR db, Utf8CP codeValue)
    {
    auto modelPtr = RoadRailCategoryModel::GetModel(db);
    if (modelPtr.IsNull())
        return DgnCategoryId();

    DgnCategoryId categoryId = DrawingCategory::QueryCategoryId(*modelPtr, codeValue);
    BeAssert(categoryId.IsValid());
    return categoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId RoadRailCategory::GetRoad(DgnDbR db)  { DgnCategoryId categoryId = QuerySpatialCategoryId(db, BRRP_CATEGORY_Road);    BeAssert(categoryId.IsValid()); return categoryId; }
DgnCategoryId RoadRailCategory::GetTrack(DgnDbR db) { DgnCategoryId categoryId = QuerySpatialCategoryId(db, BRRP_CATEGORY_Track);   BeAssert(categoryId.IsValid()); return categoryId; }

DgnCategoryId RoadRailCategory::GetTypicalSectionPoint(DgnDbR db) { DgnCategoryId categoryId = QueryDrawingCategoryId(db, BRRP_CATEGORY_TypicalSectionPoint);    BeAssert(categoryId.IsValid()); return categoryId; }
DgnCategoryId RoadRailCategory::GetTravelwayDefComponent(DgnDbR db) { DgnCategoryId categoryId = QueryDrawingCategoryId(db, BRRP_CATEGORY_TravelwayDefComponent);    BeAssert(categoryId.IsValid()); return categoryId; }
DgnCategoryId RoadRailCategory::GetTravelwayStructureDefComponent(DgnDbR db) { DgnCategoryId categoryId = QueryDrawingCategoryId(db, BRRP_CATEGORY_TravelwayStructureDefComponent);    BeAssert(categoryId.IsValid()); return categoryId; }
DgnCategoryId RoadRailCategory::GetTravelwaySideDefComponent(DgnDbR db) { DgnCategoryId categoryId = QueryDrawingCategoryId(db, BRRP_CATEGORY_TravelwaySideDefComponent);    BeAssert(categoryId.IsValid()); return categoryId; }