/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/RoadRailCategory.h>
#include <RoadRailPhysical/RoadRailPhysicalDomain.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadRailCategory::InsertDomainCategories(DgnDbR db)
    {
    auto modelPtr = RoadRailAlignmentDomain::QueryCategoryModel(db);
    if (modelPtr.IsNull())
        {
        BeAssert(false);
        }

    InsertSpatialCategory(*modelPtr, BRRP_CATEGORY_Network, ColorDef::White());
    InsertSpatialCategory(*modelPtr, BRRP_CATEGORY_Corridor, ColorDef::White());
    InsertSpatialCategory(*modelPtr, BRRP_CATEGORY_CorridorPortions, ColorDef::White());
    InsertSpatialCategory(*modelPtr, BRRP_CATEGORY_DesignSpeed, ColorDef::White());
    InsertSpatialCategory(*modelPtr, BRRP_CATEGORY_Roadway, ColorDef::White());
    InsertSpatialCategory(*modelPtr, BRRP_CATEGORY_Railway, ColorDef::White());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadRailCategory::InsertSubCategory(DgnCategoryCR category, Utf8CP codeValue, ColorDef const& color)
    {
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);

    DgnSubCategory::CreateParams params(category.GetDgnDb(), category.GetCategoryId(), codeValue, appearance);
    DgnSubCategory subCategory(params);
    subCategory.Insert();

    BeAssert(subCategory.GetSubCategoryId().IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialCategoryCPtr RoadRailCategory::InsertSpatialCategory(DefinitionModelR model, Utf8CP codeValue, ColorDef const& color)
    {
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);

    SpatialCategory category(model, codeValue, DgnCategory::Rank::Domain);
    auto retVal = category.Insert(appearance);

    BeAssert(retVal->GetCategoryId().IsValid());
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DrawingCategoryCPtr RoadRailCategory::InsertDrawingCategory(DefinitionModelR model, Utf8CP codeValue, ColorDef const& color)
    {
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);

    DrawingCategory category(model, codeValue, DgnCategory::Rank::Domain);
    auto retVal = category.Insert(appearance);

    BeAssert(retVal->GetCategoryId().IsValid());

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId RoadRailCategory::QuerySpatialCategoryId(DgnDbR db, Utf8CP codeValue)
    {
    auto modelPtr = RoadRailAlignmentDomain::QueryCategoryModel(db);
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
    auto modelPtr = RoadRailAlignmentDomain::QueryCategoryModel(db);
    if (modelPtr.IsNull())
        return DgnCategoryId();

    DgnCategoryId categoryId = DrawingCategory::QueryCategoryId(*modelPtr, codeValue);
    BeAssert(categoryId.IsValid());
    return categoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId RoadRailCategory::QuerySubCategoryId(DgnDbR dgnDb, DgnCategoryId categoryId, Utf8CP codeValue)
    {
    DgnSubCategoryId subCategoryId = DgnSubCategory::QuerySubCategoryId(dgnDb, DgnSubCategory::CreateCode(dgnDb, categoryId, codeValue));
    BeAssert(subCategoryId.IsValid());
    return subCategoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId RoadRailCategory::GetNetwork(DgnDbR db) { DgnCategoryId categoryId = QuerySpatialCategoryId(db, BRRP_CATEGORY_Network);    BeAssert(categoryId.IsValid()); return categoryId; }
DgnCategoryId RoadRailCategory::GetCorridor(DgnDbR db) { DgnCategoryId categoryId = QuerySpatialCategoryId(db, BRRP_CATEGORY_Corridor);    BeAssert(categoryId.IsValid()); return categoryId; }
DgnCategoryId RoadRailCategory::GetCorridorPortions(DgnDbR db) { DgnCategoryId categoryId = QuerySpatialCategoryId(db, BRRP_CATEGORY_CorridorPortions);    BeAssert(categoryId.IsValid()); return categoryId; }
DgnCategoryId RoadRailCategory::GetDesignSpeed(DgnDbR db) { DgnCategoryId categoryId = QuerySpatialCategoryId(db, BRRP_CATEGORY_DesignSpeed);    BeAssert(categoryId.IsValid()); return categoryId; }
DgnCategoryId RoadRailCategory::GetRoadway(DgnDbR db)  { DgnCategoryId categoryId = QuerySpatialCategoryId(db, BRRP_CATEGORY_Roadway);    BeAssert(categoryId.IsValid()); return categoryId; }
DgnCategoryId RoadRailCategory::GetRailway(DgnDbR db) { DgnCategoryId categoryId = QuerySpatialCategoryId(db, BRRP_CATEGORY_Railway);   BeAssert(categoryId.IsValid()); return categoryId; }
