/*--------------------------------------------------------------------------------------+
|
|     $Source: RoadRailCategory.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/RoadRailCategory.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadRailCategory::InsertDomainCategories(ConfigurationModelR configurationModel)
    {
    InsertSpatialCategory(configurationModel, BRRP_CATEGORY_Roadway, ColorDef::White());
    InsertSpatialCategory(configurationModel, BRRP_CATEGORY_Railway, ColorDef::White());
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
SpatialCategoryCPtr RoadRailCategory::InsertSpatialCategory(ConfigurationModelR model, Utf8CP codeValue, ColorDef const& color)
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
DrawingCategoryCPtr RoadRailCategory::InsertDrawingCategory(ConfigurationModelR model, Utf8CP codeValue, ColorDef const& color)
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
DgnCategoryId RoadRailCategory::QuerySpatialCategoryId(SubjectCR subject, Utf8CP codeValue)
    {
    auto modelPtr = ConfigurationModel::Query(subject);
    if (modelPtr.IsNull())
        return DgnCategoryId();

    DgnCategoryId categoryId = SpatialCategory::QueryCategoryId(*modelPtr, codeValue);
    BeAssert(categoryId.IsValid());
    return categoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId RoadRailCategory::QueryDrawingCategoryId(SubjectCR subject, Utf8CP codeValue)
    {
    auto modelPtr = ConfigurationModel::Query(subject);
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
DgnCategoryId RoadRailCategory::GetRoadway(SubjectCR subject)  { DgnCategoryId categoryId = QuerySpatialCategoryId(subject, BRRP_CATEGORY_Roadway);    BeAssert(categoryId.IsValid()); return categoryId; }
DgnCategoryId RoadRailCategory::GetRailway(SubjectCR subject) { DgnCategoryId categoryId = QuerySpatialCategoryId(subject, BRRP_CATEGORY_Railway);   BeAssert(categoryId.IsValid()); return categoryId; }
